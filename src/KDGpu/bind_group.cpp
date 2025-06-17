/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "bind_group.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

BindGroup::BindGroup()
{
}

BindGroup::~BindGroup()
{
    if (isValid())
        m_api->resourceManager()->deleteBindGroup(handle());
}

BindGroup::BindGroup(BindGroup &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_bindGroup = std::exchange(other.m_bindGroup, {});
}

BindGroup &BindGroup::operator=(BindGroup &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteBindGroup(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_bindGroup = std::exchange(other.m_bindGroup, {});
    }
    return *this;
}

bool BindGroup::isValid() const
{
    if (!m_bindGroup.isValid())
        return false;

    auto *apiBindGroup = m_api->resourceManager()->getBindGroup(m_bindGroup);
    if (apiBindGroup == nullptr || !apiBindGroup->hasValidHandle())
        return false;

    return true;
}

BindGroup::BindGroup(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupOptions &options)
    : m_api(api)
    , m_device(device)
    , m_bindGroup(m_api->resourceManager()->createBindGroup(m_device, options))
{
}

void BindGroup::update(const BindGroupEntry &entry)
{
    auto *apiBindGroup = m_api->resourceManager()->getBindGroup(m_bindGroup);
    apiBindGroup->update(entry);
}

bool operator==(const BindGroup &a, const BindGroup &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_bindGroup == b.m_bindGroup;
}

bool operator!=(const BindGroup &a, const BindGroup &b)
{
    return !(a == b);
}

} // namespace KDGpu
