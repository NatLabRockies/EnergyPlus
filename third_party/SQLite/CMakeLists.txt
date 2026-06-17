set(SRC sqlite3.h sqlite3.c)

add_library(sqlite ${SRC})

target_compile_options(sqlite PRIVATE $<$<COMPILE_LANG_AND_ID:C,AppleClang>:-Wno-shorten-64-to-32>)

if(MSVC) # visual c++ (VS 2013)
  target_compile_options(sqlite PRIVATE -W0)                                                
elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")# g++/Clang
  target_compile_options(sqlite PRIVATE -w)
elseif(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  target_compile_options(sqlite PRIVATE /W0)
elseif(UNIX AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  target_compile_options(sqlite PRIVATE -w)
endif() # COMPILER TYPE
