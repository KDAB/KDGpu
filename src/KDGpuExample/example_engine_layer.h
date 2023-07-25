/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/engine_layer.h>
#include <KDGpuKDGui/view.h>

#include <KDGpu/device.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
#include <KDGpu/surface.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDFoundation/logging.h>

#include <array>
#include <functional>
#include <memory>
#include <vector>

struct ImGuiContext;

namespace KDGpu {
class RenderPassCommandRecorder;
}

using namespace KDGpu;

namespace KDGpuExample {

class ImGuiItem;

// This determines the maximum number of frames that can be in-flight at any one time.
// With the default setting of 2, we can be recording the commands for frame N+1 whilst
// the GPU is executing those for frame N. We cannot then record commands for frame N+2
// until the GPU signals it is done with frame N.
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

/**
    @class ExampleEngineLayer
    @brief ExampleEngineLayer ...
    @ingroup kdgpuexample
    @headerfile example_engine_layer.h <KDGpuExample/example_engine_layer.h>
 */
class KDGPUEXAMPLE_EXPORT ExampleEngineLayer : public EngineLayer
{
public:
    ExampleEngineLayer(const SampleCountFlagBits samples = SampleCountFlagBits::Samples1Bit);
    ~ExampleEngineLayer() override;

    KDGpuKDGui::View *window() { return m_window.get(); }

protected:
    virtual void initializeScene() = 0;
    virtual void cleanupScene() = 0;
    virtual void updateScene() = 0;
    virtual void render() = 0;
    virtual void resize() = 0;

    virtual void drawImGuiOverlay(ImGuiContext *ctx);
    virtual void renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex = 0);
    void registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func);
    void clearImGuiOverlayDrawFunctions();

    void onAttached() override;
    void onDetached() override;
    void update() override;
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    void recreateSwapChain();

    void uploadBufferData(const BufferUploadOptions &options);
    void uploadTextureData(const TextureUploadOptions &options);
    void releaseStagingBuffers();

    std::shared_ptr<spdlog::logger> m_logger;
    std::unique_ptr<GraphicsApi> m_api;
    std::unique_ptr<KDGpuKDGui::View> m_window;

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
    std::vector<std::function<void(ImGuiContext *)>> m_imGuiOverlayDrawFunctions;

    uint32_t m_currentSwapchainImageIndex{ 0 };
    uint32_t m_inFlightIndex{ 0 };
    std::array<GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_presentCompleteSemaphores;
    std::array<GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderCompleteSemaphores;
    TextureUsageFlags m_depthTextureUsageFlags;

    std::vector<UploadStagingBuffer> m_stagingBuffers;

    const Format m_swapchainFormat{ Format::B8G8R8A8_UNORM };
    Format m_depthFormat;

    bool m_showSurfaceCapabilities{ false };
    std::string m_capabilitiesString;
};

} // namespace KDGpuExample
