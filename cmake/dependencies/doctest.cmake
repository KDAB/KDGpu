# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(doctest QUIET)
if(NOT TARGET doctest::doctest)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.9
    )

    FetchContent_GetProperties(doctest)

    if(NOT doctest_POPULATED)
        FetchContent_Populate(doctest)

        add_library(doctest INTERFACE)
    endif()

    target_include_directories(
        doctest INTERFACE $<BUILD_INTERFACE:${doctest_SOURCE_DIR}/doctest>
                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/doctest>
    )
    add_library(doctest::doctest ALIAS doctest)

    install(DIRECTORY ${doctest_SOURCE_DIR}/doctest/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/doctest)
endif()
