# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
function(kdgpu_check_submodule_exists name path)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${path}/.git")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
            message(
                FATAL_ERROR
                    "The ${name} git submodule is not initialized.\n"
                    "Please run the following commands in the source directory (${PROJECT_SOURCE_DIR}):\n"
                    "    git submodule update --init --recursive\n"
            )
        else()
            message(FATAL_ERROR "The ${name} submodule is missing - please report a broken source package.\n")
        endif()
    endif()
endfunction()
