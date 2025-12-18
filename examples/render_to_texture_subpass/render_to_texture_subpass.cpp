/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_to_texture_subpass.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>
#include <KDGpuExample/imgui_item.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/render_pass_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/memory_barrier.h>

#include <imgui.h>

#include <glm/gtx/transform.hpp>

#include <cmath>
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

using namespace KDGpu;

void RenderToTextureSubpass::initializeScene()
{
    registerImGuiOverlayDrawFunction([](ImGuiContext *ctx) {
        RenderToTextureSubpass::drawControls(ctx);
    });

    createRenderPass();
    initializeMainScene();
    initializePostProcess();

    // Set up the options for the 2 render passes:
    // Pass 1: Render main scene into the color texture
    // Pass 2: Render a full screen quad that samples from the color texture from pass 1

    // clang-format off
    m_renderPassOptions = {
        .renderPass = m_renderPass.handle(),
        .attachments = {
             {
                .view = m_colorOutputView, // We always render to the color texture
                .color = Attachment::ColorOperations {
                    .clearValue = ColorClearValue{ 0.0f, 0.0f, 0.0f, 1.0f },
                },
            },
            {
                .view = {}, // Not setting the swapchain texture view just yet
                .color = Attachment::ColorOperations {
                    .clearValue = ColorClearValue{ 0.3f, 0.3f, 0.3f, 1.0f },
                    .layout = TextureLayout::ColorAttachmentOptimal,
                },
            }
        },
    };
    // clang-format on

    m_filterPosData.resize(sizeof(float));
}

void RenderToTextureSubpass::createRenderPass()
{
    // attachment 1: color attachment(used as output for subpass 1 and input for subpass 2)
    // attachment 2: color attachment for presenting(output for subpass 2)
    const std::vector<AttachmentDescription> attachmentDescriptions{
        AttachmentDescription{
                .format = m_colorFormat,
                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
        },
        AttachmentDescription{
                .format = m_swapchainFormat,
                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
                .finalLayout = TextureLayout::PresentSrc }
    };

    const std::vector<SubpassDescription> subpassDescriptions{
        SubpassDescription{
                .colorAttachmentReference = { { 0 } },
        },
        SubpassDescription{
                .inputAttachmentReference = { { 0 } },
                .colorAttachmentReference = { { 1 } },
        }
    };

    // First dependency ensure that the previous renderpass must finish before it can write output to attachment 0
    // Second dependency ensure that subpass 1 wait for subpass 0 to finish writing to attachment 0 before it reads it
    const std::vector<SubpassDependenciesDescriptions> dependencyDescriptions{
        SubpassDependenciesDescriptions{
                .srcSubpass = ExternalSubpass,
                .dstSubpass = 0,
                .dstStageMask = PipelineStageFlagBit::ColorAttachmentOutputBit,
                .dstAccessMask = AccessFlagBit::ColorAttachmentReadBit | AccessFlagBit::ColorAttachmentWriteBit },
        SubpassDependenciesDescriptions{
                .srcSubpass = 0,
                .dstSubpass = 1,
                .srcStageMask = PipelineStageFlagBit::ColorAttachmentOutputBit,
                .dstStageMask = PipelineStageFlagBit::ColorAttachmentOutputBit | PipelineStageFlagBit::FragmentShaderBit,
                .srcAccessMask = AccessFlagBit::ColorAttachmentWriteBit,
                .dstAccessMask = AccessFlagBit::InputAttachmentReadBit | AccessFlagBit::ColorAttachmentWriteBit | AccessFlagBit::ColorAttachmentReadBit,
        }
    };

    const RenderPassOptions renderPassInfo = {
        .attachments = attachmentDescriptions,
        .subpassDescriptions = subpassDescriptions,
        .subpassDependencies = dependencyDescriptions
    };

    m_renderPass = m_device.createRenderPass(renderPassInfo);
}

void RenderToTextureSubpass::initializeMainScene()
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

        auto *bufferData = m_buffer.map();
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
        auto *bufferData = m_indexBuffer.map();
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

        auto *bufferData = m_transformBuffer.map();
        std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
        m_transformBuffer.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture_subpass/rotating_triangle.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture_subpass/rotating_triangle.frag.spv");
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
        .renderPass = m_renderPass.handle(),
        .subpassIndex = 0
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

void RenderToTextureSubpass::initializePostProcess()
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

        auto *bufferData = m_fullScreenQuad.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
        m_fullScreenQuad.unmap();
    }

    // Create a color texture we can render to in the 1st pass
    createOffscreenTexture();

    // Create a vertex shader and fragment shader (spir-v only for now)
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture_subpass/desaturate.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture_subpass/desaturate.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding the texture the 1st pass rendered to
    // clang-format off
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {{
            .binding = 0,
            .resourceType = ResourceBindingType::InputAttachment,
            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
        }}
    };
    // clang-format on
    m_colorBindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    // clang-format off
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { m_colorBindGroupLayout },
        .pushConstantRanges = { m_filterPosPushConstantRange }
    };
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
        .primitive = {
            .topology = PrimitiveTopology::TriangleStrip
        },
        .renderPass = m_renderPass.handle(),
        .subpassIndex = 1
    };
    // clang-format on
    m_postProcessPipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create BindGroup to bind the ColorTexture to our final pass shader for sampling
    updateColorBindGroup();
}

void RenderToTextureSubpass::createOffscreenTexture()
{
    const TextureOptions colorTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_colorFormat,
        .extent = { m_swapchainExtent.width, m_swapchainExtent.height, 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::InputAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_colorOutput = m_device.createTexture(colorTextureOptions);
    m_colorOutputView = m_colorOutput.createView();
}

void RenderToTextureSubpass::updateColorBindGroup()
{
    // Create a bindGroup to hold the Offscreen Color Texture
    // clang-format off
    const BindGroupOptions bindGroupOptions = {
        .layout = m_colorBindGroupLayout,
        .resources = {{
            .binding = 0,
            .resource = InputAttachmentBinding{ .textureView = m_colorOutputView}
        }}
    };
    // clang-format on
    m_colorBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void RenderToTextureSubpass::drawControls(ImGuiContext *ctx)
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

void RenderToTextureSubpass::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_renderPass = {};
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

void RenderToTextureSubpass::updateScene()
{
    // Each frame we want to rotate the triangle a little
    static float angle = 0.0f;
    angle += 0.01f;
    if (angle > 360.0f)
        angle -= 360.0f;

    m_transform = glm::mat4(1.0f);
    m_transform = glm::rotate(m_transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

    auto *bufferData = m_transformBuffer.map();
    std::memcpy(bufferData, &m_transform, sizeof(glm::mat4));
    m_transformBuffer.unmap();

    const float t = engine()->simulationTime().count() / 1.0e9;
    m_filterPos = 0.5f * (std::sin(t) + 1.0f);
    std::memcpy(m_filterPosData.data(), &m_filterPos, sizeof(float));
}

void RenderToTextureSubpass::resize()
{
    // Recreate Offscreen Color Texture and View with new size
    createOffscreenTexture();

    // Update OpaquePassOptions to reference new views
    m_renderPassOptions.attachments[0].view = m_colorOutputView;

    // We need to update the ColorBindGroup so that it also references the new colorOutputView
    updateColorBindGroup();
}

void RenderToTextureSubpass::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    // Pass 1: Color pass
    m_renderPassOptions.attachments[1].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

    auto opaquePass = commandRecorder.beginRenderPass(m_renderPassOptions);
    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.setBindGroup(0, m_transformBindGroup);
    opaquePass.drawIndexed(DrawIndexedCommand{ .indexCount = 3 });
    opaquePass.nextSubpass();

    // Pass 2: Post process
    opaquePass.setPipeline(m_postProcessPipeline);
    opaquePass.setVertexBuffer(0, m_fullScreenQuad);
    opaquePass.setBindGroup(0, m_colorBindGroup);
    opaquePass.pushConstant(m_filterPosPushConstantRange, m_filterPosData.data());
    opaquePass.draw(DrawCommand{ .vertexCount = 4 });
    renderImGuiOverlay(&opaquePass, m_inFlightIndex, &m_renderPass, 1);

    opaquePass.end();

    // Finalize the command recording
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}
