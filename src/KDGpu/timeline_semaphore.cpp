/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "timeline_semaphore.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

TimelineSemaphore::TimelineSemaphore()
{
}

TimelineSemaphore::TimelineSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const TimelineSemaphoreOptions &options)
    : m_api(api)
    , m_device(device)
    , m_timelineSemaphore(m_api->resourceManager()->createTimelineSemaphore(m_device, options))
{
}

TimelineSemaphore::~TimelineSemaphore()
{
    if (isValid())
        m_api->resourceManager()->deleteTimelineSemaphore(handle());
}

TimelineSemaphore::TimelineSemaphore(TimelineSemaphore &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_timelineSemaphore = std::exchange(other.m_timelineSemaphore, {});
}

TimelineSemaphore &TimelineSemaphore::operator=(TimelineSemaphore &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteTimelineSemaphore(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_timelineSemaphore = std::exchange(other.m_timelineSemaphore, {});
    }
    return *this;
}

uint64_t TimelineSemaphore::value() const
{
    auto *backend = m_api->resourceManager()->getTimelineSemaphore(m_timelineSemaphore);
    return backend->value();
}

void TimelineSemaphore::signal(uint64_t value)
{
    auto *backend = m_api->resourceManager()->getTimelineSemaphore(m_timelineSemaphore);
    backend->signal(value);
}

TimelineSemaphoreWaitResult TimelineSemaphore::wait(uint64_t value) const
{
    auto *backend = m_api->resourceManager()->getTimelineSemaphore(m_timelineSemaphore);
    return backend->wait(value);
}

} // namespace KDGpu
