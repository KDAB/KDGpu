# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mkdocs/mkdocs.yml.cmake ${CMAKE_CURRENT_BINARY_DIR}/mkdocs/mkdocs.yml)

# Step 1: Run Doxygen
add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_DIR}/xml/index.xml
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Doxygen..."
)

add_custom_target(
    kdgpu-docs-doxygen
    DEPENDS ${DOXYGEN_OUTPUT_DIR}/xml/index.xml
    COMMENT "Build Doxygen documentation"
)

# Step 2: Copy files
add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_DIR}/mkdocs/.copied
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/mkdocs ${DOXYGEN_OUTPUT_DIR}/mkdocs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LICENSES/MIT.txt
            ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/license.md
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/README.md
            ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/index.md
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/INSTALL.md
            ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/INSTALL.md
    COMMAND ${CMAKE_COMMAND} -E touch ${DOXYGEN_OUTPUT_DIR}/mkdocs/.copied
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkdocs/mkdocs.yml
    COMMENT "Copying documentation files..."
)

add_custom_target(
    kdgpu-docs-copy
    DEPENDS ${DOXYGEN_OUTPUT_DIR}/mkdocs/.copied
    COMMENT "Copy documentation files"
)

# Step 3: Run Doxybook2
add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/.doxybook2_done
    COMMAND ${DOXYBOOK2_EXECUTABLE} --config "${CMAKE_CURRENT_SOURCE_DIR}/doxybook/config.json" --input
            "${DOXYGEN_OUTPUT_DIR}/xml" --output "${DOXYGEN_OUTPUT_DIR}/mkdocs/docs"
    COMMAND ${CMAKE_COMMAND} -E touch ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/.doxybook2_done
    DEPENDS ${DOXYGEN_OUTPUT_DIR}/xml/index.xml ${DOXYGEN_OUTPUT_DIR}/mkdocs/.copied
    COMMENT "Running Doxybook2..."
)

add_custom_target(
    kdgpu-docs-doxybook2
    DEPENDS ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/.doxybook2_done
    COMMENT "Run Doxybook2"
)

# Step 4: Run MkDocs
add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_DIR}/html/index.html
    COMMAND ${MKDOCS_EXECUTABLE} build --config-file "${DOXYGEN_OUTPUT_DIR}/mkdocs/mkdocs.yml" --site-dir
            "${DOXYGEN_OUTPUT_DIR}/html"
    DEPENDS ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/.doxybook2_done
    COMMENT "Running MkDocs..."
)

add_custom_target(
    kdgpu-docs-mkdocs
    DEPENDS ${DOXYGEN_OUTPUT_DIR}/html/index.html
    COMMENT "Run MkDocs"
)

# Main target that builds all steps
add_custom_target(
    kdgpu-docs ALL
    DEPENDS kdgpu-docs-mkdocs
    COMMENT "Build all documentation"
)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
    DESTINATION docs/api
    COMPONENT kdgpu-docs
)
