# Serenity library
if(NOT TARGET Serenity::Core)
    FetchContent_Declare(
        Serenity
        GIT_REPOSITORY ssh://codereview.kdab.com:29418/kdab/serenity
        GIT_TAG master
    )

    set(SERENITY_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(SERENITY_BUILD_TESTS OFF CACHE INTERNAL "")
    set(SERENITY_FLUTTER_EMBEDDER OFF CACHE INTERNAL "")
    set(SERENITY_ENABLE_RMLUI OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(Serenity)
    set(Serenity_FOUND ON)

    # set_target_properties(Serenity
    # PROPERTIES
    # ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    # LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    # RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    # )
endif()
