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
#include <vector>

namespace KDXr {

struct Action_t;
struct ActionSet_t;
class XrApi;

/**
    @brief Holds option fields used for Action creation
    @ingroup public
    @headerfile action.h <KDXr/action.h>
 */
struct ActionOptions {
    std::string name;
    std::string localizedName;
    ActionType type{ ActionType::MaxEnum };
    std::vector<std::string> subactionPaths;
};

class KDXR_EXPORT Action
{
public:
    Action();
    ~Action();

    Action(Action &&) noexcept;
    Action &operator=(Action &&) noexcept;

    Action(const Action &) = delete;
    Action &operator=(const Action &) = delete;

    Handle<Action_t> handle() const noexcept { return m_action; }
    bool isValid() const { return m_action.isValid(); }

    operator Handle<Action_t>() const noexcept { return m_action; }

private:
    Action(XrApi *api, const Handle<ActionSet_t> &actionSetHandle, const ActionOptions &options);

    XrApi *m_api{ nullptr };
    Handle<ActionSet_t> m_actionSetHandle;
    Handle<Action_t> m_action;

    friend class ActionSet;
};

} // namespace KDXr
