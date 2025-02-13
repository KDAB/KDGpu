/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_reference_space.h"

#include <KDXr/reference_space.h>
#include <KDXr/openxr/openxr_enums.h>
#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrReferenceSpace::OpenXrReferenceSpace(OpenXrResourceManager *_openxrResourceManager,
                                           XrSpace _referenceSpace,
                                           const KDGpu::Handle<Session_t> _sessionHandle,
                                           ReferenceSpaceType _type,
                                           Pose _pose) noexcept
    : openxrResourceManager(_openxrResourceManager)
    , referenceSpace(_referenceSpace)
    , sessionHandle(_sessionHandle)
    , type(_type)
    , pose(_pose)
{
}

OpenXrReferenceSpace::OpenXrReferenceSpace(OpenXrResourceManager *_openxrResourceManager,
                                           XrSpace _referenceSpace,
                                           const KDGpu::Handle<Session_t> _sessionHandle,
                                           const KDGpu::Handle<Action_t> _actionHandle,
                                           Pose _pose) noexcept
    : openxrResourceManager(_openxrResourceManager)
    , referenceSpace(_referenceSpace)
    , sessionHandle(_sessionHandle)
    , actionHandle(_actionHandle)
    , pose(_pose)
{
}

LocateSpaceResult OpenXrReferenceSpace::locateSpace(const LocateSpaceOptions &options, SpaceState &state)
{
    OpenXrReferenceSpace *openxrBaseSpace = openxrResourceManager->getReferenceSpace(options.baseSpace);
    assert(openxrBaseSpace);

    XrSpaceVelocity spaceVelocity{ XR_TYPE_SPACE_VELOCITY };
    XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
    if (options.requestVelocity)
        spaceLocation.next = &spaceVelocity;
    const auto result = xrLocateSpace(referenceSpace, openxrBaseSpace->referenceSpace, options.time, &spaceLocation);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Failed to locate space");
        state = SpaceState{};
    } else {
        const auto &pose = spaceLocation.pose;
        state = SpaceState{
            .spaceStateFlags = xrSpaceLocationFlagsToSpaceStateFlags(spaceLocation.locationFlags),
            .pose = Pose{
                    .orientation = Quaternion{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w },
                    .position = Vector3{ pose.position.x, pose.position.y, pose.position.z } }
        };
        if (options.requestVelocity) {
            state.spaceStateFlags |= xrSpaceVelocityFlagsToSpaceStateFlags(spaceVelocity.velocityFlags);
            state.linearVelocity = Vector3{ spaceVelocity.linearVelocity.x, spaceVelocity.linearVelocity.y, spaceVelocity.linearVelocity.z };
            state.angularVelocity = Vector3{ spaceVelocity.angularVelocity.x, spaceVelocity.angularVelocity.y, spaceVelocity.angularVelocity.z };
        }
    }

    return static_cast<LocateSpaceResult>(result);
}

} // namespace KDXr
