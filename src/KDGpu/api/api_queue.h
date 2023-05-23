/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <vector>

namespace KDGpu {

struct PresentOptions;
struct SubmitOptions;

/**
 * @brief ApiQueue
 * \ingroup api
 *
 */
struct ApiQueue {
    virtual void waitUntilIdle() = 0;
    // TODO: Return type and arguments?
    virtual void submit(const SubmitOptions &options) = 0;
    virtual PresentResult present(const PresentOptions &options) = 0;
    virtual std::vector<PresentResult> lastPerSwapchainPresentResults() const = 0;
};

} // namespace KDGpu
