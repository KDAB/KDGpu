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
    set(KDUTILS_BUILD_TESTS OFF)
    set(KDUTILS_BUILD_EXAMPLES OFF)
    set(KDUTILS_BUILD_MQTT_SUPPORT OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/dependencies/kdutils/)
elseif(NOT TARGET KDUtils)
    find_package(KDUtils CONFIG)

    if(NOT KDUtils_FOUND)
        set(KDUTILS_BUILD_TESTS OFF)
        set(KDUTILS_BUILD_EXAMPLES OFF)
        set(KDUTILS_BUILD_MQTT_SUPPORT OFF)
        FetchContent_Declare(
            KDUtils
            GIT_REPOSITORY https://github.com/KDAB/KDUtils.git
            GIT_TAG d90d6894d97847ada0cbf4cd50dc0166dc81ebc7
            USES_TERMINAL_DOWNLOAD YES USES_TERMINAL_UPDATE YES
        )

        FetchContent_MakeAvailable(KDUtils)
    endif()

    find_package(KDFoundation CONFIG)
    find_package(KDGui CONFIG)
endif()
