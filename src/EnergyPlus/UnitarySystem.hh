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

#ifndef ENERGYPLUS_UNITARYSYSTEM_HH
#define ENERGYPLUS_UNITARYSYSTEM_HH

// C++ headers
#include <string>
#include <vector>

// EnergyPlus headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/PackagedThermalStorageCoil.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/SimAirServingZones.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace UnitarySystems {

    // Supply Air Sizing Option
    int constexpr None = 1;
    int constexpr SupplyAirFlowRate = 2;
    int constexpr FlowPerFloorArea = 3;
    int constexpr FractionOfAutoSizedCoolingValue = 4;
    int constexpr FractionOfAutoSizedHeatingValue = 5;
    int constexpr FlowPerCoolingCapacity = 6;
    int constexpr FlowPerHeatingCapacity = 7;

    // Last mode of operation
    int constexpr CoolingMode = 1; // last compressor operating mode was in cooling
    int constexpr HeatingMode = 2; // last compressor operating mode was in heating
    int constexpr NoCoolHeat = 3;  // last operating mode was no cooling or heating

    struct UnitarySysInputSpec
    {
        // system_type is not an object input but the actual type of object (e.g., UnitarySystem, CoilSystem:Cooling:DX, etc.).
        // Each specific getInput sets this string accordingly so that processInputSpec knows the object type
        // that will be used in warnings and reporting. This is a work in progress.
        std::string system_type;

        // object input fields
        std::string name;
        std::string control_type;
        std::string controlling_zone_or_thermostat_location;
        std::string dehumidification_control_type;
        std::string availability_schedule_name;
        std::string air_inlet_node_name;
        std::string air_outlet_node_name;
        std::string supply_fan_object_type;
        std::string supply_fan_name;
        std::string fan_placement;
        std::string supply_air_fan_operating_mode_schedule_name;
        std::string heating_coil_object_type;
        std::string heating_coil_name;
        Real64 dx_heating_coil_sizing_ratio = 1.0;
        std::string cooling_coil_object_type;
        std::string cooling_coil_name;
        std::string use_doas_dx_cooling_coil;
        Real64 minimum_supply_air_temperature = 2.0;
        std::string latent_load_control;
        std::string supplemental_heating_coil_object_type;
        std::string supplemental_heating_coil_name;
        std::string cooling_supply_air_flow_rate_method;
        Real64 cooling_supply_air_flow_rate = -999.0;
        Real64 cooling_supply_air_flow_rate_per_floor_area = -999.0;
        Real64 cooling_fraction_of_autosized_cooling_supply_air_flow_rate = -999.0;
        Real64 cooling_supply_air_flow_rate_per_unit_of_capacity = -999.0;
        std::string heating_supply_air_flow_rate_method;
        Real64 heating_supply_air_flow_rate = -999.0;
        Real64 heating_supply_air_flow_rate_per_floor_area = -999.0;
        Real64 heating_fraction_of_autosized_heating_supply_air_flow_rate = -999.0;
        Real64 heating_supply_air_flow_rate_per_unit_of_capacity = -999.0;
        std::string no_load_supply_air_flow_rate_method;
        Real64 no_load_supply_air_flow_rate = -999.0;
        std::string no_load_supply_air_flow_rate_low_speed;
        Real64 no_load_supply_air_flow_rate_per_floor_area = -999.0;
        Real64 no_load_fraction_of_autosized_cooling_supply_air_flow_rate = -999.0;
        Real64 no_load_fraction_of_autosized_heating_supply_air_flow_rate = -999.0;
        Real64 no_load_supply_air_flow_rate_per_unit_of_capacity_during_cooling_operation = -999.0;
        Real64 no_load_supply_air_flow_rate_per_unit_of_capacity_during_heating_operation = -999.0;
        Real64 maximum_supply_air_temperature = 80.0;
        Real64 maximum_supply_air_temperature_from_supplemental_heater = 50.0;
        Real64 maximum_outdoor_dry_bulb_temperature_for_supplemental_heater_operation = 21.0;
        std::string outdoor_dry_bulb_temperature_sensor_node_name;
        std::string heat_pump_coil_water_flow_mode;
        Real64 ancillary_on_cycle_electric_power = 0.0;
        Real64 ancillary_off_cycle_electric_power = 0.0;
        Real64 design_heat_recovery_water_flow_rate = 0.0;
        Real64 maximum_temperature_for_heat_recovery = 80.0;
        std::string heat_recovery_water_inlet_node_name;
        std::string heat_recovery_water_outlet_node_name;
        std::string design_specification_multispeed_object_type;
        std::string design_specification_multispeed_object_name;

        // inputs required by non-UnitarySystem parent objects
        std::string dx_cooling_coil_system_sensor_node_name;
        std::string oa_mixer_type;
        std::string oa_mixer_name;
        std::string avail_manager_list_name;
        std::string design_spec_zonehvac_sizing_object_name;
        Real64 cooling_oa_flow_rate = 0.0;
        Real64 heating_oa_flow_rate = 0.0;
        Real64 no_load_oa_flow_rate = 0.0;
        Real64 heat_conv_tol = 0.001;
        Real64 cool_conv_tol = 0.001;

        UnitarySysInputSpec() = default;
        ~UnitarySysInputSpec() = default;
    };

    struct DesignSpecMSHP
    {

    public:
        std::string name;
        static DesignSpecMSHP *factory(EnergyPlusData &state, HVAC::UnitarySysType type, std::string const &objectName);
        int numOfSpeedHeating = 0;
        int numOfSpeedCooling = 0;
        Real64 noLoadAirFlowRateRatio = 1.0;
        std::vector<Real64> coolingVolFlowRatio; // The ratio of flow to max for this speed
        std::vector<Real64> heatingVolFlowRatio; // The ratio of flow to max for this speed

        //    private:
        HVAC::UnitarySysType m_type = HVAC::UnitarySysType::Invalid;
        bool m_SingleModeFlag = false;

        static void getDesignSpecMSHP(EnergyPlusData &state);
        static void getDesignSpecMSHPdata([[maybe_unused]] EnergyPlusData &state, bool errorsFound);

        DesignSpecMSHP() = default;
        ~DesignSpecMSHP() = default;
    };

    struct UnitarySys : HVACSystemData
    {

        enum class UnitarySysCtrlType : int
        {
            Invalid = -1,
            None,
            Load,
            Setpoint,
            CCMASHRAE,
            Num
        };

        enum class DehumCtrlType : int
        {
            Invalid = -1,
            None,
            CoolReheat,
            Multimode,
            Num
        };

        // Airflow control for constant fan mode
        enum class UseCompFlow
        {
            Invalid = -1,
            On,  // set compressor OFF air flow rate equal to compressor ON air flow rate
            Off, // set compressor OFF air flow rate equal to user defined value
            Num
        };

        // Parent models simulated using UnitarySystem source code
        enum class SysType
        {
            Invalid = -1,
            Unitary,          // AirloopHVAC:UnitarySystem
            CoilCoolingDX,    // CoilSystem:Cooling:DX
            CoilCoolingWater, // CoilSystem:Cooling:Water
            PackagedAC,       // ZoneHVAC:PackagedTerminalAirConditioner
            PackagedHP,       // ZoneHVAC:PackagedTerminalHeatPump
            PackagedWSHP,     // ZoneHVAC:WaterToAirHeatPump
            Num,
        };

        UnitarySysInputSpec input_specs;
        int m_UnitarySysNum = -1;
        SysType m_sysType = SysType::Invalid;
        bool m_ThisSysInputShouldBeGotten = true;
        Sched::Schedule *m_sysAvailSched = nullptr; // availability schedule
        UnitarySysCtrlType m_ControlType = UnitarySysCtrlType::None;
        DehumCtrlType m_DehumidControlType_Num = DehumCtrlType::None;
        bool m_Humidistat = false;
        bool m_ValidASHRAECoolCoil = false;
        bool m_ValidASHRAEHeatCoil = false;
        bool m_SimASHRAEModel = false; // flag denoting that ASHRAE model (SZVAV) should be used
        bool m_setFaultModelInput = true;
        int m_FanIndex = 0;
        HVAC::FanPlace m_FanPlace = HVAC::FanPlace::Invalid;
        Sched::Schedule *m_fanOpModeSched = nullptr;
        bool m_FanExists = false;
        HVAC::FanType m_FanType = HVAC::FanType::Invalid;
        bool m_RequestAutoSize = false;
        Real64 m_ActualFanVolFlowRate = 0.0;
        Real64 m_DesignFanVolFlowRate = 0.0;
        Real64 m_DesignMassFlowRate = 0.0;
        Sched::Schedule *m_fanAvailSched = nullptr;
        HVAC::FanOp m_FanOpMode = HVAC::FanOp::Invalid;
        int m_ATMixerIndex = 0;
        int m_ATMixerPriNode = 0;
        int m_ATMixerSecNode = 0;
        bool m_AirLoopEquipment = true; // ?
        int m_ZoneInletNode = 0;
        int m_ZoneSequenceCoolingNum = 0;
        int m_ZoneSequenceHeatingNum = 0;
        bool m_HeatCoilExists = false;
        Real64 m_HeatingSizingRatio = 1.0;
        int m_HeatingCoilType_Num = 0;
        bool m_DXHeatingCoil = false;
        int m_HeatingCoilIndex = 0;
        Sched::Schedule *m_heatingCoilAvailSched = nullptr;
        Real64 m_DesignHeatingCapacity = 0.0;
        Real64 m_MaxHeatAirVolFlow = 0.0;
        int m_NumOfSpeedHeating = 0;
        int m_NumOfSpeedSuppHeating = 0;
        bool m_MultiSpeedHeatingCoil = false;
        bool m_VarSpeedHeatingCoil = false;
        int HeatCtrlNode = 0;
        bool m_CoolCoilExists = false;
        int m_CoolingCoilType_Num = 0;
        int m_NumOfSpeedCooling = 0;
        Sched::Schedule *m_coolingCoilAvailSched = nullptr;
        Real64 m_DesignCoolingCapacity = 0.0;
        Real64 m_MaxCoolAirVolFlow = 0.0;
        int m_CondenserNodeNum = 0;
        DataHeatBalance::RefrigCondenserType m_CondenserType = DataHeatBalance::RefrigCondenserType::Invalid;
        int m_CoolingCoilIndex = 0;
        bool m_HeatPump = false;
        int m_ActualDXCoilIndexForHXAssisted = 0;
        bool m_DiscreteSpeedCoolingCoil = false;
        bool m_ContSpeedCoolingCoil = false;
        int CoolCtrlNode = 0;
        HVAC::WaterFlow m_WaterCyclingMode = HVAC::WaterFlow::Invalid;
        bool m_ISHundredPercentDOASDXCoil = false;
        bool m_RunOnSensibleLoad = false;
        bool m_RunOnLatentLoad = false;
        bool m_RunOnLatentOnlyWithSensible = false;
        HVAC::CoilMode m_DehumidificationMode = HVAC::CoilMode::Normal; // Only explicitly initialized if something other than Normal
        int m_SuppHeatCoilType_Num = 0;
        bool m_SuppCoilExists = false;
        Real64 m_DesignSuppHeatingCapacity = 0.0;
        int m_SuppCoilAirInletNode = 0;
        int SuppCoilOutletNodeNum = 0;
        int m_SuppCoilFluidInletNode = 0;
        Real64 m_MaxSuppCoilFluidFlow = 0.0;
        int m_SuppHeatCoilIndex = 0;
        int SuppCtrlNode = 0;
        Real64 m_SupHeaterLoad = 0.0;
        int m_CoolingSAFMethod = 0;
        int m_HeatingSAFMethod = 0;
        int m_NoCoolHeatSAFMethod = 0;
        int m_CoolingCapMethod = 0;
        int m_HeatingCapMethod = 0;
        Real64 m_MaxNoCoolHeatAirVolFlow = 0.0;
        UseCompFlow m_AirFlowControl = UseCompFlow::Invalid;
        bool m_CoolingCoilUpstream = true;
        Real64 m_MaxOATSuppHeat = 0.0;
        Real64 m_MinOATCompressorCooling = 0.0;
        Real64 m_MinOATCompressorHeating = 0.0;
        Real64 m_AncillaryOnPower = 0.0;
        Real64 m_AncillaryOffPower = 0.0;
        Real64 m_DesignHRWaterVolumeFlow = 0.0;
        Real64 m_MaxHROutletWaterTemp = 0.0;
        bool m_HeatRecActive = false;
        int m_HeatRecoveryInletNodeNum = 0;
        int m_HeatRecoveryOutletNodeNum = 0;
        int m_DesignSpecMSHPIndex = -1;
        Real64 m_NoLoadAirFlowRateRatio = 1.0;
        bool m_useNoLoadLowSpeedAirFlow = true;
        int m_SingleMode = 0;
        bool m_MultiOrVarSpeedHeatCoil = false;
        bool m_MultiOrVarSpeedCoolCoil = false;
        Real64 m_PartLoadFrac = 0.0;
        Real64 m_CoolingPartLoadFrac = 0.0;
        Real64 m_HeatingPartLoadFrac = 0.0;
        Real64 m_SuppHeatPartLoadFrac = 0.0;
        Real64 m_HeatCompPartLoadRatio = 0.0;
        Real64 m_CoolCompPartLoadRatio = 0.0;
        Real64 m_SpeedRatio = 0.0;
        Real64 m_CycRatio = 0.0;

        bool m_MyEnvrnFlag = true;
        bool m_MyEnvrnFlag2 = true;
        bool m_MyPlantScanFlag = true;
        bool m_MySuppCoilPlantScanFlag = true;
        bool m_MySetPointCheckFlag = true;
        bool m_MySizingCheckFlag = true;
        bool m_InitHeatPump = false; // Heat pump initialization flag (for error reporting)
        PlantLocation m_HRPlantLoc;
        PlantLocation m_SuppCoilPlantLoc;
        int m_SuppCoilFluidOutletNodeNum = 0;

        Real64 m_CompPartLoadRatio = 0.0;
        Real64 m_CoolingCoilSensDemand = 0.0;
        Real64 m_CoolingCoilLatentDemand = 0.0;
        Real64 m_HeatingCoilSensDemand = 0.0;
        Real64 m_SenLoadLoss = 0.0;
        Real64 m_LatLoadLoss = 0.0;
        Real64 m_DesignHeatRecMassFlowRate = 0.0;
        Real64 m_HeatRecoveryMassFlowRate = 0.0;
        Real64 m_HeatRecoveryRate = 0.0;
        Real64 m_HeatRecoveryEnergy = 0.0;
        Real64 m_HeatRecoveryInletTemp = 0.0;
        Real64 m_HeatRecoveryOutletTemp = 0.0;

        int m_IterationCounter = 0;

        Real64 m_DesiredOutletTemp = 0.0;
        Real64 m_DesiredOutletHumRat = 0.0;
        int m_FrostControlStatus = 0;

        Real64 m_CoolingCycRatio = 0.0;
        Real64 m_CoolingSpeedRatio = 0.0;
        int m_CoolingSpeedNum = 0;

        int m_EconoSpeedNum = 0;
        Real64 m_EconoPartLoadRatio = 0;
        Real64 m_LowSpeedEconOutput = 0;
        Real64 m_LowSpeedEconRuntime = 0;

        Real64 m_HeatingCycRatio = 0.0;
        Real64 m_HeatingSpeedRatio = 0.0;
        int m_HeatingSpeedNum = 0;
        int m_SpeedNum = 0;
        Real64 m_SuppHeatingCycRatio = 0.0;
        Real64 m_SuppHeatingSpeedRatio = 0.0;
        int m_SuppHeatingSpeedNum = 0;

        bool m_EMSOverrideCoilSpeedNumOn = false;
        Real64 m_EMSOverrideCoilSpeedNumValue = 0.0;
        bool m_EMSOverrideSuppCoilSpeedNumOn = false;
        Real64 m_EMSOverrideSuppCoilSpeedNumValue = 0.0;
        int m_CoilSpeedErrIdx = 0;

        Real64 m_DehumidInducedHeatingDemandRate = 0.0;

        Real64 m_TotalAuxElecPower = 0.0;
        Real64 m_HeatingAuxElecConsumption = 0.0;
        Real64 m_CoolingAuxElecConsumption = 0.0;
        Real64 m_ElecPower = 0.0;
        Real64 m_ElecPowerConsumption = 0.0;

        int m_LastMode = 0;
        bool m_FirstPass = true;

        Real64 m_TotCoolEnergyRate = 0.0;
        Real64 m_TotCoolEnergy = 0.0;
        Real64 m_SensCoolEnergyRate = 0.0;
        Real64 m_SensCoolEnergy = 0.0;
        Real64 m_LatCoolEnergyRate = 0.0;
        Real64 m_LatCoolEnergy = 0.0;
        Real64 m_TotHeatEnergyRate = 0.0;
        Real64 m_TotHeatEnergy = 0.0;
        Real64 m_SensHeatEnergyRate = 0.0;
        Real64 m_SensHeatEnergy = 0.0;
        Real64 m_LatHeatEnergyRate = 0.0;
        Real64 m_LatHeatEnergy = 0.0;

        bool m_DesignFanVolFlowRateEMSOverrideOn = false;       // If true, then EMS is calling to override autosize fan flow
        bool m_MaxHeatAirVolFlowEMSOverrideOn = false;          // If true, then EMS is calling to override autosize fan flow
        bool m_MaxCoolAirVolFlowEMSOverrideOn = false;          // If true, then EMS is calling to override autosize fan flow
        bool m_MaxNoCoolHeatAirVolFlowEMSOverrideOn = false;    // If true, then EMS is calling to override autosize fan flow
        Real64 m_DesignFanVolFlowRateEMSOverrideValue = 0.0;    // EMS value for override of fan flow rate autosize [m3/s]
        Real64 m_MaxHeatAirVolFlowEMSOverrideValue = 0.0;       // EMS value for override of fan flow rate autosize [m3/s]
        Real64 m_MaxCoolAirVolFlowEMSOverrideValue = 0.0;       // EMS value for override of fan flow rate autosize [m3/s]
        Real64 m_MaxNoCoolHeatAirVolFlowEMSOverrideValue = 0.0; // EMS value for override of fan flow rate autosize [m3/s]
        bool m_EMSOverrideSensZoneLoadRequest = false;          // If true, then EMS is calling to override zone load
        bool m_EMSOverrideMoistZoneLoadRequest = false;         // If true, then EMS is calling to override zone load
        Real64 m_EMSSensibleZoneLoadValue = 0.0;                // Value EMS is directing to use
        Real64 m_EMSMoistureZoneLoadValue = 0.0;                // Value EMS is directing to use
        // Staged thermostat control
        int m_StageNum = 0;    // Stage number specified by staged thermostat
        bool m_Staged = false; // Using Staged thermostat

        Real64 m_HeatingFanSpeedRatio = 0.0;
        Real64 m_CoolingFanSpeedRatio = 0.0;
        Real64 m_NoHeatCoolSpeedRatio = 0.0;
        bool m_MyFanFlag = true;
        bool m_MyCheckFlag = true;
        Real64 m_SensibleLoadMet = 0.0;
        Real64 m_LatentLoadMet = 0.0;
        bool m_MyStagedFlag = false;
        Real64 m_SensibleLoadPredicted = 0.0;
        Real64 m_MoistureLoadPredicted = 0.0;

        // Fault model of coil SAT sensor
        bool m_FaultyCoilSATFlag = false;   // True if the coil has SAT sensor fault
        int m_FaultyCoilSATIndex = 0;       // Index of the fault object corresponding to the coil
        Real64 m_FaultyCoilSATOffset = 0.0; // Coil SAT sensor offset

        PackagedThermalStorageCoil::PTSCOperatingMode m_TESOpMode = PackagedThermalStorageCoil::PTSCOperatingMode::Invalid;
        bool m_initLoadBasedControlAirLoopPass = false;
        int m_airLoopPassCounter = 0;
        int m_airLoopReturnCounter = 0;
        bool m_FanCompNotSetYet = true;
        bool m_CoolCompNotSetYet = true;
        bool m_HeatCompNotSetYet = true;
        bool m_SuppCompNotSetYet = true;
        bool m_OKToPrintSizing = false;
        bool m_IsDXCoil = true;
        Real64 m_SmallLoadTolerance = 5.0;             // watts
        bool m_TemperatureOffsetControlActive = false; // true if water-side economizer coil is active
        Real64 m_minAirToWaterTempOffset = 0.0;        // coil entering air to entering water temp offset

        int m_HRcoolCoilFluidInletNode = 0;
        int m_HRcoolCoilAirInNode = 0;
        Real64 m_minWaterLoopTempForHR = 0.0;   // water coil heat recovery loops
        bool m_waterSideEconomizerFlag = false; // user input to enable lockout with economizer
        bool m_WaterHRPlantLoopModel = false;   // signifies water heat recovery loop for this CoilSystem
        std::array<int, 4> m_OAMixerNodes{0, 0, 0, 0};
        Real64 m_CoolOutAirVolFlow = 0.0;
        Real64 m_CoolOutAirMassFlow = 0.0;
        Real64 m_HeatOutAirVolFlow = 0.0;
        Real64 m_HeatOutAirMassFlow = 0.0;
        Real64 m_NoCoolHeatOutAirVolFlow = 0.0;
        Real64 m_NoCoolHeatOutAirMassFlow = 0.0;
        Real64 m_HeatConvTol = 0.001;
        Real64 m_CoolConvTol = 0.001;
        int m_HVACSizingIndex = -1;
        Avail::Status m_AvailStatus = Avail::Status::NoAction;
        bool m_IsZoneEquipment = false;
        bool m_ZoneCompFlag = true;
        std::string m_AvailManagerListName;
        int m_EquipCompNum = 0; // 1-based index of this parent type for specific equipment type processing

    public:
        // SZVAV variables
        DataZoneEquipment::ZoneEquipType zoneEquipType = DataZoneEquipment::ZoneEquipType::Invalid;
        SimAirServingZones::CompType AirloopEqType = SimAirServingZones::CompType::Invalid;
        int MaxIterIndex = 0;
        int RegulaFalsiFailedIndex = 0;
        int NodeNumOfControlledZone = 0;
        Real64 FanPartLoadRatio = 0.0;
        Real64 CoolCoilWaterFlowRatio = 0.0;
        Real64 HeatCoilWaterFlowRatio = 0.0;
        int ControlZoneNum = 0;                                 // index of unit in ZoneEquipConfig
        int AirInNode = 0;                                      // Parent inlet air node number
        int AirOutNode = 0;                                     // Parent outlet air node number
        Real64 MaxCoolAirMassFlow = 0.0;                        // Maximum coil air mass flow for cooling [kg/s]
        Real64 MaxHeatAirMassFlow = 0.0;                        // Maximum coil air mass flow for heating [kg/s]
        Real64 MaxNoCoolHeatAirMassFlow = 0.0;                  // Maximum coil air mass flow for no cooling or heating [kg/s]
        Real64 DesignMinOutletTemp = 0.0;                       // DOAS DX Cooling or SZVAV coil outlet air minimum temperature [C]
        Real64 DesignMaxOutletTemp = 0.0;                       // Maximum supply air temperature from heating coil [C]
        Real64 LowSpeedCoolFanRatio = 0.0;                      // cooling mode ratio of low speed fan flow to full flow rate
        Real64 LowSpeedHeatFanRatio = 0.0;                      // heating mode ratio of low speed fan flow to full flow rate
        Real64 MaxCoolCoilFluidFlow = 0.0;                      // Maximum cooling coil fluid flow for chilled water coil
        Real64 MaxHeatCoilFluidFlow = 0.0;                      // Maximum heating coil fluid flow for hot water or steam coil
        int CoolCoilInletNodeNum = 0;                           // Cooling coil air inlet node number
        int CoolCoilOutletNodeNum = 0;                          // Cooling coil air outlet node number
        int CoolCoilFluidOutletNodeNum = 0;                     // Cooling coil fluid outlet node number (from Plant Loop data)
        PlantLocation CoolCoilPlantLoc;                         // Location of the cooling coil in the plant loop
        int CoolCoilFluidInletNode = 0;                         // Cooling coil fluid inlet node
        PlantLocation HeatCoilPlantLoc;                         // Location of the heating coil in the plant loop
        int HeatCoilFluidInletNode = 0;                         // Heating coil fluid inlet node
        int HeatCoilFluidOutletNodeNum = 0;                     // Heating coil fluid outlet node number (from Plant Loop data)
        int HeatCoilInletNodeNum = 0;                           // Heating coil air inlet node number
        int HeatCoilOutletNodeNum = 0;                          // Heating coil air outlet node number
        bool ATMixerExists = false;                             // true if AT mixer is connected to Unitary System
        HVAC::MixerType ATMixerType = HVAC::MixerType::Invalid; // type of AT mixer, inlet-side or supply-side
        int ATMixerOutNode = 0;                                 // AT mixer outlet node number
        Real64 ControlZoneMassFlowFrac = 0.0;                   // fraction of air flow to the control zone
        DesignSpecMSHP *m_CompPointerMSHP = nullptr;
        std::string Name;
        std::string UnitType;
        Real64 LoadSHR = 0.0;                   // Load sensible heat ratio with humidity control
        Real64 CoilSHR = 0.0;                   // Load sensible heat ratio with humidity control
        int temperatureOffsetControlStatus = 0; // water side economizer status flag, also report variable
        int OAMixerIndex = -1;                  // index to zone equipment OA mixer
        int OASysIndex = -1;                    // index to OA system
        int OAControllerIndex = -1;             // index to OA controller
        HVAC::EconomizerStagingType OAControllerEconomizerStagingType =
            HVAC::EconomizerStagingType::InterlockedWithMechanicalCooling; // economizer staging operation type
        bool OAMixerExists = false;                                        // true if OA mixer is connected to inlet of UnitarySystem
        bool reportACCAManualS = false;

        //    private:
        // private members not initialized in constructor
        std::string m_FanName;
        std::string m_ATMixerName;
        std::string m_HeatingCoilName;
        std::string m_HeatingCoilTypeName;
        std::string m_CoolingCoilName;
        std::string m_SuppHeatCoilName;
        std::string m_SuppHeatCoilTypeName;
        std::string m_DesignSpecMultispeedHPType;
        std::string m_DesignSpecMultispeedHPName;
        std::vector<Real64> m_CoolVolumeFlowRate;
        std::vector<Real64> m_CoolMassFlowRate;
        std::vector<Real64> m_MSCoolingSpeedRatio;
        std::vector<Real64> m_HeatVolumeFlowRate;
        std::vector<Real64> m_HeatMassFlowRate;
        std::vector<Real64> m_MSHeatingSpeedRatio;
        std::vector<Real64> m_HeatingVolFlowRatio;
        std::vector<int> m_IterationMode;  // array of operating mode each iteration
        std::vector<Real64> FullOutput;    // Full output for different speed
        std::vector<Real64> FullLatOutput; // Full latent output for different speed
        std::vector<Real64> SpeedSHR;      // SHR at different speed

        struct WarnMessages
        {
            // Warning message variables
            int m_HXAssistedSensPLRIter = 0;        // used in HX Assisted calculations
            int m_HXAssistedSensPLRIterIndex = 0;   // used in HX Assisted calculations
            int m_HXAssistedSensPLRFail = 0;        // used in HX Assisted calculations
            int m_HXAssistedSensPLRFailIndex = 0;   // used in HX Assisted calculations
            int m_HXAssistedSensPLRFail2 = 0;       // used in HX Assisted calculations
            int m_HXAssistedSensPLRFailIndex2 = 0;  // used in HX Assisted calculations
            int m_HXAssistedLatPLRIter = 0;         // used in HX Assisted calculations
            int m_HXAssistedLatPLRIterIndex = 0;    // used in HX Assisted calculations
            int m_HXAssistedLatPLRFail = 0;         // used in HX Assisted calculations
            int m_HXAssistedLatPLRFailIndex = 0;    // used in HX Assisted calculations
            int m_HXAssistedCRLatPLRIter = 0;       // used in HX Assisted calculations
            int m_HXAssistedCRLatPLRIterIndex = 0;  // used in HX Assisted calculations
            int m_HXAssistedCRLatPLRFail = 0;       // used in HX Assisted calculations
            int m_HXAssistedCRLatPLRFailIndex = 0;  // used in HX Assisted calculations
            int m_HXAssistedCRLatPLRFail2 = 0;      // used in HX Assisted calculations
            int m_HXAssistedCRLatPLRFailIndex2 = 0; // used in HX Assisted calculations
            int m_SensPLRIter = 0;                  // used in cool coil calculations
            int m_SensPLRIterIndex = 0;             // used in cool coil calculations
            int m_SensPLRFail = 0;                  // used in cool coil calculations
            int m_SensPLRFailIndex = 0;             // used in cool coil calculations
            int m_LatPLRIter = 0;                   // used in cool coil calculations
            int m_LatPLRIterIndex = 0;              // used in cool coil calculations
            int m_LatPLRFail = 0;                   // used in cool coil calculations
            int m_LatPLRFailIndex = 0;              // used in cool coil calculations
            int m_HeatCoilSensPLRIter = 0;          // used in heat coil calculations
            int m_HeatCoilSensPLRIterIndex = 0;     // used in heat coil calculations
            int m_HeatCoilSensPLRFail = 0;          // used in heat coil calculations
            int m_HeatCoilSensPLRFailIndex = 0;     // used in heat coil calculations
            int m_SuppHeatCoilSensPLRIter = 0;      // used in supp heat coil calculations
            int m_SuppHeatCoilSensPLRIterIndex = 0; // used in supp heat coil calculations
            int m_SuppHeatCoilSensPLRFail = 0;      // used in supp heat coil calculations
            int m_SuppHeatCoilSensPLRFailIndex = 0; // used in supp heat coil calculations
            int m_LatMaxIterIndex = 0;              // used in PLR calculations for moisture load
            int m_LatRegulaFalsiFailedIndex = 0;    // used in PLR calculations for moisture load
        };
        WarnMessages warnIndex;

        static void getUnitarySystemInput(EnergyPlusData &state, std::string_view Name, bool const ZoneEquipment, int const ZoneOAUnitNum);

        void processInputSpec(EnergyPlusData &state,
                              const UnitarySysInputSpec &input_data,
                              int sysNum,
                              bool &errorsFound,
                              bool const ZoneEquipment,
                              int const ZoneOAUnitNum);

        void setSystemParams(EnergyPlusData &state, Real64 &TotalFloorAreaOnAirLoop, const std::string &thisObjectName);

        static Real64 calcUnitarySystemLoadResidual(EnergyPlusData &state,
                                                    Real64 const PartLoadRatio, // DX cooling coil part load ratio
                                                    int UnitarySysNum,
                                                    bool FirstHVACIteration,
                                                    // par 3 not used?
                                                    HVAC::CompressorOp compressorOp,
                                                    Real64 LoadToBeMet,
                                                    Real64 coolHeatFlag, // make bool?
                                                    Real64 SensibleLoad,
                                                    Real64 OnOffAirFlowRatio,
                                                    bool HXUnitOn,
                                                    // par 10 not used
                                                    int AirLoopNum);

        static Real64 DXCoilVarSpeedResidual(EnergyPlusData &state,
                                             Real64 SpeedRatio, // compressor speed ratio (1.0 is max, 0.0 is min)
                                             int CoilIndex,
                                             Real64 DesOutTemp,
                                             int UnitarySysNum,
                                             Real64 CycRatio,
                                             int SpeedNum,
                                             HVAC::FanOp const fanOp,
                                             HVAC::CompressorOp CompressorOp);

        static Real64 heatingCoilVarSpeedResidual(EnergyPlusData &state,
                                                  Real64 SpeedRatio, // compressor speed ratio (1.0 is max, 0.0 is min)
                                                  int CoilIndex,
                                                  Real64 DesOutTemp,
                                                  int UnitarySysNum,
                                                  Real64 CycRatio,
                                                  int SpeedNum,
                                                  HVAC::FanOp const fanOp,
                                                  HVAC::CompressorOp compressorOp,
                                                  bool SuppHeat);

        static Real64 DXCoilVarSpeedHumRatResidual(EnergyPlusData &state,
                                                   Real64 SpeedRatio, // compressor speed ratio (1.0 is max, 0.0 is min)
                                                   int CoilIndex,
                                                   Real64 DesOutHumRat,
                                                   int UnitarySysNum,
                                                   Real64 CycRatio,
                                                   int SpeedNum,
                                                   HVAC::FanOp const fanOp,
                                                   HVAC::CompressorOp CompressorOp);

        static Real64 DXCoilCyclingResidual(EnergyPlusData &state,
                                            Real64 CycRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                            int CoilIndex,
                                            Real64 DesOutTemp,
                                            int UnitarySysNum,
                                            Real64 SpeedRatio,
                                            int SpeedNum,
                                            HVAC::FanOp const fanOp,
                                            HVAC::CompressorOp compressorOp,
                                            int AirloopNum,
                                            bool FirstHVACIteration);

        static Real64 DXCoilCyclingHumRatResidual(EnergyPlusData &state,
                                                  Real64 CycRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                                  int CoilIndex,
                                                  Real64 DesOutHumRat,
                                                  int UnitarySysNum,
                                                  Real64 SpeedRatio,
                                                  int SpeedNum,
                                                  HVAC::FanOp const fanOp,
                                                  HVAC::CompressorOp CompressorOp);

        static Real64 heatingCoilVarSpeedCycResidual(EnergyPlusData &state,
                                                     Real64 CycRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                                     int CoilIndex,
                                                     Real64 DesOutTemp,
                                                     int UnitarySysNum,
                                                     Real64 SpeedRatio,
                                                     int SpeedNum,
                                                     HVAC::FanOp const fanOp,
                                                     HVAC::CompressorOp compressorOp,
                                                     bool SuppHeat);

        static Real64 gasElecHeatingCoilResidual(EnergyPlusData &state,
                                                 Real64 PartLoadFrac, // Compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                                 int UnitarySysNum,
                                                 bool FirstHVACIteration,
                                                 Real64 desTemp,
                                                 bool SuppHeatingCoilFlag,
                                                 HVAC::FanOp fanOp,
                                                 Real64 HeatingLoadArg);

        static Real64 coolWatertoAirHPTempResidual(EnergyPlusData &state,
                                                   Real64 PartLoadRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                                   int UnitarySysNum,
                                                   bool FirstHVACIteration,
                                                   Real64 DesOutTemp,
                                                   Real64 ReqOutput);

        void initUnitarySystems(EnergyPlusData &state, int AirLoopNum, bool FirstHVACIteration, Real64 const OAUCoilOutTemp);

        bool checkNodeSetPoint(EnergyPlusData &state,
                               int const AirLoopNum,       // number of the current air loop being simulated
                               int const ControlNode,      // Node to test for set point
                               int const CoilType,         // True if cooling coil, then test for HumRatMax set point
                               Real64 const OAUCoilOutTemp // the coil inlet temperature of OutdoorAirUnit
        );

        void frostControlSetPointLimit(EnergyPlusData &state,
                                       Real64 &TempSetPoint,       // temperature setpoint of the sensor node
                                       Real64 &HumRatSetPoint,     // humidity ratio setpoint of the sensor node
                                       Real64 const BaroPress,     // baromtric pressure, Pa [N/m^2]
                                       Real64 const TfrostControl, // minimum temperature limit for forst control
                                       int const ControlMode       // temperature or humidity control mode
        );

        void reportUnitarySystem(EnergyPlusData &state, int const AirLoopNum);

        void unitarySystemHeatRecovery(EnergyPlusData &state);

        void controlUnitarySystemtoSP(EnergyPlusData &state,
                                      int const AirLoopNum,             // Primary air loop number
                                      bool const FirstHVACIteration,    // True when first HVAC iteration
                                      HVAC::CompressorOp &CompressorOn, // compressor on/off control
                                      Real64 const OAUCoilOutTemp,      // the coil inlet temperature of OutdoorAirUnit
                                      bool HXUnitOn,                    // Flag to control HX for HXAssisted Cooling Coil
                                      Real64 &sysOutputProvided,        // sensible output at supply air node
                                      Real64 &latOutputProvided         // latent output at supply air node
        );

        void controlUnitarySystemtoLoad(EnergyPlusData &state,
                                        int const AirLoopNum,             // Primary air loop number
                                        bool const FirstHVACIteration,    // True when first HVAC iteration
                                        HVAC::CompressorOp &CompressorOn, // Determines if compressor is on or off
                                        Real64 const OAUCoilOutTemp,      // the coil inlet temperature of OutdoorAirUnit
                                        bool HXUnitOn,                    // Flag to control HX for HXAssisted Cooling Coil
                                        Real64 &sysOutputProvied,         // system sensible output at supply air node
                                        Real64 &latOutputProvided         // system latent output at supply air node
        );

        void updateUnitarySystemControl(EnergyPlusData &state,
                                        int const AirLoopNum,  // number of the current air loop being simulated
                                        int const OutNode,     // coil outlet node number
                                        int const ControlNode, // control node number
                                        Real64 &OnOffAirFlowRatio,
                                        bool const FirstHVACIteration,
                                        Real64 const OAUCoilOutletTemp, // "ONLY" for zoneHVAC:OutdoorAirUnit
                                        Real64 &ZoneLoad,
                                        Real64 const MaxOutletTemp // limits heating coil outlet temp [C]
        );
        void controlUnitarySystemOutputEMS(EnergyPlusData &state,
                                           int const AirLoopNum,          // Index to air loop
                                           bool const FirstHVACIteration, // True when first HVAC iteration
                                           Real64 &OnOffAirFlowRatio,     // ratio of heating PLR to cooling PLR (is this correct?)
                                           Real64 const ZoneLoad,
                                           Real64 &FullSensibleOutput,
                                           bool &HXUnitOn, // Flag to control HX for HXAssisted Cooling Coil
                                           HVAC::CompressorOp CompressorOn);

        void controlUnitarySystemOutput(EnergyPlusData &state,
                                        int const AirLoopNum,          // Index to air loop
                                        bool const FirstHVACIteration, // True when first HVAC iteration
                                        Real64 &OnOffAirFlowRatio,     // ratio of heating PLR to cooling PLR (is this correct?)
                                        Real64 const ZoneLoad,
                                        Real64 &FullSensibleOutput,
                                        bool &HXUnitOn, // Flag to control HX for HXAssisted Cooling Coil
                                        HVAC::CompressorOp CompressorOn);

        void initLoadBasedControl(EnergyPlusData &state,
                                  int const AirLoopNum, // number of the current air loop being simulated
                                  bool const FirstHVACIteration,
                                  Real64 &OnOffAirFlowRatio,
                                  Real64 &ZoneLoad);

        void setOnOffMassFlowRate(EnergyPlusData &state,
                                  Real64 &OnOffAirFlowRatio, // ratio of coil on to coil off air flow rate
                                  Real64 const PartLoadRatio // coil part-load ratio
        );

        void setAverageAirFlow(EnergyPlusData &state,
                               Real64 const PartLoadRatio, // unit part load ratio
                               Real64 &OnOffAirFlowRatio   // ratio of compressor ON airflow to AVERAGE airflow over timestep
        );

        void calcMultiStageSuppCoilStageByLoad(EnergyPlusData &state, Real64 const SuppHeatload, bool const FirstHVACIteration);

        void calculateCapacity(EnergyPlusData &state,
                               Real64 &SensOutput, // sensible output of AirloopHVAC:UnitarySystem
                               Real64 &LatOutput   // latent output of AirloopHVAC:UnitarySystem
        );

        void calcUnitaryCoolingSystem(EnergyPlusData &state,
                                      int const AirLoopNum,            // index to air loop
                                      bool const FirstHVACIteration,   // True when first HVAC iteration
                                      Real64 const PartLoadRatio,      // coil operating part-load ratio
                                      HVAC::CompressorOp CompressorOn, // compressor control (0=off, 1=on)
                                      Real64 const OnOffAirFlowRatio,
                                      Real64 const CoilCoolHeatRat, // ratio of cooling to heating PLR for cycling fan RH control
                                      bool const HXUnitOn           // Flag to control HX for HXAssisted Cooling Coil
        );

        void calcUnitaryHeatingSystem(EnergyPlusData &state,
                                      int const AirLoopNum,            // index to air loop
                                      bool const FirstHVACIteration,   // True when first HVAC iteration
                                      Real64 const PartLoadRatio,      // coil operating part-load ratio
                                      HVAC::CompressorOp CompressorOn, // comrpressor control (0=off, 1=on)
                                      Real64 const OnOffAirFlowRatio,  // ratio of on to off flow rate
                                      Real64 HeatCoilLoad              // adjusted heating coil load if outlet temp exceeds max (W)
        );

        void calcUnitarySuppHeatingSystem(EnergyPlusData &state,
                                          bool const FirstHVACIteration, // True when first HVAC iteration
                                          Real64 const SuppCoilLoad      // adjusted supp coil load when outlet temp exceeds max (W)
        );

        void setEMSSuppCoilStagePLR(EnergyPlusData &state);

        void calcUnitarySuppSystemToSP(EnergyPlusData &state, bool const FirstHVACIteration // True when first HVAC iteration
        );

        void controlCoolingSystemToSP(EnergyPlusData &PartLoadFrac,
                                      int const AirLoopNum,            // index to air loop
                                      bool const FirstHVACIteration,   // First HVAC iteration flag
                                      bool &HXUnitOn,                  // flag to enable heat exchanger heat recovery
                                      HVAC::CompressorOp &CompressorOp // compressor on/off control
        );

        void controlHeatingSystemToSP(EnergyPlusData &maxPartLoadFrac,
                                      int const AirLoopNum,             // index to air loop
                                      bool const FirstHVACIteration,    // First HVAC iteration flag
                                      HVAC::CompressorOp &compressorOp, // compressor on/off control
                                      Real64 &HeatCoilLoad              // load met by heating coil
        );

        void controlSuppHeatSystemToSP(EnergyPlusData &state,
                                       int const AirLoopNum,         // index to air loop
                                       bool const FirstHVACIteration // First HVAC iteration flag
        );

        void simMultiSpeedCoils(EnergyPlusData &state,
                                int const AirLoopNum,             // Index to air loop
                                bool const FirstHVACIteration,    // True when first HVAC iteration
                                HVAC::CompressorOp &CompressorOn, // compressor on/off control
                                bool const SensibleLoad,
                                bool const LatentLoad,
                                Real64 const PartLoadFrac,
                                int const CoilType,
                                int const SpeedNumber);

        void calcPassiveSystem(EnergyPlusData &state,
                               int const AirLoopNum,         // Index to air loop
                               bool const FirstHVACIteration // True when first HVAC iteration
        );

        void setSpeedVariables(EnergyPlusData &state,
                               bool const SensibleLoad,   // True when meeting a sensible load (not a moisture load)
                               Real64 const PartLoadRatio // operating PLR
        );

    public:
        static void
        getUnitarySystemInputData(EnergyPlusData &state, std::string_view Name, bool const ZoneEquipment, int const ZoneOAUnitNum, bool &errorsFound);

        static void
        getDXCoilSystemData(EnergyPlusData &state, std::string_view Name, bool const ZoneEquipment, int const ZoneOAUnitNum, bool &errorsFound);

        static void getCoilWaterSystemInputData(
            EnergyPlusData &state, std::string_view CoilSysName, bool const ZoneEquipment, int const ZoneOAUnitNum, bool &errorsFound);

        static void getPackagedTerminalUnitData(
            EnergyPlusData &state, std::string_view Name, bool const ZoneEquipment, int const ZoneOAUnitNum, bool &errorsFound);

        static void allocateUnitarySys(EnergyPlusData &state);

        static HVACSystemData *factory(
            EnergyPlusData &state, HVAC::UnitarySysType const type, std::string const &objectName, bool const ZoneEquipment, int const ZoneOAUnitNum);

        void calcUnitarySystemToLoad(EnergyPlusData &state,
                                     int const AirLoopNum,                 // index to air loop
                                     bool const FirstHVACIteration,        // True when first HVAC iteration
                                     Real64 const CoolPLR,                 // operating cooling part-load ratio []
                                     Real64 const HeatPLR,                 // operating cooling part-load ratio []
                                     Real64 &OnOffAirFlowRatio,            // ratio of heating PLR to cooling PLR (is this correct?)
                                     Real64 &SensOutput,                   // sensible capacity (W)
                                     Real64 &LatOutput,                    // latent capacity (W)
                                     bool HXUnitOn,                        // Flag to control HX for HXAssisted Cooling Coil
                                     Real64 HeatCoilLoad,                  // Adjusted load to heating coil when SAT exceeds max limit (W)
                                     Real64 SuppCoilLoad,                  // Adjusted load to supp heating coil when SAT exceeds max limit (W)
                                     HVAC::CompressorOp const CompressorOn // Determines if compressor is on or off
        );

        static void checkUnitarySysCoilInOASysExists(EnergyPlusData &state, std::string_view UnitarySysName, int const ZoneOAUnitNum);

        static void getUnitarySysHeatCoolCoil(EnergyPlusData &state,
                                              std::string_view UnitarySysName, // Name of Unitary System object
                                              bool &CoolingCoil,               // Cooling coil exists
                                              bool &HeatingCoil,               // Heating coil exists
                                              int const ZoneOAUnitNum          // index to zone OA unit
        );

        static Real64 calcUnitarySystemWaterFlowResidual(EnergyPlusData &state,
                                                         Real64 const PartLoadRatio, // coil part load ratio
                                                         int UnitarySysNum,
                                                         bool FirstHVACIteration,
                                                         Real64 QZnReq,
                                                         int AirControlNode,
                                                         Real64 OnOffAirFlowRat,
                                                         int AirLoopNum,
                                                         int WaterControlNode,
                                                         Real64 highWaterMdot,
                                                         Real64 lowSpeedRatio,
                                                         Real64 airMdot,
                                                         Real64 par13_SATempTarget,
                                                         Real64 systemMaxAirFlowRate,
                                                         Real64 par15_LoadType,
                                                         Real64 par16_IterationMethod);

        void simulate(EnergyPlusData &state,
                      std::string_view Name,
                      bool const firstHVACIteration,
                      int AirLoopNum,
                      int &CompIndex,
                      bool &HeatActive,
                      bool &CoolActive,
                      int const OAUnitNum,         // If the system is an equipment of OutdoorAirUnit
                      Real64 const OAUCoilOutTemp, // the coil inlet temperature of OutdoorAirUnit
                      bool const ZoneEquipment,    // TRUE if called as zone equipment
                      Real64 &sysOutputProvided,   // sensible output at supply air node
                      Real64 &latOutputProvided    // latent output at supply air node
                      ) override;

        Real64 getFanDeltaTemp(EnergyPlusData &state, bool const firstHVACIteration, Real64 const massFlowRate, Real64 const airFlowRatio);

        void setEconomizerStagingOperationSpeed(EnergyPlusData &state, bool const firstHVACIteration, Real64 const zoneLoad);

        void calcMixedTempAirSPforEconomizerStagingOperation(EnergyPlusData &state,
                                                             int const airLoopNum,
                                                             bool const firstHVACIteration,
                                                             Real64 const zoneLoad);

        void manageEconomizerStagingOperation(EnergyPlusData &state, int const airLoopNum, bool const firstHVACIteration, Real64 const zoneLoad);

        void sizeSystem(EnergyPlusData &state, bool const FirstHVACIteration, int const AirLoopNum) override;
        int getAirInNode(EnergyPlusData &state, std::string_view UnitarySysName, int const ZoneOAUnitNum, bool &errFlag) override;
        int getAirOutNode(EnergyPlusData &state, std::string_view UnitarySysName, int const ZoneOAUnitNum, bool &errFlag) override;
        int getAirOutletNode() override;
        int getMixerOANode() override;
        int getMixerMixNode() override;
        int getMixerRetNode() override;
        int getEquipIndex() override;

        UnitarySys() = default;
        virtual ~UnitarySys() = default;
    };

    int getDesignSpecMSHPIndex(EnergyPlusData &state, std::string_view objectName);
    int getUnitarySystemIndex(EnergyPlusData &state, std::string_view objectName);
    bool getUnitarySystemNodeNumber(EnergyPlusData &state, int const nodeNumber);

    bool searchZoneInletNodes(EnergyPlusData &state, int nodeToFind, int &ZoneEquipConfigIndex, int &InletNodeIndex);
    bool searchZoneInletNodesByEquipmentIndex(EnergyPlusData &state, int nodeToFind, int zoneEquipmentIndex);
    bool searchZoneInletNodeAirLoopNum(EnergyPlusData &state, int airLoopNumToFind, int ZoneEquipConfigIndex, int &InletNodeIndex);
    bool searchExhaustNodes(EnergyPlusData &state, const int nodeToFind, int &ZoneEquipConfigIndex, int &ExhaustNodeIndex);
    bool searchTotalComponents(EnergyPlusData &state,
                               SimAirServingZones::CompType compTypeToFind,
                               std::string_view objectNameToFind,
                               int &compIndex,
                               int &branchIndex,
                               int &airLoopIndex);
    void setupAllOutputVars(EnergyPlusData &state, int const numAllSystemTypes);
    void isWaterCoilHeatRecoveryType(EnergyPlusData const &state, int const waterCoilNodeNum, bool &nodeNotFound);

    int
    getZoneEqIndex(EnergyPlusData &state, std::string const &UnitarySysName, DataZoneEquipment::ZoneEquipType zoneEquipType, int const OAUnitNum = 0);

} // namespace UnitarySystems
struct UnitarySystemsData : BaseGlobalStruct
{

    // MODULE PARAMETER DEFINITIONS
    int numUnitarySystems = 0;
    bool economizerFlag = false;      // holds air loop economizer status
    bool SuppHeatingCoilFlag = false; // set to TRUE when simulating supplemental heating coil
    bool HeatingLoad = false;         // True when zone needs heating
    bool CoolingLoad = false;         // True when zone needs cooling
    Real64 MoistureLoad = 0.0;        // Dehumidification Load (W)
    Real64 CompOnMassFlow = 0.0;      // Supply air mass flow rate w/ compressor ON [kg/s]
    Real64 CompOffMassFlow = 0.0;     // Supply air mass flow rate w/ compressor OFF [kg/s]
    Real64 OACompOnMassFlow = 0.0;    // OA mass flow rate w/ compressor ON [kg/s]
    Real64 OACompOffMassFlow = 0.0;   // OA mass flow rate w/ compressor OFF [kg/s]

    Real64 CompOnFlowRatio = 0.0;       // fan flow ratio when coil on
    Real64 CompOffFlowRatio = 0.0;      // fan flow ratio when coil off
    Real64 FanSpeedRatio = 0.0;         // ratio of air flow ratio passed to fan object
    Real64 CoolHeatPLRRat = 1.0;        // ratio of cooling to heating PLR, used for cycling fan RH control
    Real64 OnOffAirFlowRatioSave = 0.0; // Saves the OnOffAirFlowRatio calculated in RegulaFalsi calls.
    Real64 QToCoolSetPt = 0.0;          // load to cooling set point {W}
    Real64 QToHeatSetPt = 0.0;          // load to heating set point {W}
    Real64 m_massFlow1 = 0.0;           // Mass flow rate in operating mode 1 (Compressor On) [kg/s]
    Real64 m_massFlow2 = 0.0;           // Mass flow rate in operating mode 2 (Compressor Off) [kg/s]
    Real64 m_runTimeFraction1 = 0.0;    // Fan runtime fraction in operating mode 1 (Compressor On)
    Real64 m_runTimeFraction2 = 0.0;    // Fan runtime fraction in operating mode 2 (Compressor Off)

    bool initUnitarySystemsErrFlag = false;
    bool initUnitarySystemsErrorsFound = false;
    bool initLoadBasedControlFlowFracFlagReady = true;
    Real64 initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax = 0.0;
    Real64 initUnitarySystemsQActual = 0.0;

    bool getInputOnceFlag = true;
    bool getMSHPInputOnceFlag = true;

    std::vector<UnitarySystems::UnitarySys> unitarySys;
    std::vector<UnitarySystems::DesignSpecMSHP> designSpecMSHP;

    bool getInputFlag = true;
    bool setupOutputOnce = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        numUnitarySystems = 0;
        HeatingLoad = false;
        CoolingLoad = false;
        MoistureLoad = 0.0;
        SuppHeatingCoilFlag = false;
        CompOnMassFlow = 0.0;
        CompOffMassFlow = 0.0;
        CompOnFlowRatio = 0.0;
        CompOffFlowRatio = 0.0;
        OACompOnMassFlow = 0.0;
        OACompOffMassFlow = 0.0;
        FanSpeedRatio = 0.0;
        CoolHeatPLRRat = 1.0;
        OnOffAirFlowRatioSave = 0.0;
        QToCoolSetPt = 0.0;
        QToHeatSetPt = 0.0;
        m_massFlow1 = 0.0;
        m_massFlow2 = 0.0;
        m_runTimeFraction1 = 0.0;
        m_runTimeFraction2 = 0.0;

        initUnitarySystemsErrFlag = false;
        initUnitarySystemsErrorsFound = false;
        initLoadBasedControlFlowFracFlagReady = true;
        initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax = 0.0;
        initUnitarySystemsQActual = 0.0;
        getMSHPInputOnceFlag = true;
        getInputOnceFlag = true;
        setupOutputOnce = true;
        unitarySys.clear();
        if (designSpecMSHP.size() > 0) {
            designSpecMSHP.clear();
        }
        getInputFlag = true;
    }

    // Default Constructor
    UnitarySystemsData() = default;
};
} // namespace EnergyPlus
#endif // ENERGYPLUS_UNITARYSYSTEM_HH
