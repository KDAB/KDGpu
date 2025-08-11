/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include <glm/glm.hpp>

class HelloTriangleMSAAWithDynamicRendering : public KDGpuExample::SimpleExampleEngineLayer
{
public:
    HelloTriangleMSAAWithDynamicRendering();

protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void createRenderTarget();
    bool isMsaaEnabled() const;
    void drawMsaaSettings(ImGuiContext *);
    void setMsaaSampleCount(KDGpu::SampleCountFlagBits samples);

    KDGpu::Buffer m_buffer;
    KDGpu::Buffer m_indexBuffer;
    KDGpu::Texture m_msaaTexture;
    KDGpu::TextureView m_msaaTextureView;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::RenderPassCommandRecorderWithDynamicRenderingOptions m_commandRecorderOptions;
    KDGpu::CommandBuffer m_commandBuffer;

    std::vector<KDGpu::GraphicsPipeline> m_pipelines;

    uint8_t m_requestedSampleCountIndex = 0;
    uint8_t m_currentPipelineIndex = 0;

    glm::mat4 m_transform;
    KDGpu::Buffer m_transformBuffer;
    void *m_transformBufferData{ nullptr };
    KDGpu::BindGroup m_transformBindGroup;
};
