/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "sampler.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

Sampler::Sampler() = default;
Sampler::~Sampler()
{
    if (isValid())
        m_api->resourceManager()->deleteSampler(handle());
};

Sampler::Sampler(GraphicsApi *api, const Handle<Device_t> &device, const SamplerOptions &options)
    : m_api(api)
    , m_device(device)
    , m_sampler(m_api->resourceManager()->createSampler(m_device, options))
{
}

Sampler::Sampler(Sampler &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_sampler = std::exchange(other.m_sampler, {});
}

Sampler &Sampler::operator=(Sampler &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSampler(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_sampler = std::exchange(other.m_sampler, {});
    }
    return *this;
}

bool operator==(const Sampler &a, const Sampler &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_sampler == b.m_sampler;
}

bool operator!=(const Sampler &a, const Sampler &b)
{
    return !(a == b);
}

} // namespace KDGpu
