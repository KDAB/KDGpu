# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
# KDUtils library
if(EXISTS "${CMAKE_SOURCE_DIR}/src/dependencies/kdutils")
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/dependencies/kdutils/)
elseif(NOT TARGET KDUtils)
    find_package(KDUtils CONFIG)

    if(NOT KDUtils_FOUND)
        FetchContent_Declare(
            KDUtils
            GIT_REPOSITORY https://github.com/KDAB/KDUtils.git
            GIT_TAG main
            USES_TERMINAL_DOWNLOAD YES USES_TERMINAL_UPDATE YES
        )

        option(KDUTILS_BUILD_TESTS "Build the tests" OFF)

        FetchContent_MakeAvailable(KDUtils)
    endif()

    find_package(KDFoundation CONFIG)
    find_package(KDGui CONFIG)
endif()
