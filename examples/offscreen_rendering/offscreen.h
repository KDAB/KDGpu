/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
#include <KDGpu/sampler.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Offscreen
{
public:
    //![1]
    struct Vertex {
        glm::vec2 pos;
        glm::vec4 color;
    };
    //![1]

    Offscreen();
    ~Offscreen();

    void initializeScene();
    void cleanupScene();
    void resize(uint32_t width, uint32_t height);
    void setData(const std::vector<Offscreen::Vertex> &data);
    void setProjection(float left, float right, float bottom, float top);
    void render(const std::string &baseFilename);

private:
    void releaseStagingBuffers();
    void createRenderTargets();

    // Rendering resources
    std::unique_ptr<KDGpu::GraphicsApi> m_api;
    KDGpu::Instance m_instance;
    KDGpu::Device m_device;
    KDGpu::Queue m_queue;
    std::vector<KDGpu::UploadStagingBuffer> m_stagingBuffers;

    uint32_t m_width{ 1920 };
    uint32_t m_height{ 1080 };

    KDGpu::SampleCountFlagBits m_samples{ KDGpu::SampleCountFlagBits::Samples8Bit };
    KDGpu::Texture m_msaaColorTexture;
    KDGpu::TextureView m_msaaColorTextureView;
    KDGpu::Texture m_colorTexture;
    KDGpu::TextureView m_colorTextureView;
    KDGpu::Texture m_depthTexture;
    KDGpu::TextureView m_depthTextureView;

    KDGpu::Texture m_cpuColorTexture;

    enum class TextureBarriers : uint8_t {
        CopySrcPre = 0,
        CopyDstPre,
        CopyDstPost,
        CopySrcPost,
        Count
    };
    KDGpu::TextureMemoryBarrierOptions m_barriers[uint8_t(TextureBarriers::Count)];
    KDGpu::TextureToTextureCopy m_copyOptions;

    const KDGpu::Format m_colorFormat{ KDGpu::Format::R8G8B8A8_UNORM };
#if defined(KDGPU_PLATFORM_APPLE)
    const KDGpu::Format m_depthFormat{ KDGpu::Format::D32_SFLOAT_S8_UINT };
#else
    const KDGpu::Format m_depthFormat{ KDGpu::Format::D24_UNORM_S8_UINT };
#endif

    // Scene Resources
    KDGpu::Buffer m_dataBuffer;
    uint32_t m_pointCount{ 0 };

    glm::mat4 m_proj;
    KDGpu::Buffer m_projBuffer;
    KDGpu::BindGroup m_transformBindGroup;

    KDGpu::Texture m_pointTexture;
    KDGpu::TextureView m_pointTextureView;
    KDGpu::Sampler m_pointSampler;
    KDGpu::BindGroup m_pointTextureBindGroup;

    KDGpu::GraphicsPipeline m_pipeline;
    KDGpu::PipelineLayout m_pipelineLayout;
    KDGpu::RenderPassCommandRecorderOptions m_renderPassOptions;
    KDGpu::CommandBuffer m_commandBuffer;
};
