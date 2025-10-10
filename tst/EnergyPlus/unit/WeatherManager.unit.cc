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

// EnergyPlus::WeatherManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/WeatherManager.hh>

// Fixtures
#include "Fixtures/EnergyPlusFixture.hh"
#include "Fixtures/SQLiteFixture.hh"

#include <array>
#include <numeric>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, SkyTempTest)
{
    std::string const idf_objects = delimited_string({
        "SimulationControl, NO, NO, NO, YES, YES;",
        "Timestep,4;",
        "RunPeriod,",
        "RP1,                     !- Name",
        "2,                       !- Begin Month",
        "27,                      !- Begin Day of Month",
        ",                        !- Begin Year",
        "3,                       !- End Month",
        "3,                       !- End Day of Month",
        ",                        !- End Year",
        "Tuesday,                 !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "BUILDING, Simple One Zone (Wireframe DXF), 0.0, Suburbs, .04, .004, MinimalShadowing, 30, 6;",
        "Schedule:Compact,",
        "TskySchedule,                !- Name",
        ",              !- Schedule Type Limits Name",
        "Through: 2/26, For: AllOtherDays,  Until: 24:00, 2.26,",
        "Through: 2/27, For: AllOtherDays,  Until: 24:00, 2.27,",
        "Through: 2/28, For: AllOtherDays,  Until: 24:00, 2.28,",
        "Through: 3/1, For: AllOtherDays,  Until: 24:00, 3.01,",
        "Through: 3/2, For: AllOtherDays,  Until: 24:00, 3.02,",
        "Through: 12/31, For: AllOtherDays,  Until: 24:00, 12.31;",
        "WeatherProperty:SkyTemperature,",
        ",                        !- Name",
        "ScheduleValue,           !- Calculation Type",
        "TskySchedule;                  !- Schedule Name",
        "Site:WaterMainsTemperature,",
        "Schedule,             !- Calculation Method",
        "TskySchedule,                        !- Temperature Schedule Name",
        ",                   !- Annual Average Outdoor Air Temperature{ C }",
        ";                   !- Maximum Difference In Monthly Average Outdoor Air Temperatures{ deltaC }",
        "Output:Variable,*,Schedule Value,hourly;",
        "Output:Variable,*,Site Sky Temperature,hourly;",
        "Output:Variable,*,Site Mains Water Temperature,hourly; !- Zone Average[C]",
        "  Site:Location,",
        "    USA IL-CHICAGO-OHARE,    !- Name",
        "    41.77,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190;                     !- Elevation {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->MinutesInTimeStep = 60 / state->dataGlobal->TimeStepsInHour;

    state->init_state(*state);

    auto *tSkySched = Sched::GetSchedule(*state, "TSKYSCHEDULE");
    // Febuary 27

    EXPECT_NEAR(2.27, tSkySched->getDayVals(*state, 58, 3)[0 * state->dataGlobal->TimeStepsInHour + 0], .001);

    // Febuary 28
    EXPECT_NEAR(2.28, tSkySched->getDayVals(*state, 59, 4)[0 * state->dataGlobal->TimeStepsInHour + 0], .001);

    // March 1
    EXPECT_NEAR(3.01, tSkySched->getDayVals(*state, 60, 5)[0 * state->dataGlobal->TimeStepsInHour + 0], .001);

    // Not March 2, this "Day" is ignored unless its a leap year, otherwise same data as March 1
    EXPECT_NEAR(3.01, tSkySched->getDayVals(*state, 61, 6)[0 * state->dataGlobal->TimeStepsInHour + 0], .001);

    // March 2
    EXPECT_NEAR(3.02, tSkySched->getDayVals(*state, 62, 6)[0 * state->dataGlobal->TimeStepsInHour + 0], .001);
}

TEST_F(EnergyPlusFixture, SkyEmissivityTest)
{
    // setup environment state
    state->dataWeather->Environment.allocate(4);
    state->dataWeather->Environment(1).skyTempModel = Weather::SkyTempModel::ClarkAllen;
    state->dataWeather->Environment(2).skyTempModel = Weather::SkyTempModel::Brunt;
    state->dataWeather->Environment(3).skyTempModel = Weather::SkyTempModel::Idso;
    state->dataWeather->Environment(4).skyTempModel = Weather::SkyTempModel::BerdahlMartin;

    // init local variables
    Real64 OpaqueSkyCover(0.0);
    Real64 DryBulb(25.0);
    Real64 DewPoint(16.7);
    Real64 RelHum(0.6);

    EXPECT_NEAR(0.832, CalcSkyEmissivity(*state, state->dataWeather->Environment(1).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
    EXPECT_NEAR(0.862, CalcSkyEmissivity(*state, state->dataWeather->Environment(2).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
    EXPECT_NEAR(0.867, CalcSkyEmissivity(*state, state->dataWeather->Environment(3).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
    EXPECT_NEAR(0.862, CalcSkyEmissivity(*state, state->dataWeather->Environment(4).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);

    DryBulb = 5.0;
    DewPoint = -2.13;
    EXPECT_NEAR(0.781, CalcSkyEmissivity(*state, state->dataWeather->Environment(1).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
    EXPECT_NEAR(0.746, CalcSkyEmissivity(*state, state->dataWeather->Environment(2).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
    EXPECT_NEAR(0.760, CalcSkyEmissivity(*state, state->dataWeather->Environment(3).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
    EXPECT_NEAR(0.747, CalcSkyEmissivity(*state, state->dataWeather->Environment(4).skyTempModel, OpaqueSkyCover, DryBulb, DewPoint, RelHum), 0.001);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationTest)
{

    state->dataWeather->WaterMainsTempsMethod = Weather::WaterMainsTempCalcMethod::Correlation;
    state->dataWeather->WaterMainsTempsAnnualAvgAirTemp = 9.69;
    state->dataWeather->WaterMainsTempsMaxDiffAirTemp = 28.1;
    state->dataWeather->WaterMainsTempsMultiplier = 1.0;
    state->dataWeather->WaterMainsTempsOffset = 0.0;
    state->dataEnvrn->DayOfYear = 50;

    state->dataEnvrn->Latitude = 40.0;
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 6.6667, 0.0001);

    state->dataEnvrn->Latitude = -40.0;
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 19.3799, 0.0001);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationTestWithMultiplierAndOffset)
{

    state->dataWeather->WaterMainsTempsMethod = Weather::WaterMainsTempCalcMethod::Correlation;
    state->dataWeather->WaterMainsTempsAnnualAvgAirTemp = 9.69;
    state->dataWeather->WaterMainsTempsMaxDiffAirTemp = 28.1;
    state->dataWeather->WaterMainsTempsMultiplier = 1.1;
    state->dataWeather->WaterMainsTempsOffset = 5.0;
    state->dataEnvrn->DayOfYear = 50;

    state->dataEnvrn->Latitude = 40.0;
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 12.3334, 0.0001);

    state->dataEnvrn->Latitude = -40.0;
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 26.3179, 0.0001);
}

TEST_F(EnergyPlusFixture, JGDate_Test)
{
    // used http://aa.usno.navy.mil/data/docs/JulianDate.php
    //
    int julianDate;
    Weather::GregorianDate gregorianDate = {2016, 5, 25}; // when test was made

    julianDate = computeJulianDate(gregorianDate);
    EXPECT_EQ(2457534, julianDate);
    gregorianDate = Weather::computeGregorianDate(julianDate);
    EXPECT_EQ(2016, gregorianDate.year);
    EXPECT_EQ(5, gregorianDate.month);
    EXPECT_EQ(25, gregorianDate.day);

    gregorianDate.year--; // a year before
    julianDate = computeJulianDate(gregorianDate);
    EXPECT_EQ(2457168, julianDate);
    gregorianDate = Weather::computeGregorianDate(julianDate);
    EXPECT_EQ(2015, gregorianDate.year);
    EXPECT_EQ(5, gregorianDate.month);
    EXPECT_EQ(25, gregorianDate.day);

    gregorianDate = {1966, 7, 16}; // a fine date in history
    julianDate = computeJulianDate(gregorianDate);
    EXPECT_EQ(2439323, julianDate);
    gregorianDate = Weather::computeGregorianDate(julianDate);
    EXPECT_EQ(1966, gregorianDate.year);
    EXPECT_EQ(7, gregorianDate.month);
    EXPECT_EQ(16, gregorianDate.day);

    gregorianDate = {2000, 12, 31}; // complex leap year
    julianDate = computeJulianDate(gregorianDate);
    EXPECT_EQ(2451910, julianDate);
    gregorianDate = Weather::computeGregorianDate(julianDate);
    EXPECT_EQ(2000, gregorianDate.year);
    EXPECT_EQ(12, gregorianDate.month);
    EXPECT_EQ(31, gregorianDate.day);
}

TEST_F(EnergyPlusFixture, interpolateWindDirectionTest)
{
    // simple test in each quadrant
    EXPECT_EQ(Weather::interpolateWindDirection(0, 90, 0.5), 45.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 80, 0.5), 45.);
    EXPECT_EQ(Weather::interpolateWindDirection(20, 80, 0.7), 62.);
    EXPECT_EQ(Weather::interpolateWindDirection(20, 80, 0.3), 38.);

    EXPECT_EQ(Weather::interpolateWindDirection(90, 180, 0.5), 135.);
    EXPECT_EQ(Weather::interpolateWindDirection(100, 170, 0.5), 135.);
    EXPECT_EQ(Weather::interpolateWindDirection(110, 170, 0.7), 152.);
    EXPECT_EQ(Weather::interpolateWindDirection(110, 170, 0.3), 128.);

    EXPECT_EQ(Weather::interpolateWindDirection(180, 270, 0.5), 225.);
    EXPECT_EQ(Weather::interpolateWindDirection(190, 260, 0.5), 225.);
    EXPECT_EQ(Weather::interpolateWindDirection(200, 260, 0.7), 242.);
    EXPECT_EQ(Weather::interpolateWindDirection(200, 260, 0.3), 218.);

    EXPECT_EQ(Weather::interpolateWindDirection(270, 360, 0.5), 315.);
    EXPECT_EQ(Weather::interpolateWindDirection(280, 350, 0.5), 315.);
    EXPECT_EQ(Weather::interpolateWindDirection(290, 350, 0.7), 332.);
    EXPECT_EQ(Weather::interpolateWindDirection(290, 350, 0.3), 308.);

    // tests across 180 degree angle
    EXPECT_EQ(Weather::interpolateWindDirection(170, 190, 0.7), 184.);
    EXPECT_EQ(Weather::interpolateWindDirection(170, 190, 0.3), 176.);
    EXPECT_EQ(Weather::interpolateWindDirection(100, 260, 0.7), 212.);
    EXPECT_EQ(Weather::interpolateWindDirection(100, 260, 0.3), 148.);

    // tests across 0 degree angle (which was issue #5682)
    EXPECT_EQ(Weather::interpolateWindDirection(350, 10, 0.7), 4.);
    EXPECT_EQ(Weather::interpolateWindDirection(350, 10, 0.3), 356.);
    EXPECT_EQ(Weather::interpolateWindDirection(300, 80, 0.7), 38.);
    EXPECT_EQ(Weather::interpolateWindDirection(300, 80, 0.3), 342.);

    EXPECT_EQ(Weather::interpolateWindDirection(350, 10, 0.5), 0.);
    EXPECT_EQ(Weather::interpolateWindDirection(340, 10, 0.5), 355.);
    EXPECT_EQ(Weather::interpolateWindDirection(280, 10, 0.5), 325.);
    EXPECT_EQ(Weather::interpolateWindDirection(260, 10, 0.5), 315.);
    EXPECT_EQ(Weather::interpolateWindDirection(200, 10, 0.7), 319.);
    EXPECT_EQ(Weather::interpolateWindDirection(200, 10, 0.3), 251.);
    EXPECT_EQ(Weather::interpolateWindDirection(350, 160, 0.7), 109.);
    EXPECT_EQ(Weather::interpolateWindDirection(350, 160, 0.3), 41.);

    // tests for new failures
    EXPECT_EQ(Weather::interpolateWindDirection(70, 30, 0.25), 60.);

    // tests across 180 degree angle (reversed)
    EXPECT_EQ(Weather::interpolateWindDirection(190, 170, 0.3), 184.);
    EXPECT_EQ(Weather::interpolateWindDirection(190, 170, 0.7), 176.);
    EXPECT_EQ(Weather::interpolateWindDirection(260, 100, 0.3), 212.);
    EXPECT_EQ(Weather::interpolateWindDirection(260, 100, 0.7), 148.);

    // tests across 0 degree angle (reversed)
    EXPECT_EQ(Weather::interpolateWindDirection(10, 350, 0.3), 4.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 350, 0.7), 356.);
    EXPECT_EQ(Weather::interpolateWindDirection(80, 300, 0.3), 38.);
    EXPECT_EQ(Weather::interpolateWindDirection(80, 300, 0.7), 342.);

    EXPECT_EQ(Weather::interpolateWindDirection(10, 350, 0.5), 0.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 340, 0.5), 355.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 280, 0.5), 325.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 260, 0.5), 315.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 200, 0.3), 319.);
    EXPECT_EQ(Weather::interpolateWindDirection(10, 200, 0.7), 251.);
    EXPECT_EQ(Weather::interpolateWindDirection(160, 350, 0.3), 109.);
    EXPECT_EQ(Weather::interpolateWindDirection(160, 350, 0.7), 41.);
}

TEST_F(EnergyPlusFixture, UnderwaterBoundaryConditionFullyPopulated)
{

    std::string const idf_objects =
        delimited_string({"SurfaceProperty:Underwater, UnderwaterSurfaceName, 31.4159, WaterTempSchedule, WaterVelocitySchedule;",
                          "Schedule:Constant, WaterTempSchedule, , 30;",
                          "Schedule:Constant, WaterVelocitySchedule, , 3.0;"
                          "SurfaceProperty:OtherSideConditionsModel, UnderwaterSurfaceName, ConvectiveUnderwater;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // need to populate the OSCM array by calling the get input for it
    bool errorsFound = false;
    SurfaceGeometry::GetOSCMData(*state, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_EQ(state->dataSurface->TotOSCM, 1);

    // then process the input for this underwater surface
    bool shouldBeTrue = Weather::CheckIfAnyUnderwaterBoundaries(*state);
    EXPECT_TRUE(shouldBeTrue);
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].Name, "UNDERWATERSURFACENAME");
    EXPECT_NEAR(state->dataWeather->underwaterBoundaries[0].distanceFromLeadingEdge, 31.4159, 0.0001);
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].OSCMIndex, 1);
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].waterTempSched->Name, "WATERTEMPSCHEDULE");
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].velocitySched->Name, "WATERVELOCITYSCHEDULE");
}

TEST_F(EnergyPlusFixture, UnderwaterBoundaryConditionMissingVelocityOK)
{

    std::string const idf_objects = delimited_string({"SurfaceProperty:Underwater, UnderwaterSurfaceName, 31.4159, WaterTempSchedule, ;",
                                                      "Schedule:Constant, WaterTempSchedule, , 30;",
                                                      "SurfaceProperty:OtherSideConditionsModel, UnderwaterSurfaceName, ConvectiveUnderwater;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // need to populate the OSCM array by calling the get input for it
    bool errorsFound = false;
    SurfaceGeometry::GetOSCMData(*state, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_EQ(state->dataSurface->TotOSCM, 1);

    // then process the input for this underwater surface
    bool shouldBeTrue = Weather::CheckIfAnyUnderwaterBoundaries(*state);
    EXPECT_TRUE(shouldBeTrue);
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].Name, "UNDERWATERSURFACENAME");
    EXPECT_NEAR(state->dataWeather->underwaterBoundaries[0].distanceFromLeadingEdge, 31.4159, 0.0001);
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].OSCMIndex, 1);
    EXPECT_NE(state->dataWeather->underwaterBoundaries[0].waterTempSched, nullptr);
    EXPECT_EQ(state->dataWeather->underwaterBoundaries[0].velocitySched, nullptr);
}

TEST_F(EnergyPlusFixture, UnderwaterBoundaryConditionConvectionCoefficients)
{
    EXPECT_NEAR(2483.702, Weather::calculateWaterBoundaryConvectionCoefficient(30.0, 3.0, 30.0), 0.01);
    EXPECT_NEAR(2162.188, Weather::calculateWaterBoundaryConvectionCoefficient(30.0, 3.0, 60.0), 0.01);
    EXPECT_NEAR(1993.771, Weather::calculateWaterBoundaryConvectionCoefficient(30.0, 3.0, 90.0), 0.01);
    EXPECT_NEAR(1882.294, Weather::calculateWaterBoundaryConvectionCoefficient(30.0, 3.0, 120.0), 0.01);
    EXPECT_NEAR(1800.136, Weather::calculateWaterBoundaryConvectionCoefficient(30.0, 3.0, 150.0), 0.01);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationFromWeatherFileTest)
{

    std::string const idf_objects = delimited_string({
        "   Site:WaterMainsTemperature,",
        "   CorrelationFromWeatherFile,  !- Calculation Method",
        "   ,                            !- Temperature Schedule Name",
        "   9.99,                        !- Annual Average Outdoor Air Temperature {C}",
        "  28.78;                        !- Maximum Difference In Monthly Average Outdoor Air Temperatures {deltaC}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool foundErrors(false);
    Weather::GetWaterMainsTemperatures(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_ENUM_EQ(state->dataWeather->WaterMainsTempsMethod, Weather::WaterMainsTempCalcMethod::CorrelationFromWeatherFile);
    // for calculation method CorrelationFromWeatherFile these parameters are ignored
    EXPECT_EQ(state->dataWeather->WaterMainsTempsAnnualAvgAirTemp, 0.0);
    EXPECT_EQ(state->dataWeather->WaterMainsTempsMaxDiffAirTemp, 0.0);

    // set water mains parameters for CorrelationFromWeatherFile method
    state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp = 9.99;
    state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff = 28.78;
    state->dataWeather->OADryBulbAverage.OADryBulbWeatherDataProcessed = true;
    state->dataEnvrn->Latitude = 42.00; // CHICAGO_IL_USA_WMO_725300

    // January 15th water mains temperature test
    state->dataEnvrn->DayOfYear = 15; // January 15th
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 7.5145, 0.0001);

    // July 15th water mains temperature test
    state->dataEnvrn->DayOfYear = 196; // July 15th
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 19.0452, 0.0001);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationFromWeatherFileTest_Actual)
{

    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/ThermalComfortCEN15251Test.epw";

    std::string const idf_objects = delimited_string({
        "   Site:WaterMainsTemperature,",
        "   CorrelationFromWeatherFile,  !- Calculation Method",
        "   ,                            !- Temperature Schedule Name",
        "   9.99,                        !- Annual Average Outdoor Air Temperature {C}",
        "  28.78;                        !- Maximum Difference In Monthly Average Outdoor Air Temperatures {deltaC}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool foundErrors(false);
    Weather::GetWaterMainsTemperatures(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_ENUM_EQ(state->dataWeather->WaterMainsTempsMethod, Weather::WaterMainsTempCalcMethod::CorrelationFromWeatherFile);
    // for calculation method CorrelationFromWeatherFile these parameters are ignored
    EXPECT_EQ(state->dataWeather->WaterMainsTempsAnnualAvgAirTemp, 0.0);
    EXPECT_EQ(state->dataWeather->WaterMainsTempsMaxDiffAirTemp, 0.0);

    EXPECT_TRUE(state->dataWeather->WaterMainsParameterReport);

    // CalcAnnualAndMonthlyDryBulbTemp was the one that was faulty
    state->dataWeather->OADryBulbAverage.CalcAnnualAndMonthlyDryBulbTemp(*state);

    EXPECT_TRUE(state->dataWeather->OADryBulbAverage.OADryBulbWeatherDataProcessed);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp, 7.31, 0.01);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff, 27.94, 0.01);

    // January 15th water mains temperature test
    state->dataEnvrn->DayOfYear = 15; // January 15th
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 5.8439, 0.0001);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationFromStatFileTest)
{

    int AnnualNumberOfDays(0);
    Real64 MonthlyDailyDryBulbMin(0.0);
    Real64 MonthlyDailyDryBulbMax(0.0);
    Real64 AnnualDailyAverageDryBulbTempSum(0.0);

    std::string const idf_objects = delimited_string({
        "   Site:WaterMainsTemperature,",
        "   CorrelationFromWeatherFile,  !- Calculation Method",
        "   ,                            !- Temperature Schedule Name",
        "   9.99,                        !- Annual Average Outdoor Air Temperature {C}",
        "  28.78;                        !- Maximum Difference In Monthly Average Outdoor Air Temperatures {deltaC}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool foundErrors(false);
    Weather::GetWaterMainsTemperatures(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_ENUM_EQ(state->dataWeather->WaterMainsTempsMethod, Weather::WaterMainsTempCalcMethod::CorrelationFromWeatherFile);
    // for calculation method CorrelationFromWeatherFile these parameters are ignored
    EXPECT_EQ(state->dataWeather->WaterMainsTempsAnnualAvgAirTemp, 0.0);
    EXPECT_EQ(state->dataWeather->WaterMainsTempsMaxDiffAirTemp, 0.0);

    Array1D<Real64> MonthlyDryBulbTempFromStatFile(12, {-4.60, -2.50, 3.80, 10.00, 15.30, 21.10, 24.10, 21.80, 18.10, 11.00, 4.70, -3.70});
    state->dataWeather->OADryBulbAverage.MonthlyDailyAverageDryBulbTemp = MonthlyDryBulbTempFromStatFile;

    // calc water mains parameters for CorrelationFromWeatherFile method
    for (int i = 1; i <= 12; ++i) {
        AnnualDailyAverageDryBulbTempSum +=
            state->dataWeather->OADryBulbAverage.MonthlyDailyAverageDryBulbTemp(i) * state->dataWeather->EndDayOfMonth(i);
        MonthlyDailyDryBulbMin = min(MonthlyDailyDryBulbMin, state->dataWeather->OADryBulbAverage.MonthlyDailyAverageDryBulbTemp(i));
        MonthlyDailyDryBulbMax = max(MonthlyDailyDryBulbMax, state->dataWeather->OADryBulbAverage.MonthlyDailyAverageDryBulbTemp(i));
        AnnualNumberOfDays += state->dataWeather->EndDayOfMonth(i);
    }
    state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp = AnnualDailyAverageDryBulbTempSum / AnnualNumberOfDays;
    state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff = MonthlyDailyDryBulbMax - MonthlyDailyDryBulbMin;
    // check results
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp, 9.9882, 0.0001);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff, 28.7000, 0.0001);

    // test water mains temperature
    // Weather::WaterMainsTempsMethod = Weather::CorrelationFromWeatherFileMethod;
    state->dataWeather->OADryBulbAverage.OADryBulbWeatherDataProcessed = true;
    state->dataEnvrn->Latitude = 42.00; // CHICAGO_IL_USA_WMO_725300

    // January 21st water mains temperature test
    state->dataEnvrn->DayOfYear = 21; // January 21st
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 7.23463, 0.00001);

    // July 21st water mains temperature test
    state->dataEnvrn->DayOfYear = 202; // July 21st
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 19.33812, 0.00001);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationFromStatFileTest_Actual)
{

    state->files.inStatFilePath.filePath =
        configured_source_directory() / "tst/EnergyPlus/unit/Resources/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.stat";

    std::string const idf_objects = delimited_string({
        "   Site:WaterMainsTemperature,",
        "   CorrelationFromWeatherFile;  !- Calculation Method",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool foundErrors(false);
    Weather::GetWaterMainsTemperatures(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_ENUM_EQ(state->dataWeather->WaterMainsTempsMethod, Weather::WaterMainsTempCalcMethod::CorrelationFromWeatherFile);
    // for calculation method CorrelationFromWeatherFile these parameters are ignored
    EXPECT_EQ(state->dataWeather->WaterMainsTempsAnnualAvgAirTemp, 0.0);
    EXPECT_EQ(state->dataWeather->WaterMainsTempsMaxDiffAirTemp, 0.0);

    EXPECT_TRUE(state->dataWeather->WaterMainsParameterReport);

    // CalcAnnualAndMonthlyDryBulbTemp was the one that was faulty
    state->dataWeather->OADryBulbAverage.CalcAnnualAndMonthlyDryBulbTemp(*state);

    std::array<int, 12> nDaysInMonth{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    std::array<double, 12> monthlyDryBulbTemps{-4.6, -2.5, 3.8, 10.0, 15.3, 21.1, 24.1, 21.8, 18.1, 11.0, 4.7, -3.7};
    int totDays = std::accumulate(nDaysInMonth.begin(), nDaysInMonth.end(), 0);
    double annualAvgOADryBulbTemp =
        std::inner_product(std::begin(nDaysInMonth), std::end(nDaysInMonth), std::begin(monthlyDryBulbTemps), 0.0) / totDays;

    const auto [min, max] = std::minmax_element(std::begin(monthlyDryBulbTemps), std::end(monthlyDryBulbTemps));
    double monthlyAvgOADryBulbTempMaxDiff = (*max) - (*min);

    EXPECT_TRUE(state->dataWeather->OADryBulbAverage.OADryBulbWeatherDataProcessed);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp, 9.988219178082193, 0.01);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff, 28.7, 0.01);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp, annualAvgOADryBulbTemp, 0.01);
    EXPECT_NEAR(state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff, monthlyAvgOADryBulbTempMaxDiff, 0.01);

    // January 15th water mains temperature test
    state->dataEnvrn->DayOfYear = 15; // January 15th
    Weather::CalcWaterMainsTemp(*state);
    EXPECT_NEAR(state->dataEnvrn->WaterMainsTemp, 7.5295, 0.0001);
}

TEST_F(EnergyPlusFixture, WaterMainsCorrelationFromStatFileTest_ActualBroken)
{

    state->files.inStatFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/broken.stat";

    std::string const idf_objects = delimited_string({
        "   Site:WaterMainsTemperature,",
        "   CorrelationFromWeatherFile;  !- Calculation Method",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool foundErrors(false);
    Weather::GetWaterMainsTemperatures(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_ENUM_EQ(state->dataWeather->WaterMainsTempsMethod, Weather::WaterMainsTempCalcMethod::CorrelationFromWeatherFile);
    // for calculation method CorrelationFromWeatherFile these parameters are ignored
    EXPECT_EQ(state->dataWeather->WaterMainsTempsAnnualAvgAirTemp, 0.0);
    EXPECT_EQ(state->dataWeather->WaterMainsTempsMaxDiffAirTemp, 0.0);

    EXPECT_TRUE(state->dataWeather->WaterMainsParameterReport);

    // CalcAnnualAndMonthlyDryBulbTemp was the one that was faulty
    state->dataWeather->OADryBulbAverage.CalcAnnualAndMonthlyDryBulbTemp(*state);
    EXPECT_FALSE(state->dataWeather->OADryBulbAverage.OADryBulbWeatherDataProcessed);
    std::string const error_string = delimited_string({
        "   ** Severe  ** CalcAnnualAndMonthlyDryBulbTemp: Stat file '" + state->files.inStatFilePath.filePath.string() +
            "' does not have Monthly Statistics for Dry Bulb "
            "temperatures.",
        "   **   ~~~   ** Water Mains Temperature will be set to a fixed default value of 10.0 C.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, WaterMainsOutputReports_CorrelationFromWeatherFileTest)
{

    std::string const idf_objects = delimited_string({
        "   Site:WaterMainsTemperature,",
        "   CorrelationFromWeatherFile,  !- Calculation Method",
        "   ,                            !- Temperature Schedule Name",
        "   9.99,                        !- Annual Average Outdoor Air Temperature {C}",
        "  28.78;                        !- Maximum Difference In Monthly Average Outdoor Air Temperatures {deltaC}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    compare_eio_stream_substring("", true);
    bool foundErrors(false);
    Weather::GetWaterMainsTemperatures(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_ENUM_EQ(state->dataWeather->WaterMainsTempsMethod, Weather::WaterMainsTempCalcMethod::CorrelationFromWeatherFile);
    // for calculation method CorrelationFromWeatherFile these two parameters are ignored
    EXPECT_EQ(state->dataWeather->WaterMainsTempsAnnualAvgAirTemp, 0.0);
    EXPECT_EQ(state->dataWeather->WaterMainsTempsMaxDiffAirTemp, 0.0);

    // set water mains temp parameters for CorrelationFromWeatherFile method
    state->dataWeather->OADryBulbAverage.AnnualAvgOADryBulbTemp = 9.99;
    state->dataWeather->OADryBulbAverage.MonthlyAvgOADryBulbTempMaxDiff = 28.78;
    state->dataWeather->OADryBulbAverage.OADryBulbWeatherDataProcessed = true;

    // report water mains parameters to eio file
    Weather::ReportWaterMainsTempParameters(*state);

    std::string const eiooutput = delimited_string({"! <Site Water Mains Temperature Information>,"
                                                    "Calculation Method{},"
                                                    "Water Mains Temperature Schedule Name{},"
                                                    "Annual Average Outdoor Air Temperature{C},"
                                                    "Maximum Difference In Monthly Average Outdoor Air Temperatures{deltaC},"
                                                    "Fixed Default Water Mains Temperature{C}",
                                                    "Site Water Mains Temperature Information,CorrelationFromWeatherFile,NA,9.99,28.78,NA"},
                                                   "\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}
TEST_F(EnergyPlusFixture, ASHRAE_Tau2017ModelTest)
{
    std::string const idf_objects = delimited_string({

        "  SizingPeriod:DesignDay,",
        "    Atlanta Jan 21 cooling,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    16.9,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.6,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    13.2,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    8,                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    0.0,                     !- Wind Speed {m/s}",
        "    0.0,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    0.325,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    2.461;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",

        "  SizingPeriod:DesignDay,",
        "    Atlanta Jul 21 cooling,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    33.3,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.5,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    23.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    3.5,                     !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    0.0,                     !- Wind Speed {m/s}",
        "    0.0,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    .556,                    !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    1.779;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataEnvrn->TotDesDays = 2;
    state->dataGlobal->TimeStepsInHour = 0;
    // setup environment state
    state->dataWeather->Environment.allocate(state->dataEnvrn->TotDesDays);
    state->dataWeather->DesignDay.allocate(state->dataEnvrn->TotDesDays);
    state->dataWeather->Environment(1).DesignDayNum = 1;
    state->dataWeather->Environment(2).DesignDayNum = 2;
    Weather::GetDesignDayData(*state, state->dataEnvrn->TotDesDays, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    // init local variables
    Real64 ETR = 1367.0;
    Real64 BeamRad(0.0);
    Real64 DiffRad(0.0);
    Real64 GloHorzRad(0.0);

    // EnvrnNum = 1 uses Tau values of January
    int EnvrnNum = 1;
    Real64 CosZenith = 1.0; // assumed zero zenith angle
    Real64 TauB = state->dataWeather->DesDayInput(EnvrnNum).TauB;
    Real64 TauD = state->dataWeather->DesDayInput(EnvrnNum).TauD;
    // check tau values
    EXPECT_ENUM_EQ(Weather::DesDaySolarModel::ASHRAE_Tau2017, state->dataWeather->DesDayInput(EnvrnNum).solarModel);
    EXPECT_EQ(0.325, TauB);
    EXPECT_EQ(2.461, TauD);
    // calc expected values for environment 1
    Real64 AB = 1.454 - 0.406 * TauB - 0.268 * TauD + 0.021 * TauB * TauD;
    Real64 AD = 0.507 + 0.205 * TauB - 0.080 * TauD - 0.190 * TauB * TauD;
    Real64 M = Weather::AirMass(CosZenith);
    Real64 expectedIDirN = ETR * std::exp(-TauB * std::pow(M, AB));
    Real64 expectedIDifH = ETR * std::exp(-TauD * std::pow(M, AD));
    Real64 expectedIGlbH = expectedIDirN * CosZenith + expectedIDifH;
    // calc TauModel
    ASHRAETauModel(*state, state->dataWeather->DesDayInput(EnvrnNum).solarModel, ETR, CosZenith, TauB, TauD, BeamRad, DiffRad, GloHorzRad);
    // check the coefficients are correctly applied
    EXPECT_EQ(expectedIDirN, BeamRad);
    EXPECT_EQ(expectedIDifH, DiffRad);
    EXPECT_EQ(expectedIGlbH, GloHorzRad);

    // EnvrnNum = 2 uses Tau values of July
    EnvrnNum = 2;
    CosZenith = 1.0; // assumed zero zenith angle
    TauB = state->dataWeather->DesDayInput(EnvrnNum).TauB;
    TauD = state->dataWeather->DesDayInput(EnvrnNum).TauD;
    // check tau values
    EXPECT_EQ(0.556, TauB);
    EXPECT_EQ(1.779, TauD);
    // calc expected values for environment 2
    AB = 1.454 - 0.406 * TauB - 0.268 * TauD + 0.021 * TauB * TauD;
    AD = 0.507 + 0.205 * TauB - 0.080 * TauD - 0.190 * TauB * TauD;
    M = Weather::AirMass(CosZenith);
    expectedIDirN = ETR * std::exp(-TauB * std::pow(M, AB));
    expectedIDifH = ETR * std::exp(-TauD * std::pow(M, AD));
    expectedIGlbH = expectedIDirN * CosZenith + expectedIDifH;
    // reset the arguments to zero
    BeamRad = 0.0;
    DiffRad = 0.0;
    GloHorzRad = 0.0;
    // calc TauModel
    ASHRAETauModel(*state, state->dataWeather->DesDayInput(EnvrnNum).solarModel, ETR, CosZenith, TauB, TauD, BeamRad, DiffRad, GloHorzRad);
    // check the coefficients are correctly applied
    EXPECT_EQ(expectedIDirN, BeamRad);
    EXPECT_EQ(expectedIDifH, DiffRad);
    EXPECT_EQ(expectedIGlbH, GloHorzRad);
}

TEST_F(EnergyPlusFixture, WeatherManager_NoLocation)
{

    // GetNextEnvironment Will call ReadUserWeatherInput which calls inputProcessor, so let's use process_idf to create one Environment (Design Day)
    std::string const idf_objects = delimited_string({

        "  SizingPeriod:DesignDay,",
        "    Atlanta Jan 21 cooling,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    16.9,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.6,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    13.2,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    8,                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    0.0,                     !- Wind Speed {m/s}",
        "    0.0,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    0.325,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    2.461;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}"

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->LocationGathered = false;

    bool Available{false};
    bool ErrorsFound{false};
    ASSERT_THROW(Weather::GetNextEnvironment(*state, Available, ErrorsFound), std::runtime_error);
    ASSERT_TRUE(ErrorsFound);

    std::string const error_string = delimited_string({
        "   ** Severe  ** No Location given. Must have location information for simulation.",
        "   ** Warning ** Did you realize that you have Latitude=0.0, Longitude=0.0 and TimeZone=0.0?  Your building site is in the middle of the "
        "Atlantic Ocean.",
        "   ** Severe  ** GetNextEnvironment: No location specified, program will terminate.",
        "   **  Fatal  ** GetNextEnvironment: Errors found in Weather Data Input. Program terminates.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=2",
        "   ..... Last severe error=GetNextEnvironment: No location specified, program will terminate.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
    EXPECT_EQ(1, state->dataWeather->NumOfEnvrn);
    EXPECT_ENUM_EQ(state->dataWeather->Environment(1).KindOfEnvrn, Constant::KindOfSim::DesignDay);
}

// Test for https://github.com/NREL/EnergyPlus/issues/7550
TEST_F(SQLiteFixture, DesignDay_EnthalpyAtMaxDB)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->WriteTabularFiles = true;
    state->dataOutRptTab->displayEioSummary = true;

    std::string const idf_objects = delimited_string({

        "Site:Location,",
        "  Changsha_Hunan_CHN Design_Conditions,   !- Location Name",
        "  28.22,                                  !- Latitude {N+ S-}",
        "  112.92,                                 !- Longitude {W- E+}",
        "  8.00,                                   !- Time Zone Relative to GMT {GMT+/-}",
        "  68.00;                                  !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        "  Changsha Ann Clg .4% Condns Enth=>MDB,  !- Name",
        "  7,                                      !- Month",
        "  21,                                     !- Day of Month",
        "  SummerDesignDay,                        !- Day Type",
        "  33,                                     !- Maximum Dry-Bulb Temperature {C}",
        "  6.6,                                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  DefaultMultipliers,                     !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                                       !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Enthalpy,                               !- Humidity Condition Type",
        "  ,                                       !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                                       !- Humidity Condition Day Schedule Name",
        "  ,                                       !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  90500.0,                                !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  100511.,                                !- Barometric Pressure {Pa}",
        "  3.2,                                    !- Wind Speed {m/s}",
        "  220,                                    !- Wind Direction {deg}",
        "  No,                                     !- Rain Indicator",
        "  No,                                     !- Snow Indicator",
        "  No,                                     !- Daylight Saving Time Indicator",
        "  ASHRAETau,                              !- Solar Model Indicator",
        "  ,                                       !- Beam Solar Day Schedule Name",
        "  ,                                       !- Diffuse Solar Day Schedule Name",
        "  0.773,                                  !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  1.428;                                  !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::OpenOutputFiles(*state);
    // reset eio stream
    has_eio_output(true);

    bool ErrorsFound(false);
    state->dataEnvrn->TotDesDays = 1;
    // setup environment state
    state->dataWeather->Environment.allocate(state->dataEnvrn->TotDesDays);
    state->dataWeather->DesignDay.allocate(state->dataEnvrn->TotDesDays);

    state->dataWeather->Environment(1).DesignDayNum = 1;
    state->dataWeather->Environment(1).WP_Type1 = 0;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->BeginSimFlag = true;
    state->dataReportFlag->DoWeatherInitReporting = true;

    Weather::SetupInterpolationValues(*state);
    Weather::AllocateWeatherData(*state);

    Weather::GetDesignDayData(*state, state->dataEnvrn->TotDesDays, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    Weather::SetUpDesignDay(*state, 1);
    EXPECT_ENUM_EQ(state->dataWeather->DesDayInput(1).HumIndType, Weather::DesDayHumIndType::Enthalpy);
    EXPECT_EQ(state->dataWeather->DesDayInput(1).HumIndValue, 90500.0);

    unsigned n_RH_not100 = 0;
    for (int Hour = 1; Hour <= 24; ++Hour) {
        for (int TS = 1; TS <= state->dataGlobal->TimeStepsInHour; ++TS) {
            EXPECT_GE(state->dataWeather->wvarsHrTsTomorrow(TS, Hour).OutRelHum, 0.);
            EXPECT_LE(state->dataWeather->wvarsHrTsTomorrow(TS, Hour).OutRelHum, 100.);
            if (state->dataWeather->wvarsHrTsTomorrow(TS, Hour).OutRelHum < 100.) {
                ++n_RH_not100;
            }
        }
    }
    EXPECT_TRUE(n_RH_not100 > 0) << "Expected at least one hour with RH below 100%";

    // This actually doesn't end up in the EIO stream yet, it's written to a gio::out_stream
    // That's why I used SQLiteFixture instead
    std::string const eiooutput = delimited_string(
        {
            "! <Environment:Design Day Data>, Max Dry-Bulb Temp {C}, Temp Range {dC}, Temp Range Ind Type, Hum Ind Type, Hum Ind Value at Max Temp, "
            "Hum Ind Units, Pressure {Pa}, Wind Direction {deg CW from N}, Wind Speed {m/s}, Clearness, Rain, Snow",
            "! <Environment:Design Day Misc>,DayOfYear,ASHRAE A Coeff,ASHRAE B Coeff,ASHRAE C Coeff,Solar Constant-Annual Variation,Eq of Time "
            "{minutes}, Solar Declination Angle {deg}, Solar Model",
            "Environment:Design Day Data,33.00,6.60,DefaultMultipliers,Enthalpy,90500.00,{J/kgDryAir},100511,220,3.2,0.00,No,No",
            "Environment:Design Day Misc,202,1084.4,0.2082,0.1365,1.0,-6.23,20.6,ASHRAETau",
        },
        "\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, false));

    OutputReportTabular::WriteEioTables(*state);

    // Close output files *after* the EIO has been written to
    SimulationManager::CloseOutputFiles(*state);

    std::vector<std::tuple<std::string, std::string>> results_strings(
        {{"Hum Ind Value at Max Temp", "90500.00"}, {"Hum Ind Type", "Enthalpy"}, {"Hum Ind Units", "{J/kgDryAir}"}});

    std::string columnName;
    std::string expectedValue;
    for (auto v : results_strings) {

        columnName = std::get<0>(v);
        expectedValue = std::get<1>(v);

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = 'InitializationSummary'"
                          "  AND TableName = 'Environment:Design Day Data'"
                          "  AND ColumnName = '" +
                          columnName + "'");

        std::string value = queryResult(query, "TabularDataWithStrings")[0][0];

        // Add informative message if failed
        EXPECT_EQ(value, expectedValue) << "Failed for ColumnName=" << columnName;
    }
}

TEST_F(EnergyPlusFixture, IRHoriz_InterpretWeatherZeroIRHoriz)
{

    std::vector<std::string> Lines{
        "1980,1,1,1,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,2,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,3,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,4,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,5,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,6,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,7,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,8,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,9,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,10,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
        "1980,1,1,11,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0,-8.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,0,"
        "99,0,0,0",
        "1980,1,1,12,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0,-8.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,0,"
        "99,0,0,0",
        "1980,1,1,13,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0,-8.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,0,"
        "99,0,0,0",
        "1980,1,1,14,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0.6,-7.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,15,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0.6,-7.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,16,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0.6,-7.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,17,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9?9*9*9*9*9*9,0.6,-7.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,"
        "0,99,0,0,0",
        "1980,1,1,18,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,0,-8.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0.04,0,"
        "99,0,0,0",
        "1980,1,1,19,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
        "1980,1,1,20,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-0.6,-8.7,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
        "1980,1,1,21,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
        "1980,1,1,22,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
        "1980,1,1,23,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
        "1980,1,1,24,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9?9*_*9*9*9*9*9,-1.1,-9.2,50,100000,0,0,0,0,0,0,0,0,0,0,0,2,0,0,10,77777,9,999999999,0,0."
        "04,0,99,0,0,0",
    };

    // DERIVED TYPE DEFINITIONS:
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int WYear;
    int WMonth;
    int WDay;
    int WHour;
    int WMinute;
    Real64 DryBulb;
    Real64 DewPoint;
    Real64 RelHum;
    Real64 AtmPress;
    Real64 ETHoriz;
    Real64 ETDirect;
    Real64 IRHoriz;
    Real64 GLBHoriz;
    Real64 DirectRad;
    Real64 DiffuseRad;
    Real64 GLBHorizIllum;
    Real64 DirectNrmIllum;
    Real64 DiffuseHorizIllum;
    Real64 ZenLum;
    Real64 WindDir;
    Real64 WindSpeed;
    Real64 TotalSkyCover;
    Real64 OpaqueSkyCover;
    Real64 Visibility;
    Real64 CeilHeight;
    Real64 PrecipWater;
    Real64 AerosolOptDepth;
    Real64 SnowDepth;
    Real64 DaysSinceLastSnow;
    Real64 Albedo;
    Real64 LiquidPrecip;
    int PresWeathObs;
    Array1D_int PresWeathConds(9);
    std::string WeatherDataLine;
    bool ErrorFound;
    std::string ErrOut;

    for (auto WeatherDataLine : Lines) {
        Weather::InterpretWeatherDataLine(*state,
                                          WeatherDataLine,
                                          ErrorFound,
                                          WYear,
                                          WMonth,
                                          WDay,
                                          WHour,
                                          WMinute,
                                          DryBulb,
                                          DewPoint,
                                          RelHum,
                                          AtmPress,
                                          ETHoriz,
                                          ETDirect,
                                          IRHoriz,
                                          GLBHoriz,
                                          DirectRad,
                                          DiffuseRad,
                                          GLBHorizIllum,
                                          DirectNrmIllum,
                                          DiffuseHorizIllum,
                                          ZenLum,
                                          WindDir,
                                          WindSpeed,
                                          TotalSkyCover,
                                          OpaqueSkyCover,
                                          Visibility,
                                          CeilHeight,
                                          PresWeathObs,
                                          PresWeathConds,
                                          PrecipWater,
                                          AerosolOptDepth,
                                          SnowDepth,
                                          DaysSinceLastSnow,
                                          Albedo,
                                          LiquidPrecip);

        EXPECT_EQ(IRHoriz, 0.0);
    }
}

TEST_F(EnergyPlusFixture, IRHoriz_InterpretWeatherCalculateMissingIRHoriz)
{

    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/WeatherManagerIROutputTest.epw";
    std::string const idf_objects = delimited_string({
        "  Version,9.3;",

        "  SizingPeriod:DesignDay,",
        "    Atlanta Jan 21 cooling,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    16.9,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.6,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    13.2,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    8,                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    0.0,                     !- Wind Speed {m/s}",
        "    0.0,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    0.325,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    2.461;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",

        "  SizingPeriod:DesignDay,",
        "    Atlanta Jul 21 cooling,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    33.3,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.5,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    23.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    3.5,                     !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    0.0,                     !- Wind Speed {m/s}",
        "    0.0,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    .556,                    !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    1.779;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataEnvrn->TotDesDays = 2;
    state->dataGlobal->TimeStepsInHour = 0;

    // setup environment state
    state->dataWeather->Environment.allocate(state->dataEnvrn->TotDesDays);
    state->dataWeather->DesignDay.allocate(state->dataEnvrn->TotDesDays);
    state->dataWeather->Environment(1).DesignDayNum = 1;
    state->dataWeather->Environment(2).DesignDayNum = 2;
    Weather::GetDesignDayData(*state, state->dataEnvrn->TotDesDays, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    state->dataWeather->Envrn = 1;

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataWeather->Environment.allocate(1);
    state->dataWeather->Environment(1).skyTempModel = Weather::SkyTempModel::ClarkAllen;

    Weather::AllocateWeatherData(*state);
    Weather::OpenWeatherFile(*state, ErrorsFound);
    Weather::ReadWeatherForDay(*state, 0, 1, false);

    Real64 expected_IRHorizSky = 345.73838855245953;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).HorizIRSky, expected_IRHorizSky, 0.001);
}

// Test for Issue 7957: add new sky cover weather output values;
// and test for Issue 8030: interpolate some weather input first before output values.
TEST_F(EnergyPlusFixture, Add_and_InterpolateWeatherInputOutputTest)
{
    std::string const idf_objects = delimited_string({
        "Timestep,4;"

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  Yes,                     !- Run Simulation for Sizing Periods",
        "  No;                      !- Run Simulation for Weather File Run Periods",

        "RunPeriod,",
        "  January,                 !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  1,                       !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "Site:Location,",
        "  CHICAGO_IL_USA TMY2-94846,  !- Name",
        "  41.78,                   !- Latitude {deg}",
        "  -87.75,                  !- Longitude {deg}",
        "  -6.00,                   !- Time Zone {hr}",
        "  190.00;                  !- Elevation {m}",

        "Output:Variable,",
        "*,",
        "Site Outdoor Air Drybulb Temperature,",
        "Timestep;",
        "Output:Variable,*,Site Wind Speed,Timestep;",
        "Output:Variable,*,Site Total Sky Cover,Timestep;",
        "Output:Variable,*,Site Opaque Sky Cover,Timestep;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::PostIPProcessing(*state);
    bool ErrorsFound(false);
    ErrorsFound = false;

    state->init_state(*state);

    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);

    bool Available(true);
    Available = true;

    state->dataGlobal->BeginSimFlag = true;
    Weather::GetNextEnvironment(*state, Available, ErrorsFound);

    // Test get output variables for Total Sky Cover and Opaque Sky Cover
    EXPECT_EQ("Site Outdoor Air Drybulb Temperature", state->dataOutputProcessor->outVars[0]->name);
    EXPECT_EQ("Environment:Site Outdoor Air Drybulb Temperature", state->dataOutputProcessor->outVars[0]->keyColonName);
    EXPECT_EQ("Site Wind Speed", state->dataOutputProcessor->outVars[1]->name);
    EXPECT_EQ("Environment:Site Wind Speed", state->dataOutputProcessor->outVars[1]->keyColonName);
    EXPECT_EQ("Site Total Sky Cover", state->dataOutputProcessor->outVars[2]->name);
    EXPECT_EQ("Environment:Site Total Sky Cover", state->dataOutputProcessor->outVars[2]->keyColonName);
    EXPECT_EQ("Site Opaque Sky Cover", state->dataOutputProcessor->outVars[3]->name);
    EXPECT_EQ("Environment:Site Opaque Sky Cover", state->dataOutputProcessor->outVars[3]->keyColonName);

    EXPECT_EQ(7, state->dataOutputProcessor->outVars[0]->ReportID);
    EXPECT_EQ(8, state->dataOutputProcessor->outVars[1]->ReportID);
    EXPECT_EQ(9, state->dataOutputProcessor->outVars[2]->ReportID);
    EXPECT_EQ(10, state->dataOutputProcessor->outVars[3]->ReportID);

    state->dataWeather->Envrn = 1;

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->Environment.allocate(1);
    state->dataWeather->Environment(1).skyTempModel = Weather::SkyTempModel::ClarkAllen;
    state->dataWeather->Environment(1).StartMonth = 1;
    state->dataWeather->Environment(1).StartDay = 1;

    state->dataWeather->Environment(1).UseWeatherFileHorizontalIR = false;

    Weather::AllocateWeatherData(*state);
    Weather::OpenWeatherFile(*state, ErrorsFound);
    Weather::ReadWeatherForDay(*state, 1, 1, true);

    // Test the feature of interpolating some weather inputs to calc sky temp
    Real64 expected_SkyTemp = -22.8763495;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 2).SkyTemp, expected_SkyTemp, 1e-6);
}

// Test for fixing the first sub-hour weather data interpolation
TEST_F(EnergyPlusFixture, Fix_first_hour_weather_data_interpolation_OutputTest)
{
    std::string const idf_objects = delimited_string({
        "Timestep,4;"

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  Yes,                     !- Run Simulation for Sizing Periods",
        "  No;                      !- Run Simulation for Weather File Run Periods",

        "RunPeriod,",
        "  January,                 !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  1,                       !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes,                     !- Use Weather File Snow Indicators",
        "  No,                      !-Treat Weather as Actual",
        "  Hour1;                   !-First Hour Interpolation Starting Values",

        "Site:Location,",
        "  CHICAGO_IL_USA TMY3-725300,  !- Name",
        "  41.98,                   !- Latitude {deg}",
        "  -87.92,                  !- Longitude {deg}",
        "  -6.00,                   !- Time Zone {hr}",
        "  201.00;                  !- Elevation {m}",

        "Output:Variable,*,Site Outdoor Air Drybulb Temperature,Timestep;",
        "Output:Variable,*,Site Outdoor Air Dewpoint Temperature,Timestep;",
        "Output:Variable,*,Site Outdoor Air Barometric Pressure,Timestep;",
        "Output:Variable,*,Site Outdoor Air Relative Humidity,Timestep;",
        "Output:Variable,*,Site Wind Speed,Timestep;",
        "Output:Variable,*,Site Wind Direction,Timestep;"
        "Output:Variable,*,Site Total Sky Cover,Timestep;",
        "Output:Variable,*,Site Opaque Sky Cover,Timestep;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::PostIPProcessing(*state);
    bool ErrorsFound(false);
    ErrorsFound = false;

    state->init_state(*state);

    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);

    bool Available(true);
    Available = true;

    state->dataGlobal->BeginSimFlag = true;

    // The added first hour processing will be called here:
    Weather::GetNextEnvironment(*state, Available, ErrorsFound);

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->Environment(1).skyTempModel = Weather::SkyTempModel::ClarkAllen;
    state->dataWeather->Environment(1).StartMonth = 1;
    state->dataWeather->Environment(1).StartDay = 1;

    state->dataWeather->Environment(1).UseWeatherFileHorizontalIR = false;

    Weather::AllocateWeatherData(*state);
    Weather::OpenWeatherFile(*state, ErrorsFound);
    Weather::ReadWeatherForDay(*state, 1, 1, true);

    // Test interpolating values of some weather data during the first hour
    Real64 expected_DryBulbTemp = -12.2;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).OutDryBulbTemp, expected_DryBulbTemp, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).OutDryBulbTemp, expected_DryBulbTemp, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).OutDryBulbTemp, expected_DryBulbTemp, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).OutDryBulbTemp, expected_DryBulbTemp, 1e-6);

    Real64 expected_DewPointTemp = -16.1;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).OutDewPointTemp, expected_DewPointTemp, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).OutDewPointTemp, expected_DewPointTemp, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).OutDewPointTemp, expected_DewPointTemp, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).OutDewPointTemp, expected_DewPointTemp, 1e-6);

    Real64 expected_BaroPress = 99500;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).OutBaroPress, expected_BaroPress, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).OutBaroPress, expected_BaroPress, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).OutBaroPress, expected_BaroPress, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).OutBaroPress, expected_BaroPress, 1e-6);

    Real64 expected_RelHum = 73;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).OutRelHum, expected_RelHum, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).OutRelHum, expected_RelHum, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).OutRelHum, expected_RelHum, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).OutRelHum, expected_RelHum, 1e-6);

    Real64 expected_WindSpeed = 2.6;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).WindSpeed, expected_WindSpeed, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).WindSpeed, expected_WindSpeed, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).WindSpeed, expected_WindSpeed, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).WindSpeed, expected_WindSpeed, 1e-6);

    Real64 expected_WindDir = 270;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).WindDir, expected_WindDir, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).WindDir, expected_WindDir, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).WindDir, expected_WindDir, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).WindDir, expected_WindDir, 1e-6);

    Real64 expected_TotalSkyCover = 9;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).TotalSkyCover, expected_TotalSkyCover, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).TotalSkyCover, expected_TotalSkyCover, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).TotalSkyCover, expected_TotalSkyCover, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).TotalSkyCover, expected_TotalSkyCover, 1e-6);

    Real64 expected_OpaqueSkyCover = 9;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 1).OpaqueSkyCover, expected_OpaqueSkyCover, 1e-6);

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 1).OpaqueSkyCover, expected_OpaqueSkyCover, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).OpaqueSkyCover, expected_OpaqueSkyCover, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 1).OpaqueSkyCover, expected_OpaqueSkyCover, 1e-6);
}

// Test for Issue 8760: fix opaque sky cover weather values;
TEST_F(EnergyPlusFixture, Fix_OpaqueSkyCover_Test)
{
    std::string const idf_objects = delimited_string({
        "Timestep,4;"

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  Yes,                     !- Run Simulation for Sizing Periods",
        "  No;                      !- Run Simulation for Weather File Run Periods",

        "RunPeriod,",
        "  January,                 !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  1,                       !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "Site:Location,",
        "  Univ_Of_Illinois_725315,  !- Name",
        "  40.06,                   !- Latitude {deg}",
        "  -88.37,                  !- Longitude {deg}",
        "  -6.0,                   !- Time Zone {hr}",
        "  213.0;                  !- Elevation {m}",

        "Output:Variable,*,Site Total Sky Cover,Timestep;",
        "Output:Variable,*,Site Opaque Sky Cover,Timestep;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::PostIPProcessing(*state);
    state->init_state(*state);

    bool ErrorsFound(false);

    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "weather/USA_IL_University.of.Illinois-Willard.AP.725315_TMY3.epw";
    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);

    bool Available(true);
    Available = true;

    state->dataGlobal->BeginSimFlag = true;
    Weather::GetNextEnvironment(*state, Available, ErrorsFound);

    // Test get output variables for Total Sky Cover and Opaque Sky Cover
    EXPECT_EQ("Site Total Sky Cover", state->dataOutputProcessor->outVars[0]->name);
    EXPECT_EQ("Environment:Site Total Sky Cover", state->dataOutputProcessor->outVars[0]->keyColonName);
    EXPECT_EQ("Site Opaque Sky Cover", state->dataOutputProcessor->outVars[1]->name);
    EXPECT_EQ("Environment:Site Opaque Sky Cover", state->dataOutputProcessor->outVars[1]->keyColonName);

    EXPECT_EQ(7, state->dataOutputProcessor->outVars[0]->ReportID);
    EXPECT_EQ(8, state->dataOutputProcessor->outVars[1]->ReportID);

    state->dataWeather->Envrn = 1;

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->Environment.allocate(1);
    state->dataWeather->Environment(1).skyTempModel = Weather::SkyTempModel::ClarkAllen;
    state->dataWeather->Environment(1).StartMonth = 1;
    state->dataWeather->Environment(1).StartDay = 1;

    state->dataWeather->Environment(1).UseWeatherFileHorizontalIR = false;

    Weather::AllocateWeatherData(*state);
    Weather::OpenWeatherFile(*state, ErrorsFound);
    Weather::ReadWeatherForDay(*state, 1, 1, true);

    // Test additional set of weather data on sky temp calc
    Real64 expected_SkyTemp = -1.7901122977770569;
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 1).SkyTemp, expected_SkyTemp, 1e-6);

    // Test Total Sky Cover and Opaque Sky Cover
    Real64 expected_TSC = 9;
    Real64 expected_OSC = 8;

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 3).TotalSkyCover, expected_TSC, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 3).OpaqueSkyCover, expected_OSC, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 3).TotalSkyCover, 9.25, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 3).OpaqueSkyCover, 8.25, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 3).TotalSkyCover, 9.5, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 3).OpaqueSkyCover, 8.5, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 3).TotalSkyCover, 9.75, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 3).OpaqueSkyCover, 8.75, 1e-6);

    expected_TSC = 8;
    expected_OSC = 8;

    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 4).TotalSkyCover, expected_TSC, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(4, 4).OpaqueSkyCover, expected_OSC, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 4).TotalSkyCover, 8.25, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 4).OpaqueSkyCover, 8.00, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 4).TotalSkyCover, 8.50, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(2, 4).OpaqueSkyCover, 8.00, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 4).TotalSkyCover, 8.75, 1e-6);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(1, 4).OpaqueSkyCover, 8.00, 1e-6);
}

TEST_F(EnergyPlusFixture, WeatherManager_SetRainFlag)
{
    // This unit test ensures that the WaterManager correctly calculates the Rainfall CurrentRate
    std::string const idf_objects = delimited_string({
        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    SunnyWinterDay,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    5.0,                    !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    4.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "Site:Precipitation,",
        "  ScheduleAndDesignLevel,  !- Precipitation Model Type",
        "  0.75,                    !- Design Level for Total Annual Precipitation {m/yr}",
        "  PrecipitationSchd,       !- Precipitation Rates Schedule Name",
        "  0.80771;                 !- Average Total Annual Precipitation {m/yr}",

        "Schedule:Compact,",
        "  PrecipitationSchd,       !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",
    });

    // setting up start ------------------------------------------------------------------------------
    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->DayOfSim = 2; // avoid array bounds problem in RecKeepHeatBalance
    state->dataWeather->Envrn = 1;
    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;

    state->init_state(*state);

    SimulationManager::ManageSimulation(*state);
    WaterManager::GetWaterManagerInput(*state);

    state->dataWeather->Envrn = 1;

    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 24;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::UpdateScheduleVals(*state);

    state->dataWeather->Interpolation.allocate(state->dataGlobal->TimeStepsInHour);
    state->dataWeather->Interpolation = 0;
    // setting up end ------------------------------------------------------------------------------

    // Need to instantiate some stuff to avoid a crash
    // Weather::ReadUserWeatherInput(*state);

    state->dataWeather->wvarsHrTsToday.allocate(state->dataGlobal->TimeStepsInHour, Constant::iHoursInDay);
    state->dataWeather->wvarsHrTsToday(1, 24).IsRain = false;
    state->dataEnvrn->RunPeriodEnvironment = true;
    Weather::SetCurrentWeather(*state);
    // when TodayIsRain is false, IsRain is still true as site:precipitation has non-zero rain fall
    ASSERT_TRUE(state->dataEnvrn->IsRain);

    state->dataWaterData->RainFall.ModeID = DataWater::RainfallMode::EPWPrecipitation;
    state->dataWeather->wvarsHrTsToday(1, 24).IsRain = false;
    state->dataEnvrn->RunPeriodEnvironment = true;
    Weather::SetCurrentWeather(*state);
    ASSERT_FALSE(state->dataEnvrn->IsRain);

    // site:precipitation overwritten of rain flag does not take effect during sizing period
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->wvarsHrTsToday(1, 24).IsRain = false;
    state->dataEnvrn->RunPeriodEnvironment = false;
    Weather::SetCurrentWeather(*state);
    ASSERT_FALSE(state->dataEnvrn->IsRain);
}

TEST_F(EnergyPlusFixture, WeatherManager_GetReportPeriodData)
{

    std::string const idf_objects = delimited_string({"Output:Table:ReportPeriod,",
                                                      "ThermalResilienceReportTimeWinter,  !- field Name,",
                                                      "ThermalResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "8,                            !- Begin Hour of Day",
                                                      "    ,                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "3,                            !- End Day of Month",
                                                      "18;                           !- End Hour of Day"});

    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->dataWeather->TotReportPers = 1;

    Weather::GetReportPeriodData(*state, state->dataWeather->TotReportPers, ErrorsFound);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).startYear, 0);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).startMonth, 1);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).startDay, 1);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).startHour, 8);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).endYear, 0);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).endMonth, 1);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).endDay, 3);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(1).endHour, 18);
}

TEST_F(EnergyPlusFixture, WeatherManager_CopyReportPeriodObject)
{
    int nReportPeriod = 2;
    state->dataWeather->ReportPeriodInput.allocate(nReportPeriod);

    state->dataWeather->ReportPeriodInput(1).title = "test period 1";
    state->dataWeather->ReportPeriodInput(1).reportName = "empty report 1";
    state->dataWeather->ReportPeriodInput(1).startYear = 0;
    state->dataWeather->ReportPeriodInput(1).startMonth = 3;
    state->dataWeather->ReportPeriodInput(1).startDay = 5;
    state->dataWeather->ReportPeriodInput(1).startHour = 8;
    state->dataWeather->ReportPeriodInput(1).startJulianDate = Weather::computeJulianDate(state->dataWeather->ReportPeriodInput(1).startYear,
                                                                                          state->dataWeather->ReportPeriodInput(1).startMonth,
                                                                                          state->dataWeather->ReportPeriodInput(1).startDay);
    state->dataWeather->ReportPeriodInput(1).endYear = 0;
    state->dataWeather->ReportPeriodInput(1).endMonth = 3;
    state->dataWeather->ReportPeriodInput(1).endDay = 10;
    state->dataWeather->ReportPeriodInput(1).endHour = 8;
    state->dataWeather->ReportPeriodInput(1).endJulianDate = Weather::computeJulianDate(state->dataWeather->ReportPeriodInput(1).endYear,
                                                                                        state->dataWeather->ReportPeriodInput(1).endMonth,
                                                                                        state->dataWeather->ReportPeriodInput(1).endDay);

    state->dataWeather->ReportPeriodInput(2).title = "test period 2";
    state->dataWeather->ReportPeriodInput(2).reportName = "empty report 2";
    state->dataWeather->ReportPeriodInput(2).startYear = 0;
    state->dataWeather->ReportPeriodInput(2).startMonth = 6;
    state->dataWeather->ReportPeriodInput(2).startDay = 6;
    state->dataWeather->ReportPeriodInput(2).startHour = 8;
    state->dataWeather->ReportPeriodInput(2).startJulianDate = Weather::computeJulianDate(state->dataWeather->ReportPeriodInput(2).startYear,
                                                                                          state->dataWeather->ReportPeriodInput(2).startMonth,
                                                                                          state->dataWeather->ReportPeriodInput(2).startDay);
    state->dataWeather->ReportPeriodInput(2).endYear = 0;
    state->dataWeather->ReportPeriodInput(2).endMonth = 7;
    state->dataWeather->ReportPeriodInput(2).endDay = 7;
    state->dataWeather->ReportPeriodInput(2).endHour = 8;
    state->dataWeather->ReportPeriodInput(2).endJulianDate = Weather::computeJulianDate(state->dataWeather->ReportPeriodInput(2).endYear,
                                                                                        state->dataWeather->ReportPeriodInput(2).endMonth,
                                                                                        state->dataWeather->ReportPeriodInput(2).endDay);

    state->dataWeather->ReportPeriodInput(2) = state->dataWeather->ReportPeriodInput(1);

    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).title, state->dataWeather->ReportPeriodInput(1).title);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).reportName, state->dataWeather->ReportPeriodInput(1).reportName);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).startYear, state->dataWeather->ReportPeriodInput(1).startYear);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).startMonth, state->dataWeather->ReportPeriodInput(1).startMonth);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).startDay, state->dataWeather->ReportPeriodInput(1).startDay);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).startHour, state->dataWeather->ReportPeriodInput(1).startHour);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).startJulianDate, state->dataWeather->ReportPeriodInput(1).startJulianDate);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).endYear, state->dataWeather->ReportPeriodInput(1).endYear);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).endMonth, state->dataWeather->ReportPeriodInput(1).endMonth);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).endDay, state->dataWeather->ReportPeriodInput(1).endDay);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).endHour, state->dataWeather->ReportPeriodInput(1).endHour);
    EXPECT_EQ(state->dataWeather->ReportPeriodInput(2).endJulianDate, state->dataWeather->ReportPeriodInput(1).endJulianDate);
}

TEST_F(EnergyPlusFixture, WeatherManager_GroupReportPeriodByType)
{
    std::string const idf_objects = delimited_string({"Output:Table:ReportPeriod,",
                                                      "ThermalResilienceReportTimeWinter,  !- field Name,",
                                                      "ThermalResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "8,                            !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "3,                            !- End Day of Month",
                                                      "18;                           !- End Hour of Day",

                                                      "Output:Table:ReportPeriod,",
                                                      "CO2ResilienceReportTimeWinter,  !- field Name,",
                                                      "CO2ResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "2,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "8,                            !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "2,                            !- End Month",
                                                      "5,                            !- End Day of Month",
                                                      "18;                           !- End Hour of Day",

                                                      "Output:Table:ReportPeriod,",
                                                      "ThermalResilienceReportTimeWinter,  !- field Name,",
                                                      "ThermalResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "7,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "9,                            !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "8,                            !- End Month",
                                                      "5,                            !- End Day of Month",
                                                      "10;                           !- End Hour of Day"});

    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->dataWeather->TotReportPers = 3;
    Weather::GetReportPeriodData(*state, state->dataWeather->TotReportPers, ErrorsFound);

    state->dataWeather->TotThermalReportPers = 0;
    state->dataWeather->TotCO2ReportPers = 0;
    state->dataWeather->TotVisualReportPers = 0;
    Weather::GroupReportPeriodByType(*state, state->dataWeather->TotReportPers);

    EXPECT_EQ(state->dataWeather->TotThermalReportPers, 2);
    EXPECT_EQ(state->dataWeather->TotCO2ReportPers, 1);
    EXPECT_EQ(state->dataWeather->TotVisualReportPers, 0);

    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).startYear, 0);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).startMonth, 1);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).startDay, 1);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).startHour, 8);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).endYear, 0);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).endMonth, 1);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).endDay, 3);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(1).endHour, 18);

    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).startYear, 0);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).startMonth, 7);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).startDay, 1);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).startHour, 9);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).endYear, 0);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).endMonth, 8);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).endDay, 5);
    EXPECT_EQ(state->dataWeather->ThermalReportPeriodInput(2).endHour, 10);

    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).startYear, 0);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).startMonth, 2);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).startDay, 1);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).startHour, 8);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).endYear, 0);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).endMonth, 2);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).endDay, 5);
    EXPECT_EQ(state->dataWeather->CO2ReportPeriodInput(1).endHour, 18);
}

TEST_F(EnergyPlusFixture, WeatherRunPeriod_WeatherFile_OK)
{

    // Test for #9157
    std::string const idf_objects = delimited_string({
        "Timestep,4;"

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  Yes;                     !- Run Simulation for Weather File Run Periods",

        "RunPeriod,",
        "  January,                 !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  1,                       !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "Site:Location,",
        "  CHICAGO_IL_USA TMY2-94846,  !- Name",
        "  41.78,                   !- Latitude {deg}",
        "  -87.75,                  !- Longitude {deg}",
        "  -6.00,                   !- Time Zone {hr}",
        "  190.00;                  !- Elevation {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // We do have an EPW
    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->LocationGathered = false;
    state->dataGlobal->DoWeathSim = true;

    bool Available{false};
    bool ErrorsFound{false};
    Weather::GetNextEnvironment(*state, Available, ErrorsFound); // Does not throw

    EXPECT_TRUE(compare_err_stream("", true));
    EXPECT_EQ(1, state->dataWeather->NumOfEnvrn);
    EXPECT_ENUM_EQ(state->dataWeather->Environment(1).KindOfEnvrn, Constant::KindOfSim::RunPeriodWeather);
}

TEST_F(EnergyPlusFixture, WeatherRunPeriod_WeatherFile_Missing)
{

    // Test for #9157
    std::string const idf_objects = delimited_string({
        "Timestep,4;"

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  Yes;                     !- Run Simulation for Weather File Run Periods",

        "RunPeriod,",
        "  January,                 !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  1,                       !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "Site:Location,",
        "  CHICAGO_IL_USA TMY2-94846,  !- Name",
        "  41.78,                   !- Latitude {deg}",
        "  -87.75,                  !- Longitude {deg}",
        "  -6.00,                   !- Time Zone {hr}",
        "  190.00;                  !- Elevation {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // We don't have an EPW
    state->dataWeather->WeatherFileExists = false;
    state->files.inputWeatherFilePath.filePath = "doesntnotexist.epw";

    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->LocationGathered = false;
    state->dataGlobal->DoWeathSim = true;

    bool Available{false};
    bool ErrorsFound{false};
    ASSERT_THROW(Weather::GetNextEnvironment(*state, Available, ErrorsFound), std::runtime_error);

    std::string const error_string = delimited_string({
        "   ** Severe  ** GetNextEnvironment: Weather Environment(s) requested, but no weather file found",
        "   **  Fatal  ** Due to previous error condition, simulation terminated",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=GetNextEnvironment: Weather Environment(s) requested, but no weather file found",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
    EXPECT_EQ(1, state->dataWeather->NumOfEnvrn);
    EXPECT_ENUM_EQ(state->dataWeather->Environment(1).KindOfEnvrn, Constant::KindOfSim::RunPeriodWeather);
}

TEST_F(EnergyPlusFixture, epwHeaderTest)
{
    // Test for #9743
    bool errorsFound = false;
    std::string location = "LOCATION,NADI,-,FJI,IWEC Data,916800,-17.75,177.45,12.0,18.0";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::Location, location, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    EXPECT_EQ(state->dataWeather->EPWHeaderTitle, "NADI - FJI IWEC Data WMO#=916800");
    EXPECT_EQ(state->dataWeather->WeatherFileLatitude, -17.75);
    EXPECT_EQ(state->dataWeather->WeatherFileLongitude, 177.45);
    EXPECT_EQ(state->dataWeather->WeatherFileTimeZone, 12.0);
    EXPECT_EQ(state->dataWeather->WeatherFileElevation, 18.0);

    std::string designConditions = "DESIGN CONDITIONS,1,Climate Design Data 2009 ASHRAE "
                                   "Handbook,,Heating,7,16.3,17.3,12.5,9,21.2,13.4,9.6,21.2,9,25.8,8.2,25.8,2,120,Cooling,1,7.5,32.3,25.3,31.9,25.3,"
                                   "31.2,25.2,26.9,30.5,26.5,30.1,26.2,29.7,5.2,300,26,21.4,28.8,25.6,20.8,28.5,25.2,20.3,28.2,84.8,30.6,83.2,30.3,"
                                   "81.9,29.8,21,Extremes,8.4,7.4,6.6,30.4,14.3,34.6,1,1.7,13.5,35.8,12.9,36.9,12.4,37.8,11.6,39.1";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::DesignConditions, designConditions, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    // Design Conditions are skipped - nothing to check

    std::string typicalExtreme = "TYPICAL/EXTREME PERIODS,3,No Dry Season - Week Near Average Annual,Typical,4/16,4/22,No Dry Season "
                                 "- Week Near Annual Max,Extreme,2/ 5,2/11,No Dry Season - Week Near Annual Min,Extreme,July 16,Jul 22";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::TypicalExtremePeriods, typicalExtreme, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    EXPECT_EQ(state->dataWeather->NumEPWTypExtSets, 3);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).Title, "No Dry Season - Week Near Average Annual");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).ShortTitle, "NoDrySeason");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).TEType, "Typical");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).StartMonth, 4);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).StartDay, 16);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).EndMonth, 4);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(1).EndDay, 22);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).Title, "No Dry Season - Week Near Annual Max");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).ShortTitle, "NoDrySeasonMax");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).TEType, "Extreme");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).StartMonth, 2);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).StartDay, 5);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).EndMonth, 2);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(2).EndDay, 11);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).Title, "No Dry Season - Week Near Annual Min");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).ShortTitle, "NoDrySeasonMin");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).TEType, "Extreme");
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).StartMonth, 7);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).StartDay, 16);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).EndMonth, 7);
    EXPECT_EQ(state->dataWeather->TypicalExtremePeriods(3).EndDay, 22);

    std::string groundTemps =
        "GROUND "
        "TEMPERATURES,3,.5,,,,26.85,26.98,26.68,26.23,25.09,24.22,23.64,23.49,23.82,24.51,25.43,26.27,2,,,,26.27,26.54,26.46,26.22,25.45,24.76,24.22,"
        "23.94,24.02,24.42,25.06,25.72,4,,,,25.79,26.07,26.12,26.03,25.58,25.12,24.70,24.41,24.35,24.53,24.91,25.36";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::GroundTemperatures, groundTemps, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    // apparently only the first set of ground temps are used
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(1), 26.85);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(2), 26.98);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(3), 26.68);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(4), 26.23);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(5), 25.09);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(6), 24.22);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(7), 23.64);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(8), 23.49);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(9), 23.82);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(10), 24.51);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(11), 25.43);
    EXPECT_EQ(state->dataWeather->GroundTempsFCFromEPWHeader(12), 26.27);

    std::string holidaysDST = "HOLIDAYS/DAYLIGHT SAVINGS,No,0,0,0";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::HolidaysDST, holidaysDST, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    EXPECT_FALSE(state->dataWeather->WFAllowsLeapYears);
    EXPECT_FALSE(state->dataWeather->EPWDaylightSaving);
    EXPECT_EQ(state->dataWeather->NumSpecialDays, 0);

    holidaysDST = "HOLIDAYS/DAYLIGHT SAVINGS,Yes,1st Monday in May,7/31,0";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::HolidaysDST, holidaysDST, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    EXPECT_TRUE(state->dataWeather->WFAllowsLeapYears);
    EXPECT_TRUE(state->dataWeather->EPWDaylightSaving);
    EXPECT_ENUM_EQ(state->dataWeather->EPWDST.StDateType, Weather::DateType::NthDayInMonth);
    EXPECT_EQ(state->dataWeather->EPWDST.StMon, 5);
    EXPECT_EQ(state->dataWeather->EPWDST.StDay, 1);
    EXPECT_EQ(state->dataWeather->EPWDST.StWeekDay, 2);
    EXPECT_ENUM_EQ(state->dataWeather->EPWDST.EnDateType, Weather::DateType::MonthDay);
    EXPECT_EQ(state->dataWeather->EPWDST.EnMon, 7);
    EXPECT_EQ(state->dataWeather->EPWDST.EnDay, 31);
    EXPECT_EQ(state->dataWeather->EPWDST.EnWeekDay, 2);

    std::string comments1 = "COMMENTS 1,IWEC- WMO#916800 - South-west Pacific -- Original Source Data (c) 2001 ASHRAE Inc.,";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::Comments1, comments1, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    // Comments are skipped - nothing to check

    std::string comments2 = "COMMENTS 2, -- Ground temps produced with a standard soil diffusivity of 2.3225760E-03 {m**2/day}";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::Comments2, comments2, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    // Comments are skipped - nothing to check

    std::string dataPeriods = "DATA PERIODS,2,10,Data1,Sunday, 1/ 1/1989,12/31/1990,Data2,Friday, FEBRUARY 1,Mar 15";
    Weather::ProcessEPWHeader(*state, Weather::EpwHeaderType::DataPeriods, dataPeriods, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_FALSE(has_err_output());
    EXPECT_EQ(state->dataWeather->NumDataPeriods, 2);
    EXPECT_EQ(state->dataWeather->NumIntervalsPerHour, 10);
    EXPECT_EQ(state->dataWeather->DataPeriods(1).StMon, 1);
    EXPECT_EQ(state->dataWeather->DataPeriods(1).StDay, 1);
    EXPECT_EQ(state->dataWeather->DataPeriods(1).StYear, 1989);
    EXPECT_EQ(state->dataWeather->DataPeriods(1).EnMon, 12);
    EXPECT_EQ(state->dataWeather->DataPeriods(1).EnDay, 31);
    EXPECT_EQ(state->dataWeather->DataPeriods(1).EnYear, 1990);
    EXPECT_TRUE(state->dataWeather->DataPeriods(1).HasYearData);
    EXPECT_EQ(state->dataWeather->DataPeriods(2).StMon, 2);
    EXPECT_EQ(state->dataWeather->DataPeriods(2).StDay, 1);
    EXPECT_EQ(state->dataWeather->DataPeriods(2).StYear, 0);
    EXPECT_EQ(state->dataWeather->DataPeriods(2).EnMon, 3);
    EXPECT_EQ(state->dataWeather->DataPeriods(2).EnDay, 15);
    EXPECT_EQ(state->dataWeather->DataPeriods(2).EnYear, 0);
    EXPECT_FALSE(state->dataWeather->DataPeriods(2).HasYearData);
}

TEST_F(EnergyPlusFixture, DisplayWeatherMissingDataWarnings_TMYx)
{

    // Test for #9709

    {
        int WYear = 0;
        int WMonth = 0;
        int WDay = 0;
        int WHour = 0;
        int WMinute = 0;
        Real64 DryBulb = 0.0;
        Real64 DewPoint = 0.0;
        Real64 RelHum = 0.0;
        Real64 AtmPress = 0.0;
        Real64 ETHoriz = 0.0;
        Real64 ETDirect = 0.0;
        Real64 IRHoriz = 0.0;
        Real64 GLBHoriz = 0.0;
        Real64 DirectRad = 0.0;
        Real64 DiffuseRad = 0.0;
        Real64 GLBHorizIllum = 0.0;
        Real64 DirectNrmIllum = 0.0;
        Real64 DiffuseHorizIllum = 0.0;
        Real64 ZenLum = 0.0;
        Real64 WindDir = 0.0;
        Real64 WindSpeed = 0.0;
        Real64 TotalSkyCover = 0.0;
        Real64 OpaqueSkyCover = 0.0;
        Real64 Visibility = 0.0;
        Real64 CeilHeight = 0.0;
        Real64 PrecipWater = 0.0;
        Real64 AerosolOptDepth = 0.0;
        Real64 SnowDepth = 0.0;
        Real64 DaysSinceLastSnow = 0.0;
        Real64 Albedo = 0.0;
        Real64 LiquidPrecip = 0.0;
        int PresWeathObs = 0;
        Array1D_int PresWeathConds(9);
        bool ErrorFound = false;

        // Weather codes are present, taken from PRT_Faro.085540_IWEC.epw
        std::string WeatherDataLine =
            "2018,1,5,15,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9*9?9?9?9,16.60,13.50,82,100997,588,1415,369,195,14,189,22230,1274,22230,"
            "10584,320,10.00,9,9,777.7,1050,0,909999999,37,0.0840,0,88,0.200,0.0,0.0";

        Weather::InterpretWeatherDataLine(*state,
                                          WeatherDataLine,
                                          ErrorFound,
                                          WYear,
                                          WMonth,
                                          WDay,
                                          WHour,
                                          WMinute,
                                          DryBulb,
                                          DewPoint,
                                          RelHum,
                                          AtmPress,
                                          ETHoriz,
                                          ETDirect,
                                          IRHoriz,
                                          GLBHoriz,
                                          DirectRad,
                                          DiffuseRad,
                                          GLBHorizIllum,
                                          DirectNrmIllum,
                                          DiffuseHorizIllum,
                                          ZenLum,
                                          WindDir,
                                          WindSpeed,
                                          TotalSkyCover,
                                          OpaqueSkyCover,
                                          Visibility,
                                          CeilHeight,
                                          PresWeathObs,
                                          PresWeathConds,
                                          PrecipWater,
                                          AerosolOptDepth,
                                          SnowDepth,
                                          DaysSinceLastSnow,
                                          Albedo,
                                          LiquidPrecip);

        EXPECT_FALSE(ErrorFound);
        EXPECT_EQ(0, PresWeathObs);
        EXPECT_EQ(0, state->dataWeather->wvarsMissedCounts.WeathCodes);
        EXPECT_EQ(9, PresWeathConds(1));
        EXPECT_EQ(0, PresWeathConds(2));
        EXPECT_EQ(9, PresWeathConds(3));
        EXPECT_EQ(9, PresWeathConds(4));
        EXPECT_EQ(9, PresWeathConds(5));
        EXPECT_EQ(9, PresWeathConds(6));
        EXPECT_EQ(9, PresWeathConds(7));
        EXPECT_EQ(9, PresWeathConds(8));
        EXPECT_EQ(9, PresWeathConds(9));

        EXPECT_EQ(2018, WYear);
        EXPECT_EQ(1, WMonth);
        EXPECT_EQ(5, WDay);
        EXPECT_EQ(15, WHour);
        EXPECT_EQ(0, WMinute);
        EXPECT_EQ(16.60, DryBulb);
        EXPECT_EQ(13.50, DewPoint);
        EXPECT_EQ(82.0, RelHum);
        EXPECT_EQ(100997.0, AtmPress);
        EXPECT_EQ(588.0, ETHoriz);
        EXPECT_EQ(1415.0, ETDirect);
        EXPECT_EQ(369.0, IRHoriz);
        EXPECT_EQ(195.0, GLBHoriz);
        EXPECT_EQ(14.0, DirectRad);
        EXPECT_EQ(189.0, DiffuseRad);
        EXPECT_EQ(22230.0, GLBHorizIllum);
        EXPECT_EQ(1274.0, DirectNrmIllum);
        EXPECT_EQ(22230.0, DiffuseHorizIllum);
        EXPECT_EQ(10584.0, ZenLum);
        EXPECT_EQ(320.0, WindDir);
        EXPECT_EQ(10.00, WindSpeed);
        EXPECT_EQ(9.0, TotalSkyCover);
        EXPECT_EQ(9.0, OpaqueSkyCover);
        EXPECT_EQ(777.7, Visibility);
        EXPECT_EQ(1050.0, CeilHeight);

        EXPECT_EQ(37.0, PrecipWater);
        EXPECT_EQ(0.0840, AerosolOptDepth);
        EXPECT_EQ(0.0, SnowDepth);
        EXPECT_EQ(88, DaysSinceLastSnow);
        EXPECT_EQ(0.200, Albedo);
        EXPECT_EQ(0.0, LiquidPrecip);
    }

    {
        int WYear = 0;
        int WMonth = 0;
        int WDay = 0;
        int WHour = 0;
        int WMinute = 0;
        Real64 DryBulb = 0.0;
        Real64 DewPoint = 0.0;
        Real64 RelHum = 0.0;
        Real64 AtmPress = 0.0;
        Real64 ETHoriz = 0.0;
        Real64 ETDirect = 0.0;
        Real64 IRHoriz = 0.0;
        Real64 GLBHoriz = 0.0;
        Real64 DirectRad = 0.0;
        Real64 DiffuseRad = 0.0;
        Real64 GLBHorizIllum = 0.0;
        Real64 DirectNrmIllum = 0.0;
        Real64 DiffuseHorizIllum = 0.0;
        Real64 ZenLum = 0.0;
        Real64 WindDir = 0.0;
        Real64 WindSpeed = 0.0;
        Real64 TotalSkyCover = 0.0;
        Real64 OpaqueSkyCover = 0.0;
        Real64 Visibility = 0.0;
        Real64 CeilHeight = 0.0;
        Real64 PrecipWater = 0.0;
        Real64 AerosolOptDepth = 0.0;
        Real64 SnowDepth = 0.0;
        Real64 DaysSinceLastSnow = 0.0;
        Real64 Albedo = 0.0;
        Real64 LiquidPrecip = 0.0;
        int PresWeathObs = 0;
        Array1D_int PresWeathConds(9);
        bool ErrorFound = false;

        // Weather codes not present (taken from USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw)
        std::string WeatherDataLine = "1984,1,5,15,60,A7A7E8E8*0G9G9G9I9I9I9I9A7A7A7A7A7A7*0E8*0*0,14.4,2.2,44,103200,586,1415,307,357,581,115,37300,"
                                      "54200,16400,2230,240,3.1,1,0,20.0,22000,9,999999999,0,0.1570,0,88,0.000,0.0,0.0";

        Weather::InterpretWeatherDataLine(*state,
                                          WeatherDataLine,
                                          ErrorFound,
                                          WYear,
                                          WMonth,
                                          WDay,
                                          WHour,
                                          WMinute,
                                          DryBulb,
                                          DewPoint,
                                          RelHum,
                                          AtmPress,
                                          ETHoriz,
                                          ETDirect,
                                          IRHoriz,
                                          GLBHoriz,
                                          DirectRad,
                                          DiffuseRad,
                                          GLBHorizIllum,
                                          DirectNrmIllum,
                                          DiffuseHorizIllum,
                                          ZenLum,
                                          WindDir,
                                          WindSpeed,
                                          TotalSkyCover,
                                          OpaqueSkyCover,
                                          Visibility,
                                          CeilHeight,
                                          PresWeathObs,
                                          PresWeathConds,
                                          PrecipWater,
                                          AerosolOptDepth,
                                          SnowDepth,
                                          DaysSinceLastSnow,
                                          Albedo,
                                          LiquidPrecip);

        EXPECT_FALSE(ErrorFound);
        EXPECT_EQ(9, PresWeathObs);
        EXPECT_EQ(0, state->dataWeather->wvarsMissedCounts.WeathCodes);
        EXPECT_EQ(9, PresWeathConds(1));
        EXPECT_EQ(9, PresWeathConds(2));
        EXPECT_EQ(9, PresWeathConds(3));
        EXPECT_EQ(9, PresWeathConds(4));
        EXPECT_EQ(9, PresWeathConds(5));
        EXPECT_EQ(9, PresWeathConds(6));
        EXPECT_EQ(9, PresWeathConds(7));
        EXPECT_EQ(9, PresWeathConds(8));
        EXPECT_EQ(9, PresWeathConds(9));

        EXPECT_EQ(1984, WYear);
        EXPECT_EQ(1, WMonth);
        EXPECT_EQ(5, WDay);
        EXPECT_EQ(15, WHour);
        EXPECT_EQ(60, WMinute);
        EXPECT_EQ(14.4, DryBulb);
        EXPECT_EQ(2.2, DewPoint);
        EXPECT_EQ(44, RelHum);
        EXPECT_EQ(103200, AtmPress);
        EXPECT_EQ(586, ETHoriz);
        EXPECT_EQ(1415, ETDirect);
        EXPECT_EQ(307, IRHoriz);
        EXPECT_EQ(357, GLBHoriz);
        EXPECT_EQ(581, DirectRad);
        EXPECT_EQ(115, DiffuseRad);
        EXPECT_EQ(37300, GLBHorizIllum);
        EXPECT_EQ(54200, DirectNrmIllum);
        EXPECT_EQ(16400, DiffuseHorizIllum);
        EXPECT_EQ(2230, ZenLum);
        EXPECT_EQ(240, WindDir);
        EXPECT_EQ(3.1, WindSpeed);
        EXPECT_EQ(1, TotalSkyCover);
        EXPECT_EQ(0, OpaqueSkyCover);
        EXPECT_EQ(20.0, Visibility);
        EXPECT_EQ(22000, CeilHeight);

        EXPECT_EQ(0, PrecipWater);
        EXPECT_EQ(0.1570, AerosolOptDepth);
        EXPECT_EQ(0, SnowDepth);
        EXPECT_EQ(88, DaysSinceLastSnow);
        EXPECT_EQ(0.0, Albedo);
        EXPECT_EQ(0.0, LiquidPrecip);
    }

    {
        int WYear = 0;
        int WMonth = 0;
        int WDay = 0;
        int WHour = 0;
        int WMinute = 0;
        Real64 DryBulb = 0.0;
        Real64 DewPoint = 0.0;
        Real64 RelHum = 0.0;
        Real64 AtmPress = 0.0;
        Real64 ETHoriz = 0.0;
        Real64 ETDirect = 0.0;
        Real64 IRHoriz = 0.0;
        Real64 GLBHoriz = 0.0;
        Real64 DirectRad = 0.0;
        Real64 DiffuseRad = 0.0;
        Real64 GLBHorizIllum = 0.0;
        Real64 DirectNrmIllum = 0.0;
        Real64 DiffuseHorizIllum = 0.0;
        Real64 ZenLum = 0.0;
        Real64 WindDir = 0.0;
        Real64 WindSpeed = 0.0;
        Real64 TotalSkyCover = 0.0;
        Real64 OpaqueSkyCover = 0.0;
        Real64 Visibility = 0.0;
        Real64 CeilHeight = 0.0;
        Real64 PrecipWater = 0.0;
        Real64 AerosolOptDepth = 0.0;
        Real64 SnowDepth = 0.0;
        Real64 DaysSinceLastSnow = 0.0;
        Real64 Albedo = 0.0;
        Real64 LiquidPrecip = 0.0;
        int PresWeathObs = 0;
        Array1D_int PresWeathConds(9);
        bool ErrorFound = false;

        // Not providing the 5 last records
        std::string WeatherDataLine =
            "2018,1,5,15,0,?9?9?9?9E0?9?9?9?9?9?9?9?9?9?9?9?9?9*9?9?9?9,16.60,13.50,82,100997,588,1415,369,195,14,189,22230,1274,22230,"
            "10584,320,10.00,9,9,777.7,1050,0,909999999,37,0.0840";

        Weather::InterpretWeatherDataLine(*state,
                                          WeatherDataLine,
                                          ErrorFound,
                                          WYear,
                                          WMonth,
                                          WDay,
                                          WHour,
                                          WMinute,
                                          DryBulb,
                                          DewPoint,
                                          RelHum,
                                          AtmPress,
                                          ETHoriz,
                                          ETDirect,
                                          IRHoriz,
                                          GLBHoriz,
                                          DirectRad,
                                          DiffuseRad,
                                          GLBHorizIllum,
                                          DirectNrmIllum,
                                          DiffuseHorizIllum,
                                          ZenLum,
                                          WindDir,
                                          WindSpeed,
                                          TotalSkyCover,
                                          OpaqueSkyCover,
                                          Visibility,
                                          CeilHeight,
                                          PresWeathObs,
                                          PresWeathConds,
                                          PrecipWater,
                                          AerosolOptDepth,
                                          SnowDepth,
                                          DaysSinceLastSnow,
                                          Albedo,
                                          LiquidPrecip);

        EXPECT_FALSE(ErrorFound);
        EXPECT_EQ(0, PresWeathObs);
        EXPECT_EQ(0, state->dataWeather->wvarsMissedCounts.WeathCodes);
        EXPECT_EQ(9, PresWeathConds(1));
        EXPECT_EQ(0, PresWeathConds(2));
        EXPECT_EQ(9, PresWeathConds(3));
        EXPECT_EQ(9, PresWeathConds(4));
        EXPECT_EQ(9, PresWeathConds(5));
        EXPECT_EQ(9, PresWeathConds(6));
        EXPECT_EQ(9, PresWeathConds(7));
        EXPECT_EQ(9, PresWeathConds(8));
        EXPECT_EQ(9, PresWeathConds(9));

        EXPECT_EQ(2018, WYear);
        EXPECT_EQ(1, WMonth);
        EXPECT_EQ(5, WDay);
        EXPECT_EQ(15, WHour);
        EXPECT_EQ(0, WMinute);
        EXPECT_EQ(16.60, DryBulb);
        EXPECT_EQ(13.50, DewPoint);
        EXPECT_EQ(82.0, RelHum);
        EXPECT_EQ(100997.0, AtmPress);
        EXPECT_EQ(588.0, ETHoriz);
        EXPECT_EQ(1415.0, ETDirect);
        EXPECT_EQ(369.0, IRHoriz);
        EXPECT_EQ(195.0, GLBHoriz);
        EXPECT_EQ(14.0, DirectRad);
        EXPECT_EQ(189.0, DiffuseRad);
        EXPECT_EQ(22230.0, GLBHorizIllum);
        EXPECT_EQ(1274.0, DirectNrmIllum);
        EXPECT_EQ(22230.0, DiffuseHorizIllum);
        EXPECT_EQ(10584.0, ZenLum);
        EXPECT_EQ(320.0, WindDir);
        EXPECT_EQ(10.00, WindSpeed);
        EXPECT_EQ(9.0, TotalSkyCover);
        EXPECT_EQ(9.0, OpaqueSkyCover);
        EXPECT_EQ(777.7, Visibility);
        EXPECT_EQ(1050.0, CeilHeight);

        EXPECT_EQ(37.0, PrecipWater);
        EXPECT_EQ(0.0840, AerosolOptDepth);
        EXPECT_EQ(999.0, SnowDepth);
        EXPECT_EQ(999.0, DaysSinceLastSnow);
        EXPECT_EQ(999.0, Albedo);
        EXPECT_EQ(999.0, LiquidPrecip);
        // Liquid Precipitation Quantity is not read
    }
}

TEST_F(EnergyPlusFixture, EPW_no_eol_at_end_of_file)
{
    // Test for #10064 - When the EPW does not have an EOL at the end of the file

    std::string const idf_objects = delimited_string({
        "Timestep,4;"

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  Yes;                     !- Run Simulation for Weather File Run Periods",

        "RunPeriod,",
        "  January,                 !- Name",
        "  12,                      !- Begin Month",
        "  31,                      !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  12,                      !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes,                     !- Use Weather File Snow Indicators",
        "  No,                      !-Treat Weather as Actual",
        "  Hour1;                   !-First Hour Interpolation Starting Values",

        "Site:Location,",
        "  CHICAGO_IL_USA TMY3-725300,  !- Name",
        "  41.98,                   !- Latitude {deg}",
        "  -87.92,                  !- Longitude {deg}",
        "  -6.00,                   !- Time Zone {hr}",
        "  201.00;                  !- Elevation {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // We do have an EPW
    state->dataWeather->WeatherFileExists = true;
    // This is USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw, and did `truncate -s -1 chicago_no_eol_at_end_of_file.epw`
    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/chicago_no_eol_at_end_of_file.epw";

    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->LocationGathered = false;
    state->dataGlobal->DoWeathSim = true;

    bool Available = false;
    bool ErrorsFound = false;
    Weather::GetNextEnvironment(*state, Available, ErrorsFound); // Does not throw
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream("", true));
    EXPECT_EQ(1, state->dataWeather->NumOfEnvrn);
    EXPECT_ENUM_EQ(state->dataWeather->Environment(1).KindOfEnvrn, Constant::KindOfSim::RunPeriodWeather);

    EXPECT_NO_THROW(Weather::ReadWeatherForDay(*state, 1, 1, true));
    EXPECT_TRUE(compare_err_stream("", true));
}

TEST_F(EnergyPlusFixture, WeatherManager_GetAndResolveLocationInfoTest)
{
    // Tests both GetLocationInfo and ResolveLocationInformation subroutines for GitHub Issue #10579 work

    std::string const idf_objects = delimited_string({
        "Site:Location,",
        "  SkyHighChicago,  !- Name",
        "  41.78,                   !- Latitude {deg}",
        "  -87.75,                  !- Longitude {deg}",
        "  -6.00,                   !- Time Zone {hr}",
        "  1190.00,                 !- Elevation {m}",
        "  Yes;                     !- Keep Site Location Information",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    Real64 expectedLat = 41.78;
    Real64 expectedLong = -87.75;
    Real64 expectedTZ = -6.0;
    Real64 expectedElevationIDF = 1190.0;
    Real64 expectedElevationEPW = 190.0;

    bool foundErrors(false);
    Real64 allowedTolerance = 0.00001;

    // Test 1: GetLocationInfo Test--verify read and the setting of variables
    Weather::GetLocationInfo(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_EQ(state->dataWeather->LocationTitle, "SKYHIGHCHICAGO");
    EXPECT_NEAR(state->dataEnvrn->Latitude, expectedLat, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->Longitude, expectedLong, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->TimeZoneNumber, expectedTZ, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->Elevation, expectedElevationIDF, allowedTolerance);
    EXPECT_TRUE(state->dataWeather->keepUserSiteLocationDefinition);
    EXPECT_TRUE(state->dataWeather->LocationGathered);

    // Test 2A: ResolveLocationInformation Test A--location keep IDF info (no changes in data)
    state->dataWeather->NumOfEnvrn = 1;
    state->dataWeather->Environment.allocate(state->dataWeather->NumOfEnvrn);
    state->dataWeather->Environment(state->dataWeather->NumOfEnvrn).KindOfEnvrn = Constant::KindOfSim::RunPeriodWeather;
    state->dataWeather->WeatherFileExists = true;
    state->dataWeather->LocationGathered = true;
    Weather::ResolveLocationInformation(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_EQ(state->dataWeather->LocationTitle, "SKYHIGHCHICAGO");
    EXPECT_NEAR(state->dataEnvrn->Latitude, expectedLat, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->Longitude, expectedLong, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->TimeZoneNumber, expectedTZ, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->Elevation, expectedElevationIDF, allowedTolerance);
    EXPECT_TRUE(state->dataWeather->keepUserSiteLocationDefinition);

    // Test 2B: ResolveLocationInformation Test B--location swap to EPW info (change in data)
    state->dataWeather->keepUserSiteLocationDefinition = false;
    state->dataWeather->LocationTitle = "OVERWRITETHIS";
    state->dataEnvrn->Latitude = 0.0;
    state->dataEnvrn->Longitude = 0.0;
    state->dataEnvrn->TimeZoneNumber = 0.0;
    state->dataEnvrn->Elevation = 0.0;
    state->dataEnvrn->WeatherFileLocationTitle = "CHICAGOOHARE";
    state->dataWeather->WeatherFileLatitude = expectedLat;
    state->dataWeather->WeatherFileLongitude = expectedLong;
    state->dataWeather->WeatherFileTimeZone = expectedTZ;
    state->dataWeather->WeatherFileElevation = expectedElevationEPW;
    std::string const error_text2B = delimited_string({
        "   ** Warning ** Weather file location will be used rather than entered (IDF) Location object.",
        "   **   ~~~   ** ..Location object=OVERWRITETHIS",
        "   **   ~~~   ** ..Weather File Location=CHICAGOOHARE",
        "   **   ~~~   ** ..due to location differences, Latitude difference=[41.78] degrees, Longitude difference=[87.75] degrees.",
        "   **   ~~~   ** ..Time Zone difference=[6.0] hour(s), Elevation difference=[19000.00] percent, [190.00] meters.",
    });
    Weather::ResolveLocationInformation(*state, foundErrors);
    EXPECT_FALSE(foundErrors); // expect no errors
    EXPECT_EQ(state->dataWeather->LocationTitle, "CHICAGOOHARE");
    EXPECT_NEAR(state->dataEnvrn->Latitude, expectedLat, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->Longitude, expectedLong, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->TimeZoneNumber, expectedTZ, allowedTolerance);
    EXPECT_NEAR(state->dataEnvrn->Elevation, expectedElevationEPW, allowedTolerance);
    EXPECT_FALSE(state->dataWeather->keepUserSiteLocationDefinition);
    EXPECT_TRUE(compare_err_stream(error_text2B, true));
}
