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

class MultiView : public KDGpuExample::SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void initializeMultiViewPass();
    void initializeFullScreenPass();
    void createMultiViewOffscreenTextures();
    void updateFinalPassBindGroup();

    // MultiView Scene
    KDGpu::Buffer m_vertexBuffer;
    KDGpu::PipelineLayout m_mvPipelineLayout;
    KDGpu::GraphicsPipeline m_mvPipeline;
    const KDGpu::PushConstantRange m_mvPushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit)
    };

    // Full Screen Quad Scene
    KDGpu::PipelineLayout m_fsqPipelineLayout;
    KDGpu::GraphicsPipeline m_fsqPipeline;
    KDGpu::BindGroupLayout m_fsqTextureBindGroupLayout;
    KDGpu::BindGroup m_fsqTextureBindGroup;
    const KDGpu::PushConstantRange m_fsqLayerIdxPushConstantRange{
        .offset = 0,
        .size = sizeof(int),
        .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit)
    };

    KDGpu::Texture m_multiViewColorOutput;
    KDGpu::Texture m_multiViewDepth;
    KDGpu::TextureView m_multiViewColorOutputView;
    KDGpu::TextureView m_multiViewDepthView;

    KDGpu::Sampler m_multiViewColorOutputSampler;

    KDGpu::RenderPassCommandRecorderOptions m_mvPassOptions;
    KDGpu::RenderPassCommandRecorderOptions m_fsqPassOptions;
    KDGpu::CommandBuffer m_commandBuffer;

    const KDGpu::Format m_mvColorFormat{ KDGpu::Format::R8G8B8A8_UNORM };
    const KDGpu::Format m_mvDepthFormat{ KDGpu::Format::D24_UNORM_S8_UINT };
};
