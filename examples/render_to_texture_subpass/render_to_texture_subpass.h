/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

class RenderToTextureSubpass : public KDGpuExample::SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void createRenderPass();
    void initializeMainScene();
    void initializePostProcess();
    void createOffscreenTexture();
    void updateColorBindGroup();
    static void drawControls(ImGuiContext *ctx);

    KDGpu::Buffer m_buffer;
    KDGpu::Buffer m_indexBuffer;
    KDGpu::Buffer m_transformBuffer;
    KDGpu::Buffer m_fullScreenQuad;

    KDGpu::RenderPass m_renderPass;

    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::PipelineLayout m_postProcessPipelineLayout;
    KDGpu::GraphicsPipeline m_postProcessPipeline;

    glm::mat4 m_transform;
    KDGpu::BindGroup m_transformBindGroup;
    KDGpu::BindGroup m_colorBindGroup;
    KDGpu::BindGroupLayout m_colorBindGroupLayout;

    const KDGpu::PushConstantRange m_filterPosPushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::FragmentBit)
    };
    std::vector<uint8_t> m_filterPosData;
    float m_filterPos{ 0.0f };

    // Rendering resources
    const KDGpu::Format m_colorFormat{ KDGpu::Format::R8G8B8A8_UNORM };
    KDGpu::Texture m_colorOutput;
    KDGpu::TextureView m_colorOutputView;
    KDGpu::Sampler m_colorOutputSampler;
    KDGpu::RenderPassCommandRecorderWithRenderPassOptions m_renderPassOptions;
    KDGpu::CommandBuffer m_commandBuffer;
};
