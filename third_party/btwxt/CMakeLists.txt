cmake_minimum_required(VERSION 3.10)

project(btwxt)

# Set a default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
            "MinSizeRel" "RelWithDebInfo")
endif ()

find_package(Git QUIET)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(compiler-flags)

option(${PROJECT_NAME}_BUILD_TESTING "Build ${PROJECT_NAME} testing targets" OFF)
option(${PROJECT_NAME}_COVERAGE "Add ${PROJECT_NAME} coverage reports" OFF)

# Set up testing/coverage
if (${PROJECT_NAME}_BUILD_TESTING)
    enable_testing()
    if (${PROJECT_NAME}_COVERAGE)
        set(ENABLE_COVERAGE ON CACHE BOOL "" FORCE)
        find_package(codecov)
    endif ()
endif ()

add_subdirectory("include/${PROJECT_NAME}")
add_subdirectory(src)
add_subdirectory(vendor)

if (${PROJECT_NAME}_BUILD_TESTING)
    add_subdirectory(test)
    if (${PROJECT_NAME}_COVERAGE)
        coverage_evaluate()
    endif ()
endif ()
