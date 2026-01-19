# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.12)
project(KDGpu-Tests)

# Build a test program
function(add_kdgpu_test NAME SOURCES)
    set(TARGET_NAME test_kdgpu_${NAME})
    add_executable(${TARGET_NAME} ${SOURCES})

    target_link_libraries(${TARGET_NAME} KDGpu KDUtils::KDUtils doctest::doctest)

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpu")

    if(APPLE)
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

# Build a test program that links against KDGpuUtils
function(add_kdgpu_utils_test NAME SOURCES)
    set(TARGET_NAME test_kdgpuutils_${NAME})
    add_executable(${TARGET_NAME} ${SOURCES})

    target_link_libraries(
        ${TARGET_NAME}
        KDGpu::KDGpu
        KDGpu::KDGpuUtils
        KDUtils::KDUtils
        doctest::doctest
    )

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpuUtils")

    if(APPLE)
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

if(KDGPU_BUILD_EXAMPLES)
    # Build an example test program
    function(add_kdgpuexample_test NAME SOURCES)
        set(TARGET_NAME test_kdgpuexample_${NAME})
        add_executable(${TARGET_NAME} ${SOURCES})

        target_link_libraries(${TARGET_NAME} KDGpu::KDGpu KDGpu::KDGpuExample doctest::doctest)

        add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
        set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpuExample")

        if(APPLE)
            target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
        endif()
    endfunction()

    add_subdirectory(instance)
endif()

add_subdirectory(adapter)
add_subdirectory(pool)
add_subdirectory(buffer)
add_subdirectory(texture)
add_subdirectory(textureview)
add_subdirectory(bindgroup)
add_subdirectory(bindgrouppool)
add_subdirectory(bindgrouplayout)
add_subdirectory(sampler)
add_subdirectory(compute_pipeline)
add_subdirectory(compute_pass_command_recorder)
add_subdirectory(command_recorder)
add_subdirectory(command_buffer)
add_subdirectory(graphics_pipeline)
add_subdirectory(pipelinelayout)
add_subdirectory(fence)
add_subdirectory(render_pass_command_recorder)
add_subdirectory(gpu_semaphore)
add_subdirectory(shader_module)
add_subdirectory(timestamp_query_recorder)
add_subdirectory(memory_stats)
add_subdirectory(vulkanframebufferkey)
add_subdirectory(vulkanrenderpasskey)
add_subdirectory(acceleration_structure)
add_subdirectory(raytracing_pipeline)
add_subdirectory(raytracing_pass_command_recorder)
add_subdirectory(ycbcrconversions)

if(KDGPU_BUILD_KDGPUUTILS)
    add_subdirectory(staging_buffer_pool)
    add_subdirectory(resource_deleter)
endif()

find_package(CUDAToolkit QUIET)
if(${CUDAToolkit_FOUND})
    if(CMAKE_SIZEOF_VOID_P EQUAL 8) # CUDA typically only provides x64 libraries
        add_subdirectory(fragmentation)
    else()
        message(STATUS "Skipping CUDA fragmentation tests (x86 target, CUDA is x64-only)")
    endif()
endif()

if(KDGPU_CODE_COVERAGE)

    # Detect if using Clang and set the correct gcov executable
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        find_program(LLVM_COV_PATH llvm-cov)
        if(LLVM_COV_PATH)
            set(GCOVR_ADDITIONAL_ARGS --gcov-executable "${LLVM_COV_PATH} gcov" --gcov-ignore-parse-errors
                                      --print-summary
            )
        endif()
    else()
        set(GCOVR_ADDITIONAL_ARGS --gcov-ignore-parse-errors --print-summary)
    endif()

    setup_target_for_coverage_gcovr_html(
        NAME
        coverage
        BASE_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        EXECUTABLE
        ctest
        -L
        "KDGpu;KDGpuUtils"
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
        "*/vcpkg_installed/*"
        "*/tests/*"
    )
    add_feature_info(KDGpu-Coverage ON "Generate Code Coverage - ninja coverage")
endif()
