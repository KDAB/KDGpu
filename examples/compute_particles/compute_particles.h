/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

class ComputeParticles : public KDGpuExample::SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

    void renderSingleCommandBuffer();
    void renderMultipleCommandBuffers();

private:
    KDGpu::Buffer m_particleDataBuffer;
    KDGpu::Buffer m_triangleVertexBuffer;
    KDGpu::ComputePipeline m_computePipeline;
    KDGpu::GraphicsPipeline m_graphicsPipeline;
    KDGpu::PipelineLayout m_graphicsPipelineLayout;
    KDGpu::PipelineLayout m_computePipelineLayout;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    KDGpu::CommandBuffer m_graphicsCommands;
    KDGpu::CommandBuffer m_computeCommands;
    KDGpu::CommandBuffer m_graphicsAndComputeCommands;

    KDGpu::BindGroup m_particleBindGroup;
    KDGpu::GpuSemaphore m_computeSemaphoreComplete;
};
