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

class HelloSphereRt : public KDGpuExample::SimpleExampleEngineLayer
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
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::BindGroupLayout m_rtBindGroupLayout;
    KDGpu::BindGroupLayout m_cameraBindGroupLayout;
    KDGpu::BindGroupLayout m_sphereDataBindGroupLayout;
    KDGpu::RayTracingPipeline m_pipeline;
    KDGpu::CommandBuffer m_commandBuffer;
    KDGpu::Buffer m_aabbBuffer;
    KDGpu::Buffer m_cameraUBOBuffer;
    KDGpu::Buffer m_sphereDataSSBOBuffer;
    KDGpu::RayTracingShaderBindingTable m_sbt;
    KDGpu::AccelerationStructure m_bottomLevelAs;
    KDGpu::AccelerationStructure m_topLevelAs;
    KDGpu::BindGroup m_rtBindGroup;
    KDGpu::BindGroup m_cameraBindGroup;
    KDGpu::BindGroup m_sphereDataBindGroup;
    std::vector<KDGpu::TextureLayout> m_swapchainImageLayouts;
};
