/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_cylinder_layer.h"

#include <KDXr/session.h>

namespace KDGpuExample {

XrCylinderLayer::XrCylinderLayer(const XrCylinderLayerOptions &options)
    : XrCompositorLayer(Type::Cylinder)
    , m_device(options.device)
    , m_queue(options.queue)
    , m_session(options.session)
    , m_colorSwapchainFormat(options.colorSwapchainFormat)
    , m_depthSwapchainFormat(options.depthSwapchainFormat)
    , m_samples(options.samples)
{
}

XrCylinderLayer::~XrCylinderLayer()
{
}

void XrCylinderLayer::initialize()
{
    recreateSwapchains();
}

void XrCylinderLayer::cleanup()
{
    m_colorSwapchain.textureViews.clear();
    m_colorSwapchain.swapchain = {};
    m_depthSwapchain.textureViews.clear();
    m_depthSwapchain.swapchain = {};
}

bool XrCylinderLayer::update(const KDXr::FrameState &)
{
    // Render the quad layer
    // Acquire and wait for the next swapchain textures to become available for the color and depth swapchains
    m_colorSwapchain.swapchain.getNextTextureIndex(m_currentColorImageIndex);
    m_depthSwapchain.swapchain.getNextTextureIndex(m_currentDepthImageIndex);

    m_colorSwapchain.swapchain.waitForTexture();
    m_depthSwapchain.swapchain.waitForTexture();

    // Call subclass renderCylinder() function to record and submit drawing commands for the cylinder
    renderCylinder();

    // Give the swapchain textures back to the XR runtime, allowing the compositor to use the image.
    m_colorSwapchain.swapchain.releaseTexture();
    m_depthSwapchain.swapchain.releaseTexture();

    // Set up the quad layer
    // clang-format off
    m_cylinderLayer = {
        .type = KDXr::CompositionLayerType::Cylinder,
        .referenceSpace = m_referenceSpace,
        .flags = KDXr::CompositionLayerFlagBits::BlendTextureSourceAlphaBit | KDXr::CompositionLayerFlagBits::CorrectChromaticAberrationBit,
        .eyeVisibility = KDXr::EyeVisibility::Both,
        .swapchainSubTexture = {
            .swapchain = m_colorSwapchain.swapchain,
            .rect = {
                .offset = { .x = 0, .y = 0 },
                .extent = resolution()
            }
        },
        .pose = { .orientation = orientation(), .position = position() },
        .radius = radius(),
        .centralAngle = centralAngle(),
        .aspectRatio = aspectRatio()
    };
    // clang-format on

    return true;
}

void XrCylinderLayer::recreateSwapchains()
{
    // Create quad color and depth swapchains
    m_colorSwapchain.swapchain = m_session->createSwapchain({ .format = m_colorSwapchainFormat,
                                                              .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::ColorAttachmentBit,
                                                              .width = resolution().width,
                                                              .height = resolution().height,
                                                              .sampleCount = 1 });
    const auto &textures = m_colorSwapchain.swapchain.textures();
    const auto textureCount = textures.size();
    m_colorSwapchain.textureViews.reserve(textureCount);
    for (size_t j = 0; j < textureCount; ++j)
        m_colorSwapchain.textureViews.emplace_back(textures[j].createView());

    m_depthSwapchain.swapchain = m_session->createSwapchain({ .format = m_depthSwapchainFormat,
                                                              .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::DepthStencilAttachmentBit,
                                                              .width = resolution().width,
                                                              .height = resolution().height,
                                                              .sampleCount = 1 });
    const auto &depthTextures = m_depthSwapchain.swapchain.textures();
    const auto depthTextureCount = depthTextures.size();
    m_depthSwapchain.textureViews.reserve(depthTextureCount);
    for (size_t j = 0; j < depthTextureCount; ++j)
        m_depthSwapchain.textureViews.emplace_back(depthTextures[j].createView());
}

} // namespace KDGpuExample
