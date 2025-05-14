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
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/pipeline_layout_options.h>

#include <glm/glm.hpp>

class BindGroupIndexing : public KDGpuExample::SimpleExampleEngineLayer
{
public:
    BindGroupIndexing();

protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void createRenderTarget();

    KDGpu::Buffer m_buffer;
    KDGpu::Buffer m_indexBuffer;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    KDGpu::CommandBuffer m_commandBuffer;

    std::vector<KDGpu::Buffer> m_transformBuffers;
    KDGpu::Buffer m_frameCounterSSBO;
    KDGpu::BindGroup m_transformsBindGroup;
    KDGpu::BindGroup m_ssboBindGroup;
    KDGpu::PushConstantRange m_transformCountPushConstant;
};
