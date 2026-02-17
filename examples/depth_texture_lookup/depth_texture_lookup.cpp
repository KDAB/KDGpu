/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "depth_texture_lookup.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/memory_barrier.h>

#include <glm/gtx/transform.hpp>

#include <string>

using namespace KDGpu;
using namespace KDGpuExample;

DepthTextureLookup::DepthTextureLookup()
    : SimpleExampleEngineLayer()
{
    // Request the SampledBit for the DepthTexture
    m_depthTextureUsageFlags |= TextureUsageFlagBits::SampledBit;
}

void DepthTextureLookup::initializeScene()
{
    //![1]
    // Scene Cube Pass
    {
        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/depth_texture_lookup/cube.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/depth_texture_lookup/cube.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        m_rotationPushConstantRange = {
            .size = sizeof(glm::mat4),
            .shaderStages = ShaderStageFlagBits::VertexBit,
        };

        m_sceneCubePipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .pushConstantRanges = {
                        m_rotationPushConstantRange,
                },
        });

        // Create a pipeline
        m_sceneCubePipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_sceneCubePipelineLayout,
                .vertex = {},
                .renderTargets = { { .format = m_swapchainFormat } },
                .depthStencil = { .format = m_depthFormat, .depthWritesEnabled = true, .depthCompareOperation = CompareOperation::Less },
                .primitive = { .topology = PrimitiveTopology::TriangleList },
        });
    }
    //![1]

    //![2]
    // Depth Lookup Pass
    {
        // Create a sampler to be used when sampling the detph texture
        m_depthTextureSampler = m_device.createSampler();

        // Create a vertex shader and fragment shader (spir-v only for now)
        auto vertexShaderPath = KDGpuExample::assetDir().file("shaders/examples/depth_texture_lookup/textured_quad.vert.spv");
        auto vertexShader = m_device.createShaderModule(KDGpuExample::readShaderFile(vertexShaderPath));

        auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/depth_texture_lookup/textured_quad.frag.spv");
        auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

        m_depthLookupBindGroupLayout = m_device.createBindGroupLayout(BindGroupLayoutOptions{
                .bindings = {
                        {
                                .binding = 0,
                                .resourceType = ResourceBindingType::CombinedImageSampler,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit),
                        },
                },
        });

        // Create a pipeline layout (array of bind group layouts)
        m_depthLookupPipelineLayout = m_device.createPipelineLayout(PipelineLayoutOptions{
                .bindGroupLayouts = { m_depthLookupBindGroupLayout },
        });

        m_depthLookupPipeline = m_device.createGraphicsPipeline(GraphicsPipelineOptions{
                .shaderStages = {
                        { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                        { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit } },
                .layout = m_depthLookupPipelineLayout,
                .vertex = {},
                .renderTargets = { { .format = m_swapchainFormat } },
                .primitive = { .topology = PrimitiveTopology::TriangleList },
        });

        // Create a bindGroup to hold the uniform containing the texture and sampler
        m_depthTextureBindGroup = m_device.createBindGroup(BindGroupOptions{
                .layout = m_depthLookupBindGroupLayout,
                .resources = {
                        {
                                .binding = 0,
                                .resource = TextureViewSamplerBinding{ .textureView = m_depthTextureView, .sampler = m_depthTextureSampler },
                        },
                },
        });
    }
    //![2]
}

void DepthTextureLookup::cleanupScene()
{
    m_sceneCubePipeline = {};
    m_sceneCubePipelineLayout = {};

    m_depthLookupPipeline = {};
    m_depthLookupPipelineLayout = {};
    m_depthTextureBindGroup = {};
    m_depthLookupBindGroupLayout = {};
    m_depthTextureSampler = {};

    m_commandBuffer = {};
}

void DepthTextureLookup::updateScene()
{
}

void DepthTextureLookup::resize()
{
    // Update the depth texture view on the bind group
    m_depthTextureBindGroup.update(BindGroupEntry{
            .binding = 0,
            .resource = TextureViewSamplerBinding{ .textureView = m_depthTextureView, .sampler = m_depthTextureSampler },
    });

    m_depthLayout = KDGpu::TextureLayout::Undefined;
}

void DepthTextureLookup::render()
{
    //![4]
    auto commandRecorder = m_device.createCommandRecorder();
    //![4]

    //![5]
    // Draw Cube

    static float angle = 0.0f;
    const float angularSpeed = 3.0f; // degrees per second
    const float dt = engine()->deltaTimeSeconds();
    angle += angularSpeed * dt;
    if (angle > 360.0f)
        angle -= 360.0f;

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));

    auto opaquePass = commandRecorder.beginRenderPass(RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = m_swapchainViews.at(m_currentSwapchainImageIndex),
                            .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
            },
    });
    opaquePass.setPipeline(m_sceneCubePipeline);
    opaquePass.pushConstant(m_rotationPushConstantRange, &rotation);
    opaquePass.draw(DrawCommand{ .vertexCount = 36 });
    opaquePass.end();
    //![5]

    //![6]
    // Only process depth lookup pass fragments once we are sure scene cube fragments have written to the depth buffer
    // Transition the depthTexture to a correct readable layout
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlags(PipelineStageFlagBit::AllGraphicsBit),
            .srcMask = AccessFlagBit::DepthStencilAttachmentWriteBit,
            .dstStages = PipelineStageFlags(PipelineStageFlagBit::FragmentShaderBit),
            .dstMask = AccessFlags(AccessFlagBit::ShaderReadBit),
            .oldLayout = m_depthLayout,
            .newLayout = TextureLayout::ShaderReadOnlyOptimal,
            .texture = m_depthTexture,
            .range = {
                    .aspectMask = TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit,
                    .levelCount = 1,
            },
    });

    // Draw Quad that displays depth lookup
    auto depthLookupPass = commandRecorder.beginRenderPass(RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = m_swapchainViews.at(m_currentSwapchainImageIndex),
                            .loadOperation = AttachmentLoadOperation::Load, // Don't clear color
                            .initialLayout = TextureLayout::ColorAttachmentOptimal,
                    },
            },
    });
    depthLookupPass.setPipeline(m_depthLookupPipeline);
    depthLookupPass.setBindGroup(0, m_depthTextureBindGroup);
    depthLookupPass.draw(DrawCommand{ .vertexCount = 6 });
    depthLookupPass.end();

    // Layout gets reset when we resize as the depthTexture is recreated
    if (m_depthLayout == TextureLayout::Undefined)
        m_depthLayout = TextureLayout::DepthStencilAttachmentOptimal;

    // Transition the depthTexture back to an appropriate depthBuffer layout
    commandRecorder.textureMemoryBarrier(TextureMemoryBarrierOptions{
            .srcStages = PipelineStageFlags(PipelineStageFlagBit::BottomOfPipeBit),
            .dstStages = PipelineStageFlags(PipelineStageFlagBit::TopOfPipeBit),
            .oldLayout = TextureLayout::ShaderReadOnlyOptimal,
            .newLayout = m_depthLayout,
            .texture = m_depthTexture,
            .range = {
                    .aspectMask = TextureAspectFlagBits::DepthBit | TextureAspectFlagBits::StencilBit,
                    .levelCount = 1,
            },
    });
    //![6]

    //![7]
    auto overlayPass = commandRecorder.beginRenderPass(RenderPassCommandRecorderOptions{
            .colorAttachments = {
                    {
                            .view = m_swapchainViews.at(m_currentSwapchainImageIndex),
                            .loadOperation = AttachmentLoadOperation::Load, // Don't clear color
                            .initialLayout = TextureLayout::ColorAttachmentOptimal,
                            .finalLayout = TextureLayout::PresentSrc,
                    },
            },
            .depthStencilAttachment = {
                    .view = m_depthTextureView,
                    .depthLoadOperation = AttachmentLoadOperation::Load, // Load the depth buffer as is, don't clear it
                    .initialLayout = TextureLayout::DepthStencilAttachmentOptimal,
            },
    });
    renderImGuiOverlay(&opaquePass);
    overlayPass.end();
    //![7]

    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}
