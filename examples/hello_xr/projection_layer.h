/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/xr_compositor/xr_projection_layer.h>
#include <KDXr/kdxr_core.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/command_buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <kdbindings/property.h>

#include <glm/glm.hpp>

using namespace KDGpuExample;
using namespace KDGpu;

namespace KDXr {
class Instance;
}

class ProjectionLayer : public XrProjectionLayer
{
public:
    KDBindings::Property<bool> animate{ true };
    KDBindings::Property<float> scale{ 1.0f };
    KDBindings::Property<glm::vec3> translation{ glm::vec3(0.0f, 0.0f, -1.0f) };
    KDBindings::Property<KDXr::Pose> leftPalmPose{ KDXr::Pose{} };
    KDBindings::Property<KDXr::Pose> rightPalmPose{ KDXr::Pose{} };

    explicit ProjectionLayer(const XrProjectionLayerOptions &options);
    ~ProjectionLayer() override;

    // Not copyable
    ProjectionLayer(const ProjectionLayer &) = delete;
    ProjectionLayer &operator=(const ProjectionLayer &) = delete;

    // Moveable
    ProjectionLayer(ProjectionLayer &&) = default;
    ProjectionLayer &operator=(ProjectionLayer &&) = default;

protected:
    void initialize() override;
    void cleanup() override;
    void updateScene() override;
    void renderView() override;

private:
    void initializeScene();
    void cleanupScene();

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
    Buffer m_leftHandBuffer;
    Buffer m_rightHandBuffer;
    Buffer m_indexBuffer;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    glm::mat4 m_transform;
    Buffer m_transformBuffer;
    BindGroup m_entityTransformBindGroup;

    glm::mat4 m_leftHandTransform;
    Buffer m_leftHandTransformBuffer;
    BindGroup m_leftHandTransformBindGroup;

    glm::mat4 m_rightHandTransform;
    Buffer m_rightHandTransformBuffer;
    BindGroup m_rightHandTransformBindGroup;

    Fence m_fence;
};
