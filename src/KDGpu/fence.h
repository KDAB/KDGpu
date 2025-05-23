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

/**
 * @brief Fence
 * @ingroup public
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
