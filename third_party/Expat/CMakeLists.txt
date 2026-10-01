include(ConfigureChecks.cmake)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

set(epexpat_SRCS
lib/xmlparse.c
lib/xmlrole.c
lib/xmltok.c
lib/xmltok_impl.c
lib/xmltok_ns.c
)

if(MSVC)
  set(epexpat_SRCS ${epexpat_SRCS} lib/libexpat.def)
endif(MSVC)

IF (UNIX)
  ADD_DEFINITIONS("-fPIC")
ENDIF()

IF ( CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"  ) # g++/Clang
  ADD_COMPILE_OPTIONS(-Wno-unused-parameter -Wno-missing-field-initializers)
ENDIF()

add_library(epexpat STATIC ${epexpat_SRCS})
target_include_directories(epexpat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_compile_options(epexpat PRIVATE $<$<COMPILE_LANG_AND_ID:C,AppleClang>:-Wno-shorten-64-to-32>)

set_target_properties(
  epexpat
  PROPERTIES
  INSTALL_NAME_DIR "@executable_path"
)

