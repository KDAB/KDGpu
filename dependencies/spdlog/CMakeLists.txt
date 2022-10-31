find_package(spdlog 1.8.5 QUIET)
if (NOT TARGET spdlog::spdlog)
    get_property(tmp GLOBAL PROPERTY PACKAGES_NOT_FOUND)
    list(FILTER tmp EXCLUDE REGEX spdlog)
    set_property(GLOBAL PROPERTY PACKAGES_NOT_FOUND ${tmp})

    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.8.5
        )
    set(SPDLOG_INSTALL ON CACHE BOOL "Install spdlog" FORCE)
    FetchContent_MakeAvailable(spdlog)

    set_target_properties(spdlog
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
