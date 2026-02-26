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
#include "../Fixtures/EnergyPlusFixture.hh"

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/GroundHeatExchangers/Base.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <gtest/gtest.h>

using namespace EnergyPlus;
using namespace EnergyPlus::GroundHeatExchangers;

TEST_F(EnergyPlusFixture, GroundHeatExchanger_GLHEC_Mode_Input_Parses)
{
    std::string const idf_objects = delimited_string({
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,             !- Name",
        "    1,                       !- Depth of Top of Borehole {m}",
        "    100,                     !- Borehole Length {m}",
        "    0.109982,                !- Borehole Diameter {m}",
        "    0.744,                   !- Grout Thermal Conductivity {W/m-K}",
        "    3.90E+06,                !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,                   !- Pipe Thermal Conductivity {W/m-K}",
        "    1.77E+06,                !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.0267,                  !- Pipe Outer Diameter {m}",
        "    0.00243,                 !- Pipe Thickness {m}",
        "    0.04556;                 !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Single,",
        "    GHE-Single-1,            !- Name",
        "    GHE-1 Props,             !- GHE:Vertical:Properties Object Name",
        "    0.0,                     !- X-Location",
        "    0.0;                     !- Y-Location",

        "GroundHeatExchanger:System,",
        "    GLHEC Test,              !- Name",
        "    GHLE Inlet,              !- Inlet Node Name",
        "    GHLE Outlet,             !- Outlet Node Name",
        "    0.0007571,               !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,                 !- Undisturbed Ground Temperature Model Name",
        "    2.423,                   !- Ground Thermal Conductivity {W/m-K}",
        "    2.343E+06,               !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                        !- GHE:Vertical:ResponseFactors Object Name",
        "    GLHECcalc,               !- g-Function Calculation Method",
        "    ,                        !- GHE:Vertical:Sizing Object Type",
        "    ,                        !- GHE:Vertical:Sizing Object Name",
        "    ,                        !- GHE:Vertical:Array Object Name",
        "    GHE-Single-1;            !- GHE:Vertical:Single Object Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    GetGroundHeatExchangerInput(*state);
    ASSERT_EQ(1u, state->dataGroundHeatExchanger->verticalGLHE.size());
    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    EXPECT_TRUE(thisGLHE.useGLHEC);
}
