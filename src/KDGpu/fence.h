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

namespace KDGpu {

struct Device_t;
struct Fence_t;

class GraphicsApi;

struct FenceOptions {
    bool createSignalled{ true };
};

class KDGPU_EXPORT Fence
{
public:
    Fence();
    ~Fence();

    Fence(Fence &&);
    Fence &operator=(Fence &&);

    Fence(const Fence &) = delete;
    Fence &operator=(const Fence &) = delete;

    const Handle<Fence_t> &handle() const noexcept { return m_fence; }
    bool isValid() const noexcept { return m_fence.isValid(); }

    operator Handle<Fence_t>() const noexcept { return m_fence; }

    void reset();
    void wait();
    FenceStatus status() const;

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
