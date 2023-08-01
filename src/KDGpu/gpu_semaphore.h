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

namespace KDGpu {

struct Device_t;
struct GpuSemaphore_t;

class GraphicsApi;

struct GpuSemaphoreOptions {
    ExternalSemaphoreHandleTypeFlags externalSemaphoreHandleType{ ExternalSemaphoreHandleTypeFlagBits::None };
};

/**
 * @brief GpuSemaphore
 * @ingroup public
 */
class KDGPU_EXPORT GpuSemaphore
{
public:
    GpuSemaphore();
    ~GpuSemaphore();

    GpuSemaphore(GpuSemaphore &&);
    GpuSemaphore &operator=(GpuSemaphore &&);

    GpuSemaphore(const GpuSemaphore &) = delete;
    GpuSemaphore &operator=(const GpuSemaphore &) = delete;

    const Handle<GpuSemaphore_t> &handle() const noexcept { return m_gpuSemaphore; }
    bool isValid() const noexcept { return m_gpuSemaphore.isValid(); }

    operator Handle<GpuSemaphore_t>() const noexcept { return m_gpuSemaphore; }

private:
    explicit GpuSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const GpuSemaphoreOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<GpuSemaphore_t> m_gpuSemaphore;

    friend class Device;
};

} // namespace KDGpu
