/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "bind_group_layout.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

BindGroupLayout::BindGroupLayout() = default;
BindGroupLayout::~BindGroupLayout()
{
    if (isValid())
        m_api->resourceManager()->deleteBindGroupLayout(handle());
};

BindGroupLayout::BindGroupLayout(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const BindGroupLayoutOptions &options)
    : m_api(api)
    , m_device(device)
    , m_bindGroupLayout(m_api->resourceManager()->createBindGroupLayout(m_device, options))
{
}

BindGroupLayout::BindGroupLayout(BindGroupLayout &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_bindGroupLayout = std::exchange(other.m_bindGroupLayout, {});
}

BindGroupLayout &BindGroupLayout::operator=(BindGroupLayout &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteBindGroupLayout(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_bindGroupLayout = std::exchange(other.m_bindGroupLayout, {});
    }
    return *this;
}

bool BindGroupLayout::isCompatibleWith(const Handle<BindGroupLayout_t> &other) const
{
    if (!isValid() || !other.isValid())
        return false;

    auto *apiLayout = m_api->resourceManager()->getBindGroupLayout(handle());
    auto *otherApiLayout = m_api->resourceManager()->getBindGroupLayout(other);
    if (apiLayout == nullptr || otherApiLayout == nullptr)
        return false;
    return apiLayout->isCompatibleWith(*otherApiLayout);
}

bool operator==(const BindGroupLayout &a, const BindGroupLayout &b)
{
    return (a.m_api == b.m_api && a.m_device == b.m_device &&
            (a.m_bindGroupLayout == b.m_bindGroupLayout || a.isCompatibleWith(b)));
}

bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b)
{
    return !(a == b);
}

} // namespace KDGpu
