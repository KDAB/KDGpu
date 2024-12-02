/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "fence.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

Fence::Fence() = default;

Fence::Fence(Fence &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_fence = std::exchange(other.m_fence, {});
}

Fence &Fence::operator=(Fence &&other) noexcept
{
    if (this != &other) {

        if (isValid())
            m_api->resourceManager()->deleteFence(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_fence = std::exchange(other.m_fence, {});
    }
    return *this;
}

Fence::Fence(GraphicsApi *api, const Handle<Device_t> &device, const FenceOptions &options)
    : m_api(api)
    , m_device(device)
    , m_fence(m_api->resourceManager()->createFence(m_device, options))
{
}

Fence::~Fence()
{
    if (isValid())
        m_api->resourceManager()->deleteFence(handle());
}

void Fence::reset()
{
    if (isValid())
        m_api->resourceManager()->getFence(handle())->reset();
}

void Fence::wait()
{
    if (isValid())
        m_api->resourceManager()->getFence(handle())->wait();
}

FenceStatus Fence::status() const
{
    return m_api->resourceManager()->getFence(handle())->status();
}

HandleOrFD Fence::externalFenceHandle() const
{
    auto apiFence = m_api->resourceManager()->getFence(m_fence);
    return apiFence->externalFenceHandle();
}

bool operator==(const Fence &a, const Fence &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_fence == b.m_fence;
}

bool operator!=(const Fence &a, const Fence &b)
{
    return !(a == b);
}

} // namespace KDGpu
