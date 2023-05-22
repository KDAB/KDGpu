find_package(Doxygen)
set_package_properties(Doxygen PROPERTIES
  TYPE REQUIRED
  DESCRIPTION "API Documentation system"
  URL "https://www.doxygen.org"
  PURPOSE "Needed to build the API documentation."
)

find_program(DOXYBOOK2_EXECUTABLE doxybook2)
if(NOT DOXYBOOK2_EXECUTABLE)
  message(FATAL_ERROR "\ndoxybook2 (a Doxygen XML to Markdown (or JSON) converter) could not be found. This tool is required to build the API documentation. Consider installing it by following the directions at https://github.com/matusnovak/doxybook2.\nAlternatively, re-run cmake with the -D${PROJECT_NAME}_DOCS=False option.")
endif()

find_program(MKDOCS_EXECUTABLE mkdocs)
if(NOT MKDOCS_EXECUTABLE)
  message(FATAL_ERROR "\nmkdocs (a project documentation tool using Markdown) could not be found. This tool is required to build the API documentation. Consider installing it by following the directions at https://www.mkdocs.org/user-guide/installation.\nAlso you'll need to 'pip install pymdown-extensions mkdocs-material'\nAlternatively, re-run cmake with the -D${PROJECT_NAME}_DOCS=False option.")
endif()

add_subdirectory(api)
