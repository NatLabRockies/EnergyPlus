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

// EnergyPlus::OutputReportTabular Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ChillerIndirectAbsorption.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::ChillerIndirectAbsorption;

TEST_F(EnergyPlusFixture, ChillerIndirectAbsorption_GetInput)
{
    std::string const idf_objects = delimited_string({
        "Chiller:Absorption:Indirect,",
        "  Big Chiller,             !- Name",
        "  10000,                   !- Nominal Capacity {W}",
        "  150,                     !- Nominal Pumping Power {W}",
        "  Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name",
        "  Big Chiller Outlet Node, !- Chilled Water Outlet Node Name",
        "  Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "  Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "  0.15,                    !- Minimum Part Load Ratio",
        "  1.0,                     !- Maximum Part Load Ratio",
        "  0.65,                    !- Optimum Part Load Ratio",
        "  35.0,                    !- Design Condenser Inlet Temperature {C}",
        "  10.0,                    !- Condenser Inlet Temperature Lower Limit {C}",
        "  5.0,                     !- Chilled Water Outlet Temperature Lower Limit {C}",
        "  0.0011,                  !- Design Chilled Water Flow Rate {m3/s}",
        "  0.0011,                  !- Design Condenser Water Flow Rate {m3/s}",
        "  LeavingSetpointModulated,!- Chiller Flow Mode",
        "  SteamUseFPLR,            !- Generator Heat Input Function of Part Load Ratio Curve Name",
        "  PumpUseFPLR,             !- Pump Electric Input Function of Part Load Ratio Curve Name",
        "  AbsorberSteamInletNode,  !- Generator Inlet Node Name",
        "  AbsorberSteamOutletNode, !- Generator Outlet Node Name",
        "  CAPfCOND,                !- Capacity Correction Function of Condenser Temperature Curve Name",
        "  CAPfEVAP,                !- Capacity Correction Function of Chilled Water Temperature Curve Name",
        "  ,                        !- Capacity Correction Function of Generator Temperature Curve Name",
        "  SteamFCondTemp,          !- Generator Heat Input Correction Function of Condenser Temperature Curve Name",
        "  SteamFEvapTemp,          !- Generator Heat Input Correction Function of Chilled Water Temperature Curve Name",
        "  Steam,                   !- Generator Heat Source Type",
        "  autosize,                !- Design Generator Fluid Flow Rate {m3/s}",
        "  30.0,                    !- Temperature Lower Limit Generator Inlet {C}",
        "  2.0,                     !- Degree of Subcooling in Steam Generator {C}",
        "  12.0;                    !- Degree of Subcooling in Steam Condensate Loop {C}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->DoWeathSim = true;

    GetIndirectAbsorberInput(*state);

    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber.size(), 1u);
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).Name, "BIG CHILLER");
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).NomCap, 10000.);
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).NomPumpPower, 150.);
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).MinPartLoadRat, 0.15);
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).MaxPartLoadRat, 1.00);
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).OptPartLoadRat, 0.65);
    EXPECT_EQ(state->dataChillerIndirectAbsorption->IndirectAbsorber(1).LoopSubcool, 12.0);
}
