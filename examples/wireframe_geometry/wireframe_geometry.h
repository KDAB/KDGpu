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

#include <glm/glm.hpp>

class WireframeGeometry : public KDGpuExample::SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void updateViewportBuffer();

    glm::vec3 m_cameraPosition{ 0.0f, 2.0f, 4.0f };

    struct CameraData {
        glm::mat4 view;
        glm::mat4 projection;
    };
    CameraData m_cameraData;
    KDGpu::Buffer m_cameraBuffer;
    void *m_cameraBufferData{ nullptr };
    KDGpu::BindGroup m_cameraBindGroup;

    // We also need the viewport matrix in the geometry shader to transform to screen space
    glm::mat4 m_viewportMatrix;
    KDGpu::Buffer m_viewportBuffer;
    void *m_viewportBufferData{ nullptr };
    KDGpu::BindGroup m_viewportBindGroup;
    bool m_viewportDirty{ true };

    KDGpu::Buffer m_vertexBuffer;

    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::RenderPassCommandRecorderOptions m_opaquePassOptions;
    KDGpu::CommandBuffer m_commandBuffer;

    glm::mat4 m_transform;
    KDGpu::Buffer m_transformBuffer;
    void *m_transformBufferData{ nullptr };
    KDGpu::BindGroup m_transformBindGroup;

    struct MaterialData {
        glm::vec4 baseColorFactor{ 1.0f, 0.0f, 0.0f, 1.0f };
        glm::vec4 wireframeColorAndWidth{ 0.0f, 0.0f, 0.0f, 2.0f }; // Black wireframe, 2 pixel width
    };
    MaterialData m_materialData;
    KDGpu::Buffer m_materialBuffer;
    void *m_materialBufferData{ nullptr };
    KDGpu::BindGroup m_materialBindGroup;
};
