/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_xr.h"
#include "projection_layer.h"

#include <KDGpuExample/xr_compositor/xr_quad_imgui_layer.h>

void HelloXr::onAttached()
{
    XrExampleEngineLayer::onAttached();

    // Create a projection layer to render the 3D scene
    const XrProjectionLayerOptions projectionLayerOptions = {
        .device = &m_device,
        .queue = &m_queue,
        .session = &m_session,
        .colorSwapchainFormat = m_colorSwapchainFormat,
        .depthSwapchainFormat = m_depthSwapchainFormat,
        .samples = m_samples.get()
    };
    m_projectionLayer = createCompositorLayer<ProjectionLayer>(projectionLayerOptions);
    m_projectionLayer->setReferenceSpace(m_referenceSpace);

    // Create a quad layer to render the ImGui overlay
    const XrQuadLayerOptions quadLayerOptions = {
        .device = &m_device,
        .queue = &m_queue,
        .session = &m_session,
        .colorSwapchainFormat = m_colorSwapchainFormat,
        .depthSwapchainFormat = m_depthSwapchainFormat,
        .samples = m_samples.get()
    };
    m_imguiLayer = createCompositorLayer<XrQuadImGuiLayer>(quadLayerOptions);
    m_imguiLayer->setReferenceSpace(m_referenceSpace);
    m_imguiLayer->position = { 0.0f, 0.75f, -1.5f };
}

void HelloXr::onDetached()
{
    clearCompositorLayers();
    m_imguiLayer = nullptr;
    m_projectionLayer = nullptr;
    XrExampleEngineLayer::onDetached();
}
