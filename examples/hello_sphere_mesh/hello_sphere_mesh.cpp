/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_sphere_mesh.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/graphics_pipeline_options.h>

#include <string>

using namespace KDGpu;

void HelloSphereMesh::initializeScene()
{
    // Create a mesh shader and fragment shader
    auto meshShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_sphere_mesh/hello_sphere_mesh.mesh.spv");
    auto meshShader = m_device.createShaderModule(KDGpuExample::readShaderFile(meshShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_sphere_mesh/hello_sphere_mesh.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));

    // Create a pipeline layout
    const PipelineLayoutOptions pipelineLayoutOptions = {};
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

    // Create a pipeline
    const GraphicsPipelineOptions pipelineOptions = {
        .label = "Triangle",
        .shaderStages = {
                { .shaderModule = meshShader, .stage = ShaderStageFlagBits::MeshBit },
                { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit },
        },
        .layout = m_pipelineLayout,
        .renderTargets = {
                { .format = m_swapchainFormat },
        },
        .depthStencil = {
                .format = m_depthFormat,
                .depthWritesEnabled = true,
                .depthCompareOperation = CompareOperation::Less,
        },
        .primitive = {
                .cullMode = CullModeFlagBits::None,
        }
    };
    m_pipeline = m_device.createGraphicsPipeline(pipelineOptions);

    // Most of the render pass is the same between frames. The only thing that changes, is which image
    // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
    // just update the color texture view.
    m_opaquePassOptions = {
        .colorAttachments = {
                {
                        .view = {}, // Not setting the swapchain texture view just yet
                        .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                        .finalLayout = TextureLayout::PresentSrc,
                },
        },
        .depthStencilAttachment = {
                .view = m_depthTextureView,
        }
    };
}

void HelloSphereMesh::cleanupScene()
{
    m_pipeline = {};
    m_pipelineLayout = {};
    m_commandBuffer = {};
}

void HelloSphereMesh::updateScene()
{
}

void HelloSphereMesh::resize()
{
    // Swapchain might have been resized and texture views recreated. Ensure we update the PassOptions accordingly
    m_opaquePassOptions.depthStencilAttachment.view = m_depthTextureView;
}

void HelloSphereMesh::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
    auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);

    opaquePass.setPipeline(m_pipeline);
    opaquePass.drawMeshTasks(KDGpu::DrawMeshCommand{
            .workGroupX = 1,
            .workGroupY = 1,
            .workGroupZ = 1,
    });
    renderImGuiOverlay(&opaquePass);
    opaquePass.end();
    m_commandBuffer = commandRecorder.finish();

    const SubmitOptions submitOptions = {
        .commandBuffers = { m_commandBuffer },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_renderCompleteSemaphores[m_currentSwapchainImageIndex] }
    };
    m_queue.submit(submitOptions);
}
