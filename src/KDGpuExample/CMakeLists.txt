# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# Create a static library for the needed shader resources. The raw GLSL shaders get compiled at
# build time and the resulting SPIR-V shaders are output to ${CMAKE_CURRENT_BINARY_DIR}. To handle
# this we must pass the WHENCE argument to cmrc_add_resource_library.
#
# We also enable C++17 for the resource static libs to cope with the nested namespaces we use.
include(CMakeRC)

CompileShaderSet(KDGpuExample_ImGui imgui)
cmrc_add_resource_library(
    KDGpuExampleShaderResources
    ALIAS
    KDGpuExample::ShaderResources
    WHENCE
    ${CMAKE_CURRENT_BINARY_DIR}
    NAMESPACE
    KDGpuExample::ShaderResources
    ${CMAKE_CURRENT_BINARY_DIR}/imgui.vert.spv
    ${CMAKE_CURRENT_BINARY_DIR}/imgui.frag.spv
)
target_compile_features(KDGpuExampleShaderResources PUBLIC cxx_std_17)

# As the other resources are just located in this directory and not the build dir, we must have
# a separate resource lib for these.
cmrc_add_resource_library(
    KDGpuExampleResources
    ALIAS
    KDGpuExample::Resources
    NAMESPACE
    KDGpuExample::Resources
    fonts/Roboto-Medium.ttf
)
target_compile_features(KDGpuExampleResources PUBLIC cxx_std_17)

find_package(Vulkan REQUIRED)

set(SOURCES
    advanced_example_engine_layer.cpp
    engine.cpp
    engine_layer.cpp
    example_engine_layer.cpp
    kdgpuexample.cpp
    imgui_input_handler.cpp
    imgui_item.cpp
    imgui_renderer.cpp
    simple_example_engine_layer.cpp
)

set(HEADERS
    advanced_example_engine_layer.h
    engine.h
    engine_layer.h
    example_engine_layer.h
    kdgpuexample.h
    imgui_input_handler.h
    imgui_item.h
    imgui_renderer.h
    simple_example_engine_layer.h
)

add_library(
    KDGpuExample
    ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)
add_library(
    KDGpu::KDGpuExample ALIAS KDGpuExample
)

set(KDGPUEXAMPLE_PUBLIC_LIBS
    KDGpu::KDGpu
    KDUtils::KDGui
    KDUtils::KDUtils
    imgui::imgui
    KDGpu::KDGpuKDGui
)

target_compile_definitions(KDGpuExample PRIVATE KDGPUEXAMPLE_ASSET_PATH="${CMAKE_INSTALL_FULL_DATADIR}/assets")

set(KDGPUEXAMPLE_EXPORT_TARGETS KDGpuExample)

target_link_libraries(
    KDGpuExample
    PRIVATE KDGpuExample::Resources KDGpuExample::ShaderResources
    PUBLIC ${KDGPUEXAMPLE_PUBLIC_LIBS}
)

target_include_directories(
    KDGpuExample
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include>
)

set_target_properties(
    KDGpuExample
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(KDGpuExample BASE_NAME kdgpuexample)
configure_file(
    ${CMAKE_CURRENT_BINARY_DIR}/kdgpuexample_export.h ${CMAKE_BINARY_DIR}/include/KDGpuExample/kdgpuexample_export.h
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgpuexample_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpuExample
)

add_feature_info(KDGpuExample ON "Build KDGpuExample Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpuExample/${dir})
endforeach()

install(
    TARGETS ${KDGPUEXAMPLE_EXPORT_TARGETS}
    EXPORT KDGpuExample
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDGpuExample
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuExample
    NAMESPACE KDGpu::
    FILE KDGpuExampleTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDGpuExampleConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KDGpuExampleConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuExample
)

write_basic_package_version_file(
    KDGpuExampleConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KDGpuExampleConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/KDGpuExampleConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuExample
)
