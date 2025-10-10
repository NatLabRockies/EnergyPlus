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

// C++ Headers
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HVACSizingSimulationManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantPipingSystemsManager.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

void HVACSizingSimulationManager::DetermineSizingAnalysesNeeded(EnergyPlusData &state)
{
    // currently the only type of advanced sizing analysis available is for coincident plant sizing
    // expect more specialized sizing analysis objects to be added, so minimizing code here and jump to a worker method once we know an instance is to
    // be created.

    // Loop over PlantSizData struct and find those plant loops that are to use coincident sizing
    for (int i = 1; i <= state.dataSize->NumPltSizInput; ++i) {

        if (state.dataSize->PlantSizData(i).ConcurrenceOption == DataSizing::Coincident) {

            // create an instance of analysis object for each loop
            CreateNewCoincidentPlantAnalysisObject(state, state.dataSize->PlantSizData(i).PlantLoopName, i);
        }
    }
}

void HVACSizingSimulationManager::CreateNewCoincidentPlantAnalysisObject(EnergyPlusData &state,
                                                                         std::string const &PlantLoopName,
                                                                         int const PlantSizingIndex)
{
    Real64 density;
    Real64 cp;

    // find plant loop number
    for (int i = 1; i <= state.dataPlnt->TotNumLoops; ++i) {
        if (PlantLoopName == state.dataPlnt->PlantLoop(i).Name) { // found it

            density = state.dataPlnt->PlantLoop(i).glycol->getDensity(state, Constant::CWInitConvTemp, "createNewCoincidentPlantAnalysisObject");
            cp = state.dataPlnt->PlantLoop(i).glycol->getSpecificHeat(state, Constant::CWInitConvTemp, "createNewCoincidentPlantAnalysisObject");

            plantCoincAnalyObjs.emplace_back(PlantLoopName,
                                             i,
                                             state.dataPlnt->PlantLoop(i).LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn,
                                             density,
                                             cp,
                                             state.dataSize->PlantSizData(PlantSizingIndex).NumTimeStepsInAvg,
                                             PlantSizingIndex);
        }
    }
}

void HVACSizingSimulationManager::SetupSizingAnalyses(EnergyPlusData &state)
{

    for (auto &P : plantCoincAnalyObjs) {
        // call setup log routine for each coincident plant analysis object
        P.supplyInletNodeFlow_LogIndex =
            sizingLogger.SetupVariableSizingLog(state, state.dataLoopNodes->Node(P.supplySideInletNodeNum).MassFlowRate, P.numTimeStepsInAvg);
        P.supplyInletNodeTemp_LogIndex =
            sizingLogger.SetupVariableSizingLog(state, state.dataLoopNodes->Node(P.supplySideInletNodeNum).Temp, P.numTimeStepsInAvg);
        if (state.dataSize->PlantSizData(P.plantSizingIndex).LoopType == DataSizing::TypeOfPlantLoop::Heating ||
            state.dataSize->PlantSizData(P.plantSizingIndex).LoopType == DataSizing::TypeOfPlantLoop::Steam) {
            P.loopDemand_LogIndex =
                sizingLogger.SetupVariableSizingLog(state, state.dataPlnt->PlantLoop(P.plantLoopIndex).HeatingDemand, P.numTimeStepsInAvg);
        } else if (state.dataSize->PlantSizData(P.plantSizingIndex).LoopType == DataSizing::TypeOfPlantLoop::Cooling ||
                   state.dataSize->PlantSizData(P.plantSizingIndex).LoopType == DataSizing::TypeOfPlantLoop::Condenser) {
            P.loopDemand_LogIndex =
                sizingLogger.SetupVariableSizingLog(state, state.dataPlnt->PlantLoop(P.plantLoopIndex).CoolingDemand, P.numTimeStepsInAvg);
        }
    }
}

void HVACSizingSimulationManager::PostProcessLogs()
{
    // this function calls methods on log objects to do general processing on all the logged data in the framework
    for (auto &L : sizingLogger.logObjs) {
        L.AverageSysTimeSteps();   // collapse subtimestep data into zone step data
        L.ProcessRunningAverage(); // apply zone step moving average
    }
}

void HVACSizingSimulationManager::ProcessCoincidentPlantSizeAdjustments(EnergyPlusData &state, int const HVACSizingIterCount)
{
    // first pass through coincident plant objects to check new sizes and see if more iteration needed
    plantCoinAnalyRequestsAnotherIteration = false;
    for (auto &P : plantCoincAnalyObjs) {
        // step 1 find maximum flow rate on concurrent return temp and load
        P.newFoundMassFlowRateTimeStamp = sizingLogger.logObjs[P.supplyInletNodeFlow_LogIndex].GetLogVariableDataMax(state);
        P.peakMdotCoincidentDemand = sizingLogger.logObjs[P.loopDemand_LogIndex].GetLogVariableDataAtTimestamp(P.newFoundMassFlowRateTimeStamp);
        P.peakMdotCoincidentReturnTemp =
            sizingLogger.logObjs[P.supplyInletNodeTemp_LogIndex].GetLogVariableDataAtTimestamp(P.newFoundMassFlowRateTimeStamp);

        // step 2 find maximum load and concurrent flow and return temp
        P.NewFoundMaxDemandTimeStamp = sizingLogger.logObjs[P.loopDemand_LogIndex].GetLogVariableDataMax(state);
        P.peakDemandMassFlow = sizingLogger.logObjs[P.supplyInletNodeFlow_LogIndex].GetLogVariableDataAtTimestamp(P.NewFoundMaxDemandTimeStamp);
        P.peakDemandReturnTemp = sizingLogger.logObjs[P.supplyInletNodeTemp_LogIndex].GetLogVariableDataAtTimestamp(P.NewFoundMaxDemandTimeStamp);

        P.ResolveDesignFlowRate(state, HVACSizingIterCount);
        if (P.anotherIterationDesired) {
            plantCoinAnalyRequestsAnotherIteration = true;
        }
    }

    // as more sizing adjustments are added this will need to change to consider all not just plant coincident
    //    state.dataGlobal->FinalSizingHVACSizingSimIteration = plantCoinAnalyRequestsAnotherIteration;
}
void HVACSizingSimulationManager::RedoKickOffAndResize(EnergyPlusData &state)
{
    bool ErrorsFound = false;
    state.dataGlobal->KickOffSimulation = true;
    state.dataGlobal->RedoSizesHVACSimulation = true;

    Weather::ResetEnvironmentCounter(state);
    SimulationManager::SetupSimulation(state, ErrorsFound);

    state.dataGlobal->KickOffSimulation = false;
    state.dataGlobal->RedoSizesHVACSimulation = false;
}

void HVACSizingSimulationManager::UpdateSizingLogsZoneStep(EnergyPlusData &state)
{
    sizingLogger.UpdateSizingLogValuesZoneStep(state);
}

void HVACSizingSimulationManager::UpdateSizingLogsSystemStep(EnergyPlusData &state)
{
    sizingLogger.UpdateSizingLogValuesSystemStep(state);
}

void ManageHVACSizingSimulation(EnergyPlusData &state, bool &ErrorsFound)
{
    auto &hvacSizingSimulationManager = state.dataHVACSizingSimMgr->hvacSizingSimulationManager;

    hvacSizingSimulationManager = std::make_unique<HVACSizingSimulationManager>();

    int HVACSizingIterCount;

    hvacSizingSimulationManager->DetermineSizingAnalysesNeeded(state);

    hvacSizingSimulationManager->SetupSizingAnalyses(state);

    DisplayString(state, "Beginning HVAC Sizing Simulation");
    state.dataGlobal->DoingHVACSizingSimulations = true;
    state.dataGlobal->DoOutputReporting = true;

    Weather::ResetEnvironmentCounter(state);

    // iterations over set of sizing periods for HVAC sizing Simulation, will break out if no more are needed
    for (HVACSizingIterCount = 1; HVACSizingIterCount <= state.dataGlobal->HVACSizingSimMaxIterations; ++HVACSizingIterCount) {

        // need to extend Environment structure array to distinguish the HVAC Sizing Simulations from the regular run of that sizing period, repeats
        // for each set
        Weather::AddDesignSetToEnvironmentStruct(state, HVACSizingIterCount);

        state.dataGlobal->WarmupFlag = true;
        bool Available = true;
        for (int i = 1; i <= state.dataWeather->NumOfEnvrn; ++i) { // loop over environments

            Weather::GetNextEnvironment(state, Available, ErrorsFound);
            if (ErrorsFound) {
                break;
            }
            if (!Available) {
                continue;
            }

            hvacSizingSimulationManager->sizingLogger.SetupSizingLogsNewEnvironment(state);

            //    if (!DoDesDaySim) continue; // not sure about this, may need to force users to set this on input for this method, but maybe not
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather) {
                continue;
            }
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::DesignDay) {
                continue;
            }
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodDesign) {
                continue;
            }

            if (state.dataWeather->Environment(state.dataWeather->Envrn).HVACSizingIterationNum != HVACSizingIterCount) {
                continue;
            }

            if (state.dataSysVars->ReportDuringHVACSizingSimulation) {
                if (state.dataSQLiteProcedures->sqlite) {
                    state.dataSQLiteProcedures->sqlite->sqliteBegin();
                    state.dataSQLiteProcedures->sqlite->createSQLiteEnvironmentPeriodRecord(
                        state.dataEnvrn->CurEnvirNum, state.dataEnvrn->EnvironmentName, state.dataGlobal->KindOfSim);
                    state.dataSQLiteProcedures->sqlite->sqliteCommit();
                }
            }
            state.dataErrTracking->ExitDuringSimulations = true;

            DisplayString(state, "Initializing New Environment Parameters, HVAC Sizing Simulation");

            state.dataGlobal->BeginEnvrnFlag = true;
            state.dataGlobal->EndEnvrnFlag = false;
            // EndMonthFlag = false;
            state.dataGlobal->WarmupFlag = true;
            state.dataGlobal->DayOfSim = 0;
            state.dataGlobal->DayOfSimChr = "0";
            state.dataReportFlag->NumOfWarmupDays = 0;

            bool anyEMSRan;
            ManageEMS(state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point

            while ((state.dataGlobal->DayOfSim < state.dataGlobal->NumOfDayInEnvrn) || (state.dataGlobal->WarmupFlag)) { // Begin day loop ...

                // Let's always do a transaction, except we'll roll it back if need be
                // if (ReportDuringHVACSizingSimulation) {
                if (state.dataSQLiteProcedures->sqlite) {
                    state.dataSQLiteProcedures->sqlite->sqliteBegin(); // setup for one transaction per day
                }
                // }
                ++state.dataGlobal->DayOfSim;
                state.dataGlobal->DayOfSimChr = fmt::to_string(state.dataGlobal->DayOfSim);
                if (!state.dataGlobal->WarmupFlag) {
                    ++state.dataEnvrn->CurrentOverallSimDay;
                    DisplaySimDaysProgress(state, state.dataEnvrn->CurrentOverallSimDay, state.dataEnvrn->TotalOverallSimDays);
                } else {
                    state.dataGlobal->DayOfSimChr = "0";
                }
                state.dataGlobal->BeginDayFlag = true;
                state.dataGlobal->EndDayFlag = false;

                if (state.dataGlobal->WarmupFlag) {
                    ++state.dataReportFlag->NumOfWarmupDays;
                    state.dataReportFlag->cWarmupDay = fmt::to_string(state.dataReportFlag->NumOfWarmupDays);
                    DisplayString(state, "Warming up {" + state.dataReportFlag->cWarmupDay + '}');
                } else if (state.dataGlobal->DayOfSim == 1) {
                    DisplayString(
                        state,
                        fmt::format("Starting HVAC Sizing Simulation at {} for {}", state.dataEnvrn->CurMnDy, state.dataEnvrn->EnvironmentName));
                    static constexpr std::string_view Format_700("Environment:WarmupDays,{:3}\n");
                    print(state.files.eio, Format_700, state.dataReportFlag->NumOfWarmupDays);
                } else if (state.dataReportFlag->DisplayPerfSimulationFlag) {
                    DisplayString(state, "Continuing Simulation at " + state.dataEnvrn->CurMnDy + " for " + state.dataEnvrn->EnvironmentName);
                    state.dataReportFlag->DisplayPerfSimulationFlag = false;
                }

                for (state.dataGlobal->HourOfDay = 1; state.dataGlobal->HourOfDay <= 24; ++state.dataGlobal->HourOfDay) { // Begin hour loop ...

                    state.dataGlobal->BeginHourFlag = true;
                    state.dataGlobal->EndHourFlag = false;

                    for (state.dataGlobal->TimeStep = 1; state.dataGlobal->TimeStep <= state.dataGlobal->TimeStepsInHour;
                         ++state.dataGlobal->TimeStep) {
                        if (state.dataGlobal->AnySlabsInModel || state.dataGlobal->AnyBasementsInModel) {
                            PlantPipingSystemsManager::SimulateGroundDomains(state, false);
                        }

                        state.dataGlobal->BeginTimeStepFlag = true;

                        // Set the End__Flag variables to true if necessary.  Note that
                        // each flag builds on the previous level.  EndDayFlag cannot be
                        // .TRUE. unless EndHourFlag is also .TRUE., etc.  Note that the
                        // EndEnvrnFlag and the EndSimFlag cannot be set during warmup.
                        // Note also that BeginTimeStepFlag, EndTimeStepFlag, and the
                        // SubTimeStepFlags can/will be set/reset in the HVAC Manager.

                        if (state.dataGlobal->TimeStep == state.dataGlobal->TimeStepsInHour) {
                            state.dataGlobal->EndHourFlag = true;
                            if (state.dataGlobal->HourOfDay == 24) {
                                state.dataGlobal->EndDayFlag = true;
                                if (!state.dataGlobal->WarmupFlag && (state.dataGlobal->DayOfSim == state.dataGlobal->NumOfDayInEnvrn)) {
                                    state.dataGlobal->EndEnvrnFlag = true;
                                }
                            }
                        }

                        Weather::ManageWeather(state);

                        ExteriorEnergyUse::ManageExteriorEnergyUse(state);

                        HeatBalanceManager::ManageHeatBalance(state);

                        state.dataGlobal->BeginHourFlag = false;
                        state.dataGlobal->BeginDayFlag = false;
                        state.dataGlobal->BeginEnvrnFlag = false;
                        state.dataGlobal->BeginSimFlag = false;

                    } // TimeStep loop

                    state.dataGlobal->PreviousHour = state.dataGlobal->HourOfDay;

                } // ... End hour loop.
                if (state.dataSQLiteProcedures->sqlite) {
                    if (state.dataSysVars->ReportDuringHVACSizingSimulation) {
                        state.dataSQLiteProcedures->sqlite->sqliteCommit(); // one transaction per day
                    } else {
                        state.dataSQLiteProcedures->sqlite->sqliteRollback(); // Cancel transaction
                    }
                }
            } // ... End day loop.

        } // ... End environment loop.

        if (ErrorsFound) {
            ShowFatalError(state, "Error condition occurred.  Previous Severe Errors cause termination.");
        }

        hvacSizingSimulationManager->PostProcessLogs();

        hvacSizingSimulationManager->ProcessCoincidentPlantSizeAdjustments(state, HVACSizingIterCount);

        hvacSizingSimulationManager->RedoKickOffAndResize(state);

        if (!hvacSizingSimulationManager->plantCoinAnalyRequestsAnotherIteration) {
            // jump out of for loop, or change for to a while
            break;
        }

        hvacSizingSimulationManager->sizingLogger.IncrementSizingPeriodSet();

    } // End HVAC Sizing Iteration loop

    state.dataGlobal->WarmupFlag = false;
    state.dataGlobal->DoOutputReporting = true;
    state.dataGlobal->DoingHVACSizingSimulations = false;
    hvacSizingSimulationManager.reset(); // delete/reset unique_ptr
}
} // namespace EnergyPlus
