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

// EnergyPlus::SQLite Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/SQLiteFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>

namespace EnergyPlus {

using namespace OutputProcessor;

TEST_F(SQLiteFixture, SQLiteProcedures_sqliteWriteMessage)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteWriteMessage("");
    EXPECT_EQ("SQLite3 message, \n", ss->str());
    ss->str(std::string());
    sql->sqliteWriteMessage("test message");
    EXPECT_EQ("SQLite3 message, test message\n", ss->str());
    ss->str(std::string());
}

TEST_F(SQLiteFixture, SQLiteProcedures_initializeIndexes)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->initializeIndexes();
    sql->sqliteCommit();

    EXPECT_TRUE(indexExists("rddMTR"));
    EXPECT_TRUE(indexExists("redRD"));
    EXPECT_FALSE(indexExists("dmhdHRI"));
    EXPECT_FALSE(indexExists("dmhrMNI"));
    EXPECT_FALSE(indexExists("tdI"));
}

TEST_F(SQLiteFixture, SQLiteProcedures_simulationRecords)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    // There needs to be a simulation record otherwise updateSQLiteSimulationRecord will fail
    sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");
    sql->createSQLiteSimulationsRecord(2, "EnergyPlus Version", "Current Time");
    sql->createSQLiteSimulationsRecord(3, "EnergyPlus Version", "Current Time");
    sql->updateSQLiteSimulationRecord(1, 6);
    sql->updateSQLiteSimulationRecord(true, false, 2);
    sql->updateSQLiteSimulationRecord(true, true, 3);
    auto result = queryResult("SELECT * FROM Simulations;", "Simulations");
    sql->sqliteCommit();

    ASSERT_EQ(3ul, result.size());
    std::vector<std::string> testResult0{"1", "EnergyPlus Version", "Current Time", "6", "FALSE", "FALSE"};
    std::vector<std::string> testResult1{"2", "EnergyPlus Version", "Current Time", "", "1", "0"};
    std::vector<std::string> testResult2{"3", "EnergyPlus Version", "Current Time", "", "1", "1"};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
    EXPECT_EQ(testResult2, result[2]);

    sql->sqliteBegin();
    sql->updateSQLiteSimulationRecord(true, true);
    result = queryResult("SELECT * FROM Simulations;", "Simulations");
    sql->sqliteCommit();

    ASSERT_EQ(3ul, result.size());
    std::vector<std::string> testResult3{"1", "EnergyPlus Version", "Current Time", "6", "1", "1"};
    EXPECT_EQ(testResult3, result[0]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteEnvironmentPeriodRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    // There needs to be a simulation record otherwise the foreign key constraint will fail
    sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");
    sql->createSQLiteEnvironmentPeriodRecord(1, "CHICAGO ANN HTG 99.6% CONDNS DB", Constant::KindOfSim::DesignDay);
    sql->createSQLiteEnvironmentPeriodRecord(2, "CHICAGO ANN CLG .4% CONDNS WB=>MDB", Constant::KindOfSim::DesignDay, 1);
    sql->createSQLiteEnvironmentPeriodRecord(3, "CHICAGO ANN HTG 99.6% CONDNS DB", Constant::KindOfSim::RunPeriodDesign);
    sql->createSQLiteEnvironmentPeriodRecord(4, "CHICAGO ANN CLG .4% CONDNS WB=>MDB", Constant::KindOfSim::RunPeriodWeather, 1);
    auto result = queryResult("SELECT * FROM EnvironmentPeriods;", "EnvironmentPeriods");
    sql->sqliteCommit();

    ASSERT_EQ(4ul, result.size());
    std::vector<std::string> testResult0{"1", "1", "CHICAGO ANN HTG 99.6% CONDNS DB", "1"};
    std::vector<std::string> testResult1{"2", "1", "CHICAGO ANN CLG .4% CONDNS WB=>MDB", "1"};
    std::vector<std::string> testResult2{"3", "1", "CHICAGO ANN HTG 99.6% CONDNS DB", "2"};
    std::vector<std::string> testResult3{"4", "1", "CHICAGO ANN CLG .4% CONDNS WB=>MDB", "3"};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
    EXPECT_EQ(testResult2, result[2]);
    EXPECT_EQ(testResult3, result[3]);

    sql->sqliteBegin();
    // This should fail to insert due to foreign key constraint
    sql->createSQLiteEnvironmentPeriodRecord(5, "CHICAGO ANN HTG 99.6% CONDNS DB", Constant::KindOfSim::DesignDay, 100);
    // This should fail to insert due to duplicate primary key
    sql->createSQLiteEnvironmentPeriodRecord(4, "CHICAGO ANN CLG .4% CONDNS WB=>MDB", Constant::KindOfSim::DesignDay, 1);
    result = queryResult("SELECT * FROM EnvironmentPeriods;", "EnvironmentPeriods");
    sql->sqliteCommit();

    EXPECT_EQ(4ul, result.size());
}

TEST_F(SQLiteFixture, SQLiteProcedures_errorRecords)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    // There needs to be a simulation record otherwise the foreign key constraint will fail
    sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");
    sql->createSQLiteErrorRecord(1, 0, "CheckUsedConstructions: There are 2 nominally unused constructions in input.", 1);
    auto result = queryResult("SELECT * FROM Errors;", "Errors");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, result.size());
    std::vector<std::string> testResult0{"1", "1", "0", "CheckUsedConstructions: There are 2 nominally unused constructions in input.", "1"};
    EXPECT_EQ(testResult0, result[0]);

    sql->sqliteBegin();
    // updateSQLiteErrorRecord appends the message to the current error message of the last error in the table
    sql->updateSQLiteErrorRecord("New error message");
    result = queryResult("SELECT * FROM Errors;", "Errors");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, result.size());
    std::vector<std::string> testResult1{
        "1", "1", "0", "CheckUsedConstructions: There are 2 nominally unused constructions in input.  New error message", "1"};
    EXPECT_EQ(testResult1, result[0]);

    sql->sqliteBegin();
    sql->createSQLiteErrorRecord(1, 0, "CheckUsedConstructions: There are 2 nominally unused constructions in input.", 1);
    sql->createSQLiteErrorRecord(1, 0, "This should be changed.", 1);
    sql->updateSQLiteErrorRecord("Changed error message.");
    result = queryResult("SELECT * FROM Errors;", "Errors");
    sql->sqliteCommit();

    ASSERT_EQ(3ul, result.size());
    std::vector<std::string> testResult2{
        "1", "1", "0", "CheckUsedConstructions: There are 2 nominally unused constructions in input.  New error message", "1"};
    std::vector<std::string> testResult3{"2", "1", "0", "CheckUsedConstructions: There are 2 nominally unused constructions in input.", "1"};
    std::vector<std::string> testResult4{"3", "1", "0", "This should be changed.  Changed error message.", "1"};
    EXPECT_EQ(testResult2, result[0]);
    EXPECT_EQ(testResult3, result[1]);
    EXPECT_EQ(testResult4, result[2]);

    sql->sqliteBegin();
    // This should fail to insert due to foreign key constraint
    sql->createSQLiteErrorRecord(100, 0, "CheckUsedConstructions: There are 2 nominally unused constructions in input.", 1);
    result = queryResult("SELECT * FROM Errors;", "Errors");
    sql->sqliteCommit();

    EXPECT_EQ(3ul, result.size());
}

TEST_F(SQLiteFixture, SQLiteProcedures_sqliteWithinTransaction)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    EXPECT_FALSE(sql->sqliteWithinTransaction());
    sql->sqliteBegin();
    EXPECT_TRUE(sql->sqliteWithinTransaction());
    sql->sqliteCommit();
    EXPECT_FALSE(sql->sqliteWithinTransaction());
}

TEST_F(SQLiteFixture, SQLiteProcedures_informationalErrorRecords)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    // There needs to be a simulation record otherwise the foreign key constraint will fail
    sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    ShowMessage(*state, "This is an informational message");

    auto result = queryResult("SELECT * FROM Errors;", "Errors");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, result.size());
    std::vector<std::string> testResult0{"1", "1", "-1", "This is an informational message", "0"};
    EXPECT_EQ(testResult0, result[0]);

    std::string const errMsg = delimited_string({"   ************* This is an informational message"});

    compare_err_stream(errMsg);
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteReportDictionaryRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->createSQLiteReportDictionaryRecord(
        1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
    sql->createSQLiteReportDictionaryRecord(
        2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);
    sql->createSQLiteReportDictionaryRecord(
        3, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Month, true);
    sql->createSQLiteReportDictionaryRecord(4, StoreType::Average, "HVAC", "", "AHU-1", TimeStepType::System, "", ReportFreq::Hour, false);
    sql->createSQLiteReportDictionaryRecord(
        5, StoreType::Average, "HVAC", "", "AHU-1", TimeStepType::System, "", ReportFreq::Hour, false, "test schedule");
    auto result = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");
    sql->sqliteCommit();

    ASSERT_EQ(5ul, result.size());
    std::vector<std::string> testResult0{"1", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Hourly", "", "C"};
    std::vector<std::string> testResult1{"2", "1", "Sum", "Facility:Electricity", "Zone", "", "Facility:Electricity", "Hourly", "", "J"};
    std::vector<std::string> testResult2{"3", "1", "Sum", "Facility:Electricity", "Zone", "", "Facility:Electricity", "Monthly", "", "J"};
    std::vector<std::string> testResult3{"4", "0", "Avg", "HVAC", "HVAC System", "", "AHU-1", "Hourly", "", ""};
    std::vector<std::string> testResult4{"5", "0", "Avg", "HVAC", "HVAC System", "", "AHU-1", "Hourly", "test schedule", ""};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
    EXPECT_EQ(testResult2, result[2]);
    EXPECT_EQ(testResult3, result[3]);
    EXPECT_EQ(testResult4, result[4]);

    // Do we still need to do these tests now that ReportFreq, StoreType, and TimeStep type are enums?
    sql->sqliteBegin();
    sql->createSQLiteReportDictionaryRecord(
        6, StoreType::Invalid, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
    sql->createSQLiteReportDictionaryRecord(
        7, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Invalid, "J", ReportFreq::Hour, true);
    sql->createSQLiteReportDictionaryRecord(
        8, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Invalid, true);
    sql->createSQLiteReportDictionaryRecord(9, StoreType::Average, "HVAC", "", "AHU-1", TimeStepType::System, "", ReportFreq::Invalid, false);
    result = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");
    sql->sqliteCommit();

    ASSERT_EQ(9ul, result.size());
    std::vector<std::string> testResult5{
        "6", "0", "Unknown!!!", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Hourly", "", "C"};
    std::vector<std::string> testResult6{"7", "1", "Sum", "Facility:Electricity", "Unknown!!!", "", "Facility:Electricity", "Hourly", "", "J"};
    std::vector<std::string> testResult7{"8", "1", "Sum", "Facility:Electricity", "Zone", "", "Facility:Electricity", "Unknown!!!", "", "J"};
    std::vector<std::string> testResult8{"9", "0", "Avg", "HVAC", "HVAC System", "", "AHU-1", "Unknown!!!", "", ""};
    EXPECT_EQ(testResult5, result[5]);
    EXPECT_EQ(testResult6, result[6]);
    EXPECT_EQ(testResult7, result[7]);
    EXPECT_EQ(testResult8, result[8]);

    sql->sqliteBegin();
    // This should fail to insert due to duplicate primary key
    sql->createSQLiteReportDictionaryRecord(
        9, StoreType::Invalid, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
    result = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");
    sql->sqliteCommit();
    EXPECT_EQ(9ul, result.size());
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteTimeIndexRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2017, false, 1);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Day, 1, 1, 0, 2017, false, 1, 1, 1, -1, -1, 0, "WinterDesignDay");
    sql->createSQLiteTimeIndexRecord(ReportFreq::Hour, 1, 1, 0, 2017, false, 1, 2, 2, -1, -1, 0, "SummerDesignDay");
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 0, 2017, false, 1, 1, 1, 60, 0, 0, "WinterDesignDay");
    sql->createSQLiteTimeIndexRecord(ReportFreq::EachCall, 1, 1, 0, 2017, false, 1, 2, 2, 60, 0, 0, "SummerDesignDay");
    sql->createSQLiteTimeIndexRecord(ReportFreq::EachCall, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, 0, "SummerDesignDay", true);
    auto result = queryResult("SELECT * FROM Time;", "Time");
    sql->sqliteCommit();

    ASSERT_EQ(7ul, result.size());
    // some of these are odd.........
    std::vector<std::string> testResult0{"1", "", "", "", "", "", "", "1440", "4", "1", "", "0", ""};
    std::vector<std::string> testResult1{"2", "2017", "1", "31", "24", "0", "", "44640", "3", "1", "", "0", ""};
    std::vector<std::string> testResult2{"3", "2017", "1", "1", "24", "0", "0", "1440", "2", "1", "WinterDesignDay", "0", ""};
    std::vector<std::string> testResult3{"4", "2017", "1", "2", "2", "0", "0", "60", "1", "1", "SummerDesignDay", "0", ""};
    std::vector<std::string> testResult4{"5", "2017", "1", "1", "1", "0", "0", "60", "0", "1", "WinterDesignDay", "0", "0"};
    std::vector<std::string> testResult5{"6", "2017", "1", "2", "2", "0", "0", "60", "-1", "1", "SummerDesignDay", "0", "0"};
    std::vector<std::string> testResult6{"7", "2017", "1", "3", "3", "0", "0", "60", "-1", "1", "SummerDesignDay", "1", "1"};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
    EXPECT_EQ(testResult2, result[2]);
    EXPECT_EQ(testResult3, result[3]);
    EXPECT_EQ(testResult4, result[4]);
    EXPECT_EQ(testResult5, result[5]);
    EXPECT_EQ(testResult6, result[6]);

    sql->sqliteBegin();
    sql->createSQLiteTimeIndexRecord(ReportFreq::Invalid, 1, 1, 0, 2017, false);
    sql->sqliteCommit();
    EXPECT_EQ("SQLite3 message, Illegal reportingInterval passed to CreateSQLiteTimeIndexRecord: -1\n", ss->str());
    ss->str(std::string());

    EXPECT_EQ(7ul, result.size());

    sql->sqliteBegin();
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, 0, "", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, -1, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, 1, 3, 3, 60, -1, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, 1, 3, 3, -1, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, 1, 3, -1, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, 1, -1, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::TimeStep, 1, 1, 1, 2017, false, -1, 3, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Hour, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, 0, "", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Hour, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, -1, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Hour, 1, 1, 1, 2017, false, 1, 3, -1, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Hour, 1, 1, 1, 2017, false, 1, -1, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Hour, 1, 1, 1, 2017, false, -1, 3, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Day, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, 0, "", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Day, 1, 1, 1, 2017, false, 1, 3, 3, 60, 0, -1, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Day, 1, 1, 1, 2017, false, 1, 3, -1, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Day, 1, 1, 1, 2017, false, 1, -1, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Day, 1, 1, 1, 2017, false, -1, 3, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 1, 2017, false, -1, 3, 3, 60, 0, 0, "SummerDesignDay", true);
    sql->sqliteCommit();
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteTimeIndexRecord_NonLeapDay)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    // set the leap year flag to false (6th argument) and expect the last day of february to be the 28th
    sql->sqliteBegin();
    sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2012, false);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, false, 1);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, false, 2); // February
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, false, 3);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, false, 4);
    auto result = queryResult("SELECT * FROM Time;", "Time");
    sql->sqliteCommit();

    ASSERT_EQ(5ul, result.size());
    std::vector<std::string> testResult0{"1", "", "", "", "", "", "", "1440", "4", "1", "", "0", ""};
    std::vector<std::string> testResult1{"2", "2012", "1", "31", "24", "0", "", "44640", "3", "1", "", "0", ""};
    std::vector<std::string> testResult2{"3", "2012", "2", "28", "24", "0", "", "40320", "3", "1", "", "0", ""}; // February
    std::vector<std::string> testResult3{"4", "2012", "3", "31", "24", "0", "", "44640", "3", "1", "", "0", ""};
    std::vector<std::string> testResult4{"5", "2012", "4", "30", "24", "0", "", "43200", "3", "1", "", "0", ""};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
    EXPECT_EQ(testResult2, result[2]);
    EXPECT_EQ(testResult3, result[3]);
    EXPECT_EQ(testResult4, result[4]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteTimeIndexRecord_LeapDay)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    // set the leap year flag to true (6th argument) and expect the last day of february to be the 29th
    sql->sqliteBegin();
    sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2012, true);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, true, 1);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, true, 2); // February
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, true, 3);
    sql->createSQLiteTimeIndexRecord(ReportFreq::Month, 1, 1, 0, 2012, true, 4);
    auto result = queryResult("SELECT * FROM Time;", "Time");
    sql->sqliteCommit();

    ASSERT_EQ(5ul, result.size());
    std::vector<std::string> testResult0{"1", "", "", "", "", "", "", "1440", "4", "1", "", "0", ""};
    std::vector<std::string> testResult1{"2", "2012", "1", "31", "24", "0", "", "44640", "3", "1", "", "0", ""};
    std::vector<std::string> testResult2{"3", "2012", "2", "29", "24", "0", "", "41760", "3", "1", "", "0", ""}; // February
    std::vector<std::string> testResult3{"4", "2012", "3", "31", "24", "0", "", "44640", "3", "1", "", "0", ""};
    std::vector<std::string> testResult4{"5", "2012", "4", "30", "24", "0", "", "43200", "3", "1", "", "0", ""};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
    EXPECT_EQ(testResult2, result[2]);
    EXPECT_EQ(testResult3, result[3]);
    EXPECT_EQ(testResult4, result[4]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteReportDataRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
    sql->createSQLiteReportDictionaryRecord(
        1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
    sql->createSQLiteReportDataRecord(1, 999.9);
    sql->createSQLiteReportDataRecord(1, 999.9, ReportFreq::Day, 0, 1310459, 100, 7031530, 15);
    sql->createSQLiteReportDataRecord(1, 999.9, ReportFreq::TimeStep, 0, 1310459, 100, 7031530, 15);
    sql->createSQLiteReportDataRecord(1, 999.9, ReportFreq::Day, 100, 1310459, 999, 7031530, -1);
    auto reportData = queryResult("SELECT * FROM ReportData;", "ReportData");
    auto reportExtendedData = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");
    sql->sqliteCommit();

    ASSERT_EQ(4ul, reportData.size());
    std::vector<std::string> reportData0{"1", "1", "1", "999.9"};
    std::vector<std::string> reportData1{"2", "1", "1", "999.9"};
    std::vector<std::string> reportData2{"3", "1", "1", "999.9"};
    std::vector<std::string> reportData3{"4", "1", "1", "999.9"};
    EXPECT_EQ(reportData0, reportData[0]);
    EXPECT_EQ(reportData1, reportData[1]);
    EXPECT_EQ(reportData2, reportData[2]);
    EXPECT_EQ(reportData3, reportData[3]);

    ASSERT_EQ(2ul, reportExtendedData.size());
    std::vector<std::string> reportExtendedData0{"1", "2", "100.0", "7", "3", "14", "16", "30", "0.0", "1", "31", "3", "45", "59"};
    std::vector<std::string> reportExtendedData1{"2", "4", "999.0", "7", "3", "14", "", "30", "100.0", "1", "31", "3", "", "59"};
    EXPECT_EQ(reportExtendedData0, reportExtendedData[0]);
    EXPECT_EQ(reportExtendedData1, reportExtendedData[1]);

    sql->sqliteBegin();
    sql->createSQLiteReportDataRecord(1, 999.9, ReportFreq::Invalid, 0, 1310459, 100, 7031530, 15);
    sql->sqliteCommit();
    EXPECT_EQ("SQLite3 message, Illegal reportingInterval passed to CreateSQLiteMeterRecord: -1\n", ss->str());
    ss->str(std::string());

    sql->sqliteBegin();
    sql->createSQLiteReportDataRecord(1, 999.9, ReportFreq::Invalid, 0, 1310459, 100, 7031530, -1);
    sql->sqliteCommit();
    EXPECT_EQ("SQLite3 message, Illegal reportingInterval passed to CreateSQLiteMeterRecord: -1\n", ss->str());
    ss->str(std::string());

    EXPECT_EQ(4ul, reportData.size());
    EXPECT_EQ(2ul, reportExtendedData.size());
}

TEST_F(SQLiteFixture, SQLiteProcedures_addSQLiteZoneSizingRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->addSQLiteZoneSizingRecord(
        "FLOOR 1 IT HALL", "Cooling", 175, 262, 0.013, 0.019, "CHICAGO ANN CLG .4% CONDNS WB=>MDB", "7/21 06:00:00", 20.7, 0.0157, 0.0033, 416.7);
    auto result = queryResult("SELECT * FROM ZoneSizes;", "ZoneSizes");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, result.size());
    std::vector<std::string> testResult0{"1",
                                         "FLOOR 1 IT HALL",
                                         "Cooling",
                                         "175.0",
                                         "262.0",
                                         "0.013",
                                         "0.019",
                                         "CHICAGO ANN CLG .4% CONDNS WB=>MDB",
                                         "7/21 06:00:00",
                                         "20.7",
                                         "0.0157",
                                         "0.0033",
                                         "416.7"};
    EXPECT_EQ(testResult0, result[0]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_addSQLiteSystemSizingRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->addSQLiteSystemSizingRecord("VAV_1", "Cooling", "Sensible", 23.3, 6.3, 6.03, "CHICAGO ANN CLG .4% CONDNS WB=>MDB", "7/21 06:00:00");
    auto result = queryResult("SELECT * FROM SystemSizes;", "SystemSizes");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, result.size());
    std::vector<std::string> testResult0{
        "1", "VAV_1", "Cooling", "Sensible", "23.3", "6.3", "6.03", "CHICAGO ANN CLG .4% CONDNS WB=>MDB", "7/21 06:00:00"};
    EXPECT_EQ(testResult0, result[0]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_addSQLiteComponentSizingRecord)
{
    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->addSQLiteComponentSizingRecord(
        "AirTerminal:SingleDuct:VAV:Reheat", "CORE_BOTTOM VAV BOX COMPONENT", "Design Size Maximum Air Flow Rate [m3/s]", 3.23);
    sql->addSQLiteComponentSizingRecord("Coil:Heating:Electric", "CORE_BOTTOM VAV BOX REHEAT COIL", "Design Size Nominal Capacity", 38689.18);
    auto result = queryResult("SELECT * FROM ComponentSizes;", "ComponentSizes");
    sql->sqliteCommit();

    ASSERT_EQ(2ul, result.size());
    std::vector<std::string> testResult0{
        "1", "AirTerminal:SingleDuct:VAV:Reheat", "CORE_BOTTOM VAV BOX COMPONENT", "Design Size Maximum Air Flow Rate", "3.23", "m3/s", ""};
    std::vector<std::string> testResult1{
        "2", "Coil:Heating:Electric", "CORE_BOTTOM VAV BOX REHEAT COIL", "Design Size Nominal Capacity", "38689.18", "", ""};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_privateMethods)
{
#ifdef GET_OUT
    // Don't need this test anymore we are using Enums
    // test storageType
    EXPECT_EQ("Avg", storageType(1));
    EXPECT_EQ("Sum", storageType(2));
    EXPECT_EQ("Unknown!!!", storageType(3));
    EXPECT_EQ("Unknown!!!", storageType(-1));

    // test timestepTypeName
    EXPECT_EQ("Zone", timestepTypeName(1));
    EXPECT_EQ("HVAC System", timestepTypeName(2));
    EXPECT_EQ("Unknown!!!", timestepTypeName(3));
    EXPECT_EQ("Unknown!!!", timestepTypeName(-1));
    // Let's ensure we never get an unexpected change of mapping between enum and the corresponding int value
    EXPECT_EQ(1, static_cast<int>(OutputProcessor::TimeStepType::Zone));
    EXPECT_EQ(2, static_cast<int>(OutputProcessor::TimeStepType::System));

    // test reportingFreqName
    EXPECT_EQ("HVAC System Timestep", reportingFreqName(-1));
    EXPECT_EQ("Zone Timestep", reportingFreqName(0));
    EXPECT_EQ("Hourly", reportingFreqName(1));
    EXPECT_EQ("Daily", reportingFreqName(2));
    EXPECT_EQ("Monthly", reportingFreqName(3));
    EXPECT_EQ("Run Period", reportingFreqName(4));
    EXPECT_EQ("Annual", reportingFreqName(5));
    EXPECT_EQ("Unknown!!!", reportingFreqName(6));
    EXPECT_EQ("Unknown!!!", reportingFreqName(-2));
#endif // GET_OUT

    EXPECT_EQ(1, logicalToInteger(true));
    EXPECT_EQ(0, logicalToInteger(false));

    int hour = 0, minutes = 0;
    adjustReportingHourAndMinutes(hour, minutes);
    EXPECT_EQ(-1, hour);
    EXPECT_EQ(0, minutes);
    hour = 1, minutes = 60;
    adjustReportingHourAndMinutes(hour, minutes);
    EXPECT_EQ(1, hour);
    EXPECT_EQ(0, minutes);
    hour = 1, minutes = 65;
    adjustReportingHourAndMinutes(hour, minutes);
    EXPECT_EQ(0, hour);
    EXPECT_EQ(65, minutes);
}

TEST_F(SQLiteFixture, SQLiteProcedures_DaylightMaping)
{
    auto const &zone = std::make_unique<DataHeatBalance::ZoneData>();
    zone->Name = "DAYLIT ZONE";
    zone->CeilingHeight = 3;
    zone->Volume = 302;

    Array1D<Real64> XValue({50.1, 51.3});
    Array1D<Real64> YValue({50.1, 52.1});
    Array2D<Real64> IllumValue(2, 2, {1, 3, 2, 4});

    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    sql->addZoneData(1, *zone);
    sql->createZoneExtendedOutput();
    sql->createSQLiteDaylightMapTitle(1, "DAYLIT ZONE:CHICAGO", "CHICAGO ANN CLG", 1, " RefPt1=(2.50:2.00:0.80), RefPt2=(2.50:18.00:0.80)", 0.8);
    sql->createSQLiteDaylightMap(1, 2005, 7, 21, 5, XValue.size(), XValue, YValue.size(), YValue, IllumValue);

    auto zones = queryResult("SELECT * FROM Zones;", "Zones");
    auto daylightMaps = queryResult("SELECT * FROM DaylightMaps;", "DaylightMaps");
    auto daylightMapHourlyData = queryResult("SELECT * FROM DaylightMapHourlyData;", "DaylightMapHourlyData");
    auto daylightMapHourlyReports = queryResult("SELECT * FROM DaylightMapHourlyReports;", "DaylightMapHourlyReports");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, zones.size());
    std::vector<std::string> zone0{"1",   "DAYLIT ZONE", "0.0", "0.0", "0.0", "0.0",   "0.0", "0.0", "0.0", "1",   "1.0", "1.0", "0.0", "0.0",
                                   "0.0", "0.0",         "0.0", "0.0", "3.0", "302.0", "1",   "1",   "0.0", "0.0", "0.0", "0.0", "1"};
    EXPECT_EQ(zone0, zones[0]);

    ASSERT_EQ(1ul, daylightMaps.size());
    std::vector<std::string> daylightMap0{
        "1", "DAYLIT ZONE:CHICAGO", "CHICAGO ANN CLG", "1", " RefPt1=(2.50:2.00:0.80), RefPt2=(2.50:18.00:0.80)", "0.8"};
    EXPECT_EQ(daylightMap0, daylightMaps[0]);

    ASSERT_EQ(1ul, daylightMapHourlyReports.size());
    std::vector<std::string> daylightMapHourlyReport0{"1", "1", "2005", "7", "21", "5"};
    EXPECT_EQ(daylightMapHourlyReport0, daylightMapHourlyReports[0]);

    ASSERT_EQ(4ul, daylightMapHourlyData.size());
    std::vector<std::string> daylightMapHourlyData0{"1", "1", "50.1", "50.1", "1.0"};
    std::vector<std::string> daylightMapHourlyData1{"2", "1", "51.3", "50.1", "2.0"};
    std::vector<std::string> daylightMapHourlyData2{"3", "1", "50.1", "52.1", "3.0"};
    std::vector<std::string> daylightMapHourlyData3{"4", "1", "51.3", "52.1", "4.0"};
    EXPECT_EQ(daylightMapHourlyData0, daylightMapHourlyData[0]);
    EXPECT_EQ(daylightMapHourlyData1, daylightMapHourlyData[1]);
    EXPECT_EQ(daylightMapHourlyData2, daylightMapHourlyData[2]);
    EXPECT_EQ(daylightMapHourlyData3, daylightMapHourlyData[3]);

    sql->sqliteBegin();
    // this should fail due to missing foreign key
    sql->createSQLiteDaylightMapTitle(2, "test", "test", 2, "test,test", 0.8);
    // this should fail due to duplicate primary key
    sql->createSQLiteDaylightMapTitle(1, "test", "test", 1, "test,test", 0.8);
    // this should fail due to missing foreign key
    sql->createSQLiteDaylightMap(2, 2005, 7, 21, 5, XValue.size(), XValue, YValue.size(), YValue, IllumValue);
    daylightMaps = queryResult("SELECT * FROM DaylightMaps;", "DaylightMaps");
    daylightMapHourlyData = queryResult("SELECT * FROM DaylightMapHourlyData;", "DaylightMapHourlyData");
    daylightMapHourlyReports = queryResult("SELECT * FROM DaylightMapHourlyReports;", "DaylightMapHourlyReports");
    sql->sqliteCommit();

    ASSERT_EQ(1ul, daylightMaps.size());
    ASSERT_EQ(1ul, daylightMapHourlyReports.size());
    ASSERT_EQ(4ul, daylightMapHourlyData.size());
}

TEST_F(SQLiteFixture, SQLiteProcedures_createZoneExtendedOutput)
{
    state->init_state(*state);

    auto const &zoneData0 = std::make_unique<DataHeatBalance::ZoneData>(); // Why make_unique here? And why is this a reference?
    zoneData0->Name = "test zone 1";
    zoneData0->CeilingHeight = 1;
    zoneData0->Volume = 1;
    auto const &zoneData1 = std::make_unique<DataHeatBalance::ZoneData>();
    zoneData1->Name = "test zone 2";
    zoneData1->RelNorth = 2;
    zoneData1->OriginX = 2;
    zoneData1->OriginY = 2;
    zoneData1->OriginZ = 2;
    zoneData1->Centroid.x = 2;
    zoneData1->Centroid.y = 2;
    zoneData1->Centroid.z = 2;
    zoneData1->OfType = 2;
    zoneData1->Multiplier = 2;
    zoneData1->ListMultiplier = 2;
    zoneData1->MinimumX = 2;
    zoneData1->MaximumX = 2;
    zoneData1->MinimumY = 2;
    zoneData1->MaximumY = 2;
    zoneData1->MinimumZ = 2;
    zoneData1->MaximumZ = 2;
    zoneData1->CeilingHeight = 2;
    zoneData1->Volume = 2;
    zoneData1->IntConvAlgo = Convect::HcInt::ASHRAETARP;
    zoneData1->ExtConvAlgo = Convect::HcExt::ASHRAETARP;
    zoneData1->FloorArea = 2;
    zoneData1->ExtGrossWallArea = 2;
    zoneData1->ExtNetWallArea = 2;
    zoneData1->ExtWindowArea = 2;
    zoneData1->isPartOfTotalArea = false;

    auto const zoneListData0 = std::make_unique<DataHeatBalance::ZoneListData>();
    zoneListData0->Name = "test zoneList 1";
    zoneListData0->Zone.allocate(1);
    zoneListData0->Zone(1) = 1;
    auto const zoneListData1 = std::make_unique<DataHeatBalance::ZoneListData>();
    zoneListData1->Name = "test zoneList 2";
    zoneListData1->Zone.allocate(2);
    zoneListData1->Zone(1) = 1;
    zoneListData1->Zone(2) = 2;

    auto const zoneGroupData0 = std::make_unique<DataHeatBalance::ZoneGroupData>();
    zoneGroupData0->Name = "test zoneGroup 1";
    auto const zoneGroupData1 = std::make_unique<DataHeatBalance::ZoneGroupData>();
    zoneGroupData1->Name = "test zoneGroup 2";
    zoneGroupData1->ZoneList = 2;
    zoneGroupData1->Multiplier = 99;

    auto const materialData0 = std::make_unique<Material::MaterialBase>();
    materialData0->Name = "test material 1";
    materialData0->group = Material::Group::AirGap;
    auto const materialData1 = std::make_unique<Material::MaterialShade>();
    materialData1->Name = "test material 2";
    materialData1->group = Material::Group::Shade;
    materialData1->Roughness = Material::SurfaceRoughness::Rough; // 1
    materialData1->Conductivity = 2;
    materialData1->Density = 2;
    // materialData1->IsoMoistCap = 2;
    materialData1->Porosity = 2;
    materialData1->Resistance = 2;
    materialData1->ROnly = true;
    materialData1->SpecHeat = 2;
    // materialData1->ThermGradCoef = 2;
    materialData1->Thickness = 2;
    materialData1->VaporDiffus = 2;

    auto const constructData0 = std::make_unique<Construction::ConstructionProps>();
    constructData0->Name = "test construction 1";
    auto const constructData1 = std::make_unique<Construction::ConstructionProps>();
    constructData1->Name = "test construction 2";
    constructData1->TotLayers = 2;
    constructData1->TotSolidLayers = 2;
    constructData1->TotGlassLayers = 2;
    constructData1->InsideAbsorpVis = 2;
    constructData1->OutsideAbsorpVis = 2;
    constructData1->InsideAbsorpSolar = 2;
    constructData1->OutsideAbsorpSolar = 2;
    constructData1->InsideAbsorpThermal = 2;
    constructData1->OutsideAbsorpThermal = 2;
    constructData1->OutsideRoughness = Material::SurfaceRoughness::Rough; // 1
    constructData1->TypeIsWindow = true;
    constructData1->LayerPoint.allocate(2);
    constructData1->LayerPoint(1) = 2;
    constructData1->LayerPoint(2) = 1;

    auto const surfaceData0 = std::make_unique<DataSurfaces::SurfaceData>();
    surfaceData0->Name = "test surface 1";
    auto const surfaceData1 = std::make_unique<DataSurfaces::SurfaceData>();
    surfaceData1->Name = "test surface 2";
    surfaceData1->Construction = 2;
    surfaceData1->Area = 2;
    surfaceData1->GrossArea = 2;
    surfaceData1->Perimeter = 2;
    surfaceData1->Azimuth = 2;
    surfaceData1->Height = 2;
    surfaceData1->Reveal = 2;
    surfaceData1->Shape = DataSurfaces::SurfaceShape::Quadrilateral;
    surfaceData1->Sides = 2;
    surfaceData1->Tilt = 2;
    surfaceData1->Width = 2;
    surfaceData1->HeatTransSurf = true;
    surfaceData1->BaseSurf = 1;
    surfaceData1->Zone = 1;
    surfaceData1->ExtBoundCond = 2;
    surfaceData1->ExtSolar = true;
    surfaceData1->ExtWind = true;

    auto const lightingData0 = std::make_unique<DataHeatBalance::LightsData>();
    lightingData0->Name = "test lighting 1";
    lightingData0->sched = Sched::GetScheduleAlwaysOff(*state);

    auto const lightingData1 = std::make_unique<DataHeatBalance::LightsData>();
    lightingData1->Name = "test lighting 2";
    lightingData1->ZonePtr = 1;
    lightingData1->sched = Sched::AddScheduleConstant(*state, "SCHED-1");
    lightingData1->DesignLevel = 2;
    lightingData1->FractionReturnAir = 2;
    lightingData1->FractionRadiant = 2;
    lightingData1->FractionShortWave = 2;
    lightingData1->FractionReplaceable = 2;
    lightingData1->FractionConvected = 2;
    lightingData1->EndUseSubcategory = "test";

    auto const peopleData0 = std::make_unique<DataHeatBalance::PeopleData>();
    peopleData0->Name = "test people 1";
    peopleData0->sched = Sched::GetScheduleAlwaysOff(*state);
    peopleData0->activityLevelSched = Sched::GetScheduleAlwaysOff(*state);
    peopleData0->workEffSched = Sched::GetScheduleAlwaysOff(*state);
    peopleData0->clothingSched = Sched::GetScheduleAlwaysOff(*state);
    peopleData0->airVelocitySched = Sched::GetScheduleAlwaysOff(*state);

    auto const peopleData1 = std::make_unique<DataHeatBalance::PeopleData>();
    peopleData1->Name = "test people 2";
    peopleData1->ZonePtr = 1;
    peopleData1->NumberOfPeople = 2;
    peopleData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    peopleData1->activityLevelSched = Sched::GetSchedule(*state, "SCHED-1");
    peopleData1->FractionRadiant = 2;
    peopleData1->FractionConvected = 2;
    peopleData1->workEffSched = Sched::GetSchedule(*state, "SCHED-1");
    peopleData1->clothingSched = Sched::GetSchedule(*state, "SCHED-1");
    peopleData1->airVelocitySched = Sched::GetSchedule(*state, "SCHED-1");
    peopleData1->Fanger = true;
    peopleData1->Pierce = true;
    peopleData1->KSU = true;
    peopleData1->MRTCalcType = DataHeatBalance::CalcMRT::SurfaceWeighted;
    peopleData1->SurfacePtr = 1;
    peopleData1->AngleFactorListName = "test";
    peopleData1->AngleFactorListPtr = 1;
    peopleData1->UserSpecSensFrac = 2;
    peopleData1->Show55Warning = true;

    auto const elecEquipData0 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    elecEquipData0->Name = "test elecEquip 1";
    elecEquipData0->sched = Sched::GetScheduleAlwaysOff(*state);

    auto const elecEquipData1 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    elecEquipData1->Name = "test elecEquip 2";
    elecEquipData1->ZonePtr = 1;
    elecEquipData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    elecEquipData1->DesignLevel = 2;
    elecEquipData1->FractionLatent = 2;
    elecEquipData1->FractionRadiant = 2;
    elecEquipData1->FractionLost = 2;
    elecEquipData1->FractionConvected = 2;
    elecEquipData1->EndUseSubcategory = "test";

    auto const gasEquipData0 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    gasEquipData0->Name = "test gasEquip 1";
    gasEquipData0->sched = Sched::GetScheduleAlwaysOff(*state);
    auto const gasEquipData1 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    gasEquipData1->Name = "test gasEquip 2";
    gasEquipData1->ZonePtr = 1;
    gasEquipData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    gasEquipData1->DesignLevel = 2;
    gasEquipData1->FractionLatent = 2;
    gasEquipData1->FractionRadiant = 2;
    gasEquipData1->FractionLost = 2;
    gasEquipData1->FractionConvected = 2;
    gasEquipData1->EndUseSubcategory = "test";

    auto const steamEquipData0 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    steamEquipData0->Name = "test steamEquip 1";
    steamEquipData0->sched = Sched::GetScheduleAlwaysOff(*state);
    auto const steamEquipData1 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    steamEquipData1->Name = "test steamEquip 2";
    steamEquipData1->ZonePtr = 1;
    steamEquipData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    steamEquipData1->DesignLevel = 2;
    steamEquipData1->FractionLatent = 2;
    steamEquipData1->FractionRadiant = 2;
    steamEquipData1->FractionLost = 2;
    steamEquipData1->FractionConvected = 2;
    steamEquipData1->EndUseSubcategory = "test";

    auto const hwEquipData0 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    hwEquipData0->Name = "test hwEquip 1";
    hwEquipData0->sched = Sched::GetScheduleAlwaysOff(*state);
    auto const hwEquipData1 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    hwEquipData1->Name = "test hwEquip 2";
    hwEquipData1->ZonePtr = 1;
    hwEquipData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    hwEquipData1->DesignLevel = 2;
    hwEquipData1->FractionLatent = 2;
    hwEquipData1->FractionRadiant = 2;
    hwEquipData1->FractionLost = 2;
    hwEquipData1->FractionConvected = 2;
    hwEquipData1->EndUseSubcategory = "test";

    auto const otherEquipData0 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    otherEquipData0->Name = "test otherEquip 1";
    otherEquipData0->sched = Sched::GetScheduleAlwaysOff(*state);
    auto const otherEquipData1 = std::make_unique<DataHeatBalance::ZoneEquipData>();
    otherEquipData1->Name = "test otherEquip 2";
    otherEquipData1->ZonePtr = 1;
    otherEquipData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    otherEquipData1->DesignLevel = 2;
    otherEquipData1->FractionLatent = 2;
    otherEquipData1->FractionRadiant = 2;
    otherEquipData1->FractionLost = 2;
    otherEquipData1->FractionConvected = 2;
    otherEquipData1->EndUseSubcategory = "test";

    auto const baseboardData0 = std::make_unique<DataHeatBalance::BBHeatData>();
    baseboardData0->Name = "test baseboard 1";
    baseboardData0->sched = Sched::GetScheduleAlwaysOff(*state);
    auto const baseboardData1 = std::make_unique<DataHeatBalance::BBHeatData>();
    baseboardData1->Name = "test baseboard 2";
    baseboardData1->ZonePtr = 1;
    baseboardData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    baseboardData1->CapatLowTemperature = 2;
    baseboardData1->LowTemperature = 2;
    baseboardData1->CapatHighTemperature = 2;
    baseboardData1->HighTemperature = 2;
    baseboardData1->FractionRadiant = 2;
    baseboardData1->FractionConvected = 2;
    baseboardData1->EndUseSubcategory = "test";

    auto const infiltrationData0 = std::make_unique<DataHeatBalance::InfiltrationData>();
    infiltrationData0->Name = "test infiltration 1";
    infiltrationData0->sched = Sched::GetScheduleAlwaysOff(*state);
    auto const infiltrationData1 = std::make_unique<DataHeatBalance::InfiltrationData>();
    infiltrationData1->Name = "test infiltration 2";
    infiltrationData1->ZonePtr = 1;
    infiltrationData1->sched = Sched::GetSchedule(*state, "SCHED-1");
    infiltrationData1->DesignLevel = 2;

    auto const ventilationData0 = std::make_unique<DataHeatBalance::VentilationData>();
    ventilationData0->Name = "test ventilation 1";
    ventilationData0->availSched = Sched::GetScheduleAlwaysOff(*state);
    auto const ventilationData1 = std::make_unique<DataHeatBalance::VentilationData>();
    ventilationData1->Name = "test ventilation 2";
    ventilationData1->ZonePtr = 1;
    ventilationData1->availSched = Sched::GetSchedule(*state, "SCHED-1");
    ventilationData1->DesignLevel = 2;

    auto const roomAirModelData0 = std::make_unique<RoomAir::AirModelData>();
    roomAirModelData0->Name = "test roomAirModel 1";
    auto const roomAirModelData1 = std::make_unique<RoomAir::AirModelData>();
    roomAirModelData1->Name = "test roomAirModel 2";
    roomAirModelData1->AirModel = RoomAir::RoomAirModel::DispVent1Node;
    roomAirModelData1->TempCoupleScheme = RoomAir::CouplingScheme::Direct; // hmm this was set to 3 which wasn't a valid option
    roomAirModelData1->SimAirModel = true;

    std::string const alwaysOn("always on");
    std::string const alwaysOff("always off");
    std::string const onOff("ON/OFF");
    std::string const window("Window");
    std::string const wall("Wall");
    double constexpr one = 1.0;
    double constexpr zero = 0.0;
    double constexpr two = 2.0;

    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->addScheduleData(1, alwaysOn, onOff, one, one);
    sql->addScheduleData(2, alwaysOff, onOff, zero, zero);
    sql->addZoneData(1, *zoneData0);
    sql->addZoneData(2, *zoneData1);
    sql->addZoneListData(1, *zoneListData0);
    sql->addZoneListData(2, *zoneListData1);
    sql->addZoneGroupData(1, *zoneGroupData0);
    sql->addZoneGroupData(2, *zoneGroupData1);
    sql->addMaterialData(1, &*materialData0);
    sql->addMaterialData(2, &*materialData1);
    sql->addConstructionData(1, *constructData0, zero);
    sql->addConstructionData(2, *constructData1, two);
    sql->addSurfaceData(1, *surfaceData0, window);
    sql->addSurfaceData(2, *surfaceData1, wall);
    sql->addNominalLightingData(1, *lightingData0);
    sql->addNominalLightingData(2, *lightingData1);
    sql->addNominalPeopleData(1, *peopleData0);
    sql->addNominalPeopleData(2, *peopleData1);
    sql->addNominalElectricEquipmentData(1, *elecEquipData0);
    sql->addNominalElectricEquipmentData(2, *elecEquipData1);
    sql->addNominalGasEquipmentData(1, *gasEquipData0);
    sql->addNominalGasEquipmentData(2, *gasEquipData1);
    sql->addNominalSteamEquipmentData(1, *steamEquipData0);
    sql->addNominalSteamEquipmentData(2, *steamEquipData1);
    sql->addNominalHotWaterEquipmentData(1, *hwEquipData0);
    sql->addNominalHotWaterEquipmentData(2, *hwEquipData1);
    sql->addNominalOtherEquipmentData(1, *otherEquipData0);
    sql->addNominalOtherEquipmentData(2, *otherEquipData1);
    sql->addNominalBaseboardData(1, *baseboardData0);
    sql->addNominalBaseboardData(2, *baseboardData1);
    sql->addInfiltrationData(1, *infiltrationData0);
    sql->addInfiltrationData(2, *infiltrationData1);
    sql->addVentilationData(1, *ventilationData0);
    sql->addVentilationData(2, *ventilationData1);
    sql->addRoomAirModelData(1, *roomAirModelData0);
    sql->addRoomAirModelData(2, *roomAirModelData1);

    sql->sqliteBegin();
    sql->createZoneExtendedOutput();
    auto zones = queryResult("SELECT * FROM Zones;", "Zones");
    auto zoneLists = queryResult("SELECT * FROM ZoneLists;", "ZoneLists");
    auto zoneGroups = queryResult("SELECT * FROM ZoneGroups;", "ZoneGroups");
    auto zoneInfoZoneLists = queryResult("SELECT * FROM ZoneInfoZoneLists;", "ZoneInfoZoneLists");
    auto schedules = queryResult("SELECT * FROM Schedules;", "Schedules");
    auto surfaces = queryResult("SELECT * FROM Surfaces;", "Surfaces");
    auto materials = queryResult("SELECT * FROM Materials;", "Materials");
    auto constructions = queryResult("SELECT * FROM Constructions;", "Constructions");
    auto constructionLayers = queryResult("SELECT * FROM ConstructionLayers;", "ConstructionLayers");
    auto lightings = queryResult("SELECT * FROM NominalLighting;", "NominalLighting");
    auto peoples = queryResult("SELECT * FROM NominalPeople;", "NominalPeople");
    auto elecEquips = queryResult("SELECT * FROM NominalElectricEquipment;", "NominalElectricEquipment");
    auto gasEquips = queryResult("SELECT * FROM NominalGasEquipment;", "NominalGasEquipment");
    auto steamEquips = queryResult("SELECT * FROM NominalSteamEquipment;", "NominalSteamEquipment");
    auto hwEquips = queryResult("SELECT * FROM NominalHotWaterEquipment;", "NominalHotWaterEquipment");
    auto otherEquips = queryResult("SELECT * FROM NominalOtherEquipment;", "NominalOtherEquipment");
    auto baseboards = queryResult("SELECT * FROM NominalBaseboardHeaters;", "NominalBaseboardHeaters");
    auto infiltrations = queryResult("SELECT * FROM NominalInfiltration;", "NominalInfiltration");
    auto ventilations = queryResult("SELECT * FROM NominalVentilation;", "NominalVentilation");
    auto roomAirModels = queryResult("SELECT * FROM RoomAirModels;", "RoomAirModels");
    sql->sqliteCommit();

    ASSERT_EQ(2ul, zones.size());
    std::vector<std::string> zone0{"1",   "test zone 1", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", "1",   "1.0", "1.0", "0.0", "0.0",
                                   "0.0", "0.0",         "0.0", "0.0", "1.0", "1.0", "1",   "1",   "0.0", "0.0", "0.0", "0.0", "1"};
    std::vector<std::string> zone1{"2",   "test zone 2", "2.0", "2.0", "2.0", "2.0", "2.0", "2.0", "2.0", "2",   "2.0", "2.0", "2.0", "2.0",
                                   "2.0", "2.0",         "2.0", "2.0", "2.0", "2.0", "2",   "2",   "2.0", "2.0", "2.0", "2.0", "0"};
    EXPECT_EQ(zone0, zones[0]);
    EXPECT_EQ(zone1, zones[1]);

    ASSERT_EQ(2ul, zoneLists.size());
    std::vector<std::string> zoneList0{"1", "test zoneList 1"};
    std::vector<std::string> zoneList1{"2", "test zoneList 2"};
    EXPECT_EQ(zoneList0, zoneLists[0]);
    EXPECT_EQ(zoneList1, zoneLists[1]);

    ASSERT_EQ(2ul, zoneGroups.size());
    std::vector<std::string> zoneGroup0{"1", "test zoneGroup 1", "", "1"};
    std::vector<std::string> zoneGroup1{"2", "test zoneGroup 2", "2", "99"};
    EXPECT_EQ(zoneGroup0, zoneGroups[0]);
    EXPECT_EQ(zoneGroup1, zoneGroups[1]);

    ASSERT_EQ(3ul, zoneInfoZoneLists.size());
    std::vector<std::string> zoneInfoZoneList0{"1", "1"};
    std::vector<std::string> zoneInfoZoneList1{"2", "1"};
    std::vector<std::string> zoneInfoZoneList2{"2", "2"};
    EXPECT_EQ(zoneInfoZoneList0, zoneInfoZoneLists[0]);
    EXPECT_EQ(zoneInfoZoneList1, zoneInfoZoneLists[1]);
    EXPECT_EQ(zoneInfoZoneList2, zoneInfoZoneLists[2]);

    ASSERT_EQ(2ul, schedules.size());
    std::vector<std::string> schedule0{"1", "always on", "ON/OFF", "1.0", "1.0"};
    std::vector<std::string> schedule1{"2", "always off", "ON/OFF", "0.0", "0.0"};
    EXPECT_EQ(schedule0, schedules[0]);
    EXPECT_EQ(schedule1, schedules[1]);

    ASSERT_EQ(2ul, materials.size());
    std::vector<std::string> material0{"1", "test material 1", "1", "-1", "0.0", "0.0", "0.0", "0.0", "0.0", "0", "0.0", "0.0", "0.0", "0.0"};
    std::vector<std::string> material1{"2", "test material 2", "2", "1", "2.0", "2.0", "0.0", "2.0", "2.0", "1", "2.0", "0.0", "2.0", "2.0"};
    EXPECT_EQ(material0, materials[0]);
    EXPECT_EQ(material1, materials[1]);

    ASSERT_EQ(2ul, constructions.size());
    std::vector<std::string> construction0{"1", "test construction 1", "0", "0", "0", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", "-1", "0", "0.0"};
    std::vector<std::string> construction1{"2", "test construction 2", "2", "2", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "2.0", "1", "1", "2.0"};
    EXPECT_EQ(construction0, constructions[0]);
    EXPECT_EQ(construction1, constructions[1]);

    ASSERT_EQ(2ul, constructionLayers.size());
    std::vector<std::string> constructionLayer0{"1", "2", "1", "2"};
    std::vector<std::string> constructionLayer1{"2", "2", "2", "1"};
    EXPECT_EQ(constructionLayer0, constructionLayers[0]);
    EXPECT_EQ(constructionLayer1, constructionLayers[1]);

    ASSERT_EQ(2ul, surfaces.size());
    std::vector<std::string> surface0{
        "1", "test surface 1", "", "Window", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", "0", "0", "0.0", "0.0", "0", "", "", "0", "0", "0"};
    std::vector<std::string> surface1{
        "2", "test surface 2", "2", "Wall", "2.0", "2.0", "2.0", "2.0", "2.0", "2.0", "2", "2", "2.0", "2.0", "1", "1", "1", "2", "1", "1"};
    EXPECT_EQ(surface0, surfaces[0]);
    EXPECT_EQ(surface1, surfaces[1]);

    ASSERT_EQ(2ul, lightings.size());
    std::vector<std::string> lighting0{"1", "test lighting 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> lighting1{"2", "test lighting 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(lighting0, lightings[0]);
    EXPECT_EQ(lighting1, lightings[1]);

    ASSERT_EQ(2ul, peoples.size());
    std::vector<std::string> people0{"1", "test people 1", "", "0", "", "", "0.0", "0.0", "", "", "", "0", "0", "0", "-1", "", "", "-1", "0.0", "0"};
    std::vector<std::string> people1{"2", "test people 2", "1", "2",   "2", "2", "2.0", "2.0", "2", "2", "2", "1", "1", "1", "1",
                                     "1", "test",          "1", "2.0", "1"};
    EXPECT_EQ(people0, peoples[0]);
    EXPECT_EQ(people1, peoples[1]);

    ASSERT_EQ(2ul, elecEquips.size());
    std::vector<std::string> elecEquip0{"1", "test elecEquip 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> elecEquip1{"2", "test elecEquip 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(elecEquip0, elecEquips[0]);
    EXPECT_EQ(elecEquip1, elecEquips[1]);

    ASSERT_EQ(2ul, gasEquips.size());
    std::vector<std::string> gasEquip0{"1", "test gasEquip 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> gasEquip1{"2", "test gasEquip 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(gasEquip0, gasEquips[0]);
    EXPECT_EQ(gasEquip1, gasEquips[1]);

    ASSERT_EQ(2ul, steamEquips.size());
    std::vector<std::string> steamEquip0{"1", "test steamEquip 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> steamEquip1{"2", "test steamEquip 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(steamEquip0, steamEquips[0]);
    EXPECT_EQ(steamEquip1, steamEquips[1]);

    ASSERT_EQ(2ul, hwEquips.size());
    std::vector<std::string> hwEquip0{"1", "test hwEquip 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> hwEquip1{"2", "test hwEquip 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(hwEquip0, hwEquips[0]);
    EXPECT_EQ(hwEquip1, hwEquips[1]);

    ASSERT_EQ(2ul, otherEquips.size());
    std::vector<std::string> otherEquip0{"1", "test otherEquip 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> otherEquip1{"2", "test otherEquip 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(otherEquip0, otherEquips[0]);
    EXPECT_EQ(otherEquip1, otherEquips[1]);

    ASSERT_EQ(2ul, baseboards.size());
    std::vector<std::string> baseboard0{"1", "test baseboard 1", "", "", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0", ""};
    std::vector<std::string> baseboard1{"2", "test baseboard 2", "1", "2", "2.0", "2.0", "2.0", "2.0", "2.0", "2.0", "test"};
    EXPECT_EQ(baseboard0, baseboards[0]);
    EXPECT_EQ(baseboard1, baseboards[1]);

    ASSERT_EQ(2ul, infiltrations.size());
    std::vector<std::string> infiltration0{"1", "test infiltration 1", "", "", "0.0"};
    std::vector<std::string> infiltration1{"2", "test infiltration 2", "1", "2", "2.0"};
    EXPECT_EQ(infiltration0, infiltrations[0]);
    EXPECT_EQ(infiltration1, infiltrations[1]);

    ASSERT_EQ(2ul, ventilations.size());
    std::vector<std::string> ventilation0{"1", "test ventilation 1", "", "", "0.0"};
    std::vector<std::string> ventilation1{"2", "test ventilation 2", "1", "2", "2.0"};
    EXPECT_EQ(ventilation0, ventilations[0]);
    EXPECT_EQ(ventilation1, ventilations[1]);

    ASSERT_EQ(2ul, roomAirModels.size());
    std::vector<std::string> roomAirModel0{"1", "test roomAirModel 1", "1", "0", "0"};
    std::vector<std::string> roomAirModel1{"2", "test roomAirModel 2", "2", "0", "1"}; // RoomAirModel::Mundt and CouplingScheme::Direct
    EXPECT_EQ(roomAirModel0, roomAirModels[0]);
    EXPECT_EQ(roomAirModel1, roomAirModels[1]);
}

TEST_F(SQLiteFixture, SQLiteProcedures_createSQLiteTabularDataRecords)
{
    Array1D_string const rowLabels({"Heating", "Cooling"});
    Array1D_string const columnLabels({"Electricity [GJ]", "Natural Gas [GJ]"});
    Array2D_string const body(2, 2, {"216.38", "869.08", "1822.42", "0.00"});
    Array1D_string const rowLabels2({"Heating [kWh]"});
    Array1D_string const columnLabels2({"Electricity", "Natural Gas"});
    Array2D_string const body2(1, 2, {"815.19", "256.72"});

    auto &sql = state->dataSQLiteProcedures->sqlite;
    sql->sqliteBegin();
    // tabular data references simulation record... always checks for first simulation record only.
    sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");
    sql->createSQLiteTabularDataRecords(body, rowLabels, columnLabels, "AnnualBuildingUtilityPerformanceSummary", "Entire Facility", "End Uses");
    sql->createSQLiteTabularDataRecords(
        body2, rowLabels2, columnLabels2, "AnnualBuildingUtilityPerformanceSummary", "Entire Facility", "End Uses By Subcategory");
    auto tabularData = queryResult("SELECT * FROM TabularData;", "TabularData");
    auto strings = queryResult("SELECT * FROM Strings;", "Strings");
    auto stringTypes = queryResult("SELECT * FROM StringTypes;", "StringTypes");
    sql->sqliteCommit();

    ASSERT_EQ(6ul, tabularData.size());
    // tabularDataIndex, reportNameIndex, reportForStringIndex, tableNameIndex, rowLabelIndex, columnLabelIndex, unitsIndex, simulationIndex, rowId,
    // columnId, value
    std::vector<std::string> tabularData0{"1", "1", "2", "3", "6", "4", "5", "1", "0", "0", "216.38"};
    std::vector<std::string> tabularData1{"2", "1", "2", "3", "7", "4", "5", "1", "1", "0", "869.08"};
    std::vector<std::string> tabularData2{"3", "1", "2", "3", "6", "8", "5", "1", "0", "1", "1822.42"};
    std::vector<std::string> tabularData3{"4", "1", "2", "3", "7", "8", "5", "1", "1", "1", "0.00"};
    std::vector<std::string> tabularData4{"5", "1", "2", "9", "6", "4", "10", "1", "0", "0", "815.19"};
    std::vector<std::string> tabularData5{"6", "1", "2", "9", "6", "8", "10", "1", "0", "1", "256.72"};
    EXPECT_EQ(tabularData0, tabularData[0]);
    EXPECT_EQ(tabularData1, tabularData[1]);
    EXPECT_EQ(tabularData2, tabularData[2]);
    EXPECT_EQ(tabularData3, tabularData[3]);
    EXPECT_EQ(tabularData4, tabularData[4]);
    EXPECT_EQ(tabularData5, tabularData[5]);

    ASSERT_EQ(10ul, strings.size());
    std::vector<std::string> string0{"1", "1", "AnnualBuildingUtilityPerformanceSummary"};
    std::vector<std::string> string1{"2", "2", "Entire Facility"};
    std::vector<std::string> string2{"3", "3", "End Uses"};
    std::vector<std::string> string3{"4", "5", "Electricity"};
    std::vector<std::string> string4{"5", "6", "GJ"};
    std::vector<std::string> string5{"6", "4", "Heating"};
    std::vector<std::string> string6{"7", "4", "Cooling"};
    std::vector<std::string> string7{"8", "5", "Natural Gas"};
    std::vector<std::string> string8{"9", "3", "End Uses By Subcategory"};
    std::vector<std::string> string9{"10", "6", "kWh"};
    EXPECT_EQ(string0, strings[0]);
    EXPECT_EQ(string1, strings[1]);
    EXPECT_EQ(string2, strings[2]);
    EXPECT_EQ(string3, strings[3]);
    EXPECT_EQ(string4, strings[4]);
    EXPECT_EQ(string5, strings[5]);
    EXPECT_EQ(string6, strings[6]);
    EXPECT_EQ(string7, strings[7]);
    EXPECT_EQ(string8, strings[8]);
    EXPECT_EQ(string9, strings[9]);

    ASSERT_EQ(6ul, stringTypes.size());
    std::vector<std::string> stringType0{"1", "ReportName"};
    std::vector<std::string> stringType1{"2", "ReportForString"};
    std::vector<std::string> stringType2{"3", "TableName"};
    std::vector<std::string> stringType3{"4", "RowName"};
    std::vector<std::string> stringType4{"5", "ColumnName"};
    std::vector<std::string> stringType5{"6", "Units"};
    EXPECT_EQ(stringType0, stringTypes[0]);
    EXPECT_EQ(stringType1, stringTypes[1]);
    EXPECT_EQ(stringType2, stringTypes[2]);
    EXPECT_EQ(stringType3, stringTypes[3]);
    EXPECT_EQ(stringType4, stringTypes[4]);
    EXPECT_EQ(stringType5, stringTypes[5]);
}
} // namespace EnergyPlus
