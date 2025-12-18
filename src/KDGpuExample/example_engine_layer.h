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
#include <KDGpu/gpu_core.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
#include <KDGpu/surface.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDUtils/logging.h>

#include <array>
#include <functional>
#include <memory>
#include <vector>

struct ImGuiContext;

namespace KDGpu {
class RenderPassCommandRecorder;
}

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
    ExampleEngineLayer();
    ~ExampleEngineLayer() override;

    KDGpuKDGui::View *window() { return m_window.get(); }

    void uploadBufferData(const KDGpu::BufferUploadOptions &options);
    void uploadTextureData(const KDGpu::TextureUploadOptions &options);

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    virtual void initializeScene() = 0;
    virtual void cleanupScene() = 0;
    virtual void updateScene() = 0;
    virtual void render() = 0;
    virtual void resize() = 0;

    virtual void drawImGuiOverlay(ImGuiContext *ctx);
    virtual void renderImGuiOverlay(KDGpu::RenderPassCommandRecorder *recorder,
                                    uint32_t inFlightIndex = 0,
                                    KDGpu::RenderPass *currentRenderPass = nullptr,
                                    int lastSubpassIndex = 0);
    virtual void renderImGuiOverlayDynamic(KDGpu::RenderPassCommandRecorder *recorder,
                                           uint32_t inFlightIndex = 0);
    void registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func);
    void clearImGuiOverlayDrawFunctions();
    void recreateImGuiOverlay();

    void onAttached() override;
    void onDetached() override;
    void update() override;
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    virtual void recreateSwapChain();
    void recreateDepthTexture();
    void recreateSampleDependentResources();

    void releaseStagingBuffers();

    std::shared_ptr<spdlog::logger> m_logger;
    std::unique_ptr<KDGpu::GraphicsApi> m_api;
    std::unique_ptr<KDGpuKDGui::View> m_window;

    KDGpu::Extent2D m_swapchainExtent;
    KDBindings::Property<KDGpu::SampleCountFlagBits> m_samples{ KDGpu::SampleCountFlagBits::Samples1Bit };
    KDGpu::TextureUsageFlags m_swapchainUsageFlags{ KDGpu::TextureUsageFlagBits::ColorAttachmentBit };
    std::vector<KDGpu::SampleCountFlagBits> m_supportedSampleCounts;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;
    KDGpu::Adapter *m_adapter;
    KDGpu::Device m_device;
    KDGpu::Queue m_queue;
    KDGpu::PresentMode m_presentMode;
    KDGpu::Swapchain m_swapchain;
    std::vector<KDGpu::TextureView> m_swapchainViews;
    KDGpu::Texture m_depthTexture;
    KDGpu::TextureView m_depthTextureView;

    std::unique_ptr<ImGuiItem> m_imguiOverlay;
    std::vector<std::function<void(ImGuiContext *)>> m_imGuiOverlayDrawFunctions;

    uint32_t m_currentSwapchainImageIndex{ 0 };
    uint32_t m_inFlightIndex{ 0 };
    std::array<KDGpu::GpuSemaphore, MAX_FRAMES_IN_FLIGHT> m_presentCompleteSemaphores; // One Per Frame in flight
    std::vector<KDGpu::GpuSemaphore> m_renderCompleteSemaphores; // One Per Swapchain Image
    KDGpu::TextureUsageFlags m_depthTextureUsageFlags;

    std::vector<KDGpu::UploadStagingBuffer> m_stagingBuffers;

    KDGpu::Format m_swapchainFormat{ KDGpu::Format::B8G8R8A8_UNORM };
    KDGpu::Format m_depthFormat;
    KDGpu::CompositeAlphaFlagBits m_compositeAlpha{ KDGpu::CompositeAlphaFlagBits::OpaqueBit };

    bool m_showSurfaceCapabilities{ false };
    std::string m_capabilitiesString;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace KDGpuExample
