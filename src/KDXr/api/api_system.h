/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>

#include <vector>

namespace KDXr {

struct System_t;

/**
 * @brief ApiSystem
 * \ingroup api
 *
 */
struct ApiSystem {
    virtual SystemProperties queryProperties() const = 0;
    virtual std::vector<ViewConfigurationType> queryViewConfigurations() const = 0;
};

} // namespace KDXr
