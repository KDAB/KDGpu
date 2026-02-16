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
    //![create_mesh_shader]
    // Create a mesh shader and fragment shader
    auto meshShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_sphere_mesh/hello_sphere_mesh.mesh.spv");
    auto meshShader = m_device.createShaderModule(KDGpuExample::readShaderFile(meshShaderPath));

    auto fragmentShaderPath = KDGpuExample::assetDir().file("shaders/examples/hello_sphere_mesh/hello_sphere_mesh.frag.spv");
    auto fragmentShader = m_device.createShaderModule(KDGpuExample::readShaderFile(fragmentShaderPath));
    //![create_mesh_shader]

    //![create_pipeline]
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
    //![create_pipeline]
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
}

void HelloSphereMesh::render()
{
    auto commandRecorder = m_device.createCommandRecorder();

    auto opaquePass = commandRecorder.beginRenderPass(KDGpu::RenderPassCommandRecorderOptions{
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

    //![draw_mesh_shader]
    opaquePass.setPipeline(m_pipeline);
    opaquePass.drawMeshTasks(KDGpu::DrawMeshCommand{
            .workGroupX = 1,
            .workGroupY = 1,
            .workGroupZ = 1,
    });
    //![draw_mesh_shader]
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
