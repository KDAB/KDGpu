#pragma once

#include <toy_renderer_kdgui/engine_layer.h>
#include <toy_renderer_kdgui/view.h>

#include <toy_renderer/device.h>
#include <toy_renderer/gpu_semaphore.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/surface.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/texture_view.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <toy_renderer_kdgui/toy_renderer_kdgui_export.h>

#include <array>
#include <memory>
#include <vector>

using namespace ToyRenderer;
using namespace ToyRendererKDGui;

// This determines the maximum number of frames that can be in-flight at any one time.
// With the default setting of 2, we can be recording the commands for frame N+1 whilst
// the GPU is executing those for frame N. We cannot then record commands for frame N+2
// until the GPU signals it is done with frame N.
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// TODO: Should we move this into a small library specifically for the examples that we ship?
// Let's see how stable this becomes or if we need many variations of it.
class TOY_RENDERER_KDGUI_EXPORT SimpleExampleEngineLayer : public EngineLayer
{
public:
    SimpleExampleEngineLayer();
    ~SimpleExampleEngineLayer();

    View *window() { return m_window.get(); }

protected:
    virtual void initializeScene() = 0;
    virtual void cleanupScene() = 0;
    virtual void updateScene() = 0;
    virtual void render() = 0;
    virtual void resize() = 0;

    void onAttached() override;
    void onDetached() override;
    void update() override;

    void recreateSwapChain();
    void waitForUploadBufferData(const Handle<Buffer_t> &destinationBuffer,
                                 const void *data,
                                 DeviceSize byteSize,
                                 DeviceSize dstOffset = 0);
    void uploadBufferData(const Handle<Buffer_t> &destinationBuffer,
                          const void *data,
                          DeviceSize byteSize,
                          DeviceSize dstOffset = 0);
    void releaseStagingBuffers();

    std::unique_ptr<GraphicsApi> m_api;
    std::unique_ptr<View> m_window;

    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Queue m_queue;
    Swapchain m_swapchain;
    std::vector<TextureView> m_swapchainViews;
    Texture m_depthTexture;
    TextureView m_depthTextureView;

    uint32_t m_currentSwapchainImageIndex{ 0 };
    uint32_t m_inFlightIndex{ 0 };
    std::array<GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_presentCompleteSemaphores;
    std::array<GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderCompleteSemaphores;

    struct StagingBuffer {
        Fence fence;
        Buffer buffer;
        CommandBuffer commandBuffer;
    };
    std::vector<StagingBuffer> m_stagingBuffers;

    const Format m_swapchainFormat{ Format::B8G8R8A8_UNORM };
    const Format m_depthFormat{ Format::D24_UNORM_S8_UINT };
};
