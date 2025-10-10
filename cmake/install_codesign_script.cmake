#[=======================================================================[.rst:
install_codesign_script
-----------------------

This file is meant to be used up as a ``install(SCRIPT)``

Pre-conditions:

- It requires that you're running on ``APPLE``

- **Important: In the scope of this** ``install(SCRIPT)``, you must define

    * :cmake:variable:`CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION`
    * :cmake:variable:`CPACK_CODESIGNING_MACOS_IDENTIFIER`
    * You need to also set a variable ``FILES_TO_SIGN``

**In the scope** meaning that you have to issue some ``install(CODE ..)`` commands prior to calling it, and they must be:

    * Part of the same ``project()``
    * Using the same CPack ``COMPONENT``

This script will codesign the ``FILES_TO_SIGN``, as well as the globbed copied Python .so and the root dylibs (such as ``libintl8.dylib``)

* ``python_lib/lib-dynload/*.so``
* ``lib*.dylib``

To do so, it uses the `CodeSigning`_ functions :cmake:command:`codesign_files_macos`

This script will therefore run in the CPack staging area, and should be added after installation of the Python sos and root dylib,
after any rpath adjustments, to ensure the signature sticks.

Usage::

  if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
    set(FILES_TO_SIGN "fileA" "fileB")
    install(CODE "set(FILES_TO_SIGN \"${FILES_TO_SIGN}\")" COMPONENT Unspecified)
    install(CODE "set(CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION \"${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}\")" COMPONENT Unspecified)
    install(CODE "set(CPACK_CODESIGNING_MACOS_IDENTIFIER \"${CPACK_CODESIGNING_MACOS_IDENTIFIER}\")" COMPONENT Unspecified)
    # call the script
    install(SCRIPT "${CMAKE_CURRENT_LIST_DIR}/install_codesign_script.cmake" COMPONENT Unspecified)
  endif()
#]=======================================================================]

#------------------------------------------------------------------------------
# Just a helper
function(print_relative_paths)
  set(prefix "")
  set(valueLessKeywords NAME_ONLY NEWLINE)
  set(singleValueKeywords PREFIX BASE_DIRECTORY)
  set(multiValueKeywords ABSOLUTE_PATHS)
  cmake_parse_arguments(
    PARSE_ARGV 0 # Start at one with NAME is the first param
      "${prefix}"
      "${valueLessKeywords}"
      "${singleValueKeywords}"
      "${multiValueKeywords}"
  )

  if (_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Extra unknown arguments were passed: ${_UNPARSED_ARGUMENTS}")
  endif()
  if (_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "Keywords missing values: ${_KEYWORDS_MISSING_VALUES}")
  endif()

  if(NOT _BASE_DIRECTORY)
    set(_BASE_DIRECTORY ${CMAKE_INSTALL_PREFIX})
  endif()

  foreach(path ${_ABSOLUTE_PATHS})
    if(_NAME_ONLY)
      cmake_path(GET path FILENAME rel_path)
    else()
      cmake_path(RELATIVE_PATH path BASE_DIRECTORY ${_BASE_DIRECTORY} OUTPUT_VARIABLE rel_path)
    endif()
    list(APPEND rel_paths ${rel_path})
  endforeach()

  if(_NEWLINE)
    message("${_PREFIX}")
    foreach(path ${rel_paths})
      message("   - ${path}")
    endforeach()
  else()
    message("${_PREFIX}${rel_paths}")
  endif()
endfunction()
#------------------------------------------------------------------------------

message("Codesigning inner executables and library from ${CMAKE_CURRENT_LIST_FILE}")

message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message("CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION=${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}")
message("CPACK_CODESIGNING_MACOS_IDENTIFIER=${CPACK_CODESIGNING_MACOS_IDENTIFIER}")
message("CMAKE_INSTALL_COMPONENT=${CMAKE_INSTALL_COMPONENT}")
message("CPACK_GENERATOR=${CPACK_GENERATOR}")

if(NOT CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
  message(FATAL_ERROR "CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION is required")
endif()

if(NOT CPACK_CODESIGNING_MACOS_IDENTIFIER)
  message(FATAL_ERROR "CPACK_CODESIGNING_MACOS_IDENTIFIER is required")
endif()

if(NOT FILES_TO_SIGN)
  message(FATAL_ERROR "FILES_TO_SIGN is required")
endif()

foreach(path ${FILES_TO_SIGN})
  list(APPEND _FULL_PATHS "${CMAKE_INSTALL_PREFIX}/${path}")
endforeach()

file(GLOB _all_root_dylibs "${CMAKE_INSTALL_PREFIX}/lib*.dylib")
foreach(path ${_all_root_dylibs})
  message("${path}")
  if(NOT IS_SYMLINK ${path})
    list(FIND _FULL_PATHS ${path} _found)
    if(_found EQUAL -1)
      list(APPEND _ROOT_DYLIBS ${path})
    endif()
  endif()
endforeach()

file(GLOB _PYTHON_SOS "${CMAKE_INSTALL_PREFIX}/python_lib/lib-dynload/*.so" "${CMAKE_INSTALL_PREFIX}/python_lib/lib-dynload/*.dylib")

print_relative_paths(PREFIX "FULL_PATHS=" ABSOLUTE_PATHS ${_FULL_PATHS})
print_relative_paths(PREFIX "ROOT_DYLIBS=" ABSOLUTE_PATHS ${_ROOT_DYLIBS})
print_relative_paths(PREFIX "PYTHON_SOS, in ${CMAKE_INSTALL_PREFIX}/python_lib/lib-dynload/=" ABSOLUTE_PATHS ${_PYTHON_SOS} NAME_ONLY)

include(${CMAKE_CURRENT_LIST_DIR}/CodeSigning.cmake)
codesign_files_macos(
  FILES ${_FULL_PATHS} ${_ROOT_DYLIBS} ${_PYTHON_SOS}
  SIGNING_IDENTITY ${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}
  PREFIX "${CPACK_CODESIGNING_MACOS_IDENTIFIER}."
  FORCE VERBOSE
)

# Clean up to avoid multiple passes (several components) appending to a pre-existing list
unset(FILES_TO_SIGN)
unset(_FULL_PATHS)
unset(_ROOT_DYLIBS)
unset(_PYTHON_SOS)

message("Finished Codesigning inner executables and library")
