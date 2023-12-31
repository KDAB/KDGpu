# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(MOLTENVK_HOME $ENV{VULKAN_HOME}/MoltenVK)
    message("Using Vulkan for iOS in '${MOLTENVK_HOME}'")
    add_library(Vulkan INTERFACE)
    target_link_libraries(Vulkan INTERFACE "-L${MOLTENVK_HOME}/MoltenVK.xcframework/ios-arm64" "-lMoltenVK")
    target_include_directories(Vulkan INTERFACE $ENV{VULKAN_HOME}/macOS/include)
    add_library(Vulkan::Vulkan ALIAS Vulkan)
    install(FILES $ENV{VULKAN_HOME}/macOS/include/vulkan/vulkan.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGpu/vulkan/)
    install(
        TARGETS Vulkan
        EXPORT VulkanConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    export(
        TARGETS Vulkan
        NAMESPACE Vulkan::
        FILE "${CMAKE_INSTALL_LIBDIR}/cmake/VulkanConfig.cmake"
    )

    install(
        EXPORT VulkanConfig
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Vulkan
        EXPORT_LINK_INTERFACE_LIBRARIES
        NAMESPACE Vulkan::
    )
else()
    find_package(Vulkan REQUIRED)
endif()
