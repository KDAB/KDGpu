/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuExample/kdgpuexample_export.h>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#include <cstdint>
#include <string>
#include <vector>

namespace KDGpuExample {

/**
 * @defgroup kdgpuexample KDGpuExample API
 *
 * Holds the KDGpuExample Helper API
 */

/*! \addtogroup kdgpuexample
 *  @{
 */

KDGPUEXAMPLE_EXPORT std::string assetPath();

KDGPUEXAMPLE_EXPORT KDUtils::Dir assetDir();

KDGPUEXAMPLE_EXPORT std::vector<uint32_t> readShaderFile(const std::string &filename);

KDGPUEXAMPLE_EXPORT std::vector<uint32_t> readShaderFile(KDUtils::File &file);

/*! @} */

} // namespace KDGpuExample
