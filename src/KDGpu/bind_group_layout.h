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

struct BindGroupLayout_t;
struct Device_t;
struct BindGroupLayoutOptions;
class GraphicsApi;

/**
 * @brief BindGroupLayout
 * @ingroup public
 */
class KDGPU_EXPORT BindGroupLayout
{
public:
    BindGroupLayout();
    ~BindGroupLayout();

    BindGroupLayout(BindGroupLayout &&);
    BindGroupLayout &operator=(BindGroupLayout &&);

    BindGroupLayout(const BindGroupLayout &) = delete;
    BindGroupLayout &operator=(const BindGroupLayout &) = delete;

    const Handle<BindGroupLayout_t> &handle() const noexcept { return m_bindGroupLayout; }
    bool isValid() const noexcept { return m_bindGroupLayout.isValid(); }

    operator Handle<BindGroupLayout_t>() const noexcept { return m_bindGroupLayout; }

private:
    explicit BindGroupLayout(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupLayoutOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroupLayout_t> m_bindGroupLayout;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const BindGroupLayout &, const BindGroupLayout &);
};

KDGPU_EXPORT bool operator==(const BindGroupLayout &a, const BindGroupLayout &b);
KDGPU_EXPORT bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b);

} // namespace KDGpu
