/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/utils/logging.h>

namespace KDXr {

inline std::string getSessionStateAsString(SessionState state)
{
    switch (state) {
    case SessionState::Unknown:
        return "Unknown";
    case SessionState::Idle:
        return "Idle";
    case SessionState::Ready:
        return "Ready";
    case SessionState::Synchronized:
        return "Synchronized";
    case SessionState::Visible:
        return "Visible";
    case SessionState::Focused:
        return "Focused";
    case SessionState::Stopping:
        return "Stopping";
    case SessionState::LossPending:
        return "LossPending";
    case SessionState::Exiting:
        return "Exiting";
    default:
        return "Error: Unknown state";
    }
}

inline std::string getVersionAsString(uint64_t version)
{
    return fmt::format("{}.{}.{}", KDXR_VERSION_MAJOR(version), KDXR_VERSION_MINOR(version), KDXR_VERSION_PATCH(version));
}

} // namespace KDXr

template<>
struct fmt::formatter<KDXr::SessionState> : fmt::formatter<std::string> {
    template<typename FormatContext>
    auto format(KDXr::SessionState const &state, FormatContext &ctx) const
    {
        return fmt::formatter<std::string>::format(getSessionStateAsString(state), ctx);
    }
};
