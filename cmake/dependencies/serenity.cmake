# Serenity library
if (NOT TARGET Serenity::Core)
    find_package(Serenity CONFIG)
    set_package_properties(Serenity PROPERTIES
        DESCRIPTION "Extensible 2D/3D real-time engine with a Vulkan-based renderer"
        URL "" # TODO
        TYPE OPTIONAL
    )
endif()
 
if (NOT TARGET Serenity::Core AND SERENITY_TARBALL)
    FetchContent_Declare(Serenity
        URL ${SERENITY_TARBALL}
    )
    set(SERENITY_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(Serenity)
    set(Serenity_FOUND ON)
endif()
