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

#include <array>
#include <vector>

namespace KDXr {
class Instance;
}

class ProjectionLayer : public KDGpuExample::XrProjectionLayer
{
public:
    KDBindings::Property<bool> rotateZ{ false };
    KDBindings::Property<bool> rotateY{ false };
    KDBindings::Property<float> scale{ 1.0f };
    KDBindings::Property<glm::vec3> translation{ glm::vec3(0.0f, 0.0f, -1.0f) };
    KDBindings::Property<KDXr::Pose> leftPalmPose{ KDXr::Pose{} };
    KDBindings::Property<KDXr::Pose> rightPalmPose{ KDXr::Pose{} };

    explicit ProjectionLayer(const KDGpuExample::XrProjectionLayerOptions &options);
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
    std::vector<glm::mat4> m_viewProjections{ 2 };
    KDGpu::Buffer m_cameraBuffer;
    float *m_cameraBufferData{ nullptr };
    KDGpu::BindGroup m_cameraBindGroup;
    KDGpu::PushConstantRange m_viewIndexPushConstant{
        .offset = 0,
        .size = sizeof(uint32_t),
        .shaderStages = KDGpu::ShaderStageFlagBits::VertexBit
    };

    KDGpu::Buffer m_buffer;
    KDGpu::Buffer m_leftHandBuffer;
    KDGpu::Buffer m_rightHandBuffer;
    KDGpu::Buffer m_indexBuffer;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    std::array<KDGpu::CommandBuffer, 2> m_commandBuffers;

    glm::mat4 m_transform;
    KDGpu::Buffer m_transformBuffer;
    void *m_transformBufferData{ nullptr };
    KDGpu::BindGroup m_entityTransformBindGroup;

    glm::mat4 m_leftHandTransform;
    KDGpu::Buffer m_leftHandTransformBuffer;
    void *m_leftHandTransformBufferData{ nullptr };
    KDGpu::BindGroup m_leftHandTransformBindGroup;

    glm::mat4 m_rightHandTransform;
    KDGpu::Buffer m_rightHandTransformBuffer;
    void *m_rightHandTransformBufferData{ nullptr };
    KDGpu::BindGroup m_rightHandTransformBindGroup;
};
