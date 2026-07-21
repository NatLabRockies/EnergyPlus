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

// EnergyPlus::EvaporativeCoolers / ZoneHVAC:EvaporativeCoolerUnit Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::EvaporativeCoolers;

class ZoneHVACEvapCoolerUnitTest : public EnergyPlusFixture
{
public:
    int UnitNum = 1;
    int EvapCoolNum = 1;
    int NumOfNodes = 10;
    bool ErrorsFound = false;
    bool FirstHVACIteration = true;

protected:
    void SetUp() override
    {
        EnergyPlusFixture::SetUp(); // Sets up the base fixture first.

        state->dataSize->ZoneEqSizing.allocate(1);
        state->dataEnvrn->OutBaroPress = 101325.0;
        state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

        state->dataEnvrn->OutDryBulbTemp = 20.0;
        state->dataEnvrn->OutHumRat = 0.0075;
        state->dataEnvrn->OutWetBulbTemp =
            Psychrometrics::PsyTwbFnTdbWPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat, state->dataEnvrn->OutBaroPress);

        state->dataGlobal->NumOfZones = 1;
        state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
        state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);
        state->dataZoneEquip->ZoneEquipList.allocate(state->dataGlobal->NumOfZones);
        state->dataLoopNodes->Node.allocate(NumOfNodes);
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
        state->dataHeatBalFanSys->zoneTstatSetpts.allocate(1);

        state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "One Zone";
        state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
        state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
        state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 3;
        state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
        state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
        state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 4;
        state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
        state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
        state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 9;
        state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 10;
        state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;

        state->dataHeatBal->Zone(1).Name = state->dataZoneEquip->ZoneEquipConfig(1).ZoneName;
        state->dataHeatBal->Zone(1).Multiplier = 1.0;
        state->dataHeatBal->Zone(1).Volume = 1000.0;
        state->dataHeatBal->Zone(1).SystemZoneNodeNumber = state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode;
        state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;

        state->dataZoneEquip->ZoneEquipList(1).Name = "ZONEHVACEVAPEQUIPMENT";
        state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes = 1;
        state->dataZoneEquip->ZoneEquipList(1).EquipTypeName.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).EquipType.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).EquipName.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).EquipIndex.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).EquipIndex = 1;
        state->dataZoneEquip->ZoneEquipList(1).EquipData.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).CoolingPriority.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).HeatingPriority.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
        state->dataZoneEquip->ZoneEquipList(1).EquipTypeName(1) = "ZoneHVAC:EvaporativeCoolerUnit";
        state->dataZoneEquip->ZoneEquipList(1).CoolingPriority(1) = 1;
        state->dataZoneEquip->ZoneEquipList(1).HeatingPriority(1) = 1;
        state->dataZoneEquip->ZoneEquipList(1).EquipType(1) = DataZoneEquipment::ZoneEquipType::EvaporativeCooler;
    }

    void TearDown() override
    {
        EnergyPlusFixture::TearDown();
    }
};

TEST_F(ZoneHVACEvapCoolerUnitTest, DirectCelDekPad_CyclingUnit_Sim)
{
    int ActualZoneNum = 1;
    int ZoneEquipIndex = 1;
    Real64 SensOutputProvided(0.0);
    Real64 LatOutputProvided(0.0);

    std::string const idf_objects = delimited_string({

        " ZoneHVAC:EvaporativeCoolerUnit,",
        "   ZoneEvapCooler Unit,          !- Name",
        "   ,                             !- Availability Schedule Name",
        "   ,                             !- Availability Manager List Name",
        "   ZoneEvapCool OA Inlet,        !- Outdoor Air Inlet Node Name",
        "   ZoneEvapCool Inlet Node,      !- Cooler Outlet Node Name",
        "   ZoneEvapCool Relief Node,     !- Zone Relief Air Node Name",
        "   Fan:OnOff,                    !- Supply Air Fan Object Type",
        "   ZoneEvapCool Supply Fan,      !- Supply Air Fan Name",
        "   1.0,                          !- Design Supply Air Flow Rate {m3/s}",
        "   BlowThrough,                  !- Fan Placement",
        "   ZoneTemperatureDeadbandOnOffCycling,  !- Cooler Unit Control Method",
        "   1.0,                          !- Throttling Range Temperature Difference {deltaC}",
        "   100.0,                        !- Cooling Load Control Threshold Heat Transfer Rate {W}",
        "   EvaporativeCooler:Direct:CelDekPad,  !- First Evaporative Cooler Object Type",
        "   Direct CelDekPad EvapCooler;  !- First Evaporative Cooler Object Name",

        " Fan:OnOff,",
        "    ZoneEvapCool Supply Fan,     !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.7,                         !- Fan Total Efficiency",
        "    300.0,                       !- Pressure Rise {Pa}",
        "    1.0,                         !- Maximum Flow Rate {m3/s}",
        "    0.9,                         !- Motor Efficiency",
        "    1.0,                         !- Motor In Airstream Fraction",
        "    ZoneEvapCool OA Inlet,       !- Air Inlet Node Name",
        "    ZoneEvapCool Fan outlet;     !- Air Outlet Node Name",

        " EvaporativeCooler:Direct:CelDekPad,",
        "    Direct CelDekPad EvapCooler, !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.6,                         !- Direct Pad Area {m2}",
        "    0.17,                        !- Direct Pad Depth {m}",
        "    55,                          !- Recirculating Water Pump Power Consumption {W}",
        "    ZoneEvapCool Fan outlet,     !- Air Inlet Node Name",
        "    ZoneEvapCool Inlet Node,     !- Air Outlet Node Name",
        "    ;                            !- Control Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Fans::GetFanInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetEvapInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetInputZoneEvaporativeCoolerUnit(*state);
    ASSERT_FALSE(ErrorsFound);

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;

    auto &thisZoneEvapCooler(state->dataEvapCoolers->ZoneEvapUnit(UnitNum));

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = thisZoneEvapCooler.UnitReliefNodeNum;

    state->dataLoopNodes->Node.redimension(NumOfNodes);
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0080;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);

    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 23.0;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    state->dataZoneEquip->ZoneEquipList(1).EquipName(1) = thisZoneEvapCooler.Name;

    // Evap Cooler Unit Control Method = Zone Temperature Dead Band OnOff Cycling
    EXPECT_EQ((int)thisZoneEvapCooler.fanOp, (int)HVAC::FanOp::Cycling);
    EXPECT_ENUM_EQ(thisZoneEvapCooler.ControlSchemeType, ControlType::ZoneTemperatureDeadBandOnOffCycling);
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    Real64 FullSensibleOutput = 0.0;
    Real64 FullLatentOutput = 0.0;
    Real64 PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);
    EXPECT_NEAR(FullSensibleOutput, SensOutputProvided, 0.01);
    EXPECT_NEAR(FullSensibleOutput, -thisZoneEvapCooler.UnitSensibleCoolingRate, 0.01);
    // test the unit at half of the full flow capacity
    Real64 HalfOfFullLoad = 0.50 * FullSensibleOutput;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = HalfOfFullLoad;
    // calculate part load ratio for cycling fan
    EvaporativeCoolers::ControlZoneEvapUnitOutput(*state, UnitNum, HalfOfFullLoad);
    EXPECT_NEAR(0.4747010, thisZoneEvapCooler.UnitPartLoadRatio, 0.000001);
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, thisZoneEvapCooler.UnitPartLoadRatio, SensOutputProvided, LatOutputProvided);
    EXPECT_NEAR(HalfOfFullLoad, SensOutputProvided, 0.01);

    // Evap Cooler Unit Control Method = Zone Cooling Load OnOff Cycling
    thisZoneEvapCooler.ControlSchemeType = ControlType::ZoneCoolingLoadOnOffCycling;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);
    EXPECT_NEAR(FullSensibleOutput, SensOutputProvided, 0.01);
    EXPECT_NEAR(FullSensibleOutput, -thisZoneEvapCooler.UnitSensibleCoolingRate, 0.01);
    // test the unit at half of the full flow capacity
    HalfOfFullLoad = 0.50 * FullSensibleOutput;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = HalfOfFullLoad;
    // calculate part load ratio for cycling fan
    EvaporativeCoolers::ControlZoneEvapUnitOutput(*state, UnitNum, HalfOfFullLoad);
    EXPECT_NEAR(0.4747010, thisZoneEvapCooler.UnitPartLoadRatio, 0.000001);
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, thisZoneEvapCooler.UnitPartLoadRatio, SensOutputProvided, LatOutputProvided);
    EXPECT_NEAR(HalfOfFullLoad, SensOutputProvided, 0.01);
}

TEST_F(ZoneHVACEvapCoolerUnitTest, DirectResearchSpecial_CyclingUnit_Sim)
{
    int ActualZoneNum = 1;
    int ZoneEquipIndex = 1;
    Real64 SensOutputProvided(0.0);
    Real64 LatOutputProvided(0.0);

    std::string const idf_objects = delimited_string({

        " ZoneHVAC:EvaporativeCoolerUnit,",
        "   ZoneEvapCooler Unit,          !- Name",
        "   ,                             !- Availability Schedule Name",
        "   ,                             !- Availability Manager List Name",
        "   ZoneEvapCool OA Inlet,        !- Outdoor Air Inlet Node Name",
        "   ZoneEvapCool Inlet Node,      !- Cooler Outlet Node Name",
        "   ZoneEvapCool Relief Node,     !- Zone Relief Air Node Name",
        "   Fan:OnOff,                    !- Supply Air Fan Object Type",
        "   ZoneEvapCool Supply Fan,      !- Supply Air Fan Name",
        "   1.0,                          !- Design Supply Air Flow Rate {m3/s}",
        "   BlowThrough,                  !- Fan Placement",
        "   ZoneTemperatureDeadbandOnOffCycling,  !- Cooler Unit Control Method",
        "   1.0,                          !- Throttling Range Temperature Difference {deltaC}",
        "   100.0,                        !- Cooling Load Control Threshold Heat Transfer Rate {W}",
        "   EvaporativeCooler:Direct:ResearchSpecial,  !- First Evaporative Cooler Object Type",
        "   Direct ResearchSpecial EvapCooler;  !- First Evaporative Cooler Object Name",

        " Fan:OnOff,",
        "    ZoneEvapCool Supply Fan,     !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.7,                         !- Fan Total Efficiency",
        "    300.0,                       !- Pressure Rise {Pa}",
        "    1.0,                         !- Maximum Flow Rate {m3/s}",
        "    0.9,                         !- Motor Efficiency",
        "    1.0,                         !- Motor In Airstream Fraction",
        "    ZoneEvapCool OA Inlet,       !- Air Inlet Node Name",
        "    ZoneEvapCool Fan outlet;     !- Air Outlet Node Name",

        " EvaporativeCooler:Direct:ResearchSpecial,",
        "   Direct ResearchSpecial EvapCooler,  !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.7,                         !- Cooler Design Effectiveness",
        "    ,                            !- Effectiveness Flow Ratio Modifier Curve Name",
        "    1.0,                         !- Primary Air Design Flow Rate {m3/s}",
        "    55,                          !- Recirculating Water Pump Power Consumption {W}",
        "    ,                            !- Water Pump Power Sizing Factor {W/(m3/s)}",
        "    ,                            !- Water Pump Power Modifier Curve Name",
        "    ZoneEvapCool Fan outlet,     !- Air Inlet Node Name",
        "    ZoneEvapCool Inlet Node,     !- Air Outlet Node Name",
        "    ZoneEvapCool Inlet Node;     !- Sensor Node Name",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Fans::GetFanInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetEvapInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetInputZoneEvaporativeCoolerUnit(*state);
    ASSERT_FALSE(ErrorsFound);

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;

    auto &thisZoneEvapCooler(state->dataEvapCoolers->ZoneEvapUnit(UnitNum));

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = thisZoneEvapCooler.UnitReliefNodeNum;

    state->dataLoopNodes->Node.redimension(NumOfNodes);
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0080;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);

    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 23.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    state->dataZoneEquip->ZoneEquipList(1).EquipName(1) = thisZoneEvapCooler.Name;

    // Evap Cooler Unit Control Method = Zone Temperature Dead Band OnOff Cycling
    EXPECT_ENUM_EQ(thisZoneEvapCooler.fanOp, HVAC::FanOp::Cycling);
    EXPECT_ENUM_EQ(thisZoneEvapCooler.ControlSchemeType, ControlType::ZoneTemperatureDeadBandOnOffCycling);
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    Real64 FullSensibleOutput = 0.0;
    Real64 FullLatentOutput = 0.0;
    Real64 PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);
    EXPECT_NEAR(FullSensibleOutput, SensOutputProvided, 0.01);
    EXPECT_NEAR(FullSensibleOutput, -thisZoneEvapCooler.UnitSensibleCoolingRate, 0.01);
    // test the unit at half of the full flow capacity
    Real64 HalfOfFullLoad = 0.50 * FullSensibleOutput;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = HalfOfFullLoad;
    // calculate part load ratio for cycling fan
    EvaporativeCoolers::ControlZoneEvapUnitOutput(*state, UnitNum, HalfOfFullLoad);
    EXPECT_NEAR(0.500000, thisZoneEvapCooler.UnitPartLoadRatio, 0.000001);
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, thisZoneEvapCooler.UnitPartLoadRatio, SensOutputProvided, LatOutputProvided);
    EXPECT_NEAR(HalfOfFullLoad, SensOutputProvided, 0.01);

    // Evap Cooler Unit Control Method = Zone Cooling Load OnOff Cycling
    thisZoneEvapCooler.ControlSchemeType = ControlType::ZoneCoolingLoadOnOffCycling;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);
    EXPECT_NEAR(FullSensibleOutput, SensOutputProvided, 0.01);
    EXPECT_NEAR(FullSensibleOutput, -thisZoneEvapCooler.UnitSensibleCoolingRate, 0.01);
    // test the unit at half of the full flow capacity
    HalfOfFullLoad = 0.50 * FullSensibleOutput;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = HalfOfFullLoad;
    // calculate part load ratio for cycling fan
    EvaporativeCoolers::ControlZoneEvapUnitOutput(*state, UnitNum, HalfOfFullLoad);
    EXPECT_NEAR(0.500000, thisZoneEvapCooler.UnitPartLoadRatio, 0.000001);
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, thisZoneEvapCooler.UnitPartLoadRatio, SensOutputProvided, LatOutputProvided);
    EXPECT_NEAR(HalfOfFullLoad, SensOutputProvided, 0.01);
}

TEST_F(ZoneHVACEvapCoolerUnitTest, IndirectWetCoil_CyclingUnit_Sim)
{
    int ActualZoneNum = 1;
    int ZoneEquipIndex = 1;
    Real64 SensOutputProvided(0.0);
    Real64 LatOutputProvided(0.0);

    std::string const idf_objects = delimited_string({

        " ZoneHVAC:EvaporativeCoolerUnit,",
        "   ZoneEvapCooler Unit,          !- Name",
        "   ,                             !- Availability Schedule Name",
        "   ,                             !- Availability Manager List Name",
        "   ZoneEvapCool OA Inlet,        !- Outdoor Air Inlet Node Name",
        "   ZoneEvapCool Inlet Node,      !- Cooler Outlet Node Name",
        "   ZoneEvapCool Relief Node,     !- Zone Relief Air Node Name",
        "   Fan:OnOff,                    !- Supply Air Fan Object Type",
        "   ZoneEvapCool Supply Fan,      !- Supply Air Fan Name",
        "   1.0,                          !- Design Supply Air Flow Rate {m3/s}",
        "   BlowThrough,                  !- Fan Placement",
        "   ZoneTemperatureDeadbandOnOffCycling,  !- Cooler Unit Control Method",
        "   1.0,                          !- Throttling Range Temperature Difference {deltaC}",
        "   100.0,                        !- Cooling Load Control Threshold Heat Transfer Rate {W}",
        "   EvaporativeCooler:Indirect:WetCoil,  !- First Evaporative Cooler Object Type",
        "   Indirect WetCoil EvapCooler;  !- First Evaporative Cooler Object Name",

        " Fan:OnOff,",
        "    ZoneEvapCool Supply Fan,     !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.7,                         !- Fan Total Efficiency",
        "    300.0,                       !- Pressure Rise {Pa}",
        "    1.0,                         !- Maximum Flow Rate {m3/s}",
        "    0.9,                         !- Motor Efficiency",
        "    1.0,                         !- Motor In Airstream Fraction",
        "    ZoneEvapCool OA Inlet,       !- Air Inlet Node Name",
        "    ZoneEvapCool Fan outlet;     !- Air Outlet Node Name",

        " EvaporativeCooler:Indirect:WetCoil,",
        "   Indirect WetCoil EvapCooler,  !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.7,                         !- Coil Maximum Efficiency",
        "    ,                            !- Coil Flow Ratio",
        "    55,                          !- Recirculating Water Pump Power Consumption {W}",
        "    1.0,                         !- Secondary Air Fan Flow Rate {m3/s}",
        "    0.7,                         !- Secondary Air Fan Total Efficiency",
        "    300,                         !- Secondary Air Fan Delta Pressure {Pa}",
        "    ZoneEvapCool Fan outlet,     !- Primary Air Inlet Node Name",
        "    ZoneEvapCool Inlet Node,     !- Primary Air Outlet Node Name",
        "    ,                            !- Control Type",
        "    ,                            !- Water Supply Storage Tank Name",
        "    Secondary OA inlet node;     !- Secondary Air Inlet Node Name",

        "    OutdoorAir:Node,",
        "    Secondary OA inlet node;     !- Name",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Fans::GetFanInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetEvapInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetInputZoneEvaporativeCoolerUnit(*state);
    ASSERT_FALSE(ErrorsFound);

    OutAirNodeManager::SetOutAirNodes(*state);

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;

    auto &thisZoneEvapCooler(state->dataEvapCoolers->ZoneEvapUnit(UnitNum));
    auto &thisEvapCooler(state->dataEvapCoolers->EvapCond(EvapCoolNum));

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = thisZoneEvapCooler.UnitReliefNodeNum;

    state->dataLoopNodes->Node.redimension(NumOfNodes);
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0080;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);

    state->dataLoopNodes->Node(thisEvapCooler.SecondaryInletNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisEvapCooler.SecondaryInletNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisEvapCooler.SecondaryInletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);

    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 23.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    state->dataZoneEquip->ZoneEquipList(1).EquipName(1) = thisZoneEvapCooler.Name;

    // Evap Cooler Unit Control Method = Zone Temperature Dead Band OnOff Cycling
    EXPECT_EQ((int)thisZoneEvapCooler.fanOp, (int)HVAC::FanOp::Cycling);
    EXPECT_ENUM_EQ(thisZoneEvapCooler.ControlSchemeType, ControlType::ZoneTemperatureDeadBandOnOffCycling);
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    Real64 FullSensibleOutput = 0.0;
    Real64 FullLatentOutput = 0.0;
    Real64 PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);
    EXPECT_NEAR(FullSensibleOutput, SensOutputProvided, 0.01);
    EXPECT_NEAR(FullSensibleOutput, -thisZoneEvapCooler.UnitSensibleCoolingRate, 0.01);
    // test the unit at half of the full flow capacity
    Real64 HalfOfFullLoad = 0.50 * FullSensibleOutput;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = HalfOfFullLoad;
    // calculate part load ratio for cycling fan
    EvaporativeCoolers::ControlZoneEvapUnitOutput(*state, UnitNum, HalfOfFullLoad);
    EXPECT_NEAR(0.500000, thisZoneEvapCooler.UnitPartLoadRatio, 0.000001);
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, thisZoneEvapCooler.UnitPartLoadRatio, SensOutputProvided, LatOutputProvided);
    EXPECT_NEAR(HalfOfFullLoad, SensOutputProvided, 0.01);

    // Evap Cooler Unit Control Method = Zone Cooling Load OnOff Cycling
    thisZoneEvapCooler.ControlSchemeType = ControlType::ZoneCoolingLoadOnOffCycling;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);
    EXPECT_NEAR(FullSensibleOutput, SensOutputProvided, 0.01);
    EXPECT_NEAR(FullSensibleOutput, -thisZoneEvapCooler.UnitSensibleCoolingRate, 0.01);
    // test the unit at half of the full flow capacity
    HalfOfFullLoad = 0.50 * FullSensibleOutput;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = HalfOfFullLoad;
    // calculate part load ratio for cycling fan
    EvaporativeCoolers::ControlZoneEvapUnitOutput(*state, UnitNum, HalfOfFullLoad);
    EXPECT_NEAR(0.500000, thisZoneEvapCooler.UnitPartLoadRatio, 0.000001);
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, thisZoneEvapCooler.UnitPartLoadRatio, SensOutputProvided, LatOutputProvided);
    EXPECT_NEAR(HalfOfFullLoad, SensOutputProvided, 0.01);
}

TEST_F(ZoneHVACEvapCoolerUnitTest, RHcontrol)
{

    int ActualZoneNum = 1;
    int ZoneEquipIndex = 1;
    Real64 SensOutputProvided(0.0);
    Real64 LatOutputProvided(0.0);

    std::string const idf_objects = delimited_string({
        " ZoneHVAC:EvaporativeCoolerUnit,",
        "   ZoneEvapCooler Unit,          !- Name",
        "   ,                             !- Availability Schedule Name",
        "   ,                             !- Availability Manager List Name",
        "   ZoneEvapCool OA Inlet,        !- Outdoor Air Inlet Node Name",
        "   ZoneEvapCool Inlet Node,      !- Cooler Outlet Node Name",
        "   ZoneEvapCool Relief Node,     !- Zone Relief Air Node Name",
        "   Fan:OnOff,                    !- Supply Air Fan Object Type",
        "   ZoneEvapCool Supply Fan,      !- Supply Air Fan Name",
        "   1.0,                          !- Design Supply Air Flow Rate {m3/s}",
        "   BlowThrough,                  !- Fan Placement",
        "   ZoneTemperatureDeadbandOnOffCycling,  !- Cooler Unit Control Method",
        "   1.0,                          !- Throttling Range Temperature Difference {deltaC}",
        "   100.0,                        !- Cooling Load Control Threshold Heat Transfer Rate {W}",
        "   EvaporativeCooler:Direct:CelDekPad,  !- First Evaporative Cooler Object Type",
        "   Direct CelDekPad EvapCooler,  !- First Evaporative Cooler Object Name",
        "   ,",
        "   ,",
        "   ,",
        "   40;                           !- Shut Off Relative Humidity",

        " Fan:OnOff,",
        "    ZoneEvapCool Supply Fan,     !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.7,                         !- Fan Total Efficiency",
        "    300.0,                       !- Pressure Rise {Pa}",
        "    1.0,                         !- Maximum Flow Rate {m3/s}",
        "    0.9,                         !- Motor Efficiency",
        "    1.0,                         !- Motor In Airstream Fraction",
        "    ZoneEvapCool OA Inlet,       !- Air Inlet Node Name",
        "    ZoneEvapCool Fan outlet;     !- Air Outlet Node Name",

        " EvaporativeCooler:Direct:CelDekPad,",
        "    Direct CelDekPad EvapCooler, !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.6,                         !- Direct Pad Area {m2}",
        "    0.17,                        !- Direct Pad Depth {m}",
        "    55,                          !- Recirculating Water Pump Power Consumption {W}",
        "    ZoneEvapCool Fan outlet,     !- Air Inlet Node Name",
        "    ZoneEvapCool Inlet Node,     !- Air Outlet Node Name",
        "    ;                            !- Control Type",

        "    OutdoorAir:Node,",
        "    Secondary OA inlet node;     !- Name",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Fans::GetFanInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetEvapInput(*state);
    ASSERT_FALSE(ErrorsFound);
    EvaporativeCoolers::GetInputZoneEvaporativeCoolerUnit(*state);
    ASSERT_FALSE(ErrorsFound);

    OutAirNodeManager::SetOutAirNodes(*state);

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;

    auto &thisZoneEvapCooler(state->dataEvapCoolers->ZoneEvapUnit(UnitNum));

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = thisZoneEvapCooler.UnitReliefNodeNum;

    state->dataLoopNodes->Node.redimension(NumOfNodes);
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0080;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisZoneEvapCooler.OAInletNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);

    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 23.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -15000.0;
    state->dataZoneEquip->ZoneEquipList(1).EquipName(1) = thisZoneEvapCooler.Name;

    // Evap Cooler Unit Control Method = Zone Temperature Dead Band OnOff Cycling
    EvaporativeCoolers::SimZoneEvaporativeCoolerUnit(
        *state, thisZoneEvapCooler.Name, ActualZoneNum, SensOutputProvided, LatOutputProvided, ZoneEquipIndex);
    Real64 FullSensibleOutput = 0.0;
    Real64 FullLatentOutput = 0.0;
    Real64 PartLoadRatio = 1.0;
    EvaporativeCoolers::CalcZoneEvapUnitOutput(*state, UnitNum, PartLoadRatio, FullSensibleOutput, FullLatentOutput);

    Real64 relativeHumidity =
        100.0 * Psychrometrics::PsyRhFnTdbWPb(*state,
                                              state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                              state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat,
                                              state->dataEnvrn->OutBaroPress,
                                              "CalcZoneEvaporativeCoolerUnit");
    //    when relative humidity is higher than the threshold, the evaporative cooler is off
    EXPECT_EQ(thisZoneEvapCooler.ShutOffRelativeHumidity, 40);
    ASSERT_TRUE(relativeHumidity > thisZoneEvapCooler.ShutOffRelativeHumidity);
    EXPECT_FALSE(thisZoneEvapCooler.IsOnThisTimestep);
}
