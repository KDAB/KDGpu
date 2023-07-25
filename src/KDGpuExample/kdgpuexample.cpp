/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "kdgpuexample.h"

#include <KDGpu/utils/logging.h>

#include <KDUtils/dir.h>
#include <KDUtils/file.h>

#include <cstdlib>

namespace KDGpuExample {

std::string assetPath()
{
    const char *path = std::getenv("KDGPUEXAMPLE_ASSET_PATH");
    if (path)
        return path;

    auto dir = KDUtils::Dir{ KDUtils::Dir::applicationDir().path() + "../share/assets" };
    if (dir.exists())
        return dir.path();

#if defined(KDGPUEXAMPLE_ASSET_PATH)
    return KDGPUEXAMPLE_ASSET_PATH;
#else
    return "";
#endif
}

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    using namespace KDUtils;

    File file(File::exists(filename) ? filename : Dir::applicationDir().absoluteFilePath(filename));

    if (!file.open(std::ios::in | std::ios::binary)) {
        SPDLOG_LOGGER_CRITICAL(KDGpu::Logger::logger(), "Failed to open file {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    const ByteArray fileContent = file.readAll();
    std::vector<uint32_t> buffer(fileContent.size() / 4);
    std::memcpy(buffer.data(), fileContent.data(), fileContent.size());

    return buffer;
}

} // namespace KDGpuExample
