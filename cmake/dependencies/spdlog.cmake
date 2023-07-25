# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# spdlog Logging Library
# spdlog needs to be installed. If already exists in the prefix,
# we don't want to override it, so first we try to find it.
# If we don't find it, then we fetch it and install it
find_package(spdlog 1.11.0 QUIET)

if(NOT TARGET spdlog::spdlog)
    get_property(tmp GLOBAL PROPERTY PACKAGES_NOT_FOUND)
    list(
        FILTER
        tmp
        EXCLUDE
        REGEX
        spdlog
    )
    set_property(GLOBAL PROPERTY PACKAGES_NOT_FOUND ${tmp})

    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.11.0
    )
    set(SPDLOG_INSTALL
        ON
        CACHE BOOL "Install spdlog" FORCE
    )
    FetchContent_MakeAvailable(spdlog)

    set_target_properties(
        spdlog
        PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                   LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
