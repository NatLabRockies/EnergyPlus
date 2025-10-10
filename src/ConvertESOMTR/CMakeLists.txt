project(ConvertESOMTR Fortran)

cmake_minimum_required(VERSION 3.5.1...4.0.3)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../Products")

set(SRC mainModule.f90)

set(CMAKE_SKIP_RPATH ${FORTRAN_SKIP_RPATH})
set(SKIP_BUILD_RPATH TRUE)

add_executable(convertESOMTR ${SRC})
set_target_properties(convertESOMTR PROPERTIES FOLDER Auxiliary)

if(NOT UNIX)  # Need to reinclude it on Windows
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Fortran.cmake")
endif()
get_target_property(F_INTERFACE_COMPILE_FLAGS fortran_project_options INTERFACE_COMPILE_OPTIONS)
message(DEBUG "ConvertESOMTR: F_INTERFACE_COMPILE_FLAGS=${F_INTERFACE_COMPILE_FLAGS}")
target_link_libraries(convertESOMTR PRIVATE fortran_project_options)

install(TARGETS convertESOMTR DESTINATION "PostProcess/convertESOMTRpgm/")
if(NOT ${FORTRAN_STATIC_EXE})
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ProjectMacros.cmake")
  install_target_prereqs(convertESOMTR "PostProcess/convertESOMTRpgm/")
endif()
install(FILES "convert.txt" DESTINATION "PostProcess/convertESOMTRpgm/")

if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CodeSigning.cmake")
  register_install_codesign_target(convertESOMTR "PostProcess/convertESOMTRpgm" Unspecified)
endif()
