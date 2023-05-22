# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    kdgpu_rotating_triangle
    VERSION 0.1
    LANGUAGES CXX
)

find_package(Vulkan REQUIRED)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    rotating_triangle.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
)
