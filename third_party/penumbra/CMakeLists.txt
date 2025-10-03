cmake_minimum_required(VERSION 3.7...4.0.3)  # bumped max support to hush a cmake deprecation warning
cmake_policy(SET CMP0091 NEW)
project(penumbra LANGUAGES CXX C)

cmake_policy(SET CMP0079 NEW) # target_link_libraries() allows use with targets in other directories.

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

find_package(Git QUIET)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL)

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(CMakeDependentOption)

cmake_dependent_option( ${PROJECT_NAME}_BUILD_TESTING "Build ${PROJECT_NAME} testing targets" ON "${PROJECT_NAME}_IS_TOP_LEVEL" OFF )
option( ${PROJECT_NAME}_COVERAGE "Add ${PROJECT_NAME} coverage reports" OFF )
cmake_dependent_option( ${PROJECT_NAME}_BUILD_EXAMPLES "Build ${PROJECT_NAME} examples" ON "${PROJECT_NAME}_IS_TOP_LEVEL" OFF )
cmake_dependent_option( ${PROJECT_NAME}_WARNINGS_AS_ERRORS "Treat warnings in ${PROJECT_NAME} as errors" ON "${PROJECT_NAME}_IS_TOP_LEVEL" OFF )

if (NOT ${PROJECT_NAME}_STATIC_LIB)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

include(compiler-flags)

# Set up testing/coverage
if (${PROJECT_NAME}_BUILD_TESTING)
  enable_testing()
  if (${PROJECT_NAME}_COVERAGE)
    set(ENABLE_COVERAGE ON CACHE BOOL "" FORCE)
    find_package(codecov)
  endif()
endif()


# Dependencies
add_subdirectory(vendor)

add_subdirectory(src)

if (${PROJECT_NAME}_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (${PROJECT_NAME}_BUILD_TESTING)
  add_subdirectory(test)
  if (${PROJECT_NAME}_COVERAGE)
    coverage_evaluate()
  endif()
endif()
