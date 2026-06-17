project(Basement Fortran)

cmake_minimum_required(VERSION 3.5.1...4.0.3)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")

set(SRC
    3DBasementHT.f90
    DataGlobals.f90
    DataStringGlobals.f90
    EPWRead.f90
    UtilityRoutines.f90
    DataPrecisionGlobals.f90
    EPWPrecisionGlobals.f90
    InputProcessor.f90
    BasementSimData.f90)

add_library(BasementOptions INTERFACE)
if(NOT CMAKE_Fortran_COMPILER MATCHES "ifort")
  target_compile_options(BasementOptions INTERFACE -ffloat-store -finit-local-zero)
endif()

if(NOT APPLE AND NOT UNIX) # Windows
  if(CMAKE_Fortran_COMPILER MATCHES "ifort")
    target_compile_options(
      BasementOptions
      INTERFACE /nologo
                /debug:minimal
                /O2
                /Oy-
                /assume:buffered_io
                /noaltparam
                /fpscomp:nolibs
                /fpe:0
                /fp:fast=2
                /traceback
                /check:uninit
                /libs:static
                /threads
                /c
                /assume:protect-parens
                /Qprec-div
                /Qprec-sqrt
                /Qimf-precision:high
                /Qimf-arch-consistency:true
                /Qvec-)
  endif()
endif()

set(CMAKE_SKIP_RPATH ${FORTRAN_SKIP_RPATH})
set(SKIP_BUILD_RPATH TRUE)

add_executable(Basement ${SRC})
target_compile_options(Basement PRIVATE $<$<CONFIG:Release>:-O1>)
set_target_properties(Basement PROPERTIES FOLDER Auxiliary)

if(NOT UNIX)  # Need to reinclude it on Windows
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Fortran.cmake")
endif()
get_target_property(F_INTERFACE_COMPILE_FLAGS fortran_project_options INTERFACE_COMPILE_OPTIONS)
message(DEBUG "Basement: F_INTERFACE_COMPILE_FLAGS=${F_INTERFACE_COMPILE_FLAGS}")
target_link_libraries(Basement PRIVATE BasementOptions fortran_project_options)

install(TARGETS Basement DESTINATION PreProcess/GrndTempCalc)
if(NOT ${FORTRAN_STATIC_EXE})
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ProjectMacros.cmake")
  install_target_prereqs(Basement PreProcess/GrndTempCalc)
endif()
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../idd/BasementGHT.idd" DESTINATION "PreProcess/GrndTempCalc/")

if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CodeSigning.cmake")
  register_install_codesign_target(Basement PreProcess/GrndTempCalc Unspecified)
endif()

if(BUILD_TESTING)

  macro(basement_test GHT_IN_IDF_NAME)
    set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/${GHT_IN_IDF_NAME}") # build/src/Basement/tst
    set(IDF_FILE "${PROJECT_SOURCE_DIR}/tests/${GHT_IN_IDF_NAME}.idf")

    file(MAKE_DIRECTORY ${TEST_DIR})
    file(COPY "${PROJECT_SOURCE_DIR}/../../idd/BasementGHT.idd" DESTINATION "${TEST_DIR}")
    configure_file("${PROJECT_SOURCE_DIR}/../../weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw" "${TEST_DIR}/in.epw" COPYONLY)
    configure_file(${IDF_FILE} "${TEST_DIR}/BasementGHTIn.idf" COPYONLY)

    set(TEST_NAME "Basement.${GHT_IN_IDF_NAME}")
    add_test(NAME "${TEST_NAME}"
      COMMAND $<TARGET_FILE:Basement>
      WORKING_DIRECTORY ${TEST_DIR}
    )
    set_tests_properties("${TEST_NAME}" PROPERTIES
      FAIL_REGULAR_EXPRESSION "Terminated;Error(s) Detected"
      ENVIRONMENT "CI_BAIL_EARLY=1"
    )
  endmacro()

  basement_test(AutoGriddingDepthIsHigh)
  basement_test(AutoGriddingDepthIsLowish)
  basement_test(AutoGriddingDepthIsTooHigh)
  set_tests_properties(Basement.AutoGriddingDepthIsTooHigh PROPERTIES WILL_FAIL TRUE)
endif()
