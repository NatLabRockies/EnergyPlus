# '*' indicates CMake default option
# '+' indicates default compiler behavior
add_library(penumbra_common_interface INTERFACE)

  #==================#
  # Compiler options #
  #==================#

target_compile_options(penumbra_common_interface INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>: # Visual C++ (VS 2013)
    /GR
    /nologo
    /W4
    $<$<BOOL:${${PROJECT_NAME}_WARNINGS_AS_ERRORS}>:
      /WX     # Turn warnings into errors
    >
    $<$<CONFIG:Release>:
      /GS-    # Disable buffer overrun checks for performance in release mode
    >
  >
  # GCC And Clang
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
    -pthread
    -pipe       # Faster compiler processing
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>: # Adds flag only to C++
      -pedantic   # Turn on warnings about constructs/situations that may be non-portable or outside of the standard
    >
    -Wall       # Turn on warnings
    -Wextra     # Turn on warnings
    $<$<BOOL:${${PROJECT_NAME}_WARNINGS_AS_ERRORS}>:
      -Werror     # Turn warnings into errors
    >
    $<$<CONFIG:Release>:
      -fno-stack-protector  # Produces debugging information specifically for gdb
    >
    $<$<CXX_COMPILER_ID:GNU>:
      $<$<CONFIG:Debug>:
        -ffloat-store     # Improve debug run solution stability
        -fsignaling-nans  # Disable optimizations that may have concealed NaN behavior
      >
    >
    # -finline-limit=2000 # More aggressive inlining   This is causing unit test failures on Ubuntu 14.04
  >
)

#======================#
# Compiler definitions #
#======================#

target_compile_definitions(penumbra_common_interface INTERFACE
  # GCC
  $<$<CXX_COMPILER_ID:GNU>:
    $<$<CONFIG:Debug>:
      _GLIBCXX_DEBUG  # Standard container debug mode (bounds checking, ...)
    >
  >
)

#==================#
#  Linker options  #
#==================#

target_link_options(penumbra_common_interface INTERFACE
  $<$<CXX_COMPILER_ID:GNU>:
    -pthread
  >

)

# This macro will encapsulate the CMAKE_CXX flags that should only be set for executables
macro (SET_CXX_FLAGS)
# Remove unwanted CMake defaults from global flags
if (MSVC)
    # See https://gitlab.kitware.com/cmake/cmake/-/blob/master/Modules/Platform/Windows-MSVC.cmake
    set(CMAKE_CXX_FLAGS
        /EHsc         #*Specifies the model of exception handling (sc options).
        /DWIN32       #*Windows Platform (regardless of architecture)
        /D_WINDOWS    #*
        )
    set(CMAKE_CXX_FLAGS_RELEASE
        /O2           #*Creates fast code (Og+Oi+Ot+Oy+Ob2+GF+Gy).
        # /Ob2        #*Controls inline expansion (level 2). (part of O2)
        /DNDEBUG      #*Enables or disables compilation of assertions
        )
    set(CMAKE_CXX_FLAGS_DEBUG
        /Ob0          #*Controls inline expansion (level 0 -- disabled).
        /Od           #*Disables optimization.
        /Zi           #*Generates complete debugging information.
        /RTC1         #*Enables run-time error checking.
        )
else () # GCC or Clang or AppleClang
    # See https://gitlab.kitware.com/cmake/cmake/-/blob/master/Modules/Compiler/GNU.cmake
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_RELEASE
        -O3           #*Maximum optimization (see https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Optimize-Options).
        -DNDEBUG      #*Enables or disables compilation of assertions
        )
    set(CMAKE_CXX_FLAGS_DEBUG
        -g            #*Produce debugging information in the operating system’s native format.
        )
endif()

# Convert lists to space-separated strings
list(JOIN CMAKE_CXX_FLAGS " " CMAKE_CXX_FLAGS)
list(JOIN CMAKE_CXX_FLAGS_RELEASE " " CMAKE_CXX_FLAGS_RELEASE)
list(JOIN CMAKE_CXX_FLAGS_DEBUG " " CMAKE_CXX_FLAGS_DEBUG)
endmacro()
