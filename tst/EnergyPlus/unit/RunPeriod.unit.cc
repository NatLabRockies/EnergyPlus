// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::RunPeriod Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WeatherManager.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, RunPeriod_Defaults)
{
    Weather::RunPeriodData runperiod;
    EXPECT_ENUM_EQ(Sched::DayType::Sunday, runperiod.startWeekDay);

    EXPECT_EQ(1, runperiod.startMonth);
    EXPECT_EQ(1, runperiod.startDay);
    EXPECT_EQ(2017, runperiod.startYear);
    EXPECT_EQ(2457755, runperiod.startJulianDate);

    EXPECT_EQ(12, runperiod.endMonth);
    EXPECT_EQ(31, runperiod.endDay);
    EXPECT_EQ(2017, runperiod.endYear);
    EXPECT_EQ(2458119, runperiod.endJulianDate);

    std::array<Real64, 12> startDays{{1, 4, 4, 7, 2, 5, 7, 3, 6, 1, 4, 6}};
    for (size_t i = 0; i < 12; ++i) {
        EXPECT_EQ(startDays[i], runperiod.monWeekDay[i]);
    }
}

TEST_F(EnergyPlusFixture, RunPeriod_YearTests)
{
    std::string const idf_objects = delimited_string({
        "SimulationControl, NO, NO, NO, YES, YES;",
        "Timestep,4;",
        "RunPeriod,",
        "RP1,                     !- Name",
        "2,                       !- Begin Month",
        "29,                      !- Begin Day of Month",
        "2016,                    !- Begin Year",
        "3,                       !- End Month",
        "3,                       !- End Day of Month",
        ",                        !- End Year",
        "Monday,                  !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "RunPeriod,",
        "RP2,                     !- Name",
        "2,                       !- Begin Month",
        "29,                      !- Begin Day of Month",
        ",                        !- Begin Year",
        "3,                       !- End Month",
        "3,                       !- End Day of Month",
        ",                        !- End Year",
        "Wednesday,               !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "RunPeriod,",
        "RP3,                     !- Name",
        "1,                       !- Begin Month",
        "1,                       !- Begin Day of Month",
        ",                        !- Begin Year",
        "12,                      !- End Month",
        "31,                      !- End Day of Month",
        ",                        !- End Year",
        "Thursday,                !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "RunPeriod,",
        "RP4,                     !- Name",
        "1,                       !- Begin Month",
        "1,                       !- Begin Day of Month",
        ",                        !- Begin Year",
        "12,                      !- End Month",
        "31,                      !- End Day of Month",
        ",                        !- End Year",
        ",                        !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "RunPeriod,",
        "RP5,                     !- Name",
        "8,                       !- Begin Month",
        "18,                      !- Begin Day of Month",
        ",                        !- Begin Year",
        "12,                      !- End Month",
        "31,                      !- End Day of Month",
        ",                        !- End Year",
        "Wednesday,               !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "RunPeriod,",
        "RP6,                     !- Name",
        "2,                       !- Begin Month",
        "29,                      !- Begin Day of Month",
        ",                        !- Begin Year",
        "12,                      !- End Month",
        "31,                      !- End Day of Month",
        ",                        !- End Year",
        "Saturday,                !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "RunPeriod,",
        "RP7,                     !- Name",
        "1,                       !- Begin Month",
        "1,                       !- Begin Day of Month",
        "2016,                    !- Begin Year",
        "3,                       !- End Month",
        "31,                      !- End Day of Month",
        "2020,                    !- End Year",
        ",                        !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "BUILDING, Simple One Zone (Wireframe DXF), 0.0, Suburbs, .04, .004, MinimalShadowing, 30, 6;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    bool errors_in_input(false);
    int totalrps(7);
    Weather::GetRunPeriodData(*state, totalrps, errors_in_input);
    EXPECT_FALSE(errors_in_input);

    EXPECT_ENUM_EQ(Sched::DayType::Monday, state->dataWeather->RunPeriodInput[0].startWeekDay);
    EXPECT_EQ(2016, state->dataWeather->RunPeriodInput[0].startYear);
    EXPECT_EQ(2457448, state->dataWeather->RunPeriodInput[0].startJulianDate);
    EXPECT_EQ(2457451, state->dataWeather->RunPeriodInput[0].endJulianDate);

    EXPECT_ENUM_EQ(Sched::DayType::Wednesday, state->dataWeather->RunPeriodInput[1].startWeekDay);
    EXPECT_EQ(2012, state->dataWeather->RunPeriodInput[1].startYear);
    EXPECT_EQ(2455987, state->dataWeather->RunPeriodInput[1].startJulianDate);
    EXPECT_EQ(2455990, state->dataWeather->RunPeriodInput[1].endJulianDate);

    EXPECT_ENUM_EQ(Sched::DayType::Thursday, state->dataWeather->RunPeriodInput[2].startWeekDay);
    EXPECT_EQ(2015, state->dataWeather->RunPeriodInput[2].startYear);
    EXPECT_EQ(2457024, state->dataWeather->RunPeriodInput[2].startJulianDate);
    EXPECT_EQ(2457388, state->dataWeather->RunPeriodInput[2].endJulianDate);

    EXPECT_ENUM_EQ(Sched::DayType::Sunday, state->dataWeather->RunPeriodInput[3].startWeekDay);
    EXPECT_EQ(2017, state->dataWeather->RunPeriodInput[3].startYear);
    EXPECT_EQ(2457755, state->dataWeather->RunPeriodInput[3].startJulianDate);
    EXPECT_EQ(2458119, state->dataWeather->RunPeriodInput[3].endJulianDate);
    // This is the default, check that it works properly
    std::array<Real64, 12> startDays{{1, 4, 4, 7, 2, 5, 7, 3, 6, 1, 4, 6}};
    for (size_t i = 0; i < 12; ++i) {
        EXPECT_EQ(startDays[i], state->dataWeather->RunPeriodInput[3].monWeekDay[i]);
    }

    EXPECT_ENUM_EQ(Sched::DayType::Wednesday, state->dataWeather->RunPeriodInput[4].startWeekDay);
    EXPECT_EQ(2010, state->dataWeather->RunPeriodInput[4].startYear);
    EXPECT_EQ(2455427, state->dataWeather->RunPeriodInput[4].startJulianDate);
    EXPECT_EQ(2455562, state->dataWeather->RunPeriodInput[4].endJulianDate);

    EXPECT_ENUM_EQ(Sched::DayType::Saturday, state->dataWeather->RunPeriodInput[5].startWeekDay);
    EXPECT_EQ(1992, state->dataWeather->RunPeriodInput[5].startYear);
    EXPECT_EQ(2448682, state->dataWeather->RunPeriodInput[5].startJulianDate);
    EXPECT_EQ(2448988, state->dataWeather->RunPeriodInput[5].endJulianDate);

    EXPECT_ENUM_EQ(Sched::DayType::Friday, state->dataWeather->RunPeriodInput[6].startWeekDay);
    EXPECT_EQ(2016, state->dataWeather->RunPeriodInput[6].startYear);
    EXPECT_EQ(2457389, state->dataWeather->RunPeriodInput[6].startJulianDate);
    EXPECT_EQ(2458940, state->dataWeather->RunPeriodInput[6].endJulianDate);
}

TEST_F(EnergyPlusFixture, RunPeriod_EndYearOnly)
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
        "1997,                    !- End Year",
        "Tuesday,                 !- Day of Week for Start Day",
        "Yes,                     !- Use Weather File Holidays and Special Days",
        "Yes,                     !- Use Weather File Daylight Saving Period",
        "No,                      !- Apply Weekend Holiday Rule",
        "Yes,                     !- Use Weather File Rain Indicators",
        "Yes;                     !- Use Weather File Snow Indicators",
        "BUILDING, Simple One Zone (Wireframe DXF), 0.0, Suburbs, .04, .004, MinimalShadowing, 30, 6;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    bool errors_in_input(false);
    int totalrps(1);
    Weather::GetRunPeriodData(*state, totalrps, errors_in_input);

    EXPECT_TRUE(errors_in_input);
}

// Test for #6937: this tests that whenever the RunPeriod doesn't have a name, it should be rejected
TEST_F(EnergyPlusFixture, RunPeriod_NoName)
{

    // Doesn't have a name, InputProcessor should reject it
    std::string const idf_objects = delimited_string({
        "RunPeriod,",
        "  ,                        !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  2005,                    !- Begin Year",
        "  12,                      !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

    });

    ASSERT_FALSE(process_idf(idf_objects, false));

    std::string const error_string =
        delimited_string({"   ** Severe  ** <root>[RunPeriod] - Object contains a property that could not be validated using 'properties' or "
                          "'additionalProperties' constraints: ''.",
                          "   ** Severe  ** <root>[RunPeriod] - Object name is required and cannot be blank or whitespace"});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

// Test for #6937: this tests that whenever the RunPeriod has a name, it should be used in the error warnings
TEST_F(EnergyPlusFixture, RunPeriod_NameOfPeriodInWarning)
{

    // Case 1: has a name, but mismatched start day and year
    {

        std::string const idf_objects = delimited_string({
            "RunPeriod,",
            "  Jan,                     !- Name",
            "  1,                       !- Begin Month",
            "  1,                       !- Begin Day of Month",
            "  2005,                    !- Begin Year",
            "  12,                      !- End Month",
            "  31,                      !- End Day of Month",
            "  ,                        !- End Year",
            "  Tuesday,                 !- Day of Week for Start Day",
            "  Yes,                     !- Use Weather File Holidays and Special Days",
            "  Yes,                     !- Use Weather File Daylight Saving Period",
            "  No,                      !- Apply Weekend Holiday Rule",
            "  Yes,                     !- Use Weather File Rain Indicators",
            "  Yes;                     !- Use Weather File Snow Indicators",

        });

        ASSERT_TRUE(process_idf(idf_objects));
        bool ErrorsFound = false;
        int totalrps(1);
        Weather::GetRunPeriodData(*state, totalrps, ErrorsFound);
        // This should just issue a warning
        EXPECT_FALSE(ErrorsFound);

        std::string const error_string = delimited_string(
            {"   ** Warning ** RunPeriod: object=JAN, start weekday (TUESDAY) does not match the start year (2005), corrected to SATURDAY."});

        EXPECT_TRUE(compare_err_stream(error_string, true));
    }

    // Case 2: has a name, but starts on 2/29 on a non-leap year.
    {

        std::string const idf_objects = delimited_string({
            "RunPeriod,",
            "  NotLeap,                 !- Name",
            "  2,                       !- Begin Month",
            "  29,                      !- Begin Day of Month",
            "  2005,                    !- Begin Year",
            "  12,                      !- End Month",
            "  31,                      !- End Day of Month",
            "  ,                        !- End Year",
            "  Tuesday,                 !- Day of Week for Start Day",
            "  Yes,                     !- Use Weather File Holidays and Special Days",
            "  Yes,                     !- Use Weather File Daylight Saving Period",
            "  No,                      !- Apply Weekend Holiday Rule",
            "  Yes,                     !- Use Weather File Rain Indicators",
            "  Yes;                     !- Use Weather File Snow Indicators",

        });

        ASSERT_TRUE(process_idf(idf_objects));
        bool ErrorsFound = false;
        int totalrps(1);
        Weather::GetRunPeriodData(*state, totalrps, ErrorsFound);
        // This should issue a severe
        EXPECT_TRUE(ErrorsFound);

        std::string const error_string = delimited_string(
            {"   ** Severe  ** RunPeriod: object=NOTLEAP, start year (2005) is not a leap year but the requested start date is 2/29."});

        EXPECT_TRUE(compare_err_stream(error_string, true));
    }
}

// Side issue discovered in #6937: The parsing of SizingPeriod:WeatherFileDays and WeatherFileConditionType references RunPeriod array
// when it hits a warning/severe to create the error message
// This test should not throw!
TEST_F(EnergyPlusFixture, SizingPeriod_WeatherFile)
{

    // Case 1: bad start day/month combination
    {

        std::string const idf_objects = delimited_string({
            "SizingPeriod:WeatherFileDays,",
            "  Weather File Sizing Period,  !- Name",
            "  4,                       !- Begin Month",
            "  31,                      !- Begin Day of Month",
            "  7,                       !- End Month",
            "  25,                      !- End Day of Month",
            "  SummerDesignDay,         !- Day of Week for Start Day",
            "  No,                      !- Use Weather File Daylight Saving Period",
            "  No;                      !- Use Weather File Rain and Snow Indicators",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        bool ErrorsFound = false;
        Weather::GetRunPeriodDesignData(*state, ErrorsFound);
        // This should just issue a severe
        EXPECT_TRUE(ErrorsFound);

        std::string const error_string = delimited_string(
            {"   ** Severe  ** SizingPeriod:WeatherFileDays: object=WEATHER FILE SIZING PERIOD Begin Day of Month invalid (Day of Month) [31]"});

        EXPECT_TRUE(compare_err_stream(error_string, true));
    }
}

TEST_F(EnergyPlusFixture, RunPeriod_BadLeapDayFlagLogic)
{
    std::string const idf_objects =
        delimited_string({"SimulationControl, NO, NO, NO, YES, YES;",
                          "Timestep,4;",
                          "RunPeriod,",
                          "RP3,                     !- Name",
                          "1,                       !- Begin Month",
                          "1,                       !- Begin Day of Month",
                          "2019,                    !- Begin Year",
                          "12,                      !- End Month",
                          "31,                      !- End Day of Month",
                          ",                        !- End Year",
                          ",                        !- Day of Week for Start Day",
                          "Yes,                     !- Use Weather File Holidays and Special Days",
                          "Yes,                     !- Use Weather File Daylight Saving Period",
                          "No,                      !- Apply Weekend Holiday Rule",
                          "Yes,                     !- Use Weather File Rain Indicators",
                          "Yes;                     !- Use Weather File Snow Indicators",
                          "BUILDING, Simple One Zone (Wireframe DXF), 0.0, Suburbs, .04, .004, MinimalShadowing, 30, 6;"});

    ASSERT_TRUE(process_idf(idf_objects));
    bool errors_in_input(false);
    int totalrps(1);
    Weather::GetRunPeriodData(*state, totalrps, errors_in_input);

    EXPECT_FALSE(errors_in_input);

    state->dataWeather->Environment.allocate(1);
    // These may already be set, but do it anyway
    state->dataEnvrn->TotDesDays = 0;
    state->dataWeather->TotRunPers = 1;
    state->dataWeather->TotRunDesPers = 0;

    state->dataWeather->WFAllowsLeapYears = true; // This was hitting a bad bit of logic
    Weather::SetupEnvironmentTypes(*state);

    EXPECT_FALSE(state->dataWeather->Environment[0].IsLeapYear);
    EXPECT_EQ(365, state->dataWeather->Environment[0].TotalDays);

    state->dataWeather->Environment.deallocate();
}
