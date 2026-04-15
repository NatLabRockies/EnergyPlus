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

#ifndef GeneralRoutines_hh_INCLUDED
#define GeneralRoutines_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2S.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

struct IntervalHalf
{
    // Members
    Real64 MaxFlow;
    Real64 MinFlow;
    Real64 MaxResult;
    Real64 MinResult;
    Real64 MidFlow;
    Real64 MidResult;
    bool MaxFlowCalc;
    bool MinFlowCalc;
    bool MinFlowResult;
    bool NormFlowCalc;

    // Default Constructor
    IntervalHalf() = default;

    // Member Constructor
    IntervalHalf(Real64 const MaxFlow,
                 Real64 const MinFlow,
                 Real64 const MaxResult,
                 Real64 const MinResult,
                 Real64 const MidFlow,
                 Real64 const MidResult,
                 bool const MaxFlowCalc,
                 bool const MinFlowCalc,
                 bool const MinFlowResult,
                 bool const NormFlowCalc)
        : MaxFlow(MaxFlow), MinFlow(MinFlow), MaxResult(MaxResult), MinResult(MinResult), MidFlow(MidFlow), MidResult(MidResult),
          MaxFlowCalc(MaxFlowCalc), MinFlowCalc(MinFlowCalc), MinFlowResult(MinFlowResult), NormFlowCalc(NormFlowCalc)
    {
    }
};

struct ZoneEquipControllerProps
{
    // Members
    Real64 SetPoint;           // Desired setpoint;
    Real64 MaxSetPoint;        // The maximum setpoint; either user input or reset per time step by simulation
    Real64 MinSetPoint;        // The minimum setpoint; either user input or reset per time step by simulation
    Real64 SensedValue;        // The sensed control variable of any type
    Real64 CalculatedSetPoint; // The Calculated SetPoint or new control actuated value

    // Default Constructor
    ZoneEquipControllerProps() = default;

    // Member Constructor
    ZoneEquipControllerProps(Real64 const SetPoint,          // Desired setpoint;
                             Real64 const MaxSetPoint,       // The maximum setpoint; either user input or reset per time step by simulation
                             Real64 const MinSetPoint,       // The minimum setpoint; either user input or reset per time step by simulation
                             Real64 const SensedValue,       // The sensed control variable of any type
                             Real64 const CalculatedSetPoint // The Calculated SetPoint or new control actuated value
                             )
        : SetPoint(SetPoint), MaxSetPoint(MaxSetPoint), MinSetPoint(MinSetPoint), SensedValue(SensedValue), CalculatedSetPoint(CalculatedSetPoint)
    {
    }
};

void ControlCompOutput(EnergyPlusData &state,
                       std::string const &CompName,                          // the component Name
                       std::string const &CompType,                          // Type of component
                       int &CompNum,                                         // Index of component in component array
                       bool const FirstHVACIteration,                        // flag for 1st HVAV iteration in the time step
                       Real64 const QZnReq,                                  // zone load to be met
                       int const ActuatedNode,                               // node that controls unit output
                       Real64 const MaxFlow,                                 // maximum water flow
                       Real64 const MinFlow,                                 // minimum water flow
                       Real64 const ControlOffset,                           // really the tolerance
                       int &ControlCompTypeNum,                              // Internal type num for CompType
                       int &CompErrIndex,                                    // for Recurring error call
                       ObjexxFCL::Optional_int_const TempInNode = _,         // inlet node for output calculation
                       ObjexxFCL::Optional_int_const TempOutNode = _,        // outlet node for output calculation
                       ObjexxFCL::Optional<Real64 const> AirMassFlow = _,    // air mass flow rate
                       ObjexxFCL::Optional_int_const Action = _,             // 1=reverse; 2=normal
                       ObjexxFCL::Optional_int_const EquipIndex = _,         // Identifier for equipment of Outdoor Air Unit "ONLY"
                       PlantLocation const &plantLoc = {},                   // for plant components, Location
                       ObjexxFCL::Optional_int_const ControlledZoneIndex = _ // controlled zone index for the zone containing the component
);

bool BBConvergeCheck(int const SimCompNum, Real64 const MaxFlow, Real64 const MinFlow);

void CheckSysSizing(EnergyPlusData &state,
                    std::string_view const CompType, // Component Type (e.g. Chiller:Electric)
                    std::string const &CompName      // Component Name (e.g. Big Chiller)
);

void CheckThisAirSystemForSizing(EnergyPlusData &state, int const AirLoopNum, bool &AirLoopWasSized);

void CheckZoneSizing(EnergyPlusData &state,
                     std::string_view CompType, // Component Type (e.g. Chiller:Electric)
                     std::string_view CompName  // Component Name (e.g. Big Chiller)
);

void CheckThisZoneForSizing(EnergyPlusData &state,
                            int const ZoneNum, // zone index to be checked
                            bool &ZoneWasSized);

void ValidateComponent(EnergyPlusData &state,
                       std::string_view CompType,   // Component Type (e.g. Chiller:Electric)
                       std::string const &CompName, // Component Name (e.g. Big Chiller)
                       bool &IsNotOK,               // .TRUE. if this component pair is invalid
                       std::string_view CallString  // Context of this pair -- for error message
);

void ValidateComponent(EnergyPlusData &state,
                       std::string_view CompType,      // Component Type (e.g. Chiller:Electric)
                       std::string const &CompValType, // Component "name" field type
                       std::string const &CompName,    // Component Name (e.g. Big Chiller)
                       bool &IsNotOK,                  // .TRUE. if this component pair is invalid
                       std::string_view CallString     // Context of this pair -- for error message
);

// Why is this in GeneralRoutines? Asking for a friend
void CalcBasinHeaterPower(EnergyPlusData const &state,
                          Real64 const Capacity,     // Basin heater capacity per degree C below setpoint (W/C)
                          Sched::Schedule *sched,    // Pointer to basin heater schedule
                          Real64 const SetPointTemp, // setpoint temperature for basin heater operation (C)
                          Real64 &Power              // Basin heater power (W)
);

void TestAirPathIntegrity(EnergyPlusData &state, bool &ErrFound);

void TestSupplyAirPathIntegrity(EnergyPlusData &state, bool &ErrFound);

void TestReturnAirPathIntegrity(EnergyPlusData &state, bool &ErrFound, Array2S_int ValRetAPaths);

void CalcComponentSensibleLatentOutput(Real64 const MassFlow,  // air mass flow rate, {kg/s}
                                       Real64 const TDB2,      // dry-bulb temperature at state 2 {C}
                                       Real64 const W2,        // humidity ratio at state 2
                                       Real64 const TDB1,      // dry-bulb temperature at  at state 1 {C}
                                       Real64 const W1,        // humidity ratio at state 1
                                       Real64 &SensibleOutput, // sensible output rate (state 2 -> State 1), {W}
                                       Real64 &LatentOutput,   // latent output rate (state 2 -> State 1), {W}
                                       Real64 &TotalOutput     // total = sensible + latent putput rate (state 2 -> State 1), {W}
);

void CalcZoneSensibleLatentOutput(Real64 const MassFlow,  // air mass flow rate, {kg/s}
                                  Real64 const TDBEquip,  // dry-bulb temperature at equipment outlet {C}
                                  Real64 const WEquip,    // humidity ratio at equipment outlet
                                  Real64 const TDBZone,   // dry-bulb temperature at zone air node {C}
                                  Real64 const WZone,     // humidity ratio at zone air node
                                  Real64 &SensibleOutput, // sensible output rate (state 2 -> State 1), {W}
                                  Real64 &LatentOutput,   // latent output rate (state 2 -> State 1), {W}
                                  Real64 &TotalOutput     // total = sensible + latent putput rate (state 2 -> State 1), {W}
);

Real64 calcZoneSensibleOutput(Real64 const MassFlow, // air mass flow rate, {kg/s}
                              Real64 const TDBEquip, // dry-bulb temperature at equipment outlet {C}
                              Real64 const TDBZone,  // dry-bulb temperature at zone air node {C}
                              Real64 const WZone);

struct GeneralRoutinesData : BaseGlobalStruct
{

    bool MyICSEnvrnFlag = true;
    IntervalHalf ZoneInterHalf = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, false, false, false};
    ZoneEquipControllerProps ZoneController = {0.0, 0.0, 0.0, 0.0, 0.0};

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->MyICSEnvrnFlag = true;
    }
};

} // namespace EnergyPlus

#endif
