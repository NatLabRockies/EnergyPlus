cmake_minimum_required (VERSION 3.10)
project (kiva)

option(KIVA_COVERAGE "Add coverage reports" OFF)
option(KIVA_3D "Enable 3D Calculations" ON)
option(KIVA_EXE_BUILD "Create Kiva program executable" ON)
option(ENABLE_OPENMP "Use OpenMP" OFF)
option( KIVA_GROUND_PLOT "Build ground plotting library" ON )
mark_as_advanced(FORCE BUILD_GROUND_PLOT)
option( KIVA_TESTING "Build tests" ON )
option( KIVA_WERROR "Turn on warnings into error" OFF )

find_package(Git QUIET)

set(CMAKE_MODULE_PATH "${kiva_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if (KIVA_TESTING)
  enable_testing()
  if (KIVA_COVERAGE)
    set(ENABLE_COVERAGE ON CACHE BOOL "" FORCE)
    find_package(codecov)
  endif()
endif()

include(compiler-flags)

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${kiva_BINARY_DIR}" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${kiva_BINARY_DIR}" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${kiva_BINARY_DIR}" )

add_subdirectory(vendor)
add_subdirectory(src/libkiva)

if (KIVA_EXE_BUILD OR KIVA_GROUND_PLOT)
  add_subdirectory(src/libgroundplot)
  if (KIVA_EXE_BUILD)
    add_subdirectory(src/kiva)
  endif()
endif()

# Testing
if (KIVA_TESTING)
  add_subdirectory(test)
  if (KIVA_COVERAGE)
    coverage_evaluate()
  endif()
endif()

if (KIVA_EXE_BUILD)
  include(Install)
endif()