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

struct XrQuadLayerOptions {
    KDGpu::Device *device{ nullptr };
    KDGpu::Queue *queue{ nullptr };
    KDXr::Session *session{ nullptr };
    KDGpu::Format colorSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::Format depthSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::SampleCountFlagBits samples{ KDGpu::SampleCountFlagBits::Samples1Bit };
};

class KDGPUEXAMPLE_EXPORT XrQuadLayer : public XrCompositorLayer
{
public:
    // TODO: Use properties or simple member variables? Probably members are enough for this level of abstraction.
    KDBindings::Property<KDXr::Vector3> position{};
    KDBindings::Property<KDXr::Quaternion> orientation{};
    KDBindings::Property<KDGpu::Extent2D> resolution{ KDGpu::Extent2D{ 1280, 720 } };
    KDBindings::Property<KDGpu::Extent2Df> worldSize{ KDGpu::Extent2Df{ 2.0f, 2.0f * resolution().height / resolution().width } };
    KDBindings::Property<KDXr::EyeVisibility> eyeVisibility{ KDXr::EyeVisibility::Both };

    explicit XrQuadLayer(const XrQuadLayerOptions &options);
    ~XrQuadLayer() override;

    // Not copyable
    XrQuadLayer(const XrQuadLayer &) = delete;
    XrQuadLayer &operator=(const XrQuadLayer &) = delete;

    // Moveable
    XrQuadLayer(XrQuadLayer &&) = default;
    XrQuadLayer &operator=(XrQuadLayer &&) = default;

    struct Intersection {
        KDXr::Vector3 worldSpace = { 0.0f, 0.0f, 0.0f };
        float x = 0.0f;
        float y = 0.0f;
        bool withinBounds = false;
    };

    // Cast a ray down negative Z from the given pose to find an intersection point
    // with this quad, in world and local image coordinate space, or nullopt if no
    // intersection occurs
    std::optional<Intersection> rayIntersection(KDXr::Pose rayCasterPose);

protected:
    void initialize() override;
    void cleanup() override;
    bool update(const KDXr::FrameState &frameState) override;
    void recreateSwapchains();
    virtual void renderQuad() = 0;
    KDXr::CompositionLayer *compositionLayer() override { return reinterpret_cast<KDXr::CompositionLayer *>(&m_quadLayer); }

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

    KDXr::QuadLayer m_quadLayer;
};

} // namespace KDGpuExample
