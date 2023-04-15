if(NOT TARGET KDGpuKDGui::imgui)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.5
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
    target_include_directories(imgui PUBLIC
        $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/KDGpu_KDGui/imgui>
    )
    set_target_properties(imgui PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_STANDARD 11
    )
    add_library(KDGpuKDGui::imgui ALIAS imgui)

    install(DIRECTORY ${imgui_SOURCE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu_KDGui/imgui)
endif()
