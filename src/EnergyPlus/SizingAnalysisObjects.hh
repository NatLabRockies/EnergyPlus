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

#ifndef SizingAnalysisObjects_hh_INCLUDED
#define SizingAnalysisObjects_hh_INCLUDED

// C++ Headers
#include <map>
#include <string>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

class SystemTimestepObject
{
public:
    Real64 CurMinuteStart = 0.0;   // minutes at beginning of system timestep
    Real64 CurMinuteEnd = 0.0;     // minutes at end of system timestep
    Real64 TimeStepDuration = 0.0; // in fractional hours, length of timestep
    Real64 LogDataValue = 0.0;     // raw hard value, obtained from pointer
    int stStepsIntoZoneStep = 0;
};

class ZoneTimestepObject
{
public:
    Constant::KindOfSim kindOfSim = Constant::KindOfSim::Invalid;
    int envrnNum = 0;
    int dayOfSim = 0; // since start of simulation
    int hourOfDay = 0;
    int ztStepsIntoPeriod = 0;     // count of zone timesteps into period
    Real64 stepStartMinute = 0.0;  // minutes at beginning of zone timestep
    Real64 stepEndMinute = 0.0;    // minutes at end of zone timestep
    Real64 timeStepDuration = 0.0; // in fractional hours, length of timestep
    Real64 logDataValue = 0.0;
    Real64 runningAvgDataValue = 0.0;
    bool hasSystemSubSteps = false;
    int numSubSteps = 0;
    std::vector<SystemTimestepObject> subSteps; // nested object array for system timesteps inside here.

    ZoneTimestepObject( // full constructor
        Constant::KindOfSim kindSim,
        int environmentNum,
        int daySim,
        int hourDay,
        int timeStep,
        Real64 timeStepDurat,
        int numOfTimeStepsPerHour);

    ZoneTimestepObject(); // default constructor
};

class SizingLog
{
public:
    SizingLog(double &rVariable);

    int NumOfEnvironmentsInLogSet;
    int NumOfDesignDaysInLogSet;
    int NumberOfSizingPeriodsInLogSet;
    std::map<int, int> ztStepCountByEnvrnMap; // key is the seed original Environment number, or index in the Environment Structure
    std::map<int, int>
        envrnStartZtStepIndexMap; // key is the seed original Environment number, produces index in zone step vector where that period begins
    std::map<int, int> newEnvrnToSeedEnvrnMap; // key is the new HVAC sim environment number, produces Seed environment number
    int NumOfStepsInLogSet;                    // sum of all zone timestep steps in log
    int timeStepsInAverage;                    // breadth back in time for running average, zone timesteps
    Real64 &p_rVariable;                       // reference to variable being logged

    std::vector<ZoneTimestepObject> ztStepObj; // will be sized to the sum of all steps, eg. timesteps in hour * 24 hours * 2 design days.

    void FillZoneStep(ZoneTimestepObject tmpztStepStamp);

    void FillSysStep(ZoneTimestepObject tmpztStepStamp, SystemTimestepObject tmpSysStepStamp);

    void AverageSysTimeSteps();

    void ProcessRunningAverage();

    ZoneTimestepObject GetLogVariableDataMax(EnergyPlusData &state);

    Real64 GetLogVariableDataAtTimestamp(ZoneTimestepObject tmpztStepStamp);

    void ReInitLogForIteration();

    void SetupNewEnvironment(int const seedEnvrnNum, int const newEnvrnNum);

private:
    int GetSysStepZtStepIndex(const ZoneTimestepObject tmpztStepStamp);

    int GetZtStepIndex(const ZoneTimestepObject tmpztStepStamp);
};

class SizingLoggerFramework
{
public:
    std::vector<SizingLog> logObjs;

    int SetupVariableSizingLog(EnergyPlusData &state, Real64 &rVariable, int stepsInAverage);

    ZoneTimestepObject PrepareZoneTimestepStamp(EnergyPlusData &state);

    void UpdateSizingLogValuesZoneStep(EnergyPlusData &state);

    void UpdateSizingLogValuesSystemStep(EnergyPlusData &state);

    void SetupSizingLogsNewEnvironment(EnergyPlusData &state);

    void IncrementSizingPeriodSet();

private:
    int NumOfLogs = 0;
};

class PlantCoinicidentAnalysis
{
public:
    // this object collects data and methods for analyzing coincident sizing for a single plant loop
    int plantLoopIndex = 0;         // index in plant loop data structure.
    int supplySideInletNodeNum = 0; // node index for supply side inlet node
    int plantSizingIndex = 0;
    int numTimeStepsInAvg = 0;
    ZoneTimestepObject newFoundMassFlowRateTimeStamp; // result for max mass flow, as a timestamp object
    Real64 peakMdotCoincidentReturnTemp;
    Real64 peakMdotCoincidentDemand;
    bool anotherIterationDesired = false;
    int supplyInletNodeFlow_LogIndex; // loop flow rate index for vector of log objects in the logger framework
    int supplyInletNodeTemp_LogIndex; // loop return temperature index for vector of log objects in the logger framework
    // variables related to loop demand
    int loopDemand_LogIndex; // Loop demand load index for vector of log objects in the logger framework
    bool peakDemandAndFlowMismatch;
    ZoneTimestepObject NewFoundMaxDemandTimeStamp; // result for max loop demand, as a timestamp object

    Real64 peakDemandReturnTemp;
    Real64 peakDemandMassFlow;

    PlantCoinicidentAnalysis(std::string loopName, int loopIndex, int nodeNum, Real64 density, Real64 cp, int numStepsInAvg, int sizingIndex);

    void ResolveDesignFlowRate(EnergyPlusData &state, int const HVACSizingIterCount);

private:
    std::string name;                     // name of analysis object
    Real64 newAdjustedMassFlowRate = 0.0; // with sizing factor included...
    Real64 newFoundMassFlowRate = 0.0;
    Real64 significantNormalizedChange = 0.005; // criteria for if sizing algorithm yield a change large enough worth making another pass.
    Real64 densityForSizing = 0.0;
    Real64 specificHeatForSizing = 0.0;
    Real64 previousVolDesignFlowRate = 0.0;
    Real64 newVolDesignFlowRate = 0.0;

    bool CheckTimeStampForNull(ZoneTimestepObject testStamp);
};

} // namespace EnergyPlus

#endif
