# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
set(SOURCES view.cpp)

set(HEADERS view.h)

add_library(
    KDGpuKDGui
    ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)
add_library(
    KDGpu::KDGpuKDGui ALIAS KDGpuKDGui
)

set(KDGPU_KDGUI_PUBLIC_LIBS KDGpu::KDGpu KDUtils::KDGui)

set(KDGPU_KDGUI_EXPORT_TARGETS KDGpuKDGui)

target_link_libraries(
    KDGpuKDGui
    PUBLIC ${KDGPU_KDGUI_PUBLIC_LIBS}
)

target_include_directories(
    KDGpuKDGui
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include>
)

set_target_properties(
    KDGpuKDGui
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(KDGpuKDGui PRIVATE view.mm)
endif()

include(GenerateExportHeader)
generate_export_header(KDGpuKDGui BASE_NAME kdgpukdgui)
configure_file(
    ${CMAKE_CURRENT_BINARY_DIR}/kdgpukdgui_export.h ${CMAKE_BINARY_DIR}/include/KDGpuKDGui/kdgpukdgui_export.h
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgpukdgui_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpuKDGui
)

add_feature_info(KDGpuKDGui ON "Build KDGpuKDGui Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpuKDGui/${dir})
endforeach()

install(
    TARGETS ${KDGPU_KDGUI_EXPORT_TARGETS}
    EXPORT KDGpuKDGui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDGpuKDGui
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuKDGui
    NAMESPACE KDGpu::
    FILE KDGpuKDGuiTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDGpuKDGuiConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KDGpuKDGuiConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuKDGui
)

write_basic_package_version_file(
    KDGpuKDGuiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KDGpuKDGuiConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDGpuKDGuiConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuKDGui
)
