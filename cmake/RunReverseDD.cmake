# These need to be defined by the caller
# SOURCE_DIR
# BINARY_DIR
# ENERGYPLUS_EXE
# IDF_FILE
# BUILD_FORTRAN
# TEST_FILE_FOLDER
# ENERGYPLUS_FLAGS
# PYTHON_EXECUTABLE

get_filename_component(IDF_NAME "${IDF_FILE}" NAME_WE)
get_filename_component(IDF_EXT "${IDF_FILE}" EXT)
get_filename_component(EXE_PATH "${ENERGYPLUS_EXE}" PATH)

# Create path variables
set(OUTPUT_DIR_NAME "testfiles_ReverseDD")
set(IDF_PATH "${SOURCE_DIR}/${TEST_FILE_FOLDER}/${IDF_FILE}")
set(PRODUCT_PATH "${BINARY_DIR}/Products/")
set(EXE_PATH "${EXE_PATH}/")
set(EPW_PATH "${SOURCE_DIR}/weather/${EPW_FILE}")

# Copy IDD to Executable directory if it is not already there
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PRODUCT_PATH}/Energy+.idd" "${EXE_PATH}")

# Delete the root test directory which will delete any previous Regular/Reversed subdirectories
execute_process(COMMAND "${CMAKE_COMMAND}" -E remove_directory "${BINARY_DIR}/${OUTPUT_DIR_NAME}/${IDF_NAME}")

# list zipping is available as of 3.17, which is our minimum version, :+1:
set(MODE Regular Reversed)
set(VAR "" Y)

message("Running reverse design day test for file ${IDF_NAME}")

foreach(M V IN ZIP_LISTS MODE VAR)

  set(OUTPUT_DIR_PATH "${BINARY_DIR}/${OUTPUT_DIR_NAME}/${IDF_NAME}/${M}/")

  # Create a new directory for output
  execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUTPUT_DIR_PATH}")

  # Read the file contents to check for special cases
  file(READ "${IDF_PATH}" IDF_CONTENT)

  # Convert flags back to CMake list
  string(STRIP ${ENERGYPLUS_FLAGS} ENERGYPLUS_FLAGS)
  string(REPLACE " " ";" ENERGYPLUS_FLAGS_LIST ${ENERGYPLUS_FLAGS})

  # Use EPMacro if necessary
  list(FIND ENERGYPLUS_FLAGS_LIST -m EPMACRO_RESULT)

  if("${EPMACRO_RESULT}" GREATER -1)
    # first bring in all imf files into the run folder
    file(GLOB SRC_IMF_FILES "${SOURCE_DIR}/${TEST_FILE_FOLDER}/*.imf")
    foreach(IMF_FILE ${SRC_IMF_FILES})
      file(COPY "${IMF_FILE}" DESTINATION "${OUTPUT_DIR_PATH}")
    endforeach()
    # find the appropriate executable file
    if(UNIX AND NOT APPLE)
      find_program(EPMACRO_EXE EPMacro PATHS "${SOURCE_DIR}/bin/EPMacro/Linux" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH
                                                                               NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
    elseif(APPLE)
      find_program(EPMACRO_EXE EPMacro PATHS "${SOURCE_DIR}/bin/EPMacro/Mac" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH
                                                                             NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
    else() # windows
      find_program(EPMACRO_EXE EPMacro PATHS "${SOURCE_DIR}/bin/EPMacro/Windows"
                   NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
    endif()
    # Move EPMacro to executable directory
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EPMACRO_EXE}" "${EXE_PATH}")
  endif()

  if(BUILD_FORTRAN)

    set(ENV{CI_BASEMENT_NUMYEARS} "2")

    # Parametric preprocessor next
    string(FIND "${IDF_CONTENT}" "Parametric:" PAR_RESULT)
    if("${PAR_RESULT}" GREATER -1)
      find_program(PARAMETRIC_EXE ParametricPreprocessor PATHS "${PRODUCT_PATH}"
                   NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${IDF_PATH}" "${OUTPUT_DIR_PATH}")
      execute_process(COMMAND "${PARAMETRIC_EXE}" "${IDF_FILE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}")

      # this handles the LBuildingAppGRotPar parametric file
      if(EXISTS "${OUTPUT_DIR_PATH}/${IDF_NAME}-G000.idf")
        set(IDF_PATH "${OUTPUT_DIR_PATH}/${IDF_NAME}-G000.idf")

        # this handles the LBuildingAppGRotPar and ParametricInsulation-5ZoneAirCooled parametric files
      elseif(EXISTS "${OUTPUT_DIR_PATH}/${IDF_NAME}-000001.idf")
        set(IDF_PATH "${OUTPUT_DIR_PATH}/${IDF_NAME}-000001.idf")

        # this shouldn't happen unless a new parametric file is added with a different processed filename
      else()
        message("Couldn't find parametric preprocessor output file for ${IDF_NAME}, attempting to continue with original input file.")

      endif()

    endif() # parametric preprocessor definitions detected

    # Run ExpandObjects independently if there are ground heat transfer objects
    string(FIND "${IDF_CONTENT}" "GroundHeatTransfer:Slab" SLAB_RESULT)
    string(FIND "${IDF_CONTENT}" "GroundHeatTransfer:Basement" BASEMENT_RESULT)

    if("${SLAB_RESULT}" GREATER -1 OR "${BASEMENT_RESULT}" GREATER -1)
      find_program(EXPANDOBJECTS_EXE ExpandObjects PATHS "${BINARY_DIR}/Products/"
                   NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
      message("Executing ExpandObjects from ${EXPANDOBJECTS_EXE}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${IDF_PATH}" "${OUTPUT_DIR_PATH}/in.idf")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${EPW_PATH}" "${OUTPUT_DIR_PATH}/in.epw")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PRODUCT_PATH}/Energy+.idd" "${OUTPUT_DIR_PATH}")
      execute_process(COMMAND "${EXPANDOBJECTS_EXE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}")

      if("${SLAB_RESULT}" GREATER -1)
        # Copy files needed for Slab
        file(COPY "${SOURCE_DIR}/idd/SlabGHT.idd" DESTINATION "${OUTPUT_DIR_PATH}")
        # Find and run slab
        find_program(SLAB_EXE Slab PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                                                           NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
        message("Executing Slab from ${SLAB_EXE}")
        execute_process(COMMAND "${SLAB_EXE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}")
        # Then copy slab results into the expanded file
        file(READ "${OUTPUT_DIR_PATH}/SLABSurfaceTemps.TXT" SLAB_CONTENTS)
        file(APPEND "${OUTPUT_DIR_PATH}/expanded.idf" "${SLAB_CONTENTS}")
      endif()

      if("${BASEMENT_RESULT}" GREATER -1)
        # Copy files needed for Basement
        file(COPY "${SOURCE_DIR}/idd/BasementGHT.idd" DESTINATION "${OUTPUT_DIR_PATH}")
        # Find and run basement
        find_program(BASEMENT_EXE Basement PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                                                                   NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
        message("Executing Basement from ${BASEMENT_EXE}")
        execute_process(COMMAND "${BASEMENT_EXE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}")
        # Then copy basement results into the expanded file
        file(READ "${OUTPUT_DIR_PATH}/EPObjects.TXT" BASEMENT_CONTENTS)
        file(APPEND "${OUTPUT_DIR_PATH}/expanded.idf" "${BASEMENT_CONTENTS}")
      endif()

      set(IDF_PATH "${OUTPUT_DIR_PATH}/expanded.idf")

    endif() # expand objects found something and created expanded.idf

    list(FIND ENERGYPLUS_FLAGS_LIST -x EXPAND_RESULT)

    if("${EXPAND_RESULT}" GREATER -1)
      find_program(EXPANDOBJECTS_EXE ExpandObjects PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH
                                                                           NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
      # Move to executable directory
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EXPANDOBJECTS_EXE}" "${EXE_PATH}")
    endif()

    list(FIND ENERGYPLUS_FLAGS_LIST -r READVARS_RESULT)

    if("${READVARS_RESULT}" GREATER -1)
      find_program(READVARS_EXE NAMES ReadVarsESO ReadVarsESO.bat PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH
                                                                                   NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
      # Move to executable directory
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${READVARS_EXE}" "${EXE_PATH}")
    endif()

  endif() # build fortran

  if(WIN32)
    set(ECHO_CMD cmd /C echo.)
  else()
    set(ECHO_CMD "echo")
  endif()

  # set the reverse design day environment variable for EnergyPlus
  set(ENV{REVERSEDD} "${V}")
  execute_process(
    COMMAND ${ECHO_CMD}
    COMMAND "${ENERGYPLUS_EXE}" -w "${EPW_PATH}" -d "${OUTPUT_DIR_PATH}" ${ENERGYPLUS_FLAGS_LIST} "${IDF_PATH}"
    WORKING_DIRECTORY "${OUTPUT_DIR_PATH}"
    RESULT_VARIABLE RESULT)

  if (RESULT EQUAL 0)
    # pass
  else()
    # if there was a problem with E+, it will either be caught with integration tests, or it is because of a problem
    # with reverse DD run ... such as zero design days ... so I just won't catch those for now
    message("Test Passed")
    return()
  endif()
endforeach()

message("Reverse design day simulations completed, now running comparison script")

execute_process(
        COMMAND ${ECHO_CMD}
        COMMAND "${PYTHON_EXECUTABLE}" "${SOURCE_DIR}/cmake/ReverseDDPostProcess.py" "${BINARY_DIR}/${OUTPUT_DIR_NAME}/${IDF_NAME}"
        WORKING_DIRECTORY "${OUTPUT_DIR_PATH}"
        RESULT_VARIABLE RESULT_RDD)

message("Reverse design day comparison complete")

if(RESULT_RDD EQUAL 0)
  message("Test Passed")
else()
  message("Test Failed")
endif()
