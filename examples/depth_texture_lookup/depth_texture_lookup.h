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

class DepthTextureLookup : public KDGpuExample::SimpleExampleEngineLayer
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
    KDGpu::PipelineLayout m_sceneCubePipelineLayout;
    KDGpu::GraphicsPipeline m_sceneCubePipeline;
    KDGpu::RenderPassCommandRecorderOptions m_sceneCubePassOptions;
    KDGpu::PushConstantRange m_rotationPushConstantRange;

    // Depth Lookup Pass
    KDGpu::Sampler m_depthTextureSampler;
    KDGpu::BindGroup m_depthTextureBindGroup;
    KDGpu::BindGroupLayout m_depthLookupBindGroupLayout;
    KDGpu::PipelineLayout m_depthLookupPipelineLayout;
    KDGpu::GraphicsPipeline m_depthLookupPipeline;
    KDGpu::RenderPassCommandRecorderOptions m_depthLookupPassOptions;

    // ImGui Overlay Pass
    KDGpu::RenderPassCommandRecorderOptions m_overlayPassOptions;

    KDGpu::CommandBuffer m_commandBuffer;
    KDGpu::TextureLayout m_depthLayout = KDGpu::TextureLayout::Undefined;
};
