FetchContent_Declare(
    trompeloeil
    GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
    GIT_TAG v42
    )
FetchContent_Populate(trompeloeil)

add_library(trompeloeil INTERFACE)
target_include_directories(trompeloeil INTERFACE
    $<BUILD_INTERFACE:${trompeloeil_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Serenity/trompeloeil>
    )
add_library(Serenity::trompeloeil ALIAS trompeloeil)

install(DIRECTORY ${trompeloeil_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Serenity/trompeloeil)
