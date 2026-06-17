# Linking up to Python is not necessarily complex, but it is tricky to get right.
# Our goal here is to provide a fully packaged embedded Python interpreter and Python 3.5+ standard library
# During build time, the built EnergyPlus executable tree stays linked against the Python wherever it is installed on the system.
# During install time, this file is used to copy over the Python dynamic library into the install tree
# In addition, on Windows, the proper Python filename is fixed up prior to copying.

# Prior to calling, set:
#  RESOLVED_PYTHON_LIB to the Python library file found through CMake's usual finder
#  EXECUTABLE_PATH which will be the executable directory in the INSTALL TREE, thus you need to use a generator expression

message("PYTHON: Collecting Python dynamic library")

# get some path info things
get_filename_component(BASE_PATH ${EXECUTABLE_PATH} DIRECTORY)
get_filename_component(PYTHON_LIB_FILENAME ${RESOLVED_PYTHON_LIB} NAME)

# on Windows, RESOLVED_PYTHON_LIB is pointing to the likes of `C:\Python38\libs\python38.lib`,
# when we actually want the DLL at `C:\Python38\python38.dll`
# need to fix that up here
if(WIN32)
  get_filename_component(LIB_SUB_DIR "${RESOLVED_PYTHON_LIB}" DIRECTORY)
  get_filename_component(PYTHON_ROOT_DIR "${LIB_SUB_DIR}" DIRECTORY)
  string(REPLACE ".lib" ".dll" DLL_FILE_NAME "${PYTHON_LIB_FILENAME}")
  set(RESOLVED_PYTHON_LIB "${PYTHON_ROOT_DIR}/${DLL_FILE_NAME}")
endif()

# then copy the DLL in
message("COPYING ${RESOLVED_PYTHON_LIB} to ${BASE_PATH}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${RESOLVED_PYTHON_LIB}" "${BASE_PATH}")
