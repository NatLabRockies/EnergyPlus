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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/PluginManager.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, TestTrendVariable)
{

    // this file isn't included in the gtest source unless LINK_WITH_PYTHON is ON

    // create a plugin manager instance
    EnergyPlus::PluginManagement::PluginManager pluginManager = EnergyPlus::PluginManagement::PluginManager(*state);

    // first create a plugin variable
    pluginManager.addGlobalVariable(*state, "my_var");
    int globalVarIndex = EnergyPlus::PluginManagement::PluginManager::getGlobalVariableHandle(*state, "my_var", true);
    EXPECT_EQ(0, globalVarIndex);

    // now create a trend variable to track it
    size_t const numValues = 4;
    state->dataPluginManager->trends.emplace_back(*state, "TREND_VAR", numValues, globalVarIndex);
    int trendVarIndex = EnergyPlus::PluginManagement::PluginManager::getTrendVariableHandle(*state, "trend_var");
    EXPECT_EQ(0, trendVarIndex);

    // initially it should be filled with zeroes
    EXPECT_EQ(numValues, pluginManager.getTrendVariableHistorySize(*state, trendVarIndex));
    for (size_t i = 0; i < numValues; i++) {
        EXPECT_DOUBLE_EQ(0.0, pluginManager.getTrendVariableValue(*state, trendVarIndex, i));
    }

    // now pretend to run through a few simulation time steps, setting the value a few times and updating the trend
    std::vector<Real64> fakeValues = {3.14, 2.78, 12.0};
    for (int i = 0; i < 3; i++) {
        EnergyPlus::PluginManagement::PluginManager::setGlobalVariableValue(*state, globalVarIndex, fakeValues[i]);
        EnergyPlus::PluginManagement::PluginManager::updatePluginValues(*state);
    }

    // now check the values at the end, it should still be zero at the oldest (fourth) item, and 12.0 at the recent
    EXPECT_NEAR(fakeValues[2], pluginManager.getTrendVariableValue(*state, trendVarIndex, 0), 0.001);
    EXPECT_NEAR(fakeValues[1], pluginManager.getTrendVariableValue(*state, trendVarIndex, 1), 0.001);
    EXPECT_NEAR(fakeValues[0], pluginManager.getTrendVariableValue(*state, trendVarIndex, 2), 0.001);
    EXPECT_DOUBLE_EQ(0.0, pluginManager.getTrendVariableValue(*state, trendVarIndex, 3));
}

TEST_F(EnergyPlusFixture, MultiplePluginVariableObjects)
{
    std::string const idf_objects =
        ("PythonPlugin:Variables, Variables1, VariableA, VariableB;  PythonPlugin:Variables, Variables2, VariableA, VariableC;");
    ASSERT_TRUE(process_idf(idf_objects));
    // Constructing a plugin manager instance requires a plugin to be found in input, otherwise it exits early.
    // Adding a plugin to input is beyond scope here.  We'll just test that this passes input and move on.
    // In the future maybe we can find a clever way to test the next lines
    // PluginManagement::PluginManager p{*this->state};
    // EXPECT_EQ(p.maxGlobalVariableIndex, 2);
    // EXPECT_TRUE(compare_err_stream("   ** Warning ** Found duplicate variable name in PythonPLugin:Variables objects, ignoring: \"VariableA\"\n"));
}
} // namespace EnergyPlus
