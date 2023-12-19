/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <stdint.h>
#include <string>

using HANDLE = void *;

#define KDXR_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

namespace KDXr {

/**
 * @defgroup public Public API
 *
 * Holds the Public API
 */

/*! \addtogroup public
 *  @{
 */

struct Extension {
    std::string name;
    uint32_t version{ 0 };

    friend bool operator==(const Extension &, const Extension &) = default;
};

} // namespace KDXr
