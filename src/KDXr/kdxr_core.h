/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/utils/flags.h>

#include <stdint.h>
#include <string>

using HANDLE = void *;

#define KDXR_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define KDXR_VERSION_MAJOR(version) (uint16_t)(((uint64_t)(version) >> 48) & 0xffffULL)
#define KDXR_VERSION_MINOR(version) (uint16_t)(((uint64_t)(version) >> 32) & 0xffffULL)
#define KDXR_VERSION_PATCH(version) (uint32_t)((uint64_t)(version)&0xffffffffULL)

namespace KDXr {

/**
 * @defgroup public Public API
 *
 * Holds the Public API
 */

/*! \addtogroup public
 *  @{
 */

constexpr int64_t InfiniteDuration = 0x7fffffffffffffffLL;

using Duration = int64_t;

struct ApiLayer {
    std::string name;
    std::string description;
    uint64_t specVersion{ 0 };
    uint32_t layerVersion{ 0 };

    friend bool operator==(const ApiLayer &, const ApiLayer &) = default;
};

struct Extension {
    std::string name;
    uint32_t extensionVersion{ 0 };

    friend bool operator==(const Extension &, const Extension &) = default;
};

struct InstanceProperties {
    std::string runtimeName;
    uint64_t runtimeVersion{ 0 };
};

struct SystemGraphicsProperties {
    uint32_t maxSwapchainWidth{ 0 };
    uint32_t maxSwapchainHeight{ 0 };
    uint32_t maxLayerCount{ 0 };
};

struct SystemTrackingProperties {
    bool hasOrientationTracking{ false };
    bool hasPositionTracking{ false };
};

struct SystemProperties {
    uint32_t vendorId{ 0 };
    std::string systemName;
    SystemGraphicsProperties graphicsProperties{};
    SystemTrackingProperties trackingProperties{};
};

enum class FormFactor : uint32_t {
    HeadMountedDisplay = 1,
    HandheldDisplay = 2,
    MaxEnum = 0x7fffffff
};

enum class ViewConfigurationType : uint32_t {
    PrimaryMono = 1,
    PrimaryStereo = 2,
    PrimaryQuadVarjo = 1000037000,
    MaxEnum = 0x7fffffff
};

enum class EnvironmentBlendMode : uint32_t {
    Opaque = 1,
    Additive = 2,
    AlphaBlend = 3,
    MaxEnum = 0x7fffffff
};

struct ViewConfigurationView {
    uint32_t recommendedTextureWidth{ 0 };
    uint32_t maxTextureWidth{ 0 };
    uint32_t recommendedTextureHeight{ 0 };
    uint32_t maxTextureHeight{ 0 };
    uint32_t recommendedSwapchainSampleCount{ 0 };
    uint32_t maxSwapchainSampleCount{ 0 };
};

struct GraphicsRequirements {
    uint64_t minApiVersionSupported{ 0 };
    uint64_t maxApiVersionSupported{ 0 };
};

enum class ReferenceSpaceType : uint32_t {
    View = 1,
    Local = 2,
    Stage = 3,
    LocalFloor = 1000426000,
    MaxEnum = 0x7fffffff
};

struct Quaternion {
    float x{ 0.0f };
    float y{ 0.0f };
    float z{ 0.0f };
    float w{ 1.0f };
};

struct Vector3 {
    float x{ 0.0f };
    float y{ 0.0f };
    float z{ 0.0f };
};

struct Pose {
    Quaternion orientation{};
    Vector3 position{};
};

enum class SwapchainUsageFlagBits : uint32_t {
    ColorAttachmentBit = 0x00000001,
    DepthStencilAttachmentBit = 0x00000002,
    UnorderedAccessBit = 0x00000004,
    TransferSrcBit = 0x00000008,
    TransferDstBit = 0x00000010,
    SampledBit = 0x00000020,
    MutableFormatBit = 0x00000040,
    InputAttachmentBit = 0x00000080,
    MaxEnum = 0x7fffffff
};
using SwapchainUsageFlags = KDGpu::Flags<SwapchainUsageFlagBits>;

enum class AcquireSwapchainTextureResult : int32_t {
    Success = 0,
    SessionLossPending = 3,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    HandleInvalid = -12,
    InstanceLost = -13,
    SessionLost = -17,
    CallOrderInvalid = -37,
    MaxEnum = 0x7fffffff
};

using ReleaseTextureResult = AcquireSwapchainTextureResult;

enum class WaitSwapchainTextureResult : int32_t {
    Success = 0,
    TimeoutExpired = 1,
    SessionLossPending = 3,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    HandleInvalid = -12,
    InstanceLost = -13,
    SessionLost = -17,
    CallOrderInvalid = -37,
    MaxEnum = 0x7fffffff
};

} // namespace KDXr

OPERATORS_FOR_FLAGS(KDXr::SwapchainUsageFlags)
