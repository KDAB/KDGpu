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

#include <glm/glm.hpp>

using namespace KDGpuExample;

class DepthTextureLookup : public SimpleExampleEngineLayer
{
public:
    DepthTextureLookup();

protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    // Scene Cube Pass
    PipelineLayout m_sceneCubePipelineLayout;
    GraphicsPipeline m_sceneCubePipeline;
    RenderPassCommandRecorderOptions m_sceneCubePassOptions;
    PushConstantRange m_rotationPushConstantRange;

    // Depth Lookup Pass
    Sampler m_depthTextureSampler;
    BindGroup m_depthTextureBindGroup;
    BindGroupLayout m_depthLookupBindGroupLayout;
    PipelineLayout m_depthLookupPipelineLayout;
    GraphicsPipeline m_depthLookupPipeline;
    RenderPassCommandRecorderOptions m_depthLookupPassOptions;

    // ImGui Overlay Pass
    RenderPassCommandRecorderOptions m_overlayPassOptions;

    CommandBuffer m_commandBuffer;
    KDGpu::TextureLayout m_depthLayout = KDGpu::TextureLayout::Undefined;
};
