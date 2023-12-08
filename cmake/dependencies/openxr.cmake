# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
# OpenXR SDK
if(NOT TARGET openxr_loader)
    set(BUILD_TESTS
        OFF
        CACHE INTERNAL "Build tests"
    )
    set(BUILD_API_LAYERS
        ON
        CACHE INTERNAL "Use OpenXR layers"
    )
    FetchContent_Declare(
        OpenXR
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK-Source.git
        GIT_TAG release-1.0.32.1
        USES_TERMINAL_DOWNLOAD YES USES_TERMINAL_UPDATE YES
    )
    FetchContent_MakeAvailable(OpenXR)
endif()
