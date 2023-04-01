# KDUtils library
find_package(KDUtils CONFIG)

if(NOT KDUtils_FOUND)
    FetchContent_Declare(
        KDUtils
        GIT_REPOSITORY ssh://codereview.kdab.com:29418/kdab/kdutils
        GIT_TAG master
        USES_TERMINAL_DOWNLOAD YES
        USES_TERMINAL_UPDATE YES
    )

    option(KDUTILS_BUILD_TESTS "Build the tests" OFF)

    FetchContent_MakeAvailable(KDUtils)
endif()

find_package(KDFoundation CONFIG)
find_package(KDGui CONFIG)
