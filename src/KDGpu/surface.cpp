/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "surface.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

Surface::Surface()
{
}

Surface::Surface(GraphicsApi *api, const Handle<Surface_t> &surface)
    : m_api(api)
    , m_surface(surface)
{
}

Surface::Surface(Surface &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_surface = std::exchange(other.m_surface, {});
}

Surface &Surface::operator=(Surface &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSurface(m_surface);

        m_api = std::exchange(other.m_api, nullptr);
        m_surface = std::exchange(other.m_surface, {});
    }
    return *this;
}

Surface::~Surface()
{
    if (isValid())
        m_api->resourceManager()->deleteSurface(m_surface);
}

} // namespace KDGpu
