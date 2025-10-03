#cmake_minimum_required(VERSION 2.4.4)
#set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(miniziplib C)

include(../../cmake/TargetArch.cmake)
target_architecture(TARGET_ARCH)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)

# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

IF ( CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"  ) # g++/Clang
    ADD_COMPILE_OPTIONS(-Wno-unused-parameter -Wno-implicit-function-declaration -Wno-shift-negative-value)
ENDIF()

#if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
#    # If we're doing an out of source build and the user has a zconf.h
#    # in their source tree...
#    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h)
#        message(FATAL_ERROR
#            "You must remove ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h "
#            "from the source tree.  This file is included with zlib "
#            "but CMake generates this file for you automatically "
#            "in the build directory.")
#  endif()
#endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


#============================================================================
# zlib
#============================================================================

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/zconf.h
    zlib.h
)
set(ZLIB_PRIVATE_HDRS
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zutil.h
)
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

IF(MSVC)
  LIST(APPEND ZLIB_SRCS win32/zlib1.rc win32/zlib.def)
ENDIF(MSVC)

#============================================================================
# minizip
#============================================================================

IF(APPLE)
  add_definitions("-Dftello64=ftello")
  add_definitions("-Dfseeko64=fseeko")
  add_definitions("-Dfopen64=fopen")
ENDIF(APPLE)

set(minizip_src
    "contrib/minizip/ioapi.c"
    "contrib/minizip/unzip.c"
    "contrib/minizip/zip.c"
)

IF(MSVC)
  LIST(APPEND minizip_src "contrib/minizip/iowin32.c")
ELSE(MSVC)
  add_definitions(-Dunix)
  add_definitions(-fPIC)
ENDIF(MSVC)

add_library(miniziplib STATIC ${ZLIB_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS} ${minizip_src})

if( MSVC )
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    if(TARGET_ARCH MATCHES "arm64")
      set_target_properties(miniziplib PROPERTIES STATIC_LIBRARY_FLAGS "/machine:arm64")
    else()
      set_target_properties(miniziplib PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
    endif()
  else()
    set_target_properties(miniziplib PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x86")
  endif()
endif()

#IF( NOT APPLE )
#INSTALL(TARGETS miniziplib
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION lib )
#ENDIF()

#add_executable(minizip "contrib/minizip/minizip.c")
#target_link_libraries(minizip miniziplib)

#add_executable(miniunz "contrib/minizip/miniunz.c")
#target_link_libraries(miniunz miniziplib)
