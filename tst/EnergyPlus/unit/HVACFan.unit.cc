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

// EnergyPlus::Fans Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>

namespace EnergyPlus {

using namespace EnergyPlus::EMSManager;

TEST_F(EnergyPlusFixture, SystemFanObj_TestGetFunctions1)
{
    // this unit test checks some get functions
    // the idea is to set up a fan and run its sizing routine
    // the size is filled by the value in DataSizing::DataNonZoneNonAirloopValue
    // then check three getter functions
    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                    !- Design Electric Power Consumption",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                            !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    0.50;                        !- Fan Total Efficiency",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);

    fanSystem->simulate(*state, false, _, _);          // triggers sizing call
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate; // get function
    EXPECT_NEAR(1.0000, locFanSizeVdot, 0.00000001);
    Real64 locDesignTempRise = fanSystem->getDesignTemperatureRise(*state);
    EXPECT_NEAR(locDesignTempRise, 0.166, 0.001);
    Real64 locDesignHeatGain = fanSystem->getDesignHeatGain(*state, locFanSizeVdot);
    EXPECT_NEAR(locDesignHeatGain, 200.0, 0.1);
    EXPECT_FALSE(fanSystem->speedControl == Fans::SpeedControl::Continuous);
}

TEST_F(EnergyPlusFixture, SystemFanObj_FanSizing1)
{
    // this unit test mimics "EnergyPlusFixture.Fans_FanSizing"
    // the idea is to set up a fan and run its sizing routine
    // the size is filled by the value in DataSizing::DataNonZoneNonAirloopValue
    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    AUTOSIZE ,                   !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    75.0,                        !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                    !- Design Electric Power Consumption",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                            !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    0.50;                        !- Fan Total Efficiency",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    state->dataEnvrn->StdRhoAir = 1.0;
    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->DataNonZoneNonAirloopValue = 1.00635;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);

    fanSystem->simulate(*state, false, _, _);          // triggers sizing call
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate; // get function
    EXPECT_NEAR(1.00635, locFanSizeVdot, 0.00001);
    state->dataSize->DataNonZoneNonAirloopValue = 0.0;
}

TEST_F(EnergyPlusFixture, SystemFanObj_TwoSpeedFanPowerCalc1)
{
    // this unit test checks the power averaging when cycling between a hi and low speed
    // this uses discrete power fractions at the speed levels
    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    100.0,                    !- Design Electric Power Consumption",
        "    ,                      !- Design Power Sizing Method",
        "    ,                      !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    ,                        !- Fan Total Efficiency",
        "  , !- Electric Power Function of Flow Fraction Curve Name",
        "  , !- Night Ventilation Mode Pressure Rise",
        "  , !- Night Ventilation Mode Flow Fraction",
        "  , !- Motor Loss Zone Name",
        "  , !- Motor Loss Radiative Fraction ",
        "  Fan Energy, !- End-Use Subcategory",
        "  2, !- Number of Speeds",
        "  0.5, !- Speed 1 Flow Fraction",
        "  0.125, !- Speed 1 Electric Power Fraction",
        "  1.0, !- Speed 2 Flow Fraction",
        "  1.0; !- Speed 2 Electric Power Fraction",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);

    fanSystem->simulate(*state, false, _, _);
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate; // get function
    EXPECT_NEAR(1.00, locFanSizeVdot, 0.00001);

    fanSystem->simulate(*state, false, _, _, 0.75); // call for flow fraction of 0.75
    Real64 locFanElecPower = fanSystem->totalPower;
    Real64 locExpectPower = (0.5 * 0.125 * 100.0) + (0.5 * 1.0 * 100.0);
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    fanSystem->simulate(*state, false, _, _, 0.5); // call for flow fraction of 0.5
    locFanElecPower = fanSystem->totalPower;
    locExpectPower = 0.125 * 100.0;
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);
}

TEST_F(EnergyPlusFixture, SystemFanObj_TwoSpeedFanPowerCalc2)
{
    // this unit test checks the power averaging when cycling between a hi and low speed
    // this uses fan power curve instead of speed level power fractions

    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    100.0,                    !- Design Electric Power Consumption",
        "    ,                      !- Design Power Sizing Method",
        "    ,                      !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    ,                        !- Fan Total Efficiency",
        "  simple cubic, !- Electric Power Function of Flow Fraction Curve Name",
        "  , !- Night Ventilation Mode Pressure Rise",
        "  , !- Night Ventilation Mode Flow Fraction",
        "  , !- Motor Loss Zone Name",
        "  , !- Motor Loss Radiative Fraction ",
        "  Fan Energy, !- End-Use Subcategory",
        "  2, !- Number of Speeds",
        "  0.5, !- Speed 1 Flow Fraction",
        "  , !- Speed 1 Electric Power Fraction",
        "  1.0, !- Speed 2 Flow Fraction",
        "  ; !- Speed 2 Electric Power Fraction",

        "  Curve:Cubic,",
        "    simple cubic,  !- Name",
        "    0.0,                    !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    1.0,                    !- Coefficient4 x**3",
        "    0.0,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.0,                     !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);

    fanSystem->simulate(*state, false, _, _);
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate;
    EXPECT_NEAR(1.00, locFanSizeVdot, 0.00001);

    fanSystem->simulate(*state, false, _, _, 0.75); // call for flow fraction of 0.75
    Real64 locFanElecPower = fanSystem->totalPower;
    Real64 locExpectPower = (0.5 * 0.125 * 100.0) + (0.5 * 1.0 * 100.0);
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    fanSystem->simulate(*state, false, _, _, 0.5); // call for flow fraction of 0.5
    locFanElecPower = state->dataFans->fans(1)->totalPower;
    locExpectPower = 0.125 * 100.0;
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);
}

TEST_F(EnergyPlusFixture, SystemFanObj_TwoSpeedFanPowerCalc3)
{
    // this unit test checks the power averaging when cycling between a hi and low speed
    // this uses discrete power fractions at the speed levels
    // this unit test uses the optional two-mode arguments
    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    100.0,                    !- Design Electric Power Consumption",
        "    ,                      !- Design Power Sizing Method",
        "    ,                      !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    ,                        !- Fan Total Efficiency",
        "  , !- Electric Power Function of Flow Fraction Curve Name",
        "  , !- Night Ventilation Mode Pressure Rise",
        "  , !- Night Ventilation Mode Flow Fraction",
        "  , !- Motor Loss Zone Name",
        "  , !- Motor Loss Radiative Fraction ",
        "  Fan Energy, !- End-Use Subcategory",
        "  2, !- Number of Speeds",
        "  0.5, !- Speed 1 Flow Fraction",
        "  0.125, !- Speed 1 Electric Power Fraction",
        "  1.0, !- Speed 2 Flow Fraction",
        "  1.0; !- Speed 2 Electric Power Fraction",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);

    fanSystem->simulate(*state, false, _, _);
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate; // get function
    EXPECT_NEAR(1.00, locFanSizeVdot, 0.00001);

    // 50% of the time at speed 1 (0.5 flow) and 50% of the time at speed 2 (1.0 flow), average flow 0.75, on for entire timestep
    Real64 designMassFlowRate = locFanSizeVdot * state->dataEnvrn->StdRhoAir;
    Real64 massFlow1 = 0.5 * designMassFlowRate;
    Real64 massFlow2 = designMassFlowRate;
    Real64 runTimeFrac1 = 0.5;
    Real64 runTimeFrac2 = 0.5;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    Real64 locFanElecPower = fanSystem->totalPower;
    Real64 locExpectPower = (runTimeFrac1 * 0.125 * 100.0) + (runTimeFrac2 * 1.0 * 100.0);
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 100% of the time at average flow 0.75, on for entire timestep
    massFlow1 = 0.0;
    massFlow2 = 0.75 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 1.0;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    // locExpectPower expect the same power as the previous case
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 100% of the time at full flow, on for the entire timestep
    massFlow1 = 0.0;
    massFlow2 = 1.0 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 1.0;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    locExpectPower = fanSystem->designElecPower; // expect full power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 85% of the time at full flow, on for the entire timestep
    massFlow1 = 0.0;
    massFlow2 = 1.0 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 0.85;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    locExpectPower = 0.85 * fanSystem->designElecPower; // expect 85% of full power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // reverse the 1 and 2 arguments, expect the same result
    fanSystem->simulate(*state, false, _, _, _, massFlow2, runTimeFrac2, massFlow1, runTimeFrac1);
    locFanElecPower = fanSystem->totalPower;
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);
}

TEST_F(EnergyPlusFixture, SystemFanObj_TwoSpeedFanPowerCalc4)
{
    // this unit test checks the power averaging when cycling between a hi and low speed
    // this uses fan power curve instead of speed level power fractions
    // this unit test uses the optional two-mode arguments

    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Continuous ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    100.0,                    !- Design Electric Power Consumption",
        "    ,                      !- Design Power Sizing Method",
        "    ,                      !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    ,                        !- Fan Total Efficiency",
        "  simple cubic; !- Electric Power Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    simple cubic,  !- Name",
        "    0.0,                    !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    1.0,                    !- Coefficient4 x**3",
        "    0.0,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.0,                     !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    fanSystem->simulate(*state, false, _, _);
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate;
    EXPECT_NEAR(1.00, locFanSizeVdot, 0.00001);

    // 50% of the time at speed 1 (0.5 flow) and 50% of the time at speed 2 (1.0 flow), average flow 0.75, on for entire timestep
    Real64 designMassFlowRate = locFanSizeVdot * state->dataEnvrn->StdRhoAir;
    Real64 massFlow1 = 0.5 * designMassFlowRate;
    Real64 massFlow2 = designMassFlowRate;
    Real64 runTimeFrac1 = 0.5;
    Real64 runTimeFrac2 = 0.5;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    Real64 locFanElecPower = fanSystem->totalPower;
    Real64 locExpectPower = (0.5 * pow(0.5, 3) + 0.5 * 1.0) * fanSystem->designElecPower;
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 100% of the time at average flow 0.75, on for entire timestep
    massFlow1 = 0.0;
    massFlow2 = 0.75 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 1.0;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    locExpectPower = pow(0.75, 3) * fanSystem->designElecPower;
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 100% of the time at full flow, on for the entire timestep
    massFlow1 = 0.0;
    massFlow2 = 1.0 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 1.0;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    locExpectPower = fanSystem->designElecPower; // expect full power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 85% of the time at full flow, on for the entire timestep
    massFlow1 = 0.0;
    massFlow2 = 1.0 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 0.85;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    locExpectPower = 0.85 * fanSystem->designElecPower; // expect 85% of full power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);
}

TEST_F(EnergyPlusFixture, SystemFanObj_FanEnergyIndex)
{
    // this unit test checks the functions calculating FEI
    // expanded for test of correction to defect #10509
    Real64 designFlow;
    Real64 designPower;
    Real64 designDeltaP;
    Real64 testFEI;
    Real64 expectedAnswer;
    Real64 constexpr allowedTolerance = 0.001;

    // Test 1: (already existing) normal operation of function
    designFlow = 1.0;
    designPower = 1000.0;
    designDeltaP = 100.0;
    expectedAnswer = 0.4892;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 2a: Zero power-->should result in zero result
    designFlow = 1.0;
    designPower = 0.0;
    designDeltaP = 100.0;
    expectedAnswer = 0.0;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 2b: Negative power-->should also result in zero result
    designFlow = 1.0;
    designPower = -1000.0;
    designDeltaP = 100.0;
    expectedAnswer = 0.0;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 3: Lower design power-->higher FEI
    designFlow = 1.0;
    designPower = 500.0;
    designDeltaP = 100.0;
    expectedAnswer = 0.9784;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 4: Lower flow rate-->lower FEI
    designFlow = 0.5;
    designPower = 1000.0;
    designDeltaP = 100.0;
    expectedAnswer = 0.2990;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 5: Lower pressure differential-->lower FEI
    designFlow = 1.0;
    designPower = 1000.0;
    designDeltaP = 50.0;
    expectedAnswer = 0.3833;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 6: Test at altitude (roughly Denver at a pressure of 82 kPa)-->slightly lower FEI if power stays the same
    state->dataEnvrn->StdBaroPress = 82000.0;
    designFlow = 1.0;
    designPower = 1000.0;
    designDeltaP = 100.0;
    expectedAnswer = 0.4491;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);

    // Test 7: High values to trigger high motor efficiency limit
    state->dataEnvrn->StdBaroPress = 82000.0;
    designFlow = 100.0;
    designPower = 5000000.0;
    designDeltaP = 10000.0;
    expectedAnswer = 0.3311;
    testFEI = Fans::FanSystem::report_fei(*state, designFlow, designPower, designDeltaP);
    EXPECT_NEAR(testFEI, expectedAnswer, allowedTolerance);
}

TEST_F(EnergyPlusFixture, SystemFanObj_DiscreteMode_noPowerFFlowCurve)
{
    // this unit test checks the power averaging when cycling between a hi and low speed
    // this uses fan speed level power fractions
    // this unit test uses the optional two-mode arguments

    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan ,              !- Name",
        "    ,                       !- Availability Schedule Name",
        "    TestFanAirInletNode,    !- Air Inlet Node Name",
        "    TestFanOutletNode,      !- Air Outlet Node Name",
        "    1.0 ,                   !- Design Maximum Air Flow Rate",
        "    Discrete,               !- Speed Control Method",
        "    0.0,                    !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                  !- Design Pressure Rise",
        "    0.9 ,                   !- Motor Efficiency",
        "    1.0 ,                   !- Motor In Air Stream Fraction",
        "    100.0,                  !- Design Electric Power Consumption",
        "    ,                       !- Design Power Sizing Method",
        "    ,                       !- Electric Power Per Unit Flow Rate",
        "    ,                       !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    ,                       !- Fan Total Efficiency",
        "    ;                       !- Electric Power Function of Flow Fraction Curve Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);
    fanSystem->simulate(*state, false, _, _);
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate;
    EXPECT_NEAR(1.00, locFanSizeVdot, 0.00001);

    // 50% of the time at speed 1 (0.5 flow) and 50% of the time at speed 2 (1.0 flow), average flow 0.75, on for entire timestep
    Real64 designMassFlowRate = locFanSizeVdot * state->dataEnvrn->StdRhoAir;
    Real64 massFlow1 = 0.5 * designMassFlowRate;
    Real64 massFlow2 = designMassFlowRate;
    Real64 runTimeFrac1 = 0.5;
    Real64 runTimeFrac2 = 0.5;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    Real64 locFanElecPower = fanSystem->totalPower;
    // uses flow weighted power calculation. 50% of time at 50% flow and 50% of time at 100% flow
    Real64 locExpectPower = (0.5 * 0.5 + 0.5 * 1.0) * fanSystem->designElecPower; // expect 75% of power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 100% of the time at average flow 0.75, on for entire timestep
    massFlow1 = 0.0;
    massFlow2 = 0.75 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 1.0;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    // uses flow weighted power calculation. 0% of time at 0% flow and 100% of time at 75% flow
    locExpectPower = (0.0 * 0.0 + 1.0 * 0.75) * fanSystem->designElecPower; // expect 75% of power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 100% of the time at full flow, on for the entire timestep
    massFlow1 = 0.0;
    massFlow2 = 1.0 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 1.0;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    // uses flow weighted power calculation. 0% of time at 0% flow and 100% of time at 100% flow
    locExpectPower = (0.0 * 0.0 + 1.0 * 1.0) * fanSystem->designElecPower; // expect full power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // 85% of the time at full flow, on for the entire timestep
    massFlow1 = 0.0;
    massFlow2 = 1.0 * designMassFlowRate;
    runTimeFrac1 = 0.0;
    runTimeFrac2 = 0.85;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    // uses flow weighted power calculation. 0% of time at 0% flow and 85% of time at 100% flow
    locExpectPower = (0.0 * 0.25 + 0.85 * 1.0) * fanSystem->designElecPower; // expect 85% of full power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);
}

TEST_F(EnergyPlusFixture, SystemFanObj_DiscreteMode_EMSPressureRiseResetTest)
{
    // this unit test checks the power when EMS actuator is used to reset pressure rise
    // adopted from unit test "SystemFanObj_DiscreteMode_noPowerFFlowCurve" and modified

    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan,                       !- Name",
        "    ,                               !- Availability Schedule Name",
        "    TestFanAirInletNode,            !- Air Inlet Node Name",
        "    TestFanOutletNode,              !- Air Outlet Node Name",
        "    1.0 ,                           !- Design Maximum Air Flow Rate",
        "    Discrete,                       !- Speed Control Method",
        "    0.0,                            !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                          !- Design Pressure Rise",
        "    0.9 ,                           !- Motor Efficiency",
        "    1.0 ,                           !- Motor In Air Stream Fraction",
        "    100.0,                          !- Design Electric Power Consumption",
        "    ,                               !- Design Power Sizing Method",
        "    ,                               !- Electric Power Per Unit Flow Rate",
        "    ,                               !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    ,                               !- Fan Total Efficiency",
        "    ;                               !- Electric Power Function of Flow Fraction Curve Name",

        "    EnergyManagementSystem:Actuator,",
        "      FanPressureRise_ResetValue,   !- Name",
        "      TEST FAN,                     !- Actuated Component Unique Name",
        "      Fan,                          !- Actuated Component Type",
        "      Fan Pressure Rise;            !- Actuated Component Control Type",

        "    EnergyManagementSystem:Program,",
        "      ResetFanPressureRise,                     !- Name",
        "      SET FanPressureRise_ResetValue = -100.0;  !- <none>",

        "    EnergyManagementSystem:ProgramCallingManager,",
        "      FanSystemModel_FanMainManager,  !- Name",
        "      BeginTimestepBeforePredictor,   !- EnergyPlus Model Calling Point",
        "      ResetFanPressureRise;           !- Program Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.0;

    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state->dataFans->fans(1));
    assert(fanSystem != nullptr);

    fanSystem->simulate(*state, false, _, _);
    Real64 locFanSizeVdot = fanSystem->maxAirFlowRate;
    EXPECT_NEAR(1.00, locFanSizeVdot, 0.00001);

    // 50% of the time at speed 1 (0.5 flow) and 50% of the time at speed 2 (1.0 flow)
    // average flow 0.75, on for entire timestep
    Real64 designMassFlowRate = locFanSizeVdot * state->dataEnvrn->StdRhoAir;
    Real64 massFlow1 = 0.5 * designMassFlowRate;
    Real64 massFlow2 = designMassFlowRate;
    Real64 runTimeFrac1 = 0.5;
    Real64 runTimeFrac2 = 0.5;
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    Real64 locFanElecPower = fanSystem->totalPower;
    // uses flow weighted power calculation. 50% of time at 50% flow and 50% of time at 100% flow
    Real64 locExpectPower = (0.5 * 0.5 + 0.5 * 1.0) * fanSystem->designElecPower; // expect 75% of power
    EXPECT_NEAR(locFanElecPower, locExpectPower, 0.01);

    // reset the pressure rise to -100.0 using EMS program
    bool anyRan(false);
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyRan);
    // simulate the fan with -100.0 Pa fan pressure rise
    fanSystem->simulate(*state, false, _, _, _, massFlow1, runTimeFrac1, massFlow2, runTimeFrac2);
    locFanElecPower = fanSystem->totalPower;
    // negative fan pressure rise results in zero fan power
    locExpectPower = 0.0;
    EXPECT_DOUBLE_EQ(locFanElecPower, locExpectPower); // expects zero fan power
}
} // namespace EnergyPlus
