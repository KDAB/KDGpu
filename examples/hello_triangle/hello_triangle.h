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

#include <glm/glm.hpp>

class HelloTriangle : public KDGpuExample::SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    KDGpu::Buffer m_buffer;
    KDGpu::Buffer m_indexBuffer;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    KDGpu::CommandBuffer m_commandBuffer;

    glm::mat4 m_transform;
    KDGpu::Buffer m_transformBuffer;
    KDGpu::BindGroup m_transformBindGroup;
};
