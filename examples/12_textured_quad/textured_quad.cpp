#include "textured_quad.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/bind_group_layout_options.h>
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>
#include <toy_renderer/texture_options.h>

#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <fstream>
#include <string>

struct ImageData {
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint8_t *pixelData{ nullptr };
    DeviceSize byteSize{ 0 };
    Format format{ Format::R8G8B8A8_UNORM };
};

namespace ToyRenderer {

inline std::string assetPath()
{
#if defined(TOY_RENDERER_ASSET_PATH)
    return TOY_RENDERER_ASSET_PATH;
#else
    return "";
#endif
}

ImageData loadImage(const std::string &path)
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

} // namespace ToyRenderer

void TexturedQuad::initializeScene()
{
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
        BufferOptions bufferOptions = {
            .size = dataByteSize,
            .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };
        m_buffer = m_device.createBuffer(bufferOptions);
        uploadBufferData(m_buffer,
                         PipelineStageFlags(PipelineStageFlagBit::VertexAttributeInputBit),
                         AccessFlags(AccessFlagBit::VertexAttributeReadBit),
                         vertexData.data(),
                         dataByteSize);
    }

    // Create a texture to hold the image data
    {
        // Load the image data and size
        ImageData image = loadImage(ToyRenderer::assetPath() + "/textures/samuel-ferrara-1527pjeb6jg-unsplash.jpg");

        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = image.format,
            .extent = { .width = image.width, .height = image.height, .depth = 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlags(TextureUsageFlagBits::SampledBit) | TextureUsageFlags(TextureUsageFlagBits::TransferDstBit),
            .memoryUsage = MemoryUsage::GpuOnly,
            .initialLayout = TextureLayout::Undefined
        };
        m_texture = m_device.createTexture(textureOptions);

        // Upload the texture data and transition to ShaderReadOnlyOptimal
        uploadTextureData(m_texture,
                          PipelineStageFlags(PipelineStageFlagBit::AllGraphicsBit),
                          AccessFlags(AccessFlagBit::MemoryReadBit),
                          image.pixelData,
                          image.byteSize,
                          Extent3D{ .width = image.width, .height = image.height, .depth = 1 },
                          Offset3D{},
                          TextureLayout::Undefined,
                          TextureLayout::ShaderReadOnlyOptimal);

        // Create a view and sampler
        const TextureViewOptions viewOptions = {
            .range = { .aspectMask = TextureAspectFlags(TextureAspectFlagBits::ColorBit) }
        };
        m_textureView = m_texture.createView(viewOptions);
        m_sampler = m_device.createSampler();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/12_textured_quad/textured_quad.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/12_textured_quad/textured_quad.frag.spv";
    auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

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
    GraphicsPipelineOptions pipelineOptions = {
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

    // Create a bindGroup to hold the uniform containing the texture and sampler
    // clang-format off
    BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = TextureViewBinding{ .textureView = m_textureView, .sampler = m_sampler }
        }}
    };
    // clang-format on
    m_textureBindGroup = m_device.createBindGroup(bindGroupOptions);

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

void TexturedQuad::cleanupScene()
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

void TexturedQuad::updateScene()
{
}

void TexturedQuad::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void TexturedQuad::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setBindGroup(0, m_textureBindGroup);
    opaquePass.draw(DrawCommand{ .vertexCount = 4 });
    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
