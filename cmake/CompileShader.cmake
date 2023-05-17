# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# Note: Starting with CMake version 3.21 FindVulkan.cmake will search for
# Vulkan_GLSLANG_VALIDATOR_EXECUTABLE by itself When requiring this version we
# can remove the next block then
if(NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
    find_program(
        Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
        NAMES glslangValidator
        HINTS "$ENV{VULKAN_SDK}/bin"
    )
endif()
if(NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
    message(FATAL_ERROR "glslangValidator executable not found")
endif()

# Compile a shader using glslangValidator
function(CompileShader target shader output)
    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
        COMMENT "Compile shader using glslangValidator"
    )

    add_custom_target(
        ${target}
        DEPENDS ${output}
        COMMENT "Target to compile a shader"
    )
endfunction()

# Compile s shader set
function(CompileShaderSet target name)
    # TODO: in future we probably want to check which shaders we have instead of assuming vert/frag
    CompileShader(${target}VertexShader ${name}.vert ${name}.vert.spv)
    CompileShader(${target}FragmentShader ${name}.frag ${name}.frag.spv)

    # TODO: for now generate ALL, in future would be better to build on case by case
    add_custom_target(
        ${target}Shaders ALL
        DEPENDS ${target}VertexShader ${target}FragmentShader
        COMMENT "Target to compile a shader set"
    )
endfunction()
