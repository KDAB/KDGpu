#include "gradient_blobs.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/bind_group_layout_options.h>
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>
#include <toy_renderer/texture_options.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
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

} // namespace ToyRenderer

void GradientBlobs::initializeScene()
{
    // Create a buffer to hold a full screen quad. This will be drawn as a triangle-strip (see pipeline creation below).
    {
        BufferOptions bufferOptions = {
            .size = 4 * (3 + 2) * sizeof(float),
            .usage = BufferUsageFlags(BufferUsageFlagBits::VertexBufferBit), // TODO: Use a nice Flags template class
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_fullScreenQuad = m_device.createBuffer(bufferOptions);

        std::array<float, 20> vertexData = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f
        };

        auto bufferData = m_fullScreenQuad.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        m_fullScreenQuad.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/09_gradient_blobs/gradient_blobs.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/09_gradient_blobs/gradient_blobs.frag.spv";
    auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

    // Create a bind group layout for the color stops UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::UniformBuffer,
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
                { .binding = 0, .stride = (3 + 2) * sizeof(float) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT },                          // Position
                { .location = 1, .binding = 0, .format = Format::R32G32_SFLOAT, .offset = 3 * sizeof(float) } // Texture coords
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

    // Create a buffer to hold the color stops
    {
        BufferOptions bufferOptions = {
            .size = 8 * sizeof(glm::vec4), // 4 x vec4 + 4 x vec2 (padded to vec4 by std140)
            .usage = BufferUsageFlags(BufferUsageFlagBits::UniformBufferBit),
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_colorStopsBuffer = m_device.createBuffer(bufferOptions);

        // Upload the color stops
        const glm::vec4 color0(190.0f / 255.0f, 186.0f / 255.0f, 255.0f / 255.0f, 1.0); // Top-left
        const glm::vec4 color1(230.0f / 255.0f, 161.0f / 255.0f, 243.0f / 255.0f, 1.0); // Top-right
        const glm::vec4 color2(143.0f / 255.0f, 143.0f / 255.0f, 245.0f / 255.0f, 1.0); // Bottom-left
        const glm::vec4 color3(189.0f / 255.0f, 153.0f / 255.0f, 246.0f / 255.0f, 1.0); // Bottom-right
        const glm::vec2 p0(0.35f, 0.20f); // Top-left
        const glm::vec2 p1(0.95f, 0.05f); // Top-right
        const glm::vec2 p2(0.05f, 0.90f); // Bottom-left
        const glm::vec2 p3(0.80f, 0.85f); // Bottom-right

        // clang-format off
        auto bufferData = static_cast<float *>(m_colorStopsBuffer.map());
        std::memcpy(bufferData,      glm::value_ptr(color0), sizeof(glm::vec4));
        std::memcpy(bufferData + 4,  glm::value_ptr(color1), sizeof(glm::vec4));
        std::memcpy(bufferData + 8,  glm::value_ptr(color2), sizeof(glm::vec4));
        std::memcpy(bufferData + 12, glm::value_ptr(color3), sizeof(glm::vec4));
        std::memcpy(bufferData + 16, glm::value_ptr(p0), sizeof(glm::vec2));
        std::memcpy(bufferData + 20, glm::value_ptr(p1), sizeof(glm::vec2));
        std::memcpy(bufferData + 24, glm::value_ptr(p2), sizeof(glm::vec2));
        std::memcpy(bufferData + 28, glm::value_ptr(p3), sizeof(glm::vec2));
        m_colorStopsBuffer.unmap();
        // clang-format on
    }

    // Create a bind group for the color stops buffer
    // clang-format off
    BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = BindingResource(UniformBufferBinding{ .buffer = m_colorStopsBuffer })
        }}
    };
    // clang-format on
    m_colorStopsBindGroup = m_device.createBindGroup(bindGroupOptions);

    // clang-format off
    m_renderPassOptions = {
        .colorAttachments = {
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                .finalLayout = TextureLayout::PresentSrc
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView
        }
    };
    // clang-format on
}

void GradientBlobs::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_fullScreenQuad = {};
    m_colorStopsBindGroup = {};
    m_colorStopsBuffer = {};
    m_commandBuffer = {};
}

void GradientBlobs::updateScene()
{
}

void GradientBlobs::render()
{
    auto commandRecorder = m_device.createCommandRecorder();
    m_renderPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto renderPass = commandRecorder.beginRenderPass(m_renderPassOptions);
    renderPass.setPipeline(m_pipeline);
    renderPass.setBindGroup(0, m_colorStopsBindGroup);
    renderPass.setVertexBuffer(0, m_fullScreenQuad);
    renderPass.draw(DrawCommand{ .vertexCount = 4 });
    renderPass.end();

    m_commandBuffer = commandRecorder.finish();
    SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
