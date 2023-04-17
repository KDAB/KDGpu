#pragma once

#include <KDGpu_KDGui/engine_layer.h>
#include <KDGpu_KDGui/view.h>

#include <KDGpu/device.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
#include <KDGpu/surface.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

#include <array>
#include <memory>
#include <vector>

struct ImGuiContext;

namespace KDGpu {
class RenderPassCommandRecorder;
}

using namespace KDGpu;

namespace KDGpuKDGui {

class ImGuiItem;

// This determines the maximum number of frames that can be in-flight at any one time.
// With the default setting of 2, we can be recording the commands for frame N+1 whilst
// the GPU is executing those for frame N. We cannot then record commands for frame N+2
// until the GPU signals it is done with frame N.
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class KDGPU_KDGUI_EXPORT ExampleEngineLayer : public EngineLayer
{
public:
    ExampleEngineLayer();
    explicit ExampleEngineLayer(const SampleCountFlagBits samples);
    ~ExampleEngineLayer() override;

    View *window() { return m_window.get(); }

protected:
    virtual void initializeScene() = 0;
    virtual void cleanupScene() = 0;
    virtual void updateScene() = 0;
    virtual void render() = 0;
    virtual void resize() = 0;

    virtual void drawImGuiOverlay(ImGuiContext *ctx);
    virtual void renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex);

    void onAttached() override;
    void onDetached() override;
    void update() override;

    void recreateSwapChain();

    void uploadBufferData(const BufferUploadOptions &options);
    void uploadTextureData(const TextureUploadOptions &options);
    void releaseStagingBuffers();

    std::unique_ptr<GraphicsApi> m_api;
    std::unique_ptr<View> m_window;

    const SampleCountFlagBits m_samples{ SampleCountFlagBits::Samples1Bit };
    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Queue m_queue;
    PresentMode m_presentMode;
    Swapchain m_swapchain;
    std::vector<TextureView> m_swapchainViews;
    Texture m_depthTexture;
    TextureView m_depthTextureView;

    std::unique_ptr<ImGuiItem> m_imguiOverlay;

    uint32_t m_currentSwapchainImageIndex{ 0 };
    uint32_t m_inFlightIndex{ 0 };
    std::array<GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_presentCompleteSemaphores;
    std::array<GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderCompleteSemaphores;

    std::vector<UploadStagingBuffer> m_stagingBuffers;

    const Format m_swapchainFormat{ Format::B8G8R8A8_UNORM };
#if defined(KDGPU_PLATFORM_MACOS)
    const Format m_depthFormat{ Format::D32_SFLOAT_S8_UINT };
#else
    const Format m_depthFormat{ Format::D24_UNORM_S8_UINT };
#endif
};

} // namespace KDGpuKDGui
