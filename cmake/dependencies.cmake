# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# KDUtils
if(KDGPU_BUILD_TESTS OR KDGPU_BUILD_KDGPUKDGUI)
    find_package(KDUtils REQUIRED)
    find_package(KDFoundation REQUIRED)
    find_package(KDGui REQUIRED)
endif()

# spdlog
find_package(spdlog REQUIRED)

# Vulkan & VMA
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/vulkan.cmake)
find_package(VulkanMemoryAllocator REQUIRED)

if(KDGPU_BUILD_EXAMPLES)
    # glm
    find_package(glm CONFIG REQUIRED)

    # stb (for stb image)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/stb.cmake)

    # imgui (for 2D UI)
    find_package(imgui REQUIRED)

    if(KDGPU_HLSL_SUPPORT)
        # dxc (for shader compilation)
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/dxc.cmake)
    endif()
endif()

if(KDGPU_BUILD_KDXR)
    # OpenXR
    find_package(OpenXR REQUIRED)
endif()

if(KDGPU_BUILD_TESTS)
    # doctest
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/doctest.cmake)
endif()
