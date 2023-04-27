# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
if(NOT TARGET trompeloeil::trompeloeil)
    FetchContent_Declare(
        trompeloeil
        GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
        GIT_TAG v42
    )
    FetchContent_Populate(trompeloeil)

    add_library(trompeloeil INTERFACE)
    target_include_directories(
        trompeloeil INTERFACE $<BUILD_INTERFACE:${trompeloeil_SOURCE_DIR}/include/>
                              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/trompeloeil>
    )
    add_library(trompeloeil::trompeloeil ALIAS trompeloeil)

    install(DIRECTORY ${trompeloeil_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trompeloeil)
endif()
