######################################################################################################################################################
#                                                        D E F A U L T    G E N E R A T O R S                                                        #
######################################################################################################################################################

# Default the Binary generators: problem is that none of the CPACK_BINARY_<GenName> will show up in CMakeCache,
# which makes it less clear what will happen in terms of package generation
#if(WIN32)
#set(CPACK_GENERATOR "IFW;ZIP")
#elseif(APPLE)
#set(CPACK_GENERATOR "IFW;TGZ")
#elseif(UNIX)
#set(CPACK_GENERATOR "IFW;STGZ;TGZ")
#endif()

# So instead, let's cache the default value we want for the individual options for CPACK_BINARY_<GenName>
if(UNIX)

  # Set everything to off for now
  set(CPACK_BINARY_DEB OFF CACHE BOOL "Recommended OFF")
  set(CPACK_BINARY_FREEBSD OFF CACHE BOOL "Recommended OFF")
  set(CPACK_BINARY_RPM OFF CACHE BOOL "Recommended OFF")
  set(CPACK_BINARY_TBZ2 OFF CACHE BOOL "Recommended OFF")
  set(CPACK_BINARY_NSIS OFF CACHE BOOL "Recommended OFF")

  if(APPLE)
    set(CPACK_BINARY_IFW ON CACHE BOOL "Enable to build IFW package, which is the recommended method")
    set(CPACK_BINARY_STGZ OFF CACHE BOOL "Recommended OFF")

    # Mac Specific options to turn off
    set(CPACK_BINARY_BUNDLE OFF CACHE BOOL "Recommended OFF")
    set(CPACK_BINARY_DRAGNDROP OFF CACHE BOOL "Recommended OFF")
    set(CPACK_BINARY_OSXX11 OFF CACHE BOOL "Recommended OFF")
    set(CPACK_BINARY_PACKAGEMAKER OFF CACHE BOOL "This was the legacy method on Apple, superseded by IFW")
    set(CPACK_BINARY_PRODUCTBUILD OFF CACHE BOOL "Recommended OFF")

  else()
    set(CPACK_BINARY_IFW ON CACHE BOOL "Enable to build IFW package, which is the recommended method")
    set(CPACK_BINARY_STGZ ON CACHE BOOL "Enable to build a Linux sh installer script, which is the legacy method")

    # Unix (non Apple CACHE BOOL) specific option to turn off
    set(CPACK_BINARY_TZ OFF CACHE BOOL "Recommended OFF")
  endif()

  # Tar.gz for inclusion in other programs for eg
  set(CPACK_BINARY_TGZ ON CACHE BOOL "Enable to build a tar.gz package, recommended for an official release")

elseif(WIN32)
  set(CPACK_BINARY_IFW ON CACHE BOOL "Enable to build IFW package, which is the recommend method")
  set(CPACK_BINARY_ZIP ON CACHE BOOL "Enable to build a ZIP package, recommended for an official release")

  set(CPACK_BINARY_NSIS OFF CACHE BOOL "This was the legacy method on Windows, superseded by IFW")
  set(CPACK_BINARY_7Z OFF CACHE BOOL "Recommended OFF")
  set(CPACK_BINARY_NUGET OFF CACHE BOOL "Recommended OFF")
  set(CPACK_BINARY_WIX OFF CACHE BOOL "Recommended OFF")

endif()

# Turn off source generators
# Need a list, which can't be empty, but not have sensible defined value. So a list of two empty element works as
# a workaround
# list(CPACK_SOURCE_GENERATOR ";")

# Instead use indiv CPACK_SOURCE_<GenName>: all to OFF
if(UNIX)

  set(CPACK_SOURCE_RPM OFF CACHE BOOL "Recommended OFF")
  set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "Recommended OFF")
  set(CPACK_SOURCE_TGZ OFF CACHE BOOL "Recommended OFF")
  set(CPACK_SOURCE_TXZ OFF CACHE BOOL "Recommended OFF")
  set(CPACK_SOURCE_TZ OFF CACHE BOOL "Recommended OFF")
  set(CPACK_SOURCE_ZIP OFF CACHE BOOL "Recommended OFF")

elseif(WIN32)

  set(CPACK_SOURCE_7Z OFF CACHE BOOL "Recommended OFF")
  set(CPACK_SOURCE_ZIP OFF CACHE BOOL "Recommended OFF")
endif()

######################################################################################################################################################
#                                              B A S E    I N S T A L L   &    P R O J E C T    I N F O                                              #
######################################################################################################################################################

# Base install
set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};EnergyPlus;ALL;/")

if(BUILD_FORTRAN)
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/ExpandObjects/;ExpandObjects;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/ReadVars/;ReadVars;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/Transition/;Transition;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/Basement/;Basement;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/HVAC-Diagram/;HVAC-Diagram;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/ParametricPreprocessor/;ParametricPreprocessor;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/Slab/;Slab;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/ConvertESOMTR/;ConvertESOMTR;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/CalcSoilSurfTemp/;CalcSoilSurfTemp;ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR}/src/AppGPostProcess/;AppGPostProcess;ALL;/")
endif()

# Need to install the ssc lib...
# install(TARGETS ssc DESTINATION ./)

set(CPACK_PACKAGE_VENDOR "US Department of Energy")
set(CPACK_IFW_PACKAGE_PUBLISHER "${CPACK_PACKAGE_VENDOR}")

set(CPACK_PACKAGE_CONTACT "Edwin Lee <edwin.lee@nrel.gov>")
set(CPACK_PACKAGE_DESCRIPTION
    "EnergyPlus is a whole building energy simulation program that engineers, architects, and researchers use to model both energy consumption and water use in buildings."
)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "EnergyPlus is a whole building energy simulation program.")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/Modules")

set(CPACK_PACKAGE_VERSION_MAJOR "${CMAKE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CMAKE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CMAKE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_BUILD "${CMAKE_VERSION_BUILD}")

set(CPACK_PACKAGE_VERSION
    "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CPACK_PACKAGE_VERSION_BUILD}")
# Default the debian package name to include version to allow several versions to be installed concurrently instead of overwriting any existing one
# set(CPACK_DEBIAN_PACKAGE_NAME "energyplus-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_IFW_PRODUCT_URL "https://www.energyplus.net")
# set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.energyplus.net")

include(cmake/TargetArch.cmake)
target_architecture(TARGET_ARCH)

# Debug

#cmake_host_system_information(RESULT _OS_NAME QUERY OS_NAME)
#message("-- OS_NAME variable is set to: " ${_OS_NAME})

#cmake_host_system_information(RESULT _OS_RELEASE QUERY OS_RELEASE)
#message("-- OS_RELEASE variable is set to: " ${_OS_RELEASE})

#cmake_host_system_information(RESULT _OS_VERSION QUERY OS_VERSION)
#message("-- OS_VERSION variable is set to: " ${_OS_VERSION})

#cmake_host_system_information(RESULT _OS_PLATFORM QUERY OS_PLATFORM)
#message("-- OS_PLATFORM variable is set to: " ${_OS_PLATFORM})

#message("CMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}")
#message("CMAKE_SYSTEM_NAME =${CMAKE_SYSTEM_NAME}")
#if(MSVC)
#message("CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
#message("MSVC_VERSION =${MSVC_VERSION}")
#endif()

# Mac
#-- OS_NAME variable is set to: Mac OS X
#-- OS_RELEASE variable is set to: 10.14.6
#-- OS_VERSION variable is set to: 18G2022
#-- OS_PLATFORM variable is set to: x86_64
# CMAKE_SYSTEM_VERSION=18.7.0
# CMAKE_SYSTEM_NAME =Darwin

# Ubuntu
#-- OS_NAME variable is set to: Linux
#-- OS_RELEASE variable is set to: 5.4.0-42-generic
#-- OS_VERSION variable is set to: #46~18.04.1-Ubuntu SMP Fri Jul 10 07:21:24 UTC 2020
#-- OS_PLATFORM variable is set to: x86_64
# CMAKE_SYSTEM_VERSION=5.3.0-51-generic
# CMAKE_SYSTEM_NAME =Linux

# Windows
#-- OS_NAME variable is set to: Windows
#-- OS_RELEASE variable is set to:  Professional
#-- OS_VERSION variable is set to:  (Build 19041)
#-- OS_PLATFORM variable is set to: AMD64
# CMAKE_SYSTEM_VERSION=10.0.19041
# CMAKE_SYSTEM_NAME=Windows
# CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=10.0.17763.0
# MSVC_VERSION=1926

#
# End debug

include(cmake/SystemDetails.cmake)
set_system_nickname()
if(APPLE)
  # eg: '-macOS10.13'
  set(SYSTEM_VERSION "-${SYSTEM_NICKNAME}")
elseif(UNIX)
  # eg: `-Ubuntu18.04`
  set(SYSTEM_VERSION "-${SYSTEM_NICKNAME}")
elseif(MSVC)
  # no-op
  set(SYSTEM_VERSION "")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}${SYSTEM_VERSION}-${TARGET_ARCH}")
  if (ENABLE_HARDENED_RUNTIME)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-HardenedRuntime")
  endif()
else()
  set(CPACK_PACKAGE_FILE_NAME
      "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}${SYSTEM_VERSION}-${TARGET_ARCH}-${CMAKE_BUILD_TYPE}")
endif()

message("Installer name is set to '${CPACK_PACKAGE_FILE_NAME}'")

# Installation directory on the target system (common to all CPack Genrators)
set(CPACK_PACKAGE_INSTALL_DIRECTORY
    "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}-${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}")

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE.txt" DESTINATION "./" COMPONENT Licenses)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.idd" DESTINATION ./)
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.schema.epJSON" DESTINATION ./)

#################################################################  A U T O D O C S  ##################################################################

# Some docs are generated on the fly here, create a dir for the 'built' files
set(DOCS_OUT "${PROJECT_BINARY_DIR}/autodocs")
# This is NOT an install command, we actually want it to be performed so we can generate the package, so do it at build system generation
file(MAKE_DIRECTORY ${DOCS_OUT})

# the output variables listing
install(
  CODE "execute_process(COMMAND \"${Python_EXECUTABLE}\" \"${PROJECT_SOURCE_DIR}/doc/tools/parse_output_variables.py\" \"${PROJECT_SOURCE_DIR}/src/EnergyPlus\" \"${DOCS_OUT}/SetupOutputVariables.csv\" \"${DOCS_OUT}/SetupOutputVariables.md\")"
)
install(FILES "${PROJECT_BINARY_DIR}/autodocs/SetupOutputVariables.csv" DESTINATION "./")

# the example file summary
install(
  CODE "execute_process(COMMAND \"${Python_EXECUTABLE}\" \"${PROJECT_SOURCE_DIR}/doc/tools/example_file_summary.py\" \"${PROJECT_SOURCE_DIR}/testfiles\" \"${DOCS_OUT}/ExampleFiles.html\")"
  COMPONENT ExampleFiles)
install(FILES "${DOCS_OUT}/ExampleFiles.html" DESTINATION "./ExampleFiles/" COMPONENT ExampleFiles)

# the example file objects link
install(CODE "execute_process(COMMAND \"${Python_EXECUTABLE}\" \"${PROJECT_SOURCE_DIR}/doc/tools/example_file_objects.py\"
\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.idd\" \"${PROJECT_SOURCE_DIR}/testfiles\" \"${DOCS_OUT}/ExampleFiles-ObjectsLink.html\")"
        COMPONENT ExampleFiles)
install(FILES "${DOCS_OUT}/ExampleFiles-ObjectsLink.html" DESTINATION "./ExampleFiles/" COMPONENT ExampleFiles)

option(BUILD_CHANGELOG "Build a changelog for this package -- requires GITHUB_TOKEN in environment" OFF)
mark_as_advanced(FORCE BUILD_CHANGELOG)
if(BUILD_CHANGELOG)
  # build the change log, only if we do have a github token in the environment
  # Watch out! GITHUB_TOKEN could go out of scope by the time install target is run.
  # Better to move this condition into the install CODE.
  if(NOT "$ENV{GITHUB_TOKEN}" STREQUAL "")
    install(
      CODE "execute_process(COMMAND \"${Python_EXECUTABLE}\" \"${PROJECT_SOURCE_DIR}/doc/tools/create_changelog.py\" \"${PROJECT_SOURCE_DIR}\" \"${DOCS_OUT}/changelog.md\" \"${DOCS_OUT}/changelog.html\" \"${GIT_EXECUTABLE}\" \"$ENV{GITHUB_TOKEN}\" \"${PREV_RELEASE_SHA}\" \"${CPACK_PACKAGE_VERSION}\")"
    )
    install(FILES "${DOCS_OUT}/changelog.html" DESTINATION "./" OPTIONAL)
  else()
    message(WARNING "No GITHUB_TOKEN found in environment; package won't include the change log")
  endif()
endif() # BUILD_CHANGELOG

#################################################################  D A T A S E T S  ##################################################################

# Install files that are in the current repo
install(FILES "${PROJECT_SOURCE_DIR}/datasets/AirCooledChiller.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/ASHRAE_2005_HOF_Materials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Boilers.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/California_Title_24-2008.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Chillers.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/CodeCompliantEquipment.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/CompositeWallConstructions.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/DXCoolingCoil.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/ElectricGenerators.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/ElectricityUSAEnvironmentalImpactFactors.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/ElectronicEnthalpyEconomizerCurves.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/ExhaustFiredChiller.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/FluidPropertiesRefData.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/FossilFuelEnvironmentalImpactFactors.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/GLHERefData.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/GlycolPropertiesRefData.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2012.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2013.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2014.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2015.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2016.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2017.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2018.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2019.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2020.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2021.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/LCCusePriceEscalationDataSet2022.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/MoistureMaterials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/PerfCurves.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/PrecipitationSchedulesUSA.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/RefrigerationCasesDataSet.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/RefrigerationCompressorCurves.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/ResidentialACsAndHPsPerfCurves.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/RooftopPackagedHeatPump.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/SandiaPVdata.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Schedules.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/SolarCollectors.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/StandardReports.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/SurfaceColorSchemes.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/USHolidays-DST.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WaterToAirHeatPumps.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Window5DataFile.dat" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WindowBlindMaterials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WindowConstructs.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WindowGasMaterials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WindowGlassMaterials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WindowScreenMaterials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/WindowShadeMaterials.idf" DESTINATION "./DataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/FMUs/MoistAir.fmu" DESTINATION "./DataSets/FMUs" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/FMUs/ShadingController.fmu" DESTINATION "./DataSets/FMUs" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/TDV/TDV_2008_kBtu_CTZ06.csv" DESTINATION "./DataSets/TDV" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/TDV/TDV_read_me.txt" DESTINATION "./DataSets/TDV" COMPONENT Datasets)

install(FILES "${PROJECT_SOURCE_DIR}/datasets/Macro/Locations-DesignDays.xls" DESTINATION "./MacroDataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Macro/SandiaPVdata.imf" DESTINATION "./MacroDataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Macro/SolarCollectors.imf" DESTINATION "./MacroDataSets" COMPONENT Datasets)
install(FILES "${PROJECT_SOURCE_DIR}/datasets/Macro/UtilityTariffObjects.imf" DESTINATION "./MacroDataSets" COMPONENT Datasets)

#############################################################  W E A T H E R    D A T A  #############################################################

# weather files
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_CA_San.Francisco.Intl.AP.724940_TMY3.ddy" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_CA_San.Francisco.Intl.AP.724940_TMY3.stat" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_CO_Golden-NREL.724666_TMY3.ddy" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_CO_Golden-NREL.724666_TMY3.epw" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_CO_Golden-NREL.724666_TMY3.stat" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_FL_Tampa.Intl.AP.722110_TMY3.ddy" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_FL_Tampa.Intl.AP.722110_TMY3.epw" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_FL_Tampa.Intl.AP.722110_TMY3.stat" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.ddy" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.stat" DESTINATION "./WeatherData" COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_VA_Sterling-Washington.Dulles.Intl.AP.724030_TMY3.ddy" DESTINATION "./WeatherData"
        COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_VA_Sterling-Washington.Dulles.Intl.AP.724030_TMY3.epw" DESTINATION "./WeatherData"
        COMPONENT WeatherData)
install(FILES "${PROJECT_SOURCE_DIR}/weather/USA_VA_Sterling-Washington.Dulles.Intl.AP.724030_TMY3.stat" DESTINATION "./WeatherData"
        COMPONENT WeatherData)

#############################################################   E X A M P L E    F I L E S   #########################################################

install(
  DIRECTORY testfiles/
  DESTINATION ExampleFiles/
  COMPONENT ExampleFiles
  PATTERN _* EXCLUDE
  PATTERN *.ddy EXCLUDE
  PATTERN CMakeLists.txt EXCLUDE
  PATTERN performance EXCLUDE)

#############################################################   M I S C E L L A N E O U S   ##########################################################

# TODO Remove version from file name or generate
# These files names are stored in variables because they also appear as start menu shortcuts later.
install(FILES "${PROJECT_SOURCE_DIR}/release/Bugreprt.txt" DESTINATION "./")
install(FILES "${PROJECT_SOURCE_DIR}/release/favicon.png" DESTINATION "./")
configure_file("${PROJECT_SOURCE_DIR}/release/readme.in.html" "${PROJECT_BINARY_DIR}/release/readme.html" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/release/readme.html" DESTINATION "./")
configure_file("${PROJECT_SOURCE_DIR}/release/Deprecation.in.html" "${PROJECT_BINARY_DIR}/release/Deprecation.html" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/release/Deprecation.html" DESTINATION "./")
if(LINK_WITH_PYTHON)
  install(FILES "${PROJECT_SOURCE_DIR}/release/PythonLicense.txt" DESTINATION "./")
endif()
set(CPACK_RESOURCE_FILE_README "${PROJECT_BINARY_DIR}/release/readme.html")

install(FILES "${PROJECT_SOURCE_DIR}/bin/CurveFitTools/IceStorageCurveFitTool.xlsm" DESTINATION "PreProcess/HVACCurveFitTool/")
install(FILES "${PROJECT_SOURCE_DIR}/bin/CurveFitTools/CurveFitTool.xlsm" DESTINATION "PreProcess/HVACCurveFitTool/")
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater/" RENAME "V${CMAKE_VERSION_MAJOR}-${CMAKE_VERSION_MINOR}-${CMAKE_VERSION_PATCH}-Energy+.idd")

# Workflow stuff, takes about 40KB, so not worth it proposing to not install it
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/workflows/app_g_postprocess.py" DESTINATION "workflows/") # COMPONENT Workflows)
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/workflows/calc_soil_surface_temp.py" DESTINATION "workflows/") # COMPONENT Workflows)
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/workflows/coeff_check.py" DESTINATION "workflows/") # COMPONENT Workflows)
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/workflows/coeff_conv.py" DESTINATION "workflows/") # COMPONENT Workflows)
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/workflows/energyplus.py" DESTINATION "workflows/") # COMPONENT Workflows)
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/workflows/transition.py" DESTINATION "workflows/") # COMPONENT Workflows)

######################################################################################################################################################
#                                                         P L A T F O R M    S P E C I F I C                                                         #
######################################################################################################################################################

if(WIN32)
  # calcsoilsurftemp is now built from source, just need to install the batch run script
  install(FILES "${PROJECT_SOURCE_DIR}/src/CalcSoilSurfTemp/RunCalcSoilSurfTemp.bat" DESTINATION "PreProcess/CalcSoilSurfTemp/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EP-Launch/EP-Launch.exe" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/scripts/Epl-run.bat" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/scripts/RunDirMulti.bat" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/release/RunEP.ico" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/scripts/RunEPlus.bat" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/scripts/RunReadESO.bat" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/release/Runep.pif" DESTINATION "./")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CSVProc/CSVproc.exe" DESTINATION "PostProcess/")
  install(FILES "${PROJECT_SOURCE_DIR}/scripts/RunReadESO.bat" DESTINATION "PostProcess/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/CoeffCheck.exe" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/CoeffCheckExample.cci" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/CoeffConv.exe" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/CoeffConvExample.coi" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/EPL-Check.BAT" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/EPL-Conv.BAT" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/CoeffConv/ReadMe.txt" DESTINATION "PreProcess/CoeffConv/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Basement/RunBasement.bat" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Slab/RunSlab.bat" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFEditor/IDFEditor.exe" DESTINATION "PreProcess/IDFEditor/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/ParametricPreprocessor/RunParam.bat" DESTINATION "PreProcess/ParametricPreprocessor/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/ViewFactorCalculation/readme.txt" DESTINATION "PreProcess/ViewFactorCalculation/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/ViewFactorCalculation/View3D.exe" DESTINATION "PreProcess/ViewFactorCalculation/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/ViewFactorCalculation/View3D32.pdf" DESTINATION "PreProcess/ViewFactorCalculation/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/ViewFactorCalculation/ViewFactorInterface.xls" DESTINATION "PreProcess/ViewFactorCalculation/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/Abbreviations.csv" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/ASHRAE_2013_Monthly_DesignConditions.csv" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/ASHRAE_2013_OtherMonthly_DesignConditions.csv" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/ASHRAE_2013_Yearly_DesignConditions.csv" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/Cal Climate Zone Lat Long data.csv" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/CountryCodes.txt" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/EPlusWth.dll" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/libifcoremd.dll" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/libifportmd.dll" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/libmmd.dll" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/svml_dispmd.dll" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/TimeZoneCodes.txt" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/WBANLocations.csv" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/WeatherConverter/Weather.exe" DESTINATION "PreProcess/WeatherConverter/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EP-Compare/readme.txt" DESTINATION "PostProcess/EP-Compare/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/EPDrawGUI Libs/Appearance Pak.dll" DESTINATION "PreProcess/EPDraw/EPDrawGUI Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/EPDrawGUI Libs/Shell.dll" DESTINATION "PreProcess/EPDraw/EPDrawGUI Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/EPDrawGUI.exe" DESTINATION "PreProcess/EPDraw/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/EPlusDrw.dll" DESTINATION "PreProcess/EPDraw/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/libifcoremd.dll" DESTINATION "PreProcess/EPDraw/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/libifportmd.dll" DESTINATION "PreProcess/EPDraw/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/libmmd.dll" DESTINATION "PreProcess/EPDraw/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EPDraw/Run-Win/svml_dispmd.dll" DESTINATION "PreProcess/EPDraw/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Basement/basementexample.audit" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Basement/basementexample.csv" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Basement/BasementExample.idf" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Basement/basementexample.out" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Basement/basementexample_out.idf" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Slab/slabexample.ger" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Slab/slabexample.gtp" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/src/Slab/SlabExample.idf" DESTINATION "PreProcess/GrndTempCalc/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/IDFVersionUpdater Libs/Appearance Pakx64.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/IDFVersionUpdater Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/IDFVersionUpdater Libs/Cryptox64.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/IDFVersionUpdater Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/IDFVersionUpdater Libs/GZipx64.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/IDFVersionUpdater Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/IDFVersionUpdater Libs/Internet Encodingsx64.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/IDFVersionUpdater Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/IDFVersionUpdater Libs/Shellx64.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/IDFVersionUpdater Libs/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/icudt65.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/icuin65.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/icuuc65.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/msvcp120.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/msvcp140.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/msvcr120.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/vccorlib140.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/vcruntime140.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/vcruntime140_1.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/XojoGUIFramework64.dll"
          DESTINATION "PreProcess/IDFVersionUpdater/")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Win/IDFVersionUpdater.exe" DESTINATION "PreProcess/IDFVersionUpdater/")
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/EPMacro/Windows/EPMacro.exe" DESTINATION "./")

  # This copies system DLLs into a temp folder. It is later used by the install script of this specific component
  # to check if the dll isn't present on the target system, in which case it will copy it to the system folder (eg: C:\Windows\SysWOW64\)
  # and use the regsvr32.exe to register said DLL.
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/ComDlg32.OCX" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Dforrt.dll" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Graph32.ocx" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Gsw32.exe" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Gswdll32.dll" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/MSCOMCTL.OCX" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Msflxgrd.ocx" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/MSINET.OCX" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Msvcrtd.dll" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/System/Vsflex7L.ocx" DESTINATION "./temp/" COMPONENT CopyAndRegisterSystemDLLs)
endif()

# The group, which will be used to configure the root package
# set(CPACK_IFW_PACKAGE_GROUP "EnergyPlus")
#set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH 640)
#set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT 480)
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${PROJECT_SOURCE_DIR}/release/ep_nobg.png")

set(CPACK_IFW_VERBOSE ON)

if(APPLE)
  set(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications")
  set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  set(CPACK_IFW_TARGET_DIRECTORY
      "/Applications/${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}-${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}")

  install(DIRECTORY "${PROJECT_SOURCE_DIR}/bin/EP-Launch-Lite/EP-Launch-Lite.app" DESTINATION "PreProcess")
  install(DIRECTORY "${PROJECT_SOURCE_DIR}/bin/IDFVersionUpdater/Run-Mac/IDFVersionUpdater.app" DESTINATION "PreProcess/IDFVersionUpdater")
  install(FILES "${PROJECT_SOURCE_DIR}/bin/EP-Compare/readme.txt" DESTINATION "PostProcess/EP-Compare/")
  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/EPMacro/Mac/EPMacro" DESTINATION "./")

  configure_file(scripts/runenergyplus.in "${PROJECT_BINARY_DIR}/scripts/runenergyplus" @ONLY)
  install(PROGRAMS "${PROJECT_BINARY_DIR}/scripts/runenergyplus" DESTINATION "./")
  install(PROGRAMS scripts/runepmacro DESTINATION "./")
  install(PROGRAMS scripts/runreadvars DESTINATION "./")

  # You need at least one "install(..." command for it to be registered as a component
  install(CODE "MESSAGE(\"Creating symlinks.\")" COMPONENT Symlinks)
  install(FILES "${PROJECT_SOURCE_DIR}/doc/man/energyplus.1" DESTINATION "./" COMPONENT Symlinks)

  # Custom installer icon. Has to be .icns on mac, .ico on windows, not supported on Unix
  set(CPACK_IFW_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/release/ep.icns")
elseif(WIN32)

  # Will also set CPACK_IFW_PACKAGE_START_MENU_DIRECTORY (name of default program group in Windows start menu)
  set(CPACK_IFW_PACKAGE_NAME "EnergyPlusV${CPACK_PACKAGE_VERSION_MAJOR}-${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}")

  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_IFW_PACKAGE_NAME}")
  set(CPACK_IFW_TARGET_DIRECTORY "C:/${CPACK_PACKAGE_INSTALL_DIRECTORY}")

  # Custom installer icon. Has to be .icns on mac, .ico on windows, not supported on Unix
  set(CPACK_IFW_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/release/ep.ico")

  # You need at least one "install(..." command for it to be registered as a component
  install(CODE "MESSAGE(\"Registering filetypes.\")" COMPONENT RegisterFileType)
  install(CODE "MESSAGE(\"Copying and Registering DLLs\")" COMPONENT CopyAndRegisterSystemDLLs)
  install(CODE "MESSAGE(\"Creating start menu.\")" COMPONENT CreateStartMenu)

elseif(UNIX)
  set(CPACK_PACKAGE_DEFAULT_LOCATION "/usr/local")
  set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  set(CPACK_IFW_TARGET_DIRECTORY
      "/usr/local/${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}-${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}")

  install(FILES "${PROJECT_SOURCE_DIR}/bin/EP-Compare/readme.txt" DESTINATION "PostProcess/EP-Compare/")

  install(PROGRAMS "${PROJECT_SOURCE_DIR}/bin/EPMacro/Linux/EPMacro" DESTINATION "./")

  configure_file(scripts/runenergyplus.in "${PROJECT_BINARY_DIR}/scripts/runenergyplus" @ONLY)
  install(PROGRAMS "${PROJECT_BINARY_DIR}/scripts/runenergyplus" DESTINATION "./")
  install(PROGRAMS scripts/runepmacro DESTINATION "./")
  install(PROGRAMS scripts/runreadvars DESTINATION "./")

  # You need at least one "install(..." command for it to be registered as a component
  install(CODE "MESSAGE(\"Creating symlinks.\")" COMPONENT Symlinks)
  install(FILES "${PROJECT_SOURCE_DIR}/doc/man/energyplus.1" DESTINATION "./" COMPONENT Symlinks)
endif()

##########################################################   D O C U M E N T A T I O N   #############################################################

if(BUILD_DOCS)

  # If this isn't the case, then docs were already generated since added to the default make rule
  if(BUILD_DOCS_ONLY_WITH_PACKAGE)

    # Call the build of target docs explicitly here.
    # Note: This is because you can't do `add_dependencies(package docs)` (https://gitlab.kitware.com/cmake/cmake/issues/8438)
    # Adding another custom target to be added to the "ALL" one (so it runs) and make it depend on the actual "documentation" target doesn't work
    # because it'll always run if you have enabled BUILD_DOCS, regardless of whether you are calling the target "package" or not
    #  add_custom_target(run_documentation ALL)
    #  add_dependencies(run_documentation documentation)
    #message(FATAL_ERROR "CMAKE_COMMAND=${CMAKE_COMMAND}")

    # +env will pass the current environment and will end up respecting the -j parameter
    #                                 this ↓↓↓ here -- https://stackoverflow.com/a/41268443/531179
    #install(CODE "execute_process(COMMAND +env \"${CMAKE_COMMAND}\" --build \"${PROJECT_BINARY_DIR}\" --target documentation)")
    # Except it doesn't work with install(execute_process...

    # Passing $(MAKE) doesn't work either, and isn't a great idea for cross platform support anyways
    # install(CODE "execute_process(COMMAND ${MAKE} ${DOC_BUILD_FLAGS} -C \"${PROJECT_BINARY_DIR}\" documentation)")

    # So instead, we just used the number of threads that are available. That's not ideal, since it ignores any "-j N" option passed by the user
    # But LaTeX should run quickly enough to not be a major inconvenience.
    # There no need to do that for Ninja for eg, so only do it for Make and MSVC

    # flag -j to cmake --build was added at 3.12 (VERSION_GREATER_EQUAL need cmake >= 3.7, we apparently support 2.8...)
    if(NOT (CMAKE_VERSION VERSION_LESS "3.12") AND ((CMAKE_GENERATOR MATCHES "Make") OR WIN32))
      include(ProcessorCount)
      ProcessorCount(N)
      if(NOT N EQUAL 0)
        set(DOC_BUILD_FLAGS "-j ${N}")
      endif()
    endif()
    if(WIN32)
      # Win32 is multi config, so you must specify a config when calling cmake.
      # Let's just use Release, it won't have any effect on LaTeX anyways.
      set(DOC_CONFIG_FLAG "--config Release")
    endif()

    # Getting these commands to work (especially with macro expansion) is tricky. Check the resulting `cmake_install.cmake` file in your build folder if need to debug this
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${PROJECT_BINARY_DIR}\" ${DOC_CONFIG_FLAG} ${DOC_BUILD_FLAGS} --target docs)"
            COMPONENT Documentation)
  endif()

  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/EngineeringReference.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/ExternalInterfacesApplicationGuide.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/GettingStarted.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/InputOutputReference.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/InterfaceDeveloper.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/ModuleDeveloper.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/OutputDetailsAndExamples.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/PlantApplicationGuide.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/UsingEnergyPlusForCompliance.pdf" DESTINATION "./Documentation" COMPONENT Documentation)
  install(FILES "${PROJECT_BINARY_DIR}/doc/pdf/index.html" DESTINATION "./Documentation" COMPONENT Documentation)
  if(WIN32)
    install(FILES "${PROJECT_SOURCE_DIR}/doc/urls/Acknowledgments.url" DESTINATION "./Documentation" COMPONENT Documentation)
    install(FILES "${PROJECT_SOURCE_DIR}/doc/urls/AuxiliaryPrograms.url" DESTINATION "./Documentation" COMPONENT Documentation)
    install(FILES "${PROJECT_SOURCE_DIR}/doc/urls/EMSApplicationGuide.url" DESTINATION "./Documentation" COMPONENT Documentation)
    install(FILES "${PROJECT_SOURCE_DIR}/doc/urls/EnergyPlusEssentials.url" DESTINATION "./Documentation" COMPONENT Documentation)
  endif()
else()
  message(AUTHOR_WARNING "BUILD_DOCS isn't enabled, so package won't include the PDFs")
endif()

##########################################################   S Y S T E M    L I B R A R I E S   ######################################################

# Add compiler-provided system runtime libraries
if(WIN32 AND NOT UNIX)
  # Skip the call to install(PROGRAMS) so we can specify our own install rule (using the value of `CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS`)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

  # Set to TRUE to install the Windows Universal CRT libraries for app-local deployment (e.g. to Windows XP, but apparently needed for Windows 8 too).
  # This is meaningful only with MSVC from Visual Studio 2015 or higher, which is our case
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)


  # ConvertInputFormat is added via add_subdirectory, and in there we set CMAKE_INSTALL_OPENMP_LIBRARIES at parent scope already
  # Otherwise, if either cpgfunctionEP (yes by default) or kiva (no by default) **actually** linked to OpenMP, we need to ship the libs
  if (NOT ${CMAKE_INSTALL_OPENMP_LIBRARIES} AND (${USE_OpenMP} OR ${ENABLE_OPENMP}))
    # Need to install vcomp140.dll or similar
    find_package(OpenMP COMPONENTS CXX)
    if(OpenMP_CXX_FOUND)
      set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
    endif()
  endif()

  include(InstallRequiredSystemLibraries)
  if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION "./" COMPONENT Libraries)
  endif()
endif()

if(APPLE AND CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION)

  set(FILES_TO_SIGN
    # Targets are signed already via register_install_codesign_target
    #$<TARGET_FILE_NAME:ConvertInputFormat>
    #$<TARGET_FILE_NAME:energyplus>
    #$<TARGET_FILE_NAME:energyplusapi>

    # Bash scripts, not sure if needed or not
    "runenergyplus"
    "runepmacro"
    "runreadvars"
    # Copied-verbatim apps: Already signed because just copied from bin to package
    # "EPMacro"
    # "PreProcess/EP-Launch-Lite.app"
    # "PreProcess/IDFVersionUpdater/IDFVersionUpdater.app"
  )

  # Codesign inner binaries and libraries, in the CPack staging area for the EnergyPlus project, component Unspecified
  # Define some required variables for the script in the scope of the install(SCRIPT) first
  install(CODE "set(CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION \"${CPACK_CODESIGNING_DEVELOPPER_ID_APPLICATION}\")" COMPONENT Unspecified)
  install(CODE "set(CPACK_CODESIGNING_MACOS_IDENTIFIER \"${CPACK_CODESIGNING_MACOS_IDENTIFIER}\")" COMPONENT Unspecified)
  install(CODE "set(FILES_TO_SIGN \"${FILES_TO_SIGN}\")" COMPONENT Unspecified)
  # call the script
  install(SCRIPT "${CMAKE_CURRENT_LIST_DIR}/install_codesign_script.cmake" COMPONENT Unspecified)

  # Register the CPACK_POST_BUILD_SCRIPTS
  set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_CURRENT_LIST_DIR}/CPackSignAndNotarizeDmg.cmake")

endif()

# TODO: Unused now
configure_file("${PROJECT_SOURCE_DIR}/cmake/CMakeCPackOptions.cmake.in" "${PROJECT_BINARY_DIR}/CMakeCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CMakeCPackOptions.cmake")

######################################################################################################################################################
#                                                    P A C K A G I N G   &   C O M P O N E N T S                                                     #
######################################################################################################################################################

# Careful: the position (and what you include) matters a lot!
include(CPack)
include(CPackIFW)
# include(CPackComponent)

# Note: If you ever need to debug a CPack Package Error in MSVC, right-click on "PACKAGE" target, click Properties
# and in the Build Events > Post Build Event, edit the command that calls cpack to add `--verbose --debug`:
# eg: `"C:\Program Files\CMake\bin\cpack.exe" -C $(Configuration) --config ./CPackConfig.cmake --verbose --debug`

#cpack_add_component(EnergyPlus
#DISPLAY_NAME "EnergyPlus"
#DESCRIPTION "The EnergyPlus program itself"
#REQUIRED
#)

#cpack_add_component(AuxiliaryPrograms
#DISPLAY_NAME "Auxiliary Programs"
#DESCRIPTION "The suite of Fortran auxiliary programs such as ReadVarsESO, ExpandObjects, etc"
#REQUIRED
#)

cpack_add_component(Documentation DISPLAY_NAME "Documentation" DESCRIPTION "EnergyPlus documentation in PDF format")

cpack_add_component(Datasets DISPLAY_NAME "Datasets" DESCRIPTION "Useful resources such as material and equipment performance data")

cpack_add_component(ExampleFiles DISPLAY_NAME "Example Files" DESCRIPTION "IDF Example Files")

cpack_add_component(WeatherData DISPLAY_NAME "Weather Data" DESCRIPTION "EPW Weather Files")

# This stuff actually requires admin privileges since touched system locations
cpack_add_component(Symlinks DISPLAY_NAME "Create Symlinks - requires admin"
                    DESCRIPTION "This will symlink the executable to /usr/local/bin and copy the man page")

# Could add any upstream library license to this
cpack_add_component(
  Licenses
  DISPLAY_NAME "Licenses"
  DESCRIPTION "License files for EnergyPlus"
  REQUIRED HIDDEN)

# No need for system privileges for this
cpack_add_component(CreateStartMenu DISPLAY_NAME "Start Menu links" DESCRIPTION "Create Start Menu Links")

cpack_add_component(RegisterFileType DISPLAY_NAME "Associate with EP-Launch and IDFEditor"
                    DESCRIPTION "Associate *.idf, *.imf, and *.epg files with EP-Launch, *.ddy and *.expidf with IDFEditor.exe")

cpack_add_component(
  CopyAndRegisterSystemDLLs
  DISPLAY_NAME "Copy and Register DLLs"
  DESCRIPTION "This will copy and register system DLLs such as Fortran if they don't already exist"
  REQUIRED HIDDEN)

cpack_add_component(
  Libraries
  DISPLAY_NAME "Install required system libraries"
  DESCRIPTION "Install compiler-provided system runtime libraries, and Windows Universal CRT libraries for app-local deployment"
  REQUIRED HIDDEN)

# Regular stuff, like chmod +x
cpack_ifw_configure_component(Unspecified SCRIPT cmake/qtifw/install_operations.qs)

cpack_ifw_configure_component(Symlinks SCRIPT cmake/qtifw/install_unix_createsymlinks.qs REQUIRES_ADMIN_RIGHTS)

cpack_ifw_configure_component(CreateStartMenu SCRIPT cmake/qtifw/install_win_createstartmenu.qs)

if (PYTHON_CLI)
  cpack_ifw_configure_component(Unspecified SCRIPT cmake/qtifw/install_auxiliary_python_shortcuts.qs)
endif()

cpack_ifw_configure_component(RegisterFileType SCRIPT cmake/qtifw/install_registerfiletype.qs REQUIRES_ADMIN_RIGHTS)

cpack_ifw_configure_component(CopyAndRegisterSystemDLLs SCRIPT cmake/qtifw/install_win_copydll.qs REQUIRES_ADMIN_RIGHTS)

cpack_ifw_configure_component(Licenses FORCED_INSTALLATION LICENSES "EnergyPlus" ${CPACK_RESOURCE_FILE_LICENSE})
