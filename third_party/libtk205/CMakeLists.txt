cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(libtk205)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

find_package(Git QUIET)

include(CTest)
set(JSON_BuildTests OFF CACHE INTERNAL "")

option(${PROJECT_NAME}_BUILD_TESTING "Build ${PROJECT_NAME} testing targets" OFF)
option(${PROJECT_NAME}_COVERAGE "Add ${PROJECT_NAME} coverage reports" OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(compiler-flags)

add_subdirectory(src)
add_subdirectory(vendor)

if (${PROJECT_NAME}_BUILD_TESTING)
  add_compile_definitions(TEST205_INPUT_EXAMPLES_DIR="${PROJECT_SOURCE_DIR}")
  enable_testing()
  add_subdirectory(test)
endif()
