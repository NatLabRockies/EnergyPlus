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

#ifndef HeatRecovery_hh_INCLUDED
#define HeatRecovery_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatRecovery {

    enum class HXConfiguration
    {
        Invalid = -1,
        CounterFlow,
        ParallelFlow,
        CrossFlowBothUnmixed,
        CrossFlowOther,
        Num
    };

    enum class HXExchConfigType
    {
        Invalid = -1,
        Plate,
        Rotary,
        Num
    };

    enum class FrostControlOption
    {
        Invalid = -1,
        None,
        ExhaustOnly,
        ExhaustAirRecirculation,
        MinimumExhaustTemperature,
        Num
    };

    // invalid and num are not used for this internal enum class, but if I leave them out, the custom_check script complains
    // I'm not sure whether to leave them in unused, or add them to the exception list in the script
    // leaving them for now.
    enum class CalculateNTUBoundsErrors
    {
        Invalid = -1,
        NoError,
        MassFlowRatio,
        NominalEffectiveness1,
        NominalEffectiveness2,
        Quantity,
        NominalEffectiveness3,
        Num
    };

    struct HeatExchCond
    {
        std::string Name; // name of component
        HVAC::HXType type = HVAC::HXType::Invalid;
        std::string HeatExchPerfName;                       // Desiccant balanced heat exchanger performance data name
        Sched::Schedule *availSched = nullptr;              // schedule // availability ?
        HXConfiguration FlowArr = HXConfiguration::Invalid; // flow Arrangement:
        bool EconoLockOut = false;
        Real64 hARatio = 0.0;          // ratio of supply side h*A to secondary side h*A
        Real64 NomSupAirVolFlow = 0.0; // nominal supply air volume flow rate (m3/s)
        Real64 NomSupAirInTemp = 0.0;  // nominal supply air inlet temperature (C)
        Real64 NomSupAirOutTemp = 0.0; // nominal supply air outlet temperature (C)
        Real64 NomSecAirVolFlow = 0.0; // nominal secondary air volume flow rate (m3/s)
        Real64 NomSecAirInTemp = 0.0;  // nominal secondary air inlet temperature (C)
        Real64 NomElecPower = 0.0;     // nominal electric power consumption [W]
        // values describing nominal condition (derived from input parameters)
        Real64 UA0 = 0.0;               // (Uavg*A) at nominal condition
        Real64 mTSup0 = 0.0;            // product mDot*Tabs, supply  air, nominal cond.
        Real64 mTSec0 = 0.0;            // product mDot*Tabs, exhaust air, nominal cond
        Real64 NomSupAirMassFlow = 0.0; // nominal supply air mass flow rate (kg/s)
        Real64 NomSecAirMassFlow = 0.0; // nominal secondary air mass flow rate (kg/s)
        // Nodes
        int SupInletNode = 0;  // supply air inlet node number
        int SupOutletNode = 0; // supply air outlet node number
        int SecInletNode = 0;  // secondary air inlet node number
        int SecOutletNode = 0; // secondary air outlet node number
        // inlet conditions
        Real64 SupInTemp = 0.0;     // supply air inlet temperature (C)
        Real64 SupInHumRat = 0.0;   // supply air inlet humidity ratio (kg water/kg dry air)
        Real64 SupInEnth = 0.0;     // supply air inlet enthalpy (J/kg)
        Real64 SupInMassFlow = 0.0; // supply air inlet mass flow rate (kg/s)
        Real64 SecInTemp = 0.0;     // secondary air inlet temperature (C)
        Real64 SecInHumRat = 0.0;   // secondary air inlet humidity ratio (kg water/kg dry air)
        Real64 SecInEnth = 0.0;     // secondary air inlet enthalpy (J/kg)
        Real64 SecInMassFlow = 0.0; // secondary air inlet mass flow rate (kg/s)
        // balanced desiccant inputs
        int PerfDataIndex = 0; // Performance data index allocating performance data number to heat exchanger
        Real64 FaceArea = 0.0; // face area of balanced desiccant heat exchangers to determine face velocity [m2]
        // generic hx performance inputs
        Real64 HeatEffectSensible100 = 0.0;   // heating sensible effectiveness at 100% rated air flow
        Real64 HeatEffectLatent100 = 0.0;     // heating latent effectiveness at 100% rated air flow
        Real64 CoolEffectSensible100 = 0.0;   // cooling sensible effectiveness at 100% rated air flow
        Real64 CoolEffectLatent100 = 0.0;     // cooling latent effectiveness at 100% rated air flow
        int HeatEffectSensibleCurveIndex = 0; // heating sensible effectiveness multiplier curve to HeatEffectSensible100
        int HeatEffectLatentCurveIndex = 0;   // heating latent effectiveness multiplier curve to HeatEffectLatent100
        int CoolEffectSensibleCurveIndex = 0; // cooling sensible effectiveness multiplier curve to CoolEffectSensible100
        int CoolEffectLatentCurveIndex = 0;   // cooling latent effectiveness multiplier curve to CoolEffectLatent100
        // 1 = None, 2 = Bypass, 3 = Stop Rotary HX Rotation
        HXExchConfigType ExchConfig = HXExchConfigType::Invalid; // parameter equivalent of HX configuration, plate or rotary
        // frost control parameters
        FrostControlOption FrostControlType = FrostControlOption::Invalid; // type of frost control used if any
        Real64 ThresholdTemperature = 0.0;                                 // threshold temperature for frost control
        Real64 InitialDefrostTime = 0.0;                                   // initial defrost time
        Real64 RateofDefrostTimeIncrease = 0.0;                            // rate of change of defrost time
        Real64 DefrostFraction = 0.0;                                      // fraction of time HX is in frost control mode
        bool ControlToTemperatureSetPoint = false;                         // temperature control flag for generic HX
        // outlet conditions
        Real64 SupOutTemp = 0.0;     // supply air outlet temperature (C)
        Real64 SupOutHumRat = 0.0;   // supply air outlet humidity ratio (kg water/kg dry air)
        Real64 SupOutEnth = 0.0;     // supply air outlet enthalpy (J/kg)
        Real64 SupOutMassFlow = 0.0; // supply air outlet mass flow rate (kg/s)
        Real64 SecOutTemp = 0.0;     // secondary air outlet temperature (C)
        Real64 SecOutHumRat = 0.0;   // secondary air outlet humidity ratio (kg water/kg dry air)
        Real64 SecOutEnth = 0.0;     // secondary air outlet enthalpy (J/kg)
        Real64 SecOutMassFlow = 0.0; // secondary air outlet mass flow rate (kg/s)
        // report values
        Real64 SensHeatingRate = 0.0;       // rate of sensible heat being added to the supply (primary) air [W]
        Real64 SensHeatingEnergy = 0.0;     // sensible heat added to the supply (primary) air [J]
        Real64 LatHeatingRate = 0.0;        // rate of latent heat being added to the supply (primary) air [W]
        Real64 LatHeatingEnergy = 0.0;      // latent heat added to the supply (primary) air [J]
        Real64 TotHeatingRate = 0.0;        // rate of total heat being added to the supply (primary) air [W]
        Real64 TotHeatingEnergy = 0.0;      // total heat added to the supply (primary) air [J]
        Real64 SensCoolingRate = 0.0;       // rate of sensible heat being removed from the supply (primary) air [W]
        Real64 SensCoolingEnergy = 0.0;     // sensible heat removed from the supply (primary) air [J]
        Real64 LatCoolingRate = 0.0;        // rate of latent heat being removed from the supply (primary) air [W]
        Real64 LatCoolingEnergy = 0.0;      // latent heat removed from the supply (primary) air [J]
        Real64 TotCoolingRate = 0.0;        // rate of total heat being removed from the supply (primary) air [W]
        Real64 TotCoolingEnergy = 0.0;      // total heat removed from the supply (primary) air [J]
        Real64 ElecUseEnergy = 0.0;         // electricity consumption [J]
        Real64 ElecUseRate = 0.0;           // electricity consumption rate [W]
        Real64 SensEffectiveness = 0.0;     // heat exchanger sensible effectiveness [-]
        Real64 LatEffectiveness = 0.0;      // heat exchanger latent effectiveness [-]
        Real64 SupBypassMassFlow = 0.0;     // supply air mass flow rate bypassing the heat exchanger [kg/s]
        Real64 SecBypassMassFlow = 0.0;     // secondary air mass flow rate bypassing the heat exchanger [kg/s]
        int LowFlowErrCount = 0;            // Counter for recurring warning message
        int LowFlowErrIndex = 0;            // Index to recurring warning message
        int UnBalancedErrCount = 0;         // Counter for recurring warning message
        int UnBalancedErrIndex = 0;         // Index to recurring warning message
        bool myEnvrnFlag = true;            // one-time-init flag
        bool SensEffectivenessFlag = false; // flag for error message when sensible effectiveness is negative
        bool LatEffectivenessFlag = false;  // flag for error message when latent effectiveness is negative
        Array1D_string NumericFieldNames;
        bool MySetPointTest = true;
        bool MySizeFlag = true;
        bool hasZoneERVController = false; // If this HX is controlled by a ZoneHVAC:EnergyRecoveryVentilator:Controller

        void initialize(EnergyPlusData &state, int CompanionCoilIndex, int CompanionCoilType_Num);

        void size(EnergyPlusData &state);

        void CalcAirToAirPlateHeatExch(EnergyPlusData &state,
                                       bool HXUnitOn,                                     // flag to simulate heat exchager heat recovery
                                       ObjexxFCL::Optional_bool_const EconomizerFlag = _, // economizer flag pass by air loop or OA sys
                                       ObjexxFCL::Optional_bool_const HighHumCtrlFlag = _ // high humidity control flag passed by airloop or OA sys
        );

        void CalcAirToAirGenericHeatExch(EnergyPlusData &state,
                                         bool HXUnitOn,                                      // flag to simulate heat exchanger heat recovery
                                         bool FirstHVACIteration,                            // first HVAC iteration flag
                                         HVAC::FanOp const fanOp,                            // Supply air fan operating mode (1=cycling, 2=constant)
                                         ObjexxFCL::Optional_bool_const EconomizerFlag = _,  // economizer flag pass by air loop or OA sys
                                         ObjexxFCL::Optional_bool_const HighHumCtrlFlag = _, // high humidity control flag passed by airloop or OA sys
                                         ObjexxFCL::Optional<Real64 const> HXPartLoadRatio = _ //
        );

        void
        CalcDesiccantBalancedHeatExch(EnergyPlusData &state,
                                      bool HXUnitOn,           // flag to simulate heat exchager heat recovery
                                      bool FirstHVACIteration, // First HVAC iteration flag
                                      HVAC::FanOp const fanOp, // Supply air fan operating mode (1=cycling, 2=constant)
                                      Real64 PartLoadRatio,    // Part load ratio requested of DX compressor
                                      int CompanionCoilIndex,  // index of companion cooling coil
                                      int CompanionCoilType,   // type of cooling coil
                                      bool RegenInletIsOANode, // Flag to determine if regen side inlet is OANode, if so this air stream cycles
                                      ObjexxFCL::Optional_bool_const EconomizerFlag = _, // economizer flag pass by air loop or OA sys
                                      ObjexxFCL::Optional_bool_const HighHumCtrlFlag = _ // high humidity control flag passed by airloop or OA sys
        );

        void FrostControl(EnergyPlusData &state);

        void UpdateHeatRecovery(EnergyPlusData &state);

        void ReportHeatRecovery(EnergyPlusData &state);

        void CheckModelBoundsTempEq(EnergyPlusData &state,
                                    Real64 &T_RegenInTemp,   // current regen inlet temperature (C) for regen outlet temp eqn
                                    Real64 &T_RegenInHumRat, // current regen inlet hum rat for regen outlet temp eqn
                                    Real64 &T_ProcInTemp,    // current process inlet temperature (C) for regen outlet temp eqn
                                    Real64 &T_ProcInHumRat,  // current process inlet hum rat for regen outlet temp eqn
                                    Real64 &T_FaceVel,       // current process and regen face velocity (m/s)
                                    bool FirstHVACIteration  // First HVAC iteration flag
        ) const;

        void CheckModelBoundsHumRatEq(EnergyPlusData &state,
                                      Real64 &H_RegenInTemp,   // current regen inlet temperature (C) for regen outlet hum rat eqn
                                      Real64 &H_RegenInHumRat, // current regen inlet hum rat for regen outlet hum rat eqn
                                      Real64 &H_ProcInTemp,    // current process inlet temperature (C) for regen outlet hum rat eqn
                                      Real64 &H_ProcInHumRat,  // current process inlet hum rat for regen outlet hum rat eqn
                                      Real64 &H_FaceVel,       // current process and regen face velocity (m/s)
                                      bool FirstHVACIteration  // First HVAC iteration flag
        ) const;

        void CheckModelBoundOutput_Temp(EnergyPlusData &state,
                                        Real64 RegenInTemp,     // current regen inlet temp passed to eqn
                                        Real64 &RegenOutTemp,   // current regen outlet temp from eqn
                                        bool FirstHVACIteration // First HVAC iteration flag
        ) const;

        void CheckModelBoundOutput_HumRat(EnergyPlusData &state,
                                          Real64 RegenInHumRat,   // current regen inlet hum rat passed to eqn
                                          Real64 &RegenOutHumRat, // current regen outlet hum rat from eqn
                                          bool FirstHVACIteration // First HVAC iteration flag
        ) const;

        void CheckModelBoundsRH_TempEq(EnergyPlusData &state,
                                       Real64 T_RegenInTemp,   // current regen inlet temperature passed to eqn
                                       Real64 T_RegenInHumRat, // current regen inlet hum rat passed to eqn
                                       Real64 T_ProcInTemp,    // current process inlet temperature passed to eqn
                                       Real64 T_ProcInHumRat,  // current regen outlet hum rat from eqn
                                       bool FirstHVACIteration // first HVAC iteration flag
        ) const;

        void CheckModelBoundsRH_HumRatEq(EnergyPlusData &state,
                                         Real64 H_RegenInTemp,   // current regen inlet temperature passed to eqn
                                         Real64 H_RegenInHumRat, // current regen inlet hum rat passed to eqn
                                         Real64 H_ProcInTemp,    // current process inlet temperature passed to eqn
                                         Real64 H_ProcInHumRat,  // current process inlet hum rat passed to eqn
                                         bool FirstHVACIteration // first HVAC iteration flag
        ) const;

        void CheckForBalancedFlow(EnergyPlusData &state,
                                  Real64 ProcessInMassFlow, // current process inlet air mass flow rate (m3/s)
                                  Real64 RegenInMassFlow,   // current regeneration inlet air mass flow rate (m3/s)
                                  bool FirstHVACIteration   // first HVAC iteration flag
        ) const;
    };

    struct ErrorTracker
    {
        bool print = false;  // - flag to print error message
        int index = 0;       // - index to recurring error struct
        int count = 0;       // - counter if limits are exceeded
        std::string buffer1; // - buffer for warn mess on following timestep
        std::string buffer2; // - buffer for warn mess on following timestep
        std::string buffer3; // - buffer for warn mess on following timestep
        Real64 last = 0.0;   // - last value
    };

    struct BalancedDesDehumPerfData
    {
        std::string Name;               // unique name of balanced desiccant performance data type object
        std::string PerfType;           // Type of performance data set
        Real64 NomSupAirVolFlow = 0.0;  // nominal supply air volumetric flow rate m^3/s
        Real64 NomProcAirFaceVel = 0.0; // nominal process air face velocity m/s
        Real64 NomElecPower = 0.0;      // nominal electric power consumption [W]

        std::array<Real64, 8> B = {0}; // regeneration outlet temperature equation coefficients and limits
        // index 0: constant coefficient for outlet regeneration temperature equation
        // index 1: regen inlet humidity ratio coefficient for outlet regen temperature equation
        // index 2: regen inlet temp coefficient for outlet regen temperature equation
        // index 3: (regen in humrat/regen in temp) coefficient for outlet regen temp eq
        // index 4: process inlet humidity ratio coefficient for outlet regen temp equation
        // index 5: process inlet temp coefficient for outlet regen temp equation
        // index 6: (process in humrat/proc in temp) coefficient for outlet regen temp eq
        // index 7: process, regen face velocity coefficient for outlet regen temp eq

        Real64 T_MinRegenAirInTemp = 0.0;   // min allowable regen inlet air temperature [C]
        Real64 T_MaxRegenAirInTemp = 0.0;   // max allowable regen inlet air temperature [C]
        Real64 T_MinRegenAirInHumRat = 0.0; // min allowable regen inlet air humidity ratio [kg water / kg air]
        Real64 T_MaxRegenAirInHumRat = 0.0; // max allowable regen inlet air humidity ratio [kg water / kg air]
        Real64 T_MinProcAirInTemp = 0.0;    // min allowable process inlet air temperature [C]
        Real64 T_MaxProcAirInTemp = 0.0;    // max allowable process inlet air temperature [C]
        Real64 T_MinProcAirInHumRat = 0.0;  // min allowable process inlet air humidity ratio [kg water/kg air]
        Real64 T_MaxProcAirInHumRat = 0.0;  // max allowable process inlet air humidity ratio [kg water/kg air]
        Real64 T_MinFaceVel = 0.0;          // min allowable process, regen face velocity [m/s]
        Real64 T_MaxFaceVel = 0.0;          // max allowable process, regen face velocity [m/s]
        Real64 MinRegenAirOutTemp = 0.0;    // min allowable regen outlet air temperature [C]
        Real64 MaxRegenAirOutTemp = 0.0;    // max allowable regen outlet air temperature [C]
        Real64 T_MinRegenAirInRelHum = 0.0; // min allowable regen inlet air relative humidity [%]
        Real64 T_MaxRegenAirInRelHum = 0.0; // max allowable regen inlet air relative humidity [%]
        Real64 T_MinProcAirInRelHum = 0.0;  // min allowable process inlet air relative humidity [%]
        Real64 T_MaxProcAirInRelHum = 0.0;  // max allowable process inlet air relative humidity [%]

        std::array<Real64, 8> C = {0}; // regeneration outlet humidity ratio equation coefficients and limits
        // index 0: constant coefficient for outlet regen humidity ratio equation
        // index 1: regen inlet humidity ratio coefficient for outlet regen humidity ratio eq
        // index 2: regen inlet temp coefficient for outlet regen humidity ratio equation
        // index 3: (regen in humrat/regen in temp) coefficient for outlet regen humidity ratio eq
        // index 4: process inlet humidity ratio coefficient for outlet regen humidity ratio eq
        // index 5: process inlet temp coefficient for outlet regen humidity ratio eq
        // index 6: (proc in humrat/proc in temp) coefficient for outlet regen humidity ratio eq
        // index 7: process, regen face velocity coefficient for outlet regen humidity ratio eq

        Real64 H_MinRegenAirInTemp = 0.0;   // min allowable regen inlet air temperature [C]
        Real64 H_MaxRegenAirInTemp = 0.0;   // max allowable regen inlet air temperature [C]
        Real64 H_MinRegenAirInHumRat = 0.0; // min allowable regen inlet air humidity ratio [kg water / kg air]
        Real64 H_MaxRegenAirInHumRat = 0.0; // max allowable regen inlet air humidity ratio [kg water / kg air]
        Real64 H_MinProcAirInTemp = 0.0;    // min allowable process inlet air temperature [C]
        Real64 H_MaxProcAirInTemp = 0.0;    // max allowable process inlet air temperature [C]
        Real64 H_MinProcAirInHumRat = 0.0;  // min allowable process inlet air humidity ratio [kg water/kg air]
        Real64 H_MaxProcAirInHumRat = 0.0;  // max allowable process inlet air humidity ratio [kg water/kg air]
        Real64 H_MinFaceVel = 0.0;          // min allowable process, regen face velocity [m/s]
        Real64 H_MaxFaceVel = 0.0;          // max allowable process, regen face velocity [m/s]
        Real64 MinRegenAirOutHumRat = 0.0;  // min allowable regen outlet air temperature [C]
        Real64 MaxRegenAirOutHumRat = 0.0;  // max allowable regen outlet air temperature [C]
        Real64 H_MinRegenAirInRelHum = 0.0; // min allowable regen inlet air relative humidity [%]
        Real64 H_MaxRegenAirInRelHum = 0.0; // max allowable regen inlet air relative humidity [%]
        Real64 H_MinProcAirInRelHum = 0.0;  // min allowable process inlet air relative humidity [%]
        Real64 H_MaxProcAirInRelHum = 0.0;  // max allowable process inlet air relative humidity [%]

        // for model bound checking
        ErrorTracker regenInRelHumTempErr;    // regen inlet relative humidity for temperature equation
        ErrorTracker procInRelHumTempErr;     // process inlet relative humidity for temperature equation
        ErrorTracker regenInRelHumHumRatErr;  // regen inlet relative humidity for humidity ratio equation
        ErrorTracker procInRelHumHumRatErr;   // process inlet relative humidity for humidity ratio equation
        ErrorTracker regenOutHumRatFailedErr; // regen outlet hum rat variables used when regen outlet humidity ratio is below regen inlet humrat,
                                              // verify coefficients warning issued
        ErrorTracker imbalancedFlowErr;       // used when regen and process mass flow rates are not equal to within 2%
        ErrorTracker T_RegenInTempError;      // regen outlet temp eqn
        ErrorTracker T_RegenInHumRatError;    //- T_RegenInhumidity ratio = Regen inlet humidity ratio
        ErrorTracker T_ProcInTempError;       //- T_ProcInTemp = Process inlet temperature
        ErrorTracker T_ProcInHumRatError;     //- T_ProcInhumidity ratio = Process inlet humidity ratio
        ErrorTracker T_FaceVelError;          //- T_FaceVel = Process and regen face velocity
        ErrorTracker regenOutTempError;       //- T_RegenOutTemp = Regen outlet temperature
        ErrorTracker regenOutTempFailedError;
        ErrorTracker H_RegenInTempError;   // regen outlet humidity ratio variables
        ErrorTracker H_RegenInHumRatError; //- H_RegenInhumidity ratio = Regen inlet humidity ratio
        ErrorTracker H_ProcInTempError;    //- H_ProcInTemp = Process inlet temperature
        ErrorTracker H_ProcInHumRatError;  //- H_ProcInhumidity ratio = Process inlet humidity ratio
        ErrorTracker H_FaceVelError;       //- H_FaceVel = Process and regen face velocity
        ErrorTracker regenOutHumRatError;  //- H_RegenOutTemp = Regen outlet temperature

        Array1D_string NumericFieldNames;
    };

    void SimHeatRecovery(EnergyPlusData &state,
                         std::string_view CompName,                             // name of the heat exchanger unit
                         bool FirstHVACIteration,                               // TRUE if 1st HVAC simulation of system timestep
                         int &CompIndex,                                        // Pointer to Component
                         HVAC::FanOp const fanOp,                               // Supply air fan operating mode
                         ObjexxFCL::Optional<Real64 const> HXPartLoadRatio = _, // Part load ratio requested of DX compressor
                         ObjexxFCL::Optional_bool_const HXUnitEnable = _,       // Flag to operate heat exchanger
                         ObjexxFCL::Optional_int_const CompanionCoilIndex = _,  // index of companion cooling coil
                         ObjexxFCL::Optional_bool_const RegenInletIsOANode = _, // flag to determine if supply inlet is OA node, if so air flow cycles
                         ObjexxFCL::Optional_bool_const EconomizerFlag = _,     // economizer operation flag passed by airloop or OA sys
                         ObjexxFCL::Optional_bool_const HighHumCtrlFlag = _,    // high humidity control flag passed by airloop or OA sys
                         ObjexxFCL::Optional_int_const CompanionCoilType_Num = _ // cooling coil type of coil
    );

    void GetHeatRecoveryInput(EnergyPlusData &state);

    Real64 SafeDiv(Real64 a, Real64 b);

    Real64 CalculateEpsFromNTUandZ(EnergyPlusData &state,
                                   Real64 NTU,             // number of transfer units
                                   Real64 Z,               // capacity rate ratio
                                   HXConfiguration FlowArr // flow arrangement
    );

    void CalculateNTUfromEpsAndZ(EnergyPlusData &state,
                                 Real64 &NTU,                   // number of transfer units
                                 CalculateNTUBoundsErrors &Err, // error indicator
                                 Real64 Z,                      // capacity rate ratio
                                 HXConfiguration FlowArr,       // flow arrangement
                                 Real64 Eps                     // heat exchanger effectiveness
    );

    Real64 GetNTUforCrossFlowBothUnmixed(EnergyPlusData &state,
                                         Real64 Eps, // heat exchanger effectiveness
                                         Real64 Z    // capacity rate ratio
    );

    int GetSupplyInletNode(EnergyPlusData &state,
                           std::string const &HXName, // must match HX names for the ExchCond type
                           bool &ErrorsFound          // set to true if problem
    );

    int GetSupplyOutletNode(EnergyPlusData &state,
                            std::string const &HXName, // must match HX names for the ExchCond type
                            bool &ErrorsFound          // set to true if problem
    );

    int GetSecondaryInletNode(EnergyPlusData &state,
                              std::string const &HXName, // must match HX names for the ExchCond type
                              bool &ErrorsFound          // set to true if problem
    );

    int GetSecondaryOutletNode(EnergyPlusData &state,
                               std::string const &HXName, // must match HX names for the ExchCond type
                               bool &ErrorsFound          // set to true if problem
    );

    Real64 GetSupplyAirFlowRate(EnergyPlusData &state,
                                std::string const &HXName, // must match HX names for the ExchCond type
                                bool &ErrorsFound          // set to true if problem
    );

    HVAC::HXType GetHeatExchangerObjectTypeNum(EnergyPlusData &state,
                                               std::string const &HXName, // must match HX names for the ExchCond type
                                               int &WhichHX,
                                               bool &ErrorsFound // set to true if problem
    );

} // namespace HeatRecovery

struct ErrorTracker2
{
    std::string OutputChar;          // character string for warning messages
    std::string OutputCharLo;        // character string for warning messages
    std::string OutputCharHi;        // character string for warning messages
    std::string CharValue;           // character string for warning messages
    Real64 TimeStepSysLast = 0.0;    // last system time step (used to check for downshifting)
    Real64 CurrentEndTime = 0.0;     // end time of time step for current simulation time step
    Real64 CurrentEndTimeLast = 0.0; // end time of time step for last simulation time step
};

struct HeatRecoveryData : BaseGlobalStruct
{

    int NumHeatExchangers = 0;       // number of heat exchangers
    Real64 FullLoadOutAirTemp = 0.0; // Used with desiccant HX empirical model, water coils use inlet node condition
    // DX coils use DXCoilFullLoadOutAirTemp when coil is ON otherwise inlet node
    Real64 FullLoadOutAirHumRat = 0.0; // Used with desiccant HX empirical model, water coils use inlet node condition
    // DX coils use DXCoilFullLoadOutAirHumRat when coil is ON otherwise inlet node
    bool GetInputFlag = true;           // First time, input is "gotten"
    bool CalledFromParentObject = true; // Indicates that HX is called from parent object (this object is not on a branch)
    Array1D_bool CheckEquipName;
    ErrorTracker2 error1;
    ErrorTracker2 error2;
    ErrorTracker2 error3;
    ErrorTracker2 error4;
    ErrorTracker2 error5;
    ErrorTracker2 error6;
    ErrorTracker2 error7;
    std::string OutputCharProc;       // character string for warning messages
    std::string OutputCharRegen;      // character string for warning messages
    Real64 TimeStepSysLast7 = 0.0;    // last system time step (used to check for downshifting)
    Real64 CurrentEndTime7 = 0.0;     // end time of time step for current simulation time step
    Real64 CurrentEndTimeLast7 = 0.0; // end time of time step for last simulation time step
    Real64 RegenInletRH = 0.0;        // Regeneration inlet air relative humidity
    Real64 ProcInletRH = 0.0;         // Process inlet air relative humidity
    Real64 RegenInletRH2 = 0.0;       // Regeneration inlet air relative humidity
    Real64 ProcInletRH2 = 0.0;        // Process inlet air relative humidity

    std::unordered_map<std::string, std::string> HeatExchangerUniqueNames;
    Array1D<HeatRecovery::HeatExchCond> ExchCond;
    Array1D<HeatRecovery::BalancedDesDehumPerfData> BalDesDehumPerfData;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HeatRecoveryData();
    }
};

} // namespace EnergyPlus

#endif
