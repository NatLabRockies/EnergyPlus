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
#include <map>
#include <string>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/SizingAnalysisObjects.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

ZoneTimestepObject::ZoneTimestepObject()
{
    kindOfSim = Constant::KindOfSim::Invalid;
    envrnNum = 0;
    dayOfSim = 0;
    hourOfDay = 0;
    ztStepsIntoPeriod = 0;
    stepStartMinute = 0.0;
    stepEndMinute = 0.0;
    timeStepDuration = 0.0;
}

ZoneTimestepObject::ZoneTimestepObject(
    Constant::KindOfSim kindSim, // kind of simulation
    int environmentNum,          // index in Environment data structure, usually Weather::Envrn
    int daySim,                  // days into simulation period, usually DataGlobals::DayOfSim
    int hourDay,                 // hour into day, 1-24, filled by DataGlobals::HourOfDay
    int timeStep,                // time steps into hour, filled by DataGlobals::TimeStep
    Real64 timeStepDurat,        // duration of timestep in fractional hours, usually OutputProcessor::TimeValue( ZoneIndex ).TimeStep
    int numOfTimeStepsPerHour    // timesteps in each hour, usually DataGlobals::NumOfTimeStepInHour
    )
    : kindOfSim(kindSim), envrnNum(environmentNum), dayOfSim(daySim), hourOfDay(hourDay),

      timeStepDuration(timeStepDurat)
{
    Real64 constexpr minutesPerHour(60.0);
    int constexpr hoursPerDay(24);

    stepEndMinute = timeStepDuration * minutesPerHour + (timeStep - 1) * timeStepDuration * minutesPerHour;

    stepStartMinute = stepEndMinute - timeStepDuration * minutesPerHour;

    if (stepStartMinute < 0.0) {
        stepStartMinute = 0.0;
        stepEndMinute = timeStepDuration * minutesPerHour;
    }

    ztStepsIntoPeriod = ((dayOfSim - 1) * (hoursPerDay * numOfTimeStepsPerHour)) +      // multiple days
                        ((hourOfDay - 1) * numOfTimeStepsPerHour) +                     // so far this day's hours
                        round((stepStartMinute / minutesPerHour) / (timeStepDuration)); // into current hour

    if (ztStepsIntoPeriod < 0) {
        ztStepsIntoPeriod = 0;
    }

    // We only expect this feature to be used with systems, so there will always be a system timestep update, at least one.
    hasSystemSubSteps = true;
    numSubSteps = 1;
    subSteps.resize(numSubSteps);
}

SizingLog::SizingLog(double &rVariable) : p_rVariable(rVariable)
{
}

int SizingLog::GetZtStepIndex(const ZoneTimestepObject tmpztStepStamp)
{

    int vecIndex;

    if (tmpztStepStamp.ztStepsIntoPeriod > 0) { // discard any negative value for safety
        vecIndex = envrnStartZtStepIndexMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]] + tmpztStepStamp.ztStepsIntoPeriod;
    } else {
        vecIndex = envrnStartZtStepIndexMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]];
    }

    // next for safety sake, constrain index to lie inside correct environment
    if (vecIndex < envrnStartZtStepIndexMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]]) {
        vecIndex = envrnStartZtStepIndexMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]]; // first step in environment
    }
    if (vecIndex > (envrnStartZtStepIndexMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]] +
                    ztStepCountByEnvrnMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]])) {
        vecIndex = envrnStartZtStepIndexMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]] +
                   ztStepCountByEnvrnMap[newEnvrnToSeedEnvrnMap[tmpztStepStamp.envrnNum]]; // last step in environment
    }
    return vecIndex;
}

void SizingLog::FillZoneStep(ZoneTimestepObject tmpztStepStamp)
{
    int index = GetZtStepIndex(tmpztStepStamp);

    ztStepObj[index].kindOfSim = tmpztStepStamp.kindOfSim;
    ztStepObj[index].envrnNum = tmpztStepStamp.envrnNum;
    ztStepObj[index].dayOfSim = tmpztStepStamp.dayOfSim;
    ztStepObj[index].hourOfDay = tmpztStepStamp.hourOfDay;
    ztStepObj[index].ztStepsIntoPeriod = tmpztStepStamp.ztStepsIntoPeriod;
    ztStepObj[index].stepStartMinute = tmpztStepStamp.stepStartMinute;
    ztStepObj[index].stepEndMinute = tmpztStepStamp.stepEndMinute;
    ztStepObj[index].timeStepDuration = tmpztStepStamp.timeStepDuration;

    ztStepObj[index].logDataValue = p_rVariable;
}

int SizingLog::GetSysStepZtStepIndex(ZoneTimestepObject tmpztStepStamp)
{
    // this method finds a zone timestep for the system timestep update to use
    // system timesteps are substeps inside a zone timestep, but are updated
    // before the zone step has been called.
    // the zone timestamp passed in is now accurate, not lagged, so this is simpler

    int znStepIndex = GetZtStepIndex(tmpztStepStamp);

    // safety checks for range
    if (znStepIndex >= NumOfStepsInLogSet) {
        znStepIndex = NumOfStepsInLogSet - 1;
    }
    if (znStepIndex < 0) {
        znStepIndex = 0;
    }

    return znStepIndex;
}

void SizingLog::FillSysStep(ZoneTimestepObject tmpztStepStamp, SystemTimestepObject tmpSysStepStamp)
{

    int newNumSubSteps;
    Real64 constexpr MinutesPerHour(60.0);

    int ztIndex = GetSysStepZtStepIndex(tmpztStepStamp);

    if (ztStepObj[ztIndex].hasSystemSubSteps) {

        int oldNumSubSteps = ztStepObj[ztIndex].numSubSteps;
        newNumSubSteps = round(tmpztStepStamp.timeStepDuration / tmpSysStepStamp.TimeStepDuration);
        if (newNumSubSteps != oldNumSubSteps) {
            ztStepObj[ztIndex].subSteps.resize(newNumSubSteps);
            ztStepObj[ztIndex].numSubSteps = newNumSubSteps;
        }
    } else {
        newNumSubSteps = round(tmpztStepStamp.timeStepDuration / tmpSysStepStamp.TimeStepDuration);
        ztStepObj[ztIndex].subSteps.resize(newNumSubSteps);
        ztStepObj[ztIndex].numSubSteps = newNumSubSteps;
        ztStepObj[ztIndex].hasSystemSubSteps = true;
    }

    // figure out which index this substep needs to go into
    Real64 ZoneStepStartMinutes = tmpztStepStamp.stepStartMinute;

    tmpSysStepStamp.stStepsIntoZoneStep =
        round((((tmpSysStepStamp.CurMinuteStart - ZoneStepStartMinutes) / MinutesPerHour) / tmpSysStepStamp.TimeStepDuration));

    if ((tmpSysStepStamp.stStepsIntoZoneStep >= 0) && (tmpSysStepStamp.stStepsIntoZoneStep < ztStepObj[ztIndex].numSubSteps)) {
        ztStepObj[ztIndex].subSteps[tmpSysStepStamp.stStepsIntoZoneStep] = tmpSysStepStamp;
        ztStepObj[ztIndex].subSteps[tmpSysStepStamp.stStepsIntoZoneStep].LogDataValue = p_rVariable;
    } else {
        ztStepObj[ztIndex].subSteps[0] = tmpSysStepStamp;
        ztStepObj[ztIndex].subSteps[0].LogDataValue = p_rVariable;
    }
}

void SizingLog::AverageSysTimeSteps()
{
    Real64 RunningSum;

    for (auto &zt : ztStepObj) {
        if (zt.numSubSteps > 0) {
            RunningSum = 0.0;
            for (auto const &SysT : zt.subSteps) {
                RunningSum += SysT.LogDataValue;
            }
            zt.logDataValue = RunningSum / double(zt.numSubSteps);
        }
    }
}

void SizingLog::ProcessRunningAverage()
{
    Real64 RunningSum = 0.0;
    Real64 divisor = double(timeStepsInAverage);

    std::map<int, int>::iterator end = ztStepCountByEnvrnMap.end();
    for (std::map<int, int>::iterator itr = ztStepCountByEnvrnMap.begin(); itr != end; ++itr) {
        for (int i = 0; i < itr->second; ++i) { // next inner loop over zone timestep steps

            if (timeStepsInAverage > 0) {
                RunningSum = 0.0;
                for (int j = 0; j < timeStepsInAverage; ++j) { //
                    if ((i - j) < 0) {
                        RunningSum += ztStepObj[envrnStartZtStepIndexMap[itr->first]].logDataValue; // just use first value to fill early steps
                    } else {
                        RunningSum += ztStepObj[((i - j) + envrnStartZtStepIndexMap[itr->first])].logDataValue;
                    }
                }
                ztStepObj[(i + envrnStartZtStepIndexMap[itr->first])].runningAvgDataValue = RunningSum / divisor;
            }
        }
    }
}

ZoneTimestepObject SizingLog::GetLogVariableDataMax(EnergyPlusData &state)
{
    ZoneTimestepObject tmpztStepStamp;
    Real64 MaxVal = 0.0;

    if (!ztStepObj.empty()) {
        tmpztStepStamp = ztStepObj[0];
    }

    for (auto const &zt : ztStepObj) {
        if (zt.envrnNum > 0 && zt.kindOfSim != Constant::KindOfSim::Invalid && zt.runningAvgDataValue > MaxVal) {
            MaxVal = zt.runningAvgDataValue;
            tmpztStepStamp = zt;
        } else if (zt.envrnNum == 0 && zt.kindOfSim == Constant::KindOfSim::Invalid) { // null timestamp, problem to fix
            ShowWarningMessage(state, "GetLogVariableDataMax: null timestamp in log");
        }
    }
    return tmpztStepStamp;
}

Real64 SizingLog::GetLogVariableDataAtTimestamp(ZoneTimestepObject tmpztStepStamp)
{
    int const index = GetZtStepIndex(tmpztStepStamp);
    return ztStepObj[index].runningAvgDataValue;
}

void SizingLog::ReInitLogForIteration()
{
    ZoneTimestepObject tmpNullztStepObj;
    std::fill(ztStepObj.begin(), ztStepObj.end(), tmpNullztStepObj);
}

void SizingLog::SetupNewEnvironment(int const seedEnvrnNum, int const newEnvrnNum)
{
    newEnvrnToSeedEnvrnMap[newEnvrnNum] = seedEnvrnNum;
}

int SizingLoggerFramework::SetupVariableSizingLog(EnergyPlusData &state, Real64 &rVariable, int stepsInAverage)
{
    int constexpr HoursPerDay = 24;

    SizingLog tmpLog(rVariable);
    tmpLog.NumOfEnvironmentsInLogSet = 0;
    tmpLog.NumOfDesignDaysInLogSet = 0;
    tmpLog.NumberOfSizingPeriodsInLogSet = 0;

    // search environment structure for sizing periods
    // this is coded to occur before the additions to Environment structure that will occur to run them as HVAC Sizing sims
    for (int i = 1; i <= state.dataWeather->NumOfEnvrn; ++i) {
        if (state.dataWeather->Environment(i).KindOfEnvrn == Constant::KindOfSim::DesignDay) {
            ++tmpLog.NumOfEnvironmentsInLogSet;
            ++tmpLog.NumOfDesignDaysInLogSet;
        }
        if (state.dataWeather->Environment(i).KindOfEnvrn == Constant::KindOfSim::RunPeriodDesign) {
            ++tmpLog.NumOfEnvironmentsInLogSet;
            ++tmpLog.NumberOfSizingPeriodsInLogSet;
        }
    }

    // next fill in the count of steps into map
    for (int i = 1; i <= state.dataWeather->NumOfEnvrn; ++i) {

        if (state.dataWeather->Environment(i).KindOfEnvrn == Constant::KindOfSim::DesignDay) {
            tmpLog.ztStepCountByEnvrnMap[i] = HoursPerDay * state.dataGlobal->TimeStepsInHour;
        }
        if (state.dataWeather->Environment(i).KindOfEnvrn == Constant::KindOfSim::RunPeriodDesign) {
            tmpLog.ztStepCountByEnvrnMap[i] = HoursPerDay * state.dataGlobal->TimeStepsInHour * state.dataWeather->Environment(i).TotalDays;
        }
    }

    int stepSum = 0;
    std::map<int, int>::iterator end = tmpLog.ztStepCountByEnvrnMap.end();
    for (std::map<int, int>::iterator itr = tmpLog.ztStepCountByEnvrnMap.begin(); itr != end; ++itr) {

        tmpLog.envrnStartZtStepIndexMap[itr->first] = stepSum;
        stepSum += itr->second;
    }

    tmpLog.timeStepsInAverage = stepsInAverage;

    int VectorLength = stepSum;

    tmpLog.NumOfStepsInLogSet = VectorLength;
    tmpLog.ztStepObj.resize(VectorLength);

    logObjs.push_back(tmpLog);
    ++NumOfLogs;
    return NumOfLogs - 1;
}

void SizingLoggerFramework::SetupSizingLogsNewEnvironment(EnergyPlusData &state)
{
    using namespace Weather;

    for (auto &l : logObjs) {
        l.SetupNewEnvironment(state.dataWeather->Environment(state.dataWeather->Envrn).SeedEnvrnNum, state.dataWeather->Envrn);
    }
}

ZoneTimestepObject SizingLoggerFramework::PrepareZoneTimestepStamp(EnergyPlusData &state)
{
    // prepare current timing data once and then pass into fill routines
    // function used by both zone and system frequency log updates

    int locDayOfSim;

    if (state.dataGlobal->WarmupFlag) { // DayOfSim not okay during warmup, keeps incrementing up during warmup days
        locDayOfSim = 1;
    } else {
        locDayOfSim = state.dataGlobal->DayOfSim;
    }

    ZoneTimestepObject tmpztStepStamp( // call constructor
        state.dataGlobal->KindOfSim,
        state.dataWeather->Envrn,
        locDayOfSim,
        state.dataGlobal->HourOfDay,
        state.dataGlobal->TimeStep,
        *state.dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep,
        state.dataGlobal->TimeStepsInHour);

    return tmpztStepStamp;
}

void SizingLoggerFramework::UpdateSizingLogValuesZoneStep(EnergyPlusData &state)
{
    ZoneTimestepObject tmpztStepStamp = PrepareZoneTimestepStamp(state);

    for (auto &l : logObjs) {
        l.FillZoneStep(tmpztStepStamp);
    }
}

void SizingLoggerFramework::UpdateSizingLogValuesSystemStep(EnergyPlusData &state)
{
    Real64 constexpr MinutesPerHour = 60.0;
    ZoneTimestepObject tmpztStepStamp = PrepareZoneTimestepStamp(state);
    SystemTimestepObject tmpSysStepStamp;

    // prepare system timestep stamp
    tmpSysStepStamp.CurMinuteEnd = state.dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].CurMinute;
    if (tmpSysStepStamp.CurMinuteEnd == 0.0) {
        tmpSysStepStamp.CurMinuteEnd = MinutesPerHour;
    }
    tmpSysStepStamp.CurMinuteStart =
        tmpSysStepStamp.CurMinuteEnd - (*state.dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep) * MinutesPerHour;
    tmpSysStepStamp.TimeStepDuration = *state.dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep;

    for (auto &l : logObjs) {
        l.FillSysStep(tmpztStepStamp, tmpSysStepStamp);
    }
}

void SizingLoggerFramework::IncrementSizingPeriodSet()
{
    for (auto &l : this->logObjs) {
        l.ReInitLogForIteration();
    }
}

PlantCoinicidentAnalysis::PlantCoinicidentAnalysis(
    std::string loopName, int loopIndex, int nodeNum, Real64 density, Real64 cp, int numStepsInAvg, int sizingIndex)
{
    name = loopName;
    plantLoopIndex = loopIndex;
    supplySideInletNodeNum = nodeNum;
    densityForSizing = density;
    specificHeatForSizing = cp;
    numTimeStepsInAvg = numStepsInAvg;
    plantSizingIndex = sizingIndex;
}

void PlantCoinicidentAnalysis::ResolveDesignFlowRate(EnergyPlusData &state, int const HVACSizingIterCount)
{
    using DataSizing::GlobalCoolingSizingFactorMode;
    using DataSizing::GlobalHeatingSizingFactorMode;
    using DataSizing::LoopComponentSizingFactorMode;
    using DataSizing::NoSizingFactorMode;

    using namespace DataPlant;
    using namespace OutputReportPredefined;
    using HVAC::SmallWaterVolFlow;
    bool setNewSizes;
    Real64 sizingFac;
    Real64 normalizedChange;
    Real64 newFoundVolFlowRate;
    Real64 peakLoadCalculatedMassFlow;
    std::string chIteration;
    std::string chSetSizes;
    std::string chDemandTrapUsed;
    bool changedByDemand(false);
    bool nullStampProblem;

    // first make sure we have valid time stamps to work with
    if (CheckTimeStampForNull(newFoundMassFlowRateTimeStamp) && CheckTimeStampForNull(NewFoundMaxDemandTimeStamp)) {
        // problem, don't have valid stamp, don't have any info to report either
        nullStampProblem = true;
    } else {
        nullStampProblem = false;
    }

    previousVolDesignFlowRate = state.dataSize->PlantSizData(plantSizingIndex).DesVolFlowRate;

    if (!CheckTimeStampForNull(newFoundMassFlowRateTimeStamp) && (newFoundMassFlowRateTimeStamp.runningAvgDataValue > 0.0)) { // issue 5665, was ||
        newFoundMassFlowRate = newFoundMassFlowRateTimeStamp.runningAvgDataValue;
    } else {
        newFoundMassFlowRate = 0.0;
    }

    // step 3 calculate mdot from max load and delta T
    if ((!CheckTimeStampForNull(NewFoundMaxDemandTimeStamp) && (NewFoundMaxDemandTimeStamp.runningAvgDataValue > 0.0)) &&
        ((specificHeatForSizing * state.dataSize->PlantSizData(plantSizingIndex).DeltaT) > 0.0)) {
        peakLoadCalculatedMassFlow =
            NewFoundMaxDemandTimeStamp.runningAvgDataValue / (specificHeatForSizing * state.dataSize->PlantSizData(plantSizingIndex).DeltaT);
    } else {
        peakLoadCalculatedMassFlow = 0.0;
    }

    if (peakLoadCalculatedMassFlow > newFoundMassFlowRate) {
        changedByDemand = true;
    } else {
        changedByDemand = false;
    }
    newFoundMassFlowRate = max(newFoundMassFlowRate, peakLoadCalculatedMassFlow); // step 4, take larger of the two

    newFoundVolFlowRate = newFoundMassFlowRate / densityForSizing;

    // now apply the correct sizing factor depending on input option
    sizingFac = 1.0;
    if (state.dataSize->PlantSizData(plantSizingIndex).SizingFactorOption == NoSizingFactorMode) {
        sizingFac = 1.0;
    } else if (state.dataSize->PlantSizData(plantSizingIndex).SizingFactorOption == GlobalHeatingSizingFactorMode) {
        sizingFac = state.dataSize->GlobalHeatSizingFactor;
    } else if (state.dataSize->PlantSizData(plantSizingIndex).SizingFactorOption == GlobalCoolingSizingFactorMode) {
        sizingFac = state.dataSize->GlobalCoolSizingFactor;
    } else if (state.dataSize->PlantSizData(plantSizingIndex).SizingFactorOption == LoopComponentSizingFactorMode) {
        // multiplier used for pumps, often 1.0, from component level sizing fractions
        sizingFac = state.dataPlnt->PlantLoop(plantLoopIndex).LoopSide(LoopSideLocation::Supply).Branch(1).PumpSizFac;
    }

    newAdjustedMassFlowRate = newFoundMassFlowRate * sizingFac; // apply overall heating or cooling sizing factor

    newVolDesignFlowRate = newAdjustedMassFlowRate / densityForSizing;

    // compare threshold,
    setNewSizes = false;
    normalizedChange = 0.0;
    if (newVolDesignFlowRate > SmallWaterVolFlow && !nullStampProblem) { // do not use zero size or bad stamp data

        normalizedChange = std::abs((newVolDesignFlowRate - previousVolDesignFlowRate) / previousVolDesignFlowRate);
        if (normalizedChange > significantNormalizedChange) {
            anotherIterationDesired = true;
            setNewSizes = true;
        } else {
            anotherIterationDesired = false;
        }
    }

    if (setNewSizes) {
        // set new size values for rest of simulation
        state.dataSize->PlantSizData(plantSizingIndex).DesVolFlowRate = newVolDesignFlowRate;

        if (state.dataPlnt->PlantLoop(plantLoopIndex).MaxVolFlowRateWasAutoSized) {
            state.dataPlnt->PlantLoop(plantLoopIndex).MaxVolFlowRate = newVolDesignFlowRate;
            state.dataPlnt->PlantLoop(plantLoopIndex).MaxMassFlowRate = newAdjustedMassFlowRate;
        }
        if (state.dataPlnt->PlantLoop(plantLoopIndex).VolumeWasAutoSized) {
            // Note this calculation also appears in PlantManager::SizePlantLoop and PlantManager::ResizePlantLoopLevelSizes
            state.dataPlnt->PlantLoop(plantLoopIndex).Volume =
                state.dataPlnt->PlantLoop(plantLoopIndex).MaxVolFlowRate * state.dataPlnt->PlantLoop(plantLoopIndex).CirculationTime * 60.0;
            state.dataPlnt->PlantLoop(plantLoopIndex).Mass = state.dataPlnt->PlantLoop(plantLoopIndex).Volume * densityForSizing;
        }
    }

    // add a separate eio summary report about what happened, did demand trap get used, what were the key values.
    if (!state.dataGlobal->sizingAnalysisEioHeaderDoneOnce) {
        print(state.files.eio,
              "{}",
              "! <Plant Coincident Sizing Algorithm>,Plant Loop Name,Sizing Pass {#},Measured Mass "
              "Flow{kg/s},Measured Demand {W},Demand Calculated Mass Flow{kg/s},Sizes Changed {Yes/No},Previous "
              "Volume Flow Rate {m3/s},New Volume Flow Rate {m3/s},Demand Check Applied {Yes/No},Sizing Factor "
              "{},Normalized Change {},Specific Heat{J/kg-K},Density {kg/m3}\n");
        state.dataGlobal->sizingAnalysisEioHeaderDoneOnce = true;
    }
    chIteration = fmt::to_string(HVACSizingIterCount);
    if (setNewSizes) {
        chSetSizes = "Yes";
    } else {
        chSetSizes = "No";
    }
    if (changedByDemand) {
        chDemandTrapUsed = "Yes";
    } else {
        chDemandTrapUsed = "No";
    }

    print(state.files.eio,
          "Plant Coincident Sizing Algorithm,{},{},{:.7R},{:.2R},{:.7R},{},{:.6R},{:.6R},{},{:.4R},{:.6R},{:.4R},{:.4R}\n",
          name,
          chIteration,
          newFoundMassFlowRateTimeStamp.runningAvgDataValue,
          NewFoundMaxDemandTimeStamp.runningAvgDataValue,
          peakLoadCalculatedMassFlow,
          chSetSizes,
          previousVolDesignFlowRate,
          newVolDesignFlowRate,
          chDemandTrapUsed,
          sizingFac,
          normalizedChange,
          specificHeatForSizing,
          densityForSizing);

    // report to sizing summary table called Plant Loop Coincident Design Fluid Flow Rates

    PreDefTableEntry(state,
                     state.dataOutRptPredefined->pdchPlantSizPrevVdot,
                     state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                     previousVolDesignFlowRate,
                     6);
    PreDefTableEntry(state,
                     state.dataOutRptPredefined->pdchPlantSizMeasVdot,
                     state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                     newFoundVolFlowRate,
                     6);
    PreDefTableEntry(state,
                     state.dataOutRptPredefined->pdchPlantSizCalcVdot,
                     state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                     newVolDesignFlowRate,
                     6);

    if (setNewSizes) {
        PreDefTableEntry(state,
                         state.dataOutRptPredefined->pdchPlantSizCoincYesNo,
                         state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                         "Yes");
    } else {
        PreDefTableEntry(state,
                         state.dataOutRptPredefined->pdchPlantSizCoincYesNo,
                         state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                         "No");
    }

    if (!nullStampProblem) {
        if (!changedByDemand && !CheckTimeStampForNull(newFoundMassFlowRateTimeStamp)) { // bug fix #5665
            if (newFoundMassFlowRateTimeStamp.envrnNum > 0) {                            // protect against invalid index
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchPlantSizDesDay,
                                 state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                                 state.dataWeather->Environment(newFoundMassFlowRateTimeStamp.envrnNum).Title);
            }
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchPlantSizPkTimeDayOfSim,
                             state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                             newFoundMassFlowRateTimeStamp.dayOfSim);
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchPlantSizPkTimeHour,
                             state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                             newFoundMassFlowRateTimeStamp.hourOfDay - 1);
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchPlantSizPkTimeMin,
                             state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                             newFoundMassFlowRateTimeStamp.stepStartMinute,
                             0);
        } else if (changedByDemand && !CheckTimeStampForNull(NewFoundMaxDemandTimeStamp)) { // bug fix #5665
            if (NewFoundMaxDemandTimeStamp.envrnNum > 0) {                                  // protect against invalid index
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchPlantSizDesDay,
                                 state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                                 state.dataWeather->Environment(NewFoundMaxDemandTimeStamp.envrnNum).Title);
            }
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchPlantSizPkTimeDayOfSim,
                             state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                             NewFoundMaxDemandTimeStamp.dayOfSim);
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchPlantSizPkTimeHour,
                             state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                             NewFoundMaxDemandTimeStamp.hourOfDay - 1);
            PreDefTableEntry(state,
                             state.dataOutRptPredefined->pdchPlantSizPkTimeMin,
                             state.dataPlnt->PlantLoop(plantLoopIndex).Name + " Sizing Pass " + chIteration,
                             NewFoundMaxDemandTimeStamp.stepStartMinute,
                             0);
        }
    }
}

bool PlantCoinicidentAnalysis::CheckTimeStampForNull(ZoneTimestepObject testStamp)
{

    bool isNull = true;

    if (testStamp.envrnNum != 0) {
        isNull = false;
    }
    if (testStamp.kindOfSim != Constant::KindOfSim::Invalid) {
        isNull = false;
    }

    return isNull;
}
} // namespace EnergyPlus
