/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/action.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>

#include <KDGpu/handle.h>

#include <string>

namespace KDXr {

struct ActionSet_t;
struct Instance_t;
class XrApi;

/**
    @brief Holds option fields used for ActionSet creation
    @ingroup public
    @headerfile action_set.h <KDXr/action_set.h>
 */
struct ActionSetOptions {
    std::string name;
    std::string localizedName;
    uint32_t priority{ 0 };
};

class KDXR_EXPORT ActionSet
{
public:
    ActionSet();
    ~ActionSet();

    ActionSet(ActionSet &&) noexcept;
    ActionSet &operator=(ActionSet &&) noexcept;

    ActionSet(const ActionSet &) = delete;
    ActionSet &operator=(const ActionSet &) = delete;

    KDGpu::Handle<ActionSet_t> handle() const noexcept { return m_actionSet; }
    bool isValid() const { return m_actionSet.isValid(); }

    operator KDGpu::Handle<ActionSet_t>() const noexcept { return m_actionSet; }

    Action createAction(const ActionOptions &options);

private:
    ActionSet(XrApi *api, const KDGpu::Handle<Instance_t> &instanceHandle, const ActionSetOptions &options);

    XrApi *m_api{ nullptr };
    KDGpu::Handle<Instance_t> m_instanceHandle;
    KDGpu::Handle<ActionSet_t> m_actionSet;

    friend class Instance;
};

} // namespace KDXr
