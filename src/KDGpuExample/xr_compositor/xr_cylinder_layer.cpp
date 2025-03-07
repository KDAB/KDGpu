/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_cylinder_layer.h"

#include <KDXr/session.h>
#include <KDXr/utils/logging.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <glm/gtc/constants.hpp>

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

    // Whenever resolution changes, we will need to initialize again,
    // but make sure we only set up this connection once.
    if (!m_reinitializeConnection.belongsTo(resolution.valueChanged()))
        m_reinitializeConnection = resolution.valueChanged().connect([this]() { initialize(); });
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
        .radius = radius(),
        .centralAngle = centralAngle(),
        .aspectRatio = aspectRatio()
    };
    // clang-format on

    return true;
}

void XrCylinderLayer::recreateSwapchains()
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

std::optional<XrCylinderLayer::Intersection> XrCylinderLayer::rayIntersection(const KDXr::Pose &rayCasterPose) const
{
    Intersection intersection;
    const auto cylinderPos = position();
    const auto cylinderOrient = orientation();

    auto rayOrigin = glm::vec3(rayCasterPose.position.x, rayCasterPose.position.y, rayCasterPose.position.z);
    glm::vec3 defaultDirection(0.0f, 0.0f, -1.0f);
    glm::vec3 rayDirection = glm::quat(rayCasterPose.orientation.w, rayCasterPose.orientation.x, rayCasterPose.orientation.y, rayCasterPose.orientation.z) * defaultDirection;
    glm::vec3 cylinderPosition = { cylinderPos.x, cylinderPos.y, cylinderPos.z };
    const glm::quat &cylinderRotation = { cylinderOrient.w, cylinderOrient.x, cylinderOrient.y, cylinderOrient.z };

    // Transform the ray into the cylinder's local space
    glm::mat4 cylinderModelMatrix = glm::translate(glm::mat4(1.0f), cylinderPosition) * glm::mat4_cast(cylinderRotation);
    glm::mat4 invCylinderModelMatrix = glm::inverse(cylinderModelMatrix);

    glm::vec3 localRayOrigin = glm::vec3(invCylinderModelMatrix * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 localRayDirection = glm::normalize(glm::vec3(invCylinderModelMatrix * glm::vec4(rayDirection, 0.0f)));

    // Cylinder parameters
    float radius = this->radius();
    float centralAngle = this->centralAngle();
    float arcLength = radius * centralAngle;
    float height = arcLength / aspectRatio();
    float halfCentralAngle = centralAngle / 2.0f;

    // Check for intersection with the cylindrical surface
    // The equation of a cylinder aligned with the Y-axis is: x^2 + z^2 = radius^2
    // We solve the quadratic equation: a*t^2 + b*t + c = 0
    float a = localRayDirection.x * localRayDirection.x + localRayDirection.z * localRayDirection.z;
    float b = 2.0f * (localRayOrigin.x * localRayDirection.x + localRayOrigin.z * localRayDirection.z);
    float c = localRayOrigin.x * localRayOrigin.x + localRayOrigin.z * localRayOrigin.z - radius * radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
        return std::nullopt; // No intersection
    }

    // Find the nearest positive intersection point
    float t0 = (-b - sqrt(discriminant)) / (2.0f * a);
    float t1 = (-b + sqrt(discriminant)) / (2.0f * a);
    float t = (t0 < t1 && t0 > 0.0) ? t0 : t1;

    if (t < 0.0f) {
        return std::nullopt; // No intersection
    }

    // Check if the intersection point is within the cylinder's height bounds
    glm::vec3 localIntersection = localRayOrigin + t * localRayDirection;
    intersection.withinBounds = localIntersection.y >= -height / 2.0f && localIntersection.y <= height / 2.0f;

    // Check if the intersection point is within the cylinder's angular bounds
    float angle = atan2(localIntersection.z, localIntersection.x) + glm::half_pi<float>(); // Adjusted angle calculation
    intersection.withinBounds = intersection.withinBounds && (angle >= -halfCentralAngle && angle <= halfCentralAngle);

    // Calculate UV coordinates (0..1) with origin at the top right
    glm::vec2 uv = { (angle + halfCentralAngle) / (2.0f * halfCentralAngle),
                     1.0f - (localIntersection.y + height / 2.0f) / height };

    intersection.x = static_cast<int16_t>(resolution().width * uv.x);
    intersection.y = static_cast<int16_t>(resolution().height * uv.y);

    // Transform the local intersection point back to world space
    auto worldIntersection = glm::vec3(cylinderModelMatrix * glm::vec4(localIntersection, 1.0f));
    intersection.worldSpace = { worldIntersection.x, worldIntersection.y, worldIntersection.z };

    return intersection;
}

} // namespace KDGpuExample
