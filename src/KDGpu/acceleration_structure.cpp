/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "acceleration_structure.h"

#include <KDGpu/graphics_api.h>
#include <KDGpu/api/api_buffer.h>
#include <KDGpu/buffer_options.h>

namespace KDGpu {

AccelerationStructure::AccelerationStructure() = default;

AccelerationStructure::AccelerationStructure(GraphicsApi *api, const Handle<Device_t> &device, const AccelerationStructureOptions &options)
    : m_api(api)
    , m_device(device)
    , m_accelerationStructure(api->resourceManager()->createAccelerationStructure(device, options))
{
}

AccelerationStructure::AccelerationStructure(AccelerationStructure &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_accelerationStructure = std::exchange(other.m_accelerationStructure, {});
}

AccelerationStructure &AccelerationStructure::operator=(AccelerationStructure &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteAccelerationStructure(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_accelerationStructure = std::exchange(other.m_accelerationStructure, {});

        other.m_api = nullptr;
        other.m_device = {};
        other.m_accelerationStructure = {};
    }
    return *this;
}

AccelerationStructure::~AccelerationStructure()
{
    if (isValid())
        m_api->resourceManager()->deleteAccelerationStructure(handle());
}

bool operator==(const AccelerationStructure &a, const AccelerationStructure &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_accelerationStructure == b.m_accelerationStructure;
}

bool operator!=(const AccelerationStructure &a, const AccelerationStructure &b)
{
    return !(a == b);
}

} // namespace KDGpu
