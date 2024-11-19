/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "gpu_semaphore.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

GpuSemaphore::GpuSemaphore()
{
}

GpuSemaphore::GpuSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const GpuSemaphoreOptions &options)
    : m_api(api)
    , m_device(device)
    , m_gpuSemaphore(m_api->resourceManager()->createGpuSemaphore(m_device, options))
{
}

GpuSemaphore::~GpuSemaphore()
{
    if (isValid())
        m_api->resourceManager()->deleteGpuSemaphore(handle());
}

GpuSemaphore::GpuSemaphore(GpuSemaphore &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_gpuSemaphore = std::exchange(other.m_gpuSemaphore, {});
}

GpuSemaphore &GpuSemaphore::operator=(GpuSemaphore &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteGpuSemaphore(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_gpuSemaphore = std::exchange(other.m_gpuSemaphore, {});
    }
    return *this;
}

HandleOrFD GpuSemaphore::externalSemaphoreHandle() const
{
    auto apiSemaphore = m_api->resourceManager()->getGpuSemaphore(m_gpuSemaphore);
    return apiSemaphore->externalSemaphoreHandle();
}

} // namespace KDGpu
