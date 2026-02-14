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

#include <fstream>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACManager.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace HVACManager;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::HeatBalanceAirManager;
using namespace EnergyPlus::ZoneEquipmentManager;

TEST_F(EnergyPlusFixture, CrossMixingReportTest)
{

    // Test for #5007
    state->dataGlobal->NumOfZones = 2;
    int NumOfCrossMixing = 1;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->CrossMixing.allocate(NumOfCrossMixing);
    state->dataHeatBal->ZnAirRpt.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);

    state->dataGlobal->NumOfZones = state->dataGlobal->NumOfZones;
    state->dataHeatBal->TotCrossMixing = NumOfCrossMixing;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPI = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPI = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPV = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPV = 0.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 22.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MAT = 25.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRat = 0.0011;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRatAvg = 0.0011;
    state->dataEnvrn->StdRhoAir = 1.20;

    state->dataHeatBal->CrossMixing(1).ZonePtr = 1;
    state->dataHeatBal->CrossMixing(1).FromZone = 2;
    state->dataHeatBal->CrossMixing(1).DesiredAirFlowRate = 0.1;
    state->dataHeatBal->CrossMixing(1).ReportFlag = true;
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 0;

    // Call HVACManager
    ReportAirHeatBalance(*state);

    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixVolume, state->dataHeatBal->ZnAirRpt(2).MixVolume, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixVdotCurDensity, state->dataHeatBal->ZnAirRpt(2).MixVdotCurDensity, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixVdotStdDensity, state->dataHeatBal->ZnAirRpt(2).MixVdotStdDensity, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixMass, state->dataHeatBal->ZnAirRpt(2).MixMass, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixMdot, state->dataHeatBal->ZnAirRpt(2).MixMdot, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixHeatLoss, state->dataHeatBal->ZnAirRpt(2).MixHeatGain, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixHeatGain, state->dataHeatBal->ZnAirRpt(2).MixHeatLoss, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixLatentLoss, state->dataHeatBal->ZnAirRpt(2).MixLatentGain, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixLatentGain, state->dataHeatBal->ZnAirRpt(2).MixLatentLoss, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixTotalLoss, state->dataHeatBal->ZnAirRpt(2).MixTotalGain, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->ZnAirRpt(1).MixTotalGain, state->dataHeatBal->ZnAirRpt(2).MixTotalLoss, 0.0001);
}

TEST_F(EnergyPlusFixture, InfiltrationObjectLevelReport)
{

    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Zone1,                   !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    100.0;                   !- Volume {m3}",
        "  Zone,",
        "    Zone2,                   !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    200.0;                   !- Volume {m3}",
        "  Zone,",
        "    Zone3,                   !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    300.0;                   !- Volume {m3}",
        "  Zone,",
        "    Zone4,                   !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    400.0;                   !- Volume {m3}",

        "ZoneList,",
        "  ZoneList,",
        "  Zone1,",
        "  Zone2;",

        "ZoneInfiltration:EffectiveLeakageArea,",
        "  Zone3 Infil,          !- Name",
        "  Zone3,                       !- Zone or ZoneList Name",
        "  AlwaysOn,                    !- Schedule Name",
        "  500.0,                       !- Effective Air Leakage Area",
        "  0.000145,                    !- Stack Coefficient",
        "  0.000174;                    !- Wind Coefficient",

        "ZoneInfiltration:FlowCoefficient,",
        "  Zone4 Infil,          !- Name",
        "  Zone4,                       !- Zone or ZoneList Name",
        "  AlwaysOn,                    !- Schedule Name",
        "  0.05,                        !- Flow Coefficient",
        "  0.089,                       !- Stack Coefficient",
        "  0.67,                        !- Pressure Exponent",
        "  0.156,                       !- Wind Coefficient",
        "  0.64;                        !- Shelter Factor",

        "ZoneInfiltration:DesignFlowRate,",
        "  Zonelist Infil,          !- Name",
        "  ZoneList,                       !- Zone or ZoneList Name",
        "  AlwaysOn,                    !- Schedule Name",
        "  flow/zone,                   !- Design Flow Rate Calculation Method",
        "  0.07,                        !- Design Flow Rate{ m3 / s }",
        "  ,                            !- Flow per Zone Floor Area{ m3 / s - m2 }",
        "  ,                            !- Flow per Exterior Surface Area{ m3 / s - m2 }",
        "  ,                            !- Air Changes per Hour{ 1 / hr }",
        "  1,                           !- Constant Term Coefficient",
        "  0,                           !- Temperature Term Coefficient",
        "  0,                           !- Velocity Term Coefficient",
        "  0;                           !- Velocity Squared Term Coefficient",

        "ZoneInfiltration:DesignFlowRate,",
        "  Zone2 Infil,                 !- Name",
        "  Zone2,                       !- Zone or ZoneList Name",
        "  AlwaysOn,                    !- Schedule Name",
        "  flow/zone,                   !- Design Flow Rate Calculation Method",
        "  0.07,                        !- Design Flow Rate{ m3 / s }",
        "  ,                            !- Flow per Zone Floor Area{ m3 / s - m2 }",
        "  ,                            !- Flow per Exterior Surface Area{ m3 / s - m2 }",
        "  ,                            !- Air Changes per Hour{ 1 / hr }",
        "  1,                           !- Constant Term Coefficient",
        "  0,                           !- Temperature Term Coefficient",
        "  0,                           !- Velocity Term Coefficient",
        "  0,                           !- Velocity Squared Term Coefficient",
        "  Standard;                    !- Density Basis",

        "Schedule:Constant,",
        "AlwaysOn,",
        "Fraction,",
        "1.0;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());
    state->init_state(*state);

    bool ErrorsFound(false);
    GetZoneData(*state, ErrorsFound);
    state->dataHeatBal->space(1).Volume = state->dataHeatBal->Zone(1).Volume;
    state->dataHeatBal->space(2).Volume = state->dataHeatBal->Zone(2).Volume;
    state->dataHeatBal->space(3).Volume = state->dataHeatBal->Zone(3).Volume;
    state->dataHeatBal->space(4).Volume = state->dataHeatBal->Zone(4).Volume;
    AllocateHeatBalArrays(*state);
    GetSimpleAirModelInputs(*state, ErrorsFound);

    EXPECT_EQ(state->dataHeatBal->TotInfiltration, 5); // one per zone

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);

    auto &zoneHB = state->dataZoneTempPredictorCorrector->zoneHeatBalance;
    zoneHB(1).MAT = 21.0;
    zoneHB(2).MAT = 22.0;
    zoneHB(3).MAT = 23.0;
    zoneHB(4).MAT = 24.0;
    zoneHB(1).airHumRat = 0.001;
    zoneHB(2).airHumRat = 0.001;
    zoneHB(3).airHumRat = 0.001;
    zoneHB(4).airHumRat = 0.001;

    auto &spaceHB = state->dataZoneTempPredictorCorrector->spaceHeatBalance;
    spaceHB(1).MAT = 21.0;
    spaceHB(2).MAT = 22.0;
    spaceHB(3).MAT = 23.0;
    spaceHB(4).MAT = 24.0;
    spaceHB(1).airHumRat = 0.001;
    spaceHB(2).airHumRat = 0.001;
    spaceHB(3).airHumRat = 0.001;
    spaceHB(4).airHumRat = 0.001;

    state->dataHeatBal->AirFlowFlag = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat = 0.0005;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataEnvrn->WindSpeed = 1.0;
    state->dataHeatBal->Zone(1).WindSpeed = 1.0;
    state->dataHeatBal->Zone(2).WindSpeed = 1.0;
    state->dataHeatBal->Zone(3).WindSpeed = 1.0;
    state->dataHeatBal->Zone(4).WindSpeed = 1.0;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = 15.0;
    state->dataHeatBal->Zone(2).OutDryBulbTemp = 15.0;
    state->dataHeatBal->Zone(3).OutDryBulbTemp = 15.0;
    state->dataHeatBal->Zone(4).OutDryBulbTemp = 15.0;
    Sched::GetSchedule(*state, "ALWAYSON")->currentVal = 1.0;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->TimeStepZoneSec = 3600;

    CalcAirFlowSimple(*state, 2);

    auto &infiltration(state->dataHeatBal->Infiltration);
    EXPECT_NEAR(infiltration(1).MCpI_temp, 0.07 * 1.2242 * 1005.77, 0.01); // zone level reporting matches object level
    EXPECT_NEAR(infiltration(2).MCpI_temp, 0.07 * 1.2242 * 1005.77, 0.01); // zone level reporting matches object level
    EXPECT_NEAR(infiltration(3).MCpI_temp, 0.07 * state->dataEnvrn->StdRhoAir * 1005.77,
                0.01);                                    // zone level reporting matches object level
    EXPECT_NEAR(infiltration(4).MCpI_temp, 22.486, 0.01); // zone level reporting matches object level
    EXPECT_NEAR(infiltration(5).MCpI_temp, 24.459, 0.01); // zone level reporting matches object level

    EXPECT_EQ(state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPI,
              infiltration(1).MCpI_temp); // zone level reporting matches object level
    Real64 zone2MCPIExpected = infiltration(2).MCpI_temp + infiltration(3).MCpI_temp;
    EXPECT_EQ(state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPI,
              zone2MCPIExpected); // zone level reporting matches object level
    EXPECT_EQ(state->dataZoneTempPredictorCorrector->zoneHeatBalance(3).MCPI,
              infiltration(4).MCpI_temp); // zone level reporting matches object level
    EXPECT_EQ(state->dataZoneTempPredictorCorrector->zoneHeatBalance(4).MCPI,
              infiltration(5).MCpI_temp); // zone level reporting matches object level

    ReportAirHeatBalance(*state);

    auto &ZnAirRpt(state->dataHeatBal->ZnAirRpt);
    EXPECT_NEAR(ZnAirRpt(1).InfilHeatLoss, infiltration(1).InfilHeatLoss, 0.000001); // zone level reporting matches object level
    Real64 expectedValue = infiltration(2).InfilHeatLoss + infiltration(3).InfilHeatLoss;
    EXPECT_NEAR(ZnAirRpt(2).InfilHeatLoss, expectedValue, 0.000001);                 // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilHeatLoss, infiltration(4).InfilHeatLoss, 0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilHeatLoss, infiltration(5).InfilHeatLoss, 0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilHeatGain, infiltration(1).InfilHeatGain, 0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilHeatGain + infiltration(3).InfilHeatGain;
    EXPECT_NEAR(ZnAirRpt(2).InfilHeatGain, expectedValue, 0.000001);                 // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilHeatGain, infiltration(4).InfilHeatGain, 0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilHeatGain, infiltration(5).InfilHeatGain, 0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilTotalLoss, infiltration(1).InfilTotalLoss,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilTotalLoss + infiltration(3).InfilTotalLoss;
    EXPECT_NEAR(ZnAirRpt(2).InfilTotalLoss, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilTotalLoss, infiltration(4).InfilTotalLoss,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilTotalLoss, infiltration(5).InfilTotalLoss,
                0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilTotalGain, infiltration(1).InfilTotalGain,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilTotalGain + infiltration(3).InfilTotalGain;
    EXPECT_NEAR(ZnAirRpt(2).InfilTotalGain, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilTotalGain, infiltration(4).InfilTotalGain,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilTotalGain, infiltration(5).InfilTotalGain,
                0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilMass, infiltration(1).InfilMass, 0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilMass + infiltration(3).InfilMass;
    EXPECT_NEAR(ZnAirRpt(2).InfilMass, expectedValue,
                0.000001);                                                   // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilMass, infiltration(4).InfilMass, 0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilMass, infiltration(5).InfilMass, 0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilMdot, infiltration(1).InfilMdot, 0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilMdot + infiltration(3).InfilMdot;
    EXPECT_NEAR(ZnAirRpt(2).InfilMdot, expectedValue,
                0.000001);                                                   // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilMdot, infiltration(4).InfilMdot, 0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilMdot, infiltration(5).InfilMdot, 0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilVolumeCurDensity, infiltration(1).InfilVolumeCurDensity,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilVolumeCurDensity + infiltration(3).InfilVolumeCurDensity;
    EXPECT_NEAR(ZnAirRpt(2).InfilVolumeCurDensity, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilVolumeCurDensity, infiltration(4).InfilVolumeCurDensity,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilVolumeCurDensity, infiltration(5).InfilVolumeCurDensity,
                0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilAirChangeRateCurDensity,
                infiltration(1).InfilAirChangeRateCurDensity,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilAirChangeRateCurDensity + infiltration(3).InfilAirChangeRateCurDensity;
    EXPECT_NEAR(ZnAirRpt(2).InfilAirChangeRateCurDensity, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilAirChangeRateCurDensity,
                infiltration(4).InfilAirChangeRateCurDensity,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilAirChangeRateCurDensity,
                infiltration(5).InfilAirChangeRateCurDensity,
                0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilVdotCurDensity, infiltration(1).InfilVdotCurDensity,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilVdotCurDensity + infiltration(3).InfilVdotCurDensity;
    EXPECT_NEAR(ZnAirRpt(2).InfilVdotCurDensity, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilVdotCurDensity, infiltration(4).InfilVdotCurDensity,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilVdotCurDensity, infiltration(5).InfilVdotCurDensity,
                0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilVolumeStdDensity, infiltration(1).InfilVolumeStdDensity,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilVolumeStdDensity + infiltration(3).InfilVolumeStdDensity;
    EXPECT_NEAR(ZnAirRpt(2).InfilVolumeStdDensity, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilVolumeStdDensity, infiltration(4).InfilVolumeStdDensity,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilVolumeStdDensity, infiltration(5).InfilVolumeStdDensity,
                0.000001); // zone level reporting matches object level

    EXPECT_NEAR(ZnAirRpt(1).InfilVdotStdDensity, infiltration(1).InfilVdotStdDensity,
                0.000001); // zone level reporting matches object level
    expectedValue = infiltration(2).InfilVdotStdDensity + infiltration(3).InfilVdotStdDensity;
    EXPECT_NEAR(ZnAirRpt(2).InfilVdotStdDensity, expectedValue,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(3).InfilVdotStdDensity, infiltration(4).InfilVdotStdDensity,
                0.000001); // zone level reporting matches object level
    EXPECT_NEAR(ZnAirRpt(4).InfilVdotStdDensity, infiltration(5).InfilVdotStdDensity,
                0.000001); // zone level reporting matches object level

    // test ZoneList reporting
    state->dataHeatBal->ZoneListSNLoadHeatEnergy.allocate(1);
    state->dataHeatBal->ZoneListSNLoadCoolEnergy.allocate(1);
    state->dataHeatBal->ZoneListSNLoadHeatRate.allocate(1);
    state->dataHeatBal->ZoneListSNLoadCoolRate.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(2);
    UpdateZoneListAndGroupLoads(*state);
    EXPECT_EQ(1.0, state->dataHeatBal->Zone(1).Multiplier);
    EXPECT_EQ(1.0, state->dataHeatBal->Zone(2).Multiplier);
    EXPECT_EQ(1, state->dataHeatBal->ZoneList(1).Zone(1));
    EXPECT_EQ(2, state->dataHeatBal->ZoneList(1).Zone(2));
    EXPECT_EQ(0.0, state->dataHeatBal->ZoneListSNLoadHeatEnergy(1));
    EXPECT_EQ(0.0, state->dataHeatBal->ZoneListSNLoadCoolEnergy(1));
    EXPECT_EQ(0.0, state->dataHeatBal->ZoneListSNLoadHeatRate(1));
    EXPECT_EQ(0.0, state->dataHeatBal->ZoneListSNLoadCoolRate(1));

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).airSysCoolRate = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).airSysCoolEnergy = 200.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).airSysHeatRate = 150.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).airSysHeatEnergy = 300.0;

    UpdateZoneListAndGroupLoads(*state);
    EXPECT_EQ(1.0, state->dataHeatBal->Zone(1).Multiplier);
    EXPECT_EQ(1.0, state->dataHeatBal->Zone(2).Multiplier);
    EXPECT_EQ(1, state->dataHeatBal->ZoneList(1).Zone(1));
    EXPECT_EQ(2, state->dataHeatBal->ZoneList(1).Zone(2));
    EXPECT_EQ(100.0, state->dataHeatBal->ZoneListSNLoadCoolRate(1));   // 1 * 100 + 1 * 0
    EXPECT_EQ(200.0, state->dataHeatBal->ZoneListSNLoadCoolEnergy(1)); // 1 * 200 + 1 * 0
    EXPECT_EQ(150.0, state->dataHeatBal->ZoneListSNLoadHeatRate(1));   // 1 * 150 + 1 * 0
    EXPECT_EQ(300.0, state->dataHeatBal->ZoneListSNLoadHeatEnergy(1)); // 1 * 300 + 1 * 0
    EXPECT_EQ(0.0, state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysCoolRate);
    EXPECT_EQ(0.0, state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysCoolEnergy);
    EXPECT_EQ(0.0, state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysHeatRate);
    EXPECT_EQ(0.0, state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysHeatEnergy);

    state->dataHeatBal->Zone(1).Multiplier = 2;
    UpdateZoneListAndGroupLoads(*state);
    EXPECT_EQ(200.0, state->dataHeatBal->ZoneListSNLoadCoolRate(1));   // 2 * 100 + 1 * 0
    EXPECT_EQ(400.0, state->dataHeatBal->ZoneListSNLoadCoolEnergy(1)); // 2 * 200 + 1 * 0
    EXPECT_EQ(300.0, state->dataHeatBal->ZoneListSNLoadHeatRate(1));   // 2 * 150 + 1 * 0
    EXPECT_EQ(600.0, state->dataHeatBal->ZoneListSNLoadHeatEnergy(1)); // 2 * 300 + 1 * 0

    // switch the zone index order in the ZoneList
    state->dataHeatBal->ZoneList(1).Zone(1) = 2;
    state->dataHeatBal->ZoneList(1).Zone(2) = 1;
    UpdateZoneListAndGroupLoads(*state);
    EXPECT_EQ(200.0, state->dataHeatBal->ZoneListSNLoadCoolRate(1));   // 2 * 100 + 1 * 0
    EXPECT_EQ(400.0, state->dataHeatBal->ZoneListSNLoadCoolEnergy(1)); // 2 * 200 + 1 * 0
    EXPECT_EQ(300.0, state->dataHeatBal->ZoneListSNLoadHeatRate(1));   // 2 * 150 + 1 * 0
    EXPECT_EQ(600.0, state->dataHeatBal->ZoneListSNLoadHeatEnergy(1)); // 2 * 300 + 1 * 0

    // zone 2 has a multiplier of 1
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysCoolRate = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysCoolEnergy = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysHeatRate = 100.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).airSysHeatEnergy = 100.0;

    UpdateZoneListAndGroupLoads(*state);
    EXPECT_EQ(300.0, state->dataHeatBal->ZoneListSNLoadCoolRate(1));   // 2 * 100 + 1 * 100
    EXPECT_EQ(500.0, state->dataHeatBal->ZoneListSNLoadCoolEnergy(1)); // 2 * 200 + 1 * 100
    EXPECT_EQ(400.0, state->dataHeatBal->ZoneListSNLoadHeatRate(1));   // 2 * 150 + 1 * 100
    EXPECT_EQ(700.0, state->dataHeatBal->ZoneListSNLoadHeatEnergy(1)); // 2 * 300 + 1 * 100

    // switch zone multipliers
    state->dataHeatBal->Zone(1).Multiplier = 1;
    state->dataHeatBal->Zone(2).Multiplier = 2;
    UpdateZoneListAndGroupLoads(*state);
    EXPECT_EQ(300.0, state->dataHeatBal->ZoneListSNLoadCoolRate(1));   // 1 * 100 + 2 * 100
    EXPECT_EQ(400.0, state->dataHeatBal->ZoneListSNLoadCoolEnergy(1)); // 1 * 200 + 2 * 100
    EXPECT_EQ(350.0, state->dataHeatBal->ZoneListSNLoadHeatRate(1));   // 1 * 150 + 2 * 100
    EXPECT_EQ(500.0, state->dataHeatBal->ZoneListSNLoadHeatEnergy(1)); // 1 * 300 + 2 * 100
}

TEST_F(EnergyPlusFixture, InfiltrationReportTest)
{

    state->dataGlobal->NumOfZones = 2;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->ZnAirRpt.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->TotVentilation = 1;
    state->dataHeatBal->Ventilation.allocate(state->dataHeatBal->TotVentilation);

    state->dataGlobal->NumOfZones = state->dataGlobal->NumOfZones;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPI = 1.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPI = 1.5;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPV = 2.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPV = 2.5;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat = 0.0005;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 22.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MAT = 25.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRat = 0.0011;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRatAvg = 0.0011;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = 20.0;
    state->dataHeatBal->Zone(2).OutDryBulbTemp = 20.0;
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 0;
    state->dataHeatBal->Ventilation(1).ZonePtr = 1;
    state->dataHeatBal->Ventilation(1).AirTemp = state->dataHeatBal->Zone(1).OutDryBulbTemp;
    state->dataHeatBal->Ventilation(1).MCP = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPV;
    // Call HVACManager
    ReportAirHeatBalance(*state);

    auto &znAirRpt1 = state->dataHeatBal->ZnAirRpt(1);
    auto &znAirRpt2 = state->dataHeatBal->ZnAirRpt(2);

    EXPECT_NEAR(2.9971591, znAirRpt1.InfilVolumeCurDensity, 0.0001);
    EXPECT_NEAR(5.9943183, znAirRpt1.VentilVolumeCurDensity, 0.0001);
    EXPECT_NEAR(2.9827908, znAirRpt1.InfilVolumeStdDensity, 0.0001);
    EXPECT_NEAR(5.9655817, znAirRpt1.VentilVolumeStdDensity, 0.0001);
    EXPECT_NEAR(4.5421638, znAirRpt2.InfilVolumeCurDensity, 0.0001);
    EXPECT_NEAR(7.5702731, znAirRpt2.VentilVolumeCurDensity, 0.0001);
    EXPECT_NEAR(4.4741862, znAirRpt2.InfilVolumeStdDensity, 0.0001);
    EXPECT_NEAR(7.4569771, znAirRpt2.VentilVolumeStdDensity, 0.0001);

    // #8068
    auto &zoneHB1 = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1);
    auto &zone1 = state->dataHeatBal->Zone(1);
    Real64 deltah =
        zoneHB1.MCPI / (Psychrometrics::PsyCpAirFnW(state->dataEnvrn->OutHumRat)) * 3600.0 *
        (Psychrometrics::PsyHFnTdbW(zone1.OutDryBulbTemp, state->dataEnvrn->OutHumRat) - Psychrometrics::PsyHFnTdbW(zoneHB1.MAT, zoneHB1.airHumRat));
    EXPECT_NEAR(-deltah, znAirRpt1.InfilTotalLoss, 0.0001);
    deltah =
        zoneHB1.MCPV / (Psychrometrics::PsyCpAirFnW(state->dataEnvrn->OutHumRat)) * 3600.0 *
        (Psychrometrics::PsyHFnTdbW(zone1.OutDryBulbTemp, state->dataEnvrn->OutHumRat) - Psychrometrics::PsyHFnTdbW(zoneHB1.MAT, zoneHB1.airHumRat));
    EXPECT_NEAR(-deltah, znAirRpt1.VentilTotalLoss, 0.0001);

    // Check outdoor density outputs
    Real64 outdoorRho = Psychrometrics::PsyRhoAirFnPbTdbW(state->dataEnvrn->OutBaroPress, zone1.OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    Real64 expected = znAirRpt1.InfilVdotStdDensity * outdoorRho / state->dataEnvrn->StdRhoAir;
    EXPECT_NEAR(znAirRpt1.InfilVdotOutDensity, expected, 0.0001);
    expected = znAirRpt1.VentilVdotStdDensity * outdoorRho / state->dataEnvrn->StdRhoAir;
    EXPECT_NEAR(znAirRpt1.VentilVdotOutDensity, expected, 0.0001);

    // Check ACH outputs
    expected = znAirRpt1.InfilVdotCurDensity * Constant::rSecsInHour / zone1.Volume;
    EXPECT_NEAR(znAirRpt1.InfilAirChangeRateCurDensity, expected, 0.001);
    Real64 zoneRho = Psychrometrics::PsyRhoAirFnPbTdbW(state->dataEnvrn->OutBaroPress, zoneHB1.MAT, zoneHB1.airHumRat);
    expected = znAirRpt1.InfilAirChangeRateCurDensity * outdoorRho / zoneRho;
    EXPECT_NEAR(znAirRpt1.InfilAirChangeRateOutDensity, expected, 0.001);
    expected = znAirRpt1.InfilAirChangeRateCurDensity * state->dataEnvrn->StdRhoAir / zoneRho;
    EXPECT_NEAR(znAirRpt1.InfilAirChangeRateStdDensity, expected, 0.001);
}

TEST_F(EnergyPlusFixture, ExfilAndExhaustReportTest)
{

    state->dataGlobal->NumOfZones = 2;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->ZnAirRpt.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);

    state->dataGlobal->NumOfZones = state->dataGlobal->NumOfZones;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPI = 1.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPI = 1.5;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPV = 2.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MCPV = 2.5;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat = 0.0005;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 22.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).MAT = 25.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRat = 0.0011;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(2).airHumRatAvg = 0.0011;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = 20.0;
    state->dataHeatBal->Zone(2).OutDryBulbTemp = 20.0;
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 1;

    auto *fan1 = new Fans::FanComponent;
    fan1->Name = "EXHAUST FAN 1";

    fan1->type = HVAC::FanType::Exhaust;
    fan1->outletAirMassFlowRate = 1.0;
    fan1->outletAirTemp = 22.0;
    fan1->outletAirEnthalpy = Psychrometrics::PsyHFnTdbW(fan1->outletAirTemp, 0.0005);
    fan1->inletNodeNum = 1;

    state->dataFans->fans.push_back(fan1);
    state->dataFans->fanMap.insert_or_assign(fan1->Name, state->dataFans->fans.size());

    state->dataLoopNodes->Node.allocate(1);
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;

    // Call HVACManager
    ReportAirHeatBalance(*state);

    EXPECT_NEAR(9.7853391, state->dataHeatBal->ZnAirRpt(1).ExfilTotalLoss, 0.0001);
    EXPECT_NEAR(26.056543, state->dataHeatBal->ZnAirRpt(2).ExfilTotalLoss, 0.0001);
    EXPECT_NEAR(6.0, state->dataHeatBal->ZnAirRpt(1).ExfilSensiLoss, 0.0001);
    EXPECT_NEAR(20.0, state->dataHeatBal->ZnAirRpt(2).ExfilSensiLoss, 0.0001);
    EXPECT_NEAR(23377.40, state->dataHeatBal->ZnAirRpt(1).ExhTotalLoss, 0.01);
    EXPECT_NEAR(0, state->dataHeatBal->ZnAirRpt(2).ExhTotalLoss, 0.01);
    EXPECT_NEAR(35.841882 * 3600, state->dataHeatBal->ZoneTotalExfiltrationHeatLoss, 0.01);
    EXPECT_NEAR(23377.39845 * 3600, state->dataHeatBal->ZoneTotalExhaustHeatLoss, 0.01);
}

TEST_F(EnergyPlusFixture, AirloopFlowBalanceTest)
{

    state->dataGlobal->isPulseZoneSizing = false;
    state->dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = false;
    state->dataGlobal->WarmupFlag = false;
    state->dataHVACGlobal->AirLoopsSimOnce = true;
    state->dataEnvrn->StdRhoAir = 1.0;

    state->dataHVACGlobal->NumPrimaryAirSys = 2;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataAirSystemsData->PrimaryAirSystems(1).Name = "System 1";
    state->dataAirSystemsData->PrimaryAirSystems(2).Name = "System 2";
    state->dataAirLoop->AirLoopFlow.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    auto &thisAirLoopFlow1(state->dataAirLoop->AirLoopFlow(1));
    auto &thisAirLoopFlow2(state->dataAirLoop->AirLoopFlow(2));

    // Case 1 - No flow - no error
    thisAirLoopFlow1.SupFlow = 0.0;
    thisAirLoopFlow1.SysRetFlow = 0.0;
    thisAirLoopFlow1.OAFlow = 0.0;

    thisAirLoopFlow2.SupFlow = 0.0;
    thisAirLoopFlow2.SysRetFlow = 0.0;
    thisAirLoopFlow2.OAFlow = 0.0;

    HVACManager::CheckAirLoopFlowBalance(*state);
    EXPECT_FALSE(has_err_output(true));

    // Case 2 - Both loops are balanced
    thisAirLoopFlow1.SupFlow = 2.0;
    thisAirLoopFlow1.SysRetFlow = 1.0;
    thisAirLoopFlow1.OAFlow = 1.0;

    thisAirLoopFlow2.SupFlow = 3.0;
    thisAirLoopFlow2.SysRetFlow = 3.0;
    thisAirLoopFlow2.OAFlow = 0.0;

    HVACManager::CheckAirLoopFlowBalance(*state);
    EXPECT_FALSE(has_err_output(true));

    // Case 3 - Loop 1 is unbalanced
    thisAirLoopFlow1.SupFlow = 2.0;
    thisAirLoopFlow1.SysRetFlow = 1.0;
    thisAirLoopFlow1.OAFlow = 0.0;

    thisAirLoopFlow2.SupFlow = 3.0;
    thisAirLoopFlow2.SysRetFlow = 3.0;
    thisAirLoopFlow2.OAFlow = 0.0;

    HVACManager::CheckAirLoopFlowBalance(*state);
    EXPECT_TRUE(has_err_output(false));
    std::string error_string =
        delimited_string({"   ** Severe  ** CheckAirLoopFlowBalance: AirLoopHVAC System 1 is unbalanced. Supply is > return plus outdoor air.",
                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                          "   **   ~~~   **   Flows [m3/s at standard density]: Supply=2.000000  Return=1.000000  Outdoor Air=0.000000",
                          "   **   ~~~   **   Imbalance=1.000000",
                          "   **   ~~~   **   This error will only be reported once per system."});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Case 4 - Loop 2 is unbalanced
    thisAirLoopFlow1.SupFlow = 0.0;
    thisAirLoopFlow1.SysRetFlow = 0.0;
    thisAirLoopFlow1.OAFlow = 0.0;

    thisAirLoopFlow2.SupFlow = 3.0;
    thisAirLoopFlow2.SysRetFlow = 2.0;
    thisAirLoopFlow2.OAFlow = 0.99;

    HVACManager::CheckAirLoopFlowBalance(*state);
    EXPECT_TRUE(has_err_output(false));
    error_string =
        delimited_string({"   ** Severe  ** CheckAirLoopFlowBalance: AirLoopHVAC System 2 is unbalanced. Supply is > return plus outdoor air.",
                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                          "   **   ~~~   **   Flows [m3/s at standard density]: Supply=3.000000  Return=2.000000  Outdoor Air=0.990000",
                          "   **   ~~~   **   Imbalance=1.000000E-002",
                          "   **   ~~~   **   This error will only be reported once per system."});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, HVACConvergenceErrorTest)
{
    int i;
    int AirSysNum = 1;
    std::array<bool, 3> HVACNotConverged;
    std::array<Real64, 10> DemandToSupply;
    std::array<Real64, 10> SupplyDeck1ToDemand;
    std::array<Real64, 10> SupplyDeck2ToDemand;

    HVACNotConverged[0] = true;
    HVACNotConverged[1] = false;
    HVACNotConverged[2] = false;

    state->dataAirLoop->AirToZoneNodeInfo.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(AirSysNum).AirLoopName = "AirLoop1";

    // mass flow rate
    for (i = 0; i < 10; i++) {
        DemandToSupply[i] = i * 1.0;
        SupplyDeck1ToDemand[i] = 0.1 * i;
        SupplyDeck2ToDemand[i] = 0.0;
    }
    HVACManager::ConvergenceErrors(
        *state, HVACNotConverged, DemandToSupply, SupplyDeck1ToDemand, SupplyDeck2ToDemand, AirSysNum, ConvErrorCallType::MassFlow);

    std::string const expectedErrString1 =
        delimited_string({"   **   ~~~   ** Air System Named = AirLoop1 did not converge for mass flow rate",
                          "   **   ~~~   ** Check values should be zero. Most Recent values listed first.",
                          "   **   ~~~   ** Demand-to-Supply interface mass flow rate check value iteration history trace: "
                          "0.000000,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,8.000000,9.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 1 mass flow rate check value iteration history trace: "
                          "0.000000,0.100000,0.200000,0.300000,0.400000,0.500000,0.600000,0.700000,0.800000,0.900000,"});
    EXPECT_TRUE(compare_err_stream(expectedErrString1, true));

    // humidity ratio
    for (i = 0; i < 10; i++) {
        DemandToSupply[i] = i * 1.0;
        SupplyDeck1ToDemand[i] = 0.1 * i;
        SupplyDeck2ToDemand[i] = 0.0;
    }
    HVACManager::ConvergenceErrors(
        *state, HVACNotConverged, DemandToSupply, SupplyDeck1ToDemand, SupplyDeck2ToDemand, AirSysNum, ConvErrorCallType::HumidityRatio);

    std::string const expectedErrString2 =
        delimited_string({"   **   ~~~   ** Air System Named = AirLoop1 did not converge for humidity ratio",
                          "   **   ~~~   ** Check values should be zero. Most Recent values listed first.",
                          "   **   ~~~   ** Demand-to-Supply interface humidity ratio check value iteration history trace: "
                          "0.000000,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,8.000000,9.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 1 humidity ratio check value iteration history trace: "
                          "0.000000,0.100000,0.200000,0.300000,0.400000,0.500000,0.600000,0.700000,0.800000,0.900000,"});
    EXPECT_TRUE(compare_err_stream(expectedErrString2, true));

    // temperature
    for (i = 0; i < 10; i++) {
        DemandToSupply[i] = i * 1.0;
        SupplyDeck1ToDemand[i] = 0.1 * i;
        SupplyDeck2ToDemand[i] = 0.0;
    }
    HVACManager::ConvergenceErrors(
        *state, HVACNotConverged, DemandToSupply, SupplyDeck1ToDemand, SupplyDeck2ToDemand, AirSysNum, ConvErrorCallType::Temperature);

    std::string const expectedErrString3 =
        delimited_string({"   **   ~~~   ** Air System Named = AirLoop1 did not converge for temperature",
                          "   **   ~~~   ** Check values should be zero. Most Recent values listed first.",
                          "   **   ~~~   ** Demand-to-Supply interface temperature check value iteration history trace: "
                          "0.000000,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,8.000000,9.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 1 temperature check value iteration history trace: "
                          "0.000000,0.100000,0.200000,0.300000,0.400000,0.500000,0.600000,0.700000,0.800000,0.900000,"});
    EXPECT_TRUE(compare_err_stream(expectedErrString3, true));

    // Energy
    for (i = 0; i < 10; i++) {
        DemandToSupply[i] = i * 1.0;
        SupplyDeck1ToDemand[i] = 0.0;
        SupplyDeck2ToDemand[i] = 0.0;
    }
    HVACManager::ConvergenceErrors(
        *state, HVACNotConverged, DemandToSupply, SupplyDeck1ToDemand, SupplyDeck2ToDemand, AirSysNum, ConvErrorCallType::Energy);

    std::string const expectedErrString4 =
        delimited_string({"   **   ~~~   ** Air System Named = AirLoop1 did not converge for energy",
                          "   **   ~~~   ** Check values should be zero. Most Recent values listed first.",
                          "   **   ~~~   ** Demand-to-Supply interface energy check value iteration history trace: "
                          "0.000000,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,8.000000,9.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 1 energy check value iteration history trace: "
                          "0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,"});
    EXPECT_TRUE(compare_err_stream(expectedErrString4, true));

    // CO2
    for (i = 0; i < 10; i++) {
        DemandToSupply[i] = i * 1.0;
        SupplyDeck1ToDemand[i] = 0.1 * i;
        SupplyDeck2ToDemand[i] = 0.0;
    }
    HVACManager::ConvergenceErrors(
        *state, HVACNotConverged, DemandToSupply, SupplyDeck1ToDemand, SupplyDeck2ToDemand, AirSysNum, ConvErrorCallType::CO2);

    std::string const expectedErrString5 =
        delimited_string({"   **   ~~~   ** Air System Named = AirLoop1 did not converge for CO2",
                          "   **   ~~~   ** Check values should be zero. Most Recent values listed first.",
                          "   **   ~~~   ** Demand-to-Supply interface CO2 check value iteration history trace: "
                          "0.000000,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,8.000000,9.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 1 CO2 check value iteration history trace: "
                          "0.000000,0.100000,0.200000,0.300000,0.400000,0.500000,0.600000,0.700000,0.800000,0.900000,"});
    EXPECT_TRUE(compare_err_stream(expectedErrString5, true));

    // generic contaminant
    for (i = 0; i < 10; i++) {
        DemandToSupply[i] = i * 1.0;
        SupplyDeck1ToDemand[i] = 0.0;
        SupplyDeck2ToDemand[i] = 0.1 * i;
    }
    state->dataAirLoop->AirToZoneNodeInfo(AirSysNum).NumSupplyNodes = 2;
    HVACManager::ConvergenceErrors(
        *state, HVACNotConverged, DemandToSupply, SupplyDeck1ToDemand, SupplyDeck2ToDemand, AirSysNum, ConvErrorCallType::Generic);

    std::string const expectedErrString6 =
        delimited_string({"   **   ~~~   ** Air System Named = AirLoop1 did not converge for generic contaminant",
                          "   **   ~~~   ** Check values should be zero. Most Recent values listed first.",
                          "   **   ~~~   ** Demand-to-Supply interface generic contaminant check value iteration history trace: "
                          "0.000000,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,8.000000,9.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 1 generic contaminant check value iteration history trace: "
                          "0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,",
                          "   **   ~~~   ** Supply-to-demand interface deck 2 generic contaminant check value iteration history trace: "
                          "0.000000,0.100000,0.200000,0.300000,0.400000,0.500000,0.600000,0.700000,0.800000,0.900000,"});
    EXPECT_TRUE(compare_err_stream(expectedErrString6, true));
}
