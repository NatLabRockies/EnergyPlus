cmake_minimum_required(VERSION 3.15)

project(EnergyPlusDocs LANGUAGES NONE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT EXISTS ${Python_EXECUTABLE})
  # If running docs standalone, Python isn't found yet.
  find_package(Python 3.6 COMPONENTS Interpreter REQUIRED)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
  option(DOCS_TESTING "Test the docs for issues" OFF)
endif()

find_package(LATEX COMPONENTS XELATEX)

set(TEX_INTERACTION
    "batchmode"
    CACHE STRING "Choose the interaction mode for TeX.")
set_property(CACHE TEX_INTERACTION PROPERTY STRINGS "nonstopmode" "batchmode")
if(EXISTS "${XELATEX_COMPILER}")
  # and if it does, just add the doc/CMakeLists commands
  # let's create a folder to drop ONLY PDFs in once they are built, this will allow easy upload to s3
  file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/pdf)
  file(COPY ${PROJECT_SOURCE_DIR}/index.html DESTINATION ${PROJECT_BINARY_DIR}/pdf)

  include("${PROJECT_SOURCE_DIR}/../cmake/Version.cmake")
  configure_file("${PROJECT_SOURCE_DIR}/title.tex.in" "${PROJECT_BINARY_DIR}/title.tex")

  # Add custom target to build only documentation, this target will depend on all individual PDFs
  if(hasParent AND DOCUMENTATION_BUILD STREQUAL "BuildOnlyWithPackage")
    # Because we don't want to rebuild doc automatically, we do not add it to the 'ALL' rule
    message(STATUS "The target `docs` was added, but not to the default build rule. It will be triggered by `make docs` or `make package`")
    add_custom_target(docs)
  else()
    # Building standalone, so add to ALL
    # message(STATUS "The target `docs` was added to the default build rule")
    add_custom_target(docs ALL)
  endif()

  set_target_properties(docs PROPERTIES FOLDER Documentation)

  include(cmake/doc-targets.cmake)

  # add each of the documents, they have their own CMakeLists.txt files
  add_subdirectory(engineering-reference)
  add_subdirectory(external-interfaces-application-guide)
  add_subdirectory(getting-started)
  add_subdirectory(input-output-reference)
  add_subdirectory(interface-developer)
  add_subdirectory(module-developer)
  add_subdirectory(output-details-and-examples)
  add_subdirectory(plant-application-guide)
  add_subdirectory(using-energyplus-for-compliance)

  if(NOT hasParent)
    # Standalone build of the doc/ folder: package the generated PDFs into a TGZ/ZIP archive
    if(WIN32)
      set(CPACK_GENERATOR "ZIP")
    else()
      set(CPACK_GENERATOR "TGZ")
    endif()

    set(CPACK_PACKAGE_FILE_NAME
      "EnergyPlus-PDFDocumentation-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}-${CMAKE_VERSION_BUILD}")

    message("Installer name is set to '${CPACK_PACKAGE_FILE_NAME}'")

    install(DIRECTORY "${PROJECT_BINARY_DIR}/pdf/" DESTINATION ".")

    include(CPack)
  endif()

else()
  # and if it doesn't, clearly state why it fails
  message(FATAL_ERROR "Could not find the XeLaTeX compiler; check your LaTeX installation")
endif()
