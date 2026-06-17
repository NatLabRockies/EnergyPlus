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

// EnergyPlus::HeatBalanceKivaManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/HeatBalanceKivaManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalanceKiva_SetInitialBCs)
{

    // Unit test for Issue #7291 - Kiva initialization indoor temperature assumptions

    // Create Kiva foundation and set parameters
    Kiva::Foundation fnd;

    fnd.reductionStrategy = Kiva::Foundation::RS_AP;

    Kiva::Material concrete(1.95, 2400.0, 900.0);

    Kiva::Layer tempLayer;
    tempLayer.thickness = 0.10;
    tempLayer.material = concrete;

    fnd.slab.interior.emissivity = 0.8;
    fnd.slab.layers.push_back(tempLayer);

    tempLayer.thickness = 0.2;
    tempLayer.material = concrete;

    fnd.wall.layers.push_back(tempLayer);

    fnd.wall.heightAboveGrade = 0.1;
    fnd.wall.depthBelowSlab = 0.2;
    fnd.wall.interior.emissivity = 0.8;
    fnd.wall.exterior.emissivity = 0.8;
    fnd.wall.interior.absorptivity = 0.8;
    fnd.wall.exterior.absorptivity = 0.8;

    fnd.foundationDepth = 0.0;
    fnd.numericalScheme = Kiva::Foundation::NS_ADI;

    fnd.polygon.outer().push_back(Kiva::Point(-6.0, -6.0));
    fnd.polygon.outer().push_back(Kiva::Point(-6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, -6.0));

    // Create Kiva weather data
    HeatBalanceKivaManager::KivaWeatherData kivaweather;
    kivaweather.annualAverageDrybulbTemp = 10.0;
    kivaweather.intervalsPerHour = 1;
    kivaweather.dryBulb = {10.0};
    kivaweather.windSpeed = {0.0};
    kivaweather.skyEmissivity = {0.0};

    HeatBalanceKivaManager::KivaManager km;

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Core_bottom,             !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  ,                        !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",
        " ",
        "ZoneControl:Thermostat,",
        "  Core_bottom Thermostat,  !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "  Core_bottom DualSPSched; !- Control 1 Name",
        " ",
        "Schedule:Constant,",
        "  Dual Zone Control Type Sched,  !- Name",
        "  Control Type,            !- Schedule Type Limits Name",
        "  4;                       !- Field 1",
        " ",
        "ThermostatSetpoint:DualSetpoint,",
        "  Core_bottom DualSPSched, !- Name",
        "  HTGSETP_SCH,             !- Heating Setpoint Temperature Schedule Name",
        "  CLGSETP_SCH;             !- Cooling Setpoint Temperature Schedule Name",
        " ",
        "Schedule:Constant,",
        "  CLGSETP_SCH,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  24.0;                    !- Field 1",
        " ",
        "Schedule:Constant,",
        "  HTGSETP_SCH,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  20.0;                    !- Field 1",
        " ",
        "Schedule:Constant,",
        "  CLGSETP_SCH_EXTREME,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  100.0;                    !- Field 1",
        " ",
        "Schedule:Constant,",
        "  HTGSETP_SCH_EXTREME,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  -100.0;                    !- Field 1",
        " ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false); // If errors detected in input
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    int DualZoneNum(1);

    state->dataEnvrn->DayOfYear_Schedule = 1; // must initialize this to get schedules initialized
    state->dataEnvrn->DayOfWeek = 1;          // must initialize this to get schedules initialized
    state->dataGlobal->HourOfDay = 1;         // must initialize this to get schedules initialized
    state->dataGlobal->TimeStep = 1;          // must initialize this to get schedules initialized

    ZoneTempPredictorCorrector::GetZoneAirSetPoints(*state);

    state->dataZoneCtrls->TempControlledZone(DualZoneNum).setptTypeSched->currentVal = (int)HVAC::SetptType::DualHeatCool;

    // Test Initial Indoor Temperature input of 15C with Cooling/Heating Setpoints of 24C/20C

    Real64 zoneAssumedTemperature1 = 15.0;
    HeatBalanceKivaManager::KivaInstanceMap kv1(*state, fnd, 0, {}, 0, zoneAssumedTemperature1, 1.0, 0, &km);

    kv1.zoneControlNum = 1;
    kv1.zoneControlType = 1; // Temperature

    kv1.setInitialBoundaryConditions(*state, kivaweather, 1, 1, 1);

    Real64 expectedResult1 = kv1.instance.bcs->slabConvectiveTemp;

    EXPECT_NEAR(expectedResult1, zoneAssumedTemperature1 + Constant::Kelvin, 0.001);

    // Test using default Initial Indoor Temperature with Cooling/Heating Setpoints of 24C/20C

    Real64 heatingSetpoint2 = 20.0;
    Real64 zoneAssumedTemperature2 = -9999;
    HeatBalanceKivaManager::KivaInstanceMap kv2(*state, fnd, 0, {}, 0, zoneAssumedTemperature2, 1.0, 0, &km);

    kv2.zoneControlNum = 1;
    kv2.zoneControlType = 1; // Temperature

    kv2.setInitialBoundaryConditions(*state, kivaweather, 1, 1, 1);

    Real64 expectedResult2 = kv2.instance.bcs->slabConvectiveTemp;

    EXPECT_NEAR(expectedResult2, heatingSetpoint2 + Constant::Kelvin, 0.001);

    // Test using default Initial Indoor Temperature with Cooling/Heating Setpoints of 100C/-100C

    state->dataZoneCtrls->TempControlledZone(1).setpts[(int)HVAC::SetptType::DualHeatCool].coolSetptSched =
        Sched::GetSchedule(*state, "CLGSETP_SCH_EXTREME");
    state->dataZoneCtrls->TempControlledZone(1).setpts[(int)HVAC::SetptType::DualHeatCool].heatSetptSched =
        Sched::GetSchedule(*state, "HTGSETP_SCH_EXTREME");

    Real64 heatingSetpoint3 = -100.0;
    Real64 zoneAssumedTemperature3 = -9999;
    HeatBalanceKivaManager::KivaInstanceMap kv3(*state, fnd, 0, {}, 0, zoneAssumedTemperature3, 1.0, 0, &km);

    kv3.zoneControlNum = 1;
    kv3.zoneControlType = 1; // Temperature

    kv3.setInitialBoundaryConditions(*state, kivaweather, 1, 1, 1);

    Real64 expectedResult3 = kv3.instance.bcs->slabConvectiveTemp;

    EXPECT_NEAR(expectedResult3, heatingSetpoint3 + Constant::Kelvin, 0.001);

    // Test Initial Indoor Temperature input of 15C with Cooling/Heating Setpoints of 100C/-100C

    state->dataZoneCtrls->TempControlledZone(1).setpts[(int)HVAC::SetptType::DualHeatCool].coolSetptSched =
        Sched::GetSchedule(*state, "CLGSETP_SCH_EXTREME");
    state->dataZoneCtrls->TempControlledZone(1).setpts[(int)HVAC::SetptType::DualHeatCool].heatSetptSched =
        Sched::GetSchedule(*state, "HTGSETP_SCH_EXTREME");
    Real64 zoneAssumedTemperature4 = 15.0;
    HeatBalanceKivaManager::KivaInstanceMap kv4(*state, fnd, 0, {}, 0, zoneAssumedTemperature4, 1.0, 0, &km);

    kv4.zoneControlNum = 1;
    kv4.zoneControlType = 1; // Temperature

    kv4.setInitialBoundaryConditions(*state, kivaweather, 1, 1, 1);

    Real64 expectedResult4 = kv4.instance.bcs->slabConvectiveTemp;

    EXPECT_NEAR(expectedResult4, zoneAssumedTemperature4 + Constant::Kelvin, 0.001);
}

TEST_F(EnergyPlusFixture, HeatBalanceKiva_setupKivaInstances_ThermalComfort)
{

    // Unit test for Issue #8353 - Thermal Comfort thermostat controls failure due to Kiva slab foundation object interference

    // Create Kiva foundation and set parameters
    Kiva::Foundation fnd;

    fnd.reductionStrategy = Kiva::Foundation::RS_AP;

    Kiva::Material concrete(1.95, 2400.0, 900.0);

    Kiva::Layer tempLayer;
    tempLayer.thickness = 0.10;
    tempLayer.material = concrete;

    fnd.slab.interior.emissivity = 0.8;
    fnd.slab.layers.push_back(tempLayer);

    tempLayer.thickness = 0.2;
    tempLayer.material = concrete;

    fnd.wall.layers.push_back(tempLayer);

    fnd.wall.heightAboveGrade = 0.1;
    fnd.wall.depthBelowSlab = 0.2;
    fnd.wall.interior.emissivity = 0.8;
    fnd.wall.exterior.emissivity = 0.8;
    fnd.wall.interior.absorptivity = 0.8;
    fnd.wall.exterior.absorptivity = 0.8;

    fnd.foundationDepth = 0.0;
    fnd.numericalScheme = Kiva::Foundation::NS_ADI;

    fnd.polygon.outer().push_back(Kiva::Point(-6.0, -6.0));
    fnd.polygon.outer().push_back(Kiva::Point(-6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, -6.0));

    // Create Kiva weather data
    HeatBalanceKivaManager::KivaWeatherData kivaweather;
    kivaweather.annualAverageDrybulbTemp = 10.0;
    kivaweather.intervalsPerHour = 1;
    kivaweather.dryBulb = {10.0};
    kivaweather.windSpeed = {0.0};
    kivaweather.skyEmissivity = {0.0};

    HeatBalanceKivaManager::KivaManager km;

    std::string const idf_objects = delimited_string({
        "Material,",
        "  1/2IN Gypsum,            !- Name",
        "  Smooth,                  !- Roughness",
        "  0.0127,                  !- Thickness {m}",
        "  0.1600,                  !- Conductivity {W/m-K}",
        "  784.9000,                !- Density {kg/m3}",
        "  830.0000,                !- Specific Heat {J/kg-K}",
        "  0.9000,                  !- Thermal Absorptance",
        "  0.9200,                  !- Solar Absorptance",
        "  0.9200;                  !- Visible Absorptance",
        " ",
        "Material,",
        "  MAT-CC05 4 HW CONCRETE,  !- Name",
        "  Rough,                   !- Roughness",
        "  0.1016,                  !- Thickness {m}",
        "  1.3110,                  !- Conductivity {W/m-K}",
        "  2240.0000,               !- Density {kg/m3}",
        "  836.8000,                !- Specific Heat {J/kg-K}",
        "  0.9000,                  !- Thermal Absorptance",
        "  0.7000,                  !- Solar Absorptance",
        "  0.7000;                  !- Visible Absorptance",
        " ",
        "Material:NoMass,",
        "  CP02 CARPET PAD,         !- Name",
        "  VeryRough,               !- Roughness",
        "  0.2165,                  !- Thermal Resistance {m2-K/W}",
        "  0.9000,                  !- Thermal Absorptance",
        "  0.7000,                  !- Solar Absorptance",
        "  0.8000;                  !- Visible Absorptance",
        " ",
        "Material,",
        "  Std AC02,                !- Name",
        "  MediumSmooth,            !- Roughness",
        "  1.2700000E-02,           !- Thickness {m}",
        "  5.7000000E-02,           !- Conductivity {W/m-K}",
        "  288.0000,                !- Density {kg/m3}",
        "  1339.000,                !- Specific Heat {J/kg-K}",
        "  0.9000000,               !- Thermal Absorptance",
        "  0.7000000,               !- Solar Absorptance",
        "  0.2000000;               !- Visible Absorptance",
        " ",
        "Construction,",
        "  int-walls,               !- Name",
        "  1/2IN Gypsum,            !- Outside Layer",
        "  1/2IN Gypsum;            !- Layer 2",
        " ",
        "Construction,",
        "  INT-FLOOR-TOPSIDE,       !- Name",
        "  MAT-CC05 4 HW CONCRETE,  !- Outside Layer",
        "  CP02 CARPET PAD;         !- Layer 2",
        " ",
        "Construction,",
        "  DropCeiling,             !- Name",
        "  Std AC02;                !- Outside Layer",
        " ",
        "Zone,",
        "  Core_bottom,             !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  ,                        !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Ceiling,   !- Name",
        "  Ceiling,                 !- Surface Type",
        "  DropCeiling,             !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  4.5732,44.1650,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  4.5732,4.5732,2.7440,  !- X,Y,Z ==> Vertex 2 {m}",
        "  68.5340,4.5732,2.7440,  !- X,Y,Z ==> Vertex 3 {m}",
        "  68.5340,44.1650,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Floor,     !- Name",
        "  Floor,                   !- Surface Type",
        "  INT-FLOOR-TOPSIDE,       !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  68.5340,44.1650,0.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "  68.5340,4.5732,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  4.5732,4.5732,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  4.5732,44.1650,0.0000;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_East, !- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  68.5340,4.5732,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  68.5340,4.5732,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  68.5340,44.1650,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  68.5340,44.1650,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_North,!- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  68.5340,44.1650,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  68.5340,44.1650,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  4.5732,44.1650,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  4.5732,44.1650,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_South,!- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  4.5732,4.5732,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  4.5732,4.5732,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  68.5340,4.5732,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  68.5340,4.5732,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_West, !- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  4.5732,44.1650,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  4.5732,44.1650,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  4.5732,4.5732,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  4.5732,4.5732,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "People,",
        "  Core_bottom People,      !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  Core_bottom Occupancy,   !- Number of People Schedule Name",
        "  People,                  !- Number of People Calculation Method",
        "  4,                       !- Number of People",
        "  ,                        !- People per Zone Floor Area",
        "  ,                        !- Zone Floor Area per Person",
        "  0.9,                     !- Fraction Radiant",
        "  0.1,                     !- Sensible Heat Fraction",
        "  Core_bottom Activity,    !- Activity Level Schedule Name",
        "  3.82e-08,                !- Carbon Dioxide Generation Rate",

        "  Yes,                     !- Enable ASHRAE 55 Comfort Warnings",
        "  EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        "  ,                        !- Surface NameAngle Factor List Name",
        "  Work Eff Sched,          !- Work Efficiency Schedule Name",
        "  ClothingInsulationSchedule,  !- Clothing Insulation Calculation Method",
        "  ,                        !- Clothing Insulation Calculation Method Schedule Name",
        "  Clothing Schedule,       !- Clothing Insulation Schedule Name",
        "  Air Velocity Schedule,   !- Air Velocity Schedule Name",
        "  Fanger;                  !- Thermal Comfort Model 1 Type",
        " ",
        "Schedule:Compact,",
        "  Core_bottom Occupancy,   !- Name",
        "  Fraction,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: Alldays,            !- Field 2",
        "  Until: 07:00,            !- Field 3",
        "  0,                       !- Field 4",
        "  Until: 21:00,            !- Field 13",
        "  1,                       !- Field 14",
        "  Until: 24:00,            !- Field 15",
        "  0;                       !- Field 16",
        " ",
        "Schedule:Compact,",
        "  Core_bottom Activity,    !- Name",
        "  Activity Level,          !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: Alldays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  166;                     !- Field 4",
        " ",
        "Schedule:Compact,",
        "  Work Eff Sched,          !- Name",
        "  Dimensionless,           !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0;                       !- Field 4",
        " ",
        "Schedule:Compact,",
        "  Clothing Schedule,       !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0.5;                     !- Field 4",
        " ",
        "Schedule:Compact,",
        "  Air Velocity Schedule,   !- Name",
        "  Velocity,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0.129999995231628;       !- Field 4",
        " ",
        "ZoneControl:Thermostat,",
        "  Core_bottom Thermostat,  !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "  Core_bottom DualSPSched; !- Control 1 Name",
        " ",
        "ZoneControl:Thermostat:ThermalComfort,",
        "  Core_bottom Comfort,     !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  SpecificObject,          !- Averaging Method",
        "  Core_bottom People,      !- Specific People Name",
        "  0,                       !- Minimum DryBulb Temperature Setpoint",
        "  50,                      !- Maximum DryBulb Temperature Setpoint",
        "  Comfort Control,         !- Thermal Comfort Control Type Schedule Name",
        "  ThermostatSetpoint:ThermalComfort:Fanger:SingleHeating,    !- Thermal Comfort Control 1 Object Type",
        "  Single Htg PMV,          !- Thermal Comfort Control 1 Name",
        "  ThermostatSetpoint:ThermalComfort:Fanger:SingleCooling,    !- Thermal Comfort Control 2 Object Type",
        "  Single Cooling PMV;      !- Thermal Comfort Control 2 Name,",
        " ",
        "Schedule:Compact,",
        "  Comfort Control,          !- Name",
        "  Control Type,             !- Schedule Type Limits Name",
        "  Through: 5/31,            !- Field 1",
        "  For: AllDays,             !- Field 2",
        "  Until: 24:00,             !- Field 3",
        "  1,                        !- Field 4",
        "  Through: 8/31,            !- Field 5",
        "  For: AllDays,             !- Field 6",
        "  Until: 24:00,             !- Field 7",
        "  2,                        !- Field 8",
        "  Through: 12/31,           !- Field 9",
        "  For: AllDays,             !- Field 10",
        "  Until: 24:00,             !- Field 11",
        "  1;                        !- Field 12",
        " ",
        "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLEHEATING,",
        "  Single Htg PMV,          !- Name",
        "  Single Htg PMV;          !- Fanger Thermal Comfort Schedule Name",
        " ",
        "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLECOOLING,",
        "  Single Cooling PMV,      !- Name",
        "  Single Cooling PMV;      !- Fanger Thermal Comfort Schedule Name",
        " ",
        "Schedule:Constant,",
        "  Dual Zone Control Type Sched,  !- Name",
        "  Control Type,            !- Schedule Type Limits Name",
        "  4;                       !- Field 1",
        " ",
        "ThermostatSetpoint:DualSetpoint,",
        "  Core_bottom DualSPSched, !- Name",
        "  HTGSETP_SCH,             !- Heating Setpoint Temperature Schedule Name",
        "  CLGSETP_SCH;             !- Cooling Setpoint Temperature Schedule Name",
        " ",
        "Schedule:Constant,",
        "  CLGSETP_SCH,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  24.0;                    !- Field 1",
        " ",
        "Schedule:Constant,",
        "  HTGSETP_SCH,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  20.0;                    !- Field 1",
        " ",
        "Schedule:Compact,",
        "  Single Htg PMV,          !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 6:00,             !- Field 3",
        "  -0.5,                    !- Field 4",
        "  Until: 23:00,            !- Field 5",
        "  -0.2,                    !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  -0.5;                    !- Field 8",
        " ",
        "Schedule:Compact,",
        "  Single Cooling PMV,      !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 6:00,             !- Field 3",
        "  0.5,                     !- Field 4",
        "  Until: 23:00,            !- Field 5",
        "  0.2,                     !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  0.5;                     !- Field 8",
        " ",
        "ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0,                        !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Dimensionless;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Temperature,              !- Name",
        "  -60,                      !- Lower Limit Value",
        "  200,                      !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Temperature;              !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Control Type,             !- Name",
        "  0,                        !- Lower Limit Value",
        "  4,                        !- Upper Limit Value",
        "  Discrete,                 !- Numeric Type",
        "  Dimensionless;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  On/Off,                   !- Name",
        "  0,                        !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Discrete,                 !- Numeric Type",
        "  Dimensionless;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Any Number,               !- Name",
        "  ,                         !- Lower Limit Value",
        "  ,                         !- Upper Limit Value",
        "  Continuous;               !- Numeric Type",
        " ",
        "ScheduleTypeLimits,",
        "  Velocity,                 !- Name",
        "  ,                         !- Lower Limit Value",
        "  ,                         !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Velocity;                 !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Activity Level,           !- Name",
        "  0,                        !- Lower Limit Value",
        "  ,                         !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  ActivityLevel;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Dimensionless,            !- Name",
        "  -1,                       !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Continuous;               !- Numeric Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false); // If errors detected in input
    ASSERT_FALSE(ErrorsFound);

    state->dataEnvrn->DayOfYear_Schedule = 1; // must initialize this to get schedules initialized
    state->dataEnvrn->DayOfWeek = 1;          // must initialize this to get schedules initialized
    state->dataGlobal->HourOfDay = 1;         // must initialize this to get schedules initialized
    state->dataGlobal->TimeStep = 1;          // must initialize this to get schedules initialized

    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/HeatBalanceKivaManagerOSkyTest.epw";
    HeatBalanceManager::GetHeatBalanceInput(*state);
    EXPECT_FALSE(has_err_output());
}

TEST_F(EnergyPlusFixture, OpaqueSkyCover_InterpretWeatherMissingOpaqueSkyCover)
{

    // DERIVED TYPE DEFINITIONS:
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/HeatBalanceKivaManagerOSkyTest.epw";
    state->dataWeather->wvarsMissing.OpaqueSkyCover = 5;

    HeatBalanceKivaManager::KivaManager km;
    km.readWeatherData(*state);

    Real64 TDewK = 264.25;
    Real64 expected_OSky = 5;
    Real64 expected_ESky = (0.787 + 0.764 * std::log(TDewK / Constant::Kelvin)) *
                           (1.0 + 0.0224 * expected_OSky - 0.0035 * pow_2(expected_OSky) + 0.00028 * pow_3(expected_OSky));

    EXPECT_NEAR(expected_ESky, km.kivaWeather.skyEmissivity[0], 0.01);
}

TEST_F(EnergyPlusFixture, HeatBalanceKiva_DeepGroundDepthCheck)
{

    // Create Kiva foundation and set parameters
    Kiva::Foundation fnd;

    fnd.wall.heightAboveGrade = 0.1;
    fnd.wall.depthBelowSlab = 0.2;
    fnd.foundationDepth = 10.0;

    // Initial deep ground depth is less than the depth of the wall below grade
    fnd.deepGroundDepth = 5.0;
    Real64 initDeepGroundDepth = fnd.deepGroundDepth;

    HeatBalanceKivaManager::KivaManager km;
    fnd.deepGroundDepth = km.getDeepGroundDepth(fnd);

    // Deep ground depth is modified to 1.0m below the depth of the wall below grade
    Real64 totalDepthOfWallBelowGrade = fnd.wall.depthBelowSlab + (fnd.foundationDepth - fnd.wall.heightAboveGrade) + fnd.slab.totalWidth();
    Real64 expectedValue = totalDepthOfWallBelowGrade + 1.0;
    EXPECT_NE(initDeepGroundDepth, fnd.deepGroundDepth);
    EXPECT_EQ(expectedValue, fnd.deepGroundDepth);
}

TEST_F(EnergyPlusFixture, HeatBalanceKiva_GetAccDate)
{
    // Create Kiva foundation and set parameters
    Kiva::Foundation fnd;

    fnd.reductionStrategy = Kiva::Foundation::RS_AP;

    Kiva::Material concrete(1.95, 2400.0, 900.0);

    Kiva::Layer tempLayer;
    tempLayer.thickness = 0.10;
    tempLayer.material = concrete;

    fnd.slab.interior.emissivity = 0.8;
    fnd.slab.layers.push_back(tempLayer);

    tempLayer.thickness = 0.2;
    tempLayer.material = concrete;

    fnd.wall.layers.push_back(tempLayer);

    fnd.wall.heightAboveGrade = 0.1;
    fnd.wall.depthBelowSlab = 0.2;
    fnd.wall.interior.emissivity = 0.8;
    fnd.wall.exterior.emissivity = 0.8;
    fnd.wall.interior.absorptivity = 0.8;
    fnd.wall.exterior.absorptivity = 0.8;

    fnd.foundationDepth = 0.0;
    fnd.numericalScheme = Kiva::Foundation::NS_ADI;

    fnd.polygon.outer().push_back(Kiva::Point(-6.0, -6.0));
    fnd.polygon.outer().push_back(Kiva::Point(-6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, -6.0));

    HeatBalanceKivaManager::KivaManager km;
    HeatBalanceKivaManager::KivaInstanceMap kv(*state, fnd, 0, {}, 0, 15.0, 1.0, 0, &km);

    // Set day of year to 121 (May 1st)
    state->dataEnvrn->DayOfYear = 121;

    int numAccelaratedTimesteps = 3;
    int acceleratedTimestep = 30; // days
    int accDate = kv.getAccDate(*state, numAccelaratedTimesteps, acceleratedTimestep);

    // Accelerated date should be greater than 0
    EXPECT_GT(accDate, 0);
}

TEST_F(EnergyPlusFixture, HeatBalanceKiva_setMessageCallback)
{
    // Unit test for Issue #9309

    int SurfNum;

    SurfNum = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(SurfNum).ExtBoundCond = DataSurfaces::KivaFoundation;
    state->dataSurface->Surface(SurfNum).Name = "Kiva Floor";
    state->dataSurface->AllHTKivaSurfaceList = {1};
    HeatBalanceKivaManager::KivaManager km;

    EXPECT_THROW(km.calcKivaSurfaceResults(*state), std::runtime_error);

    std::string const error_string = delimited_string({
        "   ** Severe  ** Surface=\"Kiva Floor\": The weights of associated Kiva instances do not add to unity--check exposed perimeter values.",
        "   **  Fatal  ** Kiva: Errors discovered, program terminates.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Surface=\"Kiva Floor\": The weights of associated Kiva instances do not add to "
        "unity--check exposed perimeter values.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

} // namespace EnergyPlus
