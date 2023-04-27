# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    11_kdgpu_dynamic_ubo
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    dynamic_ubo_triangles.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
)
