// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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
// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/WindTurbine.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, WindTurbineTest)
{
    // this unit test runs the window air conditioner with a Coil:Cooling:DX:VariableSpeed coil
    // set up minimal zone, zone equipment, and ZoneHVAC:WindowAirConditioner, check input processing, check sizing, check simulation results
    std::string const idf_objects = delimited_string({"Generator:WindTurbine,",
                                                      "    WT1,                     !- Name",
                                                      "    ,               !- Availability Schedule Name",
                                                      "    HorizontalAxisWindTurbine,  !- Rotor Type",
                                                      "    FixedSpeedVariablePitch, !- Power Control",
                                                      "    41,                      !- Rated Rotor Speed {rev/min}",
                                                      "    19.2,                    !- Rotor Diameter {m}",
                                                      "    30.5,                    !- Overall Height {m}",
                                                      "    3,                       !- Number of Blades",
                                                      "    55000,                   !- Rated Power {W}",
                                                      "    11,                      !- Rated Wind Speed {m/s}",
                                                      "    3.5,                     !- Cut In Wind Speed {m/s}",
                                                      "    25,                      !- Cut Out Wind Speed {m/s}",
                                                      "    0.835,                   !- Fraction system Efficiency",
                                                      "    8,                       !- Maximum Tip Speed Ratio",
                                                      "    0.5,                     !- Maximum Power Coefficient",
                                                      "    6.4,                     !- Annual Local Average Wind Speed {m/s}",
                                                      "    50,                      !- Height for Local Average Wind Speed {m}",
                                                      "    ,                        !- Blade Chord Area {m2}",
                                                      "    ,                        !- Blade Drag Coefficient",
                                                      "    ,                        !- Blade Lift Coefficient",
                                                      "    0.5176,                  !- Power Coefficient C1",
                                                      "    116,                     !- Power Coefficient C2",
                                                      "    0.4,                     !- Power Coefficient C3",
                                                      "    0,                       !- Power Coefficient C4",
                                                      "    5,                       !- Power Coefficient C5",
                                                      "    21;                      !- Power Coefficient C6"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    WindTurbine::GetWindTurbineInput(*state);
    int index = 0;

    // With the wind speed initialized to zero, the wind turbine will not produce any power
    WindTurbine::SimWindTurbine(*state, GeneratorType::WindTurbine, "WT1", index, false, 0.0);
    auto &thisTurbine = state->dataWindTurbine->WindTurbineSys[0];
    EXPECT_EQ(thisTurbine.Name, "WT1");
    EXPECT_EQ(thisTurbine.Power, 0);

    // Adding wind speed should turn it on
    state->dataEnvrn->WindSpeed = 10;
    WindTurbine::SimWindTurbine(*state, GeneratorType::WindTurbine, "WT1", index, false, 0.0);
    EXPECT_GT(thisTurbine.Power, 0);
}
