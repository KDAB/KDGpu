# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.12)
project(KDGpu-Tests)

include_directories(./doctest)

# Build a test program
function(add_kdgpu_test NAME SOURCES)
    set(TARGET_NAME test_kdgpu_${NAME})
    add_executable(${TARGET_NAME} ${SOURCES})

    target_link_libraries(${TARGET_NAME} KDGpu KDUtils::KDUtils)

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpu")

    if(APPLE)
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

if(KDGPU_BUILD_EXAMPLES)
    # Build an example test program
    function(add_kdgpuexample_test NAME SOURCES)
        set(TARGET_NAME test_kdgpuexample_${NAME})
        add_executable(${TARGET_NAME} ${SOURCES})

        target_link_libraries(${TARGET_NAME} KDGpu::KDGpu KDGpu::KDGpuExample)

        add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
        set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpuExample")

        if(APPLE)
            target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
        endif()
    endfunction()

    add_subdirectory(instance)
endif()

add_subdirectory(pool)
add_subdirectory(buffer)
add_subdirectory(texture)
add_subdirectory(textureview)
add_subdirectory(bindgroup)
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
