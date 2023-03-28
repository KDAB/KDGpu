#include "gradient_blobs.h"

#include <KDGpu_KDGui/engine.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void GradientBlobs::initializeScene()
{
    // Create a buffer to hold a full screen quad. This will be drawn as a triangle-strip (see pipeline creation below).
    {
        BufferOptions bufferOptions = {
            .size = 4 * (3 + 2) * sizeof(float),
            .usage = BufferUsageFlagBits::VertexBufferBit,
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
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/09_gradient_blobs/gradient_blobs.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpu::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/09_gradient_blobs/gradient_blobs.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpu::readShaderFile(fragmentShaderPath));

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

    // Create a buffer to hold the color stops. The data will be uploaded in updateScene()
    {
        BufferOptions bufferOptions = {
            .size = 8 * sizeof(glm::vec4), // 4 x vec4 + 4 x vec2 (padded to vec4 by std140)
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_colorStopsBuffer = m_device.createBuffer(bufferOptions);
    }

    // Create a bind group for the color stops buffer
    // clang-format off
    BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_colorStopsBuffer }
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
    // Calculate the new color stop positions from the animation data
    const float t = engine()->simulationTime().count() / 1.0e9;
    m_p0 = m_p0Anim.evaluate(t);
    m_p1 = m_p1Anim.evaluate(t);
    m_p2 = m_p2Anim.evaluate(t);
    m_p3 = m_p3Anim.evaluate(t);

    // Upload the color stops
    // clang-format off
    auto bufferData = static_cast<float *>(m_colorStopsBuffer.map());
    std::memcpy(bufferData,      glm::value_ptr(m_color0), sizeof(glm::vec4));
    std::memcpy(bufferData +  4, glm::value_ptr(m_color1), sizeof(glm::vec4));
    std::memcpy(bufferData +  8, glm::value_ptr(m_color2), sizeof(glm::vec4));
    std::memcpy(bufferData + 12, glm::value_ptr(m_color3), sizeof(glm::vec4));
    std::memcpy(bufferData + 16, glm::value_ptr(m_p0), sizeof(glm::vec2));
    std::memcpy(bufferData + 20, glm::value_ptr(m_p1), sizeof(glm::vec2));
    std::memcpy(bufferData + 24, glm::value_ptr(m_p2), sizeof(glm::vec2));
    std::memcpy(bufferData + 28, glm::value_ptr(m_p3), sizeof(glm::vec2));
    m_colorStopsBuffer.unmap();
    // clang-format on
}

void GradientBlobs::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_renderPassOptions.depthStencilAttachment.view = m_depthTextureView;
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
