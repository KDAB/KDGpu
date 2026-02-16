/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_to_texture.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>
#include <KDGpuExample/imgui_item.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/memory_barrier.h>

#include <imgui.h>

#include <glm/gtx/transform.hpp>

#include <cmath>
#include <fstream>
#include <string>

using namespace KDGpu;

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

    // Options will be created inline in the render() function

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
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture/rotating_triangle.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture/rotating_triangle.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

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

    //![sampler_creation]
    // Create a sampler we can use to sample from the color texture in the final pass
    m_colorOutputSampler = m_device.createSampler();
    //![sampler_creation]

    // Create a vertex shader and fragment shader (spir-v only for now)
    //![shader_loading]
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture/desaturate.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture/desaturate.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));
    //![shader_loading]

    // Create bind group layout consisting of a single binding holding the texture the 1st pass rendered to
    // clang-format off
    //![bind_group_layout]
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::CombinedImageSampler,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
        }}
    };
    //![bind_group_layout]
    // clang-format on
    m_colorBindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    // clang-format off
    //![pipeline_layout]
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { m_colorBindGroupLayout },
        .pushConstantRanges = { m_filterPosPushConstantRange }
    };
    //![pipeline_layout]
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
        //![triangle_strip]
        .primitive = {
            .topology = PrimitiveTopology::TriangleStrip
        }
        //![triangle_strip]
    };
    // clang-format on
    m_postProcessPipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create BindGroup to bind the ColorTexture to our final pass shader for sampling
    updateColorBindGroup();
}

//![offscreen_texture]
void RenderToTexture::createOffscreenTexture()
{
    const TextureOptions colorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_swapchainExtent.width, m_swapchainExtent.height, 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::SampledBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_colorOutput = m_device.createTexture(colorTextureOptions);
    m_colorOutputView = m_colorOutput.createView();
}
//![offscreen_texture]

//![color_bind_group]
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
//![color_bind_group]

void RenderToTexture::drawControls(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(ImVec2(10, 170), ImGuiCond_FirstUseEver);
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

    //![filter_animation]
    const float t = engine()->simulationTime().count() / 1.0e9;
    m_filterPos = 0.5f * (std::sin(t) + 1.0f);
    std::memcpy(m_filterPosData.data(), &m_filterPos, sizeof(float));
    //![filter_animation]
}

void RenderToTexture::resize()
{
    // Recreate Offscreen Color Texture and View with new size
    createOffscreenTexture();

    // We need to update the ColorBindGroup so that it also references the new colorOutputView
    updateColorBindGroup();
}

void RenderToTexture::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    //![render_loop]
    //![first_pass]
    // Pass 1: Color pass
    auto opaquePass = commandRecorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = m_colorOutputView, // We always render to the color texture
                            .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
            },
    });
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.setBindGroup(0, m_transformBindGroup);
    opaquePass.drawIndexed(DrawIndexedCommand{ .indexCount = 3 });
    opaquePass.end();
    //![first_pass]

    // Wait for Pass1 writes to offscreen texture to have been completed
    // Transition it to a shader read only layout
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::ColorAttachmentOutputBit,
            .srcMask = AccessFlagBit::ColorAttachmentWriteBit,
            .dstStages = PipelineStageFlagBit::FragmentShaderBit,
            .dstMask = AccessFlagBit::ShaderReadBit,
            .oldLayout = TextureLayout::ColorAttachmentOptimal,
            .newLayout = TextureLayout::ShaderReadOnlyOptimal,
            .texture = m_colorOutput,
            .range = {
                    .aspectMask = TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });

    // Wait for Pass1 writes to depth texture to have been completed
    commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::AllGraphicsBit,
            .srcMask = AccessFlagBit::DepthStencilAttachmentWriteBit,
            .dstStages = PipelineStageFlagBit::TopOfPipeBit,
            .dstMask = AccessFlagBit::None,
            .oldLayout = TextureLayout::DepthStencilAttachmentOptimal,
            .newLayout = TextureLayout::DepthStencilAttachmentOptimal,
            .texture = m_depthTexture,
            .range = {
                    .aspectMask = TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit,
                    .levelCount = 1,
            },
    });

    // Pass 2: Post process
    auto finalPass = commandRecorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = m_swapchainViews.at(m_currentSwapchainImageIndex),
                            .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                            .finalLayout = TextureLayout::PresentSrc,
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
            },
    });
    finalPass.setPipeline(m_postProcessPipeline);
    finalPass.setVertexBuffer(0, m_fullScreenQuad);
    finalPass.setBindGroup(0, m_colorBindGroup);
    finalPass.pushConstant(m_filterPosPushConstantRange, m_filterPosData.data());
    finalPass.draw(DrawCommand{ .vertexCount = 4 });
    renderImGuiOverlay(&finalPass);
    finalPass.end();
    //![render_loop]

    // Finalize the command recording
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}
