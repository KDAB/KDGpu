/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "bind_group_pool.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

BindGroupPool::BindGroupPool()
{
}

BindGroupPool::~BindGroupPool()
{
    if (isValid())
        m_api->resourceManager()->deleteBindGroupPool(handle());
}

BindGroupPool::BindGroupPool(BindGroupPool &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_bindGroupPool = std::exchange(other.m_bindGroupPool, {});
}

BindGroupPool &BindGroupPool::operator=(BindGroupPool &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteBindGroupPool(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_bindGroupPool = std::exchange(other.m_bindGroupPool, {});
    }
    return *this;
}

BindGroupPool::BindGroupPool(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupPoolOptions &options)
    : m_api(api)
    , m_device(device)
    , m_bindGroupPool(m_api->resourceManager()->createBindGroupPool(m_device, options))
{
}

void BindGroupPool::reset()
{
    auto apiBindGroupPool = m_api->resourceManager()->getBindGroupPool(m_bindGroupPool);
    apiBindGroupPool->reset();
}

bool operator==(const BindGroupPool &a, const BindGroupPool &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_bindGroupPool == b.m_bindGroupPool;
}

bool operator!=(const BindGroupPool &a, const BindGroupPool &b)
{
    return !(a == b);
}

} // namespace KDGpu
