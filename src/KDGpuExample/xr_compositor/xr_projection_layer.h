/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/xr_compositor/xr_compositor_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

#include <KDXr/kdxr_core.h>
#include <KDXr/swapchain.h>

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
    KDGpu::Format colorSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::Format depthSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::SampleCountFlagBits samples{ KDGpu::SampleCountFlagBits::Samples1Bit };
    bool requestMultiview{ true };
};

class KDGPUEXAMPLE_EXPORT XrProjectionLayer : public XrCompositorLayer
{
public:
    explicit XrProjectionLayer(const XrProjectionLayerOptions &options);
    ~XrProjectionLayer() override;

    // Not copyable
    XrProjectionLayer(const XrProjectionLayer &) = delete;
    XrProjectionLayer &operator=(const XrProjectionLayer &) = delete;

    // Moveable
    XrProjectionLayer(XrProjectionLayer &&) = default;
    XrProjectionLayer &operator=(XrProjectionLayer &&) = default;

protected:
    void initialize() override;
    void cleanup() override;
    bool update(const KDXr::FrameState &frameState) override;
    KDXr::CompositionLayer *compositionLayer() override { return reinterpret_cast<KDXr::CompositionLayer *>(&m_projectionLayer); }
    virtual void updateScene();
    virtual void renderView() = 0;
    void recreateSwapchains();
    uint32_t viewCount() const noexcept { return m_viewCount; }

    KDGpu::Device *m_device{ nullptr };
    KDGpu::Queue *m_queue{ nullptr };
    KDXr::Session *m_session{ nullptr };

    KDGpu::Format m_colorSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::Format m_depthSwapchainFormat{ KDGpu::Format::UNDEFINED };
    KDGpu::SampleCountFlagBits m_samples{ KDGpu::SampleCountFlagBits::Samples1Bit };
    bool m_enableMultiview{ true };

    float m_nearPlane{ 0.05f };
    float m_farPlane{ 100.0f };

    std::vector<KDXr::SwapchainInfo> m_colorSwapchains;
    std::vector<KDXr::SwapchainInfo> m_depthSwapchains;

    uint32_t m_viewCount{ 2 };
    uint32_t m_currentViewIndex{ 0 };
    uint32_t m_currentColorImageIndex{ 0 };
    uint32_t m_currentDepthImageIndex{ 0 };

    KDXr::ViewState m_viewState;

    KDXr::ProjectionLayer m_projectionLayer;
    std::vector<KDXr::ProjectionLayerView> m_projectionLayerViews{ 2 }; // Projection layer views. One per view for each projection layer
    std::vector<KDXr::DepthInfo> m_depthInfos{ 2 }; // Depth info. One per view for each projection layer
};

} // namespace KDGpuExample
