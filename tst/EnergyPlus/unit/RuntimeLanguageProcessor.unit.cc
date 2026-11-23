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

// EnergyPlus::Fans Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, ERLExpression_TestExponentials)
{
    // set the program state so that errors can be thrown
    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = false;
    state->dataEMSMgr->FinishProcessingUserInput = false;

    bool errorsFound = false;

    state->dataRuntimeLang->ErlExpression.allocate(1);
    auto &erlExpression = state->dataRuntimeLang->ErlExpression(1);
    erlExpression.Operator = DataRuntimeLanguage::ErlFunc::Exp;
    erlExpression.NumOperands = 1;
    erlExpression.Operand.allocate(1);

    erlExpression.Operand(1).Number = -25;
    auto response1 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_EQ(0, response1.Number);

    erlExpression.Operand(1).Number = -20;
    auto response2 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_EQ(0, response2.Number);

    erlExpression.Operand(1).Number = -3;
    auto response3 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_NEAR(0.05, response3.Number, 0.001);

    erlExpression.Operand(1).Number = 0;
    auto response4 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_NEAR(1, response4.Number, 0.001);

    erlExpression.Operand(1).Number = 3;
    auto response5 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_FALSE(errorsFound);
    EXPECT_NEAR(20.08, response5.Number, 0.01);

    erlExpression.Operand(1).Number = 700;
    auto response6 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_EQ(0, response6.Number);

    erlExpression.Operand(1).Number = 710;
    auto response7 = RuntimeLanguageProcessor::EvaluateExpression(*state, 1, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_EQ(0, response7.Number);
}

TEST_F(EnergyPlusFixture, TestOutOfRangeAlphaFields)
{
    std::string const idf_objects = delimited_string({"EnergyManagementSystem:Sensor,",
                                                      "  EMSSensor,",
                                                      "  *,",
                                                      "  Electricity:Facility;",
                                                      "EnergyManagementSystem:Program,",
                                                      "  DummyProgram,",
                                                      "  SET N = EMSSensor;",
                                                      "EnergyManagementSystem:ProgramCallingManager,",
                                                      "  DummyManager,",
                                                      "  BeginTimestepBeforePredictor,",
                                                      "  DummyProgram;",
                                                      "EnergyManagementSystem:MeteredOutputVariable,",
                                                      "  MyLongMeteredOutputVariable,",
                                                      "  EMSSensor,",
                                                      "  ZoneTimeStep,",
                                                      "  ,",
                                                      "  Electricity,",
                                                      "  Building,",
                                                      "  ExteriorEquipment,",
                                                      "  Transformer,",
                                                      "  J;"});
    ASSERT_TRUE(process_idf(idf_objects));
    RuntimeLanguageProcessor::GetRuntimeLanguageUserInput(*state);
}
