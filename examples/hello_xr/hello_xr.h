/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/xr_example_engine_layer.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

#include <array>

namespace KDGpuExample {
class XrQuadImGuiLayer;
}

using namespace KDGpuExample;

class HelloXr : public XrExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void renderView() override;
    void resize() override;

private:
    void updateTransformUbo();
    void updateViewUbo();

    struct CameraData {
        glm::mat4 view;
        glm::mat4 projection;
    };

    std::vector<CameraData> m_cameraData{ 2 }; // Default to 2 views
    float m_nearPlane{ 0.05f };
    float m_farPlane{ 100.0f };
    Buffer m_cameraBuffer;
    BindGroup m_cameraBindGroup;

    Buffer m_buffer;
    Buffer m_indexBuffer;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    glm::mat4 m_transform;
    Buffer m_transformBuffer;
    BindGroup m_entityTransformBindGroup;
    Fence m_fence;

    XrQuadImGuiLayer *m_imguiLayer{ nullptr };
};
