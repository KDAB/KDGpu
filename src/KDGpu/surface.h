/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

struct Surface_t;

class GraphicsApi;

class KDGPU_EXPORT Surface
{
public:
    Surface();
    ~Surface();

    Surface(Surface &&);
    Surface &operator=(Surface &&);

    Surface(const Surface &) = delete;
    Surface &operator=(const Surface &) = delete;

    Handle<Surface_t> handle() const noexcept { return m_surface; }
    bool isValid() const noexcept { return m_surface.isValid(); }

    operator Handle<Surface_t>() const noexcept { return m_surface; }

private:
    Surface(GraphicsApi *api, const Handle<Surface_t> &surface);

    GraphicsApi *m_api{ nullptr };
    Handle<Surface_t> m_surface;

    friend class Instance;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
