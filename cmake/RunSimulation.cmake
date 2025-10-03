# These need to be defined by the caller
# SOURCE_DIR
# BINARY_DIR
# ENERGYPLUS_EXE
# IDF_FILE
# EPW_FILE
# BUILD_FORTRAN
# TEST_FILE_FOLDER
# ENERGYPLUS_FLAGS
# RUN_CALLGRIND
# VALGRIND
# RUN_PERF_STAT
# PERF

get_filename_component(IDF_NAME "${IDF_FILE}" NAME_WE)
get_filename_component(IDF_EXT "${IDF_FILE}" EXT)
get_filename_component(EXE_PATH "${ENERGYPLUS_EXE}" PATH)

# Create path variables
set(OUTPUT_DIR_PATH "${BINARY_DIR}/${TEST_FILE_FOLDER}/${IDF_NAME}/")
set(IDF_PATH "${SOURCE_DIR}/${TEST_FILE_FOLDER}/${IDF_FILE}")
set(PRODUCT_PATH "${BINARY_DIR}/Products/")
set(EXE_PATH "${EXE_PATH}/")
set(EPW_PATH "${SOURCE_DIR}/weather/${EPW_FILE}")

# Copy IDD to Executable directory if it is not already there
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PRODUCT_PATH}/Energy+.idd" "${EXE_PATH}")

# Clean up old test directory
execute_process(COMMAND "${CMAKE_COMMAND}" -E remove_directory "${OUTPUT_DIR_PATH}")
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
    execute_process(COMMAND "${PARAMETRIC_EXE}" "${IDF_FILE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}" COMMAND_ERROR_IS_FATAL ANY)

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
    # This creates GHTIn.idf, and BasementGHTIn.idf IIF the GroundHeatTransfer:Control says to run it!
    execute_process(COMMAND "${EXPANDOBJECTS_EXE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}" COMMAND_ERROR_IS_FATAL ANY)

    if("${SLAB_RESULT}" GREATER -1)
      if(NOT EXISTS "${OUTPUT_DIR_PATH}/GHTIn.idf")
        string(REGEX MATCH "GroundHeatTransfer:Control.*Run Slab Preprocessor" GROUND_HT_CONTROL "${IDF_CONTENT}")
        if (GROUND_HT_CONTROL)
          message(FATAL_ERROR "Did not find ${OUTPUT_DIR_PATH}/GHTIn.idf, are you sure the GroundHeatTransfer:Control has Run Slab Preprocessor =  Yes?\nGROUND_HT_CONTROL=${GROUND_HT_CONTROL}")
        else()
          message(FATAL_ERROR "Did not find ${OUTPUT_DIR_PATH}/GHTIn.idf, are you sure the GroundHeatTransfer:Control has Run Slab Preprocessor =  Yes?")
        endif()
      endif()
      # Copy files needed for Slab
      file(COPY "${SOURCE_DIR}/idd/SlabGHT.idd" DESTINATION "${OUTPUT_DIR_PATH}")
      # Find and run slab
      find_program(SLAB_EXE Slab PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                                                         NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
      message("Executing Slab from ${SLAB_EXE}")
      # This creates SLABINP.TXT, SLABSplit Surface Temps.TXT, and SLABSurfaceTemps.TXT
      execute_process(COMMAND "${SLAB_EXE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}" COMMAND_ERROR_IS_FATAL ANY)
      # Then copy slab results into the expanded file
      file(READ "${OUTPUT_DIR_PATH}/SLABSurfaceTemps.TXT" SLAB_CONTENTS)
      file(APPEND "${OUTPUT_DIR_PATH}/expanded.idf" "${SLAB_CONTENTS}")
    endif()

    if("${BASEMENT_RESULT}" GREATER -1)
      if(NOT EXISTS "${OUTPUT_DIR_PATH}/BasementGHTIn.idf")
        string(REGEX MATCH "GroundHeatTransfer:Control.*Run Slab Preprocessor" GROUND_HT_CONTROL "${IDF_CONTENT}")
        if (GROUND_HT_CONTROL)
          message(FATAL_ERROR "Did not find ${OUTPUT_DIR_PATH}/BasementGHTIn.idf, are you sure the GroundHeatTransfer:Control has Run Basement Preprocessor =  Yes?\nGROUND_HT_CONTROL=${GROUND_HT_CONTROL}")
        else()
          message(FATAL_ERROR "Did not find ${OUTPUT_DIR_PATH}/BasementGHTIn.idf, are you sure the GroundHeatTransfer:Control has Run Basement Preprocessor =  Yes?")
        endif()
      endif()

      # Copy files needed for Basement
      file(COPY "${SOURCE_DIR}/idd/BasementGHT.idd" DESTINATION "${OUTPUT_DIR_PATH}")
      # Find and run basement
      find_program(BASEMENT_EXE Basement PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                                                                 NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
      message("Executing Basement from ${BASEMENT_EXE}")
      execute_process(COMMAND "${BASEMENT_EXE}" WORKING_DIRECTORY "${OUTPUT_DIR_PATH}" COMMAND_ERROR_IS_FATAL ANY)
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
    find_program(READVARS_EXE ReadVarsESO PATHS "${PRODUCT_PATH}" NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                                                                  NO_CMAKE_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH)
    # Move to executable directory
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${READVARS_EXE}" "${EXE_PATH}")
  endif()

endif() # build fortran

if(WIN32)
  set(ECHO_CMD cmd /C echo.)
else()
  set(ECHO_CMD "echo")
endif()

if(RUN_CALLGRIND)
  set(VALGRIND_COMMAND ${VALGRIND} --tool=callgrind --branch-sim=yes --callgrind-out-file=callgrind.performance.${IDF_NAME})
else()
  set(VALGRIND_COMMAND "")
endif()

if(RUN_PERF_STAT)
  set(PERF_STAT_COMMAND ${PERF} stat -d -d -d -x , -r 3  -o perf.performance.${IDF_NAME})
else()
  set(PERF_STAT_COMMAND "")
endif()


if (RUN_CALLGRIND)
  message("Running Simulation with Callgrind:")
  message(STATUS "${VALGRIND_COMMAND} ${ENERGYPLUS_EXE} -w ${EPW_PATH} -d ${OUTPUT_DIR_PATH} ${ENERGYPLUS_FLAGS} ${IDF_PATH}")
  execute_process(
    COMMAND ${ECHO_CMD}
    COMMAND ${VALGRIND_COMMAND} "${ENERGYPLUS_EXE}" -w "${EPW_PATH}" -d "${OUTPUT_DIR_PATH}" ${ENERGYPLUS_FLAGS_LIST} "${IDF_PATH}"
    WORKING_DIRECTORY "${OUTPUT_DIR_PATH}"
    RESULT_VARIABLE RESULT)

  if(NOT RESULT EQUAL 0)
    message("Test Failed")
    return()
  endif()
endif()

if (RUN_PERF_STAT)
  message("Running Simulation with Perf Stat:")
  message(STATUS "${PERF_STAT_COMMAND} ${ENERGYPLUS_EXE} -w ${EPW_PATH} -d ${OUTPUT_DIR_PATH} ${ENERGYPLUS_FLAGS} ${IDF_PATH}")
  execute_process(
    COMMAND ${ECHO_CMD}
    COMMAND ${PERF_STAT_COMMAND} "${ENERGYPLUS_EXE}" -w "${EPW_PATH}" -d "${OUTPUT_DIR_PATH}" ${ENERGYPLUS_FLAGS_LIST} "${IDF_PATH}"
    WORKING_DIRECTORY "${OUTPUT_DIR_PATH}"
    RESULT_VARIABLE RESULT)

  if(NOT RESULT EQUAL 0)
    message("Test Failed")
    return()
  endif()
endif()


# Run without perf stat or callgrind, if neither is requested
if (NOT RUN_PERF_STAT AND NOT RUN_CALLGRIND)
  message("Running Simulation:")
  message(STATUS "${ENERGYPLUS_EXE} -w ${EPW_PATH} -d ${OUTPUT_DIR_PATH} ${ENERGYPLUS_FLAGS} ${IDF_PATH}")
  execute_process(
    COMMAND ${ECHO_CMD}
    COMMAND "${ENERGYPLUS_EXE}" -w "${EPW_PATH}" -d "${OUTPUT_DIR_PATH}" ${ENERGYPLUS_FLAGS_LIST} "${IDF_PATH}"
    WORKING_DIRECTORY "${OUTPUT_DIR_PATH}"
    RESULT_VARIABLE RESULT)

  if(NOT RESULT EQUAL 0)
    message("Test Failed")
    return()
  endif()
endif()

set(HTML_PATH "${OUTPUT_DIR_PATH}/eplustbl.htm")
if(EXISTS "${HTML_PATH}")
  message("Checking uniqueness of HTML Table FullNames:")
  message(STATUS "${Python_EXECUTABLE} ${SOURCE_DIR}/scripts/dev/ensure_unique_html_tables.py ${OUTPUT_DIR_PATH}")
  execute_process(
    COMMAND ${Python_EXECUTABLE} "${SOURCE_DIR}/scripts/dev/ensure_unique_html_tables.py" "${OUTPUT_DIR_PATH}"
    RESULT_VARIABLE RESULT
    ECHO_OUTPUT_VARIABLE
    ECHO_ERROR_VARIABLE
  )

  if(NOT RESULT EQUAL 0)
    message("Test Failed: HTML output tables are NOT unique")
    return()
  endif()
endif()

# if we get here, then none of the required tests failed
message("Test Passed")
