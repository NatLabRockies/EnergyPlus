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

// EnergyPlus::TranspiredCollector init bug fix test

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/TranspiredCollector.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataHeatBalSurface;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SurfaceGeometry;
using namespace EnergyPlus::TranspiredCollector;
using namespace EnergyPlus::HeatBalanceManager;

TEST_F(EnergyPlusFixture, TranspiredCollectors_InitTranspiredCollectorTest)
{
    // Issue #6082
    bool ErrorsFound = false;
    int UTSCNum(1);

    std::string const idf_objects = delimited_string({

        "  Zone,",
        "    ZN1_S_Space_1,           !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0.000,                   !- X Origin {m}",
        "    0.000,                   !- Y Origin {m}",
        "    0.000,                   !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    3.000,                   !- Ceiling Height {m}",
        "    644.812;                 !- Volume {m3}",

        "  Material,",
        "    4 in Prefab Stone Wall,  !- Name",
        "    Smooth,                  !- Roughness",
        "    0.1016,                  !- Thickness {m}",
        "    0.858,                   !- Conductivity {W/m-K}",
        "    1968,                    !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",

        "  Construction,",
        "    Ext-wall,                !- Name",
        "    4 in Prefab Stone Wall;  !- Outside Layer",

        "  BuildingSurface:Detailed,",
        "    ZN1_ExtWallSouth_1,      !- Name",
        "    wall,                    !- Surface Type",
        "    Ext-Wall,                !- Construction Name",
        "    ZN1_S_Space_1,           !- Zone Name",
        "    ,                        !- Space Name",
        "    OtherSideConditionsModel,!- Outside Boundary Condition",
        "    UTSC OSCM 1,             !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.500,                   !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000,0.000,3.000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000,0.000,1.500,       !- X,Y,Z ==> Vertex 2 {m}",
        "    50.000,0.000,1.500,      !- X,Y,Z ==> Vertex 3 {m}",
        "    50.000,0.000,3.000;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    ZN1_ExtWallSouth_2,      !- Name",
        "    wall,                    !- Surface Type",
        "    Ext-Wall,                !- Construction Name",
        "    ZN1_S_Space_1,           !- Zone Name",
        "    ,                        !- Space Name",
        "    OtherSideConditionsModel,!- Outside Boundary Condition",
        "    UTSC OSCM 1,             !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.500,                   !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000,0.000,1.500,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000,0.000,0.000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    50.000,0.000,0.000,      !- X,Y,Z ==> Vertex 3 {m}",
        "    50.000,0.000,1.500;      !- X,Y,Z ==> Vertex 4 {m}",

        "  SurfaceProperty:OtherSideConditionsModel,",
        "    UTSC OSCM 1,             !- Name",
        "    GapConvectionRadiation;  !- Type of Modeling",

        "  Schedule:Compact,",
        "    HeatingAvailSched,       !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    ShopFreeHeatingSetpoints,!- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,21.0;       !- Field 3",

        "  SolarCollector:UnglazedTranspired,",
        "    OFFICE OA UTSC,          !- Name",
        "    UTSC OSCM 1,             !- Boundary Conditions Model Name",
        "    HeatingAvailSched,       !- Availability Schedule Name",
        "    InletNodeName,           !- Inlet Node Name",
        "    OutletNodeName,          !- Outlet Node Name",
        "    OutletNodeName,          !- Setpoint Node Name",
        "    ZN1_S_Space_1,           !- Zone Node Name",
        "    ShopFreeHeatingSetpoints,!- Free Heating Setpoint Schedule Name",
        "    0.001,                   !- Diameter of Perforations in Collector {m}",
        "    0.020,                   !- Distance Between Perforations in Collector {m}",
        "    0.9,                     !- Thermal Emissivity of Collector Surface {dimensionless}",
        "    0.9,                     !- Solar Absorbtivity of Collector Surface {dimensionless}",
        "    4.0,                     !- Effective Overall Height of Collector",
        "    0.1,                     !- Effective Gap Thickness of Plenum Behind Collector {m}",
        "    5.0,                     !- Effective Cross Section Area of Plenum Behind Collector {m2}",
        "    Triangle,                !- Hole Layout Pattern for Pitch",
        "    Kutscher1994,            !- Heat Exchange Effectiveness Correlation",
        "    1.165,                   !- Ratio of Actual Collector Surface Area to Projected Surface Area {dimensionless}",
        "    MediumRough,             !- Roughness of Collector",
        "    0.001,                   !- Collector Thickness {m}",
        "    0.25,                    !- Effectiveness for Perforations with Respect to Wind {dimensionless}",
        "    0.5,                     !- Discharge Coefficient for Openings with Respect to Buoyancy Driven Flow {dimensionless}",
        "    ZN1_ExtWallSouth_1,      !- Surface 1 Name",
        "    ZN1_ExtWallSouth_2;      !- Surface 2 Name",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    GetZoneEquipmentData(*state);

    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data
    EXPECT_FALSE(ErrorsFound);           // expect no errors

    state->dataEnvrn->OutDryBulbTemp = 20.0;
    state->dataEnvrn->OutWetBulbTemp = 15.0;

    SetSurfaceOutBulbTempAt(*state);

    GetTranspiredCollectorInput(*state);
    EXPECT_FALSE(ErrorsFound);

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->SkyTemp = 24.0;
    state->dataEnvrn->IsRain = false;

    InitTranspiredCollector(*state, UTSCNum);

    EXPECT_DOUBLE_EQ(22.0, state->dataTranspiredCollector->UTSC(UTSCNum).Tcoll);
    EXPECT_DOUBLE_EQ(22.5, state->dataTranspiredCollector->UTSC(UTSCNum).Tplen);
    EXPECT_NEAR(19.990, state->dataTranspiredCollector->UTSC(UTSCNum).TairHX, 0.001);
}
