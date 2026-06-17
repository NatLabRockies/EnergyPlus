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

#ifndef WindTurbine_hh_INCLUDED
#define WindTurbine_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WindTurbine {

    enum class RotorType
    {
        Invalid = -1,
        HorizontalAxis,
        VerticalAxis,
        Num
    };

    enum class ControlType
    {
        Invalid = -1,
        FixedSpeedFixedPitch,
        FixedSpeedVariablePitch,
        VariableSpeedFixedPitch,
        VariableSpeedVariablePitch,
        Num
    };

    struct WindTurbineParams
    {
        // Members
        std::string Name;                               // The component name
        RotorType rotorType = RotorType::Invalid;       // Rotor type (HAWT or VAWT)
        ControlType controlType = ControlType::Invalid; // Control type
        Sched::Schedule *availSched = nullptr;
        int NumOfBlade = 0;                        // Blade number
        Real64 RatedRotorSpeed = 0.0;              // Rated rotor speed in m/s
        Real64 RotorDiameter = 0.0;                // Diameter of rotor in m
        Real64 RotorHeight = 0.0;                  // Overall height of the rotor in m
        Real64 RatedPower = 0.0;                   // Nominal average power output at the rated wind speed in Watts
        Real64 RatedWindSpeed = 0.0;               // Rated wind speed showing maximum power output in Watts
        Real64 CutInSpeed = 0.0;                   // Minimum wind speed for system operation in m/s
        Real64 CutOutSpeed = 0.0;                  // Maximum wind speed for system operation in m/s
        Real64 SysEfficiency = 0.0;                // Overall system efficiency including subsystems and losses
        Real64 MaxTipSpeedRatio = 0.0;             // Maximum tip speed ratio
        Real64 MaxPowerCoeff = 0.0;                // Maximum power coefficient
        Real64 LocalAnnualAvgWS = 0.0;             // Annual average wind speed locally measured in m/s
        Real64 AnnualTMYWS = 0.0;                  // Annual average wind speed from stat file in m/s
        Real64 HeightForLocalWS = 0.0;             // Height of the local station in m
        Real64 ChordArea = 0.0;                    // Chord area of a single blade for VAWTs in m2
        Real64 DragCoeff = 0.0;                    // Empirical blade drag coefficient for VAWTs
        Real64 LiftCoeff = 0.0;                    // Empirical blade lift coefficient for VAWTs
        std::array<Real64, 6> PowerCoeffs = {0.0}; // Empirical power coefficients for analytical calculation
        Real64 TotPower = 0.0;                     // Maximum power produced from the wind in Watts
        Real64 Power = 0.0;                        // Actual power wind turbine supplies to the building in Watts
        Real64 TotEnergy = 0.0;                    // Maximum energy produced from the wind in Joules
        Real64 Energy = 0.0;                       // Actual energy wind turbine supplies to the building in Joules
        Real64 LocalWindSpeed = 0.0;               // Local wind speed estimated at the particular height in m/s
        Real64 LocalAirDensity = 0.0;              // Local air density estimated at the particular height kg/m3
        Real64 PowerCoeff = 0.0;                   // Power coefficient determined
        Real64 ChordalVel = 0.0;                   // Chordal velocity for VAWTs in m/s
        Real64 NormalVel = 0.0;                    // Normal velocity for VAWTs in m/s
        Real64 RelFlowVel = 0.0;                   // Relative flow velocity for VAWTs in m/s
        Real64 TipSpeedRatio = 0.0;                // Relative flow velocity for VAWTs in m/s
        Real64 WSFactor = 0.0;                     // Relative flow velocity for VAWTs in m/s
        Real64 AngOfAttack = 0.0;                  // Angle of attack in degree
        Real64 IntRelFlowVel = 0.0;                // Integral of relative flow velocity
        Real64 TanForce = 0.0;                     // Tangential force
        Real64 NorForce = 0.0;                     // Normal force in N.m
        Real64 TotTorque = 0.0;                    // Total torque in N.m
        Real64 AzimuthAng = 0.0;                   // Azimuth angle between blades
    };

    void SimWindTurbine(EnergyPlusData &state,
                        GeneratorType GeneratorType,      // Type of Generator
                        std::string const &GeneratorName, // User specified name of Generator
                        int &GeneratorIndex,              // Generator index
                        bool RunFlag,                     // ON or OFF
                        Real64 WTLoad                     // Electrical load on WT (not used)
    );

    void GetWTGeneratorResults(EnergyPlusData &state,
                               GeneratorType GeneratorType, // Type of Generator
                               int GeneratorIndex,          // Generator number
                               Real64 &GeneratorPower,      // Electrical power
                               Real64 &GeneratorEnergy,     // Electrical energy
                               Real64 &ThermalPower,
                               Real64 &ThermalEnergy);

    void GetWindTurbineInput(EnergyPlusData &state);

    void InitWindTurbine(EnergyPlusData &state, int WindTurbineNum);

    void CalcWindTurbine(EnergyPlusData &state,
                         int WindTurbineNum, // System is on
                         bool RunFlag        // System is on
    );

    void ReportWindTurbine(EnergyPlusData &state, int WindTurbineNum);

    //*****************************************************************************************

} // namespace WindTurbine

struct WindTurbineData : BaseGlobalStruct
{

    bool GetInputFlag = true;
    bool MyOneTimeFlag = true;
    EPVector<WindTurbine::WindTurbineParams> WindTurbineSys;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) WindTurbineData();
    }
};

} // namespace EnergyPlus

#endif
