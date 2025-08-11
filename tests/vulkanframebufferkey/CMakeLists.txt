# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    test-vulkanframebufferkey
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_test(${PROJECT_NAME} tst_vulkanframebufferkey.cpp)
