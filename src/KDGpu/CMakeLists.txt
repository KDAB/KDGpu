# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
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
    pool.h
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
    api/api_queue.h
    api/api_render_pass.h
    api/api_render_pass_command_recorder.h
    api/api_sampler.h
    api/api_shader_module.h
    api/api_swapchain.h
    api/api_surface.h
    api/api_texture.h
    api/api_texture_view.h
    utils/formatters.h
    utils/hash_utils.h
    utils/logging.h
    vulkan/vulkan_adapter.h
    vulkan/vulkan_bind_group.h
    vulkan/vulkan_bind_group_layout.h
    vulkan/vulkan_buffer.h
    vulkan/vulkan_command_buffer.h
    vulkan/vulkan_command_recorder.h
    vulkan/vulkan_compute_pass_command_recorder.h
    vulkan/vulkan_compute_pipeline.h
    vulkan/vulkan_config.h
    vulkan/vulkan_device.h
    vulkan/vulkan_enums.h
    vulkan/vulkan_fence.h
    vulkan/vulkan_formatters.h
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

add_library(
    KDGpu SHARED
    ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)
add_library(
    KDGpu::KDGpu ALIAS KDGpu
)

set(KDGPU_PUBLIC_LIBS KDUtils::KDUtils Vulkan::Vulkan KDGpu::vulkan_memory_allocator)

set(KDGPU_EXPORT_TARGETS KDGpu vulkan_memory_allocator)

target_link_libraries(
    KDGpu
    PUBLIC ${KDGPU_PUBLIC_LIBS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(KDGpu PUBLIC KDGPU_PLATFORM_LINUX)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(KDGpu PUBLIC KDGPU_PLATFORM_WIN32)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_WIN32)
    target_compile_definitions(KDGpu PRIVATE UNICODE)
    target_compile_definitions(KDGpu PRIVATE "VMA_CALL_PRE=__declspec(dllexport)")
    target_compile_definitions(KDGpu PRIVATE "IMGUI_API=__declspec(dllexport)")
    target_compile_definitions(KDGpu PUBLIC _USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(KDGpu PUBLIC KDGPU_PLATFORM_MACOS)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_MACOS)
    target_sources(KDGpu PRIVATE vulkan/vulkan_utils.mm)
endif()

# Require >=C++20 for us and downstream projects
target_compile_features(KDGpu PUBLIC cxx_std_20)

target_include_directories(
    KDGpu
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include>
)

set_target_properties(
    KDGpu
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(KDGpu BASE_NAME kdgpu)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdgpu_export.h ${CMAKE_BINARY_DIR}/include/KDGpu/kdgpu_export.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgpu_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu
)

add_feature_info(KDGpu ON "Build Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu/${dir})
endforeach()

install(
    TARGETS ${KDGPU_EXPORT_TARGETS}
    EXPORT KDGpu
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDGpu
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu
    NAMESPACE KDGpu::
    FILE KDGpuTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDGpuConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KDGpuConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu
)

write_basic_package_version_file(
    KDGpuConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KDGpuConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDGpuConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu
)
