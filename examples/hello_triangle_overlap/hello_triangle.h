/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/advanced_example_engine_layer.h>

#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

class HelloTriangle : public KDGpuExample::AdvancedExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    glm::mat4 m_transform = glm::mat4(1.0f);
    KDGpu::Buffer m_buffer;
    KDGpu::Buffer m_indexBuffer;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    const KDGpu::PushConstantRange m_transformPushConstantRange{ .offset = 0,
                                                                 .size = 16 * sizeof(float),
                                                                 .shaderStages = KDGpu::ShaderStageFlags(KDGpu::ShaderStageFlagBits::VertexBit) };
    std::array<KDGpu::CommandBuffer, KDGpuExample::MAX_FRAMES_IN_FLIGHT> m_commandBuffers;
};
