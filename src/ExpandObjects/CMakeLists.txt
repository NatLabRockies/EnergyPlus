project(ExpandObjects Fortran)

cmake_minimum_required(VERSION 3.5.1...4.0.3)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")

set(SRC epfilter.f90)

if(WIN32)
  # Include manifest
  configure_file(ExpandObjects.rc.in "${CMAKE_CURRENT_BINARY_DIR}/ExpandObjects.rc")
  list(APPEND SRC "${CMAKE_CURRENT_BINARY_DIR}/ExpandObjects.rc")
endif()

set(CMAKE_SKIP_RPATH ${FORTRAN_SKIP_RPATH})
set(SKIP_BUILD_RPATH TRUE)

add_executable(ExpandObjects ${SRC})
target_link_libraries(ExpandObjects PRIVATE fortran_project_options)

if(NOT UNIX)  # Need to reinclude it on Windows
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Fortran.cmake")
endif()
get_target_property(F_INTERFACE_COMPILE_FLAGS fortran_project_options INTERFACE_COMPILE_OPTIONS)
message(DEBUG "ExpandObjects: F_INTERFACE_COMPILE_FLAGS=${F_INTERFACE_COMPILE_FLAGS}")
set_target_properties(ExpandObjects PROPERTIES FOLDER Auxiliary)

install(TARGETS ExpandObjects DESTINATION "./")
if(NOT ${FORTRAN_STATIC_EXE})
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ProjectMacros.cmake")
  install_target_prereqs(ExpandObjects "./")
endif()

if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CodeSigning.cmake")
  register_install_codesign_target(ExpandObjects "." Unspecified)
endif()
