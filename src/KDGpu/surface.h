/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Surface_t;

/*!
    \class Surface
    \brief Represents a platform-specific window surface for presentation
    \ingroup public
    \headerfile surface.h <KDGpu/surface.h>

    <b>Vulkan equivalent:</b> [VkSurfaceKHR](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSurfaceKHR.html)

    Surface represents a native window or display surface where rendered images can be presented.
    It abstracts platform-specific windowing systems (X11, Wayland, Win32, etc.).

    <b>Key features:</b>
    - Platform-independent surface abstraction
    - Query surface capabilities and formats
    - Create swapchains for presentation
    - Check adapter compatibility
    .
    <br/>

    <b>Lifetime:</b> Surface objects are created by Instance from platform window handles and should
    remain valid while the swapchain is in use. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a surface (with platform window):</b>

    \snippet kdgpu_doc_snippets.cpp instance_surface

    <b>Checking adapter surface support:</b>

    \snippet kdgpu_doc_snippets.cpp surface_adapter_support

    <b>Querying surface capabilities:</b>

    \snippet kdgpu_doc_snippets.cpp surface_capabilities

    <b>Creating a swapchain from surface:</b>

    \snippet kdgpu_doc_snippets.cpp surface_swapchain

    <b>Complete window integration example:</b>

    \snippet kdgpu_doc_snippets.cpp surface_complete_integration

    ## Vulkan mapping:
    - Surface creation -> vkCreateXXXSurfaceKHR() (platform-specific)
    - Used in vkGetPhysicalDeviceSurfaceSupportKHR()
    - Used in vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
    - Used in vkCreateSwapchainKHR()

    ## See also:
    \sa Instance, Swapchain, Adapter, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Surface
{
public:
    Surface();
    ~Surface();

    Surface(Surface &&) noexcept;
    Surface &operator=(Surface &&) noexcept;

    Surface(const Surface &) = delete;
    Surface &operator=(const Surface &) = delete;

    Handle<Surface_t> handle() const noexcept { return m_surface; }
    bool isValid() const noexcept { return m_surface.isValid(); }

    operator Handle<Surface_t>() const noexcept { return m_surface; }

private:
    Surface(GraphicsApi *api, const Handle<Surface_t> &surface);

    GraphicsApi *m_api{ nullptr };
    Handle<Surface_t> m_surface;

    friend class Instance;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
