# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(Stb REQUIRED)

if(NOT TARGET stb::stb)

    add_library(stb INTERFACE)
    target_include_directories(
        stb INTERFACE $<BUILD_INTERFACE:${Stb_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/stb>
    )
    add_library(stb::stb ALIAS stb)

    # Install only STB headers (not the entire vcpkg include directory)
    file(GLOB STB_HEADERS "${Stb_INCLUDE_DIR}/stb_*.h")
    if(STB_HEADERS)
        install(FILES ${STB_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stb)
    endif()

    # Install the target so it's available when this project is consumed
    install(TARGETS stb EXPORT StbTargets)
    install(
        EXPORT StbTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/stb
        NAMESPACE stb::
    )

endif()
