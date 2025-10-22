# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

if(NOT TARGET Vulkan::Vulkan)
    find_package(Vulkan REQUIRED)
endif()

set(SOURCES
    acceleration_structure.cpp
    adapter.cpp
    buffer.cpp
    bind_group.cpp
    bind_group_layout.cpp
    bind_group_pool.cpp
    command_buffer.cpp
    command_recorder.cpp
    compute_pipeline.cpp
    compute_pass_command_recorder.cpp
    device.cpp
    fence.cpp
    graphics_pipeline.cpp
    gpu_semaphore.cpp
    instance.cpp
    pipeline_layout.cpp
    queue.cpp
    raytracing_pass_command_recorder.cpp
    raytracing_pipeline.cpp
    raytracing_shader_binding_table.cpp
    render_pass_command_recorder.cpp
    render_pass.cpp
    sampler.cpp
    shader_module.cpp
    swapchain.cpp
    surface.cpp
    texture.cpp
    texture_view.cpp
    timestamp_query_recorder.cpp
    ycbcr_conversion.cpp
    utils/logging.cpp
    vulkan/vulkan_acceleration_structure.cpp
    vulkan/vulkan_adapter.cpp
    vulkan/vulkan_bind_group.cpp
    vulkan/vulkan_bind_group_layout.cpp
    vulkan/vulkan_bind_group_pool.cpp
    vulkan/vulkan_buffer.cpp
    vulkan/vulkan_command_buffer.cpp
    vulkan/vulkan_command_recorder.cpp
    vulkan/vulkan_compute_pass_command_recorder.cpp
    vulkan/vulkan_compute_pipeline.cpp
    vulkan/vulkan_config.cpp
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
    vulkan/vulkan_raytracing_pass_command_recorder.cpp
    vulkan/vulkan_raytracing_pipeline.cpp
    vulkan/vulkan_render_pass.cpp
    vulkan/vulkan_render_pass_command_recorder.cpp
    vulkan/vulkan_resource_manager.cpp
    vulkan/vulkan_sampler.cpp
    vulkan/vulkan_shader_module.cpp
    vulkan/vulkan_swapchain.cpp
    vulkan/vulkan_surface.cpp
    vulkan/vulkan_texture.cpp
    vulkan/vulkan_texture_view.cpp
    vulkan/vulkan_timestamp_query_recorder.cpp
    vulkan/vulkan_ycbcr_conversion.cpp
    vulkan/vk_mem_alloc.cpp
)

set(HEADERS
    acceleration_structure_options.h
    acceleration_structure.h
    adapter_features.h
    adapter_group.h
    adapter_properties.h
    adapter_queue_type.h
    adapter_swapchain_properties.h
    adapter.h
    bind_group.h
    bind_group_options.h
    bind_group_description.h
    bind_group_layout.h
    bind_group_layout_options.h
    bind_group_pool.h
    bind_group_pool_options.h
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
    raytracing_pass_command_recorder.h
    raytracing_pipeline.h
    raytracing_pipeline_options.h
    raytracing_shader_binding_table.h
    render_pass_command_recorder.h
    render_pass_command_recorder_options.h
    render_pass.h
    render_pass_options.h
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
    timestamp_query_recorder.h
    timestamp_query_recorder_options.h
    ycbcr_conversion.h
    ycbcr_conversion_options.h
    api/api_type.h
    utils/flags.h
    utils/formatters.h
    utils/hash_utils.h
    utils/logging.h
    vulkan/vulkan_acceleration_structure.h
    vulkan/vulkan_adapter.h
    vulkan/vulkan_bind_group.h
    vulkan/vulkan_bind_group_layout.h
    vulkan/vulkan_bind_group_pool.h
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
    vulkan/vulkan_raytracing_pass_command_recorder.h
    vulkan/vulkan_raytracing_pipeline.h
    vulkan/vulkan_render_pass.h
    vulkan/vulkan_render_pass_command_recorder.h
    vulkan/vulkan_resource_manager.h
    vulkan/vulkan_sampler.h
    vulkan/vulkan_shader_module.h
    vulkan/vulkan_swapchain.h
    vulkan/vulkan_surface.h
    vulkan/vulkan_texture.h
    vulkan/vulkan_texture_view.h
    vulkan/vulkan_timestamp_query_recorder.h
    vulkan/vulkan_ycbcr_conversion.h
)

add_library(
    KDGpu
    ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)
add_library(
    KDGpu::KDGpu ALIAS KDGpu
)

set(KDGPU_PUBLIC_LIBS spdlog::spdlog Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)

set(KDGPU_EXPORT_TARGETS KDGpu)

target_link_libraries(
    KDGpu
    PUBLIC ${KDGPU_PUBLIC_LIBS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(KDGPU_PLATFORM_LINUX ON)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(KDGPU_PLATFORM_WIN32 ON)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_WIN32)
    target_compile_definitions(KDGpu PRIVATE UNICODE)
    target_compile_definitions(KDGpu PRIVATE "VMA_CALL_PRE=__declspec(dllexport)")
    target_compile_definitions(KDGpu PRIVATE "IMGUI_API=__declspec(dllexport)")
    target_compile_definitions(KDGpu PUBLIC _USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(KDGPU_PLATFORM_APPLE ON)
    set(KDGPU_PLATFORM_MACOS ON)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_APPLE PLATFORM_MACOS)
    target_sources(KDGpu PRIVATE vulkan/vulkan_utils.mm)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(KDGPU_PLATFORM_APPLE ON)
    set(KDGPU_PLATFORM_IOS ON)
    target_compile_definitions(KDGpu PRIVATE PLATFORM_APPLE PLATFORM_IOS)
    target_sources(KDGpu PRIVATE vulkan/vulkan_utils.mm)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(KDGPU_PLATFORM_ANDROID ON)
    target_compile_definitions(KDGpu PUBLIC VK_USE_PLATFORM_ANDROID_KHR=1)
endif()

# Require >=C++20 for us and downstream projects
target_compile_features(KDGpu PUBLIC cxx_std_20)

target_include_directories(
    KDGpu
    PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
           $<INSTALL_INTERFACE:include>
)

set_target_properties(
    KDGpu
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
               VERSION ${PROJECT_VERSION}
               SOVERSION ${PROJECT_VERSION}
)

include(GenerateExportHeader)
generate_export_header(KDGpu BASE_NAME kdgpu)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdgpu_export.h ${CMAKE_BINARY_DIR}/include/KDGpu/kdgpu_export.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgpu_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h ${CMAKE_BINARY_DIR}/include/KDGpu/config.h)
install(
    FILES ${CMAKE_BINARY_DIR}/include/KDGpu/config.h
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
          ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/KDGpuCompileShader.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpu
)
