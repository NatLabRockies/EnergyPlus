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

// EnergyPlus::GroundTemperatureModels Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlus/DataIPShortCuts.hh"
#include "EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh"
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, SiteShallowGroundTempTest)
{
    std::string const idf_objects = delimited_string({
        "Site:GroundTemperature:Shallow,",
        "	21.00,	!- January",
        "	22.00,	!- February",
        "	23.00,	!- March",
        "	24.00,	!- April",
        "	25.00,	!- May",
        "	26.00,	!- June",
        "	27.00,	!- July",
        "	28.00,	!- August",
        "	29.00,	!- September",
        "	30.00,	!- October",
        "	31.00,	!- November",
        "	32.00;	!- December",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto *thisModel = GroundTemp::GetGroundTempModelAndInit(*state, GroundTemp::ModelType::SiteShallow, "TEST");

    EXPECT_NEAR(21.0, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 1), 0.1);  // January
    EXPECT_NEAR(32.0, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 12), 0.1); // December
    EXPECT_NEAR(22.0, thisModel->getGroundTempAtTimeInMonths(*state, 0.0, 14), 0.1); // Feb of next year

    EXPECT_NEAR(23.0, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 6393600), 0.1);  // March 15
    EXPECT_NEAR(29.0, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 22291200), 0.1); // Sept 15
    EXPECT_NEAR(22.0, thisModel->getGroundTempAtTimeInSeconds(*state, 0.0, 35510400), 0.1); // Feb 15 of next year
}
