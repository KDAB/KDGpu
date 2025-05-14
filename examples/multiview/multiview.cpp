/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "multiview.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/graphics_pipeline_options.h>

#include <fstream>
#include <string>

#include <glm/glm.hpp>

using namespace KDGpu;

void MultiView::initializeScene()
{
    createMultiViewOffscreenTextures();

    initializeMultiViewPass();
    initializeFullScreenPass();

    updateFinalPassBindGroup();
}

void MultiView::cleanupScene()
{
    m_mvPipeline = {};
    m_mvPipelineLayout = {};
    m_vertexBuffer = {};

    m_fsqPipeline = {};
    m_fsqTextureBindGroup = {};
    m_fsqPipelineLayout = {};
    m_fsqTextureBindGroupLayout = {};

    m_multiViewColorOutputView = {};
    m_multiViewDepthView = {};
    m_multiViewColorOutput = {};
    m_multiViewDepth = {};

    m_multiViewColorOutputSampler = {};
    m_commandBuffer = {};
}

void MultiView::updateScene()
{
    // Nothing to do for this simple, static, non-interactive example
}

void MultiView::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_fsqPassOptions.depthStencilAttachment.view = m_depthTextureView;

    createMultiViewOffscreenTextures();

    m_mvPassOptions.colorAttachments[0].view = m_multiViewColorOutputView;
    m_mvPassOptions.depthStencilAttachment.view = m_multiViewDepthView;

    updateFinalPassBindGroup();
}

//![3]
void MultiView::createMultiViewOffscreenTextures()
{
    m_multiViewColorOutput = m_device.createTexture(TextureOptions{
            .type = TextureType::TextureType2D,
            .format = m_mvColorFormat,
            .extent = { m_window->width(), m_window->height(), 1 },
            .mipLevels = 1,
            .arrayLayers = 2,
            .samples = SampleCountFlagBits::Samples1Bit,
            .usage = TextureUsageFlagBits::ColorAttachmentBit | TextureUsageFlagBits::SampledBit,
            .memoryUsage = MemoryUsage::GpuOnly,
    });
    m_multiViewDepth = m_device.createTexture(TextureOptions{
            .type = TextureType::TextureType2D,
            .format = m_mvDepthFormat,
            .extent = { m_window->width(), m_window->height(), 1 },
            .mipLevels = 1,
            .arrayLayers = 2,
            .samples = SampleCountFlagBits::Samples1Bit,
            .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
            .memoryUsage = MemoryUsage::GpuOnly,
    });

    m_multiViewColorOutputView = m_multiViewColorOutput.createView(TextureViewOptions{
            .viewType = ViewType::ViewType2DArray,
    });
    m_multiViewDepthView = m_multiViewDepth.createView(TextureViewOptions{
            .viewType = ViewType::ViewType2DArray,
    });
}
//![3]

void MultiView::initializeMultiViewPass()
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Create a buffer to hold triangle vertex data
    {
        m_vertexBuffer = m_device.createBuffer(BufferOptions{
                .size = 3 * sizeof(Vertex), // 3 vertices * 2 attributes * 3 float components
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
        });

        const float r = 0.8f;
        std::array<Vertex, 3> vertexData;
        vertexData[0] = { { r * std::cos(7.0f * M_PI / 6.0f), -r * std::sin(7.0f * M_PI / 6.0f), 0.0f }, { 1.0f, 0.0, 0.0f } }; // Bottom-left, red
        vertexData[1] = { { r * std::cos(11.0f * M_PI / 6.0f), -r * std::sin(11.0f * M_PI / 6.0f), 0.0f }, { 0.0f, 1.0, 0.0f } }; // Bottom-right, green
        vertexData[2] = { { 0.0f, -r, 0.0f }, { 0.0f, 0.0, 1.0f } }; // Top, blue

        auto bufferData = m_vertexBuffer.map();
        std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(Vertex));
        m_vertexBuffer.unmap();
    }

    // Create a vertex shader and fragment shader (spir-v only for now)
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/multiview/rotating_triangle.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/multiview/rotating_triangle.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    //![4]
    // Create a pipeline layout (array of bind group layouts)
    m_mvPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
            .pushConstantRanges = { m_mvPushConstantRange },
    });
    //![4]

    m_mvPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
            .layout = m_mvPipelineLayout,
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
                    { .format = m_mvColorFormat },
            },
            .depthStencil = {
                    .format = m_mvDepthFormat,
                    .depthWritesEnabled = true,
                    .depthCompareOperation = CompareOperation::Less,
            },
            .viewCount = 2,
    });

    //![5]
    m_mvPassOptions = {
        .colorAttachments = {
                { .view = m_multiViewColorOutputView,
                  .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                  .finalLayout = TextureLayout::ColorAttachmentOptimal },
        },
        .depthStencilAttachment = { .view = m_multiViewDepthView },
        .viewCount = 2, // Enables multiview rendering
    };
    //![5]
}

void MultiView::initializeFullScreenPass()
{
    // Create bind group layout consisting of a single binding holding the texture the 1st pass rendered to
    m_fsqTextureBindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
            .bindings = {
                    {
                            .binding = 0,
                            .resourceType = ResourceBindingType::CombinedImageSampler,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                    },
            },
    });

    // Create a pipeline layout (array of bind group layouts)
    m_fsqPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
            .bindGroupLayouts = { m_fsqTextureBindGroupLayout },
            .pushConstantRanges = { m_fsqLayerIdxPushConstantRange },
    });

    // Create a vertex shader and fragment shader for fullscreen quad
    auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/multiview/fullscreenquad.vert.spv");
    auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/multiview/fullscreenquad.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create a pipeline
    m_fsqPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
            .shaderStages = {
                    { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                    { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit },
            },
            .layout = m_fsqPipelineLayout,
            .vertex = {
                    .buffers = {},
                    .attributes = {},
            },
            .renderTargets = { { .format = m_swapchainFormat } },
            .depthStencil = { .format = m_depthFormat, .depthWritesEnabled = true, .depthCompareOperation = CompareOperation::Less },
    });

    // Prepare pass options
    m_fsqPassOptions = {
        .colorAttachments = {
                { .view = {}, // Not setting the swapchain texture view just yet
                  .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                  .finalLayout = TextureLayout::PresentSrc } },
        .depthStencilAttachment = { .view = m_depthTextureView }
    };

    // Create a sampler we can use to sample from the color texture in the final pass
    m_multiViewColorOutputSampler = m_device.createSampler();
}

//![7]
void MultiView::updateFinalPassBindGroup()
{
    // Create a bindGroup to hold the Offscreen Color Texture
    m_fsqTextureBindGroup = m_device.createBindGroup(BindGroupOptions{
            .layout = m_fsqTextureBindGroupLayout,
            .resources = {
                    {
                            .binding = 0,
                            .resource = TextureViewSamplerBinding{ .textureView = m_multiViewColorOutputView,
                                                                   .sampler = m_multiViewColorOutputSampler },
                    },
            },
    });
}
//![7]

void MultiView::render()
{
    const float halfWidth = m_window->width() / 2;
    static float rotationAngleDeg = 0.0f;

    rotationAngleDeg += 0.1f;
    const float rotationAngleRad = glm::radians(rotationAngleDeg);

    // Create a command encoder/recorder
    auto commandRecorder = m_device.createCommandRecorder();

    //![6]
    // MultiView Pass
    auto mvPass = commandRecorder.beginRenderPass(m_mvPassOptions);
    mvPass.setPipeline(m_mvPipeline);
    mvPass.setVertexBuffer(0, m_vertexBuffer);
    mvPass.pushConstant(m_mvPushConstantRange, &rotationAngleRad);
    mvPass.draw(DrawCommand{ .vertexCount = 3 });
    mvPass.end();
    //![6]

    // Wait for writes to multiview texture to have been completed
    // Transition it to a shader read only layout
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlagBit::ColorAttachmentOutputBit,
            .srcMask = AccessFlagBit::ColorAttachmentWriteBit,
            .dstStages = PipelineStageFlagBit::FragmentShaderBit,
            .dstMask = AccessFlagBit::ShaderReadBit,
            .oldLayout = TextureLayout::ColorAttachmentOptimal,
            .newLayout = TextureLayout::ShaderReadOnlyOptimal,
            .texture = m_multiViewColorOutput,
            .range = {
                    .aspectMask = TextureAspectFlagBits::ColorBit,
                    .levelCount = 1,
            },
    });

    //![8]
    // FullScreen Pass
    m_fsqPassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto fsqPass = commandRecorder.beginRenderPass(m_fsqPassOptions);
    fsqPass.setPipeline(m_fsqPipeline);
    fsqPass.setBindGroup(0, m_fsqTextureBindGroup);

    // Left Eye
    fsqPass.setViewport(Viewport{
            .x = 0,
            .y = 0,
            .width = halfWidth,
            .height = float(m_window->height()),
    });
    const int leftEyeLayer = 0;
    fsqPass.pushConstant(m_fsqLayerIdxPushConstantRange, &leftEyeLayer);
    fsqPass.draw(DrawCommand{ .vertexCount = 6 });

    // Right Eye
    fsqPass.setViewport(Viewport{
            .x = halfWidth,
            .y = 0,
            .width = halfWidth,
            .height = float(m_window->height()),
    });
    const int rightEyeLayer = 1;
    fsqPass.pushConstant(m_fsqLayerIdxPushConstantRange, &rightEyeLayer);
    fsqPass.draw(DrawCommand{ .vertexCount = 6 });

    // Call helper to record the ImGui overlay commands
    renderImGuiOverlay(&fsqPass);

    fsqPass.end();
    //![8]

    // End recording
    m_commandBuffer = commandRecorder.finish();

    // Submit command buffer to queue
    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(submitOptions);
}
