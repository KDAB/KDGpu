# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
include(FetchContent)

# Note: FetchContent_MakeAvailable builds the project
# if it contains a CMakeLists.txt, otherwise it does nothing.
# ${package_SOURCE_DIR} ${package_BINARY_DIR} are made available by
# MakeAvailable or Populate
message(STATUS "Checking/updating dependencies. This may take a little while...
    Set the FETCHCONTENT_QUIET option to OFF to get verbose output.
"
)

# KDUtils
if(KDGPU_BUILD_TESTS OR KDGPU_BUILD_KDGPUKDGUI)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/kdutils.cmake)
endif()

# spdlog
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/spdlog.cmake)

# Vulkan & VMA
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/vulkan.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/vulkan_memory_allocator.cmake)

if(KDGPU_BUILD_EXAMPLES)
    # glm
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/glm.cmake)

    # stb (for stb image)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/stb.cmake)

    # imgui (for 2D UI)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/imgui.cmake)

    if(KDGPU_HLSL_SUPPORT)
        # dxc (for shader compilation)
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/dxc.cmake)
    endif()
endif()

if(KDGPU_OPENXR_SUPPORT)
    # OpenXR
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/openxr.cmake)
endif()

if(KDGPU_BUILD_TESTS)
    # doctest
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/doctest.cmake)
endif()
