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

#ifndef VariableSpeedCoils_hh_INCLUDED
#define VariableSpeedCoils_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.fwd.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/StandardRatings.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace VariableSpeedCoils {

    struct VariableSpeedCoilData // variable speed coil
    {
        // Members
        std::string Name;              // Name of the  Coil
        std::string VarSpeedCoilType;  // type of coil
        int NumOfSpeeds;               // Number of speeds
        int NormSpedLevel;             // Nominal speed level
        Real64 RatedWaterVolFlowRate;  // Rated/Ref Water Volumetric Flow Rate [m3/s]
        Real64 RatedWaterMassFlowRate; // Rated/Ref Water Volumetric Flow Rate [m3/s]
        Real64 RatedAirVolFlowRate;    // Rated/Ref Air Volumetric Flow Rate [m3/s]
        Real64 RatedCapHeat;           // Rated/Ref Heating Capacity [W]
        Real64 RatedCapCoolTotal;      // Rated/Ref Total Cooling Capacity [W]
        Real64 MaxONOFFCyclesperHour;  // Maximum ON/OFF cycles per hour for the compressor (cycles/hour)
        Real64 Twet_Rated;             // Nominal time for condensate to begin leaving the coil's
        // condensate drain line (sec)
        Real64 Gamma_Rated; // Initial moisture evaporation rate divided by steady-state
        // AC latent capacity (dimensionless)
        int HOTGASREHEATFLG;               // whether to use hot gas reheat
        Real64 LatentCapacityTimeConstant; // Latent capacity time constant [s]
        int PLFFPLR;                       // index of part load curve as a function of part load ratio
        std::string CoolHeatType;          // Type of WatertoAirHP ie. Heating or Cooling
        int VSCoilType;                    // type of component in plant
        bool SimFlag;                      // Heat Pump Simulation Flag
        Real64 DesignWaterMassFlowRate;    // design water mass flow rate [kg/s]
        Real64 DesignWaterVolFlowRate;     // design water volumetric flow rate [m3/s]
        Real64 DesignAirMassFlowRate;      // Design Air Mass Flow Rate [kg/s]
        Real64 DesignAirVolFlowRate;       // Design Air Volumetric Flow Rate [m3/s]
        Real64 AirVolFlowRate;             // Air Volumetric Flow Rate[m3/s], real time
        Real64 AirMassFlowRate;            // Air Mass Flow Rate[kg/s], real time
        Real64 InletAirPressure;           // air inlet pressure [pa]
        Real64 InletAirDBTemp;             // Inlet Air Dry Bulb Temperature [C], real time
        Real64 InletAirHumRat;             // Inlet Air Humidity Ratio [kg/kg], real time
        Real64 InletAirEnthalpy;           // Inlet Air Enthalpy [J/kg], real time
        Real64 OutletAirDBTemp;            // Outlet Air Dry Bulb Temperature [C], real time
        Real64 OutletAirHumRat;            // Outlet Air Humidity Ratio [kg/kg], real time
        Real64 OutletAirEnthalpy;          // Outlet Air Enthalpy [J/kg], real time
        Real64 WaterVolFlowRate;           // Water Volumetric Flow Rate [m3/s], real time
        Real64 WaterMassFlowRate;          // Water Mass Flow Rate [kg/s], real time
        Real64 InletWaterTemp;             // Inlet Water Temperature [C]
        Real64 InletWaterEnthalpy;         // Inlet Water Enthalpy [J/kg]
        Real64 OutletWaterTemp;            // Outlet Water Temperature [C]
        Real64 OutletWaterEnthalpy;        // Outlet Water Enthalpy [J/kg]
        Real64 Power;                      // Power Consumption [W]
        Real64 QLoadTotal;                 // Load Side Total Heat Transfer Rate [W]
        Real64 QSensible;                  // Sensible Load Side Heat Transfer Rate [W]
        Real64 QLatent;                    // Latent Load Side Heat Transfer Rate [W]
        Real64 QSource;                    // Source Side Heat Transfer Rate [W]
        Real64 QWasteHeat;                 // Recoverable waste Heat Transfer Rate [W]
        Real64 Energy;                     // Energy Consumption [J]
        Real64 EnergyLoadTotal;            // Load Side Total Heat Transferred [J]
        Real64 EnergySensible;             // Sensible Load Side Heat Transferred [J]
        Real64 EnergyLatent;               // Latent Load Side Heat Transferred [J]
        Real64 EnergySource;               // Source Side Heat Transferred [J]
        Real64 COP;                        // Heat Pump Coefficient of Performance [-]
        Real64 RunFrac;                    // Duty Factor
        Real64 PartLoadRatio;              // Part Load Ratio
        Real64 RatedPowerHeat;             // Rated/Ref Heating Power Consumption[W]
        Real64 RatedCOPHeat;               // Rated/Ref Heating COP [W/W]
        Real64 RatedCapCoolSens;           // Rated/Ref Sensible Cooling Capacity [W]
        Real64 RatedPowerCool;             // Rated/Ref Cooling Power Consumption[W]
        Real64 RatedCOPCool;               // Rated/Ref Cooling COP [W/W]
        int AirInletNodeNum;               // Node Number of the Air Inlet
        int AirOutletNodeNum;              // Node Number of the Air Outlet
        int WaterInletNodeNum;             // Node Number of the Water Onlet
        int WaterOutletNodeNum;            // Node Number of the Water Outlet
        PlantLocation plantLoc;
        bool FrostHeatingCapacityMultiplierEMSOverrideOn; // if true, then EMS is calling to override multiplier for heating capacity when system is
                                                          // in defrost
        Real64 FrostHeatingCapacityMultiplierEMSOverrideValue; // value to use for EMS override
        bool FrostHeatingInputPowerMultiplierEMSOverrideOn; // if true, then EMS is calling to override multiplier for power when system is in defrost
        Real64 FrostHeatingInputPowerMultiplierEMSOverrideValue; // value to use for EMS override
        // set by parent object and "pushed" to this structure in SetVSWSHPData subroutine
        bool FindCompanionUpStreamCoil; // Flag to get the companion coil in Init
        bool IsDXCoilInZone;            // true means dx coil is in zone instead of outside
        int CompanionCoolingCoilNum;    // Heating coil companion cooling coil index
        int CompanionHeatingCoilNum;    // Cooling coil companion heating coil index
        Real64 FanDelayTime;            // Fan delay time, time delay for the HP's fan to
        // beginning for multispeed coil type
        int MSHPDesignSpecIndex;              // index to UnitarySystemPerformance:Multispeed object
        Array1D_int MSErrIndex;               // index flag for num speeds/recurring messages
        Array1D<Real64> MSRatedPercentTotCap; // Percentage to the total cooling capacity for MS heat pump at the highest speed [dimensionless]
        Array1D<Real64> MSRatedTotCap;        // Rated cooling capacity for MS heat pump [W]
        Array1D<Real64> MSRatedSHR;           // Rated SHR for MS heat pump [dimensionless]
        Array1D<Real64> MSRatedCOP;           // Rated COP for MS heat pump [dimensionless]
        Array1D<Real64> MSRatedAirVolFlowPerRatedTotCap;
        // Rated Air volume flow rate per total capacity through unit at rated conditions [m^3/w]
        Array1D<Real64> MSRatedAirVolFlowRate;
        // Air volume flow rate through unit at rated conditions [m3/s]
        Array1D<Real64> MSRatedEvaporatorFanPowerPerVolumeFlowRate2017; // 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]
        Array1D<Real64> MSRatedEvaporatorFanPowerPerVolumeFlowRate2023; // 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]
        Array1D<Real64> MSRatedAirMassFlowRate;
        // Air mass flow rate through unit at rated conditions [kg/s]
        Array1D<Real64> MSRatedWaterVolFlowPerRatedTotCap;
        // Rated water volume flow rate per total  capacity through unit at rated conditions [m^3/w]
        Array1D<Real64> MSRatedWaterVolFlowRate;
        // Water volume flow rate through unit at rated conditions [m3/s]
        Array1D<Real64> MSRatedWaterMassFlowRate;
        // Water mass flow rate through unit at rated conditions [kg/s]
        Array1D<Real64> MSRatedCBF;
        // rated coil bypass factor
        Array1D<Real64> MSEffectiveAo;
        // effective heat transfer surface at each speed
        Array1D_int MSCCapFTemp;
        // index of total capacity modifier curve
        Array1D_int MSCCapAirFFlow;
        // index of total capacity modifier curve as a function of air flow
        Array1D_int MSCCapWaterFFlow;
        // index of total capacity modifier curve as a function of water flow
        Array1D_int MSEIRFTemp;
        // index of energy input ratio modifier curve as a function of temperature
        Array1D_int MSEIRAirFFlow;
        // index of energy input ratio modifier curve as a function of air flow fraction
        Array1D_int MSEIRWaterFFlow;
        // index of energy input ratio modifier curve as a function of water flow fraction
        Array1D_int MSWasteHeat;
        // index of waste heat as a function of temperature
        Array1D<Real64> MSWasteHeatFrac;
        // water heating coil pump power at various speeds
        Array1D<Real64> MSWHPumpPower;
        Array1D<Real64> MSWHPumpPowerPerRatedTotCap;
        // Waste heat fraction
        Real64 SpeedNumReport;
        // speed number for output
        Real64 SpeedRatioReport;
        // speed ratio for output between two neighboring speeds
        // End of multispeed water source coil input
        //----------------------------------------------------------------
        // added variables and arrays for variable speed air-source heat pump
        // defrosting
        int DefrostStrategy;       // defrost strategy; 1=reverse-cycle, 2=resistive
        int DefrostControl;        // defrost control; 1=timed, 2=on-demand
        int EIRFPLR;               // index of energy input ratio vs part-load ratio curve
        int DefrostEIRFT;          // index of defrost mode total cooling capacity for reverse cycle heat pump
        Real64 MinOATCompressor;   // Minimum OAT for heat pump compressor operation
        Real64 OATempCompressorOn; // The outdoor tempearture when the compressor is automatically turned back on,
        // if applicable, following automatic shut off. This field is used only for
        // HSPF calculation.
        Real64 MaxOATDefrost;           // Maximum OAT for defrost operation
        Real64 DefrostTime;             // Defrost time period in hours
        Real64 DefrostCapacity;         // Resistive defrost to nominal capacity (at 21.11C/8.33C) ratio
        Real64 HPCompressorRuntime;     // keep track of compressor runtime
        Real64 HPCompressorRuntimeLast; // keep track of last time step compressor runtime (if simulation downshifts)
        Real64 TimeLeftToDefrost;       // keep track of time left to defrost heat pump
        Real64 DefrostPower;            // power used during defrost
        Real64 DefrostConsumption;      // energy used during defrost
        // crankcase heater
        bool ReportCoolingCoilCrankcasePower;  // logical determines if the cooling coil crankcase heater power is reported
        Real64 CrankcaseHeaterCapacity;        // total crankcase heater capacity [W]
        Real64 CrankcaseHeaterPower;           // report variable for average crankcase heater power [W]
        int CrankcaseHeaterCapacityCurveIndex; // Crankcase heater power-temperature curve or table index
        Real64 MaxOATCrankcaseHeater;          // maximum OAT for crankcase heater operation [C]
        Real64 CrankcaseHeaterConsumption;     // report variable for total crankcase heater energy consumption [J]
        // condenser evaporative precooling
        int CondenserInletNodeNum;                          // Node number of outdoor condenser
        DataHeatBalance::RefrigCondenserType CondenserType; // Type of condenser for DX cooling coil: AIR COOLED or EVAP COOLED
        bool ReportEvapCondVars;                            // true if any performance mode includes an evap condenser
        Real64 EvapCondPumpElecNomPower;                    // Nominal power input to the evap condenser water circulation pump [W]
        Real64 EvapCondPumpElecPower;                       // Average power consumed by the evap condenser water circulation pump over
        // the time step [W]
        Real64 EvapWaterConsumpRate;                 // Evap condenser water consumption rate [m3/s]
        Real64 EvapCondPumpElecConsumption;          // Electric energy consumed by the evap condenser water circulation pump [J]
        Real64 EvapWaterConsump;                     // Evap condenser water consumption [m3]
        Real64 BasinHeaterConsumption;               // Basin heater energy consumption (J)
        Real64 BasinHeaterPowerFTempDiff;            // Basin heater capacity per degree C below setpoint (W/C)
        Real64 BasinHeaterSetPointTemp;              // setpoint temperature for basin heater operation (C)
        Real64 BasinHeaterPower;                     // Basin heater power (W)
        Sched::Schedule *basinHeaterSched = nullptr; // basin heater schedule
        Array1D<Real64> EvapCondAirFlow;             // Air flow rate through the evap condenser at high speed, volumetric flow rate
        // for water use calcs [m3/s]
        Array1D<Real64> EvapCondEffect; // effectiveness of the evaporatively cooled condenser
        // [high speed for multi-speed unit] (-)
        Array1D<Real64> MSRatedEvapCondVolFlowPerRatedTotCap; // evap condenser air flow ratio to capacity
        // begin variables for Water System interactions
        int EvapWaterSupplyMode;         // where does water come from
        std::string EvapWaterSupplyName; // name of water source e.g. water storage tank
        int EvapWaterSupTankID;
        int EvapWaterTankDemandARRID;
        int CondensateCollectMode;         // where does water come from
        std::string CondensateCollectName; // name of water source e.g. water storage tank
        int CondensateTankID;
        int CondensateTankSupplyARRID;
        Real64 CondensateVdot;         // rate of water condensation from air stream [m3/s]
        Real64 CondensateVol;          // amount of water condensed from air stream [m3]
        Real64 CondInletTemp;          // Evap condenser inlet temperature [C], report variable
        int SupplyFanIndex;            // index of this fan in fan array or vector
        HVAC::FanType supplyFanType;   // type of fan, in DataHVACGlobals
        std::string SupplyFanName;     // name of fan associated with this dx coil
        Real64 SourceAirMassFlowRate;  // source air mass flow rate [kg/s]
        Real64 InletSourceAirTemp;     // source air temperature entering the outdoor coil [C]
        Real64 InletSourceAirEnthalpy; // source air enthalpy entering the outdoor coil [J/kg]
        // end variables for water system interactions

        // begin varibles for HPWH
        Real64 RatedCapWH;                                              // Rated water heating Capacity [W]
        HVAC::OATType InletAirTemperatureType = HVAC::OATType::Invalid; // Specifies to use either air wet-bulb or dry-bulb temp for curve objects
        Real64 WHRatedInletDBTemp;                                      // Rated inlet air dry-bulb temperature [C]
        Real64 WHRatedInletWBTemp;                                      // Rated inlet air wet-bulb temperature [C]
        Real64 WHRatedInletWaterTemp;                                   // Rated condenser water inlet temperature [C]
        Real64 HPWHCondPumpElecNomPower;                                // Nominal power input to the condenser water circulation pump [W]
        Real64 HPWHCondPumpFracToWater;                                 // Nominal power fraction to water for the condenser water circulation pump
        Real64 RatedHPWHCondWaterFlow;                                  // Rated water flow rate through the condenser of the HPWH DX coil [m3/s]
        Real64 ElecWaterHeatingPower;                                   // Total electric power consumed by compressor and condenser pump [W]
        Real64 ElecWaterHeatingConsumption;                             // Total electric consumption by compressor and condenser pump [J]
        bool FanPowerIncludedInCOP;                                     // Indicates that fan heat is included in heating capacity and COP
        bool CondPumpHeatInCapacity;                                    // Indicates that condenser pump heat is included in heating capacity
        bool CondPumpPowerInCOP;                                        // Indicates that condenser pump power is included in heating COP
        bool AirVolFlowAutoSized;                                       // Used to report autosizing info for the HPWH DX coil
        bool WaterVolFlowAutoSized;                                     // Used to report autosizing info for the HPWH DX coil
        Real64 TotalHeatingEnergy;                                      // total water heating energy
        Real64 TotalHeatingEnergyRate;                                  // total WH energy rate
        bool bIsDesuperheater; // whether the coil is used for a desuperheater, i.e. zero all the cooling capacity and power
        // end variables for HPWH
        bool reportCoilFinalSizes; // one time report of sizes to coil selection report
        Real64 capModFacTotal;     // coil  TotCapTempModFac * TotCapAirFFModFac * TotCapWaterFFModFac, for result for simulation peak reporting

        // default constructor
        VariableSpeedCoilData()
            : NumOfSpeeds(2), NormSpedLevel(HVAC::MaxSpeedLevels), RatedWaterVolFlowRate(DataSizing::AutoSize),
              RatedWaterMassFlowRate(DataSizing::AutoSize), RatedAirVolFlowRate(DataSizing::AutoSize), RatedCapHeat(DataSizing::AutoSize),
              RatedCapCoolTotal(DataSizing::AutoSize), MaxONOFFCyclesperHour(0.0), Twet_Rated(0.0), Gamma_Rated(0.0), HOTGASREHEATFLG(0),
              LatentCapacityTimeConstant(0.0), PLFFPLR(0), VSCoilType(0), SimFlag(false), DesignWaterMassFlowRate(0.0), DesignWaterVolFlowRate(0.0),
              DesignAirMassFlowRate(0.0), DesignAirVolFlowRate(0.0), AirVolFlowRate(0.0), AirMassFlowRate(0.0), InletAirPressure(0.0),
              InletAirDBTemp(0.0), InletAirHumRat(0.0), InletAirEnthalpy(0.0), OutletAirDBTemp(0.0), OutletAirHumRat(0.0), OutletAirEnthalpy(0.0),
              WaterVolFlowRate(0.0), WaterMassFlowRate(0.0), InletWaterTemp(0.0), InletWaterEnthalpy(0.0), OutletWaterTemp(0.0),
              OutletWaterEnthalpy(0.0), Power(0.0), QLoadTotal(0.0), QSensible(0.0), QLatent(0.0), QSource(0.0), QWasteHeat(0.0), Energy(0.0),
              EnergyLoadTotal(0.0), EnergySensible(0.0), EnergyLatent(0.0), EnergySource(0.0), COP(0.0), RunFrac(0.0), PartLoadRatio(0.0),
              RatedPowerHeat(0.0), RatedCOPHeat(0.0), RatedCapCoolSens(0.0), RatedPowerCool(0.0), RatedCOPCool(0.0), AirInletNodeNum(0),
              AirOutletNodeNum(0), WaterInletNodeNum(0), WaterOutletNodeNum(0), plantLoc{}, FrostHeatingCapacityMultiplierEMSOverrideOn(false),
              FrostHeatingCapacityMultiplierEMSOverrideValue(0.0), FrostHeatingInputPowerMultiplierEMSOverrideOn(false),
              FrostHeatingInputPowerMultiplierEMSOverrideValue(0.0), FindCompanionUpStreamCoil(true), IsDXCoilInZone(false),
              CompanionCoolingCoilNum(0), CompanionHeatingCoilNum(0), FanDelayTime(0.0),
              // This one calls into a std::vector, so it's 0-indexed, so we initialize it to -1
              MSHPDesignSpecIndex(-1), MSErrIndex(HVAC::MaxSpeedLevels, 0), MSRatedPercentTotCap(HVAC::MaxSpeedLevels, 0.0),
              MSRatedTotCap(HVAC::MaxSpeedLevels, 0.0), MSRatedSHR(HVAC::MaxSpeedLevels, 0.0), MSRatedCOP(HVAC::MaxSpeedLevels, 0.0),
              MSRatedAirVolFlowPerRatedTotCap(HVAC::MaxSpeedLevels, 0.0), MSRatedAirVolFlowRate(HVAC::MaxSpeedLevels, 0.0),
              MSRatedEvaporatorFanPowerPerVolumeFlowRate2017(HVAC::MaxSpeedLevels, 0.0),
              MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(HVAC::MaxSpeedLevels, 0.0), MSRatedAirMassFlowRate(HVAC::MaxSpeedLevels, 0.0),
              MSRatedWaterVolFlowPerRatedTotCap(HVAC::MaxSpeedLevels, 0.0), MSRatedWaterVolFlowRate(HVAC::MaxSpeedLevels, 0.0),
              MSRatedWaterMassFlowRate(HVAC::MaxSpeedLevels, 0.0), MSRatedCBF(HVAC::MaxSpeedLevels, 0.0), MSEffectiveAo(HVAC::MaxSpeedLevels, 0.0),
              MSCCapFTemp(HVAC::MaxSpeedLevels, 0), MSCCapAirFFlow(HVAC::MaxSpeedLevels, 0), MSCCapWaterFFlow(HVAC::MaxSpeedLevels, 0),
              MSEIRFTemp(HVAC::MaxSpeedLevels, 0), MSEIRAirFFlow(HVAC::MaxSpeedLevels, 0), MSEIRWaterFFlow(HVAC::MaxSpeedLevels, 0),
              MSWasteHeat(HVAC::MaxSpeedLevels, 0), MSWasteHeatFrac(HVAC::MaxSpeedLevels, 0.0), MSWHPumpPower(HVAC::MaxSpeedLevels, 0.0),
              MSWHPumpPowerPerRatedTotCap(HVAC::MaxSpeedLevels, 0.0), SpeedNumReport(0.0), SpeedRatioReport(0.0), DefrostStrategy(0),
              DefrostControl(0), EIRFPLR(0), DefrostEIRFT(0), MinOATCompressor(0.0), OATempCompressorOn(0.0), MaxOATDefrost(0.0), DefrostTime(0.0),
              DefrostCapacity(0.0), HPCompressorRuntime(0.0), HPCompressorRuntimeLast(0.0), TimeLeftToDefrost(0.0), DefrostPower(0.0),
              DefrostConsumption(0.0), ReportCoolingCoilCrankcasePower(true), CrankcaseHeaterCapacity(0.0), CrankcaseHeaterPower(0.0),
              CrankcaseHeaterCapacityCurveIndex(0), MaxOATCrankcaseHeater(0.0), CrankcaseHeaterConsumption(0.0), CondenserInletNodeNum(0),
              CondenserType(DataHeatBalance::RefrigCondenserType::Air), ReportEvapCondVars(false), EvapCondPumpElecNomPower(0.0),
              EvapCondPumpElecPower(0.0), EvapWaterConsumpRate(0.0), EvapCondPumpElecConsumption(0.0), EvapWaterConsump(0.0),
              BasinHeaterConsumption(0.0), BasinHeaterPowerFTempDiff(0.0), BasinHeaterSetPointTemp(0.0), BasinHeaterPower(0.0),
              EvapCondAirFlow(HVAC::MaxSpeedLevels, 0.0), EvapCondEffect(HVAC::MaxSpeedLevels, 0.0),
              MSRatedEvapCondVolFlowPerRatedTotCap(HVAC::MaxSpeedLevels, 0.0), EvapWaterSupplyMode(101), EvapWaterSupTankID(0),
              EvapWaterTankDemandARRID(0), CondensateCollectMode(1001), CondensateTankID(0), CondensateTankSupplyARRID(0), CondensateVdot(0.0),
              CondensateVol(0.0), CondInletTemp(0.0), SupplyFanIndex(0), supplyFanType(HVAC::FanType::Invalid), SourceAirMassFlowRate(0.0),
              InletSourceAirTemp(0.0), InletSourceAirEnthalpy(0.0),
              // begin varibles for HPWH
              RatedCapWH(0.0),                  // Rated water heating Capacity [W]
              WHRatedInletDBTemp(0.0),          // Rated inlet air dry-bulb temperature [C]
              WHRatedInletWBTemp(0.0),          // Rated inlet air wet-bulb temperature [C]
              WHRatedInletWaterTemp(0.0),       // Rated condenser water inlet temperature [C]
              HPWHCondPumpElecNomPower(0.0),    // Nominal power input to the condenser water circulation pump [W]
              HPWHCondPumpFracToWater(1.0),     // Nominal power fraction to water for the condenser water circulation pump
              RatedHPWHCondWaterFlow(0.0),      // Rated water flow rate through the condenser of the HPWH DX coil [m3/s]
              ElecWaterHeatingPower(0.0),       // Total electric power consumed by compressor and condenser pump [W]
              ElecWaterHeatingConsumption(0.0), // Total electric consumption by compressor and condenser pump [J]
              FanPowerIncludedInCOP(false),     // Indicates that fan heat is included in heating capacity and COP
              CondPumpHeatInCapacity(false),    // Indicates that condenser pump heat is included in heating capacity
              CondPumpPowerInCOP(false),        // Indicates that condenser pump power is included in heating COP
              AirVolFlowAutoSized(false),       // Used to report autosizing info for the HPWH DX coil
              WaterVolFlowAutoSized(false),     // Used to report autosizing info for the HPWH DX coil
              TotalHeatingEnergy(0.0),          // total water heating energy
              TotalHeatingEnergyRate(0.0),      // total WH energy rate
              bIsDesuperheater(false),          // whether the coil is used for a desuperheater, i.e. zero all the cooling capacity and power
                                                // end variables for HPWH
              reportCoilFinalSizes(true),       // coil report
              capModFacTotal(0.0)               // coil report

        {
        }
    };

    void SimVariableSpeedCoils(EnergyPlusData &state,
                               std::string_view CompName,       // Coil Name
                               int &CompIndex,                  // Index for Component name
                               HVAC::FanOp const fanOp,         // Continuous fan OR cycling compressor
                               HVAC::CompressorOp compressorOp, // compressor on/off. 0 = off; 1= on
                               Real64 const PartLoadFrac,
                               int const SpeedNum,                  // compressor speed number
                               Real64 const SpeedRatio,             // compressor speed ratio
                               Real64 const SensLoad,               // Sensible demand load [W]
                               Real64 const LatentLoad,             // Latent demand load [W]
                               const Real64 OnOffAirFlowRatio = 1.0 // ratio of comp on to comp off air flow rate
    );

    void GetVarSpeedCoilInput(EnergyPlusData &state);

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitVarSpeedCoil(EnergyPlusData &state,
                          int const DXCoilNum,            // Current DXCoilNum under simulation
                          Real64 const SensLoad,          // Control zone sensible load[W]
                          Real64 const LatentLoad,        // Control zone latent load[W]
                          HVAC::FanOp const fanOp,        // fan operating mode
                          Real64 const OnOffAirFlowRatio, // ratio of compressor on flow to average flow over time step
                          Real64 const SpeedRatio,        // compressor speed ratio
                          int const SpeedNum              // compressor speed number
    );

    void SizeVarSpeedCoil(EnergyPlusData &state, int const DXCoilNum, bool &ErrorsFound);

    void CalcVarSpeedCoilCooling(EnergyPlusData &state,
                                 int const DXCoilNum,             // Heat Pump Number
                                 HVAC::FanOp const fanOp,         // Fan/Compressor cycling scheme indicator
                                 Real64 const SensDemand,         // Cooling Sensible Demand [W] !unused1208
                                 Real64 const LatentDemand,       // Cooling Latent Demand [W]
                                 HVAC::CompressorOp compressorOp, // compressor operation flag
                                 Real64 const PartLoadRatio,      // compressor part load ratio
                                 Real64 const OnOffAirFlowRatio,  // ratio of compressor on flow to average flow over time step
                                 Real64 const SpeedRatio,         // SpeedRatio varies between 1.0 (higher speed) and 0.0 (lower speed)
                                 int const SpeedNum               // Speed number, high bound
    );

    void CalcVarSpeedCoilHeating(EnergyPlusData &state,
                                 int const DXCoilNum,             // Heat Pump Number
                                 HVAC::FanOp const fanOp,         // Fan/Compressor cycling scheme indicator
                                 Real64 const SensDemand,         // Cooling Sensible Demand [W] !unused1208
                                 HVAC::CompressorOp compressorOp, // compressor operation flag
                                 Real64 const PartLoadRatio,      // compressor part load ratio
                                 Real64 const OnOffAirFlowRatio,  // ratio of compressor on flow to average flow over time step
                                 Real64 const SpeedRatio,         // SpeedRatio varies between 1.0 (higher speed) and 0.0 (lower speed)
                                 int const SpeedNum               // Speed number, high bound, i.e. SpeedNum - 1 is the other side
    );

    Real64 GetCoilCapacityVariableSpeed(EnergyPlusData &state,
                                        std::string const &CoilType, // must match coil types in this module
                                        std::string const &CoilName, // must match coil names for the coil type
                                        bool &ErrorsFound            // set to true if problem
    );

    int GetCoilIndexVariableSpeed(EnergyPlusData &state,
                                  std::string const &CoilType, // must match coil types in this module
                                  std::string const &CoilName, // must match coil names for the coil type
                                  bool &ErrorsFound            // set to true if problem
    );

    Real64 GetCoilAirFlowRateVariableSpeed(EnergyPlusData &state,
                                           std::string const &CoilType, // must match coil types in this module
                                           std::string const &CoilName, // must match coil names for the coil type
                                           bool &ErrorsFound            // set to true if problem
    );

    int GetCoilInletNodeVariableSpeed(EnergyPlusData &state,
                                      std::string const &CoilType, // must match coil types in this module
                                      std::string const &CoilName, // must match coil names for the coil type
                                      bool &ErrorsFound            // set to true if problem
    );

    int GetCoilOutletNodeVariableSpeed(EnergyPlusData &state,
                                       std::string const &CoilType, // must match coil types in this module
                                       std::string const &CoilName, // must match coil names for the coil type
                                       bool &ErrorsFound            // set to true if problem
    );

    int GetVSCoilCondenserInletNode(EnergyPlusData &state,
                                    std::string const &CoilName, // must match coil names for the coil type
                                    bool &ErrorsFound            // set to true if problem
    );

    int GetVSCoilPLFFPLR(EnergyPlusData &state,
                         std::string const &CoilType, // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    );

    int GetVSCoilCapFTCurveIndex(EnergyPlusData &state,
                                 int CoilIndex,    // must match coil names for the coil type
                                 bool &ErrorsFound // set to true if problem
    );

    Real64 GetVSCoilMinOATCompressor(EnergyPlusData &state,
                                     int const CoilIndex, // index to cooling coil
                                     bool &ErrorsFound    // set to true if problem
    );

    int GetVSCoilNumOfSpeeds(EnergyPlusData &state,
                             std::string const &CoilName, // must match coil names for the coil type
                             bool &ErrorsFound            // set to true if problem
    );

    Real64 GetVSCoilRatedSourceTemp(EnergyPlusData &state, int const CoilIndex);

    void SetVarSpeedCoilData(EnergyPlusData &state,
                             int const WSHPNum,                                   // Number of OA Controller
                             bool &ErrorsFound,                                   // Set to true if certain errors found
                             ObjexxFCL::Optional_int CompanionCoolingCoilNum = _, // Index to cooling coil for heating coil = SimpleWSHPNum
                             ObjexxFCL::Optional_int CompanionHeatingCoilNum = _, // Index to heating coil for cooling coil = SimpleWSHPNum
                             ObjexxFCL::Optional_int MSHPDesignSpecIndex = _      // index to UnitarySystemPerformance:Multispeed object
    );

    void UpdateVarSpeedCoil(EnergyPlusData &state, int const DXCoilNum);

    Real64 CalcEffectiveSHR(EnergyPlusData &state,
                            int const DXCoilNum,     // Index number for cooling coil
                            Real64 const SHRss,      // Steady-state sensible heat ratio
                            HVAC::FanOp const fanOp, // Fan/compressor cycling scheme indicator
                            Real64 const RTF,        // Compressor run-time fraction
                            Real64 const QLatRated,  // Rated latent capacity
                            Real64 const QLatActual, // Actual latent capacity
                            Real64 const EnteringDB, // Entering air dry-bulb temperature
                            Real64 const EnteringWB  // Entering air wet-bulb temperature
    );

    void CalcTotCapSHR_VSWSHP(EnergyPlusData &state,
                              Real64 const InletDryBulb,       // inlet air dry bulb temperature [C]
                              Real64 const InletHumRat,        // inlet air humidity ratio [kg water / kg dry air]
                              Real64 const InletEnthalpy,      // inlet air specific enthalpy [J/kg]
                              Real64 &InletWetBulb,            // inlet air wet bulb temperature [C]
                              Real64 const AirMassFlowRatio,   // Ratio of actual air mass flow to nominal air mass flow
                              Real64 const WaterMassFlowRatio, // Ratio of actual water mass flow to nominal water mass flow
                              Real64 const AirMassFlow,        // actual mass flow for capacity and SHR calculation
                              Real64 const CBF,                // coil bypass factor
                              Real64 const TotCapNom1,         // nominal total capacity at low speed [W]
                              int const CCapFTemp1,            // capacity modifier curve index, function of entering wetbulb at low speed
                              int const CCapAirFFlow1,         // capacity modifier curve, function of actual air flow vs rated flow at low speed
                              int const CCapWaterFFlow1,       // capacity modifier curve, function of actual water flow vs rated flow at low speed
                              Real64 const TotCapNom2,         // nominal total capacity at high speed [W]
                              int const CCapFTemp2,            // capacity modifier curve index, function of entering wetbulb at high speed
                              int const CCapAirFFlow2,         // capacity modifier curve, function of actual air flow vs rated flow at high speed
                              int const CCapWaterFFlow2,       // capacity modifier curve, function of actual water flow vs rated flow at high speed
                              Real64 &TotCap1,                 // total capacity at the given conditions [W] at low speed
                              Real64 &TotCap2,                 // total capacity at the given conditions [W] at high speed
                              Real64 &TotCapSpeed,             // integrated total capacity corresponding to the speed ratio
                              Real64 &SHR,                     // sensible heat ratio at the given conditions
                              Real64 const CondInletTemp,      // Condenser inlet temperature [C]
                              Real64 const Pressure,           // air pressure [Pa]
                              Real64 const SpeedRatio,         // from 0.0 to 1.0
                              int const NumSpeeds,             // number of speeds for input
                              Real64 &TotCapModFac             // capacity modification factor, func of temp and func of flow
    );

    void CalcVarSpeedHPWH(EnergyPlusData &state,
                          int const DXCoilNum,        // the number of the DX coil to be simulated
                          Real64 const PartLoadRatio, // sensible water heating load / full load sensible water heating capacity
                          Real64 const SpeedRatio,    // SpeedRatio varies between 1.0 (higher speed) and 0.0 (lower speed)
                          int const SpeedNum,         // Speed number, high bound capacity
                          HVAC::FanOp const fanOp     // Continuous fan OR cycling compressor
    );

    Real64 getVarSpeedPartLoadRatio(EnergyPlusData &state, int const DXCoilNum); // the number of the DX coil to mined for current PLR

    void setVarSpeedHPWHFanType(EnergyPlusData &state, int const dXCoilNum, HVAC::FanType fanType);

    void setVarSpeedHPWHFanIndex(EnergyPlusData &state, int const dXCoilNum, int const fanIndex);

} // namespace VariableSpeedCoils

struct VariableSpeedCoilsData : BaseGlobalStruct
{
    int NumVarSpeedCoils = 0;      // The Number of Water to Air Heat Pumps found in the Input
    bool MyOneTimeFlag = true;     // one time allocation flag
    bool GetCoilsInputFlag = true; // Flag set to make sure you get input once
                                   // LOGICAL, ALLOCATABLE, DIMENSION(:) :: MySizeFlag

    Real64 SourceSideMassFlowRate = 0.0; // Source Side Mass flow rate [Kg/s]
    Real64 SourceSideInletTemp = 0.0;    // Source Side Inlet Temperature [C]
    Real64 SourceSideInletEnth = 0.0;    // Source Side Inlet Enthalpy [J/kg]
    Real64 LoadSideMassFlowRate = 0.0;   // Load Side Mass flow rate [Kg/s]
    Real64 LoadSideInletDBTemp = 0.0;    // Load Side Inlet Dry Bulb Temp [C]
    Real64 LoadSideInletWBTemp = 0.0;    // Load Side Inlet Wet Bulb Temp [C]
    Real64 LoadSideInletHumRat = 0.0;    // Load Side Outlet Humidity ratio
    Real64 LoadSideInletEnth = 0.0;      // Load Side Inlet Enthalpy [J/kg]
    Real64 LoadSideOutletDBTemp = 0.0;   // Load Side Outlet Dry Bulb Temp [C]
    Real64 LoadSideOutletHumRat = 0.0;   // Load Side Outlet Humidity ratio
    Real64 LoadSideOutletEnth = 0.0;     // Load Side Outlet Enthalpy [J/kg]
    Real64 QSensible = 0.0;              // Load side sensible heat transfer rate [W]
    Real64 QLoadTotal = 0.0;             // Load side total heat transfer rate [W]
    Real64 QLatRated = 0.0;              // Latent Capacity [W] rated at entering air conditions [Tdb=26.7C Twb=19.4C]
    Real64 QLatActual = 0.0;             // Actual Latent Capacity [W]
    Real64 QSource = 0.0;                // Source side heat transfer rate [W]
    Real64 Winput = 0.0;                 // Power Consumption [W]
    Real64 PLRCorrLoadSideMdot = 0.0;    // Load Side Mdot corrected for Part Load Ratio of the unit

    Real64 VSHPWHHeatingCapacity = 0.0; // Used by Heat Pump:Water Heater object as total water heating capacity [W]
    Real64 VSHPWHHeatingCOP = 0.0;      // Used by Heat Pump:Water Heater object as water heating COP [W/W]

    EPVector<VariableSpeedCoils::VariableSpeedCoilData> VarSpeedCoil;
    bool firstTime = true;

    Array1D_bool MyEnvrnFlag; // used for initializations each begin environment flag
    Array1D_bool MySizeFlag;  // used for sizing PTHP inputs one time
    Array1D_bool MyPlantScanFlag;

    Real64 LoadSideInletDBTemp_Init = 0.0;
    Real64 LoadSideInletWBTemp_Init = 0.0;
    Real64 LoadSideInletHumRat_Init = 0.0;
    Real64 LoadSideInletEnth_Init = 0.0;
    Real64 CpAir_Init = 0.0;

    Real64 OutdoorCoilT = 0.0;                                  // Outdoor coil temperature (C)
    Real64 OutdoorCoildw = 0.0;                                 // Outdoor coil delta w assuming coil temp of OutdoorCoilT (kg/kg)
    Real64 OutdoorDryBulb = 0.0;                                // Outdoor dry-bulb temperature at condenser (C)
    Real64 OutdoorWetBulb = 0.0;                                // Outdoor wet-bulb temperature at condenser (C)
    Real64 OutdoorHumRat = 0.0;                                 // Outdoor humidity ratio at condenser (kg/kg)
    Real64 OutdoorPressure = 0.0;                               // Outdoor barometric pressure at condenser (Pa)
    Real64 FractionalDefrostTime = 0.0;                         // Fraction of time step system is in defrost
    Real64 HeatingCapacityMultiplier = 0.0;                     // Multiplier for heating capacity when system is in defrost
    Real64 InputPowerMultiplier = 0.0;                          // Multiplier for power when system is in defrost
    Real64 LoadDueToDefrost = 0.0;                              // Additional load due to defrost
    Real64 CrankcaseHeatingPower = 0.0;                         // power due to crankcase heater
    Real64 DefrostEIRTempModFac = 0.0;                          // EIR modifier for defrost (function of entering wetbulb, outside drybulb)
    Real64 TotRatedCapacity = 0.0;                              // total rated capacity at the given speed and speed ratio for defrosting
    Real64 OutdoorDryBulb_CalcVarSpeedCoilCooling = 0.0;        // Outdoor dry-bulb temperature at condenser (C)
    Real64 OutdoorWetBulb_CalcVarSpeedCoilCooling = 0.0;        // Outdoor wet-bulb temperature at condenser (C)
    Real64 OutdoorHumRat_CalcVarSpeedCoilCooling = 0.0;         // Outdoor humidity ratio at condenser (kg/kg)
    Real64 OutdoorPressure_CalcVarSpeedCoilCooling = 0.0;       // Outdoor barometric pressure at condenser (Pa)
    Real64 CrankcaseHeatingPower_CalcVarSpeedCoilCooling = 0.0; // power due to crankcase heater
    Real64 CompAmbTemp_CalcVarSpeedCoilCooling = 0.0;           // Ambient temperature at compressor

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumVarSpeedCoils = 0;
        this->MyOneTimeFlag = true;
        this->GetCoilsInputFlag = true;
        this->SourceSideMassFlowRate = 0.0;
        this->SourceSideInletTemp = 0.0;
        this->SourceSideInletEnth = 0.0;
        this->LoadSideMassFlowRate = 0.0;
        this->LoadSideInletDBTemp = 0.0;
        this->LoadSideInletWBTemp = 0.0;
        this->LoadSideInletHumRat = 0.0;
        this->LoadSideInletEnth = 0.0;
        this->LoadSideOutletDBTemp = 0.0;
        this->LoadSideOutletHumRat = 0.0;
        this->LoadSideOutletEnth = 0.0;
        this->QSensible = 0.0;
        this->QLoadTotal = 0.0;
        this->QLatRated = 0.0;
        this->QLatActual = 0.0;
        this->QSource = 0.0;
        this->Winput = 0.0;
        this->PLRCorrLoadSideMdot = 0.0;
        this->VSHPWHHeatingCapacity = 0.0;
        this->VSHPWHHeatingCOP = 0.0;
        this->VarSpeedCoil.deallocate();
        this->firstTime = true;
        this->MyEnvrnFlag.deallocate();
        this->MySizeFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
        this->LoadSideInletDBTemp_Init = 0.0;
        this->LoadSideInletWBTemp_Init = 0.0;
        this->LoadSideInletHumRat_Init = 0.0;
        this->LoadSideInletEnth_Init = 0.0;
        this->CpAir_Init = 0.0;
        this->OutdoorCoilT = 0.0;
        this->OutdoorCoildw = 0.0;
        this->OutdoorDryBulb = 0.0;
        this->OutdoorWetBulb = 0.0;
        this->OutdoorHumRat = 0.0;
        this->OutdoorPressure = 0.0;
        this->FractionalDefrostTime = 0.0;
        this->HeatingCapacityMultiplier = 0.0;
        this->InputPowerMultiplier = 0.0;
        this->LoadDueToDefrost = 0.0;
        this->CrankcaseHeatingPower = 0.0;
        this->DefrostEIRTempModFac = 0.0;
        this->TotRatedCapacity = 0.0;
        this->OutdoorDryBulb_CalcVarSpeedCoilCooling = 0.0;
        this->OutdoorWetBulb_CalcVarSpeedCoilCooling = 0.0;
        this->OutdoorHumRat_CalcVarSpeedCoilCooling = 0.0;
        this->OutdoorPressure_CalcVarSpeedCoilCooling = 0.0;
        this->CrankcaseHeatingPower_CalcVarSpeedCoilCooling = 0.0;
        this->CompAmbTemp_CalcVarSpeedCoilCooling = 0.0;
    }

    // Default Constructor
    VariableSpeedCoilsData() = default;
};
} // namespace EnergyPlus

#endif
