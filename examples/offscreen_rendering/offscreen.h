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

using namespace KDGpu;

class Offscreen
{
public:
    struct Vertex {
        glm::vec2 pos;
        glm::vec4 color;
    };

    Offscreen();
    ~Offscreen();

    void initializeScene();
    void cleanupScene();
    void resize(uint32_t width, uint32_t height);
    void setData(const std::vector<Offscreen::Vertex> &data); // Pass in data here?
    void render(); // Return the image data?

private:
    void releaseStagingBuffers();
    void createRenderTargets();

    // Rendering resources
    std::unique_ptr<GraphicsApi> m_api;
    Instance m_instance;
    Device m_device;
    Queue m_queue;
    std::vector<UploadStagingBuffer> m_stagingBuffers;

    uint32_t m_width{ 1920 };
    uint32_t m_height{ 1080 };

    SampleCountFlagBits m_samples{ SampleCountFlagBits::Samples8Bit };
    Texture m_msaaColorTexture;
    TextureView m_msaaColorTextureView;
    Texture m_colorTexture;
    TextureView m_colorTextureView;
    Texture m_depthTexture;
    TextureView m_depthTextureView;

    Texture m_cpuColorTexture;

    enum class TextureBarriers : uint8_t {
        CopySrcPre = 0,
        CopyDstPre,
        CopyDstPost,
        CopySrcPost,
        Count
    };
    TextureMemoryBarrierOptions m_barriers[uint8_t(TextureBarriers::Count)];
    TextureToTextureCopy m_copyOptions;

    const Format m_colorFormat{ Format::R8G8B8A8_UNORM };
#if defined(KDGPU_PLATFORM_MACOS)
    const Format m_depthFormat{ Format::D32_SFLOAT_S8_UINT };
#else
    const Format m_depthFormat{ Format::D24_UNORM_S8_UINT };
#endif

    // Scene Resources
    Buffer m_dataBuffer;
    uint32_t m_pointCount{ 0 };

    Texture m_pointTexture;
    TextureView m_pointTextureView;
    Sampler m_pointSampler;
    BindGroup m_pointTextureBindGroup;

    GraphicsPipeline m_pipeline;
    PipelineLayout m_pipelineLayout;
    RenderPassCommandRecorderOptions m_renderPassOptions;
    CommandBuffer m_commandBuffer;
};
