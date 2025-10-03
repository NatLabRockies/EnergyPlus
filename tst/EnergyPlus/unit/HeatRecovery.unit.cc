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

// EnergyPlus::Heat Recovery Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlus/DataAirLoop.hh"
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReturnAirPathManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SimulationManager.hh>

using namespace EnergyPlus;
using namespace DataEnvironment;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataAirSystems;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::HeatRecovery;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SimAirServingZones;
using namespace EnergyPlus::ReturnAirPathManager;
using namespace EnergyPlus::SimulationManager;

TEST_F(EnergyPlusFixture, HeatRecovery_HRTest)
{
    state->init_state(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataHeatRecovery->NumHeatExchangers = 1;
    state->dataHeatRecovery->ExchCond.allocate(state->dataHeatRecovery->NumHeatExchangers);
    state->dataLoopNodes->Node.allocate(4);
    state->dataEnvrn->OutBaroPress = 101325.0;

    int ExchNum = 1;
    int CompanionCoilNum = 0;
    bool HXUnitOn = false;
    bool FirstHVACIteration = false;
    bool EconomizerFlag = false;
    bool HighHumCtrlFlag = false;
    HVAC::FanOp fanOp = HVAC::FanOp::Continuous; // 1 = cycling fan, 2 = constant fan
    Real64 Toutlet = 0.0;
    Real64 Tnode = 0.0;
    Real64 SetPointTemp = 19.0;
    Real64 PartLoadRatio = 0.25;
    int BalDesDehumPerfDataIndex = 1;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;

    state->dataHeatRecovery->ExchCond(ExchNum).NomSupAirVolFlow = 1.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInMassFlow = 1.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SecInMassFlow = 1.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode = 1;
    state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode = 2;
    state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode = 3;
    state->dataHeatRecovery->ExchCond(ExchNum).SecOutletNode = 4;
    state->dataHeatRecovery->ExchCond(ExchNum).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataHeatRecovery->ExchCond(ExchNum).HeatEffectSensible100 = 0.75;
    state->dataHeatRecovery->ExchCond(ExchNum).HeatEffectLatent100 = 0.0;
    state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensible100 = 0.75;
    state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectLatent100 = 0.0;
    state->dataHeatRecovery->ExchCond(ExchNum).HeatEffectSensibleCurveIndex = 0;
    state->dataHeatRecovery->ExchCond(ExchNum).HeatEffectLatentCurveIndex = 0;
    state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensibleCurveIndex = 0;
    state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectLatentCurveIndex = 0;

    state->dataHeatRecovery->ExchCond(ExchNum).Name = "Test Heat Recovery 1";
    state->dataHeatRecovery->ExchCond(ExchNum).type = HVAC::HXType::AirToAir_SensAndLatent;
    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HeatRecovery::HXExchConfigType::Rotary;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp = 24.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp = 15.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInHumRat = 0.01;
    state->dataHeatRecovery->ExchCond(ExchNum).SecInHumRat = 0.01;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInEnth =
        PsyHFnTdbW(state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp, state->dataHeatRecovery->ExchCond(ExchNum).SupInHumRat);
    state->dataHeatRecovery->ExchCond(ExchNum).SecInEnth =
        PsyHFnTdbW(state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp, state->dataHeatRecovery->ExchCond(ExchNum).SecInHumRat);
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).Temp = state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).Temp = state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).HumRat =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInHumRat;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).HumRat =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInHumRat;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).Enthalpy =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInEnth;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).Enthalpy =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInEnth;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).MassFlowRate =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInMassFlow;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).MassFlowRate =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInMassFlow;

    state->dataHeatRecovery->ExchCond(ExchNum).NumericFieldNames.allocate(5);
    state->dataHeatRecovery->BalDesDehumPerfData.allocate(BalDesDehumPerfDataIndex);
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NumericFieldNames.allocate(2);

    auto &thisHX = state->dataHeatRecovery->ExchCond(ExchNum);
    // HXUnitOn is false so expect outlet = inlet
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    Toutlet = state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp;
    Tnode = state->dataHeatRecovery->ExchCond(ExchNum).SupOutTemp;
    EXPECT_DOUBLE_EQ(Toutlet, Tnode);

    state->dataHeatRecovery->ExchCond(ExchNum).ControlToTemperatureSetPoint = false;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint = SetPointTemp;

    // HXUnitOn is true and ControlToTemperatureSetPoint is false so expect outlet = temperature based on effectiveness
    HXUnitOn = true;
    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Plate;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    Toutlet = (state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp +
               (state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensible100 *
                (state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp - state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp)));
    Tnode = state->dataHeatRecovery->ExchCond(ExchNum).SupOutTemp;
    EXPECT_DOUBLE_EQ(Toutlet, Tnode);

    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Rotary;
    HXUnitOn = true;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    Toutlet = (state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp +
               (state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensible100 *
                (state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp - state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp)));
    Tnode = state->dataHeatRecovery->ExchCond(ExchNum).SupOutTemp;
    EXPECT_DOUBLE_EQ(Toutlet, Tnode);

    state->dataHeatRecovery->ExchCond(ExchNum).ControlToTemperatureSetPoint = true;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint = 19.0;

    // HXUnitOn is true and ControlToTemperatureSetPoint is true so expect outlet = set point temperature
    HXUnitOn = true;
    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Plate;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    Toutlet = SetPointTemp;
    Tnode = state->dataHeatRecovery->ExchCond(ExchNum).SupOutTemp;
    EXPECT_DOUBLE_EQ(Toutlet, Tnode);

    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Rotary;
    HXUnitOn = true;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    Toutlet = state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint;
    Tnode = state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp;
    EXPECT_DOUBLE_EQ(Toutlet, Tnode);

    state->dataHeatRecovery->ExchCond(ExchNum).Name = "Test Heat Recovery 2";
    state->dataHeatRecovery->ExchCond(ExchNum).type = HVAC::HXType::AirToAir_SensAndLatent;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp = 15.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp = 24.0;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInHumRat = 0.01;
    state->dataHeatRecovery->ExchCond(ExchNum).SecInHumRat = 0.01;
    state->dataHeatRecovery->ExchCond(ExchNum).SupInEnth =
        PsyHFnTdbW(state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp, state->dataHeatRecovery->ExchCond(ExchNum).SupInHumRat);
    state->dataHeatRecovery->ExchCond(ExchNum).SecInEnth =
        PsyHFnTdbW(state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp, state->dataHeatRecovery->ExchCond(ExchNum).SecInHumRat);
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).Temp = state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).Temp = state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).HumRat =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInHumRat;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).HumRat =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInHumRat;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).Enthalpy =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInEnth;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).Enthalpy =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInEnth;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).MassFlowRate =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInMassFlow;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).MassFlowRate =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInMassFlow;

    // HXUnitOn is false so expect outlet = inlet
    HXUnitOn = false;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ(state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp,
                     state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp);

    state->dataHeatRecovery->ExchCond(ExchNum).ControlToTemperatureSetPoint = false;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint = 19.0;

    // HXUnitOn is true and ControlToTemperatureSetPoint is false so expect outlet = temperature based on effectiveness
    HXUnitOn = true;
    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Plate;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ((state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp +
                      (state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensible100 *
                       (state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp - state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp))),
                     state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp);

    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Rotary;
    HXUnitOn = true;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ((state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp +
                      (state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensible100 *
                       (state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp - state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp))),
                     state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp);

    state->dataHeatRecovery->ExchCond(ExchNum).ControlToTemperatureSetPoint = true;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint = 19.0;

    // HXUnitOn is true and ControlToTemperatureSetPoint is true so expect outlet = set point temperature
    HXUnitOn = true;
    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Plate;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint,
                     state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp);

    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HXExchConfigType::Rotary;
    HXUnitOn = true;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).TempSetPoint,
                     state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp);

    // test cycling fan case
    fanOp = HVAC::FanOp::Cycling;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupInletNode).MassFlowRate =
        state->dataHeatRecovery->ExchCond(ExchNum).SupInMassFlow / 4.0;
    state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SecInletNode).MassFlowRate =
        state->dataHeatRecovery->ExchCond(ExchNum).SecInMassFlow / 4.0;
    state->dataHeatRecovery->ExchCond(ExchNum).ControlToTemperatureSetPoint = false;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag, PartLoadRatio);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ((state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp +
                      (state->dataHeatRecovery->ExchCond(ExchNum).CoolEffectSensible100 *
                       (state->dataHeatRecovery->ExchCond(ExchNum).SecInTemp - state->dataHeatRecovery->ExchCond(ExchNum).SupInTemp))),
                     state->dataLoopNodes->Node(state->dataHeatRecovery->ExchCond(ExchNum).SupOutletNode).Temp);
}

TEST_F(EnergyPlusFixture, HeatRecoveryHXOnManinBranch_GetInputTest)
{

    std::string const idf_objects = delimited_string({
        " Coil:Cooling:Water,",
        "	AHU cooling coil,	!- Name",
        "	AvailSched,			!- Availability Schedule Name",
        "	autosize,			!- Design Water Flow Rate { m3 / s }",
        "	autosize,			!- Design Air Flow Rate { m3 / s }",
        "	autosize,			!- Design Inlet Water Temperature { C }",
        "	autosize,			!- Design Inlet Air Temperature { C }",
        "	autosize,			!- Design Outlet Air Temperature { C }",
        "	autosize,			!- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	autosize,			!- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Water Inlet Node,	!- Water Inlet Node Name",
        "	Water Outlet Node,  !- Water Outlet Node Name",
        "	AHU mixed air outlet,		!- Air Inlet Node Name",
        "	AHU cooling coil outlet,	!- Air Outlet Node Name",
        "	SimpleAnalysis,		!- Type of Analysis",
        "	CrossFlow;          !- Heat Exchanger Configuration",

        " Coil:Heating:Water,",
        "	AHU Heating coil, !- Name",
        "	AvailSched,       !- Availability Schedule Name",
        "	autosize, !- U - Factor Times Area Value { W / K }",
        "	autosize, !- Maximum Water Flow Rate { m3 / s }",
        "	AHU Heating COil HW Inlet, !- Water Inlet Node Name",
        "	AHU Heating COil HW Outlet, !- Water Outlet Node Name",
        "	AHU cooling coil outlet, !- Air Inlet Node Name",
        "	AHU Heating Coil Outlet, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	48.8888888888889, !- Rated Outlet Air Temperature { C }",
        "	1;          !- Rated Ratio for Air and Water Convection",

        " Controller:WaterCoil,",
        "	AHU cooling coil controller, !- Name",
        "	TemperatureAndHumidityRatio,		!- Control Variable",
        "	Reverse,			!- Action",
        "	FLOW,				!- Actuator Variable",
        "	AHU cooling coil outlet,	!- Sensor Node Name",
        "	Water Inlet Node,	!- Actuator Node Name",
        "	autosize,			!- Controller Convergence Tolerance { deltaC }",
        "	autosize,			!- Maximum Actuated Flow { m3 / s }",
        "	0.0;				!- Minimum Actuated Flow { m3 / s }",

        " Controller:WaterCoil,",
        "	AHU Heating coil, !- Name",
        "	Temperature,      !- Control Variable",
        "	Normal, !- Action",
        "	Flow,   !- Actuator Variable",
        "	AHU Heating Coil Outlet,   !- Sensor Node Name",
        "	AHU Heating COil HW Inlet, !- Actuator Node Name",
        "	autosize, !- Controller Convergence Tolerance { deltaC }",
        "	autosize, !- Maximum Actuated Flow { m3 / s }",
        "	0;        !- Minimum Actuated Flow { m3 / s }",

        " Schedule:Compact,",
        "   AvailSched,			!- Name",
        "	Fraction,			!- Schedule Type Limits Name",
        "	Through: 12/31,		!- Field 1",
        "	For: AllDays,		!- Field 2",
        "	Until: 24:00, 1.0;  !- Field 3",

        " AirLoopHVAC:ControllerList,",
        "	AHU controllers,    !- Name",
        " Controller:WaterCoil, !- Controller 1 Object Type",
        "	AHU cooling coil controller, !- Controller 1 Name",
        " Controller:WaterCoil, !- Controller 2 Object Type",
        "	AHU Heating coil;   !- Controller 2 Name",

        " HeatExchanger:AirToAir:SensibleAndLatent,",
        "   enthalpy HX,      !- Name",
        "   AvailSched,       !- Availability Schedule Name",
        "   4.71947443200001, !- Nominal Supply Air Flow Rate { m3 / s }",
        "   0,   !- Sensible Effectiveness at 100 % Heating Air Flow { dimensionless }",
        "   0.5, !- Latent Effectiveness at 100 % Heating Air Flow { dimensionless }",
        "   0,   !- Sensible Effectiveness at 100 % Cooling Air Flow { dimensionless }",
        "   0.5, !- Latent Effectiveness at 100 % Cooling Air Flow { dimensionless }",
        "   AHU Heating Coil Outlet, !- Supply Air Inlet Node Name",
        "   AHU Supply fan Inlet,    !- Supply Air Outlet Node Name",
        "   AHU relief air outlet,   !- Exhaust Air Inlet Node Name",
        "   AHU relief air outlet of ENTHALPY HX, !- Exhaust Air Outlet Node Name",
        "   0,      !- Nominal Electric Power { W }",
        "   No,     !- Supply Air Outlet Temperature Control",
        "   Rotary, !- Heat Exchanger Type",
        "   None,   !- Frost Control Type",
        "  -17.7777777777778, !- Threshold Temperature { C }",
        "   0.083,  !- Initial Defrost Time Fraction { dimensionless }",
        "   0.012,  !- Rate of Defrost Time Fraction Increase { 1 / K }",
        "   Yes;    !- Economizer Lockout",

        " Fan:VariableVolume,",
        "   AHU supply fan, !- Name",
        "   AvailSched,     !- Availability Schedule Name",
        "   0.7,            !- Fan Total Efficiency",
        "   996.355828557053, !- Pressure Rise { Pa }",
        "   autosize, !- Maximum Flow Rate { m3 / s }",
        "   Fraction, !- Fan Power Minimum Flow Rate Input Method",
        "   0,        !- Fan Power Minimum Flow Fraction",
        "   0,        !- Fan Power Minimum Air Flow Rate { m3 / s }",
        "   0.95,     !- Motor Efficiency",
        "   1,        !- Motor In Airstream Fraction",
        "   0.35071223, !- Fan Power Coefficient 1",
        "   0.30850535, !- Fan Power Coefficient 2",
        "  -0.54137364, !- Fan Power Coefficient 3",
        "   0.87198823, !- Fan Power Coefficient 4",
        "   0,          !- Fan Power Coefficient 5",
        "   AHU Supply fan Inlet,  !- Air Inlet Node Name",
        "   AHU Supply fan Outlet, !- Air Outlet Node Name",
        "   General;               !- End - Use Subcategory",

        " Branch,",
        "   AHU Main Branch, !- Name",
        "	,         !- Pressure Drop Curve Name",
        "   AirLoopHVAC:OutdoorAirSystem, !- Component 1 Object Type",
        "   AHU OA system,           !- Component 1 Name",
        "   AHU air loop inlet,      !- Component 1 Inlet Node Name",
        "   AHU mixed air outlet,    !- Component 1 Outlet Node Name",
        " Coil:Cooling:water,        !- Component 2 Object Type",
        "   AHU cooling coil,        !- Component 2 Name",
        "   AHU mixed air outlet,    !- Component 2 Inlet Node Name",
        "   AHU cooling coil outlet, !- Component 2 Outlet Node Name",
        " Coil:Heating:Water,        !- Component 3 Object Type",
        "   AHU Heating coil,        !- Component 3 Name",
        "   AHU cooling coil outlet, !- Component 3 Inlet Node Name",
        "   AHU Heating Coil Outlet, !- Component 3 Outlet Node Name",
        " HeatExchanger:AirToAir:SensibleAndLatent, !- Component 4 Object Type",
        "   enthalpy HX,             !- Component 4 Name",
        "   AHU Heating Coil Outlet, !- Component 4 Inlet Node Name",
        "   AHU Supply fan Inlet,    !- Component 4 Outlet Node Name",
        " Fan:VariableVolume,        !- Component 5 Object Type",
        "   AHU Supply Fan,          !- Component 5 Name",
        "   AHU Supply fan Inlet,    !- Component 5 Inlet Node Name",
        "   AHU Supply fan Outlet;   !- Component 5 Outlet Node Name",

        " AirLoopHVAC,",
        "   AHU,                   !- Name",
        "   AHU controllers,       !- Controller List Name",
        "   ,                      !- Availability Manager List Name",
        "   autosize,              !- Design Supply Air Flow Rate { m3 / s }",
        "   AHU Branches,          !- Branch List Name",
        "   ,                      !- Connector List Name",
        "   AHU air loop inlet,    !- Supply Side Inlet Node Name",
        "   AHU return air outlet, !- Demand Side Outlet Node Name",
        "   AHU Supply Path Inlet, !- Demand Side Inlet Node Names",
        "   AHU Supply fan Outlet; !- Supply Side Outlet Node Names",

        " BranchList,",
        "   AHU Branches,          !- Name",
        "   AHU Main Branch;       !- Branch 1 Name",

        " AirLoopHVAC:ReturnPath,",
        "   AHU return path,       !- Name",
        "   AHU return air outlet, !- Return Air Path Outlet Node Name",
        " AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "   AHU zone mixer;        !- Component 1 Name",

        " AirLoopHVAC:ZoneMixer,",
        "   AHU zone mixer,        !- Name",
        "   AHU return air outlet, !- Outlet Node Name",
        "   Main FL1 Return Outlet;!- Inlet 1 Node Name",

        " ZoneHVAC:EquipmentConnections,",
        "   Main FL1,              !- Zone Name",
        "   Main FL1 Equipment,    !- Zone Conditioning Equipment List Name",
        "   Main FL1 Supply inlet, !- Zone Air Inlet Node or NodeList Name",
        "   ,                      !- Zone Air Exhaust Node or NodeList Name",
        "   Main FL1 Zone Air node,!- Zone Air Node Name",
        "   Main FL1 Return Outlet;!- Zone Return Air Node Name",

        " AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "   AHU System equipment,  !- Name",
        " OutdoorAir:Mixer,        !- Component 1 Object Type",
        "   AHU OA Mixing Box;     !- Component 1 Name",

        " AirLoopHVAC:OutdoorAirSystem,",
        "   AHU OA System,             !- Name",
        "   AHU OA system controllers, !- Controller List Name",
        "   AHU System equipment;      !- Outdoor Air Equipment List Name",

        " OutdoorAir:Mixer,",
        "   AHU OA Mixing Box,         !- Name",
        "   AHU mixed air outlet,      !- Mixed Air Node Name",
        "   AHU Outside Air HX Outlet, !- Outdoor Air Stream Node Name",
        "   AHU relief air outlet,     !- Relief Air Stream Node Name",
        "   AHU air loop inlet;        !- Return Air Stream Node Name",

        " AirLoopHVAC:ControllerList,",
        "   AHU OA system controllers, !- Name",
        " Controller:OutdoorAir,       !- Controller 1 Object Type",
        "   AHU OA Controller;         !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetReturnAirPathInput(*state);
    GetAirPathData(*state);
    ASSERT_ENUM_EQ(SimAirServingZones::CompType::HeatXchngr, state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(4).CompType_Num);
}

TEST_F(EnergyPlusFixture, HeatRecoveryHXOnMainBranch_SimHeatRecoveryTest)
{
    Real64 Qhr_HeatingRateTot(0.0);
    int InletNode(0);  // Heat Recovery primary air inlet node number
    int OutletNode(0); // Heat Recovery primary air outlet node number

    std::string const idf_objects = delimited_string({

        "SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    Yes,                     !- Do System Sizing Calculation",
        "    Yes,                     !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "Building,",
        "    Fan Coil with DOAS,      !- Name",
        "    30.,                     !- North Axis {deg}",
        "    City,                    !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.4,                     !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "Timestep,4;",

        "Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "! CHICAGO_IL_USA Annual Cooling 1% Design Conditions, MaxDB=  31.5degC MCWB=  23.0degC",
        "SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "! CHICAGO_IL_USA Annual Heating 99% Design Conditions DB, MaxDB= -17.3degC",
        "SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "Site:GroundTemperature:BuildingSurface,21.5,21.4,21.5,21.5,22.0,22.9,23.0,23.1,23.1,22.2,21.7,21.6;",

        "ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS,              !- Numeric Type",
        "    Temperature;             !- Unit Type",

        "ScheduleTypeLimits,",
        "    Control Type,            !- Name",
        "    0,                       !- Lower Limit Value",
        "    4,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "ScheduleTypeLimits,",
        "    FlowRate,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    10,                      !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "ScheduleTypeLimits,",
        "    HVACTemplate Any Number; !- Name",

        "!-   ===========  ALL OBJECTS IN CLASS: SCHEDULE:COMPACT ===========",

        "Schedule:Compact,",
        "    OCCUPY-1,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 8:00, 0.0,        !- Field 4",
        "    Until: 11:00, 1.00,      !- Field 6",
        "    Until: 12:00, 0.80,      !- Field 8",
        "    Until: 13:00, 0.40,      !- Field 10",
        "    Until: 14:00, 0.80,      !- Field 12",
        "    Until: 18:00, 1.00,      !- Field 14",
        "    Until: 19:00, 0.50,      !- Field 16",
        "    Until: 21:00, 0.10,      !- Field 18",
        "    Until: 24:00, 0.0,       !- Field 20",
        "    For: Weekends WinterDesignDay Holiday,  !- Field 21",
        "    Until: 24:00, 0.0;       !- Field 23",

        "Schedule:Compact,",
        "    LIGHTS-1,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 8:00, 0.05,       !- Field 4",
        "    Until: 9:00, 0.9,        !- Field 6",
        "    Until: 10:00, 0.95,      !- Field 8",
        "    Until: 11:00, 1.00,      !- Field 10",
        "    Until: 12:00, 0.95,      !- Field 12",
        "    Until: 13:00, 0.8,       !- Field 14",
        "    Until: 14:00, 0.9,       !- Field 16",
        "    Until: 18:00, 1.00,      !- Field 18",
        "    Until: 19:00, 0.60,      !- Field 20",
        "    Until: 21:00, 0.40,      !- Field 22",
        "    Until: 24:00, 0.05,      !- Field 24",
        "    For: Weekends WinterDesignDay Holiday,  !- Field 25",
        "    Until: 24:00, 0.05;      !- Field 27",

        "Schedule:Compact,",
        "    EQUIP-1,                 !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 8:00, 0.02,       !- Field 4",
        "    Until: 9:00, 0.4,        !- Field 6",
        "    Until: 14:00, 0.9,       !- Field 8",
        "    Until: 15:00, 0.8,       !- Field 10",
        "    Until: 16:00, 0.7,       !- Field 12",
        "    Until: 18:00, 0.5,       !- Field 14",
        "    Until: 21:00, 0.3,       !- Field 16",
        "    Until: 24:00, 0.02,      !- Field 18",
        "    For: Weekends WinterDesignDay Holiday,  !- Field 19",
        "    Until: 24:00, 0.02;      !- Field 21",

        "Schedule:Compact,",
        "    INFIL-SCH,               !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 7:00, 1.0,        !- Field 4",
        "    Until: 21:00, 0.0,       !- Field 6",
        "    Until: 24:00, 1.0,       !- Field 8",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00, 1.0,       !- Field 11",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00, 1.0,       !- Field 14",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00, 1.0;       !- Field 17",

        "Schedule:Compact,",
        "    ActSchd,                 !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 117.239997864;",
        "                             !- Field 4",

        "Schedule:Compact,",
        "    ShadeTransSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.0;       !- Field 4",

        "! For heating, recover 2 hrs early",
        "Schedule:Compact,",
        "    Htg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 6:00, 13.0,       !- Field 4",
        "    Until: 7:00, 18.0,       !- Field 6",
        "    Until: 21:00, 23.0,      !- Field 8",
        "    Until: 24:00, 13.0,      !- Field 10",
        "    For: WeekEnds Holiday,   !- Field 11",
        "    Until: 24:00, 13.0,      !- Field 13",
        "    For: SummerDesignDay,    !- Field 14",
        "    Until: 24:00, 13.0,      !- Field 16",
        "    For: WinterDesignDay,    !- Field 17",
        "    Until: 24:00, 23.0;      !- Field 19",

        "! For cooling, recover 1 hr early",
        "Schedule:Compact,",
        "    Clg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 7:00, 32.0,       !- Field 4",
        "    Until: 21:00, 24.0,      !- Field 6",
        "    Until: 24:00, 32.0,      !- Field 8",
        "    For: WeekEnds Holiday,   !- Field 9",
        "    Until: 24:00, 32.0,      !- Field 11",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00, 24.0,      !- Field 14",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00, 32.0;      !- Field 17",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 7:00, 0.0,        !- Field 4",
        "    Until: 21:00, 1.0,       !- Field 6",
        "    Until: 24:00, 0.0,       !- Field 8",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00, 0.0,       !- Field 11",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00, 1.0,       !- Field 14",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00, 1.0;       !- Field 17",

        "Schedule:Compact,",
        "    HVACTemplate-Always 1,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 1;         !- Field 4",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 4;         !- Field 4",

        "Schedule:Compact,",
        "    HVACTemplate-Always 12.2,!- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 12.2;      !- Field 4",

        "Material,",
        "    WD10,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.667,                   !- Thickness {m}",
        "    0.115,                   !- Conductivity {W/m-K}",
        "    513,                     !- Density {kg/m3}",
        "    1381,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.78,                    !- Solar Absorptance",
        "    0.78;                    !- Visible Absorptance",

        "Material,",
        "    RG01,                    !- Name",
        "    Rough,                   !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    1.442000,                !- Conductivity {W/m-K}",
        "    881.0000,                !- Density {kg/m3}",
        "    1674.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "Material,",
        "    BR01,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    9.4999997E-03,           !- Thickness {m}",
        "    0.1620000,               !- Conductivity {W/m-K}",
        "    1121.000,                !- Density {kg/m3}",
        "    1464.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "Material,",
        "    IN46,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.6200001E-02,           !- Thickness {m}",
        "    2.3000000E-02,           !- Conductivity {W/m-K}",
        "    24.00000,                !- Density {kg/m3}",
        "    1590.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "Material,",
        "    WD01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.9099999E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    513.0000,                !- Density {kg/m3}",
        "    1381.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "Material,",
        "    GP01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1600000,               !- Conductivity {W/m-K}",
        "    801.0000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "Material,",
        "    GP02,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.5900001E-02,           !- Thickness {m}",
        "    0.1600000,               !- Conductivity {W/m-K}",
        "    801.0000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "Material:NoMass,",
        "    CP01,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.3670000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "Material:NoMass,",
        "    MAT-CLNG-1,              !- Name",
        "    Rough,                   !- Roughness",
        "    0.652259290,             !- Thermal Resistance {m2-K/W}",
        "    0.65,                    !- Thermal Absorptance",
        "    0.65,                    !- Solar Absorptance",
        "    0.65;                    !- Visible Absorptance",

        "Material:AirGap,",
        "    AL21,                    !- Name",
        "    0.1570000;               !- Thermal Resistance {m2-K/W}",

        "Material:AirGap,",
        "    AL23,                    !- Name",
        "    0.1530000;               !- Thermal Resistance {m2-K/W}",

        "WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "WindowMaterial:Glazing,",
        "    GREY 3MM,                !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.626,                   !- Solar Transmittance at Normal Incidence",
        "    0.061,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.061,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.611,                   !- Visible Transmittance at Normal Incidence",
        "    0.061,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.061,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "WindowMaterial:Glazing,",
        "    CLEAR 6MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.775,                   !- Solar Transmittance at Normal Incidence",
        "    0.071,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.071,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.881,                   !- Visible Transmittance at Normal Incidence",
        "    0.080,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.080,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "WindowMaterial:Glazing,",
        "    LoE CLEAR 6MM,           !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.600,                   !- Solar Transmittance at Normal Incidence",
        "    0.170,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.220,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.840,                   !- Visible Transmittance at Normal Incidence",
        "    0.055,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.078,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.10,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    Air,                     !- Gas Type",
        "    0.0063;                  !- Thickness {m}",

        "WindowMaterial:Gas,",
        "    AIR 13MM,                !- Name",
        "    Air,                     !- Gas Type",
        "    0.0127;                  !- Thickness {m}",

        "WindowMaterial:Gas,",
        "    ARGON 13MM,              !- Name",
        "    Argon,                   !- Gas Type",
        "    0.0127;                  !- Thickness {m}",

        "Construction,",
        "    ROOF-1,                  !- Name",
        "    RG01,                    !- Outside Layer",
        "    BR01,                    !- Layer 2",
        "    IN46,                    !- Layer 3",
        "    WD01;                    !- Layer 4",

        "Construction,",
        "    WALL-1,                  !- Name",
        "    WD01,                    !- Outside Layer",
        "    PW03,                    !- Layer 2",
        "    IN02,                    !- Layer 3",
        "    GP01;                    !- Layer 4",

        "Construction,",
        "    CLNG-1,                  !- Name",
        "    MAT-CLNG-1;              !- Outside Layer",

        "Construction,",
        "    FLOOR-SLAB-1,            !- Name",
        "    CC03;                    !- Outside Layer",

        "Construction,",
        "    INT-WALL-1,              !- Name",
        "    GP02,                    !- Outside Layer",
        "    AL21,                    !- Layer 2",
        "    GP02;                    !- Layer 3",

        "Construction,",
        "    Dbl Clr 3mm/13mm Air,    !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 13MM,                !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "Construction,",
        "    Sgl Grey 3mm,            !- Name",
        "    GREY 3MM;                !- Outside Layer",

        "GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    relative;                !- Coordinate System",

        "Zone,",
        "    PLENUM-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0.609600067,             !- Ceiling Height {m}",
        "    283.2;                   !- Volume {m3}",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "Zone,",
        "    SPACE2-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "Zone,",
        "    SPACE3-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "Zone,",
        "    SPACE4-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "Zone,",
        "    SPACE5-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    447.682556152;           !- Volume {m3}",

        "BuildingSurface:Detailed,",
        "    WALL-1PF,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 3.0,                      !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  3 {m}",
        "    30.5, 0.0, 3.0;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    WALL-1PR,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 0.0, 3.0,                     !- X,Y,Z  1 {m}",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  2 {m}",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  3 {m}",
        "    30.5, 15.2, 3.0;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    WALL-1PB,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 15.2, 3.0,                    !- X,Y,Z  1 {m}",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  3 {m}",
        "    0.0, 15.2, 3.0;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    WALL-1PL,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 15.2, 3.0,                     !- X,Y,Z  1 {m}",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  2 {m}",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  3 {m}",
        "    0.0, 0.0, 3.0;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    TOP-1,                   !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.00000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 15.2, 3.0,                     !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 3.0,                      !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 3.0,                     !- X,Y,Z  3 {m}",
        "    30.5, 15.2, 3.0;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C1-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C1-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  1 {m}",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  2 {m}",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  3 {m}",
        "    3.7, 3.7, 2.4;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C2-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C2-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 2.4,                    !- X,Y,Z  1 {m}",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  3 {m}",
        "    26.8, 3.7, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C3-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C3-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 2.4,                    !- X,Y,Z  1 {m}",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  3 {m}",
        "    30.5, 15.2, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C4-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C4-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C5-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C5-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 2.4,                    !- X,Y,Z  1 {m}",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  2 {m}",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    FRONT-1,                 !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 0.0,                      !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 0.0,                     !- X,Y,Z  3 {m}",
        "    30.5, 0.0, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C1-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C1-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  3 {m}",
        "    26.8, 3.7, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    F1-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  1 {m}",
        "    30.5, 0.0, 0.0,                     !- X,Y,Z  2 {m}",
        "    0.0, 0.0, 0.0,                      !- X,Y,Z  3 {m}",
        "    3.7, 3.7, 0.0;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB12,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB21,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  1 {m}",
        "    30.5, 0.0, 0.0,                     !- X,Y,Z  2 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  3 {m}",
        "    26.8, 3.7, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB14,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB41,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  1 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  2 {m}",
        "    0.0, 0.0, 0.0,                      !- X,Y,Z  3 {m}",
        "    0.0, 0.0, 2.4;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB15,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB51,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  1 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  2 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  3 {m}",
        "    3.7, 3.7, 2.4;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    RIGHT-1,                 !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  1 {m}",
        "    30.5, 0.0, 0.0,                     !- X,Y,Z  2 {m}",
        "    30.5, 15.2, 0.0,                    !- X,Y,Z  3 {m}",
        "    30.5, 15.2, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C2-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C2-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  1 {m}",
        "    30.5, 0.0, 2.4,                     !- X,Y,Z  2 {m}",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  3 {m}",
        "    26.8, 11.6, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    F2-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  1 {m}",
        "    30.5, 15.2, 0.0,                    !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 0.0,                     !- X,Y,Z  3 {m}",
        "    26.8, 3.7, 0.0;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB21,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB12,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  1 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  2 {m}",
        "    30.5, 0.0, 0.0,                     !- X,Y,Z  3 {m}",
        "    30.5, 0.0, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB23,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB32,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  1 {m}",
        "    30.5, 15.2, 0.0,                    !- X,Y,Z  2 {m}",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  3 {m}",
        "    26.8, 11.6, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB25,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB52,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 2.4,                    !- X,Y,Z  1 {m}",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  2 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  3 {m}",
        "    26.8, 3.7, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    BACK-1,                  !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  1 {m}",
        "    30.5, 15.2, 0.0,                    !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 0.0,                     !- X,Y,Z  3 {m}",
        "    0.0, 15.2, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C3-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C3-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5, 15.2, 2.4,                    !- X,Y,Z  1 {m}",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  2 {m}",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  3 {m}",
        "    26.8, 11.6, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    F3-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  1 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 0.0,                     !- X,Y,Z  3 {m}",
        "    30.5, 15.2, 0.0;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB32,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB23,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 2.4,                    !- X,Y,Z  1 {m}",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  2 {m}",
        "    30.5, 15.2, 0.0,                    !- X,Y,Z  3 {m}",
        "    30.5, 15.2, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB34,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB43,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  1 {m}",
        "    0.0, 15.2, 0.0,                     !- X,Y,Z  2 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB35,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB53,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  1 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  2 {m}",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  3 {m}",
        "    26.8, 11.6, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    LEFT-1,                  !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  1 {m}",
        "    0.0, 15.2, 0.0,                     !- X,Y,Z  2 {m}",
        "    0.0, 0.0, 0.0,                      !- X,Y,Z  3 {m}",
        "    0.0, 0.0, 2.4;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C4-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C4-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  1 {m}",
        "    0.0, 15.2, 2.4,                     !- X,Y,Z  2 {m}",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  3 {m}",
        "    3.7, 3.7, 2.4;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    F4-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 0.0,                      !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 0.0,                     !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 0.0;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB41,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB14,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 2.4,                      !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 0.0,                      !- X,Y,Z  2 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  3 {m}",
        "    3.7, 3.7, 2.4;                      !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB43,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB34,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  1 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  2 {m}",
        "    0.0, 15.2, 0.0,                     !- X,Y,Z  3 {m}",
        "    0.0, 15.2, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB45,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB54,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  1 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  2 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    C5-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C5-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  1 {m}",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  2 {m}",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  3 {m}",
        "    26.8, 11.6, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    F5-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  1 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  2 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 0.0;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB51,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB15,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 3.7, 2.4,                      !- X,Y,Z  1 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  2 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  3 {m}",
        "    26.8, 3.7, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB52,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB25,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 3.7, 2.4,                     !- X,Y,Z  1 {m}",
        "    26.8, 3.7, 0.0,                     !- X,Y,Z  2 {m}",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  3 {m}",
        "    26.8, 11.6, 2.4;                    !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB53,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB35,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8, 11.6, 2.4,                    !- X,Y,Z  1 {m}",
        "    26.8, 11.6, 0.0,                    !- X,Y,Z  2 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  3 {m}",
        "    3.7, 11.6, 2.4;                     !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    SB54,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB45,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7, 11.6, 2.4,                     !- X,Y,Z  1 {m}",
        "    3.7, 11.6, 0.0,                     !- X,Y,Z  2 {m}",
        "    3.7, 3.7, 0.0,                      !- X,Y,Z  3 {m}",
        "    3.7, 3.7, 2.4;                      !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    WF-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Dbl Clr 3mm/13mm Air,    !- Construction Name",
        "    FRONT-1,                 !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    3.0, 0.0, 2.1,                      !- X,Y,Z  1 {m}",
        "    3.0, 0.0, 0.9,                      !- X,Y,Z  2 {m}",
        "    16.8, 0.0, 0.9,                     !- X,Y,Z  3 {m}",
        "    16.8, 0.0, 2.1;                     !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    DF-1,                    !- Name",
        "    GLASSDOOR,               !- Surface Type",
        "    Sgl Grey 3mm,            !- Construction Name",
        "    FRONT-1,                 !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    21.3, 0.0, 2.1,                     !- X,Y,Z  1 {m}",
        "    21.3, 0.0, 0.0,                     !- X,Y,Z  2 {m}",
        "    23.8, 0.0, 0.0,                     !- X,Y,Z  3 {m}",
        "    23.8, 0.0, 2.1;                     !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    WR-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Dbl Clr 3mm/13mm Air,    !- Construction Name",
        "    RIGHT-1,                 !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    30.5, 3.8, 2.1,                     !- X,Y,Z  1 {m}",
        "    30.5, 3.8, 0.9,                     !- X,Y,Z  2 {m}",
        "    30.5, 11.4, 0.9,                    !- X,Y,Z  3 {m}",
        "    30.5, 11.4, 2.1;                    !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    WB-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Dbl Clr 3mm/13mm Air,    !- Construction Name",
        "    BACK-1,                  !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    27.4, 15.2, 2.1,                    !- X,Y,Z  1 {m}",
        "    27.4, 15.2, 0.9,                    !- X,Y,Z  2 {m}",
        "    13.7, 15.2, 0.9,                    !- X,Y,Z  3 {m}",
        "    13.7, 15.2, 2.1;                    !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    DB-1,                    !- Name",
        "    GLASSDOOR,               !- Surface Type",
        "    Sgl Grey 3mm,            !- Construction Name",
        "    BACK-1,                  !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    9.1, 15.2, 2.1,                     !- X,Y,Z  1 {m}",
        "    9.1, 15.2, 0.0,                     !- X,Y,Z  2 {m}",
        "    7.0, 15.2, 0.0,                     !- X,Y,Z  3 {m}",
        "    7.0, 15.2, 2.1;                     !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    WL-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Dbl Clr 3mm/13mm Air,    !- Construction Name",
        "    LEFT-1,                  !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.0, 11.4, 2.1,                     !- X,Y,Z  1 {m}",
        "    0.0, 11.4, 0.9,                     !- X,Y,Z  2 {m}",
        "    0.0, 3.8, 0.9,                      !- X,Y,Z  3 {m}",
        "    0.0, 3.8, 2.1;                      !- X,Y,Z  4 {m}",

        "Shading:Zone:Detailed,",
        "    Main South Overhang,     !- Name",
        "    FRONT-1,                 !- Base Surface Name",
        "    ShadeTransSch,           !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    0.0, -1.3, 2.2,                     !- X,Y,Z  1 {m}",
        "    0.0, 0.0, 2.2,                      !- X,Y,Z  2 {m}",
        "    19.8, 0.0, 2.2,                     !- X,Y,Z  3 {m}",
        "    19.8, -1.3, 2.2;                    !- X,Y,Z  4 {m}",

        "Shading:Zone:Detailed,",
        "    South Door Overhang,     !- Name",
        "    FRONT-1,                 !- Base Surface Name",
        "    ShadeTransSch,           !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    21.0, -2.0, 2.6,                    !- X,Y,Z  1 {m}",
        "    21.0, 0.0, 2.6,                     !- X,Y,Z  2 {m}",
        "    24.1, 0.0, 2.6,                     !- X,Y,Z  3 {m}",
        "    24.1, -2.0, 2.6;                    !- X,Y,Z  4 {m}",

        "People,",
        "    SPACE1-1 People 1,       !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    11,                      !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "People,",
        "    SPACE2-1 People 1,       !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    5,                       !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "People,",
        "    SPACE3-1 People 1,       !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    11,                      !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "People,",
        "    SPACE4-1 People 1,       !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    5,                       !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "People,",
        "    SPACE5-1 People 1,       !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    20,                      !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "Lights,",
        "    SPACE1-1 Lights 1,       !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    1584,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "Lights,",
        "    SPACE2-1 Lights 1,       !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    684,                     !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "Lights,",
        "    SPACE3-1 Lights 1,       !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    1584,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "Lights,",
        "    SPACE4-1 Lights 1,       !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    684,                     !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "Lights,",
        "    SPACE5-1 Lights 1,       !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    2964,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "ElectricEquipment,",
        "    SPACE1-1 ElecEq 1,       !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1056,                    !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "ElectricEquipment,",
        "    SPACE2-1 ElecEq 1,       !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    456,                     !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "ElectricEquipment,",
        "    SPACE3-1 ElecEq 1,       !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1056,                    !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "ElectricEquipment,",
        "    SPACE4-1 ElecEq 1,       !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    456,                     !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "ElectricEquipment,",
        "    SPACE5-1 ElecEq 1,       !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1976,                    !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "ZoneInfiltration:DesignFlowRate,",
        "    SPACE1-1 Infil 1,        !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.0167,                  !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "ZoneInfiltration:DesignFlowRate,",
        "    SPACE2-1 Infil 1,        !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.00717,                 !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "ZoneInfiltration:DesignFlowRate,",
        "    SPACE3-1 Infil 1,        !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.0167,                  !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "ZoneInfiltration:DesignFlowRate,",
        "    SPACE4-1 Infil 1,        !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.00717,                 !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "ZoneInfiltration:DesignFlowRate,",
        "    SPACE5-1 Infil 1,        !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.031089,                !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE1-1,        !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE2-1,        !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE3-1,        !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE4-1,        !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE5-1,        !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "Sizing:Zone,",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.5,                    !- Zone Cooling Design Supply Air Temperature {C}",
        "    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50,                      !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA SPACE1-1,        !- Design Specification Outdoor Air Object Name",
        "    ,                        !- Zone Heating Sizing Factor",
        "    ,                        !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    0,                       !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    0,                       !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    Yes,                     !- Account for Dedicated Outside Air System",
        "    ColdSupplyAir,           !- Dedicated Outside Air System Control Strategy",
        "    12.8,                    !- Dedicated Outside Air Low Setpoint for Design",
        "    15.6;                    !- Dedicated Outside Air High Setpoint for Design",

        "Sizing:Zone,",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.5,                    !- Zone Cooling Design Supply Air Temperature {C}",
        "    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50,                      !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA SPACE2-1,        !- Design Specification Outdoor Air Object Name",
        "    ,                        !- Zone Heating Sizing Factor",
        "    ,                        !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    0,                       !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    0,                       !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    Yes,                     !- Account for Dedicated Outside Air System",
        "    ColdSupplyAir,           !- Dedicated Outside Air System Control Strategy",
        "    12.8,                    !- Dedicated Outside Air Low Setpoint for Design",
        "    15.6;                    !- Dedicated Outside Air High Setpoint for Design",

        "Sizing:Zone,",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.5,                    !- Zone Cooling Design Supply Air Temperature {C}",
        "    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50,                      !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA SPACE3-1,        !- Design Specification Outdoor Air Object Name",
        "    ,                        !- Zone Heating Sizing Factor",
        "    ,                        !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    0,                       !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    0,                       !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    Yes,                     !- Account for Dedicated Outside Air System",
        "    ColdSupplyAir,           !- Dedicated Outside Air System Control Strategy",
        "    12.8,                    !- Dedicated Outside Air Low Setpoint for Design",
        "    15.6;                    !- Dedicated Outside Air High Setpoint for Design",

        "Sizing:Zone,",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.5,                    !- Zone Cooling Design Supply Air Temperature {C}",
        "    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50,                      !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA SPACE4-1,        !- Design Specification Outdoor Air Object Name",
        "    ,                        !- Zone Heating Sizing Factor",
        "    ,                        !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    0,                       !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    0,                       !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    Yes,                     !- Account for Dedicated Outside Air System",
        "    ColdSupplyAir,           !- Dedicated Outside Air System Control Strategy",
        "    12.8,                    !- Dedicated Outside Air Low Setpoint for Design",
        "    15.6;                    !- Dedicated Outside Air High Setpoint for Design",

        "Sizing:Zone,",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.5,                    !- Zone Cooling Design Supply Air Temperature {C}",
        "    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50,                      !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA SPACE5-1,        !- Design Specification Outdoor Air Object Name",
        "    ,                        !- Zone Heating Sizing Factor",
        "    ,                        !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    0,                       !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    0,                       !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    Yes,                     !- Account for Dedicated Outside Air System",
        "    ColdSupplyAir,           !- Dedicated Outside Air System Control Strategy",
        "    12.8,                    !- Dedicated Outside Air Low Setpoint for Design",
        "    15.6;                    !- Dedicated Outside Air High Setpoint for Design",

        "Sizing:System,",
        "    DOAS,                    !- AirLoop Name",
        "    VentilationRequirement,  !- Type of Load to Size On",
        "    autosize,                !- Design Outdoor Air Flow Rate {m3/s}",
        "    1.0,                     !- Central Heating Maximum System Air Flow Ratio",
        "    2,                       !- Preheat Design Temperature {C}",
        "    0.008,                   !- Preheat Design Humidity Ratio {kgWater/kgDryAir}",
        "    11,                      !- Precool Design Temperature {C}",
        "    0.008,                   !- Precool Design Humidity Ratio {kgWater/kgDryAir}",
        "    12.8,                    !- Central Cooling Design Supply Air Temperature {C}",
        "    12.2,                    !- Central Heating Design Supply Air Temperature {C}",
        "    NonCoincident,           !- Type of Zone Sum to Use",
        "    Yes,                     !- 100% Outdoor Air in Cooling",
        "    Yes,                     !- 100% Outdoor Air in Heating",
        "    0.00924,                 !- Central Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.003,                   !- Central Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    DesignDay,               !- Cooling Supply Air Flow Rate Method",
        "    0,                       !- Cooling Supply Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}",
        "    DesignDay,               !- Heating Supply Air Flow Rate Method",
        "    0,                       !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                        !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "    ,                        !- Heating Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}",
        "    ZoneSum,                 !- System Outdoor Air Method",
        "    1.0,                     !- Zone Maximum Outdoor Air Fraction {dimensionless}",
        "    CoolingDesignCapacity,   !- Cooling Design Capacity Method",
        "    autosize,                !- Cooling Design Capacity {W}",
        "    ,                        !- Cooling Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Cooling Design Capacity",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    autosize,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    OnOff;                   !- Central Cooling Capacity Control Method",

        "Sizing:Plant,",
        "    Hot Water Loop Hot Water Loop,  !- Plant or Condenser Loop Name",
        "    Heating,                 !- Loop Type",
        "    82,                      !- Design Loop Exit Temperature {C}",
        "    11.0;                    !- Loop Design Temperature Difference {deltaC}",

        "Sizing:Plant,",
        "    Chilled Water Loop Chilled Water Loop,  !- Plant or Condenser Loop Name",
        "    Cooling,                 !- Loop Type",
        "    7.22,                    !- Design Loop Exit Temperature {C}",
        "    6.67;                    !- Loop Design Temperature Difference {deltaC}",

        "ZoneControl:Thermostat,",
        "    SPACE1-1 Thermostat,     !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "ZoneControl:Thermostat,",
        "    SPACE2-1 Thermostat,     !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "ZoneControl:Thermostat,",
        "    SPACE3-1 Thermostat,     !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "ZoneControl:Thermostat,",
        "    SPACE4-1 Thermostat,     !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "ZoneControl:Thermostat,",
        "    SPACE5-1 Thermostat,     !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "ThermostatSetpoint:DualSetpoint,",
        "    All Zones Dual SP Control,  !- Name",
        "    Htg-SetP-Sch,            !- Heating Setpoint Temperature Schedule Name",
        "    Clg-SetP-Sch;            !- Cooling Setpoint Temperature Schedule Name",

        "ZoneHVAC:FourPipeFanCoil,",
        "    SPACE1-1 Fan Coil,       !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    CyclingFan,              !- Capacity Control Method",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    0.33,                    !- Low Speed Supply Air Flow Ratio",
        "    0.66,                    !- Medium Speed Supply Air Flow Ratio",
        "    0.0,                     !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    ,                        !- Outdoor Air Schedule Name",
        "    SPACE1-1 Fan Coil Return,!- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    SPACE1-1 OA Mixing Box,  !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "    SPACE1-1 Cooling Coil,   !- Cooling Coil Name",
        "    autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Cold Water Flow Rate {m3/s}",
        "    0.001,                   !- Cooling Convergence Tolerance",
        "    Coil:Heating:Water,      !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Hot Water Flow Rate {m3/s}",
        "    0.001;                   !- Heating Convergence Tolerance",

        "ZoneHVAC:FourPipeFanCoil,",
        "    SPACE2-1 Fan Coil,       !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    CyclingFan,              !- Capacity Control Method",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    0.33,                    !- Low Speed Supply Air Flow Ratio",
        "    0.66,                    !- Medium Speed Supply Air Flow Ratio",
        "    0.0,                     !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    ,                        !- Outdoor Air Schedule Name",
        "    SPACE2-1 Fan Coil Return,!- Air Inlet Node Name",
        "    SPACE2-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    SPACE2-1 OA Mixing Box,  !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE2-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "    SPACE2-1 Cooling Coil,   !- Cooling Coil Name",
        "    autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Cold Water Flow Rate {m3/s}",
        "    0.001,                   !- Cooling Convergence Tolerance",
        "    Coil:Heating:Water,      !- Heating Coil Object Type",
        "    SPACE2-1 Heating Coil,   !- Heating Coil Name",
        "    autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Hot Water Flow Rate {m3/s}",
        "    0.001;                   !- Heating Convergence Tolerance",

        "ZoneHVAC:FourPipeFanCoil,",
        "    SPACE3-1 Fan Coil,       !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    CyclingFan,              !- Capacity Control Method",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    0.33,                    !- Low Speed Supply Air Flow Ratio",
        "    0.66,                    !- Medium Speed Supply Air Flow Ratio",
        "    0.0,                     !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    ,                        !- Outdoor Air Schedule Name",
        "    SPACE3-1 Fan Coil Return,!- Air Inlet Node Name",
        "    SPACE3-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    SPACE3-1 OA Mixing Box,  !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE3-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "    SPACE3-1 Cooling Coil,   !- Cooling Coil Name",
        "    autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Cold Water Flow Rate {m3/s}",
        "    0.001,                   !- Cooling Convergence Tolerance",
        "    Coil:Heating:Water,      !- Heating Coil Object Type",
        "    SPACE3-1 Heating Coil,   !- Heating Coil Name",
        "    autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Hot Water Flow Rate {m3/s}",
        "    0.001;                   !- Heating Convergence Tolerance",

        "ZoneHVAC:FourPipeFanCoil,",
        "    SPACE4-1 Fan Coil,       !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    CyclingFan,              !- Capacity Control Method",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    0.33,                    !- Low Speed Supply Air Flow Ratio",
        "    0.66,                    !- Medium Speed Supply Air Flow Ratio",
        "    0.0,                     !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    ,                        !- Outdoor Air Schedule Name",
        "    SPACE4-1 Fan Coil Return,!- Air Inlet Node Name",
        "    SPACE4-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    SPACE4-1 OA Mixing Box,  !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE4-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "    SPACE4-1 Cooling Coil,   !- Cooling Coil Name",
        "    autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Cold Water Flow Rate {m3/s}",
        "    0.001,                   !- Cooling Convergence Tolerance",
        "    Coil:Heating:Water,      !- Heating Coil Object Type",
        "    SPACE4-1 Heating Coil,   !- Heating Coil Name",
        "    autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Hot Water Flow Rate {m3/s}",
        "    0.001;                   !- Heating Convergence Tolerance",

        "ZoneHVAC:FourPipeFanCoil,",
        "    SPACE5-1 Fan Coil,       !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    CyclingFan,              !- Capacity Control Method",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    0.33,                    !- Low Speed Supply Air Flow Ratio",
        "    0.66,                    !- Medium Speed Supply Air Flow Ratio",
        "    0.0,                     !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    ,                        !- Outdoor Air Schedule Name",
        "    SPACE5-1 Fan Coil Return,!- Air Inlet Node Name",
        "    SPACE5-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    SPACE5-1 OA Mixing Box,  !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE5-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "    SPACE5-1 Cooling Coil,   !- Cooling Coil Name",
        "    autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Cold Water Flow Rate {m3/s}",
        "    0.001,                   !- Cooling Convergence Tolerance",
        "    Coil:Heating:Water,      !- Heating Coil Object Type",
        "    SPACE5-1 Heating Coil,   !- Heating Coil Name",
        "    autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "    0,                       !- Minimum Hot Water Flow Rate {m3/s}",
        "    0.001;                   !- Heating Convergence Tolerance",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    SPACE1-1 DOAS Supply Inlet,  !- Air Outlet Node Name",
        "    SPACE1-1 Zone Equip Inlet,  !- Air Inlet Node Name",
        "    autosize,                !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    1.0;                     !- Constant Minimum Air Flow Fraction",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-1 DOAS Air Terminal,  !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    SPACE2-1 DOAS Supply Inlet,  !- Air Outlet Node Name",
        "    SPACE2-1 Zone Equip Inlet,  !- Air Inlet Node Name",
        "    autosize,                !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    1.0;                     !- Constant Minimum Air Flow Fraction",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE3-1 DOAS Air Terminal,  !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    SPACE3-1 DOAS Supply Inlet,  !- Air Outlet Node Name",
        "    SPACE3-1 Zone Equip Inlet,  !- Air Inlet Node Name",
        "    autosize,                !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    1.0;                     !- Constant Minimum Air Flow Fraction",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE4-1 DOAS Air Terminal,  !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    SPACE4-1 DOAS Supply Inlet,  !- Air Outlet Node Name",
        "    SPACE4-1 Zone Equip Inlet,  !- Air Inlet Node Name",
        "    autosize,                !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    1.0;                     !- Constant Minimum Air Flow Fraction",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE5-1 DOAS Air Terminal,  !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    SPACE5-1 DOAS Supply Inlet,  !- Air Outlet Node Name",
        "    SPACE5-1 Zone Equip Inlet,  !- Air Inlet Node Name",
        "    autosize,                !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    1.0;                     !- Constant Minimum Air Flow Fraction",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 DOAS Supply Inlet,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 DOAS ATU,       !- Name",
        "    SPACE2-1 DOAS Supply Inlet,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE3-1 DOAS ATU,       !- Name",
        "    SPACE3-1 DOAS Supply Inlet,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE3-1 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE4-1 DOAS ATU,       !- Name",
        "    SPACE4-1 DOAS Supply Inlet,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE4-1 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE5-1 DOAS ATU,       !- Name",
        "    SPACE5-1 DOAS Supply Inlet,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE5-1 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "    SPACE1-1 Fan Coil,       !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "ZoneHVAC:EquipmentList,",
        "    SPACE2-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "    SPACE2-1 Fan Coil,       !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "ZoneHVAC:EquipmentList,",
        "    SPACE3-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE3-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "    SPACE3-1 Fan Coil,       !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "ZoneHVAC:EquipmentList,",
        "    SPACE4-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE4-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "    SPACE4-1 Fan Coil,       !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "ZoneHVAC:EquipmentList,",
        "    SPACE5-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE5-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "    SPACE5-1 Fan Coil,       !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Fan Coil Return,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE2-1,                !- Zone Name",
        "    SPACE2-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE2-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE2-1 Fan Coil Return,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE2-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE2-1 Return Outlet;  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE3-1,                !- Zone Name",
        "    SPACE3-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE3-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE3-1 Fan Coil Return,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE3-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE3-1 Return Outlet;  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE4-1,                !- Zone Name",
        "    SPACE4-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE4-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE4-1 Fan Coil Return,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE4-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE4-1 Return Outlet;  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE5-1,                !- Zone Name",
        "    SPACE5-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE5-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE5-1 Fan Coil Return,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE5-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE5-1 Return Outlet;  !- Zone Return Air Node Name",

        "Fan:VariableVolume,",
        "    DOAS Supply Fan,         !- Name",
        "    FanAvailSched,                !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    1000,                    !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    Fraction,                !- Fan Power Minimum Flow Rate Input Method",
        "    0.0,                     !- Fan Power Minimum Flow Fraction",
        "    ,                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0,                       !- Fan Power Coefficient 5",
        "    DOAS Heating Coil Outlet,!- Air Inlet Node Name",
        "    DOAS Supply Fan Outlet;  !- Air Outlet Node Name",

        "Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 Mixed Air Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Fan Outlet;  !- Air Outlet Node Name",

        "Fan:OnOff,",
        "    SPACE2-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE2-1 Mixed Air Outlet,  !- Air Inlet Node Name",
        "    SPACE2-1 Supply Fan Outlet;  !- Air Outlet Node Name",

        "Fan:OnOff,",
        "    SPACE3-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE3-1 Mixed Air Outlet,  !- Air Inlet Node Name",
        "    SPACE3-1 Supply Fan Outlet;  !- Air Outlet Node Name",

        "Fan:OnOff,",
        "    SPACE4-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE4-1 Mixed Air Outlet,  !- Air Inlet Node Name",
        "    SPACE4-1 Supply Fan Outlet;  !- Air Outlet Node Name",

        "Fan:OnOff,",
        "    SPACE5-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE5-1 Mixed Air Outlet,  !- Air Inlet Node Name",
        "    SPACE5-1 Supply Fan Outlet;  !- Air Outlet Node Name",

        "Coil:Cooling:Water,",
        "    SPACE1-1 Cooling Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    SPACE1-1 Cooling Coil ChW Inlet,  !- Water Inlet Node Name",
        "    SPACE1-1 Cooling Coil ChW Outlet,  !- Water Outlet Node Name",
        "    SPACE1-1 Supply Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    DetailedAnalysis,        !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Cooling:Water,",
        "    SPACE2-1 Cooling Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    SPACE2-1 Cooling Coil ChW Inlet,  !- Water Inlet Node Name",
        "    SPACE2-1 Cooling Coil ChW Outlet,  !- Water Outlet Node Name",
        "    SPACE2-1 Supply Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE2-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    DetailedAnalysis,        !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Cooling:Water,",
        "    SPACE3-1 Cooling Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    SPACE3-1 Cooling Coil ChW Inlet,  !- Water Inlet Node Name",
        "    SPACE3-1 Cooling Coil ChW Outlet,  !- Water Outlet Node Name",
        "    SPACE3-1 Supply Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE3-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    DetailedAnalysis,        !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Cooling:Water,",
        "    SPACE4-1 Cooling Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    SPACE4-1 Cooling Coil ChW Inlet,  !- Water Inlet Node Name",
        "    SPACE4-1 Cooling Coil ChW Outlet,  !- Water Outlet Node Name",
        "    SPACE4-1 Supply Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE4-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    DetailedAnalysis,        !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Cooling:Water,",
        "    SPACE5-1 Cooling Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    SPACE5-1 Cooling Coil ChW Inlet,  !- Water Inlet Node Name",
        "    SPACE5-1 Cooling Coil ChW Outlet,  !- Water Outlet Node Name",
        "    SPACE5-1 Supply Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE5-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    DetailedAnalysis,        !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Cooling:Water,",
        "    DOAS Cooling Coil,       !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    DOAS Cooling Coil ChW Inlet,  !- Water Inlet Node Name",
        "    DOAS Cooling Coil ChW Outlet,  !- Water Outlet Node Name",
        "    DOAS Heat Recovery Supply Outlet,   !- Air Inlet Node Name",
        "    DOAS Cooling Coil Outlet,!- Air Outlet Node Name",
        "    DetailedAnalysis,        !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Heating:Water,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    SPACE1-1 Heating Coil HW Inlet,  !- Water Inlet Node Name",
        "    SPACE1-1 Heating Coil HW Outlet,  !- Water Outlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    0.5;                     !- Rated Ratio for Air and Water Convection",

        "Coil:Heating:Water,",
        "    SPACE2-1 Heating Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    SPACE2-1 Heating Coil HW Inlet,  !- Water Inlet Node Name",
        "    SPACE2-1 Heating Coil HW Outlet,  !- Water Outlet Node Name",
        "    SPACE2-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE2-1 Supply Inlet,   !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    0.5;                     !- Rated Ratio for Air and Water Convection",

        "Coil:Heating:Water,",
        "    SPACE3-1 Heating Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    SPACE3-1 Heating Coil HW Inlet,  !- Water Inlet Node Name",
        "    SPACE3-1 Heating Coil HW Outlet,  !- Water Outlet Node Name",
        "    SPACE3-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE3-1 Supply Inlet,   !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    0.5;                     !- Rated Ratio for Air and Water Convection",

        "Coil:Heating:Water,",
        "    SPACE4-1 Heating Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    SPACE4-1 Heating Coil HW Inlet,  !- Water Inlet Node Name",
        "    SPACE4-1 Heating Coil HW Outlet,  !- Water Outlet Node Name",
        "    SPACE4-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE4-1 Supply Inlet,   !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    0.5;                     !- Rated Ratio for Air and Water Convection",

        "Coil:Heating:Water,",
        "    SPACE5-1 Heating Coil,   !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    SPACE5-1 Heating Coil HW Inlet,  !- Water Inlet Node Name",
        "    SPACE5-1 Heating Coil HW Outlet,  !- Water Outlet Node Name",
        "    SPACE5-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE5-1 Supply Inlet,   !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    0.5;                     !- Rated Ratio for Air and Water Convection",

        "Coil:Heating:Water,",
        "    DOAS Heating Coil,       !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    DOAS Heating Coil HW Inlet,  !- Water Inlet Node Name",
        "    DOAS Heating Coil HW Outlet,  !- Water Outlet Node Name",
        "    DOAS Cooling Coil Outlet,!- Air Inlet Node Name",
        "    DOAS Heating Coil Outlet,!- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    0.5;                     !- Rated Ratio for Air and Water Convection",

        "HeatExchanger:AirToAir:SensibleAndLatent,",
        "    DOAS Heat Recovery,      !- Name",
        "    HVACTemplate-Always 1,   !- Availability Schedule Name",
        "    autosize,                !- Nominal Supply Air Flow Rate {m3/s}",
        "    0.7,                     !- Sensible Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.65,                    !- Latent Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.7,                     !- Sensible Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    0.65,                    !- Latent Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    DOAS Mixed Air Outlet,   !- Supply Air Inlet Node Name",
        "    DOAS Heat Recovery Supply Outlet,  !- Supply Air Outlet Node Name",
        "    DOAS Relief Air Outlet,  !- Exhaust Air Inlet Node Name",
        "    DOAS Heat Recovery Relief Outlet,  !- Exhaust Air Outlet Node Name",
        "    0,                       !- Nominal Electric Power {W}",
        "    Yes,                     !- Supply Air Outlet Temperature Control",
        "    Plate,                   !- Heat Exchanger Type",
        "    MinimumExhaustTemperature,  !- Frost Control Type",
        "    1.7,                     !- Threshold Temperature {C}",
        "    0.083,                   !- Initial Defrost Time Fraction {dimensionless}",
        "    0.012,                   !- Rate of Defrost Time Fraction Increase {1/K}",
        "    Yes,                     !- Economizer Lockout",
        "    SenEffectivenessTable,   !- Sensible Effectiveness of Heating Air Flow Curve Name",
        "    LatEffectivenessTable,   !- Latent Effectiveness of Heating Air Flow Curve Name",
        "    SenEffectivenessTable,   !- Sensible Effectiveness of Cooling Air Flow Curve Name",
        "    LatEffectivenessTable;   !- Latent Effectiveness of Cooling Air Flow Curve Name",

        "  Table:IndependentVariable,",
        "    airFlowRatio,  !- Name",
        "    Linear,                  !- Interpolation Method",
        "    Linear,                  !- Extrapolation Method",
        "    0.0,                     !- Minimum Value",
        "    1.0,                     !- Maximum Value",
        "    ,                        !- Normalization Reference Value",
        "    Dimensionless,           !- Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.75,                    !- Value 1",
        "    1.0;                     !- Value 2",

        "  Table:IndependentVariableList,",
        "    effectiveness_IndependentVariableList,  !- Name",
        "    airFlowRatio;     !- Independent Variable 1 Name",

        "  Table:Lookup,",
        "    SenEffectivenessTable,   !- Name",
        "    effectiveness_IndependentVariableList,  !- Independent Variable List Name",
        "    DivisorOnly,             !- Normalization Method",
        "    0.7,                     !- Normalization Divisor",
        "    0.0,                     !- Minimum Output",
        "    1.0,                     !- Maximum Output",
        "    Dimensionless,           !- Output Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.75,                    !- Output Value 1",
        "    0.70;                    !- Output Value 2",

        "  Table:Lookup,",
        "    LatEffectivenessTable,   !- Name",
        "    effectiveness_IndependentVariableList,  !- Independent Variable List Name",
        "    DivisorOnly,             !- Normalization Method",
        "    0.65,                    !- Normalization Divisor",
        "    0.0,                     !- Minimum Output",
        "    1.0,                     !- Maximum Output",
        "    Dimensionless,           !- Output Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.70,                    !- Output Value 1",
        "    0.65;                    !- Output Value 2",

        "Controller:WaterCoil,",
        "    DOAS Cooling Coil Controller,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Reverse,                 !- Action",
        "    Flow,                    !- Actuator Variable",
        "    DOAS Cooling Coil Outlet,!- Sensor Node Name",
        "    DOAS Cooling Coil ChW Inlet,  !- Actuator Node Name",
        "    autosize,                !- Controller Convergence Tolerance {deltaC}",
        "    autosize,                !- Maximum Actuated Flow {m3/s}",
        "    0;                       !- Minimum Actuated Flow {m3/s}",

        "Controller:WaterCoil,",
        "    DOAS Heating Coil Controller,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Normal,                  !- Action",
        "    Flow,                    !- Actuator Variable",
        "    DOAS Heating Coil Outlet,!- Sensor Node Name",
        "    DOAS Heating Coil HW Inlet,  !- Actuator Node Name",
        "    autosize,                !- Controller Convergence Tolerance {deltaC}",
        "    autosize,                !- Maximum Actuated Flow {m3/s}",
        "    0;                       !- Minimum Actuated Flow {m3/s}",

        "Controller:OutdoorAir,",
        "    DOAS OA Controller,      !- Name",
        "    DOAS Relief Air Outlet,  !- Relief Air Outlet Node Name",
        "    DOAS Air Loop Inlet,     !- Return Air Node Name",
        "    DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
        "    DOAS Outdoor Air Inlet,  !- Actuator Node Name",
        "    autosize,                !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    DifferentialEnthalpy,    !- Economizer Control Type",
        "    MinimumFlowWithBypass,   !- Economizer Control Action Type",
        "    ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
        "    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "    ,                        !- Electronic Enthalpy Limit Curve Name",
        "    12.2,                    !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "    NoLockout,               !- Lockout Type",
        "    ProportionalMinimum;     !- Minimum Limit Type",

        "AirLoopHVAC:ControllerList,",
        "    DOAS Controllers,        !- Name",
        "    Controller:WaterCoil,    !- Controller 1 Object Type",
        "    DOAS Cooling Coil Controller,  !- Controller 1 Name",
        "    Controller:WaterCoil,    !- Controller 2 Object Type",
        "    DOAS Heating Coil Controller;  !- Controller 2 Name",

        "AirLoopHVAC:ControllerList,",
        "    DOAS OA System Controllers,  !- Name",
        "    Controller:OutdoorAir,   !- Controller 1 Object Type",
        "    DOAS OA Controller;      !- Controller 1 Name",

        "AirLoopHVAC,",
        "    DOAS,                    !- Name",
        "    DOAS Controllers,        !- Controller List Name",
        "    DOAS Availability Managers,  !- Availability Manager List Name",
        "    autosize,                !- Design Supply Air Flow Rate {m3/s}",
        "    DOAS Branches,           !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    DOAS Air Loop Inlet,     !- Supply Side Inlet Node Name",
        "    DOAS Return Air Outlet,  !- Demand Side Outlet Node Name",
        "    DOAS Supply Path Inlet,  !- Demand Side Inlet Node Names",
        "    DOAS Supply Fan Outlet;  !- Supply Side Outlet Node Names",

        "AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "    DOAS OA System Equipment,!- Name",
        "    OutdoorAir:Mixer,        !- Component 2 Object Type",
        "    DOAS OA Mixing Box;      !- Component 2 Name",

        "AirLoopHVAC:OutdoorAirSystem,",
        "    DOAS OA System,           !- Name",
        "    DOAS OA System Controllers,  !- Controller List Name",
        "    DOAS OA System Equipment; !- Outdoor Air Equipment List Name",

        "OutdoorAir:Mixer,",
        "    SPACE1-1 OA Mixing Box,  !- Name",
        "    SPACE1-1 Mixed Air Outlet,  !- Mixed Air Node Name",
        "    SPACE1-1 Outside Air Inlet,  !- Outdoor Air Stream Node Name",
        "    SPACE1-1 Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    SPACE1-1 Fan Coil Return;!- Return Air Stream Node Name",

        "OutdoorAir:Mixer,",
        "    SPACE2-1 OA Mixing Box,  !- Name",
        "    SPACE2-1 Mixed Air Outlet,  !- Mixed Air Node Name",
        "    SPACE2-1 Outside Air Inlet,  !- Outdoor Air Stream Node Name",
        "    SPACE2-1 Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    SPACE2-1 Fan Coil Return;!- Return Air Stream Node Name",

        "OutdoorAir:Mixer,",
        "    SPACE3-1 OA Mixing Box,  !- Name",
        "    SPACE3-1 Mixed Air Outlet,  !- Mixed Air Node Name",
        "    SPACE3-1 Outside Air Inlet,  !- Outdoor Air Stream Node Name",
        "    SPACE3-1 Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    SPACE3-1 Fan Coil Return;!- Return Air Stream Node Name",

        "OutdoorAir:Mixer,",
        "    SPACE4-1 OA Mixing Box,  !- Name",
        "    SPACE4-1 Mixed Air Outlet,  !- Mixed Air Node Name",
        "    SPACE4-1 Outside Air Inlet,  !- Outdoor Air Stream Node Name",
        "    SPACE4-1 Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    SPACE4-1 Fan Coil Return;!- Return Air Stream Node Name",

        "OutdoorAir:Mixer,",
        "    SPACE5-1 OA Mixing Box,  !- Name",
        "    SPACE5-1 Mixed Air Outlet,  !- Mixed Air Node Name",
        "    SPACE5-1 Outside Air Inlet,  !- Outdoor Air Stream Node Name",
        "    SPACE5-1 Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    SPACE5-1 Fan Coil Return;!- Return Air Stream Node Name",

        "OutdoorAir:Mixer,",
        "    DOAS OA Mixing Box,      !- Name",
        "    DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
        "    DOAS Outdoor Air Inlet,  !- Outdoor Air Stream Node Name",
        "    DOAS Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    DOAS Air Loop Inlet;     !- Return Air Stream Node Name",

        "AirLoopHVAC:ZoneSplitter,",
        "    DOAS Zone Splitter,      !- Name",
        "    DOAS Supply Path Inlet,  !- Inlet Node Name",
        "    SPACE1-1 Zone Equip Inlet,  !- Outlet 1 Node Name",
        "    SPACE2-1 Zone Equip Inlet,  !- Outlet 2 Node Name",
        "    SPACE3-1 Zone Equip Inlet,  !- Outlet 3 Node Name",
        "    SPACE4-1 Zone Equip Inlet,  !- Outlet 4 Node Name",
        "    SPACE5-1 Zone Equip Inlet;  !- Outlet 5 Node Name",

        "AirLoopHVAC:SupplyPath,",
        "    DOAS Supply Path,        !- Name",
        "    DOAS Supply Path Inlet,  !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    DOAS Zone Splitter;      !- Component 1 Name",

        "AirLoopHVAC:ZoneMixer,",
        "    DOAS Zone Mixer,         !- Name",
        "    DOAS Return Air Outlet,  !- Outlet Node Name",
        "    SPACE1-1 Return Outlet,  !- Inlet 1 Node Name",
        "    SPACE2-1 Return Outlet,  !- Inlet 2 Node Name",
        "    SPACE3-1 Return Outlet,  !- Inlet 3 Node Name",
        "    SPACE4-1 Return Outlet,  !- Inlet 4 Node Name",
        "    SPACE5-1 Return Outlet;  !- Inlet 5 Node Name",

        "AirLoopHVAC:ReturnPath,",
        "    DOAS Return Path,        !- Name",
        "    DOAS Return Air Outlet,  !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    DOAS Zone Mixer;         !- Component 1 Name",

        "Branch,",
        "    SPACE1-1 Cooling Coil ChW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water,      !- Component 1 Object Type",
        "    SPACE1-1 Cooling Coil,   !- Component 1 Name",
        "    SPACE1-1 Cooling Coil ChW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE1-1 Cooling Coil ChW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE1-1 Heating Coil HW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    SPACE1-1 Heating Coil,   !- Component 1 Name",
        "    SPACE1-1 Heating Coil HW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE1-1 Heating Coil HW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE2-1 Cooling Coil ChW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water,      !- Component 1 Object Type",
        "    SPACE2-1 Cooling Coil,   !- Component 1 Name",
        "    SPACE2-1 Cooling Coil ChW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE2-1 Cooling Coil ChW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE2-1 Heating Coil HW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    SPACE2-1 Heating Coil,   !- Component 1 Name",
        "    SPACE2-1 Heating Coil HW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE2-1 Heating Coil HW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE3-1 Cooling Coil ChW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water,      !- Component 1 Object Type",
        "    SPACE3-1 Cooling Coil,   !- Component 1 Name",
        "    SPACE3-1 Cooling Coil ChW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE3-1 Cooling Coil ChW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE3-1 Heating Coil HW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    SPACE3-1 Heating Coil,   !- Component 1 Name",
        "    SPACE3-1 Heating Coil HW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE3-1 Heating Coil HW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE4-1 Cooling Coil ChW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water,      !- Component 1 Object Type",
        "    SPACE4-1 Cooling Coil,   !- Component 1 Name",
        "    SPACE4-1 Cooling Coil ChW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE4-1 Cooling Coil ChW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE4-1 Heating Coil HW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    SPACE4-1 Heating Coil,   !- Component 1 Name",
        "    SPACE4-1 Heating Coil HW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE4-1 Heating Coil HW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE5-1 Cooling Coil ChW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water,      !- Component 1 Object Type",
        "    SPACE5-1 Cooling Coil,   !- Component 1 Name",
        "    SPACE5-1 Cooling Coil ChW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE5-1 Cooling Coil ChW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    SPACE5-1 Heating Coil HW Branch,  !- Name",

        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    SPACE5-1 Heating Coil,   !- Component 1 Name",
        "    SPACE5-1 Heating Coil HW Inlet,  !- Component 1 Inlet Node Name",
        "    SPACE5-1 Heating Coil HW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    DOAS Main Branch,        !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "    DOAS OA System,          !- Component 1 Name",
        "    DOAS Air Loop Inlet,     !- Component 1 Inlet Node Name",
        "    DOAS Mixed Air Outlet,   !- Component 1 Outlet Node Name",
        "    HeatExchanger:AirToAir:SensibleAndLatent,  !- Component 1 Object Type",
        "    DOAS Heat Recovery,      !- Component 2 Name",
        "    DOAS Mixed Air Outlet,   !- Component 2 Inlet Node Name",
        "    DOAS Heat Recovery Supply Outlet,  !- Component 2 Outlet Node Name",
        "    Coil:Cooling:Water,      !- Component 3 Object Type",
        "    DOAS Cooling Coil,       !- Component 3 Name",
        "    DOAS Heat Recovery Supply Outlet,   !- Component 3 Inlet Node Name",
        "    DOAS Cooling Coil Outlet,!- Component 3 Outlet Node Name",
        "    Coil:Heating:Water,      !- Component 4 Object Type",
        "    DOAS Heating Coil,       !- Component 4 Name",
        "    DOAS Cooling Coil Outlet,!- Component 4 Inlet Node Name",
        "    DOAS Heating Coil Outlet,!- Component 4 Outlet Node Name",
        "    Fan:VariableVolume,      !- Component 5 Object Type",
        "    DOAS Supply Fan,         !- Component 5 Name",
        "    DOAS Heating Coil Outlet,!- Component 5 Inlet Node Name",
        "    DOAS Supply Fan Outlet;  !- Component 5 Outlet Node Name",

        "Branch,",
        "    DOAS Cooling Coil ChW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water,      !- Component 1 Object Type",
        "    DOAS Cooling Coil,       !- Component 1 Name",
        "    DOAS Cooling Coil ChW Inlet,  !- Component 1 Inlet Node Name",
        "    DOAS Cooling Coil ChW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    DOAS Heating Coil HW Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    DOAS Heating Coil,       !- Component 1 Name",
        "    DOAS Heating Coil HW Inlet,  !- Component 1 Inlet Node Name",
        "    DOAS Heating Coil HW Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Boiler HW Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    Purchased Heating,         !- Component 1 Name",
        "    Purchased Heat Inlet Node, !- Component 1 Inlet Node Name",
        "    Purchased Heat Outlet Node;   !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Chiller ChW Branch, !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictCooling,         !- Component 1 Object Type",
        "    Purchased Cooling,            !- Component 1 Name",
        "    Purchased Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Cooling Outlet Node; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Hot Water Loop HW Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Hot Water Loop HW Supply Side Bypass Pipe,  !- Component 1 Name",
        "    Hot Water Loop HW Supply Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Hot Water Loop HW Supply Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Hot Water Loop HW Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Hot Water Loop HW Supply Pump,  !- Component 1 Name",
        "    Hot Water Loop HW Supply Inlet,  !- Component 1 Inlet Node Name",
        "    Hot Water Loop HW Pump Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Hot Water Loop HW Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Hot Water Loop HW Supply Outlet Pipe,  !- Component 1 Name",
        "    Hot Water Loop HW Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Hot Water Loop HW Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Hot Water Loop HW Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Hot Water Loop HW Demand Inlet Pipe,  !- Component 1 Name",
        "    Hot Water Loop HW Demand Inlet,  !- Component 1 Inlet Node Name",
        "    Hot Water Loop HW Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Hot Water Loop HW Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Hot Water Loop HW Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Hot Water Loop HW Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Hot Water Loop HW Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Hot Water Loop HW Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Hot Water Loop HW Demand Outlet Pipe,  !- Component 1 Name",
        "    Hot Water Loop HW Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Hot Water Loop HW Demand Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Chilled Water Loop ChW Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Chilled Water Loop ChW Supply Side Bypass Pipe,  !- Component 1 Name",
        "    Chilled Water Loop ChW Supply Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Chilled Water Loop ChW Supply Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Chilled Water Loop ChW Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Chilled Water Loop ChW Supply Pump,  !- Component 1 Name",
        "    Chilled Water Loop ChW Supply Inlet,  !- Component 1 Inlet Node Name",
        "    Chilled Water Loop ChW Pump Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Chilled Water Loop ChW Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Chilled Water Loop ChW Supply Outlet Pipe,  !- Component 1 Name",
        "    Chilled Water Loop ChW Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Chilled Water Loop ChW Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Chilled Water Loop ChW Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Chilled Water Loop ChW Demand Inlet Pipe,  !- Component 1 Name",
        "    Chilled Water Loop ChW Demand Inlet,  !- Component 1 Inlet Node Name",
        "    Chilled Water Loop ChW Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Chilled Water Loop ChW Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Chilled Water Loop ChW Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Chilled Water Loop ChW Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Chilled Water Loop ChW Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Chilled Water Loop ChW Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Chilled Water Loop ChW Demand Outlet Pipe,  !- Component 1 Name",
        "    Chilled Water Loop ChW Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Chilled Water Loop ChW Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    DOAS Branches,           !- Name",
        "    DOAS Main Branch;        !- Branch 1 Name",

        "BranchList,",
        "    Hot Water Loop HW Supply Side Branches,  !- Name",
        "    Hot Water Loop HW Supply Inlet Branch,  !- Branch 1 Name",
        "    Main Boiler HW Branch,   !- Branch 2 Name",
        "    Hot Water Loop HW Supply Bypass Branch,  !- Branch 3 Name",
        "    Hot Water Loop HW Supply Outlet Branch;  !- Branch 4 Name",

        "BranchList,",
        "    Hot Water Loop HW Demand Side Branches,  !- Name",
        "    Hot Water Loop HW Demand Inlet Branch,  !- Branch 1 Name",
        "    SPACE1-1 Heating Coil HW Branch,  !- Branch 2 Name",
        "    SPACE2-1 Heating Coil HW Branch,  !- Branch 3 Name",
        "    SPACE3-1 Heating Coil HW Branch,  !- Branch 4 Name",
        "    SPACE4-1 Heating Coil HW Branch,  !- Branch 5 Name",
        "    SPACE5-1 Heating Coil HW Branch,  !- Branch 6 Name",
        "    DOAS Heating Coil HW Branch,  !- Branch 7 Name",
        "    Hot Water Loop HW Demand Bypass Branch,  !- Branch 8 Name",
        "    Hot Water Loop HW Demand Outlet Branch;  !- Branch 9 Name",

        "BranchList,",
        "    Chilled Water Loop ChW Supply Side Branches,  !- Name",
        "    Chilled Water Loop ChW Supply Inlet Branch,  !- Branch 1 Name",
        "    Main Chiller ChW Branch, !- Branch 2 Name",
        "    Chilled Water Loop ChW Supply Bypass Branch,  !- Branch 3 Name",
        "    Chilled Water Loop ChW Supply Outlet Branch;  !- Branch 4 Name",

        "BranchList,",
        "    Chilled Water Loop ChW Demand Side Branches,  !- Name",
        "    Chilled Water Loop ChW Demand Inlet Branch,  !- Branch 1 Name",
        "    SPACE1-1 Cooling Coil ChW Branch,  !- Branch 2 Name",
        "    SPACE2-1 Cooling Coil ChW Branch,  !- Branch 3 Name",
        "    SPACE3-1 Cooling Coil ChW Branch,  !- Branch 4 Name",
        "    SPACE4-1 Cooling Coil ChW Branch,  !- Branch 5 Name",
        "    SPACE5-1 Cooling Coil ChW Branch,  !- Branch 6 Name",
        "    DOAS Cooling Coil ChW Branch,  !- Branch 7 Name",
        "    Chilled Water Loop ChW Demand Bypass Branch,  !- Branch 8 Name",
        "    Chilled Water Loop ChW Demand Outlet Branch;  !- Branch 9 Name",

        "Connector:Splitter,",
        "    Hot Water Loop HW Supply Splitter,  !- Name",
        "    Hot Water Loop HW Supply Inlet Branch,  !- Inlet Branch Name",
        "    Main Boiler HW Branch,   !- Outlet Branch 1 Name",
        "    Hot Water Loop HW Supply Bypass Branch;  !- Outlet Branch 2 Name",

        "Connector:Splitter,",
        "    Hot Water Loop HW Demand Splitter,  !- Name",
        "    Hot Water Loop HW Demand Inlet Branch,  !- Inlet Branch Name",
        "    Hot Water Loop HW Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    SPACE1-1 Heating Coil HW Branch,  !- Outlet Branch 2 Name",
        "    SPACE2-1 Heating Coil HW Branch,  !- Outlet Branch 3 Name",
        "    SPACE3-1 Heating Coil HW Branch,  !- Outlet Branch 4 Name",
        "    SPACE4-1 Heating Coil HW Branch,  !- Outlet Branch 5 Name",
        "    SPACE5-1 Heating Coil HW Branch,  !- Outlet Branch 6 Name",
        "    DOAS Heating Coil HW Branch;  !- Outlet Branch 7 Name",

        "Connector:Splitter,",
        "    Chilled Water Loop ChW Supply Splitter,  !- Name",
        "    Chilled Water Loop ChW Supply Inlet Branch,  !- Inlet Branch Name",
        "    Chilled Water Loop ChW Supply Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Chiller ChW Branch; !- Outlet Branch 2 Name",

        "Connector:Splitter,",
        "    Chilled Water Loop ChW Demand Splitter,  !- Name",
        "    Chilled Water Loop ChW Demand Inlet Branch,  !- Inlet Branch Name",
        "    Chilled Water Loop ChW Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    SPACE1-1 Cooling Coil ChW Branch,  !- Outlet Branch 2 Name",
        "    SPACE2-1 Cooling Coil ChW Branch,  !- Outlet Branch 3 Name",
        "    SPACE3-1 Cooling Coil ChW Branch,  !- Outlet Branch 4 Name",
        "    SPACE4-1 Cooling Coil ChW Branch,  !- Outlet Branch 5 Name",
        "    SPACE5-1 Cooling Coil ChW Branch,  !- Outlet Branch 6 Name",
        "    DOAS Cooling Coil ChW Branch;  !- Outlet Branch 7 Name",

        "Connector:Mixer,",
        "    Hot Water Loop HW Supply Mixer,  !- Name",
        "    Hot Water Loop HW Supply Outlet Branch,  !- Outlet Branch Name",
        "    Main Boiler HW Branch,   !- Inlet Branch 1 Name",
        "    Hot Water Loop HW Supply Bypass Branch;  !- Inlet Branch 2 Name",

        "Connector:Mixer,",
        "    Hot Water Loop HW Demand Mixer,  !- Name",
        "    Hot Water Loop HW Demand Outlet Branch,  !- Outlet Branch Name",
        "    Hot Water Loop HW Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    SPACE1-1 Heating Coil HW Branch,  !- Inlet Branch 2 Name",
        "    SPACE2-1 Heating Coil HW Branch,  !- Inlet Branch 3 Name",
        "    SPACE3-1 Heating Coil HW Branch,  !- Inlet Branch 4 Name",
        "    SPACE4-1 Heating Coil HW Branch,  !- Inlet Branch 5 Name",
        "    SPACE5-1 Heating Coil HW Branch,  !- Inlet Branch 6 Name",
        "    DOAS Heating Coil HW Branch;  !- Inlet Branch 7 Name",

        "Connector:Mixer,",
        "    Chilled Water Loop ChW Supply Mixer,  !- Name",
        "    Chilled Water Loop ChW Supply Outlet Branch,  !- Outlet Branch Name",
        "    Chilled Water Loop ChW Supply Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Chiller ChW Branch; !- Inlet Branch 2 Name",

        "Connector:Mixer,",
        "    Chilled Water Loop ChW Demand Mixer,  !- Name",
        "    Chilled Water Loop ChW Demand Outlet Branch,  !- Outlet Branch Name",
        "    Chilled Water Loop ChW Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    SPACE1-1 Cooling Coil ChW Branch,  !- Inlet Branch 2 Name",
        "    SPACE2-1 Cooling Coil ChW Branch,  !- Inlet Branch 3 Name",
        "    SPACE3-1 Cooling Coil ChW Branch,  !- Inlet Branch 4 Name",
        "    SPACE4-1 Cooling Coil ChW Branch,  !- Inlet Branch 5 Name",
        "    SPACE5-1 Cooling Coil ChW Branch,  !- Inlet Branch 6 Name",
        "    DOAS Cooling Coil ChW Branch;  !- Inlet Branch 7 Name",

        "ConnectorList,",
        "    Hot Water Loop HW Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Hot Water Loop HW Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Hot Water Loop HW Supply Mixer;  !- Connector 2 Name",

        "ConnectorList,",
        "    Hot Water Loop HW Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Hot Water Loop HW Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Hot Water Loop HW Demand Mixer;  !- Connector 2 Name",

        "ConnectorList,",
        "    Chilled Water Loop ChW Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Chilled Water Loop ChW Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Chilled Water Loop ChW Supply Mixer;  !- Connector 2 Name",

        "ConnectorList,",
        "    Chilled Water Loop ChW Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Chilled Water Loop ChW Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Chilled Water Loop ChW Demand Mixer;  !- Connector 2 Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet,   !- Node 1 Name",
        "    SPACE1-1 DOAS Supply Inlet;  !- Node 2 Name",

        "NodeList,",
        "    SPACE2-1 Inlets,         !- Name",
        "    SPACE2-1 Supply Inlet,   !- Node 1 Name",
        "    SPACE2-1 DOAS Supply Inlet;  !- Node 2 Name",

        "NodeList,",
        "    SPACE3-1 Inlets,         !- Name",
        "    SPACE3-1 Supply Inlet,   !- Node 1 Name",
        "    SPACE3-1 DOAS Supply Inlet;  !- Node 2 Name",

        "NodeList,",
        "    SPACE4-1 Inlets,         !- Name",
        "    SPACE4-1 Supply Inlet,   !- Node 1 Name",
        "    SPACE4-1 DOAS Supply Inlet;  !- Node 2 Name",

        "NodeList,",
        "    SPACE5-1 Inlets,         !- Name",
        "    SPACE5-1 Supply Inlet,   !- Node 1 Name",
        "    SPACE5-1 DOAS Supply Inlet;  !- Node 2 Name",

        "NodeList,",
        "    DOAS Cooling Setpoint Nodes,  !- Name",
        "    DOAS Cooling Coil Outlet;!- Node 1 Name",

        "NodeList,",
        "    Hot Water Loop HW Supply Setpoint Nodes,  !- Name",
        "    Purchased Heat Outlet Node,   !- Node 1 Name",
        "    Hot Water Loop HW Supply Outlet;  !- Node 2 Name",

        "NodeList,",
        "    Chilled Water Loop ChW Supply Setpoint Nodes,  !- Name",
        "    Purchased Cooling Outlet Node, !- Node 1 Name",
        "    Chilled Water Loop ChW Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:NodeList,",
        "    SPACE1-1 Outside Air Inlet;  !- Node or NodeList Name 1",

        "OutdoorAir:NodeList,",
        "    SPACE2-1 Outside Air Inlet;  !- Node or NodeList Name 1",

        "OutdoorAir:NodeList,",
        "    SPACE3-1 Outside Air Inlet;  !- Node or NodeList Name 1",

        "OutdoorAir:NodeList,",
        "    SPACE4-1 Outside Air Inlet;  !- Node or NodeList Name 1",

        "OutdoorAir:NodeList,",
        "    SPACE5-1 Outside Air Inlet;  !- Node or NodeList Name 1",

        "OutdoorAir:NodeList,",
        "    DOAS Outdoor Air Inlet;  !- Node or NodeList Name 1",

        "Pipe:Adiabatic,",
        "    Hot Water Loop HW Supply Side Bypass Pipe,  !- Name",
        "    Hot Water Loop HW Supply Bypass Inlet,  !- Inlet Node Name",
        "    Hot Water Loop HW Supply Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Hot Water Loop HW Supply Outlet Pipe,  !- Name",
        "    Hot Water Loop HW Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Hot Water Loop HW Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Hot Water Loop HW Demand Inlet Pipe,  !- Name",
        "    Hot Water Loop HW Demand Inlet,  !- Inlet Node Name",
        "    Hot Water Loop HW Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Hot Water Loop HW Demand Side Bypass Pipe,  !- Name",
        "    Hot Water Loop HW Demand Bypass Inlet,  !- Inlet Node Name",
        "    Hot Water Loop HW Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Hot Water Loop HW Demand Outlet Pipe,  !- Name",
        "    Hot Water Loop HW Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Hot Water Loop HW Demand Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Chilled Water Loop ChW Supply Side Bypass Pipe,  !- Name",
        "    Chilled Water Loop ChW Supply Bypass Inlet,  !- Inlet Node Name",
        "    Chilled Water Loop ChW Supply Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Chilled Water Loop ChW Supply Outlet Pipe,  !- Name",
        "    Chilled Water Loop ChW Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Chilled Water Loop ChW Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Chilled Water Loop ChW Demand Inlet Pipe,  !- Name",
        "    Chilled Water Loop ChW Demand Inlet,  !- Inlet Node Name",
        "    Chilled Water Loop ChW Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Chilled Water Loop ChW Demand Side Bypass Pipe,  !- Name",
        "    Chilled Water Loop ChW Demand Bypass Inlet,  !- Inlet Node Name",
        "    Chilled Water Loop ChW Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Chilled Water Loop ChW Demand Outlet Pipe,  !- Name",
        "    Chilled Water Loop ChW Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Chilled Water Loop ChW Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Hot Water Loop HW Supply Pump,  !- Name",
        "    Hot Water Loop HW Supply Inlet,  !- Inlet Node Name",
        "    Hot Water Loop HW Pump Outlet,  !- Outlet Node Name",
        "    autosize,                !- Rated Flow Rate {m3/s}",
        "    179352,                  !- Rated Pump Head {Pa}",
        "    autosize,                !- Rated Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    INTERMITTENT;            !- Pump Control Type",

        "Pump:ConstantSpeed,",
        "    Chilled Water Loop ChW Supply Pump,  !- Name",
        "    Chilled Water Loop ChW Supply Inlet,  !- Inlet Node Name",
        "    Chilled Water Loop ChW Pump Outlet,  !- Outlet Node Name",
        "    autosize,                !- Rated Flow Rate {m3/s}",
        "    179352,                  !- Rated Pump Head {Pa}",
        "    autosize,                !- Rated Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    INTERMITTENT;            !- Pump Control Type",

        "  DistrictHeating:Water,",
        "    Purchased Heating,          !- Name",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name",
        "    Purchased Heat Outlet Node, !- Hot Water Outlet Node Name",
        "    35000;                      !- Nominal Capacity {W}",

        "  DistrictCooling,",
        "    Purchased Cooling,       !- Name",
        "    Purchased Cooling Inlet Node,  !- Chilled Water Inlet Node Name",
        "    Purchased Cooling Outlet Node,  !- Chilled Water Outlet Node Name",
        "    35000;                   !- Nominal Capacity {W}",

        "PlantLoop,",
        "    Hot Water Loop Hot Water Loop,  !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Hot Water Loop Operation,!- Plant Equipment Operation Scheme Name",
        "    Hot Water Loop HW Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Hot Water Loop HW Supply Inlet,  !- Plant Side Inlet Node Name",
        "    Hot Water Loop HW Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Hot Water Loop HW Supply Side Branches,  !- Plant Side Branch List Name",
        "    Hot Water Loop HW Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Hot Water Loop HW Demand Inlet,  !- Demand Side Inlet Node Name",
        "    Hot Water Loop HW Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Hot Water Loop HW Demand Side Branches,  !- Demand Side Branch List Name",
        "    Hot Water Loop HW Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    SingleSetpoint,          !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "PlantLoop,",
        "    Chilled Water Loop Chilled Water Loop,  !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Chilled Water Loop Chiller Operation,  !- Plant Equipment Operation Scheme Name",
        "    Chilled Water Loop ChW Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    98,                      !- Maximum Loop Temperature {C}",
        "    1,                       !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Chilled Water Loop ChW Supply Inlet,  !- Plant Side Inlet Node Name",
        "    Chilled Water Loop ChW Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Chilled Water Loop ChW Supply Side Branches,  !- Plant Side Branch List Name",
        "    Chilled Water Loop ChW Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Chilled Water Loop ChW Demand Inlet,  !- Demand Side Inlet Node Name",
        "    Chilled Water Loop ChW Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Chilled Water Loop ChW Demand Side Branches,  !- Demand Side Branch List Name",
        "    Chilled Water Loop ChW Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    SingleSetpoint,          !- Plant Loop Demand Calculation Scheme",
        "    None,                    !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "PlantEquipmentList,",
        "    Hot Water Loop All Equipment,           !- Name",
        "    DistrictHeating:Water,          !- Equipment 1 Object Type",
        "    Purchased Heating;        !- Equipment 1 Name",

        "PlantEquipmentList,",
        "    Chilled Water Loop All Chillers,  !- Name",
        "    DistrictCooling,    !- Equipment 1 Object Type",
        "    Purchased Cooling;            !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Chilled Water Loop Chiller Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Chilled Water Loop All Chillers;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperation:HeatingLoad,",
        "    Hot Water Loop Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Hot Water Loop All Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Hot Water Loop Operation,!- Name",
        "    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "    Hot Water Loop Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "PlantEquipmentOperationSchemes,",
        "    Chilled Water Loop Chiller Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Chilled Water Loop Chiller Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "AvailabilityManager:Scheduled,",
        "    DOAS Availability,       !- Name",
        "    FanAvailSched;                !- Schedule Name",

        "AvailabilityManagerAssignmentList,",
        "    DOAS Availability Managers,  !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    DOAS Availability;       !- Availability Manager 1 Name",

        "SetpointManager:Scheduled,",
        "    DOAS Heating Supply Air Temp Manager,",
        "    Temperature,",
        "    HVACTemplate-Always 12.2,",
        "    DOAS Supply Path Inlet;",

        "SetpointManager:OutdoorAirReset,",
        "    DOAS Cooling Supply Air Temp Manager, !- Name",
        "    Temperature,  !- Control Variable",
        "    18.3,                    !- Setpoint at Outdoor Low Temperature {C}",
        "    4.4,                    !- Outdoor Low Temperature (C)",
        "    12.8,                    !- Setpoint at Outdoor High Temperature (C)",
        "    26.7,                    !- Outdoor High Temperature (C)",
        "    DOAS Supply Fan Outlet;  !- Setpoint Node or NodeList Name",

        "SetpointManager:OutdoorAirReset,",
        "    Hot Water Loop HW Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    82.2,                    !- Setpoint at Outdoor Low Temperature {C}",
        "    -6.7,                    !- Outdoor Low Temperature {C}",
        "    65.6,                    !- Setpoint at Outdoor High Temperature {C}",
        "    10,                      !- Outdoor High Temperature {C}",
        "    Hot Water Loop HW Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "SetpointManager:OutdoorAirReset,",
        "    Chilled Water Loop ChW Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    12.2,                    !- Setpoint at Outdoor Low Temperature {C}",
        "    15.6,                    !- Outdoor Low Temperature {C}",
        "    6.7,                     !- Setpoint at Outdoor High Temperature {C}",
        "    26.7,                    !- Outdoor High Temperature {C}",
        "    Chilled Water Loop ChW Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "SetpointManager:MixedAir,",
        "    DOAS Cooling Coil Air Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    DOAS Supply Fan Outlet,  !- Reference Setpoint Node Name",
        "    DOAS Heating Coil Outlet,!- Fan Inlet Node Name",
        "    DOAS Supply Fan Outlet,  !- Fan Outlet Node Name",
        "    DOAS Cooling Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "SetpointManager:MixedAir,",
        "    DOAS Heating Coil Air Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    DOAS Supply Path Inlet,  !- Reference Setpoint Node Name",
        "    DOAS Heating Coil Outlet,!- Fan Inlet Node Name",
        "    DOAS Supply Fan Outlet,  !- Fan Outlet Node Name",
        "    DOAS Heating Coil Outlet;!- Setpoint Node or NodeList Name",

        "SetpointManager:MixedAir,",
        "    DOAS Heat Recovery Air Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    DOAS Supply Path Inlet,  !- Reference Setpoint Node Name",
        "    DOAS Heating Coil Outlet,!- Fan Inlet Node Name",
        "    DOAS Supply Fan Outlet,  !- Fan Outlet Node Name",
        "    DOAS Heat Recovery Supply Outlet;  !- Setpoint Node or NodeList Name",

        "SetpointManager:MixedAir,",
        "    DOAS Heat Recovery Economizer Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    DOAS Supply Fan Outlet,  !- Reference Setpoint Node Name",
        "    DOAS Heating Coil Outlet,!- Fan Inlet Node Name",
        "    DOAS Supply Fan Outlet,  !- Fan Outlet Node Name",
        "    DOAS Mixed Air Outlet;   !- Setpoint Node or NodeList Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // OutputProcessor::TimeValue.allocate(2); //
    ManageSimulation(*state); // run the design day

    ASSERT_EQ("DOAS HEAT RECOVERY", state->dataHeatRecovery->ExchCond(1).Name); // Name of Heat Recovery Exchange On Main Air Loop
    ASSERT_EQ(state->dataHeatRecovery->ExchCond(1).Name,
              state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(2).Name); // Heat Recovery Exchange On Main Air Loop

    ASSERT_NEAR(-17.300, state->dataHeatRecovery->ExchCond(1).SupInTemp, 0.001); // Heat Recovery Exchanger Primary Air Inlet Temp
    ASSERT_GT(state->dataHeatRecovery->ExchCond(1).SupOutTemp,
              state->dataHeatRecovery->ExchCond(1).SupInTemp);                  // Heat Recovery Exchanger is On in heating mode
    ASSERT_NEAR(22.947, state->dataHeatRecovery->ExchCond(1).SecInTemp, 0.001); // Heat Recovery Exchanger Secondary Air Inlet Temp
    ASSERT_LT(state->dataHeatRecovery->ExchCond(1).SecOutTemp,
              state->dataHeatRecovery->ExchCond(1).SecInTemp); // Heat Recovery Exchanger is On in heating mode

    InletNode = state->dataHeatRecovery->ExchCond(1).SupInletNode;
    OutletNode = state->dataHeatRecovery->ExchCond(1).SupOutletNode;
    Qhr_HeatingRateTot = state->dataHeatRecovery->ExchCond(1).SupInMassFlow *
                         (state->dataLoopNodes->Node(OutletNode).Enthalpy - state->dataLoopNodes->Node(InletNode).Enthalpy);
    ASSERT_NEAR(Qhr_HeatingRateTot, state->dataHeatRecovery->ExchCond(1).TotHeatingRate, 0.01);
}

TEST_F(EnergyPlusFixture, SizeHeatRecovery)
{

    int ExchNum(1);
    int BalDesDehumPerfDataIndex(1);
    Real64 FaceVelocity;
    Real64 SysVolFlow;

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(state->dataSize->NumSysSizInput);
    state->dataSize->SysSizPeakDDNum.allocate(state->dataSize->NumSysSizInput);
    state->dataSize->CurSysNum = 1;    // primary air system
    state->dataSize->CurOASysNum = 0;  // no OA system
    state->dataSize->CurZoneEqNum = 0; // size it based on system
    state->dataSize->SysSizInput(state->dataSize->CurSysNum).AirLoopNum = 1;

    // initialize sizing required variables
    state->dataHeatRecovery->ExchCond.allocate(ExchNum);
    state->dataHeatRecovery->ExchCond(ExchNum).type = HVAC::HXType::Desiccant_Balanced;
    state->dataHeatRecovery->ExchCond(ExchNum).ExchConfig = HeatRecovery::HXExchConfigType::Rotary;
    state->dataHeatRecovery->ExchCond(ExchNum).NomSupAirVolFlow = AutoSize;
    state->dataHeatRecovery->ExchCond(ExchNum).PerfDataIndex = BalDesDehumPerfDataIndex;

    state->dataHeatRecovery->BalDesDehumPerfData.allocate(BalDesDehumPerfDataIndex);
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).Name = "DehumPerformanceData";
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NumericFieldNames.allocate(2);

    // autosize nominal vol flow and face velocity
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NumericFieldNames(1) = "Nominal Air Flow Rate";
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NomSupAirVolFlow = AutoSize;
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NumericFieldNames(2) = "Nominal Air Face Velocity";
    state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NomProcAirFaceVel = AutoSize;

    // initialize sizing variables
    state->dataSize->CurDuctType = HVAC::AirDuctType::Main;
    state->dataSize->FinalSysSizing.allocate(state->dataSize->CurSysNum);
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 1.0;

    // initialize UnitarySysEqSizing capacity flag to false; not unitary system
    state->dataSize->UnitarySysEqSizing.allocate(state->dataSize->CurSysNum);
    state->dataSize->UnitarySysEqSizing(state->dataSize->CurSysNum).CoolingCapacity = false;
    state->dataSize->UnitarySysEqSizing(state->dataSize->CurSysNum).HeatingCapacity = false;

    OutputReportPredefined::SetPredefinedTables(*state);

    // calc heat recovery sizing
    state->dataHeatRecovery->ExchCond(ExchNum).size(*state);

    // test autosized nominal vol flow rate
    EXPECT_EQ(1.0, state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NomSupAirVolFlow); // m3/s

    // size nominal face velocity
    SysVolFlow = state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NomSupAirVolFlow;
    FaceVelocity = 4.30551 + 0.01969 * SysVolFlow;

    // test autosized face velocity
    EXPECT_EQ(FaceVelocity, state->dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfDataIndex).NomProcAirFaceVel); // m/s

    std::string_view const compName = state->dataHeatRecovery->ExchCond(ExchNum).Name;
    EXPECT_EQ("HeatExchanger:Desiccant:BalancedFlow",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRInputObjType, compName));
    EXPECT_EQ("Rotary", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRPlateOrRotary, compName));
    EXPECT_EQ("N/A",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRSenEffAt100PerHeatAirFlow, compName));
    EXPECT_EQ("N/A",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRSenEffAt100PerCoolAirFlow, compName));
    EXPECT_EQ("N/A",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRLatEffAt100PerHeatAirFlow, compName));
    EXPECT_EQ("N/A",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRLatEffAt100PerCoolAirFlow, compName));

    EXPECT_EQ("1.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRSupplyAirflow, compName));
    EXPECT_EQ("1.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRExhaustAirflow, compName));
}

TEST_F(EnergyPlusFixture, HeatRecovery_AirFlowSizing)
{

    int ExchNum = 1;

    std::string const idf_objects = delimited_string({
        "  HeatExchanger:AirToAir:SensibleAndLatent,",
        "    HEATRECOVERY HX IN ERV,  !- Name",
        "    ,                        !- Availability Schedule Name",
        "    autosize,                !- Nominal Supply Air Flow Rate {m3/s}",
        "    0.76,                    !- Sensible Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.68,                    !- Latent Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.76,                    !- Sensible Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    0.68,                    !- Latent Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    ERV OA Inlet Node,       !- Supply Air Inlet Node Name",
        "    HR Pri Air Outlet Node,  !- Supply Air Outlet Node Name",
        "    Zone 1 Exhaust Node,     !- Exhaust Air Inlet Node Name",
        "    HR Sec aIR Outlet Node,  !- Exhaust Air Outlet Node Name",
        "    50.0,                    !- Nominal Electric Power {W}",
        "    No,                      !- Supply Air Outlet Temperature Control",
        "    Rotary,                  !- Heat Exchanger Type",
        "    None,                    !- Frost Control Type",
        "    1.7;                     !- Threshold Temperature {C}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    OutputReportPredefined::SetPredefinedTables(*state);

    // get heat recovery heat exchanger generic
    GetHeatRecoveryInput(*state);

    // initialize
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;

    // the HR HX is in Zone Equipment ERV
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow = 1.0;

    // size the HX nominal supply air volume flow rate
    state->dataHeatRecovery->ExchCond(ExchNum).size(*state);

    // verify the name and autosized supply air flow rate
    EXPECT_EQ(state->dataHeatRecovery->ExchCond(ExchNum).Name, "HEATRECOVERY HX IN ERV");
    EXPECT_EQ(state->dataHeatRecovery->ExchCond(ExchNum).NomSupAirVolFlow, 1.0);

    std::string_view const compName = state->dataHeatRecovery->ExchCond(ExchNum).Name;
    EXPECT_EQ("HeatExchanger:AirToAir:SensibleAndLatent",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRInputObjType, compName));
    EXPECT_EQ("Rotary", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRPlateOrRotary, compName));
    EXPECT_EQ("0.76",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRSenEffAt100PerHeatAirFlow, compName));
    EXPECT_EQ("0.76",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRSenEffAt100PerCoolAirFlow, compName));
    EXPECT_EQ("0.68",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRLatEffAt100PerHeatAirFlow, compName));
    EXPECT_EQ("0.68",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRLatEffAt100PerCoolAirFlow, compName));

    EXPECT_EQ("1.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRSupplyAirflow, compName));
    EXPECT_EQ("1.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchAirHRExhaustAirflow, compName));
}

TEST_F(EnergyPlusFixture, HeatRecovery_HeatExchangerGenericCalcTest)
{

    std::string const idf_objects = delimited_string({

        "    AirLoopHVAC:OutdoorAirSystem,",
        "      VAV WITH REHEAT_OA,      !- Name",
        "      VAV WITH REHEAT_OA_Controllers,  !- Controller List Name",
        "      VAV WITH REHEAT_OA_Equipment;  !- Outdoor Air Equipment List Name",

        "    AirLoopHVAC:ControllerList,",
        "      VAV WITH REHEAT_OA_Controllers,  !- Name",
        "      Controller:OutdoorAir,   !- Controller 1 Object Type",
        "      VAV WITH REHEAT_OA_CONTROLLER;  !- Controller 1 Name",

        "    Controller:OutdoorAir,",
        "      VAV WITH REHEAT_OA_CONTROLLER,  !- Name",
        "      VAV WITH REHEAT_OARelief Node,  !- Relief Air Outlet Node Name",
        "      VAV WITH REHEAT Supply Equipment Inlet Node,  !- Return Air Node Name",
        "      VAV WITH REHEAT_OA-VAV WITH REHEAT_CoolCNode,  !- Mixed Air Node Name",
        "      VAV WITH REHEAT_OAInlet Node,  !- Actuator Node Name",
        "      0.5,                     !- Minimum Outdoor Air Flow Rate {m3/s}",
        "      1.0,                     !- Maximum Outdoor Air Flow Rate {m3/s}",
        "      NoEconomizer,            !- Economizer Control Type",
        "      ModulateFlow,            !- Economizer Control Action Type",
        "      ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "      64000.0,                 !- Economizer Maximum Limit Enthalpy {J/kg}",
        "      ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "      ,                        !- Electronic Enthalpy Limit Curve Name",
        "      ,                        !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "      NoLockout,               !- Lockout Type",
        "      FixedMinimum,            !- Minimum Limit Type",
        "      MinOA_Sched,             !- Minimum Outdoor Air Schedule Name",
        "      ,                        !- Minimum Fraction of Outdoor Air Schedule Name",
        "      ,                        !- Maximum Fraction of Outdoor Air Schedule Name",
        "      ,                        !- Mechanical Ventilation Controller Name",
        "      ,                        !- Time of Day Economizer Control Schedule Name",
        "      ,                        !- High Humidity Control",
        "      ,                        !- Humidistat Control Zone Name",
        "      ,                        !- High Humidity Outdoor Air Flow Ratio",
        "      ,                        !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
        "      BypassWhenOAFlowGreaterThanMinimum;  !- Heat Recovery Bypass Control Type",

        "    Schedule:Compact,",
        "      MinOA_Sched,             !- Name",
        "      Fraction,                !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,1.0;        !- Field 3",

        "    ScheduleTypeLimits,",
        "      Fraction,                !- Name",
        "      0.0,                     !- Lower Limit Value",
        "      1.0,                     !- Upper Limit Value",
        "      CONTINUOUS;              !- Numeric Type",

        "    AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "      VAV WITH REHEAT_OA_Equipment,  !- Name",
        "      HeatExchanger:AirToAir:SensibleAndLatent,  !- Component 1 Object Type",
        "      HEATRECOVERY HX GENERIC, !- Component 1 Name",
        "      OutdoorAir:Mixer,        !- Component 2 Object Type",
        "      VAV WITH REHEAT_OAMixing Box;  !- Component 2 Name",

        "    HeatExchanger:AirToAir:SensibleAndLatent,",
        "      HEATRECOVERY HX GENERIC, !- Name",
        "      ,                        !- Availability Schedule Name",
        "      0.5,                     !- Nominal Supply Air Flow Rate {m3/s}",
        "      0.70,                    !- Sensible Effectiveness at 100% Heating Air Flow {dimensionless}",
        "      0.60,                    !- Latent Effectiveness at 100% Heating Air Flow {dimensionless}",
        "      0.75,                    !- Sensible Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "      0.60,                    !- Latent Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "      VAV WITH REHEAT_OAInlet Node,  !- Supply Air Inlet Node Name",
        "      VAV WITH REHEAT Heat Recovery Outlet Node,  !- Supply Air Outlet Node Name",
        "      VAV WITH REHEAT_OARelief Node,  !- Exhaust Air Inlet Node Name",
        "      VAV WITH REHEAT Heat Recovery Secondary Outlet Node,  !- Exhaust Air Outlet Node Name",
        "      800.0,                   !- Nominal Electric Power {W}",
        "      No,                      !- Supply Air Outlet Temperature Control",
        "      Rotary,                  !- Heat Exchanger Type",
        "      None,                    !- Frost Control Type",
        "      1.7,                     !- Threshold Temperature {C}",
        "      ,                        !- Initial Defrost Time Fraction",
        "      ,                        !- Rate of Defrost Time Fraction Increase",
        "      Yes;                     !- Economizer Lockout",

        "    OutdoorAir:Mixer,",
        "      VAV WITH REHEAT_OAMixing Box,  !- Name",
        "      VAV WITH REHEAT_OA-VAV WITH REHEAT_CoolCNode,  !- Mixed Air Node Name",
        "      VAV WITH REHEAT Heat Recovery Outlet Node,  !- Outdoor Air Stream Node Name",
        "      VAV WITH REHEAT_OARelief Node,  !- Relief Air Stream Node Name",
        "      VAV WITH REHEAT Supply Equipment Inlet Node;  !- Return Air Stream Node Name",

        "OutdoorAir:NodeList,",
        "    VAV WITH REHEAT_OAInlet Node;  !- Node or NodeList Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // get OA Controller
    MixedAir::GetOAControllerInputs(*state);
    int OAContrllerNum = 1;
    auto &thisOAController(state->dataMixedAir->OAController(OAContrllerNum));
    EXPECT_EQ(1, state->dataMixedAir->NumOAControllers);
    EXPECT_EQ("VAV WITH REHEAT_OA_CONTROLLER", thisOAController.Name);
    // get OA System
    MixedAir::GetOutsideAirSysInputs(*state);
    int OASysNum = 1;
    auto &thisOASys = state->dataAirLoop->OutsideAirSys(OASysNum);
    thisOASys.OAControllerIndex = MixedAir::GetOAController(*state, thisOAController.Name);
    EXPECT_EQ(1, state->dataAirLoop->NumOASystems);
    EXPECT_EQ("VAV WITH REHEAT_OA", thisOASys.Name);
    // get HR HX generic
    GetHeatRecoveryInput(*state);
    int ExchNum = 1;
    auto &thisHX = state->dataHeatRecovery->ExchCond(ExchNum);
    EXPECT_EQ(thisHX.Name, "HEATRECOVERY HX GENERIC");
    // initialize
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 1;
    // check user-inputs
    EXPECT_ENUM_EQ(thisOAController.Econo, MixedAir::EconoOp::NoEconomizer);
    EXPECT_ENUM_EQ(thisOAController.Lockout, MixedAir::LockoutType::NoLockoutPossible); // no lockout
    EXPECT_EQ(thisOAController.HeatRecoveryBypassControlType, HVAC::BypassWhenOAFlowGreaterThanMinimum);
    EXPECT_FALSE(thisOAController.EconBypass); // no bypass

    int CompanionCoilNum = 0;
    bool HXUnitOn = true;
    bool FirstHVACIteration = false;
    bool EconomizerFlag = false;
    bool HighHumCtrlFlag = false;
    HVAC::FanOp fanOp = HVAC::FanOp::Continuous; // 2 = constant fan

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    thisHX.type = HVAC::HXType::AirToAir_SensAndLatent;
    thisHX.SupInTemp = 10.0;
    thisHX.SecInTemp = 20.0;
    thisHX.SupInHumRat = 0.01;
    thisHX.SecInHumRat = 0.01;
    thisHX.SupInEnth = PsyHFnTdbW(thisHX.SupInTemp, thisHX.SupInHumRat);
    thisHX.SecInEnth = PsyHFnTdbW(thisHX.SecInTemp, thisHX.SecInHumRat);
    state->dataLoopNodes->Node(thisHX.SupInletNode).Temp = thisHX.SupInTemp;
    state->dataLoopNodes->Node(thisHX.SecInletNode).Temp = thisHX.SecInTemp;
    state->dataLoopNodes->Node(thisHX.SupInletNode).HumRat = thisHX.SupInHumRat;
    state->dataLoopNodes->Node(thisHX.SecInletNode).HumRat = thisHX.SecInHumRat;
    state->dataLoopNodes->Node(thisHX.SupInletNode).Enthalpy = thisHX.SupInEnth;
    state->dataLoopNodes->Node(thisHX.SecInletNode).Enthalpy = thisHX.SecInEnth;

    // test 1: primary and secondary flow rate equal
    state->dataLoopNodes->Node(thisHX.SupInletNode).MassFlowRate = thisHX.NomSupAirVolFlow * state->dataEnvrn->StdRhoAir;
    thisHX.NomSecAirVolFlow = thisHX.NomSupAirVolFlow;
    state->dataLoopNodes->Node(thisHX.SecInletNode).MassFlowRate = thisHX.NomSecAirVolFlow * state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(thisHX.SupOutletNode).TempSetPoint = 19.0;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ(10.0, thisHX.SupInTemp);
    EXPECT_DOUBLE_EQ(20.0, thisHX.SecInTemp);
    EXPECT_NEAR(0.70, thisHX.SensEffectiveness, 0.0001);
    EXPECT_NEAR(0.60, thisHX.LatEffectiveness, 0.0001);
    EXPECT_GT(thisHX.SupOutTemp, thisHX.SupInTemp);
    EXPECT_EQ(0, thisHX.UnBalancedErrCount); // balanced flow
    EXPECT_EQ(0, thisHX.LowFlowErrCount);    // flow ratio within range, < 1.3

    // test 2: secondary flow is 10 times primary
    state->dataLoopNodes->Node(thisHX.SupInletNode).MassFlowRate = thisHX.NomSupAirVolFlow * state->dataEnvrn->StdRhoAir;
    thisHX.NomSecAirVolFlow = 10.0 * thisHX.NomSupAirVolFlow;
    state->dataLoopNodes->Node(thisHX.SecInletNode).MassFlowRate = thisHX.NomSecAirVolFlow * state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(thisHX.SupOutletNode).TempSetPoint = 19.0;
    thisHX.initialize(*state, CompanionCoilNum, 0);
    thisHX.CalcAirToAirGenericHeatExch(*state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag);
    thisHX.UpdateHeatRecovery(*state);
    EXPECT_DOUBLE_EQ(10.0, thisHX.SupInTemp);
    EXPECT_DOUBLE_EQ(20.0, thisHX.SecInTemp);
    EXPECT_NEAR(0.70, thisHX.SensEffectiveness, 0.0001);
    EXPECT_NEAR(0.60, thisHX.LatEffectiveness, 0.0001);
    EXPECT_GT(thisHX.SupOutTemp, thisHX.SupInTemp);
    EXPECT_EQ(1, thisHX.UnBalancedErrCount); // unbalanced flow
    EXPECT_EQ(1, thisHX.LowFlowErrCount);    // out fo range flow ratio, > 1.3
}

TEST_F(EnergyPlusFixture, HeatRecovery_NominalAirFlowAutosizeTest)
{

    std::string const idf_objects = delimited_string({
        "  HeatExchanger:AirToAir:SensibleAndLatent,",
        "    HeatRecovery HX Generic, !- Name",
        "    ,                        !- Availability Schedule Name",
        "    autosize,                !- Nominal Supply Air Flow Rate {m3/s}",
        "    0.76,                    !- Sensible Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.0,                     !- Latent Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.76,                    !- Sensible Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    0.0,                     !- Latent Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    OA Inlet Node,           !- Supply Air Inlet Node Name",
        "    HX Pri Air Outlet Node,  !- Supply Air Outlet Node Name",
        "    Return Air Node,         !- Exhaust Air Inlet Node Name",
        "    HX Sec Air Outlet Node,  !- Exhaust Air Outlet Node Name",
        "    50.0,                    !- Nominal Electric Power {W}",
        "    No,                      !- Supply Air Outlet Temperature Control",
        "    Rotary,                  !- Heat Exchanger Type",
        "    None,                    !- Frost Control Type",
        "    1.7,                     !- Threshold Temperature {C}",
        "    ,                        !- Initial Defrost Time Fraction",
        "    ,                        !- Rate of Defrost Time Fraction Increase",
        "    Yes;                     !- Economizer Lockout",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // get HR HX generic
    GetHeatRecoveryInput(*state);
    int ExchNum = 1;
    auto &thisHX = state->dataHeatRecovery->ExchCond(ExchNum);
    // check inputs
    EXPECT_EQ(thisHX.Name, "HEATRECOVERY HX GENERIC");
    EXPECT_EQ(thisHX.NomSupAirVolFlow, DataSizing::AutoSize);

    // set up sizing parameters
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);

    int OAContrllerNum = 1;
    state->dataMixedAir->OAController.allocate(OAContrllerNum);
    auto &thisOAController(state->dataMixedAir->OAController(OAContrllerNum));
    // initialize OA controller
    thisOAController.ControllerType = MixedAir::MixedAirControllerType::ControllerOutsideAir;

    int OASysNum = 1;
    state->dataAirLoop->OutsideAirSys.allocate(OASysNum);
    auto &thisOASys = state->dataAirLoop->OutsideAirSys(OASysNum);
    thisOASys.OAControllerIndex = 1;

    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.20;
    state->dataSize->CurDuctType = HVAC::AirDuctType::Main;

    // test 1: the HX is in OA System, no economizer, no-bypass
    thisOAController.Econo = MixedAir::EconoOp::NoEconomizer;
    thisOAController.Lockout = MixedAir::LockoutType::NoLockoutPossible; // no lockout
    thisOAController.HeatRecoveryBypassControlType = HVAC::BypassWhenOAFlowGreaterThanMinimum;
    thisOAController.EconBypass = false; // economizer control action type, no bypass
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 0.20);

    // test 2: the HX is on OA system but with economizer, and no-bypass
    thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulb; // with economizer
    thisHX.NomSupAirVolFlow = DataSizing::AutoSize;
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 0.20); // minimum flow

    // test 3: the HX is on OA system but with economizer, and no-bypass
    thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulb; // with economizer
    thisOAController.HeatRecoveryBypassControlType = HVAC::BypassWhenWithinEconomizerLimits;
    thisHX.NomSupAirVolFlow = DataSizing::AutoSize;
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 0.2); // maximum flow

    // test 4: the HX is on OA system, with economizer and lockout
    thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulb;
    thisOAController.Lockout = MixedAir::LockoutType::LockoutWithHeatingPossible; // lockout
    thisHX.NomSupAirVolFlow = DataSizing::AutoSize;
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 0.20);

    // test 5: the HX is on OA system, with economizer and lockout
    thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulb;
    thisOAController.Lockout = MixedAir::LockoutType::LockoutWithCompressorPossible; // lockout
    thisHX.NomSupAirVolFlow = DataSizing::AutoSize;
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 0.20);

    // test 6: the HX is on OA system but with economizer and bypass
    thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulb;
    thisOAController.EconBypass = true; // with bypass
    thisHX.NomSupAirVolFlow = DataSizing::AutoSize;
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 0.20);

    // test 7: the HX is on main air loop
    thisHX.NomSupAirVolFlow = DataSizing::AutoSize;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    // run HX sizing calculation
    thisHX.size(*state);
    // check autosized nominal supply flow
    EXPECT_EQ(thisHX.NomSupAirVolFlow, 1.0);
}
