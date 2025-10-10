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

// EnergyPlus::GroundTemperatureModels Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlus/DataIPShortCuts.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, XingGroundTempsModelTest)
{
    std::string const idf_objects = delimited_string({
        "Site:GroundTemperature:Undisturbed:Xing,",
        "	Test,			!- Name of object",
        "	1.08,			!- Soil Thermal Conductivity {W/m-K}",
        "	962,			!- Soil Density {kg/m3}",
        "	2576,			!- Soil Specific Heat {J/kg-K}",
        "	11.1,			!- Average Soil Surface Temperature {C}",
        "	13.4,			!- Soil Surface Temperature Amplitude 1 {deltaC}",
        "	0.7,			!- Soil Surface Temperature Amplitude 2 {deltaC}",
        "	25,			!- Phase Shift of Temperature Amplitude 1 {days}",
        "	30;			!- Phase Shift of Temperature Amplitude 2 {days}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto *thisModel = GroundTemp::GetGroundTempModelAndInit(*state, GroundTemp::ModelType::Xing, "TEST");

    EXPECT_NEAR(-1.43, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 0.0), 0.01);
    EXPECT_NEAR(2.15, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 6393600), 0.1);   // March 15
    EXPECT_NEAR(19.74, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 22291200), 0.1); // Sept 15
    EXPECT_NEAR(-2.03, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 35510400), 0.1); // Feb 15 of next year

    EXPECT_NEAR(-2.71, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 1), 0.1);  // January
    EXPECT_NEAR(23.61, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 7), 0.1);  // July
    EXPECT_NEAR(1.62, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 12), 0.1);  // December
    EXPECT_NEAR(-2.12, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 14), 0.1); // Feb of next year

    EXPECT_NEAR(11.1, thisModel->getGroundTempAtTimeInMonths(*state, 100.0, 1), 0.1); // January--deep
}

TEST_F(EnergyPlusFixture, XingGroundTempsGetInputTest)
{
    Real64 constexpr closeEnough = 0.00001;

    std::string const idf_objects = delimited_string({
        "Site:GroundTemperature:Undisturbed:Xing,",
        "    Test1,   !- Name of object",
        "    1.0,     !- Soil Thermal Conductivity {W/m-K}",
        "    1000.0,  !- Soil Density {kg/m3}",
        "    2500,    !- Soil Specific Heat {J/kg-K}",
        "    10.0,    !- Average Soil Surface Temperature {C}",
        "    12.0,    !- Soil Surface Temperature Amplitude 1 {deltaC}",
        "    1.0,     !- Soil Surface Temperature Amplitude 2 {deltaC}",
        "    25,      !- Phase Shift of Temperature Amplitude 1 {days}",
        "    30;      !- Phase Shift of Temperature Amplitude 2 {days}",
        "",
        "Site:GroundTemperature:Undisturbed:Xing,",
        "    Test2,   !- Name of object",
        "    2.0,     !- Soil Thermal Conductivity {W/m-K}",
        "    1200,    !- Soil Density {kg/m3}",
        "    2400,    !- Soil Specific Heat {J/kg-K}",
        "    11.1,    !- Average Soil Surface Temperature {C}",
        "    22.2,    !- Soil Surface Temperature Amplitude 1 {deltaC}",
        "    2.5,     !- Soil Surface Temperature Amplitude 2 {deltaC}",
        "    20,      !- Phase Shift of Temperature Amplitude 1 {days}",
        "    40;      !- Phase Shift of Temperature Amplitude 2 {days}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Tests to verify fix for Defect #10981 (overwrote the name of the object which resulted in a fatal error
    // Test 1: First object instance--make sure name and model type are read in correctly
    auto *thisModel1 = GroundTemp::GetGroundTempModelAndInit(*state, GroundTemp::ModelType::Xing, "TEST1");
    EXPECT_EQ(thisModel1->Name, "TEST1");
    EXPECT_EQ(thisModel1->modelType, GroundTemp::ModelType::Xing);

    // Test 2: Second object instance--another test to make sure name and model type are read in correctly
    auto *thisModel2 = GroundTemp::GetGroundTempModelAndInit(*state, GroundTemp::ModelType::Xing, "TEST2");
    EXPECT_EQ(thisModel2->Name, "TEST2");
    EXPECT_EQ(thisModel2->modelType, GroundTemp::ModelType::Xing);
}
