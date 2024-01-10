# This file is part of KDXr.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
set(SOURCES
    instance.cpp
    openxr/openxr_api.cpp
    openxr/openxr_enums.cpp
    openxr/openxr_instance.cpp
    openxr/openxr_resource_manager.cpp
    openxr/openxr_system.cpp
    resource_manager.cpp
    system.cpp
    utils/logging.cpp
    xr_api.cpp
)

set(HEADERS
    api/api_instance.h
    api/api_system.h
    handle.h
    instance.h
    openxr/openxr_api.h
    openxr/openxr_enums.h
    openxr/openxr_instance.h
    openxr/openxr_resource_manager.h
    openxr/openxr_system.h
    pool.h
    resource_manager.h
    system.h
    utils/hash_utils.h
    utils/logging.h
    xr_api.h
)

add_library(
    KDXr
    ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS}
)
add_library(
    KDXr::KDXr ALIAS KDXr
)

set(KDXR_PUBLIC_LIBS KDGpu::KDGpu spdlog::spdlog OpenXR::openxr_loader)

set(KDXR_EXPORT_TARGETS KDXr)

target_link_libraries(
    KDXr
    PUBLIC ${KDXR_PUBLIC_LIBS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(KDXR_PLATFORM_LINUX ON)
    target_compile_definitions(KDXr PRIVATE PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(KDXR_PLATFORM_WIN32 ON)
    target_compile_definitions(KDXr PRIVATE PLATFORM_WIN32)
    target_compile_definitions(KDXr PRIVATE UNICODE)
    target_compile_definitions(KDXr PUBLIC _USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(KDXR_PLATFORM_MACOS ON)
    target_compile_definitions(KDXr PRIVATE PLATFORM_MACOS)
endif()

# Require >=C++20 for us and downstream projects
target_compile_features(KDXr PUBLIC cxx_std_20)

target_include_directories(
    KDXr
    PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
           $<INSTALL_INTERFACE:include>
)

set_target_properties(
    KDXr
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(KDXr BASE_NAME kdxr)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdxr_export.h ${CMAKE_BINARY_DIR}/include/KDXr/kdxr_export.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdxr_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDXr
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h ${CMAKE_BINARY_DIR}/include/KDXr/config.h)
install(
    FILES ${CMAKE_BINARY_DIR}/include/KDXr/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDXr
)

add_feature_info(KDXr ON "Build KDXr Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDXr/${dir})
endforeach()

install(
    TARGETS ${KDXR_EXPORT_TARGETS}
    EXPORT KDXr
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDXr
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDXr
    NAMESPACE KDXr::
    FILE KDXrTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/KDXrConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KDXrConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDXr
)

write_basic_package_version_file(
    KDXrConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KDXrConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDXrConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDXr
)
