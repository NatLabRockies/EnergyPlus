# Sets up the pyenergyplus Python package, which is the EnergyPlus Python API wrapper, in the build tree, next to E+
# set REPO_ROOT, EPLUS_EXE_DIR (path to folder containing energyplus.exe), and E+ and API major/minor/etc version variables when calling

# informative messaging
message("PYTHON: Setting up API, creating pyenergyplus package at ${EPLUS_EXE_DIR}/pyenergyplus")

set(API_TARGET_DIR "${EPLUS_EXE_DIR}/pyenergyplus")
if(NOT EXISTS "${API_TARGET_DIR}")
  file(MAKE_DIRECTORY "${API_TARGET_DIR}")
endif()
set(API_SOURCE_DIR "${REPO_ROOT}/src/EnergyPlus/api")
# These two have CMake variable expansion in there
configure_file("${API_SOURCE_DIR}/api.py" "${API_TARGET_DIR}/api.py")
configure_file("${API_SOURCE_DIR}/func.py" "${API_TARGET_DIR}/func.py")
# These are copy only
configure_file("${API_SOURCE_DIR}/common.py" "${API_TARGET_DIR}/common.py" COPYONLY)
configure_file("${API_SOURCE_DIR}/datatransfer.py" "${API_TARGET_DIR}/datatransfer.py" COPYONLY)
#configure_file( "${API_SOURCE_DIR}/autosizing.py" "${API_TARGET_DIR}/autosizing.py" )
configure_file("${API_SOURCE_DIR}/runtime.py" "${API_TARGET_DIR}/runtime.py" COPYONLY)
configure_file("${API_SOURCE_DIR}/plugin.py" "${API_TARGET_DIR}/plugin.py" COPYONLY)
configure_file("${API_SOURCE_DIR}/state.py" "${API_TARGET_DIR}/state.py" COPYONLY)
configure_file("${API_SOURCE_DIR}/__init__.py" "${API_TARGET_DIR}/__init__.py" COPYONLY)
