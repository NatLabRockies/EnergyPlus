#[=======================================================================[.rst:
CodeSigning
------------

This module defines functions to codesign, notarize and staple macOS files.

.. cmake:command:: codesign_files_macos

  run ``codesign`` on the files::

    codesign_files_macos(
                         SIGNING_IDENTITY <identity>
                         [FORCE] [VERBOSE] [DEEP]
                         [IDENTIFIER <identifier>]
                         [PREFIX <prefix>]
                         [OPTIONS <options>...]
                         [ENTITLEMENTS <entitlements_file>]
                         FILES <files>...
    )


  ``codesign_files_macos()`` will codesign the files

  The options are:

  ``SIGNING_IDENTITY identity``
    Required, this is something like ``Developer ID Application: <The Name> (<TEAMID>)``

  ``FILES path...``
    The files to codesign

  ``VERBOSE``
    If specified, will append ``-vvvv`` and will print the commands used

  ``FORCE``
    If specified, will append ``--force``

  ``DEEP``
    If specified, will append ``--deep``. Note that this is supposedly deprecated by the ``codesign`` utility.

  ``OPTIONS options...``
    Specifies the options to pass to ``--options``. If not specified, uses ``--options runtime``

  ``IDENTIFIER identifier``
    Passed as ``--identifier identifier``.

  ``PREFIX``
    What to pass to ``--prefix``. eg 'com.domain.MyApp.' with a **trailing dot**. Ignored if ``IDENTIFIER`` is passed

  ``ENTITLEMENTS entitlements_file``
    The entitlements xml file to use

.. cmake:command:: notarize_files_macos

  Runs ``notarytool``, ``staple`` on the files::

      notarize_files_macos(
                           NOTARY_PROFILE_NAME <profile-name>
                           [VERBOSE]
                           [STAPLE]
                           [VERIFY]
                           FILES <files>...
      )


  The options are:

  ``NOTARY_PROFILE_NAME``
    Required, should be set to the name you used during ``xcrun notarytool store-credentials``

  ``FILES path...``
    The files to notarize

  ``VERBOSE``
    If specified, will print the commands used

  ``STAPLE``
    If specified, will run ``stappler`` after the notarytool submission

  ``VERIFY``
    If specified, will run ``spctl --assess`` to validate proper notarization

.. cmake:command:: setup_macos_codesigning_variables

  Defines CMake Configure options::

    setup_macos_codesigning_variables()

  The resulting configure options are:

  * :cmake:variable:`CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION`

    Pre-populated options from ``security-find-identity -v -p codesign``.
    This should be a valid "Developer ID Application"

    If set to non-empty, will also:

    * set ``CPACK_IFW_PACKAGE_SIGNING_IDENTITY`` to the same value, so binarycreator signs the .app installer created
    * define another ``CPACK_CODESIGNING_NOTARY_PROFILE_NAME`` option

  * :cmake:variable:`CPACK_CODESIGNING_NOTARY_PROFILE_NAME`

    Authenticate using credentials stored in the Keychain by notarytool.
    Use the profile name that you previously provided via the ``xcrun notarytool store-credentials`` command

  * :cmake:variable:`CPACK_CODESIGNING_MACOS_IDENTIFIER`

    What is passed as --identifier to the codesign signing. Defaults to ``com.domain.MyApp``

.. cmake:command:: register_install_codesign_target

  Given a target and a relative install path,
  this will register an ``install(CODE)`` command to codesign the executable or library::

      register_install_codesign_target(
        <TARGET_NAME> <DESTINATION>
      )


  It is necessary to have issued an ``install(TARGET <TARGET_NAME> DESTINATION <DESTINATION>)`` command before calling this function,
  and done any call to ``fixup_executable`` or ``install_name_tool`` that would invalidate the signature.

  This function will therefore run in the CPack staging area, after any rpath adjustments, and ensure the signature sticks.

  It will only do something on ``APPLE`` and if :cmake:variable:`CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION` is defined.

  It requires ``CMP0087`` to be set to ``NEW``.

  Internally, it requires this very file and calls ``codesign_files_macos`` on the target file.

  The required parameters are:

  ``TARGET_NAME``
    A valid target added via ``add_executable``/``add_library``
    Required, should be set to the name you used during ``xcrun notarytool store-credentials``

  ``DESTINATION``
    The destination for the installed target (eg: ``"."`` or ``lib/``)
#]=======================================================================]

function(print_cmd_if_verbose cmd VERBOSE)
  if(VERBOSE)
    list(JOIN cmd "\" \"" cmd_str)
    message("\"${cmd_str}\"")
  endif()
endfunction()

#------------------------------------------------------------------------------
function(codesign_files_macos)
  set(prefix "")
  set(valueLessKeywords FORCE VERBOSE DEEP)
  set(singleValueKeywords SIGNING_IDENTITY IDENTIFIER PREFIX ENTITLEMENTS)
  set(multiValueKeywords FILES OPTIONS)
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

  if(NOT _OPTIONS)
    set(options_str runtime)
  else()
    list(JOIN _OPTIONS "," options_str)
  endif()

  if(NOT _FILES)
    message(FATAL_ERROR "Cannot sign without FILES passed")
  endif()

  find_program(CODESIGN NAMES codesign)
  if(NOT CODESIGN)
    message(FATAL_ERROR "Cannot sign, could not find 'codesign' executable")
  endif()

  set(cmd "${CODESIGN}")
  if(_VERBOSE)
    list(APPEND cmd -vvvv)
  endif()

  if(NOT _SIGNING_IDENTITY)
    message(FATAL_ERROR "Cannot sign without a SIGNING_IDENTITY passed in")
  endif()
  list(APPEND cmd --sign  "${_SIGNING_IDENTITY}")

  if(_FORCE)
    list(APPEND cmd --force)
  endif()

  if(_DEEP)
    list(APPEND cmd --deep)
  endif()

  list(APPEND cmd --timestamp)
  list(APPEND cmd --options "${options_str}")

  if(_IDENTIFIER)
    list(APPEND cmd "--identifier" "${_IDENTIFIER}")
  elseif(_PREFIX)
    list(APPEND cmd "--prefix" "${_PREFIX}")
  endif()

  if (_ENTITLEMENTS)
    if (NOT EXISTS "${_ENTITLEMENTS}")
      message(FATAL_ERROR "Can't sign with entitlements ${_ENTITLEMENTS}, no file exists at that path.")
    endif ()

    list(APPEND cmd "--entitlements" "${_ENTITLEMENTS}")
  endif()

  foreach(path ${_FILES})
    print_cmd_if_verbose("${cmd};${path}" _VERBOSE)

    if (NOT EXISTS "${path}")
      message(FATAL_ERROR "Can't sign ${path}, no file exists at that path.")
    endif ()

    execute_process(COMMAND ${cmd} "${path}" RESULT_VARIABLE res)
    if (NOT res EQUAL 0)
      message(FATAL_ERROR "Can't sign ${path}, command '${cmd}' failed")
    endif ()
  endforeach()

endfunction()
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
function(notarize_files_macos)
  set(prefix "")
  set(valueLessKeywords VERBOSE STAPLE VERIFY)
  set(singleValueKeywords NOTARY_PROFILE_NAME)
  set(multiValueKeywords FILES)
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

  if(NOT _NOTARY_PROFILE_NAME)
    message(FATAL_ERROR "Cannot notarize without NOTARY_PROFILE_NAME passed, should be set to the name you used during `xcrun notarytool store-credentials`")
  endif()

  if(NOT _FILES)
    message(FATAL_ERROR "Cannot notarize without FILES passed")
  endif()

  find_program(XCRUN NAMES xcrun)
  if (NOT XCRUN)
    message(FATAL_ERROR "Cannot notarize, could not find 'xcrun' executable")
  endif ()

  set(cmd "${XCRUN}" notarytool submit --keychain-profile ${_NOTARY_PROFILE_NAME} --wait)
  list(JOIN cmd " " cmd_str)

  foreach(path ${_FILES})
     message(STATUS "notarytool: submitting ${path}")
      print_cmd_if_verbose("${cmd};${path}" _VERBOSE)

    if (NOT EXISTS "${path}")
      message(FATAL_ERROR "Can't notarize ${path}, no file exists at that path.")
    endif ()

    execute_process(
      COMMAND ${cmd} "${path}"
      RESULT_VARIABLE res
      OUTPUT_VARIABLE out
      ECHO_OUTPUT_VARIABLE
    )
    string(REGEX MATCH "([0-9a-z]+-[0-9a-z]+-[0-9a-z]+-[0-9a-z]+-[0-9a-z]+)" SUBMISSION_ID ${out})
    set(cmd_get_log "${XCRUN}" notarytool log --keychain-profile ${_NOTARY_PROFILE_NAME} ${SUBMISSION_ID})

    if (_VERBOSE OR NOT res EQUAL 0)
      message(STATUS "notarytool: retrieving the log")
      print_cmd_if_verbose("${cmd_get_log}" _VERBOSE)
      execute_process(
        COMMAND ${cmd_get_log}
      )
    endif()

    if (NOT res EQUAL 0)
      message(FATAL_ERROR "Can't notarize ${path}, command '${cmd}' failed, perhaps try `${cmd_get_log_str}`")
    endif()


    if(_STAPLE)
      set(cmd xcrun stapler staple "${path}")
      message(STATUS "stapler: stappling ${path}")
      print_cmd_if_verbose("${cmd}" _VERBOSE)
      execute_process(
        COMMAND ${cmd}
        RESULT_VARIABLE res
        )
      if (NOT res EQUAL 0)
        message(FATAL_ERROR "Can't stapple ${path}, command '${cmd}' failed (${res})")
      endif ()
    endif()

    if(_VERIFY)
      message(STATUS "Verifying that .dmg is properly notarized")
      get_filename_component(ext ${path} LAST_EXT)
      if (ext STREQUAL ".dmg")
        set(cmd spctl --assess --type open --context context:primary-signature -vvvv "${path}")
      else()
        set(cmd spctl --assess  -vvvv "${path}")
      endif()
      print_cmd_if_verbose("${cmd}" _VERBOSE)
      execute_process(
        COMMAND ${cmd}
        RESULT_VARIABLE res
        )
      if (NOT res EQUAL 0)
        message(FATAL_ERROR "Notarization failed for ${path}, command '${cmd}' failed (${res})")
      endif()
    endif()

  endforeach()
endfunction()
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
function(setup_macos_codesigning_variables)
  # prefix with CPACK_ so it's properly passed to the POST_BUILD_SCRIPTS
  set(CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION "${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}" CACHE STRING "code signing identity (e.g., \"Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)\") (required for code signing)")

  # Get list of valid codesigning identities from system.
  execute_process(COMMAND security find-identity -v -p codesigning
    RESULT_VARIABLE res
    OUTPUT_VARIABLE lines
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if (CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
    list(APPEND idents "${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}")
  endif ()
  if (res EQUAL 0 AND lines)
    # Split string into list of lines.
    string(REGEX REPLACE ";" "\\\\;" lines "${lines}")
    string(REGEX REPLACE "\n" ";" lines "${lines}")
    # Parse signing cert identity from each line
    foreach(line ${lines})
      # eg: 4) C5CE92B14361BF09E55990573DF07FC33B083D22 "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)"
      if (line MATCHES "[0-9]+\\)[ \t]+[0-9a-fA-F]+[ \t]+\"(.+ \\([^ \t]+\\))\"")
        list(APPEND idents "${CMAKE_MATCH_1}")
      endif ()
    endforeach()
  endif()

  set(CPACK_CODESIGNING_MACOS_IDENTIFIER "org.nrel.EnergyPlus" CACHE STRING "The code signing identifier, passed as --identifier")

  # Populate drop-down box in cmake-gui with the list of valid codesigning identities.
  set_property(CACHE CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION PROPERTY STRINGS "${idents}")

  if(CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
    set(CPACK_IFW_PACKAGE_SIGNING_IDENTITY ${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION} CACHE STRING "set from CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION" FORCE)
    mark_as_advanced(CPACK_IFW_PACKAGE_SIGNING_IDENTITY)

    set(CPACK_CODESIGNING_NOTARY_PROFILE_NAME "" CACHE STRING "Authenticate using credentials stored in the Keychain by notarytool. Use the profile name that you previously provided via the store-credentials command")

    if(NOT CPACK_CODESIGNING_NOTARY_PROFILE_NAME)
      message(AUTHOR_WARNING "Cannot notarize without CPACK_CODESIGNING_NOTARY_PROFILE_NAME defined, should be set to the name you used during `xcrun notarytool store-credentials`")
    endif()
  endif()
endfunction()
#------------------------------------------------------------------------------

function(register_install_codesign_target TARGET_NAME DESTINATION COMPONENT)

  set(prefix "")
  set(valueLessKeywords "")
  set(singleValueKeywords ENTITLEMENTS)
  set(multiValueKeywords "")
  cmake_parse_arguments(
    PARSE_ARGV 3 # We have 3 positional keywords that are required
      "${prefix}"
      "${valueLessKeywords}"
      "${singleValueKeywords}"
      "${multiValueKeywords}"
  )

  if(NOT TARGET ${TARGET_NAME})
    message(FATAL_ERROR "${TARGET_NAME} is not a valid target")
  endif()

  if(NOT APPLE)
    message(DEBUG "Not Apple, skipping signing")
    return()
  endif()

  if(NOT CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
    message(DEBUG "Missing CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION, skipping signing")
    return()
  endif()

  if(NOT CPACK_CODESIGNING_MACOS_IDENTIFIER)
    message(DEBUG "Missing CPACK_CODESIGNING_MACOS_IDENTIFIER, skipping signing")
    return()
  endif()

  if (_ENTITLEMENTS)
    install(
      CODE "
      include(\"${CMAKE_CURRENT_FUNCTION_LIST_FILE}\")
      codesign_files_macos(
        FILES \"\${CMAKE_INSTALL_PREFIX}/${DESTINATION}/$<IF:$<BOOL:$<TARGET_PROPERTY:${TARGET_NAME},MACOSX_BUNDLE>>,$<TARGET_BUNDLE_DIR_NAME:${TARGET_NAME}>,$<TARGET_FILE_NAME:${TARGET_NAME}>>\"
        SIGNING_IDENTITY \"${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}\"
        IDENTIFIER \"${CPACK_CODESIGNING_MACOS_IDENTIFIER}.${TARGET_NAME}\"
        FORCE VERBOSE $<$<BOOL:$<TARGET_PROPERTY:${TARGET_NAME},MACOSX_BUNDLE>>:DEEP>
        ENTITLEMENTS \"${_ENTITLEMENTS}\"
        )
    "
      COMPONENT ${COMPONENT}
    )
  else()
    install(
      CODE "
      include(\"${CMAKE_CURRENT_FUNCTION_LIST_FILE}\")
      codesign_files_macos(
        FILES \"\${CMAKE_INSTALL_PREFIX}/${DESTINATION}/$<IF:$<BOOL:$<TARGET_PROPERTY:${TARGET_NAME},MACOSX_BUNDLE>>,$<TARGET_BUNDLE_DIR_NAME:${TARGET_NAME}>,$<TARGET_FILE_NAME:${TARGET_NAME}>>\"
        SIGNING_IDENTITY \"${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}\"
        IDENTIFIER \"${CPACK_CODESIGNING_MACOS_IDENTIFIER}.${TARGET_NAME}\"
        FORCE VERBOSE $<$<BOOL:$<TARGET_PROPERTY:${TARGET_NAME},MACOSX_BUNDLE>>:DEEP>
        )
    "
      COMPONENT ${COMPONENT}
    )
  endif()

endfunction()
