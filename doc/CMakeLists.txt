project(EnergyPlusDocs LANGUAGES NONE)

cmake_minimum_required(VERSION 3.12)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT EXISTS ${Python_EXECUTABLE})
  # If running docs standalone, Python isn't found yet.
  find_package(Python 3.6 COMPONENTS Interpreter REQUIRED)
endif()

option(DOCS_TESTING "Test the docs for issues" OFF)

# this _should_ find the xelatex compiler, but for some reason it isn't populating for me
find_package(LATEX)
# so we'll create a cache variable which will be available to the add_subdirectory call below
set(XELATEX
    "XeLaTeX Compiler"
    CACHE INTERNAL "")
# and we'll simply take the found pdflatex compiler path and replace pdflatex with xelatex
string(REPLACE pdflatex xelatex XELATEX ${PDFLATEX_COMPILER})
# and then verify it actually exists at that location...which it should pretty much every time...
set(TEX_INTERACTION
    "batchmode"
    CACHE STRING "Choose the interaction mode for TeX.")
set_property(CACHE TEX_INTERACTION PROPERTY STRINGS "nonstopmode" "batchmode")
if(EXISTS ${XELATEX})
  # and if it does, just add the doc/CMakeLists commands
  # let's create a folder to drop ONLY PDFs in once they are built, this will allow easy upload to s3
  file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/pdf)
  file(COPY ${PROJECT_SOURCE_DIR}/index.html DESTINATION ${PROJECT_BINARY_DIR}/pdf)

  include("${PROJECT_SOURCE_DIR}/../cmake/Version.cmake")
  # add a configure rule to the header file though
  configure_file(${PROJECT_SOURCE_DIR}/title.tex.in ${PROJECT_SOURCE_DIR}/title.tex)

  # Add custom target to build only documentation, this target will depend on all individual PDFs
  get_directory_property(hasParent PARENT_DIRECTORY)
  if(hasParent AND BUILD_DOCS_ONLY_WITH_PACKAGE)
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
  message(FATAL_ERROR "Expected to find xelatex at: ${XELATEX} ; check your LaTeX installation")
endif()
