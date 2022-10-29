find_package(Vulkan REQUIRED)

set(SOURCES
    adapter.cpp
    device.cpp
    graphics_api.cpp
    instance.cpp
    resource_manager.cpp
    queue.cpp
    vulkan/vulkan_adapter.cpp
    vulkan/vulkan_device.cpp
    vulkan/vulkan_enums.cpp
    vulkan/vulkan_graphics_api.cpp
    vulkan/vulkan_instance.cpp
    vulkan/vulkan_resource_manager.cpp
    vulkan/vulkan_queue.cpp
)

set(HEADERS
    adapter_features.h
    adapter_properties.h
    adapter_queue_type.h
    adapter.h
    device.h
    device_options.h
    graphics_api.h
    gpu_core.h
    instance.h
    bind_group_description.h
    handle.h
    queue.h
    queue_description.h
    resource_manager.h
    api/api_adapter.h
    api/api_device.h
    api/api_instance.h
    vulkan/vulkan_adapter.h
    vulkan/vulkan_device.h
    vulkan/vulkan_enums.h
    vulkan/vulkan_graphics_api.h
    vulkan/vulkan_instance.h
    vulkan/vulkan_resource_manager.h
    vulkan/vulkan_queue.h
)

add_library(toy-renderer SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(Toy::Renderer ALIAS toy-renderer)

set(TOY_RENDERER_GUI_PUBLIC_LIBS
    Vulkan::Vulkan
    spdlog::spdlog

    # Serenity::Core
    # Serenity::Gui
)

target_link_libraries(toy-renderer
    PUBLIC
    ${TOY_RENDERER_GUI_PUBLIC_LIBS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(toy-renderer PRIVATE PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(toy-renderer PRIVATE PLATFORM_WIN32)
    target_compile_definitions(toy-renderer PRIVATE UNICODE)
    target_compile_definitions(toy-renderer PRIVATE "VMA_CALL_PRE=__declspec(dllexport)")
    target_compile_definitions(toy-renderer PRIVATE "IMGUI_API=__declspec(dllexport)")
    target_compile_definitions(toy-renderer PUBLIC _USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(toy-renderer PRIVATE PLATFORM_MACOS)
endif()

# Require >=C++20 for us and downstream projects
target_compile_features(toy-renderer PUBLIC cxx_std_20)

target_include_directories(toy-renderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(toy-renderer
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(toy-renderer BASE_NAME toy_renderer)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/toy_renderer_export.h ${CMAKE_BINARY_DIR}/include/toy_renderer/toy_renderer_export.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/toy_renderer_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer)

add_feature_info(Toy-Renderer ON "Build Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer/${dir})
endforeach()
