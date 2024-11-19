/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct RenderPass_t;
struct RenderPassOptions;

class KDGPU_EXPORT RenderPass
{
public:
    RenderPass();
    ~RenderPass();

    RenderPass(RenderPass &&);
    RenderPass &operator=(RenderPass &&);

    RenderPass(const RenderPass &) = delete;
    RenderPass &operator=(const RenderPass &) = delete;

    Handle<RenderPass_t> handle() const noexcept { return m_renderPass; }
    bool isValid() const noexcept { return m_renderPass.isValid(); }

    operator Handle<RenderPass_t>() const noexcept { return m_renderPass; }

    /* could add dynamic support for changing for attachment load/store op */

private:
    explicit RenderPass(GraphicsApi *api, const Handle<Device_t> &device, const RenderPassOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<RenderPass_t> m_renderPass;

    friend class Device;
};

} // namespace KDGpu
