# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    02_kdgpu_hello_triangle
    VERSION 0.1
    LANGUAGES CXX
)

find_package(Vulkan REQUIRED)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    LIBS
    KDGpu::KDGpu
    KDGpu::KDGpuExample
    glm::glm
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(02_kdgpu_hello_triangle PRIVATE utils.mm)
endif()
