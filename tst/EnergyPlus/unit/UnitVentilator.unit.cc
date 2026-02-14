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

// EnergyPlus::Pumps Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/UnitVentilator.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, UnitVentilatorSetOAMassFlowRateForCoolingVariablePercentTest)
{
    Real64 MinOA;
    Real64 MassFlowRate;
    Real64 MaxOA;
    Real64 Tinlet;
    Real64 Toutdoor;
    Real64 OAMassFlowRate;
    Real64 ExpectedOAMassFlowRate;
    Real64 UnitVentNum;

    state->dataLoopNodes->clear_state();
    state->dataUnitVentilators->clear_state();

    state->dataLoopNodes->Node.allocate(4);
    state->dataUnitVentilators->UnitVent.allocate(1);

    state->dataUnitVentilators->UnitVent(1).ATMixerExists = false;
    state->dataUnitVentilators->UnitVent(1).ATMixerType = HVAC::MixerType::InletSide;
    state->dataUnitVentilators->UnitVent(1).FanOutletNode = 1;
    state->dataUnitVentilators->UnitVent(1).OAMixerOutNode = 2;
    state->dataUnitVentilators->UnitVent(1).ATMixerOutNode = 3;
    state->dataUnitVentilators->UnitVent(1).AirInNode = 4;
    state->dataLoopNodes->Node(1).Enthalpy = 0.0;
    state->dataLoopNodes->Node(2).Enthalpy = 0.0;
    state->dataLoopNodes->Node(3).Enthalpy = 0.0;
    state->dataLoopNodes->Node(4).Enthalpy = 0.0;

    // Test 1: Tinlet <= Toutdoor
    UnitVentNum = 1;
    Tinlet = 20.0;
    Toutdoor = 23.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 2345.6;
    state->dataEnvrn->OutHumRat = 0.008;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 0.1234;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);

    // Test 2: Toutdoor < Tinlet, OA limited by OAMin
    Tinlet = 23.0;
    Toutdoor = 1.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 1.5678;
    state->dataEnvrn->OutHumRat = 0.008;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 0.1234;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);

    // Test 3: Toutdoor < Tinlet, OA limited by OAMax
    Tinlet = 23.0;
    Toutdoor = 20.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 4567.89;
    state->dataEnvrn->OutHumRat = 0.010;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 1.1106;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);

    // Test 4: Toutdoor < Tinlet, OA between OAMin and OAMax
    Tinlet = 23.0;
    Toutdoor = 8.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 15678.9;
    state->dataEnvrn->OutHumRat = 0.010;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 1.02133;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);

    // Test 5: Toutdoor < Tinlet, OA between OAMin and OAMax, with fan heat, no AT mixer
    Tinlet = 23.0;
    Toutdoor = 8.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 15678.9 - 12.34;
    state->dataEnvrn->OutHumRat = 0.010;
    state->dataLoopNodes->Node(1).Enthalpy = 11.0;
    state->dataLoopNodes->Node(2).Enthalpy = 1.0;
    state->dataLoopNodes->Node(3).Enthalpy = 0.0;
    state->dataLoopNodes->Node(4).Enthalpy = 0.0;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 1.02133;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);

    // Test 6: Toutdoor < Tinlet, OA between OAMin and OAMax, with fan heat, with AT mixer inlet side
    Tinlet = 23.0;
    Toutdoor = 8.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 15678.9 - 12.34;
    state->dataEnvrn->OutHumRat = 0.010;
    state->dataLoopNodes->Node(1).Enthalpy = 11.0;
    state->dataLoopNodes->Node(2).Enthalpy = 0.0;
    state->dataLoopNodes->Node(3).Enthalpy = 1.0;
    state->dataLoopNodes->Node(4).Enthalpy = 0.0;
    state->dataUnitVentilators->UnitVent(1).ATMixerExists = true;
    state->dataUnitVentilators->UnitVent(1).ATMixerType = HVAC::MixerType::InletSide;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 1.02133;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);

    // Test 7: Toutdoor < Tinlet, OA between OAMin and OAMax, with fan heat, with AT mixer inlet side
    Tinlet = 23.0;
    Toutdoor = 8.0;
    MinOA = 0.1;
    MaxOA = 0.9;
    MassFlowRate = 1.234;
    state->dataUnitVentilators->QZnReq = 15678.9 - 12.34;
    state->dataEnvrn->OutHumRat = 0.010;
    state->dataLoopNodes->Node(1).Enthalpy = 11.0;
    state->dataLoopNodes->Node(2).Enthalpy = 0.0;
    state->dataLoopNodes->Node(3).Enthalpy = 0.0;
    state->dataLoopNodes->Node(4).Enthalpy = 1.0;
    state->dataUnitVentilators->UnitVent(1).ATMixerExists = true;
    state->dataUnitVentilators->UnitVent(1).ATMixerType = HVAC::MixerType::SupplySide;

    OAMassFlowRate = UnitVentilator::SetOAMassFlowRateForCoolingVariablePercent(*state, UnitVentNum, MinOA, MassFlowRate, MaxOA, Tinlet, Toutdoor);
    ExpectedOAMassFlowRate = 1.02133;

    EXPECT_NEAR(ExpectedOAMassFlowRate, OAMassFlowRate, 0.0001);
}

TEST_F(EnergyPlusFixture, UnitVentilatorCalcMdotCCoilCycFanTest)
{

    Real64 QZnReq;
    Real64 QCoilReq;
    int UnitVentNum;
    Real64 PartLoadRatio;
    Real64 mdot;
    Real64 ExpectedResult;

    UnitVentNum = 1;
    state->dataUnitVentilators->UnitVent.allocate(UnitVentNum);
    state->dataUnitVentilators->UnitVent(UnitVentNum).FanOutletNode = 1;
    state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode = 2;
    state->dataLoopNodes->Node.allocate(2);
    state->dataLoopNodes->Node(2).HumRat = 0.006;
    state->dataLoopNodes->Node(2).Temp = 23.0;
    state->dataLoopNodes->Node(1).Temp = 23.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 1.0;

    // Test 1: QZnReq is greater than zero (heating) so mdot should be zero after the call
    state->dataUnitVentilators->UnitVent(1).MaxColdWaterFlow = 0.1234;
    mdot = -0.9999;
    QCoilReq = 5678.9;
    QZnReq = 5678.9;
    PartLoadRatio = 1.0;
    ExpectedResult = 0.0;
    UnitVentilator::CalcMdotCCoilCycFan(*state, mdot, QCoilReq, QZnReq, UnitVentNum, PartLoadRatio);

    EXPECT_NEAR(ExpectedResult, mdot, 0.0001);
    EXPECT_NEAR(ExpectedResult, QCoilReq, 0.0001);

    // Test 2: QZnReq is zero (no conditioning) so mdot should be zero after the call
    state->dataUnitVentilators->UnitVent(1).MaxColdWaterFlow = 0.1234;
    mdot = -0.9999;
    QCoilReq = 0.0;
    QZnReq = 0.0;
    PartLoadRatio = 1.0;
    ExpectedResult = 0.0;
    UnitVentilator::CalcMdotCCoilCycFan(*state, mdot, QCoilReq, QZnReq, UnitVentNum, PartLoadRatio);

    EXPECT_NEAR(ExpectedResult, mdot, 0.0001);
    EXPECT_NEAR(ExpectedResult, QCoilReq, 0.0001);

    // Test 3a: QZnReq is less than zero (cooling) so mdot should be non-zero, calculated based on the other variables
    state->dataUnitVentilators->UnitVent(1).MaxColdWaterFlow = 0.1234;
    mdot = -0.9999;
    QCoilReq = -5678.9;
    QZnReq = -5678.9;
    PartLoadRatio = 1.0;
    ExpectedResult = 0.1234;
    UnitVentilator::CalcMdotCCoilCycFan(*state, mdot, QCoilReq, QZnReq, UnitVentNum, PartLoadRatio);

    EXPECT_NEAR(ExpectedResult, mdot, 0.0001);
    EXPECT_NEAR(QCoilReq, QZnReq, 0.1);

    // Test 3b: QZnReq is less than zero (cooling) so mdot should be non-zero, calculated based on the other variables
    state->dataUnitVentilators->UnitVent(1).MaxColdWaterFlow = 1.6;
    mdot = -0.9999;
    QCoilReq = -5678.9;
    QZnReq = -5678.9;
    PartLoadRatio = 0.5;
    ExpectedResult = 0.8;
    UnitVentilator::CalcMdotCCoilCycFan(*state, mdot, QCoilReq, QZnReq, UnitVentNum, PartLoadRatio);

    EXPECT_NEAR(ExpectedResult, mdot, 0.0001);
    EXPECT_NEAR(QCoilReq, QZnReq, 0.1);
}

TEST_F(EnergyPlusFixture, UnitVentilatorOASizing)
{
    int UnitVentNum = 1;
    int numNumericFields = 5;
    state->dataUnitVentilators->UnitVent.allocate(UnitVentNum);
    state->dataSize->ZoneEqSizing.allocate(UnitVentNum);
    state->dataSize->FinalZoneSizing.allocate(UnitVentNum);
    state->dataSize->ZoneHVACSizing.allocate(UnitVentNum);
    state->dataUnitVentilators->UnitVentNumericFields.allocate(UnitVentNum);
    state->dataUnitVentilators->UnitVentNumericFields(UnitVentNum).FieldNames.allocate(numNumericFields);

    state->dataSize->CurZoneEqNum = UnitVentNum;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA = 0.1;

    state->dataUnitVentilators->UnitVent(UnitVentNum).HVACSizingIndex = 1;
    state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow = 1.0;
    state->dataUnitVentilators->UnitVent(UnitVentNum).CoilOption = UnitVentilator::CoilsUsed::None;

    // test fixed OA control
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow = DataSizing::AutoSize;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OAControlType = UnitVentilator::OAControl::FixedAmount;
    UnitVentilator::SizeUnitVentilator(*state, UnitVentNum);

    EXPECT_NEAR(state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow,
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA,
                0.001);

    // test variable percent OA control
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow = DataSizing::AutoSize;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OAControlType = UnitVentilator::OAControl::VariablePercent;
    UnitVentilator::SizeUnitVentilator(*state, UnitVentNum);

    EXPECT_NEAR(
        state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow, state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow, 0.001);

    // test fixed temperature OA control
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow = DataSizing::AutoSize;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OAControlType = UnitVentilator::OAControl::FixedTemperature;
    UnitVentilator::SizeUnitVentilator(*state, UnitVentNum);

    EXPECT_NEAR(
        state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow, state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow, 0.001);

    // retest without HVAC sizing object
    state->dataUnitVentilators->UnitVent(UnitVentNum).HVACSizingIndex = 0;

    // test fixed OA control
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow = DataSizing::AutoSize;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OAControlType = UnitVentilator::OAControl::FixedAmount;
    UnitVentilator::SizeUnitVentilator(*state, UnitVentNum);

    EXPECT_NEAR(state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow,
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA,
                0.001);

    // test variable percent OA control
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow = DataSizing::AutoSize;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OAControlType = UnitVentilator::OAControl::VariablePercent;
    UnitVentilator::SizeUnitVentilator(*state, UnitVentNum);

    EXPECT_NEAR(
        state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow, state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow, 0.001);

    // test fixed temperature OA control
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow = DataSizing::AutoSize;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OAControlType = UnitVentilator::OAControl::FixedTemperature;
    UnitVentilator::SizeUnitVentilator(*state, UnitVentNum);

    EXPECT_NEAR(
        state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow, state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow, 0.001);
}

} // namespace EnergyPlus
