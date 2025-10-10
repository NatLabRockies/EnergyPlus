project(Transition Fortran)

cmake_minimum_required(VERSION 3.5.1...4.0.3)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")

set(CMAKE_SKIP_RPATH ${FORTRAN_SKIP_RPATH})

# first create a static library of shared stuff
set(LIB_SRC
    DataGlobals.f90
    DataStringGlobals.f90
    DataVCompareGlobals.f90
    DisplayRoutines.f90
    General.f90
    InputProcessor.f90
    platformDepUtilityRoutines.f90
    SortAndStringUtilities.f90
    UtilityRoutines.f90
    VCompareGlobalRoutines.f90
    VCompareUtilityRoutines.f90
)

add_library(TransitionLib STATIC ${LIB_SRC})
target_include_directories(TransitionLib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(TransitionLib PUBLIC -fbounds-check)

if(NOT UNIX)  # Need to reinclude it on Windows
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Fortran.cmake")
endif()
get_target_property(F_INTERFACE_COMPILE_FLAGS fortran_project_options INTERFACE_COMPILE_OPTIONS)
message(DEBUG "Transition: F_INTERFACE_COMPILE_FLAGS=${F_INTERFACE_COMPILE_FLAGS}")
target_link_libraries(TransitionLib PUBLIC fortran_project_options) # ffree-line-length-none

set(VERSIONS
    1_0_0  1_0_1  1_0_2  1_0_3  1_1_0  1_1_1  1_2_0  1_2_1  1_2_2  1_2_3  1_3_0  1_4_0
    2_0_0  2_1_0  2_2_0  3_0_0  3_1_0  4_0_0  5_0_0  6_0_0  7_0_0  7_1_0  7_2_0
    8_0_0  8_1_0  8_2_0  8_3_0  8_4_0  8_5_0  8_6_0  8_7_0  8_8_0  8_9_0
    9_0_0  9_1_0  9_2_0  9_3_0  9_4_0  9_5_0  9_6_0
    22_1_0 22_2_0 23_1_0 23_2_0 24_1_0 24_2_0 25_1_0 25_2_0
)
list(LENGTH VERSIONS VERSIONS_SIZE)
math(EXPR end "${VERSIONS_SIZE} - 1")

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ProjectMacros.cmake")

foreach(i RANGE 1 ${end})

  math(EXPR ii "${i} - 1")

  list(GET VERSIONS ${ii} OLD_VERSION)
  list(GET VERSIONS ${i} NEW_VERSION)

  # then create all the binaries using just the Transition source and the appropriate version of the main sub
  set(SRC Transition.f90 CreateNewIDFUsingRulesV${NEW_VERSION}.f90)

  string(REPLACE _ - OLD-VERSION ${OLD_VERSION})
  string(REPLACE _ - NEW-VERSION ${NEW_VERSION})

  set(LAST_NAME "Transition-V${OLD-VERSION}-to-V${NEW-VERSION}")

  set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LAST_NAME})
  add_executable("${LAST_NAME}" ${SRC})
  target_include_directories("${LAST_NAME}" PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  set_target_properties("${LAST_NAME}" PROPERTIES FOLDER Auxiliary)
  target_link_libraries("${LAST_NAME}" TransitionLib)

  if("${i}" GREATER "33")
    install(TARGETS "${LAST_NAME}" DESTINATION PreProcess/IDFVersionUpdater)
    if(NOT ${FORTRAN_STATIC_EXE})
      install_target_prereqs("${LAST_NAME}" PreProcess/IDFVersionUpdater)
    endif()

    if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
      include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CodeSigning.cmake")
      register_install_codesign_target("${LAST_NAME}" PreProcess/IDFVersionUpdater Unspecified)
    endif()
  endif()

endforeach()


#######################################################################
#             I N S T A L L    S U P P O R T    F I L E S             #
#######################################################################

install(FILES "InputRulesFiles/Rules9-0-0-to-9-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-1-0-to-9-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-2-0-to-9-3-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-3-0-to-9-4-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-4-0-to-9-5-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-5-0-to-9-6-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules9-6-0-to-22-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules22-1-0-to-22-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules22-2-0-to-23-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules23-1-0-to-23-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules23-2-0-to-24-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules24-1-0-to-24-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules24-2-0-to-25-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "InputRulesFiles/Rules25-1-0-to-25-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")

install(FILES "OutputRulesFiles/OutputChanges9-0-0-to-9-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-1-0-to-9-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-2-0-to-9-3-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-3-0-to-9-4-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-4-0-to-9-5-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-5-0-to-9-6-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges9-6-0-to-22-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges22-1-0-to-22-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges22-2-0-to-23-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges23-1-0-to-23-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges23-2-0-to-24-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges24-1-0-to-24-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges24-2-0-to-25-1-0.md" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "OutputRulesFiles/OutputChanges25-1-0-to-25-2-0.md" DESTINATION "PreProcess/IDFVersionUpdater")

install(FILES "SupportFiles/Report Variables 9-0-0 to 9-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-1-0 to 9-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-2-0 to 9-3-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-3-0 to 9-4-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-4-0 to 9-5-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-5-0 to 9-6-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 9-6-0 to 22-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 22-1-0 to 22-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 22-2-0 to 23-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 23-1-0 to 23-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 23-2-0 to 24-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 24-1-0 to 24-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 24-2-0 to 25-1-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")
install(FILES "SupportFiles/Report Variables 25-1-0 to 25-2-0.csv" DESTINATION "PreProcess/IDFVersionUpdater/")

