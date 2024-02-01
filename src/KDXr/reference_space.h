/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>
#include <KDXr/kdxr_export.h>

#include <string>

namespace KDXr {

struct Action_t;
struct ReferenceSpace_t;
struct Session_t;
class XrApi;

/**
    @brief Holds option fields used for ReferenceSpace creation
    @ingroup public
    @headerfile reference_space.h <KDXr/reference_space.h>
 */
struct ReferenceSpaceOptions {
    ReferenceSpaceType type{ ReferenceSpaceType::Local };
    Pose pose{};
};

struct ActionSpaceOptions {
    Handle<Action_t> action;
    std::string subactionPath;
    Pose poseInActionSpace{};
};

struct LocateSpaceOptions {
    Handle<ReferenceSpace_t> baseSpace;
    Time time{ 0 };
    bool requestVelocity{ false };
};

class KDXR_EXPORT ReferenceSpace
{
public:
    ReferenceSpace();
    ~ReferenceSpace();

    ReferenceSpace(ReferenceSpace &&);
    ReferenceSpace &operator=(ReferenceSpace &&);

    ReferenceSpace(const ReferenceSpace &) = delete;
    ReferenceSpace &operator=(const ReferenceSpace &) = delete;

    Handle<ReferenceSpace_t> handle() const noexcept { return m_referenceSpace; }
    bool isValid() const { return m_referenceSpace.isValid(); }

    operator Handle<ReferenceSpace_t>() const noexcept { return m_referenceSpace; }

    LocateSpaceResult locateSpace(const LocateSpaceOptions &options, SpaceState &state) const;

private:
    explicit ReferenceSpace(const Handle<Session_t> &sessionHandle, XrApi *api, const ReferenceSpaceOptions &options);
    explicit ReferenceSpace(const Handle<Session_t> &sessionHandle, XrApi *api, const ActionSpaceOptions &options);

    XrApi *m_api{ nullptr };
    Handle<Session_t> m_sessionHandle;
    Handle<ReferenceSpace_t> m_referenceSpace;

    friend class Session;
};

} // namespace KDXr
