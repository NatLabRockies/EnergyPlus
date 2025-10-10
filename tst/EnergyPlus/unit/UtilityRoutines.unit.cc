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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, RecurringWarningTest)
{

    // void ShowRecurringSevereErrorAtEnd(std::string const &Message,         // Message automatically written to "error file" at end of simulation
    //    int &MsgIndex,                      // Recurring message index, if zero, next available index is assigned
    //    Optional<Real64 const> ReportMaxOf, // Track and report the max of the values passed to this argument
    //    Optional<Real64 const> ReportMinOf, // Track and report the min of the values passed to this argument
    //    Optional<Real64 const> ReportSumOf, // Track and report the sum of the values passed to this argument
    //    std::string const &ReportMaxUnits,  // optional char string (<=15 length) of units for max value
    //    std::string const &ReportMinUnits,  // optional char string (<=15 length) of units for min value
    //    std::string const &ReportSumUnits   // optional char string (<=15 length) of units for sum value
    //)

    std::string myMessage1 = "Test message 1";
    // proper call to ShowRecurringWarningErrorAtEnd to set up new recurring warning
    int ErrIndex1 = 0;
    ShowRecurringWarningErrorAtEnd(*state, myMessage1, ErrIndex1);
    EXPECT_EQ(ErrIndex1, 1);
    EXPECT_EQ(state->dataErrTracking->RecurringErrors.size(), 1u);
    EXPECT_EQ(" ** Warning ** " + myMessage1, state->dataErrTracking->RecurringErrors(1).Message);
    EXPECT_EQ(1, state->dataErrTracking->RecurringErrors(1).Count);

    std::string myMessage2 = "Test message 2";
    // improper call to ShowRecurringWarningErrorAtEnd to set up new recurring warning
    int ErrIndex2 = 6;
    ShowRecurringWarningErrorAtEnd(*state, myMessage2, ErrIndex2);
    EXPECT_EQ(ErrIndex2, 2); // ShowRecurringWarningErrorAtEnd handles improper index and returns correct value
    EXPECT_EQ(state->dataErrTracking->RecurringErrors.size(), 2u);
    EXPECT_EQ(" ** Warning ** " + myMessage2, state->dataErrTracking->RecurringErrors(2).Message);
    EXPECT_EQ(1, state->dataErrTracking->RecurringErrors(2).Count);

    ErrIndex2 = 6;
    ShowRecurringWarningErrorAtEnd(*state, myMessage2, ErrIndex2);
    EXPECT_EQ(ErrIndex2, 2); // ShowRecurringWarningErrorAtEnd handles improper index and returns correct value
    EXPECT_EQ(state->dataErrTracking->RecurringErrors.size(), 2u);
    EXPECT_EQ(" ** Warning ** " + myMessage2, state->dataErrTracking->RecurringErrors(2).Message);
    EXPECT_EQ(2, state->dataErrTracking->RecurringErrors(2).Count);

    std::string myMessage3 = "Test message 3";
    ShowRecurringContinueErrorAtEnd(*state, myMessage3, ErrIndex1);
    // index gets updated with correct value
    EXPECT_EQ(ErrIndex1, 3);
    EXPECT_EQ(state->dataErrTracking->RecurringErrors.size(), 3u);
    EXPECT_EQ(" **   ~~~   ** " + myMessage3, state->dataErrTracking->RecurringErrors(3).Message);
    EXPECT_EQ(1, state->dataErrTracking->RecurringErrors(3).Count);

    std::string myMessage4 = "Test message 4";
    ShowRecurringSevereErrorAtEnd(*state, myMessage4, ErrIndex1);
    // index gets updated with correct value
    EXPECT_EQ(ErrIndex1, 4);
    EXPECT_EQ(state->dataErrTracking->RecurringErrors.size(), 4u);
    EXPECT_EQ(" ** Severe  ** " + myMessage4, state->dataErrTracking->RecurringErrors(4).Message);
    EXPECT_EQ(1, state->dataErrTracking->RecurringErrors(4).Count);

    // same message for different show message type (changed severe to warning) should be valid
    ShowRecurringWarningErrorAtEnd(*state, myMessage4, ErrIndex1);
    // index gets updated with correct value
    EXPECT_EQ(ErrIndex1, 5);
    EXPECT_EQ(" ** Warning ** " + myMessage4, state->dataErrTracking->RecurringErrors(5).Message);
}

TEST_F(EnergyPlusFixture, DisplayMessageTest)
{
    DisplayString(*state, "Testing");
    EXPECT_TRUE(has_cout_output(true));
    // Open six files to get unit number beyond 6 - these all get closed later by EnergyPlusFixture
    DisplayString(*state, "Testing");
    EXPECT_TRUE(has_cout_output(true));
    // repeat this one - before fix, this broke cout_stream
    EXPECT_FALSE(has_cout_output(true));
    DisplayString(*state, "Testing");
    EXPECT_TRUE(has_cout_output(true));
}

TEST_F(EnergyPlusFixture, UtilityRoutines_appendPerfLog1)
{
    state->dataStrGlobals->outputPerfLogFilePath = "eplusout_1_perflog.csv";

    // start with no file
    fs::remove(state->dataStrGlobals->outputPerfLogFilePath);

    // make sure the static variables are cleared
    Util::appendPerfLog(*state, "RESET", "RESET");

    // add headers and values
    Util::appendPerfLog(*state, "header1", "value1-1");
    Util::appendPerfLog(*state, "header2", "value1-2");
    Util::appendPerfLog(*state, "header3", "value1-3", true);

    std::ifstream perfLogFile;
    std::stringstream perfLogStrSteam;

    perfLogFile.open(state->dataStrGlobals->outputPerfLogFilePath);
    perfLogStrSteam << perfLogFile.rdbuf();
    perfLogFile.close();
    std::string perfLogContents = perfLogStrSteam.str();

    std::string expectedContents = "header1,header2,header3,\n"
                                   "value1-1,value1-2,value1-3,\n";

    EXPECT_EQ(perfLogContents, expectedContents);

    // clean up the file
    fs::remove(state->dataStrGlobals->outputPerfLogFilePath);
}

TEST_F(EnergyPlusFixture, UtilityRoutines_appendPerfLog2)
{
    // make sure the static variables are cleared
    Util::appendPerfLog(*state, "RESET", "RESET");

    state->dataStrGlobals->outputPerfLogFilePath = "eplusout_2_perflog.csv";

    // create a file for the equivalent of the previous run
    std::ofstream initPerfLogFile;
    initPerfLogFile.open(state->dataStrGlobals->outputPerfLogFilePath);
    initPerfLogFile << "header1,header2,header3,\n";
    initPerfLogFile << "value1-1,value1-2,value1-3,\n";
    initPerfLogFile.close();

    // without deleting file add headers and values again
    Util::appendPerfLog(*state, "ignored1", "value2-1");
    Util::appendPerfLog(*state, "ignored2", "value2-2");
    Util::appendPerfLog(*state, "ignored3", "value2-3", true);

    std::ifstream perfLogFile;
    std::stringstream perfLogStrSteam;

    perfLogFile.open(state->dataStrGlobals->outputPerfLogFilePath);
    perfLogStrSteam << perfLogFile.rdbuf();
    perfLogFile.close();
    std::string perfLogContents = perfLogStrSteam.str();

    std::string expectedContents = "header1,header2,header3,\n"
                                   "value1-1,value1-2,value1-3,\n"
                                   "value2-1,value2-2,value2-3,\n";

    EXPECT_EQ(perfLogContents, expectedContents);

    // clean up the file
    fs::remove(state->dataStrGlobals->outputPerfLogFilePath);
}

TEST_F(EnergyPlusFixture, UtilityRoutines_ProcessNumber)
{
    // acceptable strings
    std::string goodString{"3.14159"};
    double expectedVal{3.14159};
    bool expectedError{false};
    EXPECT_NEAR(Util::ProcessNumber(goodString, expectedError), expectedVal, 1E-5);
    EXPECT_FALSE(expectedError);

    goodString = "3.14159+E0";
    EXPECT_NEAR(Util::ProcessNumber(goodString, expectedError), expectedVal, 1E-5);
    EXPECT_FALSE(expectedError);

    goodString = "3.14159+e0";
    EXPECT_NEAR(Util::ProcessNumber(goodString, expectedError), expectedVal, 1E-5);
    EXPECT_FALSE(expectedError);

    goodString = "3.14159+D0";
    EXPECT_NEAR(Util::ProcessNumber(goodString, expectedError), expectedVal, 1E-5);
    EXPECT_FALSE(expectedError);

    goodString = "3.14159+d0";
    EXPECT_NEAR(Util::ProcessNumber(goodString, expectedError), expectedVal, 1E-5);
    EXPECT_FALSE(expectedError);

    // invalid strings
    std::string badString{"É.14159"};
    expectedVal = 0.0;
    EXPECT_NEAR(Util::ProcessNumber(badString, expectedError), expectedVal, 1E-5);
    EXPECT_TRUE(expectedError);

    badString = "3.14159É0";
    expectedVal = 0.0;
    EXPECT_NEAR(Util::ProcessNumber(badString, expectedError), expectedVal, 1E-5);
    EXPECT_TRUE(expectedError);

    badString = "3.14159 0";
    expectedVal = 0.0;
    EXPECT_NEAR(Util::ProcessNumber(badString, expectedError), expectedVal, 1E-5);
    EXPECT_TRUE(expectedError);

    // invalid argument
    badString = "E3.14159";
    expectedVal = 0.0;
    EXPECT_NEAR(Util::ProcessNumber(badString, expectedError), expectedVal, 1E-5);
    EXPECT_TRUE(expectedError);

    // out of range
    badString = "1E5000";
    expectedVal = 0.0;
    EXPECT_NEAR(Util::ProcessNumber(badString, expectedError), expectedVal, 1E-5);
    EXPECT_TRUE(expectedError);
}

TEST_F(EnergyPlusFixture, UtilityRoutines_setDesignObjectNameAndPointerTest)
{
    // Test for Defect #11111: Autosized ZoneHVAC:LowTemperatureRadiant:VariableFlow with no Design Object throws allocation error
    std::string nameResult;
    std::string expectedName;
    int ptrResult;
    int expectedPtr;
    std::string userName;
    Array1D_string userNames;
    userNames.allocate(4);
    userNames = {"First Name", "Second Name", "Third Name", "Fourth Name"};
    std::string objectType;
    std::string objectName;
    bool gotErrors;

    // Test 1: Valid input (userName matches one of the userNames)
    userName = "Second Name";
    expectedName = "Second Name";
    expectedPtr = 2;
    objectType = {"ZoneHVAC:LowTemperatureRadiant:VariableFlow"};
    objectName = {"MyVarFlowRadSys"};
    gotErrors = false;

    EnergyPlus::Util::setDesignObjectNameAndPointer(*state, nameResult, ptrResult, userName, userNames, objectType, objectName, gotErrors);
    EXPECT_FALSE(gotErrors);
    EXPECT_EQ(nameResult, expectedName);
    EXPECT_EQ(ptrResult, expectedPtr);

    // Test 2: Invalid input (userName does not match one of the userNames)
    userName = "No Name";
    expectedName = "";
    expectedPtr = 0;
    objectType = {"ZoneHVAC:Baseboard:RadiantConvective:Water"};
    objectName = {"MyWaterBB"};
    gotErrors = false;

    EnergyPlus::Util::setDesignObjectNameAndPointer(*state, nameResult, ptrResult, userName, userNames, objectType, objectName, gotErrors);
    std::string const error_stringTest2 = delimited_string({
        "   ** Severe  ** Object = ZoneHVAC:Baseboard:RadiantConvective:Water with the Name = MyWaterBB has an invalid Design Object Name = No Name.",
        "   **   ~~~   **   The Design Object Name was not found or was left blank.  This is not allowed.",
        "   **   ~~~   **   A valid Design Object Name must be provided for any ZoneHVAC:Baseboard:RadiantConvective:Water object.",
    });
    EXPECT_TRUE(compare_err_stream(error_stringTest2, true));
    EXPECT_TRUE(gotErrors);

    // Test 3: Invalid input (userName is blank)
    userName = "";
    expectedName = "";
    expectedPtr = 0;
    objectType = {"ZoneHVAC:Baseboard:RadiantConvective:Steam"};
    objectName = {"MySteamBB"};
    gotErrors = false;

    EnergyPlus::Util::setDesignObjectNameAndPointer(*state, nameResult, ptrResult, userName, userNames, objectType, objectName, gotErrors);
    std::string const error_stringTest3 = delimited_string({
        "   ** Severe  ** Object = ZoneHVAC:Baseboard:RadiantConvective:Steam with the Name = MySteamBB has an invalid Design Object Name = .",
        "   **   ~~~   **   The Design Object Name was not found or was left blank.  This is not allowed.",
        "   **   ~~~   **   A valid Design Object Name must be provided for any ZoneHVAC:Baseboard:RadiantConvective:Steam object.",
    });
    EXPECT_TRUE(compare_err_stream(error_stringTest3, true));
    EXPECT_TRUE(gotErrors);
}
