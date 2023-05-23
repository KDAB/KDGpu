# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
# Build and example
function(add_kdgpu_example)
    include(CMakeParseArguments)
    cmake_parse_arguments(
        KD_ARGS
        ""
        "NAME"
        "SOURCES;LIBS"
        ${ARGN}
    )

    add_executable(${KD_ARGS_NAME} ${KD_ARGS_SOURCES})

    if(WIN32)
        # force no console window, but keep main() as cross-platform entry point
        target_link_options(${KD_ARGS_NAME} PRIVATE "/SUBSYSTEM:windows" "/ENTRY:mainCRTStartup")
    endif()

    target_link_libraries(${KD_ARGS_NAME} PRIVATE ${KD_ARGS_LIBS})

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE PLATFORM_LINUX)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE PLATFORM_WIN32)
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE UNICODE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE PLATFORM_MACOS)
    endif()

    target_compile_features(${KD_ARGS_NAME} PUBLIC cxx_std_20)

    # set rpath for the installed binary to work
    if(APPLE)
        set_target_properties(${KD_ARGS_NAME} PROPERTIES INSTALL_RPATH "@loader_path;${CMAKE_INSTALL_FULL_LIBDIR}")
    elseif(UNIX)
        set_target_properties(${KD_ARGS_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/:${CMAKE_INSTALL_FULL_LIBDIR}")
    endif()

    install(TARGETS ${KD_ARGS_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction()

add_subdirectory(hello_triangle_native)
add_subdirectory(hello_triangle)
add_subdirectory(compute_particles)
add_subdirectory(render_to_texture)
add_subdirectory(hello_triangle_overlap)
add_subdirectory(dynamic_ubo)
add_subdirectory(textured_quad)
add_subdirectory(hello_triangle_msaa)
add_subdirectory(offscreen_rendering)
add_subdirectory(multiview)
