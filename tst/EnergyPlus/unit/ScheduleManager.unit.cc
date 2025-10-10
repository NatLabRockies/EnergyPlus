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

// EnergyPlus::OutputReportTabular Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/WeatherManager.hh>

#include <nlohmann/json.hpp>

#include <map>
#include <set>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, ScheduleManager_isMinuteMultipleOfTimestep)
{
    // EnergyPlus can accept 1,  2, 3,   4,  5,  6, 10, 12, 15, 20, 30, 60 timesteps per hour which correspond to
    //                      60, 30, 20, 15, 12, 10,  5,  5,  4,  3,  2,  1 minutes per timestep
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(0, 15));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(15, 15));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(30, 15));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(45, 15));

    EXPECT_FALSE(Sched::isMinuteMultipleOfTimestep(22, 15));
    EXPECT_FALSE(Sched::isMinuteMultipleOfTimestep(53, 15));

    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(0, 12));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(12, 12));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(24, 12));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(36, 12));
    EXPECT_TRUE(Sched::isMinuteMultipleOfTimestep(48, 12));

    EXPECT_FALSE(Sched::isMinuteMultipleOfTimestep(22, 12));
    EXPECT_FALSE(Sched::isMinuteMultipleOfTimestep(53, 12));
}

TEST_F(EnergyPlusFixture, ScheduleManager_UpdateScheduleVals)
{
    auto &s_glob = state->dataGlobal;
    auto &s_sched = state->dataSched;
    state->dataEnvrn->DSTIndicator = 0;

    auto *sched1 = Sched::AddScheduleDetailed(*state, "Detailed-1");

    auto *weekSched1 = Sched::AddWeekSchedule(*state, "Week-1");
    auto *weekSched2 = Sched::AddWeekSchedule(*state, "Week-2");
    auto *weekSched3 = Sched::AddWeekSchedule(*state, "Week-3");

    s_glob->TimeStepsInHour = 1;

    auto *daySched1 = Sched::AddDaySchedule(*state, "Day-1");
    auto *daySched2 = Sched::AddDaySchedule(*state, "Day-2");
    auto *daySched3 = Sched::AddDaySchedule(*state, "Day-3");

    for (int i = 1; i <= 249; i++) {
        sched1->weekScheds[i] = weekSched1;
    }
    sched1->weekScheds[250] = weekSched2;
    for (int i = 251; i <= 366; i++) {
        sched1->weekScheds[i] = weekSched3;
    }

    std::fill(weekSched1->dayScheds.begin() + 1, weekSched1->dayScheds.end(), daySched1);
    std::fill(weekSched2->dayScheds.begin() + 1, weekSched2->dayScheds.end(), daySched2);
    std::fill(weekSched3->dayScheds.begin() + 1, weekSched3->dayScheds.end(), daySched3);

    std::fill(daySched1->tsVals.begin(), daySched1->tsVals.end(), 1.0);
    std::fill(daySched2->tsVals.begin(), daySched2->tsVals.end(), 2.0);
    std::fill(daySched3->tsVals.begin(), daySched3->tsVals.end(), 3.0);

    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->DayOfWeekTomorrow = 2;
    s_glob->TimeStep = 1;
    s_glob->HourOfDay = 1;

    // check day schedules
    EXPECT_EQ(daySched1->tsVals[0 * s_glob->TimeStepsInHour], 1.0); // day < 250 points to this schedule
    EXPECT_EQ(daySched1->tsVals[23 * s_glob->TimeStepsInHour], 1.0);

    EXPECT_EQ(daySched2->tsVals[0 * s_glob->TimeStepsInHour], 2.0); // day = 250 points to this schedule
    EXPECT_EQ(daySched2->tsVals[23 * s_glob->TimeStepsInHour], 2.0);

    EXPECT_EQ(daySched3->tsVals[0 * s_glob->TimeStepsInHour], 3.0); // day > 250 points to this schedule
    EXPECT_EQ(daySched3->tsVals[23 * s_glob->TimeStepsInHour], 3.0);

    // schedule values are 1 through day 249, 2 for day 250, and 3 for remainder of year
    state->dataEnvrn->DayOfYear_Schedule = 1;
    Sched::UpdateScheduleVals(*state);
    // expect 1.0 on day 1
    EXPECT_EQ(sched1->currentVal, 1.0);

    state->dataEnvrn->DayOfYear_Schedule = 250;
    Sched::UpdateScheduleVals(*state);
    // expect 2.0 on day 250
    EXPECT_EQ(sched1->currentVal, 2.0);

    // test end of day 250 with daylight savings time active
    s_glob->HourOfDay = 24;
    state->dataEnvrn->DSTIndicator = 1;
    Sched::UpdateScheduleVals(*state);
    // expect a 3 on day 251, which on day 250 at midnight with DST of hour 1 of day 251
    EXPECT_EQ(sched1->currentVal, 3.0);

    s_glob->HourOfDay = 2;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfYear_Schedule = 251;
    Sched::UpdateScheduleVals(*state);
    // expect 3.0 for remainder of year regardless of DST
    EXPECT_EQ(sched1->currentVal, 3.0);
    s_glob->HourOfDay = 24;
    state->dataEnvrn->DSTIndicator = 1;
    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(sched1->currentVal, 3.0);
}

TEST_F(EnergyPlusFixture, ScheduleAnnualFullLoadHours_test)
{
    // J.Glazer - August 2017

    std::string const idf_objects = delimited_string({
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",
        " ",
        "Schedule:Compact,",
        " OnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " OffSched,                 !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 0.0;        !- Field 3",
        " ",
        "Schedule:Compact,",
        " JanOnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 1/31,            !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0,        !- Field 26",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 1.0,        !- Field 26",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched2,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 0.75,        !- Field 26",
        " Until: 24:00, 0.25;        !- Field 26",
        " ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;

    state->init_state(*state);

    auto *onSched = Sched::GetSchedule(*state, "ONSCHED");
    EXPECT_EQ(8760., onSched->getAnnualHoursFullLoad(*state, 1, false));

    auto *offSched = Sched::GetSchedule(*state, "OFFSCHED");
    EXPECT_EQ(0., offSched->getAnnualHoursFullLoad(*state, 1, false));

    auto *janOnSched = Sched::GetSchedule(*state, "JANONSCHED");
    EXPECT_EQ(744., janOnSched->getAnnualHoursFullLoad(*state, 1, false));

    auto *halfOnSched = Sched::GetSchedule(*state, "HALFONSCHED");
    EXPECT_EQ(4380., halfOnSched->getAnnualHoursFullLoad(*state, 1, false));

    auto *halfOnSched2 = Sched::GetSchedule(*state, "HALFONSCHED2");
    EXPECT_EQ(4380., halfOnSched2->getAnnualHoursFullLoad(*state, 1, false));
}

TEST_F(EnergyPlusFixture, ScheduleAverageHoursPerWeek_test)
{
    // J.Glazer - August 2017

    std::string const idf_objects = delimited_string({
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",
        " ",
        "Schedule:Compact,",
        " OnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " OffSched,                 !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 0.0;        !- Field 3",
        " ",
        "Schedule:Compact,",
        " JanOnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 1/31,            !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0,        !- Field 26",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 1.0,        !- Field 26",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched2,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 0.75,        !- Field 26",
        " Until: 24:00, 0.25;        !- Field 26",
        " ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;

    state->init_state(*state);

    auto *onSched = Sched::GetSchedule(*state, "ONSCHED");
    EXPECT_EQ(168., onSched->getAverageWeeklyHoursFullLoad(*state, 1, false));

    auto *offSched = Sched::GetSchedule(*state, "OFFSCHED");
    EXPECT_EQ(0., offSched->getAverageWeeklyHoursFullLoad(*state, 1, false));

    auto *janOnSched = Sched::GetSchedule(*state, "JANONSCHED");
    EXPECT_NEAR(14.3, janOnSched->getAverageWeeklyHoursFullLoad(*state, 1, false), 0.1);

    auto *halfOnSched = Sched::GetSchedule(*state, "HALFONSCHED");
    EXPECT_EQ(84., halfOnSched->getAverageWeeklyHoursFullLoad(*state, 1, false));

    auto *halfOnSched2 = Sched::GetSchedule(*state, "HALFONSCHED2");
    EXPECT_EQ(84., halfOnSched2->getAverageWeeklyHoursFullLoad(*state, 1, false));
}

TEST_F(EnergyPlusFixture, ScheduleHoursGT1perc_test)
{
    // J.Glazer - August 2017

    std::string const idf_objects = delimited_string({
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",
        " ",
        "Schedule:Compact,",
        " OnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " OffSched,                 !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 0.0;        !- Field 3",
        " ",
        "Schedule:Compact,",
        " JanOnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 1/31,            !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0,        !- Field 26",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 1.0,        !- Field 26",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched2,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 0.75,        !- Field 26",
        " Until: 24:00, 0.25;        !- Field 26",
        " ",
        "Schedule:Compact,",
        " HalfOnSched3,                  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 12:00, 0.2,        !- Field 26",
        " Until: 24:00, 0.0;        !- Field 26",
        " ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;
    s_glob->TimeStepZone = 0.25;

    state->init_state(*state);

    auto *onSched = Sched::GetSchedule(*state, "ONSCHED");
    EXPECT_EQ(8760., onSched->getAnnualHoursGreaterThan1Percent(*state, 1, false));

    auto *offSched = Sched::GetSchedule(*state, "OFFSCHED");
    EXPECT_EQ(0., offSched->getAnnualHoursGreaterThan1Percent(*state, 1, false));

    auto *janOnSched = Sched::GetSchedule(*state, "JANONSCHED");
    EXPECT_EQ(744., janOnSched->getAnnualHoursGreaterThan1Percent(*state, 1, false));

    auto *halfOnSched = Sched::GetSchedule(*state, "HALFONSCHED");
    EXPECT_EQ(4380., halfOnSched->getAnnualHoursGreaterThan1Percent(*state, 1, false));

    auto *halfOnSched2 = Sched::GetSchedule(*state, "HALFONSCHED2");
    EXPECT_EQ(8760., halfOnSched2->getAnnualHoursGreaterThan1Percent(*state, 1, false));

    auto *halfOnSched3 = Sched::GetSchedule(*state, "HALFONSCHED3");
    EXPECT_EQ(4380., halfOnSched3->getAnnualHoursGreaterThan1Percent(*state, 1, false));
}

TEST_F(EnergyPlusFixture, ScheduleDayInterval_SimpLinearInterp)
{
    // J.Glazer - September 2017

    std::string const idf_objects = delimited_string({
        "Schedule:Year,",
        "  SchYr_A,   !- Name",
        "  AnyNumber, !- Schedule Type Limits Name",
        "  SchWk_A1,  !- Schedule:Week Name 1",
        "  1,         !- Start Month 1",
        "  1,         !- Start Day 1",
        "  12,        !- End Month 1",
        "  31;        !- End Day 1",
        "",
        "Schedule:Week:Daily,",
        "  SchWk_A1,  !- Name",
        "  SchDy_A1a,  !- Sunday Schedule:Day Name",
        "  SchDy_A1a,  !- Monday Schedule:Day Name",
        "  SchDy_A1a,  !- Tuesday Schedule:Day Name",
        "  SchDy_A1a,  !- Wednesday Schedule:Day Name",
        "  SchDy_A1a,  !- Thursday Schedule:Day Name",
        "  SchDy_A1a,  !- Friday Schedule:Day Name",
        "  SchDy_A1a,  !- Saturday Schedule:Day Name",
        "  SchDy_A1a,  !- Holiday Schedule:Day Name",
        "  SchDy_A1a,  !- SummerDesignDay Schedule:Day Name",
        "  SchDy_A1a,  !- WinterDesignDay Schedule:Day Name",
        "  SchDy_A1a,  !- CustomDay1 Schedule:Day Name",
        "  SchDy_A1a;  !- CustomDay2 Schedule:Day Name",
        "  ",
        "Schedule:Day:Interval,",
        "  SchDy_A1a,  !- Name",
        "  AnyNumber,  !- Schedule Type Limits Name",
        "  Linear,     !- Interpolate to Timestep",
        "  07:00,      !- Time 1",
        "  0.001,      !- Value Until Time 1",
        "  08:00,      !- Time 2",
        "  100.001,    !- Value Until Time 2",
        "  10:00,      !- Time 4",
        "  300.001,    !- Value Until Time 4",
        "  14:00,      !- Time 8",
        "  700.001,    !- Value Until Time 8",
        "  15:00,      !- Time 9",
        "  600.001,    !- Value Until Time 9",
        "  19:00,      !- Time 13",
        "  200.001,    !- Value Until Time 13",
        "  24:00,      !- Time 14",
        "  0.001;      !- Value Until Time 14",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;
    s_glob->TimeStepZone = 0.25;

    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    s_glob->HourOfDay = 1;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    auto *ASched = Sched::GetSchedule(*state, "SCHYR_A"); // interpolate Linear
    EXPECT_NEAR(0.001, ASched->getHrTsVal(*state, 7, 4), 0.000001);

    // interpolate over one hour

    EXPECT_NEAR(25.001, ASched->getHrTsVal(*state, 8, 1), 0.000001);
    EXPECT_NEAR(50.001, ASched->getHrTsVal(*state, 8, 2), 0.000001);
    EXPECT_NEAR(75.001, ASched->getHrTsVal(*state, 8, 3), 0.000001);
    EXPECT_NEAR(100.001, ASched->getHrTsVal(*state, 8, 4), 0.000001);

    // interpolate over two hours

    EXPECT_NEAR(125.001, ASched->getHrTsVal(*state, 9, 1), 0.000001);
    EXPECT_NEAR(150.001, ASched->getHrTsVal(*state, 9, 2), 0.000001);
    EXPECT_NEAR(175.001, ASched->getHrTsVal(*state, 9, 3), 0.000001);
    EXPECT_NEAR(200.001, ASched->getHrTsVal(*state, 9, 4), 0.000001);

    EXPECT_NEAR(225.001, ASched->getHrTsVal(*state, 10, 1), 0.000001);
    EXPECT_NEAR(250.001, ASched->getHrTsVal(*state, 10, 2), 0.000001);
    EXPECT_NEAR(275.001, ASched->getHrTsVal(*state, 10, 3), 0.000001);
    EXPECT_NEAR(300.001, ASched->getHrTsVal(*state, 10, 4), 0.000001);

    // interpolate over four hours

    EXPECT_NEAR(325.001, ASched->getHrTsVal(*state, 11, 1), 0.000001);
    EXPECT_NEAR(350.001, ASched->getHrTsVal(*state, 11, 2), 0.000001);
    EXPECT_NEAR(375.001, ASched->getHrTsVal(*state, 11, 3), 0.000001);
    EXPECT_NEAR(400.001, ASched->getHrTsVal(*state, 11, 4), 0.000001);

    EXPECT_NEAR(525.001, ASched->getHrTsVal(*state, 13, 1), 0.000001);
    EXPECT_NEAR(550.001, ASched->getHrTsVal(*state, 13, 2), 0.000001);
    EXPECT_NEAR(575.001, ASched->getHrTsVal(*state, 13, 3), 0.000001);
    EXPECT_NEAR(600.001, ASched->getHrTsVal(*state, 13, 4), 0.000001);

    // interpolate over one hour - decreasing

    EXPECT_NEAR(675.001, ASched->getHrTsVal(*state, 15, 1), 0.000001);
    EXPECT_NEAR(650.001, ASched->getHrTsVal(*state, 15, 2), 0.000001);
    EXPECT_NEAR(625.001, ASched->getHrTsVal(*state, 15, 3), 0.000001);
    EXPECT_NEAR(600.001, ASched->getHrTsVal(*state, 15, 4), 0.000001);

    // interpolate over four hours - decreasing

    EXPECT_NEAR(375.001, ASched->getHrTsVal(*state, 18, 1), 0.000001);
    EXPECT_NEAR(350.001, ASched->getHrTsVal(*state, 18, 2), 0.000001);
    EXPECT_NEAR(325.001, ASched->getHrTsVal(*state, 18, 3), 0.000001);
    EXPECT_NEAR(300.001, ASched->getHrTsVal(*state, 18, 4), 0.000001);

    EXPECT_NEAR(275.001, ASched->getHrTsVal(*state, 19, 1), 0.000001);
    EXPECT_NEAR(250.001, ASched->getHrTsVal(*state, 19, 2), 0.000001);
    EXPECT_NEAR(225.001, ASched->getHrTsVal(*state, 19, 3), 0.000001);
    EXPECT_NEAR(200.001, ASched->getHrTsVal(*state, 19, 4), 0.000001);
}

TEST_F(EnergyPlusFixture, ScheduleDayInterval_PartialHourLinearInterp)
{
    // J.Glazer - September 2017

    std::string const idf_objects = delimited_string({
        "Schedule:Year,",
        "  SchYr_A,   !- Name",
        "  AnyNumber, !- Schedule Type Limits Name",
        "  SchWk_A1,  !- Schedule:Week Name 1",
        "  1,         !- Start Month 1",
        "  1,         !- Start Day 1",
        "  12,        !- End Month 1",
        "  31;        !- End Day 1",
        "",
        "Schedule:Week:Daily,",
        "  SchWk_A1,  !- Name",
        "  SchDy_A1a,  !- Sunday Schedule:Day Name",
        "  SchDy_A1a,  !- Monday Schedule:Day Name",
        "  SchDy_A1a,  !- Tuesday Schedule:Day Name",
        "  SchDy_A1a,  !- Wednesday Schedule:Day Name",
        "  SchDy_A1a,  !- Thursday Schedule:Day Name",
        "  SchDy_A1a,  !- Friday Schedule:Day Name",
        "  SchDy_A1a,  !- Saturday Schedule:Day Name",
        "  SchDy_A1a,  !- Holiday Schedule:Day Name",
        "  SchDy_A1a,  !- SummerDesignDay Schedule:Day Name",
        "  SchDy_A1a,  !- WinterDesignDay Schedule:Day Name",
        "  SchDy_A1a,  !- CustomDay1 Schedule:Day Name",
        "  SchDy_A1a;  !- CustomDay2 Schedule:Day Name",
        "  ",
        "Schedule:Day:Interval,",
        "  SchDy_A1a,  !- Name",
        "  AnyNumber,  !- Schedule Type Limits Name",
        "  Linear,     !- Interpolate to Timestep",
        "  07:00,      !- Time 1",
        "  0.001,      !- Value Until Time 1",
        "  07:30,      !- Time 2",
        "  50.001,    !- Value Until Time 2",
        "  08:00,      !- Time 4",
        "  100.001,    !- Value Until Time 2",
        "  24:00,      !- Time 14",
        "  0.001;      !- Value Until Time 14",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;
    s_glob->TimeStepZone = 0.25;

    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    s_glob->HourOfDay = 1;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    auto *ASched = Sched::GetSchedule(*state, "SCHYR_A"); // interpolate Linear
    EXPECT_NEAR(0.001, ASched->getHrTsVal(*state, 7, 4), 0.000001);

    // interpolate over first half hour

    EXPECT_NEAR(25.001, ASched->getHrTsVal(*state, 8, 1), 0.000001);
    EXPECT_NEAR(50.001, ASched->getHrTsVal(*state, 8, 2), 0.000001);

    // interpolate over second half hour

    EXPECT_NEAR(75.001, ASched->getHrTsVal(*state, 8, 3), 0.000001);
    EXPECT_NEAR(100.001, ASched->getHrTsVal(*state, 8, 4), 0.000001);
}

TEST_F(EnergyPlusFixture, ScheduleDayInterval_LinearInterpIntervalHittingIntervals)
{
    // J.Thomas - Feb 2021

    std::string const idf_objects = delimited_string({
        //    This schedule should cause issues if interpolation issue is not fixed. Got the initial schedule from the unmethours issue.
        "ScheduleTypeLimits, ",
        "  Fractional,                     !-Name ",
        "  0.1,                            !-Lower Limit Value",
        "  0.9,                            !-Upper Limit Value",
        "  Continuous;                     !-Numeric Type",

        "Schedule:Day:Interval,",
        "  2LLO Weekday,                   !- Name",
        "  Fractional,                     !- Schedule Type Limits Name",
        "  Linear,                         !- Interpolate to Timestep",
        "  06:00,                          !- Time 1 {hh:mm}",
        "  0.1,                            !- Value Until Time 1",
        "  07:15,                          !- Time 2 {hh:mm}",
        "  0.9,                            !- Value Until Time 2",
        "  16:15,                          !- Time 3 {hh:mm}",
        "  0.1,                            !- Value Until Time 3",
        "  18:15,                          !- Time 4 {hh:mm}",
        "  0.1,                            !- Value Until Time 4",
        "  24:00,                          !- Time 5 {hh:mm}",
        "  0.9;                            !-Value Until Time 5 ",

        "Schedule:Week:Daily,",
        "  Week Rule - Jan1-Dec31,         !- Name",
        "  2LLO Weekday,                   !- Sunday Schedule:Day Name",
        "  2LLO Weekday,                   !- Monday Schedule:Day Name",
        "  2LLO Weekday,                   !- Tuesday Schedule:Day Name",
        "  2LLO Weekday,                   !- Wednesday Schedule:Day Name",
        "  2LLO Weekday,                   !- Thursday Schedule:Day Name",
        "  2LLO Weekday,                   !- Friday Schedule:Day Name",
        "  2LLO Weekday,                   !- Saturday Schedule:Day Name",
        "  2LLO Weekday,                   !- Holiday Schedule:Day Name",
        "  2LLO Weekday,                   !- SummerDesignDay Schedule:Day Name",
        "  2LLO Weekday,                   !- WinterDesignDay Schedule:Day Name",
        "  2LLO Weekday,                   !- CustomDay1 Schedule:Day Name",
        "  2LLO Weekday;                   !- CustomDay2 Schedule:Day Name",

        "Schedule:Year,",
        "  2LLOYEAR,                       !- Name ",
        "  Fractional,                     !-Schedule Type Limits Name",
        "  Week Rule - Jan1-Dec31,         !- Schedule:Week Name 1",
        "  1,                              !- Start Month 1 ",
        "  1,                              !- Start Day 1",
        "  12,                             !- End Month 1",
        "  31;                             !- End Day 1",
        //    End of problem schedule
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;
    s_glob->TimeStepZone = 0.25;

    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    s_glob->HourOfDay = 1;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    auto *ASched = Sched::GetSchedule(*state, "2LLOYEAR"); // interpolate Linear
    // Timesteps will go 1,2,3,4; Not 0,1,2,3, Hours to go as (actual hour+1) therefore 7:15 is 8,1
    // Check for values specified in schedule (Lower and upper limits)
    EXPECT_NEAR(0.1, ASched->getHrTsVal(*state, 6, 4), 0.000001);  // at 6:00
    EXPECT_NEAR(0.1, ASched->getHrTsVal(*state, 17, 1), 0.000001); // at 16:15
    EXPECT_NEAR(0.1, ASched->getHrTsVal(*state, 19, 1), 0.000001); // at 18:15
    EXPECT_NEAR(0.9, ASched->getHrTsVal(*state, 24, 4), 0.000001); // at 24:00

    //    Interpolation check
    EXPECT_NEAR(0.4199999, ASched->getHrTsVal(*state, 7, 2), 0.000001);  // Value at 06:30
    EXPECT_NEAR(0.1000000, ASched->getHrTsVal(*state, 18, 3), 0.000001); // Value at 06:30
    EXPECT_NEAR(0.8304347, ASched->getHrTsVal(*state, 24, 2), 0.000001); // Value at 06:30
}

TEST_F(EnergyPlusFixture, ScheduleDayInterval_LinearInterpIntervalNotTimestep)
{
    // J.Glazer - September 2017

    std::string const idf_objects = delimited_string({
        "Schedule:Year,",
        "  SchYr_A,   !- Name",
        "  AnyNumber, !- Schedule Type Limits Name",
        "  SchWk_A1,  !- Schedule:Week Name 1",
        "  1,         !- Start Month 1",
        "  1,         !- Start Day 1",
        "  12,        !- End Month 1",
        "  31;        !- End Day 1",
        "",
        "Schedule:Week:Daily,",
        "  SchWk_A1,  !- Name",
        "  SchDy_A1a,  !- Sunday Schedule:Day Name",
        "  SchDy_A1a,  !- Monday Schedule:Day Name",
        "  SchDy_A1a,  !- Tuesday Schedule:Day Name",
        "  SchDy_A1a,  !- Wednesday Schedule:Day Name",
        "  SchDy_A1a,  !- Thursday Schedule:Day Name",
        "  SchDy_A1a,  !- Friday Schedule:Day Name",
        "  SchDy_A1a,  !- Saturday Schedule:Day Name",
        "  SchDy_A1a,  !- Holiday Schedule:Day Name",
        "  SchDy_A1a,  !- SummerDesignDay Schedule:Day Name",
        "  SchDy_A1a,  !- WinterDesignDay Schedule:Day Name",
        "  SchDy_A1a,  !- CustomDay1 Schedule:Day Name",
        "  SchDy_A1a;  !- CustomDay2 Schedule:Day Name",
        "  ",
        "Schedule:Day:Interval,",
        "  SchDy_A1a,  !- Name",
        "  AnyNumber,  !- Schedule Type Limits Name",
        "  Linear,     !- Interpolate to Timestep",
        "  07:00,      !- Time 1",
        "  0.0,        !- Value Until Time 1",
        "  07:20,      !- Time 2",
        "  33.33333333,!- Value Until Time 2",
        "  08:00,      !- Time 4",
        "  100.0,      !- Value Until Time 2",
        "  24:00,      !- Time 14",
        "  0.0;        !- Value Until Time 14",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;
    s_glob->TimeStepZone = 0.25;

    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    s_glob->HourOfDay = 1;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    auto *ASched = Sched::GetSchedule(*state, "SCHYR_A"); // interpolate Linear
    EXPECT_NEAR(0.0, ASched->getHrTsVal(*state, 7, 4), 0.000001);

    // interpolate over first half hour

    EXPECT_NEAR(25.0, ASched->getHrTsVal(*state, 8, 1), 0.000001);
    EXPECT_NEAR(50.0, ASched->getHrTsVal(*state, 8, 2), 0.000001);

    // interpolate over second half hour

    EXPECT_NEAR(75.0, ASched->getHrTsVal(*state, 8, 3), 0.000001);
    EXPECT_NEAR(100.0, ASched->getHrTsVal(*state, 8, 4), 0.000001);
}

TEST_F(EnergyPlusFixture, ScheduleYearMaxItems)
{
    std::string const idf_objects = delimited_string({
        "Schedule:Year,",
        "  SchYr_A,   !- Name",
        "  AnyNumber, !- Schedule Type Limits Name",
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31,"
        "  SchWk_A1,1,1,12,31;"
        "",
    });

    ASSERT_FALSE(process_idf(idf_objects, false));

    EXPECT_TRUE(compare_err_stream(
        delimited_string({"   ** Severe  ** <root>[Schedule:Year][SchYr_A][schedule_weeks] - Array should contain no more than 53 elements."})));
}

TEST_F(EnergyPlusFixture, ScheduleFileColumnSeparator)
{
    std::string const idf_objects = delimited_string({"Schedule:File,",
                                                      "  Test1,                   !- Name",
                                                      "  ,                        !- Schedule Type Limits Name",
                                                      "  nofile.txt,              !- File Name",
                                                      "  1,                       !- Column Number",
                                                      "  0,                       !- Rows to Skip at Top",
                                                      "  8760,                    !- Number of Hours of Data",
                                                      "  Space,                   !- Column Separator",
                                                      "  No;                      !- Interpolate to Timestep"});

    ASSERT_TRUE(process_idf(idf_objects));
}

TEST_F(EnergyPlusFixture, Schedule_GetCurrentScheduleValue_DST)
{
    std::string const idf_objects = delimited_string({
        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
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

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;

    state->init_state(*state);

    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 31;
    s_glob->HourOfDay = 24;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::UpdateScheduleVals(*state);

    auto *sched = Sched::GetSchedule(*state, "ELECTRICITY SEASON SCHEDULE");

    EXPECT_EQ(1.0, sched->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
    EXPECT_EQ(1.0, sched->currentVal);

    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    Sched::UpdateScheduleVals(*state);
    // Since DST is on, you're actually on the next day, on 6/1 at 1:00
    // so it **should** return 3.0
    EXPECT_EQ(3.0, sched->currentVal);
    EXPECT_EQ(3.0, sched->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
}

TEST_F(EnergyPlusFixture, Schedule_GetCurrentScheduleValue_DST_SouthernHemisphere)
{
    std::string const idf_objects = delimited_string({
        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 12/31,          !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  2;                       !- Field 8",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;

    state->init_state(*state);

    auto *sched = Sched::GetSchedule(*state, "ELECTRICITY SEASON SCHEDULE");

    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 31;
    s_glob->HourOfDay = 24;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(2.0, sched->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
    EXPECT_EQ(2.0, sched->currentVal);

    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    Sched::UpdateScheduleVals(*state);
    // Since DST is on, you're actually on the next day, which in this specific case should be 1/1 at 0:15
    // so it **should** return 1.0
    EXPECT_EQ(1.0, sched->currentVal);
    EXPECT_EQ(1.0, sched->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
}

TEST_F(EnergyPlusFixture, Schedule_GetCurrentScheduleValue_DST_RampUp_Leap)
{
    auto &s_sched = state->dataSched;
    auto &s_glob = state->dataGlobal;
    // So here we'll mimic using a Schedule:Compact that ramps up constantly

    // Schedule:Compact,
    //   RampingUp,                 !- Name
    //   Any Number,                !- Schedule Type Limits Name
    //   Through: 1/1,
    //   For: AllDays,
    //    Until: 1:00, 1,
    //    Until: 2:00, 2,
    //    [...]
    //    Until: 24:00, 24,
    //   Through: 1/2,
    //   For: AllDays,
    //    Until: 1:00, 25,
    //    Until: 2:00, 26,
    //    Until: 3:00, 27,
    //    [...]
    //   Through: 12/31,
    //   For: AllDays,
    //    Until: 1:00, 8761,
    //    [...]
    //    Until: 24:00, 8784;

    // # 'THROUGH" => Number of additional week schedules
    // # 'FOR' => Number of additional day schedules
    // So we use 366 Week Schedules, all with one day (LeapYear)
    state->dataEnvrn->CurrentYearIsLeapYear = true;
    state->dataWeather->WFAllowsLeapYears = true;
    state->dataWeather->LeapYearAdd = 1;

    int nDays = 366;
    s_glob->TimeStepsInHour = 4;

    auto *sched1 = Sched::AddScheduleDetailed(*state, "SCHED-1");

    for (int i = 1; i <= 366; ++i) {
        Sched::AddWeekSchedule(*state, format("WEEK_{}", i));
        Sched::AddDaySchedule(*state, format("DAY_{}", i));
    }

    for (int day = 1; day <= 366; ++day) {
        // int DayOfWeek = ((day-1) % 7) + 1;
        auto *weekSched = sched1->weekScheds[day] = Sched::GetWeekSchedule(*state, format("WEEK_{}", day));

        for (int d = 1; d <= 7; ++d) {
            auto *daySched = weekSched->dayScheds[d] = Sched::GetDaySchedule(*state, format("DAY_{}", day));

            for (int hr = 0; hr < Constant::iHoursInDay; hr++) {
                for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                    daySched->tsVals[hr * s_glob->TimeStepsInHour + ts] = (hr + 1) + (day - 1) * Constant::iHoursInDay;
                }
            }
        }
    }

    EXPECT_EQ(365, sched1->weekScheds[366]->Num);
    EXPECT_EQ(365, Sched::GetWeekSchedule(*state, format("WEEK_{}", 366))->dayScheds[2]->Num);
    EXPECT_EQ(8784.0, Sched::GetDaySchedule(*state, format("DAY_{}", 366))->tsVals[23 * s_glob->TimeStepsInHour + 3]);

    s_glob->TimeStepsInHour = s_glob->TimeStepsInHour; // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15;                    // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;

    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 31;
    s_glob->HourOfDay = 24;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->DayOfWeekTomorrow = 3;
    state->dataEnvrn->HolidayIndex = 0;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    EXPECT_EQ(366, state->dataEnvrn->DayOfYear_Schedule);

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(8784.0, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
    EXPECT_EQ(8784.0, sched1->currentVal);

    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    Sched::UpdateScheduleVals(*state);
    // Since DST is on, you're actually on the next day, which in this specific case should be 1/1 at 0:15
    // so it **should** return 1.0
    EXPECT_EQ(1.0, sched1->currentVal);
    EXPECT_EQ(1.0, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));

    Array1D_int EndDayOfMonth(12, {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31});

    {
        state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
        state->dataEnvrn->DayOfWeek = 0;
        state->dataEnvrn->DayOfWeekTomorrow = 1;

        Real64 HourOfYear = 0.0;

        for (int month = 1; month <= 12; ++month) {
            state->dataEnvrn->Month = month;
            for (int day = 1; day <= EndDayOfMonth(month); ++day) {
                state->dataEnvrn->DayOfMonth = day;
                ++state->dataEnvrn->DayOfWeek;
                if (state->dataEnvrn->DayOfWeek > 7) {
                    state->dataEnvrn->DayOfWeek = 1;
                }
                ++state->dataEnvrn->DayOfWeekTomorrow;
                if (state->dataEnvrn->DayOfWeekTomorrow > 7) {
                    state->dataEnvrn->DayOfWeekTomorrow = 1;
                }

                state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

                for (int hr = 1; hr <= 24; ++hr) {
                    ++HourOfYear;
                    s_glob->HourOfDay = hr;
                    for (int ts = 1; ts <= 4; ++ts) {
                        s_glob->TimeStep = ts;

                        Sched::UpdateScheduleVals(*state);
                        EXPECT_EQ(HourOfYear, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
                        EXPECT_EQ(HourOfYear, sched1->currentVal);
                    }
                }
            }
        }

        EXPECT_EQ(8784.0, HourOfYear);
    }

    {
        state->dataEnvrn->DSTIndicator = 1; // DST IS ON
        state->dataEnvrn->DayOfWeek = 0;
        state->dataEnvrn->DayOfWeekTomorrow = 1;

        Real64 HourOfYear = 0.0;
        for (int month = 1; month <= 12; ++month) {
            state->dataEnvrn->Month = month;
            for (int day = 1; day <= EndDayOfMonth(month); ++day) {
                state->dataEnvrn->DayOfMonth = day;
                ++state->dataEnvrn->DayOfWeek;
                if (state->dataEnvrn->DayOfWeek > 7) {
                    state->dataEnvrn->DayOfWeek = 1;
                }
                ++state->dataEnvrn->DayOfWeekTomorrow;
                if (state->dataEnvrn->DayOfWeekTomorrow > 7) {
                    state->dataEnvrn->DayOfWeekTomorrow = 1;
                }

                state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

                for (int hr = 1; hr <= 24; ++hr) {
                    ++HourOfYear;
                    s_glob->HourOfDay = hr;
                    for (int ts = 1; ts <= 4; ++ts) {
                        s_glob->TimeStep = ts;

                        Sched::UpdateScheduleVals(*state);
                        int thisHourOfYear = HourOfYear + 1;
                        if (thisHourOfYear > 8784.0) {
                            thisHourOfYear = 1;
                        }
                        EXPECT_EQ(thisHourOfYear, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
                        EXPECT_EQ(thisHourOfYear, sched1->currentVal);
                    }
                }
            }
        }

        EXPECT_EQ(8784.0, HourOfYear);
    }
}

TEST_F(EnergyPlusFixture, Schedule_GetCurrentScheduleValue_DST_RampUp_NoLeap)
{
    auto &s_glob = state->dataGlobal;
    // So here we'll mimic using a Schedule:Compact that ramps up constantly

    // Schedule:Compact,
    //   RampingUp,                 !- Name
    //   Any Number,                !- Schedule Type Limits Name
    //   Through: 1/1,
    //   For: AllDays,
    //    Until: 1:00, 1,
    //    Until: 2:00, 2,
    //    [...]
    //    Until: 24:00, 24,
    //   Through: 1/2,
    //   For: AllDays,
    //    Until: 1:00, 25,
    //    Until: 2:00, 26,
    //    Until: 3:00, 27,
    //    [...]
    //   Through: 12/31,
    //   For: AllDays,
    //    Until: 1:00, 8737,
    //    [...]
    //    Until: 24:00, 8760.0;

    // # 'THROUGH" => Number of additional week schedules
    // # 'FOR' => Number of additional day schedules
    // So we use 366 Week Schedules, all with one day (LeapYear)
    state->dataEnvrn->CurrentYearIsLeapYear = false;
    state->dataWeather->WFAllowsLeapYears = false;
    state->dataWeather->LeapYearAdd = 0;

    // ScheduleManager always assume LeapYear really.
    int nDays = 365;
    s_glob->TimeStepsInHour = 4;

    auto *sched1 = Sched::AddScheduleDetailed(*state, "SCHED-1");

    for (int i = 1; i <= 366; ++i) {
        Sched::AddWeekSchedule(*state, format("WEEK_{}", i));
    }
    for (int i = 1; i <= 365; ++i) {
        Sched::AddDaySchedule(*state, format("DAY_{}", i));
    }

    Array1D_int EndDayOfMonth(12, {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31});

    int dayOfYear = 0;
    for (int month = 1; month <= 12; ++month) {
        for (int day = 1; day <= EndDayOfMonth(month); ++day) {
            ++dayOfYear;
            int DayOfYear_Schedule = General::OrdinalDay(month, day, 1);
            if (month <= 2) {
                EXPECT_EQ(dayOfYear, DayOfYear_Schedule);
            } else {
                EXPECT_EQ(dayOfYear + 1, DayOfYear_Schedule);
            }

            auto *weekSched = sched1->weekScheds[DayOfYear_Schedule] = Sched::GetWeekSchedule(*state, format("WEEK_{}", DayOfYear_Schedule));
            auto *daySched = Sched::GetDaySchedule(*state, format("DAY_{}", dayOfYear));
            for (int d = 1; d <= 7; ++d) {
                weekSched->dayScheds[d] = daySched;
            }

            for (int hr = 0; hr < Constant::iHoursInDay; hr++) {
                for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                    daySched->tsVals[hr * s_glob->TimeStepsInHour + ts] = (hr + 1) + (dayOfYear - 1) * Constant::iHoursInDay;
                }
            }
        }
    }

    // Feb 28
    EXPECT_EQ(58, sched1->weekScheds[59]->Num);
    EXPECT_EQ(58, Sched::GetWeekSchedule(*state, "WEEK_59")->dayScheds[1]->Num);
    EXPECT_EQ(59 * Constant::rHoursInDay, Sched::GetDaySchedule(*state, "DAY_59")->tsVals[23 * s_glob->TimeStepsInHour + 3]);

    // Feb 29: doesn't exist, and I default initialized everything above to -1
    EXPECT_EQ(nullptr, sched1->weekScheds[60]);
    // ProcessSchedule would have treated the "Until: 3/1" to include the 2/29, so do that too.
    sched1->weekScheds[60] = Sched::GetWeekSchedule(*state, "WEEK_61");

    // March 1
    EXPECT_EQ(60, sched1->weekScheds[61]->Num);
    EXPECT_EQ(59, sched1->weekScheds[61]->dayScheds[1]->Num);
    EXPECT_EQ(60 * Constant::rHoursInDay, sched1->weekScheds[61]->dayScheds[1]->tsVals[23 * s_glob->TimeStepsInHour + 3]);

    EXPECT_EQ(365, sched1->weekScheds[366]->Num);
    EXPECT_EQ(364, sched1->weekScheds[366]->dayScheds[1]->Num);
    EXPECT_EQ(8760.0, sched1->weekScheds[366]->dayScheds[1]->tsVals[23 * s_glob->TimeStepsInHour + 3]);

    s_glob->TimeStepsInHour = s_glob->TimeStepsInHour; // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15;                    // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;

    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 31;
    s_glob->HourOfDay = 24;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->DayOfWeekTomorrow = 2;
    state->dataEnvrn->HolidayIndex = 0;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    EXPECT_EQ(366, state->dataEnvrn->DayOfYear_Schedule);

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(8760.0, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
    EXPECT_EQ(8760.0, sched1->currentVal);

    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    Sched::UpdateScheduleVals(*state);
    // Since DST is on, you're actually on the next day, which in this specific case should be 1/1 at 0:15
    // so it **should** return 1.0
    EXPECT_EQ(1.0, sched1->currentVal);
    EXPECT_EQ(1.0, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    state->dataEnvrn->DayOfWeek = 0;
    state->dataEnvrn->DayOfWeekTomorrow = 1;

    Real64 HourOfYear = 0.0;
    for (int month = 1; month <= 12; ++month) {
        state->dataEnvrn->Month = month;
        for (int day = 1; day <= EndDayOfMonth(month); ++day) {
            state->dataEnvrn->DayOfMonth = day;
            ++state->dataEnvrn->DayOfWeek;
            if (state->dataEnvrn->DayOfWeek > 7) {
                state->dataEnvrn->DayOfWeek = 1;
            }
            ++state->dataEnvrn->DayOfWeekTomorrow;
            if (state->dataEnvrn->DayOfWeekTomorrow > 7) {
                state->dataEnvrn->DayOfWeekTomorrow = 1;
            }

            state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

            for (int hr = 1; hr <= Constant::iHoursInDay; ++hr) {
                ++HourOfYear;
                s_glob->HourOfDay = hr;
                for (int ts = 1; ts <= s_glob->TimeStepsInHour; ++ts) {
                    s_glob->TimeStep = ts;

                    Sched::UpdateScheduleVals(*state);
                    EXPECT_EQ(HourOfYear, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
                    EXPECT_EQ(HourOfYear, sched1->currentVal);
                }
            }
        }
    }

    EXPECT_EQ(8760.0, HourOfYear);

    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    state->dataEnvrn->DayOfWeek = 0;
    state->dataEnvrn->DayOfWeekTomorrow = 1;

    HourOfYear = 0.0;
    for (int month = 1; month <= 12; ++month) {
        state->dataEnvrn->Month = month;
        for (int day = 1; day <= EndDayOfMonth(month); ++day) {
            state->dataEnvrn->DayOfMonth = day;
            ++state->dataEnvrn->DayOfWeek;
            if (state->dataEnvrn->DayOfWeek > 7) {
                state->dataEnvrn->DayOfWeek = 1;
            }
            ++state->dataEnvrn->DayOfWeekTomorrow;
            if (state->dataEnvrn->DayOfWeekTomorrow > 7) {
                state->dataEnvrn->DayOfWeekTomorrow = 1;
            }

            state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

            for (int hr = 1; hr <= 24; ++hr) {
                ++HourOfYear;
                s_glob->HourOfDay = hr;
                for (int ts = 1; ts <= 4; ++ts) {
                    s_glob->TimeStep = ts;

                    Sched::UpdateScheduleVals(*state);
                    int thisHourOfYear = HourOfYear + 1;
                    if (thisHourOfYear > 8760.0) {
                        thisHourOfYear = 1;
                    }
                    EXPECT_EQ(thisHourOfYear, sched1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep));
                    EXPECT_EQ(thisHourOfYear, sched1->currentVal);
                }
            }
        }
    }

    EXPECT_EQ(8760.0, HourOfYear);
}

TEST_F(EnergyPlusFixture, ScheduleFileDSTtoggleOptionTest)
{
    // P. Shrestha and N. Merket - February 2022

    // Create this file with test data in it. This test will fail until you do that.

    // scheduleFile object of class "path" created here in the next line is basically a long string that provides the full path to the file
    fs::path scheduleFile = configured_source_directory() / "tst/EnergyPlus/unit/Resources/schedule_file1.csv";

    // Adding Schedule:File blocks that test other possibilities to make sure they work right, such as:
    //          1.) Setting to "Yes"
    //          2.) Setting to "No"
    //          3.) Leaving empty (should default to "No")
    //          4.) Omitting the last field (should default to "No")

    // Defining the idf chunk needed to run the relevant tests - "idf_objects" as a comma-delimited string vector

    std::string const idf_objects = delimited_string({
        "Schedule:File,",
        "  Test1,                   !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  " + scheduleFile.string() + ",              !- File Name",
        "  2,                       !- Column Number",
        "  1,                       !- Rows to Skip at Top",
        "  8760,                    !- Number of Hours of Data",
        "  Comma,                   !- Column Separator",
        "  No,                      !- Interpolate to Timestep",
        "  60,                      !- Minutes per item",
        "  Yes;                     !- Adjust Schedule for Daylight Savings",
        " ",
        "Schedule:File,",
        "  Test2,                   !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  " + scheduleFile.string() + ",              !- File Name",
        "  2,                       !- Column Number",
        "  1,                       !- Rows to Skip at Top",
        "  8760,                    !- Number of Hours of Data",
        "  Comma,                   !- Column Separator",
        "  No,                      !- Interpolate to Timestep",
        "  60,                      !- Minutes per item",
        "  No;                     !- Adjust Schedule for Daylight Savings",
        " ",
        "Schedule:File,",
        "  Test3,                   !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  " + scheduleFile.string() + ",              !- File Name",
        "  2,                       !- Column Number",
        "  1,                       !- Rows to Skip at Top",
        "  8760,                    !- Number of Hours of Data",
        "  Comma,                   !- Column Separator",
        "  No,                      !- Interpolate to Timestep",
        "  60,                      !- Minutes per item",
        "  ;                     !- Adjust Schedule for Daylight Savings",
        " ",
        "Schedule:File,",
        "  Test4,                   !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  " + scheduleFile.string() + ",              !- File Name",
        "  2,                       !- Column Number",
        "  1,                       !- Rows to Skip at Top",
        "  8760,                    !- Number of Hours of Data",
        "  Comma,                   !- Column Separator",
        "  No,                      !- Interpolate to Timestep",
        "  60;                      !- Minutes per item",
        " ",
    });

    // This will process the provided idf chunk within the test fixture (must pass this step in order to proceed)
    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 1;
    s_glob->MinutesInTimeStep = 60;
    s_glob->TimeStep = 1; // Checking to see if omitting this is OK here

    state->init_state(*state);

    state->dataEnvrn->DayOfWeek = 1;         // Sunday
    state->dataEnvrn->DayOfWeekTomorrow = 2; // Monday
    state->dataEnvrn->DayOfYear_Schedule = 1;
    s_glob->HourOfDay = 24;

    // Test 1 condition
    // "YES" Adjusts schedule for daylight savings
    auto const *sch1 = dynamic_cast<Sched::ScheduleDetailed const *>(Sched::GetSchedule(*state, "TEST1"));
    EXPECT_TRUE(sch1->UseDaylightSaving); // Checks that the member variable got set correctly.

    state->dataEnvrn->DSTIndicator = 1; // Tells the simulation that we're currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 0.0);
    state->dataEnvrn->DSTIndicator = 0; // Tells the simulation that we're NOT currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch1->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 1.0);

    // Test 2 condition
    // "NO" Does not adjust for daylight savings
    auto const *sch2 = dynamic_cast<Sched::ScheduleDetailed const *>(Sched::GetSchedule(*state, "TEST2"));
    EXPECT_FALSE(sch2->UseDaylightSaving);

    state->dataEnvrn->DSTIndicator = 1; // Tells the simulation that we're currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch2->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 1.0);
    state->dataEnvrn->DSTIndicator = 0; // Tells the simulation that we're NOT currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch2->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 1.0);

    // Test 3 condition
    // Default: "YES", changes schedule for daylight savings
    auto const *sch3 = dynamic_cast<Sched::ScheduleDetailed const *>(Sched::GetSchedule(*state, "TEST3"));
    EXPECT_TRUE(sch3->UseDaylightSaving);

    state->dataEnvrn->DSTIndicator = 1; // Tells the simulation that we're currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch3->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 0.0);
    state->dataEnvrn->DSTIndicator = 0; // Tells the simulation that we're NOT currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch3->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 1.0);

    // Test 4 condition
    // Default: "YES", changes schedule for daylight savings
    auto const *sch4 = dynamic_cast<Sched::ScheduleDetailed const *>(Sched::GetSchedule(*state, "TEST4"));
    EXPECT_TRUE(sch4->UseDaylightSaving); // Checks that the member variable got set correctly.

    state->dataEnvrn->DSTIndicator = 1; // Tells the simulation that we're currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch4->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 0.0);
    state->dataEnvrn->DSTIndicator = 0; // Tells the simulation that we're NOT currently observing daylight savings
    EXPECT_DOUBLE_EQ(sch4->getHrTsVal(*state, s_glob->HourOfDay, s_glob->TimeStep), 1.0);
}

TEST_F(EnergyPlusFixture, ScheduleFile_Blanks)
{

    // On the third line (second data record after header), there is a blank in the second column
    fs::path scheduleFile = FileSystem::makeNativePath(configured_source_directory() / "tst/EnergyPlus/unit/Resources/schedule_file_with_blank.csv");

    std::string const idf_objects = delimited_string({
        "Schedule:File,",
        "  Test1,                   !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  " + scheduleFile.string() + ",              !- File Name",
        "  2,                       !- Column Number",
        "  1,                       !- Rows to Skip at Top",
        "  8760,                    !- Number of Hours of Data",
        "  Comma,                   !- Column Separator",
        "  No,                      !- Interpolate to Timestep",
        "  60,                      !- Minutes per item",
        "  Yes;                     !- Adjust Schedule for Daylight Savings",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;
    state->dataEnvrn->CurrentYearIsLeapYear = false;

    ASSERT_THROW(state->init_state(*state), EnergyPlus::FatalError); // read schedules

    const std::string expected_error = delimited_string({
        "   ** Warning ** ProcessScheduleInput: Schedule:File = TEST1",
        "   **   ~~~   ** Schedule Type Limits Name is empty.",
        "   **   ~~~   ** Schedule will not be validated.",
        "   ** Severe  ** CsvParser - Line 3 - Expected 3 columns, got 2. Error in following line.",
        "   **   ~~~   ** 1,,0.33",
        "   **   ~~~   ** Error Occurred in " + scheduleFile.string(),
        "   **  Fatal  ** Program terminates due to previous condition.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=CsvParser - Line 3 - Expected 3 columns, got 2. Error in following line.",
    });

    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, ShadowCalculation_CSV_extra_parenthesis)
{
    // 9753 - Test backward compat:
    // a CSV exported with the extra '()' at the end (22.2.0 and below) should still be importable in E+ without crashing
    const fs::path scheduleFile = configured_source_directory() / "tst/EnergyPlus/unit/Resources/shading_data_2220.csv";

    std::string const idf_objects = delimited_string({
        "Schedule:File:Shading,",
        "  " + scheduleFile.string() + ";              !- Name of File",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;
    auto &s_sched = state->dataSched;

    s_glob->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;
    state->init_state(*state);

    state->dataEnvrn->CurrentYearIsLeapYear = false;

    const std::string expected_error = delimited_string({
        "   ** Warning ** ProcessScheduleInput: Schedule:File:Shading=\"" + scheduleFile.string() +
            "\" Removing last column of the CSV since it has '()' for the surface name.",
        "   **   ~~~   ** This was a problem in E+ 22.2.0 and below, consider removing it from the file to suppress this warning.",
    });
    compare_err_stream(expected_error);

    EXPECT_TRUE(s_sched->ScheduleFileShadingProcessed);
    EXPECT_EQ(3, s_sched->schedules.size()); // AlwaysOn, AlwaysOff, plus file
    EXPECT_EQ(365, s_sched->weekSchedules.size());
    EXPECT_EQ(365, s_sched->daySchedules.size());
    EXPECT_EQ(1, s_sched->UniqueProcessedExternalFiles.size());

    auto &[fPath, root] = *(s_sched->UniqueProcessedExternalFiles.begin());
    EXPECT_EQ(scheduleFile, fPath);
    EXPECT_EQ(2, root["header"].size());
    const std::set<std::string> expectedHeaders{"Surface Name", "EAST SIDE TREE"};
    EXPECT_EQ(expectedHeaders, root["header"].get<std::set<std::string>>());
    ASSERT_EQ(2, root["values"].size());

    EXPECT_EQ(8760 * 4, root["values"].at(0).size());
    EXPECT_EQ(8760 * 4, root["values"].at(1).size());

    EXPECT_EQ("01/01 00:15", root["values"].at(0).at(0).get<std::string>());
    EXPECT_EQ(0.00000000, root["values"].at(1).at(0).get<Real64>());

    EXPECT_EQ("01/01 13:00", root["values"].at(0).at(51).get<std::string>());
    EXPECT_EQ(0.96107882, root["values"].at(1).at(51).get<Real64>());

    EXPECT_EQ("12/31 24:00", root["values"].at(0).at(8760 * 4 - 1).get<std::string>());
    EXPECT_EQ(0.00000000, root["values"].at(1).at(8760 * 4 - 1).get<Real64>());

    EXPECT_EQ(Sched::GetWeekScheduleNum(*state, "EAST SIDE TREE_SHADING_WK_1"), 0);
    EXPECT_EQ(Sched::GetWeekScheduleNum(*state, "EAST SIDE TREE_SHADING_WK_59"), 58);
    EXPECT_EQ(Sched::GetWeekScheduleNum(*state, "EAST SIDE TREE_SHADING_WK_61"), 59);
    EXPECT_EQ(Sched::GetWeekScheduleNum(*state, "EAST SIDE TREE_SHADING_WK_62"), 60);
    EXPECT_EQ(Sched::GetWeekScheduleNum(*state, "EAST SIDE TREE_SHADING_WK_366"), 364);

    EXPECT_EQ(Sched::GetDayScheduleNum(*state, "EAST SIDE TREE_SHADING_DY_1"), 0);
    EXPECT_EQ(Sched::GetDayScheduleNum(*state, "EAST SIDE TREE_SHADING_DY_59"), 58);
    EXPECT_EQ(Sched::GetDayScheduleNum(*state, "EAST SIDE TREE_SHADING_DY_61"), 59);
    EXPECT_EQ(Sched::GetDayScheduleNum(*state, "EAST SIDE TREE_SHADING_DY_62"), 60);
    EXPECT_EQ(Sched::GetDayScheduleNum(*state, "EAST SIDE TREE_SHADING_DY_366"), 364);

    auto const *sched = dynamic_cast<Sched::ScheduleDetailed const *>(Sched::GetSchedule(*state, "EAST SIDE TREE_SHADING"));

    EXPECT_EQ(0, sched->weekScheds[1]->Num);
    EXPECT_EQ(58, sched->weekScheds[59]->Num);
    EXPECT_EQ(58, sched->weekScheds[60]->Num); // 29 Feb points to 28 Feb
    EXPECT_EQ(364, sched->weekScheds[366]->Num);

    for (int iDay = 0; iDay < 365; ++iDay) {
        if (iDay <= 58) {
            EXPECT_EQ(fmt::format("EAST SIDE TREE_shading_wk_{}", iDay + 1), s_sched->weekSchedules[iDay]->Name);
            EXPECT_EQ(fmt::format("EAST SIDE TREE_shading_dy_{}", iDay + 1), s_sched->daySchedules[iDay]->Name);
        } else {
            EXPECT_EQ(fmt::format("EAST SIDE TREE_shading_wk_{}", iDay + 2), s_sched->weekSchedules[iDay]->Name);
            EXPECT_EQ(fmt::format("EAST SIDE TREE_shading_dy_{}", iDay + 2), s_sched->daySchedules[iDay]->Name);
        }
    }

    // 01/01 00:15
    int iDay = 1;
    int TS = 1;
    int iHour = 1;
    EXPECT_EQ(0.00000000, s_sched->daySchedules[(iDay - 1)]->tsVals[(iHour - 1) * s_glob->TimeStepsInHour + (TS - 1)]);

    // 01/01 13:00
    iDay = 1;
    TS = 4;
    iHour = 13;
    EXPECT_EQ(0.96107882, s_sched->daySchedules[(iDay - 1)]->tsVals[(iHour - 1) * s_glob->TimeStepsInHour + (TS - 1)]);

    // 12/31 16:15,0.19556231,
    iDay = 365;
    TS = 1;
    iHour = 17;
    EXPECT_EQ(0.19556231, s_sched->daySchedules[(iDay - 1)]->tsVals[(iHour - 1) * s_glob->TimeStepsInHour + (TS - 1)]);

    // 12/31 24:00
    iDay = 365;
    TS = 4;
    iHour = 24;
    EXPECT_EQ(0.00000000, s_sched->daySchedules[(iDay - 1)]->tsVals[(iHour - 1) * s_glob->TimeStepsInHour + (TS - 1)]);
}

TEST_F(EnergyPlusFixture, getScheduleMinMaxByDayType_test)
{
    // J.Glazer - March 2024

    std::string const idf_objects = delimited_string({
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",
        " ",
        "Schedule:Compact,",
        "  HighLow01,",
        "  Any Number,",
        "  Through: 12/31,",
        "  For: WeekDays CustomDay1 CustomDay2,",
        "  Until: 8:00,0.42,",
        "  Until: 11:00,0.75,",
        "  Until: 12:00,0.80,",
        "  Until: 13:00,0.40,",
        "  Until: 14:00,0.87,",
        "  Until: 18:00,0.75,",
        "  Until: 19:00,0.50,",
        "  Until: 24:00,0.40,",
        "  For: Weekends Holiday,",
        "  Until: 8:00,0.30,",
        "  Until: 24:00,0.83,",
        "  For: SummerDesignDay,",
        "  Until: 8:00,0.85,",
        "  Until: 24:00,0.95,",
        "  For: WinterDesignDay,",
        "  Until: 8:00,0.17,",
        "  Until: 24:00,0.15;",
        " ",
        "Schedule:Compact,",
        "  HighLow02,",
        "  Any Number,",
        "  Through: 4/30,",
        "  For: WeekDays CustomDay1 CustomDay2,",
        "  Until: 24:00,0.21,",
        "  For: Weekends Holiday,",
        "  Until: 24:00,0.65,",
        "  For: SummerDesignDay,",
        "  Until: 24:00,0.76,",
        "  Until: 24:00,0.79,",
        "  For: WinterDesignDay,",
        "  Until: 24:00,0.16,",
        "  Until: 24:00,0.18,",
        "  Through: 12/31,",
        "  For: WeekDays CustomDay1 CustomDay2,",
        "  Until: 24:00,0.73,",
        "  For: Weekends Holiday,",
        "  Until: 24:00,0.27,",
        "  For: SummerDesignDay,",
        "  Until: 8:00,0.77,",
        "  Until: 24:00,0.85,",
        "  For: WinterDesignDay,",
        "  Until: 8:00,0.19,",
        "  Until: 24:00,0.25;",
        " ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;
    s_glob->MinutesInTimeStep = 15;
    s_glob->TimeStepZone = 0.25;

    state->init_state(*state);

    Real64 schMin;
    Real64 schMax;

    std::tie(schMin, schMax) = Sched::GetScheduleAlwaysOn(*state)->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::Weekday);
    EXPECT_EQ(1.0, schMin);
    EXPECT_EQ(1.0, schMax);

    std::tie(schMin, schMax) = Sched::GetScheduleAlwaysOff(*state)->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::Weekday);
    EXPECT_EQ(0.0, schMin);
    EXPECT_EQ(0.0, schMax);

    auto *sched = Sched::GetSchedule(*state, "HIGHLOW01");

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::Weekday);
    EXPECT_EQ(0.40, schMin);
    EXPECT_EQ(0.87, schMax);

    // repeat test to see if using cached values
    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::Weekday);
    EXPECT_EQ(0.40, schMin);
    EXPECT_EQ(0.87, schMax);

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::WeekEndHoliday);
    EXPECT_EQ(0.30, schMin);
    EXPECT_EQ(0.83, schMax);

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::SummerDesignDay);
    EXPECT_EQ(0.85, schMin);
    EXPECT_EQ(0.95, schMax);

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::WinterDesignDay);
    EXPECT_EQ(0.15, schMin);
    EXPECT_EQ(0.17, schMax);

    sched = Sched::GetSchedule(*state, "HIGHLOW02");

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::Weekday);
    EXPECT_EQ(0.21, schMin);
    EXPECT_EQ(0.73, schMax);

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::WeekEndHoliday);
    EXPECT_EQ(0.27, schMin);
    EXPECT_EQ(0.65, schMax);

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::SummerDesignDay);
    EXPECT_EQ(0.76, schMin);
    EXPECT_EQ(0.85, schMax);

    std::tie(schMin, schMax) = sched->getMinMaxValsByDayType(*state, Sched::DayTypeGroup::WinterDesignDay);
    EXPECT_EQ(0.16, schMin);
    EXPECT_EQ(0.25, schMax);
}

TEST_F(EnergyPlusFixture, ScheduleCompact_MissingDayTypes)
{
    // Test for #11054
    std::string const idf_objects = delimited_string({
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",

        "Schedule:Compact,",
        "  WindowVentSched,         !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: Monday Tuesday Wednesday Thursday Friday Saturday, !- Field 2",
        "  Until: 24:00,1;          !- Field 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_glob = state->dataGlobal;

    s_glob->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    s_glob->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    s_glob->TimeStepZone = 0.25;
    s_glob->TimeStepZoneSec = s_glob->TimeStepZone * Constant::rSecsInHour;
    state->dataEnvrn->CurrentYearIsLeapYear = false;

    state->init_state(*state); // read schedules (this calls ProcessScheduleInput via ScheduleManagerData::init_state)

    const std::string expected_error = delimited_string({
        "   ** Warning ** ProcessScheduleInput: Schedule:Compact = WINDOWVENTSCHED",
        "   **   ~~~   ** has missing day types in Through=12/31",
        "   **   ~~~   ** Last \"For\" field=FOR: MONDAY TUESDAY WEDNESDAY THURSDAY FRIDAY SATURDAY",
        R"(   **   ~~~   ** Missing day types= "Sunday", "Holiday", "SummerDesignDay", "WinterDesignDay", "CustomDay1", "CustomDay2")",
        "   **   ~~~   ** Missing day types will have 0.0 as Schedule Values",
    });

    compare_err_stream(expected_error);

    auto const *sch = dynamic_cast<Sched::ScheduleDetailed const *>(Sched::GetSchedule(*state, "WINDOWVENTSCHED"));
    EXPECT_NE(sch, nullptr);
    EXPECT_EQ(367, sch->weekScheds.size());
    EXPECT_EQ(sch->weekScheds.front(), nullptr);
    const auto &weekSched = sch->weekScheds[1];
    EXPECT_EQ("WINDOWVENTSCHED_wk_1", weekSched->Name);
    for (size_t i = 2; i < 367; ++i) {
        EXPECT_EQ(weekSched, sch->weekScheds[i]);
    }
    EXPECT_EQ((int)Sched::DayType::Num, weekSched->dayScheds.size());

    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::Unused]);
    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::Sunday]);

    ASSERT_NE(nullptr, weekSched->dayScheds[(int)Sched::DayType::Monday]);
    ASSERT_NE(nullptr, weekSched->dayScheds[(int)Sched::DayType::Tuesday]);
    ASSERT_NE(nullptr, weekSched->dayScheds[(int)Sched::DayType::Wednesday]);
    ASSERT_NE(nullptr, weekSched->dayScheds[(int)Sched::DayType::Thursday]);
    ASSERT_NE(nullptr, weekSched->dayScheds[(int)Sched::DayType::Friday]);
    ASSERT_NE(nullptr, weekSched->dayScheds[(int)Sched::DayType::Saturday]);
    auto const &daySched = weekSched->dayScheds[(int)Sched::DayType::Monday];
    for (int i = (int)Sched::DayType::Monday; i <= (int)Sched::DayType::Saturday; ++i) {
        EXPECT_EQ(daySched, weekSched->dayScheds[i]);
    }

    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::Holiday]);
    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::SummerDesignDay]);
    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::WinterDesignDay]);
    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::CustomDay1]);
    EXPECT_EQ(nullptr, weekSched->dayScheds[(int)Sched::DayType::CustomDay2]);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    s_glob->HourOfDay = 1;
    s_glob->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->HolidayIndex = 0;

    // Monday is defined, so we should get 1.0
    state->dataEnvrn->DayOfWeek = 2;
    EXPECT_NEAR(1.0, sch->getHrTsVal(*state, 7, 4), 0.000001);

    // Now test a day that is not defined, like Sunday
    // We shouldn't segfault, and it should default to returning 0.0
    state->dataEnvrn->DayOfWeek = 1;
    ASSERT_NO_THROW(sch->getHrTsVal(*state, 7, 4));
    EXPECT_NEAR(0.0, sch->getHrTsVal(*state, 7, 4), 0.000001);
}
