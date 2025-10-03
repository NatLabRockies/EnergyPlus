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
#include "../Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/GroundHeatExchangers/Base.hh>
#include <EnergyPlus/GroundHeatExchangers/BoreholeSingle.hh>
#include <gtest/gtest.h>

using namespace EnergyPlus;
using namespace EnergyPlus::GroundHeatExchangers;

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_GetSingleBH)
{
    std::string const idf_objects = delimited_string({
        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.288,              !- Borehole Diameter {m}",
        "    3.0,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "    8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.0016279,          !- Pipe Thickness {m}",
        "    0.1066667;          !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:Vertical:Single,",
        "    GHE-1,              !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    0,                  !- X Location {m}",
        "    0;                  !- Y Location {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    1.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",
    });

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetGroundHeatExchangerInput(*state);

    // good object name
    constexpr std::string_view goodObjName = "GHE-1";
    auto foundObject = GLHEVertSingle::GetSingleBH(*state, std::string(goodObjName));
    ASSERT_EQ(foundObject->name, goodObjName);

    // bad object name
    ASSERT_THROW(GLHEVertSingle::GetSingleBH(*state, "BAD NAME"), std::runtime_error);

    std::string const errorString = delimited_string({
        "   ** Severe  ** Object=GroundHeatExchanger:Vertical:Single, Name=BAD NAME - not found.",
        "   **  Fatal  ** Preceding errors cause program termination",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Object=GroundHeatExchanger:Vertical:Single, Name=BAD NAME - not found.",
    });

    EXPECT_TRUE(compare_err_stream(errorString, true));
}
