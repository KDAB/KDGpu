/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_system.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

// Define XR_USE_GRAPHICS_API_* and include vulkan.h before openxr_platform.h
// Add other graphics APIs here as needed
#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>

namespace KDXr {

class OpenXrResourceManager;
struct Instance_t;

/**
 * @brief OpenXrSystem
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSystem : public ApiSystem {
    explicit OpenXrSystem(OpenXrResourceManager *_openxrResourceManager,
                          XrSystemId _system,
                          const KDGpu::Handle<Instance_t> &instanceHandle) noexcept;

    SystemProperties queryProperties() const final;
    std::vector<ViewConfigurationType> queryViewConfigurations() const final;
    std::vector<EnvironmentBlendMode> queryEnvironmentBlendModes(ViewConfigurationType viewConfiguration) const final;
    std::vector<ViewConfigurationView> queryViews(ViewConfigurationType viewConfiguration) const final;
    GraphicsRequirements queryGraphicsRequirements(KDGpu::GraphicsApi *graphicsApi) const final;
    std::vector<std::string> requiredGraphicsInstanceExtensions(KDGpu::GraphicsApi *graphicsApi) const final;
    KDGpu::Adapter *requiredGraphicsAdapter(KDGpu::GraphicsApi *graphicsApi, const KDGpu::Instance &graphicsInstance) const final;
    std::vector<std::string> requiredGraphicsDeviceExtensions(KDGpu::GraphicsApi *graphicsApi) const final;

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
