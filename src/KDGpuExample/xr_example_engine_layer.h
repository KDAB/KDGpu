/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/engine_layer.h>

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

#include <KDBindings/property.h>

#include <openxr/openxr.h>

#include <array>
#include <functional>
#include <memory>
#include <vector>

namespace KDGpu {
class RenderPassCommandRecorder;
}

using namespace KDGpu;

namespace KDGpuExample {

// This determines the maximum number of frames that can be in-flight at any one time.
// With the default setting of 2, we can be recording the commands for frame N+1 whilst
// the GPU is executing those for frame N. We cannot then record commands for frame N+2
// until the GPU signals it is done with frame N.
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

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
    virtual void render() = 0;
    virtual void resize() = 0;

    void onAttached() override;
    void onDetached() override;
    void update() override;
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

    // XR related helpers
    void createXrInstance();
    void destroyXrInstance();

    void createXrDebugMessenger();
    void destroyXrDebugMessenger();

    void getXrInstanceProperties();
    void getXrSystemId();

    virtual void recreateSwapChain();
    void recreateDepthTexture();
    void recreateSampleDependentResources();

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
    Swapchain m_swapchain;
    std::vector<TextureView> m_swapchainViews;
    Texture m_depthTexture;
    TextureView m_depthTextureView;

    uint32_t m_currentSwapchainImageIndex{ 0 };
    uint32_t m_inFlightIndex{ 0 };
    TextureUsageFlags m_depthTextureUsageFlags{};

    std::vector<UploadStagingBuffer> m_stagingBuffers;

    const Format m_swapchainFormat{ Format::B8G8R8A8_UNORM };
    Format m_depthFormat;

    // Xr related members
    std::vector<std::string> m_xrRequestedApiLayers{};
    std::vector<const char *> m_xrActiveApiLayers{};

    std::vector<std::string> m_xrRequestedInstanceExtensions{};
    std::vector<const char *> m_xrActiveInstanceExtensions{};

    XrInstance m_xrInstance{ XR_NULL_HANDLE };
    XrDebugUtilsMessengerEXT m_debugUtilsMessenger{};

    XrFormFactor m_formFactor{ XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY };
    XrSystemId m_systemID = {};
    XrSystemProperties m_systemProperties{ XR_TYPE_SYSTEM_PROPERTIES };
};

} // namespace KDGpuExample
