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

using namespace KDGpuExample;

class HelloTriangle : public AdvancedExampleEngineLayer
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
    Buffer m_buffer;
    Buffer m_indexBuffer;
    GraphicsPipeline m_pipeline;
    PipelineLayout m_pipelineLayout;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    const PushConstantRange m_transformPushConstantRange{ .offset = 0,
                                                          .size = 16 * sizeof(float),
                                                          .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) };
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;
};
