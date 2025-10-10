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

// EnergyPlus::ConvectionCoefficients unit tests

// Google test headers
#include <gtest/gtest.h>

// C++ Headers
#include <algorithm>
#include <array>

// EnergyPlus Headers
#include <EnergyPlus/BaseboardElectric.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

class ConvectionCoefficientsFixture : public EnergyPlus::EnergyPlusFixture
{
public:
protected:
    void SetUp() override
    {
        EnergyPlus::EnergyPlusFixture::SetUp();
    }

    void TearDown() override
    {
        EnergyPlus::EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    };

    std::string getIDFString()
    {
        std::vector<std::string> idf_lines = {

            "  Zone,",
            "    Zone 1,                  !- Name",
            "    0,                       !- Direction of Relative North {deg}",
            "    0,                       !- X Origin {m}",
            "    0,                       !- Y Origin {m}",
            "    0;                       !- Z Origin {m}",

            "  SurfaceConvectionAlgorithm:Inside,AdaptiveConvectionAlgorithm;",

            "  ZoneHVAC:EquipmentConnections,",
            "    Zone 1,                  !- Zone Name",
            "    Zone 1 Eq,               !- Zone Conditioning Equipment List Name",
            "    ,                        !- Zone Air Inlet Node or NodeList Name",
            "    ,                        !- Zone Air Exhaust Node or NodeList Name",
            "    SPACE2-1 Node,           !- Zone Air Node Name",
            "    SPACE2-1 ret node;       !- Zone Return Air Node Name",

            "  ZoneHVAC:EquipmentList,",
            "    Zone 1 Eq,               !- Name",
            "    SequentialLoad,          !- Load Distribution Scheme",
            "    ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 1 Object Type",
            "    Zone 1 Baseboard,        !- Zone Equipment 1 Name",
            "    1,                       !- Zone Equipment 1 Cooling Sequence",
            "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

            " ZoneHVAC:Baseboard:Convective:Electric,",
            "    Zone 1 Baseboard,        !- Name",
            "    ,                        !- Availability Schedule Name",
            "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
            "    1000.0,                  !- Heating Design Capacity {W}",
            "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
            "    ,                        !- Fraction of Autosized Heating Design Capacity",
            "    0.97;                    !- Efficiency",

            "  GlobalGeometryRules,LowerLeftCorner,CounterClockwise,World,World;",

            "  BuildingSurface:Detailed,",
            "    Vertical Wall,                 !- Name",
            "    WALL,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Tilted Down Wall,                 !- Name",
            "    WALL,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,-2.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,-2.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Tilted Up Wall,                 !- Name",
            "    WALL,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,2.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,2.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Horizontal Up Wall,                 !- Name",
            "    WALL,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,10.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,10.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Horizontal Down Wall,                 !- Name",
            "    WALL,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    0.0,10.0,3.0,  !- X,Y,Z ==> Vertex 4 {m}",
            "    10.0,10.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    10.0,0.0,3.0;  !- X,Y,Z ==> Vertex 2 {m}",

            "  BuildingSurface:Detailed,",
            "    Vertical Roof,                 !- Name",
            "    ROOF,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Tilted Down Roof,                 !- Name",
            "    ROOF,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,-2.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,-2.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Tilted Up Roof,                 !- Name",
            "    ROOF,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,2.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,2.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Horizontal Up Roof,                 !- Name",
            "    ROOF,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,10.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,10.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Horizontal Down Roof,                 !- Name",
            "    ROOF,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    0.0,10.0,3.0,  !- X,Y,Z ==> Vertex 4 {m}",
            "    10.0,10.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    10.0,0.0,3.0;  !- X,Y,Z ==> Vertex 2 {m}",

            "  BuildingSurface:Detailed,",
            "    Vertical Floor,                 !- Name",
            "    FLOOR,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Tilted Down Floor,                 !- Name",
            "    FLOOR,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,-2.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,-2.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Tilted Up Floor,                 !- Name",
            "    FLOOR,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,2.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,2.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Horizontal Up Floor,                 !- Name",
            "    FLOOR,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    10.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
            "    10.0,10.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    0.0,10.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

            "  BuildingSurface:Detailed,",
            "    Horizontal Down Floor,                 !- Name",
            "    FLOOR,                    !- Surface Type",
            "    WALL-1,                  !- Construction Name",
            "    Zone 1,                  !- Zone Name",
            "    ,                        !- Space Name",
            "    Outdoors,                !- Outside Boundary Condition",
            "    ,                        !- Outside Boundary Condition Object",
            "    SunExposed,              !- Sun Exposure",
            "    WindExposed,             !- Wind Exposure",
            "    0.50000,                 !- View Factor to Ground",
            "    4,                       !- Number of Vertices",
            "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
            "    0.0,10.0,3.0,  !- X,Y,Z ==> Vertex 4 {m}",
            "    10.0,10.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
            "    10.0,0.0,3.0;  !- X,Y,Z ==> Vertex 2 {m}",

            "  Construction,",
            "    WALL-1,                  !- Name",
            "    GP01;                    !- Outside Layer",

            "  Material,",
            "    GP01,                    !- Name",
            "    MediumSmooth,            !- Roughness",
            "    1.2700000E-02,           !- Thickness {m}",
            "    0.1600000,               !- Conductivity {W/m-K}",
            "    801.0000,                !- Density {kg/m3}",
            "    837.0000,                !- Specific Heat {J/kg-K}",
            "    0.9000000,               !- Thermal Absorptance",
            "    0.7500000,               !- Solar Absorptance",
            "    0.7500000;               !- Visible Absorptance",

        };

        return delimited_string(idf_lines);
    }
};

TEST_F(ConvectionCoefficientsFixture, initExtConvCoeffAdjRatio)
{

    std::string const idf_objects = delimited_string({
        "WindowMaterial:SimpleGlazingSystem,",
        "NonRes Fixed Assembly Window,  !- Name",
        "6.9000,                  !- U-Factor {W/m2-K}",
        "0.39;                    !- Solar Heat Gain Coefficient",
        "Material:NoMass,",
        "R13LAYER,                !- Name",
        "Rough,                   !- Roughness",
        "2.290965,                !- Thermal Resistance {m2-K/W}",
        "0.9000000,               !- Thermal Absorptance",
        "0.7500000,               !- Solar Absorptance",
        "0.7500000;               !- Visible Absorptance",
        "Material:NoMass,",
        "R31LAYER,                !- Name",
        "Rough,                   !- Roughness",
        "5.456,                   !- Thermal Resistance {m2-K/W}",
        "0.9000000,               !- Thermal Absorptance",
        "0.7500000,               !- Solar Absorptance",
        "0.7500000;               !- Visible Absorptance",
        "Material,",
        "C5 - 4 IN HW CONCRETE,   !- Name",
        "MediumRough,             !- Roughness",
        "0.1014984,               !- Thickness {m}",
        "1.729577,                !- Conductivity {W/m-K}",
        "2242.585,                !- Density {kg/m3}",
        "836.8000,                !- Specific Heat {J/kg-K}",
        "0.9000000,               !- Thermal Absorptance",
        "0.6500000,               !- Solar Absorptance",
        "0.6500000;               !- Visible Absorptance",
        "Construction,",
        "R13WALL,                 !- Name",
        "R13LAYER;                !- Outside Layer",
        "Construction,",
        "FLOOR,                   !- Name",
        "C5 - 4 IN HW CONCRETE;   !- Outside Layer",
        "Construction,",
        "ROOF31,                  !- Name",
        "R31LAYER;                !- Outside Layer",
        "Construction,",
        "Window Non-res Fixed,    !- Name",
        "NonRes Fixed Assembly Window;  !- Outside Layer",
        "Zone,",
        "ZONE ONE,                !- Name",
        "0,                       !- Direction of Relative North {deg}",
        "0,                       !- X Origin {m}",
        "0,                       !- Y Origin {m}",
        "0,                       !- Z Origin {m}",
        "1,                       !- Type",
        "1,                       !- Multiplier",
        "autocalculate,           !- Ceiling Height {m}",
        "autocalculate;           !- Volume {m3}",
        "ScheduleTypeLimits,",
        "Fraction,                !- Name",
        "0.0,                     !- Lower Limit Value",
        "1.0,                     !- Upper Limit Value",
        "CONTINUOUS;              !- Numeric Type",
        "GlobalGeometryRules,",
        "UpperLeftCorner,         !- Starting Vertex Position",
        "CounterClockWise,        !- Vertex Entry Direction",
        "World;                   !- Coordinate System",
        "FenestrationSurface:Detailed,",
        "Zn001:Wall001:Win001,    !- Name",
        "Window,                  !- Surface Type",
        "Window Non-res Fixed,    !- Construction Name",
        "Zn001:Wall001,           !- Building Surface Name",
        ",                        !- Outside Boundary Condition Object",
        "0.5000000,               !- View Factor to Ground",
        ",                        !- Frame and Divider Name",
        "1.0,                     !- Multiplier",
        "4,                       !- Number of Vertices",
        "0.548000,0,2.5000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.548000,0,0.5000,  !- X,Y,Z ==> Vertex 2 {m}",
        "5.548000,0,0.5000,  !- X,Y,Z ==> Vertex 3 {m}",
        "5.548000,0,2.5000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall001,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall002,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "15.24000,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "15.24000,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,15.24000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall003,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "15.24000,15.24000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "0,15.24000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "0,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall004,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0,15.24000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "0,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Flr001,            !- Name",
        "Floor,                   !- Surface Type",
        "FLOOR,                   !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Adiabatic,               !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "NoSun,                   !- Sun Exposure",
        "NoWind,                  !- Wind Exposure",
        "1.000000,                !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "15.24000,0.000000,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.000000,0.000000,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "0.000000,15.24000,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,15.24000,0.0;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Roof001,           !- Name",
        "Roof,                    !- Surface Type",
        "ROOF31,                  !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0,                       !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0.000000,15.24000,4.572,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.000000,0.000000,4.572,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,0.000000,4.572,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,15.24000,4.572;  !- X,Y,Z ==> Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound); // this calls GetSurfaceData(), but will also calculate the zone volume which we need
    EXPECT_FALSE(ErrorsFound);

    Real64 HMovInsul = 1.0;
    Material::SurfaceRoughness RoughSurf = Material::SurfaceRoughness::VerySmooth;
    Real64 AbsThermSurf = 0.84;
    Real64 TempExt = -20.0;
    Real64 HExt;
    Real64 HSky;
    Real64 HGround;
    Real64 HAir;
    Real64 HSrdSurf;

    Real64 HExtAdj;
    Real64 adjRatio = 2.0;

    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.allocate(1);
    // without adjust ratio
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio(1) = 1.0;
    Convect::InitExtConvCoeff(*state, 1, HMovInsul, RoughSurf, AbsThermSurf, TempExt, HExt, HSky, HGround, HAir, HSrdSurf);
    // with adjust ratio
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio(1) = adjRatio;
    Convect::InitExtConvCoeff(*state, 1, HMovInsul, RoughSurf, AbsThermSurf, TempExt, HExtAdj, HSky, HGround, HAir, HSrdSurf);
    // adjust ratio scales the returned exterior convection coefficient
    EXPECT_EQ(HExtAdj, HExt * adjRatio);
}

TEST_F(ConvectionCoefficientsFixture, initIntConvCoeffAdjRatio)
{
    std::string const idf_objects = delimited_string({
        "WindowMaterial:SimpleGlazingSystem,",
        "NonRes Fixed Assembly Window,  !- Name",
        "6.9000,                  !- U-Factor {W/m2-K}",
        "0.39;                    !- Solar Heat Gain Coefficient",
        "Material:NoMass,",
        "R13LAYER,                !- Name",
        "Rough,                   !- Roughness",
        "2.290965,                !- Thermal Resistance {m2-K/W}",
        "0.9000000,               !- Thermal Absorptance",
        "0.7500000,               !- Solar Absorptance",
        "0.7500000;               !- Visible Absorptance",
        "Material:NoMass,",
        "R31LAYER,                !- Name",
        "Rough,                   !- Roughness",
        "5.456,                   !- Thermal Resistance {m2-K/W}",
        "0.9000000,               !- Thermal Absorptance",
        "0.7500000,               !- Solar Absorptance",
        "0.7500000;               !- Visible Absorptance",
        "Material,",
        "C5 - 4 IN HW CONCRETE,   !- Name",
        "MediumRough,             !- Roughness",
        "0.1014984,               !- Thickness {m}",
        "1.729577,                !- Conductivity {W/m-K}",
        "2242.585,                !- Density {kg/m3}",
        "836.8000,                !- Specific Heat {J/kg-K}",
        "0.9000000,               !- Thermal Absorptance",
        "0.6500000,               !- Solar Absorptance",
        "0.6500000;               !- Visible Absorptance",
        "Construction,",
        "R13WALL,                 !- Name",
        "R13LAYER;                !- Outside Layer",
        "Construction,",
        "FLOOR,                   !- Name",
        "C5 - 4 IN HW CONCRETE;   !- Outside Layer",
        "Construction,",
        "ROOF31,                  !- Name",
        "R31LAYER;                !- Outside Layer",
        "Construction,",
        "Window Non-res Fixed,    !- Name",
        "NonRes Fixed Assembly Window;  !- Outside Layer",
        "Zone,",
        "ZONE ONE,                !- Name",
        "0,                       !- Direction of Relative North {deg}",
        "0,                       !- X Origin {m}",
        "0,                       !- Y Origin {m}",
        "0,                       !- Z Origin {m}",
        "1,                       !- Type",
        "1,                       !- Multiplier",
        "autocalculate,           !- Ceiling Height {m}",
        "autocalculate;           !- Volume {m3}",
        "ScheduleTypeLimits,",
        "Fraction,                !- Name",
        "0.0,                     !- Lower Limit Value",
        "1.0,                     !- Upper Limit Value",
        "CONTINUOUS;              !- Numeric Type",
        "GlobalGeometryRules,",
        "UpperLeftCorner,         !- Starting Vertex Position",
        "CounterClockWise,        !- Vertex Entry Direction",
        "World;                   !- Coordinate System",
        "FenestrationSurface:Detailed,",
        "Zn001:Wall001:Win001,    !- Name",
        "Window,                  !- Surface Type",
        "Window Non-res Fixed,    !- Construction Name",
        "Zn001:Wall001,           !- Building Surface Name",
        ",                        !- Outside Boundary Condition Object",
        "0.5000000,               !- View Factor to Ground",
        ",                        !- Frame and Divider Name",
        "1.0,                     !- Multiplier",
        "4,                       !- Number of Vertices",
        "0.548000,0,2.5000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.548000,0,0.5000,  !- X,Y,Z ==> Vertex 2 {m}",
        "5.548000,0,0.5000,  !- X,Y,Z ==> Vertex 3 {m}",
        "5.548000,0,2.5000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall001,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall002,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "15.24000,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "15.24000,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,15.24000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall003,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "15.24000,15.24000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "0,15.24000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "0,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Wall004,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0,15.24000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "0,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Flr001,            !- Name",
        "Floor,                   !- Surface Type",
        "FLOOR,                   !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Adiabatic,               !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "NoSun,                   !- Sun Exposure",
        "NoWind,                  !- Wind Exposure",
        "1.000000,                !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "15.24000,0.000000,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.000000,0.000000,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "0.000000,15.24000,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,15.24000,0.0;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,",
        "Zn001:Roof001,           !- Name",
        "Roof,                    !- Surface Type",
        "ROOF31,                  !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0,                       !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0.000000,15.24000,4.572,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.000000,0.000000,4.572,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,0.000000,4.572,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,15.24000,4.572;  !- X,Y,Z ==> Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound); // this calls GetSurfaceData(), but will also calculate the zone volume which we need
    EXPECT_FALSE(ErrorsFound);

    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(7, 1.0);

    state->dataHeatBalSurf->SurfTempInTmp.dimension(7, 20.0);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 25.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.006;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 25.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRatAvg = 0.006;
    state->dataHeatBalSurf->SurfHConvInt.allocate(7);
    state->dataHeatBalSurf->SurfHConvInt(7) = 0.0;

    Convect::InitIntConvCoeff(*state, state->dataHeatBalSurf->SurfTempInTmp);
    // exterior window interior surface convection coefficient without adjustment
    Real64 hcin = state->dataHeatBalSurf->SurfHConvInt(7);
    Real64 adjRatio = 2.0;
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio(7) = adjRatio;
    Convect::InitIntConvCoeff(*state, state->dataHeatBalSurf->SurfTempInTmp);
    // exterior window interior surface convection coefficient with adjustment
    Real64 hcinAdj = state->dataHeatBalSurf->SurfHConvInt(7);
    // adjustment ratio properly applied
    EXPECT_EQ(hcinAdj, adjRatio * hcin);
}

TEST_F(ConvectionCoefficientsFixture, ConvectionCofficients)
{

    Real64 DeltaTemp;     // [C] temperature difference between surface and air
    Real64 Height;        // [m] characteristic size
    Real64 SurfTemp;      // [C] surface temperature
    Real64 SupplyAirTemp; // [C] temperature of supply air into zone
    Real64 AirChangeRate; // [ACH] [1/hour] supply air ACH for zone
    Real64 Hc;

    DeltaTemp = 1.0;
    Height = 2.0;
    SurfTemp = 23.0;
    SupplyAirTemp = 35.0;
    AirChangeRate = 2.0;

    Hc = Convect::CalcBeausoleilMorrisonMixedAssistedWall(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    EXPECT_NEAR(-1.19516, Hc, 0.0001);

    Hc = Convect::CalcBeausoleilMorrisonMixedOpposingWall(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    EXPECT_NEAR(1.8378, Hc, 0.0001);

    Hc = Convect::CalcBeausoleilMorrisonMixedStableFloor(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    EXPECT_NEAR(-4.3290, Hc, 0.0001);

    Hc = Convect::CalcBeausoleilMorrisonMixedUnstableFloor(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    EXPECT_NEAR(-4.24778, Hc, 0.0001);

    Hc = Convect::CalcBeausoleilMorrisonMixedStableCeiling(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    EXPECT_NEAR(-8.11959, Hc, 0.0001);

    Hc = Convect::CalcBeausoleilMorrisonMixedUnstableCeiling(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    EXPECT_NEAR(-8.09685, Hc, 0.0001);
}

TEST_F(ConvectionCoefficientsFixture, DynamicIntConvSurfaceClassification)
{

    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    // Surface 1 is a vertical wall
    // Surface 2 is a wall tilted down (interior faces up)
    // Surface 3 is a wall tilted up (interior faces down)
    // Surface 4 is a horizontal wall facing up (interior faces down)
    // Surface 5 is a horizontal wall facing down (interior faces up)

    // apparently floors get added to the data structure before roofs regardless of idf order
    // Surface 6 is a vertical floor
    // Surface 7 is a floor tilted down (interior faces up)
    // Surface 8 is a floor tilted up (interior faces down) - this gets automatically flipped over so interior faces up
    // Surface 9 is a horizontal floor facing up (interior faces down) - this gets automatically flipped over so interior faces up
    // Surface 10 is a horizontal floor facing down (interior faces up)

    // Surface 11 is a vertical roof
    // Surface 12 is a roof tilted down (interior faces up) - this gets automatically flipped over so interior faces down
    // Surface 13 is a roof tilted up (interior faces down)
    // Surface 14 is a horizontal roof facing up (interior faces down)
    // Surface 15 is a horizontal roof facing down (interior faces up) - this gets automatically flipped over so interior faces down

    // Surface temps are 20C
    for (int surf = 1; surf <= state->dataSurface->TotSurfaces; ++surf) {
        state->dataHeatBalSurf->SurfInsideTempHist(1)(surf) = 20.0;
    }

    // Case 1 - Zone air warmer than surfaces
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 30.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 30.0;

    Convect::DynamicIntConvSurfaceClassification(*state, 1);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(1).convClass, Convect::IntConvClass::A3_SimpleBuoy_VertWalls);

    Convect::DynamicIntConvSurfaceClassification(*state, 2);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(2).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 3);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(3).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 4);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(4).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 5);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(5).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableHoriz);

    // vertical floor is currently not a valid case, so returns zero with a severe error
    //    DynamicIntConvSurfaceClassification(*state, 6);
    //    EXPECT_EQ(state->dataSurface->SurfIntConvClassification(6), 0);

    Convect::DynamicIntConvSurfaceClassification(*state, 7);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(7).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 8);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(8).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 9);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(9).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 10);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(10).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 11);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(11).convClass, Convect::IntConvClass::A3_SimpleBuoy_VertWalls);

    Convect::DynamicIntConvSurfaceClassification(*state, 12);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(12).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 13);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(13).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 14);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(14).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 15);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(15).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz);

    // Case 2 - Zone air colder than surfaces
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 10.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 10.0;

    Convect::DynamicIntConvSurfaceClassification(*state, 1);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(1).convClass, Convect::IntConvClass::A3_SimpleBuoy_VertWalls);

    Convect::DynamicIntConvSurfaceClassification(*state, 2);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(2).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 3);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(3).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 4);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(4).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 5);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(5).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz);

    // vertical floor is currently not a valid case, so returns zero with a severe error
    //    DynamicIntConvSurfaceClassification(*state, 6);
    //    EXPECT_EQ(state->dataSurface->surfIntConv().convClass(6), 0));

    Convect::DynamicIntConvSurfaceClassification(*state, 7);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(7).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 8);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(8).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 9);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(9).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 10);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(10).convClass, Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 11);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(11).convClass, Convect::IntConvClass::A3_SimpleBuoy_VertWalls);

    Convect::DynamicIntConvSurfaceClassification(*state, 12);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(12).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 13);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(13).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableTilted);

    Convect::DynamicIntConvSurfaceClassification(*state, 14);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(14).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableHoriz);

    Convect::DynamicIntConvSurfaceClassification(*state, 15);
    EXPECT_ENUM_EQ(state->dataSurface->surfIntConv(15).convClass, Convect::IntConvClass::A3_SimpleBuoy_StableHoriz);
}

TEST_F(ConvectionCoefficientsFixture, EvaluateIntHcModelsFisherPedersen)
{

    int SurfNum;
    Convect::HcInt ConvModelEquationNum;
    Real64 Hc;
    Real64 HcExpectedValue;

    SurfNum = 1;

    state->dataSurface->TotSurfaces = 1;
    state->dataGlobal->NumOfZones = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->surfIntConv.allocate(1);
    state->dataSurface->surfExtConv.allocate(1);
    state->dataConstruction->Construct.allocate(1);
    state->dataHeatBal->Zone.allocate(1);
    state->dataLoopNodes->Node.allocate(1);

    state->dataSurface->Surface(SurfNum).Zone = 1;
    state->dataSurface->Surface(SurfNum).spaceNum = 1;
    state->dataSurface->Surface(SurfNum).Construction = 1;
    state->dataSurface->SurfTAirRef.allocate(1);
    state->dataSurface->SurfTAirRefRpt.allocate(1);
    state->dataSurface->SurfTAirRef(SurfNum) = 0;
    state->dataConstruction->Construct(1).TypeIsWindow = false;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataLoopNodes->Node(1).Temp = 20.0;
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    for (int surf = 1; surf <= state->dataSurface->TotSurfaces; ++surf) {
        state->dataHeatBalSurf->SurfInsideTempHist(1)(surf) = 20.0;
    }

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 30.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 30.0;

    // Case 1 - Low ACH (should default to CalcASHRAETARPNatural)
    Real64 ACH = 0.25;
    state->dataHeatBal->Zone(1).Volume = 125.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 1.17653 / 3600.0 * state->dataHeatBal->Zone(1).Volume * ACH;

    // Test 1: Floor Diffuser Model
    ConvModelEquationNum = Convect::HcInt::FisherPedersenCeilDiffuserFloor;
    Hc = 0.0;
    state->dataSurface->Surface(SurfNum).CosTilt = -1;

    HcExpectedValue = Convect::CalcASHRAETARPNatural(state->dataHeatBalSurf->SurfInsideTempHist(1)(1),
                                                     state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT,
                                                     -state->dataSurface->Surface(SurfNum).CosTilt);

    Hc = Convect::EvaluateIntHcModels(*state, SurfNum, ConvModelEquationNum);
    EXPECT_EQ(state->dataSurface->SurfTAirRef(SurfNum), DataSurfaces::RefAirTemp::ZoneMeanAirTemp);
    EXPECT_NEAR(Hc, HcExpectedValue, 0.1);

    // Test 2: Ceiling Diffuser Model
    ConvModelEquationNum = Convect::HcInt::FisherPedersenCeilDiffuserCeiling;
    Hc = 0.0;
    state->dataSurface->Surface(SurfNum).CosTilt = 1;

    HcExpectedValue = Convect::CalcASHRAETARPNatural(state->dataHeatBalSurf->SurfInsideTempHist(1)(1),
                                                     state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT,
                                                     -state->dataSurface->Surface(SurfNum).CosTilt);

    Hc = Convect::EvaluateIntHcModels(*state, SurfNum, ConvModelEquationNum);
    EXPECT_EQ(state->dataSurface->SurfTAirRef(SurfNum), DataSurfaces::RefAirTemp::ZoneMeanAirTemp);
    EXPECT_NEAR(Hc, HcExpectedValue, 0.1);

    // Test 3: Ceiling Diffuser Model
    ConvModelEquationNum = Convect::HcInt::FisherPedersenCeilDiffuserWalls;
    Hc = 0.0;
    state->dataSurface->Surface(SurfNum).CosTilt = 0;

    HcExpectedValue = Convect::CalcASHRAETARPNatural(state->dataHeatBalSurf->SurfInsideTempHist(1)(1),
                                                     state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT,
                                                     -state->dataSurface->Surface(SurfNum).CosTilt);

    Hc = Convect::EvaluateIntHcModels(*state, SurfNum, ConvModelEquationNum);
    EXPECT_EQ(state->dataSurface->SurfTAirRef(SurfNum), DataSurfaces::RefAirTemp::ZoneMeanAirTemp);
    EXPECT_NEAR(Hc, HcExpectedValue, 0.1);

    // Case 2 - High ACH
    ACH = 3.1;
    state->dataHeatBal->Zone(1).Volume = 125.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 1.17653 / 3600.0 * state->dataHeatBal->Zone(1).Volume * ACH;

    // Test 1: Floor Diffuser Model
    ConvModelEquationNum = Convect::HcInt::FisherPedersenCeilDiffuserFloor;
    Hc = 0.0;
    state->dataSurface->Surface(SurfNum).CosTilt = -1;

    HcExpectedValue = 4.122;

    Hc = Convect::EvaluateIntHcModels(*state, SurfNum, ConvModelEquationNum);
    EXPECT_EQ(state->dataSurface->SurfTAirRef(SurfNum), DataSurfaces::RefAirTemp::ZoneMeanAirTemp);
    EXPECT_NEAR(Hc, HcExpectedValue, 0.1);

    // Test 2: Ceiling Diffuser Model
    ConvModelEquationNum = Convect::HcInt::FisherPedersenCeilDiffuserCeiling;
    Hc = 0.0;
    state->dataSurface->Surface(SurfNum).CosTilt = 1;

    HcExpectedValue = 9.476;

    Hc = Convect::EvaluateIntHcModels(*state, SurfNum, ConvModelEquationNum);
    EXPECT_EQ(state->dataSurface->SurfTAirRef(SurfNum), DataSurfaces::RefAirTemp::ZoneMeanAirTemp);
    EXPECT_NEAR(Hc, HcExpectedValue, 0.1);

    // Test 3: Ceiling Diffuser Model
    ConvModelEquationNum = Convect::HcInt::FisherPedersenCeilDiffuserWalls;
    Hc = 0.0;
    state->dataSurface->Surface(SurfNum).CosTilt = 0;

    HcExpectedValue = 3.212;

    Hc = Convect::EvaluateIntHcModels(*state, SurfNum, ConvModelEquationNum);
    EXPECT_EQ(state->dataSurface->SurfTAirRef(SurfNum), DataSurfaces::RefAirTemp::ZoneMeanAirTemp);
    EXPECT_NEAR(Hc, HcExpectedValue, 0.1);
}

TEST_F(ConvectionCoefficientsFixture, EvaluateHnModels)
{

    int SurfNum;
    Real64 DeltaTemp;
    Real64 CosineTilt;
    Real64 Hn;
    Array1D<Real64> SurfTemp;
    Array1D<Real64> HcIn;
    Array1D<Real64> Vhc;

    SurfNum = 1;
    state->dataSurface->Surface.allocate(SurfNum);
    state->dataSurface->Surface(SurfNum).Zone = 1;
    state->dataRoomAir->AirModel.allocate(1);
    state->dataHeatBal->SurfTempEffBulkAir.allocate(1);
    state->dataHeatBal->SurfTempEffBulkAir(1) = 1.0;
    SurfTemp.allocate(1);
    HcIn.allocate(1);
    Vhc.allocate(1);
    state->dataSurface->surfIntConv.allocate(SurfNum);
    state->dataSurface->SurfTAirRef.allocate(SurfNum);
    // Test 1: CalcWaltonUnstableHorizontalOrTilt calculation for Hn
    DeltaTemp = 1.0;
    CosineTilt = 1.0;
    Hn = 0.0;
    Hn = Convect::CalcWaltonUnstableHorizontalOrTilt(DeltaTemp, CosineTilt);
    EXPECT_NEAR(Hn, 1.520, 0.001);

    // Test 2/3: CalcDetailedHcInForDVModel calculation for Hn
    state->dataSurface->Surface(SurfNum).HeatTransSurf = true;
    state->dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->surfIntConv(SurfNum).model = Convect::HcInt::SetByZone;
    state->dataSurface->surfIntConv(SurfNum).userModelNum = 0;
    state->dataRoomAir->AirModel(state->dataSurface->Surface(SurfNum).Zone).AirModel = RoomAir::RoomAirModel::DispVent3Node;
    state->dataSurface->Surface(SurfNum).CosTilt = 1.0;
    SurfTemp(1) = 0.0;
    HcIn(1) = 0.0;
    Convect::CalcDetailedHcInForDVModel(*state, SurfNum, SurfTemp, HcIn);
    Hn = HcIn(1);
    EXPECT_NEAR(Hn, 1.520, 0.001);

    state->dataSurface->Surface(SurfNum).HeatTransSurf = true;
    state->dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->surfIntConv(SurfNum).model = Convect::HcInt::SetByZone;
    state->dataSurface->surfIntConv(SurfNum).userModelNum = 0;
    state->dataRoomAir->AirModel(state->dataSurface->Surface(SurfNum).Zone).AirModel = RoomAir::RoomAirModel::CrossVent;
    state->dataSurface->Surface(SurfNum).CosTilt = 1.0;
    SurfTemp(1) = 0.0;
    HcIn(1) = 0.0;
    Vhc(1) = 1.0;
    Convect::CalcDetailedHcInForDVModel(*state, SurfNum, SurfTemp, HcIn, Vhc);
    Hn = HcIn(1);
    EXPECT_NEAR(Hn, 4.347, 0.001);
}

TEST_F(ConvectionCoefficientsFixture, TestCalcZoneSystemACH)
{

    int ZoneNum = 1;
    int TotalNumberofZones = 1;
    Real64 ACHAnswer;
    Real64 ACHExpected;

    if (!allocated(state->dataHeatBal->Zone)) {
        state->dataHeatBal->Zone.allocate(TotalNumberofZones);
    }
    state->dataHeatBal->Zone(ZoneNum).Volume = 100.0;
    state->dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber = 1;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataHeatBal->Zone(ZoneNum).Multiplier = 1.0;
    state->dataHeatBal->Zone(ZoneNum).ListMultiplier = 1.0;
    state->dataEnvrn->OutBaroPress = 101400.0;
    Real64 ZoneNode = state->dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber;

    // Test 1: Node not allocated, returns a zero ACH
    if (allocated(state->dataLoopNodes->Node)) {
        state->dataLoopNodes->Node.deallocate();
    }
    ACHExpected = 0.0;
    ACHAnswer = Convect::CalcZoneSystemACH(*state, ZoneNum);
    EXPECT_NEAR(ACHExpected, ACHAnswer, 0.0001);

    // Test 2: Node now allocated, needs to return a proper ACH
    state->dataLoopNodes->Node.allocate(state->dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber);
    state->dataLoopNodes->Node(ZoneNode).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneNode).MassFlowRate = 0.2;
    ACHExpected = 6.11506;
    ACHAnswer = Convect::CalcZoneSystemACH(*state, ZoneNum);
    EXPECT_NEAR(ACHExpected, ACHAnswer, 0.0001);
}

TEST_F(ConvectionCoefficientsFixture, TestCalcFisherPedersenCeilDiffuserNatConv)
{

    Real64 Hforced;
    Real64 ACH;
    Real64 Tsurf;
    Real64 Tair;
    Real64 cosTilt;
    Real64 humRat;
    Real64 height;
    bool isWindow;
    Real64 ExpectedHconv;
    Real64 CalculatedHconv;

    state->dataEnvrn->OutBaroPress = 101325.0;

    // Test 1: Non-window, all natural
    Hforced = 10.0;
    ACH = 0.25;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 1.2994;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserNatConv(*state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);

    // Test 2: Window, all natural
    Hforced = 10.0;
    ACH = 0.25;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = true;
    ExpectedHconv = 0.8067;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserNatConv(*state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);

    // Test 3: Non-window, all natural
    Hforced = 10.0;
    ACH = 0.5;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 1.2994;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserNatConv(*state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);

    // Test 4: Non-window, transition
    Hforced = 10.0;
    ACH = 0.75;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 2.16942;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserNatConv(*state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);

    // Test 5: Non-window, all ceiling diffuser correlation
    Hforced = 10.0;
    ACH = 3.0;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 10.0;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserNatConv(*state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
}

TEST_F(ConvectionCoefficientsFixture, TestCalcFisherPedersenCeilDiffuserCorrelations)
{

    Real64 ACH;
    Real64 Tsurf;
    Real64 Tair;
    Real64 cosTilt;
    Real64 humRat;
    Real64 height;
    bool isWindow;
    Real64 ExpectedHconv;
    Real64 CalculatedHconv;

    state->dataEnvrn->OutBaroPress = 101325.0;

    // Test 1: Forced Convection All Correlations (Floor, Ceiling, Wall)
    ACH = 3.3;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 4.13721502661183;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserFloor(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
    ExpectedHconv = 9.70692167003631;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserCeiling(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
    ExpectedHconv = 3.28943537910741;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserWalls(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);

    // Test 2: Natural Convection All Correlations (Floor, Ceiling, Wall)--note, all should give same answer because of how variables are set
    ACH = 0.25;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 1.2994;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserFloor(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserCeiling(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserWalls(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);

    // Test 3: Mixed Convection All Correlations (Floor, Ceiling, Wall)
    ACH = 1.75;
    Tsurf = 23.0;
    Tair = 18.0;
    cosTilt = 1.0;
    humRat = 0.08;
    height = 1.0;
    isWindow = false;
    ExpectedHconv = 2.70653;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserFloor(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
    ExpectedHconv = 5.32826;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserCeiling(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
    ExpectedHconv = 2.23620;
    CalculatedHconv = Convect::CalcFisherPedersenCeilDiffuserWalls(*state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    EXPECT_NEAR(ExpectedHconv, CalculatedHconv, 0.0001);
}

TEST_F(ConvectionCoefficientsFixture, TestWindward)
{

    bool AgainstWind;

    Real64 CosTilt;
    Real64 Azimuth;
    Real64 WindDirection;

    // Test 1: Horizontal surface
    CosTilt = 1.0;
    Azimuth = 180.0;
    WindDirection = 180.0;
    AgainstWind = Convect::Windward(CosTilt, Azimuth, WindDirection);
    EXPECT_TRUE(AgainstWind);

    // Test 2: Vertical surface, Azimuth and WindDiretion within 90 degrees of one another (windward or against wind)
    CosTilt = 0.5;
    Azimuth = 269.0;
    WindDirection = 180.0;
    AgainstWind = Convect::Windward(CosTilt, Azimuth, WindDirection);
    EXPECT_TRUE(AgainstWind);

    // Test 3: Vertical surface, Azimuth and WindDiretion not within 90 degrees of one another (leeward or not against wind)
    CosTilt = 0.5;
    Azimuth = 271.0;
    WindDirection = 180.0;
    AgainstWind = Convect::Windward(CosTilt, Azimuth, WindDirection);
    EXPECT_FALSE(AgainstWind);
}

TEST_F(ConvectionCoefficientsFixture, CalcBeausoleilMorrisonMixedAssistedWall)
{
    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    Real64 tolerance = 1E-3;

    // Calculate convection coefficient DeltaT and Height != 0
    Real64 deltaTemp = 10.0;
    Real64 height = 1.0;
    Real64 surfTemp = 20.0;
    int zoneNum = 1;
    Real64 convCoeff = Convect::CalcBeausoleilMorrisonMixedAssistedWall(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 2.667, tolerance);

    // DeltaT = 0 Error Path
    deltaTemp = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedAssistedWall(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);

    // Height = 0 Error Path
    deltaTemp = 10.0;
    height = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedAssistedWall(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);
}

TEST_F(ConvectionCoefficientsFixture, CalcBeausoleilMorrisonMixedOpposingWall)
{

    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    Real64 tolerance = 1E-3;

    // Calculate convection coefficient DeltaT and Height != 0
    Real64 deltaTemp = 10.0;
    Real64 height = 1.0;
    Real64 surfTemp = 20.0;
    int zoneNum = 1;
    Real64 convCoeff = Convect::CalcBeausoleilMorrisonMixedOpposingWall(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 2.673, tolerance);

    // DeltaT = 0 Error Path
    deltaTemp = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedOpposingWall(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);

    // Height = 0 Error Path
    deltaTemp = 10.0;
    height = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedOpposingWall(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);
}

TEST_F(ConvectionCoefficientsFixture, CalcBeausoleilMorrisonMixedStableFloor)
{

    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    Real64 tolerance = 1E-3;

    // Calculate convection coefficient DeltaT and Height != 0
    Real64 deltaTemp = 10.0;
    Real64 height = 1.0;
    Real64 surfTemp = 20.0;
    int zoneNum = 1;
    Real64 convCoeff = Convect::CalcBeausoleilMorrisonMixedStableFloor(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 0.962, tolerance);

    // DeltaT = 0 Error Path
    deltaTemp = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedStableFloor(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);

    // Height = 0 Error Path
    deltaTemp = 10.0;
    height = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedStableFloor(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);
}

TEST_F(ConvectionCoefficientsFixture, CalcBeausoleilMorrisonMixedUnstableFloor)
{

    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    Real64 tolerance = 1E-3;

    // Calculate convection coefficient DeltaT and Height != 0
    Real64 deltaTemp = 10.0;
    Real64 height = 1.0;
    Real64 surfTemp = 20.0;
    int zoneNum = 1;
    Real64 convCoeff = Convect::CalcBeausoleilMorrisonMixedUnstableFloor(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 3.583, tolerance);

    // DeltaT = 0 Error Path
    deltaTemp = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedUnstableFloor(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);

    // Height = 0 Error Path
    deltaTemp = 10.0;
    height = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedUnstableFloor(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);
}

TEST_F(ConvectionCoefficientsFixture, CalcBeausoleilMorrisonMixedStableCeiling)
{

    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    Real64 tolerance = 1E-3;

    // Calculate convection coefficient DeltaT and Height != 0
    Real64 deltaTemp = 10.0;
    Real64 height = 1.0;
    Real64 surfTemp = 20.0;
    int zoneNum = 1;
    Real64 convCoeff = Convect::CalcBeausoleilMorrisonMixedStableCeiling(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 0.937, tolerance);

    // DeltaT = 0 Error Path
    deltaTemp = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedStableCeiling(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);

    // Height = 0 Error Path
    deltaTemp = 10.0;
    height = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedStableCeiling(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);
}

TEST_F(ConvectionCoefficientsFixture, CalcBeausoleilMorrisonMixedUnstableCeiling)
{

    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, errorsFound);
    ASSERT_FALSE(errorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    BaseboardElectric::GetBaseboardInput(*state);

    state->dataGlobal->ZoneSizingCalc = true;

    Real64 tolerance = 1E-3;

    // Calculate convection coefficient DeltaT and Height != 0
    Real64 deltaTemp = 10.0;
    Real64 height = 1.0;
    Real64 surfTemp = 20.0;
    int zoneNum = 1;
    Real64 convCoeff = Convect::CalcBeausoleilMorrisonMixedUnstableCeiling(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 3.581, tolerance);

    // DeltaT = 0 Error Path
    deltaTemp = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedUnstableCeiling(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);

    // Height = 0 Error Path
    deltaTemp = 10.0;
    height = 0.0;
    convCoeff = Convect::CalcBeausoleilMorrisonMixedUnstableCeiling(*state, deltaTemp, height, surfTemp, zoneNum);
    EXPECT_NEAR(convCoeff, 9.999, tolerance);
}

TEST_F(ConvectionCoefficientsFixture, ConvectionCoefficientsTest_CalcASHRAESimpleIntConvCoeff)
{
    // Unit test for the function CalcASHRAESimpleIntConvCoeff, used to determine the Convection Coefficient
    // for the Ashrae Simple algorithm setting

    Real64 Tsurf;
    Real64 Tamb;
    Real64 CosTilt;
    Real64 ConvectionCoefficient;
    Real64 ExpectedCoefficient;

    // Scenario: Vertical Surface
    // Hcov expected = 3.076
    // Delta_T is not relevant for this calculation

    Tsurf = 30.0;
    Tamb = 20.0;
    CosTilt = 0.0; // cos(90 degrees)
    ExpectedCoefficient = 3.076;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: Vertical Surface, CosTilt not exactly zero
    // Hcov expected = 3.076

    Tsurf = 19.0;
    Tamb = 20.0;
    CosTilt = 0.0001; // cos(90 degrees)
    ExpectedCoefficient = 3.076;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: Vertical Surface, Zero Delta T
    // Hcov expected = 3.076

    Tsurf = 23.0;
    Tamb = 23.0;
    CosTilt = 0; // cos(90 degrees)
    ExpectedCoefficient = 3.076;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: Horizontal Surface with reduced convection
    // Hcov expected = 0.948
    // A negative Delta_T is required for reduced convection

    Tsurf = 30.0;
    Tamb = 20.0;
    CosTilt = 0.9239; // cos(22.5 degrees)
    ExpectedCoefficient = 0.948;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: Horizontal surface with enhanced convection:
    // Hcov expected = 4.040
    // A positive Delta_T is required for enhanced convection

    Tsurf = 20.0;
    Tamb = 30.0;
    CosTilt = 0.9239; // cos(22.5 degrees)
    ExpectedCoefficient = 4.040;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: horizontal surface, enhanced convection
    // 180 degree surface, negative Delta_T
    // Hcov expected = 4.040

    Tsurf = 30.0;
    Tamb = 20.0;
    CosTilt = -1; // cos(180 degrees)
    ExpectedCoefficient = 4.040;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: horizontal surface, reduced convection
    // 180 degree surface, positive Delta_T
    // Hcov expected = 0.948

    Tsurf = 20.0;
    Tamb = 30.0;
    CosTilt = -1; // cos(180 degrees)
    ExpectedCoefficient = 0.948;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: tilted surface with reduced convection
    // Hcov expected = 2.281
    // A negative Delta_T is required for reduced convection

    Tsurf = 30.0;
    Tamb = 20.0;
    CosTilt = 0.707; // cos(45 degrees)
    ExpectedCoefficient = 2.281;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);

    // Scenario: tilted surface with enhanced convection
    // Hcov expected = 3.870

    Tsurf = 20.0;
    Tamb = 30.0;
    CosTilt = 0.707; // cos(45 degrees)
    ExpectedCoefficient = 3.870;

    ConvectionCoefficient = Convect::CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, CosTilt);
    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);
}

TEST_F(ConvectionCoefficientsFixture, ConvectionCoefficientsTest_HConvInDependence)
{
    Real64 ConvectionCoefficient;
    Real64 ExpectedCoefficient = 3.076;

    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).CosTilt = 0;

    state->dataHeatBalSurf->SurfHConvInt.allocate(1);

    Convect::CalcASHRAESimpleIntConvCoeff(*state, 1, 20.0, 30.0);

    ConvectionCoefficient = state->dataHeatBalSurf->SurfHConvInt(1);

    EXPECT_EQ(ConvectionCoefficient, ExpectedCoefficient);
}

TEST_F(EnergyPlusFixture, AdaptiveModelSelections_ProperConstruction)
{
    // This unit test checks to make sure the InsideFaceAdaptiveConvectionAlgo and OutsideFaceAdaptiveConvectionAlgo objects default construct their
    // members to the correct algorithm integer identifiers

    std::string const idf_objects = delimited_string({"SurfaceConvectionAlgorithm:Inside,AdaptiveConvectionAlgorithm;",
                                                      "SurfaceConvectionAlgorithm:Outside,AdaptiveConvectionAlgorithm;",

                                                      "SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections,",
                                                      "Default Algorithm;       !- Name",

                                                      "SurfaceConvectionAlgorithm:Outside:AdaptiveModelSelections,",
                                                      "Default algorithms;      !- Name"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Convect::HcInt algo;

    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FohannoPolidoriVerticalWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondUnstableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq3WallAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq4CeilingAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_HeatedFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AwbiHattonHeatedFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_ChilledCeil];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KaradagChilledCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_VertWallsNonHeated];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq6NonHeatedWalls);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_HeatedVerticalWall];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AwbiHattonHeatedWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq7Ceiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FohannoPolidoriVerticalWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_VertWallsNearHeat];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq5WallNearHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq7Ceiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Walls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWalls);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Ceiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FisherPedersenCeilDiffuserCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Floor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWindow);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_Walls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq3WallAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq4CeilingAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_AssistFlowWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedAssistingWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_OpposFlowWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedOppossingWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_StableFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedStableFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_UnstableFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedUnstableFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_StableCeiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedStableCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_UnstableCeiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedUnstableCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWindow);

    Convect::HcExt algo2;
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallWindward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::SparrowWindward);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallLeeward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::SparrowLeeward);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_HorizRoof];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::ClearRoof);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_VertWall];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalASHRAEVerticalWall);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_StableHoriz];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalWaltonStableHorizontalOrTilt);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_UnstableHoriz];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalWaltonUnstableHorizontalOrTilt);
}

TEST_F(EnergyPlusFixture, AdaptiveModelSelections_Implicit)
{
    // This unit test checks to make sure the intAdaptiveConvAlgo and extAdaptiveConvAlgo objects assign their
    // members to the correct algorithm integer identifiers inside the GetUserConvectionCoefficients() function
    std::string const idf_objects = delimited_string({
        "SurfaceConvectionAlgorithm:Inside,AdaptiveConvectionAlgorithm;",
        "SurfaceConvectionAlgorithm:Outside,AdaptiveConvectionAlgorithm;",

        "SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections,",
        "Default Algorithm,       !- Name",
        "FohannoPolidoriVerticalWall;  !- Simple Buoyancy Vertical Wall Equation Source",

        "SurfaceConvectionAlgorithm:Outside:AdaptiveModelSelections,",
        "Default algorithms,      !- Name",
        "TARPWindward;            !- Wind Convection Windward Vertical Wall Equation Source",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;
    Convect::InitIntConvCoeff(*state, state->dataHeatBalSurf->SurfTempInTmp);

    Convect::HcInt algo;

    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FohannoPolidoriVerticalWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondUnstableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq3WallAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq4CeilingAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_HeatedFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AwbiHattonHeatedFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_ChilledCeil];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KaradagChilledCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_VertWallsNonHeated];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq6NonHeatedWalls);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_HeatedVerticalWall];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AwbiHattonHeatedWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq7Ceiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FohannoPolidoriVerticalWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_VertWallsNearHeat];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq5WallNearHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq7Ceiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Walls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWalls);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Ceiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FisherPedersenCeilDiffuserCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Floor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWindow);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_Walls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq3WallAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq4CeilingAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_AssistFlowWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedAssistingWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_OpposFlowWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedOppossingWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_StableFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedStableFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_UnstableFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedUnstableFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_StableCeiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedStableCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_UnstableCeiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedUnstableCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWindow);

    Convect::HcExt algo2;
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallWindward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::SparrowWindward);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallLeeward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::SparrowLeeward);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_HorizRoof];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::ClearRoof);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_VertWall];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalASHRAEVerticalWall);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_StableHoriz];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalWaltonStableHorizontalOrTilt);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_UnstableHoriz];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalWaltonUnstableHorizontalOrTilt);

    state->dataHeatBalSurf->SurfTempInTmp.deallocate();
}

TEST_F(EnergyPlusFixture, AdaptiveModelSelections_ExplicitSelection)
{
    // This unit test checks to make sure the intAdaptiveConvAlgo and extAdaptiveConvAlgo objects assign their
    // members to the correct algorithm integer identifiers inside the GetUserConvectionCoefficients() function when non-default assignments
    // are specified in the idf
    std::string const idf_objects = delimited_string({

        "SurfaceConvectionAlgorithm:Inside,AdaptiveConvectionAlgorithm;",
        "SurfaceConvectionAlgorithm:Outside,AdaptiveConvectionAlgorithm;",

        "SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections,",
        "Default Algorithm,       !- Name",
        "ISO15099Windows,         !- Simple Buoyancy Vertical Wall Equation Source",
        ",                        !- Simple Buoyancy Vertical Wall User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Simple Buoyancy Stable Horizontal Equation Source",
        ",                        !- Simple Buoyancy Stable Horizontal Equation User Curve Name",
        "AlamdariHammondUnstableHorizontal,  !- Simple Buoyancy Unstable Horizontal Equation Source",
        ",                        !- Simple Buoyancy Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Simple Buoyancy Stable Tilted Equation Source",
        ",                        !- Simple Buoyancy Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Simple Buoyancy Unstable Tilted Equation Source",
        ",                        !- Simple Buoyancy Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Simple Buoyancy Windows Equation Source",
        ",                        !- Simple Buoyancy Windows Equation User Curve Name",
        "KhalifaEq3WallAwayFromHeat,  !- Floor Heat Ceiling Cool Vertical Wall Equation Source",
        ",                        !- Floor Heat Ceiling Cool Vertical Wall Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Floor Heat Ceiling Cool Stable Horizontal Equation Source",
        ",                        !- Floor Heat Ceiling Cool Stable Horizontal Equation User Curve Name",
        "KhalifaEq4CeilingAwayFromHeat,  !- Floor Heat Ceiling Cool Unstable Horizontal Equation Source",
        ",                        !- Floor Heat Ceiling Cool Unstable Horizontal Equation User Curve Name",
        "AwbiHattonHeatedFloor,   !- Floor Heat Ceiling Cool Heated Floor Equation Source",
        ",                        !- Floor Heat Ceiling Cool Heated Floor Equation User Curve Name",
        "KaradagChilledCeiling,   !- Floor Heat Ceiling Cool Chilled Ceiling Equation Source",
        ",                        !- Floor Heat Ceiling Cool Chilled Ceiling Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Floor Heat Ceiling Cool Stable Tilted Equation Source",
        ",                        !- Floor Heat Ceiling Cool Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Floor Heat Ceiling Cool Unstable Tilted Equation Source",
        ",                        !- Floor Heat Ceiling Cool Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Floor Heat Ceiling Cool Window Equation Source",
        ",                        !- Floor Heat Ceiling Cool Window Equation User Curve Name",
        "KhalifaEq6NonHeatedWalls,!- Wall Panel Heating Vertical Wall Equation Source",
        ",                        !- Wall Panel Heating Vertical Wall Equation User Curve Name",
        "AwbiHattonHeatedWall,    !- Wall Panel Heating Heated Wall Equation Source",
        ",                        !- Wall Panel Heating Heated Wall Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Wall Panel Heating Stable Horizontal Equation Source",
        ",                        !- Wall Panel Heating Stable Horizontal Equation User Curve Name",
        "KhalifaEq7Ceiling,       !- Wall Panel Heating Unstable Horizontal Equation Source",
        ",                        !- Wall Panel Heating Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Wall Panel Heating Stable Tilted Equation Source",
        ",                        !- Wall Panel Heating Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Wall Panel Heating Unstable Tilted Equation Source",
        ",                        !- Wall Panel Heating Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Wall Panel Heating Window Equation Source",
        ",                        !- Wall Panel Heating Window Equation User Curve Name",
        "FohannoPolidoriVerticalWall,  !- Convective Zone Heater Vertical Wall Equation Source",
        ",                        !- Convective Zone Heater Vertical Wall Equation User Curve Name",
        "KhalifaEq5WallNearHeat,  !- Convective Zone Heater Vertical Walls Near Heater Equation Source",
        ",                        !- Convective Zone Heater Vertical Walls Near Heater Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Convective Zone Heater Stable Horizontal Equation Source",
        ",                        !- Convective Zone Heater Stable Horizontal Equation User Curve Name",
        "KhalifaEq7Ceiling,       !- Convective Zone Heater Unstable Horizontal Equation Source",
        ",                        !- Convective Zone Heater Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Convective Zone Heater Stable Tilted Equation Source",
        ",                        !- Convective Zone Heater Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Convective Zone Heater Unstable Tilted Equation Source",
        ",                        !- Convective Zone Heater Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Convective Zone Heater Windows Equation Source",
        ",                        !- Convective Zone Heater Windows Equation User Curve Name",
        "GoldsteinNovoselacCeilingDiffuserWalls,  !- Central Air Diffuser Wall Equation Source",
        ",                        !- Central Air Diffuser Wall Equation User Curve Name",
        "FisherPedersenCeilingDiffuserCeiling,  !- Central Air Diffuser Ceiling Equation Source",
        ",                        !- Central Air Diffuser Ceiling Equation User Curve Name",
        "GoldsteinNovoselacCeilingDiffuserFloor,  !- Central Air Diffuser Floor Equation Source",
        ",                        !- Central Air Diffuser Floor Equation User Curve Name",
        "GoldsteinNovoselacCeilingDiffuserWindow,  !- Central Air Diffuser Window Equation Source",
        ",                        !- Central Air Diffuser Window Equation User Curve Name",
        "KhalifaEq3WallAwayFromHeat,  !- Mechanical Zone Fan Circulation Vertical Wall Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Vertical Wall Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Mechanical Zone Fan Circulation Stable Horizontal Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Stable Horizontal Equation User Curve Name",
        "KhalifaEq4CeilingAwayFromHeat,  !- Mechanical Zone Fan Circulation Unstable Horizontal Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Mechanical Zone Fan Circulation Stable Tilted Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Mechanical Zone Fan Circulation Unstable Tilted Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Mechanical Zone Fan Circulation Window Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Window Equation User Curve Name",
        ",  !- Mixed Regime Buoyancy Assisting Flow on Walls Equation Source",
        ",                        !- Mixed Regime Buoyancy Assisting Flow on Walls Equation User Curve Name",
        ",  !- Mixed Regime Buoyancy Opposing Flow on Walls Equation Source",
        ",                        !- Mixed Regime Buoyancy Opposing Flow on Walls Equation User Curve Name",
        ",  !- Mixed Regime Stable Floor Equation Source",
        ",                        !- Mixed Regime Stable Floor Equation User Curve Name",
        ",  !- Mixed Regime Unstable Floor Equation Source",
        ",                        !- Mixed Regime Unstable Floor Equation User Curve Name",
        ",  !- Mixed Regime Stable Ceiling Equation Source",
        ",                        !- Mixed Regime Stable Ceiling Equation User Curve Name",
        ",  !- Mixed Regime Unstable Ceiling Equation Source",
        ",                        !- Mixed Regime Unstable Ceiling Equation User Curve Name",
        ",  !- Mixed Regime Window Equation Source",
        ";                        !- Mixed Regime Window Equation User Curve Name",

        "SurfaceConvectionAlgorithm:Outside:AdaptiveModelSelections,",
        "Default algorithms,      !- Name",
        "TARPWindward,            !- Wind Convection Windward Vertical Wall Equation Source",
        ",                        !- Wind Convection Windward Equation Vertical Wall User Curve Name",
        "TARPLeeward,             !- Wind Convection Leeward Vertical Wall Equation Source",
        ",                        !- Wind Convection Leeward Vertical Wall Equation User Curve Name",
        "ClearRoof,               !- Wind Convection Horizontal Roof Equation Source",
        ",                        !- Wind Convection Horizontal Roof User Curve Name",
        "ASHRAEVerticalWall,      !- Natural Convection Vertical Wall Equation Source",
        ",                        !- Natural Convection Vertical Wall Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Natural Convection Stable Horizontal Equation Source",
        ",                        !- Natural Convection Stable Horizontal Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Natural Convection Unstable Horizontal Equation Source",
        ";                        !- Natural Convection Unstable Horizontal Equation User Curve Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;
    Convect::InitIntConvCoeff(*state, state->dataHeatBalSurf->SurfTempInTmp);
    Convect::GetUserConvCoeffs(*state);

    Convect::HcInt algo;

    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondUnstableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq3WallAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq4CeilingAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_HeatedFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AwbiHattonHeatedFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_ChilledCeil];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KaradagChilledCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A1_FloorHeatCeilCool_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_VertWallsNonHeated];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq6NonHeatedWalls);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_HeatedVerticalWall];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AwbiHattonHeatedWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq7Ceiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A2_WallPanelHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FohannoPolidoriVerticalWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_VertWallsNearHeat];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq5WallNearHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq7Ceiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::B_ConvectiveHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Walls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWalls);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Ceiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::FisherPedersenCeilDiffuserCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Floor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::C_CentralAirHeat_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWindow);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_Walls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq3WallAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_StableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::AlamdariHammondStableHorizontal);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_UnstableHoriz];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::KhalifaEq4CeilingAwayFromHeat);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_StableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonStableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_UnstableTilted];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::WaltonUnstableHorizontalOrTilt);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::D_ZoneFanCirc_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::ISO15099Windows);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_AssistFlowWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedAssistingWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_OpposFlowWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedOppossingWall);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_StableFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedStableFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_UnstableFloor];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedUnstableFloor);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_StableCeiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedStableCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_UnstableCeiling];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::BeausoleilMorrisonMixedUnstableCeiling);
    algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::E_MixedBuoy_Windows];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::GoldsteinNovoselacCeilingDiffuserWindow);

    Convect::HcExt algo2;
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallWindward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::SparrowWindward);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallLeeward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::SparrowLeeward);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_HorizRoof];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::ClearRoof);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_VertWall];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalASHRAEVerticalWall);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_StableHoriz];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalWaltonStableHorizontalOrTilt);
    algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::NaturalConvection_UnstableHoriz];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::NaturalWaltonUnstableHorizontalOrTilt);

    state->dataHeatBalSurf->SurfTempInTmp.deallocate();
}

TEST_F(ConvectionCoefficientsFixture, TestASTMC1340)
{
    Real64 Tsurf;
    Real64 Tair;
    Real64 AirStreamV;
    Real64 Tilt;
    Real64 Hin;

    state->dataSurface->Surface.allocate(3);
    state->dataHeatBal->Zone.allocate(3);
    state->dataSurface->SurfOutWindSpeed.allocate(3);
    // Horizontal Roof, heat flow down
    state->dataSurface->Surface(1).Zone = 1;
    state->dataHeatBal->Zone(1).Volume = 1000;
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Roof;
    state->dataSurface->Surface(1).Tilt = 0;
    state->dataSurface->Surface(1).Area = 100;
    state->dataSurface->Surface(1).ExtBoundCond = 0;
    state->dataSurface->SurfOutWindSpeed(1) = 1;

    Tsurf = 18.0;
    Tair = 15.0;
    AirStreamV = 2.0;
    Tilt = state->dataSurface->Surface(1).Tilt;

    Hin = Convect::CalcASTMC1340ConvCoeff(*state, 1, Tsurf, Tair, AirStreamV, Tilt);

    EXPECT_NEAR(Hin, 1.977, 0.001);

    // Pitched Roof, heat flow up
    state->dataSurface->Surface(2).Zone = 2;
    state->dataHeatBal->Zone(2).Volume = 1000;
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Roof;
    state->dataSurface->Surface(2).Tilt = 20;
    state->dataSurface->Surface(2).Area = 100;
    state->dataSurface->Surface(2).ExtBoundCond = 0;
    state->dataSurface->Surface(2).Height = 8;
    state->dataSurface->SurfOutWindSpeed(2) = 1;

    Tsurf = 15.0;
    Tair = 18.0;
    AirStreamV = 2.0;
    Tilt = state->dataSurface->Surface(2).Tilt;

    Hin = Convect::CalcASTMC1340ConvCoeff(*state, 2, Tsurf, Tair, AirStreamV, Tilt);

    EXPECT_NEAR(Hin, 2.666, 0.001);

    // Vertical Wall
    state->dataSurface->Surface(3).Zone = 3;
    state->dataHeatBal->Zone(3).Volume = 1000;
    state->dataSurface->Surface(3).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 90;
    state->dataSurface->Surface(3).Area = 100;
    state->dataSurface->Surface(3).ExtBoundCond = 1;
    state->dataSurface->Surface(3).Height = 3;

    Tsurf = 15.0;
    Tair = 18.0;
    AirStreamV = 0.0055;
    Tilt = state->dataSurface->Surface(3).Tilt;

    Hin = Convect::CalcASTMC1340ConvCoeff(*state, 3, Tsurf, Tair, AirStreamV, Tilt);

    EXPECT_NEAR(Hin, 1.756, 0.001);
}

TEST_F(ConvectionCoefficientsFixture, TestSetAdaptiveConvectionAlgoCoefficient)
{

    std::string const idf_objects = delimited_string({

        "SurfaceConvectionAlgorithm:Inside,AdaptiveConvectionAlgorithm;",
        "SurfaceConvectionAlgorithm:Outside,AdaptiveConvectionAlgorithm;",

        "SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections,",
        "Default Algorithm,       !- Name",
        "UserCurve,               !- Simple Buoyancy Vertical Wall Equation Source",
        "ASHRAE Vert Duplicate,   !- Simple Buoyancy Vertical Wall User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Simple Buoyancy Stable Horizontal Equation Source",
        ",                        !- Simple Buoyancy Stable Horizontal Equation User Curve Name",
        "AlamdariHammondUnstableHorizontal,  !- Simple Buoyancy Unstable Horizontal Equation Source",
        ",                        !- Simple Buoyancy Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Simple Buoyancy Stable Tilted Equation Source",
        ",                        !- Simple Buoyancy Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Simple Buoyancy Unstable Tilted Equation Source",
        ",                        !- Simple Buoyancy Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Simple Buoyancy Windows Equation Source",
        ",                        !- Simple Buoyancy Windows Equation User Curve Name",
        "KhalifaEq3WallAwayFromHeat,  !- Floor Heat Ceiling Cool Vertical Wall Equation Source",
        ",                        !- Floor Heat Ceiling Cool Vertical Wall Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Floor Heat Ceiling Cool Stable Horizontal Equation Source",
        ",                        !- Floor Heat Ceiling Cool Stable Horizontal Equation User Curve Name",
        "KhalifaEq4CeilingAwayFromHeat,  !- Floor Heat Ceiling Cool Unstable Horizontal Equation Source",
        ",                        !- Floor Heat Ceiling Cool Unstable Horizontal Equation User Curve Name",
        "AwbiHattonHeatedFloor,   !- Floor Heat Ceiling Cool Heated Floor Equation Source",
        ",                        !- Floor Heat Ceiling Cool Heated Floor Equation User Curve Name",
        "KaradagChilledCeiling,   !- Floor Heat Ceiling Cool Chilled Ceiling Equation Source",
        ",                        !- Floor Heat Ceiling Cool Chilled Ceiling Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Floor Heat Ceiling Cool Stable Tilted Equation Source",
        ",                        !- Floor Heat Ceiling Cool Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Floor Heat Ceiling Cool Unstable Tilted Equation Source",
        ",                        !- Floor Heat Ceiling Cool Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Floor Heat Ceiling Cool Window Equation Source",
        ",                        !- Floor Heat Ceiling Cool Window Equation User Curve Name",
        "KhalifaEq6NonHeatedWalls,!- Wall Panel Heating Vertical Wall Equation Source",
        ",                        !- Wall Panel Heating Vertical Wall Equation User Curve Name",
        "AwbiHattonHeatedWall,    !- Wall Panel Heating Heated Wall Equation Source",
        ",                        !- Wall Panel Heating Heated Wall Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Wall Panel Heating Stable Horizontal Equation Source",
        ",                        !- Wall Panel Heating Stable Horizontal Equation User Curve Name",
        "KhalifaEq7Ceiling,       !- Wall Panel Heating Unstable Horizontal Equation Source",
        ",                        !- Wall Panel Heating Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Wall Panel Heating Stable Tilted Equation Source",
        ",                        !- Wall Panel Heating Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Wall Panel Heating Unstable Tilted Equation Source",
        ",                        !- Wall Panel Heating Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Wall Panel Heating Window Equation Source",
        ",                        !- Wall Panel Heating Window Equation User Curve Name",
        "FohannoPolidoriVerticalWall,  !- Convective Zone Heater Vertical Wall Equation Source",
        ",                        !- Convective Zone Heater Vertical Wall Equation User Curve Name",
        "KhalifaEq5WallNearHeat,  !- Convective Zone Heater Vertical Walls Near Heater Equation Source",
        ",                        !- Convective Zone Heater Vertical Walls Near Heater Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Convective Zone Heater Stable Horizontal Equation Source",
        ",                        !- Convective Zone Heater Stable Horizontal Equation User Curve Name",
        "KhalifaEq7Ceiling,       !- Convective Zone Heater Unstable Horizontal Equation Source",
        ",                        !- Convective Zone Heater Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Convective Zone Heater Stable Tilted Equation Source",
        ",                        !- Convective Zone Heater Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Convective Zone Heater Unstable Tilted Equation Source",
        ",                        !- Convective Zone Heater Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Convective Zone Heater Windows Equation Source",
        ",                        !- Convective Zone Heater Windows Equation User Curve Name",
        "GoldsteinNovoselacCeilingDiffuserWalls,  !- Central Air Diffuser Wall Equation Source",
        ",                        !- Central Air Diffuser Wall Equation User Curve Name",
        "FisherPedersenCeilingDiffuserCeiling,  !- Central Air Diffuser Ceiling Equation Source",
        ",                        !- Central Air Diffuser Ceiling Equation User Curve Name",
        "GoldsteinNovoselacCeilingDiffuserFloor,  !- Central Air Diffuser Floor Equation Source",
        ",                        !- Central Air Diffuser Floor Equation User Curve Name",
        "GoldsteinNovoselacCeilingDiffuserWindow,  !- Central Air Diffuser Window Equation Source",
        ",                        !- Central Air Diffuser Window Equation User Curve Name",
        "KhalifaEq3WallAwayFromHeat,  !- Mechanical Zone Fan Circulation Vertical Wall Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Vertical Wall Equation User Curve Name",
        "AlamdariHammondStableHorizontal,  !- Mechanical Zone Fan Circulation Stable Horizontal Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Stable Horizontal Equation User Curve Name",
        "KhalifaEq4CeilingAwayFromHeat,  !- Mechanical Zone Fan Circulation Unstable Horizontal Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Unstable Horizontal Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Mechanical Zone Fan Circulation Stable Tilted Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Stable Tilted Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Mechanical Zone Fan Circulation Unstable Tilted Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Unstable Tilted Equation User Curve Name",
        "ISO15099Windows,         !- Mechanical Zone Fan Circulation Window Equation Source",
        ",                        !- Mechanical Zone Fan Circulation Window Equation User Curve Name",
        ",                        !- Mixed Regime Buoyancy Assisting Flow on Walls Equation Source",
        ",                        !- Mixed Regime Buoyancy Assisting Flow on Walls Equation User Curve Name",
        ",                        !- Mixed Regime Buoyancy Opposing Flow on Walls Equation Source",
        ",                        !- Mixed Regime Buoyancy Opposing Flow on Walls Equation User Curve Name",
        ",                        !- Mixed Regime Stable Floor Equation Source",
        ",                        !- Mixed Regime Stable Floor Equation User Curve Name",
        ",                        !- Mixed Regime Unstable Floor Equation Source",
        ",                        !- Mixed Regime Unstable Floor Equation User Curve Name",
        ",                        !- Mixed Regime Stable Ceiling Equation Source",
        ",                        !- Mixed Regime Stable Ceiling Equation User Curve Name",
        ",                        !- Mixed Regime Unstable Ceiling Equation Source",
        ",                        !- Mixed Regime Unstable Ceiling Equation User Curve Name",
        ",                        !- Mixed Regime Window Equation Source",
        ";                        !- Mixed Regime Window Equation User Curve Name",

        "SurfaceConvectionAlgorithm:Outside:AdaptiveModelSelections,",
        "Default algorithms,      !- Name",
        "UserCurve,               !- Wind Convection Windward Vertical Wall Equation Source",
        "NusseltJurgesDupCurve,   !- Wind Convection Windward Equation Vertical Wall User Curve Name",
        "TARPLeeward,             !- Wind Convection Leeward Vertical Wall Equation Source",
        ",                        !- Wind Convection Leeward Vertical Wall Equation User Curve Name",
        "ClearRoof,               !- Wind Convection Horizontal Roof Equation Source",
        ",                        !- Wind Convection Horizontal Roof User Curve Name",
        "ASHRAEVerticalWall,      !- Natural Convection Vertical Wall Equation Source",
        ",                        !- Natural Convection Vertical Wall Equation User Curve Name",
        "WaltonStableHorizontalOrTilt,  !- Natural Convection Stable Horizontal Equation Source",
        ",                        !- Natural Convection Stable Horizontal Equation User Curve Name",
        "WaltonUnstableHorizontalOrTilt,  !- Natural Convection Unstable Horizontal Equation Source",
        ";                        !- Natural Convection Unstable Horizontal Equation User Curve Name",

        "SurfaceConvectionAlgorithm:Inside:UserCurve,",
        "ASHRAE Vert Duplicate,   !- Name",
        "MeanAirTemperature,      !- Reference Temperature for Convection Heat Transfer",
        "ASHRAEVerticalModel,     !- Hc Function of Temperature Difference Curve Name",
        ",                        !- Hc Function of Temperature Difference Divided by Height Curve Name",
        ",                        !- Hc Function of Air Change Rate Curve Name",
        ";                        !- Hc Function of Air System Volume Flow Rate Divided by Zone Perimeter Length Curve Name",

        "Curve:Exponent,",
        "ASHRAEVerticalModel,     !- Name",
        "0.0,                     !- Coefficient1 Constant",
        "1.31,                    !- Coefficient2 Constant",
        "0.33333333333333333,     !- Coefficient3 Constant",
        "0.1,                     !- Minimum Value of x",
        "50.0,                    !- Maximum Value of x",
        "0.1,                     !- Minimum Curve Output",
        "20.0;                    !- Maximum Curve Output",

        "SurfaceConvectionAlgorithm:Outside:UserCurve,",
        "NusseltJurgesDupCurve,   !- Name",
        "HeightAdjust,            !- Wind Speed Type for Curve",
        "MyNusseltJurgesCurve,    !- Hf Function of Wind Speed Curve Name",
        ",                        !- Hn Function of Temperature Difference Curve Name",
        ";                        !- Hn Function of Temperature Difference Divided by Height Curve Name",

        "Curve:Linear,",
        "MyNusseltJurgesCurve,    !- Name",
        "5.8,                     !- Coefficient1 Constant",
        "3.94,                    !- Coefficient2 x",
        "0.0,                     !- Minimum Value of x",
        "100.0;                   !- Maximum Value of x",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;
    Convect::InitIntConvCoeff(*state, state->dataHeatBalSurf->SurfTempInTmp);
    Convect::GetUserConvCoeffs(*state);

    int curve;
    int expected_curve;

    Convect::HcInt algo = state->dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)Convect::IntConvClass::A3_SimpleBuoy_VertWalls];
    EXPECT_ENUM_EQ(algo, Convect::HcInt::UserCurve);
    curve = state->dataConvect->intAdaptiveConvAlgo.intConvClassUserCurveNums[(int)Convect::IntConvClass::A3_SimpleBuoy_VertWalls];
    expected_curve = Util::FindItemInList("ASHRAE VERT DUPLICATE", state->dataConvect->hcIntUserCurve);
    ASSERT_EQ(curve, expected_curve);

    Convect::HcExt algo2 = state->dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)Convect::ExtConvClass2::WindConvection_WallWindward];
    EXPECT_ENUM_EQ(algo2, Convect::HcExt::UserCurve);
    curve = state->dataConvect->extAdaptiveConvAlgo.extConvClass2UserCurveNums[(int)Convect::ExtConvClass2::WindConvection_WallWindward];
    expected_curve = Util::FindItemInList("NUSSELTJURGESDUPCURVE", state->dataConvect->hcExtUserCurve);
}

TEST_F(ConvectionCoefficientsFixture, TestCalcWindSurfaceTheta)
{
    // theta angle for surface orientations of 0-360 deg with 0-360 deg wind angles
    static constexpr std::array<std::array<Real64, 13>, 13> expectedVals = {{{0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0},
                                                                             {30, 0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30},
                                                                             {60, 30, 0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60},
                                                                             {90, 60, 30, 0, 30, 60, 90, 120, 150, 180, 150, 120, 90},
                                                                             {120, 90, 60, 30, 0, 30, 60, 90, 120, 150, 180, 150, 120},
                                                                             {150, 120, 90, 60, 30, 0, 30, 60, 90, 120, 150, 180, 150},
                                                                             {180, 150, 120, 90, 60, 30, 0, 30, 60, 90, 120, 150, 180},
                                                                             {150, 180, 150, 120, 90, 60, 30, 0, 30, 60, 90, 120, 150},
                                                                             {120, 150, 180, 150, 120, 90, 60, 30, 0, 30, 60, 90, 120},
                                                                             {90, 120, 150, 180, 150, 120, 90, 60, 30, 0, 30, 60, 90},
                                                                             {60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0, 30, 60},
                                                                             {30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0, 30},
                                                                             {0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0}}};

    static constexpr std::array<Real64, 13> angles = {0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 360};

    for (int idxWindDir = 0; idxWindDir < 13; idxWindDir++) {
        Real64 windDir = angles[idxWindDir];
        for (int idxSurfAz = 0; idxSurfAz < 13; idxSurfAz++) {
            Real64 surfAz = angles[idxSurfAz];
            Real64 expectedVal = expectedVals[idxWindDir][idxSurfAz];
            Real64 actualVal = Convect::CalcWindSurfaceTheta(windDir, surfAz);
            EXPECT_EQ(expectedVal, actualVal);
        }
    }
}

TEST_F(ConvectionCoefficientsFixture, TestEmmelVertical)
{
    // wind speeds
    static constexpr std::array<Real64, 15> windSpeedAt10m{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // expected values
    static constexpr std::array<Real64, 15> actualHcZeroDegTheta = {
        5.150, 9.029, 12.539, 15.829, 18.965, 21.984, 24.907, 27.753, 30.531, 33.251, 35.919, 38.542, 41.124, 43.668, 46.178};

    static constexpr std::array<Real64, 15> actualHcFortyFiveDegTheta = {
        3.340, 5.978, 8.404, 10.702, 12.908, 15.045, 17.124, 19.157, 21.149, 23.107, 25.033, 26.931, 28.804, 30.654, 32.483};

    static constexpr std::array<Real64, 15> actualHcNinetyDegTheta = {
        4.780, 7.819, 10.427, 12.790, 14.986, 17.057, 19.030, 20.922, 22.747, 24.514, 26.231, 27.902, 29.534, 31.129, 32.692};

    static constexpr std::array<Real64, 15> actualHcOneThirtyFiveDegTheta = {
        4.050, 6.906, 9.437, 11.777, 13.985, 16.093, 18.121, 20.083, 21.990, 23.848, 25.664, 27.443, 29.187, 30.901, 32.587};

    static constexpr std::array<Real64, 15> actualHcOneEightyDegTheta = {
        3.540, 5.995, 8.159, 10.152, 12.029, 13.817, 15.534, 17.193, 18.803, 20.371, 21.901, 23.398, 24.866, 26.306, 27.723};

    for (int idx = 0; idx < 15; idx++) {
        Real64 windSpeed = windSpeedAt10m[idx];

        // test at 0 deg theta
        Real64 actualHc = actualHcZeroDegTheta[idx];
        Real64 expectedHc = Convect::CalcEmmelVertical(windSpeed, 0, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 45 deg theta
        actualHc = actualHcFortyFiveDegTheta[idx];
        expectedHc = Convect::CalcEmmelVertical(windSpeed, 45, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 90 deg theta
        actualHc = actualHcNinetyDegTheta[idx];
        expectedHc = Convect::CalcEmmelVertical(windSpeed, 90, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 135 deg theta
        actualHc = actualHcOneThirtyFiveDegTheta[idx];
        expectedHc = Convect::CalcEmmelVertical(windSpeed, 135, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 180 deg theta
        actualHc = actualHcOneEightyDegTheta[idx];
        expectedHc = Convect::CalcEmmelVertical(windSpeed, 180, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);
    }
}

TEST_F(ConvectionCoefficientsFixture, TestEmmelRoof)
{
    // wind speeds
    static constexpr std::array<Real64, 15> windSpeedAt10m{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // expected values
    static constexpr std::array<Real64, 15> actualHcZeroDegTheta = {
        5.110, 8.775, 12.039, 15.067, 17.932, 20.672, 23.313, 25.872, 28.362, 30.791, 33.167, 35.496, 37.783, 40.031, 42.245};

    static constexpr std::array<Real64, 15> actualHcFortyFiveDegTheta = {
        4.600, 7.954, 10.957, 13.753, 16.404, 18.945, 21.399, 23.779, 26.098, 28.363, 30.581, 32.758, 34.896, 37.000, 39.072};

    static constexpr std::array<Real64, 15> actualHcNinetyDegTheta = {
        3.670, 6.615, 9.337, 11.924, 14.414, 16.830, 19.187, 21.493, 23.756, 25.982, 28.174, 30.337, 32.473, 34.584, 36.673};

    static constexpr std::array<Real64, 15> actualHcOneThirtyFiveDegTheta = {
        4.600, 7.954, 10.957, 13.753, 16.404, 18.945, 21.399, 23.779, 26.098, 28.363, 30.581, 32.758, 34.896, 37.000, 39.072};

    static constexpr std::array<Real64, 15> actualHcOneEightyDegTheta = {
        5.110, 8.775, 12.039, 15.067, 17.932, 20.672, 23.313, 25.872, 28.362, 30.791, 33.167, 35.496, 37.783, 40.031, 42.245};

    for (int idx = 0; idx < 15; idx++) {
        Real64 windSpeed = windSpeedAt10m[idx];

        // test at 0 deg theta
        Real64 actualHc = actualHcZeroDegTheta[idx];
        Real64 expectedHc = Convect::CalcEmmelRoof(windSpeed, 0, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 45 deg theta
        actualHc = actualHcFortyFiveDegTheta[idx];
        expectedHc = Convect::CalcEmmelRoof(windSpeed, 45, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 90 deg theta
        actualHc = actualHcNinetyDegTheta[idx];
        expectedHc = Convect::CalcEmmelRoof(windSpeed, 90, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 135 deg theta
        actualHc = actualHcOneThirtyFiveDegTheta[idx];
        expectedHc = Convect::CalcEmmelRoof(windSpeed, 135, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 180 deg theta
        actualHc = actualHcOneEightyDegTheta[idx];
        expectedHc = Convect::CalcEmmelRoof(windSpeed, 180, 0);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);
    }
}

TEST_F(ConvectionCoefficientsFixture, TestBlockenWindward)
{
    std::string const idf_objects = this->getIDFString();

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    SurfaceGeometry::GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data

    // wind speeds
    static constexpr std::array<Real64, 15> windSpeedAt10m{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // expected values
    static constexpr std::array<Real64, 15> actualHcZeroDegTheta = {
        4.600, 8.525, 12.229, 15.798, 19.268, 22.663, 25.995, 29.276, 32.511, 35.707, 38.868, 41.998, 45.099, 48.174, 51.225};

    static constexpr std::array<Real64, 15> actualHcTwentyTwoDegTheta = {
        5.000, 8.706, 12.041, 15.157, 18.119, 20.965, 23.716, 26.390, 28.998, 31.548, 34.047, 36.502, 38.916, 41.293, 43.636};

    static constexpr std::array<Real64, 15> actualHcFortyFiveTheta = {
        4.600, 8.234, 11.575, 14.740, 17.778, 20.721, 23.585, 26.385, 29.129, 31.824, 34.477, 37.091, 39.671, 42.219, 44.738};

    static constexpr std::array<Real64, 15> actualHcSixtySevenDegTheta = {
        4.500, 7.889, 10.957, 13.832, 16.572, 19.209, 21.764, 24.250, 26.678, 29.054, 31.386, 33.678, 35.934, 38.157, 40.350};

    static constexpr std::array<Real64, 15> emmelVertActualHcOneThirtyFiveDegTheta = {
        4.050, 6.906, 9.437, 11.777, 13.985, 16.093, 18.121, 20.083, 21.990, 23.848, 25.664, 27.443, 29.187, 30.901, 32.587};

    for (int idx = 0; idx < 15; idx++) {
        Real64 windSpeed = windSpeedAt10m[idx];

        // test at 0 deg theta
        Real64 actualHc = actualHcZeroDegTheta[idx];
        Real64 expectedHc = Convect::CalcBlockenWindward(*state, windSpeed, 0, 0, 1);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 22.5 deg theta
        actualHc = actualHcTwentyTwoDegTheta[idx];
        expectedHc = Convect::CalcBlockenWindward(*state, windSpeed, 22.5, 0, 1);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 45 deg theta
        actualHc = actualHcFortyFiveTheta[idx];
        expectedHc = Convect::CalcBlockenWindward(*state, windSpeed, 45, 0, 1);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 67.5 deg theta
        actualHc = actualHcSixtySevenDegTheta[idx];
        expectedHc = Convect::CalcBlockenWindward(*state, windSpeed, 67.5, 0, 1);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);

        // test at 120 deg. should throw warnings and pick up the EmmelVertical correlation
        actualHc = emmelVertActualHcOneThirtyFiveDegTheta[idx];
        expectedHc = Convect::CalcBlockenWindward(*state, windSpeed, 135, 0, 1);
        ASSERT_NEAR(actualHc, expectedHc, 0.001);
        ASSERT_EQ(state->dataConvect->CalcBlockenWindwardErrorIDX, 1);
    }
}

TEST_F(ConvectionCoefficientsFixture, TestMultipleSurfaceConvectionArrayAllocation1)
{
    std::string const idf_objects = delimited_string({
        "SurfaceProperty:ConvectionCoefficients:MultipleSurface,",
        "AllExteriorWindows,      !- Surface Type",
        "Inside,                 !- Convection Coefficient 1 Location",
        "MoWitt,                  !- Convection Coefficient 1 Type",
        ", !- Convection Coefficient 1",
        ", !- Convection Coefficient 1 Schedule Name",
        ", !- Convection Coefficient 1 User Curve Name",
        "Inside,                 !- Convection Coefficient 2 Location",
        "MoWitt,                  !- Convection Coefficient 2 Type",
        ", !- Convection Coefficient 2",
        ", !- Convection Coefficient 2 Schedule Name",
        "; !- Convection Coefficient 2 User Curve Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSurface->surfIntConv.allocate(2);
    state->dataSurface->surfExtConv.allocate(2);

    Convect::GetUserConvCoeffs(*state);
    EXPECT_EQ(state->dataSurface->userIntConvModels.size(), 2u);
    EXPECT_EQ(state->dataSurface->userExtConvModels.size(), 0u);
}

TEST_F(ConvectionCoefficientsFixture, TestMultipleSurfaceConvectionArrayAllocation2)
{
    std::string const idf_objects = delimited_string({
        "SurfaceProperty:ConvectionCoefficients:MultipleSurface,",
        "AllExteriorSurfaces,      !- Surface Type",
        "Outside,                 !- Convection Coefficient 1 Location",
        "MoWitt,                  !- Convection Coefficient 1 Type",
        ", !- Convection Coefficient 1",
        ", !- Convection Coefficient 1 Schedule Name",
        ", !- Convection Coefficient 1 User Curve Name",
        "Outside,                 !- Convection Coefficient 2 Location",
        "MoWitt,                  !- Convection Coefficient 2 Type",
        ", !- Convection Coefficient 2",
        ", !- Convection Coefficient 2 Schedule Name",
        "; !- Convection Coefficient 2 User Curve Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSurface->surfIntConv.allocate(2);
    state->dataSurface->surfExtConv.allocate(2);

    Convect::GetUserConvCoeffs(*state);
    EXPECT_EQ(state->dataSurface->userIntConvModels.size(), 0u);
    EXPECT_EQ(state->dataSurface->userExtConvModels.size(), 2u);
}

TEST_F(ConvectionCoefficientsFixture, TestSurfaceConvectionArrayAllocation1)
{
    std::string const idf_objects = delimited_string({"SurfaceProperty:ConvectionCoefficients,",
                                                      "  FakeSurface, !- Surface Name",
                                                      "  Inside, !- Convection Coefficient 1 Location",
                                                      "  Value, !- Convection Coefficient 1 Type",
                                                      "  1, !- Convection Coefficient 1",
                                                      "  FakeSchName, !- Convection Coefficient 1 Schedule Name",
                                                      "  FakeCurve, !- Convection Coefficient 1 User Curve Name",
                                                      "  Outside, !- Convection Coefficient 2 Location",
                                                      "  Value, !- Convection Coefficient 2 Type",
                                                      "  10;   !- Convection Coefficient 2"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSurface->Surface.allocate(2);
    state->dataSurface->Surface(1).Name = "FAKESURFACE";
    state->dataSurface->surfIntConv.allocate(1);
    state->dataSurface->surfIntConv(1).model = Convect::HcInt::SetByZone;
    state->dataSurface->surfIntConv(1).userModelNum = 0;
    state->dataSurface->surfExtConv.allocate(1);
    state->dataSurface->surfExtConv(1).model = Convect::HcExt::SetByZone;
    state->dataSurface->surfExtConv(1).userModelNum = 0;

    Convect::GetUserConvCoeffs(*state);
    EXPECT_EQ(state->dataSurface->userIntConvModels.size(), 1u);
    EXPECT_EQ(state->dataSurface->userExtConvModels.size(), 1u);
}

TEST_F(ConvectionCoefficientsFixture, TestSurfaceConvectionArrayAllocation2)
{
    std::string const idf_objects = delimited_string({"SurfaceProperty:ConvectionCoefficients,",
                                                      "  FakeSurface, !- Surface Name",
                                                      "  Outside, !- Convection Coefficient 1 Location",
                                                      "  Value, !- Convection Coefficient 1 Type",
                                                      "  1, !- Convection Coefficient 1",
                                                      "  FakeSchName, !- Convection Coefficient 1 Schedule Name",
                                                      "  FakeCurve, !- Convection Coefficient 1 User Curve Name",
                                                      "  Inside, !- Convection Coefficient 2 Location",
                                                      "  Value, !- Convection Coefficient 2 Type",
                                                      "  10;   !- Convection Coefficient 2"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSurface->Surface.allocate(2);
    state->dataSurface->Surface(1).Name = "FAKESURFACE";
    state->dataSurface->surfIntConv.allocate(1);
    state->dataSurface->surfIntConv(1).model = Convect::HcInt::SetByZone;
    state->dataSurface->surfIntConv(1).userModelNum = 0;
    state->dataSurface->surfExtConv.allocate(1);
    state->dataSurface->surfExtConv(1).model = Convect::HcExt::SetByZone;
    state->dataSurface->surfExtConv(1).userModelNum = 0;

    Convect::GetUserConvCoeffs(*state);
    EXPECT_EQ(state->dataSurface->userIntConvModels.size(), 1u);
    EXPECT_EQ(state->dataSurface->userExtConvModels.size(), 1u);
}

TEST_F(ConvectionCoefficientsFixture, RoofPerimeter_PerfectSquare_Rotated)
{

    // Test for #9432
    // So this is a perfect square. Expect I am going to rotate the building by 15 degrees (Not by building north axis, but by vertices adjustments)
    // Surface.Vertex(1) will end up having the highest X and highest Y, and we will trigger a bug where if will fail
    // the XdYu ones (lo X, high Y), that is the top left corner [2] because  the RoofGeo BoundingBoxVertStruct are initialized to Vertex(1) and:
    //     vertex.x <= RoofGeo.XdYuZd.Vertex.x => True
    //     vertex.y => RoofGeo.XdYuZd.Vertex.y => False
    //
    //          y
    //         ▲      ◄──┐ Rotate by 15 degrees
    //       10│         │
    // [2]┌────┼────┐[1]
    //    │    │    │
    //    │    │    │
    //  ──┼────┼────┼────►
    // -10│    │    │10   x
    //    │    │    │
    // [3]└────┼────┘[4]
    //      -10│

    std::string const idf_objects = delimited_string({

        "GlobalGeometryRules,",
        "  UpperLeftCorner,                        !- Starting Vertex Position",
        "  Counterclockwise,                       !- Vertex Entry Direction",
        "  Relative,                               !- Coordinate System",
        "  Relative,                               !- Daylighting Reference Point Coordinate System",
        "  Relative;                               !- Rectangular Surface Coordinate System",

        "Building,",
        "  Building 1,                             !- Name",
        "  20,                                     !- North Axis {deg}",
        "  ,                                       !- Terrain",
        "  ,                                       !- Loads Convergence Tolerance Value {W}",
        "  ,                                       !- Temperature Convergence Tolerance Value {deltaC}",
        "  ,                                       !- Solar Distribution",
        "  ,                                       !- Maximum Number of Warmup Days",
        "  ;                                       !- Minimum Number of Warmup Days",

        "Zone,",
        "  Zone1,                                  !- Name",
        "  0,                                      !- Direction of Relative North {deg}",
        "  0,                                      !- X Origin {m}",
        "  0,                                      !- Y Origin {m}",
        "  0,                                      !- Z Origin {m}",
        "  ,                                       !- Type",
        "  1,                                      !- Multiplier",
        "  ,                                       !- Ceiling Height {m}",
        "  ,                                       !- Volume {m3}",
        "  ,                                       !- Floor Area {m2}",
        "  ,                                       !- Zone Inside Convection Algorithm",
        "  ,                                       !- Zone Outside Convection Algorithm",
        "  Yes;                                    !- Part of Total Floor Area",

        "Material:NoMass,",
        "  R13-IP,                                 !- Name",
        "  Smooth,                                 !- Roughness",
        "  2.28943238786998,                       !- Thermal Resistance {m2-K/W}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Construction,",
        "  R13 Construction,                       !- Name",
        "  R13-IP;                                 !- Layer 1",

        "BuildingSurface:Detailed,",
        "  1-SOUTH - ABS AZIMUTH 165.00,           !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  -7.07106781186548, -12.2474487139159, 3, !- X,Y,Z Vertex 1 {m}",
        "  -7.07106781186548, -12.2474487139159, 0, !- X,Y,Z Vertex 2 {m}",
        "  12.2474487139159, -7.07106781186548, 0, !- X,Y,Z Vertex 3 {m}",
        "  12.2474487139159, -7.07106781186548, 3; !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  2-WEST - ABS AZIMUTH 255.00,            !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  -12.2474487139159, 7.07106781186548, 3, !- X,Y,Z Vertex 1 {m}",
        "  -12.2474487139159, 7.07106781186548, 0, !- X,Y,Z Vertex 2 {m}",
        "  -7.07106781186548, -12.2474487139159, 0, !- X,Y,Z Vertex 3 {m}",
        "  -7.07106781186548, -12.2474487139159, 3; !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  3-EAST - ABS AZIMUTH 75.00,             !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  12.2474487139159, -7.07106781186548, 3, !- X,Y,Z Vertex 1 {m}",
        "  12.2474487139159, -7.07106781186548, 0, !- X,Y,Z Vertex 2 {m}",
        "  7.07106781186548, 12.2474487139159, 0,  !- X,Y,Z Vertex 3 {m}",
        "  7.07106781186548, 12.2474487139159, 3;  !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  4-NORTH - ABS AZIMUTH 345.00,           !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  7.07106781186548, 12.2474487139159, 3,  !- X,Y,Z Vertex 1 {m}",
        "  7.07106781186548, 12.2474487139159, 0,  !- X,Y,Z Vertex 2 {m}",
        "  -12.2474487139159, 7.07106781186548, 0, !- X,Y,Z Vertex 3 {m}",
        "  -12.2474487139159, 7.07106781186548, 3; !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  FLOOR,                                  !- Name",
        "  Floor,                                  !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  12.2474487139159, -7.07106781186548, 0, !- X,Y,Z Vertex 1 {m}",
        "  -7.07106781186548, -12.2474487139159, 0, !- X,Y,Z Vertex 2 {m}",
        "  -12.2474487139159, 7.07106781186548, 0, !- X,Y,Z Vertex 3 {m}",
        "  7.07106781186548, 12.2474487139159, 0;  !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ROOF,                                   !- Name",
        "  Roof,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  7.07106781186548, 12.2474487139159, 3,  !- X,Y,Z Vertex 1 {m}",
        "  -12.2474487139159, 7.07106781186548, 3, !- X,Y,Z Vertex 2 {m}",
        "  -7.07106781186548, -12.2474487139159, 3, !- X,Y,Z Vertex 3 {m}",
        "  12.2474487139159, -7.07106781186548, 3; !- X,Y,Z Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // expect no errors

    // TODO: I think GetSurfaceData is enough? SetupZoneGeometry => GetSurfaceData => CalcSurfaceCentroid
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    // This is a square of width 20, just rotated around the center point, so the centroid is still xy=(0, 0) and the perimeter is 80 m

    auto &surfaces = state->dataSurface->Surface;
    auto it = std::find_if(surfaces.begin(), surfaces.end(), [](const auto &s) { return s.Name == "ROOF"; });
    ASSERT_NE(it, surfaces.end());
    auto &surface = *it;
    EXPECT_NEAR(surface.Centroid.x, 0.0, 0.0001);
    EXPECT_NEAR(surface.Centroid.y, 0.0, 0.0001);
    EXPECT_NEAR(surface.Centroid.z, 3.0, 0.0001);

    // GetUserConvectionCoefficients => SetupAdaptiveConvectionStaticMetaData (which is where the perimeter thing is calculated)
    SurfaceGeometry::GeoSummary geoSummaryRoof;
    SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);

    double actual_roof_perimeter = 0.0;
    for (int i = 1; i <= surface.Sides; ++i) {
        int inext = i + 1;
        if (i == surface.Sides) {
            inext = 1;
        }
        actual_roof_perimeter += distance(surface.Vertex(i), surface.Vertex(inext));
    }
    EXPECT_NEAR(80.0, actual_roof_perimeter, 0.0001);

    EXPECT_NEAR(actual_roof_perimeter, geoSummaryRoof.Perimeter, 0.0001);
}

TEST_F(ConvectionCoefficientsFixture, RoofPerimeter_WeirderShape)
{
    // Test for #9432
    //
    // This is a top view of the building:
    //                  y
    //                   ▲
    //                   │
    //          [2]      │
    //           x    20 ┤
    //                   │         [1]
    //                15 ┤       x
    //                   │
    //                   ┤
    //      Centroid     │
    //   (-1.39, 0.60)   ┤
    //          │        │
    //          └────► o │
    //  ─┬───┬───┬───┬───┼───┬───┬───┬───┬───►
    //                   │   5   10      20   x
    //                   ┤               x
    //                   │                 [4]
    //                   ┤
    //                   │
    //                   ┤
    //                   │
    //   x               ┤
    //    [3]            │
    //                   │

    std::string const idf_objects = delimited_string({
        "GlobalGeometryRules,",
        "  UpperLeftCorner,                        !- Starting Vertex Position",
        "  Counterclockwise,                       !- Vertex Entry Direction",
        "  Relative,                               !- Coordinate System",
        "  Relative,                               !- Daylighting Reference Point Coordinate System",
        "  Relative;                               !- Rectangular Surface Coordinate System",

        "Building,",
        "  Building 1,                             !- Name",
        "  0,                                      !- North Axis {deg}",
        "  ,                                       !- Terrain",
        "  ,                                       !- Loads Convergence Tolerance Value {W}",
        "  ,                                       !- Temperature Convergence Tolerance Value {deltaC}",
        "  ,                                       !- Solar Distribution",
        "  ,                                       !- Maximum Number of Warmup Days",
        "  ;                                       !- Minimum Number of Warmup Days",

        "Zone,",
        "  Zone1,                                  !- Name",
        "  0,                                      !- Direction of Relative North {deg}",
        "  0,                                      !- X Origin {m}",
        "  0,                                      !- Y Origin {m}",
        "  0,                                      !- Z Origin {m}",
        "  ,                                       !- Type",
        "  1,                                      !- Multiplier",
        "  ,                                       !- Ceiling Height {m}",
        "  ,                                       !- Volume {m3}",
        "  ,                                       !- Floor Area {m2}",
        "  ,                                       !- Zone Inside Convection Algorithm",
        "  ,                                       !- Zone Outside Convection Algorithm",
        "  Yes;                                    !- Part of Total Floor Area",

        "Material:NoMass,",
        "  R13-IP,                                 !- Name",
        "  Smooth,                                 !- Roughness",
        "  2.28943238786998,                       !- Thermal Resistance {m2-K/W}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Construction,",
        "  R13 Construction,                       !- Name",
        "  R13-IP;                                 !- Layer 1",

        "BuildingSurface:Detailed,",
        "  1-SOUTH - ABS AZIMUTH 159.44,           !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  -20, -20, 3,                            !- X,Y,Z Vertex 1 {m}",
        "  -20, -20, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  20, -5, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, -5, 3;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  2-WEST - ABS AZIMUTH 284.04,            !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  -10, 20, 3,                             !- X,Y,Z Vertex 1 {m}",
        "  -10, 20, 0,                             !- X,Y,Z Vertex 2 {m}",
        "  -20, -20, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  -20, -20, 3;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  3-EAST - ABS AZIMUTH 63.43,             !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, -5, 3,                              !- X,Y,Z Vertex 1 {m}",
        "  20, -5, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  10, 15, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 15, 3;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  4-NORTH - ABS AZIMUTH 14.04,            !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 15, 3,                              !- X,Y,Z Vertex 1 {m}",
        "  10, 15, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  -10, 20, 0,                             !- X,Y,Z Vertex 3 {m}",
        "  -10, 20, 3;                             !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  FLOOR,                                  !- Name",
        "  Floor,                                  !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, -5, 0,                              !- X,Y,Z Vertex 1 {m}",
        "  -20, -20, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  -10, 20, 0,                             !- X,Y,Z Vertex 3 {m}",
        "  10, 15, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ROOF,                                   !- Name",
        "  Roof,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 15, 3,                              !- X,Y,Z Vertex 1 {m}",
        "  -10, 20, 3,                             !- X,Y,Z Vertex 2 {m}",
        "  -20, -20, 3,                            !- X,Y,Z Vertex 3 {m}",
        "  20, -5, 3;                              !- X,Y,Z Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // expect no errors

    // TODO: I think GetSurfaceData is enough? SetupZoneGeometry => GetSurfaceData => CalcSurfaceCentroid
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    auto &surfaces = state->dataSurface->Surface;
    auto it = std::find_if(surfaces.begin(), surfaces.end(), [](const auto &s) { return s.Name == "ROOF"; });
    ASSERT_NE(it, surfaces.end());
    auto &surface = *it;
    EXPECT_NEAR(surface.Centroid.x, -1.38889, 0.0001);
    EXPECT_NEAR(surface.Centroid.y, 0.601852, 0.0001);
    EXPECT_NEAR(surface.Centroid.z, 3.0, 0.0001);

    // GetUserConvectionCoefficients => SetupAdaptiveConvectionStaticMetaData (which is where the perimeter thing is calculated)
    SurfaceGeometry::GeoSummary geoSummaryRoof;
    SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);

    double actual_roof_perimeter = 0.0;
    for (int i = 1; i <= surface.Sides; ++i) {
        int inext = i + 1;
        if (i == surface.Sides) {
            inext = 1;
        }
        actual_roof_perimeter += distance(surface.Vertex(i), surface.Vertex(inext));
    }
    EXPECT_NEAR(126.92728, actual_roof_perimeter, 0.0001);

    EXPECT_NEAR(actual_roof_perimeter, geoSummaryRoof.Perimeter, 0.0001);
}

TEST_F(ConvectionCoefficientsFixture, RoofGeometryInformation)
{

    {
        state->dataSurface->Surface.allocate(1);

        // 20 x 20 rectangle, centered on zero
        auto &surf1 = state->dataSurface->Surface(1);
        surf1.Name = "Normal Surface";
        surf1.Sides = 4;
        surf1.Vertex.dimension(4);
        surf1.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf1.Tilt = 0.0;
        surf1.Azimuth = 0.0;
        surf1.Area = 400.0;
        surf1.Vertex(1) = Vector3<Real64>(10.0, 10.0, 3.0);
        surf1.Vertex(2) = Vector3<Real64>(-10.0, 10.0, 3.0);
        surf1.Vertex(3) = Vector3<Real64>(-10.0, -10.0, 3.0);
        surf1.Vertex(4) = Vector3<Real64>(10.0, -10.0, 3.0);
        surf1.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf1.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(400.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(80.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Azimuth);
    }

    {
        // Same, just translated by Vector(20.0, 0.0, 0.0) so that it's next to it (sharing one edge)
        state->dataSurface->Surface.resize(2);
        auto &surf2 = state->dataSurface->Surface(2);
        surf2.Name = "Translated Normal Surface";
        surf2.Sides = 4;
        surf2.Vertex.dimension(4);
        surf2.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf2.Tilt = 0.0;
        surf2.Azimuth = 0.0;
        surf2.Area = 400.0;
        surf2.Vertex(1) = Vector3<Real64>(30.0, 10.0, 3.0);
        surf2.Vertex(2) = Vector3<Real64>(10.0, 10.0, 3.0);
        surf2.Vertex(3) = Vector3<Real64>(10.0, -10.0, 3.0);
        surf2.Vertex(4) = Vector3<Real64>(30.0, -10.0, 3.0);
        surf2.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf2.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(800.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(120.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Azimuth);
    }

    {
        // Same, just translated by Vector(20.0, 0.0, 3.0) so that it's next to it but at a different height (not sharing an edge)
        state->dataSurface->Surface.resize(3);
        auto &surf3 = state->dataSurface->Surface(3);
        surf3.Name = "Translated Normal Surface different Z";
        surf3.Sides = 4;
        surf3.Vertex.dimension(4);
        surf3.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf3.Tilt = 0.0;
        surf3.Azimuth = 0.0;
        surf3.Area = 400.0;
        surf3.Vertex(1) = Vector3<Real64>(50.0, 10.0, 6.0);
        surf3.Vertex(2) = Vector3<Real64>(30.0, 10.0, 6.0);
        surf3.Vertex(3) = Vector3<Real64>(30.0, -10.0, 6.0);
        surf3.Vertex(4) = Vector3<Real64>(50.0, -10.0, 6.0);
        surf3.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf3.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(1200.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(200.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Azimuth);
    }

    // This is getting confusing, let's clear and restart
    state->dataSurface->Surface.deallocate();

    {
        state->dataSurface->Surface.allocate(1);

        auto &surf1 = state->dataSurface->Surface(1);
        // 20 x 20 rectangle, centered on zero
        surf1.Name = "Titled Roof 1";
        surf1.Sides = 4;
        surf1.Vertex.dimension(4);
        surf1.Class = DataSurfaces::SurfaceClass::Wall;
        surf1.Tilt = 22.61986494804042;
        surf1.Azimuth = 270.0;
        surf1.Area = 130.0;
        surf1.Vertex(1) = Vector3<Real64>(12.0, 10.0, 5.0);
        surf1.Vertex(2) = Vector3<Real64>(0.0, 10.0, 0.0);
        surf1.Vertex(3) = Vector3<Real64>(0.0, 0.0, 0.0);
        surf1.Vertex(4) = Vector3<Real64>(12.0, 0.0, 5.0);
        surf1.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf1.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(130.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(5.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ((13.0 + 10.0) * 2.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(22.61986494804042, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(270, geoSummaryRoof.Azimuth);
    }
    {
        // This becomes a gabbled roof
        state->dataSurface->Surface.resize(2);

        auto &surf2 = state->dataSurface->Surface(2);
        // 20 x 20 rectangle, centered on zero
        surf2.Name = "Titled Roof 2";
        surf2.Sides = 4;
        surf2.Vertex.dimension(4);
        surf2.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf2.Tilt = 22.61986494804042;
        surf2.Azimuth = 90.0;
        surf2.Area = 130.0;
        surf2.Vertex(1) = Vector3<Real64>(24.0, 10.0, 0.0);
        surf2.Vertex(2) = Vector3<Real64>(12.0, 10.0, 5.0);
        surf2.Vertex(3) = Vector3<Real64>(12.0, 0.0, 5.0);
        surf2.Vertex(4) = Vector3<Real64>(24.0, 0.0, 0.0);
        surf2.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf2.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(260.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(5.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(13.0 * 4 + 10.0 * 2.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(22.61986494804042, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(180.0, geoSummaryRoof.Azimuth);
    }
    {
        // This becomes a gabbled roof + another surface that is flat (horizontal) and that has has the same area as the two gabbled ones combined
        state->dataSurface->Surface.resize(3);
        auto &surf3 = state->dataSurface->Surface(3);
        // 20 x 20 rectangle, centered on zero
        surf3.Name = "Flat Roof";
        surf3.Sides = 4;
        surf3.Vertex.dimension(4);
        surf3.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf3.Tilt = 0.0;
        surf3.Azimuth = 0.0;
        surf3.Area = 260.0;
        surf3.Vertex(1) = Vector3<Real64>(50.0, 10.0, 0.0);
        surf3.Vertex(2) = Vector3<Real64>(24.0, 10.0, 0.0);
        surf3.Vertex(3) = Vector3<Real64>(24.0, 0.0, 0.0);
        surf3.Vertex(4) = Vector3<Real64>(50.0, 0.0, 0.0);
        surf3.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf3.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(520.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(2.5, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(13.0 * 4 + 10.0 * 2.0 + 26.0 * 2, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(22.61986494804042 / 2.0, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(90.0, geoSummaryRoof.Azimuth);
    }

    // This is getting confusing, let's clear and restart
    state->dataSurface->Surface.deallocate();

    {
        state->dataSurface->Surface.allocate(1);

        auto &surf1 = state->dataSurface->Surface(1);
        // 20 x 20 rectangle, centered on zero
        surf1.Name = "Normal Surface";
        surf1.Sides = 4;
        surf1.Vertex.dimension(4);
        surf1.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf1.Tilt = 0.0;
        surf1.Azimuth = 0.0;
        surf1.Area = 400.0;
        surf1.Vertex(1) = Vector3<Real64>(10.0, 10.0, 3.0);
        surf1.Vertex(2) = Vector3<Real64>(-10.0, 10.0, 3.0);
        surf1.Vertex(3) = Vector3<Real64>(-10.0, -10.0, 3.0);
        surf1.Vertex(4) = Vector3<Real64>(10.0, -10.0, 3.0);
        surf1.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf1.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(400.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(80.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Azimuth);
    }

    {
        // Same, just translated by Vector3<Real64>(30.0, 0.0, 0.0) so that it's detached
        state->dataSurface->Surface.resize(2);
        auto &surf2 = state->dataSurface->Surface(2);
        surf2.Name = "Translated Normal Surface not touching";
        surf2.Sides = 4;
        surf2.Vertex.dimension(4);
        surf2.Class = EnergyPlus::DataSurfaces::SurfaceClass::Wall;
        surf2.Tilt = 0.0;
        surf2.Azimuth = 0.0;
        surf2.Area = 400.0;
        surf2.Vertex(1) = Vector3<Real64>(40.0, 10.0, 3.0);
        surf2.Vertex(2) = Vector3<Real64>(20.0, 10.0, 3.0);
        surf2.Vertex(3) = Vector3<Real64>(20.0, -10.0, 3.0);
        surf2.Vertex(4) = Vector3<Real64>(40.0, -10.0, 3.0);
        surf2.ExtBoundCond = EnergyPlus::DataSurfaces::ExternalEnvironment;
        surf2.HeatTransSurf = true;

        SurfaceGeometry::GeoSummary geoSummaryRoof;
        SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
        EXPECT_DOUBLE_EQ(800.0, geoSummaryRoof.Area);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Height);
        EXPECT_DOUBLE_EQ(160.0, geoSummaryRoof.Perimeter);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Tilt);
        EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Azimuth);
    }
}

TEST_F(ConvectionCoefficientsFixture, testTARPNaturalConvectionAlgorithm)
{

    Real64 surfT;
    Real64 ambT;
    Real64 cosTilt;
    Real64 expectedResult;
    Real64 actualResult;
    constexpr Real64 allowableTolerance = 0.00001;

    // Test 1a: cosTilt zero, positive delta T--surface is vertical (should use "reduced" convection correlation, vertical correlation would
    // return 1.31)
    surfT = 1.0;
    ambT = 0.0;
    cosTilt = 0.0;
    expectedResult = 1.31;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 1b: cosTilt zero, negative delta T--surface is vertical (should use "reduced" convection correlation, vertical correlation would
    // return 1.31)
    surfT = -1.0;
    ambT = 0.0;
    cosTilt = 0.0;
    expectedResult = 1.31;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 2a: cosTilt positive, negative delta T--use "reduced" convection correlation
    surfT = -1.0;
    ambT = 0.0;
    cosTilt = 0.01;
    expectedResult = 1.30029;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 2b: cosTilt negative, positive delta T--use "reduced" convection correlation
    surfT = 1.0;
    ambT = 0.0;
    cosTilt = -0.01;
    expectedResult = 1.30029;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 3a: cosTilt negative, negative delta T--use "enhanced" convection correlation
    surfT = -1.0;
    ambT = 0.0;
    cosTilt = -0.01;
    expectedResult = 1.31184;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 3b: cosTilt positive, positive delta T--use "enhanced" convection correlation
    surfT = 1.0;
    ambT = 0.0;
    cosTilt = 0.01;
    expectedResult = 1.31184;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 4a: zero delta T, cosTilt positive--use "vertical" convection correlation (answer should be zero)
    surfT = 1.0;
    ambT = 1.0;
    cosTilt = 0.01;
    expectedResult = 0.0;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 4b: zero delta T, cosTilt zero--use "vertical" convection correlation (answer should be zero)
    surfT = 1.0;
    ambT = 1.0;
    cosTilt = 0.0;
    expectedResult = 0.0;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);

    // Test 4c: zero delta T, cosTilt negative--use "vertical" convection correlation (answer should be zero)
    surfT = 1.0;
    ambT = 1.0;
    cosTilt = -0.01;
    expectedResult = 0.0;
    actualResult = Convect::CalcASHRAETARPNatural(surfT, ambT, cosTilt);
    EXPECT_NEAR(actualResult, expectedResult, allowableTolerance);
}

TEST_F(ConvectionCoefficientsFixture, RoofExtConvectionCoefficient)
{
    auto &s_mat = state->dataMaterial;

    state->dataSurface->Surface.allocate(1);

    auto &surf1 = state->dataSurface->Surface(1);
    // 20 x 20 rectangle, centered on zero
    surf1.Name = "Roof Surface";
    surf1.Sides = 4;
    surf1.Vertex.dimension(4);
    surf1.Class = DataSurfaces::SurfaceClass::Roof; // DataSurfaces::SurfaceClass::Roof
    surf1.Tilt = 0.0;
    surf1.Azimuth = 0.0;
    surf1.Area = 400.0;
    surf1.Vertex(1) = Vector3<Real64>(10.0, 10.0, 3.0);
    surf1.Vertex(2) = Vector3<Real64>(-10.0, 10.0, 3.0);
    surf1.Vertex(3) = Vector3<Real64>(-10.0, -10.0, 3.0);
    surf1.Vertex(4) = Vector3<Real64>(10.0, -10.0, 3.0);
    surf1.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    surf1.HeatTransSurf = true;
    surf1.Construction = 1;

    SurfaceGeometry::GeoSummary geoSummaryRoof;
    SurfaceGeometry::GetGeoSummaryRoof(*state, geoSummaryRoof);
    EXPECT_DOUBLE_EQ(400.0, geoSummaryRoof.Area);
    EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Height);
    EXPECT_DOUBLE_EQ(80.0, geoSummaryRoof.Perimeter);
    EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Tilt);
    EXPECT_DOUBLE_EQ(0.0, geoSummaryRoof.Azimuth);

    // constructs with regular single layer material
    state->dataConstruction->Construct.allocate(1);
    state->dataHeatBal->TotConstructs = 1;
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataConstruction->Construct(1).TotLayers = 1;
    state->dataConstruction->Construct(1).LayerPoint.allocate(1);
    state->dataConstruction->Construct(1).LayerPoint(1) = 1;

    // define material
    auto *mat1 = new Material::MaterialBase;
    s_mat->materials.push_back(mat1);
    mat1->AbsorpThermalFront = 0.1;
    mat1->Roughness = Material::SurfaceRoughness::Rough;
    mat1->Name = "Roof_Material";
    mat1->group = Material::Group::Regular;

    // set environment air conditions
    state->dataEnvrn->OutBaroPress = 101325.0; // Pa
    state->dataEnvrn->OutHumRat = 0.0075;      // kgH20/kgDryAir

    // set convection coefficient calc parameters
    int constexpr SurfNum = 1;
    Real64 constexpr SurfTemp = 20.0;
    Real64 constexpr AirTemp = 15.0;
    Real64 constexpr WindAtZ = 2.0; // m/s
    Real64 constexpr WindDirect = 0.0;
    Real64 RoofArea = geoSummaryRoof.Area;
    Real64 RoofPerimeter = geoSummaryRoof.Perimeter;

    // test 1: calc exterior convection coefficient
    // non zero roof area and non zero perimeter
    Real64 Hf_result1 = Convect::CalcClearRoof(*state, SurfNum, SurfTemp, AirTemp, WindAtZ, WindDirect, RoofArea, RoofPerimeter);
    EXPECT_NEAR(10.42, Hf_result1, 0.01); //
    EXPECT_EQ(surf1.ExtBoundCond, EnergyPlus::DataSurfaces::ExternalEnvironment);
    EXPECT_TRUE(compare_err_stream(""));

    // test 2: calc exterior convection coefficient
    // reset roof area and perimeter to 0.0 to generate error message
    RoofArea = 0.0;
    RoofPerimeter = 0.0;
    Real64 Hf_result2 = Convect::CalcClearRoof(*state, SurfNum, SurfTemp, AirTemp, WindAtZ, WindDirect, RoofArea, RoofPerimeter);
    EXPECT_NEAR(9.9999, Hf_result2, 0.0001);
    EXPECT_EQ(surf1.ExtBoundCond, EnergyPlus::DataSurfaces::ExternalEnvironment);
    // expect severe error message
    EXPECT_TRUE(has_err_output(false));
    std::string error_string =
        delimited_string({"   ** Severe  ** CalcClearRoof: Convection model not evaluated (bad value for distance to roof edge)\n   **   ~~~   ** "
                          "Value for distance to roof edge =0.000\n   **   ~~~   ** Occurs for surface named = Roof Surface\n   **   ~~~   ** "
                          "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues"});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // test 3: calc exterior convection coefficient
    // reset ext boundary conditions to OtherSideCondModeledExt and expect no error
    surf1.ExtBoundCond = EnergyPlus::DataSurfaces::OtherSideCondModeledExt;
    RoofArea = 0.0;
    RoofPerimeter = 0.0;
    Real64 Hf_result3 = Convect::CalcClearRoof(*state, SurfNum, SurfTemp, AirTemp, WindAtZ, WindDirect, RoofArea, RoofPerimeter);
    EXPECT_DOUBLE_EQ(9.9999, Hf_result3);
    EXPECT_EQ(surf1.ExtBoundCond, EnergyPlus::DataSurfaces::OtherSideCondModeledExt);
    EXPECT_TRUE(compare_err_stream(""));
}

TEST_F(ConvectionCoefficientsFixture, SurroundingSurfacesHRadCoefTest)
{
    std::string_view constexpr idf_objects = R"IDF(
      Zone,
        Zone,                         !- Name
        0,                            !- Direction of Relative North {deg}
        6.000000,                     !- X Origin {m}
        6.000000,                     !- Y Origin {m}
        0,                            !- Z Origin {m}
        1,                            !- Type
        1,                            !- Multiplier
        autocalculate,                !- Ceiling Height {m}
        autocalculate;                !- Volume {m3}
                          
	  Material,
        NothWallConcreteBlock,        !- Name
        MediumRough,                  !- Roughness
        0.1014984,                    !- Thickness {m}
        0.3805070,                    !- Conductivity {W/m-K}
        608.7016,                     !- Density {kg/m3}
        836.8000,                     !- Specific Heat {J/kg-K}
        0.9,                          !- Thermal Absorptance",
        0.6,                          !- Solar Absorptance",
        1.0;                          !- Visible Absorptance",

      Construction,
        NorthWallConstruction,        !- Name
        NothWallConcreteBlock;        !- Outside Layer

	  Material,
        EastWallConcreteBlock,        !- Name
        MediumRough,                  !- Roughness
        0.1014984,                    !- Thickness {m}
        0.3805070,                    !- Conductivity {W/m-K}
        608.7016,                     !- Density {kg/m3}
        836.8000,                     !- Specific Heat {J/kg-K}
        0.7,                          !- Thermal Absorptance",
        0.6,                          !- Solar Absorptance",
        1.0;                          !- Visible Absorptance",

      Construction,
        EastWallConstruction,         !- Name
        EastWallConcreteBlock;        !- Outside Layer

      BuildingSurface:Detailed,
        North-Wall,                   !- Name
        Wall,                         !- Surface Type
        NorthWallConstruction,        !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        SunExposed,                   !- Sun Exposure
        WindExposed,                  !- Wind Exposure
        0.1,                          !- View Factor to Ground
        4,                            !- Number of Vertices
        0.0, 0.0, 10.0,               !- X,Y,Z ==> Vertex 1 {m}
        0.0, 0.0, 0.0,                !- X,Y,Z ==> Vertex 2 {m}
        10.0, 0.0, 0.0,               !- X,Y,Z ==> Vertex 3 {m}
        10.0, 0.0, 10.0;              !- X,Y,Z ==> Vertex 4 {m}

      SurfaceProperty:LocalEnvironment,
        LocEnv:North-Wall,            !- Name
        North-Wall,                   !- Exterior Surface Name
        ,                             !- External Shading Fraction Schedule Name
        SrdSurfs:North-Wall;          !- Surrounding Surfaces Object Name

      SurfaceProperty:SurroundingSurfaces,
        SrdSurfs:North-Wall,          !- Name
        0.4,                          !- Sky View Factor
        Sky Temp Sch,                 !- Sky Temperature Schedule Name
        ,                             !- Ground View Factor
        ,                             !- Ground Temperature Schedule Name
        SrdSurfs:Surface 1,           !- Surrounding Surface 1 Name
        0.3,                          !- Surrounding Surface 1 View Factor
        Surrounding Temp Sch 1,       !- Surrounding Surface 1 Temperature Schedule Name
        SrdSurfs:Surface 2,           !- Surrounding Surface 2 Name
        0.2,                          !- Surrounding Surface 2 View Factor
        Surrounding Temp Sch 2;       !- Surrounding Surface 2 Temperature Schedule Name

      Schedule:Compact,
        Sky Temp Sch,                 !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 6.0;            !- Field 3

      BuildingSurface:Detailed,
        East-Wall,                    !- Name
        Wall,                         !- Surface Type
        EastWallConstruction,         !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        SunExposed,                   !- Sun Exposure
        WindExposed,                  !- Wind Exposure
        0.4,                          !- View Factor to Ground
        4,                            !- Number of Vertices
        10.0, 10.0, 0.0,              !- X,Y,Z ==> Vertex 1 {m}
        10.0, 10.0, 10.0,             !- X,Y,Z ==> Vertex 2 {m}
        10.0, 0.0, 10.0,              !- X,Y,Z ==> Vertex 3 {m}
        10.0, 0.0, 0.0;               !- X,Y,Z ==> Vertex 4 {m}

      SurfaceProperty:LocalEnvironment,
        LocEnv:East-Wall,             !- Name
        East-Wall,                    !- Exterior Surface Name
        ,                             !- External Shading Fraction Schedule Name
        SrdSurfs:East-Wall;           !- Surrounding Surfaces Object Name

      SurfaceProperty:SurroundingSurfaces,
        SrdSurfs:East-Wall,           !- Name
        0.3,                          !- Sky View Factor
        Sky Temp Sch,                 !- Sky Temperature Schedule Name
        ,                             !- Ground View Factor
        ,                             !- Ground Temperature Schedule Name
        SrdSurfs:Surface 3,           !- Surrounding Surface 1 Name
        0.1,                          !- Surrounding Surface 1 View Factor
        Surrounding Temp Sch 3,       !- Surrounding Surface 1 Temperature Schedule Name
        SrdSurfs:Surface 4,           !- Surrounding Surface 2 Name
        0.2,                          !- Surrounding Surface 2 View Factor
        Surrounding Temp Sch 4;       !- Surrounding Surface 2 Temperature Schedule Name
							
      Schedule:Compact,
        Surrounding Temp Sch 1,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 10.0;           !- Field 3

      Schedule:Compact,
        Surrounding Temp Sch 2,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 12.0;           !- Field 3

      Schedule:Compact,
        Surrounding Temp Sch 3,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 14.0;           !- Field 3

      Schedule:Compact,
        Surrounding Temp Sch 4,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 16.0;           !- Field 3

      BuildingSurface:Detailed,
        Floor,                        !- Name
        Floor,                        !- Surface Type
        NorthWallConstruction,        !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        NoSun,                        !- Sun Exposure
        NoWind,                       !- Wind Exposure
        1.0,                          !- View Factor to Ground
        4,                            !- Number of Vertices
        0.000000,0.000000,0,          !- X,Y,Z ==> Vertex 1 {m}
        0.000000,10.000000,0,         !- X,Y,Z ==> Vertex 2 {m}
        10.00000,10.000000,0,         !- X,Y,Z ==> Vertex 3 {m}
        10.00000,0.000000,0;          !- X,Y,Z ==> Vertex 4 {m}

    )IDF";

    bool ErrorsFound = false;
    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;
    state->dataGlobal->TimeStepZoneSec = 3600.0;

    // set global and environmental variables
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->HourOfDay = 15;
    state->dataEnvrn->Month = 7;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->OutBaroPress = 101325;

    state->dataHeatBal->ZoneIntGain.allocate(1);
    // createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    HeatBalanceManager::GetFrameAndDividerData(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    HeatBalanceManager::GetBuildingData(*state, ErrorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);
    //  reset sky and ground view factors
    HeatBalanceSurfaceManager::InitSurfacePropertyViewFactors(*state);
    // update schedule values for surrounding surfaces temperature
    Sched::UpdateScheduleVals(*state);
    HeatBalanceSurfaceManager::GetSurroundingSurfacesTemperatureAverage(*state);
    // set outside face temperature of the exterior surfaces
    state->dataHeatBalSurf->SurfOutsideTempHist(1).dimension(state->dataSurface->TotSurfaces, 0.0);
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(1) = 20.0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(2) = 30.0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(3) = 20.0;

    int surfNum = 0;
    int srdSurfsNum = 0;
    int srdSurfsPropNum = 0;
    // test 1: exterior north wall radiation coefficient
    srdSurfsPropNum = Util::FindItemInList("SRDSURFS:NORTH-WALL", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(srdSurfsPropNum).SurroundingSurfsPtr);
    surfNum = Util::FindItemInList("NORTH-WALL", state->dataSurface->Surface);
    auto &surface_north_wall = state->dataSurface->Surface(surfNum);
    srdSurfsNum = state->dataSurface->Surface(surfNum).SurfSurroundingSurfacesNum;
    auto &srdSurfsProperty_north = state->dataSurface->SurroundingSurfsProperty(srdSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.4, srdSurfsProperty_north.SkyViewFactor);
    EXPECT_NEAR(0.1, srdSurfsProperty_north.GroundViewFactor, 1.0e-06);
    //  check surrounding surfaces view factors
    EXPECT_EQ("SRDSURFS:SURFACE 1", srdSurfsProperty_north.SurroundingSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.3, srdSurfsProperty_north.SurroundingSurfs(1).ViewFactor);
    EXPECT_EQ("SRDSURFS:SURFACE 2", srdSurfsProperty_north.SurroundingSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.2, srdSurfsProperty_north.SurroundingSurfs(2).ViewFactor);
    // check surrounding surfaces view factors sum (viewed by an exterior surface)
    Real64 srdSurfacesViewFactorSum_result1 =
        srdSurfsProperty_north.SurroundingSurfs(1).ViewFactor + srdSurfsProperty_north.SurroundingSurfs(2).ViewFactor;
    EXPECT_DOUBLE_EQ(0.5, srdSurfacesViewFactorSum_result1);
    // calculate surrounding surface radiation coefficient
    Real64 surf_tempExtK = state->dataHeatBalSurf->SurfOutsideTempHist(1)(surfNum) + Constant::Kelvin;
    auto &north_wall_const = state->dataConstruction->Construct(surface_north_wall.Construction);
    auto const *north_wall_mat = state->dataMaterial->materials(north_wall_const.LayerPoint(1));
    Real64 surf_absExt_north_wall = north_wall_mat->AbsorpThermal; // LW emittance of the exterior surface
    // calculate surrounding surfaces radiation exchange coefficient
    Real64 north_wall_srdSurfsTK = surface_north_wall.SrdSurfTemp + Constant::Kelvin;
    EXPECT_NEAR(10.80, surface_north_wall.SrdSurfTemp, 1.0e-02);
    EXPECT_NEAR(0.9, surf_absExt_north_wall, 1.0e-06);
    Real64 HRadSrdSurf_result1 =
        Constant::StefanBoltzmann * 0.9 * 0.5 * (pow_4(surf_tempExtK) - pow_4(north_wall_srdSurfsTK)) / (surf_tempExtK - north_wall_srdSurfsTK);
    // get rad exchange coefficient for exterior north wall from the function
    Real64 HRadSrdSurf_func1 = Convect::SurroundingSurfacesRadCoeffAverage(*state, surfNum, surf_tempExtK, surf_absExt_north_wall);
    // check the radiation exchange coefficient
    EXPECT_DOUBLE_EQ(HRadSrdSurf_result1, 2.4525479915842845);
    EXPECT_DOUBLE_EQ(HRadSrdSurf_result1, HRadSrdSurf_func1);

    surfNum = 0;
    srdSurfsNum = 0;
    srdSurfsPropNum = 0;
    // test 2: exterior east wall radiation coefficient
    srdSurfsPropNum = Util::FindItemInList("SRDSURFS:EAST-WALL", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(2, state->dataSurface->SurfLocalEnvironment(srdSurfsPropNum).SurroundingSurfsPtr);
    surfNum = Util::FindItemInList("EAST-WALL", state->dataSurface->Surface);
    auto &surface_east_wall = state->dataSurface->Surface(surfNum);
    srdSurfsNum = state->dataSurface->Surface(surfNum).SurfSurroundingSurfacesNum;
    auto &srdSurfsProperty_east = state->dataSurface->SurroundingSurfsProperty(srdSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.3, srdSurfsProperty_east.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.4, srdSurfsProperty_east.GroundViewFactor);
    // check surrounding surfaces view factors
    EXPECT_EQ("SRDSURFS:SURFACE 3", srdSurfsProperty_east.SurroundingSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.1, srdSurfsProperty_east.SurroundingSurfs(1).ViewFactor);
    EXPECT_EQ("SRDSURFS:SURFACE 4", srdSurfsProperty_east.SurroundingSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.2, srdSurfsProperty_east.SurroundingSurfs(2).ViewFactor);
    // check surrounding surfaces view factors sum (viewed by an exterior surface)
    Real64 srdSurfacesViewFactorSum_result2 =
        srdSurfsProperty_east.SurroundingSurfs(1).ViewFactor + srdSurfsProperty_east.SurroundingSurfs(2).ViewFactor;
    EXPECT_DOUBLE_EQ(0.3, srdSurfacesViewFactorSum_result2);
    // calculate surrounding surface radiation coefficient
    surf_tempExtK = state->dataHeatBalSurf->SurfOutsideTempHist(1)(surfNum) + Constant::Kelvin;
    auto &east_wall_const = state->dataConstruction->Construct(surface_east_wall.Construction);
    auto const *east_wall_mat = state->dataMaterial->materials(east_wall_const.LayerPoint(1));
    Real64 surf_absExt_east_wall = east_wall_mat->AbsorpThermal; // LW emittance of the exterior surface
    // calculate surrounding surfaces radiation exchange coefficient
    Real64 east_wall_srdSurfsTK = surface_east_wall.SrdSurfTemp + Constant::Kelvin;
    EXPECT_NEAR(15.34, surface_east_wall.SrdSurfTemp, 1.0e-02);
    EXPECT_NEAR(0.7, surf_absExt_east_wall, 1.0e-06);
    Real64 HRadSrdSurf_result2 =
        Constant::StefanBoltzmann * 0.7 * 0.3 * (pow_4(surf_tempExtK) - pow_4(east_wall_srdSurfsTK)) / (surf_tempExtK - east_wall_srdSurfsTK);
    // get rad exchange coefficient for exterior east wall from the function
    Real64 HRadSrdSurf_func2 = Convect::SurroundingSurfacesRadCoeffAverage(*state, surfNum, surf_tempExtK, surf_absExt_east_wall);
    // check the radiation exchange coefficient
    EXPECT_DOUBLE_EQ(HRadSrdSurf_result2, 1.2336276277402503);
    EXPECT_DOUBLE_EQ(HRadSrdSurf_result2, HRadSrdSurf_func2);
}
