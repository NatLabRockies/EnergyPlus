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

// EnergyPlus::DataPlant Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/SizingAnalysisObjects.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

using namespace EnergyPlus;
using namespace OutputProcessor;
using namespace DataPlant;
using namespace DataSizing;
using namespace OutputReportPredefined;

class SizingAnalysisObjectsTest : public EnergyPlusFixture
{

protected:
    void SetUp() override
    {
        EnergyPlus::EnergyPlusFixture::SetUp();

        state->files.eio.open_as_stringstream();
        // fill in test log data values
        lowLogVal = 50.0;
        midLogVal = 75.0;
        hiLogVal = 100.0;

        state->dataGlobal->TimeStepsInHour = 4; // in DataGlobals
        state->dataGlobal->TimeStepZone = 0.25;

        // setup weather manager state needed
        state->dataWeather->NumOfEnvrn = 2;
        state->dataWeather->Environment.allocate(state->dataWeather->NumOfEnvrn);
        state->dataWeather->Environment(1).KindOfEnvrn = Constant::KindOfSim::DesignDay;
        state->dataWeather->Environment(1).DesignDayNum = 1;

        state->dataWeather->Environment(2).KindOfEnvrn = Constant::KindOfSim::DesignDay;
        state->dataWeather->Environment(2).DesignDayNum = 2;

        averagingWindow = 1;
        logIndex = sizingLoggerFrameObj.SetupVariableSizingLog(*state, LogVal, averagingWindow);

        state->dataWeather->NumOfEnvrn = 4;
        state->dataWeather->Environment.redimension(state->dataWeather->NumOfEnvrn);

        state->dataWeather->Environment(3).KindOfEnvrn = Constant::KindOfSim::HVACSizeDesignDay;
        state->dataWeather->Environment(3).DesignDayNum = 1;
        state->dataWeather->Environment(3).SeedEnvrnNum = 1;

        state->dataWeather->Environment(4).KindOfEnvrn = Constant::KindOfSim::HVACSizeDesignDay;
        state->dataWeather->Environment(4).DesignDayNum = 2;
        state->dataWeather->Environment(4).SeedEnvrnNum = 2;

        OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone);
        OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);

        state->dataSize->PlantSizData.allocate(1);

        state->dataSize->PlantSizData(1).SizingFactorOption = NoSizingFactorMode;
        state->dataSize->PlantSizData(1).DesVolFlowRate = 0.002;
        state->dataSize->PlantSizData(1).DeltaT = 10;

        state->dataPlnt->TotNumLoops = 1;
        state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
        state->dataPlnt->PlantLoop(1).Name = "Test Plant Loop 1";
        state->dataPlnt->PlantLoop(1).MaxVolFlowRateWasAutoSized = true;
        state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 0.002;
        state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 2.0;
        state->dataPlnt->PlantLoop(1).VolumeWasAutoSized = true;

        SetPredefinedTables(*state);
    }

    void TearDown() override
    {
        EnergyPlus::EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    };

public:
    Real64 lowLogVal;
    Real64 midLogVal;
    Real64 hiLogVal;
    Real64 LogVal; // actual variable pointed to
    int averagingWindow;
    int logIndex;
    SizingLoggerFramework sizingLoggerFrameObj;
};

TEST_F(SizingAnalysisObjectsTest, testZoneUpdateInLoggerFramework)
{
    ShowMessage(*state, "Begin Test: SizingAnalysisObjectsTest, testZoneUpdateInLoggerFramework");

    // first step
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->HourOfDay = 1;
    state->dataWeather->Envrn = 3;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 1;
    sizingLoggerFrameObj.SetupSizingLogsNewEnvironment(*state);
    state->dataGlobal->TimeStep = 1;

    LogVal = lowLogVal;
    sizingLoggerFrameObj.UpdateSizingLogValuesZoneStep(*state);

    EXPECT_DOUBLE_EQ(lowLogVal, sizingLoggerFrameObj.logObjs[logIndex].ztStepObj[0].logDataValue);

    // last step of first design day
    state->dataGlobal->HourOfDay = 24;
    state->dataGlobal->TimeStep = 4;
    LogVal = hiLogVal;
    sizingLoggerFrameObj.UpdateSizingLogValuesZoneStep(*state);

    EXPECT_DOUBLE_EQ(hiLogVal, sizingLoggerFrameObj.logObjs[logIndex].ztStepObj[95].logDataValue);

    // first step of second design day
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataWeather->Envrn = 4;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 2;
    sizingLoggerFrameObj.SetupSizingLogsNewEnvironment(*state);
    LogVal = midLogVal;
    sizingLoggerFrameObj.UpdateSizingLogValuesZoneStep(*state);

    EXPECT_DOUBLE_EQ(midLogVal, sizingLoggerFrameObj.logObjs[logIndex].ztStepObj[96].logDataValue);
}

TEST_F(SizingAnalysisObjectsTest, BasicLogging4stepsPerHour)
{
    ShowMessage(*state, "Begin Test: SizingAnalysisObjectsTest, BasicLogging4stepsPerHour");

    // basic test of method FillZoneStep and zone time stamp constructor
    // setup a log for 4 timesteps per hour and fill the first 4 steps, then check that values are there
    SizingLog TestLogObj(LogVal);

    TestLogObj.NumOfEnvironmentsInLogSet = 2;
    TestLogObj.NumOfDesignDaysInLogSet = 2;
    TestLogObj.NumberOfSizingPeriodsInLogSet = 0;

    TestLogObj.NumOfStepsInLogSet = 8; // test

    TestLogObj.ztStepCountByEnvrnMap[1] = 4;
    TestLogObj.ztStepCountByEnvrnMap[2] = 4;

    // as for 2 DDs and a run period
    TestLogObj.envrnStartZtStepIndexMap[1] = 0;
    TestLogObj.envrnStartZtStepIndexMap[2] = 4;

    TestLogObj.newEnvrnToSeedEnvrnMap[3] = 1;
    TestLogObj.newEnvrnToSeedEnvrnMap[4] = 2;

    TestLogObj.ztStepObj.resize(TestLogObj.NumOfStepsInLogSet);

    // fill first step in log with zone step data
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    int Envrn(3);
    state->dataGlobal->DayOfSim = 1;
    int HourofDay(1);
    int timeStp(1);
    Real64 timeStepDuration(0.25);
    int numTimeStepsInHour(4);
    LogVal = lowLogVal;
    ZoneTimestepObject tmpztStepStamp1( // call constructor
        state->dataGlobal->KindOfSim,
        Envrn,
        state->dataGlobal->DayOfSim,
        HourofDay,
        timeStp,
        timeStepDuration,
        numTimeStepsInHour);
    TestLogObj.FillZoneStep(tmpztStepStamp1);

    // fill second step log with zone step data

    timeStp = 2;
    LogVal = midLogVal;
    ZoneTimestepObject tmpztStepStamp2( // call constructor
        state->dataGlobal->KindOfSim,
        Envrn,
        state->dataGlobal->DayOfSim,
        HourofDay,
        timeStp,
        timeStepDuration,
        numTimeStepsInHour);
    TestLogObj.FillZoneStep(tmpztStepStamp2);

    // fill third step log with zone step data
    timeStp = 3;
    LogVal = midLogVal;
    ZoneTimestepObject tmpztStepStamp3( // call constructor
        state->dataGlobal->KindOfSim,
        Envrn,
        state->dataGlobal->DayOfSim,
        HourofDay,
        timeStp,
        timeStepDuration,
        numTimeStepsInHour);
    TestLogObj.FillZoneStep(tmpztStepStamp3);

    // fill fourth step log with zone step data
    timeStp = 4;
    LogVal = hiLogVal;
    ZoneTimestepObject tmpztStepStamp4( // call constructor
        state->dataGlobal->KindOfSim,
        Envrn,
        state->dataGlobal->DayOfSim,
        HourofDay,
        timeStp,
        timeStepDuration,
        numTimeStepsInHour);
    TestLogObj.FillZoneStep(tmpztStepStamp4);

    // now check that the correct values were stored in the right spot
    EXPECT_DOUBLE_EQ(lowLogVal, TestLogObj.ztStepObj[0].logDataValue);
    EXPECT_DOUBLE_EQ(midLogVal, TestLogObj.ztStepObj[2].logDataValue);
    EXPECT_NE(lowLogVal, TestLogObj.ztStepObj[1].logDataValue);

    // store this in the logger framework
    sizingLoggerFrameObj.logObjs.push_back(TestLogObj);
}

TEST_F(SizingAnalysisObjectsTest, LoggingDDWrap1stepPerHour)
{
    ShowMessage(*state, "Begin Test: SizingAnalysisObjectsTest, LoggingDDWrap1stepPerHour");

    // this test uses one timestep per hour and checks as for two design days

    SizingLog TestLogObj(LogVal);

    TestLogObj.NumOfEnvironmentsInLogSet = 2;
    TestLogObj.NumOfDesignDaysInLogSet = 2;
    TestLogObj.NumberOfSizingPeriodsInLogSet = 0;

    TestLogObj.NumOfStepsInLogSet = 48; // test
    TestLogObj.ztStepCountByEnvrnMap[1] = 24;
    TestLogObj.ztStepCountByEnvrnMap[2] = 24;

    TestLogObj.envrnStartZtStepIndexMap[1] = 0;
    TestLogObj.envrnStartZtStepIndexMap[2] = 24;

    TestLogObj.newEnvrnToSeedEnvrnMap[3] = 1;
    TestLogObj.newEnvrnToSeedEnvrnMap[4] = 2;

    TestLogObj.ztStepObj.resize(TestLogObj.NumOfStepsInLogSet);

    // fill first step in log with zone step data
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    int Envrn(3);
    state->dataGlobal->DayOfSim = 1;
    int HourofDay(1);
    int timeStp(1);
    Real64 timeStepDuration(1.0);
    int numTimeStepsInHour(1);

    LogVal = lowLogVal;
    for (int hr = 1; hr <= 24; ++hr) {
        HourofDay = hr;
        ZoneTimestepObject tmpztStepStamp1( // call constructor
            state->dataGlobal->KindOfSim,
            Envrn,
            state->dataGlobal->DayOfSim,
            HourofDay,
            timeStp,
            timeStepDuration,
            numTimeStepsInHour);
        TestLogObj.FillZoneStep(tmpztStepStamp1);
    }

    Envrn = 4;
    LogVal = hiLogVal;
    for (int hr = 1; hr <= 24; ++hr) {
        HourofDay = hr;
        ZoneTimestepObject tmpztStepStamp1( // call constructor
            state->dataGlobal->KindOfSim,
            Envrn,
            state->dataGlobal->DayOfSim,
            HourofDay,
            timeStp,
            timeStepDuration,
            numTimeStepsInHour);
        TestLogObj.FillZoneStep(tmpztStepStamp1);
    }

    // check values at wrap of environment change over
    EXPECT_DOUBLE_EQ(lowLogVal, TestLogObj.ztStepObj[23].logDataValue);
    EXPECT_DOUBLE_EQ(hiLogVal, TestLogObj.ztStepObj[24].logDataValue);

    // store this in the logger framework
    sizingLoggerFrameObj.logObjs.push_back(TestLogObj);
}

TEST_F(SizingAnalysisObjectsTest, PlantCoincidentAnalyObjTest)
{
    ShowMessage(*state, "Begin Test: SizingAnalysisObjectsTest, PlantCoincidentAnalyObjTest");

    std::string loopName;
    int loopNum;
    int nodeNum;
    Real64 density;
    Real64 cp;
    int timestepsInAvg;
    int plantSizingIndex;

    loopName = "Test Plant Loop 1";
    loopNum = 1;
    nodeNum = 1;
    density = 1000;
    cp = 1.0;
    timestepsInAvg = 1;
    plantSizingIndex = 1;

    PlantCoinicidentAnalysis TestAnalysisObj(loopName, loopNum, nodeNum, density, cp, timestepsInAvg, plantSizingIndex);

    // fill first step in log with zone step data
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    int Envrn(4);
    state->dataGlobal->DayOfSim = 1;
    int HourofDay(1);
    int timeStp(1);
    Real64 timeStepDuration(0.25);
    int numTimeStepsInHour(4);

    ZoneTimestepObject tmpztStepStamp1( // call constructor
        state->dataGlobal->KindOfSim,
        Envrn,
        state->dataGlobal->DayOfSim,
        HourofDay,
        timeStp,
        timeStepDuration,
        numTimeStepsInHour);
    LogVal = 1.5; // kg/s
    tmpztStepStamp1.runningAvgDataValue = 1.5;
    sizingLoggerFrameObj.logObjs[logIndex].FillZoneStep(tmpztStepStamp1);

    TestAnalysisObj.newFoundMassFlowRateTimeStamp = tmpztStepStamp1;
    TestAnalysisObj.peakMdotCoincidentDemand = 1000.0;
    TestAnalysisObj.peakMdotCoincidentReturnTemp = 10.0;
    TestAnalysisObj.NewFoundMaxDemandTimeStamp = tmpztStepStamp1;
    TestAnalysisObj.peakDemandMassFlow = 1.5;
    TestAnalysisObj.peakDemandReturnTemp = 10.0;

    EXPECT_DOUBLE_EQ(0.002, state->dataPlnt->PlantLoop(1).MaxVolFlowRate); //  m3/s

    TestAnalysisObj.ResolveDesignFlowRate(*state, 1);

    EXPECT_DOUBLE_EQ(0.0015, state->dataPlnt->PlantLoop(1).MaxVolFlowRate); //  m3/s
    EXPECT_DOUBLE_EQ(1.5, state->dataPlnt->PlantLoop(1).MaxMassFlowRate);   //  m3/s
    EXPECT_TRUE(TestAnalysisObj.anotherIterationDesired);
}

TEST_F(SizingAnalysisObjectsTest, LoggingSubStep4stepPerHour)
{
    ShowMessage(*state, "Begin Test: SizingAnalysisObjectsTest, LoggingSubStep4stepPerHour");

    // this test uses 4 zone timesteps per hour and 5 sub system time steps per zone timestep
    // tests FillSysStep over two design days

    SizingLog TestLogObj(LogVal);

    TestLogObj.NumOfEnvironmentsInLogSet = 2;
    TestLogObj.NumOfDesignDaysInLogSet = 2;
    TestLogObj.NumberOfSizingPeriodsInLogSet = 0;

    TestLogObj.NumOfStepsInLogSet = 24 * 2 * 4;
    TestLogObj.ztStepCountByEnvrnMap[1] = 96;
    TestLogObj.ztStepCountByEnvrnMap[2] = 96;

    TestLogObj.envrnStartZtStepIndexMap[1] = 0;
    TestLogObj.envrnStartZtStepIndexMap[2] = 96;

    TestLogObj.newEnvrnToSeedEnvrnMap[3] = 1;
    TestLogObj.newEnvrnToSeedEnvrnMap[4] = 2;

    TestLogObj.ztStepObj.resize(TestLogObj.NumOfStepsInLogSet);

    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    int Envrn(3);
    state->dataGlobal->DayOfSim = 1;
    int HourofDay(0);
    state->dataHVACGlobal->TimeStepSys = 1.0 / (4.0 * 5.0); // fractional hours, duration
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    Real64 zoneTimeStepDuration(0.25);
    int numTimeStepsInHour(4);

    LogVal = lowLogVal;
    for (int hr = 1; hr <= 24; ++hr) {
        HourofDay = hr;
        for (int timeStp = 1; timeStp <= 4; ++timeStp) {              // 15 minute zone timestep
            for (int subTimeStp = 1; subTimeStp <= 5; ++subTimeStp) { // 5 system substeps, so 3 minute system timestep
                Real64 constexpr minutesPerHour(60.0);
                ZoneTimestepObject tmpztStepStamp(state->dataGlobal->KindOfSim,
                                                  Envrn,
                                                  state->dataGlobal->DayOfSim,
                                                  HourofDay,
                                                  timeStp,
                                                  zoneTimeStepDuration,
                                                  numTimeStepsInHour); // call constructor
                SystemTimestepObject tmpSysStepStamp;
                tmpSysStepStamp.CurMinuteEnd =
                    (timeStp - 1) * (minutesPerHour * zoneTimeStepDuration) +
                    (subTimeStp) * (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * minutesPerHour;
                if (tmpSysStepStamp.CurMinuteEnd == 0.0) {
                    tmpSysStepStamp.CurMinuteEnd = minutesPerHour;
                }
                tmpSysStepStamp.CurMinuteStart =
                    tmpSysStepStamp.CurMinuteEnd -
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * minutesPerHour;
                tmpSysStepStamp.TimeStepDuration = *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep;
                TestLogObj.FillSysStep(tmpztStepStamp, tmpSysStepStamp);
            }

            ZoneTimestepObject tmpztStepStamp1(state->dataGlobal->KindOfSim,
                                               Envrn,
                                               state->dataGlobal->DayOfSim,
                                               HourofDay,
                                               timeStp,
                                               zoneTimeStepDuration,
                                               numTimeStepsInHour); // call constructor
            TestLogObj.FillZoneStep(tmpztStepStamp1);
        }
    }

    Envrn = 4;
    LogVal = hiLogVal;
    for (int hr = 1; hr <= 24; ++hr) {
        HourofDay = hr;
        for (int timeStp = 1; timeStp <= 4; ++timeStp) {              // 15 minute zone timestep
            for (int subTimeStp = 1; subTimeStp <= 5; ++subTimeStp) { // 5 system substeps, so 3 minute system timestep
                Real64 constexpr minutesPerHour(60.0);
                ZoneTimestepObject tmpztStepStamp(state->dataGlobal->KindOfSim,
                                                  Envrn,
                                                  state->dataGlobal->DayOfSim,
                                                  HourofDay,
                                                  timeStp,
                                                  zoneTimeStepDuration,
                                                  numTimeStepsInHour); // call constructor
                SystemTimestepObject tmpSysStepStamp;
                tmpSysStepStamp.CurMinuteEnd =
                    (timeStp - 1) * (minutesPerHour * zoneTimeStepDuration) +
                    (subTimeStp) * (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * minutesPerHour;
                if (tmpSysStepStamp.CurMinuteEnd == 0.0) {
                    tmpSysStepStamp.CurMinuteEnd = minutesPerHour;
                }
                tmpSysStepStamp.CurMinuteStart =
                    tmpSysStepStamp.CurMinuteEnd -
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * minutesPerHour;
                tmpSysStepStamp.TimeStepDuration = *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep;
                TestLogObj.FillSysStep(tmpztStepStamp, tmpSysStepStamp);
            }

            ZoneTimestepObject tmpztStepStamp1(state->dataGlobal->KindOfSim,
                                               Envrn,
                                               state->dataGlobal->DayOfSim,
                                               HourofDay,
                                               timeStp,
                                               zoneTimeStepDuration,
                                               numTimeStepsInHour); // call constructor
            TestLogObj.FillZoneStep(tmpztStepStamp1);
        }
    }

    // check values at wrap of environment change over, lower value up until the end of the first and then higher value at the new day

    // these should be from the FillZoneStep at this point
    EXPECT_DOUBLE_EQ(lowLogVal, TestLogObj.ztStepObj[95].logDataValue);
    EXPECT_DOUBLE_EQ(hiLogVal, TestLogObj.ztStepObj[96].logDataValue);

    TestLogObj.AverageSysTimeSteps();
    TestLogObj.ProcessRunningAverage();

    // now these should be filled from the sub timesteps, and still have the same data.
    EXPECT_DOUBLE_EQ(lowLogVal, TestLogObj.ztStepObj[95].logDataValue);
    EXPECT_DOUBLE_EQ(hiLogVal, TestLogObj.ztStepObj[96].logDataValue);

    // dig into data structure and check substeps have the correct value
    EXPECT_DOUBLE_EQ(lowLogVal, TestLogObj.ztStepObj[95].subSteps[4].LogDataValue);
    EXPECT_DOUBLE_EQ(hiLogVal, TestLogObj.ztStepObj[96].subSteps[0].LogDataValue);
}

TEST_F(SizingAnalysisObjectsTest, PlantCoincidentAnalyObjTestNullMassFlowRateTimestamp)
{
    // similar to PlantCoincidentAnalyObjTest but exercise logic problem resolved as issue #5665
    std::string loopName;
    int loopNum;
    int nodeNum;
    Real64 density;
    Real64 cp;
    int timestepsInAvg;
    int plantSizingIndex;

    loopName = "Test Plant Loop 1";
    loopNum = 1;
    nodeNum = 1;
    density = 1000;
    cp = 1.0;
    timestepsInAvg = 1;
    plantSizingIndex = 1;

    PlantCoinicidentAnalysis TestAnalysisObj(loopName, loopNum, nodeNum, density, cp, timestepsInAvg, plantSizingIndex);

    // fill first step in log with zone step data
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    int Envrn(4);
    state->dataGlobal->DayOfSim = 1;
    int HourofDay(1);
    int timeStp(1);
    Real64 timeStepDuration(0.25);
    int numTimeStepsInHour(4);

    ZoneTimestepObject tmpztStepStamp1( // call full constructor
        state->dataGlobal->KindOfSim,
        Envrn,
        state->dataGlobal->DayOfSim,
        HourofDay,
        timeStp,
        timeStepDuration,
        numTimeStepsInHour);
    LogVal = 1.5; // kg/s
    tmpztStepStamp1.runningAvgDataValue = 1.5;
    sizingLoggerFrameObj.logObjs[logIndex].FillZoneStep(tmpztStepStamp1);

    ZoneTimestepObject tmpNullztStep2; // call default constructor

    TestAnalysisObj.newFoundMassFlowRateTimeStamp = tmpNullztStep2; // use null timestamp and check to logic works with a valid max demand timestamp
    TestAnalysisObj.peakMdotCoincidentDemand = 1000.0;
    TestAnalysisObj.peakMdotCoincidentReturnTemp = 10.0;
    TestAnalysisObj.NewFoundMaxDemandTimeStamp = tmpztStepStamp1;
    TestAnalysisObj.peakDemandMassFlow = 1.5;
    TestAnalysisObj.peakDemandReturnTemp = 10.0;

    EXPECT_DOUBLE_EQ(0.002, state->dataPlnt->PlantLoop(1).MaxVolFlowRate); //  m3/s

    TestAnalysisObj.ResolveDesignFlowRate(*state, 1);

    EXPECT_NEAR(0.00015, state->dataPlnt->PlantLoop(1).MaxVolFlowRate, 0.00001); //  m3/s
    EXPECT_NEAR(0.15, state->dataPlnt->PlantLoop(1).MaxMassFlowRate, 0.001);     //  m3/s
    EXPECT_TRUE(TestAnalysisObj.anotherIterationDesired);
}
