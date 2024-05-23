/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_projection_layer.h"

#include <KDGpuExample/xr_example_engine_layer.h>

#include <KDXr/kdxr_core.h>
#include <KDXr/session.h>

#include <spdlog/spdlog.h>

namespace KDGpuExample {

XrProjectionLayer::XrProjectionLayer(const XrProjectionLayerOptions &options)
    : XrCompositorLayer(Type::Projection)
    , m_device(options.device)
    , m_queue(options.queue)
    , m_session(options.session)
    , m_colorSwapchainFormat(options.colorSwapchainFormat)
    , m_depthSwapchainFormat(options.depthSwapchainFormat)
    , m_samples(options.samples)
    , m_enableMultiview(options.requestMultiview)
{
}

XrProjectionLayer::~XrProjectionLayer()
{
}

void XrProjectionLayer::initialize()
{
    if (m_enableMultiview && !m_device->adapter()->features().multiView) {
        SPDLOG_LOGGER_ERROR(logger(), "Application requested multiview but the GPU does not support this feature.");
        SPDLOG_LOGGER_ERROR(logger(), "Attempting to fallback to a non-multiview configuration.");
        m_enableMultiview = false;
    }
    recreateSwapchains();
}

void XrProjectionLayer::cleanup()
{
    m_colorSwapchains.clear();
    m_depthSwapchains.clear();
}

bool XrProjectionLayer::update(const KDXr::FrameState &frameState)
{
    // Locate the views from the view configuration within the (reference) space at the display time.
    const auto locateViewsOptions = KDXr::LocateViewsOptions{
        .displayTime = frameState.predictedDisplayTime,
        .referenceSpace = m_referenceSpace
    };

    const auto result = m_session->locateViews(locateViewsOptions, m_viewState);
    if (result != KDXr::LocateViewsResult::Success) {
        SPDLOG_LOGGER_CRITICAL(logger(), "Failed to locate views.");
        return false;
    }

    // Call updateScene() function to update scene state.
    updateScene();

    // Render the projection layer
    m_projectionLayerViews.resize(viewCount());
    const auto &viewConfigurationViews = engineLayer()->viewConfigurationViews();

    // Set up the per-view data for the compositor
    for (uint32_t i = 0; i < viewCount(); ++i) {
        // If using multiview, we only have a single color swapchain
        KDXr::SwapchainInfo &colorSwapchainInfo = m_colorSwapchains[m_enableMultiview ? 0 : i];

        // clang-format off
        m_projectionLayerViews[i] = {
            .pose = m_viewState.views[i].pose,
            .fieldOfView = m_viewState.views[i].fieldOfView,
            .swapchainSubTexture = {
                .swapchain = colorSwapchainInfo.swapchain,
                .rect = {
                    .offset = { .x = 0, .y = 0 },
                    .extent = {
                        .width = viewConfigurationViews[i].recommendedTextureWidth,
                        .height = viewConfigurationViews[i].recommendedTextureHeight
                    }
                },
                .arrayIndex = m_enableMultiview ? i : 0
            }
        };
        // clang-format on
    }

    // If we are using multiview, then we only need a single call to renderView() to render all views at the
    // same time. If we are not using multiview, then we call renderView() once per view. The subclass can
    // access the current view via m_currentViewIndex.
    const uint32_t renderViewCount = m_enableMultiview ? 1 : m_viewState.viewCount();
    for (m_currentViewIndex = 0; m_currentViewIndex < renderViewCount; ++m_currentViewIndex) {
        // Acquire and wait for the next swapchain textures to become available for the color and depth swapchains
        KDXr::SwapchainInfo &colorSwapchainInfo = m_colorSwapchains[m_currentViewIndex];
        KDXr::SwapchainInfo &depthSwapchainInfo = m_depthSwapchains[m_currentViewIndex];

        colorSwapchainInfo.swapchain.getNextTextureIndex(m_currentColorImageIndex);
        depthSwapchainInfo.swapchain.getNextTextureIndex(m_currentDepthImageIndex);

        colorSwapchainInfo.swapchain.waitForTexture();
        depthSwapchainInfo.swapchain.waitForTexture();

        // Call subclass renderView() function to record and submit drawing commands for the current view(s)
        renderView();

        // Give the swapchain textures back to the XR runtime, allowing the compositor to use the image.
        colorSwapchainInfo.swapchain.releaseTexture();
        depthSwapchainInfo.swapchain.releaseTexture();
    }

    // Set up the projection layer
    m_projectionLayer = {
        .type = KDXr::CompositionLayerType::Projection,
        .referenceSpace = m_referenceSpace,
        .flags = KDXr::CompositionLayerFlagBits::BlendTextureSourceAlphaBit | KDXr::CompositionLayerFlagBits::CorrectChromaticAberrationBit,
        .views = m_projectionLayerViews
    };

    return true;
}

void XrProjectionLayer::updateScene()
{
}

void XrProjectionLayer::recreateSwapchains()
{
    // TODO: Handle multiview rendering option
    const auto &viewConfigurationViews = engineLayer()->viewConfigurationViews();
    m_viewCount = viewConfigurationViews.size();
    m_viewState.views.resize(m_viewCount);

    if (!m_enableMultiview) {
        // In a non-multiview configuration we have a color and depth swapchain for each view.
        m_colorSwapchains.resize(m_viewCount);
        m_depthSwapchains.resize(m_viewCount);

        for (size_t i = 0; i < m_viewCount; ++i) {
            // Color swapchain and texture views
            auto &colorSwapchain = m_colorSwapchains[i];
            colorSwapchain.swapchain = m_session->createSwapchain({ .format = m_colorSwapchainFormat,
                                                                    .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::ColorAttachmentBit,
                                                                    .width = viewConfigurationViews[i].recommendedTextureWidth,
                                                                    .height = viewConfigurationViews[i].recommendedTextureHeight,
                                                                    .sampleCount = viewConfigurationViews[i].recommendedSwapchainSampleCount });
            const auto &textures = colorSwapchain.swapchain.textures();
            const auto textureCount = textures.size();
            colorSwapchain.textureViews.reserve(textureCount);
            for (size_t j = 0; j < textureCount; ++j)
                colorSwapchain.textureViews.emplace_back(textures[j].createView());

            // Depth swapchain and texture views
            auto &depthSwapchain = m_depthSwapchains[i];
            depthSwapchain.swapchain = m_session->createSwapchain({ .format = m_depthSwapchainFormat,
                                                                    .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::DepthStencilAttachmentBit,
                                                                    .width = viewConfigurationViews[i].recommendedTextureWidth,
                                                                    .height = viewConfigurationViews[i].recommendedTextureHeight,
                                                                    .sampleCount = viewConfigurationViews[i].recommendedSwapchainSampleCount });
            const auto &depthTextures = depthSwapchain.swapchain.textures();
            const auto depthTextureCount = depthTextures.size();
            depthSwapchain.textureViews.reserve(depthTextureCount);
            for (size_t j = 0; j < depthTextureCount; ++j)
                depthSwapchain.textureViews.emplace_back(depthTextures[j].createView());
        }
    } else {
        // In a multiview configuration we have a single color and depth swapchain but they each use
        // textures with multiple array layers.
        m_colorSwapchains.resize(1);
        m_depthSwapchains.resize(1);

        // Color swapchain and texture views
        auto &colorSwapchain = m_colorSwapchains[0];
        colorSwapchain.swapchain = m_session->createSwapchain({ .format = m_colorSwapchainFormat,
                                                                .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::ColorAttachmentBit,
                                                                .width = viewConfigurationViews[0].recommendedTextureWidth,
                                                                .height = viewConfigurationViews[0].recommendedTextureHeight,
                                                                .arrayLayers = m_viewCount,
                                                                .sampleCount = viewConfigurationViews[0].recommendedSwapchainSampleCount });
        const auto &textures = colorSwapchain.swapchain.textures();
        const auto textureCount = textures.size();
        colorSwapchain.textureViews.reserve(textureCount);
        for (size_t j = 0; j < textureCount; ++j)
            colorSwapchain.textureViews.emplace_back(textures[j].createView({ .viewType = KDGpu::ViewType::ViewType2DArray,
                                                                              .range = { .layerCount = m_viewCount } }));

        // Depth swapchain and texture views
        auto &depthSwapchain = m_depthSwapchains[0];
        depthSwapchain.swapchain = m_session->createSwapchain({ .format = m_depthSwapchainFormat,
                                                                .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::DepthStencilAttachmentBit,
                                                                .width = viewConfigurationViews[0].recommendedTextureWidth,
                                                                .height = viewConfigurationViews[0].recommendedTextureHeight,
                                                                .arrayLayers = m_viewCount,
                                                                .sampleCount = viewConfigurationViews[0].recommendedSwapchainSampleCount });
        const auto &depthTextures = depthSwapchain.swapchain.textures();
        const auto depthTextureCount = depthTextures.size();
        depthSwapchain.textureViews.reserve(depthTextureCount);
        for (size_t j = 0; j < depthTextureCount; ++j)
            depthSwapchain.textureViews.emplace_back(depthTextures[j].createView({ .viewType = KDGpu::ViewType::ViewType2DArray,
                                                                                   .range = { .layerCount = m_viewCount } }));
    }
}

} // namespace KDGpuExample
