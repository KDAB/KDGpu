/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/engine_layer.h>
#include <KDGpuExample/xr_compositor/xr_compositor_layer.h>

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

#include <KDUtils/logging.h>

#include <kdbindings/property.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

#include <array>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace KDGpu {
class RenderPassCommandRecorder;
}

using namespace KDGpu;

namespace KDGpuExample {

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

    template<typename T>
    XrCompositorLayer *addCompositorLayer(std::unique_ptr<T> &&layer)
    {
        // Caller has to transfer ownership to us so there should not be an old parent
        assert(layer->m_engineLayer == nullptr);

        layer->m_engineLayer = this;
        static_cast<XrCompositorLayer *>(layer.get())->initialize();
        m_compositorLayerObjects.push_back(std::move(layer));
        XrCompositorLayer *layerPtr = m_compositorLayerObjects.back().get();
        return layerPtr;
    }

    template<typename T, typename... Ts>
    T *createCompositorLayer(Ts... args)
    {
        auto layer = std::make_unique<T>(std::forward<Ts>(args)...);
        return static_cast<T *>(this->addCompositorLayer(std::move(layer)));
    }

    template<typename T>
    std::unique_ptr<XrCompositorLayer> takeCompositorLayer(T *layer)
    {
        // Find the layer from the raw pointer
        auto layerIt = std::find_if(
                m_compositorLayerObjects.begin(),
                m_compositorLayerObjects.end(),
                [layer](const auto &v) {
                    return v.get() == layer;
                });

        // Didn't find a matching layer?
        if (layerIt == m_compositorLayerObjects.end())
            return {};

        // Unparent the layer and return it along with ownership!
        auto takenLayer = std::move(*layerIt);
        takenLayer->cleanup();
        takenLayer->m_engineLayer = nullptr;
        m_compositorLayerObjects.erase(layerIt);
        return takenLayer;
    }

    std::span<const std::unique_ptr<XrCompositorLayer>> compositorLayers() const noexcept { return m_compositorLayerObjects; }

    void clearCompositorLayers()
    {
        while (!m_compositorLayerObjects.empty())
            m_compositorLayerObjects.pop_back();
    }

    std::span<const KDXr::ViewConfigurationView> viewConfigurationViews() const noexcept { return m_viewConfigurationViews; }

protected:
    virtual void onInstanceLost();
    virtual void onInteractionProfileChanged();

    void onAttached() override;
    void onDetached() override;
    void update() override;
    virtual void pollActions(KDXr::Time predictedDisplayTime);
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    // Helpers for the composition layers to use
    std::shared_ptr<spdlog::logger> logger() const noexcept { return m_logger; }
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
    KDXr::Instance m_xrInstance;
    KDXr::System *m_system{ nullptr };
    KDXr::Session m_session;
    KDXr::ReferenceSpace m_referenceSpace;

    const std::vector<KDXr::ViewConfigurationType> m_applicationViewConfigurations{
        KDXr::ViewConfigurationType::PrimaryStereo,
        KDXr::ViewConfigurationType::PrimaryMono
    };
    KDXr::ViewConfigurationType m_selectedViewConfiguration{ KDXr::ViewConfigurationType::MaxEnum };
    KDXr::EnvironmentBlendMode m_selectedEnvironmentBlendMode{ KDXr::EnvironmentBlendMode::MaxEnum };
    std::vector<KDXr::ViewConfigurationView> m_viewConfigurationViews;

    Format m_colorSwapchainFormat{ Format::UNDEFINED };
    Format m_depthSwapchainFormat{ Format::UNDEFINED };

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

    // The vector of composition layers to be submitted to the compositor. Each of the compositor layer objects
    // will provide a composition layer to be added to this vector.
    std::vector<KDXr::CompositionLayer *> m_compositorLayers;
    std::vector<std::unique_ptr<XrCompositorLayer>> m_compositorLayerObjects;

    friend class XrCompositorLayer;
};

} // namespace KDGpuExample
