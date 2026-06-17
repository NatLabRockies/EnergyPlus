#[=======================================================================[.rst:
CPackSignAndNotarizeDmg
-----------------------

This file is meant to be used up as a ``CPACK_POST_BUILD_SCRIPTS``

It will run only on ``APPLE`` when the generator is ``IFW`` to codesign the resulting .dmg and notarize it.

To do so, it uses the `CodeSigning`_ functions :cmake:command:`codesign_files_macos`

It requires that this be set: :cmake:variable:`CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION`

And it will only notarize if this is set: :cmake:variable:`CPACK_CODESIGNING_NOTARY_PROFILE_NAME`

#]=======================================================================]
message(STATUS "The message from ${CMAKE_CURRENT_LIST_FILE} and generator ${CPACK_GENERATOR}")
message(STATUS "Built packages: ${CPACK_PACKAGE_FILES}")

if(APPLE AND CPACK_GENERATOR STREQUAL "IFW")

  message("CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION=${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}")
  message("CPACK_CODESIGNING_NOTARY_PROFILE_NAME=${CPACK_CODESIGNING_NOTARY_PROFILE_NAME}")
  message("CPACK_IFW_PACKAGE_SIGNING_IDENTITY=${CPACK_IFW_PACKAGE_SIGNING_IDENTITY}")

  include(${CMAKE_CURRENT_LIST_DIR}/CodeSigning.cmake)

  if(NOT CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)
    message(FATAL_ERROR "CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION is required, this should not have happened")
  endif()
  codesign_files_macos(
    FILES ${CPACK_PACKAGE_FILES}
    SIGNING_IDENTITY ${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}
    IDENTIFIER "org.nrel.EnergyPlus.DmgInstaller"
    FORCE
    VERBOSE
  )

  if(CPACK_CODESIGNING_NOTARY_PROFILE_NAME)
    notarize_files_macos(
      FILES ${CPACK_PACKAGE_FILES}
      NOTARY_PROFILE_NAME ${CPACK_CODESIGNING_NOTARY_PROFILE_NAME}
      STAPLE
      VERIFY
      VERBOSE
    )
  endif()

endif()
