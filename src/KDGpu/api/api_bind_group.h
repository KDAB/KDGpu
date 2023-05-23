/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

namespace KDGpu {

struct BindGroupEntry;

/**
 * @brief ApiBindGroup
 * \ingroup api
 *
 */
struct ApiBindGroup {
    virtual void update(const BindGroupEntry &entry) = 0;
};

} // namespace KDGpu
