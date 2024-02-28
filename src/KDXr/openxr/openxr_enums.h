/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>

#include <KDGpu/gpu_core.h>

#include <openxr/openxr.h>

namespace KDXr {

XrFormFactor formFactorToXrFormFactor(FormFactor formFactor);

XrSwapchainUsageFlags swapchainUsageFlagsToXrSwapchainUsageFlags(SwapchainUsageFlags flags);

KDGpu::TextureUsageFlags kdxrSwapchainUsageFlagsToKDGpuTextureUsageFlags(SwapchainUsageFlags flags);

SessionState xrSessionStateToSessionState(XrSessionState state);

XrViewConfigurationType viewConfigurationTypeToXrViewConfigurationType(ViewConfigurationType type);

ViewStateFlags xrViewStateFlagsToViewStateFlags(XrViewStateFlags flags);

XrCompositionLayerFlags compositionLayerFlagsToXrCompositionLayerFlags(CompositionLayerFlags flags);

XrEnvironmentBlendMode environmentBlendModeToXrEnvironmentBlendMode(EnvironmentBlendMode mode);

XrEyeVisibility eyeVisibilityToXrEyeVisibility(EyeVisibility visibility);

XrActionType actionTypeToXrActionType(ActionType type);

SpaceStateFlags xrSpaceLocationFlagsToSpaceStateFlags(XrSpaceLocationFlags flags);
SpaceStateFlags xrSpaceVelocityFlagsToSpaceStateFlags(XrSpaceVelocityFlags flags);

} // namespace KDXr
