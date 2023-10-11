# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Nicolas Guichard <nicolas.guichard@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(stb QUIET IMPORTED_TARGET stb)

    if(TARGET PkgConfig::stb)
        add_library(stb::stb ALIAS PkgConfig::stb)
        set(STB_FOUND TRUE)
    endif()
endif()

if(NOT STB_FOUND)
    find_path(
        STB_INCLUDE_DIR
        NAMES stb_image.h
        PATH_SUFFIXES stb
    )

    find_package_handle_standard_args(stb DEFAULT_MSG STB_INCLUDE_DIR)

    if(STB_FOUND)
        mark_as_advanced(STB_INCLUDE_DIR)

        add_library(stb INTERFACE)
        target_include_directories(stb SYSTEM INTERFACE ${STB_INCLUDE_DIR})
        add_library(stb::stb ALIAS stb)
    endif()
endif()
