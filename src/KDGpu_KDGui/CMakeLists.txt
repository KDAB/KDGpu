find_package(Vulkan REQUIRED)

set(SOURCES
    advanced_example_engine_layer.cpp
    engine.cpp
    engine_layer.cpp
    example_engine_layer.cpp
    imgui_input_handler.cpp
    imgui_item.cpp
    imgui_renderer.cpp
    simple_example_engine_layer.cpp
    view.cpp
)

set(HEADERS
    advanced_example_engine_layer.h
    engine.h
    engine_layer.h
    example_engine_layer.h
    imgui_input_handler.h
    imgui_item.h
    imgui_renderer.h
    simple_example_engine_layer.h
    view.h
)

add_library(KDGpu_KDGui SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(KDGpu::KDGpu_KDGui ALIAS KDGpu_KDGui)

set(KDGPU_KDGUI_PUBLIC_LIBS
    KDGpu::KDGpu
    KDUtils::KDGui
    KDGpuKDGui::imgui
)

target_compile_definitions(KDGpu_KDGui PRIVATE KDGPU_KDGUI_ASSET_PATH="${CMAKE_BINARY_DIR}/assets")

set(
    KDGPU_KDGUI_EXPORT_TARGETS
    KDGpu_KDGui
)

target_link_libraries(KDGpu_KDGui
    PUBLIC
    ${KDGPU_KDGUI_PUBLIC_LIBS}
)

target_include_directories(KDGpu_KDGui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(KDGpu_KDGui
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(KDGpu_KDGui PRIVATE view.mm)
endif()

include(GenerateExportHeader)
generate_export_header(KDGpu_KDGui BASE_NAME kdgpu_kdgui)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdgpu_kdgui_export.h ${CMAKE_BINARY_DIR}/include/KDGpu_KDGui/kdgpu_kdgui_export.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgpu_kdgui_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu_KDGui)

add_feature_info(KDGpu_KDGui ON "Build KDGpu_KDGui Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu_KDGui/${dir})
endforeach()

install(
    TARGETS ${KDGPU_KDGUI_EXPORT_TARGETS}
    EXPORT KDGpu_KDGui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT KDGpu_KDGui DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu_KDGui NAMESPACE KDGpu:: FILE KDGpu_KDGuiTargets.cmake)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDGpu_KDGuiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KDGpu_KDGuiConfig.cmake
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu_KDGui
)

write_basic_package_version_file(KDGpu_KDGuiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KDGpu_KDGuiConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KDGpu_KDGuiConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu_KDGui
)
