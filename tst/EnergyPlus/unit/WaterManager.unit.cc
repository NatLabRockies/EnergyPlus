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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/WeatherManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, WaterManager_NormalAnnualPrecipitation)
{
    // This unit test ensures that the WaterManager correctly calculates the Rainfall CurrentRate
    std::string const idf_objects = delimited_string({
        "Site:Precipitation,",
        "ScheduleAndDesignLevel,  !- Precipitation Model Type",
        "0.75,                    !- Design Level for Total Annual Precipitation {m/yr}",
        "PrecipitationSchd,       !- Precipitation Rates Schedule Name",
        "0.80771;                 !- Average Total Annual Precipitation {m/yr}",

        "Schedule:Constant,",
        "PrecipitationSchd,",
        ",",
        "1;",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    WaterManager::GetWaterManagerInput(*state);
    state->dataEnvrn->Year = 2000;
    state->dataEnvrn->EndYear = 2000;
    state->dataEnvrn->Month = 1;
    state->dataGlobal->TimeStep = 2;
    state->dataGlobal->TimeStepZoneSec = 900;

    Sched::GetSchedule(*state, "PRECIPITATIONSCHD")->currentVal = 1.0;

    WaterManager::UpdatePrecipitation(*state);

    Real64 ExpectedNomAnnualRain = 0.80771;
    Real64 ExpectedCurrentRate = 1.0 * (0.75 / 0.80771) / Constant::rSecsInHour;

    Real64 NomAnnualRain = state->dataWaterData->RainFall.NomAnnualRain;
    EXPECT_NEAR(NomAnnualRain, ExpectedNomAnnualRain, 0.000001);

    Real64 CurrentRate = state->dataWaterData->RainFall.CurrentRate;
    EXPECT_NEAR(CurrentRate, ExpectedCurrentRate, 0.000001);
}

TEST_F(EnergyPlusFixture, WaterManager_UpdatePrecipitation)
{
    // with site:precipitation
    std::string const idf_objects = delimited_string({

        "Site:Precipitation,",
        "ScheduleAndDesignLevel,  !- Precipitation Model Type",
        "0.75,                    !- Design Level for Total Annual Precipitation {m/yr}",
        "PrecipitationSchd,       !- Precipitation Rates Schedule Name",
        "0.75;                    !- Average Total Annual Precipitation {m/yr}",

        "Schedule:Constant,",
        "PrecipitationSchd,",
        ",",
        "1;",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    WaterManager::GetWaterManagerInput(*state);
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataEnvrn->Year = 2000;
    state->dataEnvrn->EndYear = 2000;
    state->dataEnvrn->Month = 1;
    state->dataGlobal->TimeStep = 2;

    Sched::GetSchedule(*state, "PRECIPITATIONSCHD")->currentVal = 2.0;

    state->dataEnvrn->LiquidPrecipitation = 0.5;
    WaterManager::UpdatePrecipitation(*state);
    // note even if LiquidPrecipitation is positive, since site:precipitation is precent, still use site:precipitation first
    ASSERT_EQ(state->dataWaterData->RainFall.CurrentRate, 2.0 / 3600);

    // without site:precipitation, use epw "LiquidPrecipitation"
    state->dataEnvrn->LiquidPrecipitation = 0.5;
    state->dataWaterData->RainFall.ModeID = DataWater::RainfallMode::None;
    WaterManager::UpdatePrecipitation(*state);
    ASSERT_EQ(state->dataWaterData->RainFall.CurrentRate, 0.5 / state->dataGlobal->TimeStepZoneSec);
    // when "LiquidPrecipitation" is 0, rainfall rate is 0
    state->dataEnvrn->LiquidPrecipitation = 0.0;
    WaterManager::UpdatePrecipitation(*state);
    ASSERT_EQ(state->dataWaterData->RainFall.CurrentRate, 0.0);
}

TEST_F(EnergyPlusFixture, WaterManager_ZeroAnnualPrecipitation)
{
    // This unit test ensures that the WaterManager does not attempt a divide by zero error when
    // the average total annual precipitation input is zero.
    std::string const idf_objects = delimited_string({
        "Site:Precipitation,",
        "ScheduleAndDesignLevel,  !- Precipitation Model Type",
        "0.75,                    !- Design Level for Total Annual Precipitation {m/yr}",
        "PrecipitationSchd,       !- Precipitation Rates Schedule Name",
        "0.0;                     !- Average Total Annual Precipitation {m/yr}",

        "Schedule:Constant,",
        "PrecipitationSchd,",
        ",",
        "1;",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    WaterManager::GetWaterManagerInput(*state);
    state->dataEnvrn->Year = 2000;
    state->dataEnvrn->EndYear = 2000;
    state->dataEnvrn->Month = 1;
    state->dataGlobal->TimeStep = 2;
    state->dataGlobal->TimeStepZoneSec = 900;

    Sched::GetSchedule(*state, "PRECIPITATIONSCHD")->currentVal = 1.0;

    WaterManager::UpdatePrecipitation(*state);

    Real64 NomAnnualRain = state->dataWaterData->RainFall.NomAnnualRain;
    EXPECT_NEAR(NomAnnualRain, 0.0, 0.000001);

    Real64 CurrentRate = state->dataWaterData->RainFall.CurrentRate;
    EXPECT_NEAR(CurrentRate, 0.0, 0.000001);
}

TEST_F(EnergyPlusFixture, WaterManager_Fill)
{
    // Test for #6435 : When using a Valve from a well, it should keep on filling until it hits the ValveOffCapacity instead of stopping after
    // the first timestep as soon as it's over ValveOnCapacity
    std::string const idf_objects = delimited_string({

        "WaterUse:Storage,",
        "  Cooling Tower Water Storage Tank,  !- Name",
        "  Mains,                   !- Water Quality Subcategory",
        "  3,                       !- Maximum Capacity {m3}",
        "  0.25,                    !- Initial Volume {m3}",
        "  0.003,                   !- Design In Flow Rate {m3/s}",
        "  ,                        !- Design Out Flow Rate {m3/s}",
        "  ,                        !- Overflow Destination",
        "  GroundwaterWell,         !- Type of Supply Controlled by Float Valve",
        "  0.20,                    !- Float Valve On Capacity {m3}",
        "  3,                       !- Float Valve Off Capacity {m3}",
        "  0.10,                    !- Backup Mains Capacity {m3}",
        "  ,                        !- Other Tank Name",
        "  ScheduledTemperature,    !- Water Thermal Mode",
        "  Always 18,               !- Water Temperature Schedule Name",
        "  ,                        !- Ambient Temperature Indicator",
        "  ,                        !- Ambient Temperature Schedule Name",
        "  ,                        !- Zone Name",
        "  ,                        !- Tank Surface Area {m2}",
        "  ,                        !- Tank U Value {W/m2-K}",
        "  ;                        !- Tank Outside Surface Material Name",

        "WaterUse:Well,",
        "  Cooling Tower Transfer Pumps,  !- Name",
        "  Cooling Tower Water Storage Tank,  !- Storage Tank Name",
        "  ,                        !- Pump Depth {m}",
        "  0.003,                   !- Pump Rated Flow Rate {m3/s}",
        "  ,                        !- Pump Rated Head {Pa}",
        "  1500,                    !- Pump Rated Power Consumption {W}",
        "  ,                        !- Pump Efficiency",
        "  ,                        !- Well Recovery Rate {m3/s}",
        "  ,                        !- Nominal Well Storage Volume {m3}",
        "  ,                        !- Water Table Depth Mode",
        "  ,                        !- Water Table Depth {m}",
        "  ;                        !- Water Table Depth Schedule Name",

        "Schedule:Constant,",
        "    Always 18,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    18.0;                    !- Hourly Value",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    WaterManager::GetWaterManagerInput(*state);
    state->dataWaterManager->GetInputFlag = false;

    EXPECT_EQ(1u, state->dataWaterData->WaterStorage.size());
    int TankNum = 1;
    EXPECT_ENUM_EQ(DataWater::ControlSupplyType::WellFloatMainsBackup, state->dataWaterData->WaterStorage(TankNum).ControlSupply);
    EXPECT_EQ(0u, state->dataWaterData->WaterStorage(TankNum).NumWaterDemands);
    EXPECT_EQ(0.003, state->dataWaterData->WaterStorage(TankNum).MaxInFlowRate);
    EXPECT_EQ(0.20, state->dataWaterData->WaterStorage(TankNum).ValveOnCapacity);
    EXPECT_EQ(3.0, state->dataWaterData->WaterStorage(TankNum).ValveOffCapacity);
    EXPECT_EQ(3.0, state->dataWaterData->WaterStorage(TankNum).MaxCapacity);

    // Initialize the tank at 0.29 m3
    Real64 calcVolume = 0.26;
    state->dataWaterData->WaterStorage(TankNum).LastTimeStepVolume = calcVolume;
    state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume = calcVolume;

    // Simulate a call for tank water that would produce 0.025m3 of draw in one timestep
    state->dataHVACGlobal->TimeStepSys = 10.0 / 60.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataWaterData->WaterStorage(TankNum).NumWaterDemands = 1;
    state->dataWaterData->WaterStorage(TankNum).VdotRequestDemand.allocate(1);
    Real64 draw = 0.025;
    state->dataWaterData->WaterStorage(TankNum).VdotRequestDemand(1) = draw / state->dataHVACGlobal->TimeStepSysSec;

    // First call, should bring predicted volume above the ValveOnCapacity
    WaterManager::ManageWater(*state);
    calcVolume -= draw;
    EXPECT_DOUBLE_EQ(calcVolume, state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume);
    EXPECT_DOUBLE_EQ(0.235, calcVolume);
    EXPECT_FALSE(state->dataWaterData->WaterStorage(TankNum).LastTimeStepFilling);

    WaterManager::UpdateWaterManager(*state);

    // Second call, still above the ValveOnCapacity
    WaterManager::ManageWater(*state);
    calcVolume -= draw;
    EXPECT_DOUBLE_EQ(calcVolume, state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume);
    EXPECT_DOUBLE_EQ(0.21, calcVolume);
    EXPECT_FALSE(state->dataWaterData->WaterStorage(TankNum).LastTimeStepFilling);

    WaterManager::UpdateWaterManager(*state);

    // Third call: Predicted volume is below ValveOnCapacity, it kicks on
    WaterManager::ManageWater(*state);
    calcVolume -= draw;
    calcVolume += state->dataWaterData->WaterStorage(TankNum).MaxInFlowRate * state->dataHVACGlobal->TimeStepSysSec;
    EXPECT_DOUBLE_EQ(calcVolume, state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume);
    EXPECT_DOUBLE_EQ(1.985, calcVolume);
    EXPECT_TRUE(state->dataWaterData->WaterStorage(TankNum).LastTimeStepFilling);

    WaterManager::UpdateWaterManager(*state);

    // Fourth call: it should keep on filling, until it hits ValveOffCapacity
    WaterManager::ManageWater(*state);
    calcVolume -= draw;
    calcVolume += state->dataWaterData->WaterStorage(TankNum).MaxInFlowRate * state->dataHVACGlobal->TimeStepSysSec;
    EXPECT_DOUBLE_EQ(3.76, calcVolume);
    calcVolume = min(calcVolume, state->dataWaterData->WaterStorage(TankNum).MaxCapacity);
    EXPECT_DOUBLE_EQ(calcVolume, state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume);
    EXPECT_DOUBLE_EQ(3.0, calcVolume);
    EXPECT_FALSE(state->dataWaterData->WaterStorage(TankNum).LastTimeStepFilling);

    WaterManager::UpdateWaterManager(*state);

    // Fifth Call: now it starts drawing only
    WaterManager::ManageWater(*state);
    calcVolume -= draw;
    EXPECT_DOUBLE_EQ(calcVolume, state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume);
    EXPECT_DOUBLE_EQ(2.975, calcVolume);
    EXPECT_FALSE(state->dataWaterData->WaterStorage(TankNum).LastTimeStepFilling);

    WaterManager::UpdateWaterManager(*state);

    // Sixth call: same.
    WaterManager::ManageWater(*state);
    calcVolume -= draw;
    EXPECT_DOUBLE_EQ(calcVolume, state->dataWaterData->WaterStorage(TankNum).ThisTimeStepVolume);
    EXPECT_DOUBLE_EQ(2.95, calcVolume);
    EXPECT_FALSE(state->dataWaterData->WaterStorage(TankNum).LastTimeStepFilling);

    WaterManager::UpdateWaterManager(*state);
}

TEST_F(EnergyPlusFixture, WaterManager_MainsWater_Meter_Test)
{
    // Test for #10235: A mainswater meter with a blank Water Quality Subcategory
    // should add a "General" mainswater meter
    std::string const idf_objects = delimited_string({

        "WaterUse:Storage,",
        "  Cooling Tower Water Storage Tank,  !- Name",
        "  ,                        !- Water Quality Subcategory",
        "  3,                       !- Maximum Capacity {m3}",
        "  0.25,                    !- Initial Volume {m3}",
        "  0.003,                   !- Design In Flow Rate {m3/s}",
        "  ,                        !- Design Out Flow Rate {m3/s}",
        "  ,                        !- Overflow Destination",
        "  GroundwaterWell,         !- Type of Supply Controlled by Float Valve",
        "  0.20,                    !- Float Valve On Capacity {m3}",
        "  3,                       !- Float Valve Off Capacity {m3}",
        "  0.10,                    !- Backup Mains Capacity {m3}",
        "  ,                        !- Other Tank Name",
        "  ScheduledTemperature,    !- Water Thermal Mode",
        "  Always 18,               !- Water Temperature Schedule Name",
        "  ,                        !- Ambient Temperature Indicator",
        "  ,                        !- Ambient Temperature Schedule Name",
        "  ,                        !- Zone Name",
        "  ,                        !- Tank Surface Area {m2}",
        "  ,                        !- Tank U Value {W/m2-K}",
        "  ;                        !- Tank Outside Surface Material Name",

        "WaterUse:Well,",
        "  Cooling Tower Transfer Pumps,  !- Name",
        "  Cooling Tower Water Storage Tank,  !- Storage Tank Name",
        "  ,                        !- Pump Depth {m}",
        "  0.003,                   !- Pump Rated Flow Rate {m3/s}",
        "  ,                        !- Pump Rated Head {Pa}",
        "  1500,                    !- Pump Rated Power Consumption {W}",
        "  ,                        !- Pump Efficiency",
        "  ,                        !- Well Recovery Rate {m3/s}",
        "  ,                        !- Nominal Well Storage Volume {m3}",
        "  ,                        !- Water Table Depth Mode",
        "  ,                        !- Water Table Depth {m}",
        "  ;                        !- Water Table Depth Schedule Name",

        "Schedule:Constant,",
        "    Always 18,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    18.0;                    !- Hourly Value",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    WaterManager::GetWaterManagerInput(*state);
    state->dataWaterManager->GetInputFlag = false;

    EXPECT_EQ(state->dataWaterData->WaterStorage.size(), 1u);

    EXPECT_EQ(state->dataOutputProcessor->meters.size(), 11u);

    EXPECT_EQ(state->dataOutputProcessor->meters[3]->Name, "General:WaterSystems:MainsWater");
    EXPECT_ENUM_EQ(state->dataOutputProcessor->meters[3]->resource, Constant::eResource::MainsWater);
    EXPECT_ENUM_EQ(state->dataOutputProcessor->meters[3]->endUseCat, OutputProcessor::EndUseCat::WaterSystem);
    EXPECT_EQ(state->dataOutputProcessor->meters[3]->EndUseSub, "General");
    EXPECT_ENUM_EQ(state->dataOutputProcessor->meters[3]->group, OutputProcessor::Group::Invalid);
}
