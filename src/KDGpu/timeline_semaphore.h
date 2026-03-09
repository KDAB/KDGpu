/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/graphics_api.h>

#include <string_view>

namespace KDGpu {

struct Device_t;
struct TimelineSemaphore_t;

struct TimelineSemaphoreOptions {
    std::string_view label;
    ExternalSemaphoreHandleTypeFlags externalSemaphoreHandleType{ ExternalSemaphoreHandleTypeFlagBits::None };
    uint64_t initialValue{ 0 };
};

/*!
    \class TimelineSemaphore
    \brief GPU timeline semaphore with CPU-side signal and wait operations
    \ingroup public
    \headerfile timeline_semaphore.h <KDGpu/timeline_semaphore.h>

    <b>Vulkan equivalent:</b> [VkSemaphore (timeline type)](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSemaphoreTypeCreateInfo.html)

    TimelineSemaphore is a monotonically-increasing uint64_t counter that can be signaled and waited
    on from both the CPU and GPU. Unlike GpuSemaphore (which is binary), a timeline semaphore carries
    a value that is tested against a user-supplied threshold.

    <b>Key differences from GpuSemaphore:</b>
    - Carries a uint64_t counter rather than binary signaled/unsignaled state
    - CPU can signal and wait without submitting any GPU work
    - Cannot be used with swapchain acquire/present operations
    - Multiple pending signals and waits are allowed
    .
    <br/>

    ## Usage

    <b>CPU-side wait:</b>
    \code
    auto sem = device.createTimelineSemaphore({ .initialValue = 0 });
    // GPU submission signals value 1 via SubmitOptions::signalTimelineSemaphores
    sem.wait(1); // blocks until the GPU signals value >= 1
    \endcode

    ## Vulkan mapping:
    - TimelineSemaphore creation -> vkCreateSemaphore() with VkSemaphoreTypeCreateInfo::VK_SEMAPHORE_TYPE_TIMELINE
    - TimelineSemaphore::value()  -> vkGetSemaphoreCounterValue()
    - TimelineSemaphore::signal() -> vkSignalSemaphore()
    - TimelineSemaphore::wait()   -> vkWaitSemaphores()

    ## See also:
    \sa TimelineSemaphoreOptions, GpuSemaphore, Fence, Queue, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT TimelineSemaphore
{
public:
    TimelineSemaphore();
    ~TimelineSemaphore();

    TimelineSemaphore(TimelineSemaphore &&) noexcept;
    TimelineSemaphore &operator=(TimelineSemaphore &&) noexcept;

    TimelineSemaphore(const TimelineSemaphore &) = delete;
    TimelineSemaphore &operator=(const TimelineSemaphore &) = delete;

    const Handle<TimelineSemaphore_t> &handle() const noexcept { return m_timelineSemaphore; }
    bool isValid() const noexcept { return m_timelineSemaphore.isValid(); }

    operator Handle<TimelineSemaphore_t>() const noexcept { return m_timelineSemaphore; }

    /** Returns the current counter value from the GPU/CPU timeline. */
    uint64_t value() const;

    /** Signals the semaphore to @p value from the CPU side. */
    void signal(uint64_t value);

    /**
     * Blocks the calling CPU thread until the semaphore counter reaches @p value
     * @return TimelineSemaphoreWaitResult::Success or TimelineSemaphoreWaitResult::Timeout.
     */
    TimelineSemaphoreWaitResult wait(uint64_t value) const;

private:
    explicit TimelineSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const TimelineSemaphoreOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<TimelineSemaphore_t> m_timelineSemaphore;

    friend class Device;
};

} // namespace KDGpu
