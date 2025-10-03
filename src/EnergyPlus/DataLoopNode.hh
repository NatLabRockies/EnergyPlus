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

#ifndef DataLoopNode_hh_INCLUDED
#define DataLoopNode_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

namespace DataLoopNode {

    enum class NodeFluidType
    {
        Invalid = -1,
        Blank, // TODO: remove, should be same as Invalid
        Air,
        Water,
        Steam,
        Electric, // TODO: Electric node "fluid" type?
        Num
    };

    enum class ConnectionType
    {
        Invalid = -1,
        Blank, // TODO: remove, should be same as Invalid
        Inlet,
        Outlet,
        Internal,
        ZoneNode,
        Sensor,
        Actuator,
        OutsideAir,
        ReliefAir,
        ZoneInlet,
        ZoneReturn,
        ZoneExhaust,
        SetPoint,
        Electric,
        OutsideAirReference,
        InducedAir,
        Num
    };

    constexpr Real64 SensedLoadFlagValue(-999.0);
    constexpr Real64 SensedNodeFlagValue(-999.0);

    // Valid IsParent Types for Node Connections
    constexpr bool ObjectIsParent(true);
    constexpr bool ObjectIsNotParent(false);
    constexpr bool IncrementFluidStreamYes(true);

    // Valid Fluid Types for Nodes
    constexpr static std::array<std::string_view, static_cast<int>(NodeFluidType::Num)> NodeFluidTypeNames = {
        "blank", "Air", "Water", "Steam", "Electric"};

    constexpr static std::array<std::string_view, static_cast<int>(NodeFluidType::Num)> NodeFluidTypeNamesUC = {
        "BLANK", "AIR", "WATER", "STEAM", "ELECTRIC"};

    constexpr static std::array<std::string_view, static_cast<int>(ConnectionType::Num)> ConnectionTypeNames = {"blank",
                                                                                                                "Inlet",
                                                                                                                "Outlet",
                                                                                                                "Internal",
                                                                                                                "ZoneNode",
                                                                                                                "Sensor",
                                                                                                                "Actuator",
                                                                                                                "OutdoorAir",
                                                                                                                "ReliefAir",
                                                                                                                "ZoneInlet",
                                                                                                                "ZoneReturn",
                                                                                                                "ZoneExhaust",
                                                                                                                "Setpoint",
                                                                                                                "Electric",
                                                                                                                "OutsideAirReference",
                                                                                                                "InducedAir"};

    constexpr static std::array<std::string_view, static_cast<int>(ConnectionType::Num)> ConnectionTypeNamesUC = {"BLANK",
                                                                                                                  "INLET",
                                                                                                                  "OUTLET",
                                                                                                                  "INTERNAL",
                                                                                                                  "ZONENODE",
                                                                                                                  "SENSOR",
                                                                                                                  "ACTUATOR",
                                                                                                                  "OUTDOORAIR",
                                                                                                                  "RELIEFAIR",
                                                                                                                  "ZONEINLET",
                                                                                                                  "ZONERETURN",
                                                                                                                  "ZONEEXHAUST",
                                                                                                                  "SETPOINT",
                                                                                                                  "ELECTRIC",
                                                                                                                  "OUTSIDEAIRREFERENCE",
                                                                                                                  "INDUCEDAIR"};

    enum class ConnectionObjectType
    {
        Invalid = -1,
        Undefined,
        AirConditionerVariableRefrigerantFlow,
        AirLoopHVAC,
        AirLoopHVACDedicatedOutdoorAirSystem,
        AirLoopHVACExhaustSystem,
        AirLoopHVACMixer,
        AirLoopHVACOutdoorAirsystem,
        AirLoopHVACReturnPath,
        AirLoopHVACReturnPlenum,
        AirLoopHVACSplitter,
        AirLoopHVACSupplyPath,
        AirLoopHVACSupplyPlenum,
        AirLoopHVACUnitaryFurnaceHeatCool,
        AirLoopHVACUnitaryFurnaceHeatOnly,
        AirLoopHVACUnitaryHeatCool,
        AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
        AirLoopHVACUnitaryHeatOnly,
        AirLoopHVACUnitaryHeatPumpAirToAir,
        AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
        AirLoopHVACUnitaryHeatPumpWaterToAir,
        AirLoopHVACUnitarySystem,
        AirLoopHVACZoneMixer,
        AirLoopHVACZoneSplitter,
        AirTerminalDualDuctConstantVolume,
        AirTerminalDualDuctConstantVolumeCool,
        AirTerminalDualDuctConstantVolumeHeat,
        AirTerminalDualDuctVAV,
        AirTerminalDualDuctVAVCool,
        AirTerminalDualDuctVAVHeat,
        AirTerminalDualDuctVAVOutdoorAir,
        AirTerminalDualDuctVAVOutdoorAirOutdoorAir,
        AirTerminalDualDuctVAVOutdoorAirRecirculatedAir,
        AirTerminalSingleDuctConstantVolumeCooledBeam,
        AirTerminalSingleDuctConstantVolumeFourPipeBeam,
        AirTerminalSingleDuctConstantVolumeFourPipeInduction,
        AirTerminalSingleDuctConstantVolumeNoReheat,
        AirTerminalSingleDuctConstantVolumeReheat,
        AirTerminalSingleDuctMixer,
        AirTerminalSingleDuctParallelPIUReheat,
        AirTerminalSingleDuctSeriesPIUReheat,
        AirTerminalSingleDuctUserDefined,
        AirTerminalSingleDuctVAVHeatAndCoolNoReheat,
        AirTerminalSingleDuctVAVHeatAndCoolReheat,
        AirTerminalSingleDuctVAVNoReheat,
        AirTerminalSingleDuctVAVReheat,
        AirTerminalSingleDuctVAVReheatVariableSpeedFan,
        AvailabilityManagerDifferentialThermostat,
        AvailabilityManagerHighTemperatureTurnOff,
        AvailabilityManagerHighTemperatureTurnOn,
        AvailabilityManagerLowTemperatureTurnOff,
        AvailabilityManagerLowTemperatureTurnOn,
        BoilerHotWater,
        BoilerSteam,
        Branch,
        CentralHeatPumpSystem,
        ChillerAbsorption,
        ChillerAbsorptionIndirect,
        ChillerCombustionTurbine,
        ChillerConstantCOP,
        ChillerElectric,
        ChillerElectricEIR,
        ChillerElectricReformulatedEIR,
        ChillerElectricASHRAE205,
        ChillerEngineDriven,
        ChillerHeaterAbsorptionDirectFired,
        ChillerHeaterAbsorptionDoubleEffect,
        CoilCoolingDX,
        CoilCoolingDXCurveFitSpeed,
        CoilCoolingDXMultiSpeed,
        CoilCoolingDXSingleSpeed,
        CoilCoolingDXSingleSpeedThermalStorage,
        CoilCoolingDXSubcoolReheat,
        CoilCoolingDXTwoSpeed,
        CoilCoolingDXTwoStageWithHumidityControlMode,
        CoilCoolingDXVariableRefrigerantFlow,
        CoilCoolingDXVariableRefrigerantFlowFluidTemperatureControl,
        CoilCoolingDXVariableSpeed,
        CoilCoolingWater,
        CoilCoolingWaterDetailedGeometry,
        CoilCoolingWaterToAirHeatPumpEquationFit,
        CoilCoolingWaterToAirHeatPumpParameterEstimation,
        CoilCoolingWaterToAirHeatPumpVariableSpeedEquationFit,
        CoilHeatingDXMultiSpeed,
        CoilHeatingDXSingleSpeed,
        CoilHeatingDXVariableRefrigerantFlow,
        CoilHeatingDXVariableRefrigerantFlowFluidTemperatureControl,
        CoilHeatingDXVariableSpeed,
        CoilHeatingDesuperheater,
        CoilHeatingElectric,
        CoilHeatingElectricMultiStage,
        CoilHeatingFuel,
        CoilHeatingGasMultiStage,
        CoilHeatingSteam,
        CoilHeatingWater,
        CoilHeatingWaterToAirHeatPumpEquationFit,
        CoilHeatingWaterToAirHeatPumpParameterEstimation,
        CoilHeatingWaterToAirHeatPumpVariableSpeedEquationFit,
        CoilUserDefined,
        CoilWaterHeatingAirToWaterHeatPumpPumped,
        CoilWaterHeatingAirToWaterHeatPumpVariableSpeed,
        CoilWaterHeatingAirToWaterHeatPumpWrapped,
        CoilWaterHeatingDesuperheater,
        CoilSystemCoolingDX,
        CoilSystemCoolingDXHeatExchangerAssisted,
        CoilSystemCoolingWater,
        CoilSystemCoolingWaterHeatExchangerAssisted,
        CoilSystemHeatingDX,
        CoilSystemIntegratedHeatPumpAirSource,
        Condenser,
        CondenserLoop,
        ConnectorMixer,
        ConnectorSplitter,
        ControllerOutdoorAir,
        ControllerWaterCoil,
        CoolingTowerSingleSpeed,
        CoolingTowerTwoSpeed,
        CoolingTowerVariableSpeed,
        CoolingTowerVariableSpeedMerkel,
        DehumidifierDesiccantNoFans,
        DehumidifierDesiccantSystem,
        DistrictCooling,
        DistrictHeatingWater,
        DistrictHeatingSteam,
        Duct,
        ElectricEquipmentITEAirCooled,
        EvaporativeCoolerDirectCelDekPad,
        EvaporativeCoolerDirectResearchSpecial,
        EvaporativeCoolerIndirectCelDekPad,
        EvaporativeCoolerIndirectResearchSpecial,
        EvaporativeCoolerIndirectWetCoil,
        EvaporativeFluidCoolerSingleSpeed,
        EvaporativeFluidCoolerTwoSpeed,
        FanComponentModel,
        FanConstantVolume,
        FanOnOff,
        FanSystemModel,
        FanVariableVolume,
        FanZoneExhaust,
        FluidCoolerSingleSpeed,
        FluidCoolerTwoSpeed,
        GeneratorCombustionTurbine,
        GeneratorFuelCellAirSupply,
        GeneratorFuelCellExhaustGasToWaterHeatExchanger,
        GeneratorFuelCellPowerModule,
        GeneratorFuelCellStackCooler,
        GeneratorFuelCellWaterSupply,
        GeneratorFuelSupply,
        GeneratorInternalCombustionEngine,
        GeneratorMicroCHP,
        GeneratorMicroTurbine,
        GroundHeatExchangerHorizontalTrench,
        GroundHeatExchangerPond,
        GroundHeatExchangerSlinky,
        GroundHeatExchangerSurface,
        GroundHeatExchangerSystem,
        HeaderedPumpsConstantSpeed,
        HeaderedPumpsVariableSpeed,
        HeatExchangerAirToAirFlatPlate,
        HeatExchangerAirToAirSensibleAndLatent,
        HeatExchangerDesiccantBalancedFlow,
        HeatExchangerFluidToFluid,
        HeatPumpFuelFiredCooling,
        HeatPumpFuelFiredHeating,
        HeatPumpPlantLoopEIRCooling,
        HeatPumpPlantLoopEIRHeating,
        HeatPumpAirToWaterCooling,
        HeatPumpAirToWaterHeating,
        HeatPumpAirToWater,
        HeatPumpWaterToWaterEquationFitCooling,
        HeatPumpWaterToWaterEquationFitHeating,
        HeatPumpWaterToWaterParameterEstimationCooling,
        HeatPumpWaterToWaterParameterEstimationHeating,
        HumidifierSteamElectric,
        HumidifierSteamGas,
        Lights,
        LoadProfilePlant,
        OutdoorAirMixer,
        OutdoorAirNode,
        OutdoorAirNodeList,
        PipeAdiabatic,
        PipeAdiabaticSteam,
        PipeIndoor,
        PipeOutdoor,
        PipeUnderground,
        PipingSystemUndergroundPipeCircuit,
        PlantComponentTemperatureSource,
        PlantComponentUserDefined,
        PlantEquipmentOperationChillerHeaterChangeover,
        PlantEquipmentOperationComponentSetpoint,
        PlantEquipmentOperationOutdoorDewpointDifference,
        PlantEquipmentOperationOutdoorDrybulbDifference,
        PlantEquipmentOperationOutdoorWetbulbDifference,
        PlantEquipmentOperationThermalEnergyStorage,
        PlantLoop,
        PumpConstantSpeed,
        PumpConstantVolume,
        PumpVariableSpeed,
        PumpVariableSpeedCondensate,
        RefrigerationCompressorRack,
        RefrigerationCondenserAirCooled,
        RefrigerationCondenserEvaporativeCooled,
        RefrigerationCondenserWaterCooled,
        RefrigerationGasCoolerAirCooled,
        SetpointManagerColdest,
        SetpointManagerCondenserEnteringReset,
        SetpointManagerCondenserEnteringResetIdeal,
        SetpointManagerFollowGroundTemperature,
        SetpointManagerFollowOutdoorAirTemperature,
        SetpointManagerFollowSystemNodeTemperature,
        SetpointManagerMixedAir,
        SetpointManagerMultiZoneCoolingAverage,
        SetpointManagerMultiZoneHeatingAverage,
        SetpointManagerMultiZoneHumidityMaximum,
        SetpointManagerMultiZoneHumidityMinimum,
        SetpointManagerMultiZoneMaximumHumidityAverage,
        SetpointManagerMultiZoneMinimumHumidityAverage,
        SetpointManagerOutdoorAirPretreat,
        SetpointManagerOutdoorAirReset,
        SetpointManagerReturnTemperatureChilledWater,
        SetpointManagerReturnTemperatureHotWater,
        SetpointManagerScheduled,
        SetpointManagerScheduledDualSetpoint,
        SetpointManagerSingleZoneCooling,
        SetpointManagerSingleZoneHeating,
        SetpointManagerSingleZoneHumidityMaximum,
        SetpointManagerSingleZoneHumidityMinimum,
        SetpointManagerSingleZoneOneStageCooling,
        SetpointManagerSingleZoneOneStageHeating,
        SetpointManagerSingleZoneReheat,
        SetpointManagerSystemNodeResetTemperature,
        SetpointManagerSystemNodeResetHumidity,
        SetpointManagerWarmest,
        SetpointManagerWarmestTemperatureFlow,
        SolarCollectorFlatPlatePhotovoltaicThermal,
        SolarCollectorFlatPlateWater,
        SolarCollectorIntegralCollectorStorage,
        SolarCollectorUnglazedTranspired,
        SurfacePropertyLocalEnvironment,
        SwimmingPoolIndoor,
        TemperingValve,
        ThermalStorageChilledWaterMixed,
        ThermalStorageChilledWaterStratified,
        ThermalStorageIceDetailed,
        ThermalStorageIceSimple,
        WaterHeaterHeatPump,
        WaterHeaterHeatPumpPumpedCondenser,
        WaterHeaterHeatPumpWrappedCondenser,
        WaterHeaterMixed,
        WaterHeaterStratified,
        WaterUseConnections,
        ZoneHVACAirDistributionUnit,
        ZoneHVACBaseboardConvectiveElectric,
        ZoneHVACBaseboardConvectiveWater,
        ZoneHVACBaseboardRadiantConvectiveElectric,
        ZoneHVACBaseboardRadiantConvectiveSteam,
        ZoneHVACBaseboardRadiantConvectiveWater,
        ZoneHVACCoolingPanelRadiantConvectiveWater,
        ZoneHVACDehumidifierDX,
        ZoneHVACEnergyRecoveryVentilator,
        ZoneHVACEquipmentConnections,
        ZoneHVACEvaporativeCoolerUnit,
        ZoneHVACExhaustControl,
        ZoneHVACForcedAirUserDefined,
        ZoneHVACFourPipeFanCoil,
        ZoneHVACHighTemperatureRadiant,
        ZoneHVACHybridUnitaryHVAC,
        ZoneHVACIdealLoadsAirSystem,
        ZoneHVACLowTemperatureRadiantConstantFlow,
        ZoneHVACLowTemperatureRadiantVariableFlow,
        ZoneHVACOutdoorAirUnit,
        ZoneHVACPackagedTerminalAirConditioner,
        ZoneHVACPackagedTerminalHeatPump,
        ZoneHVACRefrigerationChillerSet,
        ZoneHVACTerminalUnitVariableRefrigerantFlow,
        ZoneHVACUnitHeater,
        ZoneHVACUnitVentilator,
        ZoneHVACVentilatedSlab,
        ZoneHVACWaterToAirHeatPump,
        ZoneHVACWindowAirConditioner,
        ZonePropertyLocalEnvironment,
        SpaceHVACEquipmentConnections,
        SpaceHVACZoneEquipmentSplitter,
        SpaceHVACZoneEquipmentMixer,
        SpaceHVACZoneReturnMixer,
        Num,
    };

    // Types
    struct NodeData
    {
        // Members
        NodeFluidType FluidType = NodeFluidType::Blank;      // must be one of the valid parameters
        int FluidIndex = 0;                                  // For Fluid Properties
        Real64 Temp = 0.0;                                   // {C}
        Real64 TempMin = 0.0;                                // {C}
        Real64 TempMax = 0.0;                                // {C}
        Real64 TempSetPoint = SensedNodeFlagValue;           // {C}
        Real64 TempLastTimestep = 0.0;                       // [C}
        Real64 MassFlowRateRequest = 0.0;                    // {kg/s}
        Real64 MassFlowRate = 0.0;                           // {kg/s}
        Real64 MassFlowRateMin = 0.0;                        // {kg/s}
        Real64 MassFlowRateMax = SensedNodeFlagValue;        // {kg/s}
        Real64 MassFlowRateMinAvail = 0.0;                   // {kg/s}
        Real64 MassFlowRateMaxAvail = 0.0;                   // {kg/s}
        Real64 MassFlowRateSetPoint = 0.0;                   // {kg/s}
        Real64 Quality = 0.0;                                // {0.0-1.0 vapor fraction/percent}
        Real64 Press = DataEnvironment::StdPressureSeaLevel; // {Pa}
        Real64 Enthalpy = 0.0;                               // {J/kg}
        Real64 EnthalpyLastTimestep = 0.0;                   // {J/kg}
        Real64 HumRat = 0.0;                                 // {}
        Real64 HumRatMin = SensedNodeFlagValue;              // {}
        Real64 HumRatMax = SensedNodeFlagValue;              // {}
        Real64 HumRatSetPoint = SensedNodeFlagValue;         // {}
        Real64 TempSetPointHi = SensedNodeFlagValue;         // {C}
        Real64 TempSetPointLo = SensedNodeFlagValue;         // {C}
        Real64 Height = -1.0;                                // {m}

        //  Following are for Outdoor Air Nodes Scheduled Properties
        bool IsLocalNode = false;
        Sched::Schedule *outAirDryBulbSched = nullptr;
        Sched::Schedule *outAirWetBulbSched = nullptr;
        Sched::Schedule *outAirWindSpeedSched = nullptr;
        Sched::Schedule *outAirWindDirSched = nullptr;

        //  Following are for Outdoor Air Nodes "read only"
        Real64 OutAirDryBulb = 0.0;              // {C}
        bool EMSOverrideOutAirDryBulb = false;   // if true, the EMS is calling to override outdoor air node drybulb setting
        Real64 EMSValueForOutAirDryBulb = 0.0;   // value EMS is directing to use for outdoor air node's drybulb {C}
        Real64 OutAirWetBulb = 0.0;              // {C}
        bool EMSOverrideOutAirWetBulb = false;   // if true, the EMS is calling to override outdoor air node wetbulb setting
        Real64 EMSValueForOutAirWetBulb = 0.0;   // value EMS is directing to use for outdoor air node's wetbulb {C}
        Real64 OutAirWindSpeed = 0.0;            // {m/s}
        bool EMSOverrideOutAirWindSpeed = false; // if true, the EMS is calling to override outdoor air node wind speed setting
        Real64 EMSValueForOutAirWindSpeed = 0.0; // value EMS is directing to use for outdoor air node's drybulb {m/s}
        Real64 OutAirWindDir = 0.0;              // {degree}
        bool EMSOverrideOutAirWindDir = false;   // if true, the EMS is calling to override outdoor air node wind direction setting
        Real64 EMSValueForOutAirWindDir = 0.0;   // value EMS is directing to use for outdoor air node's wind directio {degree}
        // Contaminant
        Real64 CO2 = 0.0;                  // {ppm}
        Real64 CO2SetPoint = 0.0;          // {ppm}
        Real64 GenContam = 0.0;            // {ppm}
        Real64 GenContamSetPoint = 0.0;    // {ppm}
        bool SPMNodeWetBulbRepReq = false; // Set to true when node has SPM which follows wetbulb

        // error message flag
        bool plantNodeErrorMsgIssued = false;

        // Default Constructor
        NodeData() = default;
    };

    struct MoreNodeData
    {
        // Members
        Real64 RelHumidity = 0.0;        // {%}
        Real64 ReportEnthalpy = 0.0;     // specific enthalpy calculated at the HVAC timestep [J/kg]
        Real64 VolFlowRateStdRho = 0.0;  // volume flow rate at standard density [m3/s]
        Real64 VolFlowRateCrntRho = 0.0; // volume flow rate at current density, only used for air nodes [m3/s]
        Real64 WetBulbTemp = 0.0;        // wetbulb temperature [C]
        Real64 Density = 0.0;            // reported density at current temperature [kg/m3]
        Real64 AirDewPointTemp = 0.0;    // reported system node dewpoint temperature [C]
        Real64 SpecificHeat = 0.0;       // reported node specific heat [J/kg-C]
    };

    struct MarkedNodeData
    {
        // Members
        bool IsMarked = false;                                           // true if this is a marked node
        ConnectionObjectType ObjectType = ConnectionObjectType::Invalid; // Object Type that needs it "marked"
        std::string ObjectName;                                          // Object Name that needs it "marked"
        std::string FieldName;                                           // FieldName that needs it "marked"
    };

    // A struct to defer checking whether a node did correctly get a setpoint via the API / PythonPlugin
    struct NodeSetpointCheckData
    {
        bool needsSetpointChecking = false;
        std::array<bool, (int)HVAC::CtrlVarType::Num> checkSetPoint = {false, false, false, false, false, false, false, false, false};
    };
} // namespace DataLoopNode

struct LoopNodeData : BaseGlobalStruct
{

    int NumOfNodes = 0;
    int NumofSplitters = 0;
    int NumofMixers = 0;
    Array1D_string NodeID;
    Array1D<DataLoopNode::NodeData> Node; // dim to num nodes in SimHVAC
    DataLoopNode::NodeData DefaultNodeValues{DataLoopNode::NodeData()};
    Array1D<DataLoopNode::MoreNodeData> MoreNodeInfo;
    Array1D<DataLoopNode::MarkedNodeData> MarkedNode;
    Array1D<DataLoopNode::NodeSetpointCheckData> NodeSetpointCheck;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) LoopNodeData();
    }
};

} // namespace EnergyPlus

#endif
