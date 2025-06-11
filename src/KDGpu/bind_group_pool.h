/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>
#include <KDGpu/handle.h>

namespace KDGpu {

struct BindGroupPool_t;
struct BindGroupPoolOptions;
struct Device_t;

class KDGPU_EXPORT BindGroupPool
{
public:
    BindGroupPool();
    ~BindGroupPool();

    BindGroupPool(BindGroupPool &&) noexcept;
    BindGroupPool &operator=(BindGroupPool &&) noexcept;

    BindGroupPool(const BindGroupPool &) = delete;
    BindGroupPool &operator=(const BindGroupPool &) = delete;

    const Handle<BindGroupPool_t> &handle() const noexcept { return m_bindGroupPool; }
    bool isValid() const noexcept { return m_bindGroupPool.isValid(); }

    operator Handle<BindGroupPool_t>() const noexcept { return m_bindGroupPool; }

    void reset();

private:
    explicit BindGroupPool(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupPoolOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroupPool_t> m_bindGroupPool;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const BindGroupPool &, const BindGroupPool &);
};

KDGPU_EXPORT bool operator==(const BindGroupPool &a, const BindGroupPool &b);
KDGPU_EXPORT bool operator!=(const BindGroupPool &a, const BindGroupPool &b);

} // namespace KDGpu
