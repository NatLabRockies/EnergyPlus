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

#ifndef DXCoils_hh_INCLUDED
#define DXCoils_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/StandardRatings.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DXCoils {

    // Using/Aliasing
    constexpr Real64 RatedInletAirTemp(26.6667);          // 26.6667C or 80F
    constexpr Real64 RatedInletWetBulbTemp(19.4444);      // 19.44 or 67F
    constexpr Real64 RatedInletAirHumRat(0.0111847);      // Humidity ratio corresponding to 80F dry bulb/67F wet bulb
    constexpr Real64 RatedOutdoorAirTemp(35.0);           // 35 C or 95F
    constexpr Real64 RatedInletAirTempHeat(21.1111);      // 21.11C or 70F
    constexpr Real64 RatedOutdoorAirTempHeat(8.3333);     // 8.33 C or 47F
    constexpr Real64 RatedOutdoorWetBulbTempHeat(6.1111); // 6.11 C or 43F
    constexpr Real64 RatedInletWetBulbTempHeat(15.5556);  // 15.55 or 60F
    constexpr Real64 DryCoilOutletHumRatioMin(0.00001);   // dry coil outlet minimum hum ratio kgWater/kgDryAir

    // Multimode DX Coil
    constexpr int MaxCapacityStages(2);                               // Maximum number of capacity stages supported
    constexpr int MaxDehumidModes(1);                                 // Maximum number of enhanced dehumidification modes supported
    constexpr int MaxModes(MaxCapacityStages *(MaxDehumidModes + 1)); // Maximum number of performance modes

    // Water Systems
    enum class CondensateCollectAction
    {
        Invalid = -1,
        Discard, // default mode where water is "lost"
        ToTank,  // collect coil condensate from air and store in water storage tank
        Num
    };

    enum class EvapWaterSupply
    {
        Invalid = -1,
        FromMains,
        FromTank,
        Num
    };

    // Types

    struct DXCoilData
    {
        // Members
        //          Some variables in this type are arrays (dimension=MaxModes) to support coil type
        //          COIL:DX:MultiMode:CoolingEmpirical.  Other coil types only use the first element.
        std::string Name;                      // Name of the DX Coil
        std::string DXCoilType;                // type of coil
        int DXCoilType_Num;                    // Integer equivalent to DXCoilType
        Sched::Schedule *availSched = nullptr; // availability schedule
        //          RatedCoolCap, RatedSHR and RatedCOP do not include the thermal or electrical
        //          effects due to the supply air fan
        Array1D<Real64> RatedTotCap;                      // Gross total cooling capacity at rated conditions [watts]
        Real64 HeatSizeRatio;                             // heat pump heating to cooling sizing ratio when autosized
        Array1D_bool RatedTotCapEMSOverrideOn;            // if true, then EMS is calling to override rated total capacity
        Array1D<Real64> RatedTotCapEMSOverrideValue;      // value to use for EMS override
        bool FrostHeatingCapacityMultiplierEMSOverrideOn; // if true, then EMS is calling to override multiplier for heating capacity when system is
                                                          // in defrost
        Real64 FrostHeatingCapacityMultiplierEMSOverrideValue; // value to use for EMS override
        bool FrostHeatingInputPowerMultiplierEMSOverrideOn; // if true, then EMS is calling to override multiplier for power when system is in defrost
        Real64 FrostHeatingInputPowerMultiplierEMSOverrideValue; // value to use for EMS override
        Array1D<Real64> RatedSHR;                                // Sensible heat ratio (sens cap/total cap) at rated conditions
        Array1D_bool RatedSHREMSOverrideOn;                      // if true, then EMS is calling to override Sensible heat ratio
        Array1D<Real64> RatedSHREMSOverrideValue;                // value to use for EMS override forSensible heat ratio
        Array1D<Real64> RatedCOP;                                // Coefficient of performance at rated conditions
        Array1D<Real64> RatedAirVolFlowRate;                     // Air volume flow rate through coil at rated conditions [m3/s]
        // This is adjusted for bypassed air if any (see BypassedFlowFrac)
        Array1D_bool RatedAirVolFlowRateEMSOverrideON;       // if true, then EMS is calling to override Air volume flow rate
        Array1D<Real64> RatedAirVolFlowRateEMSOverrideValue; // value to use for EMS override Air volume flow rate
        Array1D<Real64> FanPowerPerEvapAirFlowRate;          // Fan Power Per Air volume flow rate through the
        // Evaporator coil at rated conditions [W/(m3/s)]
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023;
        Array1D<Real64> RatedAirMassFlowRate; // Air mass flow rate through coil at rated conditions [kg/s]
        // This is adjusted for bypassed air if any (see BypassedFlowFrac)
        Array1D<Real64> BypassedFlowFrac; // Fraction of air flow bypassed around coil
        Array1D<Real64> RatedCBF;         // rated coil bypass factor, determined using RatedTotCap and RatedSHR
        int AirInNode;                    // Air inlet node number
        int AirOutNode;                   // Air outlet node number
        Array1D_int CCapFTemp;            // index of total cooling capacity modifier curve
        // (function of entering wetbulb, outside drybulb)
        int CCapFTempErrorIndex; // Used for warning messages when output of CCapFTemp is negative
        Array1D_int CCapFFlow;   // index of total cooling capacity modifier curve
        // (function of actual supply air flow vs rated air flow)
        int CCapFFlowErrorIndex; // Used for warning messages when output of CCapFFlow is negative
        Array1D_int EIRFTemp;    // index of energy input ratio modifier curve
        // (function of entering wetbulb, outside drybulb)
        int EIRFTempErrorIndex; // Used for warning messages when output of EIRFTemp is negative
        Array1D_int EIRFFlow;   // index of energy input ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int EIRFFlowErrorIndex;                // Used for warning messages when output of EIRFFlow is negative
        Array1D_int PLFFPLR;                   // index of part-load factor vs part-load ratio curve
        bool ReportCoolingCoilCrankcasePower;  // logical determines if the cooling coil crankcase heater power is reported
        Real64 CrankcaseHeaterCapacity;        // total crankcase heater capacity [W]
        Real64 CrankcaseHeaterPower;           // report variable for average crankcase heater power [W]
        Real64 MaxOATCrankcaseHeater;          // maximum OAT for crankcase heater operation [C]
        int CrankcaseHeaterCapacityCurveIndex; // Crankcase heater power-temperature curve or table index
        Real64 CrankcaseHeaterConsumption;     // report variable for total crankcase heater energy consumption [J]
        Real64 BasinHeaterPowerFTempDiff;      // Basin heater capacity per degree C below setpoint (W/C)
        Real64 BasinHeaterSetPointTemp;        // setpoint temperature for basin heater operation (C)
        int CompanionUpstreamDXCoil;           // index number of the DX coil that is "upstream" of this DX coil. Currently used for
        // UnitarySystem:HeatPump:AirToAir for proper calculation of crankcase heater energy
        // consumption
        bool FindCompanionUpStreamCoil;              // Flag to get the companion coil in Init.
        Array1D_int CondenserInletNodeNum;           // Node number of outdoor condenser(s) (actually an evaporator for heating coils)
        int LowOutletTempIndex;                      // used for low outlet temperature warnings
        Real64 FullLoadOutAirTempLast;               // used for low outlet temperature warnings
        Real64 FullLoadInletAirTempLast;             // used for low outlet temperature warnings
        bool PrintLowOutTempMessage;                 // used to print warning message for low outlet air dry-bulb conditions
        std::string LowOutTempBuffer1;               // holds warning message until next iteration (only prints 1 message/iteration)
        std::string LowOutTempBuffer2;               // holds warning message until next iteration (only prints 1 message/iteration)
        int HeatingCoilPLFCurvePTR;                  // PLF curve index to gas or electric heating coil (used in latent degradation model)
        Sched::Schedule *basinHeaterSched = nullptr; // Pointer to basin heater schedule
        // start of multi-speed compressor variables
        Real64 RatedTotCap2; // Gross total cooling capacity at rated conditions, low speed [watts]
        // Note: For HPWHs, RatedTotCap2   = Water Heating Capacity for Coil:DX:HPWH and
        //                  RatedTotCap(1) = Air Cooling Coil Capacity for Coil:DX:HPWH
        Real64 RatedSHR2;            // Sensible heat ratio (sens cap/total cap) at rated conditions, low speed
        Real64 RatedCOP2;            // Coefficient of performance at rated conditions, low speed
        Real64 RatedAirVolFlowRate2; // Air volume flow rate through unit at rated conditions, low speed [m3/s]

        Array1D<Real64> FanPowerPerEvapAirFlowRate_LowSpeed; // Fan Power Per Air volume flow rate through the
        // Evaporator coil at rated conditions [W/(m3/s)], low speed
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023_LowSpeed;

        Real64 RatedAirMassFlowRate2; // Air mass flow rate through unit at rated conditions, low speed [kg/s]
        Real64 RatedCBF2;             // rated coil bypass factor (low speed), determined using RatedTotCap2 and RatedSHR2
        int CCapFTemp2;               // index of total cooling capacity modifier curve (low speed)
        int EIRFTemp2;                // index of energy input ratio modifier curve (low speed)
        // (function of entering wetbulb, outside drybulb)
        Real64 RatedEIR2;                  // rated energy input ratio (low speed, inverse of COP2)
        Real64 InternalStaticPressureDrop; // for rating VAV system
        bool RateWithInternalStaticAndFanObject;
        int SupplyFanIndex;          // index of this fan in fan array or vector
        HVAC::FanType supplyFanType; // type of fan, in DataHVACGlobals
        std::string SupplyFanName;   // name of fan associated with this dx coil
        std::string CoilSystemName;
        // end of multi-speed compressor variables
        Array1D<Real64> RatedEIR; // rated energy input ratio (inverse of COP)
        Real64 InletAirMassFlowRate;
        Real64 InletAirMassFlowRateMax;
        Real64 InletAirTemp;
        Real64 InletAirHumRat;
        Real64 InletAirEnthalpy;
        //  Eventually inlet air conditions will be used in DX Coil, these lines are commented out and marked with this comment line
        //  REAL(r64) :: InletAirPressure       =0.0d0
        Real64 OutletAirTemp;
        Real64 OutletAirHumRat;
        Real64 OutletAirEnthalpy;
        Real64 PartLoadRatio; // Ratio of actual sensible cooling load to steady-state sensible cooling capacity
        Real64 TotalCoolingEnergy;
        Real64 SensCoolingEnergy;
        Real64 LatCoolingEnergy;
        Real64 TotalCoolingEnergyRate;
        Real64 SensCoolingEnergyRate;
        Real64 LatCoolingEnergyRate;
        Real64 ElecCoolingConsumption;
        Real64 ElecCoolingPower;
        Real64 CoolingCoilRuntimeFraction; // Run time fraction of the DX cooling unit
        // start of variables used in heat pump heating coils only
        Real64 TotalHeatingEnergy;
        Real64 TotalHeatingEnergyRate;
        Real64 ElecHeatingConsumption;
        Real64 ElecHeatingPower;
        Real64 HeatingCoilRuntimeFraction;                // Run time fraction of the DX heating unit
        StandardRatings::DefrostStrat DefrostStrategy;    // defrost strategy; 1=reverse-cycle, 2=resistive
        StandardRatings::HPdefrostControl DefrostControl; // defrost control; 1=timed, 2=on-demand
        int EIRFPLR;                                      // index of energy input ratio vs part-load ratio curve
        int DefrostEIRFT;                                 // index of defrost mode total cooling capacity for reverse cycle heat pump
        int RegionNum;                                    // Region number for calculating HSPF of single speed DX heating coil
        Real64 MinOATCompressor;                          // Minimum OAT for heat pump compressor operation
        Real64 OATempCompressorOn;                        // The outdoor tempearture when the compressor is automatically turned back on,
        // if applicable, following automatic shut off. This field is used only for
        // HSPF calculation.
        Real64 MaxOATCompressor;                 // Maximum OAT for VRF heat pump compressor operation
        Real64 MaxOATDefrost;                    // Maximum OAT for defrost operation
        Real64 DefrostTime;                      // Defrost time period in hours
        Real64 DefrostCapacity;                  // Resistive defrost to nominal capacity (at 21.11C/8.33C) ratio
        Real64 HPCompressorRuntime;              // keep track of compressor runtime
        Real64 HPCompressorRuntimeLast;          // keep track of last time step compressor runtime (if simulation downshifts)
        Real64 TimeLeftToDefrost;                // keep track of time left to defrost heat pump
        Real64 DefrostPower;                     // power used during defrost
        Real64 DefrostConsumption;               // energy used during defrost
        HVAC::OATType HeatingPerformanceOATType; // Heating performance curve OAT type (1-wetbulb, 2-drybulb)
        bool HPCoilIsInCoilSystemHeatingDX;
        bool OATempCompressorOnOffBlank;
        // end of variables used in heat pump heating coils only
        // start of variables for DX cooling coil latent degradation model
        Array1D<Real64> Twet_Rated; // Nominal time for condensate to begin leaving the coil's
        // condensate drain line (sec)
        Array1D<Real64> Gamma_Rated; // Initial moisture evaporation rate divided by steady-state
        // AC latent capacity (dimensionless)
        Array1D<Real64> MaxONOFFCyclesperHour;      // Maximum ON/OFF cycles per hour for the compressor (cycles/hour)
        Array1D<Real64> LatentCapacityTimeConstant; // Time constant for latent capacity to reach steady state
        // after startup (sec)
        // end of variables for DX cooling coil latent degradation model
        Array1D<DataHeatBalance::RefrigCondenserType> CondenserType; // Type of condenser for DX cooling coil: AIR COOLED or EVAP COOLED
        // start of variables for DX cooling coil evaporative condenser option
        bool ReportEvapCondVars;        // true if any performance mode includes an evap condenser
        Array1D<Real64> EvapCondEffect; // effectiveness of the evaporatively cooled condenser
        // [high speed for multi-speed unit] (-)
        Real64 CondInletTemp;            // Evap condenser inlet temperature [C], report variable
        Array1D<Real64> EvapCondAirFlow; // Air flow rate through the evap condenser at high speed,
        // for water use calcs [m3/s]
        Array1D<Real64> EvapCondPumpElecNomPower; // Nominal power input to the evap condenser water circulation pump
        // at high speed [W]
        Real64 EvapCondPumpElecPower; // Average power consumed by the evap condenser water circulation pump over
        // the time step [W]
        Real64 EvapCondPumpElecConsumption; // Electric energy consumed by the evap condenser water circulation pump [J]
        Real64 EvapWaterConsumpRate;        // Evap condenser water consumption rate [m3/s]
        Real64 EvapWaterConsump;            // Evap condenser water consumption [m3]
        Real64 EvapCondAirFlow2;            // Air flow rate through the evap condenser at low speed, for water use calcs [m3/s]
        Real64 EvapCondEffect2;             // effectiveness of the evaporatively cooled condenser at low speed (-)
        Real64 EvapCondPumpElecNomPower2;   // Nominal power input to the evap condenser water circulation pump at low speed [W]
        Real64 BasinHeaterPower;            // Basin heater power (W)
        Real64 BasinHeaterConsumption;      // Basin heater energy consumption (J)
        // end of variables for DX cooling coil evaporative condenser option
        // start of variables for Multimode DX cooling coil
        int NumCapacityStages; // number of capacity stages, up to MaxCapacityStages for Multimode DX coil,
        // always 1 for other coils
        int NumDehumidModes; // number of enhanced dehumidification modes, up to MaxDehumidModes for Multimode DX coil,
        // always 0 for other coils)
        Array1D_string CoilPerformanceType;                            // Coil Performance object type
        Array1D_int CoilPerformanceType_Num;                           // Coil Performance object type number
        Array1D_string CoilPerformanceName;                            // Coil Performance object names
        Real64 CoolingCoilStg2RuntimeFrac;                             // Run time fraction of stage 2
        HVAC::CoilMode DehumidificationMode = HVAC::CoilMode::Invalid; // Dehumidification mode for multimode coil,
        // 0=normal, 1+=enhanced dehumidification mode
        // end of variables for Multimode DX cooling coil
        // start of variables for heat pump water heater DX coil
        int WaterInNode;                                                // Condenser water inlet node number for HPWH DX coil
        int WaterOutNode;                                               // Condenser water outlet node number for HPWH DX coil
        int HCOPFTemp;                                                  // COP as a function of temperature curve index
        int HCOPFTempErrorIndex;                                        // Used for warning messages when output of HCOPFTemp is negative
        int HCOPFAirFlow;                                               // COP as a function of air flow rate ratio curve index
        int HCOPFAirFlowErrorIndex;                                     // Used for warning messages when output of HCOPFAirFlow is negative
        int HCOPFWaterFlow;                                             // COP as a function of water flow rate ratio curve index
        int HCOPFWaterFlowErrorIndex;                                   // Used for warning messages when output of HCOPFWaterFlow is negative
        int HCapFTemp;                                                  // Heating capacity as a function of temperature curve index
        int HCapFTempErrorIndex;                                        // Used for warning messages when output of HCapFTemp is negative
        int HCapFAirFlow;                                               // Heating capacity as a function of air flow rate ratio curve index
        int HCapFAirFlowErrorIndex;                                     // Used for warning messages when output of HCapFAirFlow is negative
        int HCapFWaterFlow;                                             // Heating capacity as a function of water flow rate ratio curve index
        int HCapFWaterFlowErrorIndex;                                   // Used for warning messages when output of HCapFWaterFlow is negative
        HVAC::OATType InletAirTemperatureType = HVAC::OATType::Invalid; // Specifies to use either air wet-bulb or dry-bulb temp for curve objects
        Real64 RatedInletDBTemp;                                        // Rated inlet air dry-bulb temperature [C]
        Real64 RatedInletWBTemp;                                        // Rated inlet air wet-bulb temperature [C]
        Real64 RatedInletWaterTemp;                                     // Rated condenser water inlet temperature [C]
        //  REAL(r64) :: CondenserInletWaterTemp     = 0.0d0     ! Actual inlet water temperature to condenser of the HPWH DX coil [C]
        Real64 HPWHCondPumpElecNomPower;    // Nominal power input to the condenser water circulation pump [W]
        Real64 HPWHCondPumpFracToWater;     // Nominal power fraction to water for the condenser water circulation pump
        Real64 RatedHPWHCondWaterFlow;      // Rated water flow rate through the condenser of the HPWH DX coil [m3/s]
        Real64 ElecWaterHeatingPower;       // Total electric power consumed by compressor and condenser pump [W]
        Real64 ElecWaterHeatingConsumption; // Total electric consumption by compressor and condenser pump [J]
        bool FanPowerIncludedInCOP;         // Indicates that fan heat is included in heating capacity and COP
        bool CondPumpHeatInCapacity;        // Indicates that condenser pump heat is included in heating capacity
        bool CondPumpPowerInCOP;            // Indicates that condenser pump power is included in heating COP
        // end of variables for heat pump water heater DX coil
        // Error tracking
        Real64 LowTempLast;  // low ambient temp entering condenser when warning message occurred
        Real64 HighTempLast; // high ambient temp entering condenser when warning message occurred
        int ErrIndex1;       // index/pointer to recurring error structure for Air volume flow rate per watt of
        // rated total cooling capacity error
        int ErrIndex2;              // index/pointer to recurring error structure for PLF curve values must be >= 0.7. error
        int ErrIndex3;              // index/pointer to recurring error structure for DX cooling coil runtime fraction > 1.0 warning
        int ErrIndex4;              // index/pointer to recurring error structure for DX heating coil runtime fraction > 1.0 warning
        int LowAmbErrIndex;         // index/pointer to recurring error structure for low ambient temp entering condenser
        int HighAmbErrIndex;        // index/pointer to recurring error structure for high ambient temp entering condenser
        int PLFErrIndex;            // index/pointer to recurring error structure for PLF <> 1 at speed 1 for a multiple speed coil
        int PLRErrIndex;            // index/pointer to recurring error structure for PLR < .7
        bool PrintLowAmbMessage;    // used to print warning message for low ambient conditions
        std::string LowAmbBuffer1;  // holds warning message until next iteration (only prints 1 message/iteration)
        std::string LowAmbBuffer2;  // holds warning message until next iteration (only prints 1 message/iteration)
        bool PrintHighAmbMessage;   // used to print warning message for high ambient conditions
        std::string HighAmbBuffer1; // holds warning message until next iteration (only prints 1 message/iteration)
        std::string HighAmbBuffer2; // holds warning message until next iteration (only prints 1 message/iteration)
        // begin variables for Water System interactions
        EvapWaterSupply EvapWaterSupplyMode; // where does water come from
        std::string EvapWaterSupplyName;     // name of water source e.g. water storage tank
        int EvapWaterSupTankID;
        int EvapWaterTankDemandARRID;
        CondensateCollectAction CondensateCollectMode; // where does water come from
        std::string CondensateCollectName;             // name of water source e.g. water storage tank
        int CondensateTankID;
        int CondensateTankSupplyARRID;
        Real64 CondensateVdot; // rate of water condensation from air stream [m3/s]
        Real64 CondensateVol;  // amount of water condensed from air stream [m3]
        // end variables for water system interactions
        // used to print low ambient warning message for DOE2 coil only after time step has incremented
        Real64 CurrentEndTimeLast; // end time of time step for last simulation time step
        Real64 TimeStepSysLast;    // last system time step (used to check for downshifting)
        // for multispeed DX coil type
        Constant::eFuel FuelType;                   // Fuel type number
        int NumOfSpeeds;                            // Number of speeds
        bool PLRImpact;                             // Part load fraction applied to Speed Number > 1
        bool LatentImpact;                          // Latent degradation applied to Speed Number > 1
        Array1D_int MSErrIndex;                     // index flag for num speeds/recurring messages
        Array1D<Real64> MSRatedTotCap;              // Rated cooling capacity for MS heat pump [W]
        Array1D<Real64> MSRatedTotCapDes;           // Autosized Gross total cooling capacity at rated conditions [watts]
        Array1D<Real64> MSRatedSHR;                 // Rated SHR for MS heat pump [dimensionless]
        Array1D<Real64> MSRatedCOP;                 // Rated COP for MS heat pump [dimensionless]
        Array1D<Real64> MSRatedAirVolFlowRate;      // Air volume flow rate through unit at rated conditions [m3/s]
        Array1D<Real64> MSRatedAirMassFlowRate;     // Air mass flow rate through unit at rated conditions [m3/s]
        Array1D<Real64> MSRatedCBF;                 // rated coil bypass factor
        Array1D_int MSCCapFTemp;                    // index of total cooling capacity modifier curve
        Array1D_int MSCCapFFlow;                    // index of total cooling capacity modifier curve
        Array1D_int MSEIRFTemp;                     // index of energy input ratio modifier curve as a function of temperature
        Array1D_int MSEIRFFlow;                     // index of energy input ratio modifier curve as a function of flow fraction
        Array1D_int MSPLFFPLR;                      // index of part load factor as a function of part load ratio
        Array1D_int MSWasteHeat;                    // index of waste heat as a function of temperature
        Array1D<Real64> MSWasteHeatFrac;            // Waste heat fraction
        Array1D<Real64> MSEvapCondEffect;           // effectiveness of the evaporatively cooled condenser
        Array1D<Real64> MSEvapCondAirFlow;          // Air flow rate through the evap condenser for water use calcs [m3/s]
        Array1D<Real64> MSEvapCondPumpElecNomPower; // Nominal power input to the evap condenser
        // water circulation pump
        Array1D<Real64> MSTwet_Rated; // Nominal time for condensate to begin leaving the coil's
        // condensate drain line (sec)
        Array1D<Real64> MSGamma_Rated; // Initial moisture evaporation rate divided by steady-state
        // AC latent capacity (dimensionless)
        Array1D<Real64> MSMaxONOFFCyclesperHour;      // Maximum ON/OFF cycles per hour for the compressor (cycles/hour)
        Array1D<Real64> MSLatentCapacityTimeConstant; // Time constant for latent capacity to reach steady state
        Array1D<Real64> MSFanPowerPerEvapAirFlowRate;
        Array1D<Real64> MSFanPowerPerEvapAirFlowRate_2023;
        Real64 FuelUsed = 0.0;     // Energy used, in addition to electricity [W]
        Real64 FuelConsumed = 0.0; // Energy consumed, in addition to electricity [J]
        Real64 MSFuelWasteHeat;    // Total waste heat [J]
        bool MSHPHeatRecActive;    // True when entered Heat Rec Vol Flow Rate > 0
        int MSHPDesignSpecIndex;   // index to MSHPDesignSpecification object used for variable speed coils
        // End of multispeed DX coil input
        // VRF system variables used for sizing
        bool CoolingCoilPresent;         // FALSE if coil not present
        bool HeatingCoilPresent;         // FALSE if coil not present
        bool ISHundredPercentDOASDXCoil; // FALSE if coil is regular dx coil
        Array1D_int SHRFTemp;            // index of sensible heat ratio modifier curve
        // (function of entering wetbulb and drybulb)
        int SHRFTempErrorIndex; // Used for warning messages when output of SHRFTemp is negative
        Array1D_int SHRFFlow;   // index of sensible heat ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        int SHRFFlowErrorIndex; // Used for warning messages when output of SHRFFlow is negative
        int SHRFTemp2;          // index of sensible heat ratio modifier curve
        // (function of entering wetbulb and drybulb)
        int SHRFFlow2; // index of sensible heat ratio modifier curve
        // (function of actual supply air flow vs rated air flow)
        bool UserSHRCurveExists; // TRUE if user specified SHR modifier curve exists
        bool ASHRAE127StdRprt;   // TRUE if user wishes to report ASHRAE 127 standard ratings

        int SecZonePtr;                                // index to the zone where the secondary coil is placed
        int SecCoilSHRFT;                              // index to the secondary coil sensible heat ratio temperature modifier curve
        int SecCoilSHRFF;                              // index to the secondary coil sensible heat ratio flor fraction modifier curve
        Real64 SecCoilAirFlow;                         // secondary coil air flow rate
        Real64 SecCoilAirFlowScalingFactor;            // secondary coil air flow rate autosize scaling factor
        Real64 SecCoilRatedSHR;                        // secondary coil nominal or rated sensible heat ratio
        Real64 SecCoilSHR;                             // secondary coil current sensible heat ratio
        Real64 EvapInletWetBulb;                       // secondary DX coil inlet wet bulb temperature (zone air node wet bulb temp.)
        Real64 SecCoilSensibleHeatGainRate;            // secondary zone sensible heat gain rate [W]
        Real64 SecCoilTotalHeatRemovalRate;            // secondary zone total heat removal rate [W]
        Real64 SecCoilSensibleHeatRemovalRate;         // secondary zone sensible heat removal rate [W]
        Real64 SecCoilLatentHeatRemovalRate;           // secondary zone latent heat removal rate [W]
        bool IsSecondaryDXCoilInZone;                  // true means secondary dx coil is zone instead of outside
        bool IsDXCoilInZone;                           // true means dx coil is in zone instead of outside
        Real64 CompressorPartLoadRatio;                // compressor part load ratio of the primary DX coil
        Array1D_int MSSecCoilSHRFT;                    // index to the multi speed secondary coil sensible heat ratio temperature modifier curve
        Array1D_int MSSecCoilSHRFF;                    //  index to the multi speed secondary coil sensible heat ratio flow fraction modifier curve
        Array1D<Real64> MSSecCoilAirFlow;              // multispeed secondary coil air flow rate
        Array1D<Real64> MSSecCoilAirFlowScalingFactor; // multispeed secondary coil air flow rate autosize scaling factor
        Array1D<Real64> MSSecCoilRatedSHR;             // multispeed secondary coil nominal or rated sensible heat ratio
        int MSSpeedNumLS;                              // current low speed number of multspeed HP
        int MSSpeedNumHS;                              // current high speed number of multspeed HP
        Real64 MSSpeedRatio;                           // current speed ratio of multspeed HP
        Real64 MSCycRatio;                             // current cycling ratio of multspeed HP

        // The following members are for VRF Coils (FluidTCtrl Model)
        int VRFIUPtr;              // index to the VRF Indoor Unit where the coil is placed
        int VRFOUPtr;              // index to the VRF Outdoor Unit that the coil serves
        Real64 EvaporatingTemp;    // indoor unit evaporating temperature [C]
        Real64 CondensingTemp;     // indoor unit condensing temperature [C]
        Real64 C1Te;               // VRF Indoor Unit Coefficient 1 to calculate Te,req [--]
        Real64 C2Te;               // VRF Indoor Unit Coefficient 2 to calculate Te,req [--]
        Real64 C3Te;               // VRF Indoor Unit Coefficient 3 to calculate Te,req [--]
        Real64 C1Tc;               // VRF Indoor Unit Coefficient 1 to calculate Tc,req [--]
        Real64 C2Tc;               // VRF Indoor Unit Coefficient 2 to calculate Tc,req [--]
        Real64 C3Tc;               // VRF Indoor Unit Coefficient 3 to calculate Tc,req [--]
        Real64 SH;                 // Superheating degrees [C]
        Real64 SC;                 // Subcooling  degrees [C]
        Real64 ActualSH;           // Actual superheating degrees [C]
        Real64 ActualSC;           // Actual subcooling degrees [C]
        Real64 RateBFVRFIUEvap;    // VRF Iutdoor Unit Evaporator Rated Bypass Factor
        Real64 RateBFVRFIUCond;    // VRF Iutdoor Unit Condenser Rated Bypass Factor
        int CAPFTErrIndex;         // index/pointer to recurring error structure for CAPFT curve value <= 0.0
        int EIRFTErrIndex;         // index/pointer to recurring error structure for EIRFT curve value <= 0.0
        bool reportCoilFinalSizes; // one time report of sizes to coil selection report
        Real64 capModFacTotal;     // current coil capacity modification factor
        int AirLoopNum;            // Airloop number

        // Default Constructor
        DXCoilData()
            : DXCoilType_Num(0), RatedTotCap(MaxModes, 0.0), HeatSizeRatio(1.0), RatedTotCapEMSOverrideOn(MaxModes, false),
              RatedTotCapEMSOverrideValue(MaxModes, 0.0), FrostHeatingCapacityMultiplierEMSOverrideOn(false),
              FrostHeatingCapacityMultiplierEMSOverrideValue(0.0), FrostHeatingInputPowerMultiplierEMSOverrideOn(false),
              FrostHeatingInputPowerMultiplierEMSOverrideValue(0.0), RatedSHR(MaxModes, 0.0), RatedSHREMSOverrideOn(MaxModes, false),
              RatedSHREMSOverrideValue(MaxModes, 0.0), RatedCOP(MaxModes, 0.0), RatedAirVolFlowRate(MaxModes, 0.0),
              RatedAirVolFlowRateEMSOverrideON(MaxModes, false), RatedAirVolFlowRateEMSOverrideValue(MaxModes, 0.0),
              FanPowerPerEvapAirFlowRate(MaxModes, 0.0), FanPowerPerEvapAirFlowRate_2023(MaxModes, 0.0), RatedAirMassFlowRate(MaxModes, 0.0),
              BypassedFlowFrac(MaxModes, 0.0), RatedCBF(MaxModes, 0.0), AirInNode(0), AirOutNode(0), CCapFTemp(MaxModes, 0), CCapFTempErrorIndex(0),
              CCapFFlow(MaxModes, 0), CCapFFlowErrorIndex(0), EIRFTemp(MaxModes, 0), EIRFTempErrorIndex(0), EIRFFlow(MaxModes, 0),
              EIRFFlowErrorIndex(0), PLFFPLR(MaxModes, 0), ReportCoolingCoilCrankcasePower(true), CrankcaseHeaterCapacity(0.0),
              CrankcaseHeaterPower(0.0), MaxOATCrankcaseHeater(0.0), CrankcaseHeaterCapacityCurveIndex(0), CrankcaseHeaterConsumption(0.0),
              BasinHeaterPowerFTempDiff(0.0), BasinHeaterSetPointTemp(0.0), CompanionUpstreamDXCoil(0), FindCompanionUpStreamCoil(true),
              CondenserInletNodeNum(MaxModes, 0), LowOutletTempIndex(0), FullLoadOutAirTempLast(0.0), FullLoadInletAirTempLast(0.0),
              PrintLowOutTempMessage(false), HeatingCoilPLFCurvePTR(0), RatedTotCap2(0.0), RatedSHR2(0.0), RatedCOP2(0.0), RatedAirVolFlowRate2(0.0),
              FanPowerPerEvapAirFlowRate_LowSpeed(MaxModes, 0.0), FanPowerPerEvapAirFlowRate_2023_LowSpeed(MaxModes, 0.0), RatedAirMassFlowRate2(0.0),
              RatedCBF2(0.0), CCapFTemp2(0), EIRFTemp2(0), RatedEIR2(0.0), InternalStaticPressureDrop(0.0), RateWithInternalStaticAndFanObject(false),
              SupplyFanIndex(0), supplyFanType(HVAC::FanType::Invalid), RatedEIR(MaxModes, 0.0), InletAirMassFlowRate(0.0),
              InletAirMassFlowRateMax(0.0), InletAirTemp(0.0), InletAirHumRat(0.0), InletAirEnthalpy(0.0), OutletAirTemp(0.0), OutletAirHumRat(0.0),
              OutletAirEnthalpy(0.0), PartLoadRatio(0.0), TotalCoolingEnergy(0.0), SensCoolingEnergy(0.0), LatCoolingEnergy(0.0),
              TotalCoolingEnergyRate(0.0), SensCoolingEnergyRate(0.0), LatCoolingEnergyRate(0.0), ElecCoolingConsumption(0.0), ElecCoolingPower(0.0),
              CoolingCoilRuntimeFraction(0.0), TotalHeatingEnergy(0.0), TotalHeatingEnergyRate(0.0), ElecHeatingConsumption(0.0),
              ElecHeatingPower(0.0), HeatingCoilRuntimeFraction(0.0), DefrostStrategy(StandardRatings::DefrostStrat::Invalid),
              DefrostControl(StandardRatings::HPdefrostControl::Invalid), EIRFPLR(0), DefrostEIRFT(0), RegionNum(0), MinOATCompressor(0.0),
              OATempCompressorOn(0.0), MaxOATCompressor(0.0), MaxOATDefrost(0.0), DefrostTime(0.0), DefrostCapacity(0.0), HPCompressorRuntime(0.0),
              HPCompressorRuntimeLast(0.0), TimeLeftToDefrost(0.0), DefrostPower(0.0), DefrostConsumption(0.0),
              HeatingPerformanceOATType(HVAC::OATType::DryBulb), HPCoilIsInCoilSystemHeatingDX(false), OATempCompressorOnOffBlank(false),
              Twet_Rated(MaxModes, 0.0), Gamma_Rated(MaxModes, 0.0), MaxONOFFCyclesperHour(MaxModes, 0.0), LatentCapacityTimeConstant(MaxModes, 0.0),
              CondenserType(MaxModes, DataHeatBalance::RefrigCondenserType::Air), ReportEvapCondVars(false), EvapCondEffect(MaxModes, 0.0),
              CondInletTemp(0.0), EvapCondAirFlow(MaxModes, 0.0), EvapCondPumpElecNomPower(MaxModes, 0.0), EvapCondPumpElecPower(0.0),
              EvapCondPumpElecConsumption(0.0), EvapWaterConsumpRate(0.0), EvapWaterConsump(0.0), EvapCondAirFlow2(0.0), EvapCondEffect2(0.0),
              EvapCondPumpElecNomPower2(0.0), BasinHeaterPower(0.0), BasinHeaterConsumption(0.0), NumCapacityStages(1), NumDehumidModes(0),
              CoilPerformanceType(MaxModes), CoilPerformanceType_Num(MaxModes, 0), CoilPerformanceName(MaxModes), CoolingCoilStg2RuntimeFrac(0.0),
              WaterInNode(0), WaterOutNode(0), HCOPFTemp(0), HCOPFTempErrorIndex(0), HCOPFAirFlow(0), HCOPFAirFlowErrorIndex(0), HCOPFWaterFlow(0),
              HCOPFWaterFlowErrorIndex(0), HCapFTemp(0), HCapFTempErrorIndex(0), HCapFAirFlow(0), HCapFAirFlowErrorIndex(0), HCapFWaterFlow(0),
              HCapFWaterFlowErrorIndex(0), RatedInletDBTemp(0.0), RatedInletWBTemp(0.0), RatedInletWaterTemp(0.0), HPWHCondPumpElecNomPower(0.0),
              HPWHCondPumpFracToWater(0.0), RatedHPWHCondWaterFlow(0.0), ElecWaterHeatingPower(0.0), ElecWaterHeatingConsumption(0.0),
              FanPowerIncludedInCOP(true), CondPumpHeatInCapacity(false), CondPumpPowerInCOP(false), LowTempLast(0.0), HighTempLast(0.0),
              ErrIndex1(0), ErrIndex2(0), ErrIndex3(0), ErrIndex4(0), LowAmbErrIndex(0), HighAmbErrIndex(0), PLFErrIndex(0), PLRErrIndex(0),
              PrintLowAmbMessage(false), PrintHighAmbMessage(false), EvapWaterSupplyMode(EvapWaterSupply::FromMains), EvapWaterSupTankID(0),
              EvapWaterTankDemandARRID(0), CondensateCollectMode(CondensateCollectAction::Discard), CondensateTankID(0), CondensateTankSupplyARRID(0),
              CondensateVdot(0.0), CondensateVol(0.0), CurrentEndTimeLast(0.0), TimeStepSysLast(0.0), FuelType(Constant::eFuel::Invalid),
              NumOfSpeeds(0), PLRImpact(false), LatentImpact(false), MSFuelWasteHeat(0.0), MSHPHeatRecActive(false), MSHPDesignSpecIndex(0),
              CoolingCoilPresent(true), HeatingCoilPresent(true), ISHundredPercentDOASDXCoil(false), SHRFTemp(MaxModes, 0), SHRFTempErrorIndex(0),
              SHRFFlow(MaxModes, 0), SHRFFlowErrorIndex(0), SHRFTemp2(0), SHRFFlow2(0), UserSHRCurveExists(false), ASHRAE127StdRprt(false),
              SecZonePtr(0), SecCoilSHRFT(0), SecCoilSHRFF(0), SecCoilAirFlow(0.0), SecCoilAirFlowScalingFactor(1.0), SecCoilRatedSHR(1.0),
              SecCoilSHR(1.0), EvapInletWetBulb(0.0), SecCoilSensibleHeatGainRate(0.0), SecCoilTotalHeatRemovalRate(0.0),
              SecCoilSensibleHeatRemovalRate(0.0), SecCoilLatentHeatRemovalRate(0.0), IsSecondaryDXCoilInZone(false), IsDXCoilInZone(false),
              CompressorPartLoadRatio(0.0), MSSpeedNumLS(1), MSSpeedNumHS(2), MSSpeedRatio(0.0), MSCycRatio(0.0), VRFIUPtr(0), VRFOUPtr(0),
              EvaporatingTemp(4.0), CondensingTemp(40.0), C1Te(0.0), C2Te(0.0), C3Te(0.0), C1Tc(0.0), C2Tc(0.0), C3Tc(0.0), SH(0.0), SC(0.0),
              ActualSH(0.0), ActualSC(0.0), RateBFVRFIUEvap(0.0592), RateBFVRFIUCond(0.1360), CAPFTErrIndex(0), EIRFTErrIndex(0),
              reportCoilFinalSizes(true), capModFacTotal(0.0), AirLoopNum(0)
        {
        }
    };

    struct PerfModeData
    {
        // Members
        Array1D<std::string> FieldNames;

        // Default Constructor
        PerfModeData() : FieldNames()
        {
        }
    };

    struct DXCoilNumericFieldData
    {
        // Members
        Array1D<PerfModeData> PerfMode; // Coil Performance object type

        // Default Constructor
        DXCoilNumericFieldData() : PerfMode(0)
        {
        }
    };

    // Functions

    void SimDXCoil(EnergyPlusData &state,
                   std::string_view CompName,             // name of the fan coil unit
                   HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                   bool const FirstHVACIteration,         // True when first HVAC iteration
                   int &CompIndex,
                   HVAC::FanOp const fanOp,                                          // allows parent object to control fan mode
                   ObjexxFCL::Optional<Real64 const> PartLoadRatio = _,              // part load ratio (for single speed cycling unit)
                   ObjexxFCL::Optional<Real64 const> OnOffAFR = _,                   // ratio of compressor on airflow to compressor off airflow
                   ObjexxFCL::Optional<Real64 const> CoilCoolingHeatingPLRRatio = _, // used for cycling fan RH control
                   ObjexxFCL::Optional<Real64 const> MaxCap = Constant::MaxCap,      // maximum cooling capacity of VRF terminal units
                   ObjexxFCL::Optional<Real64 const> CompCyclingRatio = _            // cycling ratio of VRF condenser connected to this TU
    );

    void SimDXCoilMultiSpeed(EnergyPlusData &state,
                             std::string_view CompName, // name of the fan coil unit
                             Real64 const SpeedRatio,   // = (CompressorSpeed - CompressorSpeedMin) /
                             Real64 const CycRatio,     // cycling part load ratio for variable speed
                             int &CompIndex,
                             ObjexxFCL::Optional_int_const SpeedNum = _,                // Speed number for multispeed cooling coil onlyn
                             ObjexxFCL::Optional<HVAC::FanOp const> fanOp = _,          // Fan operation mode
                             HVAC::CompressorOp CompressorOp = HVAC::CompressorOp::Off, // Compressor on/off; 1=on, 0=off
                             ObjexxFCL::Optional_int_const SingleMode = _               // Single mode operation Yes/No; 1=Yes, 0=No
    );

    void SimDXCoilMultiMode(EnergyPlusData &state,
                            std::string_view CompName,        // name of the fan coil unit
                            HVAC::CompressorOp compressorOp,  // compressor operation; 1=on, 0=off !unused1208
                            bool const FirstHVACIteration,    // true if first hvac iteration
                            Real64 const PartLoadRatio,       // part load ratio
                            HVAC::CoilMode const DehumidMode, // dehumidification mode (0=normal, 1=enhanced)
                            int &CompIndex,
                            HVAC::FanOp const fanOp // allows parent object to control fan mode
    );

    // TODO: Add New Property for AHRI 2023 Standard & Modify Numeric Fields (increment)
    void GetDXCoils(EnergyPlusData &state);

    void InitDXCoil(EnergyPlusData &state, int const DXCoilNum); // number of the current DX coil unit being simulated

    void SizeDXCoil(EnergyPlusData &state, int const DXCoilNum);

    void CalcHPWHDXCoil(EnergyPlusData &state,
                        int const DXCoilNum,       // the number of the DX coil to be simulated
                        Real64 const PartLoadRatio // sensible water heating load / full load sensible water heating capacity
    );

    void CalcDoe2DXCoil(EnergyPlusData &state,
                        int const DXCoilNum,                        // the number of the DX coil to be simulated
                        HVAC::CompressorOp compressorOp,            // compressor operation; 1=on, 0=off
                        bool const FirstHVACIteration,              // true if this is the first iteration of HVAC
                        Real64 const PartLoadRatio,                 // sensible cooling load / full load sensible cooling capacity
                        HVAC::FanOp const fanOp,                    // Allows parent object to control fan operation
                        ObjexxFCL::Optional_int_const PerfMode = _, // Performance mode for MultiMode DX coil; Always 1 for other coil types
                        ObjexxFCL::Optional<Real64 const> OnOffAirFlowRatio = _, // ratio of compressor on airflow to compressor off airflow
                        ObjexxFCL::Optional<Real64 const> CoolingHeatingPLR = _  // used for cycling fan RH control
    );

    void CalcVRFCoolingCoil(EnergyPlusData &state,
                            int const DXCoilNum,                        // the number of the DX coil to be simulated
                            HVAC::CompressorOp compressorOp,            // compressor operation; 1=on, 0=off
                            bool const FirstHVACIteration,              // true if this is the first iteration of HVAC
                            Real64 const PartLoadRatio,                 // sensible cooling load / full load sensible cooling capacity
                            HVAC::FanOp const fanOp,                    // Allows parent object to control fan operation
                            Real64 const CompCycRatio,                  // cycling ratio of VRF condenser
                            ObjexxFCL::Optional_int_const PerfMode = _, // Performance mode for MultiMode DX coil; Always 1 for other coil types
                            ObjexxFCL::Optional<Real64 const> OnOffAirFlowRatio = _, // ratio of compressor on airflow to compressor off airflow
                            ObjexxFCL::Optional<Real64 const> MaxCoolCap = _         // maximum capacity of DX coil
    );

    void CalcDXHeatingCoil(EnergyPlusData &state,
                           int const DXCoilNum,                                     // the number of the DX heating coil to be simulated
                           Real64 const PartLoadRatio,                              // sensible cooling load / full load sensible cooling capacity
                           HVAC::FanOp const fanOp,                                 // Allows parent object to control fan mode
                           ObjexxFCL::Optional<Real64 const> OnOffAirFlowRatio = _, // ratio of compressor on airflow to compressor off airflow
                           ObjexxFCL::Optional<Real64 const> MaxHeatCap = _         // maximum allowed heating capacity
    );

    void CalcMultiSpeedDXCoil(EnergyPlusData &state,
                              int const DXCoilNum,     // the number of the DX heating coil to be simulated
                              Real64 const SpeedRatio, // = (CompressorSpeed - CompressorSpeedMin) / (CompressorSpeedMax - CompressorSpeedMin)
                              Real64 const CycRatio,   // cycling part load ratio
                              ObjexxFCL::Optional_bool_const ForceOn = _);

    void CalcBasinHeaterPowerForMultiModeDXCoil(EnergyPlusData &state,
                                                int const DXCoilNum,             // Index of coil being simulated
                                                HVAC::CoilMode const DehumidMode // Dehumidification mode (0=normal, 1=enhanced)
    );

    Real64 AdjustCBF(Real64 const CBFNom,             // nominal coil bypass factor
                     Real64 const AirMassFlowRateNom, // nominal air mass flow rate [kg/s]
                     Real64 const AirMassFlowRate     // actual air mass flow rate [kg/s]
    );

    Real64 CalcCBF(EnergyPlusData &state,
                   std::string const &UnitType,
                   std::string const &UnitName,
                   Real64 const InletAirTemp,   // inlet air temperature [C]
                   Real64 const InletAirHumRat, // inlet air humidity ratio [kg water / kg dry air]
                   Real64 const TotCap,         // total cooling  capacity [Watts]
                   Real64 const AirVolFlowRate, // the air volume flow rate at the given capacity [m3/s]
                   Real64 const SHR,            // sensible heat ratio at the given capacity and flow rate
                   bool const PrintFlag = true  // flag used to print warnings if desired
    );

    Real64 ValidateADP(EnergyPlusData &state,
                       std::string const &UnitType,      // component name
                       std::string const &UnitName,      // component type
                       Real64 const RatedInletAirTemp,   // coil inlet air temperature [C]
                       Real64 const RatedInletAirHumRat, // coil inlet air humidity ratio [kg/kg]
                       Real64 const TotCap,              // coil total capacity [W]
                       Real64 const AirMassFlow,         // coil air mass flow rate [kg/s]
                       Real64 const InitialSHR,          // coil sensible heat ratio []
                       std::string const &CallingRoutine // function name calling this routine
    );

    Real64 CalcEffectiveSHR(EnergyPlusData &state,
                            int const DXCoilNum,                             // Index number for cooling coil
                            Real64 const SHRss,                              // Steady-state sensible heat ratio
                            Real64 const RTF,                                // Compressor run-time fraction
                            Real64 const QLatRated,                          // Rated latent capacity
                            Real64 const QLatActual,                         // Actual latent capacity
                            Real64 const EnteringDB,                         // Entering air dry-bulb temperature
                            Real64 const EnteringWB,                         // Entering air wet-bulb temperature
                            ObjexxFCL::Optional_int_const Mode = _,          // Performance mode for MultiMode DX coil; Always 1 for other coil types
                            ObjexxFCL::Optional<Real64 const> HeatingRTF = _ // Used to recalculate Toff for cycling fan systems
    );

    void CalcTotCapSHR(EnergyPlusData &state,
                       Real64 const InletDryBulb,     // inlet air dry bulb temperature [C]
                       Real64 const InletHumRat,      // inlet air humidity ratio [kg water / kg dry air]
                       Real64 const InletEnthalpy,    // inlet air specific enthalpy [J/kg]
                       Real64 const InletWetBulb,     // inlet air wet bulb temperature [C]
                       Real64 const AirMassFlowRatio, // Ratio of actual air mass flow to nominal air mass flow
                       Real64 const AirMassFlow,      // actual mass flow for capacity and SHR calculation
                       Real64 const TotCapNom,        // nominal total capacity [W]
                       Real64 const CBF,              // coil bypass factor
                       int const CCapFTemp,           // capacity modifier curve index, function of entering wetbulb
                       int const CCapFFlow,           // capacity modifier curve, function of actual flow vs rated flow
                       Real64 &TotCap,                // total capacity at the given conditions [W]
                       Real64 &SHR,                   // sensible heat ratio at the given conditions
                       Real64 const CondInletTemp,    // Condenser inlet temperature [C]
                       Real64 const Pressure,         // air pressure [Pa]
                       Real64 &TotCapModFac           // capacity modification factor, func of temp and func of flow
    );

    void CalcMultiSpeedDXCoilCooling(EnergyPlusData &state,
                                     int const DXCoilNum,     // the number of the DX heating coil to be simulated
                                     Real64 const SpeedRatio, // = (CompressorSpeed - CompressorSpeedMin) / (CompressorSpeedMax - CompressorSpeedMin)
                                     Real64 const CycRatio,   // cycling part load ratio
                                     int const SpeedNum,      // Speed number
                                     HVAC::FanOp const fanOp, // Sets fan control to FanOp::Cycling or FanOp::Continuous
                                     HVAC::CompressorOp compressorOp, // Compressor on/off; 1=on, 0=off
                                     int const SingleMode             // Single mode operation Yes/No; 1=Yes, 0=No
    );

    void CalcMultiSpeedDXCoilHeating(EnergyPlusData &state,
                                     int const DXCoilNum,     // the number of the DX heating coil to be simulated
                                     Real64 const SpeedRatio, // = (CompressorSpeed - CompressorSpeedMin) / (CompressorSpeedMax - CompressorSpeedMin)
                                     Real64 const CycRatio,   // cycling part load ratio
                                     int const SpeedNum,      // Speed number
                                     HVAC::FanOp const fanOp, // Fan operation mode
                                     int const SingleMode     // Single mode operation Yes/No; 1=Yes, 0=No
    );

    void UpdateDXCoil(EnergyPlusData &state, int const DXCoilNum); // number of the current fan coil unit being simulated

    void ReportDXCoil(EnergyPlusData &state, int const DXCoilNum); // number of the current fan coil unit being simulated

    void CalcTwoSpeedDXCoilStandardRating(EnergyPlusData &state, int const DXCoilNum);

    void GetFanIndexForTwoSpeedCoil(
        EnergyPlusData &state, int const CoolingCoilIndex, int &SupplyFanIndex, std::string &SupplyFanName, HVAC::FanType &supplyFanType);

    void GetDXCoilIndex(EnergyPlusData &state,
                        std::string const &DXCoilName,
                        int &DXCoilIndex,
                        bool &ErrorsFound,
                        std::string_view const ThisObjectType = {},
                        bool const SuppressWarning = false);

    std::string GetDXCoilName(
        EnergyPlusData &state, int &DXCoilIndex, bool &ErrorsFound, std::string_view const ThisObjectType = {}, bool const SuppressWarning = false);

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string const &CoilType, // must match coil types in this module
                           std::string const &CoilName, // must match coil names for the coil type
                           bool &ErrorsFound            // set to true if problem
    );

    Real64 GetCoilCapacityByIndexType(EnergyPlusData &state,
                                      int const CoilIndex,    // must match coil index for the coil type
                                      int const CoilType_Num, // must match coil types in this module
                                      bool &ErrorsFound       // set to true if problem
    );

    int GetCoilTypeNum(EnergyPlusData &state,
                       std::string const &CoilType,                    // must match coil types in this module
                       std::string const &CoilName,                    // must match coil names for the coil type
                       bool &ErrorsFound,                              // set to true if problem
                       ObjexxFCL::Optional_bool_const PrintWarning = _ // prints warning when true
    );

    Real64 GetMinOATCompressor(EnergyPlusData &state,
                               int const CoilIndex, // index to coil
                               bool &ErrorsFound    // set to true if problem
    );

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string const &CoilType, // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    );

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string const &CoilType, // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    );

    int getCoilInNodeIndex(EnergyPlusData &state,
                           int CoilIndex,    // coil index
                           bool &ErrorsFound // set to true if problem
    );

    int getCoilOutNodeIndex(EnergyPlusData &state,
                            int CoilIndex,    // coil index
                            bool &ErrorsFound // set to true if problem
    );

    int GetCoilCondenserInletNode(EnergyPlusData &state,
                                  std::string const &CoilType, // must match coil types in this module
                                  std::string const &CoilName, // must match coil names for the coil type
                                  bool &ErrorsFound            // set to true if problem
    );

    Real64 GetDXCoilBypassedFlowFrac(EnergyPlusData &state,
                                     std::string const &CoilType, // must match coil types in this module
                                     std::string const &CoilName, // must match coil names for the coil type
                                     bool &ErrorsFound            // set to true if problem
    );

    int GetHPCoolingCoilIndex(EnergyPlusData &state,
                              std::string const &HeatingCoilType, // Type of DX heating coil used in HP
                              std::string const &HeatingCoilName, // Name of DX heating coil used in HP
                              int const HeatingCoilIndex          // Index of DX heating coil used in HP
    );

    int GetDXCoilNumberOfSpeeds(EnergyPlusData &state,
                                std::string const &CoilType, // must match coil types in this module
                                std::string const &CoilName, // must match coil names for the coil type
                                bool &ErrorsFound            // set to true if problem
    );

    Sched::Schedule *GetDXCoilAvailSched(EnergyPlusData &state,
                                         std::string const &CoilType,                // must match coil types in this module
                                         std::string const &CoilName,                // must match coil names for the coil type
                                         bool &ErrorsFound,                          // set to true if problem
                                         ObjexxFCL::Optional_int_const CoilIndex = _ // Coil index number
    );

    Real64 GetDXCoilAirFlow(EnergyPlusData &state,
                            std::string const &CoilType, // must match coil types in this module
                            std::string const &CoilName, // must match coil names for the coil type
                            bool &ErrorsFound            // set to true if problem
    );

    int GetDXCoilCapFTCurveIndex(EnergyPlusData &state,
                                 int const CoilIndex, // coil index pointer
                                 bool &ErrorsFound    // set to true if problem
    );

    void SetDXCoolingCoilData(
        EnergyPlusData &state,
        int const DXCoilNum,                                                         // Number of DX Cooling Coil
        bool &ErrorsFound,                                                           // Set to true if certain errors found
        ObjexxFCL::Optional_int HeatingCoilPLFCurvePTR = _,                          // Parameter equivalent of heating coil PLR curve index
        ObjexxFCL::Optional<DataHeatBalance::RefrigCondenserType> CondenserType = _, // Parameter equivalent of condenser type parameter
        ObjexxFCL::Optional_int CondenserInletNodeNum = _,                           // Parameter equivalent of condenser inlet node number
        ObjexxFCL::Optional<Real64> MaxOATCrankcaseHeater = _, // Parameter equivalent of condenser Max OAT for Crank Case Heater temp
        ObjexxFCL::Optional<Real64> MinOATCooling = _,         // Parameter equivalent of condenser Min OAT for compressor cooling operation
        ObjexxFCL::Optional<Real64> MaxOATCooling = _,         // Parameter equivalent of condenser Max OAT for compressor cooling operation
        ObjexxFCL::Optional<Real64> MinOATHeating = _,         // Parameter equivalent of condenser Min OAT for compressor heating operation
        ObjexxFCL::Optional<Real64> MaxOATHeating = _,         // Parameter equivalent of condenser Max OAT for compressor heating operation
        ObjexxFCL::Optional<HVAC::OATType> HeatingPerformanceOATType = _, // Parameter equivalent to condenser entering air temp type (1-db, 2=wb)
        ObjexxFCL::Optional<StandardRatings::DefrostStrat> DefrostStrategy = _,
        ObjexxFCL::Optional<StandardRatings::HPdefrostControl> DefrostControl = _,
        ObjexxFCL::Optional_int DefrostEIRPtr = _,
        ObjexxFCL::Optional<Real64> DefrostFraction = _,
        ObjexxFCL::Optional<Real64> DefrostCapacity = _,
        ObjexxFCL::Optional<Real64> MaxOATDefrost = _,
        ObjexxFCL::Optional_bool CoolingCoilPresent = _,
        ObjexxFCL::Optional_bool HeatingCoilPresent = _,
        ObjexxFCL::Optional<Real64> HeatSizeRatio = _,
        ObjexxFCL::Optional<Real64> TotCap = _,
        ObjexxFCL::Optional_int SupplyFanIndex = _,
        ObjexxFCL::Optional_string SupplyFanName = _,
        ObjexxFCL::Optional<HVAC::FanType> supplyFanType = _);

    void SetCoilSystemHeatingDXFlag(EnergyPlusData &state,
                                    std::string const &CoilType, // must match coil types in this module
                                    std::string const &CoilName  // must match coil names for the coil type
    );

    void SetCoilSystemCoolingData(EnergyPlusData &state,
                                  std::string const &CoilName, // must match coil names for the coil type
                                  std::string const &CoilSystemName);

    Real64 CalcSHRUserDefinedCurves(EnergyPlusData &state,
                                    Real64 const InletDryBulb,     // inlet air dry bulb temperature [C]
                                    Real64 const InletWetBulb,     // inlet air wet bulb temperature [C]
                                    Real64 const AirMassFlowRatio, // ratio of actual air mass flow to rated air mass flow
                                    int const SHRFTempCurveIndex,  // SHR modifier curve index
                                    int const SHRFFlowCurveIndex,  // SHR modifier curve index
                                    Real64 const SHRRated          // rated sensible heat ratio, user input
    );

    void SetDXCoilTypeData(EnergyPlusData &state, std::string const &CoilName); // must match coil names for the coil type

    void CalcSecondaryDXCoils(EnergyPlusData &state, int const DXCoilNum);

    Real64 CalcSecondaryDXCoilsSHR(EnergyPlusData &state,
                                   int const DXCoilNum,
                                   Real64 const EvapAirMassFlow,
                                   Real64 const TotalHeatRemovalRate,
                                   Real64 const PartLoadRatio,
                                   Real64 const SecCoilRatedSHR,
                                   Real64 const EvapInletDryBulb,
                                   Real64 const EvapInletHumRat,
                                   Real64 const EvapInletWetBulb,
                                   Real64 const EvapInletEnthalpy,
                                   Real64 const CondInletDryBulb,
                                   Real64 const SecCoilFlowFraction,
                                   int const SecCoilSHRFT,
                                   int const SecCoilSHRFF);

    // Begin of Methods for New VRF Model: Fluid Temperature Control
    //******************************************************************************
    void
    CalcVRFCoolingCoil_FluidTCtrl(EnergyPlusData &state,
                                  int const DXCoilNum,                    // the number of the DX coil to be simulated
                                  HVAC::CompressorOp compressorOp,        // compressor operation; 1=on, 0=off
                                  bool const FirstHVACIteration,          // true if this is the first iteration of HVAC
                                  Real64 const PartLoadRatio,             // sensible cooling load / full load sensible cooling capacity
                                  HVAC::FanOp const fanOp,                // Allows parent object to control fan operation
                                  Real64 const CompCycRatio,              // cycling ratio of VRF condenser
                                  ObjexxFCL::Optional_int_const PerfMode, // Performance mode for MultiMode DX coil; Always 1 for other coil types
                                  ObjexxFCL::Optional<Real64 const> OnOffAirFlowRatio, // ratio of compressor on airflow to compressor off airflow
                                  Real64 MaxCoolCap = Constant::MaxCap                 // maximum allowed cooling capacity
    );

    void
    CalcVRFHeatingCoil_FluidTCtrl(EnergyPlusData &state,
                                  HVAC::CompressorOp compressorOp,                     // compressor operation; 1=on, 0=off
                                  int const DXCoilNum,                                 // the number of the DX heating coil to be simulated
                                  Real64 const PartLoadRatio,                          // sensible cooling load / full load sensible cooling capacity
                                  HVAC::FanOp const fanOp,                             // Allows parent object to control fan mode
                                  ObjexxFCL::Optional<Real64 const> OnOffAirFlowRatio, // ratio of compressor on airflow to compressor off airflow
                                  ObjexxFCL::Optional<Real64 const> MaxHeatCap         // maximum allowed heating capacity
    );

    void ControlVRFIUCoil(EnergyPlusData &state,
                          int const CoilIndex,     // index to VRFTU coil
                          Real64 const QCoil,      // coil load
                          Real64 const Tin,        // inlet air temperature
                          Real64 const Win,        // inlet air humidity ratio
                          Real64 const TeTc,       // evaporating or condensing temperature
                          Real64 const OAMassFlow, // mass flow rate of outdoor air
                          Real64 &FanSpdRatio,     // fan speed ratio: actual flow rate / rated flow rate
                          Real64 &Wout,            // outlet air humidity ratio
                          Real64 &Tout,            // outlet air temperature
                          Real64 &Hout,            // outlet air enthalpy
                          Real64 &SHact,           // actual SH
                          Real64 &SCact            // actual SC
    );

    void CalcVRFCoilSenCap(EnergyPlusData &state,
                           int const OperationMode, // mode 0 for cooling, 1 for heating
                           int const CoilNum,       // index to VRFTU cooling or heating coil
                           Real64 const Tinlet,     // dry bulb temperature of air entering the coil
                           Real64 const TeTc,       // evaporating or condensing temperature
                           Real64 const SHSC,       // SH at cooling /SC at heating
                           Real64 const BF,         // Bypass factor
                           Real64 &Q_sen,           // VRF coil sensible capacity per air mass flow rate
                           Real64 &T_coil_surf      // Air temperature at coil surface
    );

    void CalcVRFCoilCapModFac(EnergyPlusData &state,
                              int const OperationMode,                   // mode 0 for cooling, 1 for heating
                              ObjexxFCL::Optional<int const> CoilIndex,  // index to VRFTU cooling or heating coil
                              ObjexxFCL::Optional<std::string> CoilName, // name of VRFTU cooling or heating coil
                              Real64 const Tinlet,                       // dry bulb temperature of air entering the coil
                              ObjexxFCL::Optional<Real64 const> TeTc,    // evaporating or condensing temperature
                              ObjexxFCL::Optional<Real64 const> SHSC,    // SH at cooling /SC at heating
                              ObjexxFCL::Optional<Real64 const> BF,      // Bypass factor
                              Real64 &CapModFac                          // Coil capacity modification factor
    );

    Real64 FanSpdResidualCool(Real64 FanSpdRto, Real64 QCoilSenCoolingLoad, Real64 Ts_1, Real64 Tin, Real64 Garate, Real64 BF);

    Real64 FanSpdResidualHeat(Real64 FanSpdRto, Real64 QCoilSenHeatingLoad, Real64 Ts_1, Real64 Tin, Real64 Garate, Real64 BF);

    void SetMSHPDXCoilHeatRecoveryFlag(EnergyPlusData &state, int const DXCoilNum); // must match coil names for the coil type

    void SetDXCoilAirLoopNumber(EnergyPlusData &state, std::string const &CoilName, int const AirLoopNum); // must match coil names for the coil type

    void DisableLatentDegradation(EnergyPlusData &state, int const DXCoilNum);

} // namespace DXCoils

struct DXCoilsData : BaseGlobalStruct
{

    bool GetCoilsInputFlag = true; // First time, input is "gotten"
    bool MyOneTimeFlag = true;     // One time flag used to allocate MyEnvrnFlag and MySizeFlag
    bool CalcTwoSpeedDXCoilStandardRatingOneTimeEIOHeaderWrite = true;
    bool CrankcaseHeaterReportVarFlag = true;
    int NumVRFHeatingCoils = 0;                   // number of VRF heat pump heating coils
    int NumVRFCoolingCoils = 0;                   // number of VRF heat pump cooling coils
    int NumDXCoils = 0;                           // Total number of DX coils
    int NumVRFHeatingFluidTCtrlCoils = 0;         // number of VRF heat pump heating coils for FluidTCtrl Model
    int NumVRFCoolingFluidTCtrlCoils = 0;         // number of VRF heat pump cooling coils for FluidTCtrl Model
    int NumDXHeatingCoils = 0;                    // number of DX heat pump heating coils
    int NumDoe2DXCoils = 0;                       // number of doe2 DX  coils
    int NumDXHeatPumpWaterHeaterPumpedCoils = 0;  // number of DX  water heater coils, pumped
    int NumDXHeatPumpWaterHeaterWrappedCoils = 0; // number of DX  water heater coils, pumped
    int NumDXMulSpeedCoils = 0;                   // number of DX coils with multi-speed compressor
    int NumDXMulModeCoils = 0;                    // number of DX coils with multi-mode performance
    int NumDXMulSpeedCoolCoils = 0;               // number of multispeed DX cooling coils
    int NumDXMulSpeedHeatCoils = 0;               // number of multispeed DX heating coils
    Real64 HPWHHeatingCapacity = 0.0;             // Used by Heat Pump:Water Heater object as total water heating capacity [W]
    Real64 HPWHHeatingCOP = 0.0;                  // Used by Heat Pump:Water Heater object as water heating COP [W/W]
    Array1D_bool CheckEquipName;
    Array1D<Real64> DXCoilOutletTemp;           // DX coil outlet dry bulb temperature [C]
    Array1D<Real64> DXCoilOutletHumRat;         // DX coil outlet humidity ratio [kgWater/kgDryAir]
    Array1D<Real64> DXCoilPartLoadRatio;        // DX coil part-load ratio
    Array1D<HVAC::FanOp> DXCoilFanOp;           // supply air fan operating mode
    Array1D<Real64> DXCoilFullLoadOutAirTemp;   // DX coil full load outlet dry bulb temperature [C]
    Array1D<Real64> DXCoilFullLoadOutAirHumRat; // DX coil full load outlet humidity ratio [kgWater/kgDryAir]
    Array1D<Real64> DXCoilTotalCooling;         // DX cooling coil total cooling output [W]
    Array1D<Real64> DXCoilTotalHeating;         // DX heating coil total heating output [W]
    Array1D<Real64> DXCoilCoolInletAirWBTemp;   // DX cooling coil inlet air wet-bulb temp [C]
    Array1D<Real64> DXCoilHeatInletAirDBTemp;   // DX heating coil inlet air dry-bulb temp [C]
    Array1D<Real64> DXCoilHeatInletAirWBTemp;   // DX heating coil inlet air wet-bulb temp [C]
    Array1D<DXCoils::DXCoilData> DXCoil;
    Array1D<DXCoils::DXCoilNumericFieldData> DXCoilNumericFields;

    Array1D_bool MyEnvrnFlag; // One time environment flag
    Array1D_bool MySizeFlag;  // One time sizing flag
    Real64 CurrentEndTime = 0.0;
    Real64 CalcVRFCoolingCoil_FluidTCtrlCurrentEndTime = 0.0;
    Real64 CalcVRFCoolingCoilCurrentEndTime = 0.0;
    Real64 NetCoolingCapRated = 0.0; // Net Cooling Coil capacity at Rated conditions, accounting for supply fan heat [W]
    Real64 EER = 0.0;                // Energy Efficiency Ratio in SI [W/W]
    Real64 IEER = 0.0;               // Integerated Energy Efficiency Ratio in SI [W/W]
    Real64 TotCapTempModFac = 0.0;   // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
    Real64 TotCapFlowModFac = 0.0;   // Total capacity modifier (function of actual supply air flow vs rated flow) [-]
    Real64 EIRTempModFac = 0.0;      // EIR modifier (function of entering wetbulb, outside drybulb) [-]
    Real64 EIRFlowModFac = 0.0;      // EIR modifier (function of actual supply air flow vs rated flow) [-]
    Real64 TempDryBulb_Leaving_Apoint = 0.0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DXCoilsData();
    }
};

} // namespace EnergyPlus

#endif
