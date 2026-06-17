add_library(fortran_project_options INTERFACE)
set(FORTRAN_STATIC_EXE FALSE)
set(FORTRAN_SKIP_RPATH FALSE)

if(APPLE)
  if(CMAKE_Fortran_COMPILER MATCHES "ifort")
    target_compile_options(fortran_project_options INTERFACE -fpp)
    target_link_options(fortran_project_options INTERFACE -static-intel)
  else()
    if(NOT "Ninja" STREQUAL ${CMAKE_GENERATOR})
      target_compile_options(fortran_project_options INTERFACE -cpp)
    endif()

    # `-static` isn't possible on mac. From gcc man page:
    # > This option will not work on Mac OS X unless all libraries (including libgcc.a) have also been compiled with -static.
    # > Since neither a static version of libSystem.dylib nor crt0.o are provided, this option is not useful to most people.

    # master of gcc has a fix via a new `-static-libquadmath` but it's not even in gcc 12.2.0. So instead we have to do all kinds of shenanigans

    enable_language(Fortran)  # Needed to populate ${CMAKE_Fortran_IMPLICIT_LINK_DIRECTORIES}

    include(CheckFortranCompilerFlag)
    check_fortran_compiler_flag(-static-libgfortran HAS_STATIC_LIBGFORTRAN)
    check_fortran_compiler_flag(-static-libgcc HAS_STATIC_LIBGCC)
    check_fortran_compiler_flag(-static-libquadmath HAS_STATIC_LIBQUADMATH)
    message(DEBUG "HAS_STATIC_LIBGFORTRAN=${HAS_STATIC_LIBGFORTRAN}, HAS_STATIC_LIBGCC=${HAS_STATIC_LIBGCC}, HAS_STATIC_LIBQUADMATH=${HAS_STATIC_LIBQUADMATH}")
    # Debug:
    # set(Fortran_VERBOSE "-v")

    target_compile_options(fortran_project_options INTERFACE
      ${Fortran_VERBOSE}
    )

    if (NOT HAS_STATIC_LIBGFORTRAN OR NOT HAS_STATIC_LIBGCC)
      message(FATAL_ERROR "Please check your compiler. gfortran supports -static-libgcc and -static-libgfortran since at least 2007, but HAS_STATIC_LIBGFORTRAN=${HAS_STATIC_LIBGFORTRAN}, HAS_STATIC_LIBGCC=${HAS_STATIC_LIBGCC}")
    endif()
    if (HAS_STATIC_LIBQUADMATH)
      target_link_options(fortran_project_options INTERFACE
        ${Fortran_VERBOSE}
        -static-libgfortran
        -static-libgcc
        -static-libquadmath
      )
    else()
      # Find the static libquadmath manually
      find_library(static-libquadmath NAMES quadmath.a libquadmath.a REQUIRED PATHS ${CMAKE_Fortran_IMPLICIT_LINK_DIRECTORIES})
      message(STATUS "Found static-libquadmath at ${static-libquadmath} by searching in CMAKE_Fortran_IMPLICIT_LINK_DIRECTORIES=${CMAKE_Fortran_IMPLICIT_LINK_DIRECTORIES}")

      message(DEBUG "CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES=${CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES}")
      # On brew gcc 12.2.0
      #    => CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES=gfortran;emutls_w;gcc;quadmath;emutls_w;gcc;gcc
      # On https://github.com/fxcoudert/gfortran-for-macOS
      #  arm64 12.1                                =gfortran;emutls_w;gcc;quadmath;emutls_w;gcc;gcc
      #  x86_64 1.2-monterey-intel                 =gfortran;gcc_ext.10.5;gcc;/usr/local/gfortran/lib/libquadmath.a;m
      list(REMOVE_DUPLICATES CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES)
      list(TRANSFORM CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES REPLACE "^(-)?(lib)?quadmath$" "${static-libquadmath}")
      message(DEBUG "CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES=${CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES}")

      set(FORTRAN_LIBS "")
      foreach(_lib ${CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES} ${CURL_LIBS})
        if(_lib MATCHES ".*/.*" OR _lib MATCHES "^-")
          list(APPEND FORTRAN_LIBS "${_lib}")
        else()
          list(APPEND FORTRAN_LIBS "-l${_lib}")
        endif()
      endforeach()

      message(STATUS "FORTRAN_LIBS=${FORTRAN_LIBS}")

      # target_link_options(fortran_project_options INTERFACE
      #   ${Fortran_VERBOSE}
      #   -nodefaultlibs
      #   -static-libgfortran
      #   -lgfortran
      #   -static-libgcc
      #   -lgcc
      #   -lemutls_w
      #   ${static-libquadmath}
      #   -lSystem
      # )

      target_link_options(fortran_project_options INTERFACE
        ${Fortran_VERBOSE}
        -nodefaultlibs
        -static-libgfortran
        -static-libgcc
        "${FORTRAN_LIBS}"
        -lSystem
      )

      # We unset this so that CMake doesn't try to read -lquadmath etc
      unset(CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES)
    endif()


    # We could technically just set it to TRUE here. But it shouldn't do anything anyways, and it's a fallback
    # set(FORTRAN_STATIC_EXE TRUE)
  endif()

elseif(UNIX)
  set(FORTRAN_SKIP_RPATH TRUE)
  if(CMAKE_Fortran_COMPILER MATCHES "ifort")
    target_compile_options(fortran_project_options INTERFACE -fpp)
    target_link_options(fortran_project_options INTERFACE -static-intel)
    target_compile_options(fortran_project_options INTERFACE $<$<CONFIG:Debug>:-traceback>)
  else()
    if(NOT "Ninja" STREQUAL ${CMAKE_GENERATOR})
      target_compile_options(fortran_project_options INTERFACE -cpp)
    endif()
    set(FORTRAN_STATIC_EXE TRUE)
    target_link_options(fortran_project_options INTERFACE -static)
    target_compile_options(fortran_project_options INTERFACE $<$<CONFIG:Debug>:-fbacktrace -ffpe-trap=zero,overflow,underflow>)
  endif()
else() # Windows
  set(FORTRAN_STATIC_EXE TRUE)
  if(CMAKE_Fortran_COMPILER MATCHES "ifort")
    # Set release flags to be empty
    set(CMAKE_Fortran_FLAGS_RELEASE "")
    target_compile_options(fortran_project_options INTERFACE /fpp)
    target_link_options(fortran_project_options INTERFACE /libs:static)
  else()
    if(NOT "Ninja" STREQUAL ${CMAKE_GENERATOR})
      target_compile_options(fortran_project_options INTERFACE -cpp)
    endif()
    target_link_options(fortran_project_options INTERFACE -static)
  endif()
endif()
target_compile_options(fortran_project_options INTERFACE -ffree-line-length-275)
