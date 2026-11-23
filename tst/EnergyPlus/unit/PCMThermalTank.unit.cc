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

// EnergyPlus::WaterThermalTank Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/PCMThermalStorage.hh>
#include <EnergyPlus/ScheduleManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::PCMStorage;

TEST_F(EnergyPlusFixture, PCMThermalTankUseEnergy)
{
    // Inline IDF (from your PCM/Tank inputs). Matches the WH test flow:
    // 1) inline IDF  2) process_idf  3) state->init_state  4) Get...Input  5) asserts on struct
    std::string const idf_objects = delimited_string({"Schedule:Constant, ALWAYS_ON,,1.0;",

                                                      // Base material the PCM property attaches to
                                                      "Material,",
                                                      "  PCM_Material,           !- Name",
                                                      "  Smooth,                 !- Roughness",
                                                      "  0.10,                   !- Thickness {m}",
                                                      "  0.20,                   !- Conductivity {W/m-K}",
                                                      "  800.0,                  !- Density {kg/m3}",
                                                      "  1000.0,                 !- Specific Heat {J/kg-K}",
                                                      "  0.90,                   !- Thermal Absorptance",
                                                      "  0.70,                   !- Solar Absorptance",
                                                      "  0.70;                   !- Visible Absorptance",

                                                      // Your hysteresis block (as in your IDF)
                                                      "MaterialProperty:PhaseChangeHysteresis,",
                                                      "  PCM_Material,            !- Name",
                                                      "  60000,                   !- Latent Heat during the Entire Phase Change Process {J/kg}",
                                                      "  0.5,                     !- Liquid State Thermal Conductivity {W/m-K}",
                                                      "  800,                     !- Liquid State Density {kg/m3}",
                                                      "  2000,                    !- Liquid State Specific Heat {J/kg-K}",
                                                      "  2,                       !- High Temperature Difference of Melting Curve {deltaC}",
                                                      "  50,                      !- Peak Melting Temperature {C}",
                                                      "  2,                       !- Low Temperature Difference of Melting Curve {deltaC}",
                                                      "  0.5,                     !- Solid State Thermal Conductivity {W/m-K}",
                                                      "  800,                     !- Solid State Density {kg/m3}",
                                                      "  2000,                    !- Solid State Specific Heat {J/kg-K}",
                                                      "  2,                       !- High Temperature Difference of Freezing Curve {deltaC}",
                                                      "  35,                      !- Peak Freezing Temperature {C}",
                                                      "  2;                       !- Low Temperature Difference of Freezing Curve {deltaC}",

                                                      // The PCM storage object under test (your nodes & values)
                                                      "ThermalStorage:PCM,",
                                                      "  PCM Tank,                !- Name",
                                                      "  ALWAYS_ON,               !- Availability Schedule Name",
                                                      "  MICROCHP SENERTECH Water Inlet Node,  !- Plant Side Inlet Node Name",
                                                      "  MICROCHP SENERTECH Water Outlet Node, !- Plant Side Outlet Node Name",
                                                      "  PCM Inlet Node,          !- Use Side Inlet Node Name",
                                                      "  PCM Outlet Node,         !- Use Side Outlet Node Name",
                                                      "  PCM_Material,            !- PCM Material Name",
                                                      "  400,                     !- Tank Capacity {kg}",
                                                      "  25,                      !- Heat Loss Rate {W}",
                                                      "  autosize,                !- Use Side Design Flow Rate {m3/s}",
                                                      "  autosize;                !- Plant Side Design Flow Rate {m3/s}"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetHysteresisData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // Input routine (fatal on bad input; consistent with WH tests that just call Get...Input)
    PCMStorage::GetPCMStorageInput(*state);

    // Assert on the module’s singleton data (like WH tests assert on tank structs)
    auto &pcm = PCMStorageData::instance();

    // Names / basic numerics
    EXPECT_EQ("PCM TANK", pcm.Name);
    EXPECT_DOUBLE_EQ(400.0, pcm.TankCapacity);
    EXPECT_DOUBLE_EQ(25.0, pcm.HeatLossRate);

    // Node handles > 0 (no NodeInputManager poking; we assert on the component struct like WH tests do)
    EXPECT_GT(pcm.PlantSideInletNode, 0);
    EXPECT_GT(pcm.PlantSideOutletNode, 0);
    EXPECT_GT(pcm.UseSideInletNode, 0);
    EXPECT_GT(pcm.UseSideOutletNode, 0);

    // Material link and derived properties present
    ASSERT_NE(pcm.PCMmat, nullptr);
    EXPECT_GT(pcm.LatentHeat, 0.0);
    EXPECT_GT(pcm.MeltingTemp, 0.0);
    EXPECT_GT(pcm.FreezingTemp, 0.0);
    EXPECT_GT(pcm.SpecificHeat, 0.0);

    // Stop here — WH minimal tests end after input parsing & sanity asserts.
    // (No Init/Calculate: that would require plant context and is outside this simple parse test.)
}
