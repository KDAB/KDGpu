# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    kdgpu_hello_sphere_rt
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    hello_sphere_rt.cpp
    LIBS
    KDGpu::KDGpuExample
)

add_dependencies(
    ${PROJECT_NAME}
    KDGpu_HelloSphereRtGen
    KDGpu_HelloSphereRtMiss
    KDGpu_HelloSphereRtClosest
    KDGpu_HelloSphereRtIntersection
)
