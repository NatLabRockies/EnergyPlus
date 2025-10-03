// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef ENERGYPLUS_DATA_COMMONINCLUDES
#define ENERGYPLUS_DATA_COMMONINCLUDES

// OK, now that we have split the EnergyPlusData class into a header and source file, and we are moving member
// variables to pointer type, we no longer need to #include those headers in the Data header file.  But there are now
// other places where we do need those includes, and multiple spots actually.  So I am putting all the required includes
// here in this one file.  Basically, when a header inclusion is removed from EnergyPlusData.hh, put it here.

// these are needed so that we don't have incomplete types when we manage the EnergyPlusData instance
#include <AirflowNetwork/Properties.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/AirLoopHVACDOAS.hh>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Autosizing/BaseSizerWithFanHeatInputs.hh>
#include <EnergyPlus/Autosizing/BaseSizerWithScalableInputs.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BaseboardElectric.hh>
#include <EnergyPlus/BaseboardRadiator.hh>
#include <EnergyPlus/BoilerSteam.hh>
#include <EnergyPlus/Boilers.hh>
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/CTElectricGenerator.hh>
#include <EnergyPlus/ChilledCeilingPanelSimple.hh>
#include <EnergyPlus/ChillerAbsorption.hh>
#include <EnergyPlus/ChillerElectricASHRAE205.hh>
#include <EnergyPlus/ChillerElectricEIR.hh>
#include <EnergyPlus/ChillerExhaustAbsorption.hh>
#include <EnergyPlus/ChillerGasAbsorption.hh>
#include <EnergyPlus/ChillerIndirectAbsorption.hh>
#include <EnergyPlus/ChillerReformulatedEIR.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/CondenserLoopTowers.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/CoolTower.hh>
#include <EnergyPlus/CostEstimateManager.hh>
#include <EnergyPlus/CrossVentMgr.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/DXFEarClipping.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataDaylightingDevices.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACControllers.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataMoistureBalanceEMPD.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/DataPhotovoltaics.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataShadowingCombinations.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaceColors.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataTimings.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DemandManager.hh>
#include <EnergyPlus/DesiccantDehumidifiers.hh>
#include <EnergyPlus/DisplacementVentMgr.hh>
#include <EnergyPlus/DualDuct.hh>
#include <EnergyPlus/DuctLoss.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EarthTube.hh>
#include <EnergyPlus/EcoRoofManager.hh>
#include <EnergyPlus/EconomicLifeCycleCost.hh>
#include <EnergyPlus/EconomicTariff.hh>
#include <EnergyPlus/ElectricBaseboardRadiator.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/EvaporativeFluidCoolers.hh>
#include <EnergyPlus/ExhaustAirSystemManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/ExternalInterface.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidCoolers.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/FuelCellElectricGenerator.hh>
#include <EnergyPlus/Furnaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GeneratorFuelSupply.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/GroundTemperatureModeling/FiniteDifferenceGroundTemperatureModel.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/HVACCooledBeam.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HVACDuct.hh>
#include <EnergyPlus/HVACFourPipeBeam.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACInterfaceManager.hh>
#include <EnergyPlus/HVACManager.hh>
#include <EnergyPlus/HVACMultiSpeedHeatPump.hh>
#include <EnergyPlus/HVACSingleDuctInduc.hh>
#include <EnergyPlus/HVACSizingSimulationManager.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>
#include <EnergyPlus/HVACUnitaryBypassVAV.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HWBaseboardRadiator.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/HeatBalanceHAMTManager.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/HeatPumpWaterToWaterCOOLING.hh>
#include <EnergyPlus/HeatPumpWaterToWaterHEATING.hh>
#include <EnergyPlus/HeatPumpWaterToWaterSimple.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/HighTempRadiantSystem.hh>
#include <EnergyPlus/Humidifiers.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/HybridUnitaryAirConditioners.hh>
#include <EnergyPlus/ICEngineElectricGenerator.hh>
#include <EnergyPlus/IceThermalStorage.hh>
#include <EnergyPlus/IndoorGreen.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InputProcessing/InputValidation.hh>
#include <EnergyPlus/IntegratedHeatPump.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/LowTempRadiantSystem.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/MatrixDataManager.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/MicroturbineElectricGenerator.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/MoistureBalanceEMPDManager.hh>
#include <EnergyPlus/MundtSimMgr.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/OutputReportTabularAnnual.hh>
#include <EnergyPlus/OutputReports.hh>
#include <EnergyPlus/OutsideEnergySources.hh>
#include <EnergyPlus/PVWatts.hh>
#include <EnergyPlus/PackagedThermalStorageCoil.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>
#include <EnergyPlus/PhotovoltaicThermalCollectors.hh>
#include <EnergyPlus/Photovoltaics.hh>
#include <EnergyPlus/PipeHeatTransfer.hh>
#include <EnergyPlus/Pipes.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantCentralGSHP.hh>
#include <EnergyPlus/PlantChillers.hh>
#include <EnergyPlus/PlantComponentTemperatureSources.hh>
#include <EnergyPlus/PlantCondLoopOperation.hh>
#include <EnergyPlus/PlantHeatExchangerFluidToFluid.hh>
#include <EnergyPlus/PlantLoadProfile.hh>
#include <EnergyPlus/PlantLoopHeatPumpEIR.hh>
#include <EnergyPlus/PlantPipingSystemsManager.hh>
#include <EnergyPlus/PlantPressureSystem.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/PlantValves.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/PollutionModule.hh>
#include <EnergyPlus/PondGroundHeatExchanger.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/PsychCacheData.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/Pumps.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ResultsFramework.hh>
#include <EnergyPlus/ReturnAirPathManager.hh>
#include <EnergyPlus/RoomAirModelAirflowNetwork.hh>
#include <EnergyPlus/RoomAirModelManager.hh>
#include <EnergyPlus/RoomAirModelUserTempPattern.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SolarCollectors.hh>
#include <EnergyPlus/SolarReflectionManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/SteamBaseboardRadiator.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/SurfaceGroundHeatExchanger.hh>
#include <EnergyPlus/SwimmingPool.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/SystemReports.hh>
#include <EnergyPlus/TARCOGCommon.hh>
#include <EnergyPlus/TARCOGGasses90.hh>
#include <EnergyPlus/TARCOGMain.hh>
#include <EnergyPlus/TARCOGOutput.hh>
#include <EnergyPlus/TarcogShading.hh>
#include <EnergyPlus/ThermalChimney.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/ThermalISO15099Calc.hh>
#include <EnergyPlus/TranspiredCollector.hh>
#include <EnergyPlus/UFADManager.hh>
#include <EnergyPlus/UnitHeater.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UserDefinedComponents.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/Vectors.hh>
#include <EnergyPlus/VentilatedSlab.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WaterToAirHeatPump.hh>
#include <EnergyPlus/WaterToAirHeatPumpSimple.hh>
#include <EnergyPlus/WaterUse.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/WindTurbine.hh>
#include <EnergyPlus/WindowAC.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/WindowManagerExteriorData.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>
#include <EnergyPlus/ZoneContaminantPredictorCorrector.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#endif
