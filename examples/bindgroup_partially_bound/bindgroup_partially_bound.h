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
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/texture.h>
#include <KDGpu/sampler.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/pipeline_layout_options.h>

#include <glm/glm.hpp>

using namespace KDGpuExample;

class BindGroupPartiallyBound : public SimpleExampleEngineLayer
{
public:
    BindGroupPartiallyBound();

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
    Texture m_texture;
    TextureView m_textureView;
    Sampler m_sampler;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    BindGroup m_textureBindGroup;
    BindGroupLayout m_textureBindGroupLayout;
    PushConstantRange m_transformCountPushConstant;
    PushConstantRange m_textureInUsePushConstant;

    glm::mat4 m_transform;
};
