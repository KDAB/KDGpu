/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_quad_layer.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <KDXr/session.h>

namespace KDGpuExample {

XrQuadLayer::XrQuadLayer(const XrQuadLayerOptions &options)
    : XrCompositorLayer(Type::Quad)
    , m_device(options.device)
    , m_queue(options.queue)
    , m_session(options.session)
    , m_colorSwapchainFormat(options.colorSwapchainFormat)
    , m_depthSwapchainFormat(options.depthSwapchainFormat)
    , m_samples(options.samples)
{
}

XrQuadLayer::~XrQuadLayer()
{
}

void XrQuadLayer::initialize()
{
    recreateSwapchains();

    // Whenever resolution changes, we will need to initialize again,
    // but make sure we only set up this connection once.
    if (!m_reinitializeConnection.belongsTo(resolution.valueChanged()))
        m_reinitializeConnection = resolution.valueChanged().connect([this]() { initialize(); });
}

void XrQuadLayer::cleanup()
{
    m_colorSwapchain.textureViews.clear();
    m_colorSwapchain.swapchain = {};
    m_depthSwapchain.textureViews.clear();
    m_depthSwapchain.swapchain = {};
}

bool XrQuadLayer::update(const KDXr::FrameState &)
{
    // Render the quad layer
    // Acquire and wait for the next swapchain textures to become available for the color and depth swapchains
    m_colorSwapchain.swapchain.getNextTextureIndex(m_currentColorImageIndex);
    m_depthSwapchain.swapchain.getNextTextureIndex(m_currentDepthImageIndex);

    m_colorSwapchain.swapchain.waitForTexture();
    m_depthSwapchain.swapchain.waitForTexture();

    // Call subclass renderQuad() function to record and submit drawing commands for the quad
    renderQuad();

    // Give the swapchain textures back to the XR runtime, allowing the compositor to use the image.
    m_colorSwapchain.swapchain.releaseTexture();
    m_depthSwapchain.swapchain.releaseTexture();

    // Set up the quad layer
    // clang-format off
    m_quadLayer = {
        .type = KDXr::CompositionLayerType::Quad,
        .referenceSpace = m_referenceSpace,
        .flags = KDXr::CompositionLayerFlagBits::BlendTextureSourceAlphaBit | KDXr::CompositionLayerFlagBits::UnpremultiplyAlphaBit | KDXr::CompositionLayerFlagBits::CorrectChromaticAberrationBit,
        .eyeVisibility = KDXr::EyeVisibility::Both,
        .swapchainSubTexture = {
            .swapchain = m_colorSwapchain.swapchain,
            .rect = {
                .offset = { .x = 0, .y = 0 },
                .extent = resolution()
            }
        },
        .pose = { .orientation = orientation(), .position = position() },
        .size = worldSize()
    };
    // clang-format on

    return true;
}

void XrQuadLayer::recreateSwapchains()
{
    m_colorSwapchain = {};
    m_depthSwapchain = {};

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

std::optional<XrQuadLayer::Intersection> XrQuadLayer::rayIntersection(KDXr::Pose rayCasterPose)
{
    Intersection intersection;

    const auto quadXrPos = position();
    const auto quadXrOrient = orientation();

    auto rayOrigin = glm::vec3(rayCasterPose.position.x, rayCasterPose.position.y, rayCasterPose.position.z);

    glm::vec3 defaultDirection(0.0f, 0.0f, -1.0f);
    glm::vec3 rayDirection = glm::quat(rayCasterPose.orientation.w, rayCasterPose.orientation.x, rayCasterPose.orientation.y, rayCasterPose.orientation.z) * defaultDirection;
    glm::vec3 quadPosition = { quadXrPos.x, quadXrPos.y, quadXrPos.z };
    const glm::quat &quadRotation = { quadXrOrient.w, quadXrOrient.x, quadXrOrient.y, quadXrOrient.z };
    const auto quadWidth = worldSize().width;
    const auto quadHeight = worldSize().height;

    // Transform the ray into the quad's local space
    glm::mat4 quadModelMatrix = glm::translate(glm::mat4(1.0f), quadPosition) * glm::mat4_cast(quadRotation);
    glm::mat4 invQuadModelMatrix = glm::inverse(quadModelMatrix);

    glm::vec3 localRayOrigin = glm::vec3(invQuadModelMatrix * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 localRayDirection = glm::normalize(glm::vec3(invQuadModelMatrix * glm::vec4(rayDirection, 0.0f)));

    // Check if Quad is aligned with the XY plane in local space
    float t = -localRayOrigin.z / localRayDirection.z;
    if (t < 0.0f) {
        return std::nullopt; // No intersection
    }

    glm::vec3 localIntersection = localRayOrigin + t * localRayDirection;

    // Check if the intersection point is within the quad bounds
    intersection.withinBounds = (localIntersection.x >= -quadWidth / 2.0f && localIntersection.x <= quadWidth / 2.0f &&
                                 localIntersection.y >= -quadHeight / 2.0f && localIntersection.y <= quadHeight / 2.0f);

    // Calculate UV coordinates (0..1) with origin at the top right
    glm::vec2 uv = { (localIntersection.x + quadWidth / 2.0f) / quadWidth,
                     uv.y = 1.0f - (localIntersection.y + quadHeight / 2.0f) / quadHeight };

    intersection.x = static_cast<int16_t>(resolution().width * uv.x);
    intersection.y = static_cast<int16_t>(resolution().height * uv.y);

    // Transform the local intersection point back to world space
    auto worldIntersection = glm::vec3(quadModelMatrix * glm::vec4(localIntersection, 1.0f));
    intersection.worldSpace = { worldIntersection.x, worldIntersection.y, worldIntersection.z };

    return intersection;
}

} // namespace KDGpuExample
