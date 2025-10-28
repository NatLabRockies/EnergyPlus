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
#include <utility>

// ObjexxFCL Headers
#include <ObjexxFCL/environment.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace DataSystemVariables {

    // MODULE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   May 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This data-only module is a repository for system (such as environment) variables that are set
    // before a run or set of runs.

    constexpr const char *DDOnlyEnvVar("DDONLY");                // Only run design days
    constexpr const char *ReverseDDEnvVar("REVERSEDD");          // Reverse DD during run
    constexpr const char *FullAnnualSimulation("FULLANNUALRUN"); // Generate annual run
    constexpr const char *cDeveloperFlag("DeveloperFlag");
    constexpr const char *cDisplayAllWarnings("DisplayAllWarnings");
    constexpr const char *cDisplayExtraWarnings("DisplayExtraWarnings");
    constexpr const char *cDisplayAdvancedReportVariables("DisplayAdvancedReportVariables");
    constexpr const char *cDisplayUnusedObjects("DisplayUnusedObjects");
    constexpr const char *cDisplayUnusedSchedules("DisplayUnusedSchedules");
    constexpr const char *cDisplayZoneAirHeatBalanceOffBalance("DisplayZoneAirHeatBalanceOffBalance");
    constexpr const char *cSortIDD("SortIDD");
    constexpr const char *cReportDuringWarmup("ReportDuringWarmup");
    constexpr const char *cReportDuringHVACSizingSimulation("REPORTDURINGHVACSIZINGSIMULATION");
    constexpr const char *cIgnoreSolarRadiation("IgnoreSolarRadiation");
    constexpr const char *cIgnoreBeamRadiation("IgnoreBeamRadiation");
    constexpr const char *cIgnoreDiffuseRadiation("IgnoreDiffuseRadiation");
    constexpr const char *cSutherlandHodgman("SutherlandHodgman");
    constexpr const char *cSlaterBarsky("SlaterBarsky");
    constexpr const char *cMinimalSurfaceVariables("CreateMinimalSurfaceVariables");
    constexpr const char *cMinimalShadowing("MinimalShadowing");
    constexpr const char *cInputPath1("epin");       // EP-Launch setting.  Full path + project name
    constexpr const char *cInputPath2("input_path"); // RunEplus.bat setting.  Full path
    constexpr const char *cProgramPath("program_path");
    constexpr const char *cTimingFlag("TimingFlag");
    constexpr const char *TrackAirLoopEnvVar("TRACK_AIRLOOP"); // To generate a file with runtime statistics
    // for each controller on each air loop
    constexpr const char *TraceAirLoopEnvVar("TRACE_AIRLOOP"); // To generate a trace file with the converged
    // solutions of all controllers on each air loop at each call to SimAirLoop()
    constexpr const char *TraceHVACControllerEnvVar("TRACE_HVACCONTROLLER"); // To generate a trace file for
    //  each individual HVAC controller with all controller iterations

    constexpr const char *MinReportFrequencyEnvVar("MINREPORTFREQUENCY"); // environment var for reporting frequency.
    constexpr const char *
        cDisplayInputInAuditEnvVar("DISPLAYINPUTINAUDIT"); // environmental variable that enables the echoing of the input file into the audit file

    constexpr const char *ciForceTimeStepEnvVar("CI_FORCE_TIME_STEP"); // environment var forcing 30 minute time steps on CI for efficiency

    fs::path CheckForActualFilePath(EnergyPlusData &state,
                                    fs::path const &originalInputFilePath, // path (or filename only) as input for object
                                    const std::string &contextString       //
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // With the Windows version, there are subfolders set and the input file names may not
        // be accurate. This searches a few folders (CurrentWorkingFolder, Program folder) to see
        // if the file can be found. (It may have been input with full path so that is checked first.)

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        fs::path foundFilePath;

        if (state.dataSysVars->firstTime) {
            state.files.audit.ensure_open(state, "CheckForActualFilePath", state.files.outputControl.audit);
            std::string tmp;

            // epin is passed from Epl-run.bat as the path to the IDF file minus its extension, so take the parent directory
            get_environment_variable(cInputPath1, tmp);
            state.dataSysVars->envinputpath1 = FileSystem::getParentDirectoryPath(fs::path(tmp));

            get_environment_variable(cInputPath2, tmp);
            state.dataSysVars->envinputpath2 = fs::path(tmp);

            get_environment_variable(cProgramPath, tmp);
            state.dataStrGlobals->ProgramPath = fs::path(tmp);
            state.dataSysVars->firstTime = false;
        }

        fs::path InputFilePath = FileSystem::makeNativePath(originalInputFilePath); // save for changing out path characters

        std::vector<std::pair<fs::path, std::string>> pathsChecked;

        const std::array<std::pair<fs::path, std::string>, 7> pathsToCheck = {
            {{InputFilePath, "Current Working Directory"},
             {state.dataStrGlobals->inputDirPath / InputFilePath, "IDF Directory"},
             {state.dataStrGlobals->exeDirectoryPath / InputFilePath, "EnergyPlus Executable Directory"},
             {state.dataSysVars->envinputpath1 / InputFilePath, R"("epin" Environment Variable)"},
             {state.dataSysVars->envinputpath2 / InputFilePath, R"("input_path" Environment Variable)"},
             // These two only tested if `TestAllPaths` is true
             {state.dataStrGlobals->CurrentWorkingFolder / InputFilePath, "INI File Directory"},
             {state.dataStrGlobals->ProgramPath / InputFilePath, R"("program", "dir" from INI File)"}}};

        std::size_t numPathsToTest = (state.dataSysVars->TestAllPaths) ? pathsToCheck.size() : pathsToCheck.size() - 2;

        for (std::size_t i = 0; i < numPathsToTest; ++i) {
            if (FileSystem::fileExists(pathsToCheck[i].first)) {
                foundFilePath = pathsToCheck[i].first;
                print(state.files.audit, "found ({})={}\n", pathsToCheck[i].second, FileSystem::getAbsolutePath(foundFilePath));

                return foundFilePath;
            } else {
                std::pair<fs::path, std::string> currentPath(FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(pathsToCheck[i].first)),
                                                             pathsToCheck[i].second);
                bool found = false;
                for (auto const &path : pathsChecked) {
                    if (path.first == currentPath.first) {
                        found = true;
                    }
                }
                if (!found) {
                    pathsChecked.push_back(currentPath);
                }
                print(state.files.audit, "not found ({})={}\n", pathsToCheck[i].second, FileSystem::getAbsolutePath(pathsToCheck[i].first));
            }
        }

        // If we get here, we didn't find the file
        ShowSevereError(state, format("{}\"{}\" not found.", contextString, originalInputFilePath));
        ShowContinueError(state, "  Paths searched:");
        for (auto &path : pathsChecked) {
            ShowContinueError(state, format("    {}: \"{}\"", path.second, path.first));
        }

        return foundFilePath;
    }

    void processEnvironmentVariables(EnergyPlusData &state)
    {

        std::string cEnvValue;

        get_environment_variable(DDOnlyEnvVar, cEnvValue);
        state.dataSysVars->DDOnly = env_var_on(cEnvValue); // Yes or True
        if (state.dataGlobal->DDOnlySimulation) {
            state.dataSysVars->DDOnly = true;
        }

        get_environment_variable(ReverseDDEnvVar, cEnvValue);
        state.dataSysVars->ReverseDD = env_var_on(cEnvValue); // Yes or True

        get_environment_variable(FullAnnualSimulation, cEnvValue);
        state.dataSysVars->FullAnnualRun = env_var_on(cEnvValue); // Yes or True
        if (state.dataGlobal->AnnualSimulation) {
            state.dataSysVars->FullAnnualRun = true;
        }

        get_environment_variable(cDisplayAllWarnings, cEnvValue);
        state.dataGlobal->DisplayAllWarnings = env_var_on(cEnvValue); // Yes or True
        if (state.dataGlobal->DisplayAllWarnings) {
            state.dataGlobal->DisplayAllWarnings = true;
            state.dataGlobal->DisplayExtraWarnings = true;
            state.dataGlobal->DisplayUnusedSchedules = true;
            state.dataGlobal->DisplayUnusedObjects = true;
        }

        get_environment_variable(cDisplayExtraWarnings, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->DisplayExtraWarnings = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cDisplayUnusedObjects, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->DisplayUnusedObjects = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cDisplayUnusedSchedules, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->DisplayUnusedSchedules = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cDisplayZoneAirHeatBalanceOffBalance, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->DisplayZoneAirHeatBalanceOffBalance = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cDisplayAdvancedReportVariables, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->DisplayAdvancedReportVariables = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cReportDuringWarmup, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->ReportDuringWarmup = env_var_on(cEnvValue); // Yes or True
        }
        if (state.dataSysVars->ReverseDD) {
            state.dataSysVars->ReportDuringWarmup = false; // force to false for ReverseDD runs
        }

        get_environment_variable(cReportDuringWarmup, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->ReportDuringWarmup = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cReportDuringHVACSizingSimulation, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->ReportDuringHVACSizingSimulation = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cIgnoreSolarRadiation, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataEnvrn->IgnoreSolarRadiation = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cMinimalSurfaceVariables, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->CreateMinimalSurfaceVariables = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cSortIDD, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->SortedIDD = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(MinReportFrequencyEnvVar, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->MinReportFrequency = cEnvValue; // turned into value later
        }

        get_environment_variable(cDeveloperFlag, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->DeveloperFlag = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cIgnoreBeamRadiation, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataEnvrn->IgnoreBeamRadiation = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cIgnoreDiffuseRadiation, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataEnvrn->IgnoreDiffuseRadiation = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cSutherlandHodgman, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->SutherlandHodgman = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cSlaterBarsky, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->SlaterBarsky = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cMinimalShadowing, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->lMinimalShadowing = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cTimingFlag, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->TimingFlag = env_var_on(cEnvValue); // Yes or True
        }

        // Initialize env flags for air loop simulation debugging
        get_environment_variable(TrackAirLoopEnvVar, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->TrackAirLoopEnvFlag = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(TraceAirLoopEnvVar, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->TraceAirLoopEnvFlag = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(TraceHVACControllerEnvVar, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->TraceHVACControllerEnvFlag = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(cDisplayInputInAuditEnvVar, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataGlobal->DisplayInputInAudit = env_var_on(cEnvValue); // Yes or True
        }

        get_environment_variable(ciForceTimeStepEnvVar, cEnvValue);
        if (!cEnvValue.empty()) {
            state.dataSysVars->ciForceTimeStep = env_var_on(cEnvValue); // Yes or True
        }
    }

} // namespace DataSystemVariables

} // namespace EnergyPlus
