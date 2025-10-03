project(Slab Fortran)

cmake_minimum_required(VERSION 3.5.1...4.0.3)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")

set(SRC
    3DSlabHT.f90
    DataGlobals.f90
    DataStringGlobals.f90
    EPWRead.f90
    General.f90
    MainProgram.f90
    UtilityRoutines.f90
    DataPrecisionGlobals.f90
    EPWPrecisionGlobals.f90
    EPWWeather.f90
    InputProcessor.f90
    SimData.f90)

add_library(SlabOptions INTERFACE)
if(NOT CMAKE_Fortran_COMPILER MATCHES "ifort")
  target_compile_options(SlabOptions INTERFACE -finit-local-zero)
endif()

set(CMAKE_SKIP_RPATH ${FORTRAN_SKIP_RPATH})
set(SKIP_BUILD_RPATH true)

add_executable(Slab ${SRC})
set_target_properties(Slab PROPERTIES FOLDER Auxiliary)

if(NOT UNIX)  # Need to reinclude it on Windows
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Fortran.cmake")
endif()
get_target_property(F_INTERFACE_COMPILE_FLAGS fortran_project_options INTERFACE_COMPILE_OPTIONS)
message(DEBUG "Slab: F_INTERFACE_COMPILE_FLAGS=${F_INTERFACE_COMPILE_FLAGS}")
target_link_libraries(Slab PRIVATE SlabOptions fortran_project_options)

install(TARGETS Slab DESTINATION PreProcess/GrndTempCalc)
if(NOT ${FORTRAN_STATIC_EXE})
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ProjectMacros.cmake")
  install_target_prereqs(Slab PreProcess/GrndTempCalc)
endif()
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../idd/SlabGHT.idd" DESTINATION "PreProcess/GrndTempCalc/")

if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CodeSigning.cmake")
  register_install_codesign_target(Slab PreProcess/GrndTempCalc Unspecified)
endif()
