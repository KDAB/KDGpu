# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    test-fragmentation
    VERSION 0.1
    LANGUAGES CXX
)

set(TARGET_NAME test_kdgpu_${PROJECT_NAME})
add_executable(
    ${TARGET_NAME}
    tst_fragmentation.cpp
)

target_link_libraries(
    ${TARGET_NAME}
    KDGpu
    KDUtils::KDUtils
    doctest::doctest
    CUDA::cudart_static
)

add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpu")

if(APPLE)
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
endif()
