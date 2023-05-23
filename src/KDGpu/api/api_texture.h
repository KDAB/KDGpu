/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/texture.h>

namespace KDGpu {

/**
 * @brief ApiTexture
 * \ingroup api
 *
 */
struct ApiTexture {
    virtual void *map() = 0;
    virtual void unmap() = 0;
    virtual SubresourceLayout getSubresourceLayout(const TextureSubresource &subresource) const = 0;
};

} // namespace KDGpu
