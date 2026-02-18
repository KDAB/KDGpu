/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct GpuSemaphore_t;

struct GpuSemaphoreOptions {
    std::string_view label;
    ExternalSemaphoreHandleTypeFlags externalSemaphoreHandleType{ ExternalSemaphoreHandleTypeFlagBits::None };
};

/*!
    \class GpuSemaphore
    \brief GPU-to-GPU synchronization primitive for command buffer dependencies
    \ingroup public
    \headerfile gpu_semaphore.h <KDGpu/gpu_semaphore.h>

    <b>Vulkan equivalent:</b> [VkSemaphore](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSemaphore.html)

    GpuSemaphore synchronizes GPU operations without CPU involvement. Unlike Fence (which synchronizes
    CPU and GPU), semaphores coordinate work between queue submissions and swapchain operations.

    <b>Key features:</b>
    - GPU-only synchronization (no CPU waiting)
    - Signal/wait between queue submissions
    - Swapchain image acquisition and presentation
    - Cross-queue synchronization
    .
    <br/>

    <b>Lifetime:</b> Semaphores are created by Device and must remain valid while GPU operations
    reference them. They use RAII and clean up automatically.

    ## Usage

    <b>Basic semaphore usage:</b>

    \snippet kdgpu_doc_snippets.cpp gpusemaphore_creation

    <b>Queue synchronization:</b>

    \snippet kdgpu_doc_snippets.cpp gpusemaphore_queue_sync

    <b>Chaining multiple operations:</b>

    \snippet kdgpu_doc_snippets.cpp gpusemaphore_swapchain

    <b>Multi-queue pipeline:</b>

    \snippet kdgpu_doc_snippets.cpp gpusemaphore_multi_queue

    ## Vulkan mapping:
    - GpuSemaphore creation -> vkCreateSemaphore()
    - Used in vkQueueSubmit() wait/signal arrays
    - Used in vkAcquireNextImageKHR()
    - Used in vkQueuePresentKHR()

    ## See also:
    \sa GpuSemaphoreOptions, Fence, Queue, Swapchain, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT GpuSemaphore
{
public:
    GpuSemaphore();
    ~GpuSemaphore();

    GpuSemaphore(GpuSemaphore &&) noexcept;
    GpuSemaphore &operator=(GpuSemaphore &&) noexcept;

    GpuSemaphore(const GpuSemaphore &) = delete;
    GpuSemaphore &operator=(const GpuSemaphore &) = delete;

    const Handle<GpuSemaphore_t> &handle() const noexcept { return m_gpuSemaphore; }
    bool isValid() const noexcept { return m_gpuSemaphore.isValid(); }

    operator Handle<GpuSemaphore_t>() const noexcept { return m_gpuSemaphore; }

    HandleOrFD externalSemaphoreHandle() const;

private:
    explicit GpuSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const GpuSemaphoreOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<GpuSemaphore_t> m_gpuSemaphore;

    friend class Device;
};

} // namespace KDGpu
