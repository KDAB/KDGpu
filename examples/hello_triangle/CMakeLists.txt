# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    kdgpu_hello_triangle
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
)

if(KDGPU_HLSL_SUPPORT)
    add_kdgpu_example(
        NAME
        kdgpu_hello_triangle_hlsl
        SOURCES
        main.cpp
        hello_triangle.cpp
        LIBS
        KDGpu::KDGpuExample
    )
    target_compile_definitions(kdgpu_hello_triangle_hlsl PRIVATE USE_HLSL_SHADERS)
endif()

if(KDGPU_SLANG_SUPPORT)
    add_kdgpu_example(
        NAME
        kdgpu_hello_triangle_slang
        SOURCES
        main.cpp
        hello_triangle.cpp
        LIBS
        KDGpu::KDGpuExample
    )
    target_compile_definitions(kdgpu_hello_triangle_slang PRIVATE USE_SLANG_SHADERS)
endif()
