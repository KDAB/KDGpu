# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    kdgpu_hello_triangle_native
    VERSION 0.1
    LANGUAGES CXX
)

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
    target_sources(kdgpu_hello_triangle_native PRIVATE utils.mm)
endif()
