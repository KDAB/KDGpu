# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    08_kdgpu_hello_triangle_kdgui_fence
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    hello_triangle.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
)
