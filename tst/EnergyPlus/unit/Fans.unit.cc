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

// EnergyPlus::Fans Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/ScheduleManager.hh>

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
                                        " Component Sizing Information, Fan:OnOff, Test Fan, Design Electric Power Consumption [W], 1257.94\n");
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

// Verify alternate night performance across ConstantVolume, VAV, OnOff, and ZoneExhaust fan models.
TEST_F(EnergyPlusFixture, Fans_NightVentilationPerformanceAcrossFanTypes)
{
    // Alternate night performance changes fan pressure, efficiency, and heat without limiting requested airflow.
    state->init_state(*state);
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataHVACGlobal->NightVentOn = true;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
    state->dataFans->NightVentPerf.allocate(1);
    auto &nightVentPerf = state->dataFans->NightVentPerf(1);
    nightVentPerf.DeltaPress = 100.0;
    nightVentPerf.FanEff = 0.5;
    nightVentPerf.MotEff = 0.6;
    nightVentPerf.MotInAirFrac = 0.25;

    auto configureFan = [&](HVAC::FanType type) {
        auto fan = std::make_unique<Fans::FanComponent>();
        fan->Name = "Night Vent Fan";
        fan->type = type;
        fan->availSched = Sched::GetScheduleAlwaysOff(*state);
        fan->maxAirFlowRate = 1.0;
        fan->maxAirMassFlowRate = 1.0;
        fan->inletAirMassFlowRate = 1.0;
        fan->rhoAirStdInit = 1.0;
        fan->deltaPress = 300.0;
        fan->totalEff = 0.7;
        fan->motorEff = 0.9;
        fan->motorInAirFrac = 1.0;
        fan->nightVentPerfNum = 1;
        return fan;
    };

    // Constant-volume flow remains at its 1.0 kg/s request while night power uses the alternate settings.
    auto constantFan = configureFan(HVAC::FanType::Constant);
    constantFan->simulateConstant(*state);
    EXPECT_DOUBLE_EQ(1.0, constantFan->outletAirMassFlowRate);
    EXPECT_DOUBLE_EQ(200.0, constantFan->totalPower);
    EXPECT_DOUBLE_EQ(140.0, constantFan->powerLossToAir);

    auto variableFan = configureFan(HVAC::FanType::VAV);
    variableFan->coeffs = {0.25, 0.0, 0.0, 0.0, 0.0};
    variableFan->inletAirMassFlowRate = 0.3;
    // Night ventilation assumes fixed speed, so it must bypass the normal VAV part-load power curve.
    variableFan->simulateVAV(*state);
    EXPECT_DOUBLE_EQ(0.3, variableFan->outletAirMassFlowRate);
    EXPECT_DOUBLE_EQ(60.0, variableFan->totalPower);
    EXPECT_DOUBLE_EQ(42.0, variableFan->powerLossToAir);

    auto onOffFan = configureFan(HVAC::FanType::OnOff);
    auto *powerRatioCurve = Curve::AddCurve(*state, "Night Vent Power Ratio");
    powerRatioCurve->curveType = Curve::CurveType::Quadratic;
    powerRatioCurve->coeff[0] = 0.5;
    powerRatioCurve->inputLimits[0].min = 0.0;
    powerRatioCurve->inputLimits[0].max = 1.0;
    onOffFan->powerRatioAtSpeedRatioCurveNum = powerRatioCurve->Num;

    auto *efficiencyRatioCurve = Curve::AddCurve(*state, "Night Vent Efficiency Ratio");
    efficiencyRatioCurve->curveType = Curve::CurveType::Quadratic;
    efficiencyRatioCurve->coeff[0] = 0.8;
    efficiencyRatioCurve->inputLimits[0].min = 0.0;
    efficiencyRatioCurve->inputLimits[0].max = 1.0;
    onOffFan->effRatioCurveNum = efficiencyRatioCurve->Num;

    // Normal operation must continue to apply the parent speed ratio and its curves.
    state->dataHVACGlobal->NightVentOn = false;
    onOffFan->simulateOnOff(*state, 0.5);
    Real64 constexpr normalPowerBeforeCurves = 300.0 / 0.7;
    EXPECT_DOUBLE_EQ(1.0, onOffFan->outletAirMassFlowRate);
    EXPECT_NEAR(normalPowerBeforeCurves * 0.5 / 0.8, onOffFan->totalPower, 0.0001);
    EXPECT_DOUBLE_EQ(onOffFan->totalPower, onOffFan->powerLossToAir);

    // Night ventilation is fixed-speed, so the parent speed ratio and curves must not modify night power.
    state->dataHVACGlobal->NightVentOn = true;
    onOffFan->simulateOnOff(*state, 0.5);
    EXPECT_DOUBLE_EQ(1.0, onOffFan->outletAirMassFlowRate);
    EXPECT_DOUBLE_EQ(200.0, onOffFan->totalPower);
    EXPECT_DOUBLE_EQ(140.0, onOffFan->powerLossToAir);

    auto exhaustFan = configureFan(HVAC::FanType::Exhaust);
    exhaustFan->availManagerMode = Fans::AvailManagerMode::Coupled;
    exhaustFan->simulateZoneExhaust(*state);
    // Zone exhaust fan heat uses air power; its night motor efficiency does not alter the heat calculation.
    EXPECT_DOUBLE_EQ(1.0, exhaustFan->outletAirMassFlowRate);
    EXPECT_DOUBLE_EQ(200.0, exhaustFan->totalPower);
    EXPECT_DOUBLE_EQ(125.0, exhaustFan->powerLossToAir);
}

// Verify fouled-filter pressure scaling uses the selected night pressure when night mode is active.
TEST_F(EnergyPlusFixture, Fans_NightVentilationFouledFilterUsesNightPressure)
{
    state->init_state(*state);
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataHVACGlobal->NightVentOn = true;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataFans->NightVentPerf.allocate(1);
    auto &nightVentPerf = state->dataFans->NightVentPerf(1);
    nightVentPerf.DeltaPress = 500.0;
    nightVentPerf.FanEff = 0.5;
    nightVentPerf.MotEff = 0.9;
    nightVentPerf.MotInAirFrac = 1.0;

    auto *fanCurve = Curve::AddCurve(*state, "Night Pressure Fan Curve");
    fanCurve->curveType = Curve::CurveType::Quadratic;
    fanCurve->coeff[0] = 800.0;
    fanCurve->coeff[1] = -500.0;
    fanCurve->inputLimits[0].min = 0.0;
    fanCurve->inputLimits[0].max = 1.0;

    Sched::ScheduleConstant faultAvailableSchedule;
    faultAvailableSchedule.currentVal = 1.0;
    Sched::ScheduleConstant pressureFractionSchedule;
    pressureFractionSchedule.currentVal = 1.1;
    state->dataFaultsMgr->FaultsFouledAirFilters.allocate(1);
    auto &fault = state->dataFaultsMgr->FaultsFouledAirFilters(1);
    fault.availSched = &faultAvailableSchedule;
    fault.pressFracSched = &pressureFractionSchedule;
    fault.fanCurveNum = fanCurve->Num;

    for (HVAC::FanType const fanType : {HVAC::FanType::Constant, HVAC::FanType::VAV, HVAC::FanType::OnOff}) {
        Fans::FanComponent fan;
        fan.Name = "Night Vent Fan";
        fan.type = fanType;
        fan.availSched = Sched::GetScheduleAlwaysOff(*state);
        fan.maxAirFlowRate = 1.0;
        fan.maxAirMassFlowRate = 1.0;
        fan.inletAirMassFlowRate = 1.0;
        fan.rhoAirStdInit = 1.0;
        fan.deltaPress = 300.0;
        fan.totalEff = 0.6;
        fan.motorEff = 0.9;
        fan.motorInAirFrac = 1.0;
        fan.nightVentPerfNum = 1;
        fan.faultyFilterFlag = true;
        fan.faultyFilterIndex = 1;

        switch (fanType) {
        case HVAC::FanType::Constant:
            fan.simulateConstant(*state);
            break;
        case HVAC::FanType::VAV:
            fan.simulateVAV(*state);
            break;
        case HVAC::FanType::OnOff:
            fan.simulateOnOff(*state);
            break;
        default:
            FAIL() << "Unexpected fan type in night fouled-filter test";
        }

        EXPECT_NEAR(0.94, fan.outletAirMassFlowRate, 0.01);
        EXPECT_NEAR(fan.outletAirMassFlowRate * 550.0 / 0.5, fan.totalPower, 0.01);
    }
}

// Verify VAV night power follows actual flow below the normal low-flow heat-safeguard threshold.
TEST_F(EnergyPlusFixture, Fans_VAVNightVentilationPowerScalesWithLowFlow)
{
    state->init_state(*state);
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataHVACGlobal->NightVentOn = true;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataFans->NightVentPerf.allocate(1);
    auto &nightVentPerf = state->dataFans->NightVentPerf(1);
    nightVentPerf.DeltaPress = 20000.0;
    nightVentPerf.FanEff = 0.5;
    nightVentPerf.MotEff = 1.0;
    nightVentPerf.MotInAirFrac = 1.0;

    Fans::FanComponent fan;
    fan.Name = "Low Flow Night Vent VAV Fan";
    fan.type = HVAC::FanType::VAV;
    fan.availSched = Sched::GetScheduleAlwaysOff(*state);
    fan.maxAirFlowRate = 1.0;
    fan.maxAirMassFlowRate = 1.0;
    fan.minAirFlowRate = 0.0;
    fan.inletAirMassFlowRate = 0.005;
    fan.inletAirTemp = 20.0;
    fan.inletAirHumRat = 0.008;
    fan.inletAirEnthalpy = 40000.0;
    fan.rhoAirStdInit = 1.0;
    fan.deltaPress = 300.0;
    fan.totalEff = 0.6;
    fan.motorEff = 1.0;
    fan.motorInAirFrac = 1.0;
    fan.nightVentPerfNum = 1;
    fan.coeffs = {0.25, 0.0, 0.0, 0.0, 0.0};

    fan.simulateVAV(*state);

    EXPECT_DOUBLE_EQ(0.005, fan.outletAirMassFlowRate);
    EXPECT_NEAR(200.0, fan.totalPower, 0.0001);
}
