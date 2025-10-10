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

// EnergyPlus::OutputProcessor Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/SQLiteFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SystemReports.hh>
#include <EnergyPlus/WeatherManager.hh>

#include <climits>
#include <map>

using namespace EnergyPlus::PurchasedAirManager;
using namespace EnergyPlus::OutputProcessor;

namespace EnergyPlus {

namespace OutputProcessor {

    TEST_F(SQLiteFixture, OutputProcessor_TestGetMeteredVariables)
    {
        auto &op = state->dataOutputProcessor;
        std::string TypeOfComp = "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW";
        std::string NameOfComp = "FC-5-1B";

        int NumFound = GetNumMeteredVariables(*state, TypeOfComp, NameOfComp);

        EXPECT_EQ(0, NumFound);

        NameOfComp = "OUTSIDELIGHTS";

        OutVarReal *realVar = new OutVarReal();
        realVar->keyUC = NameOfComp;
        op->outVars.push_back(realVar);

        Meter *meter = new Meter("NewMeter");
        meter->resource = Constant::eResource::Electricity;
        op->meters.push_back(meter);

        meter->srcVarNums.push_back(op->outVars.size() - 1);
        realVar->meterNums.push_back(op->meters.size() - 1);

        NumFound = GetNumMeteredVariables(*state, TypeOfComp, NameOfComp);
        EXPECT_EQ(1, NumFound);
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportTSMeters_PrintESOTimeStamp)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1TS = meter1->periods[(int)ReportFreq::TimeStep];
        meter1->CurTSValue = 999.99;
        period1TS.Value = 999.9;
        period1TS.Rpt = true;
        period1TS.accRpt = false;
        period1TS.RptFO = false;
        period1TS.accRptFO = false;
        period1TS.RptNum = 1;
        period1TS.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2TS = meter2->periods[(int)ReportFreq::TimeStep];
        meter2->CurTSValue = 9999.99;
        period2TS.Value = 9999.9;
        period2TS.Rpt = true;
        period2TS.accRpt = false;
        period2TS.RptFO = false;
        period2TS.accRptFO = false;
        period2TS.RptNum = 2;
        period2TS.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;

        op->freqStampReportNums[(int)ReportFreq::TimeStep] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames
        int EndMinute = 10;
        int StartMinute = 0;
        bool PrintESOTimeStamp = true;

        ReportTSMeters(*state, StartMinute, EndMinute, PrintESOTimeStamp, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "0", "12", "21", "0", "10", "0", "10", "-1", "1", "WinterDesignDay", "0", "0"};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,10.00,WinterDesignDay", "1,999.9", "2,9999.9"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,10.00,WinterDesignDay", "1,999.9", "2,9999.9"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData({});

        EXPECT_EQ(reportData, reportDataResults);
        ASSERT_EQ(reportExtendedData.size(), reportExtendedDataResults.size());
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportTSMeters)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1TS = meter1->periods[(int)ReportFreq::TimeStep];
        meter1->CurTSValue = 999.99;
        period1TS.Value = 999.9;
        period1TS.Rpt = true;
        period1TS.accRpt = false;
        period1TS.RptFO = false;
        period1TS.accRptFO = false;
        period1TS.RptNum = 1;
        period1TS.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2TS = meter2->periods[(int)ReportFreq::TimeStep];
        meter2->CurTSValue = 9999.99;
        period2TS.Value = 9999.9;
        period2TS.Rpt = true;
        period2TS.accRpt = false;
        period2TS.RptFO = false;
        period2TS.accRptFO = false;
        period2TS.RptNum = 2;
        period2TS.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;

        op->freqStampReportNums[(int)ReportFreq::TimeStep] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames
        int EndMinute = 10;
        int StartMinute = 0;
        bool PrintESOTimeStamp = false;

        ReportTSMeters(*state, StartMinute, EndMinute, PrintESOTimeStamp, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "0", "12", "21", "0", "10", "0", "10", "-1", "1", "WinterDesignDay", "0", "0"};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,10.00,WinterDesignDay", "1,999.9", "2,9999.9"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9", "2,9999.9"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData({});

        EXPECT_EQ(reportData, reportDataResults);
        ASSERT_EQ(reportExtendedData.size(), reportExtendedDataResults.size());
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportHRMeters)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1HR = meter1->periods[(int)ReportFreq::Hour];
        meter1->CurTSValue = 999.99;
        period1HR.Value = 999.9;
        period1HR.Rpt = true;
        period1HR.accRpt = false;
        period1HR.RptFO = true;
        period1HR.accRptFO = false;
        period1HR.RptNum = 1;
        period1HR.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2HR = meter2->periods[(int)ReportFreq::Hour];
        meter2->CurTSValue = 9999.99;
        period2HR.Value = 9999.9;
        period2HR.Rpt = true;
        period2HR.accRpt = false;
        period2HR.RptFO = true;
        period2HR.accRptFO = false;
        period2HR.RptNum = 2;
        period2HR.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;

        op->freqStampReportNums[(int)ReportFreq::Hour] = op->freqStampReportNums[(int)ReportFreq::TimeStep] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames

        ReportMeters(*state, ReportFreq::Hour, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "0", "12", "21", "1", "0", "0", "60", "1", "1", "WinterDesignDay", "0", ""};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,60.00,WinterDesignDay", "1,999.9", "2,9999.9"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData({});

        EXPECT_EQ(reportData, reportDataResults);
        ASSERT_EQ(reportExtendedData.size(), reportExtendedDataResults.size());
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportDYMeters)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1DY = meter1->periods[(int)ReportFreq::Day];
        meter1->CurTSValue = 999.99;
        period1DY.Value = 999.9;
        period1DY.Rpt = true;
        period1DY.accRpt = false;
        period1DY.RptFO = true;
        period1DY.accRptFO = false;
        period1DY.RptNum = 1;
        period1DY.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;
        period1DY.MaxVal = 4283136.2524843821;
        period1DY.MaxValDate = 12210160;
        period1DY.MinVal = 4283136.2516839253;
        period1DY.MinValDate = 12210110;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2DY = meter2->periods[(int)ReportFreq::Day];
        meter2->CurTSValue = 9999.99;
        period2DY.Value = 9999.9;
        period2DY.Rpt = true;
        period2DY.accRpt = false;
        period2DY.RptFO = true;
        period2DY.accRptFO = false;
        period2DY.RptNum = 2;
        period2DY.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;
        period2DY.MaxVal = 4283136.2524843821;
        period2DY.MaxValDate = 12210160;
        period2DY.MinVal = 4283136.2516839253;
        period2DY.MinValDate = 12210110;

        op->freqStampReportNums[(int)ReportFreq::Day] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames

        ReportMeters(*state, ReportFreq::Day, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "0", "12", "21", "24", "0", "0", "1440", "2", "1", "WinterDesignDay", "0", ""};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0,WinterDesignDay",
                                                         "1,999.9,4283136.251683925, 1,10,4283136.252484382, 1,60",
                                                         "2,9999.9,4283136.251683925, 1,10,4283136.252484382, 1,60"},
                                                        "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "1", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"},
             {"2", "2", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportMNMeters)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1MN = meter1->periods[(int)ReportFreq::Month];
        meter1->CurTSValue = 999.99;
        period1MN.Value = 999.9;
        period1MN.Rpt = true;
        period1MN.accRpt = false;
        period1MN.RptFO = true;
        period1MN.accRptFO = false;
        period1MN.RptNum = 1;
        period1MN.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;
        period1MN.MaxVal = 4283136.2524843821;
        period1MN.MaxValDate = 12210160;
        period1MN.MinVal = 4283136.2516839253;
        period1MN.MinValDate = 12210110;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2MN = meter2->periods[(int)ReportFreq::Month];
        meter2->CurTSValue = 9999.99;
        period2MN.Value = 9999.9;
        period2MN.Rpt = true;
        period2MN.accRpt = false;
        period2MN.RptFO = true;
        period2MN.accRptFO = false;
        period2MN.RptNum = 2;
        period2MN.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;
        period2MN.MaxVal = 4283136.2524843821;
        period2MN.MaxValDate = 12210160;
        period2MN.MinVal = 4283136.2516839253;
        period2MN.MinValDate = 12210110;

        op->freqStampReportNums[(int)ReportFreq::Month] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames

        ReportMeters(*state, ReportFreq::Month, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "0", "12", "31", "24", "0", "", "44640", "3", "1", "", "0", ""};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12",
                                                         "1,999.9,4283136.251683925,21, 1,10,4283136.252484382,21, 1,60",
                                                         "2,9999.9,4283136.251683925,21, 1,10,4283136.252484382,21, 1,60"},
                                                        "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "1", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"},
             {"2", "2", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportSMMeters)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1SM = meter1->periods[(int)ReportFreq::Simulation];
        meter1->CurTSValue = 999.99;
        period1SM.Value = 999.9;
        period1SM.Rpt = true;
        period1SM.accRpt = false;
        period1SM.RptFO = true;
        period1SM.accRptFO = false;
        period1SM.RptNum = 1;
        period1SM.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;
        period1SM.MaxVal = 4283136.2524843821;
        period1SM.MaxValDate = 12210160;
        period1SM.MinVal = 4283136.2516839253;
        period1SM.MinValDate = 12210110;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2SM = meter2->periods[(int)ReportFreq::Simulation];
        meter2->CurTSValue = 9999.99;
        period2SM.Value = 9999.9;
        period2SM.Rpt = true;
        period2SM.accRpt = false;
        period2SM.RptFO = true;
        period2SM.accRptFO = false;
        period2SM.RptNum = 2;
        period2SM.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;
        period2SM.MaxVal = 4283136.2524843821;
        period2SM.MaxValDate = 12210160;
        period2SM.MinVal = 4283136.2516839253;
        period2SM.MinValDate = 12210110;

        op->freqStampReportNums[(int)ReportFreq::Simulation] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames

        ReportMeters(*state, ReportFreq::Simulation, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "", "", "", "", "", "", "1440", "4", "1", "", "0", ""};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1",
                                                         "1,999.9,4283136.251683925,12,21, 1,10,4283136.252484382,12,21, 1,60",
                                                         "2,9999.9,4283136.251683925,12,21, 1,10,4283136.252484382,12,21, 1,60"},
                                                        "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "1", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"},
             {"2", "2", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_reportYRMeters)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);
        sql->createSQLiteReportDictionaryRecord(
            2, StoreType::Sum, "Facility:Electricity", "", "Facility:Electricity", TimeStepType::Zone, "J", ReportFreq::Hour, true);

        Meter *meter1 = new Meter("Meter1");
        op->meters.push_back(meter1);
        auto &period1YR = meter1->periods[(int)ReportFreq::Year];
        meter1->CurTSValue = 999.99;
        period1YR.Value = 999.9;
        period1YR.Rpt = true;
        period1YR.accRpt = false;
        period1YR.RptFO = true;
        period1YR.accRptFO = false;
        period1YR.RptNum = 1;
        period1YR.accRptNum = 1;
        meter1->periods[(int)ReportFreq::Simulation].Value = 999.9;
        period1YR.MaxVal = 4283136.2524843821;
        period1YR.MaxValDate = 12210160;
        period1YR.MinVal = 4283136.2516839253;
        period1YR.MinValDate = 12210110;

        Meter *meter2 = new Meter("Meter2");
        op->meters.push_back(meter2);
        auto &period2YR = meter2->periods[(int)ReportFreq::Year];
        meter2->CurTSValue = 9999.99;
        period2YR.Value = 9999.9;
        period2YR.Rpt = true;
        period2YR.accRpt = false;
        period2YR.RptFO = true;
        period2YR.accRptFO = false;
        period2YR.RptNum = 2;
        period2YR.accRptNum = 2;
        meter2->periods[(int)ReportFreq::Simulation].Value = 9999.9;
        period2YR.MaxVal = 4283136.2524843821;
        period2YR.MaxValDate = 12210160;
        period2YR.MinVal = 4283136.2516839253;
        period2YR.MinValDate = 12210110;

        op->freqStampReportNums[(int)ReportFreq::Year] = 1;
        state->dataGlobal->DayOfSim = 1;
        state->dataGlobal->DayOfSimChr = "1";
        state->dataGlobal->HourOfDay = 1;
        state->dataGlobal->CalendarYear = 2017;
        state->dataGlobal->CalendarYearChr = "2017";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 21;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 2;
        state->dataEnvrn->HolidayIndex = 3 + 7; // Holiday index is now based on the full set of dayTypeNames

        ReportMeters(*state, ReportFreq::Year, true);

        auto result = queryResult("SELECT * FROM Time;", "Time");

        ASSERT_EQ(1ul, result.size());

        std::vector<std::string> testResult0{"1", "2017", "", "", "", "", "", "", "5", "", "", "0", ""};
        EXPECT_EQ(testResult0, result[0]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,2017",
                                                         "1,999.9,4283136.251683925,12,21, 1,10,4283136.252484382,12,21, 1,60",
                                                         "2,9999.9,4283136.251683925,12,21, 1,10,4283136.252484382,12,21, 1,60"},
                                                        "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"}, {"2", "1", "2", "9999.9"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "1", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"},
             {"2", "2", "4283136.25248438", "12", "21", "1", "1", "0", "4283136.25168393", "12", "21", "0", "11", "10"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeTimeStampFormatData)
    {
        auto &op = state->dataOutputProcessor;
        op->freqStampReportNums[(int)ReportFreq::TimeStep] = 1;

        int DailyStampReportNum = 1;
        int MonthlyStampReportNum = 1;
        int RunPeriodStampReportNum = 1;

        state->dataGlobal->DayOfSim = 1;
        std::string DayOfSimChr = "1";
        bool PrintTimeStamp = true;
        int Month = 12;
        int DayOfMonth = 21;
        state->dataGlobal->HourOfDay = 1;
        int EndMinute = 10;
        int StartMinute = 0;
        int DSTIndicator = 0;
        int CurDayType = 10;

        // TSMeter
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 ReportFreq::TimeStep,
                                 op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 Month,
                                 DayOfMonth,
                                 state->dataGlobal->HourOfDay,
                                 EndMinute,
                                 StartMinute,
                                 DSTIndicator,
                                 Sched::dayTypeNames[CurDayType]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,10.00,WinterDesignDay"}, "\n")));

        // TSMeter
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 ReportFreq::EachCall,
                                 op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 Month,
                                 DayOfMonth,
                                 state->dataGlobal->HourOfDay,
                                 EndMinute,
                                 StartMinute,
                                 DSTIndicator,
                                 Sched::dayTypeNames[CurDayType]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,10.00,WinterDesignDay"}, "\n")));

        // HRMeter
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 ReportFreq::Hour,
                                 op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 Month,
                                 DayOfMonth,
                                 state->dataGlobal->HourOfDay,
                                 -1, // EndMinute
                                 -1, // StartMinute
                                 DSTIndicator,
                                 Sched::dayTypeNames[CurDayType]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0, 1, 0.00,60.00,WinterDesignDay"}, "\n")));

        // DYMeter
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 ReportFreq::Day,
                                 DailyStampReportNum,
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 Month,
                                 DayOfMonth,
                                 -1, // Hour
                                 -1, // EndMinute
                                 -1, // StartMinute
                                 DSTIndicator,
                                 Sched::dayTypeNames[CurDayType]);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12,21, 0,WinterDesignDay"}, "\n")));

        // MNMeter
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 ReportFreq::Month,
                                 MonthlyStampReportNum,
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 Month,
                                 -1,  // DayOfMonth
                                 -1,  // Hour
                                 -1,  // EndMinute
                                 -1,  // StartMinute
                                 -1,  // DST
                                 ""); // dayType
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,12"}, "\n")));

        // SMMeter
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 ReportFreq::Simulation,
                                 RunPeriodStampReportNum,
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 -1,  // Month
                                 -1,  // DayOfMonth
                                 -1,  // Hour
                                 -1,  // EndMinute
                                 -1,  // StartMinute
                                 -1,  // DST
                                 ""); // dayType
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1"}, "\n")));

        // Bad input
        WriteTimeStampFormatData(*state,
                                 state->files.mtr,
                                 static_cast<ReportFreq>(999),
                                 RunPeriodStampReportNum,
                                 DayOfSimChr,
                                 PrintTimeStamp,
                                 -1,  // Month
                                 -1,  // DayOfMonth
                                 -1,  // Hour
                                 -1,  // EndMinute
                                 -1,  // StartMinute
                                 -1,  // DST
                                 ""); // dayType

        EXPECT_EQ("SQLite3 message, Illegal reportingInterval passed to WriteTimeStampFormatData: 999\n", ss->str());
        ss->str(std::string());

        auto timeResults = queryResult("SELECT * FROM Time;", "Time");

        std::vector<std::vector<std::string>> timeData({{"1", "0", "12", "21", "0", "10", "0", "10", "0", "1", "WinterDesignDay", "0", "0"},
                                                        {"2", "0", "12", "21", "0", "10", "0", "10", "-1", "1", "WinterDesignDay", "0", "0"},
                                                        {"3", "0", "12", "21", "1", "0", "0", "60", "1", "1", "WinterDesignDay", "0", "0"},
                                                        {"4", "0", "12", "21", "24", "0", "0", "1440", "2", "1", "WinterDesignDay", "0", "0"},
                                                        {"5", "0", "12", "31", "24", "0", "", "44640", "3", "1", "", "0", "0"},
                                                        {"6", "", "", "", "", "", "", "1440", "4", "1", "", "0", "0"}});

        EXPECT_EQ(timeData, timeResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeReportMeterData)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        state->dataGlobal->MinutesInTimeStep = 10;

        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);

        MeterPeriod mp;
        mp.RptNum = 1;
        mp.Value = 999.9;
        mp.MinVal = mp.MaxVal = 0.0;
        mp.MinValDate = mp.MaxValDate = 0;
        mp.RptFO = false;

        mp.WriteReportData(*state, ReportFreq::TimeStep);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,999.9"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::EachCall);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,999.9"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9"}, "\n")));

        mp.Value = 616771620.98702729;
        mp.MinVal = 4283136.2516839253;
        mp.MinValDate = 12210110;
        mp.MaxVal = 4283136.2587211775;
        mp.MaxValDate = 12212460;
        mp.WriteReportData(*state, ReportFreq::Hour);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Day);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273,4283136.251683925, 1,10,4283136.2587211775,24,60"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273,4283136.251683925, 1,10,4283136.2587211775,24,60"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Month);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273,4283136.251683925,21, 1,10,4283136.2587211775,21,24,60"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273,4283136.251683925,21, 1,10,4283136.2587211775,21,24,60"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Simulation);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273,4283136.251683925,12,21, 1,10,4283136.2587211775,12,21,24,60"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273,4283136.251683925,12,21, 1,10,4283136.2587211775,12,21,24,60"}, "\n")));

        mp.RptFO = true;
        mp.WriteReportData(*state, ReportFreq::TimeStep);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::EachCall);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Hour);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Day);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273,4283136.251683925, 1,10,4283136.2587211775,24,60"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Month);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273,4283136.251683925,21, 1,10,4283136.2587211775,21,24,60"}, "\n")));

        mp.WriteReportData(*state, ReportFreq::Simulation);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273,4283136.251683925,12,21, 1,10,4283136.2587211775,12,21,24,60"}, "\n")));

        mp.Value = 0;
        mp.MinVal = mp.MaxVal = 0.0;
        mp.MinValDate = mp.MaxValDate = 0;
        mp.RptFO = false;
        mp.WriteReportData(*state, ReportFreq::TimeStep);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,0.0"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"},
                                                          {"2", "1", "1", "999.9"},
                                                          {"3", "1", "1", "616771620.987027"},
                                                          {"4", "1", "1", "616771620.987027"},
                                                          {"5", "1", "1", "616771620.987027"},
                                                          {"6", "1", "1", "616771620.987027"},
                                                          {"7", "1", "1", "616771620.987027"},
                                                          {"8", "1", "1", "616771620.987027"},
                                                          {"9", "1", "1", "616771620.987027"},
                                                          {"10", "1", "1", "616771620.987027"},
                                                          {"11", "1", "1", "616771620.987027"},
                                                          {"12", "1", "1", "616771620.987027"},
                                                          {"13", "1", "1", "0.0"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "3", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"2", "4", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"3", "5", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"4", "6", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"5", "9", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"6", "10", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"7", "11", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"},
             {"8", "12", "4283136.25872118", "12", "21", "24", "-9", "0", "4283136.25168393", "12", "21", "0", "1", "10"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeReportRealData)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);

        OutVarReal rVar;
        rVar.ReportID = 1;
        rVar.StoreValue = 999.9;
        rVar.storeType = StoreType::Sum;
        rVar.NumStored = 1;
        rVar.MinValue = rVar.MaxValue = 0.0;
        rVar.minValueDate = rVar.maxValueDate = 0;
        rVar.freq = ReportFreq::TimeStep;

        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9"}, "\n")));

        rVar.freq = ReportFreq::EachCall;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9"}, "\n")));

        rVar.freq = ReportFreq::Hour;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9"}, "\n")));

        rVar.StoreValue = 616771620.98702729;
        rVar.MinValue = 4283136.2516839253;
        rVar.minValueDate = 12210110;
        rVar.MaxValue = 4283136.2587211775;
        rVar.maxValueDate = 12212460;
        rVar.freq = ReportFreq::Day;

        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273,4283136.251683925, 1,10,4283136.2587211775,24,60"}, "\n")));

        rVar.freq = ReportFreq::Month;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273,4283136.251683925,21, 1,10,4283136.2587211775,21,24,60"}, "\n")));

        rVar.freq = ReportFreq::Simulation;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273,4283136.251683925,12,21, 1,10,4283136.2587211775,12,21,24,60"}, "\n")));

        rVar.storeType = StoreType::Average;
        rVar.NumStored = 10;
        rVar.freq = ReportFreq::TimeStep;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.09870273"}, "\n")));

        rVar.freq = ReportFreq::EachCall;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.09870273"}, "\n")));

        rVar.freq = ReportFreq::Hour;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.09870273"}, "\n")));

        rVar.freq = ReportFreq::Day;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.09870273,4283136.251683925, 1,10,4283136.2587211775,24,60"}, "\n")));

        rVar.freq = ReportFreq::Month;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.09870273,4283136.251683925,21, 1,10,4283136.2587211775,21,24,60"}, "\n")));

        rVar.freq = ReportFreq::Simulation;
        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.09870273,4283136.251683925,12,21, 1,10,4283136.2587211775,12,21,24,60"}, "\n")));

        rVar.StoreValue = 0;
        rVar.NumStored = 1;
        rVar.MinValue = rVar.MaxValue = 0.0;
        rVar.minValueDate = rVar.maxValueDate = 0;
        rVar.freq = ReportFreq::TimeStep;

        rVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"},
                                                          {"2", "1", "1", "999.9"},
                                                          {"3", "1", "1", "999.9"},
                                                          {"4", "1", "1", "616771620.987027"},
                                                          {"5", "1", "1", "616771620.987027"},
                                                          {"6", "1", "1", "616771620.987027"},
                                                          {"7", "1", "1", "61677162.0987027"},
                                                          {"8", "1", "1", "61677162.0987027"},
                                                          {"9", "1", "1", "61677162.0987027"},
                                                          {"10", "1", "1", "61677162.0987027"},
                                                          {"11", "1", "1", "61677162.0987027"},
                                                          {"12", "1", "1", "61677162.0987027"},
                                                          {"13", "1", "1", "0.0"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "4", "4283136.25872118", "12", "21", "24", "", "0", "4283136.25168393", "12", "21", "0", "", "10"},
             {"2", "5", "4283136.25872118", "12", "21", "24", "", "0", "4283136.25168393", "12", "21", "0", "", "10"},
             {"3", "6", "4283136.25872118", "12", "21", "24", "", "0", "4283136.25168393", "12", "21", "0", "", "10"},
             {"4", "10", "4283136.25872118", "12", "21", "24", "", "0", "4283136.25168393", "12", "21", "0", "", "10"},
             {"5", "11", "4283136.25872118", "12", "21", "24", "", "0", "4283136.25168393", "12", "21", "0", "", "10"},
             {"6", "12", "4283136.25872118", "12", "21", "24", "", "0", "4283136.25168393", "12", "21", "0", "", "10"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeReportIntegerData)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);

        OutVarInt iVar;
        iVar.ReportID = 1;
        iVar.StoreValue = 999.9;
        iVar.storeType = StoreType::Sum;
        iVar.NumStored = 1;
        iVar.MinValue = iVar.MaxValue = 0;
        iVar.minValueDate = iVar.maxValueDate = 0;
        iVar.freq = ReportFreq::TimeStep;

        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.900000"}, "\n")));

        iVar.freq = ReportFreq::EachCall;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.900000"}, "\n")));

        iVar.freq = ReportFreq::Hour;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.900000"}, "\n")));

        iVar.StoreValue = 616771620.98702729;
        iVar.MinValue = 4283136;
        iVar.minValueDate = 12210110;
        iVar.MaxValue = 4283196;
        iVar.maxValueDate = 12212460;
        iVar.freq = ReportFreq::Day;

        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.987027,4283136.0, 1,10,4283196.0,24,60"}, "\n")));

        iVar.freq = ReportFreq::Month;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.987027,4283136.0,21, 1,10,4283196.0,21,24,60"}, "\n")));

        iVar.freq = ReportFreq::Simulation;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.987027,4283136.0,12,21, 1,10,4283196.0,12,21,24,60"}, "\n")));

        iVar.storeType = StoreType::Average;
        iVar.NumStored = 10;
        iVar.MinValue = iVar.MaxValue = 0;
        iVar.minValueDate = iVar.maxValueDate = 0;
        iVar.freq = ReportFreq::TimeStep;

        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.098703"}, "\n")));

        iVar.freq = ReportFreq::EachCall;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.098703"}, "\n")));

        iVar.freq = ReportFreq::Hour;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.098703"}, "\n")));

        iVar.MinValue = 4283136;
        iVar.minValueDate = 12210110;
        iVar.MaxValue = 4283196;
        iVar.maxValueDate = 12212460;
        iVar.freq = ReportFreq::Day;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.098703,4283136.0, 1,10,4283196.0,24,60"}, "\n")));

        iVar.freq = ReportFreq::Month;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.098703,4283136.0,21, 1,10,4283196.0,21,24,60"}, "\n")));

        iVar.freq = ReportFreq::Simulation;
        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,61677162.098703,4283136.0,12,21, 1,10,4283196.0,12,21,24,60"}, "\n")));

        iVar.storeType = StoreType::Sum;
        iVar.StoreValue = 0;
        iVar.NumStored = 1;
        iVar.MinValue = iVar.MaxValue = 0;
        iVar.minValueDate = iVar.minValueDate = 0;
        iVar.freq = ReportFreq::TimeStep;

        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0"}, "\n")));

        iVar.StoreValue = 25.75;
        iVar.storeType = StoreType::Average;
        iVar.NumStored = 720;
        iVar.MinValue = 0;
        iVar.MaxValue = 1;
        iVar.minValueDate = 4010115;
        iVar.maxValueDate = 4011560;
        iVar.freq = ReportFreq::Month;

        iVar.writeReportData(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.035764,0.0, 1, 1,15,1.0, 1,15,60"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "999.9"},
                                                          {"2", "1", "1", "999.9"},
                                                          {"3", "1", "1", "999.9"},
                                                          {"4", "1", "1", "616771620.987027"},
                                                          {"5", "1", "1", "616771620.987027"},
                                                          {"6", "1", "1", "616771620.987027"},
                                                          {"7", "1", "1", "61677162.0987027"},
                                                          {"8", "1", "1", "61677162.0987027"},
                                                          {"9", "1", "1", "61677162.0987027"},
                                                          {"10", "1", "1", "61677162.0987027"},
                                                          {"11", "1", "1", "61677162.0987027"},
                                                          {"12", "1", "1", "61677162.0987027"},
                                                          {"13", "1", "1", "0.0"},
                                                          {"14", "1", "1", "0.0357638888888889"}});

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "4", "4283196.0", "12", "21", "24", "", "0", "4283136.0", "12", "21", "0", "", "10"},
             {"2", "5", "4283196.0", "12", "21", "24", "", "0", "4283136.0", "12", "21", "0", "", "10"},
             {"3", "6", "4283196.0", "12", "21", "24", "", "0", "4283136.0", "12", "21", "0", "", "10"},
             {"4", "10", "4283196.0", "12", "21", "24", "", "0", "4283136.0", "12", "21", "0", "", "10"},
             {"5", "11", "4283196.0", "12", "21", "24", "", "0", "4283136.0", "12", "21", "0", "", "10"},
             {"6", "12", "4283196.0", "12", "21", "24", "", "0", "4283136.0", "12", "21", "0", "", "10"},
             {"7", "14", "1.0", "4", "1", "15", "", "0", "0.0", "4", "1", "0", "", "15"}});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeNumericData_1)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);

        WriteNumericData(*state, 1, 999);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999"}, "\n")));

        WriteNumericData(*state, 1, 0);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0"}, "\n")));

        WriteNumericData(*state, 1, -999);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-999"}, "\n")));

        WriteNumericData(*state, 1, 999.9);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,999.9"}, "\n")));

        WriteNumericData(*state, 1, 0.0);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0"}, "\n")));

        WriteNumericData(*state, 1, -999.9);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-999.9"}, "\n")));

        WriteNumericData(*state, 1, 0);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0"}, "\n")));

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({
            {"1", "1", "1", "999.0"},
            {"2", "1", "1", "0.0"},
            {"3", "1", "1", "-999.0"},
            {"4", "1", "1", "999.9"},
            {"5", "1", "1", "0.0"},
            {"6", "1", "1", "-999.9"},
            {"7", "1", "1", "0.0"},
        });

        std::vector<std::vector<std::string>> reportExtendedData({});

        EXPECT_EQ(reportData, reportDataResults);
        ASSERT_EQ(reportExtendedData.size(), reportExtendedDataResults.size());
    }

    TEST_F(SQLiteFixture, OutputProcessor_getStandardMeterResourceType)
    {
        std::map<std::string, std::string> const resource_map = {{"ELECTRICITY", "Electricity"},
                                                                 {"NATURALGAS", "NaturalGas"},
                                                                 {"GASOLINE", "Gasoline"},
                                                                 {"DIESEL", "Diesel"},
                                                                 {"COAL", "Coal"},
                                                                 {"FUELOILNO1", "FuelOilNo1"},
                                                                 {"FUELOILNO2", "FuelOilNo2"},
                                                                 {"PROPANE", "Propane"},
                                                                 {"WATER", "Water"},
                                                                 {"ONSITEWATER", "OnSiteWater"},
                                                                 {"MAINSWATER", "MainsWater"},
                                                                 {"RAINWATER", "RainWater"},
                                                                 {"WELLWATER", "WellWater"},
                                                                 {"CONDENSATE", "Condensate"},
                                                                 {"ENERGYTRANSFER", "EnergyTransfer"},
                                                                 {"DISTRICTHEATINGSTEAM", "DistrictHeatingSteam"},
                                                                 {"DISTRICTCOOLING", "DistrictCooling"},
                                                                 {"DISTRICTHEATINGWATER", "DistrictHeatingWater"},
                                                                 {"ELECTRICITYPRODUCED", "ElectricityProduced"},
                                                                 {"ELECTRICITYPURCHASED", "ElectricityPurchased"},
                                                                 {"ELECTRICITYSURPLUSSOLD", "ElectricitySurplusSold"},
                                                                 {"ELECTRICITYNET", "ElectricityNet"},
                                                                 {"SOLARWATER", "SolarWater"},
                                                                 {"SOLARAIR", "SolarAir"},
                                                                 {"SO2", "SO2"},
                                                                 {"NOX", "NOx"},
                                                                 {"N2O", "N2O"},
                                                                 {"PM", "PM"},
                                                                 {"PM2.5", "PM2.5"},
                                                                 {"PM10", "PM10"},
                                                                 {"CO", "CO"},
                                                                 {"CO2", "CO2"},
                                                                 {"CH4", "CH4"},
                                                                 {"NH3", "NH3"},
                                                                 {"NMVOC", "NMVOC"},
                                                                 {"HG", "Hg"},
                                                                 {"PB", "Pb"},
                                                                 {"NUCLEAR HIGH", "Nuclear High"},
                                                                 {"NUCLEAR LOW", "Nuclear Low"},
                                                                 {"WATERENVIRONMENTALFACTORS", "WaterEnvironmentalFactors"},
                                                                 {"CARBON EQUIVALENT", "Carbon Equivalent"},
                                                                 {"SOURCE", "Source"},
                                                                 {"PLANTLOOPHEATINGDEMAND", "PlantLoopHeatingDemand"},
                                                                 {"PLANTLOOPCOOLINGDEMAND", "PlantLoopCoolingDemand"},
                                                                 {"GENERIC", "Generic"},
                                                                 {"OTHERFUEL1", "OtherFuel1"},
                                                                 {"OTHERFUEL2", "OtherFuel2"}};

        std::string out_resource_type;
        bool error_found = false;

        for (auto const &meterType : resource_map) {
            Constant::eResource resource = static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, meterType.first));
            EXPECT_EQ(meterType.second, Constant::eResourceNames[(int)resource]);
            EXPECT_FALSE(error_found);
        }

        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

        Constant::eResource resource = static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, "BAD INPUT"));
        EXPECT_EQ((int)resource, (int)Constant::eResource::Invalid);
    }

    TEST_F(SQLiteFixture, OutputProcessor_determineIndexGroupKeyFromMeterName)
    {
        std::map<std::string, int> const resource_map = {{"Electricity:Facility", 100},
                                                         {"NaturalGas:Facility", 101},
                                                         {"DistricHeatingWater:Facility", 102},
                                                         {"DistricCooling:Facility", 103},
                                                         {"ElectricityNet:Facility", 104},
                                                         {"Electricity:Building", 201},
                                                         {"NaturalGas:Building", 202},
                                                         {"Electricity:HVAC", 301},
                                                         {"InteriorLights:Electricity", 401},
                                                         {"InteriorLights:Electricity:Zone", 501},
                                                         {"BAD INPUT", -11}};

        for (auto const &indexGroup : resource_map) {
            EXPECT_EQ(indexGroup.second, DetermineIndexGroupKeyFromMeterName(*state, indexGroup.first)) << "where meterName is " << indexGroup.first;
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_standardIndexTypeKey)
    {
        EXPECT_EQ("Zone", timeStepTypeNames[(int)TimeStepType::Zone]);
        EXPECT_EQ("System", timeStepTypeNames[(int)TimeStepType::System]);

        // It's no longer possible to pass something that isn't part of the enum, that's kind of the point of using an enum!
        // EXPECT_EQ("UNKW", StandardTimeStepTypeKey(0));
        // EXPECT_EQ("UNKW", StandardTimeStepTypeKey(-1));
        // EXPECT_EQ("UNKW", StandardTimeStepTypeKey(3));
    }

    TEST_F(SQLiteFixture, OutputProcessor_standardVariableTypeKey)
    {
        EXPECT_EQ("Average", storeTypeNames[(int)StoreType::Average]);
        EXPECT_EQ("Sum", storeTypeNames[(int)StoreType::Sum]);
    }

    TEST_F(SQLiteFixture, OutputProcessor_determineMeterIPUnits)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        RT_IPUnits ipUnits = RT_IPUnits::Invalid;
        bool errorFound = false;

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::ElectricityProduced, Constant::Units::J, errorFound);
        EXPECT_EQ((int)RT_IPUnits::Electricity, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::NaturalGas, Constant::Units::J, errorFound);
        EXPECT_EQ((int)RT_IPUnits::Gas, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::DistrictCooling, Constant::Units::J, errorFound);
        EXPECT_EQ((int)RT_IPUnits::Cooling, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::MainsWater, Constant::Units::m3, errorFound);
        EXPECT_EQ((int)RT_IPUnits::Water, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::Source, Constant::Units::m3, errorFound);
        EXPECT_EQ((int)RT_IPUnits::OtherM3, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::Source, Constant::Units::kg, errorFound);
        EXPECT_EQ((int)RT_IPUnits::OtherKG, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::Source, Constant::Units::L, errorFound);
        EXPECT_EQ((int)RT_IPUnits::OtherL, (int)ipUnits);
        EXPECT_FALSE(errorFound);

        sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::Source, Constant::Units::unknown, errorFound); // was "badunits"
        EXPECT_EQ((int)RT_IPUnits::OtherJ, (int)ipUnits);
        EXPECT_TRUE(errorFound);

        ipUnits = GetResourceIPUnits(*state, Constant::eResource::Electricity, Constant::Units::unknown, errorFound); // was "kWh"
        EXPECT_EQ((int)RT_IPUnits::Electricity, (int)ipUnits);
        EXPECT_TRUE(errorFound);

        auto errorData = queryResult("SELECT * FROM Errors;", "Errors");

        ASSERT_EQ(2ul, errorData.size());
        std::vector<std::string> errorData0{
            "1", "1", "0", "DetermineMeterIPUnits: Meter units not recognized for IP Units conversion=[unknown].", "0"};
        std::vector<std::string> errorData1{
            "2", "1", "0", "DetermineMeterIPUnits: Meter units not recognized for IP Units conversion=[unknown].", "0"};
        EXPECT_EQ(errorData0, errorData[0]);
        EXPECT_EQ(errorData1, errorData[1]);
    }

    TEST_F(SQLiteFixture, OutputProcessor_dateToStringWithMonth)
    {
        EXPECT_EQ("01-JAN-00:01", DateToStringWithMonth(1010101));
        EXPECT_EQ("01-JAN-00:00", DateToStringWithMonth(1010100));
        EXPECT_EQ("01-FEB-01:00", DateToStringWithMonth(2010160));
        EXPECT_EQ("20-MAR-01:59", DateToStringWithMonth(3200259));
        EXPECT_EQ("13-APR-23:59", DateToStringWithMonth(4132459));
        EXPECT_EQ("15-MAY-20:30", DateToStringWithMonth(5152130));
        EXPECT_EQ("19-JUN-12:10", DateToStringWithMonth(6191310));
        EXPECT_EQ("25-JUL-19:40", DateToStringWithMonth(7252040));
        EXPECT_EQ("05-AUG-06:22", DateToStringWithMonth(8050722));
        EXPECT_EQ("03-SEP-09:50", DateToStringWithMonth(9031050));
        EXPECT_EQ("27-OCT-03:31", DateToStringWithMonth(10270431));
        EXPECT_EQ("08-NOV-22:28", DateToStringWithMonth(11082328));
        EXPECT_EQ("21-DEC-00:10", DateToStringWithMonth(12210110));

        EXPECT_EQ("-", DateToStringWithMonth(0));
        EXPECT_EQ("-", DateToStringWithMonth(-9999));
        EXPECT_EQ("-", DateToStringWithMonth(-12210110));
        EXPECT_EQ("-", DateToStringWithMonth(13082328));
        EXPECT_EQ("-", DateToStringWithMonth(10100));
        EXPECT_EQ("-", DateToStringWithMonth(1000101));
        EXPECT_EQ("-", DateToStringWithMonth(990101));
        EXPECT_EQ("-", DateToStringWithMonth(1010001));
        EXPECT_EQ("-", DateToStringWithMonth(1009901));
        EXPECT_EQ("-", DateToStringWithMonth(1010099));
        EXPECT_EQ("-", DateToStringWithMonth(1320100));
        EXPECT_EQ("-", DateToStringWithMonth(1012500));
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeMeterDictionaryItem)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        InitializeOutput(*state);

        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);

        WriteMeterDictionaryItem(*state, ReportFreq::TimeStep, StoreType::Average, 1, "indexGroup", "meterName", Constant::Units::J, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,1,meterName [J] !TimeStep"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1,meterName [J] !TimeStep"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::TimeStep, StoreType::Sum, 2, "indexGroup", "meterName", Constant::Units::W, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"2,1,meterName [W] !TimeStep"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"2,1,meterName [W] !TimeStep"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::TimeStep, StoreType::Average, 3, "indexGroup", "meterName", Constant::Units::J, true, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"3,1,Cumulative meterName [J] !TimeStep"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"3,1,Cumulative meterName [J] !TimeStep"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::TimeStep, StoreType::Average, 4, "indexGroup", "meterName", Constant::Units::W, false, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"4,1,meterName [W] !TimeStep"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::TimeStep, StoreType::Average, 5, "indexGroup", "meterName", Constant::Units::W, true, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"5,1,Cumulative meterName [W] !TimeStep"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::EachCall, StoreType::Average, 6, "indexGroup", "meterName", Constant::Units::J, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"6,1,meterName [J] !Each Call"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"6,1,meterName [J] !Each Call"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::EachCall, StoreType::Sum, 7, "indexGroup", "meterName", Constant::Units::J, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"7,1,meterName [J] !Each Call"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"7,1,meterName [J] !Each Call"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::EachCall, StoreType::Average, 8, "indexGroup", "meterName", Constant::Units::J, true, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"8,1,Cumulative meterName [J] !Each Call"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"8,1,Cumulative meterName [J] !Each Call"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::EachCall, StoreType::Average, 9, "indexGroup", "meterName", Constant::Units::J, false, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"9,1,meterName [J] !Each Call"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::EachCall, StoreType::Average, 10, "indexGroup", "meterName", Constant::Units::J, true, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"10,1,Cumulative meterName [J] !Each Call"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Hour, StoreType::Average, 11, "indexGroup", "meterName", Constant::Units::J, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"11,1,meterName [J] !Hourly"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"11,1,meterName [J] !Hourly"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Hour, StoreType::Sum, 12, "indexGroup", "meterName", Constant::Units::None, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"12,1,meterName [] !Hourly"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"12,1,meterName [] !Hourly"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Hour, StoreType::Average, 13, "indexGroup", "meterName", Constant::Units::None, true, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"13,1,Cumulative meterName [] !Hourly"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"13,1,Cumulative meterName [] !Hourly"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Hour, StoreType::Average, 14, "indexGroup", "meterName", Constant::Units::None, false, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"14,1,meterName [] !Hourly"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Hour, StoreType::Average, 15, "indexGroup", "meterName", Constant::Units::None, true, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"15,1,Cumulative meterName [] !Hourly"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Day, StoreType::Average, 16, "indexGroup", "meterName", Constant::Units::None, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"16,7,meterName [] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"16,7,meterName [] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Day, StoreType::Sum, 17, "indexGroup", "meterName", Constant::Units::None, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"17,7,meterName [] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"17,7,meterName [] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Day, StoreType::Average, 18, "indexGroup", "meterName", Constant::Units::deltaC, true, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"18,1,Cumulative meterName [deltaC] !Daily "}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"18,1,Cumulative meterName [deltaC] !Daily "}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Day, StoreType::Average, 19, "indexGroup", "meterName", Constant::Units::deltaC, false, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"19,7,meterName [deltaC] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Day, StoreType::Average, 20, "indexGroup", "meterName", Constant::Units::deltaC, true, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"20,1,Cumulative meterName [deltaC] !Daily "}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Month, StoreType::Average, 21, "indexGroup", "meterName", Constant::Units::deltaC, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"21,9,meterName [deltaC] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"21,9,meterName [deltaC] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Month, StoreType::Sum, 22, "indexGroup", "meterName", Constant::Units::deltaC, false, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"22,9,meterName [deltaC] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"22,9,meterName [deltaC] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Month, StoreType::Average, 23, "indexGroup", "meterName", Constant::Units::deltaC, true, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"23,1,Cumulative meterName [deltaC] !Monthly "}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"23,1,Cumulative meterName [deltaC] !Monthly "}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Month, StoreType::Average, 24, "indexGroup", "meterName", Constant::Units::deltaC, false, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"24,9,meterName [deltaC] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(*state, ReportFreq::Month, StoreType::Average, 25, "indexGroup", "meterName", Constant::Units::deltaC, true, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"25,1,Cumulative meterName [deltaC] !Monthly "}, "\n")));

        WriteMeterDictionaryItem(
            *state, ReportFreq::Simulation, StoreType::Average, 26, "indexGroup", "meterName", Constant::Units::deltaC, false, false);
        EXPECT_TRUE(compare_mtr_stream(
            delimited_string({"26,11,meterName [deltaC] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"26,11,meterName [deltaC] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(
            *state, ReportFreq::Simulation, StoreType::Sum, 27, "indexGroup", "meterName", Constant::Units::deltaC, false, false);
        EXPECT_TRUE(compare_mtr_stream(
            delimited_string({"27,11,meterName [deltaC] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"27,11,meterName [deltaC] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(
            *state, ReportFreq::Simulation, StoreType::Average, 28, "indexGroup", "meterName", Constant::Units::deltaC, true, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"28,1,Cumulative meterName [deltaC] !RunPeriod "}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"28,1,Cumulative meterName [deltaC] !RunPeriod "}, "\n")));

        WriteMeterDictionaryItem(
            *state, ReportFreq::Simulation, StoreType::Average, 29, "indexGroup", "meterName", Constant::Units::deltaC, false, true);
        EXPECT_TRUE(compare_mtr_stream(
            delimited_string({"29,11,meterName [deltaC] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));

        WriteMeterDictionaryItem(
            *state, ReportFreq::Simulation, StoreType::Average, 30, "indexGroup", "meterName", Constant::Units::deltaC, true, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"30,1,Cumulative meterName [deltaC] !RunPeriod "}, "\n")));

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        // TODO: Double check my determination of correct behavior
        // Before it was internally calling createSQLiteReportDictionaryRecord with indexType = 1 which in SQLiteProcedures means "HVAC System"
        // but in OutputProcessor domain it means Zone...
        // According to the I/O ref guide: Output:Meter is supposed to have the "Zone Timestep interval" for its resolution
        std::string timeStepTypeString = "Zone";

        std::vector<std::vector<std::string>> reportDataDictionary(
            {{"1", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Zone Timestep", "", "J"},
             {"2", "1", "Sum", "indexGroup", timeStepTypeString, "", "meterName", "Zone Timestep", "", "W"},
             {"3", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Zone Timestep", "", "J"},
             {"4", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Zone Timestep", "", "W"},
             {"5", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Zone Timestep", "", "W"},
             {"6", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "HVAC System Timestep", "", "J"},
             {"7", "1", "Sum", "indexGroup", timeStepTypeString, "", "meterName", "HVAC System Timestep", "", "J"},
             {"8", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "HVAC System Timestep", "", "J"},
             {"9", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "HVAC System Timestep", "", "J"},
             {"10", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "HVAC System Timestep", "", "J"},
             {"11", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Hourly", "", "J"},
             {"12", "1", "Sum", "indexGroup", timeStepTypeString, "", "meterName", "Hourly", "", ""},
             {"13", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Hourly", "", ""},
             {"14", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Hourly", "", ""},
             {"15", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Hourly", "", ""},
             {"16", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Daily", "", ""},
             {"17", "1", "Sum", "indexGroup", timeStepTypeString, "", "meterName", "Daily", "", ""},
             {"18", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Daily", "", "deltaC"},
             {"19", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Daily", "", "deltaC"},
             {"20", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Daily", "", "deltaC"},
             {"21", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Monthly", "", "deltaC"},
             {"22", "1", "Sum", "indexGroup", timeStepTypeString, "", "meterName", "Monthly", "", "deltaC"},
             {"23", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Monthly", "", "deltaC"},
             {"24", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Monthly", "", "deltaC"},
             {"25", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Monthly", "", "deltaC"},
             {"26", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Run Period", "", "deltaC"},
             {"27", "1", "Sum", "indexGroup", timeStepTypeString, "", "meterName", "Run Period", "", "deltaC"},
             {"28", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Run Period", "", "deltaC"},
             {"29", "1", "Avg", "indexGroup", timeStepTypeString, "", "meterName", "Run Period", "", "deltaC"},
             {"30", "1", "Avg", "indexGroup", timeStepTypeString, "Cumulative ", "meterName", "Run Period", "", "deltaC"}});

        EXPECT_EQ(reportDataDictionary.size(), reportDataDictionaryResults.size());
        for (int i = 0; i < (int)reportDataDictionary.size(); ++i) {
            EXPECT_EQ(reportDataDictionary[i], reportDataDictionaryResults[i]);
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeReportVariableDictionaryItem)
    {
        auto &op = state->dataOutputProcessor;
        auto &sql = state->dataSQLiteProcedures->sqlite;
        InitializeOutput(*state);

        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);

        // Store expected results
        std::vector<std::vector<std::string>> expectedReportDataDictionary;

        std::string timeStepZoneString = "Zone";
        std::string timeStepSystemString = "HVAC System";

        // For now I don't accept anything else than TimeStepZone or TimeStepSystem, but to make it easier if we need to change that later
        // and to preserve the original test (passing int=3 before should have defaulted to Zone...)
        TimeStepType aThirdTimeStepType = TimeStepType::Zone;
        std::string aThirdTimeStepString = timeStepZoneString;

        OutVarReal rVar;
        rVar.freq = ReportFreq::TimeStep;
        rVar.storeType = StoreType::Average;
        rVar.timeStepType = TimeStepType::Zone;
        rVar.units = Constant::Units::m3_s;
        rVar.ReportID = 1;
        rVar.indexGroup = "indexGroup";
        rVar.key = "keyedValue";
        rVar.name = "variableName";
        rVar.unitNameCustomEMS = "";
        rVar.sched = nullptr;

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1,keyedValue,variableName [m3/s] !TimeStep"}, "\n")));

        rVar.freq = ReportFreq::TimeStep;
        rVar.storeType = StoreType::Sum;
        rVar.ReportID = 2;

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"2,1,keyedValue,variableName [m3/s] !TimeStep"}, "\n")));

        rVar.ReportID = 3;
        rVar.storeType = StoreType::Average;
        rVar.sched = Sched::AddScheduleConstant(*state, "scheduleName");

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"3,1,keyedValue,variableName [m3/s] !TimeStep,scheduleName"}, "\n")));

        rVar.ReportID = 4;
        rVar.timeStepType = TimeStepType::System;
        rVar.sched = nullptr;

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"4,1,keyedValue,variableName [m3/s] !TimeStep"}, "\n")));

        rVar.ReportID = 6;
        rVar.freq = ReportFreq::EachCall;
        rVar.timeStepType = TimeStepType::Zone;

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"6,1,keyedValue,variableName [m3/s] !Each Call"}, "\n")));

        rVar.ReportID = 7;
        rVar.storeType = StoreType::Sum;
        rVar.indexGroup = "indexGroup";

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"7,1,keyedValue,variableName [m3/s] !Each Call"}, "\n")));

        rVar.ReportID = 8;
        rVar.storeType = StoreType::Average;
        rVar.sched = Sched::GetSchedule(*state, "SCHEDULENAME");

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"8,1,keyedValue,variableName [m3/s] !Each Call,scheduleName"}, "\n")));

        rVar.ReportID = 9;
        rVar.sched = nullptr;
        rVar.timeStepType = TimeStepType::System;

        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"9,1,keyedValue,variableName [m3/s] !Each Call"}, "\n")));

        rVar.ReportID = 11;
        rVar.freq = ReportFreq::Hour;
        rVar.timeStepType = TimeStepType::Zone;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Hour]);
        op->freqTrackingVariables[(int)ReportFreq::Hour] = false;
        EXPECT_TRUE(compare_eso_stream(delimited_string({"11,1,keyedValue,variableName [m3/s] !Hourly"}, "\n")));

        rVar.ReportID = 12;
        rVar.storeType = StoreType::Sum;
        rVar.freq = ReportFreq::Hour;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Hour]);
        op->freqTrackingVariables[(int)ReportFreq::Hour] = false;
        EXPECT_TRUE(compare_eso_stream(delimited_string({"12,1,keyedValue,variableName [m3/s] !Hourly"}, "\n")));

        rVar.ReportID = 13;
        rVar.storeType = StoreType::Average;
        rVar.sched = Sched::GetSchedule(*state, "SCHEDULENAME");
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Hour]);
        op->freqTrackingVariables[(int)ReportFreq::Hour] = false;
        EXPECT_TRUE(compare_eso_stream(delimited_string({"13,1,keyedValue,variableName [m3/s] !Hourly,scheduleName"}, "\n")));

        rVar.ReportID = 14;
        rVar.sched = nullptr;
        rVar.timeStepType = TimeStepType::System;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Hour]);
        op->freqTrackingVariables[(int)ReportFreq::Hour] = false;
        EXPECT_TRUE(compare_eso_stream(delimited_string({"14,1,keyedValue,variableName [m3/s] !Hourly"}, "\n")));

        rVar.ReportID = 16;
        rVar.freq = ReportFreq::Day;
        rVar.timeStepType = TimeStepType::Zone;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Day]);
        op->freqTrackingVariables[(int)ReportFreq::Day] = false;
        EXPECT_TRUE(
            compare_eso_stream(delimited_string({"16,7,keyedValue,variableName [m3/s] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));

        rVar.ReportID = 17;
        rVar.storeType = StoreType::Sum;
        rVar.freq = ReportFreq::Day;
        rVar.timeStepType = TimeStepType::Zone;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Day]);
        op->freqTrackingVariables[(int)ReportFreq::Day] = false;
        EXPECT_TRUE(
            compare_eso_stream(delimited_string({"17,7,keyedValue,variableName [m3/s] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));

        rVar.ReportID = 18;
        rVar.storeType = StoreType::Average;
        rVar.sched = Sched::GetSchedule(*state, "SCHEDULENAME");
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Day]);
        op->freqTrackingVariables[(int)ReportFreq::Day] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"18,7,keyedValue,variableName [m3/s] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute],scheduleName"}, "\n")));

        rVar.ReportID = 19;
        rVar.timeStepType = TimeStepType::System;
        rVar.sched = nullptr;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Day]);
        op->freqTrackingVariables[(int)ReportFreq::Day] = false;
        EXPECT_TRUE(
            compare_eso_stream(delimited_string({"19,7,keyedValue,variableName [m3/s] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]"}, "\n")));

        rVar.ReportID = 21;
        rVar.freq = ReportFreq::Month;
        rVar.timeStepType = TimeStepType::Zone;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Month]);
        op->freqTrackingVariables[(int)ReportFreq::Month] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"21,9,keyedValue,variableName [m3/s] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));

        rVar.ReportID = 22;
        rVar.storeType = StoreType::Sum;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Month]);
        op->freqTrackingVariables[(int)ReportFreq::Month] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"22,9,keyedValue,variableName [m3/s] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));

        rVar.ReportID = 23;
        rVar.storeType = StoreType::Average;
        rVar.sched = Sched::GetSchedule(*state, "SCHEDULENAME");
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Month]);
        op->freqTrackingVariables[(int)ReportFreq::Month] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"23,9,keyedValue,variableName [m3/s] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute],scheduleName"}, "\n")));

        rVar.ReportID = 24;
        rVar.timeStepType = TimeStepType::System;
        rVar.sched = nullptr;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Month]);
        op->freqTrackingVariables[(int)ReportFreq::Month] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"24,9,keyedValue,variableName [m3/s] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"}, "\n")));

        rVar.ReportID = 26;
        rVar.freq = ReportFreq::Simulation;
        rVar.timeStepType = TimeStepType::Zone;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Simulation]);
        op->freqTrackingVariables[(int)ReportFreq::Simulation] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"26,11,keyedValue,variableName [m3/s] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));

        rVar.ReportID = 27;
        rVar.storeType = StoreType::Sum;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Simulation]);
        op->freqTrackingVariables[(int)ReportFreq::Simulation] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"27,11,keyedValue,variableName [m3/s] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));

        rVar.ReportID = 28;
        rVar.storeType = StoreType::Average;
        rVar.sched = Sched::GetSchedule(*state, "SCHEDULENAME");
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Simulation]);
        op->freqTrackingVariables[(int)ReportFreq::Simulation] = false;
        EXPECT_TRUE(compare_eso_stream(delimited_string(
            {"28,11,keyedValue,variableName [m3/s] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute],scheduleName"}, "\n")));

        rVar.ReportID = 29;
        rVar.timeStepType = TimeStepType::System;
        rVar.sched = nullptr;
        rVar.writeReportDictionaryItem(*state);
        EXPECT_TRUE(op->freqTrackingVariables[(int)ReportFreq::Simulation]);
        op->freqTrackingVariables[(int)ReportFreq::Simulation] = false;
        EXPECT_TRUE(compare_eso_stream(
            delimited_string({"29,11,keyedValue,variableName [m3/s] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"}, "\n")));

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary(
            {{"1", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Zone Timestep", "", "m3/s"},
             {"2", "0", "Sum", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Zone Timestep", "", "m3/s"},
             {"3", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Zone Timestep", "scheduleName", "m3/s"},
             {"4", "0", "Avg", "indexGroup", timeStepSystemString, "keyedValue", "variableName", "Zone Timestep", "", "m3/s"},
             {"6", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "HVAC System Timestep", "", "m3/s"},
             {"7", "0", "Sum", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "HVAC System Timestep", "", "m3/s"},
             {"8", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "HVAC System Timestep", "scheduleName", "m3/s"},
             {"9", "0", "Avg", "indexGroup", timeStepSystemString, "keyedValue", "variableName", "HVAC System Timestep", "", "m3/s"},
             {"11", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Hourly", "", "m3/s"},
             {"12", "0", "Sum", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Hourly", "", "m3/s"},
             {"13", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Hourly", "scheduleName", "m3/s"},
             {"14", "0", "Avg", "indexGroup", timeStepSystemString, "keyedValue", "variableName", "Hourly", "", "m3/s"},
             {"16", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Daily", "", "m3/s"},
             {"17", "0", "Sum", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Daily", "", "m3/s"},
             {"18", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Daily", "scheduleName", "m3/s"},
             {"19", "0", "Avg", "indexGroup", timeStepSystemString, "keyedValue", "variableName", "Daily", "", "m3/s"},
             {"21", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Monthly", "", "m3/s"},
             {"22", "0", "Sum", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Monthly", "", "m3/s"},
             {"23", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Monthly", "scheduleName", "m3/s"},
             {"24", "0", "Avg", "indexGroup", timeStepSystemString, "keyedValue", "variableName", "Monthly", "", "m3/s"},
             {"26", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Run Period", "", "m3/s"},
             {"27", "0", "Sum", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Run Period", "", "m3/s"},
             {"28", "0", "Avg", "indexGroup", timeStepZoneString, "keyedValue", "variableName", "Run Period", "scheduleName", "m3/s"},
             {"29", "0", "Avg", "indexGroup", timeStepSystemString, "keyedValue", "variableName", "Run Period", "", "m3/s"}});

        EXPECT_EQ(reportDataDictionary.size(), reportDataDictionaryResults.size());
        for (int i = 0; i < reportDataDictionary.size(); ++i) {
            EXPECT_EQ(reportDataDictionary[i], reportDataDictionaryResults[i]);
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeCumulativeReportMeterData)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);

        WriteCumulativeReportMeterData(*state, 1, 616771620.98702729, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273"}, "\n")));

        WriteCumulativeReportMeterData(*state, 1, 616771620.98702729, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,616771620.9870273"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,616771620.9870273"}, "\n")));

        WriteCumulativeReportMeterData(*state, 1, 0, true);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,0.0"}, "\n")));

        WriteCumulativeReportMeterData(*state, 1, 0, false);
        EXPECT_TRUE(compare_mtr_stream(delimited_string({"1,0.0"}, "\n")));
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0"}, "\n")));

        auto reportData = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedData = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        ASSERT_EQ(4ul, reportData.size());
        std::vector<std::string> reportData0{"1", "1", "1", "616771620.987027"};
        std::vector<std::string> reportData1{"2", "1", "1", "616771620.987027"};
        std::vector<std::string> reportData2{"3", "1", "1", "0.0"};
        std::vector<std::string> reportData3{"4", "1", "1", "0.0"};
        EXPECT_EQ(reportData0, reportData[0]);
        EXPECT_EQ(reportData1, reportData[1]);
        EXPECT_EQ(reportData2, reportData[2]);
        EXPECT_EQ(reportData3, reportData[3]);

        ASSERT_EQ(0ul, reportExtendedData.size());
    }

    TEST_F(SQLiteFixture, OutputProcessor_writeNumericData_2)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        sql->createSQLiteTimeIndexRecord(ReportFreq::Simulation, 1, 1, 0, 2017, false);
        sql->createSQLiteReportDictionaryRecord(
            1, StoreType::Average, "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, "C", ReportFreq::Hour, false);

        WriteNumericData(*state, 1, 0);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0"}, "\n")));

        WriteNumericData(*state, 1, 0.1);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.1"}, "\n")));

        WriteNumericData(*state, 1, -0.1);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-0.1"}, "\n")));

        WriteNumericData(*state, 1, 1.0e-2);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.01"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.01"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-3);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.001"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.001"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-4);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0001"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.0001"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-5);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.00001"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.00001"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-6);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.000001"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.000001"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-7);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-7"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-7"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-8);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-8"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-8"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-9);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-9"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-9"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-10);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-10"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-10"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-11);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-11"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-11"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-12);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-12"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-12"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-13);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-13"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-13"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-14);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-14"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-14"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-15);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-15"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-15"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-16);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-16"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-16"}, "\n")));
#endif

        WriteNumericData(*state, 1, -1.0e-16);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-1e-16"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-1e-16"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e-19);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-19"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e-19"}, "\n")));
#endif

        WriteNumericData(*state, 1, 0.5);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,0.5"}, "\n")));

        WriteNumericData(*state, 1, 1.0);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1.0"}, "\n")));

        WriteNumericData(*state, 1, 10.0);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,10.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e2);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e3);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e4);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,10000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e5);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e6);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e7);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,10000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e8);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e9);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e10);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,10000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e11);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e12);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1000000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e13);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,10000000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e14);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100000000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e15);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1000000000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e16);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,10000000000000000.0"}, "\n")));

        WriteNumericData(*state, 1, 1.0e17);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100000000000000000.0"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,100000000000000000.0"}, "\n")));
#endif

        WriteNumericData(*state, 1, -1.0e16);
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-10000000000000000.0"}, "\n")));

        WriteNumericData(*state, 1, -1.0e17);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-100000000000000000.0"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,-100000000000000000.0"}, "\n")));
#endif

        WriteNumericData(*state, 1, 1.0e25);
#if defined(_WIN32) && _MSC_VER < 1900
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e25"}, "\n")));
#else
        EXPECT_TRUE(compare_eso_stream(delimited_string({"1,1e25"}, "\n")));
#endif

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({{"1", "1", "1", "0.0"},
                                                          {"2", "1", "1", "0.1"},
                                                          {"3", "1", "1", "-0.1"},
                                                          {"4", "1", "1", "0.01"},
                                                          {"5", "1", "1", "0.001"},
                                                          {"6", "1", "1", "0.0001"},
                                                          {"7", "1", "1", "1.0e-05"},
                                                          {"8", "1", "1", "1.0e-06"},
                                                          {"9", "1", "1", "1.0e-07"},
                                                          {"10", "1", "1", "1.0e-08"},
                                                          {"11", "1", "1", "1.0e-09"},
                                                          {"12", "1", "1", "1.0e-10"},
                                                          {"13", "1", "1", "1.0e-11"},
                                                          {"14", "1", "1", "1.0e-12"},
                                                          {"15", "1", "1", "1.0e-13"},
                                                          {"16", "1", "1", "1.0e-14"},
                                                          {"17", "1", "1", "1.0e-15"},
                                                          {"18", "1", "1", "1.0e-16"},
                                                          {"19", "1", "1", "-1.0e-16"},
                                                          {"20", "1", "1", "1.0e-19"},
                                                          {"21", "1", "1", "0.5"},
                                                          {"22", "1", "1", "1.0"},
                                                          {"23", "1", "1", "10.0"},
                                                          {"24", "1", "1", "100.0"},
                                                          {"25", "1", "1", "1000.0"},
                                                          {"26", "1", "1", "10000.0"},
                                                          {"27", "1", "1", "100000.0"},
                                                          {"28", "1", "1", "1000000.0"},
                                                          {"29", "1", "1", "10000000.0"},
                                                          {"30", "1", "1", "100000000.0"},
                                                          {"31", "1", "1", "1000000000.0"},
                                                          {"32", "1", "1", "10000000000.0"},
                                                          {"33", "1", "1", "100000000000.0"},
                                                          {"34", "1", "1", "1000000000000.0"},
                                                          {"35", "1", "1", "10000000000000.0"},
                                                          {"36", "1", "1", "100000000000000.0"},
                                                          {"37", "1", "1", "1.0e+15"},
                                                          {"38", "1", "1", "1.0e+16"},
                                                          {"39", "1", "1", "1.0e+17"},
                                                          {"40", "1", "1", "-1.0e+16"},
                                                          {"41", "1", "1", "-1.0e+17"},
                                                          {"42", "1", "1", "1.0e+25"}});

        std::vector<std::vector<std::string>> reportExtendedData({});

        EXPECT_EQ(reportData, reportDataResults);
        ASSERT_EQ(reportExtendedData.size(), reportExtendedDataResults.size());
    }

    TEST_F(SQLiteFixture, OutputProcessor_addMeter)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        auto &op = state->dataOutputProcessor;

        auto const name("testMeter");
        Constant::Units const units(Constant::Units::J);
        Constant::eResource resource = Constant::eResource::Electricity;
        EndUseCat endUseCat = EndUseCat::ExteriorLights;
        auto const endUseSub("testEndUseSub");
        Group group = Group::Building;

        EXPECT_EQ(0ul, op->meters.size());

        AddMeter(*state, name, units, resource, endUseCat, endUseSub, group, -1);

        ASSERT_EQ(1ul, op->meters.size());

        EXPECT_EQ(name, op->meters[0]->Name);
        EXPECT_EQ((int)resource, (int)op->meters[0]->resource);
        EXPECT_EQ((int)endUseCat, (int)op->meters[0]->endUseCat);
        EXPECT_EQ(endUseSub, op->meters[0]->EndUseSub);
        EXPECT_EQ((int)group, (int)op->meters[0]->group);
        EXPECT_EQ((int)units, (int)op->meters[0]->units);
        EXPECT_EQ(1, op->meters[0]->periods[(int)ReportFreq::TimeStep].RptNum);
        EXPECT_EQ(2, op->meters[0]->periods[(int)ReportFreq::Hour].RptNum);
        EXPECT_EQ(3, op->meters[0]->periods[(int)ReportFreq::Day].RptNum);
        EXPECT_EQ(4, op->meters[0]->periods[(int)ReportFreq::Month].RptNum);
        EXPECT_EQ(6, op->meters[0]->periods[(int)ReportFreq::Simulation].RptNum);
        EXPECT_EQ(5, op->meters[0]->periods[(int)ReportFreq::Year].RptNum);
        EXPECT_EQ(7, op->meters[0]->periods[(int)ReportFreq::TimeStep].accRptNum);
        EXPECT_EQ(8, op->meters[0]->periods[(int)ReportFreq::Hour].accRptNum);
        EXPECT_EQ(9, op->meters[0]->periods[(int)ReportFreq::Day].accRptNum);
        EXPECT_EQ(10, op->meters[0]->periods[(int)ReportFreq::Month].accRptNum);
        EXPECT_EQ(12, op->meters[0]->periods[(int)ReportFreq::Simulation].accRptNum);
        EXPECT_EQ(11, op->meters[0]->periods[(int)ReportFreq::Year].accRptNum);

        EXPECT_EQ(1ul, op->meters.size());

        sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

        auto const name2("testMeter2");
        Constant::Units const units2 = Constant::Units::unknown; // was "kwh"
        Constant::eResource resource2 = Constant::eResource::None;
        EndUseCat endUseCat2 = EndUseCat::Refrigeration;
        auto const endUseSub2("testEndUseSub2");
        Group group2 = Group::Plant;
        AddMeter(*state, name2, units2, resource2, endUseCat2, endUseSub2, group2, -1);

        auto errorData = queryResult("SELECT * FROM Errors;", "Errors");

        ASSERT_EQ(1ul, errorData.size());
        std::vector<std::string> errorData0{"1",
                                            "1",
                                            "0",
                                            "DetermineMeterIPUnits: Meter units not recognized for IP Units conversion=[unknown].  ..on "
                                            "Meter=\"testMeter2\".  ..requests for IP units from this meter will be ignored.",
                                            "0"};
        EXPECT_EQ(errorData0, errorData[0]);

        ASSERT_EQ(2ul, op->meters.size());
    }

    // We're not really doing things this way anymore
    TEST_F(SQLiteFixture, OutputProcessor_validateNStandardizeMeterTitles)
    {
        auto &sql = state->dataSQLiteProcedures->sqlite;
        std::vector<std::vector<std::string>> input_map = {{"J", "ELECTRICITY", "INTERIORLIGHTS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "INTERIORLIGHTS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "INTERIORLIGHTS", "endUseSub", "PLANT"},
                                                           {"J", "ELECTRICITY", "INTERIORLIGHTS", "endUseSub", "BUILDING", "zoneName"},
                                                           {"J", "ELECTRICITY", "INTERIORLIGHTS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "EXTERIORLIGHTS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATING", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATPRODUCED", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "COOLING", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "WATERSYSTEMS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "COGENERATION", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "INTERIOREQUIPMENT", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "EXTERIOREQUIPMENT", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "DISTRICTHOTWATER", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "DISTRICTCHILLEDWATER", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "FANS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATINGCOILS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "COOLINGCOILS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "PUMPS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "FREECOOLING", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "LOOPTOLOOP", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "CHILLERS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "BOILERS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "BASEBOARD", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATREJECTION", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HUMIDIFIER", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATRECOVERY", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "PHOTOVOLTAIC", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "WINDTURBINE", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "ELECTRICSTORAGE", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATRECOVERYFORCOOLING", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "HEATRECOVERYFORHEATING", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "ELECTRICITYEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "PURCHASEDELECTRICITYEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "SOLDELECTRICITYEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "NATURALGASEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "FUELOILNO1EMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "FUELOILNO2EMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "COALEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "GASOLINEEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "PROPANEEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "DIESELEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "OTHERFUEL1EMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "OTHERFUEL2EMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "CARBONEQUIVALENTEMISSIONS", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "REFRIGERATION", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "COLDSTORAGECHARGE", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "COLDSTORAGEDISCHARGE", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "RAINWATER", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "CONDENSATE", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "WELLWATER", "endUseSub", "HVAC"},
                                                           {"J", "ELECTRICITY", "MAINSWATER", "endUseSub", "HVAC"}};

        std::vector<std::string> const result_map = {"Electricity:Facility",
                                                     "Electricity:HVAC",
                                                     "InteriorLights:Electricity",
                                                     "endUseSub:InteriorLights:Electricity",
                                                     "Electricity:Plant",
                                                     "Electricity:Building",
                                                     "Electricity:Zone:zoneName",
                                                     "InteriorLights:Electricity:Zone:zoneName",
                                                     "endUseSub:InteriorLights:Electricity:Zone:zoneName",
                                                     "ExteriorLights:Electricity",
                                                     "endUseSub:ExteriorLights:Electricity",
                                                     "Heating:Electricity",
                                                     "endUseSub:Heating:Electricity",
                                                     "HeatProduced:Electricity",
                                                     "endUseSub:HeatProduced:Electricity",
                                                     "Cooling:Electricity",
                                                     "endUseSub:Cooling:Electricity",
                                                     "WaterSystems:Electricity",
                                                     "endUseSub:WaterSystems:Electricity",
                                                     "Cogeneration:Electricity",
                                                     "endUseSub:Cogeneration:Electricity",
                                                     "InteriorEquipment:Electricity",
                                                     "endUseSub:InteriorEquipment:Electricity",
                                                     "ExteriorEquipment:Electricity",
                                                     "endUseSub:ExteriorEquipment:Electricity",
                                                     "DistrictHotWater:Electricity",
                                                     "endUseSub:DistrictHotWater:Electricity",
                                                     "DistrictChilledWater:Electricity",
                                                     "endUseSub:DistrictChilledWater:Electricity",
                                                     "Fans:Electricity",
                                                     "endUseSub:Fans:Electricity",
                                                     "HeatingCoils:Electricity",
                                                     "endUseSub:HeatingCoils:Electricity",
                                                     "CoolingCoils:Electricity",
                                                     "endUseSub:CoolingCoils:Electricity",
                                                     "Pumps:Electricity",
                                                     "endUseSub:Pumps:Electricity",
                                                     "FreeCooling:Electricity",
                                                     "endUseSub:FreeCooling:Electricity",
                                                     "LoopToLoop:Electricity",
                                                     "endUseSub:LoopToLoop:Electricity",
                                                     "Chillers:Electricity",
                                                     "endUseSub:Chillers:Electricity",
                                                     "Boilers:Electricity",
                                                     "endUseSub:Boilers:Electricity",
                                                     "Baseboard:Electricity",
                                                     "endUseSub:Baseboard:Electricity",
                                                     "HeatRejection:Electricity",
                                                     "endUseSub:HeatRejection:Electricity",
                                                     "Humidifier:Electricity",
                                                     "endUseSub:Humidifier:Electricity",
                                                     "HeatRecovery:Electricity",
                                                     "endUseSub:HeatRecovery:Electricity",
                                                     "Photovoltaic:Electricity",
                                                     "endUseSub:Photovoltaic:Electricity",
                                                     "WindTurbine:Electricity",
                                                     "endUseSub:WindTurbine:Electricity",
                                                     "ElectricStorage:Electricity",
                                                     "endUseSub:ElectricStorage:Electricity",
                                                     "HeatRecoveryForCooling:Electricity",
                                                     "endUseSub:HeatRecoveryForCooling:Electricity",
                                                     "HeatRecoveryForHeating:Electricity",
                                                     "endUseSub:HeatRecoveryForHeating:Electricity",
                                                     "ElectricityEmissions:Electricity",
                                                     "endUseSub:ElectricityEmissions:Electricity",
                                                     "PurchasedElectricityEmissions:Electricity",
                                                     "endUseSub:PurchasedElectricityEmissions:Electricity",
                                                     "SoldElectricityEmissions:Electricity",
                                                     "endUseSub:SoldElectricityEmissions:Electricity",
                                                     "NaturalGasEmissions:Electricity",
                                                     "endUseSub:NaturalGasEmissions:Electricity",
                                                     "FuelOilNo1Emissions:Electricity",
                                                     "endUseSub:FuelOilNo1Emissions:Electricity",
                                                     "FuelOilNo2Emissions:Electricity",
                                                     "endUseSub:FuelOilNo2Emissions:Electricity",
                                                     "CoalEmissions:Electricity",
                                                     "endUseSub:CoalEmissions:Electricity",
                                                     "GasolineEmissions:Electricity",
                                                     "endUseSub:GasolineEmissions:Electricity",
                                                     "PropaneEmissions:Electricity",
                                                     "endUseSub:PropaneEmissions:Electricity",
                                                     "DieselEmissions:Electricity",
                                                     "endUseSub:DieselEmissions:Electricity",
                                                     "OtherFuel1Emissions:Electricity",
                                                     "endUseSub:OtherFuel1Emissions:Electricity",
                                                     "OtherFuel2Emissions:Electricity",
                                                     "endUseSub:OtherFuel2Emissions:Electricity",
                                                     "CarbonEquivalentEmissions:Electricity",
                                                     "endUseSub:CarbonEquivalentEmissions:Electricity",
                                                     "Refrigeration:Electricity",
                                                     "endUseSub:Refrigeration:Electricity",
                                                     "ColdStorageCharge:Electricity",
                                                     "endUseSub:ColdStorageCharge:Electricity",
                                                     "ColdStorageDischarge:Electricity",
                                                     "endUseSub:ColdStorageDischarge:Electricity",
                                                     "RainWater:Electricity",
                                                     "endUseSub:RainWater:Electricity",
                                                     "Condensate:Electricity",
                                                     "endUseSub:Condensate:Electricity",
                                                     "WellWater:Electricity",
                                                     "endUseSub:WellWater:Electricity",
                                                     "MainsWater:Electricity",
                                                     "endUseSub:MainsWater:Electricity"};
        auto &op = state->dataOutputProcessor;
        InitializeOutput(*state);

        bool errorFound = false;
        for (auto &meter : input_map) {
            errorFound = false;
            EndUseCat endUseCat = static_cast<EndUseCat>(getEnumValue(endUseCatNamesUC, meter[2]));
            std::string stdEndUseSub = standardizeEndUseSub(endUseCat, meter[3]);
            Group group = static_cast<Group>(getEnumValue(groupNamesUC, meter[4]));

            EXPECT_ENUM_NE(EndUseCat::Invalid, endUseCat);
            EXPECT_ENUM_NE(Group::Invalid, group);

            AttachMeters(*state,
                         Constant::Units::J,
                         Constant::eResource::Electricity,
                         endUseCat,
                         meter[3],
                         group,
                         (meter.size() == 6) ? meter[5] : "",
                         "",
                         -1);
        }

        ASSERT_EQ(103ul, op->meters.size());

        for (int i = 0; i < (int)op->meters.size(); ++i) {
            EXPECT_EQ(result_map[i], op->meters[i]->Name);
        }

        sql->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupTimePointers)
    {
        // TimeValue.allocate(2);
        auto &op = state->dataOutputProcessor;
        Real64 timeStep = 1.0;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);

        EXPECT_DOUBLE_EQ(timeStep, *op->TimeValue[(int)TimeStepType::Zone].TimeStep);
        EXPECT_DOUBLE_EQ(0.0, op->TimeValue[(int)TimeStepType::Zone].CurMinute);

        timeStep = 2.0;

        SetupTimePointers(*state, TimeStepType::System, timeStep);

        EXPECT_DOUBLE_EQ(timeStep, *op->TimeValue[(int)TimeStepType::System].TimeStep);
        EXPECT_DOUBLE_EQ(0.0, op->TimeValue[(int)TimeStepType::System].CurMinute);
    }

    TEST_F(SQLiteFixture, OutputProcessor_getReportVariableInput)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        GetReportVariableInput(*state);

        EXPECT_EQ(5, state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "Output:Variable"));
        EXPECT_EQ(5, op->reqVars.size());

        EXPECT_EQ("", op->reqVars[0]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[0]->name);
        EXPECT_EQ((int)ReportFreq::TimeStep, (int)op->reqVars[0]->freq);
        EXPECT_EQ(nullptr, op->reqVars[0]->sched);
        EXPECT_FALSE(op->reqVars[0]->Used);

        EXPECT_EQ("", op->reqVars[1]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[1]->name);
        EXPECT_EQ((int)ReportFreq::Hour, (int)op->reqVars[1]->freq);
        EXPECT_EQ(nullptr, op->reqVars[1]->sched);
        EXPECT_FALSE(op->reqVars[1]->Used);

        EXPECT_EQ("", op->reqVars[2]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[2]->name);
        EXPECT_EQ((int)ReportFreq::Day, (int)op->reqVars[2]->freq);
        EXPECT_EQ(nullptr, op->reqVars[2]->sched);
        EXPECT_FALSE(op->reqVars[2]->Used);

        EXPECT_EQ("", op->reqVars[3]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[3]->name);
        EXPECT_EQ((int)ReportFreq::Month, (int)op->reqVars[3]->freq);
        EXPECT_EQ(nullptr, op->reqVars[3]->sched);
        EXPECT_FALSE(op->reqVars[3]->Used);

        EXPECT_EQ("", op->reqVars[4]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[4]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[4]->freq);
        EXPECT_EQ(nullptr, op->reqVars[4]->sched);
        EXPECT_FALSE(op->reqVars[4]->Used);
    }

    TEST_F(SQLiteFixture, OutputProcessor_buildKeyVarList)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        GetReportVariableInput(*state);

        Real64 faketmp = 0;

        SetupOutputVariable(
            *state, "Site Outdoor Air Drybulb Temperature", Constant::Units::C, faketmp, TimeStepType::Zone, StoreType::Average, "Environment");

        // EXPECT_EQ(5, op->NumExtraVars);
        EXPECT_EQ(6, op->reqVars.size());

        EXPECT_EQ("", op->reqVars[0]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[0]->name);
        EXPECT_EQ((int)ReportFreq::TimeStep, (int)op->reqVars[0]->freq);
        EXPECT_EQ(nullptr, op->reqVars[0]->sched);
        EXPECT_TRUE(op->reqVars[0]->Used);

        EXPECT_EQ("", op->reqVars[1]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[1]->name);
        EXPECT_EQ((int)ReportFreq::Hour, (int)op->reqVars[1]->freq);
        EXPECT_EQ(nullptr, op->reqVars[1]->sched);
        EXPECT_TRUE(op->reqVars[1]->Used);

        EXPECT_EQ("", op->reqVars[2]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[2]->name);
        EXPECT_EQ((int)ReportFreq::Day, (int)op->reqVars[2]->freq);
        EXPECT_EQ(nullptr, op->reqVars[2]->sched);
        EXPECT_TRUE(op->reqVars[2]->Used);

        EXPECT_EQ("", op->reqVars[3]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[3]->name);
        EXPECT_EQ((int)ReportFreq::Month, (int)op->reqVars[3]->freq);
        EXPECT_EQ(nullptr, op->reqVars[3]->sched);
        EXPECT_TRUE(op->reqVars[3]->Used);

        EXPECT_EQ("", op->reqVars[4]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[4]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[4]->freq);
        EXPECT_EQ(nullptr, op->reqVars[4]->sched);
        EXPECT_TRUE(op->reqVars[4]->Used);

        EXPECT_EQ("", op->reqVars[4]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[4]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[4]->freq);
        EXPECT_EQ(nullptr, op->reqVars[4]->sched);
        EXPECT_TRUE(op->reqVars[4]->Used);
    }

    TEST_F(SQLiteFixture, OutputProcessor_buildKeyVarListWithKey)
    {
        std::string const idf_objects = delimited_string({
            "  Output:Table:Monthly,",
            "    Test Report,  !- Name",
            "    2,                       !- Digits After Decimal",
            "    Zone Total Internal Latent Gain Rate,  !- Variable or Meter 1 Name",
            "    SumOrAverage;            !- Aggregation Type for Variable or Meter 1",
            "",
            "Output:Variable,Living,Zone Total Internal Latent Gain Rate,Hourly;",
            "Output:Variable,Attic,Zone Total Internal Latent Gain Rate,Hourly;",
            "",
            "Output:Variable,Living,Zone Total Internal Sensible Gain Rate,Hourly;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        state->dataInputProcessing->inputProcessor->preScanReportingVariables(*state);
        InitializeOutput(*state);

        Real64 ilgrGarage = 0.0;
        Real64 ilgrLiving = 0.0;
        Real64 ilgrAttic = 0.0;

        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrGarage, TimeStepType::Zone, StoreType::Sum, "Garage");
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrLiving, TimeStepType::Zone, StoreType::Sum, "Living");
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrAttic, TimeStepType::Zone, StoreType::Sum, "Attic");

        Real64 isgrGarage = 0.0;
        Real64 isgrLiving = 0.0;
        Real64 isgrAttic = 0.0;

        SetupOutputVariable(
            *state, "Zone Total Internal Sensible Gain Rate", Constant::Units::J, isgrGarage, TimeStepType::Zone, StoreType::Sum, "Garage");
        SetupOutputVariable(
            *state, "Zone Total Internal Sensible Gain Rate", Constant::Units::J, isgrLiving, TimeStepType::Zone, StoreType::Sum, "Living");
        SetupOutputVariable(
            *state, "Zone Total Internal Sensible Gain Rate", Constant::Units::J, isgrAttic, TimeStepType::Zone, StoreType::Sum, "Attic");

        state->dataGlobal->DoWeathSim = true;
        state->dataGlobal->TimeStepZone = 0.25;

        OutputReportTabular::GetInputTabularMonthly(*state);
        EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
        OutputReportTabular::InitializeTabularMonthly(*state);

        GetReportVariableInput(*state);

        Real64 fakeVar = 0.0;
        auto resetReqRepVarsUsed = [this]() {
            auto &op = state->dataOutputProcessor;
            for (auto *reqVar : op->reqVars) {
                reqVar->Used = false;
            }
        };
        auto countReqRepVarsUsed = [this]() {
            auto &op = state->dataOutputProcessor;
            int count = 0;
            for (auto const *reqVar : op->reqVars) {
                if (reqVar->Used) {
                    ++count;
                }
            }
            return count;
        };

        resetReqRepVarsUsed();
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::W, fakeVar, TimeStepType::Zone, StoreType::Average, "LIVING");

        EXPECT_EQ(1, countReqRepVarsUsed());
        // EXPECT_EQ(1, op->NumExtraVars);

        resetReqRepVarsUsed();
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::W, fakeVar, TimeStepType::Zone, StoreType::Average, "GARAGE");
        EXPECT_EQ(0, countReqRepVarsUsed()); // Garage not part of the list
        // EXPECT_EQ(1, op->NumExtraVars);

        resetReqRepVarsUsed();

        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::W, fakeVar, TimeStepType::Zone, StoreType::Average, "ATTIC");
        EXPECT_EQ(1, countReqRepVarsUsed());
        // EXPECT_EQ(1, op->NumExtraVars);
    }

    TEST_F(SQLiteFixture, OutputProcessor_buildKeyVarListWithRegexKey)
    {
        std::string const idf_objects = delimited_string({
            "  Output:Table:Monthly,",
            "    Test Report,  !- Name",
            "    2,                       !- Digits After Decimal",
            "    Zone Total Internal Latent Gain Rate,  !- Variable or Meter 1 Name",
            "    SumOrAverage;            !- Aggregation Type for Variable or Meter 1",
            "",
            "Output:Variable,Liv.*,Zone Total Internal Latent Gain Rate,Hourly;",
            "",
            "Output:Variable,Living,Zone Total Internal Sensible Gain Rate,Hourly;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        state->dataInputProcessing->inputProcessor->preScanReportingVariables(*state);
        InitializeOutput(*state);

        Real64 ilgrGarage;
        Real64 ilgrLiving1;
        Real64 ilgrLiving2;

        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrGarage, TimeStepType::Zone, StoreType::Sum, "Garage");
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrLiving1, TimeStepType::Zone, StoreType::Sum, "Living1");
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrLiving2, TimeStepType::Zone, StoreType::Sum, "Living2");

        Real64 isgrGarage;
        Real64 isgrLiving;
        Real64 isgrAttic;

        SetupOutputVariable(
            *state, "Zone Total Internal Sensible Gain Rate", Constant::Units::J, isgrGarage, TimeStepType::Zone, StoreType::Sum, "Garage");
        SetupOutputVariable(
            *state, "Zone Total Internal Sensible Gain Rate", Constant::Units::J, isgrLiving, TimeStepType::Zone, StoreType::Sum, "Living1");
        SetupOutputVariable(
            *state, "Zone Total Internal Sensible Gain Rate", Constant::Units::J, isgrAttic, TimeStepType::Zone, StoreType::Sum, "Living2");

        state->dataGlobal->DoWeathSim = true;
        state->dataGlobal->TimeStepZone = 0.25;

        OutputReportTabular::GetInputTabularMonthly(*state);
        EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
        OutputReportTabular::InitializeTabularMonthly(*state);

        auto &op = state->dataOutputProcessor;

        // This has already been called by SetupOutputVariable so it'll do nothing
        // GetReportVariableInput(*state);

        EXPECT_EQ(2, op->reqVars.size());
        auto *varLatentRegex = op->reqVars[0];
        EXPECT_EQ("Liv.*", varLatentRegex->key);
        EXPECT_FALSE(varLatentRegex->is_simple_string);
        EXPECT_NE(nullptr, varLatentRegex->case_insensitive_pattern);

        auto *varSensibleNormal = op->reqVars[1];
        EXPECT_EQ("Living", varSensibleNormal->key);
        EXPECT_TRUE(varSensibleNormal->is_simple_string);
        EXPECT_EQ(nullptr, varSensibleNormal->case_insensitive_pattern);

        auto resetReqRepVarsUsed = [this]() {
            auto &op = state->dataOutputProcessor;
            for (auto *reqVar : op->reqVars) {
                reqVar->Used = false;
            }
            // op->NumExtraVars = 0;
        };
        auto countReqRepVarsUsed = [this]() {
            auto &op = state->dataOutputProcessor;
            int count = 0;
            for (auto const *reqVar : op->reqVars) {
                if (reqVar->Used) {
                    ++count;
                }
            }
            return count;
        };

        EXPECT_EQ(1, countReqRepVarsUsed());
        // EXPECT_EQ(1, op->NumExtraVars);

        resetReqRepVarsUsed();
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrGarage, TimeStepType::Zone, StoreType::Sum, "LIVING1");
        EXPECT_EQ(1, countReqRepVarsUsed());
        // EXPECT_EQ(1, op->NumExtraVars);
        EXPECT_TRUE(varLatentRegex->Used);
        EXPECT_FALSE(varSensibleNormal->Used);

        resetReqRepVarsUsed();

        resetReqRepVarsUsed();
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrGarage, TimeStepType::Zone, StoreType::Sum, "LIVING");
        EXPECT_EQ(1, countReqRepVarsUsed());
        // EXPECT_EQ(1, op->NumExtraVars);
        EXPECT_TRUE(varLatentRegex->Used);
        EXPECT_FALSE(varSensibleNormal->Used);

        resetReqRepVarsUsed();
        SetupOutputVariable(
            *state, "Zone Total Internal Latent Gain Rate", Constant::Units::J, ilgrGarage, TimeStepType::Zone, StoreType::Sum, "GARAGE");
        EXPECT_EQ(0, countReqRepVarsUsed());
        // When NumExtraVars is 0 after CheckReportVariable, it resets to 1...
        // EXPECT_EQ(1, op->NumExtraVars);
        EXPECT_FALSE(varLatentRegex->Used);
        EXPECT_FALSE(varSensibleNormal->Used);
    }

    TEST_F(SQLiteFixture, OutputProcessor_addBlankKeys)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        InitializeOutput(*state);

        GetReportVariableInput(*state);

        Real64 fakeVar = 0.0;
        SetupOutputVariable(
            *state, "Site Outdoor Air Drybulb Temperature", Constant::Units::C, fakeVar, TimeStepType::Zone, StoreType::Average, "Environment");

        // EXPECT_EQ(5, op->NumExtraVars);
        // EXPECT_EQ(1, op->ReportList(1));
        // EXPECT_EQ(2, op->ReportList(2));
        // EXPECT_EQ(3, op->ReportList(3));
        // EXPECT_EQ(4, op->ReportList(4));
        // EXPECT_EQ(5, op->ReportList(5));
        EXPECT_EQ(5, op->reqVars.size());

        EXPECT_EQ("", op->reqVars[0]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[0]->name);
        EXPECT_EQ((int)ReportFreq::TimeStep, (int)op->reqVars[0]->freq);
        EXPECT_EQ(nullptr, op->reqVars[0]->sched);
        EXPECT_TRUE(op->reqVars[0]->Used);

        EXPECT_EQ("", op->reqVars[1]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[1]->name);
        EXPECT_EQ((int)ReportFreq::Hour, (int)op->reqVars[1]->freq);
        EXPECT_EQ(nullptr, op->reqVars[1]->sched);
        EXPECT_TRUE(op->reqVars[1]->Used);

        EXPECT_EQ("", op->reqVars[2]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[2]->name);
        EXPECT_EQ((int)ReportFreq::Day, (int)op->reqVars[2]->freq);
        EXPECT_EQ(nullptr, op->reqVars[2]->sched);
        EXPECT_TRUE(op->reqVars[2]->Used);

        EXPECT_EQ("", op->reqVars[3]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[3]->name);
        EXPECT_EQ((int)ReportFreq::Month, (int)op->reqVars[3]->freq);
        EXPECT_EQ(nullptr, op->reqVars[3]->sched);
        EXPECT_TRUE(op->reqVars[3]->Used);

        EXPECT_EQ("", op->reqVars[4]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[4]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[4]->freq);
        EXPECT_EQ(nullptr, op->reqVars[4]->sched);
        EXPECT_TRUE(op->reqVars[4]->Used);
    }

    TEST_F(SQLiteFixture, OutputProcessor_determineFrequency)
    {
        auto const valid_options = std::map<std::string, ReportFreq>({{"Detailed", ReportFreq::EachCall},
                                                                      {"Timestep", ReportFreq::TimeStep},
                                                                      {"Hourly", ReportFreq::Hour},
                                                                      {"Daily", ReportFreq::Day},
                                                                      {"Monthly", ReportFreq::Month},
                                                                      {"RunPeriod", ReportFreq::Simulation},
                                                                      {"Environment", ReportFreq::Simulation},
                                                                      {"Annual", ReportFreq::Year},
                                                                      {"Bad Input", ReportFreq::Hour}});

        ReportFreq report_freq = ReportFreq::EachCall;

        for (auto const &option : valid_options) {
            report_freq = determineFrequency(*state, option.first);
            EXPECT_EQ((int)option.second, (int)report_freq);
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_addToOutputVariableList)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Wetbulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Humidity Ratio,hourly;",
            "Output:Variable,*,Site Outdoor Air Relative Humidity,hourly;",
            "Output:Variable,*,Zone Mean Air Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        AddDDOutVar(*state, "Site Outdoor Air Drybulb Temperature", TimeStepType::Zone, StoreType::Average, VariableType::Real, Constant::Units::C);
        AddDDOutVar(*state, "Site Outdoor Air Wetbulb Temperature", TimeStepType::Zone, StoreType::Average, VariableType::Real, Constant::Units::C);
        AddDDOutVar(
            *state, "Site Outdoor Air Humidity Ratio", TimeStepType::Zone, StoreType::Average, VariableType::Real, Constant::Units::kgWater_kgDryAir);
        AddDDOutVar(*state, "Site Outdoor Air Relative Humidity", TimeStepType::Zone, StoreType::Average, VariableType::Real, Constant::Units::Perc);

        EXPECT_EQ((int)TimeStepType::Zone, (int)op->ddOutVars[0]->timeStepType);
        EXPECT_EQ((int)StoreType::Average, (int)op->ddOutVars[0]->storeType);
        EXPECT_EQ((int)VariableType::Real, (int)op->ddOutVars[0]->variableType);
        EXPECT_EQ(-1, op->ddOutVars[0]->Next);
        EXPECT_FALSE(op->ddOutVars[0]->ReportedOnDDFile);
        EXPECT_EQ("Site Outdoor Air Drybulb Temperature", op->ddOutVars[0]->name);
        EXPECT_EQ((int)Constant::Units::C, (int)op->ddOutVars[0]->units);

        EXPECT_EQ((int)TimeStepType::Zone, (int)op->ddOutVars[1]->timeStepType);
        EXPECT_EQ((int)StoreType::Average, (int)op->ddOutVars[1]->storeType);
        EXPECT_EQ((int)VariableType::Real, (int)op->ddOutVars[1]->variableType);
        EXPECT_EQ(-1, op->ddOutVars[1]->Next);
        EXPECT_FALSE(op->ddOutVars[1]->ReportedOnDDFile);
        EXPECT_EQ("Site Outdoor Air Wetbulb Temperature", op->ddOutVars[1]->name);
        EXPECT_EQ((int)Constant::Units::C, (int)op->ddOutVars[1]->units);

        EXPECT_EQ((int)TimeStepType::Zone, (int)op->ddOutVars[2]->timeStepType);
        EXPECT_EQ((int)StoreType::Average, (int)op->ddOutVars[2]->storeType);
        EXPECT_EQ((int)VariableType::Real, (int)op->ddOutVars[2]->variableType);
        EXPECT_EQ(-1, op->ddOutVars[2]->Next);
        EXPECT_FALSE(op->ddOutVars[2]->ReportedOnDDFile);
        EXPECT_EQ("Site Outdoor Air Humidity Ratio", op->ddOutVars[2]->name);
        EXPECT_EQ((int)Constant::Units::kgWater_kgDryAir, (int)op->ddOutVars[2]->units);

        EXPECT_EQ((int)TimeStepType::Zone, (int)op->ddOutVars[3]->timeStepType);
        EXPECT_EQ((int)StoreType::Average, (int)op->ddOutVars[3]->storeType);
        EXPECT_EQ((int)VariableType::Real, (int)op->ddOutVars[3]->variableType);
        EXPECT_EQ(-1, op->ddOutVars[3]->Next);
        EXPECT_FALSE(op->ddOutVars[3]->ReportedOnDDFile);
        EXPECT_EQ("Site Outdoor Air Relative Humidity", op->ddOutVars[3]->name);
        EXPECT_EQ((int)Constant::Units::Perc, (int)op->ddOutVars[3]->units);
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupOutputVariable)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        GetReportVariableInput(*state);
        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary(
            {{"1", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Run Period", "", "C"}});
        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        // EXPECT_EQ(1, op->NumExtraVars);

        EXPECT_EQ("", op->reqVars[0]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[0]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[0]->freq);
        EXPECT_EQ(nullptr, op->reqVars[0]->sched);
        EXPECT_EQ(true, op->reqVars[0]->Used);

        EXPECT_EQ((int)TimeStepType::Zone, (int)op->ddOutVars[0]->timeStepType);
        EXPECT_EQ((int)StoreType::Average, (int)op->ddOutVars[0]->storeType);
        EXPECT_EQ((int)VariableType::Real, (int)op->ddOutVars[0]->variableType);
        EXPECT_EQ(-1, op->ddOutVars[0]->Next);
        EXPECT_FALSE(op->ddOutVars[0]->ReportedOnDDFile);
        EXPECT_EQ("Site Outdoor Air Drybulb Temperature", op->ddOutVars[0]->name);
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_setupOutputVariable_endUseSubKey)
    {
        auto &op = state->dataOutputProcessor;

        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Chiller Electricity Energy,runperiod;",
            "Output:Variable,*,Lights Electricity Energy,runperiod;",
            "Output:Variable,*,Environmental Impact Fuel Oil No 2 CO2 Emissions Mass,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));

        GetReportVariableInput(*state);

        Real64 cooling_consumption = 0.;
        SetupOutputVariable(*state,
                            "Chiller Electricity Energy",
                            Constant::Units::J,
                            cooling_consumption,
                            TimeStepType::System,
                            StoreType::Sum,
                            "Cool-1",
                            Constant::eResource::Electricity,
                            Group::Plant,
                            EndUseCat::Cooling);

        Real64 light_consumption = 0.;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "RailroadCrossing", // EndUseSubKey
                            "SPACE1-1",         // Zone
                            1,
                            1);

        Real64 fuel_oil_co2 = 0.;
        SetupOutputVariable(*state,
                            "Environmental Impact Fuel Oil No 2 CO2 Emissions Mass",
                            Constant::Units::kg,
                            fuel_oil_co2,
                            TimeStepType::System,
                            StoreType::Sum,
                            "Site",
                            Constant::eResource::CO2,
                            Group::Invalid,
                            EndUseCat::FuelOilNo2Emissions);

        // Cooling
        // testing an ABUPS end use with no sub end use specified
        EXPECT_EQ(1, op->EndUseCategory(2).NumSubcategories);
        EXPECT_EQ("General", op->EndUseCategory(2).SubcategoryName(1));

        auto found = op->meterMap.find(Util::makeUPPER("Cooling:Electricity"));
        EXPECT_NE(found, op->meterMap.end());
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found->second]->resource);
        EXPECT_EQ((int)EndUseCat::Cooling, (int)op->meters[found->second]->endUseCat);
        EXPECT_EQ("", op->meters[found->second]->EndUseSub);

        found = op->meterMap.find(Util::makeUPPER("General:Cooling:Electricity"));
        EXPECT_NE(found, op->meterMap.end());
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found->second]->resource);
        EXPECT_EQ((int)EndUseCat::Cooling, (int)op->meters[found->second]->endUseCat);
        EXPECT_EQ("General", op->meters[found->second]->EndUseSub);

        // lighting
        // testing an ABUPS end use with a sub end use specified
        EXPECT_EQ(1, op->EndUseCategory(3).NumSubcategories); // lighting end use
        EXPECT_EQ("RailroadCrossing", op->EndUseCategory(3).SubcategoryName(1));

        found = op->meterMap.find(Util::makeUPPER("InteriorLights:Electricity"));
        EXPECT_NE(found, op->meterMap.end());
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found->second]->resource);
        EXPECT_EQ((int)EndUseCat::InteriorLights, (int)op->meters[found->second]->endUseCat);
        EXPECT_EQ("", op->meters[found->second]->EndUseSub);

        found = op->meterMap.find(Util::makeUPPER("General:InteriorLights:Electricity"));
        EXPECT_EQ(found, op->meterMap.end()); // should not find this

        found = op->meterMap.find(Util::makeUPPER("RailroadCrossing:InteriorLights:Electricity"));
        EXPECT_NE(found, op->meterMap.end());
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found->second]->resource);
        EXPECT_EQ((int)EndUseCat::InteriorLights, (int)op->meters[found->second]->endUseCat);
        EXPECT_EQ("RailroadCrossing", op->meters[found->second]->EndUseSub);

        // fuel oil CO2 emissions
        // testing a non-ABUPS end use with no sub end use specified
        found = op->meterMap.find(Util::makeUPPER("FuelOilNo2Emissions:CO2"));
        EXPECT_NE(found, op->meterMap.end());
        EXPECT_EQ((int)Constant::eResource::CO2, (int)op->meters[found->second]->resource);
        EXPECT_EQ((int)EndUseCat::FuelOilNo2Emissions, (int)op->meters[found->second]->endUseCat);
        EXPECT_EQ("", op->meters[found->second]->EndUseSub);
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupOutputVariable_star)
    {
        std::string const idf_objects = delimited_string({"Output:Variable,*,Boiler NaturalGas Rate,runperiod;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        GetReportVariableInput(*state);
        Real64 fuel_used = 999;
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler1");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler2");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler3");

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler NaturalGas Rate", "Run Period", "", "W"},
            {"2", "0", "Avg", "System", "HVAC System", "Boiler2", "Boiler NaturalGas Rate", "Run Period", "", "W"},
            {"3", "0", "Avg", "System", "HVAC System", "Boiler3", "Boiler NaturalGas Rate", "Run Period", "", "W"},
        });

        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        compare_eso_stream(
            delimited_string({"Program Version,",
                              "1,11,Boiler1,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                              "2,11,Boiler2,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                              "3,11,Boiler3,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"},
                             "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupOutputVariable_regex)
    {
        std::string const idf_objects = delimited_string({"Output:Variable,Boiler[13],Boiler NaturalGas Rate,runperiod;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        GetReportVariableInput(*state);
        Real64 fuel_used = 999;
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler1");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler2");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler3");

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler NaturalGas Rate", "Run Period", "", "W"},
            {"2", "0", "Avg", "System", "HVAC System", "Boiler3", "Boiler NaturalGas Rate", "Run Period", "", "W"},
        });

        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        compare_eso_stream(
            delimited_string({"Program Version,",
                              "1,11,Boiler1,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                              "2,11,Boiler3,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"},
                             "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupOutputVariable_regex_2)
    {
        std::string const idf_objects = delimited_string({"Output:Variable,Boiler.*,Boiler NaturalGas Rate,runperiod;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        GetReportVariableInput(*state);
        Real64 fuel_used = 999;
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler1");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler2");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler3");

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler NaturalGas Rate", "Run Period", "", "W"},
            {"2", "0", "Avg", "System", "HVAC System", "Boiler2", "Boiler NaturalGas Rate", "Run Period", "", "W"},
            {"3", "0", "Avg", "System", "HVAC System", "Boiler3", "Boiler NaturalGas Rate", "Run Period", "", "W"},
        });

        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        compare_eso_stream(
            delimited_string({"Program Version,",
                              "1,11,Boiler1,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                              "2,11,Boiler2,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                              "3,11,Boiler3,Boiler NaturalGas Rate [W] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"},
                             "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupOutputVariable_regex_3)
    {
        std::string const idf_objects = delimited_string({"Output:Variable,Zn003:Wall.*,AFN Linkage Node 1 to Node 2 Volume Flow Rate,timestep;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        GetReportVariableInput(*state);
        Real64 vol_flow = 999;
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "Zn003:Wall001");
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "Zn003:Wall002");
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "Zn003:Wall002:Win001");
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "Zn003:Wall003");

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "System", "HVAC System", "Zn003:Wall001", "AFN Linkage Node 1 to Node 2 Volume Flow Rate", "Zone Timestep", "", "m3/s"},
            {"2", "0", "Avg", "System", "HVAC System", "Zn003:Wall002", "AFN Linkage Node 1 to Node 2 Volume Flow Rate", "Zone Timestep", "", "m3/s"},
            {"3",
             "0",
             "Avg",
             "System",
             "HVAC System",
             "Zn003:Wall002:Win001",
             "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
             "Zone Timestep",
             "",
             "m3/s"},
            {"4", "0", "Avg", "System", "HVAC System", "Zn003:Wall003", "AFN Linkage Node 1 to Node 2 Volume Flow Rate", "Zone Timestep", "", "m3/s"},
        });

        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        compare_eso_stream(delimited_string(
            {
                "Program Version,",
                "1,1,Zn003:Wall001,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
                "2,1,Zn003:Wall002,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
                "3,1,Zn003:Wall002:Win001,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
                "4,1,Zn003:Wall003,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
            },
            "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_setupOutputVariable_regex_4)
    {
        // case-insensitive comparison
        std::string const idf_objects =
            delimited_string({"Output:Variable,(?i)Zn003:Wall.*,AFN Linkage Node 1 to Node 2 Volume Flow Rate,timestep;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        GetReportVariableInput(*state);
        Real64 vol_flow = 999;
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "ZN003:WALL001");
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "ZN003:WALL002");
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "ZN003:WALL002:WIN001");
        SetupOutputVariable(*state,
                            "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                            Constant::Units::m3_s,
                            vol_flow,
                            TimeStepType::System,
                            StoreType::Average,
                            "ZN003:WALL003");

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "System", "HVAC System", "ZN003:WALL001", "AFN Linkage Node 1 to Node 2 Volume Flow Rate", "Zone Timestep", "", "m3/s"},
            {"2", "0", "Avg", "System", "HVAC System", "ZN003:WALL002", "AFN Linkage Node 1 to Node 2 Volume Flow Rate", "Zone Timestep", "", "m3/s"},
            {"3",
             "0",
             "Avg",
             "System",
             "HVAC System",
             "ZN003:WALL002:WIN001",
             "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
             "Zone Timestep",
             "",
             "m3/s"},
            {"4", "0", "Avg", "System", "HVAC System", "ZN003:WALL003", "AFN Linkage Node 1 to Node 2 Volume Flow Rate", "Zone Timestep", "", "m3/s"},
        });

        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        compare_eso_stream(delimited_string(
            {
                "Program Version,",
                "1,1,ZN003:WALL001,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
                "2,1,ZN003:WALL002,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
                "3,1,ZN003:WALL002:WIN001,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
                "4,1,ZN003:WALL003,AFN Linkage Node 1 to Node 2 Volume Flow Rate [m3/s] !TimeStep",
            },
            "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_checkReportVariable)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        auto const keyed_value = "Environment";
        auto const var_name = "Site Outdoor Air Drybulb Temperature";

        InitializeOutput(*state);

        GetReportVariableInput(*state);

        std::vector<int> reqVarList;
        CheckReportVariable(*state, var_name, keyed_value, reqVarList);

        EXPECT_EQ(5, op->reqVars.size());

        EXPECT_EQ("", op->reqVars[0]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[0]->name);
        EXPECT_EQ((int)ReportFreq::TimeStep, (int)op->reqVars[0]->freq);
        EXPECT_EQ(nullptr, op->reqVars[0]->sched);
        EXPECT_EQ(true, op->reqVars[0]->Used);

        EXPECT_EQ("", op->reqVars[1]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[1]->name);
        EXPECT_EQ((int)ReportFreq::Hour, (int)op->reqVars[1]->freq);
        EXPECT_EQ(nullptr, op->reqVars[1]->sched);
        EXPECT_EQ(true, op->reqVars[1]->Used);

        EXPECT_EQ("", op->reqVars[2]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[2]->name);
        EXPECT_EQ((int)ReportFreq::Day, (int)op->reqVars[2]->freq);
        EXPECT_EQ(nullptr, op->reqVars[2]->sched);
        EXPECT_EQ(true, op->reqVars[2]->Used);

        EXPECT_EQ("", op->reqVars[3]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[3]->name);
        EXPECT_EQ((int)ReportFreq::Month, (int)op->reqVars[3]->freq);
        EXPECT_EQ(nullptr, op->reqVars[3]->sched);
        EXPECT_EQ(true, op->reqVars[3]->Used);

        EXPECT_EQ("", op->reqVars[4]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[4]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[4]->freq);
        EXPECT_EQ(nullptr, op->reqVars[4]->sched);
        EXPECT_EQ(true, op->reqVars[4]->Used);
    }

    TEST_F(SQLiteFixture, OutputProcessor_getMeters_WildCard)
    {
        // Test for #9150
        std::string const idf_objects = delimited_string({"Output:Meter:MeterFileOnly,InteriorLights:Electricity:Zone:*,Monthly;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        Real64 light_consumption = 0;
        for (int i = 1; i <= 5; ++i) {
            SetupOutputVariable(*state,
                                "Lights Electricity Energy",
                                Constant::Units::J,
                                light_consumption,
                                TimeStepType::Zone,
                                StoreType::Sum,
                                format("SPACE {} LIGHTS", i),
                                Constant::eResource::Electricity,
                                Group::Building,
                                EndUseCat::InteriorLights,
                                "GeneralLights",
                                "SPACE" + std::to_string(i),
                                1,
                                1);
        }

        UpdateMeterReporting(*state);

        compare_mtr_stream(
            delimited_string({"Program Version,",
                              "53,9,InteriorLights:Electricity:Zone:SPACE1 [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                              "102,9,InteriorLights:Electricity:Zone:SPACE2 [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                              "139,9,InteriorLights:Electricity:Zone:SPACE3 [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                              "176,9,InteriorLights:Electricity:Zone:SPACE4 [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                              "213,9,InteriorLights:Electricity:Zone:SPACE5 [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]"},
                             "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_getCustomMeterInput)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "  Output:Meter:MeterFileOnly,MyGeneralLights,monthly;",
            "  Output:Meter:MeterFileOnly,MyGeneralLights,runperiod;",
            "  Output:Meter:MeterFileOnly,MyBuildingOther,monthly;",
            "  Output:Meter:MeterFileOnly,MyBuildingOther,runperiod;",
            "  Output:Meter:MeterFileOnly,Building Infiltration Heat Loss,monthly;",
            "  Output:Meter:MeterFileOnly,Building Infiltration Heat Loss,runperiod;",
            "  Meter:Custom,",
            "    MyGeneralLights,         !- Name",
            "    Electricity,             !- Fuel Type",
            "    SPACE1-1 Lights 1,       !- Key Name 1",
            "    Lights Electricity Energy,  !- Output Variable or Meter Name 1",
            "    SPACE2-1 Lights 1,       !- Key Name 2",
            "    Lights Electricity Energy,  !- Output Variable or Meter Name 2",
            "    SPACE3-1 Lights 1,       !- Key Name 3",
            "    Lights Electricity Energy,  !- Output Variable or Meter Name 3",
            "    SPACE4-1 Lights 1,       !- Key Name 4",
            "    Lights Electricity Energy,  !- Output Variable or Meter Name 4",
            "    SPACE5-1 Lights 1,       !- Key Name 5",
            "    Lights Electricity Energy;  !- Output Variable or Meter Name 5",
            "  Meter:Custom,",
            "    Building Infiltration Heat Loss,  !- Name",
            "    Generic,                 !- Fuel Type",
            "    *,                       !- Key Name 1",
            "    Zone Infiltration Total Heat Loss Energy;  !- Output Variable or Meter Name 1",
            "  Meter:CustomDecrement,",
            "    MyBuildingOther,         !- Name",
            "    Electricity,             !- Fuel Type",
            "    Electricity:Building,    !- Source Meter Name",
            "    ,                        !- Key Name 1",
            "    MyGeneralLights;         !- Output Variable or Meter Name 1",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);
        Real64 light_consumption = 0;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE2-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE2-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE3-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE3-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE4-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE4-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE5-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE5-1",
                            1,
                            1);
        Real64 zone_infil_total_loss = 0;
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE1-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE2-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE3-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE4-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE5-1");

        bool errors_found = false;

        GetCustomMeterInput(*state, errors_found);

        ASSERT_FALSE(errors_found);

        ASSERT_EQ(22, op->meters.size());

        std::vector<std::tuple<MeterType, std::string_view, Constant::eResource, EndUseCat, std::string_view, Group, Constant::Units>> meter_result =
            {std::make_tuple(MeterType::Normal,
                             "Electricity:Facility",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Building",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Building,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Zone:SPACE1-1",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:Zone:SPACE1-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:Zone:SPACE1-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Zone:SPACE2-1",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:Zone:SPACE2-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:Zone:SPACE2-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Zone:SPACE3-1",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:Zone:SPACE3-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:Zone:SPACE3-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Zone:SPACE4-1",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:Zone:SPACE4-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:Zone:SPACE4-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Zone:SPACE5-1",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:Zone:SPACE5-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:Zone:SPACE5-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(
                 MeterType::Custom, "MYGENERALLIGHTS", Constant::eResource::Electricity, EndUseCat::Invalid, "", Group::Invalid, Constant::Units::J),
             std::make_tuple(MeterType::Custom,
                             "BUILDING INFILTRATION HEAT LOSS",
                             Constant::eResource::Generic,
                             EndUseCat::Invalid,
                             "",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::CustomDec,
                             "MYBUILDINGOTHER",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Invalid,
                             Constant::Units::J)};

        for (int i = 0; i < (int)meter_result.size(); ++i) {

            EXPECT_EQ((int)std::get<0>(meter_result[i]), (int)op->meters[i]->type);
            EXPECT_EQ(std::get<1>(meter_result[i]), op->meters[i]->Name);
            EXPECT_EQ((int)std::get<2>(meter_result[i]), (int)op->meters[i]->resource);
            EXPECT_EQ((int)std::get<3>(meter_result[i]), (int)op->meters[i]->endUseCat);
            EXPECT_EQ(std::get<4>(meter_result[i]), op->meters[i]->EndUseSub);
            EXPECT_EQ((int)std::get<5>(meter_result[i]), (int)op->meters[i]->group);
            EXPECT_EQ((int)std::get<6>(meter_result[i]), (int)op->meters[i]->units);
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_attachMeters)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Meter,Electricity:Facility,timestep;",
            "Output:Meter,Electricity:Facility,hourly;",
            "Output:Meter,Electricity:Facility,daily;",
            "Output:Meter,Electricity:Facility,monthly;",
            "Output:Meter,Electricity:Facility,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        InitializeOutput(*state);

        Constant::eResource resource = Constant::eResource::Electricity;
        EndUseCat endUseCat = EndUseCat::InteriorLights;
        std::string endUseSub("GeneralLights");
        Group group = Group::Building;
        std::string const zoneName("SPACE1-1");
        std::string const spaceType("OFFICE");

        AttachMeters(*state, Constant::Units::J, resource, endUseCat, endUseSub, group, zoneName, spaceType, -1);

        ASSERT_EQ(10, op->meters.size());

        std::vector<std::tuple<MeterType, std::string_view, Constant::eResource, EndUseCat, std::string_view, Group, Constant::Units>> meter_result =
            {std::make_tuple(MeterType::Normal,
                             "Electricity:Facility",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Building",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Building,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:Zone:SPACE1-1",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "Electricity:SpaceType:OFFICE",
                             Constant::eResource::Electricity,
                             EndUseCat::Invalid,
                             "",
                             Group::SpaceType,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:Zone:SPACE1-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "InteriorLights:Electricity:SpaceType:OFFICE",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "",
                             Group::SpaceType,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Invalid,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:Zone:SPACE1-1",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::Zone,
                             Constant::Units::J),
             std::make_tuple(MeterType::Normal,
                             "GeneralLights:InteriorLights:Electricity:SpaceType:OFFICE",
                             Constant::eResource::Electricity,
                             EndUseCat::InteriorLights,
                             "GeneralLights",
                             Group::SpaceType,
                             Constant::Units::J)};

        for (int i = 0; i < (int)meter_result.size(); ++i) {
            EXPECT_EQ((int)std::get<0>(meter_result[i]), (int)op->meters[i]->type);
            EXPECT_EQ(std::get<1>(meter_result[i]), op->meters[i]->Name);
            EXPECT_EQ((int)std::get<2>(meter_result[i]), (int)op->meters[i]->resource);
            EXPECT_EQ((int)std::get<3>(meter_result[i]), (int)op->meters[i]->endUseCat);
            EXPECT_EQ(std::get<4>(meter_result[i]), op->meters[i]->EndUseSub);
            EXPECT_EQ((int)std::get<5>(meter_result[i]), (int)op->meters[i]->group);
            EXPECT_EQ((int)std::get<6>(meter_result[i]), (int)op->meters[i]->units);
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_updateDataandReport_ZoneTSReporting)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
            "Output:Meter,Electricity:Facility,timestep;",
            "Output:Meter,Electricity:Facility,hourly;",
            "Output:Meter,Electricity:Facility,daily;",
            "Output:Meter,Electricity:Facility,monthly;",
            "Output:Meter,Electricity:Facility,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        state->dataGlobal->TimeStep = 4;
        state->dataGlobal->DayOfSim = 365;
        state->dataGlobal->DayOfSimChr = "365";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 31;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 3;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->HourOfDay = 24;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 10;

        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
            state->dataGlobal->EndHourFlag = true;
            if (state->dataGlobal->HourOfDay == 24) {
                state->dataGlobal->EndDayFlag = true;
                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                    state->dataGlobal->EndEnvrnFlag = true;
                }
            }
        }

        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
            state->dataEnvrn->EndMonthFlag = true;
        }

        // TimeValue.allocate(2);

        Real64 timeStep = 1.0 / 6;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 50;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 50;

        GetReportVariableInput(*state);
        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");
        Real64 light_consumption = 999;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE2-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE2-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE3-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE3-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE4-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE4-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE5-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE5-1",
                            1,
                            1);
        Real64 zone_infil_total_loss = 999;
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE1-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE2-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE3-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE4-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE5-1");

        UpdateMeterReporting(*state);

        UpdateDataandReport(*state, TimeStepType::Zone);

        auto timeResults = queryResult("SELECT * FROM Time;", "Time");

        std::vector<std::vector<std::string>> timeData({
            {"1", "0", "12", "31", "24", "0", "0", "10", "-1", "365", "Tuesday", "0", "0"},
            {"2", "0", "12", "31", "24", "0", "0", "60", "1", "365", "Tuesday", "0", "0"},
            {"3", "0", "12", "31", "24", "0", "0", "1440", "2", "365", "Tuesday", "0", "0"},
            {"4", "0", "12", "31", "24", "0", "", "44640", "3", "365", "", "0", "0"},
            {"5", "", "", "", "", "", "", "525600", "4", "365", "", "0", "0"},
        });

        for (int i = 0; i < (int)timeData.size(); ++i) {
            EXPECT_EQ(timeData[i], timeResults[i]);
        }

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Zone Timestep", "", "C"},
            {"2", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Hourly", "", "C"},
            {"3", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Daily", "", "C"},
            {"4", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Monthly", "", "C"},
            {"5", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Run Period", "", "C"},
            {"7", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Zone Timestep", "", "J"},
            {"8", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Hourly", "", "J"},
            {"9", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Daily", "", "J"},
            {"10", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Monthly", "", "J"},
            {"12", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Run Period", "", "J"},
        });

        for (int i = 0; i < (int)reportDataDictionary.size(); ++i) {
            EXPECT_EQ(reportDataDictionary[i], reportDataDictionaryResults[i]);
        }

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({
            {"1", "1", "1", "0.0"},
            {"2", "1", "7", "4995.0"},
            {"3", "2", "2", "0.0"},
            {"4", "2", "8", "4995.0"},
            {"5", "3", "3", "0.0"},
            {"6", "3", "9", "4995.0"},
            {"7", "4", "4", "0.0"},
            {"8", "4", "10", "4995.0"},
            {"9", "5", "5", "0.0"},
            {"10", "5", "12", "4995.0"},
        });

        std::vector<std::vector<std::string>> reportExtendedData(
            {{"1", "5", "0.0", "12", "31", "24", "", "0", "0.0", "12", "31", "24", "", "0"},
             {"2", "6", "4995.0", "12", "31", "24", "-9", "0", "4995.0", "12", "31", "24", "-9", "0"},
             {"3", "7", "0.0", "12", "31", "24", "", "0", "0.0", "12", "31", "24", "", "0"},
             {"4", "8", "4995.0", "12", "31", "24", "-9", "0", "4995.0", "12", "31", "24", "-9", "0"},
             {"5", "9", "0.0", "12", "31", "24", "", "0", "0.0", "12", "31", "24", "", "0"},
             {"6", "10", "4995.0", "12", "31", "24", "-9", "0", "4995.0", "12", "31", "24", "-9", "0"}});

        for (int i = 0; i < (int)reportData.size(); ++i) {
            EXPECT_EQ(reportData[i], reportDataResults[i]);
        }

        for (int i = 0; i < (int)reportExtendedData.size(); ++i) {
            EXPECT_EQ(reportExtendedData[i], reportExtendedDataResults[i]);
        }

        compare_eso_stream(delimited_string(
            {
                "Program Version,",
                "1,1,Environment,Site Outdoor Air Drybulb Temperature [C] !TimeStep",
                "2,1,Environment,Site Outdoor Air Drybulb Temperature [C] !Hourly",
                "3,7,Environment,Site Outdoor Air Drybulb Temperature [C] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "4,9,Environment,Site Outdoor Air Drybulb Temperature [C] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "5,11,Environment,Site Outdoor Air Drybulb Temperature [C] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                "7,1,Electricity:Facility [J] !TimeStep",
                "8,1,Electricity:Facility [J] !Hourly",
                "9,7,Electricity:Facility [J] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "10,9,Electricity:Facility [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "12,11,Electricity:Facility [J] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                ",365,12,31, 0,24,50.00,60.00,Tuesday",
                "1,0.0",
                "7,4995.0",
                ",365,12,31, 0,24, 0.00,60.00,Tuesday",
                "2,0.0",
                "8,4995.0",
                ",365,12,31, 0,Tuesday",
                "3,0.0,0.0,24,60,0.0,24,60",
                "9,4995.0,4995.0,24,60,4995.0,24,60",
                ",365,12",
                "4,0.0,0.0,31,24,60,0.0,31,24,60",
                "10,4995.0,4995.0,31,24,60,4995.0,31,24,60",
                ",365",
                "5,0.0,0.0,12,31,24,60,0.0,12,31,24,60",
                "12,4995.0,4995.0,12,31,24,60,4995.0,12,31,24,60",
            },
            "\n"));

        compare_mtr_stream(delimited_string(
            {
                "Program Version,",
                "7,1,Electricity:Facility [J] !TimeStep",
                "8,1,Electricity:Facility [J] !Hourly",
                "9,7,Electricity:Facility [J] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "10,9,Electricity:Facility [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "12,11,Electricity:Facility [J] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                ",365,12,31, 0,24,50.00,60.00,Tuesday",
                "7,4995.0",
                ",365,12,31, 0,24, 0.00,60.00,Tuesday",
                "8,4995.0",
                ",365,12,31, 0,Tuesday",
                "9,4995.0,4995.0,24,60,4995.0,24,60",
                ",365,12",
                "10,4995.0,4995.0,31,24,60,4995.0,31,24,60",
                ",365",
                "12,4995.0,4995.0,12,31,24,60,4995.0,12,31,24,60",
            },
            "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_updateDataandReport_ZoneTSReporting_with_detailed)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,detailed;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
            "Output:Variable,*,Boiler NaturalGas Rate,detailed;",
            "Output:Variable,*,Boiler Heating Rate,detailed;",
            "Output:Meter,Electricity:Facility,detailed;",
            "Output:Meter,Electricity:Facility,timestep;",
            "Output:Meter,Electricity:Facility,hourly;",
            "Output:Meter,Electricity:Facility,daily;",
            "Output:Meter,Electricity:Facility,monthly;",
            "Output:Meter,Electricity:Facility,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        state->dataGlobal->TimeStep = 4;
        state->dataGlobal->DayOfSim = 365;
        state->dataGlobal->DayOfSimChr = "365";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 31;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 3;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->HourOfDay = 24;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 10;

        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
            state->dataGlobal->EndHourFlag = true;
            if (state->dataGlobal->HourOfDay == 24) {
                state->dataGlobal->EndDayFlag = true;
                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                    state->dataGlobal->EndEnvrnFlag = true;
                }
            }
        }

        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
            state->dataEnvrn->EndMonthFlag = true;
        }

        // TimeValue.allocate(2);

        auto timeStep = 1.0 / 6;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 50;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 50;

        GetReportVariableInput(*state);
        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");
        Real64 light_consumption = 999;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE2-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE2-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE3-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE3-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE4-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE4-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE5-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE5-1",
                            1,
                            1);
        Real64 zone_infil_total_loss = 999;
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE1-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE2-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE3-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE4-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE5-1");
        Real64 fuel_used = 999;
        Real64 boiler_load = 999;
        SetupOutputVariable(*state, "Boiler Heating Rate", Constant::Units::W, boiler_load, TimeStepType::System, StoreType::Average, "Boiler1");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler1");

        UpdateMeterReporting(*state);

        UpdateDataandReport(*state, TimeStepType::Zone);

        auto timeResults = queryResult("SELECT * FROM Time;", "Time");

        std::vector<std::vector<std::string>> timeData({
            {"1", "0", "12", "31", "24", "0", "0", "10", "-1", "365", "Tuesday", "0", "0"},
            {"2", "0", "12", "31", "24", "0", "0", "60", "1", "365", "Tuesday", "0", "0"},
            {"3", "0", "12", "31", "24", "0", "0", "1440", "2", "365", "Tuesday", "0", "0"},
            {"4", "0", "12", "31", "24", "0", "", "44640", "3", "365", "", "0", "0"},
            {"5", "", "", "", "", "", "", "525600", "4", "365", "", "0", "0"},
        });

        EXPECT_EQ(timeData, timeResults);

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "HVAC System Timestep", "", "C"},
            {"2", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Zone Timestep", "", "C"},
            {"3", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Hourly", "", "C"},
            {"4", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Daily", "", "C"},
            {"5", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Monthly", "", "C"},
            {"6", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Run Period", "", "C"},
            {"8", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "HVAC System Timestep", "", "J"},
            {"9", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Hourly", "", "J"},
            {"10", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Daily", "", "J"},
            {"11", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Monthly", "", "J"},
            {"13", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Run Period", "", "J"},
            {"240", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler Heating Rate", "HVAC System Timestep", "", "W"},
            {"241", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler NaturalGas Rate", "HVAC System Timestep", "", "W"},
        });

        for (int i = 0; i < (int)reportDataDictionary.size(); ++i) {
            EXPECT_EQ(reportDataDictionary[i], reportDataDictionaryResults[i]);
        }

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({
            {"1", "1", "1", "0.0"},
            {"2", "1", "2", "0.0"},
            {"3", "1", "8", "4995.0"},
            {"4", "2", "3", "0.0"},
            {"5", "2", "9", "4995.0"},
            {"6", "3", "4", "0.0"},
            {"7", "3", "10", "4995.0"},
            {"8", "4", "5", "0.0"},
            {"9", "4", "11", "4995.0"},
            {"10", "5", "6", "0.0"},
            {"11", "5", "13", "4995.0"},
        });

        std::vector<std::vector<std::string>> reportExtendedData({
            {"1", "6", "0.0", "12", "31", "24", "", "0", "0.0", "12", "31", "24", "", "0"},
            {"2", "7", "4995.0", "12", "31", "24", "-9", "0", "4995.0", "12", "31", "24", "-9", "0"},
            {"3", "8", "0.0", "12", "31", "24", "", "0", "0.0", "12", "31", "24", "", "0"},
            {"4", "9", "4995.0", "12", "31", "24", "-9", "0", "4995.0", "12", "31", "24", "-9", "0"},
            {"5", "10", "0.0", "12", "31", "24", "", "0", "0.0", "12", "31", "24", "", "0"},
            {"6", "11", "4995.0", "12", "31", "24", "-9", "0", "4995.0", "12", "31", "24", "-9", "0"},
        });

        for (int i = 0; i < (int)reportData.size(); ++i) {
            EXPECT_EQ(reportData[i], reportDataResults[i]);
        }

        for (int i = 0; i < (int)reportExtendedData.size(); ++i) {
            EXPECT_EQ(reportExtendedData[i], reportExtendedDataResults[i]);
        }

        compare_eso_stream(delimited_string(
            {
                "Program Version,",
                "1,1,Environment,Site Outdoor Air Drybulb Temperature [C] !Each Call",
                "2,1,Environment,Site Outdoor Air Drybulb Temperature [C] !TimeStep",
                "3,1,Environment,Site Outdoor Air Drybulb Temperature [C] !Hourly",
                "4,7,Environment,Site Outdoor Air Drybulb Temperature [C] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "5,9,Environment,Site Outdoor Air Drybulb Temperature [C] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "6,11,Environment,Site Outdoor Air Drybulb Temperature [C] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                "240,1,Boiler1,Boiler Heating Rate [W] !Each Call",
                "241,1,Boiler1,Boiler NaturalGas Rate [W] !Each Call",
                "8,1,Electricity:Facility [J] !Each Call",
                "9,1,Electricity:Facility [J] !Hourly",
                "10,7,Electricity:Facility [J] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "11,9,Electricity:Facility [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "13,11,Electricity:Facility [J] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                ",365,12,31, 0,24,50.00,60.00,Tuesday",
                "1,0.0",
                "2,0.0",
                "8,4995.0",
                ",365,12,31, 0,24, 0.00,60.00,Tuesday",
                "3,0.0",
                "9,4995.0",
                ",365,12,31, 0,Tuesday",
                "4,0.0,0.0,24,60,0.0,24,60",
                "10,4995.0,4995.0,24,60,4995.0,24,60",
                ",365,12",
                "5,0.0,0.0,31,24,60,0.0,31,24,60",
                "11,4995.0,4995.0,31,24,60,4995.0,31,24,60",
                ",365",
                "6,0.0,0.0,12,31,24,60,0.0,12,31,24,60",
                "13,4995.0,4995.0,12,31,24,60,4995.0,12,31,24,60",
            },
            "\n"));

        compare_mtr_stream(delimited_string(
            {
                "Program Version,",
                "8,1,Electricity:Facility [J] !Each Call",
                "9,1,Electricity:Facility [J] !Hourly",
                "10,7,Electricity:Facility [J] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "11,9,Electricity:Facility [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "13,11,Electricity:Facility [J] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                ",365,12,31, 0,24,50.00,60.00,Tuesday",
                "8,4995.0",
                ",365,12,31, 0,24, 0.00,60.00,Tuesday",
                "9,4995.0",
                ",365,12,31, 0,Tuesday",
                "10,4995.0,4995.0,24,60,4995.0,24,60",
                ",365,12",
                "11,4995.0,4995.0,31,24,60,4995.0,31,24,60",
                ",365",
                "13,4995.0,4995.0,12,31,24,60,4995.0,12,31,24,60",
            },
            "\n"));
    }

    TEST_F(SQLiteFixture, OutputProcessor_updateDataandReport_HVACTSReporting)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,detailed;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,hourly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,daily;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,monthly;",
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
            "Output:Variable,*,Boiler NaturalGas Rate,detailed;",
            "Output:Variable,*,Boiler Heating Rate,detailed;",
            "Output:Meter,Electricity:Facility,detailed;",
            "Output:Meter,Electricity:Facility,timestep;",
            "Output:Meter,Electricity:Facility,hourly;",
            "Output:Meter,Electricity:Facility,daily;",
            "Output:Meter,Electricity:Facility,monthly;",
            "Output:Meter,Electricity:Facility,runperiod;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        state->dataGlobal->DayOfSim = 365;
        state->dataGlobal->DayOfSimChr = "365";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 31;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 3;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->HourOfDay = 24;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 10;

        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
            state->dataGlobal->EndHourFlag = true;
            if (state->dataGlobal->HourOfDay == 24) {
                state->dataGlobal->EndDayFlag = true;
                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                    state->dataGlobal->EndEnvrnFlag = true;
                }
            }
        }

        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
            state->dataEnvrn->EndMonthFlag = true;
        }

        // TimeValue.allocate(2);

        Real64 timeStep = 1.0 / 6;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 50;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 50;

        GetReportVariableInput(*state);
        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");
        Real64 light_consumption = 999;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE2-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE2-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE3-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE3-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE4-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE4-1",
                            1,
                            1);
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE5-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE5-1",
                            1,
                            1);
        Real64 zone_infil_total_loss = 999;
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE1-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE2-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE3-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE4-1");
        SetupOutputVariable(*state,
                            "Zone Infiltration Total Heat Loss Energy",
                            Constant::Units::J,
                            zone_infil_total_loss,
                            TimeStepType::System,
                            StoreType::Sum,
                            "SPACE5-1");
        Real64 fuel_used = 999;
        Real64 boiler_load = 999;
        SetupOutputVariable(*state, "Boiler Heating Rate", Constant::Units::W, boiler_load, TimeStepType::System, StoreType::Average, "Boiler1");
        SetupOutputVariable(*state, "Boiler NaturalGas Rate", Constant::Units::W, fuel_used, TimeStepType::System, StoreType::Average, "Boiler1");

        UpdateMeterReporting(*state);

        UpdateDataandReport(*state, TimeStepType::System);

        auto timeResults = queryResult("SELECT * FROM Time;", "Time");

        std::vector<std::vector<std::string>> timeData({
            {"1", "0", "12", "31", "24", "0", "0", "10", "-1", "365", "Tuesday", "0", "0"},
        });

        EXPECT_EQ(timeData, timeResults);

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        std::vector<std::vector<std::string>> reportDataDictionary({
            {"1", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "HVAC System Timestep", "", "C"},
            {"2", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Zone Timestep", "", "C"},
            {"3", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Hourly", "", "C"},
            {"4", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Daily", "", "C"},
            {"5", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Monthly", "", "C"},
            {"6", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Run Period", "", "C"},
            {"8", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "HVAC System Timestep", "", "J"},
            {"9", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Hourly", "", "J"},
            {"10", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Daily", "", "J"},
            {"11", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Monthly", "", "J"},
            {"13", "1", "Sum", "Facility:Electricity", "Zone", "", "Electricity:Facility", "Run Period", "", "J"},
            {"240", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler Heating Rate", "HVAC System Timestep", "", "W"},
            {"241", "0", "Avg", "System", "HVAC System", "Boiler1", "Boiler NaturalGas Rate", "HVAC System Timestep", "", "W"},
        });

        EXPECT_EQ(reportDataDictionary, reportDataDictionaryResults);

        auto reportDataResults = queryResult("SELECT * FROM ReportData;", "ReportData");
        auto reportExtendedDataResults = queryResult("SELECT * FROM ReportExtendedData;", "ReportExtendedData");

        std::vector<std::vector<std::string>> reportData({
            {"1", "1", "240", "999.0"},
            {"2", "1", "241", "999.0"},
        });

        std::vector<std::vector<std::string>> reportExtendedData({});

        EXPECT_EQ(reportData, reportDataResults);
        EXPECT_EQ(reportExtendedData, reportExtendedDataResults);

        compare_eso_stream(delimited_string(
            {
                "Program Version,",
                "1,1,Environment,Site Outdoor Air Drybulb Temperature [C] !Each Call",
                "2,1,Environment,Site Outdoor Air Drybulb Temperature [C] !TimeStep",
                "3,1,Environment,Site Outdoor Air Drybulb Temperature [C] !Hourly",
                "4,7,Environment,Site Outdoor Air Drybulb Temperature [C] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "5,9,Environment,Site Outdoor Air Drybulb Temperature [C] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "6,11,Environment,Site Outdoor Air Drybulb Temperature [C] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                "240,1,Boiler1,Boiler Heating Rate [W] !Each Call",
                "241,1,Boiler1,Boiler NaturalGas Rate [W] !Each Call",
                "8,1,Electricity:Facility [J] !Each Call",
                "9,1,Electricity:Facility [J] !Hourly",
                "10,7,Electricity:Facility [J] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "11,9,Electricity:Facility [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "13,11,Electricity:Facility [J] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
                ",365,12,31, 0,24,50.00,60.00,Tuesday",
                "240,999.0",
                "241,999.0",
            },
            "\n"));

        compare_mtr_stream(delimited_string(
            {
                "Program Version,",
                "8,1,Electricity:Facility [J] !Each Call",
                "9,1,Electricity:Facility [J] !Hourly",
                "10,7,Electricity:Facility [J] !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",
                "11,9,Electricity:Facility [J] !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",
                "13,11,Electricity:Facility [J] !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]",
            },
            "\n"));
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_UpdateMeters)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Meter,Electricity:Facility,timestep;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        state->dataGlobal->TimeStep = 4;
        state->dataGlobal->DayOfSim = 365;
        state->dataGlobal->DayOfSimChr = "365";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 31;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 3;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->HourOfDay = 24;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 10;

        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
            state->dataGlobal->EndHourFlag = true;
            if (state->dataGlobal->HourOfDay == 24) {
                state->dataGlobal->EndDayFlag = true;
                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                    state->dataGlobal->EndEnvrnFlag = true;
                }
            }
        }

        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
            state->dataEnvrn->EndMonthFlag = true;
        }

        // TimeValue.allocate(2);

        Real64 timeStep = 1.0 / 6;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 50;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 50;

        GetReportVariableInput(*state);
        Real64 light_consumption = 999;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        state->dataGlobal->WarmupFlag = true;
        UpdateMeterReporting(*state);
        UpdateDataandReport(*state, TimeStepType::Zone);

        compare_eso_stream(delimited_string(
            {
                "Program Version,",
                "2,1,Electricity:Facility [J] !TimeStep",
                ",365,12,31, 0,24,50.00,60.00,Tuesday",
                "2,0.0",
            },
            "\n"));

        state->dataGlobal->WarmupFlag = false;
        UpdateMeterReporting(*state);
        UpdateDataandReport(*state, TimeStepType::Zone);

        compare_eso_stream(delimited_string(
            {
                ",365,12,31, 0,24, 0.00,10.00,Tuesday",
                "2,999.0",
            },
            "\n"));
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_GenOutputVariablesAuditReport)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,timestep;",
            "Output:Variable,*,Boiler NaturalGas Rate,detailed;",
            "Output:Variable,*,Boiler Heating Rate,detailed;",
            "Output:Meter,Electricity:Facility,timestep;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        state->dataGlobal->DayOfSim = 365;
        state->dataGlobal->DayOfSimChr = "365";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 31;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 3;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->HourOfDay = 24;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 10;

        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
            state->dataGlobal->EndHourFlag = true;
            if (state->dataGlobal->HourOfDay == 24) {
                state->dataGlobal->EndDayFlag = true;
                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                    state->dataGlobal->EndEnvrnFlag = true;
                }
            }
        }

        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
            state->dataEnvrn->EndMonthFlag = true;
        }

        // TimeValue.allocate(2);

        Real64 timeStep = 1.0 / 6;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 50;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 50;

        GetReportVariableInput(*state);
        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");
        Real64 light_consumption = 999;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        UpdateMeterReporting(*state);
        UpdateDataandReport(*state, TimeStepType::Zone);

        GenOutputVariablesAuditReport(*state);

        std::string errMsg = delimited_string({
            "   ** Warning ** The following Report Variables were requested but not generated -- check.rdd file",
            "   **   ~~~   ** Either the IDF did not contain these elements, the variable name is misspelled,",
            "   **   ~~~   ** or the requested variable is an advanced output which requires Output : Diagnostics, DisplayAdvancedReportVariables;",
            "   ************* Key=*, VarName=BOILER NATURALGAS RATE, Frequency=Detailed",
            "   ************* Key=*, VarName=BOILER HEATING RATE, Frequency=Detailed",
        });

        compare_err_stream(errMsg);
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_fullOutputVariableKeyComparisonWithRegex)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Variable,(Air Loop 1|Air Supply) InletNode,System Node Setpoint Temperature,Hourly;",
            "Output:Variable,(Air Loop 1|Air Supply) InletNode,System Node Temperature,Hourly;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        state->dataGlobal->DayOfSim = 365;
        state->dataGlobal->DayOfSimChr = "365";
        state->dataEnvrn->Month = 12;
        state->dataEnvrn->DayOfMonth = 31;
        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->DayOfWeek = 3;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->HourOfDay = 24;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 10;

        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
            state->dataGlobal->EndHourFlag = true;
            if (state->dataGlobal->HourOfDay == 24) {
                state->dataGlobal->EndDayFlag = true;
                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                    state->dataGlobal->EndEnvrnFlag = true;
                }
            }
        }

        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
            state->dataEnvrn->EndMonthFlag = true;
        }

        // TimeValue.allocate(2);

        Real64 timeStep = 1.0 / 6;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 50;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 50;

        OutputReportTabular::GetInputTabularMonthly(*state);
        OutputReportTabular::InitializeTabularMonthly(*state);

        GetReportVariableInput(*state);

        EXPECT_EQ(2, op->reqVars.size());
        auto const *varSetpTempRegex = op->reqVars[0];
        EXPECT_EQ("(Air Loop 1|Air Supply) InletNode", varSetpTempRegex->key);
        EXPECT_EQ("SYSTEM NODE SETPOINT TEMPERATURE", varSetpTempRegex->name);
        EXPECT_FALSE(varSetpTempRegex->is_simple_string);
        EXPECT_NE(nullptr, varSetpTempRegex->case_insensitive_pattern);

        auto const *varTempRegex = op->reqVars[1];
        EXPECT_EQ("(Air Loop 1|Air Supply) InletNode", varTempRegex->key);
        EXPECT_EQ("SYSTEM NODE TEMPERATURE", varTempRegex->name);
        EXPECT_FALSE(varTempRegex->is_simple_string);
        EXPECT_NE(nullptr, varTempRegex->case_insensitive_pattern);

        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");
        Real64 light_consumption = 999;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "SPACE1-1 LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "GeneralLights",
                            "SPACE1-1",
                            1,
                            1);
        UpdateMeterReporting(*state);
        UpdateDataandReport(*state, TimeStepType::Zone);

        Real64 fakeVar = 0.0;
        SetupOutputVariable(*state,
                            "System Node Setpoint Temperature",
                            Constant::Units::C,
                            fakeVar,
                            TimeStepType::System,
                            StoreType::Average,
                            "Air Loop 1 InletNode");
        // TODO: is that supposed to look like a regex?!
        // Setup Output Variable is the only one that used to call BuildKeyVarList
        // So it should pass actual NodeID(NodeNum)
        // BuildKeyVarList(*state, "Air Loop 1|AirSupply InletNode", "SYSTEM NODE SETPOINT TEMPERATURE", 1, 2); // TODO: WHAT?
        EXPECT_TRUE(varSetpTempRegex->Used);
        EXPECT_FALSE(varTempRegex->Used);

        SetupOutputVariable(
            *state, "System Node Temperature", Constant::Units::C, fakeVar, TimeStepType::System, StoreType::Average, "Air Loop 1 InletNode");
        // TODO: is that supposed to look like a regex?!
        // BuildKeyVarList(*state, "Air Loop 1|AirSupply InletNode", "SYSTEM NODE TEMPERATURE", 1, 2);
        EXPECT_TRUE(varSetpTempRegex->Used);
        EXPECT_TRUE(varTempRegex->Used);

        GenOutputVariablesAuditReport(*state);

        compare_err_stream("");
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_MeterCustomSystemEnergy)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects =
            delimited_string({"Meter:Custom,",
                              "Meter Surface Average Face Conduction Heat Transfer Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Surface Average Face Conduction Heat Transfer Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Surface Window Heat Loss Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Surface Window Heat Loss Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Windows Total Heat Gain Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Windows Total Heat Gain Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Surface Window Heat Gain Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Surface Window Heat Gain Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Ventilation Heat Gain,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Ventilation Total Heat Gain Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Ventilation Heat Loss,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Ventilation Total Heat Loss Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Infiltration Heat Gain,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Infiltration Total Heat Gain Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Infiltration Heat Loss,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Infiltration Total Heat Loss Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Internal Loads Heating Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Electric Equipment Total Heating Energy,  !- Output Variable or Meter Name 1",
                              "* ,                       !- Key Name 2",
                              "Zone Lights Total Heating Energy,  !- Output Variable or Meter Name 2",
                              "* ,                       !- Key Name 3",
                              "People Total Heating Energy;  !- Output Variable or Meter Name 3",

                              "Meter:Custom,",
                              "Meter Zone Electric Equipment Total Heating Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Electric Equipment Total Heating Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Mechanical Ventilation XYZ,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Mechanical Ventilation Cooling Load Decrease Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Zone Mechanical Ventilation 123,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Zone Mechanical Ventilation No Load Heat Removal Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Air System Total Heating Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Air System Total Heating Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Air System Total Cooling Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Air System Total Cooling Energy;  !- Output Variable or Meter Name 1",

                              "Meter:Custom,",
                              "Meter Air System Hot Water Energy,  !- Name",
                              "Generic,                 !- Fuel Type",
                              "*,                       !- Key Name 1",
                              "Air System Hot Water Energy;  !- Output Variable or Meter Name 1",

                              "Output:Meter, Meter Surface Average Face Conduction Heat Transfer Energy, Timestep;",

                              "Output:Meter, Meter Surface Window Heat Loss Energy, Timestep;",

                              "Output:Meter, Meter Zone Windows Total Heat Gain Energy, Timestep;",

                              "Output:Meter, Meter Surface Window Heat Gain Energy, Timestep;",

                              "Output:Meter, Meter Zone Ventilation Heat Gain, Timestep;",

                              "Output:Meter, Meter Zone Ventilation Heat Loss, Timestep;",

                              "Output:Meter, Meter Zone Infiltration Heat Gain, Timestep;",

                              "Output:Meter, Meter Zone Infiltration Heat Loss, Timestep;",

                              "Output:Meter, Meter Internal Loads Heating Energy, Timestep;",

                              "Output:Meter, Meter Zone Electric Equipment Total Heating Energy, Timestep;",

                              "Output:Meter, Meter Zone Mechanical Ventilation XYZ, Timestep;",

                              "Output:Meter, Meter Zone Mechanical Ventilation 123, Timestep;",

                              "Output:Meter, Meter Air System Total Heating Energy, Timestep;",

                              "Output:Meter, Meter Air System Total Cooling Energy, Timestep;",

                              "Output:Meter, Meter Air System Hot Water Energy, Timestep;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);
        bool errors_found = false;
        Real64 transferredenergy = 0;
        state->dataGlobal->NumOfZones = 1;
        state->dataHVACGlobal->NumPrimaryAirSys = 1;
        state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
        state->dataAirSystemsData->PrimaryAirSystems(1).Name = "Air Loop 1";
        state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);
        state->dataZoneEquip->ZoneEquipConfig(state->dataGlobal->NumOfZones).IsControlled = true;
        SetupOutputVariable(*state,
                            "Surface Average Face Conduction Heat Transfer Energy",
                            Constant::Units::J,
                            transferredenergy,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "*");
        SetupOutputVariable(
            *state, "Surface Window Heat Loss Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Windows Total Heat Gain Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Surface Window Heat Gain Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Ventilation Total Heat Gain Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Ventilation Total Heat Loss Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Infiltration Total Heat Gain Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Infiltration Total Heat Loss Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Electric Equipment Total Heating Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(
            *state, "Zone Lights Total Heating Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SetupOutputVariable(*state, "People Total Heating Energy", Constant::Units::J, transferredenergy, TimeStepType::Zone, StoreType::Sum, "*");
        SystemReports::AllocateAndSetUpVentReports(*state);
        GetCustomMeterInput(*state, errors_found);
        EXPECT_FALSE(errors_found);
        EXPECT_EQ(15, op->meters.size());
        EXPECT_EQ(op->meters[0]->Name, "METER SURFACE AVERAGE FACE CONDUCTION HEAT TRANSFER ENERGY");
        EXPECT_EQ(op->meters[11]->Name, "METER ZONE MECHANICAL VENTILATION 123");
        EXPECT_EQ(op->meters[14]->Name, "METER AIR SYSTEM HOT WATER ENERGY");
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_DuplicateMeterCustom)
    {
        std::string const idf_objects = delimited_string({"Meter:Custom,",
                                                          "CustomMeter1,               !- Name",
                                                          "Generic,                    !- Fuel Type",
                                                          ",                           !- Key Name 1",
                                                          "DistrictHeatingWater:Facility;   !- Variable or Meter 1 Name",
                                                          "Meter:Custom,",
                                                          "CustomMeter2,               !- Name",
                                                          "Generic,                    !- Fuel Type",
                                                          ",                           !- Key Name 1",
                                                          "CustomMeter1;               !- Variable or Meter 1 Name",
                                                          "Output:Meter,CustomMeter1,Hourly;",
                                                          "Output:Meter,CustomMeter2,Hourly;"});

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        bool errors_found = false;

        GetCustomMeterInput(*state, errors_found);

        EXPECT_FALSE(errors_found);

        std::string errMsg = delimited_string(
            {"   ** Warning ** Meter:Custom=\"CUSTOMMETER1\", invalid Output Variable or Meter Name=\"DISTRICTHEATINGWATER:FACILITY\".",
             "   **   ~~~   ** ...will not be shown with the Meter results.",
             "   ** Warning ** Meter:Custom=\"CUSTOMMETER1\", no items assigned ",
             "   **   ~~~   ** ...will not be shown with the Meter results. This may be caused by a Meter:Custom be assigned to another "
             "Meter:Custom.",
             "   ** Warning ** Meter:Custom=\"CUSTOMMETER2\", contains a reference to another Meter:Custom in field: Output Variable or Meter "
             "Name=\"CUSTOMMETER1\".",
             "   ** Warning ** Meter:Custom=\"CUSTOMMETER2\", no items assigned ",
             "   **   ~~~   ** ...will not be shown with the Meter results. This may be caused by a Meter:Custom be assigned to another "
             "Meter:Custom."});
        compare_err_stream(errMsg);
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_unitStringToEnum)
    {

        EXPECT_EQ((int)Constant::Units::J, (int)getEnumValue(Constant::unitNamesUC, "J"));
        EXPECT_EQ((int)Constant::Units::J, (int)getEnumValue(Constant::unitNamesUC, Util::makeUPPER("j")));

        EXPECT_EQ((int)Constant::Units::kgWater_kgDryAir, (int)getEnumValue(Constant::unitNamesUC, Util::makeUPPER("kgWater/kgDryAir")));
        EXPECT_EQ((int)Constant::Units::kgWater_s, (int)getEnumValue(Constant::unitNamesUC, Util::makeUPPER("kgWater/s")));

        EXPECT_EQ((int)Constant::Units::Invalid, (int)getEnumValue(Constant::unitNamesUC, Util::makeUPPER("junk")));
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_unitEnumToString)
    {

        EXPECT_EQ("J", Constant::unitNames[(int)Constant::Units::J]);
        EXPECT_EQ("kgWater/kgDryAir", Constant::unitNames[(int)Constant::Units::kgWater_kgDryAir]);
        EXPECT_EQ("kgWater/s", Constant::unitNames[(int)Constant::Units::kgWater_s]);
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_unitStringFromDDitem)
    {

        AddDDOutVar(*state, "energy variable 1", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::J);
        AddDDOutVar(*state, "energy variable 2", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::J);
        AddDDOutVar(*state, "energy variable 3", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::J);

        AddDDOutVar(
            *state, "humidity ratio variable 1", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::kgWater_kgDryAir);
        AddDDOutVar(
            *state, "humidity ratio variable 2", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::kgWater_kgDryAir);

        AddDDOutVar(*state, "flow variable 1", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::kgWater_s);
        AddDDOutVar(*state, "flow variable 2", TimeStepType::Zone, StoreType::Average, VariableType::Integer, Constant::Units::kgWater_s);

        AddDDOutVar(*state,
                    "user defined EMS variable 1",
                    TimeStepType::Zone,
                    StoreType::Average,
                    VariableType::Integer,
                    Constant::Units::customEMS,
                    "ergs/century");
        AddDDOutVar(*state,
                    "user defined EMS variable 2",
                    TimeStepType::Zone,
                    StoreType::Average,
                    VariableType::Integer,
                    Constant::Units::customEMS,
                    "swamps/county");

        EXPECT_EQ(" [J]", unitStringFromDDitem(*state, 2));

        EXPECT_EQ(" [kgWater/kgDryAir]", unitStringFromDDitem(*state, 3));

        EXPECT_EQ(" [kgWater/s]", unitStringFromDDitem(*state, 5));

        EXPECT_EQ(" [ergs/century]", unitStringFromDDitem(*state, 7));

        EXPECT_EQ(" [swamps/county]", unitStringFromDDitem(*state, 8));
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_GetCustomMeterInput)
    {
        std::string const idf_objects = delimited_string({
            "Meter:Custom,",
            "FuelOilNo1CustomMeter,      !- Name",
            "FuelOilNo1,                 !- Fuel Type",
            ",                           !- Key Name 1",
            "FuelOilNo1:Facility;        !- Variable or Meter 1 Name",

            "Meter:Custom,",
            "FuelOilNo2CustomMeter,      !- Name",
            "FuelOilNo2,                 !- Fuel Type",
            ",                           !- Key Name 1",
            "FuelOilNo2:Facility;        !- Variable or Meter 1 Name",

            "Meter:Custom,",
            "NaturalGasCustomMeter,      !- Name",
            "NaturalGas,                 !- Fuel Type",
            ",                           !- Key Name 1",
            "NaturalGas:Facility;        !- Variable or Meter 1 Name",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        bool errors_found = false;

        GetCustomMeterInput(*state, errors_found);

        EXPECT_FALSE(errors_found);
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_FilePathInSetInitialMeterReportingAndOutputNames)
    {
        auto &op = state->dataOutputProcessor;
        Meter *meter = new Meter("Foo");
        op->meters.push_back(meter);
        meter->periods[(int)ReportFreq::TimeStep].Rpt = true;
        meter->periods[(int)ReportFreq::TimeStep].accRpt = true;

        // by default, the eso fs::path instance will just be a filename, so this will definitely pass

        // first call it as a non-cumulative variable
        SetInitialMeterReportingAndOutputNames(*state, 0, true, ReportFreq::EachCall, false);
        std::string errMsg = delimited_string({"   ** Warning ** Output:Meter:MeterFileOnly requested for \"Foo\" (TimeStep), already on "
                                               "\"Output:Meter\". Will report to both eplusout.eso and eplusout.mtr"});
        compare_err_stream(errMsg);

        // then with a cumulative variable
        SetInitialMeterReportingAndOutputNames(*state, 0, true, ReportFreq::EachCall, true);
        errMsg = delimited_string({"   ** Warning ** Output:Meter:MeterFileOnly requested for \"Cumulative Foo\" (TimeStep), already on "
                                   "\"Output:Meter\". Will report to both eplusout.eso and eplusout.mtr"});
        compare_err_stream(errMsg);

        // but then if you run EnergyPlus with `-d` to set an output directory, the variable will become a fully qualified path to the file
        // to test this we need to set up a proper path for the current platform, luckily this seems to suffic
        state->files.eso.filePath = fs::path("foo") / "bar.eso";
        state->files.mtr.filePath = fs::path("foo") / "bar.mtr";

        // first call it as a non-cumulative variable
        SetInitialMeterReportingAndOutputNames(*state, 0, true, ReportFreq::EachCall, false);
        errMsg = delimited_string({"   ** Warning ** Output:Meter:MeterFileOnly requested for \"Foo\" (TimeStep), already on \"Output:Meter\". Will "
                                   "report to both bar.eso and bar.mtr"});
        compare_err_stream(errMsg);

        // then with a cumulative variable
        SetInitialMeterReportingAndOutputNames(*state, 0, true, ReportFreq::EachCall, true);
        errMsg = delimited_string({"   ** Warning ** Output:Meter:MeterFileOnly requested for \"Cumulative Foo\" (TimeStep), already on "
                                   "\"Output:Meter\". Will report to both bar.eso and bar.mtr"});
        compare_err_stream(errMsg);
    }

    TEST_F(EnergyPlusFixture, DataOutputs_isKeyRegexLike)
    {
        std::vector<std::pair<std::string, bool>> test_cases{
            {"*", false},
            {"This is the first one", false},
            {"This is another.*one", true},
            {"This is (a|some) ones?", true},
            {"Zone 1.1", true},   // The `.` could mean any character (though in this case it's not meant as a regex really)
            {"Cafétéria", false}, // !std::isalnum, but not a regex
        };

        for (auto &[s, expectedIsRegexLike] : test_cases) {
            EXPECT_EQ(expectedIsRegexLike, DataOutputs::isKeyRegexLike(s)) << "isKeyRegexLike: Failed for " << s;
        }
    }

    TEST_F(SQLiteFixture, OutputProcessor_SetupOutputVariable_enum)
    {
        auto &op = state->dataOutputProcessor;
        // Test  calls for PR 10231 for extended tests on Calling  using new enum parameter drivers, upon different varieties such as:
        // 1).  calls for average, summed variables (regular and meters etc.)
        // 2).  handling on Output:variable processed results
        // 3). Wild cards
        // 4). Report frequencies;
        // 5). Emissions categories;
        std::string const idf_objects = delimited_string({
            "Output:Variable,*,Site Outdoor Air Drybulb Temperature,runperiod;",
            "Output:Variable,*,Chiller Electricity Energy,runperiod;",
            "Output:Variable,*,Lights Electricity Energy,runperiod;",
            "Output:Variable,*,Environmental Impact Fuel Oil No 2 CO2 Emissions Mass,runperiod;",
            "Output:Variable,*,Chiller Electricity Energy,hourly;",
            "Output:Variable,*,Lights Electricity Energy,timestep;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        GetReportVariableInput(*state);

        SetupOutputVariable(*state,
                            "Site Outdoor Air Drybulb Temperature",
                            Constant::Units::C,
                            state->dataEnvrn->OutDryBulbTemp,
                            TimeStepType::Zone,
                            StoreType::Average,
                            "Environment");

        Real64 cooling_consumption = 0.;
        SetupOutputVariable(*state,
                            "Chiller Electricity Energy",
                            Constant::Units::J,
                            cooling_consumption,
                            TimeStepType::System,
                            StoreType::Sum,
                            "Cool-1",
                            Constant::eResource::Electricity,
                            Group::Plant,
                            EndUseCat::Cooling);

        Real64 light_consumption = 0.;
        SetupOutputVariable(*state,
                            "Lights Electricity Energy",
                            Constant::Units::J,
                            light_consumption,
                            TimeStepType::Zone,
                            StoreType::Sum,
                            "LIGHTS 1",
                            Constant::eResource::Electricity,
                            Group::Building,
                            EndUseCat::InteriorLights,
                            "RailroadCrossing", // EndUseSubKey
                            "SPACE1-1",
                            1,
                            1);

        Real64 fuel_oil_co2 = 0.;
        SetupOutputVariable(*state,
                            "Environmental Impact Fuel Oil No 2 CO2 Emissions Mass",
                            Constant::Units::kg,
                            fuel_oil_co2,
                            TimeStepType::System,
                            StoreType::Sum,
                            "Site",
                            Constant::eResource::CO2,
                            Group::Invalid,
                            EndUseCat::FuelOilNo2Emissions);

        auto reportDataDictionaryResults = queryResult("SELECT * FROM ReportDataDictionary;", "ReportDataDictionary");

        // EXPECT_EQ(1, op->NumExtraVars);

        EXPECT_EQ("", op->reqVars[0]->key);
        EXPECT_EQ("SITE OUTDOOR AIR DRYBULB TEMPERATURE", op->reqVars[0]->name);
        EXPECT_EQ((int)ReportFreq::Simulation, (int)op->reqVars[0]->freq);
        EXPECT_EQ(nullptr, op->reqVars[0]->sched);
        EXPECT_EQ(true, op->reqVars[0]->Used);

        EXPECT_EQ((int)TimeStepType::Zone, (int)op->ddOutVars[0]->timeStepType);
        EXPECT_EQ((int)StoreType::Average, (int)op->ddOutVars[0]->storeType);
        EXPECT_EQ((int)VariableType::Real, (int)op->ddOutVars[0]->variableType);
        EXPECT_EQ(-1, op->ddOutVars[0]->Next);
        EXPECT_FALSE(op->ddOutVars[0]->ReportedOnDDFile);

        // Cooling
        // testing an ABUPS end use with no sub end use specified
        EXPECT_EQ(1, op->EndUseCategory(2).NumSubcategories);
        EXPECT_EQ("General", op->EndUseCategory(2).SubcategoryName(1));

        int found = GetMeterIndex(*state, "COOLING:ELECTRICITY");
        EXPECT_NE(-1, found);
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found]->resource);
        EXPECT_EQ((int)EndUseCat::Cooling, (int)op->meters[found]->endUseCat);
        EXPECT_EQ("", op->meters[found]->EndUseSub);

        found = GetMeterIndex(*state, "GENERAL:COOLING:ELECTRICITY");
        EXPECT_NE(-1, found);
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found]->resource);
        EXPECT_EQ((int)EndUseCat::Cooling, (int)op->meters[found]->endUseCat);
        EXPECT_EQ("General", op->meters[found]->EndUseSub);

        // lighting
        // testing an ABUPS end use with a sub end use specified
        EXPECT_EQ(1, op->EndUseCategory(3).NumSubcategories); // lighting end use
        EXPECT_EQ("RailroadCrossing", op->EndUseCategory(3).SubcategoryName(1));

        found = GetMeterIndex(*state, "INTERIORLIGHTS:ELECTRICITY");
        EXPECT_NE(-1, found);
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found]->resource);
        EXPECT_EQ((int)EndUseCat::InteriorLights, (int)op->meters[found]->endUseCat);
        EXPECT_EQ("", op->meters[found]->EndUseSub);

        found = GetMeterIndex(*state, "GENERAL:INTERIORLIGHTS:ELECTRICITY");
        EXPECT_EQ(-1, found); // should not find this

        found = GetMeterIndex(*state, "RAILROADCROSSING:INTERIORLIGHTS:ELECTRICITY");
        EXPECT_NE(-1, found);
        EXPECT_EQ((int)Constant::eResource::Electricity, (int)op->meters[found]->resource);
        EXPECT_EQ((int)EndUseCat::InteriorLights, (int)op->meters[found]->endUseCat);
        EXPECT_EQ("RailroadCrossing", op->meters[found]->EndUseSub);

        // fuel oil CO2 emissions
        // testing a non-ABUPS end use with no sub end use specified
        found = GetMeterIndex(*state, "FUELOILNO2EMISSIONS:CO2");
        EXPECT_NE(-1, found);
        EXPECT_EQ((int)Constant::eResource::CO2, (int)op->meters[found]->resource);
        EXPECT_EQ((int)EndUseCat::FuelOilNo2Emissions, (int)op->meters[found]->endUseCat);
        EXPECT_EQ("", op->meters[found]->EndUseSub);

        std::vector<std::vector<std::string>> reportDataDictionary(
            {{"1", "0", "Avg", "Zone", "Zone", "Environment", "Site Outdoor Air Drybulb Temperature", "Run Period", "", "C"},
             {"2", "0", "Sum", "System", "HVAC System", "Cool-1", "Chiller Electricity Energy", "Run Period", "", "J"},
             {"51", "0", "Sum", "System", "HVAC System", "Cool-1", "Chiller Electricity Energy", "Hourly", "", "J"},
             {"52", "0", "Sum", "Zone", "Zone", "LIGHTS 1", "Lights Electricity Energy", "Run Period", "", "J"},
             {"125", "0", "Sum", "Zone", "Zone", "LIGHTS 1", "Lights Electricity Energy", "Zone Timestep", "", "J"},
             {"126", "0", "Sum", "System", "HVAC System", "Site", "Environmental Impact Fuel Oil No 2 CO2 Emissions Mass", "Run Period", "", "kg"}});

        for (int i = 0; i < (int)reportDataDictionary.size(); ++i) {
            EXPECT_EQ(reportDataDictionary[i], reportDataDictionaryResults[i]);
        }
    }

    TEST_F(EnergyPlusFixture, OutputProcessor_StdoutRecordCount)
    {
        auto &op = state->dataOutputProcessor;
        std::string const idf_objects = delimited_string({
            "Output:Meter,Electricity:Facility,Timestep;",
            "Output:Variable,*,Lights Electricity Energy,Timestep;",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        // Demonstrate that it's not unreasonable to reach the INT_MAX limit at all
        constexpr int numOfTimeStepInHour = 60;
        constexpr int hoursInYear = 8760;
        constexpr int numOutputVariables = 4200;
        constexpr size_t totalRecords =
            static_cast<size_t>(numOfTimeStepInHour) * static_cast<size_t>(hoursInYear) * static_cast<size_t>(numOutputVariables);

        constexpr auto INT_MAX_AS_SIZE_T = static_cast<size_t>(INT_MAX);
        EXPECT_GT(totalRecords, INT_MAX_AS_SIZE_T);

        state->dataEnvrn->DSTIndicator = 0;
        state->dataEnvrn->HolidayIndex = 0;
        state->dataGlobal->NumOfDayInEnvrn = 365;
        state->dataGlobal->MinutesInTimeStep = 1;
        state->dataGlobal->TimeStepsInHour = 60;

        Real64 timeStep = 1.0 / state->dataGlobal->TimeStepsInHour;

        SetupTimePointers(*state, TimeStepType::Zone, timeStep);
        SetupTimePointers(*state, TimeStepType::System, timeStep);

        GetReportVariableInput(*state);
        Real64 light_consumption = 999;
        for (int i = 0; i < numOutputVariables; ++i) {
            SetupOutputVariable(*state,
                                "Lights Electricity Energy",
                                Constant::Units::J,
                                light_consumption,
                                TimeStepType::Zone,
                                StoreType::Sum,
                                fmt::format("LIGHTS {}", i + 1),
                                Constant::eResource::Electricity,
                                Group::Building,
                                EndUseCat::InteriorLights,
                                "GeneralLights",
                                "SPACE1-1",
                                1,
                                1);
        }
        state->dataGlobal->WarmupFlag = false;

        state->dataGlobal->DayOfSim = 0;
        state->dataEnvrn->DayOfWeek = 0;
        state->dataEnvrn->Year = 2005;
        state->dataGlobal->CalendarYear = 2005;

        // NOTE: the test takes way to long to run if do call UpdateMeterReporting and UpdateDataandReport
        // So I'm going to just scan for a timestamp were we're about to overflow, and fake it
        size_t records_written = 0;
        bool found_about_to_overflow = false;
        for (state->dataEnvrn->Month = 1; state->dataEnvrn->Month <= 12; ++state->dataEnvrn->Month) {
            state->dataEnvrn->EndMonthFlag = false;
            for (state->dataEnvrn->DayOfMonth = 1; state->dataEnvrn->DayOfMonth <= state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month);
                 ++state->dataEnvrn->DayOfMonth) {

                ++state->dataGlobal->DayOfSim;
                state->dataGlobal->DayOfSimChr = fmt::to_string(state->dataGlobal->DayOfSim);

                ++state->dataEnvrn->DayOfWeek;
                if (state->dataEnvrn->DayOfWeek > 7) {
                    state->dataEnvrn->DayOfWeek = 1;
                }

                state->dataGlobal->EndDayFlag = false;
                for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) {
                    for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour;
                         ++state->dataGlobal->TimeStep) {
                        if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
                            state->dataGlobal->EndHourFlag = true;
                            if (state->dataGlobal->HourOfDay == 24) {
                                state->dataGlobal->EndDayFlag = true;
                                if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                                    state->dataGlobal->EndEnvrnFlag = true;
                                }
                            }
                        }

                        if (state->dataEnvrn->DayOfMonth == state->dataWeather->EndDayOfMonth(state->dataEnvrn->Month)) {
                            state->dataEnvrn->EndMonthFlag = true;
                        }
                        records_written += numOutputVariables;
                        if (records_written > (INT_MAX_AS_SIZE_T - numOutputVariables)) {
                            EXPECT_EQ("2005-12-22 02:45",
                                      fmt::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}",
                                                  state->dataGlobal->CalendarYear,
                                                  state->dataEnvrn->Month,
                                                  state->dataEnvrn->DayOfMonth,
                                                  state->dataGlobal->HourOfDay,
                                                  state->dataGlobal->TimeStep));
                            UpdateMeterReporting(*state);
                            UpdateDataandReport(*state, TimeStepType::Zone);
                            found_about_to_overflow = true;
                            break;
                        }
                    }
                    if (found_about_to_overflow) {
                        break;
                    }
                }
                if (found_about_to_overflow) {
                    break;
                }
            }
            if (found_about_to_overflow) {
                break;
            }
        }
        ASSERT_TRUE(found_about_to_overflow);
        EXPECT_EQ(numOutputVariables + 1, state->dataGlobal->StdOutputRecordCount);
        EXPECT_EQ(1, state->dataGlobal->StdMeterRecordCount);
        EXPECT_TRUE(has_eso_output(true));
        state->dataGlobal->StdOutputRecordCount = records_written;
        state->dataGlobal->StdMeterRecordCount = INT_MAX;

        ++state->dataGlobal->TimeStep;

        UpdateMeterReporting(*state);
        UpdateDataandReport(*state, TimeStepType::Zone);
        EXPECT_GT(state->dataGlobal->StdOutputRecordCount, 0);
        EXPECT_GT(state->dataGlobal->StdMeterRecordCount, 0);
    }
} // namespace OutputProcessor

} // namespace EnergyPlus
