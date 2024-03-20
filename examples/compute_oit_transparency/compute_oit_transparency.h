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

using namespace KDGpuExample;

class ComputeOitTransparency : public SimpleExampleEngineLayer
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
        Buffer particleDataBuffer;
        PipelineLayout computePipelineLayout;
        ComputePipeline computePipeline;
        BindGroup particleBindGroup;
    } m_particles;

    struct Alpha {
        Buffer fragmentLinkedListBuffer;
        Texture fragmentHeadsPointer;
        TextureView fragmentHeadsPointerView;
        RenderPassCommandRecorderOptions renderPassOptions;
        size_t fragmentLinkedListBufferByteSize;
        BindGroupLayout alphaBindGroupLayout;
        BindGroup alphaLinkedListBindGroup;
        TextureLayout fragmentHeadsPointerLayout{ TextureLayout::Undefined };
    } m_alpha;

    struct Compositing {
        RenderPassCommandRecorderOptions renderPassOptions;
        PipelineLayout graphicsPipelineLayout;
        GraphicsPipeline graphicsPipeline;
    } m_compositing;

    struct CubeMesh {
        PipelineLayout graphicsPipelineLayout;
        GraphicsPipeline graphicsPipeline;
        Buffer vertexBuffer;
    } m_cubeMesh;

    struct SphereMesh {
        PipelineLayout graphicsPipelineLayout;
        GraphicsPipeline graphicsPipeline;
        Buffer vertexBuffer;
        size_t vertexCount;
    } m_sphereMesh;

    struct Global {
        Buffer cameraDataBuffer;
        BindGroupLayout cameraBindGroupLayout;
        BindGroup cameraBindGroup;
        CommandBuffer commandBuffer;
    } m_global;
};
