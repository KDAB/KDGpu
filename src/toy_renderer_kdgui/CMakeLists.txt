find_package(Vulkan REQUIRED)

set(SOURCES
    view.cpp
)

set(HEADERS
    view.h
)

add_library(toy-renderer-kdgui SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(Toy::RendererKDGui ALIAS toy-renderer-kdgui)

set(TOY_RENDERER_KDGUI_PUBLIC_LIBS
    ToyRenderer::ToyRenderer
    KDUtils::Gui
)

target_link_libraries(toy-renderer-kdgui
    PUBLIC
    ${TOY_RENDERER_KDGUI_PUBLIC_LIBS}
)

target_include_directories(toy-renderer-kdgui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(toy-renderer-kdgui
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(toy-renderer-kdgui BASE_NAME toy_renderer_kdgui)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/toy_renderer_kdgui_export.h ${CMAKE_BINARY_DIR}/include/toy_renderer_kdgui/toy_renderer_kdgui_export.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/toy_renderer_kdgui_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer_kdgui)

add_feature_info(Toy-Renderer-Kdgui ON "Build Toy-Renderer-Kdgui Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer_kdgui/${dir})
endforeach()
