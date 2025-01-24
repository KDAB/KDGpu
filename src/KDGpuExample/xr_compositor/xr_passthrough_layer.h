/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/xr_compositor/xr_compositor_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

#include <KDXr/compositor.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/passthrough_layer_controller.h>

#include <KDGpu/gpu_core.h>

#include <kdbindings/property.h>

namespace KDGpu {
class Device;
}

namespace KDXr {
class Session;
struct PassthroughLayer_t;
class PassthroughLayerController;
} // namespace KDXr

namespace KDGpuExample {

struct XrPassthroughLayerOptions {
    KDGpu::Device *device{ nullptr };
    KDGpu::Queue *queue{ nullptr };
    KDXr::Session *session{ nullptr };
};

class KDGPUEXAMPLE_EXPORT XrPassthroughLayer : public XrCompositorLayer
{
public:
    explicit XrPassthroughLayer(const XrPassthroughLayerOptions &options);
    ~XrPassthroughLayer() override;

    // Not copyable
    XrPassthroughLayer(const XrPassthroughLayer &) = delete;
    XrPassthroughLayer &operator=(const XrPassthroughLayer &) = delete;

    // Moveable
    XrPassthroughLayer(XrPassthroughLayer &&) = default;
    XrPassthroughLayer &operator=(XrPassthroughLayer &&) = default;

    void setRunning(bool running);

protected:
    void initialize() override;
    void cleanup() override;
    bool update(const KDXr::FrameState &frameState) override;
    KDXr::CompositionLayer *compositionLayer() override { return reinterpret_cast<KDXr::CompositionLayer *>(&m_passthroughCompositionLayer); }

    KDGpu::Device *m_device{ nullptr };
    KDGpu::Queue *m_queue{ nullptr };
    KDXr::Session *m_session{ nullptr };
    KDXr::PassthroughLayerController m_layerController;

    KDXr::PassthroughCompositionLayer m_passthroughCompositionLayer;
};

} // namespace KDGpuExample
