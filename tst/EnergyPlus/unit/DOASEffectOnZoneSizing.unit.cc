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

// EnergyPlus::Standalone unit tests of DOAS effect on zone sizing feature

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace ZoneEquipmentManager;
using namespace DataLoopNode;
using namespace DataSizing;
using namespace DataZoneEquipment;
using namespace DataEnvironment;
using namespace DataZoneEnergyDemands;
using namespace Psychrometrics;
using namespace DataHeatBalFanSys;
using namespace DataHeatBalance;

TEST_F(EnergyPlusFixture, DOASEffectOnZoneSizing_CalcDOASSupCondsForSizing)
{
    state->init_state(*state);
    // locals
    Real64 OutDB;        // outside air temperature [C]
    Real64 OutHR;        // outside humidity ratio [kg Water / kg Dry Air]
    Real64 DOASLowTemp;  // DOAS low setpoint [C]
    Real64 DOASHighTemp; // DOAS high setpoint [C]
    Real64 DOASSupTemp;  // DOAS supply temperature [C]
    Real64 DOASSupHR;    // DOAS supply humidity ratio [kg H2O / kg dry air]
    // neutral supply air
    DataSizing::DOASControl DOASControl = DataSizing::DOASControl::NeutralSup;
    DOASLowTemp = 21.1;
    DOASHighTemp = 23.9;
    OutDB = 10.0;
    OutHR = 0.005;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.016, 0.0143, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(21.1, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.005, DOASSupHR);
    OutDB = 35.6;
    OutHR = 0.0185;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.016, 0.0143, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(23.9, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.016, DOASSupHR);
    OutDB = 22.3;
    OutHR = 0.0085;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.016, 0.0143, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(22.3, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.0085, DOASSupHR);
    // neutral dehumidified supply air
    DOASControl = DataSizing::DOASControl::NeutralDehumSup;
    DOASLowTemp = 14.4;
    DOASHighTemp = 22.2;
    OutDB = 11;
    OutHR = 0.004;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.0153, 0.0092, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(22.2, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.004, DOASSupHR);
    OutDB = 35.6;
    OutHR = 0.0185;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.0153, 0.0092, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(22.2, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.0092, DOASSupHR);
    // cold supply air
    DOASControl = DataSizing::DOASControl::CoolSup;
    DOASLowTemp = 12.2;
    DOASHighTemp = 14.4;
    OutDB = 11;
    OutHR = 0.005;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.0092, 0.008, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(14.4, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.005, DOASSupHR);
    OutDB = 35.6;
    OutHR = 0.0185;
    CalcDOASSupCondsForSizing(*state, OutDB, OutHR, DOASControl, DOASLowTemp, DOASHighTemp, 0.0092, 0.008, DOASSupTemp, DOASSupHR);
    EXPECT_DOUBLE_EQ(12.2, DOASSupTemp);
    EXPECT_DOUBLE_EQ(0.008, DOASSupHR);
}

TEST_F(EnergyPlusFixture, DOASEffectOnZoneSizing_SizeZoneEquipment)
{
    state->init_state(*state);
    state->dataLoopNodes->Node.allocate(10);
    state->dataSize->ZoneEqSizing.allocate(2);
    state->dataHeatBal->Zone.allocate(2);
    state->dataSize->CalcZoneSizing.allocate(1, 2);
    state->dataSize->CalcFinalZoneSizing.allocate(2);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig.allocate(2);
    state->dataHeatBalFanSys->TempControlType.allocate(2);
    state->dataHeatBalFanSys->zoneTstatSetpts.allocate(2);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(2);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(2);
    state->dataZoneEnergyDemand->DeadBandOrSetback.allocate(2);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).returnFlowFracSched = Sched::GetScheduleAlwaysOff(*state);
    state->dataZoneEquip->ZoneEquipConfig(2).returnFlowFracSched = Sched::GetScheduleAlwaysOff(*state);
    state->dataHeatBalFanSys->ZoneMassBalanceFlag.allocate(2);
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->MassConservation.allocate(state->dataGlobal->NumOfZones);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    state->afn->AirflowNetworkNumOfExhFan = 0;
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataHeatBalFanSys->TempControlType(2) = HVAC::SetptType::DualHeatCool;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setpt = 0.0;
    state->dataHeatBalFanSys->zoneTstatSetpts(2).setpt = 0.0;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptLo = 22.;
    state->dataHeatBalFanSys->zoneTstatSetpts(2).setptLo = 22.;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 24.;
    state->dataHeatBalFanSys->zoneTstatSetpts(2).setptHi = 24.;
    state->dataSize->CurOverallSimDay = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(2).IsControlled = true;
    state->dataSize->CalcZoneSizing(1, 1).ZoneNum = 1;
    state->dataSize->CalcZoneSizing(1, 2).ZoneNum = 2;
    state->dataSize->CalcZoneSizing(1, 1).AccountForDOAS = true;
    state->dataSize->CalcZoneSizing(1, 2).AccountForDOAS = true;
    state->dataSize->CurOverallSimDay = 1;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).TotalOutputRequired = -2600;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).OutputRequiredToHeatingSP = -21100;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).OutputRequiredToCoolingSP = -2600;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).TotalOutputRequired = 3600;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToHeatingSP = 3600;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToCoolingSP = 22000.;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).TotalOutputRequired = 0.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).OutputRequiredToHumidifyingSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).OutputRequiredToDehumidifyingSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(2).TotalOutputRequired = 0.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(2).OutputRequiredToHumidifyingSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(2).OutputRequiredToDehumidifyingSP = 0.0;
    state->dataZoneEnergyDemand->DeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->DeadBandOrSetback(2) = false;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(2) = false;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 4;
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneNode = 9;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 4;
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 9;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(1) = 6;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(2) = 7;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(2).ExhaustNode(1) = 8;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 0;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).DOASHighSetpoint = 14.4;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).DOASLowSetpoint = 12.2;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).DOASHighSetpoint = 14.4;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).DOASLowSetpoint = 12.2;
    state->dataEnvrn->StdBaroPress = 101325.;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataSize->CalcFinalZoneSizing(1).MinOA = 0.1;
    state->dataSize->CalcFinalZoneSizing(2).MinOA = 0.11;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).DOASControlStrategy = DataSizing::DOASControl::CoolSup;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).DOASControlStrategy = DataSizing::DOASControl::CoolSup;
    state->dataEnvrn->OutDryBulbTemp = 28.;
    state->dataEnvrn->OutHumRat = 0.017;
    state->dataLoopNodes->Node(4).Temp = 22;
    state->dataLoopNodes->Node(4).HumRat = 0.008;
    state->dataLoopNodes->Node(9).Temp = 22.5;
    state->dataLoopNodes->Node(9).HumRat = 0.0085;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).ZnCoolDgnSAMethod = 1;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).ZnCoolDgnSAMethod = 2;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).ZnHeatDgnSAMethod = 1;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).ZnHeatDgnSAMethod = 2;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).CoolDesTemp = 12.5;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).CoolDesTemp = 12.5;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).CoolDesTempDiff = 11.11;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).CoolDesTempDiff = 11.11;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).CoolDesHumRat = 0.008;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).CoolDesHumRat = 0.008;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).HeatDesHumRat = 0.008;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).HeatDesHumRat = 0.008;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).HeatDesTemp = 50.0;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).HeatDesTemp = 50.0;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).HeatDesTempDiff = 30.0;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).HeatDesTempDiff = 30.0;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 1).SupplyAirAdjustFactor = 1.0;
    state->dataSize->CalcZoneSizing(state->dataSize->CurOverallSimDay, 2).SupplyAirAdjustFactor = 1.0;
    state->dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = false;
    state->dataHeatBalFanSys->ZoneMassBalanceFlag(1) = false;
    state->dataHeatBalFanSys->ZoneMassBalanceFlag(2) = false;
    state->dataLoopNodes->Node(1).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(2).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(2).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = 0.0;
    state->dataLoopNodes->Node(2).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRateMaxAvail = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(6).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(6).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(6).MassFlowRateMaxAvail = 0.0;
    state->dataLoopNodes->Node(6).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(7).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(7).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(7).MassFlowRateMaxAvail = 0.0;
    state->dataLoopNodes->Node(7).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(8).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(8).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(8).MassFlowRateMaxAvail = 0.0;
    state->dataLoopNodes->Node(8).MassFlowRateMax = 0.0;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExh = 0.0;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExhBalanced = 0.0;
    state->dataZoneEquip->ZoneEquipConfig(1).PlenumMassFlow = 0.0;
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneExh = 0.0;
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneExhBalanced = 0.0;
    state->dataZoneEquip->ZoneEquipConfig(2).PlenumMassFlow = 0.0;
    state->dataHeatBal->MassConservation(1).MixingMassFlowRate = 0.0;
    state->dataHeatBal->MassConservation(2).MixingMassFlowRate = 0.0;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(2).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1;
    state->dataHeatBal->Zone(2).ListMultiplier = 1;

    state->dataZoneEquipmentManager->SizeZoneEquipmentOneTimeFlag = false;
    SizeZoneEquipment(*state);

    EXPECT_DOUBLE_EQ(12.2, state->dataSize->CalcZoneSizing(1, 1).DOASSupTemp);
    EXPECT_NEAR(.00795195, state->dataSize->CalcZoneSizing(1, 1).DOASSupHumRat, .00000001);
    EXPECT_DOUBLE_EQ(0.1, state->dataSize->CalcZoneSizing(1, 1).DOASSupMassFlow);
    EXPECT_NEAR(-999.229, state->dataSize->CalcZoneSizing(1, 1).DOASHeatAdd, .001);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CalcZoneSizing(1, 1).DOASHeatLoad);
    EXPECT_NEAR(-999.229, state->dataSize->CalcZoneSizing(1, 1).DOASCoolLoad, .001);
    EXPECT_NEAR(-1011.442, state->dataSize->CalcZoneSizing(1, 1).DOASTotCoolLoad, .001);
    EXPECT_NEAR(4599.229, state->dataSize->CalcZoneSizing(1, 1).HeatLoad, .001);
    EXPECT_NEAR(.161083, state->dataSize->CalcZoneSizing(1, 1).HeatMassFlow, .00001);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CalcZoneSizing(1, 1).CoolLoad);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CalcZoneSizing(1, 1).CoolMassFlow);

    EXPECT_DOUBLE_EQ(12.2, state->dataSize->CalcZoneSizing(1, 2).DOASSupTemp);
    EXPECT_NEAR(.00795195, state->dataSize->CalcZoneSizing(1, 2).DOASSupHumRat, .00000001);
    EXPECT_DOUBLE_EQ(0.11, state->dataSize->CalcZoneSizing(1, 2).DOASSupMassFlow);
    EXPECT_NEAR(-1155.232, state->dataSize->CalcZoneSizing(1, 2).DOASHeatAdd, .001);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CalcZoneSizing(1, 2).DOASHeatLoad);
    EXPECT_NEAR(-1155.232, state->dataSize->CalcZoneSizing(1, 2).DOASCoolLoad, .001);
    EXPECT_NEAR(-1308.522, state->dataSize->CalcZoneSizing(1, 2).DOASTotCoolLoad, .001);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CalcZoneSizing(1, 2).HeatLoad);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CalcZoneSizing(1, 2).HeatMassFlow);
    EXPECT_NEAR(1444.767, state->dataSize->CalcZoneSizing(1, 2).CoolLoad, .001);
    EXPECT_NEAR(.127528, state->dataSize->CalcZoneSizing(1, 2).CoolMassFlow, .000001);
}

TEST_F(EnergyPlusFixture, TestAutoCalcDOASControlStrategy)
{
    state->init_state(*state);
    state->dataSize->NumZoneSizingInput = 2;
    state->dataSize->ZoneSizingInput.allocate(state->dataSize->NumZoneSizingInput);
    state->dataSize->ZoneSizingInput(1).AccountForDOAS = false;
    state->dataSize->ZoneSizingInput(2).AccountForDOAS = true;

    state->dataSize->ZoneSizingInput(2).DOASControlStrategy = DOASControl::NeutralSup;
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = AutoSize;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = AutoSize;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(21.1, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    EXPECT_DOUBLE_EQ(23.9, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = AutoSize;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = 23.7;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_NEAR(20.9, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint, .000001);
    EXPECT_DOUBLE_EQ(23.7, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = 21.2;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = AutoSize;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_NEAR(24.0, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint, .000001);
    EXPECT_DOUBLE_EQ(21.2, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = 21.5;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = 22.6;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(22.6, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    EXPECT_DOUBLE_EQ(21.5, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);

    state->dataSize->ZoneSizingInput(2).DOASControlStrategy = DataSizing::DOASControl::NeutralDehumSup;
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = AutoSize;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = AutoSize;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(14.4, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    EXPECT_DOUBLE_EQ(22.2, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = AutoSize;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = 22.4;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(14.4, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    EXPECT_DOUBLE_EQ(22.4, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = 13.8;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = AutoSize;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(22.2, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    EXPECT_DOUBLE_EQ(13.8, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = 13.9;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = 22.6;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(22.6, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    EXPECT_DOUBLE_EQ(13.9, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);

    state->dataSize->ZoneSizingInput(2).DOASControlStrategy = DataSizing::DOASControl::CoolSup;
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = AutoSize;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = AutoSize;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(12.2, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    EXPECT_DOUBLE_EQ(14.4, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = AutoSize;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = 14.6;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_NEAR(12.4, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint, .000001);
    EXPECT_DOUBLE_EQ(14.6, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = 12.3;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = AutoSize;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_NEAR(14.5, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint, .000001);
    EXPECT_DOUBLE_EQ(12.3, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);
    state->dataSize->ZoneSizingInput(2).DOASLowSetpoint = 12.6;
    state->dataSize->ZoneSizingInput(2).DOASHighSetpoint = 13.8;
    AutoCalcDOASControlStrategy(*state);
    EXPECT_DOUBLE_EQ(13.8, state->dataSize->ZoneSizingInput(2).DOASHighSetpoint);
    EXPECT_DOUBLE_EQ(12.6, state->dataSize->ZoneSizingInput(2).DOASLowSetpoint);

    state->dataSize->ZoneSizingInput.deallocate();
}
