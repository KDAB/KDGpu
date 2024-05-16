/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <vector>
#include <cstdint>

namespace KDGpu {

/**
 * @brief ApiRayTracingPipeline
 * \ingroup api
 *
 */
struct ApiRayTracingPipeline {
    virtual std::vector<uint8_t> shaderGroupHandles(uint32_t firstGroup, uint32_t groupCount) const = 0;
};

} // namespace KDGpu
