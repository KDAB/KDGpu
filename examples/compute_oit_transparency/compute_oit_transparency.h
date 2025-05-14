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
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

class ComputeOitTransparency : public KDGpuExample::SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void initializeGlobal();
    void initializeParticles();
    void initializeAlpha();
    void initializeCompositing();
    void initializeMeshes();

    struct Particles {
        KDGpu::Buffer particleDataBuffer;
        KDGpu::PipelineLayout computePipelineLayout;
        KDGpu::ComputePipeline computePipeline;
        KDGpu::BindGroup particleBindGroup;
    } m_particles;

    struct Alpha {
        KDGpu::Buffer fragmentLinkedListBuffer;
        KDGpu::Texture fragmentHeadsPointer;
        KDGpu::TextureView fragmentHeadsPointerView;
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
        size_t fragmentLinkedListBufferByteSize;
        KDGpu::BindGroupLayout alphaBindGroupLayout;
        KDGpu::BindGroup alphaLinkedListBindGroup;
        KDGpu::TextureLayout fragmentHeadsPointerLayout{ KDGpu::TextureLayout::Undefined };
    } m_alpha;

    struct Compositing {
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
        KDGpu::PipelineLayout graphicsPipelineLayout;
        KDGpu::GraphicsPipeline graphicsPipeline;
    } m_compositing;

    struct CubeMesh {
        KDGpu::PipelineLayout graphicsPipelineLayout;
        KDGpu::GraphicsPipeline graphicsPipeline;
        KDGpu::Buffer vertexBuffer;
    } m_cubeMesh;

    struct SphereMesh {
        KDGpu::PipelineLayout graphicsPipelineLayout;
        KDGpu::GraphicsPipeline graphicsPipeline;
        KDGpu::Buffer vertexBuffer;
        size_t vertexCount;
    } m_sphereMesh;

    struct Global {
        KDGpu::Buffer cameraDataBuffer;
        KDGpu::BindGroupLayout cameraBindGroupLayout;
        KDGpu::BindGroup cameraBindGroup;
        KDGpu::CommandBuffer commandBuffer;
    } m_global;
};
