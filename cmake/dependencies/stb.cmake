# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(stb QUIET)

if(NOT TARGET stb::stb)
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(stb)

    add_library(stb INTERFACE)
    target_include_directories(
        stb SYSTEM INTERFACE $<BUILD_INTERFACE:${stb_SOURCE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/stb>
    )
    install(DIRECTORY ${stb_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stb)
    add_library(stb::stb ALIAS stb)
endif()
