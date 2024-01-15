/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "bindgroup_partially_bound.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/device.h>
#include <KDGpu/adapter.h>

#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <fstream>
#include <string>

namespace KDGpu {

inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDGpu

namespace {

struct ImageData {
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint8_t *pixelData{ nullptr };
    DeviceSize byteSize{ 0 };
    Format format{ Format::R8G8B8A8_UNORM };
};

struct TextureInfoPushConstant {
    glm::vec2 viewportSize;
    VkBool32 useTexture;
};

ImageData
loadImage(const std::string &path)
{
    int texChannels;
    int _width = 0, _height = 0;
    std::string texturePath = path;
#ifdef PLATFORM_WIN32
    // STB fails to load if path is /C:/... instead of C:/...
    if (texturePath.rfind("/", 0) == 0)
        texturePath = texturePath.substr(1);
#endif
    auto _data = stbi_load(texturePath.c_str(), &_width, &_height, &texChannels, STBI_rgb_alpha);
    if (_data == nullptr) {
        SPDLOG_WARN("Failed to load texture {} {}", path, stbi_failure_reason());
        return {};
    }
    SPDLOG_DEBUG("Texture dimensions: {} x {}", _width, _height);

    return ImageData{
        .width = static_cast<uint32_t>(_width),
        .height = static_cast<uint32_t>(_height),
        .pixelData = static_cast<uint8_t *>(_data),
        .byteSize = 4 * static_cast<DeviceSize>(_width) * static_cast<DeviceSize>(_height)
    };
}

} // namespace

BindGroupPartiallyBound::BindGroupPartiallyBound()
    : SimpleExampleEngineLayer()
{
}

void BindGroupPartiallyBound::initializeScene()
{
    // Check that our device actually supports the Vulkan Partially Bound Descriptor features
    const AdapterFeatures &features = m_device.adapter()->features();
    if (!features.bindGroupBindingPartiallyBound) {
        SPDLOG_CRITICAL("Partially Bound BindGroup is not supported, can't run this example");
        exit(0);
    }

    ////// TRIANGLE MESH //////
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        const float r = 0.8f;
        const std::array<Vertex, 3> vertexData = {
            Vertex{ // Bottom-left, red
                    .position = { r * cosf(7.0f * M_PI / 6.0f), -r * sinf(7.0f * M_PI / 6.0f), 0.0f },
                    .color = { 1.0f, 0.0f, 0.0f } },
            Vertex{ // Bottom-right, green
                    .position = { r * cosf(11.0f * M_PI / 6.0f), -r * sinf(11.0f * M_PI / 6.0f), 0.0f },
                    .color = { 0.0f, 1.0f, 0.0f } },
            Vertex{ // Top, blue
                    .position = { 0.0f, -r, 0.0f },
                    .color = { 0.0f, 0.0f, 1.0f } }
        };

        const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
        const BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_buffer = m_device.createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_buffer,
            .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
            .dstMask = AccessFlagBit::VertexAttributeReadBit,
            .data = vertexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    // Create a buffer to hold the geometry index data
    {
        const std::array<uint32_t, 3> indexData = { 0, 1, 2 };
        const DeviceSize dataByteSize = indexData.size() * sizeof(uint32_t);
        const BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::IndexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_indexBuffer = m_device.createBuffer(bufferOptions);
        const BufferUploadOptions uploadOptions = {
            .destinationBuffer = m_indexBuffer,
            .dstStages = PipelineStageFlagBit::IndexInputBit,
            .dstMask = AccessFlagBit::IndexReadBit,
            .data = indexData.data(),
            .byteSize = dataByteSize
        };
        uploadBufferData(uploadOptions);
    }

    ////// TEXTURE //////
    //![1]
    // Create a texture
    {
        // Load the image data and size
        ImageData image = loadImage(KDGpu::assetPath() + "/textures/samuel-ferrara-1527pjeb6jg-unsplash.jpg");

        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = image.format,
            .extent = { .width = image.width, .height = image.height, .depth = 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly,
            .initialLayout = TextureLayout::Undefined
        };
        m_texture = m_device.createTexture(textureOptions);

        // Upload the texture data and transition to ShaderReadOnlyOptimal
        // clang-format off
        const std::vector<BufferTextureCopyRegion> regions = {{
            .textureSubResource = { .aspectMask = TextureAspectFlagBits::ColorBit },
            .textureExtent = { .width = image.width, .height = image.height, .depth = 1 }
        }};
        // clang-format on
        const TextureUploadOptions uploadOptions = {
            .destinationTexture = m_texture,
            .dstStages = PipelineStageFlagBit::AllGraphicsBit,
            .dstMask = AccessFlagBit::MemoryReadBit,
            .data = image.pixelData,
            .byteSize = image.byteSize,
            .oldLayout = TextureLayout::Undefined,
            .newLayout = TextureLayout::ShaderReadOnlyOptimal,
            .regions = regions
        };
        uploadTextureData(uploadOptions);

        // Create a view and sampler
        m_textureView = m_texture.createView();
        m_sampler = m_device.createSampler();
    }
    //![1]

    ////// PIPELINE //////
    // Create a vertex shader and fragment shader
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/bindgroup_partially_bound/bindgroup_partially_bound.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/bindgroup_partially_bound/bindgroup_partially_bound.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    //![2]
    // Create a bind group layout with the PartiallyBoundBit
    m_textureBindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = ResourceBindingType::CombinedImageSampler,
                            .shaderStages = ShaderStageFlagBits::FragmentBit,
                            .flags = { ResourceBindingFlagBits::PartiallyBoundBit },
                    },
            },
    });
    //![2]

    //![3]
    m_textureBindGroup = m_device.createBindGroup(BindGroupOptions{
            .layout = m_textureBindGroupLayout,
    });
    //![3]

    // Create PushConstants
    //![4]
    m_transformCountPushConstant = PushConstantRange{
        .offset = 0,
        .size = sizeof(glm::mat4),
        .shaderStages = ShaderStageFlagBits::VertexBit
    };

    m_textureInUsePushConstant = PushConstantRange{
        .offset = sizeof(glm::mat4),
        .size = sizeof(TextureInfoPushConstant),
        .shaderStages = ShaderStageFlagBits::FragmentBit
    };
    //![4]

    //![5]
    // Create a pipeline layout (array of bind group layouts)
    m_pipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
            .bindGroupLayouts = { m_textureBindGroupLayout },
            .pushConstantRanges = { m_transformCountPushConstant, m_textureInUsePushConstant },
    });

    // Create a pipeline
    m_pipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit },
            },
            .layout = m_pipelineLayout,
            .vertex = {
                    .buffers = {
                            { .binding = 0, .stride = sizeof(Vertex) },
                    },
                    .attributes = {
                            { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                            { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) } // Color
                    },
            },
            .renderTargets = {
                    {
                            .format = m_swapchainFormat,
                    },
            },
            .depthStencil = {
                    .format = m_depthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
    });
    //![5]

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    m_opaquePassOptions = {
        .colorAttachments = {
                { .view = {},
                  .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                  .finalLayout = TextureLayout::PresentSrc } },
        .depthStencilAttachment = {
                .view = m_depthTextureView,
        },
    };
}

void BindGroupPartiallyBound::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_texture = {};
    m_textureView = {};
    m_sampler = {};
    m_textureBindGroup = {};
    m_textureBindGroupLayout = {};
    m_commandBuffer = {};
}

//![6]
void BindGroupPartiallyBound::updateScene()
{
    static float angle = 0.0f;
    const float angularSpeed = 3.0f; // degrees per second
    const float dt = engine()->deltaTimeSeconds();
    angle += angularSpeed * dt;
    if (angle > 360.0f)
        angle -= 360.0f;

    m_transform = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
}
//![6]

void BindGroupPartiallyBound::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void BindGroupPartiallyBound::render()
{
    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

    //![7]
    static uint32_t frameCounter = 0;
    const VkBool32 useTexture = frameCounter++ > 1000;

    if (useTexture) {
        // Only bind the Texture to the BindGroup after a while
        // hence the Partially Bound Bind Group
        m_textureBindGroup.update({
                .binding = 0,
                .resource = TextureViewSamplerBinding{
                        .textureView = m_textureView,
                        .sampler = m_sampler,
                },
        });
    }
    //![7]

    //![8]
    auto commandRecorder = m_device.createCommandRecorder();
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    // Push Constant
    opaquePass.pushConstant(m_transformCountPushConstant, &m_transform);

    const TextureInfoPushConstant textureInfoPushConstant{
        .viewportSize = { float(m_window->width()), float(m_window->height()) },
        .useTexture = useTexture,
    };
    opaquePass.pushConstant(m_textureInUsePushConstant, &textureInfoPushConstant);
    // Bind bindGroups
    opaquePass.setBindGroup(0, m_textureBindGroup);

    const DrawIndexedCommand drawCmd = { .indexCount = 3 };
    opaquePass.drawIndexed(drawCmd);

    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();
    //![8]

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
