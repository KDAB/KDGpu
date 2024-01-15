/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>

#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDXr {

struct Session_t;

/**
 * @brief ApiSession
 * \ingroup api
 *
 */
struct ApiSession {
    virtual std::vector<KDGpu::Format> supportedSwapchainFormats() const = 0;
};

} // namespace KDXr
