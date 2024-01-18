# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(shaders)

if(KDGPU_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(KDGPU_BUILD_TESTS)
    add_subdirectory(tests)
endif()

add_custom_target(kdgpu_assets ALL COMMENT "Target to copy the shader assets into the current source directory")

foreach(shader ${SHADER_FILES})
    add_custom_command(
        TARGET kdgpu_assets
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${CMAKE_CURRENT_BINARY_DIR}/${shader}
        COMMENT "Copy shader assets into the current source directory"
    )
endforeach(shader)
