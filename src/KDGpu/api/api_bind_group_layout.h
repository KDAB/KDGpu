/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

namespace KDGpu {

/**
 * @brief ApiBindGroupLayout
 * \ingroup api
 *
 */
struct ApiBindGroupLayout {
    // TODO: Complete

    virtual bool isCompatibleWith(const ApiBindGroupLayout &other) const = 0;
};

} // namespace KDGpu
