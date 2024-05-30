# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# spdlog Logging Library
# spdlog needs to be installed. If already exists in the prefix,
# we don't want to override it, so first we try to find it.
# If we don't find it, then we fetch it and install it
find_package(spdlog 1.14.1 QUIET)

if(NOT TARGET spdlog::spdlog)
    # We need to use external fmt because the one bundled with spldog 1.x throws
    # warnings in newer Visual Studio MSVC compiler versions.
    # See https://github.com/gabime/spdlog/issues/2912
    # TODO(spdlog2): external fmt can possibly be removed once splog 2.x is used
    # which bundles newer fmt version
    find_package(fmt 10.2.1 QUIET)
    if(NOT TARGET fmt)
        FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281 # 10.2.1
        )
        FetchContent_MakeAvailable(fmt)
    endif()
    set(SPDLOG_FMT_EXTERNAL_HO ON)
    # with this spdlog is included as a system library and won't e.g. trigger
    # linter warnings
    set(SPDLOG_SYSTEM_INCLUDES ON)

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
        GIT_TAG 27cb4c76708608465c413f6d0e6b8d99a4d84302 # v1.14.1
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
