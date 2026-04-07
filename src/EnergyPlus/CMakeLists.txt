if(WIN32)
  configure_file(energyplus.rc.in "${CMAKE_CURRENT_BINARY_DIR}/energyplus.rc")
  configure_file(energyplusapi.rc.in "${CMAKE_CURRENT_BINARY_DIR}/energyplusapi.rc")
endif()

include(../../cmake/SystemDetails.cmake)
set_system_nickname()

include(../../cmake/TargetArch.cmake)
target_architecture(TARGET_ARCH)

set(PYTHON_API_VERSION_MAJOR 0)
set(PYTHON_API_VERSION_MINOR 2)
set(BUILD_PLATFORM_STRING "${SYSTEM_NICKNAME}_${TARGET_ARCH}")
configure_file(DataStringGlobals.in.cc "${CMAKE_CURRENT_BINARY_DIR}/DataStringGlobals.cc")
configure_file(ConfiguredFunctions.in.cc "${CMAKE_CURRENT_BINARY_DIR}/ConfiguredFunctions.cc")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedEpJSONSchema.cc" "Will be filled in later by GenerateEmbeddedEpJSONSchema")

if(LINK_WITH_PYTHON)
  add_compile_definitions(LINK_WITH_PYTHON)
endif()

if(USE_PSYCHROMETRICS_CACHING)
  # Caching defaulted ON
else()
  add_compile_definitions(EP_nocache_Psychrometrics)
endif()

if(USE_GLYCOL_CACHING)
  add_compile_definitions(EP_cache_GlycolSpecificHeat)
endif()

if(USE_PSYCH_STATS)
  add_compile_definitions(EP_psych_stats)
endif()

if(USE_PSYCH_ERRORS)
  add_compile_definitions(EP_psych_errors)
endif()

set(SRC
    "${PROJECT_SOURCE_DIR}/third_party/milo/dtoa.h"
    "${PROJECT_SOURCE_DIR}/third_party/milo/itoa.h"
    "${PROJECT_SOURCE_DIR}/third_party/milo/diyfp.h"
    "${PROJECT_SOURCE_DIR}/third_party/milo/ieee754.h"
    AirLoopHVACDOAS.cc
    AirLoopHVACDOAS.hh
    AirTerminalUnit.hh
    Autosizing/All_Simple_Sizing.cc
    Autosizing/All_Simple_Sizing.hh
    Autosizing/Base.cc
    Autosizing/Base.hh
    Autosizing/BaseSizerWithFanHeatInputs.cc
    Autosizing/BaseSizerWithFanHeatInputs.hh
    Autosizing/BaseSizerWithScalableInputs.cc
    Autosizing/BaseSizerWithScalableInputs.hh
    Autosizing/CoolingAirFlowSizing.cc
    Autosizing/CoolingAirFlowSizing.hh
    Autosizing/CoolingCapacitySizing.cc
    Autosizing/CoolingCapacitySizing.hh
    Autosizing/CoolingSHRSizing.cc
    Autosizing/CoolingSHRSizing.hh
    Autosizing/CoolingWaterDesAirInletHumRatSizing.cc
    Autosizing/CoolingWaterDesAirInletHumRatSizing.hh
    Autosizing/CoolingWaterDesAirInletTempSizing.cc
    Autosizing/CoolingWaterDesAirInletTempSizing.hh
    Autosizing/CoolingWaterDesAirOutletHumRatSizing.cc
    Autosizing/CoolingWaterDesAirOutletHumRatSizing.hh
    Autosizing/CoolingWaterDesAirOutletTempSizing.cc
    Autosizing/CoolingWaterDesAirOutletTempSizing.hh
    Autosizing/CoolingWaterDesWaterInletTempSizing.cc
    Autosizing/CoolingWaterDesWaterInletTempSizing.hh
    Autosizing/CoolingWaterNumofTubesPerRowSizing.cc
    Autosizing/CoolingWaterNumofTubesPerRowSizing.hh
    Autosizing/CoolingWaterflowSizing.cc
    Autosizing/CoolingWaterflowSizing.hh
    Autosizing/HeatingAirFlowSizing.cc
    Autosizing/HeatingAirFlowSizing.hh
    Autosizing/HeatingAirflowUASizing.cc
    Autosizing/HeatingAirflowUASizing.hh
    Autosizing/HeatingCapacitySizing.cc
    Autosizing/HeatingCapacitySizing.hh
    Autosizing/HeatingWaterDesAirInletHumRatSizing.cc
    Autosizing/HeatingWaterDesAirInletHumRatSizing.hh
    Autosizing/HeatingWaterDesAirInletTempSizing.cc
    Autosizing/HeatingWaterDesAirInletTempSizing.hh
    Autosizing/HeatingWaterDesCoilLoadUsedForUASizing.cc
    Autosizing/HeatingWaterDesCoilLoadUsedForUASizing.hh
    Autosizing/HeatingWaterDesCoilWaterVolFlowUsedForUASizing.cc
    Autosizing/HeatingWaterDesCoilWaterVolFlowUsedForUASizing.hh
    Autosizing/HeatingWaterflowSizing.cc
    Autosizing/HeatingWaterflowSizing.hh
    Autosizing/SystemAirFlowSizing.cc
    Autosizing/SystemAirFlowSizing.hh
    Autosizing/WaterHeatingCapacitySizing.cc
    Autosizing/WaterHeatingCapacitySizing.hh
    Autosizing/WaterHeatingCoilUASizing.cc
    Autosizing/WaterHeatingCoilUASizing.hh
    BaseboardElectric.cc
    BaseboardElectric.hh
    BaseboardRadiator.cc
    BaseboardRadiator.hh
    BoilerSteam.cc
    BoilerSteam.hh
    Boilers.cc
    Boilers.hh
    BranchInputManager.cc
    BranchInputManager.hh
    BranchNodeConnections.cc
    BranchNodeConnections.hh
    CTElectricGenerator.cc
    CTElectricGenerator.hh
    ChilledCeilingPanelSimple.cc
    ChilledCeilingPanelSimple.hh
    ChillerAbsorption.cc
    ChillerAbsorption.hh
    ChillerElectricEIR.cc
    ChillerElectricEIR.hh
    ChillerElectricASHRAE205.cc
    ChillerElectricASHRAE205.hh
    ChillerExhaustAbsorption.cc
    ChillerExhaustAbsorption.hh
    ChillerGasAbsorption.cc
    ChillerGasAbsorption.hh
    ChillerIndirectAbsorption.cc
    ChillerIndirectAbsorption.hh
    ChillerReformulatedEIR.cc
    ChillerReformulatedEIR.hh
    Coils/CoilCoolingDX.cc
    Coils/CoilCoolingDX.hh
    Coils/CoilCoolingDXCurveFitOperatingMode.cc
    Coils/CoilCoolingDXCurveFitOperatingMode.hh
    Coils/CoilCoolingDXCurveFitPerformance.cc
    Coils/CoilCoolingDXCurveFitPerformance.hh
    Coils/CoilCoolingDXCurveFitSpeed.cc
    Coils/CoilCoolingDXCurveFitSpeed.hh
    CondenserLoopTowers.cc
    CondenserLoopTowers.hh
    ConfiguredFunctions.hh
    "${CMAKE_CURRENT_BINARY_DIR}/ConfiguredFunctions.cc"
    Construction.cc
    Construction.hh
    ConvectionCoefficients.cc
    ConvectionCoefficients.hh
    ConvectionConstants.hh
    CoolTower.cc
    CoolTower.hh
    CostEstimateManager.cc
    CostEstimateManager.hh
    CrossVentMgr.cc
    CrossVentMgr.hh
    CurveManager.cc
    CurveManager.hh
    DElightManagerF.cc
    DElightManagerF.hh
    DXCoils.cc
    DXCoils.hh
    DXFEarClipping.cc
    DXFEarClipping.hh
    Data/BaseData.hh
    Data/CommonIncludes.hh
    Data/EnergyPlusData.cc
    Data/EnergyPlusData.hh
    DataAirLoop.hh
    DataAirSystems.cc
    DataAirSystems.hh
    DataBSDFWindow.hh
    DataBranchAirLoopPlant.hh
    DataBranchNodeConnections.hh
    DataComplexFenestration.hh
    DataContaminantBalance.hh
    DataConvergParams.hh
    DataConversions.hh
    DataDElight.hh
    DataDaylighting.hh
    DataDaylightingDevices.hh
    DataDefineEquip.hh
    DataEnvironment.cc
    DataEnvironment.hh
    DataErrorTracking.hh
    DataGenerators.hh
    DataGlobalConstants.cc
    DataGlobalConstants.hh
    DataGlobals.hh
    DataHVACControllers.hh
    DataHVACGlobals.cc
    DataHVACGlobals.hh
    DataHVACSystems.hh
    DataHeatBalFanSys.hh
    DataHeatBalSurface.hh
    DataHeatBalance.cc
    DataHeatBalance.hh
    DataIPShortCuts.hh
    DataLoopNode.hh
    DataMoistureBalance.hh
    DataMoistureBalanceEMPD.hh
    DataOutputs.cc
    DataOutputs.hh
    DataPhotovoltaics.hh
    DataPrecisionGlobals.hh
    DataReportingFlags.hh
    DataRoomAirModel.hh
    DataRootFinder.hh
    DataRuntimeLanguage.cc
    DataRuntimeLanguage.hh
    DataShadowingCombinations.hh
    DataSizing.cc
    DataSizing.hh
    "${CMAKE_CURRENT_BINARY_DIR}/DataStringGlobals.cc"
    DataStringGlobals.hh
    DataSurfaceColors.cc
    DataSurfaceColors.hh
    DataSurfaceLists.cc
    DataSurfaceLists.hh
    DataSurfaces.cc
    DataSurfaces.hh
    DataSystemVariables.cc
    DataSystemVariables.hh
    DataTimings.hh
    DataUCSDSharedData.hh
    DataVectorTypes.hh
    DataViewFactorInformation.hh
    DataWater.hh
    DataWindowEquivalentLayer.hh
    DataZoneControls.hh
    DataZoneEnergyDemands.cc
    DataZoneEnergyDemands.hh
    DataZoneEquipment.cc
    DataZoneEquipment.hh
    DaylightingDevices.cc
    DaylightingDevices.hh
    DaylightingManager.cc
    DaylightingManager.hh
    DemandManager.cc
    DemandManager.hh
    DesiccantDehumidifiers.cc
    DesiccantDehumidifiers.hh
    DisplacementVentMgr.cc
    DisplacementVentMgr.hh
    DisplayRoutines.cc
    DisplayRoutines.hh
    DualDuct.cc
    DualDuct.hh
    EMSManager.cc
    EMSManager.hh
    EPVector.hh
    EarthTube.cc
    EarthTube.hh
    EcoRoofManager.cc
    EcoRoofManager.hh
    EconomicLifeCycleCost.cc
    EconomicLifeCycleCost.hh
    EconomicTariff.cc
    EconomicTariff.hh
    ElectricBaseboardRadiator.cc
    ElectricBaseboardRadiator.hh
    ElectricPowerServiceManager.cc
    ElectricPowerServiceManager.hh
    EnergyPlus.hh
    EvaporativeCoolers.cc
    EvaporativeCoolers.hh
    EvaporativeFluidCoolers.cc
    EvaporativeFluidCoolers.hh
    ExhaustAirSystemManager.cc
    ExhaustAirSystemManager.hh
    ExteriorEnergyUse.cc
    ExteriorEnergyUse.hh
    ExternalInterface.cc
    ExternalInterface.hh
    FanCoilUnits.cc
    FanCoilUnits.hh
    Fans.cc
    Fans.hh
    FaultsManager.cc
    FaultsManager.hh
    FileSystem.cc
    FileSystem.hh
    FluidCoolers.cc
    FluidCoolers.hh
    FluidProperties.cc
    FluidProperties.hh
    FromChars.hh
    FuelCellElectricGenerator.cc
    FuelCellElectricGenerator.hh
    Furnaces.cc
    Furnaces.hh
    General.cc
    General.hh
    GeneralRoutines.cc
    GeneralRoutines.hh
    GeneratorDynamicsManager.cc
    GeneratorDynamicsManager.hh
    GeneratorFuelSupply.cc
    GeneratorFuelSupply.hh
    GlobalNames.cc
    GlobalNames.hh
    GroundHeatExchangers.cc
    GroundHeatExchangers.hh
    GroundTemperatureModeling/BaseGroundTemperatureModel.hh
    GroundTemperatureModeling/FiniteDifferenceGroundTemperatureModel.cc
    GroundTemperatureModeling/FiniteDifferenceGroundTemperatureModel.hh
    GroundTemperatureModeling/GroundTemperatureModelManager.cc
    GroundTemperatureModeling/GroundTemperatureModelManager.hh
    GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.cc
    GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh
    GroundTemperatureModeling/SiteBuildingSurfaceGroundTemperatures.cc
    GroundTemperatureModeling/SiteBuildingSurfaceGroundTemperatures.hh
    GroundTemperatureModeling/SiteDeepGroundTemperatures.cc
    GroundTemperatureModeling/SiteDeepGroundTemperatures.hh
    GroundTemperatureModeling/SiteFCFactorMethodGroundTemperatures.cc
    GroundTemperatureModeling/SiteFCFactorMethodGroundTemperatures.hh
    GroundTemperatureModeling/SiteShallowGroundTemperatures.cc
    GroundTemperatureModeling/SiteShallowGroundTemperatures.hh
    GroundTemperatureModeling/XingGroundTemperatureModel.cc
    GroundTemperatureModeling/XingGroundTemperatureModel.hh
    HVACControllers.cc
    HVACControllers.hh
    HVACCooledBeam.cc
    HVACCooledBeam.hh
    HVACDXHeatPumpSystem.cc
    HVACDXHeatPumpSystem.hh
    HVACDuct.cc
    HVACDuct.hh
    HVACFan.cc
    HVACFan.hh
    HVACFourPipeBeam.cc
    HVACFourPipeBeam.hh
    HVACHXAssistedCoolingCoil.cc
    HVACHXAssistedCoolingCoil.hh
    HVACInterfaceManager.cc
    HVACInterfaceManager.hh
    HVACManager.cc
    HVACManager.hh
    HVACMultiSpeedHeatPump.cc
    HVACMultiSpeedHeatPump.hh
    HVACSingleDuctInduc.cc
    HVACSingleDuctInduc.hh
    HVACSizingSimulationManager.cc
    HVACSizingSimulationManager.hh
    HVACStandAloneERV.cc
    HVACStandAloneERV.hh
    HVACSystemRootFindingAlgorithm.hh
    HVACUnitaryBypassVAV.cc
    HVACUnitaryBypassVAV.hh
    HVACVariableRefrigerantFlow.cc
    HVACVariableRefrigerantFlow.hh
    HWBaseboardRadiator.cc
    HWBaseboardRadiator.hh
    HeatBalFiniteDiffManager.cc
    HeatBalFiniteDiffManager.hh
    HeatBalanceAirManager.cc
    HeatBalanceAirManager.hh
    HeatBalanceHAMTManager.cc
    HeatBalanceHAMTManager.hh
    HeatBalanceIntRadExchange.cc
    HeatBalanceIntRadExchange.hh
    HeatBalanceInternalHeatGains.cc
    HeatBalanceInternalHeatGains.hh
    HeatBalanceKivaManager.cc
    HeatBalanceKivaManager.hh
    HeatBalanceManager.cc
    HeatBalanceManager.hh
    HeatBalanceSurfaceManager.cc
    HeatBalanceSurfaceManager.hh
    HeatPumpWaterToWaterCOOLING.cc
    HeatPumpWaterToWaterCOOLING.hh
    HeatPumpWaterToWaterHEATING.cc
    HeatPumpWaterToWaterHEATING.hh
    HeatPumpWaterToWaterSimple.cc
    HeatPumpWaterToWaterSimple.hh
    HeatRecovery.cc
    HeatRecovery.hh
    HeatingCoils.cc
    HeatingCoils.hh
    HighTempRadiantSystem.cc
    HighTempRadiantSystem.hh
    Humidifiers.cc
    Humidifiers.hh
    HybridEvapCoolingModel.cc
    HybridEvapCoolingModel.hh
    HybridModel.cc
    HybridModel.hh
    HybridUnitaryAirConditioners.cc
    HybridUnitaryAirConditioners.hh
    ICEngineElectricGenerator.cc
    ICEngineElectricGenerator.hh
    IceThermalStorage.cc
    IceThermalStorage.hh
    InputProcessing/CsvParser.cc
    InputProcessing/CsvParser.hh
    InputProcessing/DataStorage.hh
    InputProcessing/EmbeddedEpJSONSchema.hh
    "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedEpJSONSchema.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/InputEnumsAirflowNetwork.hxx"
    "${CMAKE_CURRENT_BINARY_DIR}/InputObjectsAirflowNetwork.hxx"
    IOFiles.cc
    IOFiles.hh
    InputProcessing/AutomaticObjects.hh
    InputProcessing/IdfParser.cc
    InputProcessing/IdfParser.hh
    InputProcessing/InputProcessor.cc
    InputProcessing/InputProcessor.hh
    InputProcessing/InputValidation.cc
    InputProcessing/InputValidation.hh
    IntegratedHeatPump.cc
    IntegratedHeatPump.hh
    InternalHeatGains.cc
    InternalHeatGains.hh
    LowTempRadiantSystem.cc
    LowTempRadiantSystem.hh
    Material.cc
    Material.hh
    MatrixDataManager.cc
    MatrixDataManager.hh
    MicroCHPElectricGenerator.cc
    MicroCHPElectricGenerator.hh
    MicroturbineElectricGenerator.cc
    MicroturbineElectricGenerator.hh
    MixedAir.cc
    MixedAir.hh
    MixerComponent.cc
    MixerComponent.hh
    MoistureBalanceEMPDManager.cc
    MoistureBalanceEMPDManager.hh
    MundtSimMgr.cc
    MundtSimMgr.hh
    NodeInputManager.cc
    NodeInputManager.hh
    NonZoneEquipmentManager.cc
    NonZoneEquipmentManager.hh
    OutAirNodeManager.cc
    OutAirNodeManager.hh
    OutdoorAirUnit.cc
    OutdoorAirUnit.hh
    OutputProcessor.cc
    OutputProcessor.hh
    OutputReportData.cc
    OutputReportData.hh
    OutputReportPredefined.cc
    OutputReportPredefined.hh
    OutputReportTabular.cc
    OutputReportTabular.hh
    OutputReportTabularAnnual.cc
    OutputReportTabularAnnual.hh
    OutputReports.cc
    OutputReports.hh
    OutsideEnergySources.cc
    OutsideEnergySources.hh
    PVWatts.cc
    PVWatts.hh
    PackagedThermalStorageCoil.cc
    PackagedThermalStorageCoil.hh
    PhaseChangeModeling/HysteresisModel.cc
    PhaseChangeModeling/HysteresisModel.hh
    PhotovoltaicThermalCollectors.cc
    PhotovoltaicThermalCollectors.hh
    Photovoltaics.cc
    Photovoltaics.hh
    PierceSurface.hh
    PipeHeatTransfer.cc
    PipeHeatTransfer.hh
    Pipes.cc
    Pipes.hh
    Plant/Branch.cc
    Plant/Branch.hh
    Plant/CallingOrder.hh
    Plant/Component.cc
    Plant/Component.hh
    Plant/ConnectedLoopData.hh
    Plant/Connection.hh
    Plant/DataPlant.hh
    Plant/Enums.hh
    Plant/EquipAndOperations.hh
    Plant/Loop.cc
    Plant/Loop.hh
    Plant/LoopSide.cc
    Plant/LoopSide.hh
    Plant/LoopSidePumpInformation.hh
    Plant/MeterData.hh
    Plant/MixerData.hh
    Plant/PlantAvailManager.hh
    Plant/PlantConvergencePoint.hh
    Plant/PlantLocation.hh
    Plant/PlantManager.cc
    Plant/PlantManager.hh
    Plant/ReportBranchData.hh
    Plant/ReportCompData.hh
    Plant/ReportLoopData.hh
    Plant/SplitterData.hh
    Plant/Subcomponents.hh
    PlantCentralGSHP.cc
    PlantCentralGSHP.hh
    PlantChillers.cc
    PlantChillers.hh
    PlantComponent.hh
    PlantComponentTemperatureSources.cc
    PlantComponentTemperatureSources.hh
    PlantCondLoopOperation.cc
    PlantCondLoopOperation.hh
    PlantHeatExchangerFluidToFluid.cc
    PlantHeatExchangerFluidToFluid.hh
    PlantLoadProfile.cc
    PlantLoadProfile.hh
    PlantLoopHeatPumpEIR.cc
    PlantLoopHeatPumpEIR.hh
    PlantPipingSystemsManager.cc
    PlantPipingSystemsManager.hh
    PlantPressureSystem.cc
    PlantPressureSystem.hh
    PlantUtilities.cc
    PlantUtilities.hh
    PlantValves.cc
    PlantValves.hh
    Platform.hh
    PluginManager.cc
    PluginManager.hh
    PollutionModule.cc
    PollutionModule.hh
    PondGroundHeatExchanger.cc
    PondGroundHeatExchanger.hh
    PoweredInductionUnits.cc
    PoweredInductionUnits.hh
    Psychrometrics.cc
    Psychrometrics.hh
    PsychCacheData.hh
    Pumps.cc
    Pumps.hh
    PurchasedAirManager.cc
    PurchasedAirManager.hh
    RefrigeratedCase.cc
    RefrigeratedCase.hh
    ReportCoilSelection.cc
    ReportCoilSelection.hh
    ResultsFramework.cc
    ResultsFramework.hh
    ReturnAirPathManager.cc
    ReturnAirPathManager.hh
    RoomAirModelAirflowNetwork.cc
    RoomAirModelAirflowNetwork.hh
    RoomAirModelManager.cc
    RoomAirModelManager.hh
    RoomAirModelUserTempPattern.cc
    RoomAirModelUserTempPattern.hh
    RootFinder.cc
    RootFinder.hh
    RuntimeLanguageProcessor.cc
    RuntimeLanguageProcessor.hh
    SQLiteProcedures.cc
    SQLiteProcedures.hh
    SZVAVModel.cc
    SZVAVModel.hh
    ScheduleManager.cc
    ScheduleManager.hh
    SetPointManager.cc
    SetPointManager.hh
    Shape.hh
    SimAirServingZones.cc
    SimAirServingZones.hh
    SimulationManager.cc
    SimulationManager.hh
    SingleDuct.cc
    SingleDuct.hh
    SizingAnalysisObjects.cc
    SizingAnalysisObjects.hh
    SizingManager.cc
    SizingManager.hh
    SolarCollectors.cc
    SolarCollectors.hh
    SolarReflectionManager.cc
    SolarReflectionManager.hh
    SolarShading.cc
    SolarShading.hh
    SortAndStringUtilities.cc
    SortAndStringUtilities.hh
    SplitterComponent.cc
    SplitterComponent.hh
    StandardRatings.cc
    StandardRatings.hh
    SteamBaseboardRadiator.cc
    SteamBaseboardRadiator.hh
    SteamCoils.cc
    SteamCoils.hh
    StringUtilities.hh
    SurfaceGeometry.cc
    SurfaceGeometry.hh
    SurfaceGroundHeatExchanger.cc
    SurfaceGroundHeatExchanger.hh
    SurfaceOctree.cc
    SurfaceOctree.hh
    SwimmingPool.cc
    SwimmingPool.hh
    SystemAvailabilityManager.cc
    SystemAvailabilityManager.hh
    SystemReports.cc
    SystemReports.hh
    TARCOGArgs.cc
    TARCOGArgs.hh
    TARCOGCommon.cc
    TARCOGCommon.hh
    TARCOGDeflection.cc
    TARCOGDeflection.hh
    TARCOGGasses90.cc
    TARCOGGasses90.hh
    TARCOGGassesParams.hh
    TARCOGMain.cc
    TARCOGMain.hh
    TARCOGOutput.cc
    TARCOGOutput.hh
    TARCOGParams.cc
    TARCOGParams.hh
    TarcogShading.cc
    TarcogShading.hh
    ThermalChimney.cc
    ThermalChimney.hh
    ThermalComfort.cc
    ThermalComfort.hh
    ThermalEN673Calc.cc
    ThermalEN673Calc.hh
    ThermalISO15099Calc.cc
    ThermalISO15099Calc.hh
    TranspiredCollector.cc
    TranspiredCollector.hh
    UFADManager.cc
    UFADManager.hh
    UnitHeater.cc
    UnitHeater.hh
    UnitVentilator.cc
    UnitVentilator.hh
    UnitarySystem.cc
    UnitarySystem.hh
    UserDefinedComponents.cc
    UserDefinedComponents.hh
    UtilityRoutines.cc
    UtilityRoutines.hh
    VariableSpeedCoils.cc
    VariableSpeedCoils.hh
    Vectors.cc
    Vectors.hh
    VentilatedSlab.cc
    VentilatedSlab.hh
    WaterCoils.cc
    WaterCoils.hh
    WaterManager.cc
    WaterManager.hh
    WaterThermalTanks.cc
    WaterThermalTanks.hh
    WaterToAirHeatPump.cc
    WaterToAirHeatPump.hh
    WaterToAirHeatPumpSimple.cc
    WaterToAirHeatPumpSimple.hh
    WaterUse.cc
    WaterUse.hh
    WeatherManager.cc
    WeatherManager.hh
    WindTurbine.cc
    WindTurbine.hh
    WindowAC.cc
    WindowAC.hh
    WindowComplexManager.cc
    WindowComplexManager.hh
    WindowEquivalentLayer.cc
    WindowEquivalentLayer.hh
    WindowManager.cc
    WindowManager.hh
    WindowManagerExteriorData.cc
    WindowManagerExteriorData.hh
    WindowManagerExteriorOptical.cc
    WindowManagerExteriorOptical.hh
    WindowManagerExteriorThermal.cc
    WindowManagerExteriorThermal.hh
    WindowModel.cc
    WindowModel.hh
    ZoneAirLoopEquipmentManager.cc
    ZoneAirLoopEquipmentManager.hh
    ZoneContaminantPredictorCorrector.cc
    ZoneContaminantPredictorCorrector.hh
    ZoneDehumidifier.cc
    ZoneDehumidifier.hh
    ZoneEquipmentManager.cc
    ZoneEquipmentManager.hh
    ZonePlenum.cc
    ZonePlenum.hh
    ZoneTempPredictorCorrector.cc
    ZoneTempPredictorCorrector.hh)

create_src_groups("${SRC}")

set(INPUTPARSING_SRC
    "${PROJECT_SOURCE_DIR}/third_party/milo/dtoa.h"
    "${PROJECT_SOURCE_DIR}/third_party/milo/itoa.h"
    "${PROJECT_SOURCE_DIR}/third_party/milo/diyfp.h"
    "${PROJECT_SOURCE_DIR}/third_party/milo/ieee754.h"
    "${CMAKE_CURRENT_BINARY_DIR}/DataStringGlobals.cc"
    DataStringGlobals.hh
    FileSystem.cc
    FileSystem.hh
    InputProcessing/EmbeddedEpJSONSchema.hh
    "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedEpJSONSchema.cc"
    InputProcessing/IdfParser.cc
    InputProcessing/IdfParser.hh
    InputProcessing/InputValidation.cc
    InputProcessing/InputValidation.hh
    UtilityRoutines.hh)

create_src_groups("${INPUTPARSING_SRC}")

add_custom_target(
  GenerateEmbeddedEpJSONSchema
  COMMAND
    ${CMAKE_COMMAND}
        -D "EnergyPlus_SOURCE_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}"
        -D "EnergyPlus_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        -D "EnergyPlus_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}"
        -D "EnergyPlus_CURRENT_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}"
        -D "EnergyPlus_embeddable_epJSON_schema:PATH=$<TARGET_FILE:generate_embeddable_epJSON_schema>"
        -P "${PROJECT_SOURCE_DIR}/scripts/dev/generate_embeddable_epJSON_schema/generate_embedded_epJSON_schema.cmake"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS generate_embeddable_epJSON_schema)

add_subdirectory(AirflowNetwork)

if(LINK_WITH_PYTHON)
  # check the python bit version here...if we are building against Python it has
  # to match up bitwise if we aren't, then the identified interpreter can be
  # whatever bit it wants
  if(MSVC)
    execute_process(COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/cmake/PythonGetBitSize.py" OUTPUT_VARIABLE PYTHON_BIT_SIZE)
    # PYTHON_BIT_SIZE will be either "32" or "64"
    # CMAKE_EXE_LINKER_FLAGS:STRING=/machine:x64
    # CMAKE_EXE_LINKER_FLAGS:STRING=/machine:X86
    if("${PYTHON_BIT_SIZE}" EQUAL "32")
      string(FIND "${CMAKE_EXE_LINKER_FLAGS}" "86" PY_BITS_IN_ARCH)
      if("${PY_BITS_IN_ARCH}" GREATER -1)
        message(STATUS "Architecture of Python lib and EnergyPlus build align at 32-bit")
      else()
        message(FATAL_ERROR "Python is 32-bit while EnergyPlus architecture appears to be 64-bit!")
      endif()
    else() # assuming 64 bit
      string(FIND "${CMAKE_EXE_LINKER_FLAGS}" "64" PY_BITS_IN_ARCH)
      if("${PY_BITS_IN_ARCH}" GREATER -1)
        message(STATUS "Architecture of Python lib and EnergyPlus build align at 64-bit")
      else()
        message(FATAL_ERROR "Python is 64-bit while EnergyPlus architecture appears to be 32-bit!")
      endif()
    endif()
  endif()

  # now we need to get the Python libs and add the include path for further compiling
  # The Development (libs) was already found in the root level CMakeLists.txt
  include_directories("${Python_INCLUDE_DIRS}")
endif()

add_library(energyplusparser STATIC ${INPUTPARSING_SRC})
add_dependencies(energyplusparser GenerateEmbeddedEpJSONSchema)
target_link_libraries(energyplusparser PUBLIC ModernJSON re2 fmt::fmt ${CMAKE_DL_LIBS})
target_link_libraries(energyplusparser PRIVATE project_options project_fp_options project_warnings)
if(WIN32)
  target_link_libraries(energyplusparser PUBLIC Shlwapi)
endif()
# See https://en.cppreference.com/w/cpp/filesystem#Notes
if ((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1) OR
    ( ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)))
  target_link_libraries(energyplusparser PRIVATE stdc++fs)
endif()

# first we will create a static library of EnergyPlus this will be linked
# statically to create the DLL and also the unit tests
add_library(energypluslib STATIC ${SRC})

target_link_libraries(
  energypluslib
  PUBLIC fmt::fmt
         ModernJSON
         objexx
         sqlite
         bcvtb
         epexpat
         epfmiimport
         re2
         DElight
         libkiva
         btwxt
         Windows-CalcEngine
         airflownetworklib
         ssc
         cpgfunctionEP
         libtk205)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(energypluslib PRIVATE project_options project_fp_options project_warnings cpp_pch_files)

if(OPENGL_FOUND)
  target_link_libraries(energypluslib PUBLIC penumbra)
endif()
add_dependencies(energypluslib GenerateEmbeddedEpJSONSchema)
if(BUILD_GROUND_PLOT)
  set_source_files_properties(HeatBalanceKivaManager.cc PROPERTIES COMPILE_DEFINITIONS GROUND_PLOT)
  target_link_libraries(energypluslib PUBLIC groundplot)
endif()

target_link_libraries(energypluslib PUBLIC ${CMAKE_DL_LIBS})
if(WIN32)
  target_link_libraries(energypluslib PUBLIC Shlwapi)
endif()
if(UNIX AND NOT APPLE)
  if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
    target_link_libraries(energypluslib PUBLIC util)
  endif()
endif()
if(LINK_WITH_PYTHON)
  target_link_libraries(energypluslib PUBLIC ${Python_LIBRARIES})
endif()
# See https://en.cppreference.com/w/cpp/filesystem#Notes
if ((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1) OR
    ( ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)))
  target_link_libraries(energypluslib PRIVATE stdc++fs)
endif()

# second we will create the shared library that is actually packaged with
# EnergyPlus
set(API_HEADERS
    api/func.h
    api/runtime.h
    api/datatransfer.h
    api/state.h
    # api/autosizing.h
    api/EnergyPlusPgm.hh
    api/EnergyPlusAPI.h
    api/TypeDefs.h)
set(API_CORE_SRC
    api/func.cc api/runtime.cc api/state.cc api/datatransfer.cc
    # api/autosizing.cc
    api/EnergyPlusPgm.cc)

option(BUILD_STATIC_ENERGYPLUS_API "Build a static library instead of a shared library for the EnergyPlus API" OFF)
mark_as_advanced(BUILD_STATIC_ENERGYPLUS_API)
if(BUILD_STATIC_ENERGYPLUS_API)
  set(API_LIBRARY_TYPE STATIC)
  add_compile_definitions(ENERGYPLUS_STATIC_API)
else()
  set(API_LIBRARY_TYPE SHARED)
endif()

if(APPLE OR UNIX)
  add_library(energyplusapi ${API_LIBRARY_TYPE} CommandLineInterface.hh CommandLineInterface.cc ${API_CORE_SRC} ${API_HEADERS})
else() # windows
  add_library(energyplusapi ${API_LIBRARY_TYPE} CommandLineInterface.hh CommandLineInterface.cc ${API_CORE_SRC}
                            "${CMAKE_CURRENT_BINARY_DIR}/energyplusapi.rc")
endif()
target_link_libraries(energyplusapi PRIVATE energypluslib)
target_link_libraries(energyplusapi PRIVATE project_options project_fp_options project_warnings)

set_target_properties(energyplusapi PROPERTIES INSTALL_NAME_DIR "@executable_path")


# now the actual E+ executable
if(APPLE OR UNIX)
  add_executable(energyplus main.cc)
else() # windows
  add_executable(energyplus main.cc "${CMAKE_CURRENT_BINARY_DIR}/energyplus.rc")
endif()
target_link_libraries(energyplus PRIVATE energyplusapi project_options project_fp_options project_warnings)

set_target_properties(energyplus PROPERTIES VERSION ${ENERGYPLUS_VERSION})
set_target_properties(energyplusapi PROPERTIES VERSION ${ENERGYPLUS_VERSION})

# bring in the Python API layer right next to the executable
add_custom_command(
  TARGET energyplusapi
  POST_BUILD
  DEPENDS
  "${PROJECT_SOURCE_DIR}/src/EnergyPlus/api"
  # TODO: temporary "hack" to depend on a specific file that doesn't exist to
  # force this to run
  DEPENDS
  __ALWAYSRUNME
  COMMAND
    ${CMAKE_COMMAND} -DREPO_ROOT=${PROJECT_SOURCE_DIR} -DEXECUTABLE_PATH=$<TARGET_FILE:energyplusapi> -DCMAKE_VERSION_MAJOR="${CMAKE_VERSION_MAJOR}"
    -DCMAKE_VERSION_MINOR="${CMAKE_VERSION_MINOR}" -DCMAKE_VERSION_PATCH="${CMAKE_VERSION_PATCH}" -DCMAKE_VERSION_BUILD="${CMAKE_VERSION_BUILD}"
    -DPYTHON_API_VERSION_MAJOR="${PYTHON_API_VERSION_MAJOR}" -DPYTHON_API_VERSION_MINOR="${PYTHON_API_VERSION_MINOR}" -P
    "${PROJECT_SOURCE_DIR}/cmake/PythonSetupAPIinBuild.cmake")

install(
  TARGETS energyplusapi energyplus
  EXPORT energyplus
  RUNTIME DESTINATION ./
  LIBRARY DESTINATION ./
  ARCHIVE DESTINATION ./
  INCLUDES DESTINATION include
                       include/EnergyPlus/api
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/energyplusConfigVersion.cmake"
  VERSION ${ENERGYPLUS_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(
  EXPORT
  energyplus
       NAMESPACE "energyplus::"
       FILE "${PROJECT_BINARY_DIR}/energyplusConfig.cmake"
)

install(
  EXPORT energyplus
        DESTINATION lib/cmake/energyplus
        NAMESPACE "energyplus::"
        COMPONENT "CPPAPI"
  FILE energyplusConfig.cmake
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/energyplusConfigVersion.cmake"
  DESTINATION
    lib/cmake/energyplus/
  COMPONENT "CPPAPI"
)

if(LINK_WITH_PYTHON)
  # link the executable to the dll and bring it in
  get_filename_component(RESOLVED_PYTHON_LIBRARY "${Python_LIBRARIES}" REALPATH)
  if(WIN32)

    # In case you have both release and debug Python libraries on your system, Python_LIBRARIES might be "optimized;C:/.../python38.lib;debug;C:/.../python38_d.lib"
    # so it can't be used directly, we could use a generator expression to find the $<TARGET_FILE:Python::Python> library used and that'd point directly
    # to the DLL. But it won't work nicely for the install(code ...) command below... so we do a hack, get the optimized one always...
    list(LENGTH Python_LIBRARIES _LEN)
    if (_LEN GREATER 1)
      set (_Python_DOING_RELEASE FALSE)
      foreach(_currentArg ${Python_LIBRARIES})
        if ("x${_currentArg}" STREQUAL "xoptimized")
          set(_Python_DOING_RELEASE TRUE)
        elseif(_Python_DOING_RELEASE)
          get_filename_component(RESOLVED_PYTHON_LIBRARY "${_currentArg}" REALPATH)
          break()
        endif()
      endforeach()
    # else()
      # No-op, already done above
    endif()

    # Windows is being ugly and linking against the Python DLL but not running
    # unless it's close by Force it here.
    add_custom_command(
      TARGET energyplusapi
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -DRESOLVED_PYTHON_LIB=${RESOLVED_PYTHON_LIBRARY} -DEXECUTABLE_PATH=$<TARGET_FILE:energyplusapi> -P
              ${PROJECT_SOURCE_DIR}/cmake/PythonGetLibAndLinkUp.cmake)
  endif()
  install(
    CODE "execute_process(
      COMMAND ${CMAKE_COMMAND}
      -DRESOLVED_PYTHON_LIB=${RESOLVED_PYTHON_LIBRARY}
      -DEXECUTABLE_PATH=\${CMAKE_INSTALL_PREFIX}/energyplus${CMAKE_EXECUTABLE_SUFFIX}
      -P ${PROJECT_SOURCE_DIR}/cmake/PythonGetLibAndLinkUp.cmake
    )")
  if(APPLE)
    install(
      CODE "execute_process(
        COMMAND ${CMAKE_COMMAND}
        -DRESOLVED_PYTHON_LIB=${RESOLVED_PYTHON_LIBRARY}
        -DEPLUS_DYNAMIC_LIB_NAME=libenergyplusapi.${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}.dylib
        -DEXECUTABLE_PATH=\${CMAKE_INSTALL_PREFIX}/energyplus${CMAKE_EXECUTABLE_SUFFIX}
        -P ${PROJECT_SOURCE_DIR}/cmake/PythonFixUpOnMac.cmake
      )")
  endif()
  # along with the python standard lib
  add_custom_command(
    TARGET energyplusapi
    POST_BUILD # TODO: I don't think we want to quote the generator expression
    COMMAND ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/cmake/PythonCopyStandardLib.py" "$<TARGET_FILE:energyplusapi>" "python_standard_lib")
endif()

if(BUILD_PACKAGE)
  # if we are building package, we need to drop in some API/Plugin stuff
  if(LINK_WITH_PYTHON)
    # we'll want to grab the standard lib for python plugins
    # TODO: I don't think we want to quote the generator expression
    install(DIRECTORY "$<TARGET_FILE_DIR:energyplus>/python_standard_lib/" DESTINATION "./python_standard_lib")
  endif()
  # we'll want to always provide the C API headers
  install(FILES ${API_HEADERS} DESTINATION "./include/EnergyPlus/api")
  # we'll want to always grab the python API wrapper
  # TODO: I don't think we want to quote the generator expression
  install(DIRECTORY "$<TARGET_FILE_DIR:energyplus>/pyenergyplus/" DESTINATION "./pyenergyplus")
endif()

if(BUILD_TESTING)
  # Build the test executable
  add_executable(TestEnergyPlusCallbacks test_ep_as_library.cc)
  target_link_libraries(TestEnergyPlusCallbacks PRIVATE energyplusapi project_options project_fp_options project_warnings)
  add_test(NAME "API.LegacyCallbackTest"
           COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DBINARY_DIR=${PROJECT_BINARY_DIR} -DIDF_FILE=1ZoneUncontrolled.idf
                   -DEPW_FILE=USA_CO_Golden-NREL.724666_TMY3.epw -P ${PROJECT_SOURCE_DIR}/cmake/RunCallbackTest.cmake)

  set(EPW_FILE "${PROJECT_SOURCE_DIR}/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw")
  set(IDF_FILE "${PROJECT_SOURCE_DIR}/testfiles/1ZoneUncontrolled.idf")

  add_executable(TestAPI_Functional_C ${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestFunctional.c)
  target_link_libraries(TestAPI_Functional_C PRIVATE energyplusapi project_options project_fp_options project_warnings)
  if(NOT MSVC)
    target_compile_options(TestAPI_Functional_C PRIVATE -Wno-unused -Wno-unused-parameter)
  endif()
  add_test(NAME "API.TestFunctionalC" COMMAND TestAPI_Functional_C)

  file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/tst/api/)

  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/runtime")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_executable(TestAPI_Runtime_C ${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntime.c)
  target_link_libraries(TestAPI_Runtime_C PRIVATE energyplusapi project_options project_fp_options)
  # target_compile_options(TestAPI_Runtime_C PRIVATE -Wall -Wextra -Wno-unused)
  add_test(NAME "API.TestRuntimeC" COMMAND TestAPI_Runtime_C -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")

  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/runtimeresetpython")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_executable(TestAPI_RuntimeResetState_C ${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntimeResetState.c)
  target_link_libraries(TestAPI_RuntimeResetState_C PRIVATE energyplusapi project_options project_fp_options)
  # target_compile_options(TestAPI_RuntimeResetPython_C PRIVATE -Wall -Wextra -Wno-unused)
  add_test(NAME "API.TestRuntimeResetStateC" COMMAND TestAPI_RuntimeResetState_C -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")

  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/runtimedeletepython")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_executable(TestAPI_RuntimeDeleteState_C ${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntimeDeleteState.c)
  target_link_libraries(TestAPI_RuntimeDeleteState_C PRIVATE energyplusapi project_options project_fp_options)
  # target_compile_options(TestAPI_RuntimeDeletePython_C PRIVATE -Wall -Wextra -Wno-unused)
  add_test(NAME "API.TestRuntimeDeleteStateC" COMMAND TestAPI_RuntimeDeleteState_C -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")

  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/exchange")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_executable(TestAPI_DataTransfer_C ${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestDataTransfer.c)
  target_link_libraries(TestAPI_DataTransfer_C PRIVATE energyplusapi project_options project_fp_options project_warnings)
  add_test(NAME "API.TestDataTransferC" COMMAND TestAPI_DataTransfer_C -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")

  # set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/autosizing") file(MAKE_DIRECTORY
  # ${TEST_DIR}) add_executable( TestAPI_Autosizing_C
  # ${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestAutosizing.c )
  # target_link_libraries( TestAPI_Autosizing_C energyplusapi ) add_test(NAME
  # "API.TestAutosizingC" COMMAND TestAPI_Autosizing_C -d "${TEST_DIR}" -w
  # "${EPW_FILE}" -D "${IDF_FILE}")

  # TODO: The Python API tests should depend on a completed build of energyplusapi

  set(DIR_WITH_PY_ENERGYPLUS $<TARGET_FILE_DIR:energyplusapi>)

  # Functional API does not need a run folder to be created
  add_test(NAME "API.TestFunctionalPython" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestFunctional.py")
  set_tests_properties("API.TestFunctionalPython" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})

  # Runtime tests need a folder to be created for output files, if we are building fortran, let's test that the python API properly sets the E+ install root by passing "-r"
  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/runtime_python")
  file(MAKE_DIRECTORY ${TEST_DIR})
  if(BUILD_FORTRAN)
    # TODO: This should depend on a completed build of readvars
    add_test(NAME "API.TestRuntimePython" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntime.py" -r -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")
  else()
    add_test(NAME "API.TestRuntimePython" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntime.py" -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")
  endif()
  set_tests_properties("API.TestRuntimePython" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})

  # Runtime tests need a folder to be created for output files
  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/runtime_resetstate_python")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_test(NAME "API.TestRuntimeResetStatePython" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntimeResetState.py" -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")
  set_tests_properties("API.TestRuntimeResetStatePython" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})

  # Runtime tests need a folder to be created for output files
  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/runtime_deletestate_python")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_test(NAME "API.TestRuntimeDeleteStatePython" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestRuntimeDeleteState.py" -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")
  set_tests_properties("API.TestRuntimeDeleteStatePython" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})

  # Data exchange tests also need an output folder
  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/exchange_python")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_test(NAME "API.TestDataTransferPython" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/TestDataTransfer.py" -d "${TEST_DIR}" -w "${EPW_FILE}" -D "${IDF_FILE}")
  set_tests_properties("API.TestDataTransferPython" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})

  set(TEST_DIR "${PROJECT_BINARY_DIR}/tst/api/plugin_tester_tester")
  file(MAKE_DIRECTORY ${TEST_DIR})
  add_test(NAME "API.PluginTesterTester" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/EnergyPlus/api/plugin_tester.py" "${PROJECT_SOURCE_DIR}/testfiles/PythonPluginCustomTrendVariable.py")
  set_tests_properties("API.PluginTesterTester" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})

  add_test(NAME "API.test_DataTransferTypes" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/test_DataTransferTypes.py")
  set_tests_properties("API.test_DataTransferTypes" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})
  add_test(NAME "API.test_SysExit" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/test_SysExit.py")
  set_tests_properties("API.test_SysExit" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})
  add_test(NAME "API.test_OutputFiles" COMMAND "${Python_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tst/EnergyPlus/api/test_OutputFiles.py")
  set_tests_properties("API.test_OutputFiles" PROPERTIES ENVIRONMENT PYTHONPATH=${DIR_WITH_PY_ENERGYPLUS})
endif()

if(UNIX AND NOT APPLE)
  add_custom_command(
    TARGET energyplus
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink energyplus EnergyPlus
    COMMAND ${CMAKE_COMMAND} -E rename EnergyPlus "${PROJECT_BINARY_DIR}/Products/EnergyPlus")

  if (BUILD_PERFORMANCE_TESTS)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/performance_tests")
    add_custom_command(
      TARGET energyplus
      POST_BUILD
      COMMAND size $<TARGET_FILE:energyplus> > "${PROJECT_BINARY_DIR}/performance_tests/size.energyplus"
      )
    add_custom_command(
      TARGET energyplusapi
      POST_BUILD
      COMMAND size $<TARGET_FILE:energyplusapi> > "${PROJECT_BINARY_DIR}/performance_tests/size.energyplusapi"
      )
  endif()
endif()
