# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
KDSelectLibraryConfigurations
---------------------------

.. code-block:: cmake

  kd_select_library_configurations(basename)

This macro takes a library base name as an argument, and will choose
good values for the variables

::

  basename_LIBRARY
  basename_LIBRARIES
  basename_LIBRARY_DEBUG
  basename_LIBRARY_RELEASE

depending on what has been found and set.

If CMAKE_BUILD_TYPE == Debug and  ``basename_LIBRARY_DEBUG`` is set, then we set
``basename_LIBRARY`` to ``basename_LIBRARY_DEBUG``.

If CMAKE_BUILD_TYPE == Release or RelWithDebInfo and  ``basename_LIBRARY_RELEASE`` is set, then we set
``basename_LIBRARY`` to ``basename_LIBRARY_RELEASE``.

This does not support multiconfig generators (yet).
#]=======================================================================]

# This macro is modified from the upstream selectlibraryconfigurations

macro(kd_select_library_configurations basename)
    if(NOT ${basename}_LIBRARY_RELEASE)
        set(${basename}_LIBRARY_RELEASE "${basename}_LIBRARY_RELEASE-NOTFOUND" CACHE FILEPATH "Path to a library.")
    endif()
    if(NOT ${basename}_LIBRARY_DEBUG)
        set(${basename}_LIBRARY_DEBUG "${basename}_LIBRARY_DEBUG-NOTFOUND" CACHE FILEPATH "Path to a library.")
    endif()

    # This won't work for multiconfig generators like MS Visual Studio or XCode
    # See https://stackoverflow.com/questions/24460486/cmake-build-type-is-not-being-used-in-cmakelists-txt
    # Would be good to find a nicer solution that works on both Ninja/Makefile and multiconfig generators.
    if( CMAKE_BUILD_TYPE STREQUAL Debug AND ${basename}_LIBRARY_DEBUG )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_DEBUG} )
    elseif( ( CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo ) AND ${basename}_LIBRARY_RELEASE )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_RELEASE} )
    else()
        set( ${basename}_LIBRARY "${basename}_LIBRARY-NOTFOUND")
    endif()

    set( ${basename}_LIBRARIES "${${basename}_LIBRARY}" )

    if( ${basename}_LIBRARY )
        set( ${basename}_FOUND TRUE )
    endif()

    mark_as_advanced( ${basename}_LIBRARY_RELEASE
        ${basename}_LIBRARY_DEBUG
    )
endmacro()
