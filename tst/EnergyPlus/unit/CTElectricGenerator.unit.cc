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

// EnergyPlus::CTElectricGenerator Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/CTElectricGenerator.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus::CTElectricGenerator;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, CTElectricGenerator_Fueltype)
{
    std::string const idf_objects = delimited_string({
        "Generator:CombustionTurbine,",
        "  Solar Turbine,           !- Name",
        "  30000,                   !- Rated Power Output {W}",
        "  GT gen Electric Node,    !- Electric Circuit Node Name",
        "  0.15,                    !- Minimum Part Load Ratio",
        "  1.0,                     !- Maximum Part Load Ratio",
        "  0.65,                    !- Optimum Part Load Ratio",
        "  BG PL Based Fuel Input Curve,  !- Part Load Based Fuel Input Curve Name",
        "  BG Temp Based Fuel Input Curve,  !- Temperature Based Fuel Input Curve Name",
        "  BG Exhaust Flow Curve,   !- Exhaust Flow Curve Name",
        "  BG PL Based Exhaust Temp Curve,  !- Part Load Based Exhaust Temperature Curve Name",
        "  BG Temp Based Exhaust Temp Curve,  !- Temperature Based Exhaust Temperature Curve Name",
        "  BG Tur Recovery Lube Heat Curve,  !- Heat Recovery Lube Energy Curve Name",
        "  0.01907045,              !- Coefficient 1 of U-Factor Times Area Curve",
        "  0.9,                     !- Coefficient 2 of U-Factor Times Area Curve",
        "  0.00000504,              !- Maximum Exhaust Flow per Unit of Power Output {(kg/s)/W}",
        "  150,                     !- Design Minimum Exhaust Temperature {C}",
        "  25,                      !- Design Air Inlet Temperature {C}",
        "  43500,                   !- Fuel Higher Heating Value {kJ/kg}",
        "  0.0,                     !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  ,                        !- Heat Recovery Inlet Node Name",
        "  ,                        !- Heat Recovery Outlet Node Name",
        "  NaturalGas,              !- Fuel Type",
        "  ,                        !- Heat Recovery Maximum Temperature {C}",
        "  ;                        !- Outdoor Air Inlet Node Name",

        "Curve:Quadratic,",
        "  BG PL Based Fuel Input Curve,  !- Name",
        "  9.41,                    !- Coefficient1 Constant",
        "  -9.48,                   !- Coefficient2 x",
        "  4.32,                    !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        "Curve:Quadratic,",
        "  BG Temp Based Fuel Input Curve,  !- Name",
        "  1.0044,                  !- Coefficient1 Constant",
        "  -0.0008,                 !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  -30,                     !- Minimum Value of x",
        "  +30;                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "  BG Exhaust Flow Curve,   !- Name",
        "  0.05,                    !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        "Curve:Quadratic,",
        "  BG PL Based Exhaust Temp Curve,  !- Name",
        "  450,                     !- Coefficient1 Constant",
        "  0,                       !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        "Curve:Quadratic,",
        "  BG Temp Based Exhaust Temp Curve,  !- Name",
        "  1.005,                   !- Coefficient1 Constant",
        "  0.0018,                  !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  -30,                     !- Minimum Value of x",
        "  +30;                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "  BG Tur Recovery Lube Heat Curve,  !- Name",
        "  0.223,                   !- Coefficient1 Constant",
        "  -0.4,                    !- Coefficient2 x",
        "  0.2286,                  !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state); // Calls GetCurveInput
    GetCTGeneratorInput(*state);

    EXPECT_ENUM_EQ(state->dataCTElectricGenerator->CTGenerator(1).FuelType, Constant::eFuel::NaturalGas);
}

} // namespace EnergyPlus
