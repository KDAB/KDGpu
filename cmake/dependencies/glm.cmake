# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(glm QUIET)

if(NOT TARGET glm::glm AND TARGET glm)
    add_library(glm::glm ALIAS glm)
endif()

if(NOT TARGET glm::glm)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )

    option(GLM_BUILD_LIBRARY "Build dynamic/static library" OFF)

    if(NOT glm_POPULATED)
        FetchContent_Populate(glm)

        # Note: internally glm does
        # add_library(glm INTERFACE)
        # add_library(glm::glm ALIAS glm)
        add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
    endif()

    # Work-around for:
    # CMake Error in android/build/_deps/glm-src/glm/CMakeLists.txt:
    # Target "glm" INTERFACE_INCLUDE_DIRECTORIES property contains path:
    #
    # "/home/kfunk/devel/src/kdab/serenity/android/build/_deps/glm-src/glm/../"
    set_target_properties(glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")

    target_include_directories(
        glm INTERFACE $<BUILD_INTERFACE:${glm_SOURCE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/glm>
    )

    install(DIRECTORY ${glm_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glm)

    # Create CMake Package File for glm so that it can be found with find_package(glm)
    # and exports glm with namespace glm:: to link against glm::glm
    install(
        TARGETS glm glm-header-only
        EXPORT glmConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    export(
        TARGETS glm glm-header-only
        NAMESPACE glm::
        FILE "${glm_BINARY_DIR}/glmConfig.cmake"
    )

    install(
        EXPORT glmConfig
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/glm
        EXPORT_LINK_INTERFACE_LIBRARIES
        NAMESPACE glm::
    )
endif()
