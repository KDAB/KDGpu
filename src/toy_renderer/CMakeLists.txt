find_package(Vulkan REQUIRED)

set(SOURCES
    adapter.cpp
    buffer.cpp
    bind_group.cpp
    bind_group_layout.cpp
    command_buffer.cpp
    command_recorder.cpp
    compute_pipeline.cpp
    compute_pass_command_recorder.cpp
    device.cpp
    fence.cpp
    graphics_api.cpp
    graphics_pipeline.cpp
    gpu_semaphore.cpp
    instance.cpp
    pipeline_layout.cpp
    queue.cpp
    render_pass_command_recorder.cpp
    resource_manager.cpp
    sampler.cpp
    shader_module.cpp
    swapchain.cpp
    surface.cpp
    texture.cpp
    texture_view.cpp
    vulkan/vulkan_adapter.cpp
    vulkan/vulkan_bind_group.cpp
    vulkan/vulkan_bind_group_layout.cpp
    vulkan/vulkan_buffer.cpp
    vulkan/vulkan_command_buffer.cpp
    vulkan/vulkan_command_recorder.cpp
    vulkan/vulkan_compute_pass_command_recorder.cpp
    vulkan/vulkan_compute_pipeline.cpp
    vulkan/vulkan_device.cpp
    vulkan/vulkan_enums.cpp
    vulkan/vulkan_fence.cpp
    vulkan/vulkan_framebuffer.cpp
    vulkan/vulkan_gpu_semaphore.cpp
    vulkan/vulkan_graphics_api.cpp
    vulkan/vulkan_graphics_pipeline.cpp
    vulkan/vulkan_instance.cpp
    vulkan/vulkan_pipeline_layout.cpp
    vulkan/vulkan_queue.cpp
    vulkan/vulkan_render_pass.cpp
    vulkan/vulkan_render_pass_command_recorder.cpp
    vulkan/vulkan_resource_manager.cpp
    vulkan/vulkan_sampler.cpp
    vulkan/vulkan_shader_module.cpp
    vulkan/vulkan_swapchain.cpp
    vulkan/vulkan_surface.cpp
    vulkan/vulkan_texture.cpp
    vulkan/vulkan_texture_view.cpp
)

set(HEADERS
    adapter_features.h
    adapter_properties.h
    adapter_queue_type.h
    adapter_swapchain_properties.h
    adapter.h
    bind_group.h
    bind_group_options.h
    bind_group_description.h
    bind_group_layout.h
    bind_group_layout_options.h
    buffer.h
    buffer_options.h
    command_buffer.h
    command_recorder.h
    compute_pipeline.h
    compute_pipeline_options.h
    compute_pass_command_recorder.h
    device.h
    device_options.h
    fence.h
    graphics_api.h
    graphics_pipeline.h
    graphics_pipeline_options.h
    gpu_core.h
    gpu_semaphore.h
    instance.h
    handle.h
    memory_barrier.h
    pipeline_layout.h
    pipeline_layout_options.h
    queue.h
    queue_description.h
    render_pass_command_recorder.h
    render_pass_command_recorder_options.h
    resource_manager.h
    sampler.h
    sampler_options.h
    shader_module.h
    swapchain.h
    swapchain_options.h
    surface.h
    surface_options.h
    texture.h
    texture_options.h
    texture_view.h
    texture_view_options.h
    api/api_adapter.h
    api/api_bind_group.h
    api/api_bind_group_layout.h
    api/api_buffer.h
    api/api_command_buffer.h
    api/api_command_recorder.h
    api/api_compute_pipeline.h
    api/api_compute_pass_command_recorder.h
    api/api_device.h
    api/api_fence.h
    api/api_framebuffer.h
    api/api_gpu_semaphore.h
    api/api_graphics_pipeline.h
    api/api_instance.h
    api/api_pipeline_layout.h
    api/api_render_pass.h
    api/api_render_pass_command_recorder.h
    api/api_sampler.h
    api/api_shader_module.h
    api/api_swapchain.h
    api/api_surface.h
    api/api_texture.h
    api/api_texture_view.h
    utils/hash_utils.h
    vulkan/vulkan_adapter.h
    vulkan/vulkan_bind_group.h
    vulkan/vulkan_bind_group_layout.h
    vulkan/vulkan_buffer.h
    vulkan/vulkan_command_buffer.h
    vulkan/vulkan_command_recorder.h
    vulkan/vulkan_compute_pass_command_recorder.h
    vulkan/vulkan_compute_pipeline.h
    vulkan/vulkan_device.h
    vulkan/vulkan_enums.h
    vulkan/vulkan_fence.h
    vulkan/vulkan_framebuffer.h
    vulkan/vulkan_gpu_semaphore.h
    vulkan/vulkan_graphics_api.h
    vulkan/vulkan_graphics_pipeline.h
    vulkan/vulkan_instance.h
    vulkan/vulkan_pipeline_layout.h
    vulkan/vulkan_queue.h
    vulkan/vulkan_render_pass.h
    vulkan/vulkan_render_pass_command_recorder.h
    vulkan/vulkan_resource_manager.h
    vulkan/vulkan_sampler.h
    vulkan/vulkan_shader_module.h
    vulkan/vulkan_swapchain.h
    vulkan/vulkan_surface.h
    vulkan/vulkan_texture.h
    vulkan/vulkan_texture_view.h
)

add_library(toy-renderer SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(ToyRenderer::ToyRenderer ALIAS toy-renderer)

set(TOY_RENDERER_PUBLIC_LIBS
    Vulkan::Vulkan
    ToyRenderer::vulkan_memory_allocator
    spdlog::spdlog
)

target_link_libraries(toy-renderer
    PUBLIC
    ${TOY_RENDERER_PUBLIC_LIBS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(toy-renderer PUBLIC TOY_RENDERER_PLATFORM_LINUX)
    target_compile_definitions(toy-renderer PRIVATE PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(toy-renderer PUBLIC TOY_RENDERER_PLATFORM_WIN32)
    target_compile_definitions(toy-renderer PRIVATE PLATFORM_WIN32)
    target_compile_definitions(toy-renderer PRIVATE UNICODE)
    target_compile_definitions(toy-renderer PRIVATE "VMA_CALL_PRE=__declspec(dllexport)")
    target_compile_definitions(toy-renderer PRIVATE "IMGUI_API=__declspec(dllexport)")
    target_compile_definitions(toy-renderer PUBLIC _USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(toy-renderer PUBLIC TOY_RENDERER_PLATFORM_MACOS)
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
