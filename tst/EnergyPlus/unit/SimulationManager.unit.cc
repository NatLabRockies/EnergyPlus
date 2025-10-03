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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/SimulationManager.hh>

#include <stdexcept>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, CheckThreading)
{
    std::string const idf_objects = delimited_string({
        "ProgramControl,",
        "	1;",

    });

    EXPECT_FALSE(process_idf(idf_objects, false));
    state->init_state(*state);

    std::string const error_string = delimited_string({
        "   ** Severe  ** Line: 1 Index: 14 - \"ProgramControl\" is not a valid Object Type.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, Test_PerformancePrecisionTradeoffs)
{
    std::string const idf_objects = delimited_string({
        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  PerformancePrecisionTradeoffs,",
        "    No;       ! - Use Coil Direct Solutions",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    SimulationManager::GetProjectData(*state);

    // no error message from PerformancePrecisionTradeoffs objects
    EXPECT_TRUE(compare_err_stream("", true));
}

TEST_F(EnergyPlusFixture, Test_PerformancePrecisionTradeoffs_DirectSolution_Message)
{
    // issue 7646
    std::string const idf_objects = delimited_string({
        "  PerformancePrecisionTradeoffs,",
        "     Yes; ! - Use Coil Direct Solutions",

    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    std::string const error_string = delimited_string({
        "   ** Warning ** PerformancePrecisionTradeoffs: Coil Direct Solution simulation is selected.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, Simulationmanager_bool_to_string)
{
    EXPECT_EQ(SimulationManager::bool_to_string(true), "True");
    EXPECT_EQ(SimulationManager::bool_to_string(false), "False");
}

TEST_F(EnergyPlusFixture, Simulationmanager_writeInitialPerfLogValues)
{
    state->dataStrGlobals->outputPerfLogFilePath = "eplusout_perflog.csv";

    // start with no file
    fs::remove(state->dataStrGlobals->outputPerfLogFilePath);

    // make sure the static variables are cleared
    Util::appendPerfLog(*state, "RESET", "RESET");

    // call the function to test
    SimulationManager::writeInitialPerfLogValues(*state, "MODE193");

    // force the file to be written
    Util::appendPerfLog(*state, "lastHeader", "lastValue", true);

    std::ifstream perfLogFile;
    std::stringstream perfLogStrSteam;

    perfLogFile.open(state->dataStrGlobals->outputPerfLogFilePath);
    perfLogStrSteam << perfLogFile.rdbuf();
    perfLogFile.close();
    std::string perfLogContents = perfLogStrSteam.str();

    std::string expectedContents = "Program, Version, TimeStamp,Use Coil Direct Solution,Zone Radiant Exchange Algorithm,"
                                   "Override Mode,Number of Timesteps per Hour,Minimum Number of Warmup "
                                   "Days,SuppressAllBeginEnvironmentResets,Minimum System Timestep,MaxZoneTempDiff,MaxAllowedDelTemp,lastHeader,\n" +
                                   state->dataStrGlobals->VerStringVar + ",False,ScriptF,MODE193,0,1,False,1.0,0.30,2.0000E-003,lastValue,\n";

    EXPECT_EQ(perfLogContents, expectedContents);

    // clean up the file
    fs::remove(state->dataStrGlobals->outputPerfLogFilePath);
}

TEST_F(EnergyPlusFixture, SimulationManager_OutputDebuggingData)
{
    {
        std::string const idf_objects = delimited_string({
            "  Output:DebuggingData,",
            "    No;                      !- Report Debugging Data",
        });

        EXPECT_TRUE(process_idf(idf_objects));

        EXPECT_FALSE(state->dataReportFlag->DebugOutput);
        EXPECT_FALSE(state->dataReportFlag->EvenDuringWarmup);

        // no error message from
        EXPECT_TRUE(compare_err_stream("", true));
    }

    {
        std::string const idf_objects = delimited_string({
            "  Output:DebuggingData,",
            "    Yes,                     !- Report Debugging Data",
            "    ;                        !- Report During Warmup",
        });

        state->init_state_called = false;
        EXPECT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        EXPECT_TRUE(state->dataReportFlag->DebugOutput);
        EXPECT_FALSE(state->dataReportFlag->EvenDuringWarmup);

        // no error message from
        EXPECT_TRUE(compare_err_stream("", true));
    }

    {
        std::string const idf_objects = delimited_string({
            "  Output:DebuggingData,",
            "    No,                      !- Report Debugging Data",
            "    Yes;                     !- Report During Warmup",
        });

        state->init_state_called = false;
        EXPECT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        EXPECT_FALSE(state->dataReportFlag->DebugOutput);
        EXPECT_TRUE(state->dataReportFlag->EvenDuringWarmup);

        // no error message from
        EXPECT_TRUE(compare_err_stream("", true));
    }

    // Unicity warning
    {
        std::string const idf_objects = delimited_string({
            "  Output:DebuggingData,",
            "    No,                      !- Report Debugging Data",
            "    Yes;                     !- Report During Warmup",

            "  Output:DebuggingData,",
            "    Yes,                     !- Report Debugging Data",
            "    No;                      !- Report During Warmup",
        });

        state->init_state_called = false;
        compare_err_stream_substring("", true);
        // Input processor with throw a severe, so do not use assertions
        EXPECT_FALSE(process_idf(idf_objects, false));
        state->init_state(*state);

        // Instead do it here, making sure to reset the stream
        {
            std::string const expectedError = delimited_string({
                "   ** Severe  ** <root>[Output:DebuggingData] - Object should have no more than 1 properties.",
                "   ** Warning ** Output:DebuggingData: More than 1 occurrence of this object found, only first will be used.",
            });
            EXPECT_TRUE(compare_err_stream(expectedError, true));
        }

        EXPECT_FALSE(state->dataReportFlag->DebugOutput);
        EXPECT_TRUE(state->dataReportFlag->EvenDuringWarmup);
    }
}

TEST_F(EnergyPlusFixture, SimulationManager_OutputDiagnostics_DefaultState)
{
    std::string const idf_objects = delimited_string({
        "  Output:Diagnostics;",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_FALSE(state->dataGlobal->DisplayAllWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayExtraWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedObjects);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedSchedules);
    EXPECT_FALSE(state->dataGlobal->DisplayAdvancedReportVariables);
    EXPECT_FALSE(state->dataGlobal->DisplayZoneAirHeatBalanceOffBalance);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredDetachedShading);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredAttachedShading);
    EXPECT_FALSE(state->dataSysVars->ReportDuringWarmup);
    EXPECT_FALSE(state->dataEnvrn->DisplayWeatherMissingDataWarnings);
    EXPECT_FALSE(state->dataSysVars->ReportDetailedWarmupConvergence);
    EXPECT_FALSE(state->dataSysVars->ReportDuringHVACSizingSimulation);

    // Undocumented ones, see SimulationManager_OutputDiagnostics_UndocumentedFlags
    EXPECT_FALSE(state->dataEnvrn->IgnoreSolarRadiation);
    EXPECT_FALSE(state->dataEnvrn->IgnoreBeamRadiation);
    EXPECT_FALSE(state->dataEnvrn->IgnoreDiffuseRadiation);
    EXPECT_FALSE(state->dataSysVars->DeveloperFlag);
    EXPECT_FALSE(state->dataSysVars->TimingFlag);

    // no error message from
    EXPECT_TRUE(compare_err_stream("", true));
}

TEST_F(EnergyPlusFixture, SimulationManager_OutputDiagnostics_SimpleCase)
{
    std::string const idf_objects = delimited_string({
        "  Output:Diagnostics,",
        "    DisplayAllWarnings,      !- Key 1",
        "    DisplayAdvancedReportVariables;    !- Key 2",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_TRUE(state->dataGlobal->DisplayAllWarnings);
    EXPECT_TRUE(state->dataGlobal->DisplayExtraWarnings);
    EXPECT_TRUE(state->dataGlobal->DisplayUnusedObjects);
    EXPECT_TRUE(state->dataGlobal->DisplayUnusedSchedules);
    EXPECT_TRUE(state->dataGlobal->DisplayAdvancedReportVariables);
    EXPECT_FALSE(state->dataGlobal->DisplayZoneAirHeatBalanceOffBalance);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredDetachedShading);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredAttachedShading);
    EXPECT_FALSE(state->dataSysVars->ReportDuringWarmup);
    EXPECT_FALSE(state->dataEnvrn->DisplayWeatherMissingDataWarnings);
    EXPECT_FALSE(state->dataSysVars->ReportDetailedWarmupConvergence);
    EXPECT_FALSE(state->dataSysVars->ReportDuringHVACSizingSimulation);

    // no error message from
    EXPECT_TRUE(compare_err_stream("", true));
}

TEST_F(EnergyPlusFixture, SimulationManager_OutputDiagnostics_AllKeys)
{
    // All keys
    std::string const idf_objects = delimited_string({
        "  Output:Diagnostics,",
        "    DisplayAllWarnings,",
        "    DisplayExtraWarnings,",
        "    DisplayUnusedSchedules,",
        "    DisplayUnusedObjects,",
        "    DisplayAdvancedReportVariables,",
        "    DisplayZoneAirHeatBalanceOffBalance,",
        "    DoNotMirrorDetachedShading,",
        "    DoNotMirrorAttachedShading,",
        "    DisplayWeatherMissingDataWarnings,",
        "    ReportDuringWarmup,",
        "    ReportDetailedWarmupConvergence,",
        "    ReportDuringHVACSizingSimulation;",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_TRUE(state->dataGlobal->DisplayAllWarnings);
    EXPECT_TRUE(state->dataGlobal->DisplayExtraWarnings);
    EXPECT_TRUE(state->dataGlobal->DisplayUnusedObjects);
    EXPECT_TRUE(state->dataGlobal->DisplayUnusedSchedules);
    EXPECT_TRUE(state->dataGlobal->DisplayAdvancedReportVariables);
    EXPECT_TRUE(state->dataGlobal->DisplayZoneAirHeatBalanceOffBalance);
    EXPECT_FALSE(state->dataReportFlag->MakeMirroredDetachedShading);
    EXPECT_FALSE(state->dataReportFlag->MakeMirroredAttachedShading);
    EXPECT_TRUE(state->dataSysVars->ReportDuringWarmup);
    EXPECT_TRUE(state->dataEnvrn->DisplayWeatherMissingDataWarnings);
    EXPECT_TRUE(state->dataSysVars->ReportDetailedWarmupConvergence);
    EXPECT_TRUE(state->dataSysVars->ReportDuringHVACSizingSimulation);

    // no error message from
    EXPECT_TRUE(compare_err_stream("", true));
}

TEST_F(EnergyPlusFixture, SimulationManager_OutputDiagnostics_Unicity)
{
    std::string const idf_objects = delimited_string({
        "  Output:Diagnostics,",
        "    DisplayAdvancedReportVariables;    !- Key 1",

        "  Output:Diagnostics,",
        "    DisplayAllWarnings;      !- Key 1",
    });

    compare_err_stream_substring("", true);
    // Input processor will throw a severe, so do not use assertions
    EXPECT_FALSE(process_idf(idf_objects, false));
    state->init_state(*state);

    // Instead do it here, making sure to reset the stream
    {
        std::string const expectedError = delimited_string({
            "   ** Severe  ** <root>[Output:Diagnostics] - Object should have no more than 1 properties.",
            "   ** Warning ** Output:Diagnostics: More than 1 occurrence of this object found, only first will be used.",
        });
        EXPECT_TRUE(compare_err_stream(expectedError, true));
    }

    EXPECT_FALSE(state->dataGlobal->DisplayAllWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayExtraWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedObjects);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedSchedules);
    EXPECT_TRUE(state->dataGlobal->DisplayAdvancedReportVariables); // Only first object has been processed
    EXPECT_FALSE(state->dataGlobal->DisplayZoneAirHeatBalanceOffBalance);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredDetachedShading);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredAttachedShading);
    EXPECT_FALSE(state->dataSysVars->ReportDuringWarmup);
    EXPECT_FALSE(state->dataEnvrn->DisplayWeatherMissingDataWarnings);
    EXPECT_FALSE(state->dataSysVars->ReportDetailedWarmupConvergence);
    EXPECT_FALSE(state->dataSysVars->ReportDuringHVACSizingSimulation);
}

TEST_F(EnergyPlusFixture, SimulationManager_OutputDiagnostics_UndocumentedFlags)
{
    std::string const idf_objects = delimited_string({
        "  Output:Diagnostics,",
        "    IgnoreSolarRadiation,",
        "    IgnoreBeamRadiation,",
        "    IgnoreDiffuseRadiation,",
        "    DeveloperFlag,",
        "    TimingFlag;",
    });

    // This will throw a warning in InputProcessor since these aren't supported keys, so do not use assertions
    EXPECT_FALSE(process_idf(idf_objects, false));
    state->init_state(*state);

    const std::string expected_warning = delimited_string({
        "   ** Severe  ** <root>[Output:Diagnostics][Output:Diagnostics 1][diagnostics][0][key] - \"IgnoreSolarRadiation\" - Failed to match against "
        "any enum values.",
        "   ** Severe  ** <root>[Output:Diagnostics][Output:Diagnostics 1][diagnostics][1][key] - \"IgnoreBeamRadiation\" - Failed to match against "
        "any enum values.",
        "   ** Severe  ** <root>[Output:Diagnostics][Output:Diagnostics 1][diagnostics][2][key] - \"IgnoreDiffuseRadiation\" - Failed to match "
        "against any enum values.",
        "   ** Severe  ** <root>[Output:Diagnostics][Output:Diagnostics 1][diagnostics][3][key] - \"DeveloperFlag\" - Failed to match against any "
        "enum values.",
        "   ** Severe  ** <root>[Output:Diagnostics][Output:Diagnostics 1][diagnostics][4][key] - \"TimingFlag\" - Failed to match against any enum "
        "values.",
    });
    EXPECT_TRUE(compare_err_stream(expected_warning, true));

    EXPECT_FALSE(state->dataGlobal->DisplayAllWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayExtraWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedObjects);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedSchedules);
    EXPECT_FALSE(state->dataGlobal->DisplayAdvancedReportVariables);
    EXPECT_FALSE(state->dataGlobal->DisplayZoneAirHeatBalanceOffBalance);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredDetachedShading);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredAttachedShading);
    EXPECT_FALSE(state->dataSysVars->ReportDuringWarmup);
    EXPECT_FALSE(state->dataEnvrn->DisplayWeatherMissingDataWarnings);
    EXPECT_FALSE(state->dataSysVars->ReportDetailedWarmupConvergence);
    EXPECT_FALSE(state->dataSysVars->ReportDuringHVACSizingSimulation);

    // Still works
    EXPECT_TRUE(state->dataEnvrn->IgnoreSolarRadiation);
    EXPECT_TRUE(state->dataEnvrn->IgnoreBeamRadiation);
    EXPECT_TRUE(state->dataEnvrn->IgnoreDiffuseRadiation);
    EXPECT_TRUE(state->dataSysVars->DeveloperFlag);
    EXPECT_TRUE(state->dataSysVars->TimingFlag);

    // no error message from
    EXPECT_TRUE(compare_err_stream("", true));
}

// We want to avoid cryptic failures such as this one: "[json.exception.out_of_range.403] key 'key' not found"
TEST_F(EnergyPlusFixture, SimulationManager_OutputDiagnostics_HasEmpty)
{
    std::string const idf_objects = delimited_string({
        "  Output:Diagnostics,",
        "    ,                                  !- Key 1",
        "    DisplayAdvancedReportVariables;    !- Key 2",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_FALSE(state->dataGlobal->DisplayAllWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayExtraWarnings);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedObjects);
    EXPECT_FALSE(state->dataGlobal->DisplayUnusedSchedules);

    EXPECT_TRUE(state->dataGlobal->DisplayAdvancedReportVariables);

    EXPECT_FALSE(state->dataGlobal->DisplayZoneAirHeatBalanceOffBalance);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredDetachedShading);
    EXPECT_TRUE(state->dataReportFlag->MakeMirroredAttachedShading);
    EXPECT_FALSE(state->dataSysVars->ReportDuringWarmup);
    EXPECT_FALSE(state->dataEnvrn->DisplayWeatherMissingDataWarnings);
    EXPECT_FALSE(state->dataSysVars->ReportDetailedWarmupConvergence);
    EXPECT_FALSE(state->dataSysVars->ReportDuringHVACSizingSimulation);

    // Warning that an empty key was entered
    std::string const expectedError = delimited_string({
        "   ** Warning ** Output:Diagnostics: empty key found, consider removing it to avoid this warning.",
    });
    EXPECT_TRUE(compare_err_stream(expectedError, true));
}

TEST_F(EnergyPlusFixture, SimulationManager_HVACSizingSimulationChoiceTest)
{
    std::string const idf_objects = delimited_string({
        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    Yes,                     !- Run Simulation for Weather File Run Periods",
        "    Yes;                     !- Do HVAC Sizing Simulation for Sizing Periods",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_FALSE(state->dataGlobal->DoHVACSizingSimulation); // flag gets reset because Do Plant is NO
    // get a default value
    EXPECT_EQ(state->dataGlobal->HVACSizingSimMaxIterations, 0); // this is no longer set because
}

TEST_F(EnergyPlusFixture, Test_SimulationControl_ZeroSimulation)
{
    // Test for #9191
    std::string const idf_objects = delimited_string({
        "SimulationControl,",
        "  No,                       !- Do Zone Sizing Calculation",
        "  No,                       !- Do System Sizing Calculation",
        "  Yes,                      !- Do Plant Sizing Calculation",
        "  No,                       !- Run Simulation for Sizing Periods",
        "  No,                       !- Run Simulation for Weather File Run Periods",
        "  No,                       !- Do HVAC Sizing Simulation for Sizing Periods",
        "  1;                        !- Maximum Number of HVAC Sizing Simulation Passes",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    ASSERT_THROW(SimulationManager::CheckForMisMatchedEnvironmentSpecifications(*state), std::runtime_error);
    // no error message from PerformancePrecisionTradeoffs objects
    //
    std::string const error_string = delimited_string({
        "   ** Severe  ** All elements of SimulationControl are set to \"No\". No simulations can be done. Program terminates.",
        "   **  Fatal  ** Program terminates due to preceding conditions.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=All elements of SimulationControl are set to \"No\". No simulations can be done. Program terminates.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, Test_SimulationControl_PureLoadCalc)
{
    // Test for #9191
    std::string const idf_objects = delimited_string({
        "SimulationControl,",
        "  Yes,                      !- Do Zone Sizing Calculation",
        "  Yes,                      !- Do System Sizing Calculation",
        "  No,                       !- Do Plant Sizing Calculation",
        "  No,                       !- Run Simulation for Sizing Periods",
        "  No,                       !- Run Simulation for Weather File Run Periods",
        "  No,                       !- Do HVAC Sizing Simulation for Sizing Periods",
        "  1;                        !- Maximum Number of HVAC Sizing Simulation Passes",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_NO_THROW(SimulationManager::CheckForMisMatchedEnvironmentSpecifications(*state));
    // no error message from PerformancePrecisionTradeoffs objects
    //
    std::string const error_string = delimited_string({
        "   ** Warning ** \"Run Simulation for Sizing Periods\" and \"Run Simulation for Weather File Run Periods\" are both set to \"No\". "
        "No simulations will be performed, and most input will not be read.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, SimulationManager_ReportLoopConnectionsTest)
{
    // Unit Test for Defect #11061: Test Error Message (modified level of message)
    state->dataBranchNodeConnections->NumCompSets = 1;
    state->dataBranchNodeConnections->CompSets.allocate(1);
    state->dataBranchNodeConnections->CompSets(1).ParentObjectType = DataLoopNode::ConnectionObjectType::WaterHeaterMixed;
    state->dataBranchNodeConnections->CompSets(1).ComponentObjectType = DataLoopNode::ConnectionObjectType::WaterHeaterMixed;
    state->dataBranchNodeConnections->CompSets(1).CName = "WaterHeaterMixed1";
    state->dataBranchNodeConnections->CompSets(1).InletNodeName = "MixedWaterHeater1Inlet";
    state->dataBranchNodeConnections->CompSets(1).OutletNodeName = "MixedWaterHeater1Outlet";
    state->dataSimulationManager->WarningOut = false;
    state->dataBranchNodeConnections->CompSets(1).Description = "UNDEFINED";

    EXPECT_THROW(EnergyPlus::SimulationManager::ReportLoopConnections(*state), std::runtime_error);

    std::string const error_string = delimited_string({
        "   ** Severe  ** Potential Node Connection Error for object WATERHEATER:MIXED, name=WaterHeaterMixed1",
        "   **   ~~~   **   Node Types are still UNDEFINED -- See Branch/Node Details file for further information",
        "   **   ~~~   **   Inlet Node : MixedWaterHeater1Inlet",
        "   **   ~~~   **   Outlet Node: MixedWaterHeater1Outlet",
        "   ************* There was 1 node connection error noted.",
        "   **  Fatal  ** Please see severe error(s) and correct either the branch nodes or the component nodes so that they match.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Potential Node Connection Error for object WATERHEATER:MIXED, name=WaterHeaterMixed1",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, SimulationManager_PlantSizingInputTest)
{
    // Unit Test for Defect #10797: Check for Bad Input (avoid a hard crash)
    std::string const idf_objects = delimited_string({
        "SimulationControl,",
        "  No,                       !- Do Zone Sizing Calculation",
        "  No,                       !- Do System Sizing Calculation",
        "  Yes,                      !- Do Plant Sizing Calculation",
        "  No,                       !- Run Simulation for Sizing Periods",
        "  No,                       !- Run Simulation for Weather File Run Periods",
        "  Yes,                      !- Do HVAC Sizing Simulation for Sizing Periods",
        "  2;                        !- Maximum Number of HVAC Sizing Simulation Passes",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    EXPECT_THROW(SimulationManager::GetProjectData(*state), std::runtime_error);

    EXPECT_TRUE(compare_err_stream_substring(
        "GetProjectData: No Sizing:Plant object entered when the Do HVAC Sizing Simulation and Do Plant Sizing are both YES", true));
}
