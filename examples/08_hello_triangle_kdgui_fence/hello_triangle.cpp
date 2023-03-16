#include "hello_triangle.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>

#include <fstream>
#include <string>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ToyRenderer {

inline std::string assetPath()
{
#if defined(TOY_RENDERER_ASSET_PATH)
    return TOY_RENDERER_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace ToyRenderer

void HelloTriangle::initializeScene()
{
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
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/08_hello_triangle/hello_triangle.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/08_hello_triangle/hello_triangle.frag.spv";
    auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .pushConstantRanges = { m_transformPushConstantRange }
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

void HelloTriangle::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_commandBuffers = {};
}

void HelloTriangle::updateScene()
{
    updateTransform();
}

void HelloTriangle::render()
{
    // Create a command encoder/recorder
    auto commandRecorder = m_device.createCommandRecorder();

    // Begin render pass - oscillate the clear color just to show something changing.
    updateClearColor();
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    // Bind pipeline
    opaquePass.setPipeline(m_pipeline);

    // Bind vertex buffer
    opaquePass.setVertexBuffer(0, m_buffer);

    // Set Push Constant
    static float angle = 0.0f;
    angle = (angle + 0.1f);
    opaquePass.pushConstant(m_transformPushConstantRange,
                            glm::value_ptr(m_transform));

    // Issue draw command
    const DrawCommand drawCmd = { .vertexCount = 3 };
    opaquePass.draw(drawCmd);

    // End render pass
    opaquePass.end();

    // End recording
    m_commandBuffers[m_inFlightIndex] = commandRecorder.finish();

    // Submit command buffer to queue
    SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffers[m_inFlightIndex] },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] },
        .signalFence = m_frameFences[m_inFlightIndex] // Signal Fence once submission and execution is complete
    };
    m_queue.submit(submitOptions);
}

void HelloTriangle::updateClearColor()
{
    static constexpr int32_t color1[3] = { 30, 64, 175 };
    static constexpr int32_t color2[3] = { 107, 33, 168 };
    static constexpr int32_t deltaColor[3] = {
        color2[0] - color1[0],
        color2[1] - color1[1],
        color2[2] - color1[2]
    };

    // Calculate a sinusoidal interpolation value with a period of 5s.
    const auto timeNS = engine()->simulationTime();
    const std::chrono::nanoseconds periodNS(5'000'000'000);
    const double t = fmod(timeNS.count(), periodNS.count()) / double(periodNS.count());
    const double lambda = 0.5 * (sin(t * 2 * M_PI) + 1.0);
    const uint32_t color[3] = {
        color1[0] + uint32_t(lambda * deltaColor[0]),
        color1[1] + uint32_t(lambda * deltaColor[1]),
        color1[2] + uint32_t(lambda * deltaColor[2])
    };

    m_opaquePassOptions.colorAttachments[0].clearValue.float32[0] = float(color[0]) / 255.0f;
    m_opaquePassOptions.colorAttachments[0].clearValue.float32[1] = float(color[1]) / 255.0f;
    m_opaquePassOptions.colorAttachments[0].clearValue.float32[2] = float(color[2]) / 255.0f;
}

void HelloTriangle::updateTransform()
{
    static float angle = 0.0f;
    angle += 0.1f;
    if (angle > 360.0f)
        angle -= 360.0f;

    m_transform = glm::mat4(1.0f);
    m_transform = glm::rotate(m_transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
}
