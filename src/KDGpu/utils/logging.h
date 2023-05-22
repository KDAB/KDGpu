/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace KDGpu {

namespace Logger {

inline std::shared_ptr<spdlog::logger> logger()
{
    static std::shared_ptr<spdlog::logger> l = spdlog::get("KDGpu");
    if (!l) {
        l = spdlog::stdout_color_mt("KDGpu");
        SPDLOG_LOGGER_INFO(l, "Hello from the KDGpu Logger");
    }
    return l;
}

} // namespace Logger

} // namespace KDGpu
