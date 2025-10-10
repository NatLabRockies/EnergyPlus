# Force an annual simulation by calling add_simulation_test with these args
# IDF_FILE 1ZoneEvapCooler.idf EPW_FILE GBR_London.Gatwick.037760_IWEC.epw ANNUAL_SIMULATION
# or set the TEST_ANNUAL_SIMULATION cache variable to true and all files will run annual
# that is unless DESIGN_DAY_ONLY is set with args like this
# IDF_FILE 1ZoneEvapCooler.idf EPW_FILE GBR_London.Gatwick.037760_IWEC.epw DESIGN_DAY_ONLY
# This will override any attempt to run an annual simulation.  Use DESIGN_DAY_ONLY for files without annual run periods

# Putting a couple files first because they are very long running and CI ends up running them while all the others are done
# Hopefully putting them up here will get them to start sooner and better distribute CI resources over the build time
# OK, so HospitalLowEnergy is failing every single time on debug CI, it's not helping us at all, just being a false error
# I'm muting it on Debug builds for right now
if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_simulation_test(IDF_FILE HospitalLowEnergy.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
endif ()
add_simulation_test(IDF_FILE VSHeatPumpWaterToAirWithRHControl.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)

add_simulation_test(IDF_FILE 1ZoneDataCenterCRAC_wPumpedDXCoolingCoil.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneDataCenterCRAC_wApproachTemp.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneDataCenterCRAC_wApproachTemp_ClassH1.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 2ZoneDataCenterHVAC_wEconomizer.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 2ZoneDataCenterHVAC_wEconomizer_ClassA1A2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneEvapCooler.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneEvapCooler_4Win_incidentSolarMultiplier.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneEvapCooler_ReportPeriodPowerOutage.idf EPW_FILE USA_CO_Denver-Stapleton_TMY.epw)
add_simulation_test(IDF_FILE 1ZoneEvapCooler_ReportPeriodPowerOutage_naturalVent.idf EPW_FILE USA_CO_Denver-Stapleton_TMY.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE _1ZoneUncontrolled_9161.idf EPW_FILE weather_file_issue_9161.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled3SurfaceZone.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolledAnnualOutputs.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolledCondFDWithVariableKat24C.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolledFourAlgorithms.idf EPW_FILE 94810-1956.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolledResLayers.idf EPW_FILE 94810-1956.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolledUTF8.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_DD2009.idf EPW_FILE 94810-1953-1957.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_DDChanges.idf EPW_FILE 94810-1953-1957.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_OtherEquipmentWithFuel.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_variableThermalSolarAbs.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_FCfactor_Slab_UGWall.idf EPW_FILE 94810-1953-1957.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_win_1.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_win_2.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
ADD_SIMULATION_TEST(IDF_FILE 1ZoneUncontrolled_Win_ASH55_Thermal_Comfort.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_Win_ResilienceReports.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_Win_ResilienceReports_ReportingPeriod.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolled_Win_ResilienceReports_ReportingPeriod_AllTable.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneUncontrolledWithHysteresisPCM.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 1ZoneWith14ControlledHeat-CoolPanels.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 4ZoneWithShading_Simple_1.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 4ZoneWithShading_Simple_2.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledConvCoef.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledConvCoef_VSFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledKeepSiteLocation.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithSpaces.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithSpacesDaylightingIntMass.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithSpaceHeatBalance.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithSpacesHVAC.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledDemandLimiting.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledDemandLimiting_FixedRateVentilation.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledDemandLimiting_ReductionRatioVentilation.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithCoupledInGradeSlab.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithCoupledInGradeSlab_HorizInsThickness.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithDOASAirLoop.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooledWithDOASAirLoop_ExhaustSystem.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_AirBoundaries.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_AirBoundaries_Daylighting.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_LatentSizing_OtherEquipmentLatentLoads.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_RunaroundHeatRecovery.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_UniformLoading.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_VAVBoxMinAirFlowTurnDown.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_VRPSizing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_VRPSizing_MaxZd.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_ZoneAirMassFlowBalance.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCoolingPanelBaseboard.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCoolingPanelBaseboardAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCoolingPanelBaseboardTotalLoad.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCoolingPanelBaseboardVarOff.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAirCooled_ZoneAirMassFlowBalance_Pressurized.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneAutoDXVAV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneBoilerOutsideAirReset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneBranchSupplyPumps.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCAV_MaxTemp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCAV_MaxTemp_JSON_Outputs.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCAVtoVAVWarmestTempFlow.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCoolBeam.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneCostEst.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDDCycOnAny.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDDCycOnOne.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDesignInputCoolingCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDetailedIceStorage.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDetailedIceStorage2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDetailedIceStorageCubicLinear.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneDetailedIceStorageSimpleCtrl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneEconomicsTariffAndLifeCycleCosts.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneElectricBaseboard.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneEndUses.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneEngChill.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneFanCoilDOASCool.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneFanCoilDOAS_ERVOnAirLoopMainBranch.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneFanCoilDOAS_HumidifierOnOASystem.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneFPIU.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneGeometryTransform.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneIceStorage.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneNightVent1.idf EPW_FILE USA_CA_Fresno.Air.Terminal.723890_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneNightVent2.idf EPW_FILE USA_CA_Fresno.Air.Terminal.723890_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneNightVent3.idf EPW_FILE USA_CA_Fresno.Air.Terminal.723890_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneReturnFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneSteamBaseboard.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneSupRetPlenRAB.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneSupRetPlenVSATU.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneSwimmingPool.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneSwimmingPoolZoneMultipliers.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneSystemNodeReset.idf EPW_FILE USA_IL_Chicago-Ohare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneTDV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneVAV-ChilledWaterStorage-Mixed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneVAV-ChilledWaterStorage-Mixed_DCV_MaxZd.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneVAV-ChilledWaterStorage-Mixed_DCV_MultiPath.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneVAV-ChilledWaterStorage-Stratified.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneVAV-Pri-SecLoop.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneVAV-Pri-SecLoop_csv.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWLHPPlantLoopTower.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmestMultDDSizBypass.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmestMultDDSizOnOff.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmestMultDDSizVAV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmestMultDDSizVT.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmestVFD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWarmestVFD_FCMAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterCooled_Baseboard.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterCooled_BaseboardScalableSizing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterCooled_GasFiredSteamHumidifier.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterCooled_HighRHControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterCooled_MultizoneAverageRHControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterCooled_MultizoneMinMaxRHControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterLoopHeatPump.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5Zone_IdealLoadsAirSystems_ReturnPlenum.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5Zone_Transformer.idf EPW_FILE CZ06RV2.epw)
add_simulation_test(IDF_FILE 5Zone_Unitary_HXAssistedCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5Zone_Unitary_VSDesuperheater.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5Zone_Unitary_VSDesuperheatWaterHeater.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterSystems.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE 5ZoneWaterSystems_NoSitePrec.idf EPW_FILE USA_FL_Orlando.Intl.AP.722050_TMY3.epw)
add_simulation_test(IDF_FILE AbsorptionChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ActiveTrombeWall.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AdaptiveThermostatASH55_5Zone_Miami.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE AirCooledElectricChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirEconomizerFaults_RefBldgLargeOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirEconomizerWithMaxMinOAFractions.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork3zVent.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork3zVentAutoWPC.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetworkAdvanced_SingleSided_NV.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetworkOccupantVentilationControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_Attic_Duct.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiAirLoops.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_House.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MULTIZONE_House_DuctSizing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_House_FanModel.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_House_OvercoolDehumid.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_House_TwoSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_LocalNode.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_SmallOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_SmallOffice_CoilHXAssistedDX.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_SmallOffice_GenericContam.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_SmallOffice_HeatRecoveryHXSL.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_MultiZone_SmallOffice_VAV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_Multizone_HorizontalOpening.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_PressureControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_Simple_House.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowNetwork_Simple_SmallOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowWindowsAndBetweenGlassBlinds.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AirflowWindowsAndBetweenGlassShades.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ASIHPMixedTank.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OfficeLarge_STD2019_Denver_ChillerEIR.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OfficeLarge_STD2019_Denver_Chiller205.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OfficeLarge_STD2019_Denver_Chiller205_Detailed.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE AtticRoof_RadiantBarriers.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE BaseBoardElectric.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CVRhMinHum.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CarrollMRT-RefBldgLargeOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CentralChillerHeaterSystem_Cooling_Heating.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CentralChillerHeaterSystem_Simultaneous_Cooling_Heating.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ChangeoverBypassVAV.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE ChangeoverBypassVAV_AirToAir.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE ChangeoverBypassVAV_AirToAirHeatPump.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE ChangeoverBypassVAV_MaxTemp.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE ChillerPartLoadCurve_RefBldgLargeOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_Daylighting_SouthVB45deg.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_Daylighting_SouthVerticalVB45deg.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_InShadeGasMix.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_MeasuredDeflectionAndShading.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_SchedSurfGains.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_SingleZone_Deflection.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_SingleZone_DoubleClearAir.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_SingleZone_Vacuum.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CmplxGlz_SmOff_IntExtShading.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoilWaterDesuperheating.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE CoilWaterDesuperheatingUnitary.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE CoilWaterDesuperheatingUnitaryCoilDX.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE CommonPipe_Pri-Sec.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CompSetPtControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CondFD1ZonePurchAirAutoSizeWithPCM.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ConstSpeedBranchPumpExercise.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Convection.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ConvectionAdaptiveSmallOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingCoilFreezingPrevention.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTowerDryBulbRangeOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTowerNomCap.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTowerRHRangeOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTowerWetBulbRangeOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTowerWithDBDeltaTempOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTowerWithWBDeltaTempOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_FluidBypass.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_MerkelVariableSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_SingleSpeed_MultiCell.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_TwoSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_TwoSpeed_CondEntTempReset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_TwoSpeed_MultiCell.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_VariableSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_VariableSpeed_CondEntTempReset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_VariableSpeed_CondEntTempReset_MultipleTowers.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_VariableSpeed_IdealCondEntTempSetpoint.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_VariableSpeed_IdealCondEntTempSetpoint_MultipleTowers.idf EPW_FILE
        USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CoolingTower_VariableSpeed_MultiCell.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CooltowerSimpleTest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CooltowerSimpleTestwithVentilation.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE CrossVent_1Zone_AirflowNetwork.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE CrossVent_1Zone_AirflowNetwork_with2CrossflowJets.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE CustomSolarVisibleSpectrum_RefBldgSmallOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DDAutoSize.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
if (NOT WIN32)
    add_simulation_test(IDF_FILE DElight-Detailed-Comparison.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE DElightCFSLightShelf.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE DElightCFSWindow.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
endif ()
add_simulation_test(IDF_FILE DOAS_wNeutralSupplyAir_wFanCoilUnits.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOASDXCOIL_wADPBFMethod.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOASDXCOIL_wADPBFMethod_NoReturnPath.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOASDualDuctSchool.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOAToFanCoilInlet.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOAToFanCoilSupply.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOAToPTAC.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOAToPTAC_DrawReturnAir.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOAToPTHP.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOAToUnitarySystem.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOAToUnitVentilator.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOAToVRF.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DOAToWaterToAirHPInlet.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOAToWaterToAirHPInlet_MultispeedFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DOAToWaterToAirHPSupply.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DXCoilSystemAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DaylightingDeviceShelf.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DaylightingDeviceTubular.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DesiccantCVRh.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DesiccantCVRhZoneRHCtrl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DesiccantCVRhZoneRHCtrl_AddedAutosize.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DesiccantDehumidifierWithAirToAirCoil.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DesiccantDehumidifierWithCompanionCoil.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE DirectIndirectEvapCoolers.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE DirectIndirectEvapCoolersVSAS.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE DisplacementVent_1ZoneOffice.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE DisplacementVent_Nat_AirflowNetwork.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE DisplacementVent_Nat_AirflowNetwork_AdaptiveComfort.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE DisplacementVent_VAV.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE DualDuctConstVolDamper.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DualDuctConstVolGasHC.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DualDuctVarVolDamper.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DualDuctWaterCoils.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE DynamicClothing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMPD5ZoneWaterCooled_HighRHControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSAirflowNetworkOpeningControlByHumidity.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSConstantVolumePurchasedAir.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSCurveOverride_PackagedTerminalHeatPump.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSCustomOutputVariable.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSCustomSchedule.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSDemandManager_LargeOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSDiscreteAirSystemSizes.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSLoadBasedMultiSpeedDXCoilOverrideControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSMultiSpeedHeatPumpOverrideControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSPlantLoopOverrideControl.idf EPW_FILE USA_FL_Orlando.Intl.AP.722050_TMY3.epw)
add_simulation_test(IDF_FILE EMSPlantOperation_largeOff.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSReplaceTraditionalManagers_LargeOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSSetpointBasedMultiSpeedDXCoilOverrideControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSThermochromicWindow.idf EPW_FILE USA_NV_Las.Vegas-McCarran.Intl.AP.723860_TMY3.epw)
add_simulation_test(IDF_FILE EMSTestMathAndKill.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw EXPECT_FATAL) # Expected to Fatal
add_simulation_test(IDF_FILE EMSUserDefined5ZoneAirCooled.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSUserDefinedWindACAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EMSWindowShadeControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EarthTubeSimpleTest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EarthTubeVerticalTest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EcoroofOrlando.idf EPW_FILE USA_FL_Orlando.Intl.AP.722050_TMY3.epw)
add_simulation_test(IDF_FILE EcoroofOrlando_NoSitePrec.idf EPW_FILE USA_FL_Orlando.Intl.AP.722050_TMY3.epw)
add_simulation_test(IDF_FILE ElectricChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ElectricEIRChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ElectricEIRChiller_EvapCooledCondenser.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ElectricEIRChillerHeatRecoveryAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EngineChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EquivalentLayerWindow.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EvaporativeFluidCooler.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE EvaporativeFluidCooler_TwoSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ExhFiredAbsorptionChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ExteriorLightsAndEq.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE FanCoilAutoSize.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE FanCoilAutoSizeScalableSizing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FanCoilAutoSize_ASHRAE90VariableFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FanCoilAutoSize_MultiSpeedFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FanCoil_HybridVent_VentSch.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_ChillerSWTSensorOffset_RefBldgLargeOfficeNew2004.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_CoilSATSensorOffset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_CondenserSWTSensorOffset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_FoulingAirFilter_RefBldgMediumOfficeNew2004.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE Fault_FoulingCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_FoulingChillerBoiler_RefBldgLargeOfficeNew2004.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_FoulingCoolingTower.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_FoulingEvapCooler_StripMallZoneEvapCooler.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Fault_HumidistatOffset_Supermarket.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE Fault_HumidistatOffset_ThermostatOffset_Supermarket.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE Fault_ThermostatOffset_RefBldgMediumOfficeNew2004.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE Flr_Rf_8Sides.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE FluidCooler.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FluidCoolerTwoSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FourPipeBeamLargeOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FreeCoolingChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FreeCoolingChiller_wWaterSideEconomizer.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Furnace.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceFuelOil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FurnacePLRHeatingCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceWithDXSystem.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceWithDXSystemComfortControl.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceWithDXSystemRHcontrol.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceWithDXSystemRHcontrol_cyclingfan.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceWithDXSystem_CoolingHXAssisted.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE FurnaceWithDXSystem_CoolingHXAssisted_NewDXCoil.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE GSHP-GLHE.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GSHP-GLHE-CalcGFunctions.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GSHP-GLHE-CalcGFunctions-cpg.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GSHPSimple-GLHE.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GSHP-Slinky.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GasTurbChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GasTurbChillerHeatRecoveryAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Generator_PVWatts.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE Generators.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Generators_Transformer.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GeneratorswithPV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GeneratorwithWindTurbine.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE GeometryTest.idf EPW_FILE USA_CO_Colorado.Springs-Peterson.Field.724660_TMY3.epw)
add_simulation_test(IDF_FILE GroundTempOSCCompactSched.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HPAirToAir_wSolarCollectorHWCoil.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HP_wICSSolarCollector.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HVACStandAloneERV_Economizer.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeaderedPumpsConSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeaderedPumpsVarSpeed.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPump.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPump_variableCrankcaseHeaterCapacity.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpAirToAirWithRHcontrol.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpCycFanWithEcono.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpIAQP_DCV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpIAQP_GenericContamControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpProportionalControl_DCV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpProportionalControl_DCVDesignRate.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpProportionalControl_DCVDesignOccAllZones.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpSecondaryCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpSimpleDCV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpVRP_DCV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpVSAS.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpVSAS_variableCrankcaseHeaterCapacity.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterHeater.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterHeaterStratified.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterToAir.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterToAirEquationFit_WaterHeatingDesuperheater_StratifiedTank.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterToAirEquationFit.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterToAirWithAntifreezeAndLatentModel.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterToAirWithAntifreezeAndLatentModel2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpWaterToAirWithRHControl.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE HeatPumpwithBiquadraticCurves.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatRecoveryElectricChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatRecoveryPlantLoop.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatRecoveryPlantLoopAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HeatRecoverywithStorageTank.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HospitalBaseline.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HospitalBaselineReheatReportEMS.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE House-2FurnaceAC-SequentialLoad.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE House-2FurnaceAC-SequentialLoadFractions.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE House-2FurnaceAC-SequentialUniformPLR.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE House-2FurnaceAC-UniformLoad.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE House-2FurnaceAC-UniformPLR.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HybridVentilationControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HybridVentilationControlGlobalSimple.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HybridZoneModel.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HybridModel_4Zone_Solve_Infiltration_free_floating.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE HybridModel_4Zone_Solve_PeopleCount_with_HVAC.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE IceStorage-Parallel.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE IceStorage-Series-ChillerDownstream.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE IceStorage-Series-ChillerUpstream.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE IndEvapCoolerRTUoffice.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE IndirectAbsorptionChiller.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE IndoorLivingWall.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE InternalMass_wZoneList.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE Fenestration_RefBldgSmallHotelNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE LgOffVAV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE LrgOff_GridStorageDemandLeveling.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE LrgOff_GridStorageEMSSmoothing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE LrgOff_GridStorageScheduled.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE LookupTables.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE MicroCogeneration.idf EPW_FILE USA_NJ_Newark.Intl.AP.725020_TMY3.epw)
add_simulation_test(IDF_FILE Minimal.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MovableExtInsulationSimple.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MovableIntInsulationLights.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MovableIntInsulationLightsLowE.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MovableIntInsulationSimple.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MultiSpeedACFurnace.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MultiSpeedHP_StagedThermostat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MultiStory.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MultiSpeedHeatPump.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MultiSpeedHeatPump_DirectSolution.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE MultiSpeedHeatPump_MultiSolvers.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE Mundt_System_Always_On.idf EPW_FILE USA_FL_Tampa.Intl.AP.722110_TMY3.epw)
add_simulation_test(IDF_FILE Mundt_System_On_During_the_Day.idf EPW_FILE USA_FL_Tampa.Intl.AP.722110_TMY3.epw)
add_simulation_test(IDF_FILE OptimalStart_RefBldgLargeOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE OutdoorAirUnit.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE OutdoorAirUnitwithAirloopHVAC.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PIUAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PackagedTerminalAirConditioner.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE PackagedTerminalAirConditionerVSAS.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE PackagedTerminalHeatPump.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE PackagedTerminalHeatPumpVSAS.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE PackagedTerminalHeatPump_LatentSizing_SensibleAndLatent.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE PassiveTrombeWall.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PerformancePrecision_ASHRAE9012016_SchoolSecondary_Denver.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE PipeHeatTransfer_Outair.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PipeHeatTransfer_Schedule.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PipeHeatTransfer_Underground.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PipeHeatTransfer_Zone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PipingSystem_Underground_FHX.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE PipingSystem_Underground_TwoPipe.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE PipingSystem_Underground_TwoPipe_FD_GroundTemps.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE PipingSystem_Underground_TwoPipe_Xing_GroundTemps.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE PlantApplicationsGuide_Example1.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantApplicationsGuide_Example2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantApplicationsGuide_Example3.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantComponentTemperatureSource.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantHorizontalGroundHX.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoadProfile.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoadProfileCoolingReturnReset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoadProfileCoolingReturnResetLookup.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoadProfile_AutosizedDistrictHeating.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoadProfileSteam.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopChainCooling.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopChainDeadband.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopChainDualDeadband.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopChainHeating.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopHeatPump_EIR_AirSource.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopHeatPump_EIR_WaterSource.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopHeatPump_Fuel-Fired.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopHeatPump_EIR_Large-Office-2-AWHP-DedHR-AuxBoiler-Pri-Sec-HW.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopHeatPump_EIR_LargeOffice-2-AWHP-AuxBoiler-Pri-Sec-4PipeBeam.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantLoopHeatPump_EIR_AirSource_Hospital_wSixPipeHeatRecovery.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantPressureDrop.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantPressure_PumpCurve.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlantPressure_VFD_Scheduled.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlateHeatExchanger.idf EPW_FILE USA_FL_Tampa.Intl.AP.722110_TMY3.epw)
add_simulation_test(IDF_FILE Plenum.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PlenumwithRetAirHeatGain.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PondGroundHeatExchanger.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirTables.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirTables_SQL.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirTables_wAnnual.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirWindowBlind.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirWindowBlind_BlockBeamSolar.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirWithDaylighting.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirWithDaylightingAndShadeControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirWithDaylightingAngleFac.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE PurchAirWithDoubleFacadeDaylighting.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
if (LINK_WITH_PYTHON)
    add_simulation_test(IDF_FILE PythonPlugin1ZoneUncontrolledCondFD.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPlugin1ZoneUncontrolledTrackHistory.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginAirflowNetworkOpeningControlByHumidity.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginConstantVolumePurchasedAir.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginCurveOverride_PackagedTerminalHeatPump.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginCustomOutputVariable.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginCustomSchedule.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginDemandManager_LargeOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginDiscreteAirSystemSizes.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginLoadBasedMultiSpeedDXCoilOverrideControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginMultiSpeedHeatPumpOverrideControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPlugin_SingleFamilyHouse_TwoSpeed_MultiStageElectricSuppCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginPlantLoopOverrideControl.idf EPW_FILE USA_FL_Orlando.Intl.AP.722050_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginPlantOperation_largeOff.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginReplaceTraditionalManagers_LargeOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginTestMathAndKill.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw EXPECT_FATAL) # Expected to Fatal
    add_simulation_test(IDF_FILE PythonPluginThermochromicWindow.idf EPW_FILE USA_NV_Las.Vegas-McCarran.Intl.AP.723860_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginUserDefined5ZoneAirCooled.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginUserDefinedWindACAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginWindowShadeControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginLrgOff_GridStorageSmoothing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE PythonPluginCustomTrendVariable.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
endif ()
add_simulation_test(IDF_FILE QTFtest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadHiTempElecTermReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadHiTempGasCtrlOpt.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadHiTempGasTermReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoHydrHeatCoolAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoHydrHeatCoolAutoCondFD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloChangeoverDelay.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloCtrlOpt2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloCtrlOpt3.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloHeatCool.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloHeatCool_AddedAutosizing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloHeatCoolCondFD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloHeatCoolRMT.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloISOPipeHT.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempCFloTermReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempElecCtrlOpt2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempElecTermReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempElecTermReheatCondFD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrChangeoverDelay.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrCoolTower.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrCoolTowerCondFD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrCtrlOpt.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrCtrlOpt2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrCtrlOpt3.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrHeatCool.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrHeatCool2D.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrHeatCoolDry.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrHeatCoolDryCondFD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrHeatCoolMultiZone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrInterMulti.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrISOPipeHT.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrMulti10.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RadLoTempHydrTermReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgFullServiceRestaurantNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgHospitalNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgLargeHotelNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgLargeOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgLargeOfficeNew2004_Chicago-ReturnReset.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgMediumOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgMediumOfficeNew2004_Chicago_OutputControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgMediumOfficeNew2004_Chicago_JSON_Outputs.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgMediumOfficeNew2004_Chicago_epJSON.epJSON EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgMidriseApartmentNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgOutPatientNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgPrimarySchoolNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgQuickServiceRestaurantNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgSecondarySchoolNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgSmallHotelNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgSmallOfficeNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgStand-aloneRetailNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgStripMallNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgSuperMarketNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefBldgWarehouseNew2004_Chicago.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ReflectiveAdjacentBuilding.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefMedOffVAVAllDefVRP.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RefrigeratedWarehouse.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ReliefIndEvapCoolerRTUoffice.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE ReportDaylightFactors.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ReportHeatEmission_RefFSRestaurant.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RetailPackagedTESCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE RoomAirflowNetwork.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SeriesActiveBranch.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ShopWithPVandBattery.idf EPW_FILE USA_OK_Oklahoma.City-Will.Rogers.World.AP.723530_TMY3.epw)
# this file is failing on our debug check due to a NaN inside SSC
# the code is actually working as designed, but there are checks that
# rely on a NaN calculation as a sentinel value, which is triggering a FPE in our debug build
# for release builds it is working fine, and so it is included in normal builds and regression builds
# but for debug builds I am disabling it
if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_simulation_test(IDF_FILE ShopWithPVandLiIonBattery.idf EPW_FILE USA_OK_Oklahoma.City-Will.Rogers.World.AP.723530_TMY3.epw)
endif ()
add_simulation_test(IDF_FILE ShopWithPVandStorage.idf EPW_FILE USA_OK_Oklahoma.City-Will.Rogers.World.AP.723530_TMY3.epw)
add_simulation_test(IDF_FILE ShopWithSimplePVT.idf EPW_FILE USA_OK_Oklahoma.City-Will.Rogers.World.AP.723530_TMY3.epw)
add_simulation_test(IDF_FILE ShopWithBIPVT.idf EPW_FILE USA_OK_Oklahoma.City-Will.Rogers.World.AP.723530_TMY3.epw)
add_simulation_test(IDF_FILE SingleFamilyHouse_TwoSpeed_ZoneAirBalance.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SingleFamilyHouse_TwoSpeed_CutoutTemperature.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SingleFamilyHouse_TwoSpeed_MultiStageElectricSuppCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SingleFamilyHouse_TwoSpeed_MultiStageElectricSuppCoil_SPControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SingleFamilyHouse_TwoSpeed_MultiStageElectricSuppCoilEMS.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SmallOffice_CentralDOAS.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SmOffPSZ-MultiModeDX.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SmOffPSZ.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SmOffPSZ_OnOffStagedControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SolarCollectorFlatPlateWater.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTest.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTestGPU.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTest_SQL.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTest_ExternalFraction.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTest_DisableSelfShading.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTest_DisableSelfShadingGroup.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SolarShadingTest_ImportedShading.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SpectralAngularOpticalProperties_TableData.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE StackedZonesWithInterzoneIRTLayers.idf EPW_FILE USA_IL_University.of.Illinois-Willard.AP.725315_TMY3.epw)
add_simulation_test(IDF_FILE Standard621SimplifiedProcedure_PIUterminal.idf EPW_FILE USA_IL_Chicago-OHare_TMY2.epw)
add_simulation_test(IDF_FILE Standard621SimplifiedProcedure_VAVterminal.idf EPW_FILE USA_IL_Chicago-OHare_TMY2.epw)
add_simulation_test(IDF_FILE SteamSystemAutoSize.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SteamSystemAutoSize_DistrictHeatingSteam.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE StormWindow.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE StripMallZoneEvapCooler.idf EPW_FILE USA_CO_Boulder-Broomfield-Jefferson.County.AP.724699_TMY3.epw)
add_simulation_test(IDF_FILE StripMallZoneEvapCoolerRHcutoff.idf EPW_FILE USA_CO_Boulder-Broomfield-Jefferson.County.AP.724699_TMY3.epw)
add_simulation_test(IDF_FILE StripMallZoneEvapCoolerAutosized.idf EPW_FILE USA_CO_Boulder-Broomfield-Jefferson.County.AP.724699_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarketDetailed_DesuperHeatingCoil.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarket_DesuperHeatingCoil.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarket_DetailedEvapCondenser.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarket_DetailedWaterCondenser.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarket_EvapCondenser.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarket_SharedEvapCondenser.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SuperMarket_WaterCondenser.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE Supermarket.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SupermarketSecondary.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SupermarketSubCoolersVariableSuction.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SupermarketTranscriticalCO2.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SupermarketTwoStageFlashIntercooler.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SupermarketTwoStageShellCoilIntercooler.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE Supermarket_CascadeCond.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE Supermarket_Detailed.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE Supermarket_SharedAirCondenser.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE SupplyPlenumVAV.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SurfaceGroundHeatExchanger.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SurfaceHeatSourceTerm_RefBldgSmallOfficeNew2004.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SurfacePropGroundSurfaces_LWR.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SurfacePropTest_SurfLWR.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE SurfaceTest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE SurfaceZonePropTest_LocalEnv.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE TRHConstFlowChillerOneBranch.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TRHEvapCoolerOAStaged.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE TRHEvapCoolerOAStagedWetCoil.idf EPW_FILE USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw)
add_simulation_test(IDF_FILE TermRHDXSystem.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermRHGasElecCoils.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermReheatPri-SecLoop.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermReheatScheduledPump.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermReheatSurfTC.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermReheatZoneExh.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermRhDualSetpointWithDB.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermRhGenericOAHeatRecMinExh.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermRhGenericOAHeatRecPreheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TermRhSingleHeatCoolNoDB.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ThermalChimneyTest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ThermochromicWindow.idf EPW_FILE USA_NV_Las.Vegas-McCarran.Intl.AP.723860_TMY3.epw)
add_simulation_test(IDF_FILE TransparentInsulationSimple.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE TranspiredCollectors.idf EPW_FILE USA_CO_Boulder-Broomfield-Jefferson.County.AP.724699_TMY3.epw)
add_simulation_test(IDF_FILE TwoWayCommonPipe_Pri-Sec.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitHeater.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitHeaterAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitHeaterGasElec.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitVent5Zone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitVent5ZoneAuto-CycFan2-Variable-MinOAOn-Annual-SelectMonths-Detailed.idf EPW_FILE
        USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitVent5ZoneAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitVent5ZoneFixedOANoCoilOpt.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitaryHybridAC_DedicatedOutsideAir.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_5ZoneWaterLoopHeatPump.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_DXCoilSystemAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_FurnaceWithDXSystemRHcontrol.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_HeatPumpAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_MultiSpeedCoils_SingleMode.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_MultiSpeedDX.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_SingleSpeedDX.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_SubcoolReheatDX.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_TwoSpeedDX.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_TwoStageDX.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_TwoStageDXWithHumidityControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_VSCoolingCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_VSCoolingCoil_2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_VSCoolingCoil_ASHRAE205.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_VSHeatPumpWaterToAirEquationFit.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_WaterCoils_wMultiSpeedFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UnitarySystem_ZoneVSWSHP_wDOAS.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE UserDefinedRoomAirPatterns.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE UserInputViewFactorFile-LshapedZone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctConstFlowBoiler.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctReheatBaseboard.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctReheatNoReheat.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctReheat_DualMax.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctReheat_MaxSAT_ReverseActing.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VAVSingleDuctVarFlowBoiler.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VSDXCoilSystemAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VSHeatPumpWaterHeater.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE VSHeatPumpWaterToAirEquationFit.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VSWaterHeaterHeatPumpStratifiedTank.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VaryingLocationAndOrientation.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE VariableRefrigerantFlow_5Zone.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE VariableRefrigerantFlow_5Zone_wAirloop.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE VariableRefrigerantFlow_FluidTCtrl_5Zone.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE VariableRefrigerantFlow_FluidTCtrl_HR_5Zone.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE VariableRefrigerantFlow_FluidTCtrl_wSuppHeater_5Zone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VariableRefrigerantFlow_wSuppHeater_5Zone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VentilatedSlab.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VentilatedSlab_SeriesSlabs.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VentilationSimpleTest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE VRFMultispeedFan.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterCooledChillerWithVSCondenserControl.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterHeaterDHWPlantLoop.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterHeaterHeatPumpStratifiedTank.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterHeaterHeatPumpWrappedCondenser.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE WaterHeaterStandAlone.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterSideEconomizer_Integrated.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterSideEconomizer_NonIntegrated.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WaterSideEconomizer_PreCoolCoil.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WeatherTimeBins.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WindACAirtoAir.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WindACAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WindACRHControl.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE WindowTests.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE WindowTestsSimple.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE WWHPSimpleAuto.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledGroundHTBasement.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledGroundHTSlabInGrade.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledGroundHTSlabOnGrade.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaSlab.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaBasement.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaWalkoutBasement.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaRefBldgMediumOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaRefBldgMediumOfficeNoClg.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaBasementAdaptiveConvection.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaConvection.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneCoupledKivaConvectionAdaptiveSmallOffice.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE ZoneSysAvailManager.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE ZoneVSWSHP_wDOAS.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE ZoneWSHP_wDOAS.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _1ZoneUncontrolled_Feb29.idf EPW_FILE 94810-1956.epw)
add_simulation_test(IDF_FILE _1ZoneUncontrolled_SineOSC.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE _1ZoneUncontrolled_customrange.idf EPW_FILE 94810-1953-1957.epw)
add_simulation_test(IDF_FILE _1Zone_Heavy_AdiabaticX2.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE _1Zone_Heavy_MassX2.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE _1Zone_Heavy_SelfRef.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE _5ZoneEvapCooled.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _AllOffOpScheme.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _BranchPumpsWithCommonPipe.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _CrashLights_9680.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _CTFTestsPart1.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE _CTFTestsPart2.idf EPW_FILE USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw)
add_simulation_test(IDF_FILE _ConvCoefftest.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _CoolingTowerDewPointRangeOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _CoolingTowerWithDPDeltaTempOp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _DOASDXCOIL_wUserSHRMethod.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE _DemandVentilationFixedRateAndHighPriority.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _DemandVentilationReductionRatioAndHighPriority.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _DemandVentilationReductionRatioAndLowPriority.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _DualDuctConstVolDamperMultizoneAverageSetPointManager.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _ElectricREformulatedEIRChiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _ElectricASHRAE205Chiller.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _FanCoilHybridVentAFN.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw)
add_simulation_test(IDF_FILE _FollowSysNodeTemp.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _FuelCellTest200.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _HybridVentilationControlGlobalAN.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _HybridVentilationControl_MinTime.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _MaterialTest.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE _MicroCHPTest301.idf EPW_FILE USA_FL_Miami.Intl.AP.722020_TMY3.epw)
add_simulation_test(IDF_FILE _MultiSpeedACElecFurnace.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _PurchAirWindowBlind2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _PurchAirWindowBlind3.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _PurchAirWindowBlind4.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _SingleZoneTestCTFTwoDD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _SingleZoneTestCondFDTwoDD.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _SmallOffice_Dulles.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _VAVSingleDuctConstFlowBoiler_otherfuel.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE gasAbsorptionChillerHeater.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WCE_Diffuse_Shade.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WCE_DoubleClear_BSDF.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE WCE_Interior_VB_-45_deg.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE _5ZoneAirCooled_annual.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw ANNUAL_SIMULATION)
add_simulation_test(IDF_FILE _5ZoneAirCooled_LeapYear_annual.idf EPW_FILE MadeUpLeapYear.epw ANNUAL_SIMULATION)
add_simulation_test(IDF_FILE UnitarySystem_MultiSpeedDX_EconoStaging.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw ANNUAL_SIMULATION)
add_simulation_test(IDF_FILE _ResidentialBase.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw ANNUAL_SIMULATION)
add_simulation_test(IDF_FILE _ResidentialBaseboardHardSize.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)


# ASHRAE 90.1-2019 DOE commercial prototype models
add_simulation_test(IDF_FILE ASHRAE901_ApartmentHighRise_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_ApartmentMidRise_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_Hospital_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_HotelLarge_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_HotelSmall_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OfficeLarge_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OfficeMedium_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OfficeSmall_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_OutPatientHealthCare_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_RestaurantFastFood_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_RestaurantSitDown_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_RetailStandalone_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_RetailStripmall_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_SchoolPrimary_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_SchoolSecondary_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)
add_simulation_test(IDF_FILE ASHRAE901_Warehouse_STD2019_Denver.idf EPW_FILE USA_CO_Denver-Aurora-Buckley.AFB.724695_TMY3.epw)

# Additional files in subdirectories
add_simulation_test(IDF_FILE AdvancedOutput/ExerciseOutput1.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE AdvancedOutput/ExerciseOutput1A-Solution.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE BasicsFiles/Exercise1A.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE BasicsFiles/Exercise1B-Solution.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE BasicsFiles/Exercise1C-Solution.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE BasicsFiles/Exercise1D-Solution.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
add_simulation_test(IDF_FILE BasicsFiles/Exercise2.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)

# Macro files
add_simulation_test(IDF_FILE AbsorptionChiller_Macro.imf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw ENERGYPLUS_FLAGS -m)

add_api_simulation_test(PYTHON_FILE user_defined_equipment.py)

# External interface files -- note they don't work on Mac, and they only exchange data for runperiods
if (NOT APPLE)
    add_simulation_test(IDF_FILE _ExternalInterface-functionalmockupunit-to-actuator.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw ANNUAL_SIMULATION)
    add_simulation_test(IDF_FILE _ExternalInterface-functionalmockupunit-to-schedule.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw ANNUAL_SIMULATION)
    add_simulation_test(IDF_FILE _ExternalInterface-functionalmockupunit-to-variable.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw ANNUAL_SIMULATION)
endif ()

if (BUILD_FORTRAN)
    # ExpandObjects dependent files
    add_simulation_test(
            IDF_FILE
            HAMT_DailyProfileReport.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HAMT_HourlyProfileReport.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneBaseboardHeat.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneConstantVolumeChillerBoiler.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneDualDuct.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneFanCoil-DOAS.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneFanCoil.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneFurnaceDX.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZonePTAC-DOAS.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZonePTAC-DOAS-TwoHX.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZonePTAC.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZonePTHP.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZonePackagedVAV.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZonePurchAir.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneUnitaryHeatPump.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneUnitarySystem.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneVAVFanPowered.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneVAVWaterCooled-ObjectReference.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneVAVWaterCooled.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneVRF.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            HVACTemplate-5ZoneWaterToAirHeatPumpTowerBoiler.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            LBuilding-G000.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            LBuilding-G090.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            LBuilding-G180.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            LBuilding-G270.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    # Parametric preprocessor dependent files
    add_simulation_test(
            IDF_FILE
            LBuildingAppGRotPar.idf
            EPW_FILE
            USA_FL_Miami.Intl.AP.722020_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            5)
    add_simulation_test(IDF_FILE 1ZoneParameterAspect.idf EPW_FILE USA_CO_Golden-NREL.724666_TMY3.epw COST 5)
    add_simulation_test(IDF_FILE ParametricInsulation-5ZoneAirCooled.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw COST 5)
    # Basement/slab dependent files
    add_simulation_test(IDF_FILE 5ZoneAirCooledWithSlab.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw COST 10)
    add_simulation_test(IDF_FILE LgOffVAVusingBasement.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw COST 10)
    add_simulation_test(IDF_FILE SingleFamilyHouse_HP_Slab.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE SingleFamilyHouse_HP_Slab_Dehumidification.idf EPW_FILE USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
    add_simulation_test(IDF_FILE US+SF+CZ4A+hp+crawlspace+IECC_2006_VRF.idf EPW_FILE USA_NY_New.York-John.F.Kennedy.Intl.AP.744860_TMY3.epw)
    # Additional files in subdirectories that depend on ExpandObjects
    add_simulation_test(
            IDF_FILE
            BasicsFiles/AdultEducationCenter.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            BasicsFiles/Exercise2A-Solution.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            BasicsFiles/Exercise2B-Solution.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)
    add_simulation_test(
            IDF_FILE
            BasicsFiles/Exercise2C-Solution.idf
            EPW_FILE
            USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw
            ENERGYPLUS_FLAGS
            -x
            COST
            2)

    add_test(NAME integration.5ZoneCAV_MaxTemp_JSON_Outputs.CompareOutputReports
             COMMAND ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/dev/compare_output_reports.py" "${PROJECT_BINARY_DIR}/testfiles/5ZoneCAV_MaxTemp_JSON_Outputs/")

    set_tests_properties("integration.5ZoneCAV_MaxTemp_JSON_Outputs.CompareOutputReports" PROPERTIES DEPENDS "integration.5ZoneCAV_MaxTemp_JSON_Outputs")

endif ()
