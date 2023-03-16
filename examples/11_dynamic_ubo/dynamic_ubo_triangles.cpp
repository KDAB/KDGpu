#include "dynamic_ubo_triangles.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/bind_group_layout_options.h>
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>

#include <glm/gtx/transform.hpp>

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

namespace {
constexpr uint32_t entityCount = 4;
}

void DynamicUBOTriangles::initializeScene()
{
    // Create a buffer to hold triangle vertex data
    {
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
    }

    // Create a buffer to hold the geometry index data
    {
        BufferOptions bufferOptions = {
            .size = 3 * sizeof(uint32_t),
            .usage = BufferUsageFlags(BufferUsageFlagBits::IndexBufferBit),
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        m_indexBuffer = m_device.createBuffer(bufferOptions);
        std::vector<uint32_t> indexData = { 0, 1, 2 };
        auto bufferData = m_indexBuffer.map();
        std::memcpy(bufferData, indexData.data(), indexData.size() * sizeof(uint32_t));
        m_indexBuffer.unmap();
    }

    // Create a buffer to hold dynamic transformation matrix
    {
        // Retrieve minimum buffer offset alignment
        const size_t minDynamicUBOOffsetAlignment = m_device.adapter()->properties().limits.minUniformBufferOffsetAlignment;
        m_dynamicUBOByteStride = std::max(minDynamicUBOOffsetAlignment, sizeof(glm::mat4));

        BufferOptions bufferOptions = {
            .size = entityCount * m_dynamicUBOByteStride,
            .usage = BufferUsageFlags(BufferUsageFlagBits::UniformBufferBit),
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_transformDynamicUBOBuffer = m_device.createBuffer(bufferOptions);
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/11_dynamic_ubo/dynamic_ubo.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/11_dynamic_ubo/dynamic_ubo.frag.spv";
    auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::DynamicUniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
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

    // Create a bindGroup to hold the UBO with the transform
    // clang-format off
    BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            // We are dealing with a Dynamic UBO expected to hold
            // a set of transform matrces. The size we specify for the binding is the size of a single entry in the buffer
            .resource = DynamicUniformBufferBinding{ .buffer = m_transformDynamicUBOBuffer, .size = uint32_t(m_dynamicUBOByteStride) }
        }}
    };
    // clang-format on
    m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);

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

void DynamicUBOTriangles::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_commandBuffer = {};
    m_transformBindGroup = {};
    m_transformDynamicUBOBuffer = {};
}

void DynamicUBOTriangles::updateScene()
{
    // Each frame we want to rotate the triangle a little
    static float angle = 0.0f;
    angle += 0.1f;
    if (angle > 360.0f)
        angle -= 360.0f;

    std::vector<uint8_t> rawTransformData(entityCount * m_dynamicUBOByteStride, 0U);

    // Update EntityCount matrices into the single buffer we have
    for (size_t i = 0; i < entityCount; ++i) {
        auto transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(-0.7f + i * 0.5f, 0.0f, 0.0f));
        transform = glm::scale(transform, glm::vec3(0.2f));
        transform = glm::rotate(transform, glm::radians(angle + 45.0f * i), glm::vec3(0.0f, 0.0f, 1.0f));

        std::memcpy(rawTransformData.data() + i * m_dynamicUBOByteStride, &transform, sizeof(glm::mat4));
    }

    auto bufferData = m_transformDynamicUBOBuffer.map();
    std::memcpy(bufferData, rawTransformData.data(), rawTransformData.size());
    m_transformDynamicUBOBuffer.unmap();
}

void DynamicUBOTriangles::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);

    for (size_t i = 0; i < entityCount; ++i) {
        // Bind Group and provide offset into the Dynamic UBO that holds all the transform matrices
        const uint32_t dynamicUBOOffset = i * m_dynamicUBOByteStride;
        opaquePass.setBindGroup(0, m_transformBindGroup, m_pipelineLayout, { dynamicUBOOffset });
        const DrawIndexedCommand drawCmd = { .indexCount = 3 };
        opaquePass.drawIndexed(drawCmd);
    }

    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
