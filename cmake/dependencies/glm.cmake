if(NOT TARGET glm::glm)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
    )
    FetchContent_Populate(glm)

    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE
        $<BUILD_INTERFACE:${glm_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/glm>
    )
    target_compile_definitions(glm INTERFACE
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
        GLM_LANG_STL11_FORCED
    )
    add_library(glm::glm ALIAS glm)

    install(DIRECTORY ${glm_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glm)
endif()
