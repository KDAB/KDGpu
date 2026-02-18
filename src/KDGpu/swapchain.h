/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/texture.h>
#include <KDGpu/kdgpu_export.h>

#include <span>
#include <vector>

namespace KDGpu {

struct GpuSemaphore_t;
struct Swapchain_t;
struct SwapchainOptions;

/*!
    \class Swapchain
    \brief Manages presentation images for displaying rendered content to a window
    \ingroup public
    \headerfile swapchain.h <KDGpu/swapchain.h>

    <b>Vulkan equivalent:</b> [VkSwapchainKHR](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSwapchainKHR.html)

    Swapchain owns a collection of presentable images (textures) that are displayed to a Surface.
    It manages double/triple buffering and handles image acquisition and presentation.

    <b>Key features:</b>
    - Multiple presentation images for buffering
    - Automatic image rotation and synchronization
    - Format and resolution configuration
    - VSync and presentation modes
    .
    <br/>

    <b>Lifetime:</b> Swapchains are created by Device from a Surface and should remain valid while
    presenting. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a swapchain:</b>

    \snippet kdgpu_doc_snippets.cpp swapchain_creation

    <b>Accessing swapchain images:</b>

    \snippet kdgpu_doc_snippets.cpp swapchain_images

    <b>Render loop with swapchain:</b>

    \snippet kdgpu_doc_snippets.cpp swapchain_render_loop

    <b>Handling window resize:</b>

    \snippet kdgpu_doc_snippets.cpp swapchain_resize

    <b>Choosing presentation mode:</b>

    \snippet kdgpu_doc_snippets.cpp swapchain_present_mode

    <b>Getting swapchain optimal format:</b>

    \snippet kdgpu_doc_snippets.cpp swapchain_format

    ## Vulkan mapping:
    - Swapchain creation -> vkCreateSwapchainKHR()
    - Swapchain::getNextImageIndex() -> vkAcquireNextImageKHR()
    - Swapchain::textures() -> vkGetSwapchainImagesKHR()
    - Queue::present() -> vkQueuePresentKHR()

    ## See also:
    \sa Surface, Texture, Queue, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Swapchain
{
public:
    Swapchain();
    ~Swapchain();

    Swapchain(Swapchain &&) noexcept;
    Swapchain &operator=(Swapchain &&) noexcept;

    Swapchain(const Swapchain &) = delete;
    Swapchain &operator=(const Swapchain &) = delete;

    const Handle<Swapchain_t> &handle() const noexcept { return m_swapchain; }
    bool isValid() const noexcept { return m_swapchain.isValid(); }

    operator Handle<Swapchain_t>() const noexcept { return m_swapchain; }

    const std::vector<Texture> &textures() const { return m_textures; }

    AcquireImageResult getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore = Handle<GpuSemaphore_t>());

private:
    explicit Swapchain(GraphicsApi *api, const Handle<Device_t> &device, const SwapchainOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Swapchain_t> m_swapchain;
    std::vector<Texture> m_textures;

    friend class Device;
};

} // namespace KDGpu
