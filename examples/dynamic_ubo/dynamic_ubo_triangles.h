/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>
class DynamicUBOTriangles : public KDGpuExample::SimpleExampleEngineLayer
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
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    KDGpu::CommandBuffer m_commandBuffer;
    KDGpu::Buffer m_transformDynamicUBOBuffer;
    KDGpu::BindGroup m_transformBindGroup;

    size_t m_dynamicUBOByteStride{ 0 };
};
