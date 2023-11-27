/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <functional>

namespace KDGpu {

class KDGPU_EXPORT Logger
{
public:
    static std::shared_ptr<spdlog::logger> logger()
    {
        if (!ms_logger)
            ms_logger = createLogger();
        return ms_logger;
    }

    using LoggerFactoryFunction = std::function<std::shared_ptr<spdlog::logger>(const std::string &)>;

    static void setLoggerFactory(LoggerFactoryFunction factory);
    static LoggerFactoryFunction loggerFactory();

private:
    static std::shared_ptr<spdlog::logger> createLogger();

    static std::shared_ptr<spdlog::logger> ms_logger;
    static LoggerFactoryFunction ms_loggerFactory;
};

} // namespace KDGpu
