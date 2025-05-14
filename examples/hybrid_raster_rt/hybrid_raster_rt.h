/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/acceleration_structure.h>
#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/raytracing_shader_binding_table.h>
#include <KDGpu/raytracing_pipeline.h>

#include <glm/glm.hpp>

class HybridRasterRt : public KDGpuExample::SimpleExampleEngineLayer
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
    void initializeGBuffer();
    void initializeAlpha();
    void initializeShadows();
    void initializeCompositing();
    void initializeLightDisplay();
    void initializeMeshes();
    void initializeAccelerationStructures();

    struct Particles {
        KDGpu::Buffer particleDataBuffer;
        KDGpu::Buffer blasTransformBuffer;
        KDGpu::PipelineLayout computePipelineLayout;
        KDGpu::ComputePipeline computePipeline;
        KDGpu::BindGroup particleBindGroup;
    } m_particles;

    struct GBuffer {
        KDGpu::Texture posTexture;
        KDGpu::Texture normalTexture;
        KDGpu::Texture colorTexture;
        KDGpu::Texture depthTexture;
        KDGpu::Texture fragmentHeadsPointer;
        KDGpu::Texture shadowTexture;

        KDGpu::TextureView posTextureView;
        KDGpu::TextureView normalTextureView;
        KDGpu::TextureView colorTextureView;
        KDGpu::TextureView depthTextureView;
        KDGpu::TextureView fragmentHeadsPointerView;
        KDGpu::TextureView shadowTextureView;

        KDGpu::Buffer fragmentLinkedListBuffer;
        size_t fragmentLinkedListBufferByteSize;
        KDGpu::TextureLayout fragmentHeadsPointerLayout{ KDGpu::TextureLayout::Undefined };
        KDGpu::TextureLayout shadowTextureLayout{ KDGpu::TextureLayout::Undefined };

        KDGpu::BindGroupLayout opaqueNormalDepthBindGroupLayout;
        KDGpu::BindGroupLayout alphaBindGroupLayout;
        KDGpu::BindGroupLayout shadowBindGroupLayout;

        KDGpu::BindGroup opaqueNormalDepthBindGroup;
        KDGpu::BindGroup alphaLinkedListBindGroup;
        KDGpu::BindGroup shadowBindGroup;

        KDGpu::Sampler sampler;

        void initialize(KDGpu::Device *device);
        void resize(KDGpu::Device *device, KDGpu::Extent2D extent);
        void cleanup();

    } m_gbuffer;

    struct DepthFillPass {
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
    } m_zfillPass;

    struct AlphaFillPass {
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
    } m_alphaPass;

    struct OpaquePass {
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
    } m_opaquePass;

    struct LightDisplayPass {
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
        KDGpu::PipelineLayout graphicsPipelineLayout;
        KDGpu::GraphicsPipeline graphicsPipeline;
    } m_lightDisplayPass;

    struct ShadowPass {
        KDGpu::RayTracingPassCommandRecorderOptions rtPassOptions;
        KDGpu::PipelineLayout pipelineLayout;
        KDGpu::RayTracingPipeline pipeline;
        KDGpu::RayTracingShaderBindingTable sbt;
    } m_shadowPass;

    struct Compositing {
        KDGpu::RenderPassCommandRecorderOptions renderPassOptions;
        KDGpu::PipelineLayout graphicsPipelineLayout;
        KDGpu::GraphicsPipeline graphicsPipeline;
    } m_compositing;

    struct PlaneMesh {
        KDGpu::PipelineLayout zFillGraphicsPipelineLayout;
        KDGpu::PipelineLayout opaqueFillGraphicsPipelineLayout;
        KDGpu::GraphicsPipeline zFillGraphicsPipeline;
        KDGpu::GraphicsPipeline opaqueFillGraphicsPipeline;
        KDGpu::Buffer vertexBuffer;
        size_t vertexCount;
    } m_planeMesh;

    struct SphereMesh {
        KDGpu::PipelineLayout zFillGraphicsPipelineLayout;
        KDGpu::PipelineLayout alphaFillGraphicsPipelineLayout;
        KDGpu::PipelineLayout opaqueFillGraphicsPipelineLayout;
        KDGpu::GraphicsPipeline zFillGraphicsPipeline;
        KDGpu::GraphicsPipeline alphaFillGraphicsPipeline;
        KDGpu::GraphicsPipeline opaqueFillGraphicsPipeline;
        KDGpu::Buffer vertexBuffer;
        size_t vertexCount;
    } m_sphereMesh;

    struct AccelerationStructures {
        KDGpu::AccelerationStructure opaqueSpheresBlas;
        KDGpu::AccelerationStructure alphaSpheresBlas;
        KDGpu::AccelerationStructure opaquePlaneBlas;
        KDGpu::AccelerationStructure tBlas;

        KDGpu::BuildAccelerationStructureOptions opaqueSpheresASBuildOptions;
        KDGpu::BuildAccelerationStructureOptions alphaSpheresASBuildOptions;
        KDGpu::BuildAccelerationStructureOptions opaquePlaneASBuildOptions;
        KDGpu::BuildAccelerationStructureOptions tlASBuildOptions;

        KDGpu::BindGroupLayout tsASBindGroupLayout;
        KDGpu::BindGroup tsASBindGroup;

        bool hasBuiltStaticBlas = false;
    } m_as;

    struct Global {
        KDGpu::Buffer cameraDataBuffer;
        KDGpu::BindGroupLayout cameraBindGroupLayout;
        KDGpu::BindGroup cameraBindGroup;

        KDGpu::PushConstantRange lightPosPushConstant;
        glm::vec3 lightPos;

        KDGpu::CommandBuffer commandBuffer;
    } m_global;
};
