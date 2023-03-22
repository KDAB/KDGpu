#include "styled_rectangles.h"

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

void StyledRectangles::initializeScene()
{
    initializeRectangles();
    initializeBackground();

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

void StyledRectangles::initializeRectangles()
{
    // Create a vertex buffer holding a normalized rectangle. We will use this to draw
    // all the rectangles by scaling and offsetting them in the vertex shader according
    // to the unique data in the UBO for each rectangle
    {
        BufferOptions bufferOptions = {
            .size = 4 * 2 * sizeof(float),
            .usage = BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        m_normalizedQuad = m_device.createBuffer(bufferOptions);

        std::array<float, 20> vertexData = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
        };

        auto bufferData = m_normalizedQuad.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        m_normalizedQuad.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/10_styled_rectangles/styled_rectangles.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/10_styled_rectangles/styled_rectangles.frag.spv";
    auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

    // Create a bind group layout for the rectangle data UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0, // GlobalData UBO
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }, {
            .binding = 1, // RectData UBO
            .resourceType = ResourceBindingType::UniformBuffer,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
        }}
    };
    // clang-format on
    const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { bindGroupLayout }
    };
    m_rectPipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_rectPipelineLayout,
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = 2 * sizeof(float) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32_SFLOAT } // Position
            }
        },
        .renderTargets = {
            {
                .format = m_swapchainFormat,
                .blending = { // Enable typical alpha blending
                    .blendingEnabled = true,
                    .color = {
                        .srcFactor = BlendFactor::SrcAlpha,
                        .dstFactor = BlendFactor::OneMinusSrcAlpha
                    }
                }
            }
        },
        .depthStencil = {
            .format = m_depthFormat,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        },
        .primitive = {
            .topology = PrimitiveTopology::TriangleStrip,
            .cullMode = CullModeFlagBits::None
        }
    };
    // clang-format on
    m_rectPipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create a buffer to hold the global data. For now just the viewport dimensions
    {
        BufferOptions bufferOptions = {
            .size = sizeof(glm::vec4), // 1 x vec2 (padded to vec4 by std140)
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_globalBuffer = m_device.createBuffer(bufferOptions);

        const glm::vec2 viewportSize{ m_window->width(), m_window->height() };

        auto bufferData = static_cast<float *>(m_globalBuffer.map());
        std::memcpy(bufferData, glm::value_ptr(viewportSize), sizeof(glm::vec2));
        m_globalBuffer.unmap();
    }

    // Create a buffer to hold the rectangle data. We will just concern ourselves with a single rectangle for now.
    {
        BufferOptions bufferOptions = {
            .size = 3 * sizeof(glm::vec2), // 2 x vec2 + float (padded to vec2 by std140)
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_rectBuffer = m_device.createBuffer(bufferOptions);

        const glm::vec2 offset{ 100.0f, 60.0f };
        const glm::vec2 extent{ 600.0f, 450.0f };
        const float z = 0.2f;

        auto bufferData = static_cast<float *>(m_rectBuffer.map());
        std::memcpy(bufferData, glm::value_ptr(offset), sizeof(glm::vec2));
        std::memcpy(bufferData + 2, glm::value_ptr(extent), sizeof(glm::vec2));
        std::memcpy(bufferData + 4, &z, sizeof(float));
        m_rectBuffer.unmap();
    }

    // Create a bind group for the color stops buffer
    // clang-format off
    BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = BindingResource(UniformBufferBinding{ .buffer = m_globalBuffer })
        }, {
            .binding = 1,
            .resource = BindingResource(UniformBufferBinding{ .buffer = m_rectBuffer })
        }}
    };
    // clang-format on
    m_rectBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void StyledRectangles::initializeBackground()
{
    // Create a buffer to hold a full screen quad. This will be drawn as a triangle-strip (see pipeline creation below).
    {
        BufferOptions bufferOptions = {
            .size = 4 * (3 + 2) * sizeof(float),
            .usage = BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_fullScreenQuad = m_device.createBuffer(bufferOptions);

        // NB: The z coord is now 1.0 to push it to the far plane
        std::array<float, 20> vertexData = {
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f
        };

        auto bufferData = m_fullScreenQuad.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        m_fullScreenQuad.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/10_styled_rectangles/gradient_blobs.vert.spv";
    auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/10_styled_rectangles/gradient_blobs.frag.spv";
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
    m_bgPipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_bgPipelineLayout,
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
            .depthCompareOperation = CompareOperation::LessOrEqual // Allow to pass when z = 1.0
        },
        .primitive = {
            .topology = PrimitiveTopology::TriangleStrip
        }
    };
    // clang-format on
    m_bgPipeline = m_device.createGraphicsPipeline(pipelineOptions);

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
            .resource = BindingResource(UniformBufferBinding{ .buffer = m_colorStopsBuffer })
        }}
    };
    // clang-format on
    m_colorStopsBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void StyledRectangles::cleanupScene()
{
    m_rectPipeline = {};
    m_rectPipelineLayout = {};
    m_rectBindGroup = {};
    m_rectBuffer = {};
    m_globalBuffer = {};
    m_normalizedQuad = {};

    m_bgPipeline = {};
    m_bgPipelineLayout = {};
    m_fullScreenQuad = {};
    m_colorStopsBindGroup = {};
    m_colorStopsBuffer = {};
    m_commandBuffer = {};
}

void StyledRectangles::updateScene()
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

void StyledRectangles::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_renderPassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void StyledRectangles::render()
{
    auto commandRecorder = m_device.createCommandRecorder();
    m_renderPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto renderPass = commandRecorder.beginRenderPass(m_renderPassOptions);

    // Draw the background
    renderPass.setPipeline(m_bgPipeline);
    renderPass.setBindGroup(0, m_colorStopsBindGroup);
    renderPass.setVertexBuffer(0, m_fullScreenQuad);
    renderPass.draw(DrawCommand{ .vertexCount = 4 });

    // Draw the rectangle last (as we are alpha blending)
    renderPass.setPipeline(m_rectPipeline);
    renderPass.setBindGroup(0, m_rectBindGroup);
    renderPass.setVertexBuffer(0, m_normalizedQuad);
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
