# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
set(SOURCES resource_deleter.cpp)

set(HEADERS resource_deleter.h staging_buffer_pool.h)

add_library(
    KDGpuUtils
    ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)
add_library(
    KDGpu::KDGpuUtils ALIAS KDGpuUtils
)

set(KDGPUUTILS_PUBLIC_LIBS KDGpu::KDGpu KDUtils::KDUtils)

set(KDGPUUTILS_EXPORT_TARGETS KDGpuUtils)

target_link_libraries(
    KDGpuUtils
    PUBLIC ${KDGPUUTILS_PUBLIC_LIBS}
)

target_include_directories(
    KDGpuUtils
    PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
           $<INSTALL_INTERFACE:include>
)

set_target_properties(
    KDGpuUtils
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
               VERSION ${PROJECT_VERSION}
               SOVERSION ${PROJECT_VERSION}
)

if(KDGPU_CODE_COVERAGE)
    append_coverage_compiler_flags()
endif()

include(GenerateExportHeader)
generate_export_header(KDGpuUtils BASE_NAME kdgpuutils)
configure_file(
    ${CMAKE_CURRENT_BINARY_DIR}/kdgpuutils_export.h ${CMAKE_BINARY_DIR}/include/KDGpuUtils/kdgpuutils_export.h
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgpuutils_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpuUtils
)

add_feature_info(KDGpuUtils ON "Build KDGpuUtils Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpuUtils/${dir})
endforeach()

install(
    TARGETS ${KDGPUUTILS_EXPORT_TARGETS}
    EXPORT KDGpuUtils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDGpuUtils
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuUtils
    NAMESPACE KDGpu::
    FILE KDGpuUtilsTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDGpuUtilsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KDGpuUtilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuUtils
)

write_basic_package_version_file(
    KDGpuUtilsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KDGpuUtilsConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDGpuUtilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGpuUtils
)
