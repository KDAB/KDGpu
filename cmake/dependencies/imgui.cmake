# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(imgui QUIET)

if(NOT TARGET imgui::imgui)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9
    )
    FetchContent_MakeAvailable(imgui)

    # imgui doesn't provide a CMakeLists.txt, we have to add sources manually
    # Note: FetchContent_MakeAvailable provides ${imgui_SOURCE_DIR}
    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
    )

    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(
        imgui PUBLIC $<BUILD_INTERFACE:${imgui_SOURCE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/imgui>
    )
    set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON CXX_STANDARD 11)
    add_library(imgui::imgui ALIAS imgui)

    install(DIRECTORY ${imgui_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imgui)

    # Create CMake Package File for imgui so that it can be found with find_package(imgui)
    # and exports target imgui::imgui to link against
    install(
        TARGETS imgui
        EXPORT imguiConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    export(
        TARGETS imgui
        NAMESPACE imgui::
        FILE "${imgui_BINARY_DIR}/imguiConfig.cmake"
    )

    install(
        EXPORT imguiConfig
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imgui
        EXPORT_LINK_INTERFACE_LIBRARIES
        NAMESPACE imgui::
    )
endif()
