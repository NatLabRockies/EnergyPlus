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

// EnergyPlus::GroundTemperatureModels Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <EnergyPlus/GroundTemperatureModeling/FiniteDifferenceGroundTemperatureModel.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/WeatherManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, FiniteDiffGroundTempModelTest)
{

    GroundTemp::FiniteDiffGroundTempsModel thisModel;

    thisModel.modelType = GroundTemp::ModelType::FiniteDiff;
    thisModel.Name = "Test";
    thisModel.baseConductivity = 1.08;
    thisModel.baseDensity = 962.0;
    thisModel.baseSpecificHeat = 2576.0;
    thisModel.waterContent = 30.0 / 100.0;
    thisModel.saturatedWaterContent = 50.0 / 100.0;
    thisModel.evapotransCoeff = 0.408;

    EXPECT_NEAR(2.0, thisModel.interpolate(2.0, 3.0, 1.0, 3.0, 1.0), 0.0000001);

    thisModel.developMesh();

    // Setting weather data manually here
    thisModel.weatherDataArray.dimension(state->dataWeather->NumDaysInYear);

    Real64 drybulb_minTemp = 5;
    Real64 drybulb_amp = 10;
    Real64 relHum_const = 0.5;
    Real64 windSpeed_const = 3.0;
    Real64 solar_min = 100;
    Real64 solar_amp = 100;

    for (int day = 1; day <= state->dataWeather->NumDaysInYear; ++day) {
        auto &tdwd = thisModel.weatherDataArray(day); // "This day weather data"

        Real64 theta = 2 * Constant::Pi * day / state->dataWeather->NumDaysInYear;
        Real64 omega = 2 * Constant::Pi * 130 / state->dataWeather->NumDaysInYear; // Shifts min to around the end of Jan

        tdwd.dryBulbTemp = drybulb_amp * std::sin(theta - omega) + (drybulb_minTemp + drybulb_amp);
        tdwd.relativeHumidity = relHum_const;
        tdwd.windSpeed = windSpeed_const;
        tdwd.horizontalRadiation = solar_amp * std::sin(theta - omega) + (solar_min + solar_amp);
        ;
        tdwd.airDensity = 1.2;
    }

    thisModel.annualAveAirTemp = 15.0;
    thisModel.maxDailyAirTemp = 25.0;
    thisModel.minDailyAirTemp = 5.0;
    thisModel.dayOfMinDailyAirTemp = 30;

    thisModel.performSimulation(*state);

    EXPECT_NEAR(4.51, thisModel.getGroundTempAtTimeInMonths(*state, 0.0, 1), 0.01);
    EXPECT_NEAR(19.14, thisModel.getGroundTempAtTimeInMonths(*state, 0.0, 6), 0.01);
    EXPECT_NEAR(7.96, thisModel.getGroundTempAtTimeInMonths(*state, 0.0, 12), 0.01);
    EXPECT_NEAR(3.46, thisModel.getGroundTempAtTimeInMonths(*state, 0.0, 14), 0.01);

    EXPECT_NEAR(14.36, thisModel.getGroundTempAtTimeInMonths(*state, 3.0, 1), 0.01);
    EXPECT_NEAR(11.78, thisModel.getGroundTempAtTimeInMonths(*state, 3.0, 6), 0.01);
    EXPECT_NEAR(15.57, thisModel.getGroundTempAtTimeInMonths(*state, 3.0, 12), 0.01);

    EXPECT_NEAR(14.58, thisModel.getGroundTempAtTimeInMonths(*state, 25.0, 1), 0.01);
    EXPECT_NEAR(14.55, thisModel.getGroundTempAtTimeInMonths(*state, 25.0, 6), 0.01);
    EXPECT_NEAR(14.53, thisModel.getGroundTempAtTimeInMonths(*state, 25.0, 12), 0.01);

    EXPECT_NEAR(5.04, thisModel.getGroundTempAtTimeInSeconds(*state, 0.0, 0.0), 0.01);
    EXPECT_NEAR(19.28, thisModel.getGroundTempAtTimeInSeconds(*state, 0.0, 14342400), 0.01);
    EXPECT_NEAR(7.32, thisModel.getGroundTempAtTimeInSeconds(*state, 0.0, 30153600), 0.01);
    EXPECT_NEAR(3.53, thisModel.getGroundTempAtTimeInSeconds(*state, 0.0, 35510400), 0.01);

    EXPECT_NEAR(14.36, thisModel.getGroundTempAtTimeInSeconds(*state, 3.0, 1296000), 0.01);
    EXPECT_NEAR(11.80, thisModel.getGroundTempAtTimeInSeconds(*state, 3.0, 14342400), 0.01);
    EXPECT_NEAR(15.46, thisModel.getGroundTempAtTimeInSeconds(*state, 3.0, 30153600), 0.01);

    EXPECT_NEAR(14.52, thisModel.getGroundTempAtTimeInSeconds(*state, 25.0, 0.0), 0.01);
    EXPECT_NEAR(14.55, thisModel.getGroundTempAtTimeInSeconds(*state, 25.0, 14342400), 0.01);
    EXPECT_NEAR(14.52, thisModel.getGroundTempAtTimeInSeconds(*state, 25.0, 30153600), 0.01);
}

TEST_F(EnergyPlusFixture, FiniteDiffGroundTempModel_GetWeather_NoWeather)
{

    GroundTemp::FiniteDiffGroundTempsModel thisModel;

    thisModel.modelType = GroundTemp::ModelType::FiniteDiff;
    thisModel.Name = "Test";
    thisModel.baseConductivity = 1.08;
    thisModel.baseDensity = 962.0;
    thisModel.baseSpecificHeat = 2576.0;
    thisModel.waterContent = 30.0 / 100.0;
    thisModel.saturatedWaterContent = 50.0 / 100.0;
    thisModel.evapotransCoeff = 0.408;

    // No Weather file specified, so we expect it to fail
    ASSERT_THROW(thisModel.getWeatherData(*state), std::runtime_error);

    std::string const error_string = delimited_string(
        {"   ** Severe  ** Site:GroundTemperature:Undisturbed:FiniteDifference -- using this model requires specification of a weather file.",
         "   **   ~~~   ** Either place in.epw in the working directory or specify a weather file on the command line using -w /path/to/weather.epw",
         "   **  Fatal  ** Simulation halted due to input error in ground temperature model.",
         "   ...Summary of Errors that led to program termination:",
         "   ..... Reference severe error count=1",
         "   ..... Last severe error=Site:GroundTemperature:Undisturbed:FiniteDifference -- using this model requires specification of a weather "
         "file."});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, FiniteDiffGroundTempModel_GetWeather_Weather)
{

    // I have to actually specify the RunPerod and SizingPeriods because in getWeatherData calls state->dataWeather->GetNextEnvironment
    // I cannot hard set WeatherManager's GetBranchInputOneTimeFlag (in anonymous namespace) to false,
    // so it'll end up calling >state->dataWeather->ReadUserWeatherInput which calls the inputProcessor to set the NumOfEnvrn in particular.
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

        "SizingPeriod:DesignDay,",
        "  CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "  7,                       !- Month",
        "  21,                      !- Day of Month",
        "  SummerDesignDay,         !- Day Type",
        "  31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "  10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                 !- Humidity Condition Type",
        "  23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                        !- Humidity Condition Day Schedule Name",
        "  ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  99063.,                  !- Barometric Pressure {Pa}",
        "  5.3,                     !- Wind Speed {m/s}",
        "  230,                     !- Wind Direction {deg}",
        "  No,                      !- Rain Indicator",
        "  No,                      !- Snow Indicator",
        "  No,                      !- Daylight Saving Time Indicator",
        "  ASHRAEClearSky,          !- Solar Model Indicator",
        "  ,                        !- Beam Solar Day Schedule Name",
        "  ,                        !- Diffuse Solar Day Schedule Name",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  1.0;                     !- Sky Clearness",

        "SizingPeriod:DesignDay,",
        "  CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "  1,                       !- Month",
        "  21,                      !- Day of Month",
        "  WinterDesignDay,         !- Day Type",
        "  -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "  0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                 !- Humidity Condition Type",
        "  -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                        !- Humidity Condition Day Schedule Name",
        "  ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  99063.,                  !- Barometric Pressure {Pa}",
        "  4.9,                     !- Wind Speed {m/s}",
        "  270,                     !- Wind Direction {deg}",
        "  No,                      !- Rain Indicator",
        "  No,                      !- Snow Indicator",
        "  No,                      !- Daylight Saving Time Indicator",
        "  ASHRAEClearSky,          !- Solar Model Indicator",
        "  ,                        !- Beam Solar Day Schedule Name",
        "  ,                        !- Diffuse Solar Day Schedule Name",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  0.0;                     !- Sky Clearness",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Set an actual weather file to Chicago EPW
    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = fs::path(configured_source_directory()) / "weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

    // Read the project data, such as Timestep
    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);
    EXPECT_EQ(state->dataGlobal->TimeStepsInHour, 4);

    // Needed to avoid crash in SetupSimulation (from ElectricPowerServiceManager.hh)
    createFacilityElectricPowerServiceObject(*state);

    bool ErrorsFound(false);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    EXPECT_EQ(state->dataWeather->NumOfEnvrn, 3);
    EXPECT_EQ(state->dataEnvrn->TotDesDays, 2);
    EXPECT_EQ(state->dataWeather->TotRunPers, 1);

    GroundTemp::FiniteDiffGroundTempsModel thisModel;

    thisModel.modelType = GroundTemp::ModelType::FiniteDiff;
    thisModel.Name = "Test";
    thisModel.baseConductivity = 1.08;
    thisModel.baseDensity = 962.0;
    thisModel.baseSpecificHeat = 2576.0;
    thisModel.waterContent = 30.0 / 100.0;
    thisModel.saturatedWaterContent = 50.0 / 100.0;
    thisModel.evapotransCoeff = 0.408;

    // Shouldn't throw
    thisModel.getWeatherData(*state);

    // It should have reverted the added period
    EXPECT_EQ(state->dataWeather->NumOfEnvrn, 3);
    EXPECT_EQ(state->dataEnvrn->TotDesDays, 2);
    EXPECT_EQ(state->dataWeather->TotRunPers, 1);

    // And should have populated a 365-day array of averages
    EXPECT_EQ(365u, thisModel.weatherDataArray.size());

    // Checking the first day against manually calculated value from EPW (24-hour averages for Jan 1)
    auto &firstDay = thisModel.weatherDataArray(1);
    EXPECT_DOUBLE_EQ(firstDay.dryBulbTemp, -5.4);
    EXPECT_NEAR(firstDay.relativeHumidity, 0.7083, 0.005);
    EXPECT_NEAR(firstDay.windSpeed, 2.8083, 0.001);
    // Sum of (BeamSolarRad + DifSolarRad)/24
    EXPECT_NEAR(firstDay.horizontalRadiation, 68, 2);
}
