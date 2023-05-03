if(NOT TARGET stb::stb)
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(stb)

    add_library(stb INTERFACE)
    target_include_directories(
        stb SYSTEM INTERFACE $<BUILD_INTERFACE:${stb_SOURCE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/stb>
    )
    install(DIRECTORY ${stb_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stb)
    add_library(stb::stb ALIAS stb)
endif()
