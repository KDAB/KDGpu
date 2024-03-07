# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
# OpenXR SDK
find_package(OpenXR QUIET)

if(NOT TARGET OpenXR::openxr_loader)
    set(BUILD_TESTS
        OFF
        CACHE INTERNAL "Build tests"
    )
    set(BUILD_API_LAYERS
        ON
        CACHE INTERNAL "Use OpenXR layers"
    )
    set(DYNAMIC_LOADER
        ON
        CACHE INTERNAL "Use dynamic loader"
    )
    FetchContent_Declare(
        OpenXR
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK-Source.git
        GIT_TAG release-1.1.43
        USES_TERMINAL_DOWNLOAD YES USES_TERMINAL_UPDATE YES
    )
    FetchContent_MakeAvailable(OpenXR)
endif()
