# INTERFACE targets

# JSON for Modern C++ (header-only)
add_subdirectory(nlohmann)

# CLI11 (header-only, used for command-line parsing)
add_subdirectory(CLI)

# valijson (header-only, used for epJSON schema validation)
add_subdirectory(valijson)

# fast_float (header-only, used for fast string-to-number parsing)
add_subdirectory(fast_float)

# milo (header-only, used for fast number-to-string formatting)
add_subdirectory(milo)

# END of INTERFACE targets

set(all_third_party_targets "")

# SAM Is included up here to make sure it doesn't get the C++20 flags
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
target_compile_features(ssc PRIVATE cxx_std_11)
if(MSVC)
  target_compile_options(shared PRIVATE /wd4244)
endif()
list(APPEND all_third_party_targets ssc shared splinter)

# Make sure C++20 flag is kept at 20 after SSC
set(CMAKE_CXX_STANDARD 20)

set(INSTALL_GTEST CACHE BOOL OFF)
mark_as_advanced(INSTALL_GTEST)

set(RE2_BUILD_TESTING
    OFF
    CACHE BOOL "" FORCE)

# We don't want to enable the global project_warnings for any of the third party projects
add_subdirectory(SQLite)
set_target_properties(sqlite PROPERTIES FOLDER ThirdParty/SQLite)
# We don't want to enable the global project_fp_options for SQLite because it cannot compile with any of the checked FP options
target_link_libraries(sqlite PRIVATE project_options)
list(APPEND all_third_party_targets sqlite)

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

  list(APPEND all_third_party_targets gtest gtest_main gmock gmock_main)

  set(is_clang "$<CXX_COMPILER_ID:Clang>")
  set(v21_or_later "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,21>")
  set(is_clang_v21_or_later "$<AND:${is_clang},${v21_or_later}>")
  foreach(_gtest_target gtest gtest_main gmock gmock_main)
    if(TARGET ${_gtest_target})
      target_link_libraries(${_gtest_target}
        PRIVATE
          project_options project_fp_options
      )
      target_compile_options(${_gtest_target}
        PRIVATE
          "$<${is_clang_v21_or_later}:-Wno-character-conversion>"
          $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-deprecated-declarations>
      )
    endif()
  endforeach()
endif()

add_subdirectory(ObjexxFCL)
set_target_properties(objexx PROPERTIES FOLDER ThirdParty/ObjexxFCL)
target_link_libraries(objexx PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets objexx)

add_subdirectory(Expat)
set_target_properties(epexpat PROPERTIES FOLDER ThirdParty/Expat)
target_link_libraries(epexpat PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets epexpat)

add_subdirectory(BCVTB)
set_target_properties(bcvtb PROPERTIES FOLDER ThirdParty/BCVTB)
target_link_libraries(bcvtb PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets bcvtb)

add_subdirectory(FMI)
set_target_properties(epfmiimport PROPERTIES FOLDER ThirdParty/FMI)
target_link_libraries(epfmiimport PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets epfmiimport)

add_subdirectory(zlib)
set_target_properties(miniziplib PROPERTIES FOLDER ThirdParty/zlib)
target_link_libraries(miniziplib PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets miniziplib)

add_subdirectory(DElight)
set_target_properties(DElight PROPERTIES FOLDER ThirdParty/DElight)
target_link_libraries(DElight PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets DElight)

add_subdirectory(re2)
set_target_properties(re2 PROPERTIES FOLDER ThirdParty/re2)
target_link_libraries(re2 PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets re2)

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
target_link_libraries(libkiva PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets libkiva)
# libkiva exports "${kiva_SOURCE_DIR}/vendor/eigen" as a PUBLIC include, but our vendored Kiva
# has vendor/eigen removed: point it at the shared copy instead. PUBLIC because libkiva's public
# headers (e.g. Ground.hpp) include Eigen, so consumers (energypluslib) need it too.
# Note: ssc/splinter carries its own private, flattened Eigen 3.2.8 copy and does not use this one
# (upstream ssc removed it in favor of OR-Tools in 2026, see NatLabRockies/ssc#1367).
# SYSTEM is needed here: the add_subdirectory(third_party SYSTEM) property only makes interface
# includes system for *consumers*, while this Eigen dir must also be -isystem when libkiva
# compiles its own sources (old Eigen triggers -Wdeprecated-enum-enum-conversion under C++20).
target_include_directories(libkiva SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/eigen")
# EnergyPlus includes kiva headers with a <libkiva/...> prefix (e.g. HeatBalanceKivaManager.hh),
# which libkiva's own exported include dirs don't serve: export the parent dir for that spelling.
target_include_directories(libkiva PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/kiva/src")

if(NOT APPLE)
  add_subdirectory(FMUParser)
  set_target_properties(parser PROPERTIES FOLDER ThirdParty/FMI)
  target_link_libraries(parser PRIVATE project_options project_fp_options)
  list(APPEND all_third_party_targets parser)
endif()

add_subdirectory(Windows-CalcEngine)
set_target_properties(Windows-CalcEngine PROPERTIES FOLDER ThirdParty/Windows-CalcEngine)
target_link_libraries(Windows-CalcEngine PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets Windows-CalcEngine)

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
target_link_libraries(btwxt PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets btwxt)

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
  list(APPEND all_third_party_targets glfw glad penumbra tess2)
  foreach(_penumbra_target glfw glad penumbra tess2)
    target_link_libraries(${_penumbra_target}
      PRIVATE
      project_options project_fp_options
    )
  endforeach()
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
target_link_libraries(libtk205 PRIVATE project_options project_fp_options)
list(APPEND all_third_party_targets libtk205)

# unconditionnally turn off zlib and ssc
set(too_noisy_targets miniziplib ssc splinter shared)
foreach(target IN LISTS too_noisy_targets)
  target_link_libraries(${target} PRIVATE turn_off_warnings)
endforeach()

if(NOT ENABLE_WARNINGS_IN_THIRD_PARTY)
  foreach(target IN LISTS all_third_party_targets)
    if(TARGET ${target})
      target_link_libraries(${target} PRIVATE turn_off_warnings)
    endif()
  endforeach()
else()
  set(skip_libs
    ${too_noisy_targets}
    # Too noisy as well
    epexpat re2
    # vendored libraries
    glad glfw
  )

  add_library(skip_common_warnings INTERFACE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(skip_common_warnings
      INTERFACE
        -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function
        -Wno-suggest-override
    )
  endif()

  foreach(target IN LISTS all_third_party_targets)
    # Skip
    if("${target}" IN_LIST skip_libs)
      message(DEBUG "Skipping ${target} for project_warnings")
      continue()
    endif()

    if(TARGET ${target})
      target_link_libraries(${target} PRIVATE project_warnings warnings_as_not_error skip_common_warnings)
    endif()
  endforeach()
endif()
