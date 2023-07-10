# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/kdutils.cmake)

# VMA
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/vulkan_memory_allocator.cmake)

# glm
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/glm.cmake)

# stb (for stb image)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/stb.cmake)

# imgui (for 2D UI)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/imgui.cmake)

# dxc (for shader compilation)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/dxc.cmake)

if(TOYRENDERER_BUILD_TESTS)
    # doctest
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/doctest.cmake)

    # trompeloeil
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/trompeloeil.cmake)
endif()
