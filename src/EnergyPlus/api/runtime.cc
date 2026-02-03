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

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/api/EnergyPlusPgm.hh>
#include <EnergyPlus/api/runtime.h>

// disabling "maybe unused" warnings for all the API functions so the IDE isn't lit up with messages
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

int energyplus(EnergyPlusState state, int argc, const char *argv[])
{
    //    argv[0] = "energyplus";
    //    argv[1] = "-d";
    //    argv[2] = workingPath.string().c_str();
    //    argv[3] = "-w";
    //    argv[4] = epcomp->weatherFilePath.c_str();
    //    argv[5] = "-i";
    //    argv[6] = epcomp->iddPath.c_str();
    //    argv[7] = epcomp->idfInputPath.c_str();
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    if (!thisState->ready) {
        std::cerr << "Attempted to re-run EnergyPlus using a state that was not yet cleared, call stateReset() on this instance and try again\n";
        return 1;
    }
    thisState->ready = false;
    std::vector<std::string> args(argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)));
    return runEnergyPlusAsLibrary(*thisState, args);
}

void stopSimulation(EnergyPlusState state)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->stopSimulation = true;
}

void setConsoleOutputState(EnergyPlusState state, int outputStatus)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->printConsoleOutput = (outputStatus != 0);
}

void setEnergyPlusRootDirectory(EnergyPlusState state, const char *path)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->installRootOverride = true;
    thisState->dataStrGlobals->exeDirectoryPath = fs::path(path);
}

void issueWarning(EnergyPlusState state, const char *message)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    ShowWarningError(*thisState, message);
}
void issueSevere(EnergyPlusState state, const char *message)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    ShowSevereError(*thisState, message);
}
void issueText(EnergyPlusState state, const char *message)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    ShowContinueError(*thisState, message);
}

void registerProgressCallback([[maybe_unused]] EnergyPlusState state, std::function<void(int const)> f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->progressCallback = std::move(f);
}

void registerProgressCallback(EnergyPlusState state, void (*f)(int const))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->progressCallback = f;
}

void registerStdOutCallback([[maybe_unused]] EnergyPlusState state, std::function<void(const std::string &)> f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->messageCallback = std::move(f);
}

void registerStdOutCallback(EnergyPlusState state, void (*f)(const char *))
{
    // Extra lamba needed to go from `const std::string&` to `const char *`
    const auto stdf = [f](const std::string &message) { f(message.c_str()); };
    registerStdOutCallback(state, std::function<void(const std::string &)>(stdf));
}

void registerExternalHVACManager(EnergyPlusState state, std::function<void(EnergyPlusState)> f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->externalHVACManager = std::move(f);
}

void registerExternalHVACManager(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    thisState->dataGlobal->externalHVACManager = f;
}

void callbackBeginNewEnvironment(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginNewEnvironment, f);
}

void callbackBeginNewEnvironment(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginNewEnvironment, f);
}

void callbackBeginZoneTimestepBeforeSetCurrentWeather(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginZoneTimestepBeforeSetCurrentWeather, f);
}

void callbackBeginZoneTimestepBeforeSetCurrentWeather(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginZoneTimestepBeforeSetCurrentWeather, f);
}

void callbackAfterNewEnvironmentWarmupComplete(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp, f);
}

void callbackAfterNewEnvironmentWarmupComplete(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp, f);
}

void callbackBeginZoneTimeStepBeforeInitHeatBalance(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginZoneTimestepBeforeInitHeatBalance, f);
}

void callbackBeginZoneTimeStepBeforeInitHeatBalance(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginZoneTimestepBeforeInitHeatBalance, f);
}

void callbackBeginZoneTimeStepAfterInitHeatBalance(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginZoneTimestepAfterInitHeatBalance, f);
}

void callbackBeginZoneTimeStepAfterInitHeatBalance(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginZoneTimestepAfterInitHeatBalance, f);
}

void callbackBeginTimeStepBeforePredictor(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, f);
}

void callbackBeginTimeStepBeforePredictor(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, f);
}

void callbackAfterPredictorBeforeHVACManagers(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeforeHVACManagers, f);
}

void callbackAfterPredictorBeforeHVACManagers(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::BeforeHVACManagers, f);
}

void callbackAfterPredictorAfterHVACManagers(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::AfterHVACManagers, f);
}

void callbackAfterPredictorAfterHVACManagers(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::AfterHVACManagers, f);
}

void callbackInsideSystemIterationLoop(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::HVACIterationLoop, f);
}

void callbackInsideSystemIterationLoop(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::HVACIterationLoop, f);
}

void callbackEndOfZoneTimeStepBeforeZoneReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndZoneTimestepBeforeZoneReporting, f);
}

void callbackEndOfZoneTimeStepBeforeZoneReporting(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndZoneTimestepBeforeZoneReporting, f);
}

void callbackEndOfZoneTimeStepAfterZoneReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndZoneTimestepAfterZoneReporting, f);
}

void callbackEndOfZoneTimeStepAfterZoneReporting(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndZoneTimestepAfterZoneReporting, f);
}

void callbackEndOfSystemTimeStepBeforeHVACReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndSystemTimestepBeforeHVACReporting, f);
}

void callbackEndOfSystemTimeStepBeforeHVACReporting(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndSystemTimestepBeforeHVACReporting, f);
}

void callbackEndOfSystemTimeStepAfterHVACReporting(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndSystemTimestepAfterHVACReporting, f);
}

void callbackEndOfSystemTimeStepAfterHVACReporting(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::EndSystemTimestepAfterHVACReporting, f);
}

void callbackEndOfZoneSizing(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::ZoneSizing, f);
}

void callbackEndOfZoneSizing(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::ZoneSizing, f);
}

void callbackEndOfSystemSizing(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::SystemSizing, f);
}

void callbackEndOfSystemSizing(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::SystemSizing, f);
}

void callbackEndOfAfterComponentGetInput(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::ComponentGetInput, f);
}

void callbackEndOfAfterComponentGetInput(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::ComponentGetInput, f);
}

void callbackUserDefinedComponentModel(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f, const char *programNameInInputFile)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerUserDefinedCallback(*thisState, f, programNameInInputFile);
}

void callbackUserDefinedComponentModel(EnergyPlusState state, void (*f)(EnergyPlusState), const char *programNameInInputFile)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerUserDefinedCallback(*thisState, f, programNameInInputFile);
}

void callbackUnitarySystemSizing(EnergyPlusState state, std::function<void(EnergyPlusState)> const &f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::UnitarySystemSizing, f);
}

void callbackUnitarySystemSizing(EnergyPlusState state, void (*f)(EnergyPlusState))
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    EnergyPlus::PluginManagement::registerNewCallback(*thisState, EnergyPlus::EMSManager::EMSCallFrom::UnitarySystemSizing, f);
}

#pragma clang diagnostic pop
