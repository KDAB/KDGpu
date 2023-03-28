include(FetchContent)

# Note: FetchContent_MakeAvailable builds the project
# if it contains a CMakeLists.txt, otherwise it does nothing.
# ${package_SOURCE_DIR} ${package_BINARY_DIR} are made available by
# MakeAvailable or Populate
message(STATUS "Checking/updating dependencies. This may take a little while...
    Set the FETCHCONTENT_QUIET option to OFF to get verbose output.
")

# KDUtils
include(cmake/dependencies/kdutils.cmake)

# VMA
include(cmake/dependencies/vulkan_memory_allocator.cmake)

# glm
include(cmake/dependencies/glm.cmake)

# stb (for stb image)
include(cmake/dependencies/stb.cmake)

if(TOYRENDERER_BUILD_TESTS)
    # doctest
    include(cmake/dependencies/doctest.cmake)

    # trompeloeil
    include(cmake/dependencies/trompeloeil.cmake)
endif()
