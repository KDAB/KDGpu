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

using namespace KDGpuExample;

class MultiViewStereo : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;
    void recreateSwapChain() override;

private:
    Buffer m_vertexBuffer;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    const PushConstantRange m_pushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
    };

    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    const Format m_mvColorFormat{ Format::R8G8B8A8_UNORM };
    const Format m_mvDepthFormat{ Format::D24_UNORM_S8_UINT };
};
