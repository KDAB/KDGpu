/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/utils/flags.h>

#include <span>
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

using Time = int64_t;
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

inline uint32_t viewCount(ViewConfigurationType viewConfigurationType)
{
    switch (viewConfigurationType) {
    case ViewConfigurationType::PrimaryMono:
        return 1;
    case ViewConfigurationType::PrimaryStereo:
        return 2;
    case ViewConfigurationType::PrimaryQuadVarjo:
        return 4;
    default:
        return 0;
    }
}

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

    bool operator==(const Quaternion &other) const = default;
};

struct Vector3 {
    float x{ 0.0f };
    float y{ 0.0f };
    float z{ 0.0f };

    bool operator==(const Vector3 &other) const = default;
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

enum class ProcessEventsResult : int32_t {
    Success = 0,
    EventUnavailable = 4,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    HandleInvalid = -12,
    InstanceLost = -13,
    MaxEnum = 0x7fffffff
};

enum class SessionState : int32_t {
    Unknown = 0,
    Idle = 1,
    Ready = 2,
    Synchronized = 3,
    Visible = 4,
    Focused = 5,
    Stopping = 6,
    LossPending = 7,
    Exiting = 8,
    MaxEnum = 0x7fffffff
};

enum class WaitFrameResult : int32_t {
    Success = 0,
    SessionLossPending = 3,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    HandleInvalid = -12,
    InstanceLost = -13,
    SessionNotRunning = -16,
    SessionLost = -17,
    MaxEnum = 0x7fffffff
};

struct FrameState {
    WaitFrameResult waitFrameResult{ WaitFrameResult::MaxEnum };
    Time predictedDisplayTime{ 0 };
    Duration predictedDisplayPeriod{ 0 };
    bool shouldRender{ false };
};

enum class BeginFrameResult : int32_t {
    Success = 0,
    SessionLossPending = 3,
    FrameDiscarded = 9,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    HandleInvalid = -12,
    InstanceLost = -13,
    SessionNotRunning = -16,
    SessionLost = -17,
    CallOrderInvalid = -37,
    MaxEnum = 0x7fffffff
};

enum class EndFrameResult : int32_t {
    Success = 0,
    SessionLossPending = 3,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    HandleInvalid = -12,
    InstanceLost = -13,
    SessionNotRunning = -16,
    SessionLost = -17,
    LayerInvalid = -23,
    LayerLimitExceeded = -24,
    SwapchainRectInvalid = -25,
    TimeInvalid = -30,
    CallOrderInvalid = -37,
    PoseInvalid = -39,
    EnvironmentBlendModeUnsupported = -42,
    MaxEnum = 0x7fffffff
};

enum class ViewStateFlagBits : uint32_t {
    OrientationValidBit = 0x00000001,
    PositionValidBit = 0x00000002,
    OrientationTrackedBit = 0x00000004,
    PositionTrackedBit = 0x00000008,
    MaxEnum = 0x7fffffff
};

using ViewStateFlags = KDGpu::Flags<ViewStateFlagBits>;

struct FieldOfView {
    float angleLeft{ 0.0f };
    float angleRight{ 0.0f };
    float angleUp{ 0.0f };
    float angleDown{ 0.0f };
};

struct View {
    Pose pose{};
    FieldOfView fieldOfView{};
};
struct ViewState {
    ViewStateFlags viewStateFlags{ ViewStateFlagBits::MaxEnum };
    uint32_t viewCount{ 0 };
    std::span<View> views;
};

enum class LocateViewsResult : int64_t {
    Success = 0,
    SessionLossPending = 3,
    ValidationFailure = -1,
    RuntimeFailure = -2,
    SizeInsufficient = -11,
    HandleInvalid = -12,
    InstanceLost = -13,
    SessionLost = -17,
    TimeInvalid = -30,
    ViewConfigurationTypeUnsupported = -41,
    MaxEnum = 0x7fffffffffffffff
};

enum class CompositionLayerFlagBits : uint32_t {
    CorrectChromaticAberrationBit = 0x00000001,
    BlendTextureSourceAlphaBit = 0x00000002,
    UnpremultiplyAlphaBit = 0x00000004,
    MaxEnum = 0x7fffffff
};

using CompositionLayerFlags = KDGpu::Flags<CompositionLayerFlagBits>;

enum class EyeVisibility : uint32_t {
    Both = 0,
    Left = 1,
    Right = 2,
    MaxEnum = 0x7fffffff
};

} // namespace KDXr

OPERATORS_FOR_FLAGS(KDXr::CompositionLayerFlags)
OPERATORS_FOR_FLAGS(KDXr::SwapchainUsageFlags)
OPERATORS_FOR_FLAGS(KDXr::ViewStateFlags)
