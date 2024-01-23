/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/engine_layer.h>

#include <KDXr/instance.h>
#include <KDXr/reference_space.h>
#include <KDXr/session.h>
#include <KDXr/openxr/openxr_api.h>

#include <KDGpu/device.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/instance.h>
#include <KDGpu/queue.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDFoundation/logging.h>

#include <kdbindings/property.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

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

constexpr uint32_t MAX_VIEWS = 2;

/**
    @class XrExampleEngineLayer
    @brief XrExampleEngineLayer provides a base for OpenXR examples.
    @ingroup kdgpuexample
    @headerfile xr_example_engine_layer.h <KDGpuExample/xr_example_engine_layer.h>
 */
class KDGPUEXAMPLE_EXPORT XrExampleEngineLayer : public EngineLayer
{
public:
    XrExampleEngineLayer();
    ~XrExampleEngineLayer() override;

protected:
    virtual void initializeScene() = 0;
    virtual void cleanupScene() = 0;
    virtual void updateScene() = 0;
    virtual void renderView() = 0; // To render a single view at a time for the projection layer
    virtual void renderQuad() = 0; // To render the quad layer
    virtual void resize() = 0;

    // TODO: Can we share this with ExampleEngineLayer?
    virtual void drawImGuiOverlay(ImGuiContext *ctx);
    virtual void renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex = 0);
    void registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func);
    void clearImGuiOverlayDrawFunctions();
    void recreateImGuiOverlay();
    void updateImGuiOverlay();

    virtual void onInstanceLost();

    void onAttached() override;
    void onDetached() override;
    void update() override;
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    void uploadBufferData(const BufferUploadOptions &options);
    void uploadTextureData(const TextureUploadOptions &options);
    void releaseStagingBuffers();

    std::shared_ptr<spdlog::logger> m_logger;
    std::unique_ptr<GraphicsApi> m_api;

    KDBindings::Property<SampleCountFlagBits> m_samples{ SampleCountFlagBits::Samples1Bit };
    std::vector<SampleCountFlagBits> m_supportedSampleCounts;
    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Queue m_queue;

    std::vector<UploadStagingBuffer> m_stagingBuffers;

    // Xr related members
    std::unique_ptr<KDXr::XrApi> m_xrApi;
    KDXr::Instance m_kdxrInstance; // TODO: Rename to m_xrInstance etc as we replace raw OpenXR calls with KDXr
    KDXr::System *m_kdxrSystem{ nullptr };
    KDXr::Session m_kdxrSession;
    KDXr::ReferenceSpace m_kdxrReferenceSpace;

    std::vector<KDXr::ViewConfigurationType> m_applicationViewConfigurations{ KDXr::ViewConfigurationType::PrimaryStereo, KDXr::ViewConfigurationType::PrimaryMono };
    KDXr::ViewConfigurationType m_selectedViewConfiguration{ KDXr::ViewConfigurationType::MaxEnum };
    KDXr::EnvironmentBlendMode m_selectedEnvironmentBlendMode{ KDXr::EnvironmentBlendMode::MaxEnum };
    std::vector<KDXr::ViewConfigurationView> m_viewConfigurationViews;

    Format m_colorSwapchainFormat{ Format::UNDEFINED };
    Format m_depthSwapchainFormat{ Format::UNDEFINED };

    // For projection layer
    std::vector<KDXr::SwapchainInfo> m_colorSwapchains;
    std::vector<KDXr::SwapchainInfo> m_depthSwapchains;

    // For quad layer
    const KDGpu::Extent2D m_quadSize{ 1280, 720 };
    KDXr::SwapchainInfo m_quadColorSwapchain;
    KDXr::SwapchainInfo m_quadDepthSwapchain;
    KDXr::Pose m_quadPose{ .orientation = { 0.0f, 0.0f, 0.0f, 1.0f }, .position = { 0.0f, 0.75f, -1.5f } };
    KDGpu::Extent2Df m_quadWorldSize{ 2.0f, 2.0f * m_quadSize.height / m_quadSize.width };

    std::unique_ptr<ImGuiItem> m_imguiOverlay;
    std::vector<std::function<void(ImGuiContext *)>> m_imGuiOverlayDrawFunctions;

    const std::vector<Format> m_applicationColorSwapchainFormats{
        Format::B8G8R8A8_SRGB,
        Format::R8G8B8A8_SRGB,
        Format::B8G8R8A8_UNORM,
        Format::R8G8B8A8_UNORM
    };
    const std::vector<Format> m_applicationDepthSwapchainFormats{
        Format::D32_SFLOAT,
        Format::D16_UNORM
    };

    // TODO: Add api to the example engine layer to manage layers for the compositor.
    // For now we assume a single projection layer with however many views were queried.
    std::vector<KDXr::CompositionLayer *> m_compositorLayers; // Pointers to all the layers to be rendered
    std::vector<KDXr::ProjectionLayer> m_projectionLayers{ 1 }; // Projection layers to be rendered. Default to 1 projection layer
    std::vector<KDXr::ProjectionLayerView> m_projectionLayerViews{ MAX_VIEWS }; // Projection layer views. One per view for each projection layer
    std::vector<KDXr::QuadLayer> m_quadLayers{ 1 }; // Quad layers to be rendered. Default to 1 quad layer
    // TODO: Add support for other types of layers

    uint32_t m_currentViewIndex{ 0 };
    uint32_t m_currentColorImageIndex{ 0 };
    uint32_t m_currentDepthImageIndex{ 0 };

    std::array<KDXr::View, MAX_VIEWS> m_views;
};

} // namespace KDGpuExample
