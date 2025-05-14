/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "host_image_copy.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <glm/gtx/transform.hpp>

//![1]
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//![1]

#include <cmath>
#include <fstream>
#include <string>

using namespace KDGpuExample;
using namespace KDGpu;

//![2]
struct ImageData {
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint8_t *pixelData{ nullptr };
    DeviceSize byteSize{ 0 };
    Format format{ Format::R8G8B8A8_UNORM };
};
//![2]

namespace KDGpu {

//![3]
ImageData loadImage(KDUtils::File &file)
{
    int texChannels;
    int _width = 0, _height = 0;

    if (!file.open(std::ios::in | std::ios::binary)) {
        SPDLOG_LOGGER_CRITICAL(KDGpu::Logger::logger(), "Failed to open file {}", file.path());
        throw std::runtime_error("Failed to open file");
    }

    const KDUtils::ByteArray fileContent = file.readAll();
    std::vector<uint32_t> buffer(fileContent.size() / 4);

    auto _data = stbi_load_from_memory(
            fileContent.data(), fileContent.size(), &_width, &_height, &texChannels, STBI_rgb_alpha);

    if (_data == nullptr) {
        SPDLOG_WARN("Failed to load texture {} {}", file.path(), stbi_failure_reason());
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
//![3]

} // namespace KDGpu

void HostImageCopy::initializeScene()
{
    if (!m_adapter->features().hostImageCopy) {
        SPDLOG_ERROR("Adapter does not support HostImageCopy");
    }

    //![10]
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    // Create a buffer to hold the quad vertex data
    {
        const float scale = 0.8f;
        const std::array<Vertex, 4> vertexData = {
            Vertex{ // Bottom-left
                    .position = { -1.0f * scale, 1.0f * scale, 0.0f },
                    .texCoord = { 0.0f, 1.0f } },
            Vertex{ // Bottom-right
                    .position = { 1.0f * scale, 1.0f * scale, 0.0f },
                    .texCoord = { 1.0f, 1.0f } },
            Vertex{ // Top-left
                    .position = { -1.0f * scale, -1.0f * scale, 0.0f },
                    .texCoord = { 0.0f, 0.0f } },
            Vertex{ // Top-right
                    .position = { 1.0f * scale, -1.0f * scale, 0.0f },
                    .texCoord = { 1.0f, 0.0f } }
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
    //![10]

    // Create a texture to hold the image data
    {
        //![4]
        // Load the image data and size
        auto imageFile = KDGpuExample::assetDir().file("textures/samuel-ferrara-1527pjeb6jg-unsplash.jpg");
        ImageData image = loadImage(imageFile);
        //![4]

        //![5]
        // Create Texture compatible with Host Transfers
        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = image.format,
            .extent = { .width = image.width, .height = image.height, .depth = 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::TransferDstBit | KDGpu::TextureUsageFlagBits::HostTransferBit,
            .memoryUsage = MemoryUsage::GpuOnly,
            .initialLayout = TextureLayout::Undefined
        };
        m_texture = m_device.createTexture(textureOptions);
        //![5]

        //![6]
        // Transition the texture to the General Layout on the host
        m_texture.hostLayoutTransition(HostLayoutTransition{
                .oldLayout = TextureLayout::Undefined,
                .newLayout = TextureLayout::General,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });
        //![6]

        //![7]
        // Upload the texture data through the host
        m_texture.copyHostMemoryToTexture(HostMemoryToTextureCopy{
                .dstTextureLayout = TextureLayout::General,
                .regions = {
                        HostMemoryToTextureCopyRegion{
                                .srcHostMemoryPointer = image.pixelData,
                                .dstSubresource = TextureSubresourceLayers{
                                        .aspectMask = TextureAspectFlagBits::ColorBit,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1,
                                },
                                .dstOffset = { .x = 0, .y = 0, .z = 0 },
                                .dstExtent = { image.width, image.height, 1 },
                        },
                },
        });
        //![7]

        //![8]
        // Transition the texture to the ShaderReadOnlyOptimal Layout on the host
        m_texture.hostLayoutTransition(HostLayoutTransition{
                .oldLayout = TextureLayout::General,
                .newLayout = TextureLayout::ShaderReadOnlyOptimal,
                .range = {
                        .aspectMask = TextureAspectFlagBits::ColorBit,
                },
        });
        //![8]

        //![9]
        // Create a view and sampler
        m_textureView = m_texture.createView();
        m_sampler = m_device.createSampler();
        //![9]
    }

    // ![11]
    // Create a vertex shader and fragment shader (spir-v only for now)
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/textured_quad/textured_quad.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/textured_quad/textured_quad.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::CombinedImageSampler,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { bindGroupLayout }
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    const GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_pipelineLayout,
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = sizeof(Vertex) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32_SFLOAT, .offset = sizeof(glm::vec3) } // TexCoord
            }
        },
        .renderTargets = {
            { .format = m_swapchainFormat }
        },
        .depthStencil = {
            .format = m_depthFormat,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        },
        .primitive = {
            .topology = PrimitiveTopology::TriangleStrip
        }
    };
    // clang-format on
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);
    // ![11]

    // Create a bindGroup to hold the uniform containing the texture and sampler
    // clang-format off
    //![12]
    const BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = TextureViewSamplerBinding{ .textureView = m_textureView, .sampler = m_sampler }
        }}
    };
    // clang-format on
    m_textureBindGroup = m_device.createBindGroup(bindGroupOptions);
    //![12]

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    m_opaquePassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                .finalLayout = TextureLayout::PresentSrc
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView,
        }
    };
    // clang-format on
}

void HostImageCopy::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_sampler = {};
    m_textureView = {};
    m_texture = {};
    m_textureBindGroup = {};
    m_commandBuffer = {};
}

void HostImageCopy::updateScene()
{
}

void HostImageCopy::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

//![13]
void HostImageCopy::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setBindGroup(0, m_textureBindGroup);
    opaquePass.draw(DrawCommand{ .vertexCount = 4 });
    renderImGuiOverlay(&opaquePass);
    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
//![13]
