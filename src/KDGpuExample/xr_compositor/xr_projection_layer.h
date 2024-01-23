/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/xr_compositor/xr_compositor_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

namespace KDGpu {
class Device;
class Queue;
} // namespace KDGpu

namespace KDXr {
class Session;
}

namespace KDGpuExample {

struct XrProjectionLayerOptions {
    KDGpu::Device *device{ nullptr };
    KDGpu::Queue *queue{ nullptr };
    KDXr::Session *session{ nullptr };
};

class KDGPUEXAMPLE_EXPORT XrProjectionLayer : public XrCompositorLayer
{
public:
    explicit XrProjectionLayer(const XrProjectionLayerOptions &options);
    ~XrProjectionLayer() override;

protected:
    void update() override;

    KDGpu::Device *m_device{ nullptr };
    KDGpu::Queue *m_queue{ nullptr };
    KDXr::Session *m_session{ nullptr };
};

} // namespace KDGpuExample
