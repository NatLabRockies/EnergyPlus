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

// EnergyPlus::MicroturbineElectricGenerator Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/MicroturbineElectricGenerator.hh>

using namespace EnergyPlus::MicroturbineElectricGenerator;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, MicroturbineElectricGenerator_Fueltype)
{
    std::string const idf_objects = delimited_string({
        "Generator:MicroTurbine,",
        "  Capstone C65,            !- Name",
        "  65000,                   !- Reference Electrical Power Output {W}",
        "  29900,                   !- Minimum Full Load Electrical Power Output {W}",
        "  65000,                   !- Maximum Full Load Electrical Power Output {W}",
        "  0.29,                    !- Reference Electrical Efficiency Using Lower Heating Value",
        "  15.0,                    !- Reference Combustion Air Inlet Temperature {C}",
        "  0.00638,                 !- Reference Combustion Air Inlet Humidity Ratio {kgWater/kgDryAir}",
        "  0.0,                     !- Reference Elevation {m}",
        "  Capstone C65 Power_vs_Temp_Elev,  !- Electrical Power Function of Temperature and Elevation Curve Name",
        "  Capstone C65 Efficiency_vs_Temp,  !- Electrical Efficiency Function of Temperature Curve Name",
        "  Capstone C65 Efficiency_vs_PLR,  !- Electrical Efficiency Function of Part Load Ratio Curve Name",
        "  NaturalGas,              !- Fuel Type",
        "  50000,                   !- Fuel Higher Heating Value {kJ/kg}",
        "  45450,                   !- Fuel Lower Heating Value {kJ/kg}",
        "  300,                     !- Standby Power {W}",
        "  4500;                    !- Ancillary Power {W}",

        "Curve:Biquadratic,",
        "  Capstone C65 Power_vs_Temp_Elev,  !- Name",
        "  1.2027697,               !- Coefficient1 Constant",
        "  -9.671305E-03,           !- Coefficient2 x",
        "  -4.860793E-06,           !- Coefficient3 x**2",
        "  -1.542394E-04,           !- Coefficient4 y",
        "  9.111418E-09,            !- Coefficient5 y**2",
        "  8.797885E-07,            !- Coefficient6 x*y",
        "  -17.8,                   !- Minimum Value of x",
        "  50.0,                    !- Maximum Value of x",
        "  0.0,                     !- Minimum Value of y",
        "  3050.,                   !- Maximum Value of y",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Distance,                !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "  Capstone C65 Efficiency_vs_Temp,  !- Name",
        "  1.0402217,               !- Coefficient1 Constant",
        "  -0.0017314,              !- Coefficient2 x",
        "  -6.497040E-05,           !- Coefficient3 x**2",
        "  5.133175E-07,            !- Coefficient4 x**3",
        "  -20.0,                   !- Minimum Value of x",
        "  50.0,                    !- Maximum Value of x",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "  Capstone C65 Efficiency_vs_PLR,  !- Name",
        "  0.215290,                !- Coefficient1 Constant",
        "  2.561463,                !- Coefficient2 x",
        "  -3.24613,                !- Coefficient3 x**2",
        "  1.497306,                !- Coefficient4 x**3",
        "  0.03,                    !- Minimum Value of x",
        "  1.0;                     !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state); // Calls GetCurveInput

    state->dataIPShortCut->cAlphaArgs(1) = "Capstone C65";
    state->dataIPShortCut->cCurrentModuleObject = "Generator:MicroTurbine";
    GetMTGeneratorInput(*state);

    EXPECT_ENUM_EQ(state->dataMircoturbElectGen->MTGenerator(1).FuelType, Constant::eFuel::NaturalGas);
}

} // namespace EnergyPlus
