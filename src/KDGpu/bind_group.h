/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group_description.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct BindGroupEntry;
struct BindGroup_t;
struct Device_t;
struct BindGroupOptions;

// A BindGroup is what is known as a descriptor set in Vulkan parlance. Other APIs such
// as web-gpu call them bind groups which to me helps with the mental model a little more.
//

/**
 * @brief BindGroup
 * @ingroup public
 */
class KDGPU_EXPORT BindGroup
{
public:
    BindGroup();
    ~BindGroup();

    BindGroup(BindGroup &&) noexcept;
    BindGroup &operator=(BindGroup &&) noexcept;

    BindGroup(const BindGroup &) = delete;
    BindGroup &operator=(const BindGroup &) = delete;

    const Handle<BindGroup_t> &handle() const noexcept { return m_bindGroup; }
    bool isValid() const;

    operator Handle<BindGroup_t>() const noexcept { return m_bindGroup; }

    void update(const BindGroupEntry &entry);

private:
    explicit BindGroup(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroup_t> m_bindGroup;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const BindGroup &, const BindGroup &);
};

KDGPU_EXPORT bool operator==(const BindGroup &a, const BindGroup &b);
KDGPU_EXPORT bool operator!=(const BindGroup &a, const BindGroup &b);

} // namespace KDGpu
