/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace KDGpuExample;

class HelloSphereMesh : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;
};
