# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    test-raytracing-pipeline
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_test(${PROJECT_NAME} tst_raytracing_pipeline.cpp)

add_dependencies(
    test_kdgpu_${PROJECT_NAME}
    KDGpu_RayTracingPipelineGen
    KDGpu_RayTracingPipelineGenSC
    KDGpu_RayTracingPipelineMiss
    KDGpu_RayTracingPipelineClosest
)
