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

#include <EnergyPlus/api/runtime.h>
#include <EnergyPlus/api/state.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#    define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

int numWarnings = 0;
int oneTimeHalfway = 0;
int progressValue = 0;

void BeginNewEnvironmentHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
    issueWarning(state, "Fake Warning at new environment");
    issueSevere(state, "Fake Severe at new environment");
    issueText(state, "Just some text at the new environment");
}
void AfterNewEnvironmentWarmupCompleteHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void BeginZoneTimeStepBeforeInitHeatBalanceHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void BeginZoneTimeStepAfterInitHeatBalanceHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void BeginTimeStepBeforePredictorHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void AfterPredictorBeforeHVACManagersHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void AfterPredictorAfterHVACManagersHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void InsideSystemIterationLoopHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfZoneTimeStepBeforeZoneReportingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfZoneTimeStepAfterZoneReportingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfSystemTimeStepBeforeHVACReportingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfSystemTimeStepAfterHVACReportingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfZoneSizingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfSystemSizingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void EndOfAfterComponentGetInputHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void UnitarySystemSizingHandler(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void UserDefinedCallback(EnergyPlusState state)
{
    printf("CALLBACK: %s\n", __PRETTY_FUNCTION__);
}
void stdOutHandler(const char *message)
{
    printf("STANDARD OUTPUT CALLBACK: %s\n", message);
}

void newEnvrnHandler(EnergyPlusState state)
{
    printf("NEW ENVIRONMENT CALLBACK: Starting a new environment\n");
}

void progressHandler(int const progress)
{
    progressValue = progress;
    if (oneTimeHalfway == 0 && progress > 50) {
        printf("Were halfway there!\n");
        oneTimeHalfway = 1;
    }
}

void ShouldStopHandler(EnergyPlusState state)
{
    stopSimulation(state);
}

// void errorHandler(EnergyPlusState state, int level, const char *message)
//{
//    char *warning = strstr(message, "Warning");
//    if (warning) {
//        numWarnings++;
//    }
//}

void externalHVAC(EnergyPlusState state)
{
    printf("External HVAC called\n");
}

int main(int argc, const char *argv[])
{
    EnergyPlusState state = stateNew();
    callbackBeginNewEnvironment(state, BeginNewEnvironmentHandler);
    callbackAfterNewEnvironmentWarmupComplete(state, AfterNewEnvironmentWarmupCompleteHandler);
    callbackBeginZoneTimeStepBeforeInitHeatBalance(state, BeginZoneTimeStepBeforeInitHeatBalanceHandler);
    callbackBeginZoneTimeStepAfterInitHeatBalance(state, BeginZoneTimeStepAfterInitHeatBalanceHandler);
    callbackBeginTimeStepBeforePredictor(state, BeginTimeStepBeforePredictorHandler);
    callbackAfterPredictorBeforeHVACManagers(state, AfterPredictorBeforeHVACManagersHandler);
    callbackAfterPredictorAfterHVACManagers(state, AfterPredictorAfterHVACManagersHandler);
    callbackInsideSystemIterationLoop(state, InsideSystemIterationLoopHandler);
    callbackEndOfZoneTimeStepBeforeZoneReporting(state, EndOfZoneTimeStepBeforeZoneReportingHandler);
    callbackEndOfZoneTimeStepAfterZoneReporting(state, EndOfZoneTimeStepAfterZoneReportingHandler);
    callbackEndOfSystemTimeStepBeforeHVACReporting(state, EndOfSystemTimeStepBeforeHVACReportingHandler);
    callbackEndOfSystemTimeStepAfterHVACReporting(state, EndOfSystemTimeStepAfterHVACReportingHandler);
    callbackEndOfZoneSizing(state, EndOfZoneSizingHandler);
    callbackEndOfSystemSizing(state, EndOfSystemSizingHandler);
    callbackEndOfAfterComponentGetInput(state, EndOfAfterComponentGetInputHandler);
    callbackUnitarySystemSizing(state, UnitarySystemSizingHandler);
    callbackUserDefinedComponentModel(state, UserDefinedCallback, "Hello");
    registerProgressCallback(state, progressHandler);
    // registerErrorCallback(errorHandler);
    energyplus(state, argc, argv);
    if (numWarnings > 0) {
        printf("There were %d warnings!\n", numWarnings);
        numWarnings = 0;
    }
    if (progressValue != 100) {
        return 1;
    }
    oneTimeHalfway = 0;
    // reset and run again
    EnergyPlusState state2 = stateNew(); // stateReset(state); // note previous callbacks are cleared here
    callbackAfterNewEnvironmentWarmupComplete(state2, newEnvrnHandler);
    registerStdOutCallback(state2, stdOutHandler);
    // have fun toggling the console print status off/on/off
    setConsoleOutputState(state2, 0);
    setConsoleOutputState(state2, 1);
    setConsoleOutputState(state2, 0);
    printf("Running EnergyPlus with Console Output Muted...\n");
    energyplus(state2, argc, argv);
    printf("...and it is done.\n");
    if (numWarnings > 0) {
        printf("There were %d warnings!\n", numWarnings);
        numWarnings = 0;
    }
    // reset and run a test with an external hvac manager
    EnergyPlusState state3 = stateNew(); // stateReset(state);
    registerExternalHVACManager(state3, externalHVAC);
    energyplus(state3, argc, argv);
    // We would like to know call EnergyPlus through the C API and have it run the auxiliary tools, like readvars
    // With the Python API, we can leverage Python's introspection to find the pyenergyplus folder, and thus the E+ repo
    // For C programs that link to the API, we don't have this.
    // Of course, we do have the path to the E+ library in CMake land, but that would then require configuring this file, adding complexity
    // For now we will call the setEnergyPlusRootDirectory function to exercise the funcional interface, but not attempt anything further
    // The Python API tests will exercise the functionality of the setEnergyPlusRootDirectory implicitly
    printf("Setting EnergyPlus root directory for potential runs with auxiliary tools...\n");
    EnergyPlusState state4 = stateNew();
    setEnergyPlusRootDirectory(state4, "/path/to/EnergyPlus/Root");

    // OK, now just try to run one of the dirty states and it should fail
    int bad = energyplus(state3, argc, argv);
    if (bad == 0) {
        return 1;
    }

    // OK, so now let's run with a callback that stops the simulation
    EnergyPlusState state5 = stateNew();
    callbackBeginNewEnvironment(state, ShouldStopHandler);
    energyplus(state5, argc, argv);

    return 0;
}
