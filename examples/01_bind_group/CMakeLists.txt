# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    01_kdgpu_bind_group
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
)
