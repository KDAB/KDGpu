/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDGpu/handle.h>

namespace KDXr {

struct PassthroughLayeOptions;
struct PassthroughLayer_t;

/**
 * @brief ApiPassthroughLayer
 * \ingroup api
 *
 */
struct ApiPassthroughLayer {
    virtual void setRunning(bool running) = 0;
};

} // namespace KDXr
