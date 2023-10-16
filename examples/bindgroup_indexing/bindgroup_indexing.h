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

using namespace KDGpuExample;

class BindGroupIndexing : public SimpleExampleEngineLayer
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

    Buffer m_buffer;
    Buffer m_indexBuffer;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    std::vector<Buffer> m_transformBuffers;
    Buffer m_frameCounterSSBO;
    BindGroup m_transformsBindGroup;
    BindGroup m_ssboBindGroup;
    PushConstantRange m_transformCountPushConstant;
};
