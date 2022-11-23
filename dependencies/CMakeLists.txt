include(FetchContent)

# Note: FetchContent_MakeAvailable builds the project
# if it contains a CMakeLists.txt, otherwise it does nothing.
# ${package_SOURCE_DIR} ${package_BINARY_DIR} are made available by
# MakeAvailable or Populate
message(STATUS "Checking/updating dependencies. This may take a little while...
    Set the FETCHCONTENT_QUIET option to OFF to get verbose output.
")

# spdlog Logging Library
add_subdirectory(spdlog)

# serenity
add_subdirectory(serenity)

# VMA
add_subdirectory(vulkan_memory_allocator)

if(TOYRENDERER_BUILD_TESTS)
    # doctest
    add_subdirectory(doctest)

    # trompeloeil
    add_subdirectory(trompeloeil)
endif()
