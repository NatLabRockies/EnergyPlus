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

// EnergyPlus::DataPlant Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include "Fixtures/SQLiteFixture.hh"
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HVACSizingSimulationManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

using namespace EnergyPlus;
using namespace DataPlant;
using namespace DataSizing;
using namespace OutputReportPredefined;
using namespace DataLoopNode;
using namespace OutputProcessor;

class HVACSizingSimulationManagerTest : public EnergyPlusFixture
{
protected:
    virtual void SetUp()
    {
        EnergyPlusFixture::SetUp(); // Sets up the base fixture first.

        state->dataFluid->init_state(*state);

        // setup weather manager state needed
        state->dataWeather->NumOfEnvrn = 2;
        state->dataWeather->Environment.allocate(state->dataWeather->NumOfEnvrn);
        state->dataWeather->Environment(1).KindOfEnvrn = Constant::KindOfSim::DesignDay;
        state->dataWeather->Environment(1).DesignDayNum = 1;

        state->dataWeather->Environment(2).KindOfEnvrn = Constant::KindOfSim::DesignDay;
        state->dataWeather->Environment(2).DesignDayNum = 2;

        // setup plant sizing data structure
        state->dataSize->NumPltSizInput = 1;
        state->dataSize->PlantSizData.allocate(state->dataSize->NumPltSizInput);
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).SizingFactorOption = NoSizingFactorMode;
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).DesVolFlowRate = 0.002;
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).DeltaT = 10;
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).ConcurrenceOption = DataSizing::SizingConcurrence::Coincident;
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).NumTimeStepsInAvg = 1;
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).PlantLoopName = "Test Plant Loop 1";
        state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).LoopType = DataSizing::TypeOfPlantLoop::Heating;

        // set up a plant loop
        state->dataPlnt->TotNumLoops = 1;
        state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
        state->dataPlnt->PlantLoop(1).Name = "Test Plant Loop 1";
        state->dataPlnt->PlantLoop(1).MaxVolFlowRateWasAutoSized = true;
        state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 0.002;
        state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 2.0;
        state->dataPlnt->PlantLoop(1).VolumeWasAutoSized = true;
        state->dataPlnt->PlantLoop(1).FluidName = "WATER";
        state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

        state->dataPlnt->PlantLoop(1).LoopSide(LoopSideLocation::Supply).NodeNumIn = 1;

        SetPredefinedTables(*state);

        // need a node to log mass flow rate from
        state->dataLoopNodes->Node.allocate(1);
        // OutputProcessor::TimeValue.allocate(2);
        // set up time related
        SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
        SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);

        state->dataGlobal->TimeStepsInHour = 4;
        state->dataWeather->TimeStepFraction = 1.0 / double(state->dataGlobal->TimeStepsInHour);

        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep = &state->dataGlobal->TimeStepZone;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0; // init
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep = &state->dataHVACGlobal->TimeStepSys;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0;
    }

    virtual void TearDown()
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(HVACSizingSimulationManagerTest, WeatherFileDaysTest3)
{
    // this test emulates two design days and two sizing weather file days periods
    // calls code related to coincident plant sizing with HVAC sizing simulation
    // this test runs 3 system timesteps for each zone timestep

    state->dataWeather->Environment.deallocate();
    // setup weather manager state needed
    state->dataWeather->NumOfEnvrn = 4;
    state->dataWeather->Environment.allocate(state->dataWeather->NumOfEnvrn);
    state->dataWeather->Environment(1).KindOfEnvrn = Constant::KindOfSim::DesignDay;
    state->dataWeather->Environment(1).DesignDayNum = 1;

    state->dataWeather->Environment(2).KindOfEnvrn = Constant::KindOfSim::DesignDay;
    state->dataWeather->Environment(2).DesignDayNum = 2;

    state->dataWeather->Environment(3).KindOfEnvrn = Constant::KindOfSim::RunPeriodDesign;
    state->dataWeather->Environment(3).DesignDayNum = 0;
    state->dataWeather->Environment(3).TotalDays = 4;

    state->dataWeather->Environment(4).KindOfEnvrn = Constant::KindOfSim::RunPeriodDesign;
    state->dataWeather->Environment(4).DesignDayNum = 0;
    state->dataWeather->Environment(4).TotalDays = 4;

    HVACSizingSimulationManager testSizeSimManagerObj;

    testSizeSimManagerObj.DetermineSizingAnalysesNeeded(*state);

    EXPECT_EQ(1, testSizeSimManagerObj.plantCoincAnalyObjs[0].supplySideInletNodeNum);

    testSizeSimManagerObj.SetupSizingAnalyses(*state);

    EXPECT_EQ(4, state->dataWeather->NumOfEnvrn);
    Weather::AddDesignSetToEnvironmentStruct(*state, 1);
    EXPECT_EQ(8, state->dataWeather->NumOfEnvrn);

    // now fill with three system timesteps for each zone timestep
    state->dataGlobal->TimeStepZone = 15.0 / 60.0;
    state->dataHVACGlobal->NumOfSysTimeSteps = 3;
    state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone / state->dataHVACGlobal->NumOfSysTimeSteps;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // first HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 5;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 1;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);

    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;
                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // ... End hour loop.

    // second HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 6;

    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 2;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;

                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // End hour loop.

    // first HVAC Sizing Simulation WeatherFileDays emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeRunPeriodDesign;
    state->dataGlobal->DayOfSim = 0;
    state->dataWeather->Envrn = 7;
    state->dataGlobal->NumOfDayInEnvrn = 4;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    while (state->dataGlobal->DayOfSim < state->dataGlobal->NumOfDayInEnvrn) {
        ++state->dataGlobal->DayOfSim;
        for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
            for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
                for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                    state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                        (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                    state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                    state->dataLoopNodes->Node(1).Temp = 10.0;
                    state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;
                    testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
                }
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
            } // TimeStep loop
        } // ... End hour loop.
    } // day loop

    // second HVAC Sizing Simulation WEatherFileDAys emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeRunPeriodDesign;
    state->dataGlobal->DayOfSim = 0;
    state->dataWeather->Envrn = 8;
    state->dataGlobal->NumOfDayInEnvrn = 4;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    while (state->dataGlobal->DayOfSim < state->dataGlobal->NumOfDayInEnvrn) {
        ++state->dataGlobal->DayOfSim;
        for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
            for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
                for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                    state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                        (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                    state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                    state->dataLoopNodes->Node(1).Temp = 10.0;
                    state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;
                    testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
                }
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
            } // TimeStep loop
        } // ... End hour loop.
    } // day loop

    testSizeSimManagerObj.PostProcessLogs();

    // check plant resizing
    EXPECT_DOUBLE_EQ(2.0, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // original size
    testSizeSimManagerObj.ProcessCoincidentPlantSizeAdjustments(*state, 1);
    EXPECT_DOUBLE_EQ(2.4, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // resize check

    // check that the data are as expected in the logs
    // first timestep
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[0]
                         .subSteps[0]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[0]
                         .runningAvgDataValue);

    // last timestep of first hour
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[3]
                         .subSteps[2]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[3]
                         .runningAvgDataValue);

    // first timestep of second hour
    EXPECT_DOUBLE_EQ(0.2,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[7]
                         .subSteps[0]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.2,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[7]
                         .runningAvgDataValue);

    // last timestep of first DD, hour = 24
    EXPECT_DOUBLE_EQ(2.4,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[95]
                         .subSteps[2]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(2.4,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[95]
                         .runningAvgDataValue);

    // first timestep of second DD, hour = 1
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[96]
                         .subSteps[0]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[96]
                         .runningAvgDataValue);

    // first timestep of third sizing state->dataWeather->Environment WeatherFileDays
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[192]
                         .runningAvgDataValue);

    // first timestep of fourth sizing state->dataWeather->Environment WeatherFileDays
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[576]
                         .runningAvgDataValue);
}

TEST_F(HVACSizingSimulationManagerTest, TopDownTestSysTimestep3)
{
    // this test emulates two design days and calls nearly all the OO code related
    // to coincident plant sizing with HVAC sizing simulation
    // this test runs 3 system timesteps for each zone timestep

    HVACSizingSimulationManager testSizeSimManagerObj;

    testSizeSimManagerObj.DetermineSizingAnalysesNeeded(*state);

    EXPECT_EQ(1, testSizeSimManagerObj.plantCoincAnalyObjs[0].supplySideInletNodeNum);

    testSizeSimManagerObj.SetupSizingAnalyses(*state);

    EXPECT_EQ(2, state->dataWeather->NumOfEnvrn);
    Weather::AddDesignSetToEnvironmentStruct(*state, 1);

    EXPECT_EQ(4, state->dataWeather->NumOfEnvrn);

    // now fill with three system timesteps for each zone timestep
    state->dataGlobal->TimeStepZone = 15.0 / 60.0;
    state->dataHVACGlobal->NumOfSysTimeSteps = 3;
    state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone / state->dataHVACGlobal->NumOfSysTimeSteps;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // first HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 3;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 1;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;
                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // ... End hour loop.

    // second HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 4;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 2;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;

                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // End hour loop.

    testSizeSimManagerObj.PostProcessLogs();

    // check plant resizing
    EXPECT_DOUBLE_EQ(2.0, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // original size
    testSizeSimManagerObj.ProcessCoincidentPlantSizeAdjustments(*state, 1);
    EXPECT_DOUBLE_EQ(2.4, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // resize check

    // check that the data are as expected in the logs
    // first timestep
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[0]
                         .subSteps[0]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[0]
                         .runningAvgDataValue);

    // last timestep of first hour
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[3]
                         .subSteps[2]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[3]
                         .runningAvgDataValue);

    // first timestep of second hour
    EXPECT_DOUBLE_EQ(0.2,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[7]
                         .subSteps[0]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.2,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[7]
                         .runningAvgDataValue);

    // last timestep of first DD, hour = 24
    EXPECT_DOUBLE_EQ(2.4,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[95]
                         .subSteps[2]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(2.4,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[95]
                         .runningAvgDataValue);

    // first timestep of second DD, hour = 1
    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[96]
                         .subSteps[0]
                         .LogDataValue);

    EXPECT_DOUBLE_EQ(0.1,
                     testSizeSimManagerObj.sizingLogger.logObjs[testSizeSimManagerObj.plantCoincAnalyObjs[0].supplyInletNodeFlow_LogIndex]
                         .ztStepObj[96]
                         .runningAvgDataValue);
}

TEST_F(HVACSizingSimulationManagerTest, TopDownTestSysTimestep1)
{
    // this test emulates two design days and calls nearly all the OO code related
    // to coincident plant sizing with HVAC sizing simulation
    // this test runs 1 system timestep for each zone timestep

    state->dataSize->GlobalCoolSizingFactor = 1.0;
    state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).SizingFactorOption = GlobalCoolingSizingFactorMode;

    HVACSizingSimulationManager testSizeSimManagerObj;

    testSizeSimManagerObj.DetermineSizingAnalysesNeeded(*state);

    EXPECT_EQ(1, testSizeSimManagerObj.plantCoincAnalyObjs[0].supplySideInletNodeNum);

    testSizeSimManagerObj.SetupSizingAnalyses(*state);

    EXPECT_EQ(2, state->dataWeather->NumOfEnvrn);
    Weather::AddDesignSetToEnvironmentStruct(*state, 1);
    EXPECT_EQ(4, state->dataWeather->NumOfEnvrn);

    // now fill with one system timesteps for each zone timestep
    state->dataGlobal->TimeStepZone = 15.0 / 60.0;
    state->dataHVACGlobal->NumOfSysTimeSteps = 1;
    state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone / state->dataHVACGlobal->NumOfSysTimeSteps;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // first HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 3;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 1;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {

            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;
                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            // E+ doesn't really update zone step data until system steps are done
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // ... End hour loop.

    // second HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 4;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 2;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {

            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;

                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // End hour loop.

    testSizeSimManagerObj.PostProcessLogs();

    EXPECT_DOUBLE_EQ(2.0, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // original size
    testSizeSimManagerObj.ProcessCoincidentPlantSizeAdjustments(*state, 1);
    EXPECT_DOUBLE_EQ(2.4, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // resize check
}

TEST_F(HVACSizingSimulationManagerTest, VarySysTimesteps)
{
    // this test emulates two design days and calls nearly all the OO code related
    // to coincident plant sizing with HVAC sizing simulation
    // this test run varies the system timestep some to test irregular

    state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).NumTimeStepsInAvg = 2;
    state->dataSize->GlobalHeatSizingFactor = 1.0;
    state->dataSize->PlantSizData(state->dataSize->NumPltSizInput).SizingFactorOption = GlobalHeatingSizingFactorMode;

    HVACSizingSimulationManager testSizeSimManagerObj;

    testSizeSimManagerObj.DetermineSizingAnalysesNeeded(*state);

    EXPECT_EQ(1, testSizeSimManagerObj.plantCoincAnalyObjs[0].supplySideInletNodeNum);

    testSizeSimManagerObj.SetupSizingAnalyses(*state);

    EXPECT_EQ(2, state->dataWeather->NumOfEnvrn);
    Weather::AddDesignSetToEnvironmentStruct(*state, 1);
    EXPECT_EQ(4, state->dataWeather->NumOfEnvrn);

    // now fill with one system timesteps for each zone timestep
    state->dataGlobal->TimeStepZone = 15.0 / 60.0;
    state->dataHVACGlobal->NumOfSysTimeSteps = 1;
    state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone / state->dataHVACGlobal->NumOfSysTimeSteps;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // first HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 3;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 1;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {

            state->dataHVACGlobal->NumOfSysTimeSteps = state->dataGlobal->TimeStep;
            state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone / state->dataHVACGlobal->NumOfSysTimeSteps;
            state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;
                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            // E+ doesn't really update zone step data until system steps are done
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // ... End hour loop.

    // second HVAC Sizing Simulation DD emulation
    state->dataGlobal->KindOfSim = Constant::KindOfSim::HVACSizeDesignDay;
    state->dataGlobal->DayOfSim = 1;
    state->dataWeather->Envrn = 4;
    state->dataWeather->Environment(state->dataWeather->Envrn).DesignDayNum = 2;
    testSizeSimManagerObj.sizingLogger.SetupSizingLogsNewEnvironment(*state);
    for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute = 0.0;
        state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute = 0.0;
        for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour; ++state->dataGlobal->TimeStep) {
            state->dataHVACGlobal->NumOfSysTimeSteps = state->dataGlobal->TimeStep;
            state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone / state->dataHVACGlobal->NumOfSysTimeSteps;
            state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

            for (int SysTimestepLoop = 1; SysTimestepLoop <= state->dataHVACGlobal->NumOfSysTimeSteps; ++SysTimestepLoop) {
                state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute +=
                    (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * 60.0;

                state->dataLoopNodes->Node(1).MassFlowRate = state->dataGlobal->HourOfDay * 0.1;
                state->dataLoopNodes->Node(1).Temp = 10.0;
                state->dataPlnt->PlantLoop(1).HeatingDemand = state->dataGlobal->HourOfDay * 10.0;

                testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesSystemStep(*state);
            }
            state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].CurMinute +=
                (*state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep) * 60.0;
            testSizeSimManagerObj.sizingLogger.UpdateSizingLogValuesZoneStep(*state);
        } // TimeStep loop
    } // End hour loop.

    testSizeSimManagerObj.PostProcessLogs();

    EXPECT_DOUBLE_EQ(2.0, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // original size
    testSizeSimManagerObj.ProcessCoincidentPlantSizeAdjustments(*state, 1);
    EXPECT_DOUBLE_EQ(2.4, state->dataPlnt->PlantLoop(1).MaxMassFlowRate); // resize check

    testSizeSimManagerObj.ProcessCoincidentPlantSizeAdjustments(*state, 1);

    testSizeSimManagerObj.sizingLogger.IncrementSizingPeriodSet();
}

// TODO: Need to fix this unit test since it fails with the #8268 fix when it doesn't use
// the on-disk sqlite database. I'm not convinced #8268 is actually fixed.
//
// TEST_F(SQLiteFixture, HVACSizing_Passes_SQL_Output)
// {
//     // Test for #8268 - More info in the "Time" table than EnvironmentPeriods
//     // To reproduce, we need two things:
//     // * a Sizing:Plant object set to "Coincident", and,
//     // * the SimulatioNControl "Do HVAC Sizing Simulation for Sizing Periods" set to Yes

//     std::string const idf_objects = delimited_string({

//         "Timestep,",
//         "  4;                                      !- Number of Timesteps per Hour",

//         "Site:Location,",
//         "  Chicago Ohare Intl Ap,                  !- Name",
//         "  41.98,                                  !- Latitude {deg}",
//         "  -87.92,                                 !- Longitude {deg}",
//         "  -6,                                     !- Time Zone {hr}",
//         "  201;                                    !- Elevation {m}",

//         "SizingPeriod:DesignDay,",
//         "  Chicago Ohare Intl Ap Ann Htg 99.6% Condns DB, !- Name",
//         "  1,                                      !- Month",
//         "  21,                                     !- Day of Month",
//         "  WinterDesignDay,                        !- Day Type",
//         "  -20,                                    !- Maximum Dry-Bulb Temperature {C}",
//         "  0,                                      !- Daily Dry-Bulb Temperature Range {deltaC}",
//         "  DefaultMultipliers,                     !- Dry-Bulb Temperature Range Modifier Type",
//         "  ,                                       !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
//         "  Wetbulb,                                !- Humidity Condition Type",
//         "  -20,                                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
//         "  ,                                       !- Humidity Condition Day Schedule Name",
//         "  ,                                       !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
//         "  ,                                       !- Enthalpy at Maximum Dry-Bulb {J/kg}",
//         "  ,                                       !- Daily Wet-Bulb Temperature Range {deltaC}",
//         "  98934,                                  !- Barometric Pressure {Pa}",
//         "  4.9,                                    !- Wind Speed {m/s}",
//         "  270,                                    !- Wind Direction {deg}",
//         "  No,                                     !- Rain Indicator",
//         "  No,                                     !- Snow Indicator",
//         "  No,                                     !- Daylight Saving Time Indicator",
//         "  ASHRAEClearSky,                         !- Solar Model Indicator",
//         "  ,                                       !- Beam Solar Day Schedule Name",
//         "  ,                                       !- Diffuse Solar Day Schedule Name",
//         "  ,                                       !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
//         "  ,                                       !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
//         "  0;                                      !- Sky Clearness",

//         "SimulationControl,",
//         "  Yes,                                    !- Do Zone Sizing Calculation",
//         "  Yes,                                    !- Do System Sizing Calculation",
//         "  Yes,                                    !- Do Plant Sizing Calculation",
//         "  Yes,                                    !- Run Simulation for Sizing Periods",
//         "  No,                                     !- Run Simulation for Weather File Run Periods",
//         "  Yes,                                    !- Do HVAC Sizing Simulation for Sizing Periods",
//         "  2;                                      !- Maximum Number of HVAC Sizing Simulation Passes",

//         "ScheduleTypeLimits,",
//         "  OnOff,                                  !- Name",
//         "  0,                                      !- Lower Limit Value {BasedOnField A3}",
//         "  1,                                      !- Upper Limit Value {BasedOnField A3}",
//         "  Discrete,                               !- Numeric Type",
//         "  availability;                           !- Unit Type",
//         "",
//         "ScheduleTypeLimits,",
//         "  Temperature,                            !- Name",
//         "  ,                                       !- Lower Limit Value {BasedOnField A3}",
//         "  ,                                       !- Upper Limit Value {BasedOnField A3}",
//         "  Continuous,                             !- Numeric Type",
//         "  temperature;                            !- Unit Type",

//         "ScheduleTypeLimits,",
//         "  Dimensionless,                          !- Name",
//         "  ,                                       !- Lower Limit Value {BasedOnField A3}",
//         "  ,                                       !- Upper Limit Value {BasedOnField A3}",
//         "  Continuous;                             !- Numeric Type",

//         "Schedule:Constant,",
//         "  Always On Discrete,                     !- Name",
//         "  OnOff,                                  !- Schedule Type Limits Name",
//         "  1;                                      !- Hourly Value",

//         "Schedule:Constant,",
//         "  Ambient Temp 22C,                       !- Name",
//         "  Temperature,                            !- Schedule Type Limits Name",
//         "  22;                                     !- Hourly Value",

//         "Schedule:Constant,",
//         "  DHW Setpoint Temp 60C,                  !- Name",
//         "  Temperature,                            !- Schedule Type Limits Name",
//         "  60;                                     !- Hourly Value",

//         "Schedule:Constant,",
//         "  Load Schedule,                          !- Name",
//         "  Dimensionless,                          !- Schedule Type Limits Name",
//         "  8000;                                   !- Hourly Value",

//         "PlantLoop,",
//         "  Plant Loop 1,                           !- Name",
//         "  Water,                                  !- Fluid Type",
//         "  ,                                       !- User Defined Fluid Type",
//         "  Plant Loop 1 Operation Schemes,         !- Plant Equipment Operation Scheme Name",
//         "  Node 2,                                 !- Loop Temperature Setpoint Node Name",
//         "  100,                                    !- Maximum Loop Temperature {C}",
//         "  0,                                      !- Minimum Loop Temperature {C}",
//         "  Autosize,                               !- Maximum Loop Flow Rate {m3/s}",
//         "  0,                                      !- Minimum Loop Flow Rate {m3/s}",
//         "  Autocalculate,                          !- Plant Loop Volume {m3}",
//         "  Node 1,                                 !- Plant Side Inlet Node Name",
//         "  Node 2,                                 !- Plant Side Outlet Node Name",
//         "  Plant Loop 1 Supply Branches,           !- Plant Side Branch List Name",
//         "  Plant Loop 1 Supply Connector List,     !- Plant Side Connector List Name",
//         "  Node 4,                                 !- Demand Side Inlet Node Name",
//         "  Node 5,                                 !- Demand Side Outlet Node Name",
//         "  Plant Loop 1 Demand Branches,           !- Demand Side Branch List Name",
//         "  Plant Loop 1 Demand Connector List,     !- Demand Side Connector List Name",
//         "  Optimal,                                !- Load Distribution Scheme",
//         "  ,                                       !- Availability Manager List Name",
//         "  SingleSetpoint,                         !- Plant Loop Demand Calculation Scheme",
//         "  ;                                       !- Common Pipe Simulation",

//         "SetpointManager:Scheduled,",
//         "  Setpoint Manager Scheduled 1,           !- Name",
//         "  Temperature,                            !- Control Variable",
//         "  DHW Setpoint Temp 60C,                  !- Schedule Name",
//         "  Node 2;                                 !- Setpoint Node or NodeList Name",

//         "Sizing:Plant,",
//         "  Plant Loop 1,                           !- Plant or Condenser Loop Name",
//         "  Heating,                                !- Loop Type",
//         "  82,                                     !- Design Loop Exit Temperature {C}",
//         "  11,                                     !- Loop Design Temperature Difference {deltaC}",
//         "  Coincident,                             !- Sizing Option",
//         "  2,                                      !- Zone Timesteps in Averaging Window",
//         "  None;                                   !- Coincident Sizing Factor Mode",

//         "BranchList,",
//         "  Plant Loop 1 Supply Branches,           !- Name",
//         "  Plant Loop 1 Supply Inlet Branch,       !- Branch Name 1",
//         "  Plant Loop 1 Supply Branch 1,           !- Branch Name 2",
//         "  Plant Loop 1 Supply Outlet Branch;      !- Branch Name 3",

//         "ConnectorList,",
//         "  Plant Loop 1 Supply Connector List,     !- Name",
//         "  Connector:Splitter,                     !- Connector Object Type 1",
//         "  Plant Loop 1 Supply Splitter,           !- Connector Name 1",
//         "  Connector:Mixer,                        !- Connector Object Type 2",
//         "  Plant Loop 1 Supply Mixer;              !- Connector Name 2",

//         "Connector:Splitter,",
//         "  Plant Loop 1 Supply Splitter,           !- Name",
//         "  Plant Loop 1 Supply Inlet Branch,       !- Inlet Branch Name",
//         "  Plant Loop 1 Supply Branch 1;           !- Outlet Branch Name 1",

//         "Connector:Mixer,",
//         "  Plant Loop 1 Supply Mixer,              !- Name",
//         "  Plant Loop 1 Supply Outlet Branch,      !- Outlet Branch Name",
//         "  Plant Loop 1 Supply Branch 1;           !- Inlet Branch Name 1",

//         "Branch,",
//         "  Plant Loop 1 Supply Inlet Branch,       !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pump:VariableSpeed,                     !- Component Object Type 1",
//         "  Pump Variable Speed 1,                  !- Component Name 1",
//         "  Node 1,                                 !- Component Inlet Node Name 1",
//         "  Node 7;                                 !- Component Outlet Node Name 1",

//         "Pump:VariableSpeed,",
//         "  Pump Variable Speed 1,                  !- Name",
//         "  Node 1,                                 !- Inlet Node Name",
//         "  Node 7,                                 !- Outlet Node Name",
//         "  Autosize,                               !- Design Maximum Flow Rate {m3/s}",
//         "  179352,                                 !- Design Pump Head {Pa}",
//         "  Autosize,                               !- Design Power Consumption {W}",
//         "  0.9,                                    !- Motor Efficiency",
//         "  0,                                      !- Fraction of Motor Inefficiencies to Fluid Stream",
//         "  0,                                      !- Coefficient 1 of the Part Load Performance Curve",
//         "  1,                                      !- Coefficient 2 of the Part Load Performance Curve",
//         "  0,                                      !- Coefficient 3 of the Part Load Performance Curve",
//         "  0,                                      !- Coefficient 4 of the Part Load Performance Curve",
//         "  0,                                      !- Design Minimum Flow Rate {m3/s}",
//         "  Intermittent,                           !- Pump Control Type",
//         "  ,                                       !- Pump Flow Rate Schedule Name",
//         "  ,                                       !- Pump Curve Name",
//         "  ,                                       !- Impeller Diameter {m}",
//         "  ,                                       !- VFD Control Type",
//         "  ,                                       !- Pump RPM Schedule Name",
//         "  ,                                       !- Minimum Pressure Schedule {Pa}",
//         "  ,                                       !- Maximum Pressure Schedule {Pa}",
//         "  ,                                       !- Minimum RPM Schedule {rev/min}",
//         "  ,                                       !- Maximum RPM Schedule {rev/min}",
//         "  ,                                       !- Zone Name",
//         "  0.5,                                    !- Skin Loss Radiative Fraction",
//         "  PowerPerFlowPerPressure,                !- Design Power Sizing Method",
//         "  348701.1,                               !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
//         "  1.282051282,                            !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
//         "  0,                                      !- Design Minimum Flow Rate Fraction",
//         "  General;                                !- End-Use Subcategory",

//         "Branch,",
//         "  Plant Loop 1 Supply Branch 1,           !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  WaterHeater:Mixed,                      !- Component Object Type 1",
//         "  Water Heater Mixed 1,                   !- Component Name 1",
//         "  Node 3,                                 !- Component Inlet Node Name 1",
//         "  Node 8;                                 !- Component Outlet Node Name 1",

//         "WaterHeater:Mixed,",
//         "  Water Heater Mixed 1,                   !- Name",
//         "  0.3785,                                 !- Tank Volume {m3}",
//         "  DHW Setpoint Temp 60C,                  !- Setpoint Temperature Schedule Name",
//         "  2,                                      !- Deadband Temperature Difference {deltaC}",
//         "  82.22,                                  !- Maximum Temperature Limit {C}",
//         "  Cycle,                                  !- Heater Control Type",
//         "  845000,                                 !- Heater Maximum Capacity {W}",
//         "  ,                                       !- Heater Minimum Capacity {W}",
//         "  0,                                      !- Heater Ignition Minimum Flow Rate {m3/s}",
//         "  0,                                      !- Heater Ignition Delay {s}",
//         "  NaturalGas,                             !- Heater Fuel Type",
//         "  0.8,                                    !- Heater Thermal Efficiency",
//         "  ,                                       !- Part Load Factor Curve Name",
//         "  20,                                     !- Off Cycle Parasitic Fuel Consumption Rate {W}",
//         "  NaturalGas,                             !- Off Cycle Parasitic Fuel Type",
//         "  0.8,                                    !- Off Cycle Parasitic Heat Fraction to Tank",
//         "  0,                                      !- On Cycle Parasitic Fuel Consumption Rate {W}",
//         "  NaturalGas,                             !- On Cycle Parasitic Fuel Type",
//         "  0,                                      !- On Cycle Parasitic Heat Fraction to Tank",
//         "  Schedule,                               !- Ambient Temperature Indicator",
//         "  Ambient Temp 22C,                       !- Ambient Temperature Schedule Name",
//         "  ,                                       !- Ambient Temperature Zone Name",
//         "  ,                                       !- Ambient Temperature Outdoor Air Node Name",
//         "  6,                                      !- Off Cycle Loss Coefficient to Ambient Temperature {W/K}",
//         "  1,                                      !- Off Cycle Loss Fraction to Zone",
//         "  6,                                      !- On Cycle Loss Coefficient to Ambient Temperature {W/K}",
//         "  1,                                      !- On Cycle Loss Fraction to Zone",
//         "  ,                                       !- Peak Use Flow Rate {m3/s}",
//         "  ,                                       !- Use Flow Rate Fraction Schedule Name",
//         "  ,                                       !- Cold Water Supply Temperature Schedule Name",
//         "  Node 3,                                 !- Use Side Inlet Node Name",
//         "  Node 8,                                 !- Use Side Outlet Node Name",
//         "  1,                                      !- Use Side Effectiveness",
//         "  ,                                       !- Source Side Inlet Node Name",
//         "  ,                                       !- Source Side Outlet Node Name",
//         "  1,                                      !- Source Side Effectiveness",
//         "  Autosize,                               !- Use Side Design Flow Rate {m3/s}",
//         "  Autosize,                               !- Source Side Design Flow Rate {m3/s}",
//         "  1.5,                                    !- Indirect Water Heating Recovery Time {hr}",
//         "  IndirectHeatPrimarySetpoint,            !- Source Side Flow Control Mode",
//         "  ,                                       !- Indirect Alternate Setpoint Temperature Schedule Name",
//         "  General;                                !- End-Use Subcategory",

//         "Branch,",
//         "  Plant Loop 1 Supply Outlet Branch,      !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  Plant Loop 1 Supply Outlet Pipe,        !- Component Name 1",
//         "  Plant Loop 1 Supply Outlet Pipe Node,   !- Component Inlet Node Name 1",
//         "  Node 2;                                 !- Component Outlet Node Name 1",

//         "Pipe:Adiabatic,",
//         "  Plant Loop 1 Supply Outlet Pipe,        !- Name",
//         "  Plant Loop 1 Supply Outlet Pipe Node,   !- Inlet Node Name",
//         "  Node 2;                                 !- Outlet Node Name",

//         "BranchList,",
//         "  Plant Loop 1 Demand Branches,           !- Name",
//         "  Plant Loop 1 Demand Inlet Branch,       !- Branch Name 1",
//         "  Plant Loop 1 Demand Branch 1,           !- Branch Name 2",
//         "  Plant Loop 1 Demand Bypass Branch,      !- Branch Name 3",
//         "  Plant Loop 1 Demand Outlet Branch;      !- Branch Name 4",

//         "ConnectorList,",
//         "  Plant Loop 1 Demand Connector List,     !- Name",
//         "  Connector:Splitter,                     !- Connector Object Type 1",
//         "  Plant Loop 1 Demand Splitter,           !- Connector Name 1",
//         "  Connector:Mixer,                        !- Connector Object Type 2",
//         "  Plant Loop 1 Demand Mixer;              !- Connector Name 2",

//         "Connector:Splitter,",
//         "  Plant Loop 1 Demand Splitter,           !- Name",
//         "  Plant Loop 1 Demand Inlet Branch,       !- Inlet Branch Name",
//         "  Plant Loop 1 Demand Branch 1,           !- Outlet Branch Name 1",
//         "  Plant Loop 1 Demand Bypass Branch;      !- Outlet Branch Name 2",

//         "Connector:Mixer,",
//         "  Plant Loop 1 Demand Mixer,              !- Name",
//         "  Plant Loop 1 Demand Outlet Branch,      !- Outlet Branch Name",
//         "  Plant Loop 1 Demand Branch 1,           !- Inlet Branch Name 1",
//         "  Plant Loop 1 Demand Bypass Branch;      !- Inlet Branch Name 2",

//         "Branch,",
//         "  Plant Loop 1 Demand Inlet Branch,       !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  Plant Loop 1 Demand Inlet Pipe,         !- Component Name 1",
//         "  Node 4,                                 !- Component Inlet Node Name 1",
//         "  Plant Loop 1 Demand Inlet Pipe Node;    !- Component Outlet Node Name 1",

//         "Pipe:Adiabatic,",
//         "  Plant Loop 1 Demand Inlet Pipe,         !- Name",
//         "  Node 4,                                 !- Inlet Node Name",
//         "  Plant Loop 1 Demand Inlet Pipe Node;    !- Outlet Node Name",

//         "Branch,",
//         "  Plant Loop 1 Demand Branch 1,           !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  LoadProfile:Plant,                      !- Component Object Type 1",
//         "  Load Profile Plant 1,                   !- Component Name 1",
//         "  Node 6,                                 !- Component Inlet Node Name 1",
//         "  Node 9;                                 !- Component Outlet Node Name 1",

//         "LoadProfile:Plant,",
//         "  Load Profile Plant 1,                   !- Name",
//         "  Node 6,                                 !- Inlet Node Name",
//         "  Node 9,                                 !- Outlet Node Name",
//         "  Load Schedule,                          !- Load Schedule Name",
//         "  0.002,                                  !- Peak Flow Rate {m3/s}",
//         "  Always On Discrete;                     !- Flow Rate Fraction Schedule Name",

//         "Branch,",
//         "  Plant Loop 1 Demand Bypass Branch,      !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  Plant Loop 1 Demand Bypass Pipe,        !- Component Name 1",
//         "  Plant Loop 1 Demand Bypass Pipe Inlet Node, !- Component Inlet Node Name 1",
//         "  Plant Loop 1 Demand Bypass Pipe Outlet Node; !- Component Outlet Node Name 1",

//         "Pipe:Adiabatic,",
//         "  Plant Loop 1 Demand Bypass Pipe,        !- Name",
//         "  Plant Loop 1 Demand Bypass Pipe Inlet Node, !- Inlet Node Name",
//         "  Plant Loop 1 Demand Bypass Pipe Outlet Node; !- Outlet Node Name",

//         "Branch,",
//         "  Plant Loop 1 Demand Outlet Branch,      !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  Plant Loop 1 Demand Outlet Pipe,        !- Component Name 1",
//         "  Plant Loop 1 Demand Outlet Pipe Node,   !- Component Inlet Node Name 1",
//         "  Node 5;                                 !- Component Outlet Node Name 1",

//         "Pipe:Adiabatic,",
//         "  Plant Loop 1 Demand Outlet Pipe,        !- Name",
//         "  Plant Loop 1 Demand Outlet Pipe Node,   !- Inlet Node Name",
//         "  Node 5;                                 !- Outlet Node Name",

//         "PlantEquipmentOperationSchemes,",
//         "  Plant Loop 1 Operation Schemes,         !- Name",
//         "  PlantEquipmentOperation:HeatingLoad,    !- Control Scheme Object Type 1",
//         "  Plant Loop 1 Heating Operation Scheme,  !- Control Scheme Name 1",
//         "  Always On Discrete;                     !- Control Scheme Schedule Name 1",

//         "PlantEquipmentOperation:HeatingLoad,",
//         "  Plant Loop 1 Heating Operation Scheme,  !- Name",
//         "  0,                                      !- Load Range Lower Limit 1 {W}",
//         "  1000000000,                             !- Load Range Upper Limit 1 {W}",
//         "  Plant Loop 1 Heating Equipment List;    !- Range Equipment List Name 1",

//         "PlantEquipmentList,",
//         "  Plant Loop 1 Heating Equipment List,    !- Name",
//         "  WaterHeater:Mixed,                      !- Equipment Object Type 1",
//         "  Water Heater Mixed 1;                   !- Equipment Name 1",

//         // Obviously need to add the sqlite output since we query it...
//         "Output:SQLite,",
//         "  SimpleAndTabular;                       !- Option Type",

//         // Need at least one meter for the query below to succeed
//         "Output:Meter,",
//         "  NaturalGas:Facility,                    !- Key Name",
//         "  Timestep;                               !- Reporting Frequency",
//     });

//     ASSERT_TRUE(process_idf(idf_objects));

//     SimulationManager::ManageSimulation(*state); // run the design day over the warmup period (24 hrs, 25 days)

//     const std::string environmentPeriodsQuery = R"(SELECT COUNT(EnvironmentName) FROM EnvironmentPeriods)";
//     Real64 number_of_periods_in_environmentperiods = execAndReturnFirstDouble(environmentPeriodsQuery);
//     EXPECT_EQ(1.0, number_of_periods_in_environmentperiods);

//     const std::string timeQuery = R"(SELECT COUNT(DISTINCT(EnvironmentPeriodIndex)) FROM Time)";
//     Real64 number_of_periods_in_time = execAndReturnFirstDouble(timeQuery);
//     EXPECT_EQ(1.0, number_of_periods_in_time);

//     const std::string query = R"(
//         SELECT DISTINCT(Time.EnvironmentPeriodIndex), EnvironmentPeriods.EnvironmentName FROM Time
//          LEFT JOIN EnvironmentPeriods ON Time.EnvironmentPeriodIndex = EnvironmentPeriods.EnvironmentPeriodIndex
//          ORDER BY Time.EnvironmentPeriodIndex
//     )";

//     auto periodResults = queryResult(query, "Time");

//     // queryResults uses the tableName passed in second argument to determine the size of each row vector based on the columnCount
//     // So not exactly meant to run JOIN queries. Anyways, here we only care about the first two in the row, the rest are blanks.
//     int colCount = columnCount("Time");
//     std::vector<std::string> rowData(colCount);
//     rowData[0] = "1";
//     rowData[1] = "CHICAGO OHARE INTL AP ANN HTG 99.6% CONDNS DB";

//     std::vector<std::vector<std::string>> timeData({// Just one row, with a proper name
//                                                     rowData});

//     EXPECT_EQ(timeData, periodResults);
// }
