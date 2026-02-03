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
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::Fans;

TEST_F(EnergyPlusFixture, Fans_FanSizing)
{
    // state->init_state(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;

    auto *fan1 = new Fans::FanComponent;
    fan1->Name = "Test Fan";
    fan1->type = HVAC::FanType::OnOff;
    fan1->maxAirFlowRate = AutoSize;
    fan1->deltaPress = 500.0;
    fan1->totalEff = 0.4; // Prevent divide by zero computing RatedPower
    fan1->sizingPrefix = "Maximum Flow Rate";

    state->dataFans->fans.push_back(fan1);
    state->dataFans->fanMap.insert_or_assign(fan1->Name, state->dataFans->fans.size());

    state->dataEnvrn->StdRhoAir = 1.2;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;

    // DataNonZoneNonAirloopValue must be set when CurZoneEqNum and CurSysNum = 0
    state->dataSize->DataNonZoneNonAirloopValue = 1.00635;
    fan1->set_size(*state);
    EXPECT_DOUBLE_EQ(1.00635, fan1->maxAirFlowRate);
    state->dataSize->DataNonZoneNonAirloopValue = 0.0;
    EXPECT_NEAR(1.0352, fan1->designPointFEI, 0.0001);

    std::string eiooutput = std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                                        " Component Sizing Information, Fan:OnOff, Test Fan, Design Size Maximum Flow Rate [m3/s], 1.00635\n"
                                        " Component Sizing Information, Fan:OnOff, Test Fan, Design Electric Power Consumption [W], 1257.93750\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(EnergyPlusFixture, Fans_ConstantVolume_EMSPressureRiseResetTest)
{
    state->init_state(*state);

    // set standard air density
    state->dataEnvrn->StdRhoAir = 1.0;
    // set fan model inputs

    auto *fan1 = new Fans::FanComponent;
    fan1->Name = "Test Fan";
    fan1->type = HVAC::FanType::Constant;
    fan1->sizingPrefix = "Fan Total Efficiency"; // "Pressure Rise"
    fan1->maxAirFlowRate = AutoSize;
    fan1->deltaPress = 300.0;
    fan1->totalEff = 1.0;
    fan1->motorEff = 0.8;
    fan1->motorInAirFrac = 1.0;
    fan1->availSched = Sched::GetScheduleAlwaysOff(*state);
    fan1->maxAirFlowRate = 1.0;
    fan1->minAirMassFlowRate = 0.0;
    fan1->maxAirMassFlowRate = fan1->maxAirFlowRate;
    fan1->inletAirMassFlowRate = fan1->maxAirMassFlowRate;
    fan1->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    fan1->EMSPressureOverrideOn = false;
    fan1->EMSPressureValue = 0.0;

    state->dataHVACGlobal->TurnFansOn = true;
    state->dataHVACGlobal->TurnFansOff = false;
    // simulate the fan
    fan1->simulateConstant(*state);

    // fan power = MassFlow * DeltaPress / (FanEff * RhoAir)
    Real64 Result_FanPower = max(0.0, fan1->maxAirMassFlowRate * fan1->deltaPress / (fan1->totalEff * fan1->rhoAirStdInit));
    EXPECT_DOUBLE_EQ(Result_FanPower, fan1->totalPower); // expects 300 W

    // negative fan pressure rise set using EMS
    fan1->EMSPressureOverrideOn = true;
    fan1->EMSPressureValue = -300.0;
    // simulate the fan with negative pressure rise
    // set using fans EMS actuator for Pressure Rise
    fan1->simulateConstant(*state);
    Real64 Result2_FanPower = max(0.0, fan1->maxAirMassFlowRate * fan1->EMSPressureValue / (fan1->totalEff * fan1->rhoAirStdInit));
    EXPECT_DOUBLE_EQ(Result2_FanPower, fan1->totalPower); // expects zero
}

TEST_F(EnergyPlusFixture, Fans_OnOff_EMSPressureRiseResetTest)
{
    state->init_state(*state);
    // set standard air density
    state->dataEnvrn->StdRhoAir = 1.0;
    // set fan model inputs
    auto *fan1 = new Fans::FanComponent;

    fan1->Name = "Test Fan";
    fan1->type = HVAC::FanType::OnOff;
    fan1->sizingPrefix = "Fan Total Efficiency"; // "Pressure Rise"

    fan1->maxAirFlowRate = AutoSize;
    fan1->deltaPress = 300.0;
    fan1->totalEff = 1.0;
    fan1->motorEff = 0.8;
    fan1->motorInAirFrac = 1.0;
    fan1->availSched = Sched::GetScheduleAlwaysOff(*state);
    fan1->maxAirFlowRate = 1.0;
    fan1->minAirMassFlowRate = 0.0;
    fan1->maxAirMassFlowRate = fan1->maxAirFlowRate;
    fan1->inletAirMassFlowRate = fan1->maxAirMassFlowRate;
    fan1->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    fan1->EMSPressureOverrideOn = false;
    fan1->EMSPressureValue = 0.0;

    state->dataFans->fans.push_back(fan1);
    state->dataFans->fanMap.insert_or_assign(fan1->Name, state->dataFans->fans.size());

    state->dataHVACGlobal->TurnFansOn = true;
    state->dataHVACGlobal->TurnFansOff = false;
    // simulate the fan
    fan1->simulateOnOff(*state);
    // fan power = MassFlow * DeltaPress / (FanEff * RhoAir)
    Real64 Result_FanPower = max(0.0, fan1->maxAirMassFlowRate * fan1->deltaPress / (fan1->totalEff * fan1->rhoAirStdInit));
    EXPECT_DOUBLE_EQ(Result_FanPower, fan1->totalPower); // expects 300 W

    // negative fan pressure rise set using EMS
    fan1->EMSPressureOverrideOn = true;
    fan1->EMSPressureValue = -300.0;
    // simulate the fan with negative pressure rise
    // set using fans EMS actuator for Pressure Rise
    fan1->simulateOnOff(*state);
    Real64 Result2_FanPower = max(0.0, fan1->maxAirMassFlowRate * fan1->EMSPressureValue / (fan1->totalEff * fan1->rhoAirStdInit));
    EXPECT_DOUBLE_EQ(Result2_FanPower, fan1->totalPower); // expects zero
}

TEST_F(EnergyPlusFixture, Fans_VariableVolume_EMSPressureRiseResetTest)
{
    state->init_state(*state);
    // set standard air density
    state->dataEnvrn->StdRhoAir = 1.0;
    // set fan model inputs
    auto *fan1 = new Fans::FanComponent;
    fan1->Name = "Test Fan";
    fan1->type = HVAC::FanType::VAV;
    fan1->sizingPrefix = "Fan Total Efficiency"; // "Pressure Rise"
    fan1->maxAirFlowRate = AutoSize;
    fan1->deltaPress = 300.0;
    fan1->totalEff = 1.0;
    fan1->motorEff = 0.8;
    fan1->motorInAirFrac = 1.0;
    fan1->availSched = Sched::GetScheduleAlwaysOff(*state);
    fan1->maxAirFlowRate = 1.0;
    fan1->minAirMassFlowRate = 0.0;
    fan1->maxAirMassFlowRate = fan1->maxAirFlowRate;
    fan1->inletAirMassFlowRate = fan1->maxAirMassFlowRate;
    fan1->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    // VAV Fan Power Coefficients
    fan1->coeffs[0] = 0.06990146;
    fan1->coeffs[1] = 1.39500612;
    fan1->coeffs[2] = -3.35487336;
    fan1->coeffs[3] = 2.89232315;
    fan1->coeffs[4] = 0.000;
    fan1->EMSPressureOverrideOn = false;
    fan1->EMSPressureValue = 0.0;

    state->dataFans->fans.push_back(fan1);
    state->dataFans->fanMap.insert_or_assign(fan1->Name, state->dataFans->fans.size());

    state->dataHVACGlobal->TurnFansOn = true;
    state->dataHVACGlobal->TurnFansOff = false;
    // simulate the fan
    fan1->simulateVAV(*state);
    // fan power = PartLoadFrac * MassFlow * DeltaPress / (FanEff * RhoAir)
    Real64 FlowRatio = 1.0;
    Real64 PartLoadFrac =
        fan1->coeffs[0] + fan1->coeffs[1] * FlowRatio + fan1->coeffs[2] * FlowRatio * FlowRatio + fan1->coeffs[3] * FlowRatio * FlowRatio * FlowRatio;

    Real64 Result_FanPower = max(0.0, PartLoadFrac * fan1->maxAirMassFlowRate * fan1->deltaPress / (fan1->totalEff * fan1->rhoAirStdInit));
    EXPECT_DOUBLE_EQ(Result_FanPower, fan1->totalPower); // expects 300 W

    // negative fan pressure rise set using EMS
    fan1->EMSPressureOverrideOn = true;
    fan1->EMSPressureValue = -300.0;
    // simulate the fan with negative pressure rise
    // set using fans EMS actuator for Pressure Rise
    fan1->simulateVAV(*state);
    Real64 Result2_FanPower = max(0.0, PartLoadFrac * fan1->maxAirMassFlowRate * fan1->EMSPressureValue / (fan1->totalEff * fan1->rhoAirStdInit));
    EXPECT_DOUBLE_EQ(Result2_FanPower, fan1->totalPower); // expects zero
}
