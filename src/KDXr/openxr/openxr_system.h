/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>
#include <KDXr/kdxr_core.h>

#include <KDGpu/handle.h>
#include <KDGpu/graphics_api.h>

#include <openxr/openxr.h>

// Define XR_USE_GRAPHICS_API_* and include vulkan.h before openxr_platform.h
// Add other graphics APIs here as needed
#define XR_USE_GRAPHICS_API_VULKAN

#ifdef XR_USE_PLATFORM_ANDROID
// Android OpenXR integration need the definition of jobject
#include <jni.h>
#endif

#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>

namespace KDGpu {
class Adapter;
class Instance;
} // namespace KDGpu

namespace KDXr {

class OpenXrResourceManager;
struct Instance_t;
struct System_t;

/**
 * @brief OpenXrSystem
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSystem {
    explicit OpenXrSystem(OpenXrResourceManager *_openxrResourceManager,
                          XrSystemId _system,
                          const KDGpu::Handle<Instance_t> &instanceHandle) noexcept;

    SystemProperties queryProperties() const;
    std::vector<ViewConfigurationType> queryViewConfigurations() const;
    std::vector<EnvironmentBlendMode> queryEnvironmentBlendModes(ViewConfigurationType viewConfiguration) const;
    std::vector<ViewConfigurationView> queryViews(ViewConfigurationType viewConfiguration) const;
    GraphicsRequirements queryGraphicsRequirements(KDGpu::GraphicsApi *graphicsApi) const;
    std::vector<std::string> requiredGraphicsInstanceExtensions(KDGpu::GraphicsApi *graphicsApi) const;
    KDGpu::Adapter *requiredGraphicsAdapter(KDGpu::GraphicsApi *graphicsApi, const KDGpu::Instance &graphicsInstance) const;
    std::vector<std::string> requiredGraphicsDeviceExtensions(KDGpu::GraphicsApi *graphicsApi) const;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSystemId system{ XR_NULL_SYSTEM_ID };
    KDGpu::Handle<Instance_t> instanceHandle;

    // Vulkan support for OpenXR
    mutable PFN_xrGetVulkanGraphicsRequirementsKHR m_xrGetVulkanGraphicsRequirementsKHR{ nullptr };
    mutable PFN_xrGetVulkanInstanceExtensionsKHR m_xrGetVulkanInstanceExtensionsKHR{ nullptr };
    mutable PFN_xrGetVulkanDeviceExtensionsKHR m_xrGetVulkanDeviceExtensionsKHR{ nullptr };
    mutable PFN_xrGetVulkanGraphicsDeviceKHR m_xrGetVulkanGraphicsDeviceKHR{ nullptr };

private:
    void resolveVulkanFunctions(XrInstance instance) const;
};

} // namespace KDXr
