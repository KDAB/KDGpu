/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/acceleration_structure.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/raytracing_pass_command_recorder.h>
#include <KDGpu/raytracing_shader_binding_table.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace KDGpuExample;

class HelloSphereRt : public SimpleExampleEngineLayer
{
public:
    HelloSphereRt();

protected:
    void createRayTracingPipeline();
    void createShaderBindingTable();
    void createAccelerationStructures();
    void createBindGroups();

    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    PipelineLayout m_pipelineLayout;
    BindGroupLayout m_rtBindGroupLayout;
    BindGroupLayout m_cameraBindGroupLayout;
    BindGroupLayout m_sphereDataBindGroupLayout;
    RayTracingPipeline m_pipeline;
    CommandBuffer m_commandBuffer;
    Buffer m_aabbBuffer;
    Buffer m_cameraUBOBuffer;
    Buffer m_sphereDataSSBOBuffer;
    RayTracingShaderBindingTable m_sbt;
    AccelerationStructure m_bottomLevelAs;
    AccelerationStructure m_topLevelAs;
    BindGroup m_rtBindGroup;
    BindGroup m_cameraBindGroup;
    BindGroup m_sphereDataBindGroup;
    std::vector<TextureLayout> m_swapchainImageLayouts;
};
