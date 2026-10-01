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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/EquipAndOperations.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantCondLoopOperation.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>

using namespace EnergyPlus;

class DistributeEquipOpTest : public EnergyPlusFixture
{

public:
    //    static void SetUpTestCase()
    //    {
    //        EnergyPlusFixture::SetUpTestCase(); // Sets up the base fixture
    //    }
    static void TearDownTestCase()
    {
    }

    void SetUp() override
    {
        EnergyPlusFixture::SetUp(); // Sets up individual test cases.

        // unit test for PlantEquipmentOperation:ChillerHeaterChangeover
        state->dataHeatBal->Zone.allocate(4);
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(4);

        // set up two plantloop side2 with 2 branches, 2 components
        state->dataHeatBal->NumOfZoneLists = 1;
        state->dataHeatBal->ZoneList.allocate(1);
        state->dataHeatBal->ZoneList(1).Name = "THIS ZONE LIST";
        state->dataHeatBal->ZoneList(1).NumOfZones = 4;
        state->dataHeatBal->ZoneList(1).Zone.allocate(4);
        state->dataHeatBal->ZoneList(1).Zone(1) = 1;
        state->dataHeatBal->ZoneList(1).Zone(2) = 2;
        state->dataHeatBal->ZoneList(1).Zone(3) = 3;
        state->dataHeatBal->ZoneList(1).Zone(4) = 4;

        state->dataHVACGlobal->NumPrimaryAirSys = 1;
        state->dataAirLoop->AirLoopFlow.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
        state->dataAirLoop->AirToZoneNodeInfo.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
        state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopReturnNodeNum.allocate(1);
        state->dataAirLoop->AirToZoneNodeInfo(1).NumZonesCooled = 4;
        state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums.allocate(4);
        state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums(1) = state->dataHeatBal->ZoneList(1).Zone(3);
        state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums(2) = state->dataHeatBal->ZoneList(1).Zone(1);
        state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums(3) = state->dataHeatBal->ZoneList(1).Zone(4);
        state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums(4) = state->dataHeatBal->ZoneList(1).Zone(2);

        state->dataHVACGlobal->NumPlantLoops = 2;
        state->dataPlnt->TotNumLoops = state->dataHVACGlobal->NumPlantLoops;
        state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

        int loop = 0;
        for (auto &thisPlantLoop : state->dataPlnt->PlantLoop) {

            ++loop;
            if (loop == 1) {
                thisPlantLoop.Name = "Cooling Plant";
                thisPlantLoop.OperationScheme = "Cooling Loop Operation Scheme List";
            } else {
                thisPlantLoop.Name = "Heating Plant";
                thisPlantLoop.OperationScheme = "Heating Loop Operation Scheme List";
            }

            thisPlantLoop.FluidName = "WATER";
            thisPlantLoop.glycol = Fluid::GetWater(*state);

            thisPlantLoop.NumOpSchemes = 1;
            thisPlantLoop.OpScheme.allocate(thisPlantLoop.NumOpSchemes);
            auto &opSch1 = thisPlantLoop.OpScheme(1);
            opSch1.NumEquipLists = 1;
            opSch1.EquipList.allocate(2);

            for (DataPlant::LoopSideLocation LoopSideNum : DataPlant::LoopSideKeys) {
                thisPlantLoop.LoopSide(LoopSideNum).TotalBranches = 2;
                thisPlantLoop.LoopSide(LoopSideNum).Branch.allocate(2);
                thisPlantLoop.LoopSide(LoopSideNum).Branch(1).TotalComponents = 1;
                thisPlantLoop.LoopSide(LoopSideNum).Branch(2).TotalComponents = 1;
                thisPlantLoop.LoopSide(LoopSideNum).Branch(1).Comp.allocate(1);
                thisPlantLoop.LoopSide(LoopSideNum).Branch(2).Comp.allocate(1);
            }

            for (auto &opSch : thisPlantLoop.OpScheme) {
                for (size_t eqListNum = 1; eqListNum <= opSch.EquipList.size(); ++eqListNum) {
                    opSch.NumEquipLists = eqListNum;
                    auto &thisEquipList(opSch.EquipList(eqListNum));
                    thisEquipList.NumComps = eqListNum;
                    thisEquipList.Comp.allocate(thisEquipList.NumComps);

                    for (int compNum = 1; compNum <= opSch.EquipList(eqListNum).NumComps; ++compNum) {
                        // set up equipment list data
                        thisEquipList.Comp(compNum).CompNumPtr = compNum;
                        thisEquipList.Comp(compNum).BranchNumPtr = 1;
                    }
                }
            }
        }
    }

    virtual void ResetLoads()
    {
        // reset loads
        for (auto &thisPlantLoop : state->dataPlnt->PlantLoop) {
            auto &thisBranch(thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
            for (int compNum = 1; compNum <= thisPlantLoop.OpScheme(1).EquipList(1).NumComps; ++compNum) {
                thisBranch.Comp(compNum).MyLoad = 0.0;
            }
        }
    }
    void TearDown() override
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(DistributeEquipOpTest, EvaluateChillerHeaterChangeoverOpSchemeTest)
{

    std::string_view constexpr idf_objects = R"IDF(

      PlantEquipmentOperationSchemes,
        Cooling Loop Operation Scheme List, !- Name
        PlantEquipmentOperation:ChillerHeaterChangeover, !- Control Scheme 1 Object Type
        Two AWHP Operation Scheme,         !- Control Scheme 1 Name
        ALWAYS_ON;                         !- Control Scheme 1 Schedule Name

      PlantEquipmentOperationSchemes,
        Heating Loop Operation Scheme List, !- Name
        PlantEquipmentOperation:ChillerHeaterChangeover,  !- Control Scheme 1 Object Type
        Two AWHP Operation Scheme,         !- Control Scheme 1 Name
        ALWAYS_ON;                         !- Control Scheme 1 Schedule Name

      Schedule:Compact, ALWAYS_ON, On/Off, Through: 12/31, For: AllDays, Until: 24:00,1;

    PlantEquipmentOperation:ChillerHeaterChangeover,
      Two AWHP Operation Scheme ,          !- Name
      6.6 ,                                !- Primary Cooling Plant Setpoint Temperature
      13.7,                                !- Secondary Distribution Cooling Plant Setpoint Temperature
      59.8 ,                               !- Primary Heating Plant Setpoint at Outdoor High Temperature
      10.0,                                !- Outdoor High Temperature
      37.6 ,                               !- Primary Heating Plant Setpoint at Outdoor Low Temperature
      0.0 ,                                !- Outdoor Low Temperature
      45.0 ,                               !- Secondary Distribution Heating Plant Setpoint Temperature
      This Zone List,                      !- Zone Load Polling ZoneList Name
      Two AWHP Cooling Operation Scheme,   !- Cooling Only Load Plant Equipment Operation Cooling Load Name
      Two AWHP Heating Operation Scheme,   !- Heating Only Load Plant Equipment Operation Heating Load Name
      One AWHP Cooling Operation Scheme,   !- Simultaneous Cooling And Heating Plant Equipment Operation Cooling Load Name
      One AWHP Heating Operation Scheme,   !- Simultaneous Cooling And Heating Plant Equipment Operation Heating Load Name
      ,                                    !- Dedicated Chilled Water Return Recovery HeatPump Name
      ,                                    !- Dedicated Hot Water Return Recovery HeatPump Name
      1.0;                                 !-  Dedicated Recovery Heat Pump Control Load Capacity Factor

      PlantEquipmentOperation:CoolingLoad,
        Two AWHP Cooling Operation Scheme, !- Name
        0.0,                               !- Load Range 1 Lower Limit {W}
        50000,                             !- Load Range 1 Upper Limit {W}
        One AWHP Cooling Equipment List,   !- Range 1 Equipment List Name
        50000,                             !- Load Range 2 Lower Limit {W}
        10000000000000,                    !- Load Range 2 Upper Limit {W}
        Two AWHP Cooling Equipment List;   !- Range 2 Equipment List Name

      PlantEquipmentOperation:HeatingLoad,
        Two AWHP Heating Operation Scheme, !- Name
        0.0,                               !- Load Range 1 Lower Limit {W}
        100000,                            !- Load Range 1 Upper Limit {W}
        One AWHP Heating Equipment List,   !- Range 1 Equipment List Name
        100000,                            !- Load Range 2 Lower Limit {W}
        10000000000000,                    !- Load Range 2 Upper Limit {W}
        Two AWHP Heating Equipment List;   !- Range 2 Equipment List Name

      PlantEquipmentOperation:CoolingLoad,
        One AWHP Cooling Operation Scheme, !- Name
        0.0,                               !- Load Range 1 Lower Limit {W}
        10000000000000000,                 !- Load Range 1 Upper Limit {W}
        One AWHP Cooling Equipment List;   !- Range 1 Equipment List Name

      PlantEquipmentOperation:HeatingLoad,
        One AWHP Heating Operation Scheme, !- Name
        0.0,                               !- Load Range 1 Lower Limit {W}
        10000000000000000,                 !- Load Range 1 Upper Limit {W}
        One AWHP Heating Equipment List;   !- Range 1 Equipment List Name

      PlantEquipmentList,
        One AWHP Heating Equipment List,   !- Name
        HeatPump:PlantLoop:EIR:Heating,    !- Equipment 1 Object Type
        AWHP_2 Heating Side;               !- Equipment 1 Name

      PlantEquipmentList,
        One AWHP Cooling Equipment List,   !- Name
        HeatPump:PlantLoop:EIR:Cooling,    !- Equipment 1 Object Type
        AWHP_1 Cooling Side;               !- Equipment 1 Name

      PlantEquipmentList,
        Two AWHP Heating Equipment List,   !- Name
        HeatPump:PlantLoop:EIR:Heating,    !- Equipment 1 Object Type
        AWHP_1 Heating Side,               !- Equipment 1 Name
        HeatPump:PlantLoop:EIR:Heating,    !- Equipment 2 Object Type
        AWHP_2 Heating Side;               !- Equipment 2 Name

      PlantEquipmentList,
        Two AWHP Cooling Equipment List,   !- Name
        HeatPump:PlantLoop:EIR:Cooling,    !- Equipment 1 Object Type
        AWHP_1 Cooling Side,               !- Equipment 1 Name
        HeatPump:PlantLoop:EIR:Cooling,    !- Equipment 2 Object Type
        AWHP_2 Cooling Side;               !- Equipment 2 Name

      ZoneList,
        This Zone List,                    !- Name
        Zone1,                             !- Zone Name 1
        Zone2,                             !- Zone Name 2
        Zone3,                             !- Zone Name 3
        Zone4;                             !- Zone Name 4
     )IDF";

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    auto &heatBranch1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1);
    auto &heatComp1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    heatComp1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    heatComp1.Name = "AWHP_1 Heating Side";
    heatBranch1.NodeNumIn = 1;
    heatBranch1.NodeNumOut = 2;
    heatComp1.NodeNumIn = 1;
    heatComp1.NodeNumOut = 2;
    state->dataPlnt->PlantLoop(2).TempSetPointNodeNum = 2;
    auto &heatBranch2 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2);
    auto &heatComp2 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp(1);
    heatComp2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    heatComp2.Name = "AWHP_2 Heating Side";
    heatBranch2.NodeNumIn = 3;
    heatBranch2.NodeNumOut = 4;
    heatComp2.NodeNumIn = 3;
    heatComp2.NodeNumOut = 4;

    auto &coolBranch1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1);
    auto &coolComp1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    coolComp1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    coolComp1.Name = "AWHP_1 Cooling Side";
    coolBranch1.NodeNumIn = 5;
    coolBranch1.NodeNumOut = 6;
    coolComp1.NodeNumIn = 5;
    coolComp1.NodeNumOut = 6;
    state->dataPlnt->PlantLoop(1).TempSetPointNodeNum = 6;
    auto &coolBranch2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2);
    auto &coolComp2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp(1);
    coolComp2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    coolComp2.Name = "AWHP_2 Cooling Side";
    coolBranch2.NodeNumIn = 7;
    coolBranch2.NodeNumOut = 8;
    coolComp2.NodeNumIn = 7;
    coolComp2.NodeNumOut = 8;

    state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopReturnNodeNum(1) = 9;
    state->dataLoopNodes->Node.allocate(10);

    bool FirstHVACIteration = false;
    std::string CurrentModuleObject = "PlantEquipmentOperation:ChillerHeaterChangeover";
    PlantCondLoopOperation::InitLoadDistribution(*state, FirstHVACIteration);

    auto &chillerHeaterSupervisor = state->dataPlantCondLoopOp->ChillerHeaterSupervisoryOperationSchemes(1);
    EXPECT_TRUE(chillerHeaterSupervisor.oneTimeSetupComplete); // getInput completed

    EXPECT_EQ(1, chillerHeaterSupervisor.PlantOps.NumOfAirLoops);
    EXPECT_EQ(0, chillerHeaterSupervisor.PlantOps.numPlantLoadProfiles);
    EXPECT_EQ(2, chillerHeaterSupervisor.PlantOps.NumHeatingOnlyEquipLists);
    EXPECT_EQ(2, chillerHeaterSupervisor.PlantOps.NumCoolingOnlyEquipLists);
    EXPECT_EQ(1, chillerHeaterSupervisor.PlantOps.NumSimultHeatCoolHeatingEquipLists);
    EXPECT_EQ(1, chillerHeaterSupervisor.PlantOps.NumSimultHeatCoolCoolingEquipLists);

    EXPECT_TRUE(chillerHeaterSupervisor.PlantOps.SimultHeatCoolOpAvailable);

    EXPECT_NEAR(6.60, chillerHeaterSupervisor.Setpoint.PrimCW, 0.001);      // cooling set point temperature
    EXPECT_NEAR(59.8, chillerHeaterSupervisor.Setpoint.PrimHW_High, 0.001); // heating set point temperature
    EXPECT_NEAR(37.6, chillerHeaterSupervisor.Setpoint.PrimHW_Low, 0.001);  // heating set point temperature
    EXPECT_NEAR(13.7, chillerHeaterSupervisor.Setpoint.SecCW, 0.001);       // cooling set point temperature
    EXPECT_NEAR(45.0, chillerHeaterSupervisor.Setpoint.SecHW, 0.001);       // cooling set point temperature

    EXPECT_NEAR(0.00, chillerHeaterSupervisor.TempReset.LowOutdoorTemp, 0.001);
    EXPECT_NEAR(10.0, chillerHeaterSupervisor.TempReset.HighOutdoorTemp, 0.001);

    EXPECT_EQ(1, chillerHeaterSupervisor.ZonePtrs(1));
    EXPECT_EQ(2, chillerHeaterSupervisor.ZonePtrs(2));
    EXPECT_EQ(3, chillerHeaterSupervisor.ZonePtrs(3));
    EXPECT_EQ(4, chillerHeaterSupervisor.ZonePtrs(4));

    FirstHVACIteration = true;
    auto &thisSupervisor = state->dataPlnt->PlantLoop(1).OpScheme(1).ChillerHeaterSupervisoryOperation;
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);

    // zone loads have not been initialized
    EXPECT_EQ(0, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // off
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(1)).OutputRequiredToHeatingSP = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(2)).OutputRequiredToHeatingSP = 200.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(3)).OutputRequiredToHeatingSP = 300.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(4)).OutputRequiredToHeatingSP = 400.0;
    state->dataLoopNodes->Node(heatBranch1.NodeNumIn).MassFlowRate = 0.001; // set fake HW plant flow rate

    // zones have a heating load
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(1, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // heating
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(1000.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.001);
    EXPECT_NEAR(158.559, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.001);

    // check to see if correct plant equipment are active
    auto &eqcool = chillerHeaterSupervisor.CoolingOnlyEquipList(1).Comp(1);
    // set cooling demand node temp above cooling set point so equipment is activated if needed
    // heating demand temp = 0 (not initialized) so heating will be active if needed (i.e., temp < heating set point)
    state->dataLoopNodes->Node(eqcool.DemandNodeNum).Temp = 10.0;
    auto &eqheat = chillerHeaterSupervisor.HeatingOnlyEquipList(1).Comp(1);
    bool thisCoolEq1 = state->dataPlnt->PlantLoop(1).LoopSide(eqcool.LoopSideNumPtr).Branch(eqcool.BranchNumPtr).Comp(eqcool.CompNumPtr).ON;
    bool thisHeatEq1 = state->dataPlnt->PlantLoop(2).LoopSide(eqheat.LoopSideNumPtr).Branch(eqheat.BranchNumPtr).Comp(eqheat.CompNumPtr).ON;
    EXPECT_FALSE(thisCoolEq1); // cooling equipment is not active
    EXPECT_TRUE(thisHeatEq1);  // heating equipment active
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, "Cooling Plant");
    EXPECT_EQ(state->dataPlnt->PlantLoop(2).Name, "Heating Plant");
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.001);
    EXPECT_NEAR(158.559, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.001);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(1)).OutputRequiredToHeatingSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(2)).OutputRequiredToHeatingSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(3)).OutputRequiredToHeatingSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(4)).OutputRequiredToHeatingSP = 0.0;
    state->dataLoopNodes->Node(heatBranch1.NodeNumIn).MassFlowRate = 0.0; // set HW plant flow rate

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(1)).OutputRequiredToCoolingSP = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(2)).OutputRequiredToCoolingSP = 200.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(3)).OutputRequiredToCoolingSP = 300.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(4)).OutputRequiredToCoolingSP = 400.0;
    state->dataLoopNodes->Node(coolBranch1.NodeNumIn).MassFlowRate = 0.0; // set CW plant flow rate

    // zone heating loads are 0 and zone cooling loads are positive (no cooling)
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(0, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // off
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.001);
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.001);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(1)).OutputRequiredToCoolingSP = -100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(2)).OutputRequiredToCoolingSP = -200.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(3)).OutputRequiredToCoolingSP = -300.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(4)).OutputRequiredToCoolingSP = -400.0;
    state->dataLoopNodes->Node(coolBranch1.NodeNumIn).MassFlowRate = 0.001; // set fake CW plant flow rate

    // zone heating loads are 0 and zone cooling loads are negative (cooling)
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(2, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // cooling
    EXPECT_EQ(-1000.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    bool thisCoolEq2 = state->dataPlnt->PlantLoop(1).LoopSide(eqcool.LoopSideNumPtr).Branch(eqcool.BranchNumPtr).Comp(eqcool.CompNumPtr).ON;
    bool thisHeatEq2 = state->dataPlnt->PlantLoop(2).LoopSide(eqheat.LoopSideNumPtr).Branch(eqheat.BranchNumPtr).Comp(eqheat.CompNumPtr).ON;
    EXPECT_FALSE(thisHeatEq2); // heating equipment is not active
    EXPECT_TRUE(thisCoolEq2);  // cooling equipment active
    EXPECT_NEAR(-14.249, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.001);
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.001);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(1)).OutputRequiredToHeatingSP = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(2)).OutputRequiredToHeatingSP = 200.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(3)).OutputRequiredToHeatingSP = 300.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(4)).OutputRequiredToHeatingSP = 400.0;
    state->dataLoopNodes->Node(heatBranch1.NodeNumIn).MassFlowRate = 0.001; // set fake HW plant flow rate

    // zone heating loads are positive (heating) and zone cooling loads are negative (cooling)
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(3, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // simultaneous cooling and heating
    EXPECT_EQ(-1000.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(1000.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    EXPECT_NEAR(-14.249, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.001);
    EXPECT_NEAR(158.559, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.001);
}

TEST_F(DistributeEquipOpTest, SupervisoryControlLogicForAirSourcePlantsTest)
{

    std::string_view constexpr idf_objects = R"IDF(

      PlantEquipmentOperationSchemes,
        Cooling Loop Operation Scheme List, !- Name
        PlantEquipmentOperation:ChillerHeaterChangeover, !- Control Scheme 1 Object Type
        Two AWHP Operation Scheme,         !- Control Scheme 1 Name
        ALWAYS_ON;                         !- Control Scheme 1 Schedule Name

      PlantEquipmentOperationSchemes,
        Heating Loop Operation Scheme List, !- Name
        PlantEquipmentOperation:ChillerHeaterChangeover,  !- Control Scheme 1 Object Type
        Two AWHP Operation Scheme,         !- Control Scheme 1 Name
        ALWAYS_ON;                         !- Control Scheme 1 Schedule Name

      Schedule:Compact, ALWAYS_ON, On/Off, Through: 12/31, For: AllDays, Until: 24:00,1;

      PlantEquipmentOperation:ChillerHeaterChangeover,
        Two AWHP Operation Scheme ,          !- Name
        6.6 ,                                !- Primary Cooling Plant Setpoint Temperature
        13.7,                                !- Secondary Distribution Cooling Plant Setpoint Temperature
        59.8 ,                               !- Primary Heating Plant Setpoint at Outdoor High Temperature
        10.0,                                !- Outdoor High Temperature
        37.6 ,                               !- Primary Heating Plant Setpoint at Outdoor Low Temperature
        0.0 ,                                !- Outdoor Low Temperature
        45.0 ,                               !- Secondary Distribution Heating Plant Setpoint Temperature
        This Zone List,                      !- Zone Load Polling ZoneList Name
        One AWHP Cooling Operation Scheme,   !- Cooling Only Load Plant Equipment Operation Cooling Load Name
        One AWHP Heating Operation Scheme,   !- Heating Only Load Plant Equipment Operation Heating Load Name
        ,                                    !- Simultaneous Cooling And Heating Plant Equipment Operation Cooling Load Name
        ,                                    !- Simultaneous Cooling And Heating Plant Equipment Operation Heating Load Name
        ,                                    !- Dedicated Chilled Water Return Recovery HeatPump Name
        ,                                    !- Dedicated Hot Water Return Recovery HeatPump Name
        1.0;                                 !-  Dedicated Recovery Heat Pump Control Load Capacity Factor

      PlantEquipmentOperation:CoolingLoad,
        One AWHP Cooling Operation Scheme, !- Name
        0.0,                               !- Load Range 1 Lower Limit {W}
        10000000000000000,                 !- Load Range 1 Upper Limit {W}
        One AWHP Cooling Equipment List;   !- Range 1 Equipment List Name

      PlantEquipmentList,
        One AWHP Cooling Equipment List,   !- Name
        HeatPump:PlantLoop:EIR:Cooling,    !- Equipment 1 Object Type
        AWHP_1 Cooling Side;               !- Equipment 1 Name

      PlantEquipmentOperation:HeatingLoad,
        One AWHP Heating Operation Scheme, !- Name
        0.0,                               !- Load Range 1 Lower Limit {W}
        10000000000000000,                 !- Load Range 1 Upper Limit {W}
        One AWHP Heating Equipment List;   !- Range 1 Equipment List Name

      PlantEquipmentList,
        One AWHP Heating Equipment List,   !- Name
        HeatPump:PlantLoop:EIR:Heating,    !- Equipment 1 Object Type
        AWHP_1 Heating Side;               !- Equipment 1 Name

      ZoneList,
        This Zone List,                    !- Name
        Zone1,                             !- Zone Name 1
        Zone2,                             !- Zone Name 2
        Zone3,                             !- Zone Name 3
        Zone4;                             !- Zone Name 4
     )IDF";

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    auto &heatBranch1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1);
    auto &heatComp1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    heatComp1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    heatComp1.Name = "AWHP_1 Heating Side";
    heatBranch1.NodeNumIn = 1;
    heatBranch1.NodeNumOut = 2;
    heatComp1.NodeNumIn = 1;
    heatComp1.NodeNumOut = 2;
    state->dataPlnt->PlantLoop(2).TempSetPointNodeNum = 2;
    auto &coolBranch1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1);
    auto &coolComp1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    coolComp1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    coolComp1.Name = "AWHP_1 Cooling Side";
    coolBranch1.NodeNumIn = 5;
    coolBranch1.NodeNumOut = 6;
    coolComp1.NodeNumIn = 5;
    coolComp1.NodeNumOut = 6;
    state->dataPlnt->PlantLoop(1).TempSetPointNodeNum = 6;
    state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopReturnNodeNum(1) = 9;
    state->dataLoopNodes->Node.allocate(10);

    bool FirstHVACIteration = false;
    std::string CurrentModuleObject = "PlantEquipmentOperation:ChillerHeaterChangeover";
    PlantCondLoopOperation::InitLoadDistribution(*state, FirstHVACIteration);
    EXPECT_EQ(state->dataPlnt->PlantLoop(1).Name, "Cooling Plant");
    EXPECT_EQ(state->dataPlnt->PlantLoop(2).Name, "Heating Plant");

    auto &chillerHeaterSupervisor = state->dataPlantCondLoopOp->ChillerHeaterSupervisoryOperationSchemes(1);
    EXPECT_TRUE(chillerHeaterSupervisor.oneTimeSetupComplete); // getInput completed

    EXPECT_EQ(1, chillerHeaterSupervisor.PlantOps.NumOfAirLoops);
    EXPECT_EQ(0, chillerHeaterSupervisor.PlantOps.numPlantLoadProfiles);
    EXPECT_EQ(1, chillerHeaterSupervisor.PlantOps.NumHeatingOnlyEquipLists);
    EXPECT_EQ(1, chillerHeaterSupervisor.PlantOps.NumCoolingOnlyEquipLists);
    EXPECT_EQ(0, chillerHeaterSupervisor.PlantOps.NumSimultHeatCoolHeatingEquipLists);
    EXPECT_EQ(0, chillerHeaterSupervisor.PlantOps.NumSimultHeatCoolCoolingEquipLists);
    EXPECT_FALSE(chillerHeaterSupervisor.PlantOps.SimultHeatCoolOpAvailable);
    EXPECT_NEAR(6.60, chillerHeaterSupervisor.Setpoint.PrimCW, 0.001);      // cooling set point temperature
    EXPECT_NEAR(59.8, chillerHeaterSupervisor.Setpoint.PrimHW_High, 0.001); // heating set point temperature
    EXPECT_NEAR(37.6, chillerHeaterSupervisor.Setpoint.PrimHW_Low, 0.001);  // heating set point temperature
    EXPECT_NEAR(13.7, chillerHeaterSupervisor.Setpoint.SecCW, 0.001);       // cooling set point temperature
    EXPECT_NEAR(45.0, chillerHeaterSupervisor.Setpoint.SecHW, 0.001);       // cooling set point temperature
    EXPECT_NEAR(0.00, chillerHeaterSupervisor.TempReset.LowOutdoorTemp, 0.001);
    EXPECT_NEAR(10.0, chillerHeaterSupervisor.TempReset.HighOutdoorTemp, 0.001);
    EXPECT_EQ(1, chillerHeaterSupervisor.ZonePtrs(1));
    EXPECT_EQ(2, chillerHeaterSupervisor.ZonePtrs(2));
    EXPECT_EQ(3, chillerHeaterSupervisor.ZonePtrs(3));
    EXPECT_EQ(4, chillerHeaterSupervisor.ZonePtrs(4));

    FirstHVACIteration = true;
    auto &thisSupervisor = state->dataPlnt->PlantLoop(1).OpScheme(1).ChillerHeaterSupervisoryOperation;
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    // zone loads have not been initialized
    EXPECT_EQ(0, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // off
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);

    // check to see if correct plant equipment are active
    auto &eqheat = chillerHeaterSupervisor.HeatingOnlyEquipList(1).Comp(1);
    auto &eqcool = chillerHeaterSupervisor.CoolingOnlyEquipList(1).Comp(1);
    auto &CoolEq1_status = state->dataPlnt->PlantLoop(1).LoopSide(eqcool.LoopSideNumPtr).Branch(eqcool.BranchNumPtr).Comp(eqcool.CompNumPtr).ON;
    auto &HeatEq1_status = state->dataPlnt->PlantLoop(2).LoopSide(eqheat.LoopSideNumPtr).Branch(eqheat.BranchNumPtr).Comp(eqheat.CompNumPtr).ON;
    // set cooling demand node temp above cooling set point so equipment is activated if needed
    state->dataLoopNodes->Node(eqcool.DemandNodeNum).Temp = 10.0;
    auto &zone1SysEnergyDemand = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(1));
    auto &zone2SysEnergyDemand = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(2));
    auto &zone3SysEnergyDemand = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(3));
    auto &zone4SysEnergyDemand = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(chillerHeaterSupervisor.ZonePtrs(4));
    // set zone heating load
    zone1SysEnergyDemand.OutputRequiredToHeatingSP = 100.0;
    zone2SysEnergyDemand.OutputRequiredToHeatingSP = 200.0;
    zone3SysEnergyDemand.OutputRequiredToHeatingSP = 300.0;
    zone4SysEnergyDemand.OutputRequiredToHeatingSP = 400.0;
    // set fake HW plant flow rate
    state->dataLoopNodes->Node(heatBranch1.NodeNumIn).MassFlowRate = 0.00189204;

    // zones have a heating load and primary plant has heating load
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(1, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // heating
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(1000.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.001);
    EXPECT_NEAR(300.0, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.001);
    EXPECT_TRUE(chillerHeaterSupervisor.PlantOps.AirSourcePlantHeatingOnly);
    EXPECT_FALSE(chillerHeaterSupervisor.PlantOps.AirSourcePlantCoolingOnly);
    EXPECT_FALSE(CoolEq1_status); // cooling equipment is not active
    EXPECT_TRUE(HeatEq1_status);  // heating equipment is active

    // reset the building heating load to zeros
    zone1SysEnergyDemand.OutputRequiredToHeatingSP = 0.0;
    zone2SysEnergyDemand.OutputRequiredToHeatingSP = 0.0;
    zone3SysEnergyDemand.OutputRequiredToHeatingSP = 0.0;
    zone4SysEnergyDemand.OutputRequiredToHeatingSP = 0.0;
    // reset HW plant flow rate to zero
    state->dataLoopNodes->Node(heatBranch1.NodeNumIn).MassFlowRate = 0.0;
    // set the building cooling load to large negative numbers
    zone1SysEnergyDemand.OutputRequiredToCoolingSP = -100.0;
    zone2SysEnergyDemand.OutputRequiredToCoolingSP = -200.0;
    zone3SysEnergyDemand.OutputRequiredToCoolingSP = -300.0;
    zone4SysEnergyDemand.OutputRequiredToCoolingSP = -400.0;
    // set fake CW plant flow rate to very small value
    state->dataLoopNodes->Node(coolBranch1.NodeNumIn).MassFlowRate = 0.00002;

    // zone heating loads are 0 and zone cooling loads are negative (cooling)
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(-1000.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(0.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    EXPECT_NEAR(-0.28, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.01);
    EXPECT_NEAR(0.0, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.01);
    EXPECT_EQ(2, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // cooling plant on
    EXPECT_FALSE(chillerHeaterSupervisor.PlantOps.AirSourcePlantHeatingOnly);
    EXPECT_TRUE(chillerHeaterSupervisor.PlantOps.AirSourcePlantCoolingOnly);
    EXPECT_TRUE(CoolEq1_status);  // cooling equipment is active
    EXPECT_FALSE(HeatEq1_status); // heating equipment is not active

    // reset zone heating loads to > 0 but lower than the cooling loads
    zone1SysEnergyDemand.OutputRequiredToHeatingSP = 20.0;
    zone2SysEnergyDemand.OutputRequiredToHeatingSP = 40.0;
    zone3SysEnergyDemand.OutputRequiredToHeatingSP = 60.0;
    zone4SysEnergyDemand.OutputRequiredToHeatingSP = 80.0;
    // reset fake HW plant flow rate for a higher primary plant heating load
    state->dataLoopNodes->Node(heatBranch1.NodeNumIn).MassFlowRate = 0.0189204;
    // zone heating loads are positive (heating) and zone cooling loads are negative (cooling)
    // primary plant heating load is larger than the primary plant cooling load
    thisSupervisor->EvaluateChillerHeaterChangeoverOpScheme(*state);
    EXPECT_EQ(-1000.0, chillerHeaterSupervisor.Report.BuildingPolledCoolingLoad);
    EXPECT_EQ(200.0, chillerHeaterSupervisor.Report.BuildingPolledHeatingLoad);
    EXPECT_NEAR(-0.28, chillerHeaterSupervisor.Report.PrimaryPlantCoolingLoad, 0.01);
    EXPECT_NEAR(3000.0, chillerHeaterSupervisor.Report.PrimaryPlantHeatingLoad, 0.01);
    EXPECT_EQ(1, chillerHeaterSupervisor.Report.AirSourcePlant_OpMode); // heating plant on
    EXPECT_TRUE(chillerHeaterSupervisor.PlantOps.AirSourcePlantHeatingOnly);
    EXPECT_FALSE(chillerHeaterSupervisor.PlantOps.AirSourcePlantCoolingOnly);
    EXPECT_FALSE(CoolEq1_status); // cooling equipment is not active
    EXPECT_TRUE(HeatEq1_status);  // heating equipment is active
}
