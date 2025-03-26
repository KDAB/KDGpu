/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "kdgpuexample.h"

#include <KDGpu/utils/logging.h>

#include <KDFoundation/core_application.h>
#include <KDUtils/dir.h>

#include <cstdlib>

namespace KDGpuExample {

std::string assetPath()
{
    return assetDir().path();
}

KDUtils::Dir assetDir()
{
    const char *path = std::getenv("KDGPUEXAMPLE_ASSET_PATH");
    if (path)
        return KDUtils::Dir(path);

    auto app = KDFoundation::CoreApplication::instance();
    auto dir = app->standardDir(KDFoundation::StandardDir::Assets);
    if (dir.exists())
        return dir;

#if defined(KDGPUEXAMPLE_ASSET_PATH)
    return KDUtils::Dir(KDGPUEXAMPLE_ASSET_PATH);
#else
    return {};
#endif
}

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    using namespace KDUtils;
    using namespace KDFoundation;

    auto file = [filename]() {
        if (File::exists(filename))
            return File(filename);
        return assetDir().file(filename);
    }();

    return readShaderFile(file);
}

std::vector<uint32_t> readShaderFile(KDUtils::File &file)
{
    using namespace KDUtils;

    if (!file.open(std::ios::in | std::ios::binary)) {
        SPDLOG_LOGGER_CRITICAL(KDGpu::Logger::logger(), "Failed to open file {}", file.path());
        throw std::runtime_error("Failed to open file");
    }

    const ByteArray fileContent = file.readAll();
    std::vector<uint32_t> buffer(fileContent.size() / 4);
    std::memcpy(buffer.data(), fileContent.data(), fileContent.size());

    return buffer;
}

} // namespace KDGpuExample
