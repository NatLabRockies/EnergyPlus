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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include "Fixtures/SQLiteFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataEnvironment.hh>
// #include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, ReportCoilSelection_ChWCoil)
{
    std::string coil1Name("Coil 1");             // user-defined name of the coil
    std::string coil1Type("Coil:Cooling:Water"); // idf input object class name of coil
    int chWInletNodeNum = 9;
    int chWOutletNodeNum = 15;

    state->init_state(*state);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "Chilled Water Loop";
    state->dataPlnt->PlantLoop(1).FluidName = "Water";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 0.1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = chWInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = chWOutletNodeNum;

    Real64 airVdot(0.052);   // air flow rate in m3/s
    bool isAutoSized(false); // true if autosized
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilAirFlow(*state, coil1Name, coil1Type, airVdot, isAutoSized);
    auto &c1(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0]);
    EXPECT_EQ(coil1Name, c1->coilName_);
    EXPECT_EQ(coil1Type, c1->coilObjName);
    EXPECT_EQ(airVdot, c1->coilDesVolFlow);
    EXPECT_EQ(isAutoSized, c1->volFlowIsAutosized);

    int loopNum = 1;
    Real64 waterVdot = 0.05;
    // First with no plant sizing objects defined
    isAutoSized = false; // true if autosized
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowNodeNums(
        *state, coil1Name, coil1Type, waterVdot, isAutoSized, chWInletNodeNum, chWOutletNodeNum, loopNum);
    EXPECT_EQ(-999, c1->pltSizNum);
    EXPECT_EQ(loopNum, c1->waterLoopNum);
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, c1->plantLoopName);
    EXPECT_EQ(-999, c1->rhoFluid);
    EXPECT_EQ(-999, c1->cpFluid);
    EXPECT_EQ(-999, c1->coilDesWaterMassFlow);
    EXPECT_EQ("No", c1->coilWaterFlowAutoMsg);

    // Exercise report writing with mostly defaults
    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);

    // Use the other form for coil 2
    std::string coil2Name("Coil 2");             // user-defined name of the coil
    std::string coil2Type("Coil:Cooling:Water"); // idf input object class name of coil
    int pltSizNum = -999;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowPltSizNum(
        *state, coil2Name, coil2Type, waterVdot, isAutoSized, pltSizNum, loopNum);
    auto &c2(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[1]);
    EXPECT_EQ(-999, c2->pltSizNum);
    EXPECT_EQ(loopNum, c2->waterLoopNum);
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, c2->plantLoopName);
    EXPECT_EQ(-999, c2->rhoFluid);
    EXPECT_EQ(-999, c2->cpFluid);
    EXPECT_EQ(-999, c2->coilDesWaterMassFlow);
    EXPECT_EQ("No", c2->coilWaterFlowAutoMsg);

    // Now add a plant sizing object
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).PlantLoopName = "Chilled Water Loop";
    isAutoSized = true; // true if autosized
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowNodeNums(
        *state, coil1Name, coil1Type, waterVdot, isAutoSized, chWInletNodeNum, chWOutletNodeNum, loopNum);
    auto &c1b(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0]);
    EXPECT_EQ(1, c1b->pltSizNum);
    EXPECT_EQ(loopNum, c1b->waterLoopNum);
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, c1b->plantLoopName);
    EXPECT_NEAR(999.9, c1b->rhoFluid, 0.1);
    EXPECT_NEAR(4197.9, c1b->cpFluid, 0.1);
    Real64 expFlow = waterVdot * c1b->rhoFluid;
    EXPECT_NEAR(expFlow, c1b->coilDesWaterMassFlow, 0.01);
    EXPECT_EQ("Yes", c1b->coilWaterFlowAutoMsg);

    Real64 uA = 1000.00;
    Real64 sizingCap = 500.0;
    int curSysNum = 1;
    int curZoneEqNum = 0;
    isAutoSized = true; // true if autosized
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).NumZonesHeated = 2;
    state->dataAirLoop->AirToZoneNodeInfo(1).HeatCtrlZoneNums.allocate(state->dataAirLoop->AirToZoneNodeInfo(1).NumZonesHeated);
    state->dataAirLoop->AirToZoneNodeInfo(1).HeatCtrlZoneNums(1) = 2;
    state->dataAirLoop->AirToZoneNodeInfo(1).HeatCtrlZoneNums(2) = 3;
    state->dataGlobal->NumOfZones = 3;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "Zone 1";
    state->dataHeatBal->Zone(2).Name = "Zone 2";
    state->dataHeatBal->Zone(3).Name = "Zone 3";

    // This triggers doAirLoopSetUp
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilUA(*state, coil2Name, coil2Type, uA, sizingCap, isAutoSized, curSysNum, curZoneEqNum);
    EXPECT_EQ(uA, c2->coilUA);
    EXPECT_EQ(sizingCap, c2->coilTotCapAtPeak);
    EXPECT_EQ(curSysNum, c2->airloopNum);
    EXPECT_EQ(curZoneEqNum, c2->zoneEqNum);

    // This coil serves zones 2 and 3 - rmLatentAtPeak is summed for all applicable zones
    Real64 zoneCoolingLatentLoad = 1000.0;
    int zoneNum = 1;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadCoolingIdealPeak(zoneNum, zoneCoolingLatentLoad);
    EXPECT_EQ(0.0, c2->rmLatentAtPeak);
    zoneNum = 2;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadCoolingIdealPeak(zoneNum, zoneCoolingLatentLoad);
    EXPECT_EQ(1000.0, c2->rmLatentAtPeak);
    zoneNum = 3;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadCoolingIdealPeak(zoneNum, zoneCoolingLatentLoad);
    EXPECT_EQ(2000.0, c2->rmLatentAtPeak);

    // Add a heating coil
    std::string coil3Name("Coil 3");                // user-defined name of the coil
    std::string coil3Type("Coil:Heating:Electric"); // idf input object class name of coil
    uA = -999.0;
    sizingCap = 500.0;
    curSysNum = 1;
    curZoneEqNum = 0;
    isAutoSized = false; // true if autosized
    // This triggers doAirLoopSetUp
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilUA(*state, coil3Name, coil3Type, uA, sizingCap, isAutoSized, curSysNum, curZoneEqNum);
    auto &c3(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[2]);
    EXPECT_EQ(uA, c3->coilUA);
    EXPECT_EQ(sizingCap, c3->coilTotCapAtPeak);
    EXPECT_EQ(curSysNum, c3->airloopNum);
    EXPECT_EQ(curZoneEqNum, c3->zoneEqNum);

    // This coil serves zones 2 and 3 - rmLatentAtPeak is summed for all applicable zones
    Real64 zoneHeatingLatentLoad = 100.0;
    zoneNum = 1;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadHeatingIdealPeak(zoneNum, zoneHeatingLatentLoad);
    EXPECT_EQ(0.0, c3->rmLatentAtPeak);
    zoneNum = 2;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadHeatingIdealPeak(zoneNum, zoneHeatingLatentLoad);
    EXPECT_EQ(100.0, c3->rmLatentAtPeak);
    zoneNum = 3;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadHeatingIdealPeak(zoneNum, zoneHeatingLatentLoad);
    EXPECT_EQ(200.0, c3->rmLatentAtPeak);

    // Exercise report writing again
    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);
}

TEST_F(EnergyPlusFixture, ReportCoilSelection_SteamCoil)
{
    std::string coil1Name("Coil 1");             // user-defined name of the coil
    std::string coil1Type("Coil:Heating:Steam"); // idf input object class name of coil
    int wInletNodeNum = 9;
    int wOutletNodeNum = 15;

    state->init_state(*state);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "Steam Loop";
    state->dataPlnt->PlantLoop(1).FluidName = "Steam";
    state->dataPlnt->PlantLoop(1).steam = Fluid::GetSteam(*state);
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 0.1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = wInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = wOutletNodeNum;

    Real64 airVdot(0.052);   // air flow rate in m3/s
    bool isAutoSized(false); // true if autosized
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilAirFlow(*state, coil1Name, coil1Type, airVdot, isAutoSized);
    auto &c1(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0]);
    EXPECT_EQ(coil1Name, c1->coilName_);
    EXPECT_EQ(coil1Type, c1->coilObjName);
    EXPECT_EQ(airVdot, c1->coilDesVolFlow);
    EXPECT_EQ(isAutoSized, c1->volFlowIsAutosized);

    int loopNum = 1;
    Real64 waterVdot = 0.05;
    // First with no plant sizing objects defined
    isAutoSized = false; // true if autosized
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowNodeNums(
        *state, coil1Name, coil1Type, waterVdot, isAutoSized, wInletNodeNum, wOutletNodeNum, loopNum);
    EXPECT_EQ(-999, c1->pltSizNum);
    EXPECT_EQ(loopNum, c1->waterLoopNum);
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, c1->plantLoopName);
    EXPECT_EQ(-999, c1->rhoFluid);
    EXPECT_EQ(-999, c1->cpFluid);
    EXPECT_EQ(-999, c1->coilDesWaterMassFlow);
    EXPECT_EQ("No", c1->coilWaterFlowAutoMsg);

    // Now add a plant sizing object
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).PlantLoopName = "Steam Loop";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Steam;
    isAutoSized = true; // true if autosized
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowNodeNums(
        *state, coil1Name, coil1Type, waterVdot, isAutoSized, wInletNodeNum, wOutletNodeNum, loopNum);
    auto &c1b(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0]);
    EXPECT_EQ(1, c1b->pltSizNum);
    EXPECT_EQ(loopNum, c1b->waterLoopNum);
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, c1b->plantLoopName);
    EXPECT_NEAR(0.6, c1b->rhoFluid, 0.01);
    EXPECT_NEAR(4216.0, c1b->cpFluid, 0.1);
    Real64 expFlow = waterVdot * c1b->rhoFluid;
    EXPECT_NEAR(expFlow, c1b->coilDesWaterMassFlow, 0.01);
    EXPECT_EQ("Yes", c1b->coilWaterFlowAutoMsg);

    // Exercise report writing again
    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);
}

TEST_F(EnergyPlusFixture, ReportCoilSelection_ZoneEqCoil)
{
    std::string coil1Name("Coil 1");            // user-defined name of the coil
    std::string coil1Type("Coil:Heating:Fuel"); // idf input object class name of coil

    state->dataGlobal->NumOfZones = 3;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "Zone 1";
    state->dataHeatBal->Zone(2).Name = "Zone 2";
    state->dataHeatBal->Zone(3).Name = "Zone 3";

    int curSysNum = 0;
    int curZoneEqNum = 2;
    int curOASysNum = 0;
    state->dataZoneEquip->ZoneEquipList.allocate(3);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).NumOfEquipTypes = 2;
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipName.allocate(2);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipTypeName.allocate(2);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipType.allocate(2);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipName(1) = "Zone 2 Fan Coil";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipTypeName(1) = "ZoneHVAC:FourPipeFanCoil";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipType(1) = DataZoneEquipment::ZoneEquipType::FourPipeFanCoil;
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipName(2) = "Zone 2 Unit Heater";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipTypeName(2) = "ZoneHVAC:UnitHeater";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipType(2) = DataZoneEquipment::ZoneEquipType::UnitHeater;
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipData.allocate(3);

    Real64 totGrossCap = 500.0;
    Real64 sensGrossCap = 500.0;
    Real64 airFlowRate = 0.11;
    Real64 waterFlowRate = 0.0;

    state->dataRptCoilSelection->coilSelectionReportObj->setCoilFinalSizes(
        *state, coil1Name, coil1Type, totGrossCap, sensGrossCap, airFlowRate, waterFlowRate);
    auto &c1(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0]);
    EXPECT_EQ(totGrossCap, c1->coilTotCapFinal);
    EXPECT_EQ(sensGrossCap, c1->coilSensCapFinal);
    EXPECT_EQ(airFlowRate, c1->coilRefAirVolFlowFinal);
    EXPECT_EQ(waterFlowRate, c1->coilRefWaterVolFlowFinal);

    Real64 RatedCoilTotCap = 400.0;
    Real64 RatedCoilSensCap = 399.0;
    Real64 RatedAirMassFlow = 0.001;
    Real64 RatedCoilInDb = -999.0;
    Real64 RatedCoilInHumRat = -999.0;
    Real64 RatedCoilInWb = 20.0;
    Real64 RatedCoilOutDb = -999.0;
    Real64 RatedCoilOutHumRat = -999.0;
    Real64 RatedCoilOutWb = 30.0;
    Real64 RatedCoilOadbRef = 24.0;
    Real64 RatedCoilOawbRef = 16.0;
    Real64 RatedCoilBpFactor = 0.2;
    Real64 RatedCoilEff = 0.8;

    // First without setting coil inlet/outlet conditions
    state->dataRptCoilSelection->coilSelectionReportObj->setRatedCoilConditions(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                RatedCoilSensCap,
                                                                                RatedAirMassFlow,
                                                                                RatedCoilInDb,
                                                                                RatedCoilInHumRat,
                                                                                RatedCoilInWb,
                                                                                RatedCoilOutDb,
                                                                                RatedCoilOutHumRat,
                                                                                RatedCoilOutWb,
                                                                                RatedCoilOadbRef,
                                                                                RatedCoilOawbRef,
                                                                                RatedCoilBpFactor,
                                                                                RatedCoilEff);

    EXPECT_EQ(RatedCoilTotCap, c1->coilRatedTotCap);
    EXPECT_NEAR(c1->coilCapFTIdealPeak, 1.0, 0.000001);
    EXPECT_EQ(RatedCoilSensCap, c1->coilRatedSensCap);
    EXPECT_EQ(RatedAirMassFlow, c1->ratedAirMassFlow);
    EXPECT_EQ(RatedCoilInDb, c1->ratedCoilInDb);
    EXPECT_EQ(RatedCoilInWb, c1->ratedCoilInWb);
    EXPECT_EQ(RatedCoilInHumRat, c1->ratedCoilInHumRat);
    EXPECT_EQ(-999.0, c1->ratedCoilInEnth);
    EXPECT_EQ(RatedCoilOutDb, c1->ratedCoilOutDb);
    EXPECT_EQ(RatedCoilOutWb, c1->ratedCoilOutWb);
    EXPECT_EQ(RatedCoilOutHumRat, c1->ratedCoilOutHumRat);
    EXPECT_EQ(-999.0, c1->ratedCoilOutEnth);
    EXPECT_EQ(RatedCoilEff, c1->ratedCoilEff);
    EXPECT_EQ(RatedCoilBpFactor, c1->ratedCoilBpFactor);
    EXPECT_EQ(RatedCoilOadbRef, c1->ratedCoilOadbRef);
    EXPECT_EQ(RatedCoilOawbRef, c1->ratedCoilOawbRef);

    // again with setting coil inlet/outlet conditions
    RatedCoilInDb = 23.0;
    RatedCoilInHumRat = 0.008;
    RatedCoilOutDb = 40.0;
    RatedCoilOutHumRat = 0.009;
    state->dataRptCoilSelection->coilSelectionReportObj->setRatedCoilConditions(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                RatedCoilSensCap,
                                                                                RatedAirMassFlow,
                                                                                RatedCoilInDb,
                                                                                RatedCoilInHumRat,
                                                                                RatedCoilInWb,
                                                                                RatedCoilOutDb,
                                                                                RatedCoilOutHumRat,
                                                                                RatedCoilOutWb,
                                                                                RatedCoilOadbRef,
                                                                                RatedCoilOawbRef,
                                                                                RatedCoilBpFactor,
                                                                                RatedCoilEff);
    EXPECT_EQ(RatedCoilInDb, c1->ratedCoilInDb);
    EXPECT_EQ(RatedCoilInHumRat, c1->ratedCoilInHumRat);
    EXPECT_NEAR(43460.9, c1->ratedCoilInEnth, 0.1);
    EXPECT_EQ(RatedCoilOutDb, c1->ratedCoilOutDb);
    EXPECT_EQ(RatedCoilOutHumRat, c1->ratedCoilOutHumRat);
    EXPECT_NEAR(63371.3, c1->ratedCoilOutEnth, 0.1);

    Real64 entAirDryBulbTemp = 24.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntAirTemp(*state, coil1Name, coil1Type, entAirDryBulbTemp, curSysNum, curZoneEqNum);
    EXPECT_EQ(entAirDryBulbTemp, c1->coilDesEntTemp);
    EXPECT_EQ(curSysNum, c1->airloopNum);
    EXPECT_EQ(curZoneEqNum, c1->zoneEqNum);

    Real64 entAirHumRat = 0.004;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntAirHumRat(*state, coil1Name, coil1Type, entAirHumRat);
    EXPECT_EQ(entAirHumRat, c1->coilDesEntHumRat);

    Real64 entWaterTemp = 60.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntWaterTemp(*state, coil1Name, coil1Type, entWaterTemp);
    EXPECT_EQ(entWaterTemp, c1->coilDesWaterEntTemp);

    Real64 lvgWaterTemp = 50.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgWaterTemp(*state, coil1Name, coil1Type, lvgWaterTemp);
    EXPECT_EQ(lvgWaterTemp, c1->coilDesWaterLvgTemp);

    Real64 CoilWaterDeltaT = 50.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilWaterDeltaT(*state, coil1Name, coil1Type, CoilWaterDeltaT);
    EXPECT_EQ(CoilWaterDeltaT, c1->coilDesWaterTempDiff);

    Real64 lvgAirDryBulbTemp = 12.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirTemp(*state, coil1Name, coil1Type, lvgAirDryBulbTemp);
    EXPECT_EQ(lvgAirDryBulbTemp, c1->coilDesLvgTemp);

    Real64 lvgAirHumRat = 0.006;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirHumRat(*state, coil1Name, coil1Type, lvgAirHumRat);
    EXPECT_EQ(lvgAirHumRat, c1->coilDesLvgHumRat);

    int zoneNum = 1;
    Real64 zoneCoolingLatentLoad = 1234.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadCoolingIdealPeak(zoneNum, zoneCoolingLatentLoad);
    // Expect zero because it's a heating coil
    EXPECT_EQ(0.0, c1->rmLatentAtPeak);

    Real64 zoneHeatingLatentLoad = 4321.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadHeatingIdealPeak(zoneNum, zoneHeatingLatentLoad);
    // Expect zero because doZoneEqSetup isn't currently executed
    EXPECT_EQ(0.0, c1->rmLatentAtPeak);

    // Exercise report writing again
    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);

    // Test coil reporting
    curZoneEqNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->CurTermUnitSizingNum = curZoneEqNum;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU = RatedCoilInDb;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum).DesHeatCoilInHumRatTU = RatedCoilInHumRat;
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(curZoneEqNum).ZoneName = state->dataHeatBal->Zone(1).Name;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(curZoneEqNum).HeatDesDay = "Heat Design Day";
    state->dataSize->FinalZoneSizing(curZoneEqNum).DesHeatLoad = RatedCoilSensCap;
    state->dataSize->FinalZoneSizing(curZoneEqNum).OutTempAtHeatPeak = RatedCoilOutDb;
    state->dataSize->FinalZoneSizing(curZoneEqNum).OutHumRatAtHeatPeak = RatedCoilOutHumRat;
    state->dataSize->FinalZoneSizing(curZoneEqNum).ZoneRetTempAtHeatPeak = 21.6;
    state->dataSize->FinalZoneSizing(curZoneEqNum).ZoneHumRatAtHeatPeak = 0.007;
    state->dataSize->FinalZoneSizing(curZoneEqNum).ZoneTempAtHeatPeak = 21.0;
    state->dataSize->FinalZoneSizing(curZoneEqNum).HeatDesTemp = 30.0;
    state->dataSize->FinalZoneSizing(curZoneEqNum).HeatDesHumRat = 0.007;

    Real64 fanHeatGain = 1.3;
    Real64 coilCapFunTempFac = 1.0;
    Real64 DXFlowPerCapMinRatio = 0.00004;
    Real64 DXFlowPerCapMaxRatio = 0.00006;
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataSize->DataFlowUsedForSizing = airFlowRate / state->dataEnvrn->StdRhoAir;

    // setCoilHeatingCapacity will not overwrite previously set temperature data
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilHeatingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    EXPECT_EQ(entAirDryBulbTemp, c1->coilDesEntTemp);

    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadHeatingIdealPeak(zoneNum, zoneHeatingLatentLoad);
    // Expect non-zero because this is a heating coil
    EXPECT_LT(0.0, zoneHeatingLatentLoad);
    EXPECT_NEAR(zoneHeatingLatentLoad, c1->rmLatentAtPeak, 0.000001);

    entAirDryBulbTemp = 21.0; // change coil entering air temp
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntAirTemp(*state, coil1Name, coil1Type, entAirDryBulbTemp, curSysNum, curZoneEqNum);
    lvgAirDryBulbTemp = 30.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirTemp(*state, coil1Name, coil1Type, lvgAirDryBulbTemp);
    EXPECT_EQ(entAirDryBulbTemp, c1->coilDesEntTemp);
    EXPECT_EQ(lvgAirDryBulbTemp, c1->coilDesLvgTemp);

    state->dataSize->TermUnitSingDuct = true;
    // now reset entering/leaving air temps so that setCoilHeatingCapacity will initialize any uninitialized temperature data
    // for example if a non-water coil is used in a terminal unit and setCoilEntAirTemp is not called
    c1->coilDesEntTemp = -999.0;
    c1->coilDesEntHumRat = -999.0;
    c1->coilDesLvgTemp = -999.0;
    c1->coilDesLvgHumRat = -999.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilHeatingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    EXPECT_EQ(RatedCoilInDb, c1->coilDesEntTemp);
    EXPECT_EQ(RatedCoilInHumRat, c1->coilDesEntHumRat);
    EXPECT_EQ(state->dataSize->FinalZoneSizing(curZoneEqNum).HeatDesTemp, c1->coilDesLvgTemp);
    EXPECT_EQ(state->dataSize->FinalZoneSizing(curZoneEqNum).HeatDesHumRat, c1->coilDesLvgHumRat);

    // test heating capacity with default CapFT
    EXPECT_NEAR(RatedCoilTotCap * coilCapFunTempFac, c1->coilTotCapAtPeak, 0.000001);
    EXPECT_NEAR(coilCapFunTempFac, 1.0, 0.000001);
    EXPECT_NEAR(RatedCoilTotCap, c1->coilTotCapAtPeak, 0.000001);
    // test heating capacity adjustment with a CapFT curve
    coilCapFunTempFac = 1.15;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilHeatingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    EXPECT_NEAR(RatedCoilTotCap * coilCapFunTempFac, c1->coilTotCapAtPeak, 0.000001);
    EXPECT_LT(RatedCoilTotCap, c1->coilTotCapAtPeak);
}

TEST_F(EnergyPlusFixture, ReportCoilSelection_ZoneEqCoolingCoil)
{
    std::string coil1Name("Coil 1");          // user-defined name of the coil
    std::string coil1Type("Coil:Cooling:DX"); // idf input object class name of coil

    state->dataGlobal->NumOfZones = 3;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "Zone 1";
    state->dataHeatBal->Zone(2).Name = "Zone 2";
    state->dataHeatBal->Zone(3).Name = "Zone 3";

    int curSysNum = 0;
    int curZoneEqNum = 2;
    int curOASysNum = 0;
    state->dataZoneEquip->ZoneEquipList.allocate(3);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).NumOfEquipTypes = 2;
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipName.allocate(2);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipTypeName.allocate(2);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipType.allocate(2);
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipName(1) = "Zone 2 DX Eq";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipTypeName(1) = "ZoneHVAC:WindowAirConditioner";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipType(1) = DataZoneEquipment::ZoneEquipType::WindowAirConditioner;
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipName(2) = "Zone 2 Unit Heater";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipTypeName(2) = "ZoneHVAC:UnitHeater";
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipType(2) = DataZoneEquipment::ZoneEquipType::UnitHeater;
    state->dataZoneEquip->ZoneEquipList(curZoneEqNum).EquipData.allocate(3);

    Real64 totGrossCap = 500.0;
    Real64 sensGrossCap = 400.0;
    Real64 airFlowRate = 0.11;
    Real64 waterFlowRate = 0.0;

    state->dataRptCoilSelection->coilSelectionReportObj->setCoilFinalSizes(
        *state, coil1Name, coil1Type, totGrossCap, sensGrossCap, airFlowRate, waterFlowRate);
    auto &c1(state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0]);
    EXPECT_EQ(totGrossCap, c1->coilTotCapFinal);
    EXPECT_EQ(sensGrossCap, c1->coilSensCapFinal);
    EXPECT_EQ(airFlowRate, c1->coilRefAirVolFlowFinal);
    EXPECT_EQ(waterFlowRate, c1->coilRefWaterVolFlowFinal);

    Real64 RatedCoilTotCap = 400.0;
    Real64 RatedCoilSensCap = 300.0;
    Real64 RatedAirMassFlow = 0.001;
    Real64 RatedCoilInDb = -999.0;
    Real64 RatedCoilInHumRat = -999.0;
    Real64 RatedCoilInWb = 20.0;
    Real64 RatedCoilOutDb = -999.0;
    Real64 RatedCoilOutHumRat = -999.0;
    Real64 RatedCoilOutWb = 30.0;
    Real64 RatedCoilOadbRef = 24.0;
    Real64 RatedCoilOawbRef = 16.0;
    Real64 RatedCoilBpFactor = 0.2;
    Real64 RatedCoilEff = 0.8;

    // First without setting coil inlet/outlet conditions
    state->dataRptCoilSelection->coilSelectionReportObj->setRatedCoilConditions(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                RatedCoilSensCap,
                                                                                RatedAirMassFlow,
                                                                                RatedCoilInDb,
                                                                                RatedCoilInHumRat,
                                                                                RatedCoilInWb,
                                                                                RatedCoilOutDb,
                                                                                RatedCoilOutHumRat,
                                                                                RatedCoilOutWb,
                                                                                RatedCoilOadbRef,
                                                                                RatedCoilOawbRef,
                                                                                RatedCoilBpFactor,
                                                                                RatedCoilEff);

    EXPECT_EQ(RatedCoilTotCap, c1->coilRatedTotCap);
    EXPECT_NEAR(c1->coilCapFTIdealPeak, 1.0, 0.000001);
    EXPECT_EQ(RatedCoilSensCap, c1->coilRatedSensCap);
    EXPECT_EQ(RatedAirMassFlow, c1->ratedAirMassFlow);
    EXPECT_EQ(RatedCoilInDb, c1->ratedCoilInDb);
    EXPECT_EQ(RatedCoilInWb, c1->ratedCoilInWb);
    EXPECT_EQ(RatedCoilInHumRat, c1->ratedCoilInHumRat);
    EXPECT_EQ(-999.0, c1->ratedCoilInEnth);
    EXPECT_EQ(RatedCoilOutDb, c1->ratedCoilOutDb);
    EXPECT_EQ(RatedCoilOutWb, c1->ratedCoilOutWb);
    EXPECT_EQ(RatedCoilOutHumRat, c1->ratedCoilOutHumRat);
    EXPECT_EQ(-999.0, c1->ratedCoilOutEnth);
    EXPECT_EQ(RatedCoilEff, c1->ratedCoilEff);
    EXPECT_EQ(RatedCoilBpFactor, c1->ratedCoilBpFactor);
    EXPECT_EQ(RatedCoilOadbRef, c1->ratedCoilOadbRef);
    EXPECT_EQ(RatedCoilOawbRef, c1->ratedCoilOawbRef);

    // again with setting coil inlet/outlet conditions
    RatedCoilInDb = 23.0;
    RatedCoilInHumRat = 0.008;
    RatedCoilOutDb = 12.0;
    RatedCoilOutHumRat = 0.006;
    state->dataRptCoilSelection->coilSelectionReportObj->setRatedCoilConditions(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                RatedCoilSensCap,
                                                                                RatedAirMassFlow,
                                                                                RatedCoilInDb,
                                                                                RatedCoilInHumRat,
                                                                                RatedCoilInWb,
                                                                                RatedCoilOutDb,
                                                                                RatedCoilOutHumRat,
                                                                                RatedCoilOutWb,
                                                                                RatedCoilOadbRef,
                                                                                RatedCoilOawbRef,
                                                                                RatedCoilBpFactor,
                                                                                RatedCoilEff);
    EXPECT_EQ(RatedCoilInDb, c1->ratedCoilInDb);
    EXPECT_EQ(RatedCoilInHumRat, c1->ratedCoilInHumRat);
    EXPECT_NEAR(43460.9, c1->ratedCoilInEnth, 0.1);
    EXPECT_EQ(RatedCoilOutDb, c1->ratedCoilOutDb);
    EXPECT_EQ(RatedCoilOutHumRat, c1->ratedCoilOutHumRat);
    EXPECT_NEAR(27197.5, c1->ratedCoilOutEnth, 0.1);

    Real64 entAirDryBulbTemp = 24.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntAirTemp(*state, coil1Name, coil1Type, entAirDryBulbTemp, curSysNum, curZoneEqNum);
    EXPECT_EQ(entAirDryBulbTemp, c1->coilDesEntTemp);
    EXPECT_EQ(curSysNum, c1->airloopNum);
    EXPECT_EQ(curZoneEqNum, c1->zoneEqNum);

    Real64 entAirHumRat = 0.004;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntAirHumRat(*state, coil1Name, coil1Type, entAirHumRat);
    EXPECT_EQ(entAirHumRat, c1->coilDesEntHumRat);

    Real64 lvgAirDryBulbTemp = 14.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirTemp(*state, coil1Name, coil1Type, lvgAirDryBulbTemp);
    EXPECT_EQ(lvgAirDryBulbTemp, c1->coilDesLvgTemp);

    Real64 lvgAirHumRat = 0.005;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirHumRat(*state, coil1Name, coil1Type, lvgAirHumRat);
    EXPECT_EQ(lvgAirHumRat, c1->coilDesLvgHumRat);

    int zoneNum = 1;
    Real64 zoneCoolingLatentLoad = 1234.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadCoolingIdealPeak(zoneNum, zoneCoolingLatentLoad);
    // Expect zero because doZoneEqSetup isn't currently executed
    EXPECT_EQ(0.0, c1->rmLatentAtPeak);

    Real64 zoneHeatingLatentLoad = 4321.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadHeatingIdealPeak(zoneNum, zoneHeatingLatentLoad);
    // Expect zero because doZoneEqSetup isn't currently executed
    EXPECT_EQ(0.0, c1->rmLatentAtPeak);

    // Exercise report writing again
    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);

    // Test coil reporting
    curZoneEqNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(curZoneEqNum).ZoneName = state->dataHeatBal->Zone(1).Name;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(curZoneEqNum).CoolDesDay = "Cool Design Day";
    state->dataSize->FinalZoneSizing(curZoneEqNum).DesCoolLoad = RatedCoilSensCap;
    state->dataSize->FinalZoneSizing(curZoneEqNum).OutTempAtCoolPeak = RatedCoilOutDb;
    state->dataSize->FinalZoneSizing(curZoneEqNum).OutHumRatAtCoolPeak = RatedCoilOutHumRat;
    state->dataSize->FinalZoneSizing(curZoneEqNum).DesCoolCoilInTemp = 25.0;
    state->dataSize->FinalZoneSizing(curZoneEqNum).DesCoolCoilInHumRat = 0.007;
    state->dataSize->FinalZoneSizing(curZoneEqNum).CoolDesTemp = 12.0;
    state->dataSize->FinalZoneSizing(curZoneEqNum).CoolDesHumRat = 0.007;

    Real64 fanHeatGain = 1.3;
    Real64 coilCapFunTempFac = 1.0;
    Real64 DXFlowPerCapMinRatio = 0.00004;
    Real64 DXFlowPerCapMaxRatio = 0.00006;
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataSize->DataFlowUsedForSizing = airFlowRate / state->dataEnvrn->StdRhoAir;

    // setCoilCoolingCapacity will not overwrite previously set temperature data
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilCoolingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    EXPECT_EQ(entAirDryBulbTemp, c1->coilDesEntTemp);
    EXPECT_EQ(entAirHumRat, c1->coilDesEntHumRat);
    EXPECT_EQ(lvgAirDryBulbTemp, c1->coilDesLvgTemp);
    EXPECT_EQ(lvgAirHumRat, c1->coilDesLvgHumRat);
    Real64 CpMoistAir = Psychrometrics::PsyCpAirFnW(c1->coilDesEntHumRat);
    EXPECT_EQ(CpMoistAir, c1->cpMoistAir);
    EXPECT_NEAR(c1->fanHeatGainIdealPeak, fanHeatGain, 0.000001);

    state->dataRptCoilSelection->coilSelectionReportObj->setZoneLatentLoadCoolingIdealPeak(zoneNum, zoneCoolingLatentLoad);
    // Expect non-zero zone cooling latent load
    EXPECT_EQ(zoneCoolingLatentLoad, c1->rmLatentAtPeak);

    entAirDryBulbTemp = 21.0; // change coil entering air temp
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilEntAirTemp(*state, coil1Name, coil1Type, entAirDryBulbTemp, curSysNum, curZoneEqNum);
    lvgAirDryBulbTemp = 12.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirTemp(*state, coil1Name, coil1Type, lvgAirDryBulbTemp);
    EXPECT_EQ(entAirDryBulbTemp, c1->coilDesEntTemp);
    EXPECT_EQ(lvgAirDryBulbTemp, c1->coilDesLvgTemp);

    // now reset entering/leaving air temps so that setCoilCoolingCapacity will initialize any uninitialized temperature data
    // for example if setCoilEntAirTemp is not called
    c1->coilDesEntTemp = -999.0;
    c1->coilDesEntHumRat = -999.0;
    c1->coilDesLvgTemp = -999.0;
    c1->coilDesLvgHumRat = -999.0;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilCoolingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    EXPECT_EQ(state->dataSize->FinalZoneSizing(curZoneEqNum).DesCoolCoilInTemp, c1->coilDesEntTemp);
    EXPECT_EQ(state->dataSize->FinalZoneSizing(curZoneEqNum).DesCoolCoilInHumRat, c1->coilDesEntHumRat);
    EXPECT_EQ(state->dataSize->FinalZoneSizing(curZoneEqNum).CoolDesTemp, c1->coilDesLvgTemp);
    EXPECT_EQ(state->dataSize->FinalZoneSizing(curZoneEqNum).CoolDesHumRat, c1->coilDesLvgHumRat);

    // test cooling capacity with default CapFT = 1
    EXPECT_NEAR(RatedCoilTotCap * coilCapFunTempFac, c1->coilTotCapAtPeak, 0.000001);
    EXPECT_NEAR(coilCapFunTempFac, 1.0, 0.000001);
    EXPECT_NEAR(RatedCoilTotCap, c1->coilTotCapAtPeak, 0.000001);

    // test cooling capacity adjustment with a CapFT curve != 1
    coilCapFunTempFac = 1.15;
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilCoolingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    EXPECT_NEAR(RatedCoilTotCap * coilCapFunTempFac, c1->coilTotCapAtPeak, 0.000001);
    EXPECT_LT(RatedCoilTotCap, c1->coilTotCapAtPeak);
}

TEST_F(EnergyPlusFixture, ReportCoilSelection_4PipeFCU_ElecHeatingCoil)
{
    std::string coil1Name("ElecHeatCoil");          // user-defined name of the coil
    std::string coil1Type("Coil:Heating:Electric"); // idf input object class name of coil

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "Zone 1";

    int curSysNum = 0;
    int curOASysNum = 0;
    int curZoneEqNum = 1;
    state->dataZoneEquip->ZoneEquipList.allocate(1);
    auto &zoneEquipList = state->dataZoneEquip->ZoneEquipList(curZoneEqNum);

    zoneEquipList.NumOfEquipTypes = 1;
    zoneEquipList.EquipName.allocate(1);
    zoneEquipList.EquipTypeName.allocate(1);
    zoneEquipList.EquipType.allocate(1);
    zoneEquipList.EquipName(1) = "Zone 1 FCU";
    zoneEquipList.EquipTypeName(1) = "ZoneHVAC:FourPipeFanCoil";
    zoneEquipList.EquipType(1) = DataZoneEquipment::ZoneEquipType::FourPipeFanCoil;

    Real64 totGrossCap = 6206.4;
    Real64 sensGrossCap = 6206.4;
    Real64 airVolFlowRate = 0.1385;
    Real64 waterFlowRate = 0.0;

    state->dataRptCoilSelection->coilSelectionReportObj->setCoilFinalSizes(
        *state, coil1Name, coil1Type, totGrossCap, sensGrossCap, airVolFlowRate, waterFlowRate);
    auto &c1 = state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0];

    EXPECT_EQ(totGrossCap, c1->coilTotCapFinal);
    EXPECT_EQ(sensGrossCap, c1->coilSensCapFinal);
    EXPECT_EQ(airVolFlowRate, c1->coilRefAirVolFlowFinal);
    EXPECT_EQ(waterFlowRate, c1->coilRefWaterVolFlowFinal);

    // set electric heating coil conditions
    Real64 RatedCoilTotCap = 6206.5;
    Real64 RatedCoilSensCap = 6206.5;
    Real64 RatedAirMassFlow = 0.163;
    Real64 RatedCoilInDb = 12.3785;
    Real64 RatedCoilInHumRat = 0.00335406;
    Real64 RatedCoilInWb = 6.02;
    Real64 RatedCoilOutDb = 50.0;
    Real64 RatedCoilOutHumRat = 0.004;
    Real64 RatedCoilOutWb = -999.0;
    Real64 RatedCoilOadbRef = -17.30;
    Real64 RatedCoilOawbRef = -17.30;
    Real64 MinOaFrac = 0.0;
    Real64 RatedCoilBpFactor = 0.0;
    Real64 RatedCoilEff = 0.80;
    Real64 result_coilDesEntEnth = Psychrometrics::PsyHFnTdbW(RatedCoilInDb, RatedCoilInHumRat);
    Real64 result_coilDesOutEnth = Psychrometrics::PsyHFnTdbW(RatedCoilOutDb, RatedCoilOutHumRat);
    // set coil inlet/outlet conditions
    state->dataRptCoilSelection->coilSelectionReportObj->setRatedCoilConditions(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                RatedCoilSensCap,
                                                                                RatedAirMassFlow,
                                                                                RatedCoilInDb,
                                                                                RatedCoilInHumRat,
                                                                                RatedCoilInWb,
                                                                                RatedCoilOutDb,
                                                                                RatedCoilOutHumRat,
                                                                                RatedCoilOutWb,
                                                                                RatedCoilOadbRef,
                                                                                RatedCoilOawbRef,
                                                                                RatedCoilBpFactor,
                                                                                RatedCoilEff);
    // check coil inlet/outlet conditions set correctly
    EXPECT_EQ(RatedCoilInDb, c1->ratedCoilInDb);
    EXPECT_EQ(RatedCoilInHumRat, c1->ratedCoilInHumRat);
    EXPECT_NEAR(result_coilDesEntEnth, c1->ratedCoilInEnth, 0.1);
    EXPECT_EQ(RatedCoilOutDb, c1->ratedCoilOutDb);
    EXPECT_EQ(RatedCoilOutHumRat, c1->ratedCoilOutHumRat);
    EXPECT_NEAR(result_coilDesOutEnth, c1->ratedCoilOutEnth, 0.1);

    // test electric heating coil reporting
    state->dataSize->ZoneEqSizing.allocate(1);
    auto &zoneEqSizing = state->dataSize->ZoneEqSizing(curZoneEqNum);
    state->dataSize->FinalZoneSizing.allocate(1);
    auto &finalZoneSizing = state->dataSize->FinalZoneSizing(curZoneEqNum);
    state->dataSize->ZoneEqFanCoil = true;
    zoneEqSizing.OAVolFlow = 0.02830;
    state->dataEnvrn->StdRhoAir = 1.1759;
    MinOaFrac = zoneEqSizing.OAVolFlow * state->dataEnvrn->StdRhoAir / RatedAirMassFlow;
    // set final zone sizing object inputs
    finalZoneSizing.HeatDesDay = "Heat Design Day";
    finalZoneSizing.DesHeatLoad = RatedCoilSensCap;
    finalZoneSizing.OutTempAtHeatPeak = -17.30;
    finalZoneSizing.OutHumRatAtHeatPeak = 0.00083893;
    finalZoneSizing.ZoneRetTempAtHeatPeak = 20.0;
    finalZoneSizing.ZoneHumRatAtHeatPeak = 0.007;
    finalZoneSizing.ZoneTempAtHeatPeak = 20.0;
    finalZoneSizing.HeatDesTemp = 50.0;
    finalZoneSizing.HeatDesHumRat = 0.004;
    finalZoneSizing.DesHeatOAFlowFrac = MinOaFrac;
    finalZoneSizing.DesHeatMassFlow = RatedAirMassFlow;
    // set additional function arguments
    Real64 fanHeatGain = 0.0;
    Real64 coilCapFunTempFac = 1.0;
    Real64 DXFlowPerCapMinRatio = 0.00004;
    Real64 DXFlowPerCapMaxRatio = 0.00006;
    // calculate coil entering conditions
    Real64 result_coilEntAirDryBulbTemp = MinOaFrac * finalZoneSizing.OutTempAtHeatPeak + (1.0 - MinOaFrac) * finalZoneSizing.ZoneTempAtHeatPeak;
    Real64 result_coilEntAirHumRat = MinOaFrac * finalZoneSizing.OutHumRatAtHeatPeak + (1.0 - MinOaFrac) * finalZoneSizing.ZoneHumRatAtHeatPeak;
    // calc sensible capacity
    Real64 result_sensCapacity =
        Psychrometrics::PsyCpAirFnW(finalZoneSizing.HeatDesHumRat) * RatedAirMassFlow * (finalZoneSizing.HeatDesTemp - result_coilEntAirDryBulbTemp);
    // set coil entering and leaving condition to autosize
    c1->coilDesEntTemp = -999.0;
    c1->coilDesEntHumRat = -999.0;
    c1->coilDesLvgTemp = -999.0;
    c1->coilDesLvgHumRat = -999.0;
    // check setCoilHeatingCapacity calculations and coil conditions are set correctly
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilHeatingCapacity(*state,
                                                                                coil1Name,
                                                                                coil1Type,
                                                                                RatedCoilTotCap,
                                                                                false,
                                                                                curSysNum,
                                                                                curZoneEqNum,
                                                                                curOASysNum,
                                                                                fanHeatGain,
                                                                                coilCapFunTempFac,
                                                                                DXFlowPerCapMinRatio,
                                                                                DXFlowPerCapMaxRatio);
    // check design outlet conditions
    EXPECT_EQ(finalZoneSizing.HeatDesTemp, c1->coilDesLvgTemp);
    EXPECT_EQ(finalZoneSizing.HeatDesHumRat, c1->coilDesLvgHumRat);
    // check design coil entering air conditions
    EXPECT_NEAR(result_coilEntAirDryBulbTemp, c1->coilDesEntTemp, 0.0001);
    EXPECT_NEAR(result_coilEntAirHumRat, c1->coilDesEntHumRat, 0.000001);
    // check total and sensible heating capacity
    EXPECT_EQ(6206.5, RatedCoilTotCap);
    EXPECT_NEAR(RatedCoilTotCap, c1->coilTotCapAtPeak, 0.1);
    EXPECT_NEAR(RatedCoilTotCap, c1->coilSensCapAtPeak, 0.1);
    EXPECT_NEAR(result_sensCapacity, c1->coilSensCapAtPeak, 0.1);
}

TEST_F(EnergyPlusFixture, Test_finishCoilSummaryReportTable)
{
    Real64 constexpr mult = 1.0;
    int curZoneEqNum = 1;
    std::string coil1Name = "ElecHeatCoil";          // user-defined name of the coil
    std::string coil1Type = "Coil:Heating:Electric"; // idf input object class name of coil

    // set up coil selection report object by calling a public function (i.e., calls getIndexForOrCreateDataObjFromCoilName)
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilReheatMultiplier(*state, coil1Name, coil1Type, mult);
    bool isValidCoilType = state->dataRptCoilSelection->coilSelectionReportObj->isCompTypeCoil(coil1Type);
    EXPECT_TRUE(isValidCoilType);
    auto &c1 = state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0];
    c1->zoneEqNum = curZoneEqNum;

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ThisZone";
    state->dataZoneEquip->ZoneEquipList.allocate(1);
    auto &zoneEquipList = state->dataZoneEquip->ZoneEquipList(curZoneEqNum);

    zoneEquipList.NumOfEquipTypes = 1;
    zoneEquipList.EquipName.allocate(1);
    zoneEquipList.EquipTypeName.allocate(1);
    zoneEquipList.EquipData.allocate(1);
    zoneEquipList.EquipType.allocate(1);
    zoneEquipList.EquipName(1) = "Zone 1 FCU";
    zoneEquipList.EquipTypeName(1) = "ZoneHVAC:FourPipeFanCoil";
    zoneEquipList.EquipType(1) = DataZoneEquipment::ZoneEquipType::FourPipeFanCoil;
    zoneEquipList.EquipData(1).Name = "ZoneHVAC:FourPipeFanCoil";
    zoneEquipList.EquipData(1).NumSubEquip = 2;
    zoneEquipList.EquipData(1).SubEquipData.allocate(2);
    zoneEquipList.EquipData(1).SubEquipData(1).Name = "ElecHeatCoil";
    zoneEquipList.EquipData(1).SubEquipData(1).TypeOf = "Coil:Heating:Electric";
    zoneEquipList.EquipData(1).SubEquipData(2).Name = "MyFan1";
    zoneEquipList.EquipData(1).SubEquipData(2).TypeOf = "FAN:ONOFF";

    // test that 1 equipment in the equipment list has data that is read from EquipmentList data.
    EXPECT_TRUE(Util::SameString(c1->coilLocation, "unknown"));
    EXPECT_TRUE(Util::SameString(c1->typeHVACname, "unknown"));
    EXPECT_TRUE(Util::SameString(c1->userNameforHVACsystem, "unknown"));

    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);

    EXPECT_TRUE(Util::SameString(c1->coilLocation, "Zone Equipment"));
    EXPECT_TRUE(Util::SameString(c1->typeHVACname, "ZoneHVAC:FourPipeFanCoil"));
    EXPECT_TRUE(Util::SameString(c1->userNameforHVACsystem, "Zone 1 FCU"));
    EXPECT_TRUE(Util::SameString(c1->zoneName[0], "ThisZone"));
    EXPECT_TRUE(Util::SameString(c1->fanTypeName, "FAN:ONOFF"));
    EXPECT_TRUE(Util::SameString(c1->fanAssociatedWithCoilName, "MyFan1"));

    // add another coil and hvac system and increase equipment list to 2
    zoneEquipList.NumOfEquipTypes = 2;
    zoneEquipList.EquipName.allocate(2);
    zoneEquipList.EquipTypeName.allocate(2);
    zoneEquipList.EquipData.allocate(2);
    zoneEquipList.EquipType.allocate(2);
    zoneEquipList.EquipIndex.allocate(2);

    EXPECT_TRUE(Util::SameString(zoneEquipList.EquipName(1), "")); // equipment list data is cleared
    EXPECT_TRUE(Util::SameString(zoneEquipList.EquipName(2), ""));

    // test that 2 equipment in the equipment list will fill coil selection data
    std::string coil2Name = "ElecHeatCoil 2";        // user-defined name of the coil
    std::string coil2Type = "Coil:Heating:Electric"; // idf input object class name of coil

    zoneEquipList.EquipName(1) = "Zone 1 FCU";
    zoneEquipList.EquipTypeName(1) = "ZoneHVAC:FourPipeFanCoil";
    zoneEquipList.EquipType(1) = DataZoneEquipment::ZoneEquipType::FourPipeFanCoil;
    zoneEquipList.EquipData(1).Name = "ZoneHVAC:FourPipeFanCoil";
    zoneEquipList.EquipData(1).NumSubEquip = 2;
    zoneEquipList.EquipData(1).SubEquipData.allocate(2);
    zoneEquipList.EquipData(1).SubEquipData(1).Name = coil1Name;
    zoneEquipList.EquipData(1).SubEquipData(2).Name = "MyFan1";
    zoneEquipList.EquipData(1).SubEquipData(2).TypeOf = "FAN:ONOFF";

    zoneEquipList.EquipName(2) = "Zone 1 ADU";
    zoneEquipList.EquipTypeName(2) = "ZoneHVAC:AirDistributionUnit";
    zoneEquipList.EquipType(2) = DataZoneEquipment::ZoneEquipType::AirDistributionUnit;
    zoneEquipList.EquipData(2).Name = "Zone 1 ADU";
    zoneEquipList.EquipData(2).NumSubEquip = 1;
    zoneEquipList.EquipData(2).SubEquipData.allocate(1);
    zoneEquipList.EquipData(2).SubEquipData(1).Name = "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT";
    zoneEquipList.EquipData(2).SubEquipData(1).NumSubSubEquip = 1;
    zoneEquipList.EquipData(2).SubEquipData(1).SubSubEquipData.allocate(1);
    zoneEquipList.EquipData(2).SubEquipData(1).SubSubEquipData(1).Name = coil2Name;

    // set up coil selection report object by calling a public function that calls getIndexForOrCreateDataObjFromCoilName
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilReheatMultiplier(*state, coil2Name, coil2Type, mult);
    auto &c1a = state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0];
    auto &c2a = state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[1];
    c2a->zoneEqNum = curZoneEqNum;

    EXPECT_TRUE(Util::SameString(c2a->coilLocation, "unknown"));
    EXPECT_TRUE(Util::SameString(c2a->typeHVACname, "unknown"));
    EXPECT_TRUE(Util::SameString(c2a->userNameforHVACsystem, "unknown"));

    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);

    EXPECT_TRUE(Util::SameString(c1a->coilLocation, "Zone Equipment"));
    EXPECT_TRUE(Util::SameString(c1a->typeHVACname, "ZoneHVAC:FourPipeFanCoil"));
    EXPECT_TRUE(Util::SameString(c1a->userNameforHVACsystem, "Zone 1 FCU"));
    EXPECT_TRUE(Util::SameString(c1a->coilName_, coil1Name));
    EXPECT_TRUE(Util::SameString(c1a->zoneName[0], "ThisZone"));
    EXPECT_TRUE(Util::SameString(c1a->fanTypeName, "FAN:ONOFF"));
    EXPECT_TRUE(Util::SameString(c1a->fanAssociatedWithCoilName, "MyFan1"));

    EXPECT_TRUE(Util::SameString(c2a->coilLocation, "Zone Equipment"));
    EXPECT_TRUE(Util::SameString(c2a->typeHVACname, "ZoneHVAC:AirDistributionUnit"));
    EXPECT_TRUE(Util::SameString(c2a->userNameforHVACsystem, "Zone 1 ADU"));
    EXPECT_TRUE(Util::SameString(c2a->coilName_, coil2Name));
    EXPECT_TRUE(Util::SameString(c2a->zoneName[0], "ThisZone"));

    // check equipment order and note coil1Name is associated with 4PipeFanCoil
    EXPECT_ENUM_EQ(zoneEquipList.EquipType(1), DataZoneEquipment::ZoneEquipType::FourPipeFanCoil);
    EXPECT_TRUE(Util::SameString(zoneEquipList.EquipData(1).SubEquipData(1).Name, coil1Name));
    EXPECT_ENUM_EQ(zoneEquipList.EquipType(2), DataZoneEquipment::ZoneEquipType::AirDistributionUnit);
    EXPECT_TRUE(Util::SameString(zoneEquipList.EquipData(2).SubEquipData(1).SubSubEquipData(1).Name, coil2Name));

    // delete coil report objects to start from scratch
    state->dataRptCoilSelection->coilSelectionReportObj->numCoilsReported_ = 0;
    state->dataRptCoilSelection->coilSelectionReportObj.reset(nullptr);
    createCoilSelectionReportObj(*state);

    // switch coil order in coil reports to try to find issues
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilReheatMultiplier(*state, coil2Name, coil2Type, mult);
    state->dataRptCoilSelection->coilSelectionReportObj->setCoilReheatMultiplier(*state, coil1Name, coil1Type, mult);
    auto &c1b = state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[0];
    auto &c2b = state->dataRptCoilSelection->coilSelectionReportObj->coilSelectionDataObjs[1];
    c1b->zoneEqNum = curZoneEqNum;
    c2b->zoneEqNum = curZoneEqNum;

    // switch the equipment order to try to find issues
    std::string tmpEquipName = zoneEquipList.EquipName(1);
    std::string tmpEquipTypeName = zoneEquipList.EquipTypeName(1);
    auto tmpEqData = zoneEquipList.EquipData(1);
    auto tmpEquipType = zoneEquipList.EquipType(1);

    zoneEquipList.EquipName(1) = zoneEquipList.EquipName(2);
    zoneEquipList.EquipTypeName(1) = zoneEquipList.EquipTypeName(2);
    zoneEquipList.EquipType(1) = zoneEquipList.EquipType(2);
    zoneEquipList.EquipData(1) = zoneEquipList.EquipData(2);

    zoneEquipList.EquipName(2) = tmpEquipName;
    zoneEquipList.EquipTypeName(2) = tmpEquipTypeName;
    zoneEquipList.EquipType(2) = tmpEquipType;
    zoneEquipList.EquipData(2) = tmpEqData;

    // check that equipment order is reversed in equipment list and note coil1Name is associated with 4PipeFanCoil
    EXPECT_ENUM_EQ(zoneEquipList.EquipType(1), DataZoneEquipment::ZoneEquipType::AirDistributionUnit);
    EXPECT_TRUE(Util::SameString(zoneEquipList.EquipData(1).SubEquipData(1).SubSubEquipData(1).Name, coil2Name));
    EXPECT_ENUM_EQ(zoneEquipList.EquipType(2), DataZoneEquipment::ZoneEquipType::FourPipeFanCoil);
    EXPECT_TRUE(Util::SameString(zoneEquipList.EquipData(2).SubEquipData(1).Name, coil1Name));

    state->dataRptCoilSelection->coilSelectionReportObj->finishCoilSummaryReportTable(*state);

    EXPECT_TRUE(Util::SameString(c1b->coilLocation, "Zone Equipment"));
    EXPECT_TRUE(Util::SameString(c1b->typeHVACname, "ZoneHVAC:AirDistributionUnit"));
    EXPECT_TRUE(Util::SameString(c1b->userNameforHVACsystem, "Zone 1 ADU"));
    EXPECT_TRUE(Util::SameString(c1b->coilName_, coil2Name));
    EXPECT_TRUE(Util::SameString(c1b->zoneName[0], "ThisZone"));

    // note coil1Name is associated with 4PipeFanCoil
    EXPECT_TRUE(Util::SameString(c2b->coilLocation, "Zone Equipment"));
    EXPECT_TRUE(Util::SameString(c2b->typeHVACname, "ZoneHVAC:FourPipeFanCoil"));
    EXPECT_TRUE(Util::SameString(c2b->userNameforHVACsystem, "Zone 1 FCU"));
    EXPECT_TRUE(Util::SameString(c2b->coilName_, coil1Name));
    EXPECT_TRUE(Util::SameString(c2b->zoneName[0], "ThisZone"));
    EXPECT_TRUE(Util::SameString(c2b->fanTypeName, "FAN:ONOFF"));
    EXPECT_TRUE(Util::SameString(c2b->fanAssociatedWithCoilName, "MyFan1"));
}
