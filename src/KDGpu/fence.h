/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct Fence_t;

struct FenceOptions {
    std::string_view label;
    bool createSignalled{ true };
    ExternalFenceHandleTypeFlags externalFenceHandleType{ ExternalFenceHandleTypeFlagBits::None };
};

/*!
    \class Fence
    \brief Synchronization primitive for CPU-GPU synchronization
    \ingroup public
    \headerfile fence.h <KDGpu/fence.h>

    <b>Vulkan equivalent:</b> [VkFence](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFence.html)

    Fence is used to synchronize CPU and GPU execution. The GPU signals a fence when it completes work,
    and the CPU can wait for that signal. This is essential for knowing when GPU operations have finished.

    <b>Key features:</b>
    - CPU-side waiting for GPU completion
    - Signaling when GPU work is done
    - Resetting for reuse
    - Checking status without blocking
    .
    <br/>

    <b>Lifetime:</b> Fences are created by Device and should remain valid while GPU work that signals
    them is in flight. They use RAII and clean up automatically.

    ## Usage

    <b>Basic fence usage:</b>

    \snippet kdgpu_doc_snippets.cpp fence_creation

    <b>Submit and wait:</b>

    \snippet kdgpu_doc_snippets.cpp fence_submit_wait

    <b>Checking status:</b>

    \snippet kdgpu_doc_snippets.cpp fence_status

    <b>CPU-GPU synchronization:</b>

    \snippet kdgpu_doc_snippets.cpp fence_cpu_gpu_sync

    <b>Resetting fences:</b>

    \snippet kdgpu_doc_snippets.cpp fence_reset

    ## Vulkan mapping:
    - Fence creation -> vkCreateFence()
    - Fence::wait() -> vkWaitForFences()
    - Fence::reset() -> vkResetFences()
    - Fence::status() -> vkGetFenceStatus()

    ## See also:
    \sa FenceOptions, Queue, GpuSemaphore, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Fence
{
public:
    Fence();
    ~Fence();

    Fence(Fence &&) noexcept;
    Fence &operator=(Fence &&) noexcept;

    Fence(const Fence &) = delete;
    Fence &operator=(const Fence &) = delete;

    const Handle<Fence_t> &handle() const noexcept { return m_fence; }
    bool isValid() const noexcept { return m_fence.isValid(); }

    operator Handle<Fence_t>() const noexcept { return m_fence; }

    void reset();
    void wait();
    FenceStatus status() const;

    HandleOrFD externalFenceHandle() const;

private:
    explicit Fence(GraphicsApi *api, const Handle<Device_t> &device, const FenceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Fence_t> m_fence;

    friend KDGPU_EXPORT bool operator==(const Fence &, const Fence &);
    friend class Device;
    friend class Queue;
};

KDGPU_EXPORT bool operator==(const Fence &a, const Fence &b);
KDGPU_EXPORT bool operator!=(const Fence &a, const Fence &b);

} // namespace KDGpu
