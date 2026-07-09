// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

#ifndef HVACVariableRefrigerantFlow_hh_INCLUDED
#define HVACVariableRefrigerantFlow_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/UnitarySystem.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACVariableRefrigerantFlow {

    // Parameters describing variable refrigerant flow terminal unit types
    enum class TUType
    {
        Invalid = -1,
        ConstantVolume,
        Num
    };

    // Thermostat Priority Control Type
    enum class ThermostatCtrlType
    {
        Invalid = -1,
        LoadPriority,             // total of zone loads dictate operation in cooling or heating
        MasterThermostatPriority, // Master zone thermostat dictates operation
        Scheduled,                // cooling and heating modes are scheduled
        ThermostatOffsetPriority, // zone with largest deviation from setpoint dictates operation
        ZonePriority,             // # of zones requiring cooling or heating dictate operation in cooling or heating
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(ThermostatCtrlType::Num)> ThermostatCtrlTypeUC = {
        "LOADPRIORITY",
        "MASTERTHERMOSTATPRIORITY",
        "SCHEDULED",
        "THERMOSTATOFFSETPRIORITY",
        "ZONEPRIORITY",
    };

    enum class EvapWaterSupply
    {
        Invalid = -1,
        FromMains, // mains water line used as water source
        FromTank,  // storage tank used as water source
        Num
    };

    // VRF Algorithm Type
    enum class AlgorithmType
    {
        Invalid = -1,
        SysCurve,   // VRF model based on system curve
        FluidTCtrl, // VRF model based on physics, appreciable for Fluid Temperature Control
        Num
    };

    // VRF System Types (strings used in integer conversions)
    constexpr int VRF_HeatPump(1);

    constexpr auto cVRFTypes(int i)
    {
        if (i == 1) {
            return "AirConditioner:VariableRefrigerantFlow";
        }
        assert(false);
        return "";
    }

    // Flag for hex operation
    enum class HXOpMode
    {
        Invalid = -1,
        CondMode, // Flag for the hex running as condenser [-]
        EvapMode, // Flag for the hex running as evaporator [-]
        Num
    };

    // Flag for VRF operational mode
    constexpr int ModeCoolingOnly = 1;       // Flag for Cooling Only Mode [-]
    constexpr int ModeHeatingOnly = 2;       // Flag for Heating Only Mode [-]
    constexpr int ModeCoolingAndHeating = 3; // Flag for Simultaneous Cooling and Heating Only Mode [-]

    // Subroutine Specifications for the Module
    struct VRFCondenserEquipment : PlantComponent
    {
        // Members
        std::string Name;                      // Name of the VRF Terminal Unit
        int VRFSystemTypeNum;                  // integer equivalent of system type
        AlgorithmType VRFAlgorithmType;        // Algorithm type: 1_system curve based model; 2_physics based model (FluidTCtrl)
        DataPlant::PlantEquipmentType VRFType; // integer equivalent of index to DataPlant type
        PlantLocation SourcePlantLoc;          // plant data for water-cooled only
        Real64 WaterCondenserDesignMassFlow;   // plant data for water-cooled only
        Real64 WaterCondenserMassFlow;         // Water condenser flow rate (kg/s)
        Real64 QCondenser;                     // Water condenser heat rejection/absorption (W)
        Real64 QCondEnergy;                    // Water condenser heat rejection/absorption energy (J)
        Real64 CondenserSideOutletTemp;        // Water condenser outlet temp (C)
        Sched::Schedule *availSched = nullptr; // Pointer to the correct schedule // LOL
        Real64 CoolingCapacity;                // Nominal VRF heat pump cooling capacity (W)
        Real64 TotalCoolingCapacity;           // Nominal VRF heat pump cooling capacity (W)
        Real64 CoolingCombinationRatio;        // Ratio or terminal unit cooling capacity to VRF condenser capacity
        Real64 VRFCondPLR;                     // Condenser part-load ratio wrt total capacity
        Real64 VRFCondRTF;                     // Condenser runtime fraction
        Real64 VRFCondCyclingRatio;            // Condenser cycling ratio below MinPLR
        Real64 CondenserInletTemp;             // Condenser entering air temperature (C)
        Real64 CoolingCOP;                     // Nominal VRF heat pump cooling COP (W/W)
        Real64 OperatingCoolingCOP;            // Operating VRF heat pump cooling COP (W/W)
        Real64 RatedCoolingPower;              // Rated cooling power = Rated Cooling Capacity / Rated COP (W)
        Real64 HeatingCapacity;                // Nominal VRF heat pump heating capacity (W)
        Real64 HeatingCapacitySizeRatio;       // Ratio of heating to cooling when autosizing
        bool LockHeatingCapacity;              // used in sizing to size VRF heat cap to VRF cool cap
        Real64 TotalHeatingCapacity;           // Nominal VRF heat pump heating capacity (W)
        Real64 HeatingCombinationRatio;        // Ratio or terminal unit heating capacity to VRF condenser capacity
        Real64 HeatingCOP;                     // Nominal VRF heat pump heating COP
        Real64 OperatingHeatingCOP;            // Operating VRF heat pump heating COP
        Real64 RatedHeatingPower;              // Rated heating power = Rated Heating Capacity / Rated COP (W)
        Real64 MinOATCooling;                  // Minimum outdoor air dry-bulb temp in cooling mode (C)
        Real64 MaxOATCooling;                  // Maximum outdoor air dry-bulb temp in cooling mode (C)
        Real64 MinOATHeating;                  // Minimum outdoor air dry-bulb temp in heating mode (C)
        Real64 MaxOATHeating;                  // Maximum outdoor air dry-bulb temp in heating mode (C)
        int CoolCapFT;                         // index to cooling capacity function of temperature curve
        int CoolEIRFT;                         // index to cooling EIR function of temperature curve
        int HeatCapFT;                         // index to heating capacity function of temperature curve
        int HeatEIRFT;                         // index to heating EIR function of temperature curve
        int CoolBoundaryCurvePtr;              // index to cooling capacity boundary curve
        int HeatBoundaryCurvePtr;              // index to cooling capacity boundary curve
        int EIRCoolBoundaryCurvePtr;           // index to cooling EIR boundary curve
        int CoolEIRFPLR1;                      // index to cooling EIR function of PLR curve < 1
        int CoolEIRFPLR2;                      // index to cooling EIR function of PLR curve >= 1
        int CoolCapFTHi;                       // index to cooling capacity function of temperature curve
        int CoolEIRFTHi;                       // index to cooling EIR function of temperature curve
        int HeatCapFTHi;                       // index to heating capacity function of temperature curve
        int HeatEIRFTHi;                       // index to heating EIR function of temperature curve
        int EIRHeatBoundaryCurvePtr;           // index to heating EIR boundary curve
        int HeatEIRFPLR1;                      // index to heating EIR function of PLR curve < 1
        int HeatEIRFPLR2;                      // index to heating EIR function of PLR curve >= 1
        int CoolPLFFPLR;                       // index to cooling PLF function of PLR curve
        int HeatPLFFPLR;                       // index to heating PLF function of PLR curve
        HVAC::OATType HeatingPerformanceOATType = HVAC::OATType::Invalid; // Temperature type for heating performance curves
        Real64 MinPLR;                                                    // minimum PLR before cycling occurs
        int MasterZonePtr;                                                // index to master thermostat zone
        int MasterZoneTUIndex;                                            // index to TU in master thermostat zone
        ThermostatCtrlType ThermostatPriority;                            // VRF priority control (1=LoadPriority, 2=ZonePriority, etc)
        Sched::Schedule *prioritySched = nullptr;                         // VRF priority control schedule
        int ZoneTUListPtr;                                                // index to zone terminal unit list
        bool HeatRecoveryUsed;                                            // .TRUE. = heat recovery used
        Real64 VertPipeLngth;                                             // vertical piping length (m)
        int PCFLengthCoolPtr;                                             // piping correction factor for length in cooling mode curve index
        Real64 PCFHeightCool;                                             // piping correction factor for height in cooling mode
        Real64 EquivPipeLngthCool;                                        // equivalent piping length for cooling
        Real64 PipingCorrectionCooling;                                   // piping correction factor for cooling
        int PCFLengthHeatPtr;                                             // piping correction factor for length in heating mode curve index
        Real64 PCFHeightHeat;                                             // piping correction factor for height in heating mode
        Real64 EquivPipeLngthHeat;                                        // equivalent piping length for heating
        Real64 PipingCorrectionHeating;                                   // piping correction factor for heating
        Real64 CCHeaterPower;                                             // crankcase heater power per compressor (W)
        Real64 CompressorSizeRatio;                                       // ratio of min compressor size to total capacity
        int NumCompressors;                                               // number of compressors in VRF condenser
        Real64 MaxOATCCHeater;                                            // maximum outdoor air dry-bulb temp for crankcase heater operation (C)
        // begin variables used for Defrost
        int DefrostEIRPtr;                                // index to defrost EIR curve
        Real64 DefrostFraction;                           // defrost time period fraction (hr)
        StandardRatings::DefrostStrat DefrostStrategy;    // Type of defrost (reversecycle or resistive)
        StandardRatings::HPdefrostControl DefrostControl; // type of defrost control (timed or ondemand)
        Real64 DefrostCapacity;                           // capacity of resistive defrost heating element (W)
        Real64 DefrostPower;                              // power used during defrost (W)
        Real64 DefrostConsumption;                        // energy used during defrost (J)
        Real64 MaxOATDefrost;                             // maximum outdoor air dry-bulb temp for defrost operation (C)
        // end variables used for Defrost
        DataHeatBalance::RefrigCondenserType CondenserType; // condenser type, evap- or air-cooled
        int CondenserNodeNum;                               // condenser inlet node number
        bool SkipCondenserNodeNumCheck;                     // used to check for duplicate node names
        int CondenserOutletNodeNum;                         // condenser outlet node number
        Real64 WaterCondVolFlowRate;                        // water condenser volume flow rate (m3/s)
        Real64 EvapCondEffectiveness;                       // evaporative condenser effectiveness
        Real64 EvapCondAirVolFlowRate;                      // air volume flow rate through condenser (m3/s)
        Real64 EvapCondPumpPower;                           // evaporative condenser water pump power (W)
        int CoolCombRatioPTR;                               // index to cooling combination ratio curve pointer
        int HeatCombRatioPTR;                               // index to heating combination ratio curve pointer
        int OperatingMode;                                  // VRF Condenser operating mode, 0=off, 1=cooling, 2=heating, 3=HR
        Real64 ElecPower;                                   // VRF Condenser power (W)
        Real64 ElecCoolingPower;                            // VRF Condenser power in cooling mode (W)
        Real64 ElecHeatingPower;                            // VRF Condenser power in heating mode (W)
        Real64 CoolElecConsumption;                         // VRF Condenser cooling energy (J)
        Real64 HeatElecConsumption;                         // VRF Condenser heating energy (J)
        Real64 CrankCaseHeaterPower;                        // VRF Condenser crankcase heater power (W)
        Real64 CrankCaseHeaterElecConsumption;              // VRF Condenser crankcase heater energy (J)
        Real64 EvapCondPumpElecPower;                       // VRF Condenser evaporatively cooled condenser pump power (W)
        Real64 EvapCondPumpElecConsumption;                 // VRF Condenser evaporatively cooled condenser pump elec consumption (J)
        Real64 EvapWaterConsumpRate;                        // VRF Condenser evaporatively cooled condenser water consumption (m3/s)
        int HRMaxTempLimitIndex = 0;                        // Warning message recurring error index
        int CoolingMaxTempLimitIndex = 0;                   // Warning message recurring error index
        int HeatingMaxTempLimitIndex = 0;                   // Warning message recurring error index
        Constant::eFuel fuel = Constant::eFuel::Invalid;    // Fuel type number
        Real64 SUMultiplier;                                // exponential timer for mode changes
        Real64 TUCoolingLoad;                               // total TU cooling load for each VRF system
        Real64 TUHeatingLoad;                               // total TU heating load for each VRF system
        bool SwitchedMode;                                  // used to derate capacity/power when system changes operating mode
        // begin variables used for heat recovery mode
        Real64 OperatingCOP;            // Operating VRF heat pump COP (total TU capacity/total power)
        Real64 MinOATHeatRecovery;      // Minimum outdoor air temperature for heat recovery operation (C)
        Real64 MaxOATHeatRecovery;      // Maximum outdoor air temperature for heat recovery operation (C)
        int HRCAPFTCool;                // Index to cool capacity as a function of temperature curve for heat recovery
        Real64 HRCAPFTCoolConst;        // constant used if curve is blank
        Real64 HRInitialCoolCapFrac;    // Fractional cooling degradation at the start of heat recovery from cooling mode
        Real64 HRCoolCapTC;             // Time constant used to recover from initial degratation in cooling heat recovery
        int HREIRFTCool;                // Index to cool EIR as a function of temperature curve for heat recovery
        Real64 HREIRFTCoolConst;        // constant used if curve is blank
        Real64 HRInitialCoolEIRFrac;    // Fractional EIR degradation at the start of heat recovery from cooling mode
        Real64 HRCoolEIRTC;             // Time constant used to recover from initial degratation in cooling heat recovery
        int HRCAPFTHeat;                // Index to heat capacity as a function of temperature curve for heat recovery
        Real64 HRCAPFTHeatConst;        // constant used if curve is blank
        Real64 HRInitialHeatCapFrac;    // Fractional heating degradation at the start of heat recovery from heating mode
        Real64 HRHeatCapTC;             // Time constant used to recover from initial degratation in heating heat recovery
        int HREIRFTHeat;                // Index to heat EIR as a function of temperature curve for heat recovery
        Real64 HREIRFTHeatConst;        // constant used if curve is blank
        Real64 HRInitialHeatEIRFrac;    // Fractional EIR degradation at the start of heat recovery from heating mode
        Real64 HRHeatEIRTC;             // Time constant used to recover from initial degratation in heating heat recovery
        bool HRCoolingActive;           // heat recovery mode active in cooling mode
        bool HRHeatingActive;           // heat recovery mode active in heating mode
        bool ModeChange;                // tracks changes in operating mode
        bool HRModeChange;              // tracks changes in heat recovery operating mode
        Real64 HRTimer;                 // timer used to model changes in system performance as mode changes
        Real64 HRTime;                  // length of time system has been in same mode (hr)
        int EIRFTempCoolErrorIndex = 0; // warning message index for recurring warnings
        int EIRFTempHeatErrorIndex = 0; // warning message index for recurring warnings
        int DefrostHeatErrorIndex = 0;  // warning message index for recurring warnings
        // end variables used for heat recovery mode
        // begin variables for Water System interactions
        EvapWaterSupply EvapWaterSupplyMode; // where does water come from
        std::string EvapWaterSupplyName;     // name of water source e.g. water storage tank
        int EvapWaterSupTankID;
        int EvapWaterTankDemandARRID;
        std::string CondensateCollectName; // name of water source e.g. water storage tank
        int CondensateTankID;
        int CondensateTankSupplyARRID;
        Real64 CondensateVdot; // rate of water condensation from air stream [m3/s]
        Real64 CondensateVol;  // amount of water condensed from air stream [m3]
        // end variables for water system interactions
        // begin variables for Basin Heater interactions
        Real64 BasinHeaterPowerFTempDiff;            // Basin heater capacity per degree C below setpoint (W/C)
        Real64 BasinHeaterSetPointTemp;              // setpoint temperature for basin heater operation (C)
        Real64 BasinHeaterPower;                     // Basin heater power (W)
        Real64 BasinHeaterConsumption;               // Basin heater energy consumption (J)
        Sched::Schedule *basinHeaterSched = nullptr; // Pointer to basin heater schedule
        // end variables for Basin Heater interactions
        bool EMSOverrideHPOperatingMode;
        Real64 EMSValueForHPOperatingMode;
        int HPOperatingModeErrorIndex;
        Real64 VRFHeatRec;             // Heat Recovery heat reclaim power (W)
        Real64 VRFHeatEnergyRec;       // Heat Recovery heat reclain energy (J)
        int HeatCapFTErrorIndex = 0;   // warning message index
        int CoolCapFTErrorIndex = 0;   // warning message index
        int HeatEIRFPLRErrorIndex = 0; // warning message index
        int CoolEIRFPLRErrorIndex = 0; // warning message index
        int LowLoadTeIterError = 0;
        int LowLoadTeError2Neg = 0;
        int LowLoadTeError2NegIndex = 0; // warning message index
        int LowLoadTeError2PosTsuc = 0;
        int LowLoadTeError2PosTsucIndex = 0; // warning message index
        int LowLoadTeError2PosOUTe = 0;
        int LowLoadTeError2PosOUTeIndex = 0; // warning message index
        int LowLoadTeErrorIndex = 0;         // warning message index
        // The following are for the Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
        int AlgorithmIUCtrl;             // VRF indoor unit control algorithm, 1-High sensible, 2-Te/Tc constant
        Array1D<Real64> CompressorSpeed; // compressor speed array [rps]
        Real64 CondensingTemp;           // VRV system outdoor unit condensing temperature [C]
        Real64 CondTempFixed;            // Inddor unit condensing temperature, fixed, for AlgorithmIUCtrl is 2-Te/Tc constant [C]
        Real64 CoffEvapCap;              // Evaporative Capacity Correction Factor
        Real64 CompActSpeed;             // Compressor speed [rps]
        Real64 CompMaxDeltaP;            // maximum compressor pressure rise [Pa]
        Real64 C1Te;                     // VRF Outdoor Unit Coefficient 1 to calculate Te,req [--]
        Real64 C2Te;                     // VRF Outdoor Unit Coefficient 2 to calculate Te,req [--]
        Real64 C3Te;                     // VRF Outdoor Unit Coefficient 3 to calculate Te,req [--]
        Real64 C1Tc;                     // VRF Outdoor Unit Coefficient 1 to calculate Tc,req [--]
        Real64 C2Tc;                     // VRF Outdoor Unit Coefficient 2 to calculate Tc,req [--]
        Real64 C3Tc;                     // VRF Outdoor Unit Coefficient 3 to calculate Tc,req [--]
        Real64 DiffOUTeTo;               // Difference between Outdoor Unit Te and OAT during Simultaneous Heating and Cooling operations
        Real64 EffCompInverter;          // Compressor Inverter Efficiency
        Real64 EvaporatingTemp;          // VRV system outdoor unit evaporating temperature [C]
        Real64 EvapTempFixed;            // Indoor unit evaporating temperature, fixed, for AlgorithmIUCtrl is 2-Te/Tc constant [C]
        Real64 HROUHexRatio;             // HR OU Heat Exchanger Capacity Ratio [--]
        Real64 IUEvaporatingTemp;        // VRV system indoor unit evaporating temperature, min among all indoor units [C]
        Real64 IUCondensingTemp;         // VRV system indoor unit condensing temperature, max among all indoor units [C]
        Real64 IUEvapTempLow;            // VRV system indoor unit evaporating temperature, lower bound[C]
        Real64 IUEvapTempHigh;           // VRV system indoor unit evaporating temperature, higher bound [C]
        Real64 IUCondTempLow;            // VRV system indoor unit condensing temperature, lower bound [C]
        Real64 IUCondTempHigh;           // VRV system indoor unit condensing temperature, higher bound [C]
        Real64 IUCondHeatRate;           // Indoor Unit Condensers Total Heat Release Rate, excluding piping loss  [W]
        Real64 IUEvapHeatRate;           // Outdoor Unit Evaporators Total Heat Extract Rate, excluding piping loss  [W]
        Real64 Ncomp;                    // compressor electric power [W]
        Real64 NcompCooling;             // compressor electric power at cooling mode [W]
        Real64 NcompHeating;             // compressor electric power at heating mode [W]
        Array1D_int OUCoolingCAPFT;      // index to outdoor unit cooling capacity function of temperature at different compressor speed
        Array1D_int OUCoolingPWRFT;      // index to outdoor unit cooling power function of temperature at different compressor speed
        Real64 OUEvapTempLow;            // VRV system outdoor unit evaporating temperature, lower bound[C]
        Real64 OUEvapTempHigh;           // VRV system outdoor unit evaporating temperature, higher bound [C]
        Real64 OUCondTempLow;            // VRV system outdoor unit condensing temperature, lower bound [C]
        Real64 OUCondTempHigh;           // VRV system outdoor unit condensing temperature, higher bound [C]
        Real64 OUAirFlowRate;            // Max condenser air flow rate [m3/s]
        Real64 OUAirFlowRatePerCapcity;  // Max condenser air flow rate per Evaporative Capacity [m3/s]
        Real64 OUCondHeatRate;           // Outdoor Unit Condenser Heat Release Rate, excluding piping loss [W]
        Real64 OUEvapHeatRate;           // Outdoor Unit Evaporator Heat Extract Rate, excluding piping loss  [W]
        Real64 OUFanPower;               // Outdoor unit fan power at real conditions[W]
        std::string refrigName;          // Name of refrigerant, must match name in FluidName (see fluidpropertiesrefdata.idf)
        Fluid::RefrigProps *refrig;
        Real64 RatedEvapCapacity;         // Rated Evaporative Capacity [W]
        Real64 RatedHeatCapacity;         // Rated Heating Capacity [W]
        Real64 RatedCompPower;            // Rated Compressor Power [W]
        Real64 RatedCompPowerPerCapcity;  // Rated Compressor Power per Evaporative Capacity [W]
        Real64 RatedOUFanPower;           // Outdoor unit fan power at rated conditions [W]
        Real64 RatedOUFanPowerPerCapcity; // Rated outdoor unit fan power per Evaporative Capacity [W]
        Real64 RateBFOUEvap;              // Outdoor Unit Evaporator Rated Bypass Factor
        Real64 RateBFOUCond;              // Outdoor Unit Condenser Rated Bypass Factor
        Real64 RefPipDiaSuc;              // diameter of refrigerant pipe (suction gas) that links the outdoor unit to the indoor units [m]
        Real64 RefPipDiaDis;              // diameter of refrigerant pipe (discharge gas) that links the outdoor unit to the indoor units [m]
        Real64 RefPipLen;                 // length of refrigerant pipe that links the outdoor unit to the indoor units [m]
        Real64 RefPipEquLen;              // Equivalent length of refrigerant pipe for pressure drop calculations [m]
        Real64 RefPipHei;                 // height of refrigerant pipe that links the outdoor unit to the indoor units [m]
        Real64 RefPipInsThi;              // thickness of refrigerant pipe insulation [m]
        Real64 RefPipInsCon;              // thermal conductivity of refrigerant pipe insulation [W/mk]
        Real64 SH;                        // VRF outdoor unit superheating degrees [C]
        Real64 SC;                        // VRF outdoor unit subcooling degrees [C]
        Real64 SCHE;                      // Simultaneous Cooling and Heating Efficiency [Btu/h/W]
        Real64 SHLow;                     // VRF outdoor unit superheating degrees lower limit [C]
        Real64 SCLow;                     // VRF outdoor unit subcooling degrees lower limit [C]
        Real64 SHHigh;                    // VRF outdoor unit superheating degrees upper limit [C]
        Real64 SCHigh;                    // VRF outdoor unit subcooling degrees upper limit [C]
        Real64 VRFOperationSimPath;       // simulation path indicating the VRF operation mode [--]
        bool checkPlantCondTypeOneTime;
        int CondenserCapErrIdx; // recurring condenser capacity error index
        bool adjustedTe;

        // Default Constructor
        VRFCondenserEquipment()
            : VRFSystemTypeNum(0), VRFAlgorithmType(AlgorithmType::Invalid), VRFType(DataPlant::PlantEquipmentType::Invalid), SourcePlantLoc{},
              WaterCondenserDesignMassFlow(0.0), WaterCondenserMassFlow(0.0), QCondenser(0.0), QCondEnergy(0.0), CondenserSideOutletTemp(0.0),
              CoolingCapacity(0.0), TotalCoolingCapacity(0.0), CoolingCombinationRatio(1.0), VRFCondPLR(0.0), VRFCondRTF(0.0),
              VRFCondCyclingRatio(0.0), CondenserInletTemp(0.0), CoolingCOP(0.0), OperatingCoolingCOP(0.0), RatedCoolingPower(0.0),
              HeatingCapacity(0.0), HeatingCapacitySizeRatio(1.0), LockHeatingCapacity(false), TotalHeatingCapacity(0.0),
              HeatingCombinationRatio(1.0), HeatingCOP(0.0), OperatingHeatingCOP(0.0), RatedHeatingPower(0.0), MinOATCooling(0.0), MaxOATCooling(0.0),
              MinOATHeating(0.0), MaxOATHeating(0.0), CoolCapFT(0), CoolEIRFT(0), HeatCapFT(0), HeatEIRFT(0), CoolBoundaryCurvePtr(0),
              HeatBoundaryCurvePtr(0), EIRCoolBoundaryCurvePtr(0), CoolEIRFPLR1(0), CoolEIRFPLR2(0), CoolCapFTHi(0), CoolEIRFTHi(0), HeatCapFTHi(0),
              HeatEIRFTHi(0), EIRHeatBoundaryCurvePtr(0), HeatEIRFPLR1(0), HeatEIRFPLR2(0), CoolPLFFPLR(0), HeatPLFFPLR(0), MinPLR(0.0),
              MasterZonePtr(0), MasterZoneTUIndex(0), ThermostatPriority(ThermostatCtrlType::Invalid), ZoneTUListPtr(0), HeatRecoveryUsed(false),
              VertPipeLngth(0.0), PCFLengthCoolPtr(0), PCFHeightCool(0.0), EquivPipeLngthCool(0.0), PipingCorrectionCooling(1.0), PCFLengthHeatPtr(0),
              PCFHeightHeat(0.0), EquivPipeLngthHeat(0.0), PipingCorrectionHeating(1.0), CCHeaterPower(0.0), CompressorSizeRatio(0.0),
              NumCompressors(0), MaxOATCCHeater(0.0), DefrostEIRPtr(0), DefrostFraction(0.0), DefrostStrategy(StandardRatings::DefrostStrat::Invalid),
              DefrostControl(StandardRatings::HPdefrostControl::Invalid), DefrostCapacity(0.0), DefrostPower(0.0), DefrostConsumption(0.0),
              MaxOATDefrost(0.0), CondenserType(DataHeatBalance::RefrigCondenserType::Invalid), CondenserNodeNum(0), SkipCondenserNodeNumCheck(false),
              CondenserOutletNodeNum(0), WaterCondVolFlowRate(0.0), EvapCondEffectiveness(0.0), EvapCondAirVolFlowRate(0.0), EvapCondPumpPower(0.0),
              CoolCombRatioPTR(0), HeatCombRatioPTR(0), OperatingMode(0), ElecPower(0.0), ElecCoolingPower(0.0), ElecHeatingPower(0.0),
              CoolElecConsumption(0.0), HeatElecConsumption(0.0), CrankCaseHeaterPower(0.0), CrankCaseHeaterElecConsumption(0.0),
              EvapCondPumpElecPower(0.0), EvapCondPumpElecConsumption(0.0), EvapWaterConsumpRate(0.0), SUMultiplier(0.0), TUCoolingLoad(0.0),
              TUHeatingLoad(0.0), SwitchedMode(false), OperatingCOP(0.0), MinOATHeatRecovery(0.0), MaxOATHeatRecovery(0.0), HRCAPFTCool(0),
              HRCAPFTCoolConst(0.9), HRInitialCoolCapFrac(0.5), HRCoolCapTC(0.15), HREIRFTCool(0), HREIRFTCoolConst(1.1), HRInitialCoolEIRFrac(1.0),
              HRCoolEIRTC(0.0), HRCAPFTHeat(0), HRCAPFTHeatConst(1.1), HRInitialHeatCapFrac(1.0), HRHeatCapTC(0.0), HREIRFTHeat(0),
              HREIRFTHeatConst(1.1), HRInitialHeatEIRFrac(1.0), HRHeatEIRTC(0.0), HRCoolingActive(false), HRHeatingActive(false), ModeChange(false),
              HRModeChange(false), HRTimer(0.0), HRTime(0.0), EvapWaterSupplyMode(EvapWaterSupply::FromMains), EvapWaterSupTankID(0),
              EvapWaterTankDemandARRID(0), CondensateTankID(0), CondensateTankSupplyARRID(0), CondensateVdot(0.0), CondensateVol(0.0),
              BasinHeaterPowerFTempDiff(0.0), BasinHeaterSetPointTemp(0.0), BasinHeaterPower(0.0), BasinHeaterConsumption(0.0),
              EMSOverrideHPOperatingMode(false), EMSValueForHPOperatingMode(0.0), HPOperatingModeErrorIndex(0), VRFHeatRec(0.0),
              VRFHeatEnergyRec(0.0), AlgorithmIUCtrl(1), CondensingTemp(44.0), CondTempFixed(0.0), CoffEvapCap(1.0), CompActSpeed(0.0),
              CompMaxDeltaP(0.0), C1Te(0.0), C2Te(0.0), C3Te(0.0), C1Tc(0.0), C2Tc(0.0), C3Tc(0.0), DiffOUTeTo(5), EffCompInverter(0.95),
              EvaporatingTemp(6.0), EvapTempFixed(0.0), HROUHexRatio(0.0), IUEvaporatingTemp(6.0), IUCondensingTemp(44.0), IUEvapTempLow(4.0),
              IUEvapTempHigh(15.0), IUCondTempLow(42.0), IUCondTempHigh(46.0), IUCondHeatRate(0.0), IUEvapHeatRate(0.0), Ncomp(0.0),
              NcompCooling(0.0), NcompHeating(0.0), OUEvapTempLow(-30.0), OUEvapTempHigh(20.0), OUCondTempLow(30.0), OUCondTempHigh(96.0),
              OUAirFlowRate(0.0), OUAirFlowRatePerCapcity(0.0), OUCondHeatRate(0.0), OUEvapHeatRate(0.0), OUFanPower(0.0), RatedEvapCapacity(40000.0),
              RatedHeatCapacity(0.0), RatedCompPower(14000.0), RatedCompPowerPerCapcity(0.35), RatedOUFanPower(0.0), RatedOUFanPowerPerCapcity(0.0),
              RateBFOUEvap(0.45581), RateBFOUCond(0.21900), RefPipDiaSuc(0.0), RefPipDiaDis(0.0), RefPipLen(0.0), RefPipEquLen(0.0), RefPipHei(0.0),
              RefPipInsThi(0.0), RefPipInsCon(0.0), SH(0.0), SC(0.0), SCHE(0.0), SHLow(0.0), SCLow(0.0), SHHigh(0.0), SCHigh(0.0),
              VRFOperationSimPath(0.0), checkPlantCondTypeOneTime(true), CondenserCapErrIdx(0), adjustedTe(false)
        {
        }

        // Begin of Methods for New VRF Model: Fluid Temperature Control
        //******************************************************************************

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void SizeVRFCondenser(EnergyPlusData &state);

        void CalcVRFCondenser_FluidTCtrl(EnergyPlusData &state, const bool FirstHVACIteration);

        void CalcVRFIUTeTc_FluidTCtrl(EnergyPlusData &state);

        void VRFOU_TeTc(EnergyPlusData &state,
                        HXOpMode OperationMode, // Flag for hex operation
                        Real64 Q_coil,          // // OU coil heat release at cooling mode or heat extract at heating mode [W]
                        Real64 SHSC,            // SH at cooling or SC at heating [C]
                        Real64 m_air,           // OU coil air mass flow rate [kg/s]
                        Real64 T_coil_in,       // Temperature of air at OU coil inlet [C]
                        Real64 W_coil_in,       // Humidity ratio of air at OU coil inlet [kg/kg]
                        Real64 OutdoorPressure, // Outdoor air pressure (Pa)
                        Real64 &T_coil_surf,    // Air temperature at coil surface [C]
                        Real64 &TeTc            // VRF Tc at cooling mode, or Te at heating mode [C]
        );

        Real64 VRFOU_FlowRate(EnergyPlusData &state,
                              HXOpMode OperationMode, // Flag for hex operation
                              Real64 TeTc,            // VRF Tc at cooling mode, or Te at heating mode [C]
                              Real64 SHSC,            // SC for OU condenser or SH for OU evaporator [C]
                              Real64 Q_coil,          // absolute value of OU coil heat release or heat extract [W]
                              Real64 T_coil_in,       // Temperature of air at OU coil inlet [C]
                              Real64 W_coil_in        // Humidity ratio of air at OU coil inlet [kg/kg]
        ) const;

        Real64 VRFOU_Cap(EnergyPlusData &state,
                         HXOpMode OperationMode, // Flag for hex operation
                         Real64 TeTc,            // VRF Tc at cooling mode, or Te at heating mode [C]
                         Real64 SHSC,            // SC for OU condenser or SH for OU evaporator [C]
                         Real64 m_air,           // OU coil air mass flow rate [kg/s]
                         Real64 T_coil_in,       // Temperature of air at OU coil inlet [C]
                         Real64 W_coil_in        // Humidity ratio of air at OU coil inlet [kg/kg]
        );

        Real64 VRFOU_SCSH(EnergyPlusData &state,
                          HXOpMode OperationMode, // Mode 0 for running as evaporator, 1 for condenser
                          Real64 Q_coil,          // // OU coil heat release at cooling mode or heat extract at heating mode [W]
                          Real64 TeTc,            // VRF Tc at cooling mode, or Te at heating mode [C]
                          Real64 m_air,           // OU coil air mass flow rate [kg/s]
                          Real64 T_coil_in,       // Temperature of air at OU coil inlet [C]
                          Real64 W_coil_in,       // Humidity ratio of air at OU coil inlet [kg/kg]
                          Real64 OutdoorPressure  // Outdoor air pressure [Pa]
        ) const;

        Real64 VRFOU_CapModFactor(EnergyPlusData &state,
                                  Real64 h_comp_in_real, // Enthalpy of refrigerant at the compressor inlet at real conditions [kJ/kg]
                                  Real64 h_evap_in_real, // Enthalpy of refrigerant at the evaporator inlet at real conditions [kJ/kg]
                                  Real64 P_evap_real,    // Evaporative pressure at real conditions [Pa]
                                  Real64 T_comp_in_real, // Temperature of the refrigerant at the compressor inlet at real conditions [C]
                                  Real64 T_comp_in_rate, // Temperature of the refrigerant at the compressor inlet at rated conditions [C]
                                  Real64 T_cond_out_rate // Temperature of the refrigerant at the condenser outlet at rated conditions [C]
        );

        void VRFOU_TeModification(EnergyPlusData &state,
                                  Real64 Te_up,          // Upper bound of Te during iteration, i.e., Te before reduction [C]
                                  Real64 Te_low,         // Lower bound of Te during iteration, i.e., the given suction temperature Te' [C]
                                  Real64 Pipe_h_IU_in,   // Piping Loss Algorithm Parameter: enthalpy of IU at inlet [kJ/kg]
                                  Real64 OutdoorDryBulb, // outdoor dry-bulb temperature [C]
                                  Real64 &Te_update,     // Updated Te that can generate the required Tsuction [C]
                                  Real64 &Pe_update,     // Piping Loss Algorithm Parameter: evaporating pressure assumed for iterations [Pa]
                                  Real64 &Pipe_m_ref,    // Piping Loss Algorithm Parameter: Refrigerant mass flow rate [kg/s]
                                  Real64 &Pipe_h_IU_out, // Piping Loss Algorithm Parameter: enthalpy of IU at outlet [kJ/kg]
                                  Real64 &Pipe_SH_merged // Piping Loss Algorithm Parameter: Average SH after the indoor units [C]
        );

        void VRFOU_CalcCompC(EnergyPlusData &state,
                             Real64 TU_load,            // Indoor unit cooling load [W]
                             Real64 T_suction,          // Compressor suction temperature Te' [C]
                             Real64 T_discharge,        // Compressor discharge temperature Tc' [C]
                             Real64 P_suction,          // Compressor suction pressure Pe' [Pa]
                             Real64 Pipe_T_comp_in,     // Refrigerant temperature at compressor inlet (after piping loss) [C]
                             Real64 Pipe_h_comp_in,     // Enthalpy after piping loss (compressor inlet) [kJ/kg]
                             Real64 Pipe_h_IU_in,       // Enthalpy of IU at inlet [kJ/kg]
                             Real64 Pipe_Q,             // Piping Loss Algorithm Parameter: Heat loss [W]
                             Real64 MaxOutdoorUnitTc,   // The maximum temperature that Tc can be at heating mode [C]
                             Real64 &OUCondHeatRelease, // Condenser heat release (cooling mode) [W]
                             Real64 &CompSpdActual,     // Actual compressor running speed [rps]
                             Real64 &t_Ncomp,           // Compressor power [W]
                             Real64 &CyclingRatio       // Cycling Ratio [W]
        );

        void
        VRFOU_CalcCompH(EnergyPlusData &state,
                        Real64 TU_load,            // Indoor unit cooling load [W]
                        Real64 T_suction,          // Compressor suction temperature Te' [C]
                        Real64 T_discharge,        // Compressor discharge temperature Tc' [C]
                        Real64 Pipe_h_out_ave,     // Average Enthalpy of the refrigerant leaving IUs [kJ/kg]
                        Real64 IUMaxCondTemp,      // VRV IU condensing temperature, max among all indoor units [C]
                        Real64 MinOutdoorUnitTe,   // The minimum temperature that Te can be at cooling mode (only used for calculating Min capacity)
                        Real64 Tfs,                // Temperature of the air at the coil surface [C]]
                        Real64 Pipe_Q,             // Piping Loss Algorithm Parameter: Heat loss [W]
                        Real64 &OUEvapHeatExtract, // Condenser heat release (cooling mode) [W]
                        Real64 &CompSpdActual,     // Actual compressor running speed [rps]
                        Real64 &t_Ncomp,           // Compressor power [W]
                        Real64 &CyclingRatio       // Compressor cycling ratio
        );

        void VRFHR_OU_HR_Mode(EnergyPlusData &state,
                              Real64 h_IU_evap_in,   // enthalpy of IU evaporator at inlet [kJ/kg]
                              Real64 h_comp_out,     // enthalpy of refrigerant at compressor outlet [kJ/kg]
                              Real64 Q_c_TU_PL,      // IU evaporator load, including piping loss [W]
                              Real64 Q_h_TU_PL,      // IU condenser load, including piping loss [W]
                              Real64 Tdischarge,     // VRF Compressor discharge refrigerant temperature [C]
                              Real64 &Tsuction,      // VRF compressor suction refrigerant temperature [C]
                              Real64 &Te_update,     // updated evaporating temperature, only updated when Tsuction is updated [C]
                              Real64 &h_comp_in,     // enthalpy of refrigerant at compressor inlet [kJ/kg]
                              Real64 &h_IU_PLc_out,  // enthalpy of refrigerant at the outlet of IU evaporator side main pipe [kJ/kg]
                              Real64 &Pipe_Q_c,      // IU evaporator side piping loss [W]
                              Real64 &Q_c_OU,        // OU evaporator load [W]
                              Real64 &Q_h_OU,        // OU condenser load [W]
                              Real64 &m_ref_IU_evap, // mass flow rate of Refrigerant through IU evaporators [kg/s]
                              Real64 &m_ref_OU_evap, // mass flow rate of Refrigerant through OU evaporator [kg/s]
                              Real64 &m_ref_OU_cond, // mass flow rate of Refrigerant through OU condenser [kg/s]
                              Real64 &N_fan_OU,      // outdoor unit fan power [W]
                              Real64 &CompSpdActual, // Actual compressor running speed [rps]
                              Real64 &t_Ncomp        // compressor power [W]
        );

        void VRFOU_CompSpd(EnergyPlusData &state,
                           Real64 Q_req,         // Required capacity [W]
                           HXOpMode Q_type,      // Required capacity type: 0 for evaporator, 1 for condenser
                           Real64 T_suction,     // Compressor suction temperature Te' [C]
                           Real64 T_discharge,   // Compressor discharge temperature Tc' [C]
                           Real64 h_IU_evap_in,  // Enthalpy of IU at inlet, for C_cap_operation calculation [kJ/kg]
                           Real64 h_comp_in,     // Enthalpy after piping loss (compressor inlet), for C_cap_operation calculation [kJ/kg]
                           Real64 &CompSpdActual // Actual compressor running speed [rps]
        );

        void VRFOU_CompCap(EnergyPlusData &state,
                           Real64 CompSpdActual, // Given compressor speed
                           Real64 T_suction,     // Compressor suction temperature Te' [C]
                           Real64 T_discharge,   // Compressor discharge temperature Tc' [C]
                           Real64 h_IU_evap_in,  // Enthalpy of IU at inlet, for C_cap_operation calculation [kJ/kg]
                           Real64 h_comp_in,     // Enthalpy after piping loss (compressor inlet), for C_cap_operation calculation [kJ/kg]
                           Real64 &Q_c_tot,      // Compressor evaporative capacity [W]
                           Real64 &t_Ncomp       // Compressor power [W]
        );

        void VRFOU_PipeLossC(EnergyPlusData &state,
                             Real64 Pipe_m_ref,     // Refrigerant mass flow rate [kg/s]
                             Real64 Pevap,          // VRF evaporating pressure [Pa]
                             Real64 Pipe_h_IU_out,  // Enthalpy of IU at outlet [kJ/kg]
                             Real64 Pipe_SH_merged, // Average super heating degrees after the indoor units [C]
                             Real64 OutdoorDryBulb, // outdoor dry-bulb temperature (C)
                             Real64 &Pipe_Q,        // unit part load ratio
                             Real64 &Pipe_DeltP,    // ratio of compressor ON airflow to AVERAGE airflow over timestep
                             Real64 &Pipe_h_comp_in // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
        );

        void VRFOU_PipeLossH(EnergyPlusData &state,
                             Real64 Pipe_m_ref,      // Refrigerant mass flow rate [kg/s]
                             Real64 Pcond,           // VRF condensing pressure [Pa]
                             Real64 Pipe_h_IU_in,    // Enthalpy of IU at outlet [kJ/kg]
                             Real64 OutdoorDryBulb,  // outdoor dry-bulb temperature (C)
                             Real64 &Pipe_Q,         // unit part load ratio
                             Real64 &Pipe_DeltP,     // ratio of compressor ON airflow to AVERAGE airflow over timestep
                             Real64 &Pipe_h_comp_out // Piping Loss Algorithm Parameter: Enthalpy before piping loss (compressor outlet) [kJ/kg]
        ) const;

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    struct TerminalUnitListData
    {
        // Members
        std::string Name;                                  // Name of the VRF Terminal Unit List
        int NumTUInList;                                   // Number of VRF Terminal Units in List
        bool reset_isSimulatedFlags;                       // used to align simulate flags with order of each TU in simulation
        Array1D_int ZoneTUPtr;                             // index to VRF Terminal Unit
        Array1D_string ZoneTUName;                         // Name of the VRF Terminal Unit
        Array1D_bool IsSimulated;                          // TRUE if TU has been simulated
        Array1D<Real64> TotalCoolLoad;                     // Total zone cooling coil load met by TU
        Array1D<Real64> TotalHeatLoad;                     // Total zone heating coil load met by TU
        Array1D_bool CoolingCoilPresent;                   // FALSE if coil not present
        Array1D_bool HeatingCoilPresent;                   // FALSE if coil not present
        Array1D_bool SuppHeatingCoilPresent;               // FALSE if supplemental heating coil not present
        Array1D_bool TerminalUnitNotSizedYet;              // TRUE if terminal unit not sized
        Array1D_bool HRHeatRequest;                        // defines a heating load on VRFTerminalUnits when QZnReq < 0
        Array1D_bool HRCoolRequest;                        // defines a cooling load on VRFTerminalUnits when QZnReq > 0
        Array1D_bool CoolingCoilAvailable;                 // cooling coil availability scheduled on
        Array1D_bool HeatingCoilAvailable;                 // cooling coil availability scheduled on
        Array1D<Sched::Schedule *> coolingCoilAvailScheds; // cooling coil availability schedule index
        Array1D<Sched::Schedule *> heatingCoilAvailScheds; // heating coil availability schedule index

        // Default Constructor
        TerminalUnitListData() : NumTUInList(0), reset_isSimulatedFlags(true)
        {
        }
    };

    struct VRFTerminalUnitEquipment
    {
        // Members
        std::string Name;                                          // Name of the VRF Terminal Unit
        TUType type = TUType::Invalid;                             // DataHVACGlobals VRF Terminal Unit type
        Sched::Schedule *availSched = nullptr;                     // Pointer to the correct schedule // LOL // avail?
        int VRFSysNum = 0;                                         // index to VRF Condenser
        int TUListIndex = 0;                                       // index to VRF Terminal Unit List
        int IndexToTUInTUList = 0;                                 // index to TU in VRF Terminal Unit List
        int ZoneNum = 0;                                           // index to zone where VRF Terminal Unit resides
        int ZoneAirNode = 0;                                       // zone air node number
        int VRFTUInletNodeNum = 0;                                 // VRF Terminal Unit inlet node number
        int VRFTUOutletNodeNum = 0;                                // VRF Terminal Unit outlet node number
        int VRFTUOAMixerOANodeNum = 0;                             // OA node number for this TU's OA mixer
        int VRFTUOAMixerRelNodeNum = 0;                            // Relief node number for this TU's OA mixer
        int VRFTUOAMixerRetNodeNum = 0;                            // Return node number for this TU's OA mixer
        int VRFTUOAMixerMixedNodeNum = 0;                          // Mixed node number for this TU's OA mixer
        Real64 MaxCoolAirVolFlow = 0.0;                            // supply air volumetric flow rate during cooling operation [m3/s]
        Real64 MaxHeatAirVolFlow = 0.0;                            // supply air volumetric flow rate during heating operation [m3/s]
        Real64 MaxNoCoolAirVolFlow = 0.0;                          // supply air volumetric flow rate when no cooling [m3/s]
        Real64 MaxNoHeatAirVolFlow = 0.0;                          // supply air volumetric flow rate when no heating [m3/s]
        Real64 MaxCoolAirMassFlow = 0.0;                           // supply air mass flow rate during cooling operation [kg/s]
        Real64 MaxHeatAirMassFlow = 0.0;                           // supply air mass flow rate during heating operation [kg/s]
        Real64 MaxNoCoolAirMassFlow = 0.0;                         // supply air mass flow rate when no cooling [kg/s]
        Real64 MaxNoHeatAirMassFlow = 0.0;                         // supply air mass flow rate when no heating [kg/s]
        Real64 CoolOutAirVolFlow = 0.0;                            // OA volumetric flow rate during cooling operation [m3/s]
        Real64 HeatOutAirVolFlow = 0.0;                            // OA volumetric flow rate during heating operation [m3/s]
        Real64 NoCoolHeatOutAirVolFlow = 0.0;                      // OA volumetric flow rate when no cooling or heating [m3/s]
        Real64 CoolOutAirMassFlow = 0.0;                           // OA mass flow rate during cooling operation [kg/s]
        Real64 HeatOutAirMassFlow = 0.0;                           // OA mass flow rate during heating operation [kg/s]
        Real64 NoCoolHeatOutAirMassFlow = 0.0;                     // OA mass flow rate when no cooling or heating [kg/s]
        Real64 MinOperatingPLR = 1.0E-20;                          // minimum part-load ratio for operating of fan/coil
        Real64 SuppHeatCoilFluidMaxFlow = 0.0;                     // supplemental heating coil fluid (hot water or steam) maximum flow rate [kg/s]
        Real64 DesignSuppHeatingCapacity = 0.0;                    // supplemental heating coil design capacity  [W]
        Real64 MaxSATFromSuppHeatCoil = 0.0;                       // maximum supply air temperature from supplemental heating coil [C]
        Real64 MaxOATSuppHeatingCoil = 0.0;                        // maximum outdoor dry-bulb temperature for supplemental heating coil [C]
        Real64 SuppHeatPartLoadRatio = 0.0;                        // supplemental heating coil part load ratio
        Real64 SuppHeatingCoilLoad = 0.0;                          // supplemental heating coil heating load
        HVAC::FanType fanType = HVAC::FanType::Invalid;            // index to fan type
        Sched::Schedule *fanOpModeSched = nullptr;                 // Pointer to the correct fan operating mode schedule
        Sched::Schedule *fanAvailSched = nullptr;                  // Pointer to the correct fan availability schedule
        int FanIndex = 0;                                          // Index to fan object
        Real64 FanPower = 0.0;                                     // power reported by fan component
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid;                  // operation mode: 1 = cycling fan, cycling coil 2 = constant fan, cycling coil
        HVAC::FanPlace fanPlace = HVAC::FanPlace::Invalid;         // fan placement; 1=blow through, 2=draw through
        Real64 ActualFanVolFlowRate = 0.0;                         // volumetric flow rate from fan object
        std::string SuppHeatCoilType;                              // type of supplemental heating coil
        std::string SuppHeatCoilName;                              // name of supplemental heating coil
        std::string OAMixerName;                                   // name of outside air mixer
        int OAMixerIndex = 0;                                      // index to outside air mixer
        bool OAMixerUsed = false;                                  // true if OA Mixer object is used
        int CoolCoilIndex = 0;                                     // index to terminal unit cooling coil
        int HeatCoilIndex = 0;                                     // index to terminal unit heating coil
        int SuppHeatCoilIndex = 0;                                 // index to terminal unit supplemental heating coil
        HVAC::CoilType coolCoilType = HVAC::CoilType::Invalid;     // type of VRF cooling coil
        HVAC::CoilType heatCoilType = HVAC::CoilType::Invalid;     // type of VRF heating coil
        HVAC::CoilType suppHeatCoilType = HVAC::CoilType::Invalid; // type of VRF supplemental heating coil
        Real64 ParasiticElec = 0.0;                                // parasitic electric for VRF terminal unit
        Real64 ParasiticOffElec = 0.0;                             // parasitic electric for VRF terminal unit when off
        Real64 HeatingSpeedRatio = 1.0;                            // Fan speed ratio in heating mode
        Real64 NoHeatingSpeedRatio = 0.0;                          // Fan speed ratio when no heating
        Real64 HeatingCapacitySizeRatio = 1.0;                     // Ratio of heating to cooling when autosizing
        Real64 CoolingSpeedRatio = 1.0;                            // Fan speed ratio in cooling mode
        Real64 NoCoolingSpeedRatio = 1.0;                          // Fan speed ratio when no cooling
        Real64 ParasiticCoolElecPower = 0.0;                       // Terminal unit cooling parasitic electric power [W]
        Real64 ParasiticHeatElecPower = 0.0;                       // Terminal unit heating parasitic electric power [W]
        Real64 ParasiticElecCoolConsumption = 0.0;                 // Terminal unit parasitic electric consumption in cooling [J]
        Real64 ParasiticElecHeatConsumption = 0.0;                 // Terminal unit parasitic electric consumption in heating [J]
        bool CoolingCoilPresent = true;                            // FALSE if coil not present
        bool HeatingCoilPresent = true;                            // FALSE if coil not present
        bool SuppHeatingCoilPresent = false;                       // FALSE if coil not present
        std::string AvailManagerListName;                          // Name of an availability manager list object
        Avail::Status availStatus = Avail::Status::NoAction;
        Real64 TerminalUnitSensibleRate = 0.0;                  // sensible cooling/heating rate of VRF terminal unit (W)
        Real64 TerminalUnitLatentRate = 0.0;                    // latent dehumidification/humidification rate of VRF terminal unit (W)
        Real64 TotalCoolingRate = 0.0;                          // report variable for total cooling rate (W)
        Real64 TotalHeatingRate = 0.0;                          // report variable for total heating rate (W)
        Real64 SensibleCoolingRate = 0.0;                       // report variable for sensible cooling rate (W)
        Real64 SensibleHeatingRate = 0.0;                       // report variable for sensible heating rate (W)
        Real64 LatentCoolingRate = 0.0;                         // report variable for latent cooling rate (W)
        Real64 LatentHeatingRate = 0.0;                         // report variable for latent heating rate (W)
        Real64 TotalCoolingEnergy = 0.0;                        // report variable for total cooling energy (J)
        Real64 TotalHeatingEnergy = 0.0;                        // report variable for total heating energy (J)
        Real64 SensibleCoolingEnergy = 0.0;                     // report variable for sensible cooling energy (J)
        Real64 SensibleHeatingEnergy = 0.0;                     // report variable for sensible heating energy (J)
        Real64 LatentCoolingEnergy = 0.0;                       // report variable for latent cooling energy (J)
        Real64 LatentHeatingEnergy = 0.0;                       // report variable for latent heating energy (J)
        bool EMSOverridePartLoadFrac = false;                   // User defined EMS function
        Real64 EMSValueForPartLoadFrac = 0.0;                   // user defined value for EMS function
        int IterLimitExceeded = 0;                              // index used for warning messages
        int FirstIterfailed = 0;                                // index used for warning messages
        int HVACSizingIndex = 0;                                // index of a HVACSizing object for a VRF terminal
        bool ATMixerExists = false;                             // True if there is an ATMixer
        std::string ATMixerName;                                // name of air terminal mixer
        int ATMixerIndex = 0;                                   // index to the air terminal mixer
        HVAC::MixerType ATMixerType = HVAC::MixerType::Invalid; // 1 = inlet side mixer, 2 = supply side mixer
        int ATMixerPriNode = 0;                                 // primary inlet air node number for the air terminal mixer
        int ATMixerSecNode = 0;                                 // secondary air inlet node number for the air terminal mixer
        int ATMixerOutNode = 0;                                 // outlet air node number for the air terminal mixer
        int SuppHeatCoilAirInletNode = 0;                       // supplemental heating coil air inlet node
        int SuppHeatCoilAirOutletNode = 0;                      // supplemental heating coil air outlet node
        int SuppHeatCoilFluidInletNode = 0;                     // supplemental heating coil fluid inlet node
        int SuppHeatCoilFluidOutletNode = 0;                    // supplemental heating coil fluid outlet node
        bool firstPass = true;                                  // used to reset global sizing data
        PlantLocation SuppHeatCoilPlantLoc{};                   // supplemental heating coil plant component index
        Real64 coilInNodeT = 0.0;                               // coil inlet node temp at full flow (C)
        Real64 coilInNodeW = 0.0;                               // coil inlet node humidity ratio at full flow (kg/kg)
        int fanInletNode = 0;                                   // fan inlet node index
        int fanOutletNode = 0;                                  // fan outlet node index
        bool MySuppCoilPlantScanFlag = true;                    // flag used to initialize plant comp for water and steam heating coils
        int airLoopNum = 0;                                     // index to air loop
        bool isInOASys = false;                                 // true if TU is configured in outside air system
        bool isInAirLoop = false;                               // true if TU is configured in an air loop
        bool isInZone = false;                                  // true if TU is configured as zone equipment
        bool isSetPointControlled = false;                      // TU is controlled via setpoint instead of the standard load control
        bool coolSPActive = false;                              // set point controlled cooling coil active (needs to operate)
        bool heatSPActive = false;                              // set point controlled heating coil active (needs to operate)
        Real64 coolLoadToSP = 0.0;                              // load to set point in cooling mode
        Real64 heatLoadToSP = 0.0;                              // load to set point in heating mode
        Real64 coilTempSetPoint = 0.0;                          // coil control temperature
        Real64 suppTempSetPoint = 0.0;                          // supplemental heating coil control temperature
        Real64 controlZoneMassFlowFrac = 1.0;                   // ratio of control zone air mass flow rate to total zone air mass flow rate
        int zoneSequenceCoolingNum = 0;                         // zone equipment cooling sequence
        int zoneSequenceHeatingNum = 0;                         // zone equipment heating sequence
        int coolCoilAirInNode = 0;                              // cooling coil air inlet node number
        int coolCoilAirOutNode = 0;                             // cooling coil air outlet node number
        int heatCoilAirInNode = 0;                              // heating coil air inlet node number
        int heatCoilAirOutNode = 0;                             // heating coil air outlet node number
        std::string DesignSpecMultispeedHPType;                 // Multiuple performance object type
        std::string DesignSpecMultispeedHPName;                 // Multiuple performance object name
        int DesignSpecMSHPIndex = -1;                           //  Multiuple performance index
        int NumOfSpeedHeating = 0;                              // Number of heating speed
        int NumOfSpeedCooling = 0;                              // Number of cooling speed
        int HeatSpeedNum = 0;                                   // Heating speed number
        int CoolSpeedNum = 0;                                   // Cooling speed number
        std::vector<Real64> CoolVolumeFlowRate;
        std::vector<Real64> CoolMassFlowRate;
        std::vector<Real64> HeatVolumeFlowRate;
        std::vector<Real64> HeatMassFlowRate;
        int SpeedNum = 0;
        Real64 SpeedRatio = 0.0;
        Real64 CycRatio = 0.0;

        // Methods for New VRF Model: Fluid Temperature Control
        //******************************************************************************
        // Note: the argument VRFTUNum should be removed later in the deeper OO re-factor. Now this argument may be used by other functions that are
        // not member functions of this class.

        void CalcVRFIUVariableTeTc(EnergyPlusData &state,
                                   Real64 &EvapTemp, // evaporating temperature
                                   Real64 &CondTemp  // condensing temperature
        );

        void ControlVRF_FluidTCtrl(EnergyPlusData &state,
                                   int VRFTUNum,              // Index to VRF terminal unit
                                   Real64 QZnReq,             // Index to zone number
                                   bool FirstHVACIteration,   // flag for 1st HVAC iteration in the time step
                                   Real64 &PartLoadRatio,     // unit part load ratio
                                   Real64 &OnOffAirFlowRatio, // ratio of compressor ON airflow to AVERAGE airflow over timestep
                                   Real64 &SuppHeatCoilLoad   // supplemental heating coil load (W)
        );

        void CalcVRF_FluidTCtrl(EnergyPlusData &state,
                                int VRFTUNum,                                     // Index to VRF terminal unit
                                bool FirstHVACIteration,                          // flag for 1st HVAC iteration in the time step
                                Real64 PartLoadRatio,                             // compressor part load fraction
                                Real64 &LoadMet,                                  // load met by unit (W)
                                Real64 &OnOffAirFlowRatio,                        // ratio of ON air flow to average air flow
                                Real64 &SuppHeatCoilLoad,                         // supplemental heating coil load (W)
                                ObjexxFCL::Optional<Real64> LatOutputProvided = _ // delivered latent capacity (W)
        );

        Real64 CalVRFTUAirFlowRate_FluidTCtrl(EnergyPlusData &state,
                                              int VRFTUNum,                            // Index to VRF terminal unit
                                              Real64 PartLoadRatio,                    // part load ratio of the coil
                                              bool FirstHVACIteration,                 // FirstHVACIteration flag
                                              ObjexxFCL::Optional<Real64 const> MaxCap // maximum allowed heating capacity
        );

        // Methods for cruve based VRF Model
        //******************************************************************************
        void ControlVRF(EnergyPlusData &state,
                        int VRFTUNum,              // Index to VRF terminal unit
                        Real64 QZnReq,             // Index to zone number
                        bool FirstHVACIteration,   // flag for 1st HVAC iteration in the time step
                        Real64 &PartLoadRatio,     // unit part load ratio
                        Real64 &OnOffAirFlowRatio, // ratio of compressor ON airflow to AVERAGE airflow over timestep
                        Real64 &SuppHeatCoilLoad   // supplemental heating coil load (W)
        );

        void ControlVRFToLoad(EnergyPlusData &state,
                              int const VRFTUNum,            // Index to VRF terminal unit
                              Real64 const QZnReq,           // Index to zone number
                              bool const FirstHVACIteration, // flag for 1st HVAC iteration in the time step
                              Real64 &PartLoadRatio,         // unit part load ratio
                              Real64 &OnOffAirFlowRatio,     // ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 &SuppHeatCoilLoad       // supplemental heating coil load (W)
        );

        void CalcVRF(EnergyPlusData &state,
                     int const VRFTUNum,                               // Unit index in VRF terminal unit array
                     bool const FirstHVACIteration,                    // flag for 1st HVAC iteration in the time step
                     Real64 const PartLoadRatio,                       // compressor part load fraction
                     Real64 &LoadMet,                                  // load met by unit (W)
                     Real64 &OnOffAirFlowRatio,                        // ratio of ON air flow to average air flow
                     Real64 &SuppHeatCoilLoad,                         // supplemental heating coil load (W)
                     ObjexxFCL::Optional<Real64> LatOutputProvided = _ // delivered latent capacity (W)
        );

        // Methods for curve based and refrigerant flow control based models
        //******************************************************************************
        void CalcVRFSuppHeatingCoil(EnergyPlusData &state,
                                    int VRFTUNum,            // index of vrf terminal unit
                                    bool FirstHVACIteration, // True when first HVAC iteration
                                    Real64 PartLoadRatio,    // coil operating part-load ratio
                                    Real64 &SuppCoilLoad     // adjusted supp coil load when outlet temp exceeds max (W)
        );

        static Real64 HotWaterHeatingCoilResidual(EnergyPlusData &state,
                                                  Real64 PartLoadFrac,           // water heating coil part-load ratio
                                                  std::vector<Real64> const &Par // par(1) = VRF TU Numberindex to current VRF terminal unit
        );

        static Real64
        HeatingCoilCapacityLimit(EnergyPlusData &state,
                                 Real64 const HeatCoilAirInletNode, // supplemental heating coil air inlet node
                                 Real64 const HeatCoilMaxSATAllowed // supplemental heating coil maximum supply air temperature allowed [C]
        );
    };

    struct VRFTUNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        VRFTUNumericFieldData() = default;
    };

    // Functions

    void SimulateVRF(EnergyPlusData &state,
                     std::string_view CompName,
                     bool const FirstHVACIteration,
                     int const ZoneNum,
                     int &CompIndex,
                     bool &HeatingActive,
                     bool &CoolingActive,
                     int const OAUnitNum,         // If the system is an equipment of OutdoorAirUnit
                     Real64 const OAUCoilOutTemp, // the coil inlet temperature of OutdoorAirUnit
                     bool const ZoneEquipment,    // TRUE if called as zone equipment
                     Real64 &SysOutputProvided,
                     Real64 &LatOutputProvided);

    void CalcVRFCondenser(EnergyPlusData &state, int VRFCond);

    void GetVRFInput(EnergyPlusData &state);

    void GetVRFInputData(EnergyPlusData &state, bool &ErrorsFound // flag for errors in GetInput
    );

    void CheckVRFTUNodeConnections(EnergyPlusData &state, int const VRFTUNum, bool &ErrorsFound);

    void InitVRF(EnergyPlusData &state, int VRFTUNum, int ZoneNum, bool FirstHVACIteration, Real64 &OnOffAirFlowRatio, Real64 &QZnReq);

    void SetCompFlowRate(EnergyPlusData &state, int VRFTUNum, int VRFCond, bool const UseCurrentMode = false);

    void SizeVRF(EnergyPlusData &state, int const VRFTUNum);

    void initCapSizingVars(EnergyPlusData &state,
                           int sizingMethod,
                           int capSizingMethod,
                           int &eqSizingMethod,
                           Real64 scaledCapacity,
                           bool &modeCapacity,
                           Real64 &designLoad,
                           bool &scalableCapSizingOn,
                           Real64 &fracOfAutosizedCapacity);

    void SimVRF(EnergyPlusData &state,
                int VRFTUNum,
                bool FirstHVACIteration,
                Real64 &OnOffAirFlowRatio,
                Real64 &SysOutputProvided,
                Real64 &LatOutputProvided,
                Real64 QZnReq);

    int GetVRFTUOutAirNode(EnergyPlusData &state, int VRFTUNum);

    int GetVRFTUZoneInletAirNode(EnergyPlusData &state, int VRFTUNum);

    int GetVRFTUMixedAirNode(EnergyPlusData &state, int VRFTUNum);

    int GetVRFTUOutAirNodeFromName(EnergyPlusData &state, std::string const &VRFTUName, bool &errorsFound);

    int GetVRFTUInAirNodeFromName(EnergyPlusData &state, std::string const &VRFTUName, bool &errorsFound);

    int GetVRFTUReturnAirNode(EnergyPlusData &state, int const VRFTUNum);

    void getVRFTUZoneLoad(
        EnergyPlusData &state, int const VRFTUNum, Real64 &zoneLoad, Real64 &LoadToHeatingSP, Real64 &LoadToCoolingSP, bool const InitFlag);

    bool getVRFTUNodeNumber(EnergyPlusData &state, int const nodeNumber);

    void ReportVRFTerminalUnit(EnergyPlusData &state, int VRFTUNum); // index to VRF terminal unit

    void ReportVRFCondenser(EnergyPlusData &state, int VRFCond); // index to VRF condensing unit

    void UpdateVRFCondenser(EnergyPlusData &state, int VRFCond); // index to VRF condensing unit

    void isVRFCoilPresent(EnergyPlusData &state, std::string_view VRFTUName, bool &CoolCoilPresent, bool &HeatCoilPresent);

    void SetAverageAirFlow(EnergyPlusData &state,
                           int VRFTUNum,             // Unit index
                           Real64 PartLoadRatio,     // unit part load ratio
                           Real64 &OnOffAirFlowRatio // ratio of compressor ON airflow to average airflow over timestep
    );

    void InitializeOperatingMode(EnergyPlusData &state,
                                 bool FirstHVACIteration,  // flag for first time through HVAC systems
                                 int VRFCond,              // Condenser Unit index
                                 int TUListNum,            // Condenser Unit terminal unit list
                                 Real64 &OnOffAirFlowRatio // ratio of on to off flow rate
    );

    void LimitTUCapacity(EnergyPlusData &state,
                         int VRFCond,                     // Condenser Unit index
                         int NumTUInList,                 // Number of terminal units in list
                         Real64 StartingCapacity,         // temporary variable holding condenser capacity [W]
                         const Array1D<Real64> &CapArray, // Array of coil capacities in either cooling or heating mode [W]
                         Real64 &MaxLimit,                // Maximum terminal unit capacity for coils in same operating mode [W]
                         Real64 AltCapacity,              // temporary variable holding heat recovery capacity [W]
                         const Array1D<Real64> &AltArray, // Array of coil capacities of heat recovery [W]
                         Real64 &AltLimit                 // Maximum terminal unit capacity of heat recovery coils [W]
    );

    void LimitCoilCapacity(int NumTUInList,                 // Number of terminal units in list
                           Real64 TotalCapacity,            // temporary variable holding condenser capacity [W]
                           const Array1D<Real64> &CapArray, // Array of coil capacities in either cooling or heating mode [W]
                           Real64 &MaxLimit                 // Maximum terminal unit capacity for coils in same operating mode [W]
    );

    Real64 CompResidual_FluidTCtrl(EnergyPlusData &state,
                                   Real64 T_dis,
                                   Real64 CondHeat,
                                   int CAPFT,
                                   Real64 const T_suc // Compressor suction temperature Te' [C]
    );

    int getEqIndex(EnergyPlusData &state, std::string_view VRFTUName);

} // namespace HVACVariableRefrigerantFlow

struct HVACVarRefFlowData : BaseGlobalStruct
{

    bool GetVRFInputFlag = true;             // Flag set to make sure you get input once
    bool MyOneTimeFlag = true;               // One time flag used to allocate MyEnvrnFlag and MySizeFlag
    bool MyOneTimeSizeFlag = true;           // One time flag used to allocate MyEnvrnFlag and MySizeFlag
    bool ZoneEquipmentListNotChecked = true; // False after the Zone Equipment List has been checked for items
    int NumVRFCond = 0;                      // total number of VRF condensers (All VRF Algorithm Types)
    int NumVRFCond_SysCurve = 0;             // total number of VRF condensers with VRF Algorithm Type 1
    int NumVRFCond_FluidTCtrl_HP = 0;        // total number of VRF condensers with VRF Algorithm Type 2 (HP)
    int NumVRFCond_FluidTCtrl_HR = 0;        // total number of VRF condensers with VRF Algorithm Type 2 (HR)
    int NumVRFTU = 0;                        // total number of VRF terminal units
    int NumVRFTULists = 0;                   // The number of VRF TU lists
    Real64 CompOnMassFlow = 0.0;             // Supply air mass flow rate w/ compressor ON
    Real64 OACompOnMassFlow = 0.0;           // OA mass flow rate w/ compressor ON
    Real64 CompOffMassFlow = 0.0;            // Supply air mass flow rate w/ compressor OFF
    Real64 OACompOffMassFlow = 0.0;          // OA mass flow rate w/ compressor OFF
    Real64 CompOnFlowRatio = 0.0;            // fan flow ratio when coil on
    Real64 CompOffFlowRatio = 0.0;           // fan flow ratio when coil off
    Real64 FanSpeedRatio = 0.0;              // ratio of air flow ratio passed to fan object
    Real64 LoopDXCoolCoilRTF = 0.0;          // holds value of DX cooling coil RTF
    Real64 LoopDXHeatCoilRTF = 0.0;          // holds value of DX heating coil RTF
    Real64 CondenserWaterMassFlowRate = 0.0; // VRF water-cooled condenser mass flow rate (kg/s)
    Real64 CurrentEndTimeLast = 0.0;         // end time of last time step
    Array1D_bool HeatingLoad;                // defines a heating load on VRFTerminalUnits
    Array1D_bool CoolingLoad;                // defines a cooling load on VRFTerminalUnits
    Array1D_bool LastModeHeating;            // defines last mode was heating mode
    Array1D_bool LastModeCooling;            // defines last mode was cooling mode
    Array1D_bool CheckEquipName;             // Flag set to check equipment connections once
    Array1D_bool MyEnvrnFlag;                // Flag for initializing at beginning of each new environment
    Array1D_bool MySizeFlag;                 // False after TU has been sized
    Array1D_bool MyBeginTimeStepFlag;        // Flag to sense beginning of time step
    Array1D_bool MyVRFFlag;                  // used for sizing VRF inputs one time
    Array1D_bool MyVRFCondFlag;              // used to reset timer counter
    Array1D_bool MyZoneEqFlag;               // used to set up zone equipment availability managers
    Array1D_int NumCoolingLoads;             // number of TU's requesting cooling
    Array1D_int NumHeatingLoads;             // number of TU's requesting heating
    Array1D<Real64> MaxCoolingCapacity;      // maximum capacity of any terminal unit
    Array1D<Real64> MaxHeatingCapacity;      // maximum capacity of any terminal unit
    Array1D<Real64> CoolCombinationRatio;    // ratio of terminal unit capacity to VRF condenser capacity
    Array1D<Real64> HeatCombinationRatio;    // ratio of terminal unit capacity to VRF condenser capacity
    Array1D<Real64> MaxDeltaT;               // maximum zone temperature difference from setpoint
    Array1D<Real64> MinDeltaT;               // minimum zone temperature difference from setpoint
    Array1D<Real64> SumCoolingLoads;         // sum of cooling loads
    Array1D<Real64> SumHeatingLoads;         // sum of heating loads
    Array1D_bool CheckVRFCombinationRatio;
    bool MyOneTimeEIOFlag = true; // eio header flag reporting
    int ATMixOutNode = 0;         // terminal unit mixer outlet node
    int ATMixOutNode2 = 0;        // terminal unit mixer outlet node

    // Object Data
    EPVector<HVACVariableRefrigerantFlow::VRFCondenserEquipment> VRF; // AirConditioner:VariableRefrigerantFlow object
    std::unordered_map<std::string, std::string> VrfUniqueNames;
    EPVector<HVACVariableRefrigerantFlow::VRFTerminalUnitEquipment> VRFTU;           // ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object
    EPVector<HVACVariableRefrigerantFlow::TerminalUnitListData> TerminalUnitList;    // zoneTerminalUnitList object
    EPVector<HVACVariableRefrigerantFlow::VRFTUNumericFieldData> VRFTUNumericFields; // holds VRF TU numeric input fields character field name

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetVRFInputFlag = true;
        this->MyOneTimeFlag = true;
        this->MyOneTimeSizeFlag = true;
        this->ZoneEquipmentListNotChecked = true;
        this->NumVRFCond = 0;
        this->NumVRFCond_SysCurve = 0;
        this->NumVRFCond_FluidTCtrl_HP = 0;
        this->NumVRFCond_FluidTCtrl_HR = 0;
        this->NumVRFTU = 0;
        this->NumVRFTULists = 0;
        this->CompOnMassFlow = 0.0;
        this->OACompOnMassFlow = 0.0;
        this->CompOffMassFlow = 0.0;
        this->OACompOffMassFlow = 0.0;
        this->CompOnFlowRatio = 0.0;
        this->CompOffFlowRatio = 0.0;
        this->FanSpeedRatio = 0.0;
        this->LoopDXCoolCoilRTF = 0.0;
        this->LoopDXHeatCoilRTF = 0.0;
        this->CondenserWaterMassFlowRate = 0.0;
        this->CurrentEndTimeLast = 0.0;
        this->HeatingLoad.deallocate();
        this->CoolingLoad.deallocate();
        this->LastModeHeating.deallocate();
        this->LastModeCooling.deallocate();
        this->CheckEquipName.deallocate();
        this->MyEnvrnFlag.deallocate();
        this->MySizeFlag.deallocate();
        this->MyBeginTimeStepFlag.deallocate();
        this->MyVRFFlag.deallocate();
        this->MyVRFCondFlag.deallocate();
        this->MyZoneEqFlag.deallocate();
        this->NumCoolingLoads.deallocate();
        this->NumHeatingLoads.deallocate();
        this->MaxCoolingCapacity.deallocate();
        this->MaxHeatingCapacity.deallocate();
        this->CoolCombinationRatio.deallocate();
        this->HeatCombinationRatio.deallocate();
        this->MaxDeltaT.deallocate();
        this->MinDeltaT.deallocate();
        this->SumCoolingLoads.deallocate();
        this->SumHeatingLoads.deallocate();
        this->VRF.deallocate();
        this->VrfUniqueNames.clear();
        this->VRFTU.deallocate();
        this->TerminalUnitList.deallocate();
        this->VRFTUNumericFields.deallocate();
        this->CheckVRFCombinationRatio.clear();
        this->MyOneTimeEIOFlag = true;
        this->ATMixOutNode = 0;
        this->ATMixOutNode2 = 0;
    }
};

} // namespace EnergyPlus

#endif
