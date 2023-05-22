
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mkdocs/mkdocs.yml.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/mkdocs/mkdocs.yml
)
add_custom_target(kdgpu-docs ALL

  # Execute Doxygen
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile

  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/mkdocs ${DOXYGEN_OUTPUT_DIR}/mkdocs
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LICENSES/MIT.txt ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/license.md
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/README.md ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/index.md
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/INSTALL.md ${DOXYGEN_OUTPUT_DIR}/mkdocs/docs/INSTALL.md

  # Run Doxybook2 to generate mkdocs compatible documentation
  COMMAND ${DOXYBOOK2_EXECUTABLE} --config "${CMAKE_CURRENT_SOURCE_DIR}/doxybook/config.json" --input "${DOXYGEN_OUTPUT_DIR}/xml" --output "${DOXYGEN_OUTPUT_DIR}/mkdocs/docs"

  # Run MkDocs to build html documentation
  COMMAND ${MKDOCS_EXECUTABLE} build --config-file "${DOXYGEN_OUTPUT_DIR}/mkdocs/mkdocs.yml" --site-dir "${DOXYGEN_OUTPUT_DIR}/html"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/mkdocs/mkdocs.yml
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Building Documentation"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
  DESTINATION docs/api
  COMPONENT kdgpu-docs)
