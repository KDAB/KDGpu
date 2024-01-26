/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logging.h"
#if defined(ANDROID)
#include <spdlog/sinks/android_sink.h>
#endif

namespace KDGpu {

std::shared_ptr<spdlog::logger> Logger::ms_logger = {};
Logger::LoggerFactoryFunction Logger::ms_loggerFactory = {};

void Logger::setLoggerFactory(LoggerFactoryFunction factory)
{
    ms_loggerFactory = factory;
}

Logger::LoggerFactoryFunction Logger::loggerFactory()
{
    return ms_loggerFactory;
}

std::shared_ptr<spdlog::logger> Logger::createLogger()
{
    std::shared_ptr<spdlog::logger> logger;
    if (ms_loggerFactory) {
        // Use the factory set by the application which should check
        // its own spdlog registry first before creating a new logger.
        logger = ms_loggerFactory("KDGpu");
    } else {
        // No factory set, use the spdlog registry from KDGpu
        logger = spdlog::get("KDGpu");
        if (!logger) {
#if defined(ANDROID)
            logger = spdlog::android_logger_mt("KDGpu", "KDGpu");
#else
            logger = spdlog::stdout_color_mt("KDGpu");
#endif
        }
    }
    return logger;
}

} // namespace KDGpu
