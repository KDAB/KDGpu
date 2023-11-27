/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logging.h"

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
        logger = ms_loggerFactory("KDGpu");
        SPDLOG_LOGGER_INFO(logger, "Hello from the custom KDGpu Logger");
    } else {
        logger = spdlog::stdout_color_mt("KDGpu");
        SPDLOG_LOGGER_INFO(logger, "Hello from the default KDGpu Logger");
    }
    return logger;
}

} // namespace KDGpu
