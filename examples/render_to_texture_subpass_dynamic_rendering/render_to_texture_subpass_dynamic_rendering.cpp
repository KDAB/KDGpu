/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_to_texture_subpass_dynamic_rendering.h"

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

using namespace KDGpu;

void RenderToTextureSubpassDynamicRendering::initializeScene()
{
    using std::placeholders::_1;
    auto func = std::bind(&RenderToTextureSubpassDynamicRendering::drawControls, this, _1);
    registerImGuiOverlayDrawFunction(func);

    initializeMainScene();
    initializePostProcess();

    // Set up the options for the 2 passes:
    // Pass 1: Render main scene into the color texture
    // Pass 2: Render a full screen quad that samples from the color texture from pass 1

    m_dynamicRenderPassOptions = RenderPassCommandRecorderWithDynamicRenderingOptions{
        .colorAttachments = {
                {
                        // Offscreen Texture (Pass 1)
                        .view = m_colorOutputView, // We always render to the color texture
                        .clearValue = ColorClearValue{ 0.0f, 0.0f, 0.0f, 1.0f },
                        .layout = TextureLayout::DynamicLocalRead,
                },
                {
                        // Swapchain Output (Pass 2)
                        .view = {}, // Not setting the swapchain texture view just yet
                        .clearValue = ColorClearValue{ 0.3f, 0.3f, 0.3f, 1.0f },
                        .layout = TextureLayout::ColorAttachmentOptimal,
                },
        },
    };

    m_filterPosData.resize(sizeof(float));
}

void RenderToTextureSubpassDynamicRendering::initializeMainScene() // Pass 1
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
    const GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
                { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
        .layout = m_pipelineLayout,
        .vertex = {
                .buffers = {
                        { .binding = 0, .stride = sizeof(Vertex) },
                },
                .attributes = {
                        { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                        { .location = 1, .binding = 0, .format = Format::R32G32B32_SFLOAT, .offset = sizeof(glm::vec3) }, // Color
                },
        },
        .renderTargets = {
                // We need to specify all our RenderTarget even if we will only target 1
                { .format = m_colorFormat },
                { .format = m_swapchainFormat },
        },
        .dynamicRendering = {
                .enabled = true, // Mark that we want to use it with dynamic rendering
                .dynamicInputLocations = DynamicInputAttachmentLocations{
                        // Specify that we have no input attachments
                        .inputColorAttachments = {
                                {
                                        .enabled = false,
                                },
                                {
                                        .enabled = false,
                                },
                        },
                },
                .dynamicOutputLocations = DynamicOutputAttachmentLocations{
                        // Specify that we want frag output[0] to write only to color attachment[0]
                        .outputAttachments = {
                                {
                                        .enabled = true,
                                        .remappedIndex = 0,
                                },
                                {
                                        .enabled = false,
                                },
                        },
                },
        },
    };
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create a bindGroup to hold the UBO with the transform
    const BindGroupOptions bindGroupOptions = {
        .layout = bindGroupLayout,
        .resources = {
                {
                        .binding = 0,
                        .resource = UniformBufferBinding{ .buffer = m_transformBuffer },
                },
        }
    };
    m_transformBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void RenderToTextureSubpassDynamicRendering::initializePostProcess() // Pass 2
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

    // Create a vertex shader and fragment shader (spir-v only for now)
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture_subpass/desaturate.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/render_to_texture_subpass/desaturate.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create bind group layout consisting of a single binding holding the texture the 1st pass rendered to
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {
                {
                        .binding = 0,
                        .resourceType = ResourceBindingType::InputAttachment,
                        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                },
        }
    };
    m_colorBindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

    // Create a pipeline layout (array of bind group layouts)
    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { m_colorBindGroupLayout },
        .pushConstantRanges = { m_filterPosPushConstantRange },
    };
    m_postProcessPipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    const GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
                { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
        .layout = m_postProcessPipelineLayout,
        .vertex = {
                .buffers = { { .binding = 0, .stride = (3 + 2) * sizeof(float) } },
                .attributes = {
                        { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Position
                        { .location = 1, .binding = 0, .format = Format::R32G32_SFLOAT, .offset = 3 * sizeof(float) } // Texture coords
                },
        },
        .renderTargets = {
                // We need to specify all our RenderTarget even if we will only target 1
                { .format = m_colorFormat },
                { .format = m_swapchainFormat },
        },
        .primitive = { .topology = PrimitiveTopology::TriangleStrip },
        .dynamicRendering = {
                .enabled = true, // Mark that we want to use it with dynamic rendering
                .dynamicInputLocations = DynamicInputAttachmentLocations{
                        // Specify that we want color attachment[0] to be fed as input attachment[0]
                        .inputColorAttachments = {
                                {
                                        .enabled = true,
                                        .remappedIndex = 0,
                                },
                                {
                                        .enabled = false,
                                },
                        },
                },
                .dynamicOutputLocations = DynamicOutputAttachmentLocations{
                        // Specify that we want frag output[0] to write only to color attachment[1]
                        .outputAttachments = {
                                {
                                        .enabled = false,
                                },
                                {
                                        .enabled = true,
                                        .remappedIndex = 0,
                                },
                        },
                },
        },
    };
    m_postProcessPipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Create BindGroup to bind the ColorTexture to our final pass shader for sampling
    updateColorBindGroup();
}

void RenderToTextureSubpassDynamicRendering::createOffscreenTexture()
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

void RenderToTextureSubpassDynamicRendering::updateColorBindGroup()
{
    // Create a bindGroup to hold the Offscreen Color Texture
    const BindGroupOptions bindGroupOptions = {
        .layout = m_colorBindGroupLayout,
        .resources = {
                {
                        .binding = 0,
                        .resource = InputAttachmentBinding{
                                .textureView = m_colorOutputView,
                                .layout = TextureLayout::DynamicLocalRead,
                        },
                },
        },
    };
    m_colorBindGroup = m_device.createBindGroup(bindGroupOptions);
}

void RenderToTextureSubpassDynamicRendering::drawControls(ImGuiContext *ctx)
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

void RenderToTextureSubpassDynamicRendering::cleanupScene()
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

void RenderToTextureSubpassDynamicRendering::updateScene()
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

    const float t = engine()->simulationTime().count() / 1.0e9;
    m_filterPos = 0.5f * (std::sin(t) + 1.0f);
    std::memcpy(m_filterPosData.data(), &m_filterPos, sizeof(float));
}

void RenderToTextureSubpassDynamicRendering::resize()
{
    // Recreate Offscreen Color Texture and View with new size
    createOffscreenTexture();

    // Update OpaquePassOptions to reference new views
    m_dynamicRenderPassOptions.colorAttachments[0].view = m_colorOutputView;

    // We need to update the ColorBindGroup so that it also references the new colorOutputView
    updateColorBindGroup();
}

void RenderToTextureSubpassDynamicRendering::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    // We need to perform a layout transition since dynamic rendering doesn't perform implicit initial layout transition
    // like using RenderPasses would
    //![3]
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::TopOfPipeBit,
            .srcMask = AccessFlagBit::None,
            .dstStages = PipelineStageFlagBit::ColorAttachmentOutputBit,
            .dstMask = AccessFlagBit::ColorAttachmentWriteBit,
            .oldLayout = TextureLayout::Undefined,
            .newLayout = TextureLayout::DynamicLocalRead,
            .texture = m_colorOutput,
            .range = TextureSubresourceRange{
                    .aspectMask = TextureAspectFlagBits::ColorBit,
            },
    });
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::TopOfPipeBit,
            .srcMask = AccessFlagBit::None,
            .dstStages = PipelineStageFlagBit::ColorAttachmentOutputBit,
            .dstMask = AccessFlagBit::ColorAttachmentWriteBit,
            .oldLayout = TextureLayout::Undefined,
            .newLayout = TextureLayout::ColorAttachmentOptimal,
            .texture = m_swapchain.textures().at(m_currentSwapchainImageIndex),
            .range = TextureSubresourceRange{
                    .aspectMask = TextureAspectFlagBits::ColorBit,
            },
    });
    //![3]

    m_dynamicRenderPassOptions.colorAttachments[1].view = m_swapchainViews.at(m_currentSwapchainImageIndex);

    auto opaquePass = commandRecorder.beginRenderPass(m_dynamicRenderPassOptions);

    // Pass 1: Color pass

    // fragOutput[0] maps to ColorAttachment[0] -> m_colorOutput

    opaquePass.setPipeline(m_pipeline);
    opaquePass.setVertexBuffer(0, m_buffer);
    opaquePass.setIndexBuffer(m_indexBuffer);
    opaquePass.setBindGroup(0, m_transformBindGroup);
    opaquePass.drawIndexed(DrawIndexedCommand{ .indexCount = 3 });

    commandRecorder.memoryBarrier(MemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::ColorAttachmentOutputBit,
            .dstStages = PipelineStageFlagBit::FragmentShaderBit,
            .memoryBarriers = {
                    {
                            .srcMask = AccessFlagBit::ColorAttachmentWriteBit,
                            .dstMask = AccessFlagBit::InputAttachmentReadBit,
                    },
            },
    });

    // Pass 2: Post process

    // input attachment[0] maps to ColorAttachment[0] -> m_swapchainOutput
    // fragOutput[0] maps to ColorAttachment[1] -> m_swapchainOutput

    opaquePass.setPipeline(m_postProcessPipeline);
    opaquePass.setVertexBuffer(0, m_fullScreenQuad);
    opaquePass.setBindGroup(0, m_colorBindGroup);
    opaquePass.pushConstant(m_filterPosPushConstantRange, m_filterPosData.data());
    opaquePass.draw(DrawCommand{ .vertexCount = 4 });
    // renderImGuiOverlayDynamic(&opaquePass, m_inFlightIndex);

    opaquePass.end();

    // We need to perform a layout transition since dynamic rendering doesn't perform implicit final layout transition
    // like using RenderPasses would
    //![5]
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::AllGraphicsBit,
            .srcMask = AccessFlagBit::ColorAttachmentWriteBit,
            .dstStages = PipelineStageFlagBit::BottomOfPipeBit,
            .dstMask = AccessFlagBit::MemoryReadBit,
            .oldLayout = TextureLayout::ColorAttachmentOptimal,
            .newLayout = TextureLayout::PresentSrc,
            .texture = m_swapchain.textures().at(m_currentSwapchainImageIndex),
            .range = TextureSubresourceRange{
                    .aspectMask = TextureAspectFlagBits::ColorBit,
            },
    });
    //![5]

    // Finalize the command recording
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}
