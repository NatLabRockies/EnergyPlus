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

#ifndef EnergyPlusData_hh_INCLUDED
#define EnergyPlusData_hh_INCLUDED

// C++ Headers
#include <memory>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/IOFiles.hh>

namespace EnergyPlus {

// forward declare all structs
struct AirLoopHVACDOASData;
struct AirSystemsData;
namespace AirflowNetwork {
    struct Solver;
} // namespace AirflowNetwork
struct BSDFWindowData;
struct BaseSizerWithFanHeatInputsData;
struct BaseSizerWithScalableInputsData;
struct BaseboardElectricData;
struct BaseboardRadiatorData;
struct BoilerSteamData;
struct BoilersData;
struct BranchInputManagerData;
struct BranchNodeConnectionsData;
struct CTElectricGeneratorData;
struct ChilledCeilingPanelSimpleData;
struct ChillerAbsorberData;
struct ChillerElectricEIRData;
struct ChillerExhaustAbsorptionData;
struct ChillerGasAbsorptionData;
struct ChillerIndirectAbsoprtionData;
struct ChillerReformulatedEIRData;
struct ChillerElectricASHRAE205Data;
struct CoilCoolingDXData;
struct CondenserLoopTowersData;
struct ConstructionData;
struct ContaminantBalanceData;
struct ConvectionCoefficientsData;
struct ConvergParamsData;
struct CoolTowerData;
struct CostEstimateManagerData;
struct CrossVentMgrData;
struct CurveManagerData;
struct DXCoilsData;
struct DXFEarClippingData;
struct DataAirLoopData;
struct DataBranchAirLoopPlantData;
struct DataDaylightingDevicesData;
struct DataGlobal;
struct DataInputProcessing;
struct DataPlantData;
struct DataStringGlobalsData;
struct DataTimingsData;
struct DataWaterData;
struct DataZoneControlsData;
struct DataZoneEnergyDemandsData;
struct DataZoneEquipmentData;
struct DaylightingDevicesData;
struct DaylightingData;
struct DefineEquipData;
struct DemandManagerData;
struct DesiccantDehumidifiersData;
struct DisplacementVentMgrData;
struct DualDuctData;
struct DuctLossData;
struct EIRFuelFiredHeatPumpsData;
struct EIRPlantLoopHeatPumpsData;
struct HeatPumpAirToWatersData;
struct EMSManagerData;
struct EarthTubeData;
struct EcoRoofManagerData;
struct EconomicLifeCycleCostData;
struct EconomicTariffData;
struct ElectPwrSvcMgrData;
struct ElectricBaseboardRadiatorData;
struct EnvironmentData;
struct ErrorTrackingData;
struct EvaporativeCoolersData;
struct EvaporativeFluidCoolersData;
struct ExteriorEnergyUseData;
struct ExternalInterfaceData;
struct FanCoilUnitsData;
struct FansData;
struct FaultsManagerData;
struct FluidCoolersData;
struct FluidData;
struct FourPipeBeamData;
struct FuelCellElectricGeneratorData;
struct FurnacesData;
struct GeneralData;
struct GeneralRoutinesData;
struct GeneratorFuelSupplyData;
struct GeneratorsData;
struct GlobalNamesData;
struct GroundHeatExchangerData;
struct GroundTemperatureManagerData;
struct HVACControllersData;
struct HVACCooledBeamData;
struct HVACCtrlData;
struct HVACDXHeatPumpSystemData;
struct HVACDuctData;
struct HVACGlobalsData;
struct HVACHXAssistedCoolingCoilData;
struct HVACInterfaceManagerData;
struct HVACManagerData;
struct HVACMultiSpeedHeatPumpData;
struct HVACSingleDuctInducData;
struct HVACSizingSimMgrData;
struct HVACStandAloneERVData;
struct HVACUnitaryBypassVAVData;
struct HVACVarRefFlowData;
struct HWBaseboardRadiatorData;
struct HeatBalFanSysData;
struct HeatBalFiniteDiffMgr;
struct HeatBalHAMTMgrData;
struct HeatBalInternalHeatGainsData;
struct HeatBalSurfData;
struct HeatBalSurfMgr;
struct HeatBalanceAirMgrData;
struct HeatBalanceData;
struct HeatBalanceIntRadExchgData;
struct HeatBalanceMgrData;
struct HeatPumpWaterToWaterCOOLINGData;
struct HeatPumpWaterToWaterHEATINGData;
struct HeatPumpWaterToWaterSimpleData;
struct HeatRecoveryData;
struct HeatingCoilsData;
struct HighTempRadiantSystemData;
struct HumidifiersData;
struct HybridModelData;
struct HybridUnitaryAirConditionersData;
struct HysteresisPhaseChangeData;
struct ICEngineElectricGeneratorData;
struct IPShortCutsData;
struct IceThermalStorageData;
struct IndoorGreenData;
struct IntegratedHeatPumpGlobalData;
struct InternalHeatGainsData;
struct LoopNodeData;
struct LowTempRadiantSystemData;
struct MaterialData;
struct MatrixDataManagerData;
struct MicroCHPElectricGeneratorData;
struct MicroturbineElectricGeneratorData;
struct MixedAirData;
struct MixerComponentData;
struct MoistureBalanceData;
struct MoistureBalanceEMPDData;
struct MoistureBalanceEMPDManagerData;
struct MundtSimMgrData;
struct NodeInputManagerData;
struct OutAirNodeManagerData;
struct OutdoorAirUnitData;
struct OutputProcessorData;
struct OutputReportPredefinedData;
struct OutputReportTabularAnnualData;
struct OutputReportTabularData;
struct OutputReportsData;
struct OutputsData;
struct OutsideEnergySourcesData;
struct PackagedThermalStorageCoilData;
struct PhotovoltaicStateData;
struct PhotovoltaicThermalCollectorsData;
struct PhotovoltaicsData;
struct PipeHeatTransferData;
struct PipesData;
struct PlantCentralGSHPData;
struct PlantChillersData;
struct PlantCompTempSrcData;
struct PlantCondLoopOperationData;
struct PlantHeatExchangerFluidToFluidData;
struct PlantLoadProfileData;
struct PlantMgrData;
struct PlantPipingSysMgrData;
struct PlantPressureSysData;
struct PlantUtilitiesData;
struct PlantValvesData;
struct PluginManagerData;
struct PollutionData;
struct PondGroundHeatExchangerData;
struct PoweredInductionUnitsData;
struct PsychrometricsData;
struct PsychrometricCacheData;
struct PumpsData;
struct PurchasedAirManagerData;
struct RefrigeratedCaseData;
struct ReportCoilSelectionData;
struct ReportFlagData;
struct ResultsFrameworkData;
struct ReturnAirPathMgr;
struct ExhaustAirSystemMgr;
struct ExhaustControlSystemMgr;
struct RoomAirModelAirflowNetworkData;
struct RoomAirModelData;
struct RoomAirModelUserTempPatternData;
struct RootFindingData;
struct RuntimeLanguageData;
struct RuntimeLanguageProcessorData;
struct SQLiteProceduresData;
struct ScheduleManagerData;
struct SetPointManagerData;
struct ShadowCombData;
struct SimAirServingZonesData;
struct SimulationManagerData;
struct SingleDuctData;
struct SizingData;
struct SizingManagerData;
struct SolarCollectorsData;
struct SolarReflectionManagerData;
struct SolarShadingData;
struct SplitterComponentData;
struct SteamBaseboardRadiatorData;
struct SteamCoilsData;
struct SurfaceColorData;
struct SurfaceGeometryData;
struct SurfaceGroundHeatExchangersData;
struct SurfaceListsData;
struct SurfacesData;
struct SwimmingPoolsData;
struct SystemAirFlowSizerData;
struct SystemAvailabilityManagerData;
struct SystemReportsData;
struct SystemVarsData;
struct TARCOGCommonData;
struct TARCOGOutputData;
struct ThermalChimneysData;
struct ThermalComfortsData;
struct ThermalISO15099CalcData;
struct TARCOGGasses90Data;
struct TARCOGMainData;
struct TarcogShadingData;
struct TranspiredCollectorData;
struct UFADManagerData;
struct UnitHeatersData;
struct UnitVentilatorsData;
struct UnitarySystemsData;
struct UserDefinedComponentsData;
struct UtilityRoutinesData;
struct VariableSpeedCoilsData;
struct VectorsData;
struct VentilatedSlabData;
struct ViewFactorInfoData;
struct WaterCoilsData;
struct WaterManagerData;
struct WaterThermalTanksData;
struct WaterToAirHeatPumpData;
struct WaterToAirHeatPumpSimpleData;
struct WaterUseData;
struct WeatherManagerData;
struct WindTurbineData;
struct WindowACData;
struct WindowComplexManagerData;
struct WindowEquivLayerData;
struct WindowEquivalentLayerData;
struct WindowManagerData;
struct WindowManagerExteriorData;
struct ZoneAirLoopEquipmentManagerData;
struct ZoneContaminantPredictorCorrectorData;
struct ZoneDehumidifierData;
struct ZoneEquipmentManagerData;
struct ZonePlenumData;
struct ZoneTempPredictorCorrectorData;

struct EnergyPlusData : BaseGlobalStruct
{
    bool ready = true;

    IOFiles files;

    // module globals
    std::unique_ptr<AirLoopHVACDOASData> dataAirLoopHVACDOAS;
    std::unique_ptr<AirSystemsData> dataAirSystemsData;
    std::unique_ptr<AirflowNetwork::Solver> afn;
    std::unique_ptr<BSDFWindowData> dataBSDFWindow;
    std::unique_ptr<BaseSizerWithFanHeatInputsData> dataBaseSizerFanHeatInputs;
    std::unique_ptr<BaseSizerWithScalableInputsData> dataBaseSizerScalableInputs;
    std::unique_ptr<BaseboardElectricData> dataBaseboardElectric;
    std::unique_ptr<BaseboardRadiatorData> dataBaseboardRadiator;
    std::unique_ptr<BoilerSteamData> dataBoilerSteam;
    std::unique_ptr<BoilersData> dataBoilers;
    std::unique_ptr<BranchInputManagerData> dataBranchInputManager;
    std::unique_ptr<BranchNodeConnectionsData> dataBranchNodeConnections;
    std::unique_ptr<CTElectricGeneratorData> dataCTElectricGenerator;
    std::unique_ptr<ChilledCeilingPanelSimpleData> dataChilledCeilingPanelSimple;
    std::unique_ptr<ChillerAbsorberData> dataChillerAbsorber;
    std::unique_ptr<ChillerElectricEIRData> dataChillerElectricEIR;
    std::unique_ptr<ChillerExhaustAbsorptionData> dataChillerExhaustAbsorption;
    std::unique_ptr<ChillerGasAbsorptionData> dataChillerGasAbsorption;
    std::unique_ptr<ChillerIndirectAbsoprtionData> dataChillerIndirectAbsorption;
    std::unique_ptr<ChillerReformulatedEIRData> dataChillerReformulatedEIR;
    std::unique_ptr<ChillerElectricASHRAE205Data> dataChillerElectricASHRAE205;
    std::unique_ptr<CoilCoolingDXData> dataCoilCoolingDX;
    std::unique_ptr<CondenserLoopTowersData> dataCondenserLoopTowers;
    std::unique_ptr<ConstructionData> dataConstruction;
    std::unique_ptr<ContaminantBalanceData> dataContaminantBalance;
    std::unique_ptr<ConvectionCoefficientsData> dataConvect;
    std::unique_ptr<ConvergParamsData> dataConvergeParams;
    std::unique_ptr<CoolTowerData> dataCoolTower;
    std::unique_ptr<CostEstimateManagerData> dataCostEstimateManager;
    std::unique_ptr<CrossVentMgrData> dataCrossVentMgr;
    std::unique_ptr<CurveManagerData> dataCurveManager;
    std::unique_ptr<DXCoilsData> dataDXCoils;
    std::unique_ptr<DXFEarClippingData> dataDXFEarClipping;
    std::unique_ptr<DataAirLoopData> dataAirLoop;
    std::unique_ptr<DataBranchAirLoopPlantData> dataBranchAirLoopPlant;
    std::unique_ptr<DataDaylightingDevicesData> dataDaylightingDevicesData;
    std::unique_ptr<DataGlobal> dataGlobal;
    std::unique_ptr<DataInputProcessing> dataInputProcessing;
    std::unique_ptr<DataPlantData> dataPlnt;
    std::unique_ptr<DataStringGlobalsData> dataStrGlobals;
    std::unique_ptr<DataTimingsData> dataTimingsData;
    std::unique_ptr<DataWaterData> dataWaterData;
    std::unique_ptr<DataZoneControlsData> dataZoneCtrls;
    std::unique_ptr<DataZoneEnergyDemandsData> dataZoneEnergyDemand;
    std::unique_ptr<DataZoneEquipmentData> dataZoneEquip;
    std::unique_ptr<DaylightingDevicesData> dataDaylightingDevices;
    std::unique_ptr<DaylightingData> dataDayltg;
    std::unique_ptr<DefineEquipData> dataDefineEquipment;
    std::unique_ptr<DemandManagerData> dataDemandManager;
    std::unique_ptr<DesiccantDehumidifiersData> dataDesiccantDehumidifiers;
    std::unique_ptr<DisplacementVentMgrData> dataDispVentMgr;
    std::unique_ptr<DualDuctData> dataDualDuct;
    std::unique_ptr<DuctLossData> dataDuctLoss;
    std::unique_ptr<EIRFuelFiredHeatPumpsData> dataEIRFuelFiredHeatPump;
    std::unique_ptr<EIRPlantLoopHeatPumpsData> dataEIRPlantLoopHeatPump;
    std::unique_ptr<HeatPumpAirToWatersData> dataHeatPumpAirToWater;
    std::unique_ptr<EMSManagerData> dataEMSMgr;
    std::unique_ptr<EarthTubeData> dataEarthTube;
    std::unique_ptr<EcoRoofManagerData> dataEcoRoofMgr;
    std::unique_ptr<EconomicLifeCycleCostData> dataEconLifeCycleCost;
    std::unique_ptr<EconomicTariffData> dataEconTariff;
    std::unique_ptr<ElectPwrSvcMgrData> dataElectPwrSvcMgr;
    std::unique_ptr<ElectricBaseboardRadiatorData> dataElectBaseboardRad;
    std::unique_ptr<EnvironmentData> dataEnvrn;
    std::unique_ptr<ErrorTrackingData> dataErrTracking;
    std::unique_ptr<EvaporativeCoolersData> dataEvapCoolers;
    std::unique_ptr<EvaporativeFluidCoolersData> dataEvapFluidCoolers;
    std::unique_ptr<ExteriorEnergyUseData> dataExteriorEnergyUse;
    std::unique_ptr<ExternalInterfaceData> dataExternalInterface;
    std::unique_ptr<FanCoilUnitsData> dataFanCoilUnits;
    std::unique_ptr<FansData> dataFans;
    std::unique_ptr<FaultsManagerData> dataFaultsMgr;
    std::unique_ptr<FluidCoolersData> dataFluidCoolers;
    std::unique_ptr<FluidData> dataFluid;
    std::unique_ptr<FourPipeBeamData> dataFourPipeBeam;
    std::unique_ptr<FuelCellElectricGeneratorData> dataFuelCellElectGen;
    std::unique_ptr<FurnacesData> dataFurnaces;
    std::unique_ptr<GeneralData> dataGeneral;
    std::unique_ptr<GeneralRoutinesData> dataGeneralRoutines;
    std::unique_ptr<GeneratorFuelSupplyData> dataGeneratorFuelSupply;
    std::unique_ptr<GeneratorsData> dataGenerator;
    std::unique_ptr<GlobalNamesData> dataGlobalNames;
    std::unique_ptr<GroundHeatExchangerData> dataGroundHeatExchanger;
    std::unique_ptr<GroundTemperatureManagerData> dataGrndTempModelMgr;
    std::unique_ptr<HVACControllersData> dataHVACControllers;
    std::unique_ptr<HVACCooledBeamData> dataHVACCooledBeam;
    std::unique_ptr<HVACCtrlData> dataHVACCtrl;
    std::unique_ptr<HVACDXHeatPumpSystemData> dataHVACDXHeatPumpSys;
    std::unique_ptr<HVACDuctData> dataHVACDuct;
    std::unique_ptr<HVACGlobalsData> dataHVACGlobal;
    std::unique_ptr<HVACHXAssistedCoolingCoilData> dataHVACAssistedCC;
    std::unique_ptr<HVACInterfaceManagerData> dataHVACInterfaceMgr;
    std::unique_ptr<HVACManagerData> dataHVACMgr;
    std::unique_ptr<HVACMultiSpeedHeatPumpData> dataHVACMultiSpdHP;
    std::unique_ptr<HVACSingleDuctInducData> dataHVACSingleDuctInduc;
    std::unique_ptr<HVACSizingSimMgrData> dataHVACSizingSimMgr;
    std::unique_ptr<HVACStandAloneERVData> dataHVACStandAloneERV;
    std::unique_ptr<HVACUnitaryBypassVAVData> dataHVACUnitaryBypassVAV;
    std::unique_ptr<HVACVarRefFlowData> dataHVACVarRefFlow;
    std::unique_ptr<HWBaseboardRadiatorData> dataHWBaseboardRad;
    std::unique_ptr<HeatBalFanSysData> dataHeatBalFanSys;
    std::unique_ptr<HeatBalFiniteDiffMgr> dataHeatBalFiniteDiffMgr;
    std::unique_ptr<HeatBalHAMTMgrData> dataHeatBalHAMTMgr;
    std::unique_ptr<HeatBalInternalHeatGainsData> dataHeatBalIntHeatGains;
    std::unique_ptr<HeatBalSurfData> dataHeatBalSurf;
    std::unique_ptr<HeatBalSurfMgr> dataHeatBalSurfMgr;
    std::unique_ptr<HeatBalanceAirMgrData> dataHeatBalAirMgr;
    std::unique_ptr<HeatBalanceData> dataHeatBal;
    std::unique_ptr<HeatBalanceIntRadExchgData> dataHeatBalIntRadExchg;
    std::unique_ptr<HeatBalanceMgrData> dataHeatBalMgr;
    std::unique_ptr<HeatPumpWaterToWaterCOOLINGData> dataHPWaterToWaterClg;
    std::unique_ptr<HeatPumpWaterToWaterHEATINGData> dataHPWaterToWaterHtg;
    std::unique_ptr<HeatPumpWaterToWaterSimpleData> dataHPWaterToWaterSimple;
    std::unique_ptr<HeatRecoveryData> dataHeatRecovery;
    std::unique_ptr<HeatingCoilsData> dataHeatingCoils;
    std::unique_ptr<HighTempRadiantSystemData> dataHighTempRadSys;
    std::unique_ptr<HumidifiersData> dataHumidifiers;
    std::unique_ptr<HybridModelData> dataHybridModel;
    std::unique_ptr<HybridUnitaryAirConditionersData> dataHybridUnitaryAC;
    std::unique_ptr<HysteresisPhaseChangeData> dataHysteresisPhaseChange;
    std::unique_ptr<ICEngineElectricGeneratorData> dataICEngElectGen;
    std::unique_ptr<IPShortCutsData> dataIPShortCut;
    std::unique_ptr<IceThermalStorageData> dataIceThermalStorage;
    std::unique_ptr<IndoorGreenData> dataIndoorGreen;
    std::unique_ptr<IntegratedHeatPumpGlobalData> dataIntegratedHP;
    std::unique_ptr<InternalHeatGainsData> dataInternalHeatGains;
    std::unique_ptr<LoopNodeData> dataLoopNodes;
    std::unique_ptr<LowTempRadiantSystemData> dataLowTempRadSys;
    std::unique_ptr<MaterialData> dataMaterial;
    std::unique_ptr<MatrixDataManagerData> dataMatrixDataManager;
    std::unique_ptr<MicroCHPElectricGeneratorData> dataCHPElectGen;
    std::unique_ptr<MicroturbineElectricGeneratorData> dataMircoturbElectGen;
    std::unique_ptr<MixedAirData> dataMixedAir;
    std::unique_ptr<MixerComponentData> dataMixerComponent;
    std::unique_ptr<MoistureBalanceData> dataMstBal;
    std::unique_ptr<MoistureBalanceEMPDData> dataMstBalEMPD;
    std::unique_ptr<MoistureBalanceEMPDManagerData> dataMoistureBalEMPD;
    std::unique_ptr<MundtSimMgrData> dataMundtSimMgr;
    std::unique_ptr<NodeInputManagerData> dataNodeInputMgr;
    std::unique_ptr<OutAirNodeManagerData> dataOutAirNodeMgr;
    std::unique_ptr<OutdoorAirUnitData> dataOutdoorAirUnit;
    std::unique_ptr<OutputProcessorData> dataOutputProcessor;
    std::unique_ptr<OutputReportPredefinedData> dataOutRptPredefined;
    std::unique_ptr<OutputReportTabularAnnualData> dataOutputReportTabularAnnual;
    std::unique_ptr<OutputReportTabularData> dataOutRptTab;
    std::unique_ptr<OutputReportsData> dataOutputReports;
    std::unique_ptr<OutputsData> dataOutput;
    std::unique_ptr<OutsideEnergySourcesData> dataOutsideEnergySrcs;
    std::unique_ptr<PackagedThermalStorageCoilData> dataPackagedThermalStorageCoil;
    std::unique_ptr<PhotovoltaicStateData> dataPhotovoltaicState;
    std::unique_ptr<PhotovoltaicThermalCollectorsData> dataPhotovoltaicThermalCollector;
    std::unique_ptr<PhotovoltaicsData> dataPhotovoltaic;
    std::unique_ptr<PipeHeatTransferData> dataPipeHT;
    std::unique_ptr<PipesData> dataPipes;
    std::unique_ptr<PlantCentralGSHPData> dataPlantCentralGSHP;
    std::unique_ptr<PlantChillersData> dataPlantChillers;
    std::unique_ptr<PlantCompTempSrcData> dataPlantCompTempSrc;
    std::unique_ptr<PlantCondLoopOperationData> dataPlantCondLoopOp;
    std::unique_ptr<PlantHeatExchangerFluidToFluidData> dataPlantHXFluidToFluid;
    std::unique_ptr<PlantLoadProfileData> dataPlantLoadProfile;
    std::unique_ptr<PlantMgrData> dataPlantMgr;
    std::unique_ptr<PlantPipingSysMgrData> dataPlantPipingSysMgr;
    std::unique_ptr<PlantPressureSysData> dataPlantPressureSys;
    std::unique_ptr<PlantUtilitiesData> dataPlantUtilities;
    std::unique_ptr<PlantValvesData> dataPlantValves;
    std::unique_ptr<PluginManagerData> dataPluginManager;
    std::unique_ptr<PollutionData> dataPollution;
    std::unique_ptr<PondGroundHeatExchangerData> dataPondGHE;
    std::unique_ptr<PoweredInductionUnitsData> dataPowerInductionUnits;
    std::unique_ptr<PsychrometricsData> dataPsychrometrics;
    std::unique_ptr<PsychrometricCacheData> dataPsychCache;
    std::unique_ptr<PumpsData> dataPumps;
    std::unique_ptr<PurchasedAirManagerData> dataPurchasedAirMgr;
    std::unique_ptr<RefrigeratedCaseData> dataRefrigCase;
    std::unique_ptr<ReportCoilSelectionData> dataRptCoilSelection;
    std::unique_ptr<ReportFlagData> dataReportFlag;
    std::unique_ptr<ResultsFrameworkData> dataResultsFramework;
    std::unique_ptr<ReturnAirPathMgr> dataRetAirPathMrg;
    std::unique_ptr<ExhaustAirSystemMgr> dataExhAirSystemMrg;
    std::unique_ptr<ExhaustControlSystemMgr> dataExhCtrlSystemMrg;
    std::unique_ptr<RoomAirModelAirflowNetworkData> dataRoomAirflowNetModel;
    std::unique_ptr<RoomAirModelData> dataRoomAir;
    std::unique_ptr<RoomAirModelUserTempPatternData> dataRoomAirModelTempPattern;
    std::unique_ptr<RootFindingData> dataRootFinder;
    std::unique_ptr<RuntimeLanguageData> dataRuntimeLang;
    std::unique_ptr<RuntimeLanguageProcessorData> dataRuntimeLangProcessor;
    std::unique_ptr<SQLiteProceduresData> dataSQLiteProcedures;
    std::unique_ptr<ScheduleManagerData> dataSched;
    std::unique_ptr<SetPointManagerData> dataSetPointManager;
    std::unique_ptr<ShadowCombData> dataShadowComb;
    std::unique_ptr<SimAirServingZonesData> dataSimAirServingZones;
    std::unique_ptr<SimulationManagerData> dataSimulationManager;
    std::unique_ptr<SingleDuctData> dataSingleDuct;
    std::unique_ptr<SizingData> dataSize;
    std::unique_ptr<SizingManagerData> dataSizingManager;
    std::unique_ptr<SolarCollectorsData> dataSolarCollectors;
    std::unique_ptr<SolarReflectionManagerData> dataSolarReflectionManager;
    std::unique_ptr<SolarShadingData> dataSolarShading;
    std::unique_ptr<SplitterComponentData> dataSplitterComponent;
    std::unique_ptr<SteamBaseboardRadiatorData> dataSteamBaseboardRadiator;
    std::unique_ptr<SteamCoilsData> dataSteamCoils;
    std::unique_ptr<SurfaceColorData> dataSurfColor;
    std::unique_ptr<SurfaceGeometryData> dataSurfaceGeometry;
    std::unique_ptr<SurfaceGroundHeatExchangersData> dataSurfaceGroundHeatExchangers;
    std::unique_ptr<SurfaceListsData> dataSurfLists;
    std::unique_ptr<SurfacesData> dataSurface;
    std::unique_ptr<SwimmingPoolsData> dataSwimmingPools;
    std::unique_ptr<SystemAirFlowSizerData> dataSysAirFlowSizer;
    std::unique_ptr<SystemAvailabilityManagerData> dataAvail;
    std::unique_ptr<SystemReportsData> dataSysRpts;
    std::unique_ptr<SystemVarsData> dataSysVars;
    std::unique_ptr<TARCOGCommonData> dataTARCOGCommon;
    std::unique_ptr<TARCOGOutputData> dataTARCOGOutputs;
    std::unique_ptr<ThermalChimneysData> dataThermalChimneys;
    std::unique_ptr<ThermalComfortsData> dataThermalComforts;
    std::unique_ptr<ThermalISO15099CalcData> dataThermalISO15099Calc;
    std::unique_ptr<TARCOGGasses90Data> dataTARCOGGasses90;
    std::unique_ptr<TARCOGMainData> dataTARCOGMain;
    std::unique_ptr<TarcogShadingData> dataTarcogShading;
    std::unique_ptr<TranspiredCollectorData> dataTranspiredCollector;
    std::unique_ptr<UFADManagerData> dataUFADManager;
    std::unique_ptr<UnitHeatersData> dataUnitHeaters;
    std::unique_ptr<UnitVentilatorsData> dataUnitVentilators;
    std::unique_ptr<UnitarySystemsData> dataUnitarySystems;
    std::unique_ptr<UserDefinedComponentsData> dataUserDefinedComponents;
    std::unique_ptr<UtilityRoutinesData> dataUtilityRoutines;
    std::unique_ptr<VariableSpeedCoilsData> dataVariableSpeedCoils;
    std::unique_ptr<VectorsData> dataVectors;
    std::unique_ptr<VentilatedSlabData> dataVentilatedSlab;
    std::unique_ptr<ViewFactorInfoData> dataViewFactor;
    std::unique_ptr<WaterCoilsData> dataWaterCoils;
    std::unique_ptr<WaterManagerData> dataWaterManager;
    std::unique_ptr<WaterThermalTanksData> dataWaterThermalTanks;
    std::unique_ptr<WaterToAirHeatPumpData> dataWaterToAirHeatPump;
    std::unique_ptr<WaterToAirHeatPumpSimpleData> dataWaterToAirHeatPumpSimple;
    std::unique_ptr<WaterUseData> dataWaterUse;
    std::unique_ptr<WeatherManagerData> dataWeather;
    std::unique_ptr<WindTurbineData> dataWindTurbine;
    std::unique_ptr<WindowACData> dataWindowAC;
    std::unique_ptr<WindowComplexManagerData> dataWindowComplexManager;
    std::unique_ptr<WindowEquivLayerData> dataWindowEquivLayer;
    std::unique_ptr<WindowEquivalentLayerData> dataWindowEquivalentLayer;
    std::unique_ptr<WindowManagerData> dataWindowManager;
    std::unique_ptr<WindowManagerExteriorData> dataWindowManagerExterior;
    std::unique_ptr<ZoneAirLoopEquipmentManagerData> dataZoneAirLoopEquipmentManager;
    std::unique_ptr<ZoneContaminantPredictorCorrectorData> dataZoneContaminantPredictorCorrector;
    std::unique_ptr<ZoneDehumidifierData> dataZoneDehumidifier;
    std::unique_ptr<ZoneEquipmentManagerData> dataZoneEquipmentManager;
    std::unique_ptr<ZonePlenumData> dataZonePlenum;
    std::unique_ptr<ZoneTempPredictorCorrectorData> dataZoneTempPredictorCorrector;

    EnergyPlusData();
    ~EnergyPlusData();

    // Cannot safely copy or delete this until we eradicate all remaining
    // calls to IOFiles::getSingleton and IOFiles::setSingleton
    EnergyPlusData(const EnergyPlusData &) = delete;
    EnergyPlusData(EnergyPlusData &&) = delete;

    // Ok, so what's up with these two?  Why do we need both of them?

    // First, what do they do?  init_constant_state creates and
    // initalizes state objects that are built into EnergyPlus and do
    // not appear in the IDF file.  Examples include the AlwaysOn and
    // AlwaysOff Schedule objects, the Water and Steam FluidProperties
    // objects, and perhaps other objects that will be identified as
    // refactoring continues.  init_state creates and initializes
    // state objects from the IDF file.

    // Why do we need both of them?  We actually don't for normal
    // EnergyPlus execution.  The EnergyPlus executable calls
    // init_constant_state() and init_state() back to back in
    // ManageSimulation() immediately after the IDF file is parsed.

    // The reason these are split is because of the unit testing
    // framework.  There are a good number of fixture objects that do
    // a lot of setup before reading the IDF snippet, and that setup
    // often requires some of hte constant objects.  The unit testing
    // framework calls init_constant_state in
    // EnergyPlusFixture::Setup() and then inidividual unit tests call
    // init_state() after calling process_idf().  In fact, we can
    // probably move init_state() into process_idf().

    // init_constant_state() also needs to be appropriately called by
    // the API, e.g., in resetState.
    void init_constant_state(EnergyPlusData &state) override;
    void init_state(EnergyPlusData &state) override;

    bool init_state_called = false;
    bool init_constant_state_called = false;

    void clear_state() override;
};

} // namespace EnergyPlus
#endif
