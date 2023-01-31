#include "hello_triangle.h"

#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>

#include <fstream>
#include <string>

namespace ToyRenderer {

inline std::string assetPath()
{
#if defined(TOY_RENDERER_ASSET_PATH)
    return TOY_RENDERER_ASSET_PATH;
#else
    return "";
#endif
}

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file");

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / 4);
    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(fileSize));
    file.close();
    return buffer;
}

} // namespace ToyRenderer

void HelloTriangle::onAttached()
{
    ExampleEngineLayer::onAttached();

    // Create a buffer to hold triangle vertex data
    BufferOptions bufferOptions = {
        .size = 3 * 2 * 4 * sizeof(float), // 3 vertices * 2 attributes * 4 float components
        .usage = BufferUsageFlags(BufferUsageFlagBits::VertexBufferBit), // TODO: Use a nice Flags template class
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    m_buffer = m_device.createBuffer(bufferOptions);

    // clang-format off
    std::vector<float> vertexData = {
         1.0f, -1.0f, 0.0f, 1.0f, // position
         1.0f,  0.0f, 0.0f, 1.0f, // color
        -1.0f, -1.0f, 0.0f, 1.0f, // position
         0.0f,  1.0f, 0.0f, 1.0f, // color
         0.0f,  1.0f, 0.0f, 1.0f, // position
         0.0f,  0.0f, 1.0f, 1.0f, // color
    };
    // clang-format on
    auto bufferData = m_buffer.map();
    std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
    m_buffer.unmap();

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/02_hello_triangle/hello_triangle.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/02_hello_triangle/hello_triangle.frag.spv";
    auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

    // Create a pipeline layout (array of bind group layouts)
    auto pipelineLayout = m_device.createPipelineLayout();

    // Create a pipeline
    // clang-format off
    GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = pipelineLayout.handle(),
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = 2 * 4 * sizeof(float) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32A32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = 4 * sizeof(float) } // Color
            }
        },
        .renderTargets = {
            { .format = m_swapchainFormat }
        },
        .depthStencil = {
            .format = m_depthFormat,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        }
    };
    // clang-format on
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    RenderPassOptions opaquePassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f }
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView.handle(),
        }
    };
    // clang-format on
}

void HelloTriangle::onDetached()
{
    ExampleEngineLayer::onDetached();
}

// TODO: Implement the render loop
