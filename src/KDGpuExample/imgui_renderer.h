/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/buffer.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/sampler.h>
#include <KDGpu/shader_module.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>

#include <vector>

namespace KDGpu {
class Device;
class Queue;
class RenderPassCommandRecorder;
class RenderPass;
} // namespace KDGpu

struct ImGuiContext;

namespace KDGpuExample {

/**
    @class ImGuiRenderer
    @brief ImGuiRenderer ...
    @ingroup kdgpuexample
    @headerfile imgui_renderer.h <KDGpuExample/imgui_renderer.h>
 */
class KDGPUEXAMPLE_EXPORT ImGuiRenderer
{
public:
    ImGuiRenderer(KDGpu::Device *device, KDGpu::Queue *queue, ImGuiContext *imGuiContext);
    ~ImGuiRenderer();

    ImGuiRenderer(const ImGuiRenderer &other) noexcept = delete;
    ImGuiRenderer &operator=(const ImGuiRenderer &other) noexcept = delete;

    ImGuiRenderer(ImGuiRenderer &&other) noexcept = default;
    ImGuiRenderer &operator=(ImGuiRenderer &&other) noexcept = default;

    void initialize(float scaleFactor, KDGpu::SampleCountFlagBits samples, KDGpu::Format colorFormat, KDGpu::Format depthFormat);
    void updateScale(float scaleFactor);
    void cleanup();

    bool updateGeometryBuffers(uint32_t inFlightIndex);
    void recordCommands(KDGpu::RenderPassCommandRecorder *recorder,
                        KDGpu::Extent2D extent,
                        uint32_t inFlightIndex,
                        KDGpu::RenderPass *currentRenderPass = nullptr,
                        int lastSubpassIndex = 0,
                        bool dynamicRendering = false);

private:
    void initializeFontData(float scaleFactor);

    struct MeshData {
        KDGpu::Buffer vertices;
        KDGpu::Buffer indexBuffer;
        bool isIndexed{ false };
        uint32_t vertexCount{ 0 };
        uint32_t indexCount{ 0 };
        KDGpu::IndexType indexType{ KDGpu::IndexType::Uint32 };
    };

    // TODO: Handle multiple frames in flight
    std::vector<MeshData> m_meshes;
    MeshData *m_mesh{ nullptr };

    KDGpu::BindGroupLayout m_bindGroupLayout;
    KDGpu::BindGroup m_bindGroup;
    KDGpu::Texture m_texture;
    KDGpu::TextureView m_textureView;
    KDGpu::Sampler m_sampler;

    struct PushConstantBlock {
        float scale[2];
        float translate[2];
    };
    PushConstantBlock m_pushConstantBlock;

    KDGpu::Device *m_device{ nullptr };
    KDGpu::Queue *m_queue{ nullptr };
    ImGuiContext *m_imGuiContext{ nullptr };

    KDGpu::ShaderModule m_vertexShader;
    KDGpu::ShaderModule m_fragmentShader;
    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::GraphicsPipelineOptions m_pipelineInfo;
    KDGpu::PipelineLayout m_pipelineLayout;

    float m_oldScaleFactor = 1.0f;
};

} // namespace KDGpuExample
