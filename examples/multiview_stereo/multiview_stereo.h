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
class MultiViewStereo : public KDGpuExample::SimpleExampleEngineLayer
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
    KDGpu::Buffer m_vertexBuffer;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::GraphicsPipeline m_pipeline;
    const KDGpu::PushConstantRange m_pushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit)
    };

    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    KDGpu::CommandBuffer m_commandBuffer;

    const KDGpu::Format m_mvColorFormat{ KDGpu::Format::R8G8B8A8_UNORM };
    const KDGpu::Format m_mvDepthFormat{ KDGpu::Format::D24_UNORM_S8_UINT };
};
