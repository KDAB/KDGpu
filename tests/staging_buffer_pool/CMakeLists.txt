# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    staging-buffer-pool
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_utils_test(${PROJECT_NAME} tst_staging_buffer_pool.cpp)
