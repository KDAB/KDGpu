/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/engine_layer.h>

#include <KDXr/instance.h>
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

namespace KDGpu {
class RenderPassCommandRecorder;
}

using namespace KDGpu;

namespace KDGpuExample {

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
    virtual void renderView() = 0; // To render a single view at a time
    virtual void resize() = 0;

    void onAttached() override;
    void onDetached() override;
    void update() override;
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    // Graphics API related helpers
    void createGraphicsInstance();
    void destroyGraphicsInstance();

    void createGraphicsDevice();
    void destroyGraphicsDevice();

    // XR related helpers
    void createXrInstance();
    void destroyXrInstance();

    void createXrDebugMessenger();
    void destroyXrDebugMessenger();

    void getXrInstanceProperties();
    void getXrSystemId();
    void getXrViewConfigurations();

    void createXrSession();
    void destroyXrSession();

    void createXrReferenceSpace();
    void destroyXrReferenceSpace();

    void createXrSwapchains();
    void destroyXrSwapchains();

    void pollXrEvents();

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

    Format m_colorSwapchainFormat{ Format::UNDEFINED };
    Format m_depthSwapchainFormat{ Format::UNDEFINED };

    // Xr related members
    std::unique_ptr<KDXr::XrApi> m_xrApi;
    KDXr::Instance m_kdxrInstance; // TODO: Rename to m_xrInstance etc as we replace raw OpenXR calls with KDXr
    KDXr::System *m_kdxrSystem{ nullptr };

    std::vector<KDXr::ViewConfigurationType> m_applicationViewConfigurations{ KDXr::ViewConfigurationType::PrimaryStereo, KDXr::ViewConfigurationType::PrimaryMono };
    KDXr::ViewConfigurationType m_selectedViewConfiguration{ KDXr::ViewConfigurationType::MaxEnum };
    KDXr::EnvironmentBlendMode m_selectedEnvironmentBlendMode{ KDXr::EnvironmentBlendMode::MaxEnum };

    // OpenXR related members (to be removed once KDXr is suitable for use)
    std::vector<std::string> m_xrRequestedApiLayers{};
    std::vector<const char *> m_xrActiveApiLayers{};

    std::vector<std::string> m_xrRequestedInstanceExtensions{};
    std::vector<const char *> m_xrActiveInstanceExtensions{};

    XrInstance m_xrInstance{ XR_NULL_HANDLE };
    XrDebugUtilsMessengerEXT m_debugUtilsMessenger{};

    XrFormFactor m_formFactor{ XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY };
    XrSystemId m_systemId = {};
    XrSystemProperties m_systemProperties{ XR_TYPE_SYSTEM_PROPERTIES };

    std::vector<XrViewConfigurationType> m_xrApplicationViewConfigurationTypes{ XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO };
    std::vector<XrViewConfigurationType> m_xrViewConfigurationTypes;

    XrViewConfigurationType m_xrViewConfiguration{ XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM };
    std::vector<XrEnvironmentBlendMode> m_xrEnvironmentBlendModes;
    XrEnvironmentBlendMode m_xrEnvironmentBlendMode{ XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM };
    std::vector<XrViewConfigurationView> m_xrViewConfigurationViews;

    PFN_xrGetVulkanGraphicsRequirementsKHR m_xrGetVulkanGraphicsRequirementsKHR{ nullptr };
    PFN_xrGetVulkanInstanceExtensionsKHR m_xrGetVulkanInstanceExtensionsKHR{ nullptr };
    PFN_xrGetVulkanDeviceExtensionsKHR m_xrGetVulkanDeviceExtensionsKHR{ nullptr };
    PFN_xrGetVulkanGraphicsDeviceKHR m_xrGetVulkanGraphicsDeviceKHR{ nullptr };

    XrSession m_xrSession{ XR_NULL_HANDLE };
    XrSessionState m_xrSessionState{ XR_SESSION_STATE_UNKNOWN };
    bool m_xrSessionRunning{ false };

    XrSpace m_xrReferenceSpace{ XR_NULL_HANDLE };

    struct SwapchainInfo {
        XrSwapchain swapchain{ XR_NULL_HANDLE };
        std::vector<Texture> images;
        std::vector<TextureView> imageViews;
    };
    std::vector<SwapchainInfo> m_colorSwapchainInfos;
    std::vector<SwapchainInfo> m_depthSwapchainInfos;

    std::vector<Format> m_applicationColorSwapchainFormats{
        Format::B8G8R8A8_SRGB,
        Format::R8G8B8A8_SRGB,
        Format::B8G8R8A8_UNORM,
        Format::R8G8B8A8_UNORM
    };
    std::vector<Format> m_applicationDepthSwapchainFormats{
        Format::D32_SFLOAT,
        Format::D16_UNORM
    };
    std::vector<int64_t> m_xrSwapchainFormats;
    int64_t m_xrColorSwapchainFormat{ 0 };
    int64_t m_xrDepthSwapchainFormat{ 0 };

    // TODO: Extent so this supports other types of composition layers beyond projection
    struct CompositorLayerInfo {
        XrTime predictedDisplayTime{ 0 };
        std::vector<XrCompositionLayerBaseHeader *> layers;
        std::vector<XrCompositionLayerProjection> layerProjections;
        std::vector<XrCompositionLayerProjectionView> layerProjectionViews;

        void reset(XrTime displayTime)
        {
            predictedDisplayTime = displayTime;
            layers.clear();
            layerProjections.clear();
            layerProjectionViews.clear();
        }
    };
    CompositorLayerInfo m_xrCompositorLayerInfo;

    uint32_t m_currentViewIndex{ 0 };
    uint32_t m_currentColorImageIndex{ 0 };
    uint32_t m_currentDepthImageIndex{ 0 };

    // TODO: Extract into structs in KDXr
    struct Pose {
        glm::quat orientation{};
        glm::vec3 position{ 0.0f };
    };

    struct FieldOfView {
        float angleLeft{ 0.0f };
        float angleRight{ 0.0f };
        float angleUp{ 0.0f };
        float angleDown{ 0.0f };
    };

    struct View {
        Pose pose{};
        FieldOfView fieldOfView{};
    };

    std::array<View, MAX_VIEWS> m_views;
};

} // namespace KDGpuExample
