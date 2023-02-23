find_package(Vulkan REQUIRED)

set(SOURCES
    view.cpp
)

set(HEADERS
    view.h
)

add_library(toy-renderer-serenity SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(Toy::RendererSerenity ALIAS toy-renderer-serenity)

set(TOY_RENDERER_SERENITY_PUBLIC_LIBS
    Toy::Renderer
    KDUtils::Gui
)

target_link_libraries(toy-renderer-serenity
    PUBLIC
    ${TOY_RENDERER_SERENITY_PUBLIC_LIBS}
)

target_include_directories(toy-renderer-serenity PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(toy-renderer-serenity
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(toy-renderer-serenity BASE_NAME toy_renderer_serenity)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/toy_renderer_serenity_export.h ${CMAKE_BINARY_DIR}/include/toy_renderer_serenity/toy_renderer_serenity_export.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/toy_renderer_serenity_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer_serenity)

add_feature_info(Toy-Renderer-Serenity ON "Build Toy-Renderer-Serenity Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer_serenity/${dir})
endforeach()
