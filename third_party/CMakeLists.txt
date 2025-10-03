#### Create interface for JSON for Modern C++ so we can include it in targets
# add_library( ModernJSON INTERFACE )
# target_include_directories( ModernJSON INTERFACE "${PROJECT_SOURCE_DIR}/nlohmann" "${PROJECT_SOURCE_DIR}/doj" )

add_library( nlohmann_json INTERFACE )
target_compile_definitions(
        nlohmann_json
        INTERFACE
        JSON_USE_IMPLICIT_CONVERSIONS=1
)
if(NOT MSVC)  # This is really only needed with newer Clang, but gcc is fine with it
  target_compile_options(nlohmann_json INTERFACE -Wno-deprecated-declarations)
endif()
target_include_directories(
        nlohmann_json
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/nlohmann>
)
set_source_files_properties(
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/nlohmann>/json.hpp
  TARGET_DIRECTORY nlohmann_json
  PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations
)

# To avoid modifying every third_party library
# we link the project_options in here for every library in this
# subdirectory

# SAM Is included up here to make sure it doesn't get the C++17 flags

# SAM Simulation Core (SSC)
mark_as_advanced(FORCE JSONCPP_USE_SECURE_MEMORY)
mark_as_advanced(FORCE JSONCPP_WITH_CMAKE_PACKAGE)
mark_as_advanced(FORCE JSONCPP_WITH_EXAMPLE)
mark_as_advanced(FORCE JSONCPP_WITH_PKGCONFIG_SUPPORT)
mark_as_advanced(FORCE JSONCPP_WITH_STRICT_ISO)
mark_as_advanced(FORCE JSONCPP_WITH_TESTS)
set(JSONCPP_WITH_TESTS
    OFF
    CACHE BOOL "Compile and (for jsoncpp_check) run JsonCpp test executables" FORCE)
mark_as_advanced(FORCE JSONCPP_WITH_WARNING_AS_ERROR)
mark_as_advanced(FORCE DEBUG_LIBNAME_SUFFIX)
mark_as_advanced(FORCE JSONCPP_WITH_POST_BUILD_UNITTEST)
set(JSONCPP_WITH_POST_BUILD_UNITTEST
    OFF
    CACHE BOOL "Automatically run unit-tests as a post build step" FORCE)
mark_as_advanced(FORCE CCACHE_FOUND)
set(SAM_SKIP_TOOLS
    ON
    CACHE BOOL "Skips the sdktool and tcsconsole builds" FORCE)
mark_as_advanced(FORCE SAM_SKIP_TOOLS)
set(SAM_SKIP_TESTS
    ON
    CACHE BOOL "Skips building tests" FORCE)
mark_as_advanced(FORCE SAM_SKIP_TESTS)
set(SAMAPI_EXPORT
    OFF
    CACHE BOOL "Export of ssc binaries to the SAM_api directory; for Unix, compile ssc libraries for SAM_api" FORCE)
mark_as_advanced(FORCE SAMAPI_EXPORT)
add_subdirectory(ssc)
set_target_properties(ssc PROPERTIES FOLDER ThirdParty/ssc)
set_target_properties(shared PROPERTIES FOLDER ThirdParty/ssc)
set_target_properties(splinter PROPERTIES FOLDER ThirdParty/ssc)

if(CMAKE_COMPILER_IS_GNUCXX
   OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
   OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") # g++/Clang
  target_compile_options(splinter PRIVATE -Wno-error -w)
  target_compile_options(shared PRIVATE -Wno-error -w)
  target_compile_options(ssc PRIVATE -Wno-error -w)
elseif(MSVC) # VisualStudio
  target_compile_options(splinter PRIVATE /W0)
  target_compile_options(shared PRIVATE /W0 /wd4244)
  target_compile_options(ssc PRIVATE /W0)
endif()
target_compile_features(ssc PRIVATE cxx_std_11)

# C++17 flag is intentionally after ssc
set(CMAKE_CXX_STANDARD 17)

#Note: the Tarcog files are improperly initializing their virtual base classes, and this 4589 warning
# on MSVC is legitimate, but silencing the warning because it's a third-party library

# Shush every warning for any lib added after it?
# link_libraries(turn_off_warnings)

if(NOT MSVC)
  add_compile_options(-Wno-pedantic -Wno-unused-parameter -Wno-unknown-pragmas)
else()
  add_compile_options(/wd4267 /wd4996 /wd4068 /wd4244 /wd4589)
endif()

set(INSTALL_GTEST CACHE BOOL OFF)
mark_as_advanced(INSTALL_GTEST)

set(RE2_BUILD_TESTING
    OFF
    CACHE BOOL "" FORCE)

# Don't force Penumbra testing just because OpenGL is found, allow the developer to set it to ON or OFF as they want
#if(OPENGL_FOUND)
#  set(BUILD_PENUMBRA_TESTING
#      ${BUILD_TESTING}
#      CACHE BOOL "" FORCE)
#endif()

# We don't want to enable the global project_warnings for any of the third party projects
link_libraries(project_options)
add_subdirectory(SQLite)
set_target_properties(sqlite PROPERTIES FOLDER ThirdParty/SQLite)

# Then add in the FP options after SQLite project
# because it cannot compile with any of the checked FP options
link_libraries(project_fp_options)

if(BUILD_TESTING)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  set(BUILD_GTEST
      ON
      CACHE BOOL "" FORCE)
  set(BUILD_GMOCK
      ON
      CACHE BOOL "" FORCE)
  option(TEST_ANNUAL_SIMULATION "Use annual simulations for tests instead of only design days" OFF)
  enable_testing()
  add_subdirectory(gtest)
  set_target_properties(gtest PROPERTIES FOLDER ThirdParty/gtest)
  set_target_properties(gtest_main PROPERTIES FOLDER ThirdParty/gtest)
  set_target_properties(gmock PROPERTIES FOLDER ThirdParty/gtest)
  set_target_properties(gmock_main PROPERTIES FOLDER ThirdParty/gtest)
endif()

add_subdirectory(ObjexxFCL)
set_target_properties(objexx PROPERTIES FOLDER ThirdParty/ObjexxFCL)

add_subdirectory(BCVTB)
set_target_properties(bcvtb PROPERTIES FOLDER ThirdParty/BCVTB)

add_subdirectory(Expat)
set_target_properties(epexpat PROPERTIES FOLDER ThirdParty/Expat)

add_subdirectory(FMI)
set_target_properties(epfmiimport PROPERTIES FOLDER ThirdParty/FMI)
target_link_libraries(epfmiimport PRIVATE turn_off_warnings)

add_subdirectory(zlib)
set_target_properties(miniziplib PROPERTIES FOLDER ThirdParty/zlib)
target_link_libraries(miniziplib PRIVATE turn_off_warnings)

add_subdirectory(DElight)
set_target_properties(DElight PROPERTIES FOLDER ThirdParty/DElight)
target_link_libraries(DElight PRIVATE turn_off_warnings)

add_subdirectory(re2)
set_target_properties(re2 PROPERTIES FOLDER ThirdParty/re2)

add_subdirectory(fmt-8.0.1)
target_compile_definitions(fmt PRIVATE FMT_USE_FULL_CACHE_DRAGONBOX=1)
set_target_properties(fmt PROPERTIES FOLDER ThirdParty/fmt)
mark_as_advanced(FORCE FMT_SYSTEM_HEADERS)

# Kiva
option(BUILD_GROUND_PLOT "Build ground plotting library (for Kiva debugging only)" OFF)
mark_as_advanced(FORCE BUILD_GROUND_PLOT)
mark_as_advanced(FORCE KIVA_WERROR)

set(KIVA_TESTING OFF CACHE BOOL "" FORCE)
set(KIVA_EXE_BUILD OFF CACHE BOOL "" FORCE)
set(KIVA_3D OFF CACHE BOOL "" FORCE)
set(KIVA_GROUND_PLOT ${BUILD_GROUND_PLOT} CACHE BOOL "" FORCE)
set(KIVA_COVERAGE OFF CACHE BOOL "" FORCE)

add_subdirectory(kiva)
set_target_properties(libkiva PROPERTIES FOLDER ThirdParty/Kiva)

if(NOT APPLE)
  add_subdirectory(FMUParser)
  set_target_properties(parser PROPERTIES FOLDER ThirdParty/FMI)
  if(NOT MSVC)
    target_compile_options(parser PRIVATE -Wno-switch)
  endif()
endif()

add_subdirectory(Windows-CalcEngine)
set_target_properties(Windows-CalcEngine PROPERTIES FOLDER ThirdParty/Windows-CalcEngine)

# Btwxt
include_directories(${gtest_SOURCE_DIR}/include/ SYSTEM)
set(BUILD_BTWXT_TESTING
    OFF
    CACHE BOOL "" FORCE)
add_subdirectory(btwxt)
set_target_properties(btwxt PROPERTIES FOLDER ThirdParty/Btwxt)
mark_as_advanced(FORCE btwxt_STATIC_LIB)
mark_as_advanced(FORCE btwxt_COVERAGE)
mark_as_advanced(FORCE btwxt_BUILD_TESTING)

if(OPENGL_FOUND)
  set(BUILD_PENUMBRA_TESTING
      OFF
      CACHE BOOL "" FORCE)
  add_subdirectory(penumbra)
  set_target_properties(glfw PROPERTIES FOLDER ThirdParty/Penumbra/GLFW)
  set_target_properties(glad PROPERTIES FOLDER ThirdParty/Penumbra)
  set_target_properties(penumbra PROPERTIES FOLDER ThirdParty/Penumbra)
  set_target_properties(tess2 PROPERTIES FOLDER ThirdParty/Penumbra)
  mark_as_advanced(FORCE penumbra_STATIC_LIB)
  mark_as_advanced(FORCE penumbra_COVERAGE)
endif()

# libtk205
set(LIBTK205_BUILD_TESTING
        OFF
        CACHE BOOL "" FORCE)
set(libtk205_USE_SUBMODULES
        OFF
        CACHE BOOL "" FORCE)
add_subdirectory(libtk205)
mark_as_advanced(FORCE libtk205_STATIC_LIB)
mark_as_advanced(FORCE libtk205_COVERAGE)
mark_as_advanced(FORCE libtk205_BUILD_TESTING)

