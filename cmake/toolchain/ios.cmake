# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
# Usage:
# > cmake -G (Xcode|Ninja) --toolchain .../cmake/toolchain/ios.cmake [-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=XXXX] ...

set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_IOS_INSTALL_COMBINED YES)

set(BUILD_SHARED_LIBS OFF)
# Set the CMP0042 policy to NEW to make libraries default to static
cmake_policy(SET CMP0042 NEW)

option(KDGPU_BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(KDGPU_BUILD_TESTS "Build tests" OFF)
option(KDGPU_BUILD_KDGPUKDGUI "Build KDGpuKDGui" OFF)
option(KDGPU_BUILD_ASSETS "Build Assets" OFF)
