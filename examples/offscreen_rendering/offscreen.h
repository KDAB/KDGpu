#pragma once

#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
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
    Offscreen();
    ~Offscreen();

    void initializeScene();
    void cleanupScene();
    void resize(uint32_t width, uint32_t height);
    void updateScene(const std::vector<glm::vec2> &data); // Pass in data here?
    void render(); // Return the image data?

private:
    void releaseStagingBuffers();
    void createRenderTargets();

    // Rendering resources
    std::unique_ptr<GraphicsApi> m_api;
    Instance m_instance;
    Device m_device;
    Queue m_queue;

    uint32_t m_width{ 1920 };
    uint32_t m_height{ 1280 };

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
    std::vector<UploadStagingBuffer> m_stagingBuffers;

    Buffer m_buffer;
    GraphicsPipeline m_pipeline;
    PipelineLayout m_pipelineLayout;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;
};
