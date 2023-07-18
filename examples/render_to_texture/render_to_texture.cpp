/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_to_texture.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/imgui_item.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>

#include <imgui.h>

#include <glm/gtx/transform.hpp>

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

void RenderToTexture::initializeScene()
{
    using std::placeholders::_1;
    auto func = std::bind(&RenderToTexture::drawControls, this, _1);
    registerImGuiOverlayDrawFunction(func);

    initializeMainScene();
    initializePostProcess();

    // Set up the options for the 2 render passes:
    // Pass 1: Render main scene into the color texture
    // Pass 2: Render a full screen quad that samples from the color texture from pass 1

    // clang-format off
    //![2]
    m_opaquePassOptions = {
        .colorAttachments = {
            {
                .view = m_colorOutputView, // We always render to the color texture
                .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                .finalLayout = TextureLayout::ShaderReadOnlyOptimal
            }
        },
        .depthStencilAttachment = {
            .view = m_depthTextureView
        }
    };
    //![2]
    // clang-format on

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    // clang-format off
    m_finalPassOptions = {
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

    m_filterPosData.resize(sizeof(float));
}

void RenderToTexture::initializeMainScene()
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        const BufferOptions bufferOptions = {
            .size = 3 * sizeof(Vertex), // 3 vertices * 2 attributes * 3 float components
            .usage = BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_buffer = m_device.createBuffer(bufferOptions);

        const float r = 0.8f;
        std::array<Vertex, 3> vertexData;
        vertexData[0] = { { r * std::cos(7.0f * M_PI / 6.0f), -r * std::sin(7.0f * M_PI / 6.0f), 0.0f }, { 1.0f, 0.0, 0.0f } }; // Bottom-left, red
        vertexData[1] = { { r * std::cos(11.0f * M_PI / 6.0f), -r * std::sin(11.0f * M_PI / 6.0f), 0.0f }, { 0.0f, 1.0, 0.0f } }; // Bottom-right, green
        vertexData[2] = { { 0.0f, -r, 0.0f }, { 0.0f, 0.0, 1.0f } }; // Top, blue

        auto bufferData = m_buffer.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(Vertex));
        m_buffer.unmap();
    }

    // Create a buffer to hold the geometry index data
    {
        const BufferOptions bufferOptions = {
            .size = 3 * sizeof(uint32_t),
            .usage = BufferUsageFlagBits::IndexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };
        m_indexBuffer = m_device.createBuffer(bufferOptions);
        std::vector<uint32_t> indexData = { 0, 1, 2 };
        auto bufferData = m_indexBuffer.map();
        std::memcpy(bufferData, indexData.data(), indexData.size() * sizeof(uint32_t));
        m_indexBuffer.unmap();
    }

    // Create a buffer to hold the transformation matrix
    {
        const BufferOptions bufferOptions = {
            .size = sizeof(glm::mat4),
            .usage = BufferUsageFlagBits::UniformBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        };
        m_transformBuffer = m_device.createBuffer(bufferOptions);

        // Upload identify matrix
        m_transform = glm::mat4(1.0f);

        auto bufferData = m_transformBuffer.map();
        std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
        m_transformBuffer.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/render_to_texture/rotating_triangle.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpu::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/render_to_texture/rotating_triangle.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpu::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding a UBO
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
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
                { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) } // Color
            }
        },
        .renderTargets = {
            { .format = m_colorFormat }
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
    const BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = UniformBufferBinding{ .buffer = m_transformBuffer }
        }}
    };
    // clang-format on
    m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void RenderToTexture::initializePostProcess()
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

    // Create a color texture we can render to in the 1st pass
    createOffscreenTexture();

    //![1]
    // Create a sampler we can use to sample from the color texture in the final pass
    m_colorOutputSampler = m_device.createSampler();
    //![1]

    // Create a vertex shader and fragment shader (spir-v only for now)
    //![3]
    const auto vertexShaderPath = KDGpu::assetPath() + "/shaders/examples/render_to_texture/desaturate.vert.spv";
    auto vertexShader = m_device.createShaderModule(KDGpu::readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = KDGpu::assetPath() + "/shaders/examples/render_to_texture/desaturate.frag.spv";
    auto fragmentShader = m_device.createShaderModule(KDGpu::readShaderFile(fragmentShaderPath));
    //![3]

    // Create bind group layout consisting of a single binding holding the texture the 1st pass rendered to
    // clang-format off
    //![4]
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::CombinedImageSampler,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
        }}
    };
    //![4]
    // clang-format on
    m_colorBindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    // clang-format off
    //![5]
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { m_colorBindGroupLayout },
        .pushConstantRanges = { m_filterPosPushConstantRange }
    };
    //![5]
    // clang-format on
    m_postProcessPipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    // clang-format off
    const GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = m_postProcessPipelineLayout,
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
        //![6]
        .primitive = {
            .topology = PrimitiveTopology::TriangleStrip
        }
        //![6]
    };
    // clang-format on
    m_postProcessPipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create BindGroup to bind the ColorTexture to our final pass shader for sampling
    updateColorBindGroup();
}

//![7]
void RenderToTexture::createOffscreenTexture()
{
    const TextureOptions colorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_window->width(), m_window->height(), 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::SampledBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_colorOutput = m_device.createTexture(colorTextureOptions);
    m_colorOutputView = m_colorOutput.createView();
}
//![7]

//![8]
void RenderToTexture::updateColorBindGroup()
{
    // Create a bindGroup to hold the Offscreen Color Texture
    // clang-format off
    const BindGroupOptions bindGroupOptions = {
        .layout = m_colorBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = TextureViewSamplerBinding{ .textureView = m_colorOutputView, .sampler = m_colorOutputSampler }
        }}
    };
    // clang-format on
    m_colorBindGroup = m_device.createBindGroup(bindGroupOptions);
}
//![8]

void RenderToTexture::drawControls(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(ImVec2(10, 150));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin(
            "About",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    ImGui::Text("Renders a colorful triangle and then post-processes it.");
    ImGui::End();
}

void RenderToTexture::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_buffer = {};
    m_indexBuffer = {};
    m_transformBindGroup = {};
    m_transformBuffer = {};
    m_fullScreenQuad = {};
    m_colorBindGroup = {};
    m_colorBindGroupLayout = {};
    m_colorOutputSampler = {};
    m_colorOutputView = {};
    m_colorOutput = {};
    m_postProcessPipeline = {};
    m_postProcessPipelineLayout = {};
    m_commandBuffer = {};
}

void RenderToTexture::updateScene()
{
    // Each frame we want to rotate the triangle a little
    static float angle = 0.0f;
    angle += 0.01f;
    if (angle > 360.0f)
        angle -= 360.0f;

    m_transform = glm::mat4(1.0f);
    m_transform = glm::rotate(m_transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

    auto bufferData = m_transformBuffer.map();
    std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
    m_transformBuffer.unmap();

    //![9]
    const float t = engine()->simulationTime().count() / 1.0e9;
    m_filterPos = 0.5f * (std::sin(t) + 1.0f);
    std::memcpy(m_filterPosData.data(), &m_filterPos, sizeof(float));
    //![9]
}

void RenderToTexture::resize()
{
    // Recreate Offscreen Color Texture and View with new size
    createOffscreenTexture();

    // Update OpaquePassOptions to reference new views
    m_opaquePassOptions.colorAttachments[0].view = m_colorOutputView;
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;

    // We need to update the ColorBindGroup so that it also references the new colorOutputView
    updateColorBindGroup();

    // Update FinalPass to reference new depthView (colorAttachment is handled in render)
    m_finalPassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void RenderToTexture::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    //![10]
    // Pass 1: Color pass
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.setBindGroup(0, m_transformBindGroup);
    opaquePass.drawIndexed(DrawIndexedCommand{ .indexCount = 3 });
    opaquePass.end();

    // Pass 2: Post process
    m_finalPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto finalPass = commandRecorder.beginRenderPass(m_finalPassOptions);
    finalPass.setPipeline(m_postProcessPipeline);
    finalPass.setVertexBuffer(0, m_fullScreenQuad);
    finalPass.setBindGroup(0, m_colorBindGroup);
    finalPass.pushConstant(m_filterPosPushConstantRange, m_filterPosData.data());
    finalPass.draw(DrawCommand{ .vertexCount = 4 });
    renderImGuiOverlay(&finalPass);
    finalPass.end();
    //![10]

    // Finalize the command recording
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
