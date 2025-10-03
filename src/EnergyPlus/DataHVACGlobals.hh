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

#ifndef DataHVACGlobals_hh_INCLUDED
#define DataHVACGlobals_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace HVAC {

    // Using/Aliasing

    // Data
    // -only module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.

    enum class CtrlVarType
    {
        Invalid = -1,
        Temp,
        MaxTemp,
        MinTemp,
        HumRat,
        MaxHumRat,
        MinHumRat,
        MassFlowRate,
        MaxMassFlowRate,
        MinMassFlowRate,
        Num
    };

    // MODULE PARAMETER DEFINITIONS:

    Real64 constexpr SmallHumRatDiff(1.0E-7);
    Real64 constexpr SmallTempDiff(1.0E-5);
    Real64 constexpr SmallMassFlow(0.001);
    Real64 constexpr VerySmallMassFlow(1.0E-30);
    Real64 constexpr SmallLoad(1.0);
    Real64 constexpr TempControlTol(0.1); // temperature control tolerance for packaged equip. [deg C]
    Real64 constexpr SmallAirVolFlow(0.001);
    Real64 constexpr SmallWaterVolFlow(1.0E-9);
    Real64 constexpr BlankNumeric(-99999.0);      // indicates numeric input field was blank
    Real64 constexpr RetTempMax(60.0);            // maximum return air temperature [deg C]
    Real64 constexpr RetTempMin(-30.0);           // minimum return air temperature [deg C]
    Real64 constexpr DesCoilHWInletTempMin(46.0); // minimum heating water coil water inlet temp for UA sizing only. [deg C]

    int constexpr NumOfSizingTypes(35); // request sizing for cooling air flow rate

    // Sizing types
    int constexpr CoolingAirflowSizing(1);                  // request sizing for cooling air flow rate
    int constexpr CoolingWaterDesWaterInletTempSizing(6);   // request sizing for cooling water coil inlet water temp
    int constexpr HeatingAirflowSizing(14);                 // request sizing for heating air flow rate
    int constexpr SystemAirflowSizing(16);                  // request sizing for system air flow rate
    int constexpr CoolingCapacitySizing(17);                // request sizing for cooling capacity
    int constexpr HeatingCapacitySizing(18);                // request sizing for heating capacity
    int constexpr SystemCapacitySizing(21);                 // request sizing for system capacity
    [[maybe_unused]] int constexpr AutoCalculateSizing(25); // identifies an autocalulate input

    // The following parameters describe the setpoint types in TempControlType(ActualZoneNum)
    enum class SetptType
    {
        Invalid = -1,
        Uncontrolled,
        SingleHeat,
        SingleCool,
        SingleHeatCool,
        DualHeatCool,
        Num
    };

    static constexpr std::array<SetptType, 4> controlledSetptTypes = {
        SetptType::SingleHeat, SetptType::SingleCool, SetptType::SingleHeatCool, SetptType::DualHeatCool};

    static constexpr std::array<std::string_view, (int)SetptType::Num> setptTypeNames = {
        "Uncontrolled", "SingleHeating", "SingleCooling", "SingleHeatCool", "DualSetPointWithDeadBand"};

    enum class AirDuctType
    // parameters describing air duct type
    {
        Invalid = -1,
        Main,
        Cooling,
        Heating,
        Other,
        RAB,
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(AirDuctType::Num)> airDuctTypeNames = {
        "Main", "Cooling", "Heating", "Other", "Return Air Bypass"};

    int constexpr Cooling(2);
    int constexpr Heating(3);

    enum class FanType
    {
        Invalid = -1,
        Constant,
        VAV,
        OnOff,
        Exhaust,
        ComponentModel,
        SystemModel,
        Num
    };

    extern const std::array<std::string_view, (int)FanType::Num> fanTypeNames;
    extern const std::array<std::string_view, (int)FanType::Num> fanTypeNamesUC;

    // Fan mode

    enum class FanOp
    {
        Invalid = -1,
        Cycling,
        Continuous,
        Num
    };

    // Fan placement
    enum class FanPlace
    {
        Invalid = -1,
        BlowThru,
        DrawThru,
        Num
    };

    static constexpr std::array<std::string_view, (int)FanPlace::Num> fanPlaceNamesUC = {"BLOWTHROUGH", "DRAWTHROUGH"};

    // OA Controller Heat Recovery Bypass Control Types
    int constexpr BypassWhenWithinEconomizerLimits(0);   // heat recovery controlled by economizer limits
    int constexpr BypassWhenOAFlowGreaterThanMinimum(1); // heat recovery ON at minimum OA in economizer mode

    enum class EconomizerStagingType
    // OA Controller Economizer Staging
    {
        Invalid = -1,
        EconomizerFirst,                  // system air flow rate and economizer is ramped-up before using mechanical cooling
        InterlockedWithMechanicalCooling, // economizer operation (flow rate) depends on the cooling speed chosen by the system
        Num
    };

    static constexpr std::array<std::string_view, (int)EconomizerStagingType::Num> economizerStagingTypeNamesUC = {
        "ECONOMIZERFIRST",
        "INTERLOCKEDWITHMECHANICALCOOLING",
    };
    static constexpr std::array<std::string_view, (int)EconomizerStagingType::Num> economizerStagingTypeNames = {
        "EconomizerFirst",
        "InterlockedWithMechanicalCooling",
    };

    enum class UnitarySysType
    {
        Invalid = -1,
        Furnace_HeatOnly,
        Furnace_HeatCool,
        Unitary_HeatOnly,
        Unitary_HeatCool,
        Unitary_HeatPump_AirToAir,
        Unitary_HeatPump_WaterToAir,
        Unitary_AnyCoilType,
        Num
    };

    extern const std::array<std::string_view, (int)UnitarySysType::Num> unitarySysTypeNames;
    extern const std::array<std::string_view, (int)UnitarySysType::Num> unitarySysTypeNamesUC;

    enum class CoilType
    {
        Invalid = -1,
        DXCoolingSingleSpeed,
        DXHeatingEmpirical,
        DXCoolingTwoSpeed,
        DXCoolingHXAssisted,
        DXCoolingTwoStageWHumControl,
        DXHeatPumpWaterHeaterPumped,
        DXHeatPumpWaterHeaterWrapped,
        DXMultiSpeedCooling,
        DXMultiSpeedHeating,
        HeatingGasOrOtherFuel,
        HeatingGasMultiStage,
        HeatingElectric,
        HeatingElectricMultiStage,
        HeatingDesuperheater,
        CoolingWater,
        CoolingWaterDetailed,
        HeatingWater,
        HeatingSteam,
        WaterCoolingHXAssisted,
        CoolingWaterToAirHP,
        HeatingWaterToAirHP,
        CoolingWaterToAirHPSimple,
        HeatingWaterToAirHPSimple,
        VRFCooling,
        VRFHeating,
        UserDefined,
        DXPackagedThermalStorageCooling,
        CoolingWaterToAirHPVSEquationFit,
        HeatingWaterToAirHPVSEquationFit,
        CoolingAirToAirVariableSpeed,
        HeatingAirToAirVariableSpeed,
        DXHeatPumpWaterHeaterVariableSpeed,
        VRFFluidTCtrlCooling,
        VRFFluidTCtrlHeating,
        DXCooling,
        DXSubcoolReheat,
        DXCurveFitSpeed,
        Num
    };
    static constexpr std::array<std::string_view, static_cast<int>(CoilType::Num)> coilTypeNamesUC = {
        "COIL:COOLING:DX:SINGLESPEED",
        "COIL:HEATING:DX:SINGLESPEED",
        "COIL:COOLING:DX:TWOSPEED",
        "COILSYSTEM:COOLING:DX:HEATEXCHANGERASSISTED",
        "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE",
        "COIL:WATERHEATING:AIRTOWATERHEATPUMP:PUMPED",
        "COIL:WATERHEATING:AIRTOWATERHEATPUMP:WRAPPED",
        "COIL:COOLING:DX:MULTISPEED",
        "COIL:HEATING:DX:MULTISPEED",
        "COIL:HEATING:FUEL",
        "COIL:HEATING:GAS:MULTISTAGE",
        "COIL:HEATING:ELECTRIC",
        "COIL:HEATING:ELECTRIC:MULTISTAGE",
        "COIL:HEATING:DESUPERHEATER",
        "COIL:COOLING:WATER",
        "COIL:COOLING:WATER:DETAILEDGEOMETRY",
        "COIL:HEATING:WATER",
        "COIL:HEATING:STEAM",
        "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED",
        "COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION",
        "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION",
        "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT",
        "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT",
        "COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW",
        "COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW",
        "COIL:USERDEFINED",
        "COIL:COOLING:DX:SINGLESPEED:THERMALSTORAGE",
        "COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT",
        "COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT",
        "COIL:COOLING:DX:VARIABLESPEED",
        "COIL:HEATING:DX:VARIABLESPEED",
        "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED",
        "COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL",
        "COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL",
        "COIL:COOLING:DX",
        "COIL:COOLING:DX:SUBCOOLREHEAT",
        "COIL:COOLING:DX:CURVEFIT:SPEED"};

    // parameters describing coil types
    int constexpr NumAllCoilTypes(37);
    int constexpr CoilDX_CoolingSingleSpeed(1);
    int constexpr CoilDX_HeatingEmpirical(2);
    int constexpr CoilDX_CoolingTwoSpeed(3);
    int constexpr CoilDX_CoolingHXAssisted(4);
    int constexpr CoilDX_CoolingTwoStageWHumControl(5);
    int constexpr CoilDX_HeatPumpWaterHeaterPumped(6);
    int constexpr CoilDX_HeatPumpWaterHeaterWrapped(7);
    int constexpr CoilDX_MultiSpeedCooling(8);
    int constexpr CoilDX_MultiSpeedHeating(9);
    int constexpr Coil_HeatingGasOrOtherFuel(10);
    int constexpr Coil_HeatingGas_MultiStage(11);
    int constexpr Coil_HeatingElectric(12);
    int constexpr Coil_HeatingElectric_MultiStage(13);
    int constexpr Coil_HeatingDesuperheater(14);
    int constexpr Coil_CoolingWater(15);
    int constexpr Coil_CoolingWaterDetailed(16);
    int constexpr Coil_HeatingWater(17);
    int constexpr Coil_HeatingSteam(18);
    int constexpr CoilWater_CoolingHXAssisted(19);
    int constexpr Coil_CoolingWaterToAirHP(20);
    int constexpr Coil_HeatingWaterToAirHP(21);
    int constexpr Coil_CoolingWaterToAirHPSimple(22);
    int constexpr Coil_HeatingWaterToAirHPSimple(23);
    int constexpr CoilVRF_Cooling(24);
    int constexpr CoilVRF_Heating(25);
    int constexpr Coil_UserDefined(26);
    int constexpr CoilDX_PackagedThermalStorageCooling(27);
    int constexpr Coil_CoolingWaterToAirHPVSEquationFit(28);
    int constexpr Coil_HeatingWaterToAirHPVSEquationFit(29);
    int constexpr Coil_CoolingAirToAirVariableSpeed(30);
    int constexpr Coil_HeatingAirToAirVariableSpeed(31);
    int constexpr CoilDX_HeatPumpWaterHeaterVariableSpeed(32);
    int constexpr CoilVRF_FluidTCtrl_Cooling(33);
    int constexpr CoilVRF_FluidTCtrl_Heating(34);
    int constexpr CoilDX_Cooling(35);
    //    int constexpr CoilDX_SubcoolReheat(36);
    int constexpr CoilDX_CurveFit_Speed(37);

    enum class CoilMode
    {
        Invalid = -1,
        Normal,
        Enhanced,
        SubcoolReheat,
        Num
    };

    enum class WaterFlow
    {
        Invalid = -1,
        Cycling,
        Constant,
        ConstantOnDemand,
        Num
    };

    extern const std::array<std::string_view, (int)WaterFlow::Num> waterFlowNames;
    extern const std::array<std::string_view, (int)WaterFlow::Num> waterFlowNamesUC;

    // parameters describing coil performance types
    int constexpr CoilPerfDX_CoolBypassEmpirical(100);

    // Airflow per total capacity range (Regular DX coils)
    Real64 constexpr MaxRatedVolFlowPerRatedTotCap1(0.00006041); // m3/s per watt = 450 cfm/ton
    Real64 constexpr MinRatedVolFlowPerRatedTotCap1(0.00004027); // m3/s per watt = 300 cfm/ton
    Real64 constexpr MaxHeatVolFlowPerRatedTotCap1(0.00008056);  // m3/s per watt = 600 cfm/ton
    Real64 constexpr MaxCoolVolFlowPerRatedTotCap1(0.00006713);  // m3/s per watt = 500 cfm/ton
    Real64 constexpr MinOperVolFlowPerRatedTotCap1(0.00002684);  // m3/s per watt = 200 cfm/ton

    // 100% DOAS DX coils Airflow per total capacity ratio
    Real64 constexpr MaxRatedVolFlowPerRatedTotCap2(0.00003355); // m3/s per watt = 250 cfm/ton
    Real64 constexpr MinRatedVolFlowPerRatedTotCap2(0.00001677); // m3/s per watt = 125 cfm/ton
    Real64 constexpr MaxHeatVolFlowPerRatedTotCap2(0.00004026);  // m3/s per watt = 300 cfm/ton
    Real64 constexpr MaxCoolVolFlowPerRatedTotCap2(0.00004026);  // m3/s per watt = 300 cfm/ton
    Real64 constexpr MinOperVolFlowPerRatedTotCap2(0.00001342);  // m3/s per watt = 100 cfm/ton

    constexpr std::array<Real64, 2> MaxRatedVolFlowPerRatedTotCap = {MaxRatedVolFlowPerRatedTotCap1, MaxRatedVolFlowPerRatedTotCap2};
    constexpr std::array<Real64, 2> MinRatedVolFlowPerRatedTotCap = {MinRatedVolFlowPerRatedTotCap1, MinRatedVolFlowPerRatedTotCap2};
    constexpr std::array<Real64, 2> MaxHeatVolFlowPerRatedTotCap = {MaxHeatVolFlowPerRatedTotCap1, MaxHeatVolFlowPerRatedTotCap2};
    constexpr std::array<Real64, 2> MaxCoolVolFlowPerRatedTotCap = {MaxCoolVolFlowPerRatedTotCap1, MaxCoolVolFlowPerRatedTotCap2};
    constexpr std::array<Real64, 2> MinOperVolFlowPerRatedTotCap = {MinOperVolFlowPerRatedTotCap1, MinOperVolFlowPerRatedTotCap2};

    // dx coil type (DXCT)
    enum class DXCoilType
    {
        Invalid = -1,
        Regular,
        DOAS,
        Num
    };

    enum class HXType
    {
        Invalid = -1,
        AirToAir_FlatPlate,
        AirToAir_SensAndLatent,
        Desiccant_Balanced,
        Num
    };

    extern const std::array<std::string_view, (int)HXType::Num> hxTypeNames;
    extern const std::array<std::string_view, (int)HXType::Num> hxTypeNamesUC;

    enum class MixerType
    {
        Invalid = -1,
        InletSide,
        SupplySide,
        Num
    };

    extern const std::array<std::string_view, (int)MixerType::Num> mixerTypeNames;
    extern const std::array<std::string_view, (int)MixerType::Num> mixerTypeNamesUC;

    extern const std::array<std::string_view, (int)MixerType::Num> mixerTypeLocNames;
    extern const std::array<std::string_view, (int)MixerType::Num> mixerTypeLocNamesUC;

    enum class OATType
    {
        Invalid = -1,
        WetBulb,
        DryBulb,
        Num
    };

    extern const std::array<std::string_view, (int)OATType::Num> oatTypeNames;
    extern const std::array<std::string_view, (int)OATType::Num> oatTypeNamesUC;

    // parameter concerning the amount of change in zone temperature is needed
    // for oscillation of zone temperature to be detected.
    Real64 constexpr OscillateMagnitude(0.15);

    int constexpr MaxSpeedLevels = 10;

    extern Array1D_string const cAllCoilTypes;
    extern Array1D_string const cCoolingCoilTypes;
    extern Array1D_string const cHeatingCoilTypes;

    struct ComponentSetPtData
    {
        // Members
        std::string EquipmentType;
        std::string EquipmentName;
        int NodeNumIn = 0;
        int NodeNumOut = 0;
        Real64 EquipDemand = 0.0;
        Real64 DesignFlowRate = 0.0;
        std::string HeatOrCool;
        int OpType = 0;
    };

    // Compressor operation
    enum class CompressorOp
    {
        Invalid = -1,
        Off, // signal DXCoil that compressor shouldn't run
        On,  // normal compressor operation
        Num
    };
} // namespace HVAC

struct HVACGlobalsData : BaseGlobalStruct
{
    // Object Data
    Array1D<HVAC::ComponentSetPtData> CompSetPtEquip;

    // For multispeed heat pump only
    Real64 MSHPMassFlowRateLow = 0.0;       // Mass flow rate at low speed
    Real64 MSHPMassFlowRateHigh = 0.0;      // Mass flow rate at high speed
    Real64 MSHPWasteHeat = 0.0;             // Waste heat
    Real64 PreviousTimeStep = 0.0;          // The time step length at the previous time step
    bool ShortenTimeStepSysRoomAir = false; // Logical flag that triggers shortening of system time step
    // For multispeed unitary systems
    Real64 MSUSEconoSpeedNum = 0; // Economizer speed

    Real64 deviationFromSetPtThresholdHtg = -0.2; // heating threshold for reporting setpoint deviation
    Real64 deviationFromSetPtThresholdClg = 0.2;  // cooling threshold for reporting setpoint deviation

    bool SimAirLoopsFlag = false;           // True when the air loops need to be (re)simulated
    bool SimElecCircuitsFlag = false;       // True when electic circuits need to be (re)simulated
    bool SimPlantLoopsFlag = false;         // True when the main plant loops need to be (re)simulated
    bool SimZoneEquipmentFlag = false;      // True when zone equipment components need to be (re)simulated
    bool SimNonZoneEquipmentFlag = false;   // True when non-zone equipment components need to be (re)simulated
    bool ZoneMassBalanceHVACReSim = false;  // True when zone air mass flow balance and air loop needs (re)simulated
    int MinAirLoopIterationsAfterFirst = 1; // minimum number of HVAC iterations after FirstHVACIteration

    HVAC::DXCoilType DXCT = HVAC::DXCoilType::Regular; // dx coil type: regular DX coil ==1, 100% DOAS DX coil = 2
    bool FirstTimeStepSysFlag = false;                 // Set to true at the start of each sub-time step

    Real64 TimeStepSys = 0.0;                  // System Time Increment - the adaptive time step used by the HVAC simulation (hours)
    Real64 TimeStepSysSec = 0.0;               // System Time Increment in seconds
    Real64 SysTimeElapsed = 0.0;               // elapsed system time in zone timestep (hours)
    Real64 FracTimeStepZone = 0.0;             // System time step divided by the zone time step
    bool ShortenTimeStepSys = false;           // Logical flag that triggers shortening of system time step
    int NumOfSysTimeSteps = 1;                 // for current zone time step, number of system timesteps inside  it
    int NumOfSysTimeStepsLastZoneTimeStep = 1; // previous zone time step, num of system timesteps inside
    int LimitNumSysSteps = 0;

    bool UseZoneTimeStepHistory = true;    // triggers use of zone time step history, else system time step history, for ZTM1, ZTMx
    int NumPlantLoops = 0;                 // Number of plant loops specified in simulation
    int NumCondLoops = 0;                  // Number of condenser plant loops specified in simulation
    int NumElecCircuits = 0;               // Number of electric circuits specified in simulation
    int NumGasMeters = 0;                  // Number of gas meters specified in simulation
    int NumPrimaryAirSys = 0;              // Number of primary HVAC air systems
    Real64 OnOffFanPartLoadFraction = 1.0; // fan part-load fraction (Fan:OnOff)
    Real64 DXCoilTotalCapacity = 0.0;      // DX coil total cooling capacity (eio report var for HPWHs)
    Real64 DXElecCoolingPower = 0.0;       // Electric power consumed by DX cooling coil last DX simulation
    Real64 DXElecHeatingPower = 0.0;       // Electric power consumed by DX heating coil last DX simulation
    Real64 ElecHeatingCoilPower = 0.0;     // Electric power consumed by electric heating coil
    Real64 SuppHeatingCoilPower = 0.0;     // Electric power consumed by electric supplemental heating coil
    Real64 AirToAirHXElecPower = 0.0;      // Electric power consumed by Heat Exchanger:Air To Air (Generic or Flat Plate)
    Real64 DefrostElecPower = 0.0;         // Electric power consumed by DX heating coil for defrosting (Resistive or ReverseCycle)
    // from last simulation in HeatRecovery.cc
    Real64 UnbalExhMassFlow = 0.0;      // unbalanced zone exhaust from a zone equip component [kg/s]
    Real64 BalancedExhMassFlow = 0.0;   // balanced zone exhaust (declared as so by user)  [kg/s]
    Real64 PlenumInducedMassFlow = 0.0; // secondary air mass flow rate induced from a return plenum [kg/s]
    bool TurnFansOn = false;            // If true overrides fan schedule and cycles fans on
    bool TurnFansOff = false;           // If True overides fan schedule and TurnFansOn and forces fans off
    bool SetPointErrorFlag = false;     // True if any needed setpoints not set; if true, program terminates
    bool DoSetPointTest = false;        // True one time only for sensed node setpoint test
    bool NightVentOn = false;           // set TRUE in SimAirServingZone if night ventilation is happening

    int NumTempContComps = 0;
    Real64 HPWHInletDBTemp = 0.0;     // Used by curve objects when calculating DX coil performance for HEAT PUMP:WATER HEATER
    Real64 HPWHInletWBTemp = 0.0;     // Used by curve objects when calculating DX coil performance for HEAT PUMP:WATER HEATER
    Real64 HPWHCrankcaseDBTemp = 0.0; // Used for HEAT PUMP:WATER HEATER crankcase heater ambient temperature calculations
    bool AirLoopInit = false;         // flag for whether InitAirLoops has been called
    bool AirLoopsSimOnce = false;     // True means that the air loops have been simulated once in this environment
    bool GetAirPathDataDone = false;  // True means that air loops inputs have been processed
    bool StandardRatingsMyOneTimeFlag = true;
    bool StandardRatingsMyCoolOneTimeFlag = true;
    bool StandardRatingsMyCoolOneTimeFlag2 = true;
    bool StandardRatingsMyCoolOneTimeFlag3 = true;
    bool StandardRatingsMyHeatOneTimeFlag = true;
    bool StandardRatingsMyHeatOneTimeFlag2 = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HVACGlobalsData();
    }
};

} // namespace EnergyPlus

#endif
