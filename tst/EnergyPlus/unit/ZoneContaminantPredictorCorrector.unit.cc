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

// EnergyPlus::ZoneTempPredictorCorrector Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneContaminantPredictorCorrector.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataHeatBalance;
using namespace DataStringGlobals;
using namespace EnergyPlus::DataZoneControls;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::HybridModel;
using namespace EnergyPlus::ZonePlenum;
using namespace EnergyPlus::ZoneTempPredictorCorrector;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::Psychrometrics;
using namespace SimulationManager;
using namespace EnergyPlus::ZoneContaminantPredictorCorrector;

TEST_F(EnergyPlusFixture, ZoneContaminantPredictorCorrector_AddMDotOATest)
{
    state->init_state(*state);

    state->dataHVACGlobal->ShortenTimeStepSys = false;
    state->dataHVACGlobal->UseZoneTimeStepHistory = false;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataGlobal->NumOfZones = 1;
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataContaminantBalance->Contaminant.GenericContamSimulation = true;

    state->dataContaminantBalance->AZ.allocate(1);
    state->dataContaminantBalance->BZ.allocate(1);
    state->dataContaminantBalance->CZ.allocate(1);
    state->dataContaminantBalance->AZGC.allocate(1);
    state->dataContaminantBalance->BZGC.allocate(1);
    state->dataContaminantBalance->CZGC.allocate(1);

    state->dataContaminantBalance->CO2ZoneTimeMinus1Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus2Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus3Temp.allocate(1);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus1.allocate(1);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2.allocate(1);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3.allocate(1);
    state->dataContaminantBalance->GCZoneTimeMinus1Temp.allocate(1);
    state->dataContaminantBalance->GCZoneTimeMinus2Temp.allocate(1);
    state->dataContaminantBalance->GCZoneTimeMinus3Temp.allocate(1);
    state->dataContaminantBalance->DSGCZoneTimeMinus1.allocate(1);
    state->dataContaminantBalance->DSGCZoneTimeMinus2.allocate(1);
    state->dataContaminantBalance->DSGCZoneTimeMinus3.allocate(1);

    state->dataContaminantBalance->MixingMassFlowCO2.allocate(1);
    state->dataContaminantBalance->MixingMassFlowGC.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2Temp.allocate(1);
    state->dataContaminantBalance->ZoneCO21.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2.allocate(1);
    state->dataContaminantBalance->ZoneAirGCTemp.allocate(1);
    state->dataContaminantBalance->ZoneGC1.allocate(1);
    state->dataContaminantBalance->ZoneAirGC.allocate(1);

    state->dataContaminantBalance->ZoneCO2SetPoint.allocate(1);
    state->dataContaminantBalance->CO2PredictedRate.allocate(1);
    state->dataContaminantBalance->GCPredictedRate.allocate(1);
    state->dataContaminantBalance->ContaminantControlledZone.allocate(1);
    state->dataContaminantBalance->ZoneGCSetPoint.allocate(1);

    state->dataContaminantBalance->ZoneAirDensityCO.allocate(1);
    state->dataContaminantBalance->ZoneCO2Gain.allocate(1);
    state->dataContaminantBalance->ZoneCO2GainExceptPeople.allocate(1);
    state->dataContaminantBalance->ZoneGCGain.allocate(1);
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.0001;
    state->dataContaminantBalance->ZoneCO2GainExceptPeople = 0.0001;
    state->dataContaminantBalance->ZoneGCGain(1) = 0.0000001;
    state->dataContaminantBalance->MixingMassFlowCO2(1) = 0.0;
    state->dataContaminantBalance->MixingMassFlowGC(1) = 0.0;

    state->dataContaminantBalance->DSCO2ZoneTimeMinus1(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3(1) = 200.0;
    state->dataContaminantBalance->OutdoorCO2 = 400.0;
    state->dataContaminantBalance->OutdoorGC = 0.001;
    state->dataContaminantBalance->ZoneCO21(1) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneGC1(1) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->ZoneCO2SetPoint(1) = 450.0;
    state->dataContaminantBalance->ZoneAirCO2(1) = state->dataContaminantBalance->ZoneCO21(1);
    state->dataContaminantBalance->ZoneAirGC(1) = state->dataContaminantBalance->ZoneGC1(1);

    Real64 PriorTimeStep;

    state->dataHVACGlobal->TimeStepSys = 15.0 / 60.0; // System timestep in hours
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    PriorTimeStep = state->dataHVACGlobal->TimeStepSys;

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Zone 1";

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataLoopNodes->Node.allocate(5);

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = state->dataZoneEquip->ZoneEquipConfig(1).ZoneName;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).Volume = 1000.0;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataZonePlenum->NumZoneReturnPlenums = 0;
    state->dataZonePlenum->NumZoneSupplyPlenums = 0;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MDotOA = 0.001;

    state->dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::EulerMethod;

    state->dataLoopNodes->Node(1).MassFlowRate = 0.01; // Zone inlet node 1
    state->dataLoopNodes->Node(1).HumRat = 0.008;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.02; // Zone inlet node 2
    state->dataLoopNodes->Node(2).HumRat = 0.008;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExhBalanced = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.00; // Zone exhaust node 1
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExh = state->dataLoopNodes->Node(3).MassFlowRate;
    state->dataLoopNodes->Node(3).HumRat = 0.008;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.03; // Zone return node
    state->dataLoopNodes->Node(4).HumRat = 0.000;
    state->dataLoopNodes->Node(5).HumRat = 0.000;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MixingMassFlowZone = 0.0;

    state->dataContaminantBalance->CO2PredictedRate.allocate(1);
    state->dataContaminantBalance->ZoneSysContDemand.allocate(1);

    state->dataContaminantBalance->ContaminantControlledZone.allocate(1);

    state->dataContaminantBalance->ContaminantControlledZone(1).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(1).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(1).ActualZoneNum = 1;
    state->dataContaminantBalance->ContaminantControlledZone(1).NumOfZones = 1;
    state->dataContaminantBalance->ZoneGCSetPoint(1) = 0.0025;

    PredictZoneContaminants(*state, state->dataHVACGlobal->ShortenTimeStepSys, state->dataHVACGlobal->UseZoneTimeStepHistory, PriorTimeStep);
    EXPECT_NEAR(1.041692180, state->dataContaminantBalance->CO2PredictedRate(1), 0.00001);
    EXPECT_NEAR(76.89754831, state->dataContaminantBalance->GCPredictedRate(1), 0.00001);

    CorrectZoneContaminants(*state, state->dataHVACGlobal->UseZoneTimeStepHistory);
    EXPECT_NEAR(489.931000, state->dataLoopNodes->Node(5).CO2, 0.00001);
    EXPECT_NEAR(0.09093100, state->dataLoopNodes->Node(5).GenContam, 0.00001);
}

TEST_F(EnergyPlusFixture, ZoneContaminantPredictorCorrector_CorrectZoneContaminantsTest)
{
    state->init_state(*state);

    state->dataHVACGlobal->ShortenTimeStepSys = false;
    state->dataHVACGlobal->UseZoneTimeStepHistory = false;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataGlobal->NumOfZones = 1;
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataContaminantBalance->Contaminant.GenericContamSimulation = true;

    state->dataContaminantBalance->AZ.allocate(1);
    state->dataContaminantBalance->BZ.allocate(1);
    state->dataContaminantBalance->CZ.allocate(1);
    state->dataContaminantBalance->AZGC.allocate(1);
    state->dataContaminantBalance->BZGC.allocate(1);
    state->dataContaminantBalance->CZGC.allocate(1);

    state->dataContaminantBalance->CO2ZoneTimeMinus1Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus2Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus3Temp.allocate(1);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus1.allocate(1);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2.allocate(1);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3.allocate(1);
    state->dataContaminantBalance->GCZoneTimeMinus1Temp.allocate(1);
    state->dataContaminantBalance->GCZoneTimeMinus2Temp.allocate(1);
    state->dataContaminantBalance->GCZoneTimeMinus3Temp.allocate(1);
    state->dataContaminantBalance->DSGCZoneTimeMinus1.allocate(1);
    state->dataContaminantBalance->DSGCZoneTimeMinus2.allocate(1);
    state->dataContaminantBalance->DSGCZoneTimeMinus3.allocate(1);

    state->dataContaminantBalance->MixingMassFlowCO2.allocate(1);
    state->dataContaminantBalance->MixingMassFlowGC.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2Temp.allocate(1);
    state->dataContaminantBalance->ZoneCO21.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2.allocate(1);
    state->dataContaminantBalance->ZoneAirGCTemp.allocate(1);
    state->dataContaminantBalance->ZoneGC1.allocate(1);
    state->dataContaminantBalance->ZoneAirGC.allocate(1);

    state->dataContaminantBalance->ZoneAirDensityCO.allocate(1);
    state->dataContaminantBalance->ZoneCO2Gain.allocate(1);
    state->dataContaminantBalance->ZoneCO2GainExceptPeople.allocate(1);
    state->dataContaminantBalance->ZoneGCGain.allocate(1);
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.0001;
    state->dataContaminantBalance->ZoneCO2GainExceptPeople(1) = 0.0001;
    state->dataContaminantBalance->ZoneGCGain(1) = 0.0001;
    state->dataContaminantBalance->MixingMassFlowCO2(1) = 0.0;
    state->dataContaminantBalance->MixingMassFlowGC(1) = 0.0;

    state->dataContaminantBalance->DSCO2ZoneTimeMinus1(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3(1) = 200.0;
    state->dataContaminantBalance->OutdoorCO2 = 400.0;
    state->dataContaminantBalance->OutdoorGC = 0.001;
    state->dataContaminantBalance->ZoneCO21(1) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneGC1(1) = state->dataContaminantBalance->OutdoorGC;

    state->dataHVACGlobal->TimeStepSys = 15.0 / 60.0; // System timestep in hours
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Zone 1";

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataLoopNodes->Node.allocate(5);

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = state->dataZoneEquip->ZoneEquipConfig(1).ZoneName;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).Volume = 1000.0;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataZonePlenum->NumZoneReturnPlenums = 0;
    state->dataZonePlenum->NumZoneSupplyPlenums = 0;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::EulerMethod;

    state->dataLoopNodes->Node(1).MassFlowRate = 0.01; // Zone inlet node 1
    state->dataLoopNodes->Node(1).HumRat = 0.008;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.02; // Zone inlet node 2
    state->dataLoopNodes->Node(2).HumRat = 0.008;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExhBalanced = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.00; // Zone exhaust node 1
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExh = state->dataLoopNodes->Node(3).MassFlowRate;
    state->dataLoopNodes->Node(3).HumRat = 0.008;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.03; // Zone return node
    state->dataLoopNodes->Node(4).HumRat = 0.000;
    state->dataLoopNodes->Node(5).HumRat = 0.000;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MixingMassFlowZone = 0.0;

    CorrectZoneContaminants(*state, state->dataHVACGlobal->UseZoneTimeStepHistory);
    EXPECT_NEAR(490.0, state->dataLoopNodes->Node(5).CO2, 0.00001);
    EXPECT_NEAR(90.000999, state->dataLoopNodes->Node(5).GenContam, 0.00001);
}

TEST_F(EnergyPlusFixture, ZoneContaminantPredictorCorrector_MultiZoneCO2ControlTest)
{
    state->init_state(*state);

    state->dataHVACGlobal->ShortenTimeStepSys = false;
    state->dataHVACGlobal->UseZoneTimeStepHistory = false;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(3);

    state->dataGlobal->NumOfZones = 3;

    state->dataContaminantBalance->Contaminant.CO2Simulation = true;

    state->dataContaminantBalance->AZ.allocate(3);
    state->dataContaminantBalance->BZ.allocate(3);
    state->dataContaminantBalance->CZ.allocate(3);

    state->dataContaminantBalance->CO2ZoneTimeMinus1Temp.allocate(3);
    state->dataContaminantBalance->CO2ZoneTimeMinus2Temp.allocate(3);
    state->dataContaminantBalance->CO2ZoneTimeMinus3Temp.allocate(3);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus1.allocate(3);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2.allocate(3);
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3.allocate(3);

    state->dataContaminantBalance->GCZoneTimeMinus1Temp.allocate(3);
    state->dataContaminantBalance->GCZoneTimeMinus2Temp.allocate(3);
    state->dataContaminantBalance->GCZoneTimeMinus3Temp.allocate(3);
    state->dataContaminantBalance->DSGCZoneTimeMinus1.allocate(3);
    state->dataContaminantBalance->DSGCZoneTimeMinus2.allocate(3);
    state->dataContaminantBalance->DSGCZoneTimeMinus3.allocate(3);

    state->dataContaminantBalance->MixingMassFlowCO2.allocate(3);
    state->dataContaminantBalance->MixingMassFlowGC.allocate(3);
    state->dataContaminantBalance->ZoneAirCO2Temp.allocate(3);
    state->dataContaminantBalance->ZoneCO21.allocate(3);
    state->dataContaminantBalance->ZoneAirCO2.allocate(3);
    state->dataContaminantBalance->ZoneAirGCTemp.allocate(3);

    state->dataContaminantBalance->ZoneCO2SetPoint.allocate(3);
    state->dataContaminantBalance->CO2PredictedRate.allocate(3);

    state->dataContaminantBalance->ZoneAirDensityCO.allocate(3);
    state->dataContaminantBalance->ZoneCO2Gain.allocate(3);
    state->dataContaminantBalance->ZoneGCGain.allocate(3);
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.0001;
    state->dataContaminantBalance->ZoneCO2Gain(2) = 0.0002;
    state->dataContaminantBalance->ZoneCO2Gain(3) = 0.0003;
    state->dataContaminantBalance->MixingMassFlowCO2(1) = 0.0;
    state->dataContaminantBalance->MixingMassFlowCO2(2) = 0.0;
    state->dataContaminantBalance->MixingMassFlowCO2(3) = 0.0;

    state->dataContaminantBalance->DSCO2ZoneTimeMinus1(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3(1) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus1(2) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2(2) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3(2) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus1(3) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus2(3) = 200.0;
    state->dataContaminantBalance->DSCO2ZoneTimeMinus3(3) = 200.0;
    state->dataContaminantBalance->OutdoorCO2 = 400.0;
    state->dataContaminantBalance->ZoneCO21(1) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneCO21(2) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneCO21(3) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneCO2SetPoint(1) = 450.0;
    state->dataContaminantBalance->ZoneCO2SetPoint(2) = 500.0;
    state->dataContaminantBalance->ZoneCO2SetPoint(3) = 550.0;
    state->dataContaminantBalance->ZoneAirCO2(1) = state->dataContaminantBalance->ZoneCO21(1);
    state->dataContaminantBalance->ZoneAirCO2(2) = state->dataContaminantBalance->ZoneCO21(2);
    state->dataContaminantBalance->ZoneAirCO2(3) = state->dataContaminantBalance->ZoneCO21(3);

    Real64 PriorTimeStep;

    state->dataHVACGlobal->TimeStepSys = 15.0 / 60.0; // System timestep in hours
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    PriorTimeStep = state->dataHVACGlobal->TimeStepSys;

    state->dataZoneEquip->ZoneEquipConfig.allocate(3);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Zone 1";

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(2).ZoneName = "Zone 2";

    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(1) = 6;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode(1) = 7;
    state->dataZoneEquip->ZoneEquipConfig(2).FixedReturnFlow.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(3).ZoneName = "Zone 3";

    state->dataZoneEquip->ZoneEquipConfig(3).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(3).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(3).InletNode(1) = 8;
    state->dataZoneEquip->ZoneEquipConfig(3).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(3).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(3).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(3).ReturnNode(1) = 9;
    state->dataZoneEquip->ZoneEquipConfig(3).FixedReturnFlow.allocate(1);

    state->dataLoopNodes->Node.allocate(10);

    state->dataHeatBal->Zone.allocate(3);
    state->dataHeatBal->Zone(1).Name = state->dataZoneEquip->ZoneEquipConfig(1).ZoneName;
    state->dataSize->ZoneEqSizing.allocate(3);
    state->dataSize->CurZoneEqNum = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).Volume = 1000.0;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    state->dataHeatBal->Zone(2).Name = state->dataZoneEquip->ZoneEquipConfig(2).ZoneName;
    state->dataHeatBal->Zone(2).Multiplier = 1.0;
    state->dataHeatBal->Zone(2).Volume = 1000.0;
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(2).ZoneVolCapMultpMoist = 1.0;
    state->dataHeatBal->Zone(3).Name = state->dataZoneEquip->ZoneEquipConfig(3).ZoneName;
    state->dataHeatBal->Zone(3).Multiplier = 1.0;
    state->dataHeatBal->Zone(3).Volume = 1000.0;
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(3).ZoneVolCapMultpMoist = 1.0;

    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataZonePlenum->NumZoneReturnPlenums = 0;
    state->dataZonePlenum->NumZoneSupplyPlenums = 0;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(3);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MDotOA = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MDotOA = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).MDotOA = 0.001;

    state->dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::EulerMethod;

    state->dataLoopNodes->Node(1).MassFlowRate = 0.01; // Zone inlet node 1
    state->dataLoopNodes->Node(1).HumRat = 0.008;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.02; // Zone inlet node 2
    state->dataLoopNodes->Node(2).HumRat = 0.008;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExhBalanced = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.00; // Zone exhaust node 1
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExh = state->dataLoopNodes->Node(3).MassFlowRate;
    state->dataLoopNodes->Node(3).HumRat = 0.008;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.03; // Zone return node
    state->dataLoopNodes->Node(4).HumRat = 0.000;
    state->dataLoopNodes->Node(5).HumRat = 0.000;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).ZT = 23.5;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).ZT = 24.5;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MixingMassFlowZone = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MixingMassFlowZone = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).MixingMassFlowZone = 0.0;

    state->dataLoopNodes->Node(6).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(7).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(8).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(9).MassFlowRate = 0.01;

    state->dataContaminantBalance->CO2PredictedRate.allocate(3);
    state->dataContaminantBalance->ZoneSysContDemand.allocate(3);

    state->dataContaminantBalance->ContaminantControlledZone.allocate(3);

    state->dataContaminantBalance->ContaminantControlledZone(1).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(1).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(1).ActualZoneNum = 1;
    state->dataContaminantBalance->ContaminantControlledZone(1).NumOfZones = 1;
    state->dataContaminantBalance->ContaminantControlledZone(2).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(2).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(2).ActualZoneNum = 2;
    state->dataContaminantBalance->ContaminantControlledZone(2).NumOfZones = 1;
    state->dataContaminantBalance->ContaminantControlledZone(3).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(3).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(3).ActualZoneNum = 3;
    state->dataContaminantBalance->ContaminantControlledZone(3).NumOfZones = 1;

    PredictZoneContaminants(*state, state->dataHVACGlobal->ShortenTimeStepSys, state->dataHVACGlobal->UseZoneTimeStepHistory, PriorTimeStep);
    EXPECT_NEAR(1.0416921806, state->dataContaminantBalance->CO2PredictedRate(1), 0.00001);
    EXPECT_NEAR(1.0434496257, state->dataContaminantBalance->CO2PredictedRate(2), 0.00001);
    EXPECT_NEAR(1.0399406399, state->dataContaminantBalance->CO2PredictedRate(3), 0.00001);
}

TEST_F(EnergyPlusFixture, ZoneContaminantPredictorCorrector_MultiZoneGCControlTest)
{
    state->init_state(*state);

    state->dataHVACGlobal->ShortenTimeStepSys = false;
    state->dataHVACGlobal->UseZoneTimeStepHistory = false;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(3);

    state->dataGlobal->NumOfZones = 3;

    state->dataContaminantBalance->Contaminant.CO2Simulation = false;
    state->dataContaminantBalance->Contaminant.GenericContamSimulation = true;

    state->dataContaminantBalance->AZGC.allocate(3);
    state->dataContaminantBalance->BZGC.allocate(3);
    state->dataContaminantBalance->CZGC.allocate(3);

    state->dataContaminantBalance->GCZoneTimeMinus1Temp.allocate(3);
    state->dataContaminantBalance->GCZoneTimeMinus2Temp.allocate(3);
    state->dataContaminantBalance->GCZoneTimeMinus3Temp.allocate(3);
    state->dataContaminantBalance->DSGCZoneTimeMinus1.allocate(3);
    state->dataContaminantBalance->DSGCZoneTimeMinus2.allocate(3);
    state->dataContaminantBalance->DSGCZoneTimeMinus3.allocate(3);

    state->dataContaminantBalance->MixingMassFlowGC.allocate(3);
    state->dataContaminantBalance->ZoneAirGCTemp.allocate(3);
    state->dataContaminantBalance->ZoneGC1.allocate(3);
    state->dataContaminantBalance->ZoneAirGC.allocate(3);

    state->dataContaminantBalance->ZoneGCSetPoint.allocate(3);
    state->dataContaminantBalance->GCPredictedRate.allocate(3);

    state->dataContaminantBalance->ZoneGCGain.allocate(3);
    state->dataContaminantBalance->ZoneGCGain(1) = 0.0001;
    state->dataContaminantBalance->ZoneGCGain(2) = 0.0002;
    state->dataContaminantBalance->ZoneGCGain(3) = 0.0003;
    state->dataContaminantBalance->MixingMassFlowGC(1) = 0.0;
    state->dataContaminantBalance->MixingMassFlowGC(2) = 0.0;
    state->dataContaminantBalance->MixingMassFlowGC(3) = 0.0;

    state->dataContaminantBalance->OutdoorGC = 10.0;
    state->dataContaminantBalance->DSGCZoneTimeMinus1(1) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus2(1) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus3(1) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus1(2) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus2(2) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus3(2) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus1(3) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus2(3) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->DSGCZoneTimeMinus3(3) = state->dataContaminantBalance->OutdoorGC;
    state->dataContaminantBalance->ZoneGC1(1) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneGC1(2) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneGC1(3) = state->dataContaminantBalance->OutdoorCO2;
    state->dataContaminantBalance->ZoneGCSetPoint(1) = 15.0;
    state->dataContaminantBalance->ZoneGCSetPoint(2) = 20.0;
    state->dataContaminantBalance->ZoneGCSetPoint(3) = 25.0;
    state->dataContaminantBalance->ZoneAirGC(1) = state->dataContaminantBalance->ZoneGC1(1);
    state->dataContaminantBalance->ZoneAirGC(2) = state->dataContaminantBalance->ZoneGC1(2);
    state->dataContaminantBalance->ZoneAirGC(3) = state->dataContaminantBalance->ZoneGC1(3);

    Real64 PriorTimeStep;

    state->dataHVACGlobal->TimeStepSys = 15.0 / 60.0; // System timestep in hours
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    PriorTimeStep = state->dataHVACGlobal->TimeStepSys;

    state->dataZoneEquip->ZoneEquipConfig.allocate(3);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Zone 1";

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(2).ZoneName = "Zone 2";

    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(1) = 6;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode(1) = 7;
    state->dataZoneEquip->ZoneEquipConfig(2).FixedReturnFlow.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(3).ZoneName = "Zone 3";

    state->dataZoneEquip->ZoneEquipConfig(3).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(3).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(3).InletNode(1) = 8;
    state->dataZoneEquip->ZoneEquipConfig(3).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(3).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(3).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(3).ReturnNode(1) = 9;
    state->dataZoneEquip->ZoneEquipConfig(3).FixedReturnFlow.allocate(1);

    state->dataLoopNodes->Node.allocate(10);

    state->dataHeatBal->Zone.allocate(3);
    state->dataHeatBal->Zone(1).Name = state->dataZoneEquip->ZoneEquipConfig(1).ZoneName;
    state->dataSize->ZoneEqSizing.allocate(3);
    state->dataSize->CurZoneEqNum = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).Volume = 1000.0;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    state->dataHeatBal->Zone(2).Name = state->dataZoneEquip->ZoneEquipConfig(2).ZoneName;
    state->dataHeatBal->Zone(2).Multiplier = 1.0;
    state->dataHeatBal->Zone(2).Volume = 1000.0;
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(2).ZoneVolCapMultpMoist = 1.0;
    state->dataHeatBal->Zone(3).Name = state->dataZoneEquip->ZoneEquipConfig(3).ZoneName;
    state->dataHeatBal->Zone(3).Multiplier = 1.0;
    state->dataHeatBal->Zone(3).Volume = 1000.0;
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(3).ZoneVolCapMultpMoist = 1.0;

    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataZonePlenum->NumZoneReturnPlenums = 0;
    state->dataZonePlenum->NumZoneSupplyPlenums = 0;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(3);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MDotOA = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MDotOA = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).MDotOA = 0.001;

    state->dataHeatBal->ZoneAirSolutionAlgo = DataHeatBalance::SolutionAlgo::EulerMethod;

    state->dataLoopNodes->Node(1).MassFlowRate = 0.01; // Zone inlet node 1
    state->dataLoopNodes->Node(1).HumRat = 0.008;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.02; // Zone inlet node 2
    state->dataLoopNodes->Node(2).HumRat = 0.008;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExhBalanced = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.00; // Zone exhaust node 1
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExh = state->dataLoopNodes->Node(3).MassFlowRate;
    state->dataLoopNodes->Node(3).HumRat = 0.008;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.03; // Zone return node
    state->dataLoopNodes->Node(4).HumRat = 0.000;
    state->dataLoopNodes->Node(5).HumRat = 0.000;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).ZT = 23.5;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).airHumRat = 0.008;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).ZT = 24.5;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MixingMassFlowZone = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MixingMassFlowZone = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).MixingMassFlowZone = 0.0;

    state->dataLoopNodes->Node(6).MassFlowRate = 0.01;

    state->dataLoopNodes->Node(7).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(8).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(9).MassFlowRate = 0.01;

    state->dataContaminantBalance->GCPredictedRate.allocate(3);

    state->dataContaminantBalance->ZoneSysContDemand.allocate(3);

    state->dataContaminantBalance->ContaminantControlledZone.allocate(3);

    state->dataContaminantBalance->ContaminantControlledZone(1).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(1).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(1).ActualZoneNum = 1;
    state->dataContaminantBalance->ContaminantControlledZone(1).NumOfZones = 1;
    state->dataContaminantBalance->ContaminantControlledZone(2).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(2).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(2).ActualZoneNum = 2;
    state->dataContaminantBalance->ContaminantControlledZone(2).NumOfZones = 1;
    state->dataContaminantBalance->ContaminantControlledZone(3).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(3).genericContamAvailSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataContaminantBalance->ContaminantControlledZone(3).ActualZoneNum = 3;
    state->dataContaminantBalance->ContaminantControlledZone(3).NumOfZones = 1;

    PredictZoneContaminants(*state, state->dataHVACGlobal->ShortenTimeStepSys, state->dataHVACGlobal->UseZoneTimeStepHistory, PriorTimeStep);

    EXPECT_NEAR(19.549478386, state->dataContaminantBalance->GCPredictedRate(1), 0.00001);
    EXPECT_NEAR(20.887992514, state->dataContaminantBalance->GCPredictedRate(2), 0.00001);
    EXPECT_NEAR(21.251538064, state->dataContaminantBalance->GCPredictedRate(3), 0.00001);
}

TEST_F(EnergyPlusFixture, ZoneContaminantPredictorCorrector_MissingGenericContaminantSchedules)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Zone1,                                  !- Name",
        "  0,                                      !- Direction of Relative North {deg}",
        "  0,                                      !- X Origin {m}",
        "  0,                                      !- Y Origin {m}",
        "  0,                                      !- Z Origin {m}",
        "  ,                                       !- Type",
        "  1,                                      !- Multiplier",
        "  ,                                       !- Ceiling Height {m}",
        "  ,                                       !- Volume {m3}",
        "  ,                                       !- Floor Area {m2}",
        "  ,                                       !- Zone Inside Convection Algorithm",
        "  ,                                       !- Zone Outside Convection Algorithm",
        "  Yes;                                    !- Part of Total Floor Area",

        "ZoneAirContaminantBalance,",
        "  Yes,                                    !- Carbon Dioxide Concentration",
        "  Always 400ppm,                          !- Outdoor Carbon Dioxide Schedule Name",
        "  Yes,                                    !- Generic Contaminant Concentration",
        "  Always Generic Outdoor;                 !- Outdoor Generic Contaminant Schedule Name",

        "ZoneControl:ContaminantController,",
        "  Zone1 Contaminant Controller,           !- Name",
        "  Zone1,                                  !- Zone Name",
        "  Always On,                              !- Carbon Dioxide Control Availability Schedule Name",
        "  Always 1000ppm;                         !- Carbon Dioxide Setpoint Schedule Name",

        "ScheduleTypeLimits,",
        "  Fraction,                               !- Name",
        "  0,                                      !- Lower Limit Value",
        "  1,                                      !- Upper Limit Value",
        "  Continuous;                             !- Numeric Type",

        "ScheduleTypeLimits,",
        "  Any Number;                             !- Name",

        "Schedule:Constant,",
        "  Always On,                              !- Name",
        "  Fraction,                               !- Schedule Type Limits Name",
        "  1.0;                                    !- Hourly Value",

        "Schedule:Constant,",
        "  Always 1000ppm,                         !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  1000.0;                                 !- Hourly Value",

        "Schedule:Constant,",
        "  Always 400ppm,                          !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  400.0;                                  !- Hourly Value",

        "Schedule:Constant,",
        "  Always Generic Outdoor,                 !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  1.0;                                    !- Hourly Value",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    bool ErrorsFound(false);
    GetProjectControlData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    EXPECT_NO_THROW(GetZoneContaminantSetPoints(*state));
    AllocateHeatBalArrays(*state);
    EXPECT_THROW(InitZoneContSetPoints(*state), EnergyPlus::FatalError);
    EXPECT_TRUE(compare_err_stream_substring("ZoneControl:ContaminantController", false));
    EXPECT_TRUE(compare_err_stream_substring("Generic Contaminant Setpoint Schedule", false));
    EXPECT_TRUE(compare_err_stream_substring("Program terminates for preceding reason(s).", true));
}

TEST_F(EnergyPlusFixture, ZoneContaminantPredictorCorrector_EMSActuatesContaminantSetpointSchedules)
{
    // CO2 and generic contaminant setpoint schedules can be actuated through EMS Schedule Value actuators, so EMSOverrideCO2 and EMSOverrideGC are
    // unnecessary.
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Zone1;                                  !- Name",

        "ZoneControl:ContaminantController,",
        "  Zone1 Contaminant Controller,           !- Name",
        "  Zone1,                                  !- Zone Name",
        "  Always On,                              !- Carbon Dioxide Control Availability Schedule Name",
        "  CO2 Setpoint Schedule,                  !- Carbon Dioxide Setpoint Schedule Name",
        "  ,                                       !- Carbon Dioxide Minimum Setpoint Schedule Name",
        "  ,                                       !- Carbon Dioxide Maximum Setpoint Schedule Name",
        "  Always On,                              !- Generic Contaminant Control Availability Schedule Name",
        "  Generic Contaminant Setpoint Schedule;  !- Generic Contaminant Setpoint Schedule Name",

        "Schedule:Constant,",
        "  Always On,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  CO2 Setpoint Schedule,                  !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1000;                                   !- Hourly Value",

        "Schedule:Constant,",
        "  Generic Contaminant Setpoint Schedule,  !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  10;                                     !- Hourly Value",

        "EnergyManagementSystem:Actuator,",
        "  CO2SetpointScheduleActuator,            !- Name",
        "  CO2 Setpoint Schedule,                  !- Actuated Component Unique Name",
        "  Schedule:Constant,                      !- Actuated Component Type",
        "  Schedule Value;                         !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "  GenericContaminantSetpointScheduleActuator,  !- Name",
        "  Generic Contaminant Setpoint Schedule,  !- Actuated Component Unique Name",
        "  Schedule:Constant,                      !- Actuated Component Type",
        "  Schedule Value;                         !- Actuated Component Control Type",

        "EnergyManagementSystem:Program,",
        "  SetContaminantSetpointSchedules,        !- Name",
        "  SET CO2SetpointScheduleActuator = 900,  !- Program Line 1",
        "  SET GenericContaminantSetpointScheduleActuator = 20;  !- Program Line 2",

        "EnergyManagementSystem:ProgramCallingManager,",
        "  SetContaminantSetpointSchedulesManager, !- Name",
        "  BeginTimestepBeforePredictor,           !- EnergyPlus Model Calling Point",
        "  SetContaminantSetpointSchedules;        !- Program Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EMSManager::CheckIfAnyEMS(*state);
    EXPECT_TRUE(state->dataGlobal->AnyEnergyManagementSystemInModel);

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->MinutesInTimeStep = 15;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;
    state->init_state(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;

    auto *co2SetpointSchedule = Sched::GetSchedule(*state, "CO2 SETPOINT SCHEDULE");
    auto *genericSetpointSchedule = Sched::GetSchedule(*state, "GENERIC CONTAMINANT SETPOINT SCHEDULE");
    ASSERT_NE(co2SetpointSchedule, nullptr);
    ASSERT_NE(genericSetpointSchedule, nullptr);

    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataEnvrn->DayOfYear_Schedule = 365;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataGlobal->HourOfDay = 24;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->CurrentTime = 24.0;
    Sched::UpdateScheduleVals(*state);

    bool anyEMSRan = false;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyEMSRan, ObjexxFCL::Optional_int_const());
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyEMSRan);
    Sched::UpdateScheduleVals(*state);

    EXPECT_TRUE(co2SetpointSchedule->EMSActuatedOn);
    EXPECT_EQ(900.0, co2SetpointSchedule->EMSVal);
    EXPECT_EQ(900.0, co2SetpointSchedule->getCurrentVal());
    EXPECT_TRUE(genericSetpointSchedule->EMSActuatedOn);
    EXPECT_EQ(20.0, genericSetpointSchedule->EMSVal);
    EXPECT_EQ(20.0, genericSetpointSchedule->getCurrentVal());
}
