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
#include <KDXr/swapchain.h>

#include <KDGpu/gpu_core.h>

#include <kdbindings/property.h>

namespace KDGpu {
class Device;
}

namespace KDXr {
class Session;
}

namespace KDGpuExample {

struct XrCubeLayerOptions {
    KDGpu::Device *device{ nullptr };
    KDGpu::Queue *queue{ nullptr };
    KDXr::Session *session{ nullptr };
    KDGpu::Format colorSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::Format depthSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::SampleCountFlagBits samples{ KDGpu::SampleCountFlagBits::Samples1Bit };
};

// TODO: Add a convenience class for static cube layers where we can simply provide a cubemap texture.
class KDGPUEXAMPLE_EXPORT XrCubeLayer : public XrCompositorLayer
{
public:
    KDBindings::Property<KDXr::Quaternion> orientation{};
    KDBindings::Property<KDGpu::Extent2D> resolution{ KDGpu::Extent2D{ 512, 512 } };
    KDBindings::Property<KDXr::EyeVisibility> eyeVisibility{ KDXr::EyeVisibility::Both };

    explicit XrCubeLayer(const XrCubeLayerOptions &options);
    ~XrCubeLayer() override;

    // Not copyable
    XrCubeLayer(const XrCubeLayer &) = delete;
    XrCubeLayer &operator=(const XrCubeLayer &) = delete;

    // Moveable
    XrCubeLayer(XrCubeLayer &&) = default;
    XrCubeLayer &operator=(XrCubeLayer &&) = default;

protected:
    void initialize() override;
    void cleanup() override;
    bool update(const KDXr::FrameState &frameState) override;
    void recreateSwapchains();
    virtual void renderCube() = 0;
    KDXr::CompositionLayer *compositionLayer() override { return reinterpret_cast<KDXr::CompositionLayer *>(&m_cubeLayer); }

    KDGpu::Device *m_device{ nullptr };
    KDGpu::Queue *m_queue{ nullptr };
    KDXr::Session *m_session{ nullptr };

    KDGpu::Format m_colorSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::Format m_depthSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::SampleCountFlagBits m_samples{ KDGpu::SampleCountFlagBits::Samples1Bit };
    KDXr::SwapchainInfo m_colorSwapchain;
    KDXr::SwapchainInfo m_depthSwapchain;
    uint32_t m_currentColorImageIndex{ 0 };
    uint32_t m_currentDepthImageIndex{ 0 };

    KDBindings::ConnectionHandle m_reinitializeConnection;

    KDXr::CubeLayer m_cubeLayer;
};

} // namespace KDGpuExample
