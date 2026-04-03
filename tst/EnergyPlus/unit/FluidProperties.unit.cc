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

// EnergyPlus::ZoneEquipmentManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/FluidProperties.hh>

#include <cmath>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, FluidProperties_GetDensityGlycol)
{
    std::string const idf_objects = delimited_string({"FluidProperties:GlycolConcentration,",
                                                      "  GLHXFluid,       !- Name",
                                                      "  PropyleneGlycol, !- Glycol Type",
                                                      "  ,                !- User Defined Glycol Name",
                                                      "  0.3;             !- Glycol Concentration",
                                                      " "});

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());
    state->init_state(*state);

    auto *fluid = Fluid::GetGlycol(*state, "GLHXFLUID");

    EXPECT_NEAR(1037.89, fluid->getDensity(*state, -35.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1037.89, fluid->getDensity(*state, -15.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1034.46, fluid->getDensity(*state, 5.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1030.51, fluid->getDensity(*state, 15.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1026.06, fluid->getDensity(*state, 25.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1021.09, fluid->getDensity(*state, 35.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1015.62, fluid->getDensity(*state, 45.0, "UnitTest"), 0.01);
    EXPECT_NEAR(1003.13, fluid->getDensity(*state, 65.0, "UnitTest"), 0.01);
    EXPECT_NEAR(988.60, fluid->getDensity(*state, 85.0, "UnitTest"), 0.01);
    EXPECT_NEAR(972.03, fluid->getDensity(*state, 105.0, "UnitTest"), 0.01);
    EXPECT_NEAR(953.41, fluid->getDensity(*state, 125.0, "UnitTest"), 0.01);
}

TEST_F(EnergyPlusFixture, FluidProperties_GetSpecificHeatGlycol)
{
    std::string const idf_objects = delimited_string({"FluidProperties:GlycolConcentration,",
                                                      "  GLHXFluid,       !- Name",
                                                      "  PropyleneGlycol, !- Glycol Type",
                                                      "  ,                !- User Defined Glycol Name",
                                                      "  0.3;             !- Glycol Concentration",
                                                      " "});

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    state->init_state(*state);
    auto *fluid = Fluid::GetGlycol(*state, "GLHXFLUID");

    EXPECT_NEAR(3779, fluid->getSpecificHeat(*state, -35.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3779, fluid->getSpecificHeat(*state, -15.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3807, fluid->getSpecificHeat(*state, 5.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3834, fluid->getSpecificHeat(*state, 15.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3862, fluid->getSpecificHeat(*state, 25.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3889, fluid->getSpecificHeat(*state, 35.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3917, fluid->getSpecificHeat(*state, 45.0, "UnitTest"), 0.01);
    EXPECT_NEAR(3972, fluid->getSpecificHeat(*state, 65.0, "UnitTest"), 0.01);
    EXPECT_NEAR(4027, fluid->getSpecificHeat(*state, 85.0, "UnitTest"), 0.01);
    EXPECT_NEAR(4082, fluid->getSpecificHeat(*state, 105.0, "UnitTest"), 0.01);
    EXPECT_NEAR(4137, fluid->getSpecificHeat(*state, 125.0, "UnitTest"), 0.01);
}

TEST_F(EnergyPlusFixture, FluidProperties_InterpValuesForGlycolConc)
{
    // Test fluid property interpolations with only one concentration
    int NumCon = 1;
    int NumTemp = 5;
    Array1D<Real64> ConData = {1.0};

    Array2D<Real64> PropData;
    PropData.allocate(NumCon, NumTemp);

    // This array contains the temperature dependent fluid property data
    // e.g. one of the types of density, specific heat, viscosity, or conductivity
    for (int i = 1; i <= NumCon; ++i) {
        for (int j = 1; j <= NumTemp; ++j) {
            // assume some varying density close to water
            PropData(i, j) = 1030.0 - 10.0 * j;
        }
    }

    Real64 ActCon = 1.0;
    Array1D<Real64> Result;

    Result.allocate(NumTemp);

    // Test interpolation for the single-concentration scenario
    Fluid::InterpValuesForGlycolConc(*state,
                                     NumCon,   // number of concentrations (dimension of raw data)
                                     NumTemp,  // number of temperatures (dimension of raw data)
                                     ConData,  // concentrations for raw data
                                     PropData, // raw property data (temperature,concentration)
                                     ActCon,   // concentration of actual fluid mix
                                     Result);  // interpolated output data at proper concentration

    EXPECT_NEAR(1020.0, Result(1), 1e-6);
    EXPECT_NEAR(1010.0, Result(2), 1e-6);
    EXPECT_NEAR(1000.0, Result(3), 1e-6);
    EXPECT_NEAR(990.0, Result(4), 1e-6);
    EXPECT_NEAR(980.0, Result(5), 1e-6);
}
