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

// EnergyPlus::ZoneHVAC:Dehumidifier:DX Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, ZoneDehumidifierDX_MissingHumidistatSevere)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Living_Unit1,             !- Name",
        "  0,                        !- Direction of Relative North {deg}",
        "  0,                        !- X Origin {m}",
        "  0,                        !- Y Origin {m}",
        "  0,                        !- Z Origin {m}",
        "  1,                        !- Type",
        "  1,                        !- Multiplier",
        "  autocalculate,            !- Ceiling Height {m}",
        "  autocalculate;            !- Volume {m3}",

        "ZoneHVAC:Dehumidifier:DX,",
        "  North Zone Dehumidifier,  !- Name",
        "  ,                         !- Availability Schedule Name",
        "  Zone3DehumidifierInlet,   !- Air Inlet Node Name",
        "  Dehumidifier Outlet Node, !- Air Outlet Node Name",
        "  50.16,                    !- Rated Water Removal {L/day}",
        "  3.412,                    !- Rated Energy Factor {L/kWh}",
        "  0.12036,                  !- Rated Air Flow Rate {m3/s}",
        "  ZoneDehumidWaterRemoval,  !- Water Removal Curve Name",
        "  ZoneDehumidEnergyFactor,  !- Energy Factor Curve Name",
        "  ZoneDehumidPLFFPLR,       !- Part Load Fraction Correlation Curve Name",
        "  10.0,                     !- Minimum Dry-Bulb Temperature for Dehumidifier Operation {C}",
        "  32.0,                     !- Maximum Dry-Bulb Temperature for Dehumidifier Operation {C}",
        "  0.0;                      !- Off-Cycle Parasitic Electric Load {W}",

        "Curve:Biquadratic,",
        "  ZoneDehumidWaterRemoval,  !- Name",
        "  1.0,                      !- Coefficient1 Constant",
        "  0.0,                      !- Coefficient2 x",
        "  0.0,                      !- Coefficient3 x**2",
        "  0.0,                      !- Coefficient4 y",
        "  0.0,                      !- Coefficient5 y**2",
        "  0.0,                      !- Coefficient6 x*y",
        "  0.0,                      !- Minimum Value of x",
        "  100.0,                    !- Maximum Value of x",
        "  0.0,                      !- Minimum Value of y",
        "  100.0;                    !- Maximum Value of y",

        "Curve:Biquadratic,",
        "  ZoneDehumidEnergyFactor,  !- Name",
        "  1.0,                      !- Coefficient1 Constant",
        "  0.0,                      !- Coefficient2 x",
        "  0.0,                      !- Coefficient3 x**2",
        "  0.0,                      !- Coefficient4 y",
        "  0.0,                      !- Coefficient5 y**2",
        "  0.0,                      !- Coefficient6 x*y",
        "  0.0,                      !- Minimum Value of x",
        "  100.0,                    !- Maximum Value of x",
        "  0.0,                      !- Minimum Value of y",
        "  100.0;                    !- Maximum Value of y",

        "Curve:Quadratic,",
        "  ZoneDehumidPLFFPLR,       !- Name",
        "  1.0,                      !- Coefficient1 Constant",
        "  0.0,                      !- Coefficient2 x",
        "  0.0,                      !- Coefficient3 x**2",
        "  0.0,                      !- Minimum Value of x",
        "  1.0;                      !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool errorsFound = false;
    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    ASSERT_EQ(1, state->dataGlobal->NumOfZones);

    ZoneDehumidifier::GetZoneDehumidifierInput(*state);
    ASSERT_EQ(1u, state->dataZoneDehumidifier->ZoneDehumid.size());

    state->dataEnvrn->StdBaroPress = DataEnvironment::StdPressureSeaLevel;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataGlobal->DoingInputProcessing = false;
    state->dataHVACGlobal->DoSetPointTest = true;
    state->dataHVACGlobal->SetPointErrorFlag = false;
    ASSERT_EQ(0, state->dataHeatBal->Zone(1).humidityControlZoneIndex);

    ZoneDehumidifier::InitZoneDehumidifier(*state, 1, 1);

    EXPECT_TRUE(state->dataHVACGlobal->SetPointErrorFlag);
    EXPECT_TRUE(compare_err_stream_substring("Missing ZoneControl:Humidistat for", false));
    EXPECT_TRUE(compare_err_stream_substring("ZoneHVAC:Dehumidifier:DX: NORTH ZONE DEHUMIDIFIER", false));
    EXPECT_TRUE(compare_err_stream_substring("Zone: LIVING_UNIT1", true));
}

} // namespace EnergyPlus
