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

#include <glm/glm.hpp>

using namespace KDGpuExample;

class HelloTriangleMSAA : public SimpleExampleEngineLayer
{
public:
    HelloTriangleMSAA();

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
    void setMsaaSampleCount(SampleCountFlagBits samples);

    Buffer m_buffer;
    Buffer m_indexBuffer;
    Texture m_msaaTexture;
    TextureView m_msaaTextureView;
    PipelineLayout m_pipelineLayout;
    RenderPassCommandRecorderOptions m_commandRecorderOptions;
    CommandBuffer m_commandBuffer;

    std::vector<GraphicsPipeline> m_pipelines;

    uint8_t m_requestedSampleCountIndex = 0;
    uint8_t m_currentPipelineIndex = 0;

    glm::mat4 m_transform;
    Buffer m_transformBuffer;
    BindGroup m_transformBindGroup;
};
