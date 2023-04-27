/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "kdgpuexample.h"

#include <cstdlib>

namespace KDGpuExample {

std::string assetPath()
{
    const char *path = std::getenv("KDGPUEXAMPLE_ASSET_PATH");
    if (path)
        return path;

#if defined(KDGPUEXAMPLE_ASSET_PATH)
    return KDGPUEXAMPLE_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDGpuExample
