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

// EnergyPlus::PurchasedAirManager (Ideal Loads) Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::PurchasedAirManager;
using namespace EnergyPlus::ZoneEquipmentManager;
using namespace EnergyPlus::ZonePlenum;

class ZoneIdealLoadsTest : public EnergyPlusFixture
{
public:
    int IdealLoadsSysNum = 1;
    int NumNodes = 1; // number of zone inlet and zone exhaust nodes
    bool ErrorsFound = false;

protected:
    virtual void SetUp()
    {
        EnergyPlusFixture::SetUp(); // Sets up the base fixture first.

        state->dataHeatBalFanSys->zoneTstatSetpts.allocate(1);
        state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 23.9; // 75F
        state->dataHeatBalFanSys->zoneTstatSetpts(1).setptLo = 23.0; // 73.4F

        state->dataSize->FinalZoneSizing.allocate(1);
        state->dataSize->ZoneEqSizing.allocate(1);
        state->dataSize->CurZoneEqNum = 1;
        state->dataSize->CurSysNum = 0;
        state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
        state->dataSize->ZoneSizingRunDone = true;

        state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).TotalOutputRequired = 1000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToHeatingSP = 1000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToCoolingSP = 2000.0;
        state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
        state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
        state->dataHeatBal->MassConservation.allocate(1);
        state->dataHeatBal->ZoneIntGain.allocate(1);
        state->dataHeatBal->spaceIntGain.allocate(1);
        state->dataHeatBal->spaceIntGainDevices.allocate(1);
        state->dataSurface->SurfaceWindow.allocate(1);
        state->dataHeatBal->RefrigCaseCredit.allocate(1);

        state->dataHeatBalFanSys->TempControlType.allocate(1);
        state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::SingleHeat;
        state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
        state->dataZoneEnergyDemand->DeadBandOrSetback.allocate(1);
        state->dataZoneEnergyDemand->DeadBandOrSetback(1) = false;

        state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.07;

        state->dataZoneEquip->ZoneEquipInputsFilled = false;
    }

    virtual void TearDown()
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(EnergyPlusFixture, SizePurchasedAirTest_Test1)
{

    int PurchAirNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero in Sizer
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(24);
    state->dataSize->CurSysNum = 0;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA = 0.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).OutTempAtHeatPeak = 5.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 80.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesHumRat = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 60.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).OutTempAtCoolPeak = 70.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 50.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.010;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataPurchasedAirMgr->PurchAir.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames.allocate(8);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(5) = "Maximum Heating Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(6) = "Maximum Sensible Heating Capacity";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(7) = "Maximum Cooling Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(8) = "Maximum Total Cooling Capacity";

    state->dataSize->ZoneSizingRunDone = true;

    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).HeatingLimit = LimitType::FlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).CoolingLimit = LimitType::FlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).cObjectName = "ZONEHVAC:IDEALLOADSAIRSYSTEM";
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).Name = "Ideal Loads 1";

    SizePurchasedAir(*state, PurchAirNum);
    EXPECT_DOUBLE_EQ(1.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate);
    EXPECT_NEAR(50985.58, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap, 0.1);
    EXPECT_DOUBLE_EQ(2.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate);
    EXPECT_NEAR(30844.14, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap, 0.1);
}

TEST_F(EnergyPlusFixture, SizePurchasedAirTest_Test2)
{

    int PurchAirNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero in Sizer
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(24);
    state->dataSize->CurSysNum = 0;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).OutTempAtHeatPeak = 5.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 30.0; // this isn't used so don't change it
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 80.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesHumRat = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 65.0; // this is used, so make it higher
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).OutTempAtCoolPeak = 70.0; // this is not currently used for cooling
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 50.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.010;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataPurchasedAirMgr->PurchAir.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames.allocate(8);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(5) = "Maximum Heating Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(6) = "Maximum Sensible Heating Capacity";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(7) = "Maximum Cooling Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(8) = "Maximum Total Cooling Capacity";

    state->dataSize->ZoneSizingRunDone = true;

    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).HeatingLimit = LimitType::FlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).CoolingLimit = LimitType::FlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).cObjectName = "ZONEHVAC:IDEALLOADSAIRSYSTEM";
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).Name = "Ideal Loads 1";

    SizePurchasedAir(*state, PurchAirNum);
    EXPECT_DOUBLE_EQ(1.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate);
    EXPECT_NEAR(63731.97, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap, 0.1); // larger than test 1 above
    EXPECT_DOUBLE_EQ(2.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate);
    EXPECT_NEAR(41078.43, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap, 0.1); // larger than test1 above
}

TEST_F(EnergyPlusFixture, IdealLoadsAirSystem_GetInput)
{
    std::string const idf_objects = delimited_string({
        "ZoneHVAC:IdealLoadsAirSystem,",
        "ZONE 1 Ideal Loads, !- Name",
        ", !- Availability Schedule Name",
        "ZONE 1 INLETS, !- Zone Supply Air Node Name",
        ", !- Zone Exhaust Air Node Name",
        ", !- System Inlet Air Node Name",
        "50, !- Maximum Heating Supply Air Temperature{ C }",
        "13, !- Minimum Cooling Supply Air Temperature{ C }",
        "0.015, !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "0.009, !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "NoLimit, !- Heating Limit",
        "autosize, !- Maximum Heating Air Flow Rate{ m3 / s }",
        ", !- Maximum Sensible Heating Capacity{ W }",
        "NoLimit, !- Cooling Limit",
        "autosize, !- Maximum Cooling Air Flow Rate{ m3 / s }",
        ", !- Maximum Total Cooling Capacity{ W }",
        ", !- Heating Availability Schedule Name",
        ", !- Cooling Availability Schedule Name",
        "ConstantSupplyHumidityRatio, !- Dehumidification Control Type",
        ", !- Cooling Sensible Heat Ratio{ dimensionless }",
        "ConstantSupplyHumidityRatio, !- Humidification Control Type",
        ", !- Design Specification Outdoor Air Object Name",
        ", !- Outdoor Air Inlet Node Name",
        ", !- Demand Controlled Ventilation Type",
        ", !- Outdoor Air Economizer Type",
        ", !- Heat Recovery Type",
        ", !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "; !- Latent Heat Recovery Effectiveness{ dimensionless }",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    GetPurchasedAir(*state);

    auto &PurchAir(state->dataPurchasedAirMgr->PurchAir);
    EXPECT_EQ(PurchAir.size(), 1u);
    EXPECT_EQ(PurchAir(1).Name, "ZONE 1 IDEAL LOADS");
    EXPECT_EQ(PurchAir(1).MaxHeatSuppAirTemp, 50.0);
    EXPECT_EQ(PurchAir(1).MinCoolSuppAirTemp, 13.0);
    EXPECT_EQ(PurchAir(1).MaxHeatSuppAirHumRat, 0.015);
    EXPECT_EQ(PurchAir(1).MinCoolSuppAirHumRat, 0.009);
    EXPECT_ENUM_EQ(PurchAir(1).HeatingLimit, LimitType::None);
    EXPECT_ENUM_EQ(PurchAir(1).CoolingLimit, LimitType::None);
    EXPECT_ENUM_EQ(PurchAir(1).DehumidCtrlType, HumControl::ConstantSupplyHumidityRatio);
    EXPECT_ENUM_EQ(PurchAir(1).HumidCtrlType, HumControl::ConstantSupplyHumidityRatio);
    // confirm a constant fuel efficiency defalt value of 1.0
    EXPECT_EQ(PurchAir(1).heatFuelEffSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_EQ(PurchAir(1).coolFuelEffSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_EQ(PurchAir(1).heatFuelEffSched->getCurrentVal(), 1.0);
    EXPECT_EQ(PurchAir(1).coolFuelEffSched->getCurrentVal(), 1.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_PlenumTest)
{

    std::string const idf_objects = delimited_string({

        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "Zone,",
        "  PLENUM ZONE,                    !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  Plenum Outlet Node,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "AirLoopHVAC:ReturnPlenum,",
        "  DOAS Zone Return Plenum,        !- Name",
        "  PLENUM ZONE,                    !- Zone Name",
        "  Plenum Node,                    !- Zone Node Name", // illegal use of non-unique zone node name
        "  Plenum Outlet Node,             !- Outlet Node Name",
        "  ,                               !- Induced Air Outlet Node or NodeList Name",
        "  Zone Exhaust Node;              !- Inlet 1 Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);

    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero

    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system
    auto &PurchAir(state->dataPurchasedAirMgr->PurchAir);
    EXPECT_EQ(PurchAir(1).Name, "ZONE 1 IDEAL LOADS");
    // Ideal loads air system found the plenum it is attached to
    EXPECT_EQ(PurchAir(1).ReturnPlenumIndex, 1);
    // The ideal loads air system inlet air node is equal to the zone return plenum outlet node
    EXPECT_EQ(PurchAir(1).PlenumExhaustAirNodeNum, state->dataZonePlenum->ZoneRetPlenCond(1).OutletNode);
    // The ideal loads air system ZoneSupplyAirNodeNum is equal to the zone air inlet node
    EXPECT_EQ(PurchAir(1).ZoneSupplyAirNodeNum, state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1));
    // The ideal loads air system ZoneExhaustAirNodeNum is equal to the zone exhaust air node num
    EXPECT_EQ(PurchAir(1).ZoneExhaustAirNodeNum, state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1));
    // The zone exhaust air node is equal to the zone return plenum inlet air node
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1), state->dataZonePlenum->ZoneRetPlenCond(1).InletNode(1));
    // The ideal loads air system has a non-zero mass flow rate
    EXPECT_GT(PurchAir(1).SupplyAirMassFlowRate, 0.0);
    // The ideal loads air system mass flow rate is equal to all nodes attached to this system
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, state->dataLoopNodes->Node(PurchAir(1).ZoneSupplyAirNodeNum).MassFlowRate);
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, state->dataLoopNodes->Node(PurchAir(1).ZoneExhaustAirNodeNum).MassFlowRate);
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, state->dataLoopNodes->Node(PurchAir(1).PlenumExhaustAirNodeNum).MassFlowRate);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_ExhaustNodeTest)
{

    std::string const idf_objects = delimited_string({

        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ,                               !- Latent Heat Recovery Effectiveness{ dimensionless }",
        "  ,                               !- Design Specification ZoneHVAC Sizing Object Name }",
        "  DXHeatingCoilFuelEffSched,      !- Heating Fuel Efficiency Schedule Name",
        "  Electricity,                    !- Heating Fuel Type",
        "  DXCoolingCoilFuelEffSched,      !- Cooling Fuel Efficiency Schedule Name",
        "  Electricity;                    !- Cooling Fuel Type",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "AirLoopHVAC:ReturnPlenum,",
        "  DOAS Zone Return Plenum,        !- Name",
        "  PLENUM ZONE,                    !- Zone Name",
        "  Plenum Node,                    !- Zone Node Name", // illegal use of non-unique zone node name
        "  Plenum Outlet Node,             !- Outlet Node Name",
        "  ,                               !- Induced Air Outlet Node or NodeList Name",
        "  Zone Exhaust Node;              !- Inlet 1 Node Name",

        "  Schedule:Constant,",
        "    DXHeatingCoilFuelEffSched,    !- Name",
        "    AnyValue,                     !- Schedule Type Limits Name",
        "    2.0;                          !- Field 1",

        "  Schedule:Constant,",
        "    DXCoolingCoilFuelEffSched,    !- Name",
        "    AnyValue,                     !- Schedule Type Limits Name",
        "    3.0;                          !- Field 1",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);

    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system

    auto &PurchAir = state->dataPurchasedAirMgr->PurchAir(1);
    EXPECT_EQ(PurchAir.Name, "ZONE 1 IDEAL LOADS");
    // Ideal loads air system found the plenum it is attached to
    EXPECT_EQ(PurchAir.SupplyAirMassFlowRate, state->dataLoopNodes->Node(PurchAir.ZoneSupplyAirNodeNum).MassFlowRate);
    EXPECT_EQ(PurchAir.SupplyAirMassFlowRate, state->dataLoopNodes->Node(PurchAir.ZoneExhaustAirNodeNum).MassFlowRate);
    EXPECT_ENUM_EQ(PurchAir.heatingFuelType, Constant::eFuel::Electricity);
    EXPECT_EQ(PurchAir.heatFuelEffSched->getCurrentVal(), 2.0);
    EXPECT_ENUM_EQ(PurchAir.coolingFuelType, Constant::eFuel::Electricity);
    EXPECT_EQ(PurchAir.coolFuelEffSched->getCurrentVal(), 3.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_IntermediateOutputVarsTest)
{

    std::string const idf_objects = delimited_string({

        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,                               !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  Office Outdoor Air Spec,        !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  NoEconomizer,                   !- Outdoor Air Economizer Type",
        "  Sensible,                       !- Heat Recovery Type",
        "  0.7,                            !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  0.65;                           !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "DesignSpecification:OutdoorAir,",
        "  Office Outdoor Air Spec,        !- Name",
        "  Flow/Zone,                      !- Outdoor Air Method",
        "  0.0,                            !- Outdoor Air Flow per Person {m3/s-person}",
        "  0.00305,                        !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "  0.0,                            !- Outdoor Air Flow per Zone {m3/s}",
        "  0.0,                            !- Outdoor Air Flow Air Changes per Hour {1/hr}",
        "  Min OA Sched;                   !- Outdoor Air Schedule Name",

        "Schedule:Compact,",
        "  Min OA Sched,                   !- Name",
        "  Fraction,                       !- Schedule Type Limits Name",
        "  Through: 12/31,                 !- Field 1",
        "  For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "  Until: 8:00,0.0,                !- Field 3",
        "  Until: 21:00,1.0,               !- Field 5",
        "  Until: 24:00,0.0,               !- Field 7",
        "  For: Weekends Holiday,          !- Field 9",
        "  Until: 24:00,0.0,               !- Field 10",
        "  For: SummerDesignDay,           !- Field 12",
        "  Until: 24:00,1.0,               !- Field 13",
        "  For: WinterDesignDay,           !- Field 15",
        "  Until: 24:00,1.0;               !- Field 16",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "AirLoopHVAC:ReturnPlenum,",
        "  DOAS Zone Return Plenum,        !- Name",
        "  PLENUM ZONE,                    !- Zone Name",
        "  Plenum Node,                    !- Zone Node Name", // illegal use of non-unique zone node name
        "  Plenum Outlet Node,             !- Outlet Node Name",
        "  ,                               !- Induced Air Outlet Node or NodeList Name",
        "  Zone Exhaust Node;              !- Inlet 1 Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    auto &PurchAir(state->dataPurchasedAirMgr->PurchAir);

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);

    EnergyPlus::SizingManager::GetOARequirements(*state);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system

    EXPECT_EQ(PurchAir(1).Name, "ZONE 1 IDEAL LOADS");
    // Expecting SupplyTemp to be the same as Zone supply temp
    EXPECT_EQ(PurchAir(1).SupplyTemp, state->dataLoopNodes->Node(PurchAir(1).ZoneSupplyAirNodeNum).Temp);
    EXPECT_EQ(PurchAir(1).SupplyHumRat, state->dataLoopNodes->Node(PurchAir(1).ZoneSupplyAirNodeNum).HumRat);

    // Test for intermediate variables, MixedAirTemp, MixedAirHumRat
    state->dataLoopNodes->Node(PurchAir(1).ZoneRecircAirNodeNum).Temp = 24;
    state->dataLoopNodes->Node(PurchAir(1).ZoneRecircAirNodeNum).HumRat = 0.00929;
    state->dataLoopNodes->Node(PurchAir(1).ZoneRecircAirNodeNum).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(PurchAir(1).ZoneRecircAirNodeNum).Temp, state->dataLoopNodes->Node(PurchAir(1).ZoneRecircAirNodeNum).HumRat);
    state->dataLoopNodes->Node(PurchAir(1).OutdoorAirNodeNum).Temp = 3;
    state->dataLoopNodes->Node(PurchAir(1).OutdoorAirNodeNum).HumRat = 0.004586;
    state->dataLoopNodes->Node(PurchAir(1).OutdoorAirNodeNum).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(PurchAir(1).OutdoorAirNodeNum).Temp, state->dataLoopNodes->Node(PurchAir(1).OutdoorAirNodeNum).HumRat);
    PurchAir(1).MixedAirTemp = 0;
    PurchAir(1).MixedAirHumRat = 0;
    Real64 MixedAirEnthalpy = 0;
    Real64 OAMassFlowRate = 10;
    Real64 SupplyMassFlowRate = 11;
    CalcPurchAirMixedAir(*state,
                         1,                          // index to ideal loads unit
                         OAMassFlowRate,             // outside air mass flow rate [kg/s]
                         SupplyMassFlowRate,         // supply air mass flow rate [kg/s]
                         PurchAir(1).MixedAirTemp,   // Mixed air dry bulb temperature [C]
                         PurchAir(1).MixedAirHumRat, // Mixed air humidity ratio [kgWater/kgDryAir]
                         MixedAirEnthalpy,           // Mixed air enthalpy [J/kg]
                         OpMode::Cool                // current operating mode, Off, Heating, Cooling, or DeadBand
    );
    // Calculations:
    // Stream 1: Recirc stream:         T1: 24 C; W1: 0.00929 kg/kg; h1: 47764.36 J/kg; m_dot1: 11 kg/s
    // Stream 2: Outdoor Air stream:    T2:  3 C; W2:0.004586 kg/kg; h2: 14509.40 J/kg; m_dot2: 10 kg/s

    // Mixed stream:
    // When SupplyMassFlowRate > OAMassFlowRate
    // RecircMassFlowRate = SupplyMassFlowRate - OAMassFlowRate = 1 kg/s
    // h_mix_stream = (RecircMassFlowRate X h_Recirc + m_dotOA X h_OA)/SupplyFlowRate = 17532.58 J/kg
    // W_mix_stream = (RecircMassFlowRate X W_Recirc + m_dotOA X W_OA)/SupplyFlowRate = 0.005013 kg/kg
    // T_mix_stream = T as fn (h_mix_stream,W_mix_stream) = 4.924 C

    EXPECT_EQ(PurchAir(1).MixedAirTemp, 4.9240554165264818);
    EXPECT_EQ(PurchAir(1).MixedAirHumRat, 0.0050136363636363633);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_EMSOverrideTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "  Output:EnergyManagementSystem,                                                                ",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting                      ",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting             ",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level                         ",

        "EnergyManagementSystem:Actuator,",
        "Mdot,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Mass Flow Rate;",

        "EnergyManagementSystem:Actuator,",
        "Tsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air TEMPERATURE;",

        "EnergyManagementSystem:Sensor,",
        "ZoneAirTemp,",
        "EAST ZONE,",
        "Zone Mean Air Temperature;",

        "EnergyManagementSystem:OutputVariable,",
        "MassstromIdealLoad_EMS, ! - Name",
        "Mdot, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:OutputVariable,",
        "SupplyTempIdealLoad_EMS, ! - Name",
        "Tsupply, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set Mdot = 0.1;",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 2;
    state->dataGlobal->TimeStepZone = 0.25;

    EMSManager::CheckIfAnyEMS(*state); // get EMS input
    EMSManager::GetEMSInput(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;

    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideMdotOn = true;
    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideSupplyTempOn = true;
    state->dataLoopNodes->Node(2).Temp = 25.0;
    state->dataLoopNodes->Node(2).HumRat = 0.001;

    InitPurchasedAir(*state, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1);

    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueMassFlowRate, 0.0);
    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueSupplyTemp, 0.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_NoCapacityTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,                               !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  LimitCapacity,                  !- Heating Limit",
        "  ,                               !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  0,                              !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  ,                               !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system

    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    InitPurchasedAir(*state, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1);

    EXPECT_EQ(SysOutputProvided, 0.0);
    // #8365 Supply air mass flow rate should be zero during heating mode when capacity is limited to zero
    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).SupplyAirMassFlowRate, 0.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_EMSOverrideTest_Revised)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "  Output:EnergyManagementSystem,                                                                ",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting                      ",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting             ",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level                         ",

        "EnergyManagementSystem:Actuator,",
        "Mdot,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Mass Flow Rate;",

        "EnergyManagementSystem:Actuator,",
        "Tsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air TEMPERATURE;",

        "EnergyManagementSystem:Actuator,",
        "HRsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Humidity Ratio;",

        "EnergyManagementSystem:Sensor,",
        "ZoneAirTemp,",
        "EAST ZONE,",
        "Zone Mean Air Temperature;",

        "EnergyManagementSystem:OutputVariable,",
        "MassstromIdealLoad_EMS, ! - Name",
        "Mdot, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:OutputVariable,",
        "SupplyTempIdealLoad_EMS, ! - Name",
        "Tsupply, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set Mdot = 0.1,",
        "set Tsupply = 18,",
        "set HRsupply = 0.010;",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 2;
    state->dataGlobal->TimeStepZone = 0.25;

    EMSManager::CheckIfAnyEMS(*state); // get EMS input

    state->dataEMSMgr->FinishProcessingUserInput = true;

    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideMdotOn = true;
    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideSupplyTempOn = true;
    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideSupplyHumRatOn = true;

    state->dataLoopNodes->Node(2).Temp = 25.0;
    state->dataLoopNodes->Node(2).HumRat = 0.001;

    InitPurchasedAir(*state, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    bool anyEMSRan;
    ManageEMS(*state, EMSManager::EMSCallFrom::HVACIterationLoop, anyEMSRan, ObjexxFCL::Optional_int_const());

    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;
    state->dataPurchasedAirMgr->PurchAir(1).OutdoorAirNodeNum = 2;
    state->dataPurchasedAirMgr->PurchAir(1).ZoneRecircAirNodeNum = 1;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1);

    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueSupplyTemp, 18.0);
    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueSupplyHumRat, 0.01);

    EXPECT_EQ(state->dataLoopNodes->Node(1).Enthalpy, 43431.131);
    EXPECT_EQ(state->dataLoopNodes->Node(1).HumRat, 0.01);
    EXPECT_EQ(state->dataLoopNodes->Node(1).Temp, 18.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_EMSOverrideTest_Revised_ZeroFlow)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "  Output:EnergyManagementSystem,                                                                ",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting                      ",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting             ",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level                         ",

        "EnergyManagementSystem:Actuator,",
        "Mdot,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Mass Flow Rate;",

        "EnergyManagementSystem:Actuator,",
        "Tsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air TEMPERATURE;",

        "EnergyManagementSystem:Actuator,",
        "HRsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Humidity Ratio;",

        "EnergyManagementSystem:Sensor,",
        "ZoneAirTemp,",
        "EAST ZONE,",
        "Zone Mean Air Temperature;",

        "EnergyManagementSystem:OutputVariable,",
        "MassstromIdealLoad_EMS, ! - Name",
        "Mdot, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:OutputVariable,",
        "SupplyTempIdealLoad_EMS, ! - Name",
        "Tsupply, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set Mdot = 0.0,",
        "set Tsupply = 18,",
        "set HRsupply = 0.010;",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 2;
    state->dataGlobal->TimeStepZone = 0.25;

    EMSManager::CheckIfAnyEMS(*state); // get EMS input

    state->dataEMSMgr->FinishProcessingUserInput = true;

    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideMdotOn = true;
    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideSupplyTempOn = true;
    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideSupplyHumRatOn = true;

    state->dataLoopNodes->Node(2).Temp = 25.0;
    state->dataLoopNodes->Node(2).HumRat = 0.001;
    state->dataEnvrn->StdRhoAir = 1.2;

    InitPurchasedAir(*state, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    bool anyEMSRan;
    ManageEMS(*state, EMSManager::EMSCallFrom::HVACIterationLoop, anyEMSRan, ObjexxFCL::Optional_int_const());

    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;
    state->dataPurchasedAirMgr->PurchAir(1).OutdoorAirNodeNum = 2;
    state->dataPurchasedAirMgr->PurchAir(1).ZoneRecircAirNodeNum = 1;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1);

    EXPECT_EQ(SysOutputProvided, 0.0);
    EXPECT_EQ(MoistOutputProvided, 0.0);

    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueSupplyTemp, 18.0);
    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueSupplyHumRat, 0.01);

    EXPECT_EQ(state->dataLoopNodes->Node(1).Enthalpy, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(1).HumRat, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(1).Temp, 0.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_Fix_SA_HumRat_Test)
{
    // Test PR 9723 which fixes issue 9686
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,                               !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  Humidistat,                     !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ,                               !- Latent Heat Recovery Effectiveness{ dimensionless }",
        "  ,                               !- Design Specification ZoneHVAC Sizing Object Name }",
        "  DXHeatingCoilFuelEffSched,      !- Heating Fuel Efficiency Schedule Name }",
        "  Electricity,                    !- Heating Fuel Type",
        "  DXCoolingCoilFuelEffSched,      !- Cooling Fuel Efficiency Schedule Name }",
        "  Electricity;                    !- Cooling Fuel Type",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "  Output:EnergyManagementSystem,                                                                ",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting                      ",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting             ",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level                         ",

        "EnergyManagementSystem:Actuator,",
        "Mdot,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Mass Flow Rate;",

        "EnergyManagementSystem:Actuator,",
        "Tsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air TEMPERATURE;",

        "EnergyManagementSystem:Actuator,",
        "HRsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Humidity Ratio;",

        "EnergyManagementSystem:Sensor,",
        "ZoneAirTemp,",
        "EAST ZONE,",
        "Zone Mean Air Temperature;",

        "EnergyManagementSystem:OutputVariable,",
        "MassstromIdealLoad_EMS, ! - Name",
        "Mdot, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:OutputVariable,",
        "SupplyTempIdealLoad_EMS, ! - Name",
        "Tsupply, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set Mdot = 0.1;",

        "Schedule:Constant,",
        "  DXHeatingCoilFuelEffSched,    !- Name",
        "  AnyValue,                     !- Schedule Type Limits Name",
        "  1.0;                          !- Field 1",

        "Schedule:Constant,",
        "  DXCoolingCoilFuelEffSched,    !- Name",
        "  AnyValue,                     !- Schedule Type Limits Name",
        "  1.0;                          !- Field 1",

        "EnergyManagementSystem:Actuator,",
        "  HeatingFuelEff_SCH_Override,  !- Name",
        "  DXHeatingCoilFuelEffSched,    !- Actuated Component Unique Name",
        "  Schedule:Constant,            !- Actuated Component Type",
        "  Schedule Value;               !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "  CoolingFuelEff_SCH_Override,  !- Name",
        "  DXCoolingCoilFuelEffSched,    !- Actuated Component Unique Name",
        "  Schedule:Constant,            !- Actuated Component Type",
        "  Schedule Value;               !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "  IdealLoads_Fuel_Eff_Reset_ProgMgr, !- Name",
        "  InsideHVACSystemIterationLoop,     !- EnergyPlus Model Calling Point",
        "  IdealLoadsAirSystemFuelEffProg;    !- Program Name 2",

        "EnergyManagementSystem:Program,",
        "  IdealLoadsAirSystemFuelEffProg,    !- Name",
        "  Set HeatingFuelEff_SCH_Override = 2.0,  !- Program Line 1",
        "  Set CoolingFuelEff_SCH_Override = 3.0;  !- Program Line 2",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 2;
    state->dataGlobal->TimeStepZone = 0.25;

    EMSManager::CheckIfAnyEMS(*state); // get EMS input

    state->dataEMSMgr->FinishProcessingUserInput = true;

    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    auto &PurchAir = state->dataPurchasedAirMgr->PurchAir(1);
    PurchAir.EMSOverrideMdotOn = true;
    PurchAir.EMSOverrideSupplyTempOn = false;
    PurchAir.EMSOverrideSupplyHumRatOn = false;

    state->dataLoopNodes->Node(2).Temp = 25.0;
    state->dataLoopNodes->Node(2).HumRat = 0.001;

    InitPurchasedAir(*state, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    bool anyEMSRan;
    ManageEMS(*state, EMSManager::EMSCallFrom::HVACIterationLoop, anyEMSRan, ObjexxFCL::Optional_int_const());

    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;
    PurchAir.OutdoorAirNodeNum = 2;
    PurchAir.ZoneRecircAirNodeNum = 1;

    int ControlledZoneNum = 1;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP = -1000.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToDehumidSP = -0.0002;
    state->dataHeatBalFanSys->TempControlType(ControlledZoneNum) = HVAC::SetptType::SingleCool;

    state->dataLoopNodes->Node(1).Temp = 30;
    state->dataLoopNodes->Node(1).HumRat = 0.012;
    state->dataEnvrn->StdRhoAir = 1.2;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1);

    EXPECT_DOUBLE_EQ(PurchAir.MinCoolSuppAirHumRat, 0.009);

    EXPECT_DOUBLE_EQ(PurchAir.SupplyTemp, 20.228931255157292);
    // Without the current fix, this SupplyHumRat value would be 0.009, which is incorrect:
    EXPECT_DOUBLE_EQ(PurchAir.SupplyHumRat, 0.01);

    EXPECT_DOUBLE_EQ(PurchAir.MixedAirTemp, 30.0);
    EXPECT_DOUBLE_EQ(PurchAir.MixedAirHumRat, 0.012);

    EXPECT_DOUBLE_EQ(PurchAir.SenCoilLoad, -1003.6327856486452);
    EXPECT_DOUBLE_EQ(PurchAir.LatCoilLoad, 5574.8612856486452);

    EXPECT_DOUBLE_EQ(PurchAir.SenOutputToZone, -1000.0000000000002);
    EXPECT_DOUBLE_EQ(PurchAir.LatOutputToZone, 5571.2285000000002);

    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(1).Enthalpy, 45712.285000000003);
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(1).HumRat, 0.01);
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(1).Temp, 20.228931255157292);

    ReportPurchasedAir(*state, 1);
    EXPECT_ENUM_EQ(PurchAir.heatingFuelType, Constant::eFuel::Electricity);
    EXPECT_EQ(PurchAir.heatFuelEffSched->getCurrentVal(), 2.0);
    EXPECT_ENUM_EQ(PurchAir.heatingFuelType, Constant::eFuel::Electricity);
    EXPECT_EQ(PurchAir.coolFuelEffSched->getCurrentVal(), 3.0);
    EXPECT_DOUBLE_EQ(PurchAir.TotCoolRate, 1003.6327856486452);
    EXPECT_DOUBLE_EQ(PurchAir.TotCoolFuelRate, 1003.6327856486452 / 3.0);
    EXPECT_DOUBLE_EQ(PurchAir.TotHeatRate, 5574.8612856486452);
    EXPECT_DOUBLE_EQ(PurchAir.TotHeatFuelRate, 5574.8612856486452 / 2.0);
    EXPECT_DOUBLE_EQ(PurchAir.ZoneTotCoolRate, 1000.0000000000002);
    EXPECT_DOUBLE_EQ(PurchAir.ZoneTotCoolFuelRate, 1000.0000000000002 / 3.0);
    EXPECT_DOUBLE_EQ(PurchAir.ZoneTotHeatRate, 5571.2285000000002);
    EXPECT_DOUBLE_EQ(PurchAir.ZoneTotHeatFuelRate, 5571.2285000000002 / 2.0);
}
