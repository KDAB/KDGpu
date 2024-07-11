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

using namespace KDGpuExample;

class RenderToTextureSubpass : public SimpleExampleEngineLayer
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
    void drawControls(ImGuiContext *ctx);

    Buffer m_buffer;
    Buffer m_indexBuffer;
    Buffer m_transformBuffer;
    Buffer m_fullScreenQuad;

    RenderPass m_renderPass;

    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    PipelineLayout m_postProcessPipelineLayout;
    GraphicsPipeline m_postProcessPipeline;

    glm::mat4 m_transform;
    BindGroup m_transformBindGroup;
    BindGroup m_colorBindGroup;
    BindGroupLayout m_colorBindGroupLayout;

    const PushConstantRange m_filterPosPushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
    };
    std::vector<uint8_t> m_filterPosData;
    float m_filterPos{ 0.0f };

    // Rendering resources
    const Format m_colorFormat{ Format::R8G8B8A8_UNORM };
    Texture m_colorOutput;
    TextureView m_colorOutputView;
    Sampler m_colorOutputSampler;
    RenderPassCommandRecorderOptions m_renderPassOptions;
    CommandBuffer m_commandBuffer;
};
