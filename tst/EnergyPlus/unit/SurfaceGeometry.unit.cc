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

// EnergyPlus::SurfaceGeometry Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include <algorithm>
#include <iterator>
#include <vector>

using namespace EnergyPlus;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::SurfaceGeometry;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::Material;

TEST_F(EnergyPlusFixture, BaseSurfaceRectangularTest)
{
    state->init_state(*state);
    // Test base surfaces for rectangular shape in ProcessSurfaceVertices

    state->dataSurface->TotSurfaces = 5;
    state->dataSurface->MaxVerticesPerSurface = 5;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->ShadeV.allocate(state->dataSurface->TotSurfaces);
    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
        state->dataSurface->Surface(SurfNum).Vertex.allocate(state->dataSurface->MaxVerticesPerSurface);
    }

    bool ErrorsFound(false);
    int ThisSurf(0);

    // Surface 1 - Rectangle
    ThisSurf = 1;
    state->dataSurface->Surface(ThisSurf).Azimuth = 180.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 90.0;
    state->dataSurface->Surface(ThisSurf).Sides = 4;
    state->dataSurface->Surface(ThisSurf).GrossArea = 10.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 2.0;

    state->dataSurface->Surface(ThisSurf).Vertex(4).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).z = 2.0;

    ProcessSurfaceVertices(*state, ThisSurf, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Rectangle, state->dataSurface->Surface(ThisSurf).Shape);

    // Surface 2 - Isosceles Trapezoid
    ThisSurf = 2;
    state->dataSurface->Surface(ThisSurf).Azimuth = 180.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 90.0;
    state->dataSurface->Surface(ThisSurf).Sides = 4;
    state->dataSurface->Surface(ThisSurf).GrossArea = 8.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 4.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 2.0;

    state->dataSurface->Surface(ThisSurf).Vertex(4).x = 1.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).z = 2.0;

    ProcessSurfaceVertices(*state, ThisSurf, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Quadrilateral, state->dataSurface->Surface(ThisSurf).Shape);

    // Surface 3 - Parallelogram
    ThisSurf = 3;
    state->dataSurface->Surface(ThisSurf).Azimuth = 180.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 90.0;
    state->dataSurface->Surface(ThisSurf).Sides = 4;
    state->dataSurface->Surface(ThisSurf).GrossArea = 10.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 7.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 2.0;

    state->dataSurface->Surface(ThisSurf).Vertex(4).x = 2.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).z = 2.0;

    ProcessSurfaceVertices(*state, ThisSurf, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Quadrilateral, state->dataSurface->Surface(ThisSurf).Shape);

    // Surface 4 - Triangle
    ThisSurf = 4;
    state->dataSurface->Surface(ThisSurf).Azimuth = 180.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 90.0;
    state->dataSurface->Surface(ThisSurf).Sides = 3;
    state->dataSurface->Surface(ThisSurf).GrossArea = 10.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 2.0;

    ProcessSurfaceVertices(*state, ThisSurf, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Triangle, state->dataSurface->Surface(ThisSurf).Shape);

    // Surface 5 - Polygon
    ThisSurf = 5;
    state->dataSurface->Surface(ThisSurf).Azimuth = 180.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 90.0;
    state->dataSurface->Surface(ThisSurf).Sides = 5;
    state->dataSurface->Surface(ThisSurf).GrossArea = 10.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 7.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 2.0;

    state->dataSurface->Surface(ThisSurf).Vertex(4).x = 3.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).z = 5.0;

    state->dataSurface->Surface(ThisSurf).Vertex(5).x = 1.0;
    state->dataSurface->Surface(ThisSurf).Vertex(5).y = 0.0;
    state->dataSurface->Surface(ThisSurf).Vertex(5).z = 3.0;

    ProcessSurfaceVertices(*state, ThisSurf, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Polygonal, state->dataSurface->Surface(ThisSurf).Shape);
}

TEST_F(EnergyPlusFixture, DataSurfaces_SurfaceShape)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        " BuildingSurface:Detailed,",
        "    Surface 1 - Triangle,    !- Name",
        "    Floor,                   !- Surface Type",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name",
        "    Zone1,                   !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    ,                        !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- Vertex 1 X-coordinate {m}",
        "    1.0, 2.0, 0.0,           !- Vertex 2 X-coordinate {m}",
        "    2.0, 0.0, 0.0;           !- Vertex 3 X-coordinate {m}",

        " BuildingSurface:Detailed,",
        "    Surface 2 - Quadrilateral,  !- Name",
        "    Floor,                   !- Surface Type",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name",
        "    Zone1,                   !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    ,                        !- Number of Vertices",
        "    -73.4395447868102,       !- Vertex 1 X-coordinate {m}",
        "    115.81641271866,         !- Vertex 1 Y-coordinate {m}",
        "    -4.90860981523342e-014,  !- Vertex 1 Z-coordinate {m}",
        "    -58.0249751030646,       !- Vertex 2 X-coordinate {m}",
        "    93.1706338416311,        !- Vertex 2 Y-coordinate {m}",
        "    -6.93120848813091e-014,  !- Vertex 2 Z-coordinate {m}",
        "    -68.9295447868101,       !- Vertex 3 X-coordinate {m}",
        "    74.3054685889134,        !- Vertex 3 Y-coordinate {m}",
        "    -6.06384403665968e-014,  !- Vertex 3 Z-coordinate {m}",
        "    -58.0345461881513,       !- Vertex 4 X-coordinate {m}",
        "    93.1761597101821,        !- Vertex 4 Y-coordinate {m}",
        "    -6.9300904918858e-014;   !- Vertex 4 Z-coordinate {m}",

        " BuildingSurface:Detailed,",
        "    Surface 3 - Rectangle,   !- Name",
        "    Wall,                    !- Surface Type",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name",
        "    Zone1,                   !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    ,                        !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    1.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 2 {m}",
        "    1.0, 0.0, 1.0,           !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 0.0, 1.0;           !- X,Y,Z ==> Vertex 4 {m}",

        " FenestrationSurface:Detailed,",
        "    Surface 4 - RectangularDoorWindow,    !- Name",
        "    Window,                  !- Surface Type",
        "    SINGLE PANE HW WINDOW,   !- Construction Name",
        "    Surface 3 - Rectangle,   !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    Autocalculate,           !- Number of Vertices",
        "    0.2, 0.0, 0.2,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.8, 0.0, 0.2,           !- X,Y,Z ==> Vertex 2 {m}",
        "    0.8, 0.0, 0.8,           !- X,Y,Z ==> Vertex 3 {m}",
        "    0.2, 0.0, 0.8;           !- X,Y,Z ==> Vertex 4 {m}",

        " Shading:Overhang:Projection,",
        "    Surface 5 - RectangularOverhang,  !- Name",
        "    Surface 9 - TriangularDoor,       !- Window or Door Name",
        "    .01,                     !- Height above Window or Door {m}",
        "    91,                      !- Tilt Angle from Window/Door {deg}",
        "    .01,                     !- Left extension from Window/Door Width {m}",
        "    .01,                     !- Right extension from Window/Door Width {m}",
        "    .2;                      !- Depth as Fraction of Window/Door Height {dimensionless}",

        " Shading:Fin:Projection,",
        "    Surface 6 - RectangularLeftFin,    !- Name",
        "    Surface 3 - Rectangle,   !- Window or Door Name",
        "    .01,                     !- Left Extension from Window/Door {m}",
        "    .01,                     !- Left Distance Above Top of Window {m}",
        "    .01,                     !- Left Distance Below Bottom of Window {m}",
        "    90,                      !- Left Tilt Angle from Window/Door {deg}",
        "    .1,                      !- Left Depth as Fraction of Window/Door Width {dimensionless}",
        "    0,                       !- Right Extension from Window/Door {m}",
        "    0,                       !- Right Distance Above Top of Window {m}",
        "    0,                       !- Right Distance Below Bottom of Window {m}",
        "    0,                       !- Right Tilt Angle from Window/Door {deg}",
        "    0;                       !- Right Depth as Fraction of Window/Door Width {dimensionless}",

        " Shading:Fin:Projection,",
        "    Surface 7 - RectangularRightFin,   !- Name",
        "    Surface 3 - Rectangle,   !- Window or Door Name",
        "    0,                       !- Left Extension from Window/Door {m}",
        "    0,                       !- Left Distance Above Top of Window {m}",
        "    0,                       !- Left Distance Below Bottom of Window {m}",
        "    0,                       !- Left Tilt Angle from Window/Door {deg}",
        "    0,                       !- Left Depth as Fraction of Window/Door Width {dimensionless}",
        "    .01,                     !- Right Extension from Window/Door {m}",
        "    .01,                     !- Right Distance Above Top of Window {m}",
        "    .01,                     !- Right Distance Below Bottom of Window {m}",
        "    90,                      !- Right Tilt Angle from Window/Door {deg}",
        "    .1;                      !- Right Depth as Fraction of Window/Door Width {dimensionless}",

        " FenestrationSurface:Detailed,",
        "    Surface 8 - TriangularWindow,    !- Name",
        "    Window,                  !- Surface Type",
        "    SINGLE PANE HW WINDOW,   !- Construction Name",
        "    Surface 3 - Rectangle,   !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    Autocalculate,           !- Number of Vertices",
        "    0.05, 0.0, 0.05,         !- X,Y,Z ==> Vertex 1 {m}",
        "    0.15, 0.0, 0.05,         !- X,Y,Z ==> Vertex 2 {m}",
        "    0.10, 0.0, 0.15;         !- X,Y,Z ==> Vertex 3 {m}",

        " FenestrationSurface:Detailed,",
        "    Surface 9 - TriangularDoor,      !- Name",
        "    Door,                    !- Surface Type",
        "    External door,           !- Construction Name",
        "    Surface 3 - Rectangle,   !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    3,                       !- Number of Vertices",
        "    0.80, 0.0, 0.05,         !- X,Y,Z ==> Vertex 1 {m}",
        "    0.95, 0.0, 0.05,         !- X,Y,Z ==> Vertex 2 {m}",
        "    0.90, 0.0, 0.15;         !- X,Y,Z ==> Vertex 3 {m}",

        " BuildingSurface:Detailed,",
        "    Surface 10 - Polygonal,  !- Name",
        "    Floor,                   !- Surface Type",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name",
        "    Zone1,                   !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    ,                        !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- Vertex 1 X-coordinate {m}",
        "    0.0, 1.0, 0.0,           !- Vertex 2 X-coordinate {m}",
        "    0.5, 2.0, 0.0,           !- Vertex 3 X-coordinate {m}",
        "    1.0, 1.0, 0.0,           !- Vertex 4 X-coordinate {m}",
        "    1.0, 0.0, 0.0;           !- Vertex 5 X-coordinate {m}",

        " Zone,",
        "    Zone1,                   !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0.0,                     !- X Origin {m}",
        "    0.0,                     !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    ,                        !- Type",
        "    ,                        !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    ,                        !- Volume {m3}",
        "    ,                        !- Floor Area {m2}",
        "    ,                        !- Zone Inside Convection Algorithm",
        "    ,                        !- Zone Outside Convection Algorithm",
        "    No;                      !- Part of Total Floor Area",

        " Construction,",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Name",
        "    MAT-CC05 4 HW CONCRETE,  !- Outside Layer",
        "    CP02 CARPET PAD;         !- Layer 2",

        " Construction,",
        "    External door,           !- Name",
        "    Painted Oak;             !- Outside Layer",

        " Material,",
        "    MAT-CC05 4 HW CONCRETE,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1016,                  !- Thickness {m}",
        "    1.311,                   !- Conductivity {W/m-K}",
        "    2240,                    !- Density {kg/m3}",
        "    836.800000000001,        !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.85,                    !- Solar Absorptance",
        "    0.85;                    !- Visible Absorptance",

        " Material,",
        "    Painted Oak,             !- Name",
        "    Rough,                   !- Roughness",
        "    0.035,                   !- Thickness {m}",
        "    0.19,                    !- Conductivity {W/m-K}",
        "    700,                     !- Density {kg/m3}",
        "    2390,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.5,                     !- Solar Absorptance",
        "    0.5;                     !- Visible Absorptance",

        " Material:NoMass,",
        "    CP02 CARPET PAD,         !- Name",
        "    Smooth,                  !- Roughness",
        "    0.1,                     !- Thermal Resistance {m2-K/W}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.8,                     !- Solar Absorptance",
        "    0.8;                     !- Visible Absorptance",

        " Construction,",
        "    SINGLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",

        " WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    6.0000001E-03,           !- Thickness {m}",
        "    0.7750000,               !- Solar Transmittance at Normal Incidence",
        "    7.1000002E-02,           !- Front Side Solar Reflectance at Normal Incidence",
        "    7.1000002E-02,           !- Back Side Solar Reflectance at Normal Incidence",
        "    0.8810000,               !- Visible Transmittance at Normal Incidence",
        "    7.9999998E-02,           !- Front Side Visible Reflectance at Normal Incidence",
        "    7.9999998E-02,           !- Back Side Visible Reflectance at Normal Incidence",
        "    0,                       !- Infrared Transmittance at Normal Incidence",
        "    0.8400000,               !- Front Side Infrared Hemispherical Emissivity",
        "    0.8400000,               !- Back Side Infrared Hemispherical Emissivity",
        "    0.9000000;               !- Conductivity {W/m-K}",

        "SurfaceConvectionAlgorithm:Inside,TARP;",

        "SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "HeatBalanceAlgorithm,ConductionTransferFunction;",

        "ZoneAirHeatBalanceAlgorithm,",
        "    AnalyticalSolution;      !- Algorithm",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data
    EXPECT_FALSE(ErrorsFound);           // expect no errors

    // compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::AllocateSurfaceArrays(*state);

    //  Adding additional surfaces will change the index of the following based on where the surfaces are added in the array.
    //	If adding new tests, break here and look at EnergyPlus::state->dataSurface->Surface to see the order.

    //	enum surfaceShape:Triangle = 1
    //	"Surface 1 - Triangle"
    int surfNum = Util::FindItemInList("SURFACE 1 - TRIANGLE", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Triangle, state->dataSurface->Surface(surfNum).Shape);

    //	enum surfaceShape:Quadrilateral = 2
    //	"Surface 2 - Quadrilateral"
    surfNum = Util::FindItemInList("SURFACE 2 - QUADRILATERAL", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Quadrilateral, state->dataSurface->Surface(surfNum).Shape);

    //	enum surfaceShape:Rectangle = 3
    //	"Surface 3 - Rectangle"
    surfNum = Util::FindItemInList("SURFACE 3 - RECTANGLE", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Rectangle, state->dataSurface->Surface(surfNum).Shape);

    //	enum surfaceShape:RectangularDoorWindow = 4
    //	"Surface 4 - RectangularDoorWindow"
    surfNum = Util::FindItemInList("SURFACE 4 - RECTANGULARDOORWINDOW", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::RectangularDoorWindow, state->dataSurface->Surface(surfNum).Shape);

    //	enum surfaceShape:RectangularOverhang = 5
    //	"Surface 5 - RectangularOverhang"
    surfNum = Util::FindItemInList("SURFACE 5 - RECTANGULAROVERHANG", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_NE(SurfaceShape::RectangularOverhang,
                   state->dataSurface->Surface(surfNum).Shape); // fins and overhangs will not get set to the proper surface shape.

    //	enum surfaceShape:RectangularLeftFin = 6
    //	"Surface 6 - RectangularLeftFin"
    surfNum = Util::FindItemInList("SURFACE 6 - RECTANGULARLEFTFIN Left", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_NE(SurfaceShape::RectangularLeftFin,
                   state->dataSurface->Surface(surfNum).Shape); // fins and overhangs will not get set to the proper surface shape.

    //	enum surfaceShape:RectangularRightFin = 7
    //	"Surface 7 - RectangularRightFin"
    surfNum = Util::FindItemInList("SURFACE 7 - RECTANGULARRIGHTFIN Right", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_NE(SurfaceShape::RectangularRightFin,
                   state->dataSurface->Surface(surfNum).Shape); // fins and overhangs will not get set to the proper surface shape.

    //	enum surfaceShape:TriangularWindow = 8
    //	"Surface 8 - TriangularWindow"
    surfNum = Util::FindItemInList("SURFACE 8 - TRIANGULARWINDOW", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::TriangularWindow, state->dataSurface->Surface(surfNum).Shape);

    //	enum surfaceShape:TriangularDoor = 9
    //	"Surface 9 - TriangularDoor"
    surfNum = Util::FindItemInList("SURFACE 9 - TRIANGULARDOOR", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::TriangularDoor, state->dataSurface->Surface(surfNum).Shape);

    //	enum surfaceShape:Polygonal = 10
    //	"Surface 10 - Polygonal"
    surfNum = Util::FindItemInList("SURFACE 10 - POLYGONAL", state->dataSurface->Surface);
    ProcessSurfaceVertices(*state, surfNum, ErrorsFound);
    EXPECT_ENUM_EQ(SurfaceShape::Polygonal, state->dataSurface->Surface(surfNum).Shape);
}

TEST_F(EnergyPlusFixture, ConfirmCheckSubSurfAzTiltNorm)
{
    state->init_state(*state);
    SurfaceData BaseSurface;
    SurfaceData SubSurface;
    bool surfaceError;

    // Case 1 - Base surface and subsurface face the same way - should be no error message and no surfaceError
    surfaceError = false;
    BaseSurface.Azimuth = 0.;
    BaseSurface.Tilt = 0.;
    BaseSurface.NewellSurfaceNormalVector.x = 0.;
    BaseSurface.NewellSurfaceNormalVector.y = 0.;
    BaseSurface.NewellSurfaceNormalVector.z = 1.;

    SubSurface.Azimuth = 0.;
    SubSurface.Tilt = 0.;
    SubSurface.NewellSurfaceNormalVector.x = 0.;
    SubSurface.NewellSurfaceNormalVector.y = 0.;
    SubSurface.NewellSurfaceNormalVector.z = 1.;
    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);
    EXPECT_FALSE(surfaceError);
    EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
    EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 1); // IDF version object

    // Case 2 - Base surface and subsurface face the opposite way - should be error message and surfaceError=true
    surfaceError = false;
    SubSurface.Azimuth = 180.;
    SubSurface.Tilt = 180.;
    SubSurface.NewellSurfaceNormalVector.x = 1.;
    SubSurface.NewellSurfaceNormalVector.y = 0.;
    SubSurface.NewellSurfaceNormalVector.z = 0.;
    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);
    EXPECT_TRUE(surfaceError);
    EXPECT_TRUE(has_err_output());

    // Case 3 - Base surface is horizontal and subsurface is different by 45 degrees azimuth - should be no warning message and surfaceError=false
    surfaceError = false;
    SubSurface.Azimuth = 45.;
    SubSurface.Tilt = 0.;
    SubSurface.NewellSurfaceNormalVector.x = 0.;
    SubSurface.NewellSurfaceNormalVector.y =
        1.; // This doesn't match the tilt and azimuth, but want it to be different so tilt and azimuth tests are executed
    SubSurface.NewellSurfaceNormalVector.z = 1.;
    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);
    EXPECT_FALSE(surfaceError);
    EXPECT_FALSE(has_err_output());

    // Case 4 - Base surface is not horizontal and subsurface is different by 45 degrees azimuth and tilt - should be warning error message but
    // surfaceError=false
    surfaceError = false;
    BaseSurface.Azimuth = 90.;
    BaseSurface.Tilt = 90.;
    BaseSurface.NewellSurfaceNormalVector.x = 1.;
    BaseSurface.NewellSurfaceNormalVector.y = 0.;
    BaseSurface.NewellSurfaceNormalVector.z = 0.;

    SubSurface.Azimuth = 45.;
    SubSurface.Tilt = 45.;
    SubSurface.NewellSurfaceNormalVector.x = 1.;
    SubSurface.NewellSurfaceNormalVector.y = 1.;
    SubSurface.NewellSurfaceNormalVector.z = 1.;
    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);
    EXPECT_FALSE(surfaceError);
    EXPECT_TRUE(has_err_output());
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_MakeMirrorSurface)
{
    std::string const idf_objects = delimited_string({
        "BuildingSurface:Detailed,",
        " FRONT-1,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " SunExposed,               !- Sun Exposure",
        " WindExposed,              !- Wind Exposure",
        " 0.50000,                  !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "Zone,",
        "  Space,                   !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  2.0,                     !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  6.,                      !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",
        " ",
        "Construction,",
        " INT-WALL-1,               !- Name",
        " GP02;                     !- Outside Layer",
        " ",
        "Material,",
        " GP02,                     !- Name",
        " MediumSmooth,             !- Roughness",
        " 1.5900001E-02,            !- Thickness{ m }",
        " 0.1600000,                !- Conductivity{ W / m - K }",
        " 801.0000,                 !- Density{ kg / m3 }",
        " 837.0000,                 !- Specific Heat{ J / kg - K }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.7500000,                !- Solar Absorptance",
        " 0.7500000;                !- Visible Absorptance",
        " ",
        "  Timestep, 4;",
        " ",
        "BUILDING, Bldg2, 0.0, Suburbs, .04, .4, FullExterior, 25, 6;",
        " ",
        "SimulationControl, YES, NO, NO, YES, NO;",
        " ",
        "  Site:Location,",
        "    Miami Intl Ap FL USA TMY3 WMO=722020E,    !- Name",
        "    25.82,                   !- Latitude {deg}",
        "    -80.30,                  !- Longitude {deg}",
        "    -5.00,                   !- Time Zone {hr}",
        "    11;                      !- Elevation {m}",
        " ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;
    GetMaterialData(*state, FoundError);
    GetConstructData(*state, FoundError);
    GetZoneData(*state, FoundError); // Read Zone data from input file
    state->dataHeatBal->AnyCTF = true;
    SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()

    EXPECT_FALSE(FoundError);

    // test coordinate on existing surface
    EXPECT_EQ(state->dataSurface->TotSurfaces, 1);

    EXPECT_EQ(state->dataSurface->Surface(state->dataSurface->TotSurfaces).Name, "FRONT-1");

    // move surface to SurfaceTmp since MakeMirrorSurface uses that array
    state->dataSurfaceGeometry->SurfaceTmp.allocate(10);
    state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces) = state->dataSurface->Surface(state->dataSurface->TotSurfaces);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Name, "FRONT-1");

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(1).x, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(1).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(1).z, 2.4);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(2).x, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(2).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(2).z, 0.);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(3).x, 30.5);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(3).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(3).z, 0.);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(4).x, 30.5);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(4).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(4).z, 2.4);

    MakeMirrorSurface(
        *state, state->dataSurface->TotSurfaces); // This call increments TotSurfaces so the references after this are for the created mirror surface

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Name, "Mir-FRONT-1");

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(1).x, 30.5);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(1).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(1).z, 2.4);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(2).x, 30.5);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(2).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(2).z, 0.);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(3).x, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(3).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(3).z, 0.);

    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(4).x, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(4).y, 0.);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(state->dataSurface->TotSurfaces).Vertex(4).z, 2.4);
}

TEST_F(EnergyPlusFixture, SurfacesGeometry_CalcSurfaceCentroid_NonconvexRealisticZ)
{
    state->init_state(*state);
    state->dataSurface->TotSurfaces = 10;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Roof;
    state->dataSurface->Surface(1).GrossArea = 1000.;
    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Vertex.allocate(4);

    state->dataSurface->Surface(1).Vertex(1).x = 2000.;
    state->dataSurface->Surface(1).Vertex(1).y = -1000.;
    state->dataSurface->Surface(1).Vertex(1).z = 10.;

    state->dataSurface->Surface(1).Vertex(2).x = 1.;
    state->dataSurface->Surface(1).Vertex(2).y = 0.;
    state->dataSurface->Surface(1).Vertex(2).z = 10.;

    state->dataSurface->Surface(1).Vertex(3).x = 2000.;
    state->dataSurface->Surface(1).Vertex(3).y = 1000.;
    state->dataSurface->Surface(1).Vertex(3).z = 10.;

    state->dataSurface->Surface(1).Vertex(4).x = 0.;
    state->dataSurface->Surface(1).Vertex(4).y = 0.;
    state->dataSurface->Surface(1).Vertex(4).z = 10.;

    CalcSurfaceCentroid(*state);

    EXPECT_EQ(state->dataSurface->Surface(1).Centroid.x, 667.);
    EXPECT_EQ(state->dataSurface->Surface(1).Centroid.y, 0.);
    EXPECT_EQ(state->dataSurface->Surface(1).Centroid.z, 10.);
}

TEST_F(EnergyPlusFixture, MakeEquivalentRectangle)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "FenestrationSurface:Detailed,                                   ",
        "	Surface-1-Rectangle,     !- Name                             ",
        "	Window,                  !- Surface Type                     ",
        "	SINGLE PANE HW WINDOW,   !- Construction Name                ",
        "	WallExample,             !- Building Surface Name            ",
        "	,                        !- Outside Boundary Condition Object",
        "	0.50000,                 !- View Factor to Ground            ",
        "	,                        !- Frame and Divider Name           ",
        "	1,                       !- Multiplier                       ",
        "	4,                       !- Number of Vertices               ",
        "	0.0, 11.4, 2.1,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 11.4, 0.9,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 3.8,  0.9,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 3.8,  2.1;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "                                                                ",
        "FenestrationSurface:Detailed,                                   ",
        "	Surface-2-Trapzoid,      !- Name                             ",
        "	Window,                  !- Surface Type                     ",
        "	SINGLE PANE HW WINDOW,   !- Construction Name                ",
        "	WallExample,             !- Building Surface Name            ",
        "	,                        !- Outside Boundary Condition Object",
        "	0.50000,                 !- View Factor to Ground            ",
        "	,                        !- Frame and Divider Name           ",
        "	1,                       !- Multiplier                       ",
        "	4,                       !- Number of Vertices               ",
        "	0.0, 11.2, 2.1,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 11.6, 0.9,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 3.6,  0.9,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 4.0,  2.1;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "                                                                ",
        "FenestrationSurface:Detailed,                                   ",
        "	Surface-3-parallelogram, !- Name                             ",
        "	Window,                  !- Surface Type                     ",
        "	SINGLE PANE HW WINDOW,   !- Construction Name                ",
        "	WallExample,             !- Building Surface Name            ",
        "	,                        !- Outside Boundary Condition Object",
        "	0.50000,                 !- View Factor to Ground            ",
        "	,                        !- Frame and Divider Name           ",
        "	1,                       !- Multiplier                       ",
        "	4,                       !- Number of Vertices               ",
        "	0.0, 11.4, 2.1,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 12.4, 0.9,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 4.8,  0.9,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 3.8,  2.1;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "                                                                ",
        "BuildingSurface:Detailed,                                       ",
        "	WallExample,   !- Name                                       ",
        "	Wall,                    !- Surface Type                     ",
        "	ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name     ",
        "	ZoneExample,             !- Zone Name                        ",
        "   ,                        !- Space Name",
        "	Outdoors,                !- Outside Boundary Condition       ",
        "	,                        !- Outside Boundary Condition Object",
        "	NoSun,                   !- Sun Exposure                     ",
        "	NoWind,                  !- Wind Exposure                    ",
        "	,                        !- View Factor to Ground            ",
        "	,                        !- Number of Vertices               ",
        "	0.0, 15.2, 2.4,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 15.2, 0.0,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 0.0,  0.0,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 0.0,  2.4;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "	                                                             ",
        "BuildingSurface:Detailed,                                       ",
        "	FloorExample,            !- Name                             ",
        "	Floor,                   !- Surface Type                     ",
        "	ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name     ",
        "	ZoneExample,             !- Zone Name                        ",
        "   ,                        !- Space Name",
        "	Outdoors,                !- Outside Boundary Condition       ",
        "	,                        !- Outside Boundary Condition Object",
        "	NoSun,                   !- Sun Exposure                     ",
        "	NoWind,                  !- Wind Exposure                    ",
        "	,                        !- View Factor to Ground            ",
        "	,                        !- Number of Vertices               ",
        "	0.0, 0.0,  0.0,          !- Vertex 1 X-coordinate {m}        ",
        "	5.0, 15.2, 0.0,          !- Vertex 2 X-coordinate {m}        ",
        "	0.0, 15.2, 0.0;          !- Vertex 3 X-coordinate {m}        ",
        "                                                                ",
        "Zone,                                                           ",
        "	ZoneExample,             !- Name                             ",
        "	0,                       !- Direction of Relative North {deg}",
        "	0.0,                     !- X Origin {m}                     ",
        "	0.0,                     !- Y Origin {m}                     ",
        "	0.0,                     !- Z Origin {m}                     ",
        "	,                        !- Type                             ",
        "	,                        !- Multiplier                       ",
        "	,                        !- Ceiling Height {m}               ",
        "	,                        !- Volume {m3}                      ",
        "	,                        !- Floor Area {m2}                  ",
        "	,                        !- Zone Inside Convection Algorithm ",
        "	,                        !- Zone Outside Convection Algorithm",
        "	No;                      !- Part of Total Floor Area         ",
        "                                                                ",
        "Construction,                                                   ",
        "	ExtSlabCarpet 4in ClimateZone 1-8,  !- Name                  ",
        "	MAT-CC05 4 HW CONCRETE,  !- Outside Layer                    ",
        "	CP02 CARPET PAD;         !- Layer 2                          ",
        "                                                                ",
        "Construction,                                                   ",
        "	SINGLE PANE HW WINDOW,   !- Name                             ",
        "	CLEAR 3MM;  !- Outside Layer                                 ",
        "                                                                ",
        "Material,                                                       ",
        "	MAT-CC05 4 HW CONCRETE,  !- Name                             ",
        "	Rough,                   !- Roughness                        ",
        "	0.1016,                  !- Thickness {m}                    ",
        "	1.311,                   !- Conductivity {W/m-K}             ",
        "	2240,                    !- Density {kg/m3}                  ",
        "	836.800000000001,        !- Specific Heat {J/kg-K}           ",
        "	0.9,                     !- Thermal Absorptance              ",
        "	0.85,                    !- Solar Absorptance                ",
        "	0.85;                    !- Visible Absorptance              ",
        "                                                                ",
        "Material:NoMass,                                                ",
        "	CP02 CARPET PAD,         !- Name                             ",
        "	Smooth,                  !- Roughness                        ",
        "	0.1,                     !- Thermal Resistance {m2-K/W}      ",
        "	0.9,                     !- Thermal Absorptance              ",
        "	0.8,                     !- Solar Absorptance                ",
        "	0.8;                     !- Visible Absorptance              ",
        "                                                                ",
        "WindowMaterial:Glazing,                                                          ",
        "	CLEAR 3MM,               !- Name                                              ",
        "	SpectralAverage,         !- Optical Data Type                                 ",
        "	,                        !- Window Glass Spectral Data Set Name               ",
        "	0.003,                   !- Thickness {m}                                     ",
        "	0.837,                   !- Solar Transmittance at Normal Incidence           ",
        "	0.075,                   !- Front Side Solar Reflectance at Normal Incidence  ",
        "	0.075,                   !- Back Side Solar Reflectance at Normal Incidence   ",
        "	0.898,                   !- Visible Transmittance at Normal Incidence         ",
        "	0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "	0.081,                   !- Back Side Visible Reflectance at Normal Incidence ",
        "	0.0,                     !- Infrared Transmittance at Normal Incidence        ",
        "	0.84,                    !- Front Side Infrared Hemispherical Emissivity      ",
        "	0.84,                    !- Back Side Infrared Hemispherical Emissivity       ",
        "	0.9;                     !- Conductivity {W/m-K}                              ",
    });

    // Prepare data for the test
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);
    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);
    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);
    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data
    EXPECT_FALSE(ErrorsFound);           // expect no errors

    // For each surface Run the test then Check the result
    // (1) rectangle window
    int surfNum = Util::FindItemInList("SURFACE-1-RECTANGLE", state->dataSurface->Surface);
    MakeEquivalentRectangle(*state, surfNum, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    EXPECT_NEAR(7.60, state->dataSurface->Surface(surfNum).Width, 0.01);
    EXPECT_NEAR(1.20, state->dataSurface->Surface(surfNum).Height, 0.01);
    // (2) trapzoid window
    surfNum = Util::FindItemInList("SURFACE-2-TRAPZOID", state->dataSurface->Surface);
    MakeEquivalentRectangle(*state, surfNum, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    EXPECT_NEAR(7.80, state->dataSurface->Surface(surfNum).Width, 0.01);
    EXPECT_NEAR(1.17, state->dataSurface->Surface(surfNum).Height, 0.01);
    // (3) parallelogram window
    surfNum = Util::FindItemInList("SURFACE-3-PARALLELOGRAM", state->dataSurface->Surface);
    MakeEquivalentRectangle(*state, surfNum, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    EXPECT_NEAR(8.08, state->dataSurface->Surface(surfNum).Width, 0.01);
    EXPECT_NEAR(1.13, state->dataSurface->Surface(surfNum).Height, 0.01);
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_distance)
{
    DataVectorTypes::Vector a;
    DataVectorTypes::Vector b;

    a.x = 0.;
    a.y = 0.;
    a.z = 0.;

    b.x = 0.;
    b.y = 0.;
    b.z = 0.;

    EXPECT_EQ(0., distance(a, b));

    b.x = 1.;
    b.y = 1.;
    b.z = 1.;

    EXPECT_NEAR(1.7321, distance(a, b), 0.0001);

    b.x = 7.;
    b.y = 11.;
    b.z = 17.;

    EXPECT_NEAR(21.4243, distance(a, b), 0.0001);

    a.x = 2.;
    a.y = 3.;
    a.z = 4.;

    EXPECT_NEAR(16.0624, distance(a, b), 0.0001);

    a.x = -2.;
    a.y = -3.;
    a.z = -4.;

    EXPECT_NEAR(26.7955, distance(a, b), 0.0001);
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isAlmostEqual3dPt)
{
    DataVectorTypes::Vector a;
    DataVectorTypes::Vector b;

    a.x = 0.;
    a.y = 0.;
    a.z = 0.;

    b.x = 0.;
    b.y = 0.;
    b.z = 0.;

    EXPECT_TRUE(isAlmostEqual3dPt(a, b));

    b.x = 7.;
    b.y = 11.;
    b.z = 17.;

    EXPECT_FALSE(isAlmostEqual3dPt(a, b));

    a.x = 7.;
    a.y = 11.;
    a.z = 17.;

    EXPECT_TRUE(isAlmostEqual3dPt(a, b));

    b.x = 7.0095;
    b.y = 11.0095;
    b.z = 17.0095;

    EXPECT_TRUE(isAlmostEqual3dPt(a, b));

    b.x = 7.05;
    b.y = 11.05;
    b.z = 17.05;

    EXPECT_FALSE(isAlmostEqual3dPt(a, b));

    a.x = -7.;
    a.y = -11.;
    a.z = -17.;

    b.x = -7.0095;
    b.y = -11.0095;
    b.z = -17.0095;

    EXPECT_TRUE(isAlmostEqual3dPt(a, b));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isAlmostEqual2dPt)
{
    DataVectorTypes::Vector_2d a;
    DataVectorTypes::Vector_2d b;

    a.x = 0.;
    a.y = 0.;

    b.x = 0.;
    b.y = 0.;

    EXPECT_TRUE(isAlmostEqual2dPt(a, b));

    b.x = 7.;
    b.y = 11.;

    EXPECT_FALSE(isAlmostEqual2dPt(a, b));

    a.x = 7.;
    a.y = 11.;

    EXPECT_TRUE(isAlmostEqual2dPt(a, b));

    b.x = 7.0095;
    b.y = 11.0095;

    EXPECT_TRUE(isAlmostEqual2dPt(a, b));

    b.x = 7.05;
    b.y = 11.05;

    EXPECT_FALSE(isAlmostEqual2dPt(a, b));

    a.x = -7.;
    a.y = -11.;

    b.x = -7.0095;
    b.y = -11.0095;

    EXPECT_TRUE(isAlmostEqual2dPt(a, b));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isPointOnLineBetweenPoints)
{
    DataVectorTypes::Vector a;
    DataVectorTypes::Vector b;
    DataVectorTypes::Vector t;

    a.x = 0.;
    a.y = 0.;
    a.z = 0.;

    b.x = 10.;
    b.y = 10.;
    b.z = 10.;

    t.x = 6.;
    t.y = 6.;
    t.z = 6.;

    EXPECT_TRUE(isPointOnLineBetweenPoints(a, b, t));

    t.x = 6.00;
    t.y = 6.01;
    t.z = 6.00;

    EXPECT_TRUE(isPointOnLineBetweenPoints(a, b, t));

    t.x = 7.;
    t.y = 11.;
    t.z = 17.;

    EXPECT_FALSE(isPointOnLineBetweenPoints(a, b, t));

    a.x = 5.;
    a.y = 3.;
    a.z = 13.;

    b.x = 7.;
    b.y = 11.;
    b.z = 4.;

    t.x = 6.;
    t.y = 7.;
    t.z = 8.5;

    EXPECT_TRUE(isPointOnLineBetweenPoints(a, b, t));

    a.x = -5.;
    a.y = 3.;
    a.z = -13.;

    b.x = 7.;
    b.y = -11.;
    b.z = 4.;

    t.x = 1.;
    t.y = -4.;
    t.z = -4.5;

    EXPECT_TRUE(isPointOnLineBetweenPoints(a, b, t));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isPointOnLineBetweenPoints_FalsePositive)
{
    // cf #7383

    DataVectorTypes::Vector a(0.0, 0.0, 0.0);
    DataVectorTypes::Vector b(30.0, 0.0, 0.0);
    DataVectorTypes::Vector t(15.0, 0.0, 0.3); // Notice wrong z, it's 30cm off!

    EXPECT_FALSE(isPointOnLineBetweenPoints(a, b, t));
    EXPECT_EQ(0.3, distanceFromPointToLine(a, b, t));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_findIndexOfVertex)
{
    DataVectorTypes::Vector a;
    std::vector<DataVectorTypes::Vector> list;

    a.x = 0.;
    a.y = 0.;
    a.z = 0.;

    EXPECT_EQ(-1, findIndexOfVertex(a, list)); // not found

    list.emplace_back(a);

    EXPECT_EQ(0, findIndexOfVertex(a, list));

    list.emplace_back(3., 2., 4.);
    list.emplace_back(4., 3., 5.);
    list.emplace_back(8., 1., 2.);
    list.emplace_back(4., 7., 3.);

    EXPECT_EQ(0, findIndexOfVertex(a, list));

    a.x = 4.;
    a.y = 3.;
    a.z = 5.;

    EXPECT_EQ(2, findIndexOfVertex(a, list));

    a.x = 4.01;
    a.y = 7.01;
    a.z = 3.01;

    EXPECT_EQ(4, findIndexOfVertex(a, list));

    a.x = 4.03;
    a.y = 7.03;
    a.z = 3.03;

    EXPECT_EQ(-1, findIndexOfVertex(a, list)); // not found
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_listOfFacesFacingAzimuth_test)
{
    DataVectorTypes::Polyhedron zonePoly;
    std::vector<int> results;

    state->dataSurface->Surface.allocate(9);
    state->dataSurface->Surface(1).Azimuth = 0;
    state->dataSurface->Surface(2).Azimuth = 30.;
    state->dataSurface->Surface(3).Azimuth = 30.;
    state->dataSurface->Surface(4).Azimuth = 30.;
    state->dataSurface->Surface(5).Azimuth = 45.;
    state->dataSurface->Surface(6).Azimuth = 45.;
    state->dataSurface->Surface(7).Azimuth = 72.;
    state->dataSurface->Surface(8).Azimuth = 72.5;
    state->dataSurface->Surface(9).Azimuth = 73.;

    zonePoly.NumSurfaceFaces = 9;
    zonePoly.SurfaceFace.allocate(9);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(5).SurfNum = 5;
    zonePoly.SurfaceFace(6).SurfNum = 6;
    zonePoly.SurfaceFace(7).SurfNum = 7;
    zonePoly.SurfaceFace(8).SurfNum = 8;
    zonePoly.SurfaceFace(9).SurfNum = 9;

    results = listOfFacesFacingAzimuth(*state, zonePoly, 90.);
    EXPECT_EQ(size_t(0), results.size());

    results = listOfFacesFacingAzimuth(*state, zonePoly, 0.);
    EXPECT_EQ(size_t(1), results.size());
    EXPECT_EQ(1, results.at(0));

    results = listOfFacesFacingAzimuth(*state, zonePoly, 30.);
    EXPECT_EQ(size_t(3), results.size());
    EXPECT_EQ(2, results.at(0));
    EXPECT_EQ(3, results.at(1));
    EXPECT_EQ(4, results.at(2));

    results = listOfFacesFacingAzimuth(*state, zonePoly, 45.);
    EXPECT_EQ(size_t(2), results.size());
    EXPECT_EQ(5, results.at(0));
    EXPECT_EQ(6, results.at(1));

    results = listOfFacesFacingAzimuth(*state, zonePoly, 71.9);
    EXPECT_EQ(size_t(2), results.size());
    EXPECT_EQ(7, results.at(0));
    EXPECT_EQ(8, results.at(1));

    results = listOfFacesFacingAzimuth(*state, zonePoly, 72.0);
    EXPECT_EQ(size_t(2), results.size());
    EXPECT_EQ(7, results.at(0));
    EXPECT_EQ(8, results.at(1));

    results = listOfFacesFacingAzimuth(*state, zonePoly, 72.1);
    EXPECT_EQ(size_t(3), results.size());
    EXPECT_EQ(7, results.at(0));
    EXPECT_EQ(8, results.at(1));
    EXPECT_EQ(9, results.at(2));

    results = listOfFacesFacingAzimuth(*state, zonePoly, 73.0);
    EXPECT_EQ(size_t(2), results.size());
    EXPECT_EQ(8, results.at(0));
    EXPECT_EQ(9, results.at(1));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_areSurfaceHorizAndVert_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    state->dataSurface->Surface.allocate(9);
    state->dataSurface->Surface(1).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(1).Tilt = 180.;

    state->dataSurface->Surface(2).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(2).Tilt = 179.5;

    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 89.1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(4).Tilt = 90.;

    state->dataSurface->Surface(5).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(5).Tilt = 90.;

    state->dataSurface->Surface(6).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(6).Tilt = 90.9;

    state->dataSurface->Surface(7).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(7).Tilt = -0.9;

    state->dataSurface->Surface(8).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(8).Tilt = 0.;

    state->dataSurface->Surface(9).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(9).Tilt = 0.9;

    zonePoly.NumSurfaceFaces = 9;
    zonePoly.SurfaceFace.allocate(9);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(5).SurfNum = 5;
    zonePoly.SurfaceFace(6).SurfNum = 6;
    zonePoly.SurfaceFace(7).SurfNum = 7;
    zonePoly.SurfaceFace(8).SurfNum = 8;
    zonePoly.SurfaceFace(9).SurfNum = 9;

    bool isFloorHorizontal;
    bool isCeilingHorizontal;
    bool areWallsVertical;

    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(1).Tilt = 170.;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_FALSE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(1).Tilt = 180.;
    state->dataSurface->Surface(2).Tilt = 178.9;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_FALSE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(2).Tilt = 181.0;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(2).Tilt = 181.1;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_FALSE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(2).Tilt = 179.5;
    state->dataSurface->Surface(8).Tilt = 180.;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_FALSE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(8).Tilt = 1.1;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_FALSE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(8).Tilt = -1.1;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_FALSE(isCeilingHorizontal);
    EXPECT_TRUE(areWallsVertical);

    state->dataSurface->Surface(8).Tilt = 0.;
    state->dataSurface->Surface(4).Tilt = 270.;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_FALSE(areWallsVertical);

    state->dataSurface->Surface(4).Tilt = 91.1;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_FALSE(areWallsVertical);

    state->dataSurface->Surface(4).Tilt = 88.9;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_TRUE(isFloorHorizontal);
    EXPECT_TRUE(isCeilingHorizontal);
    EXPECT_FALSE(areWallsVertical);

    state->dataSurface->Surface(1).Tilt = 170.;
    state->dataSurface->Surface(8).Tilt = 1.1;
    std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(*state, zonePoly);
    EXPECT_FALSE(isFloorHorizontal);
    EXPECT_FALSE(isCeilingHorizontal);
    EXPECT_FALSE(areWallsVertical);
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_areWallHeightSame_test)
{
    DataVectorTypes::Polyhedron zonePoly;
    std::vector<int> results;

    state->dataSurface->Surface.allocate(3);
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;

    zonePoly.NumSurfaceFaces = 3;
    zonePoly.SurfaceFace.allocate(3);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);
    zonePoly.SurfaceFace(1).FacePoints(1).z = 10.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 10.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);
    zonePoly.SurfaceFace(2).FacePoints(1).z = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 10.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 10.;

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);
    zonePoly.SurfaceFace(3).FacePoints(1).z = 0.;
    zonePoly.SurfaceFace(3).FacePoints(2).z = 10.;
    zonePoly.SurfaceFace(3).FacePoints(3).z = 10.;
    zonePoly.SurfaceFace(3).FacePoints(4).z = 0.;

    EXPECT_TRUE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(3).FacePoints(2).z = 9.;
    EXPECT_TRUE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(3).FacePoints(2).z = 11.;
    EXPECT_FALSE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(3).FacePoints(2).z = 10.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 10.02;
    EXPECT_TRUE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(2).FacePoints(2).z = 10.03;
    EXPECT_FALSE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(1).FacePoints(1).z = -10.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = -0.5;
    zonePoly.SurfaceFace(1).FacePoints(3).z = -0.5;
    zonePoly.SurfaceFace(1).FacePoints(4).z = -10.;

    zonePoly.SurfaceFace(2).FacePoints(1).z = -0.5;
    zonePoly.SurfaceFace(2).FacePoints(2).z = -10.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = -0.5;
    zonePoly.SurfaceFace(2).FacePoints(4).z = -10.;

    zonePoly.SurfaceFace(3).FacePoints(1).z = -0.5;
    zonePoly.SurfaceFace(3).FacePoints(2).z = -10.;
    zonePoly.SurfaceFace(3).FacePoints(3).z = -10.;
    zonePoly.SurfaceFace(3).FacePoints(4).z = -0.5;

    EXPECT_TRUE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(3).FacePoints(1).z = -0.6;
    EXPECT_TRUE(areWallHeightSame(*state, zonePoly));

    zonePoly.SurfaceFace(3).FacePoints(1).z = -0.4;
    EXPECT_FALSE(areWallHeightSame(*state, zonePoly));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_findPossibleOppositeFace_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    state->dataSurface->Surface.allocate(4);
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).Area = 10.;

    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).Area = 10.;

    state->dataSurface->Surface(3).Azimuth = 180.;
    state->dataSurface->Surface(3).Area = 10.;

    state->dataSurface->Surface(4).Azimuth = 270.;
    state->dataSurface->Surface(4).Area = 10.;

    zonePoly.NumSurfaceFaces = 4;
    zonePoly.SurfaceFace.allocate(4);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;

    EXPECT_EQ(3, findPossibleOppositeFace(*state, zonePoly, 1));
    EXPECT_EQ(1, findPossibleOppositeFace(*state, zonePoly, 3));

    EXPECT_EQ(4, findPossibleOppositeFace(*state, zonePoly, 2));
    EXPECT_EQ(2, findPossibleOppositeFace(*state, zonePoly, 4));

    state->dataSurface->Surface(2).Azimuth = 90.5;

    EXPECT_EQ(4, findPossibleOppositeFace(*state, zonePoly, 2));
    EXPECT_EQ(2, findPossibleOppositeFace(*state, zonePoly, 4));

    state->dataSurface->Surface(2).Azimuth = 89.5;

    EXPECT_EQ(4, findPossibleOppositeFace(*state, zonePoly, 2));
    EXPECT_EQ(2, findPossibleOppositeFace(*state, zonePoly, 4));

    state->dataSurface->Surface(2).Azimuth = 45.;

    EXPECT_EQ(-1, findPossibleOppositeFace(*state, zonePoly, 2)); // not found
    EXPECT_EQ(-1, findPossibleOppositeFace(*state, zonePoly, 4)); // not found

    state->dataSurface->Surface(1).Area = 9.;

    EXPECT_EQ(-1, findPossibleOppositeFace(*state, zonePoly, 1)); // not found
    EXPECT_EQ(-1, findPossibleOppositeFace(*state, zonePoly, 3)); // not found

    state->dataSurface->Surface(1).Area = 10.;

    EXPECT_EQ(3, findPossibleOppositeFace(*state, zonePoly, 1));
    EXPECT_EQ(1, findPossibleOppositeFace(*state, zonePoly, 3));

    zonePoly.SurfaceFace(1).NSides = 3;

    EXPECT_EQ(-1, findPossibleOppositeFace(*state, zonePoly, 1)); // not found
    EXPECT_EQ(-1, findPossibleOppositeFace(*state, zonePoly, 3)); // not found
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_areCornersEquidistant_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 2;
    zonePoly.SurfaceFace.allocate(2);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);

    zonePoly.SurfaceFace(1).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(1).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);

    zonePoly.SurfaceFace(2).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(2).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(2).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 3.;

    Real64 dist;
    EXPECT_TRUE(areCornersEquidistant(zonePoly, 1, 2, dist));
    EXPECT_EQ(8., dist);

    zonePoly.SurfaceFace(2).FacePoints(4).y = 7.;
    EXPECT_FALSE(areCornersEquidistant(zonePoly, 1, 2, dist));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_areOppositeWallsSame_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    state->dataSurface->Surface.allocate(4);
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Area = 30.;

    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Area = 24.;

    state->dataSurface->Surface(3).Azimuth = 180.;
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Area = 30.;

    state->dataSurface->Surface(4).Azimuth = 270.;
    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(4).Area = 24.;

    zonePoly.NumSurfaceFaces = 4;
    zonePoly.SurfaceFace.allocate(4);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);

    zonePoly.SurfaceFace(1).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(1).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);

    zonePoly.SurfaceFace(2).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);

    zonePoly.SurfaceFace(3).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(3).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);

    zonePoly.SurfaceFace(4).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(4).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(4).z = 3.;

    Real64 area;
    Real64 dist;

    EXPECT_TRUE(areOppositeWallsSame(*state, zonePoly, area, dist));
    EXPECT_EQ(30., area);
    EXPECT_EQ(8., dist);

    state->dataSurface->Surface(3).Area = 29.; // make surface 1 and 3 no longer match areas - now compare 2 and 4
    EXPECT_TRUE(areOppositeWallsSame(*state, zonePoly, area, dist));
    EXPECT_EQ(24., area);
    EXPECT_EQ(10., dist);

    state->dataSurface->Surface(4).Area = 23.; // make surface 2 and 4 no longer match areas
    EXPECT_FALSE(areOppositeWallsSame(*state, zonePoly, area, dist));

    state->dataSurface->Surface(3).Area = 30.; // make surface 1 and 3 have same areas again
    state->dataSurface->Surface(4).Area = 24.; // make surface 2 and 4 have same areas again

    EXPECT_TRUE(areOppositeWallsSame(*state, zonePoly, area, dist)); // retest

    zonePoly.SurfaceFace(3).FacePoints(3).y = 7.;                    // move one corner in so distances are not all equal
    EXPECT_TRUE(areOppositeWallsSame(*state, zonePoly, area, dist)); // should pick other walls
    EXPECT_EQ(24., area);
    EXPECT_EQ(10., dist);

    zonePoly.SurfaceFace(4).FacePoints(3).x = 11.;                    // move one corner out so distances are not all equal
    EXPECT_FALSE(areOppositeWallsSame(*state, zonePoly, area, dist)); // now neither wall matches
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_areFloorAndCeilingSame_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    state->dataSurface->Surface.allocate(2);
    state->dataSurface->Surface(1).Class = SurfaceClass::Floor;

    state->dataSurface->Surface(2).Class = SurfaceClass::Roof;

    zonePoly.NumSurfaceFaces = 2;
    zonePoly.SurfaceFace.allocate(2);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);

    zonePoly.SurfaceFace(1).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 0.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);

    zonePoly.SurfaceFace(2).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(2).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(2).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 3.;

    EXPECT_TRUE(areFloorAndCeilingSame(*state, zonePoly));

    zonePoly.SurfaceFace(2).FacePoints(4).x = 7.; // move one corner

    EXPECT_FALSE(areFloorAndCeilingSame(*state, zonePoly));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_makeListOfUniqueVertices_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 6;
    zonePoly.SurfaceFace.allocate(6);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);

    zonePoly.SurfaceFace(1).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(1).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);

    zonePoly.SurfaceFace(2).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);

    zonePoly.SurfaceFace(3).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(3).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);

    zonePoly.SurfaceFace(4).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(4).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(5).SurfNum = 1;
    zonePoly.SurfaceFace(5).NSides = 4;
    zonePoly.SurfaceFace(5).FacePoints.allocate(4);

    zonePoly.SurfaceFace(5).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(5).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(5).FacePoints(1).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(5).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(5).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(5).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(5).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(5).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(5).FacePoints(4).z = 0.;

    zonePoly.SurfaceFace(6).SurfNum = 2;
    zonePoly.SurfaceFace(6).NSides = 4;
    zonePoly.SurfaceFace(6).FacePoints.allocate(4);

    zonePoly.SurfaceFace(6).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(6).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(2).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(6).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(3).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(6).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(6).FacePoints(4).z = 3.;

    std::vector<Vector> uniqueVertices = makeListOfUniqueVertices(zonePoly);

    EXPECT_EQ(size_t(8), uniqueVertices.size());
    EXPECT_EQ(Vector(0., 0., 3.), uniqueVertices.at(0));
    EXPECT_EQ(Vector(0., 0., 0.), uniqueVertices.at(1));
    EXPECT_EQ(Vector(10., 0., 0.), uniqueVertices.at(2));
    EXPECT_EQ(Vector(10., 0., 3.), uniqueVertices.at(3));
    EXPECT_EQ(Vector(0., 8., 3.), uniqueVertices.at(4));
    EXPECT_EQ(Vector(0., 8., 0.), uniqueVertices.at(5));
    EXPECT_EQ(Vector(10., 8., 3.), uniqueVertices.at(6));
    EXPECT_EQ(Vector(10., 8., 0.), uniqueVertices.at(7));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_numberOfEdgesNotTwoForEnclosedVolumeTest_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 6;
    zonePoly.SurfaceFace.allocate(6);
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);

    zonePoly.SurfaceFace(1).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(1).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(1).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);

    zonePoly.SurfaceFace(2).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);

    zonePoly.SurfaceFace(3).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(3).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);

    zonePoly.SurfaceFace(4).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(4).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(5).SurfNum = 1;
    zonePoly.SurfaceFace(5).NSides = 4;
    zonePoly.SurfaceFace(5).FacePoints.allocate(4);

    zonePoly.SurfaceFace(5).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(5).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(5).FacePoints(1).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(5).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(5).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(5).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(5).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(5).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(5).FacePoints(4).z = 0.;

    zonePoly.SurfaceFace(6).SurfNum = 2;
    zonePoly.SurfaceFace(6).NSides = 4;
    zonePoly.SurfaceFace(6).FacePoints.allocate(4);

    zonePoly.SurfaceFace(6).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(6).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(2).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(6).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(3).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(6).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(6).FacePoints(4).z = 3.;

    std::vector<Vector> uniqueVertices = makeListOfUniqueVertices(zonePoly);

    EXPECT_EQ(size_t(8), uniqueVertices.size());

    std::vector<EdgeOfSurf> e1 = edgesNotTwoForEnclosedVolumeTest(zonePoly, uniqueVertices);
    EXPECT_EQ(size_t(0), e1.size());

    zonePoly.SurfaceFace(6).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(4).z = 0.;

    uniqueVertices = makeListOfUniqueVertices(zonePoly);
    EXPECT_EQ(size_t(8), uniqueVertices.size());

    std::vector<EdgeOfSurf> e2 = edgesNotTwoForEnclosedVolumeTest(zonePoly, uniqueVertices);
    EXPECT_EQ(size_t(4), e2.size());
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_updateZonePolygonsForMissingColinearPoints_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 7;
    zonePoly.SurfaceFace.allocate(7);

    // split old surface 1 into two new surfaces 1 and 7
    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);

    zonePoly.SurfaceFace(1).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(1).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(3).x = 4.;
    zonePoly.SurfaceFace(1).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(1).FacePoints(4).x = 4.;
    zonePoly.SurfaceFace(1).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(1).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(7).SurfNum = 7;
    zonePoly.SurfaceFace(7).NSides = 4;
    zonePoly.SurfaceFace(7).FacePoints.allocate(4);

    zonePoly.SurfaceFace(7).FacePoints(1).x = 4.;
    zonePoly.SurfaceFace(7).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(7).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(7).FacePoints(2).x = 4.;
    zonePoly.SurfaceFace(7).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(7).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(7).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(7).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(7).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(7).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(7).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(7).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);

    zonePoly.SurfaceFace(2).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(2).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(2).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(2).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(2).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);

    zonePoly.SurfaceFace(3).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(3).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(3).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(3).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(3).FacePoints(4).x = 0.;
    zonePoly.SurfaceFace(3).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(3).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);

    zonePoly.SurfaceFace(4).FacePoints(1).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(4).FacePoints(2).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(4).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(4).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(4).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(4).FacePoints(4).z = 3.;

    zonePoly.SurfaceFace(5).SurfNum = 5;
    zonePoly.SurfaceFace(5).NSides = 4;
    zonePoly.SurfaceFace(5).FacePoints.allocate(4);

    zonePoly.SurfaceFace(5).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(5).FacePoints(1).y = 0.;
    zonePoly.SurfaceFace(5).FacePoints(1).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(5).FacePoints(2).y = 8.;
    zonePoly.SurfaceFace(5).FacePoints(2).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(5).FacePoints(3).y = 8.;
    zonePoly.SurfaceFace(5).FacePoints(3).z = 0.;

    zonePoly.SurfaceFace(5).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(5).FacePoints(4).y = 0.;
    zonePoly.SurfaceFace(5).FacePoints(4).z = 0.;

    zonePoly.SurfaceFace(6).SurfNum = 6;
    zonePoly.SurfaceFace(6).NSides = 4;
    zonePoly.SurfaceFace(6).FacePoints.allocate(4);

    zonePoly.SurfaceFace(6).FacePoints(1).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(1).y = 8.;
    zonePoly.SurfaceFace(6).FacePoints(1).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(2).x = 0.;
    zonePoly.SurfaceFace(6).FacePoints(2).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(2).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(3).x = 10.;
    zonePoly.SurfaceFace(6).FacePoints(3).y = 0.;
    zonePoly.SurfaceFace(6).FacePoints(3).z = 3.;

    zonePoly.SurfaceFace(6).FacePoints(4).x = 10.;
    zonePoly.SurfaceFace(6).FacePoints(4).y = 8.;
    zonePoly.SurfaceFace(6).FacePoints(4).z = 3.;

    std::vector<Vector> uniqueVertices = makeListOfUniqueVertices(zonePoly);

    EXPECT_EQ(size_t(10), uniqueVertices.size());

    std::vector<EdgeOfSurf> e1 = edgesNotTwoForEnclosedVolumeTest(zonePoly, uniqueVertices);
    EXPECT_EQ(size_t(6), e1.size());

    DataVectorTypes::Polyhedron updatedZonePoly = updateZonePolygonsForMissingColinearPoints(
        zonePoly, uniqueVertices); // this is done after initial test since it is computationally intensive.

    std::vector<EdgeOfSurf> e2 = edgesNotTwoForEnclosedVolumeTest(updatedZonePoly, uniqueVertices);
    EXPECT_EQ(size_t(0), e2.size());
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isEnclosedVolume_SimpleBox_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 6;
    zonePoly.SurfaceFace.allocate(6);

    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);
    zonePoly.SurfaceFace(1).FacePoints(1) = Vector(0., 0., 3.);
    zonePoly.SurfaceFace(1).FacePoints(2) = Vector(0., 0., 0.);
    zonePoly.SurfaceFace(1).FacePoints(3) = Vector(10., 0., 0.);
    zonePoly.SurfaceFace(1).FacePoints(4) = Vector(10., 0., 3.);

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);
    zonePoly.SurfaceFace(2).FacePoints(1) = Vector(0., 8., 3.);
    zonePoly.SurfaceFace(2).FacePoints(2) = Vector(0., 8., 0.);
    zonePoly.SurfaceFace(2).FacePoints(3) = Vector(0., 0., 0.);
    zonePoly.SurfaceFace(2).FacePoints(4) = Vector(0., 0., 3.);

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);
    zonePoly.SurfaceFace(3).FacePoints(1) = Vector(10., 8., 3.);
    zonePoly.SurfaceFace(3).FacePoints(2) = Vector(10., 8., 0.);
    zonePoly.SurfaceFace(3).FacePoints(3) = Vector(0., 8., 0.);
    zonePoly.SurfaceFace(3).FacePoints(4) = Vector(0., 8., 3.);

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);
    zonePoly.SurfaceFace(4).FacePoints(1) = Vector(10., 0., 3.);
    zonePoly.SurfaceFace(4).FacePoints(2) = Vector(10., 0., 0.);
    zonePoly.SurfaceFace(4).FacePoints(3) = Vector(10., 8., 0.);
    zonePoly.SurfaceFace(4).FacePoints(4) = Vector(10., 8., 3.);

    zonePoly.SurfaceFace(5).SurfNum = 1;
    zonePoly.SurfaceFace(5).NSides = 4;
    zonePoly.SurfaceFace(5).FacePoints.allocate(4);
    zonePoly.SurfaceFace(5).FacePoints(1) = Vector(0., 0., 0.);
    zonePoly.SurfaceFace(5).FacePoints(2) = Vector(0., 8, 0.);
    zonePoly.SurfaceFace(5).FacePoints(3) = Vector(10., 8, 0.);
    zonePoly.SurfaceFace(5).FacePoints(4) = Vector(10., 0, 0.);

    zonePoly.SurfaceFace(6).SurfNum = 2;
    zonePoly.SurfaceFace(6).NSides = 4;
    zonePoly.SurfaceFace(6).FacePoints.allocate(4);
    zonePoly.SurfaceFace(6).FacePoints(1) = Vector(0., 8., 3.);
    zonePoly.SurfaceFace(6).FacePoints(2) = Vector(0., 0., 3.);
    zonePoly.SurfaceFace(6).FacePoints(3) = Vector(10., 0., 3.);
    zonePoly.SurfaceFace(6).FacePoints(4) = Vector(10., 8., 3.);

    std::vector<EdgeOfSurf> edgeNot2;
    EXPECT_TRUE(isEnclosedVolume(zonePoly, edgeNot2));

    // leave gap
    zonePoly.SurfaceFace(1).FacePoints(3) = Vector(9., 0., 0.);
    EXPECT_FALSE(isEnclosedVolume(zonePoly, edgeNot2));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isEnclosedVolume_BoxWithSplitSide_test)
{
    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 7;
    zonePoly.SurfaceFace.allocate(7);

    // split old surface 1 into two new surfaces 1 and 7

    zonePoly.SurfaceFace(1).SurfNum = 1;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);
    zonePoly.SurfaceFace(1).FacePoints(1) = Vector(0., 0., 3.);
    zonePoly.SurfaceFace(1).FacePoints(2) = Vector(0., 0., 0.);
    zonePoly.SurfaceFace(1).FacePoints(3) = Vector(4., 0., 0.);
    zonePoly.SurfaceFace(1).FacePoints(4) = Vector(4., 0., 3.);

    zonePoly.SurfaceFace(7).SurfNum = 7;
    zonePoly.SurfaceFace(7).NSides = 4;
    zonePoly.SurfaceFace(7).FacePoints.allocate(4);
    zonePoly.SurfaceFace(7).FacePoints(1) = Vector(4., 0., 3.);
    zonePoly.SurfaceFace(7).FacePoints(2) = Vector(4., 0., 0.);
    zonePoly.SurfaceFace(7).FacePoints(3) = Vector(10., 0., 0.);
    zonePoly.SurfaceFace(7).FacePoints(4) = Vector(10., 0., 3.);

    zonePoly.SurfaceFace(2).SurfNum = 2;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);
    zonePoly.SurfaceFace(2).FacePoints(1) = Vector(0., 8., 3.);
    zonePoly.SurfaceFace(2).FacePoints(2) = Vector(0., 8., 0.);
    zonePoly.SurfaceFace(2).FacePoints(3) = Vector(0., 0., 0.);
    zonePoly.SurfaceFace(2).FacePoints(4) = Vector(0., 0., 3.);

    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);
    zonePoly.SurfaceFace(3).FacePoints(1) = Vector(10., 8., 3.);
    zonePoly.SurfaceFace(3).FacePoints(2) = Vector(10., 8., 0.);
    zonePoly.SurfaceFace(3).FacePoints(3) = Vector(0., 8., 0.);
    zonePoly.SurfaceFace(3).FacePoints(4) = Vector(0., 8., 3.);

    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);
    zonePoly.SurfaceFace(4).FacePoints(1) = Vector(10., 0., 3.);
    zonePoly.SurfaceFace(4).FacePoints(2) = Vector(10., 0., 0.);
    zonePoly.SurfaceFace(4).FacePoints(3) = Vector(10., 8., 0.);
    zonePoly.SurfaceFace(4).FacePoints(4) = Vector(10., 8., 3.);

    zonePoly.SurfaceFace(5).SurfNum = 1;
    zonePoly.SurfaceFace(5).NSides = 4;
    zonePoly.SurfaceFace(5).FacePoints.allocate(4);
    zonePoly.SurfaceFace(5).FacePoints(1) = Vector(0., 0., 0.);
    zonePoly.SurfaceFace(5).FacePoints(2) = Vector(0., 8, 0.);
    zonePoly.SurfaceFace(5).FacePoints(3) = Vector(10., 8, 0.);
    zonePoly.SurfaceFace(5).FacePoints(4) = Vector(10., 0, 0.);

    zonePoly.SurfaceFace(6).SurfNum = 2;
    zonePoly.SurfaceFace(6).NSides = 4;
    zonePoly.SurfaceFace(6).FacePoints.allocate(4);
    zonePoly.SurfaceFace(6).FacePoints(1) = Vector(0., 8., 3.);
    zonePoly.SurfaceFace(6).FacePoints(2) = Vector(0., 0., 3.);
    zonePoly.SurfaceFace(6).FacePoints(3) = Vector(10., 0., 3.);
    zonePoly.SurfaceFace(6).FacePoints(4) = Vector(10., 8., 3.);

    std::vector<EdgeOfSurf> edgeNot2;
    EXPECT_TRUE(isEnclosedVolume(zonePoly, edgeNot2));

    // leave gap
    zonePoly.SurfaceFace(1).FacePoints(3) = Vector(9., 0., 0.);
    EXPECT_FALSE(isEnclosedVolume(zonePoly, edgeNot2));
}

TEST_F(EnergyPlusFixture, SurfaceGeometryUnitTests_isEnclosedVolume_VeryFlatShape)
{
    // Test for #7383
    // This is a shoebox model that has a very flat aspect: a 30x10x0.3m one
    // The south wall is split in two equal segments, the rest is just one surface per orientation

    DataVectorTypes::Polyhedron zonePoly;

    zonePoly.NumSurfaceFaces = 7;
    zonePoly.SurfaceFace.allocate(7);

    // 1-SOUTH-1
    zonePoly.SurfaceFace(1).SurfNum = 2;
    zonePoly.SurfaceFace(1).NSides = 4;
    zonePoly.SurfaceFace(1).FacePoints.allocate(4);
    zonePoly.SurfaceFace(1).FacePoints(1) = Vector(0.0, 0.0, 0.3);
    zonePoly.SurfaceFace(1).FacePoints(2) = Vector(0.0, 0.0, 0.0);
    zonePoly.SurfaceFace(1).FacePoints(3) = Vector(15.0, 0.0, 0.0);
    zonePoly.SurfaceFace(1).FacePoints(4) = Vector(15.0, 0.0, 0.3);

    // 1-SOUTH-2
    zonePoly.SurfaceFace(2).SurfNum = 1;
    zonePoly.SurfaceFace(2).NSides = 4;
    zonePoly.SurfaceFace(2).FacePoints.allocate(4);
    zonePoly.SurfaceFace(2).FacePoints(1) = Vector(15.0, 0.0, 0.3);
    zonePoly.SurfaceFace(2).FacePoints(2) = Vector(15.0, 0.0, 0.0);
    zonePoly.SurfaceFace(2).FacePoints(3) = Vector(30.0, 0.0, 0.0);
    zonePoly.SurfaceFace(2).FacePoints(4) = Vector(30.0, 0.0, 0.3);

    // 4-NORTH
    zonePoly.SurfaceFace(3).SurfNum = 3;
    zonePoly.SurfaceFace(3).NSides = 4;
    zonePoly.SurfaceFace(3).FacePoints.allocate(4);
    zonePoly.SurfaceFace(3).FacePoints(1) = Vector(30.0, 10.0, 0.3);
    zonePoly.SurfaceFace(3).FacePoints(2) = Vector(30.0, 10.0, 0.0);
    zonePoly.SurfaceFace(3).FacePoints(3) = Vector(0.0, 10.0, 0.0);
    zonePoly.SurfaceFace(3).FacePoints(4) = Vector(0.0, 10.0, 0.3);

    // 3-EAST
    zonePoly.SurfaceFace(4).SurfNum = 4;
    zonePoly.SurfaceFace(4).NSides = 4;
    zonePoly.SurfaceFace(4).FacePoints.allocate(4);
    zonePoly.SurfaceFace(4).FacePoints(1) = Vector(30.0, 0.0, 0.3);
    zonePoly.SurfaceFace(4).FacePoints(2) = Vector(30.0, 0.0, 0.0);
    zonePoly.SurfaceFace(4).FacePoints(3) = Vector(30.0, 10.0, 0.0);
    zonePoly.SurfaceFace(4).FacePoints(4) = Vector(30.0, 10.0, 0.3);

    // ROOF
    zonePoly.SurfaceFace(5).SurfNum = 5;
    zonePoly.SurfaceFace(5).NSides = 4;
    zonePoly.SurfaceFace(5).FacePoints.allocate(4);
    zonePoly.SurfaceFace(5).FacePoints(1) = Vector(30.0, 0.0, 0.3);
    zonePoly.SurfaceFace(5).FacePoints(2) = Vector(30.0, 10.0, 0.3);
    zonePoly.SurfaceFace(5).FacePoints(3) = Vector(0.0, 10.0, 0.3);
    zonePoly.SurfaceFace(5).FacePoints(4) = Vector(0.0, 0.0, 0.3);

    // 2-WEST
    zonePoly.SurfaceFace(6).SurfNum = 6;
    zonePoly.SurfaceFace(6).NSides = 4;
    zonePoly.SurfaceFace(6).FacePoints.allocate(4);
    zonePoly.SurfaceFace(6).FacePoints(1) = Vector(0.0, 10.0, 0.3);
    zonePoly.SurfaceFace(6).FacePoints(2) = Vector(0.0, 10.0, 0.0);
    zonePoly.SurfaceFace(6).FacePoints(3) = Vector(0.0, 0.0, 0.0);
    zonePoly.SurfaceFace(6).FacePoints(4) = Vector(0.0, 0.0, 0.3);

    // FLOOR
    zonePoly.SurfaceFace(7).SurfNum = 7;
    zonePoly.SurfaceFace(7).NSides = 4;
    zonePoly.SurfaceFace(7).FacePoints.allocate(4);
    zonePoly.SurfaceFace(7).FacePoints(1) = Vector(0.0, 0.0, 0.0);
    zonePoly.SurfaceFace(7).FacePoints(2) = Vector(0.0, 10.0, 0.0);
    zonePoly.SurfaceFace(7).FacePoints(3) = Vector(30.0, 10.0, 0.0);
    zonePoly.SurfaceFace(7).FacePoints(4) = Vector(30.0, 0.0, 0.0);

    std::vector<EdgeOfSurf> edgeNot2;
    EXPECT_TRUE(isEnclosedVolume(zonePoly, edgeNot2));
}

TEST_F(EnergyPlusFixture, CalculateZoneVolume_SimpleBox_test)
{
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).HasFloor = true;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceLast = 6;

    state->dataSurface->Surface.allocate(6);

    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Vertex.dimension(4);
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Vertex(1) = Vector(0., 0., 3.);
    state->dataSurface->Surface(1).Vertex(2) = Vector(0., 0., 0.);
    state->dataSurface->Surface(1).Vertex(3) = Vector(10., 0., 0.);
    state->dataSurface->Surface(1).Vertex(4) = Vector(10., 0., 3.);

    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Vertex.dimension(4);
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(2).Vertex(2) = Vector(0., 8., 0.);
    state->dataSurface->Surface(2).Vertex(3) = Vector(0., 0., 0.);
    state->dataSurface->Surface(2).Vertex(4) = Vector(0., 0., 3.);

    state->dataSurface->Surface(3).Sides = 4;
    state->dataSurface->Surface(3).Vertex.dimension(4);
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Vertex(1) = Vector(10., 8., 3.);
    state->dataSurface->Surface(3).Vertex(2) = Vector(10., 8., 0.);
    state->dataSurface->Surface(3).Vertex(3) = Vector(0., 8., 0.);
    state->dataSurface->Surface(3).Vertex(4) = Vector(0., 8., 3.);

    state->dataSurface->Surface(4).Sides = 4;
    state->dataSurface->Surface(4).Vertex.dimension(4);
    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Vertex(1) = Vector(10., 0., 3.);
    state->dataSurface->Surface(4).Vertex(2) = Vector(10., 0., 0.);
    state->dataSurface->Surface(4).Vertex(3) = Vector(10., 8., 0.);
    state->dataSurface->Surface(4).Vertex(4) = Vector(10., 8., 3.);

    state->dataSurface->Surface(5).Sides = 4;
    state->dataSurface->Surface(5).Vertex.dimension(4);
    state->dataSurface->Surface(5).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(5).Tilt = 180.;
    state->dataSurface->Surface(5).Vertex(1) = Vector(0., 0., 0.);
    state->dataSurface->Surface(5).Vertex(2) = Vector(0., 8, 0.);
    state->dataSurface->Surface(5).Vertex(3) = Vector(10., 8, 0.);
    state->dataSurface->Surface(5).Vertex(4) = Vector(10., 0, 0.);

    state->dataSurface->Surface(6).Sides = 4;
    state->dataSurface->Surface(6).Vertex.dimension(4);
    state->dataSurface->Surface(6).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(6).Tilt = 0.;
    state->dataSurface->Surface(6).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(6).Vertex(2) = Vector(0., 0., 3.);
    state->dataSurface->Surface(6).Vertex(3) = Vector(10., 0., 3.);
    state->dataSurface->Surface(6).Vertex(4) = Vector(10., 8., 3.);

    CalculateZoneVolume(*state);
    EXPECT_EQ(240., state->dataHeatBal->Zone(1).Volume);
}

TEST_F(EnergyPlusFixture, CalculateZoneVolume_BoxOneWallMissing_test)
{
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).HasFloor = true;
    //    Zone(1).HTSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceLast = 5;

    state->dataSurface->Surface.allocate(5);

    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Vertex.dimension(4);
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Vertex(1) = Vector(0., 0., 3.);
    state->dataSurface->Surface(1).Vertex(2) = Vector(0., 0., 0.);
    state->dataSurface->Surface(1).Vertex(3) = Vector(10., 0., 0.);
    state->dataSurface->Surface(1).Vertex(4) = Vector(10., 0., 3.);

    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Vertex.dimension(4);
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(2).Vertex(2) = Vector(0., 8., 0.);
    state->dataSurface->Surface(2).Vertex(3) = Vector(0., 0., 0.);
    state->dataSurface->Surface(2).Vertex(4) = Vector(0., 0., 3.);

    state->dataSurface->Surface(3).Sides = 4;
    state->dataSurface->Surface(3).Vertex.dimension(4);
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Vertex(1) = Vector(10., 8., 3.);
    state->dataSurface->Surface(3).Vertex(2) = Vector(10., 8., 0.);
    state->dataSurface->Surface(3).Vertex(3) = Vector(0., 8., 0.);
    state->dataSurface->Surface(3).Vertex(4) = Vector(0., 8., 3.);

    state->dataSurface->Surface(4).Sides = 4;
    state->dataSurface->Surface(4).Vertex.dimension(4);
    state->dataSurface->Surface(4).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(4).Tilt = 180.;
    state->dataSurface->Surface(4).Vertex(1) = Vector(0., 0., 0.);
    state->dataSurface->Surface(4).Vertex(2) = Vector(0., 8, 0.);
    state->dataSurface->Surface(4).Vertex(3) = Vector(10., 8, 0.);
    state->dataSurface->Surface(4).Vertex(4) = Vector(10., 0, 0.);

    state->dataSurface->Surface(5).Sides = 4;
    state->dataSurface->Surface(5).Vertex.dimension(4);
    state->dataSurface->Surface(5).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(5).Tilt = 0.;
    state->dataSurface->Surface(5).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(5).Vertex(2) = Vector(0., 0., 3.);
    state->dataSurface->Surface(5).Vertex(3) = Vector(10., 0., 3.);
    state->dataSurface->Surface(5).Vertex(4) = Vector(10., 8., 3.);

    state->dataHeatBal->Zone(1).FloorArea = 80.;
    state->dataHeatBal->Zone(1).CeilingHeight = 3.;

    CalculateZoneVolume(*state);
    EXPECT_EQ(240., state->dataHeatBal->Zone(1).Volume);
}

TEST_F(EnergyPlusFixture, CalculateZoneVolume_BoxNoCeiling_test)
{
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).HasFloor = true;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceLast = 5;

    state->dataSurface->Surface.allocate(5);

    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Vertex.dimension(4);
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Vertex(1) = Vector(0., 0., 3.);
    state->dataSurface->Surface(1).Vertex(2) = Vector(0., 0., 0.);
    state->dataSurface->Surface(1).Vertex(3) = Vector(10., 0., 0.);
    state->dataSurface->Surface(1).Vertex(4) = Vector(10., 0., 3.);

    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Vertex.dimension(4);
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(2).Vertex(2) = Vector(0., 8., 0.);
    state->dataSurface->Surface(2).Vertex(3) = Vector(0., 0., 0.);
    state->dataSurface->Surface(2).Vertex(4) = Vector(0., 0., 3.);

    state->dataSurface->Surface(3).Sides = 4;
    state->dataSurface->Surface(3).Vertex.dimension(4);
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Vertex(1) = Vector(10., 8., 3.);
    state->dataSurface->Surface(3).Vertex(2) = Vector(10., 8., 0.);
    state->dataSurface->Surface(3).Vertex(3) = Vector(0., 8., 0.);
    state->dataSurface->Surface(3).Vertex(4) = Vector(0., 8., 3.);

    state->dataSurface->Surface(4).Sides = 4;
    state->dataSurface->Surface(4).Vertex.dimension(4);
    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Vertex(1) = Vector(10., 0., 3.);
    state->dataSurface->Surface(4).Vertex(2) = Vector(10., 0., 0.);
    state->dataSurface->Surface(4).Vertex(3) = Vector(10., 8., 0.);
    state->dataSurface->Surface(4).Vertex(4) = Vector(10., 8., 3.);

    state->dataSurface->Surface(5).Sides = 4;
    state->dataSurface->Surface(5).Vertex.dimension(4);
    state->dataSurface->Surface(5).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(5).Tilt = 180.;
    state->dataSurface->Surface(5).Vertex(1) = Vector(0., 0., 0.);
    state->dataSurface->Surface(5).Vertex(2) = Vector(0., 8, 0.);
    state->dataSurface->Surface(5).Vertex(3) = Vector(10., 8, 0.);
    state->dataSurface->Surface(5).Vertex(4) = Vector(10., 0, 0.);

    state->dataHeatBal->Zone(1).FloorArea = 80.;
    state->dataHeatBal->Zone(1).CeilingHeight = 3.;

    CalculateZoneVolume(*state);
    EXPECT_EQ(240., state->dataHeatBal->Zone(1).Volume);
}

TEST_F(EnergyPlusFixture, CalculateZoneVolume_BoxNoFloor_test)
{
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).HasFloor = true;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceLast = 5;

    state->dataSurface->Surface.allocate(5);

    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Vertex.dimension(4);
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Vertex(1) = Vector(0., 0., 3.);
    state->dataSurface->Surface(1).Vertex(2) = Vector(0., 0., 0.);
    state->dataSurface->Surface(1).Vertex(3) = Vector(10., 0., 0.);
    state->dataSurface->Surface(1).Vertex(4) = Vector(10., 0., 3.);

    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Vertex.dimension(4);
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(2).Vertex(2) = Vector(0., 8., 0.);
    state->dataSurface->Surface(2).Vertex(3) = Vector(0., 0., 0.);
    state->dataSurface->Surface(2).Vertex(4) = Vector(0., 0., 3.);

    state->dataSurface->Surface(3).Sides = 4;
    state->dataSurface->Surface(3).Vertex.dimension(4);
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Vertex(1) = Vector(10., 8., 3.);
    state->dataSurface->Surface(3).Vertex(2) = Vector(10., 8., 0.);
    state->dataSurface->Surface(3).Vertex(3) = Vector(0., 8., 0.);
    state->dataSurface->Surface(3).Vertex(4) = Vector(0., 8., 3.);

    state->dataSurface->Surface(4).Sides = 4;
    state->dataSurface->Surface(4).Vertex.dimension(4);
    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Vertex(1) = Vector(10., 0., 3.);
    state->dataSurface->Surface(4).Vertex(2) = Vector(10., 0., 0.);
    state->dataSurface->Surface(4).Vertex(3) = Vector(10., 8., 0.);
    state->dataSurface->Surface(4).Vertex(4) = Vector(10., 8., 3.);

    state->dataSurface->Surface(5).Sides = 4;
    state->dataSurface->Surface(5).Vertex.dimension(4);
    state->dataSurface->Surface(5).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(5).Tilt = 0.;
    state->dataSurface->Surface(5).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(5).Vertex(2) = Vector(0., 0., 3.);
    state->dataSurface->Surface(5).Vertex(3) = Vector(10., 0., 3.);
    state->dataSurface->Surface(5).Vertex(4) = Vector(10., 8., 3.);

    state->dataHeatBal->Zone(1).CeilingArea = 80.;
    state->dataHeatBal->Zone(1).CeilingHeight = 3.;

    CalculateZoneVolume(*state);
    EXPECT_EQ(240., state->dataHeatBal->Zone(1).Volume);
}

TEST_F(EnergyPlusFixture, CalculateZoneVolume_BoxNoCeilingFloor_test)
{
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceFirst = 1;
    state->dataHeatBal->Zone(1).AllSurfaceLast = 4;

    state->dataSurface->Surface.allocate(4);

    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Vertex.dimension(4);
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Azimuth = 180.;
    state->dataSurface->Surface(1).Area = 30.;
    state->dataSurface->Surface(1).Vertex(1) = Vector(0., 0., 3.);
    state->dataSurface->Surface(1).Vertex(2) = Vector(0., 0., 0.);
    state->dataSurface->Surface(1).Vertex(3) = Vector(10., 0., 0.);
    state->dataSurface->Surface(1).Vertex(4) = Vector(10., 0., 3.);

    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Vertex.dimension(4);
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Azimuth = 270.;
    state->dataSurface->Surface(2).Area = 24.;
    state->dataSurface->Surface(2).Vertex(1) = Vector(0., 8., 3.);
    state->dataSurface->Surface(2).Vertex(2) = Vector(0., 8., 0.);
    state->dataSurface->Surface(2).Vertex(3) = Vector(0., 0., 0.);
    state->dataSurface->Surface(2).Vertex(4) = Vector(0., 0., 3.);

    state->dataSurface->Surface(3).Sides = 4;
    state->dataSurface->Surface(3).Vertex.dimension(4);
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).Area = 30.;
    state->dataSurface->Surface(3).Vertex(1) = Vector(10., 8., 3.);
    state->dataSurface->Surface(3).Vertex(2) = Vector(10., 8., 0.);
    state->dataSurface->Surface(3).Vertex(3) = Vector(0., 8., 0.);
    state->dataSurface->Surface(3).Vertex(4) = Vector(0., 8., 3.);

    state->dataSurface->Surface(4).Sides = 4;
    state->dataSurface->Surface(4).Vertex.dimension(4);
    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).Area = 24.;
    state->dataSurface->Surface(4).Vertex(1) = Vector(10., 0., 3.);
    state->dataSurface->Surface(4).Vertex(2) = Vector(10., 0., 0.);
    state->dataSurface->Surface(4).Vertex(3) = Vector(10., 8., 0.);
    state->dataSurface->Surface(4).Vertex(4) = Vector(10., 8., 3.);

    CalculateZoneVolume(*state);
    EXPECT_EQ(240., state->dataHeatBal->Zone(1).Volume);
}

TEST_F(EnergyPlusFixture, MakeRectangularVertices)
{
    int surfNum = 1;
    int zoneNum = 1;

    state->dataSurfaceGeometry->SurfaceTmp.allocate(surfNum);
    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Class = SurfaceClass::Wall;
    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Zone = zoneNum;
    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Azimuth = 0.;
    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Tilt = 90.;
    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Sides = 4;
    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex.allocate(4);

    state->dataHeatBal->Zone.allocate(zoneNum);
    state->dataHeatBal->Zone(zoneNum).RelNorth = 0.;

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(zoneNum);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(zoneNum);
    state->dataSurfaceGeometry->CosZoneRelNorth(zoneNum) = std::cos(-state->dataHeatBal->Zone(zoneNum).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(zoneNum) = std::sin(-state->dataHeatBal->Zone(zoneNum).RelNorth * Constant::DegToRad);

    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    // facing north

    MakeRectangularVertices(*state, 1, 0., 0., 0., 5., 3., false);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).z, 0.001);

    EXPECT_NEAR(-5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).z, 0.001);

    EXPECT_NEAR(-5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).z, 0.001);

    // facing east

    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Azimuth = 90.;

    MakeRectangularVertices(*state, 1, 0., 0., 0., 5., 3., false);

    EXPECT_NEAR(0, state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).x, 0.001);
    EXPECT_NEAR(5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).x, 0.001);
    EXPECT_NEAR(5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).z, 0.001);

    // facing south

    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Azimuth = 180.;

    MakeRectangularVertices(*state, 1, 0., 0., 0., 5., 3., false);

    EXPECT_NEAR(0, state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).z, 0.001);

    EXPECT_NEAR(5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).z, 0.001);

    EXPECT_NEAR(5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).z, 0.001);

    // facing west

    state->dataSurfaceGeometry->SurfaceTmp(surfNum).Azimuth = 270.;

    MakeRectangularVertices(*state, 1, 0., 0., 0., 5., 3., false);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(1).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).x, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(2).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).x, 0.001);
    EXPECT_NEAR(-5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).y, 0.001);
    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(3).z, 0.001);

    EXPECT_NEAR(0., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).x, 0.001);
    EXPECT_NEAR(-5., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).y, 0.001);
    EXPECT_NEAR(3., state->dataSurfaceGeometry->SurfaceTmp(surfNum).Vertex(4).z, 0.001);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_VertexNumberMismatchTest)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Material,",
        "  8 in.Concrete Block Basement Wall,     !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2032,                                 !- Thickness{ m }",
        "  1.326,                                  !- Conductivity{ W / m - K }",
        "  1841.99999999999,                       !- Density{ kg / m3 }",
        "  911.999999999999,                       !- Specific Heat{ J / kg - K }",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",
        "Construction,",
        "   Typical,   !- Name",
        "   8 in.Concrete Block Basement Wall;     !- Layer 1",

        "BuildingSurface:Detailed,                                   ",
        "	016W88_WaterMeter - Floor : a, !- Name",
        "	Floor, !- Surface Type",
        "	Typical, !- Construction Name",
        "	ZONE 1, !- Zone Name",
        "    ,                        !- Space Name",
        "	Surface, !- Outside Boundary Condition",
        "	006W27_Restrooms - RoofCeiling : a, !- Outside Boundary Condition Object",
        "	NoSun, !- Sun Exposure",
        "	NoWind, !- Wind Exposure",
        "	, !- View Factor to Ground",
        "	, !- Number of Vertices",
        "	251.4600375, 3.5052, 0, !- X, Y, Z Vertex 1 {m}",
        "	251.4600375, 0, 0, !- X, Y, Z Vertex 2 {m}",
        "	249.9571375, 0, 0, !- X, Y, Z Vertex 3 {m}",
        "	248.5215375, 1.0, 0, !- X, Y, Z Vertex 4 {m}",
        "	248.5215375, 3.5052, 0;                 !- X, Y, Z Vertex 5 {m}",

        "BuildingSurface:Detailed,",
        "   006W27_Restrooms - RoofCeiling : a, !- Name",
        "   Ceiling, !- Surface Type",
        "   Typical, !- Construction Name",
        "   ZONE 2, !- Zone Name",
        "    ,                        !- Space Name",
        "   Surface, !- Outside Boundary Condition",
        "   016W88_WaterMeter - Floor : a, !- Outside Boundary Condition Object",
        "   NoSun, !- Sun Exposure",
        "   NoWind, !- Wind Exposure",
        "   , !- View Factor to Ground",
        "   , !- Number of Vertices",
        "   174.6425, 0, 6.1976, !- X, Y, Z Vertex 1 {m}",
        "   174.6425, 3.5052, 6.1976, !- X, Y, Z Vertex 2 {m}",
        "   171.704, 3.5052, 6.1976, !- X, Y, Z Vertex 3 {m}",
        "   171.704, 0, 6.1976;                     !- X, Y, Z Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(2).Name = "ZONE 2";
    state->dataSurfaceGeometry->SurfaceTmp.allocate(2);
    int SurfNum = 0;
    int TotHTSurfs = 2;
    Array1D_string const BaseSurfCls(3, {"WALL", "FLOOR", "ROOF"});
    Array1D<DataSurfaces::SurfaceClass> const BaseSurfIDs(
        3, {DataSurfaces::SurfaceClass::Wall, DataSurfaces::SurfaceClass::Floor, DataSurfaces::SurfaceClass::Roof});
    int NeedToAddSurfaces;

    GetGeometryParameters(*state, ErrorsFound);
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);

    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;

    GetHTSurfaceData(*state, ErrorsFound, SurfNum, TotHTSurfs, 0, 0, 0, BaseSurfCls, BaseSurfIDs, NeedToAddSurfaces);

    EXPECT_EQ(2, SurfNum);
    std::string const error_string =
        delimited_string({"   ** Severe  ** BuildingSurface:Detailed=\"016W88_WATERMETER - FLOOR : A\", invalid Construction Name=\"TYPICAL\".",
                          "   ** Severe  ** BuildingSurface:Detailed=\"006W27_RESTROOMS - ROOFCEILING : A\", invalid Construction Name=\"TYPICAL\".",
                          "   ** Severe  ** RoofCeiling:Detailed=\"016W88_WATERMETER - FLOOR : A\", Vertex size mismatch between base surface "
                          ":016W88_WATERMETER - FLOOR : A and outside boundary surface: 006W27_RESTROOMS - ROOFCEILING : A",
                          "   **   ~~~   ** The vertex sizes are 5 for base surface and 4 for outside boundary surface. Please check inputs.",
                          "   ** Severe  ** RoofCeiling:Detailed=\"006W27_RESTROOMS - ROOFCEILING : A\", Vertex size mismatch between base surface "
                          ":006W27_RESTROOMS - ROOFCEILING : A and outside boundary surface: 016W88_WATERMETER - FLOOR : A",
                          "   **   ~~~   ** The vertex sizes are 4 for base surface and 5 for outside boundary surface. Please check inputs."});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_CheckConvexityTest)
{

    // Test a multiple vertex surfaces in ProcessSurfaceVertices and CalcCoordinateTransformation for #6384

    state->dataSurface->TotSurfaces = 2;
    state->dataSurface->MaxVerticesPerSurface = 9;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->ShadeV.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->Surface(1).Vertex.allocate(7);
    state->dataSurface->Surface(2).Vertex.allocate(9);
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);
    state->dataSurfaceGeometry->SurfaceTmp(1).Vertex.allocate(7);
    state->dataSurfaceGeometry->SurfaceTmp(2).Vertex.allocate(9);

    int ThisSurf(0);

    // Surface 1 - Rectangle with 7 points
    ThisSurf = 1;
    state->dataSurface->Surface(ThisSurf).Azimuth = 0.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 180.0;
    state->dataSurface->Surface(ThisSurf).Sides = 7;
    state->dataSurface->Surface(ThisSurf).GrossArea = 20.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 2.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 3.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 3.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 3.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 4.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 3.0;

    state->dataSurface->Surface(ThisSurf).Vertex(4).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).y = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).z = 3.0;

    state->dataSurface->Surface(ThisSurf).Vertex(5).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(5).y = 6.0;
    state->dataSurface->Surface(ThisSurf).Vertex(5).z = 3.0;

    state->dataSurface->Surface(ThisSurf).Vertex(6).x = 15.0;
    state->dataSurface->Surface(ThisSurf).Vertex(6).y = 6.0;
    state->dataSurface->Surface(ThisSurf).Vertex(6).z = 3.0;

    state->dataSurface->Surface(ThisSurf).Vertex(7).x = 15.0;
    state->dataSurface->Surface(ThisSurf).Vertex(7).y = 2.0;
    state->dataSurface->Surface(ThisSurf).Vertex(7).z = 3.0;

    state->dataSurfaceGeometry->SurfaceTmp(ThisSurf) = state->dataSurface->Surface(ThisSurf);
    CheckConvexity(*state, ThisSurf, state->dataSurfaceGeometry->SurfaceTmp(ThisSurf).Sides);
    state->dataSurface->Surface(ThisSurf) = state->dataSurfaceGeometry->SurfaceTmp(ThisSurf);
    EXPECT_EQ(4, state->dataSurface->Surface(ThisSurf).Sides);
    EXPECT_EQ(10.0, state->dataSurface->Surface(ThisSurf).Vertex(2).x);
    EXPECT_EQ(6.0, state->dataSurface->Surface(ThisSurf).Vertex(2).y);
    EXPECT_EQ(15.0, state->dataSurface->Surface(ThisSurf).Vertex(3).x);
    EXPECT_EQ(6.0, state->dataSurface->Surface(ThisSurf).Vertex(3).y);

    // Surface 2 - Rectangle with 9 points
    ThisSurf = 2;
    state->dataSurface->Surface(ThisSurf).Azimuth = 0.0;
    state->dataSurface->Surface(ThisSurf).Tilt = 0.0;
    state->dataSurface->Surface(ThisSurf).Sides = 9;
    state->dataSurface->Surface(ThisSurf).GrossArea = 30.0;

    state->dataSurface->Surface(ThisSurf).Vertex(1).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).y = 2.0;
    state->dataSurface->Surface(ThisSurf).Vertex(1).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(2).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).y = 3.0;
    state->dataSurface->Surface(ThisSurf).Vertex(2).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(3).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).y = 4.0;
    state->dataSurface->Surface(ThisSurf).Vertex(3).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(4).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).y = 5.0;
    state->dataSurface->Surface(ThisSurf).Vertex(4).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(5).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(5).y = 6.0;
    state->dataSurface->Surface(ThisSurf).Vertex(5).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(6).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(6).y = 7.0;
    state->dataSurface->Surface(ThisSurf).Vertex(6).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(7).x = 10.0;
    state->dataSurface->Surface(ThisSurf).Vertex(7).y = 8.0;
    state->dataSurface->Surface(ThisSurf).Vertex(7).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(8).x = 15.0;
    state->dataSurface->Surface(ThisSurf).Vertex(8).y = 8.0;
    state->dataSurface->Surface(ThisSurf).Vertex(8).z = 0.0;

    state->dataSurface->Surface(ThisSurf).Vertex(9).x = 15.0;
    state->dataSurface->Surface(ThisSurf).Vertex(9).y = 2.0;
    state->dataSurface->Surface(ThisSurf).Vertex(9).z = 0.0;

    state->dataSurfaceGeometry->SurfaceTmp(ThisSurf) = state->dataSurface->Surface(ThisSurf);
    CheckConvexity(*state, ThisSurf, state->dataSurfaceGeometry->SurfaceTmp(ThisSurf).Sides);
    state->dataSurface->Surface(ThisSurf) = state->dataSurfaceGeometry->SurfaceTmp(ThisSurf);
    EXPECT_EQ(4, state->dataSurface->Surface(ThisSurf).Sides);
    EXPECT_EQ(10.0, state->dataSurface->Surface(ThisSurf).Vertex(2).x);
    EXPECT_EQ(8.0, state->dataSurface->Surface(ThisSurf).Vertex(2).y);
    EXPECT_EQ(15.0, state->dataSurface->Surface(ThisSurf).Vertex(3).x);
    EXPECT_EQ(8.0, state->dataSurface->Surface(ThisSurf).Vertex(3).y);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_CheckConvexityTest_9118)
{

    // Test for #9118. The key point is that a collinear vertex should be found on the first occurrence (that is vertex 2 is collinear with 1 and 3)
    //      y
    //     ▲
    //     │ [5]     [4]
    //  10 o──────o──────o[3]
    //     │             │
    //     │             │
    //     │             │
    //   5 o [6]         o [2]
    //     │             │
    //     │             │
    //     │ [7]  [8]    │ [1]
    //     o──────o──────o───►
    //   0        5      10   x

    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->MaxVerticesPerSurface = 8;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);
    auto &surface = state->dataSurfaceGeometry->SurfaceTmp(1);
    surface.Vertex.allocate(8);

    surface.Azimuth = 0.0;
    surface.Tilt = 0.0;
    surface.Sides = 8;
    surface.GrossArea = 100.0;

    std::vector<Vector> baseVertices = {
        {10.0, 0.0, 0.0},
        {10.0, 10.0, 0.0},
        {0.0, 10.0, 0.0},
        {0.0, 0.0, 0.0},
    };
    int i = 0;
    auto &vertices = surface.Vertex;

    for (auto it = std::cbegin(baseVertices); it != std::cend(baseVertices); ++it) {
        auto itnext = std::next(it);
        if (itnext == std::cend(baseVertices)) {
            itnext = std::cbegin(baseVertices);
        }
        vertices(++i) = *it;
        Vector midPoint = (*it + *itnext) / 2.0; // V1 + (V2 - V1) / 2.0 == (V1 + V2) / 2.0
        vertices(++i) = midPoint;
    }

    std::vector<Vector> actualVertices = {
        {+10.0000, +0.0000, +0.0000},
        {+10.0000, +5.0000, +0.0000},
        {+10.0000, +10.0000, +0.0000},
        {+5.0000, +10.0000, +0.0000},
        {+0.0000, +10.0000, +0.0000},
        {+0.0000, +5.0000, +0.0000},
        {+0.0000, +0.0000, +0.0000},
        {+5.0000, +0.0000, +0.0000},
    };
    i = -1;
    for (const auto &v : vertices) {
        EXPECT_EQ(actualVertices[++i], v);
    }

    CheckConvexity(*state, 1, surface.Sides);

    EXPECT_EQ(4, surface.Sides);
    EXPECT_TRUE(surface.IsConvex);

    for (int i = 1; i <= 4; ++i) {
        EXPECT_EQ(baseVertices[i - 1], vertices(i)) << "Failed for Vertex " << i;
    }

    // Now perform the same test, except the first vertices is colinear, to ensure we also get that case covered
    // We move all vertices in the clockwise order by one
    //      y
    //     ▲
    //     │ [6]     [5]
    //  10 o──────o──────o[4]
    //     │             │
    //     │             │
    //     │             │
    //   5 o [7]         o [3]
    //     │             │
    //     │             │
    //     │ [8]  [1]    │ [2]
    //     o──────o──────o───►
    //   0        5      10   x
    std::rotate(actualVertices.rbegin(), actualVertices.rbegin() + 1, actualVertices.rend());
    surface.Azimuth = 0.0;
    surface.Tilt = 0.0;
    surface.Sides = 8;
    surface.GrossArea = 100.0;
    vertices.deallocate();
    vertices.allocate(8);
    vertices = actualVertices;
    CheckConvexity(*state, 1, surface.Sides);

    EXPECT_EQ(4, surface.Sides);
    EXPECT_TRUE(surface.IsConvex);

    for (int i = 1; i <= 4; ++i) {
        EXPECT_EQ(baseVertices[i - 1], vertices(i)) << "Failed for Vertex " << i;
    }
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_CheckConvexityTest_ASHRAE901_Hospital_STD2019_Denver)
{

    // Test for #9118. This is ASHRAE901_Hospital_STD2019_Denver.idf
    //  y
    //    ▲
    //    │
    //    │            6 ┌──────┐ 5
    //    │    8         │      │
    //    │    ┌─────────┘7     │
    //    │    │                │
    //    │    │   ┌────────────┘
    //    │    │   │3             4
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    │    │   │
    //    └────┴───┴────────────────────►
    //         1   2                     x

    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->MaxVerticesPerSurface = 8;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);
    auto &surface = state->dataSurfaceGeometry->SurfaceTmp(1);

    surface.Azimuth = 0.0;
    surface.Tilt = 0.0;
    surface.Sides = 8;
    surface.GrossArea = 100.0;
    surface.Name = "CORRIDOR_FLR_5_CEILING";
    surface.Vertex.allocate(8);
    std::vector<Vector> vertices = {
        {9.1440, 0.0000, 21.3415},
        {15.2400, 0.0000, 21.3415},
        {15.2400, 42.6720, 21.3415},
        {39.6240, 42.6720, 21.3415},
        {39.6240, 53.3400, 21.3415},
        {27.4320, 53.3400, 21.3415},
        {27.4320, 48.7680, 21.3415},
        {9.1440, 48.7680, 21.3415},
    };
    surface.Vertex = vertices;

    CheckConvexity(*state, 1, surface.Sides);

    EXPECT_EQ(8, surface.Sides);
    EXPECT_FALSE(surface.IsConvex);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_CheckConvexity_ColinearStability)
{
    // Test for #10103 - Regardless of the order of the vertices, we should drop colinear vertices consistently to avoid a fatal error due to vertex
    // side mismatch
    //
    //  y
    //    ▲          7     8     9
    //    │          ┌─────x────┐
    //    │          │          │
    //    │          │          │
    //    │          │          │
    //    │5         │          │
    //    ├──────────┘          │
    //    │           6         │
    //    │                     │
    //    │                     │
    //    │                     │10
    //    └────x─────x─────x────┴─────►
    //    4     3      2      1           x

    const std::vector<Vector> floorVertices = {
        {30.0, 0., 0.},
        {20.0, 0., 0.},
        {10.0, 0., 0.},
        {0.0, 0., 0.},
        {0.0, 20., 0.},
        {20.0, 20., 0.},
        {20.0, 40., 0.},
        {30.0, 40., 0.},
        {40.0, 40., 0.},
        {40.0, 0., 0.},
    };
    const int nVertices = static_cast<int>(floorVertices.size());

    state->dataSurface->TotSurfaces = 2;
    constexpr int floorSurfNum = 1;
    constexpr int ceilingSurfNum = 2;
    state->dataSurface->MaxVerticesPerSurface = nVertices;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);

    auto &floorSurface = state->dataSurfaceGeometry->SurfaceTmp(floorSurfNum);
    auto &ceilingSurface = state->dataSurfaceGeometry->SurfaceTmp(ceilingSurfNum);
    {
        floorSurface.Azimuth = 0.0;
        floorSurface.Tilt = 0.0;
        floorSurface.Sides = nVertices;
        floorSurface.GrossArea = 100.0;
        floorSurface.Name = "Floor";
        floorSurface.Vertex.allocate(nVertices);

        floorSurface.Vertex = floorVertices;

        CheckConvexity(*state, floorSurfNum, floorSurface.Sides);

        EXPECT_EQ(6, floorSurface.Sides);
        EXPECT_FALSE(floorSurface.IsConvex);
    }

    {
        auto ceilingVertices = floorVertices;
        std::reverse(ceilingVertices.begin(), ceilingVertices.end());

        ceilingSurface.Azimuth = 0.0;
        ceilingSurface.Tilt = 0.0;
        ceilingSurface.Sides = nVertices;
        ceilingSurface.GrossArea = 100.0;
        ceilingSurface.Name = "Ceiling";
        ceilingSurface.Vertex.allocate(nVertices);

        ceilingSurface.Vertex = ceilingVertices;

        CheckConvexity(*state, ceilingSurfNum, ceilingSurface.Sides);

        EXPECT_EQ(6, ceilingSurface.Sides);
        EXPECT_FALSE(ceilingSurface.IsConvex);
    }

    EXPECT_EQ(floorSurface.Sides, ceilingSurface.Sides);
}

TEST_F(EnergyPlusFixture, InitialAssociateWindowShadingControlFenestration_test)
{
    state->dataSurface->TotWinShadingControl = 3;
    state->dataSurface->WindowShadingControl.allocate(state->dataSurface->TotWinShadingControl);
    int zn = 1;

    state->dataSurface->WindowShadingControl(1).Name = "WSC1";
    state->dataSurface->WindowShadingControl(1).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(1).SequenceNumber = 2;
    state->dataSurface->WindowShadingControl(1).multiSurfaceControl = MultiSurfaceControl::Group;
    state->dataSurface->WindowShadingControl(1).FenestrationCount = 3;
    state->dataSurface->WindowShadingControl(1).FenestrationName.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationName(1) = "Fene-01";
    state->dataSurface->WindowShadingControl(1).FenestrationName(2) = "Fene-02";
    state->dataSurface->WindowShadingControl(1).FenestrationName(3) = "Fene-03";

    state->dataSurface->WindowShadingControl(2).Name = "WSC2";
    state->dataSurface->WindowShadingControl(2).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(2).SequenceNumber = 3;
    state->dataSurface->WindowShadingControl(2).multiSurfaceControl = MultiSurfaceControl::Sequential;
    state->dataSurface->WindowShadingControl(2).FenestrationCount = 4;
    state->dataSurface->WindowShadingControl(2).FenestrationName.allocate(state->dataSurface->WindowShadingControl(2).FenestrationCount);
    state->dataSurface->WindowShadingControl(2).FenestrationName(1) = "Fene-04";
    state->dataSurface->WindowShadingControl(2).FenestrationName(2) = "Fene-05";
    state->dataSurface->WindowShadingControl(2).FenestrationName(3) = "Fene-06";
    state->dataSurface->WindowShadingControl(2).FenestrationName(4) = "Fene-07";

    state->dataSurface->WindowShadingControl(3).Name = "WSC3";
    state->dataSurface->WindowShadingControl(3).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(3).SequenceNumber = 1;
    state->dataSurface->WindowShadingControl(3).multiSurfaceControl = MultiSurfaceControl::Group;
    state->dataSurface->WindowShadingControl(3).FenestrationCount = 2;
    state->dataSurface->WindowShadingControl(3).FenestrationName.allocate(state->dataSurface->WindowShadingControl(3).FenestrationCount);
    state->dataSurface->WindowShadingControl(3).FenestrationName(1) = "Fene-08";
    state->dataSurface->WindowShadingControl(3).FenestrationName(2) = "Fene-09";

    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).WindowTypeEQL = false;

    state->dataSurfaceGeometry->SurfaceTmp.allocate(9);

    state->dataSurfaceGeometry->SurfaceTmp(1).Name = "Fene-04";
    state->dataSurfaceGeometry->SurfaceTmp(1).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(1).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(2).Name = "Fene-05";
    state->dataSurfaceGeometry->SurfaceTmp(2).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(2).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(3).Name = "Fene-06";
    state->dataSurfaceGeometry->SurfaceTmp(3).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(3).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(4).Name = "Fene-01";
    state->dataSurfaceGeometry->SurfaceTmp(4).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(4).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(5).Name = "Fene-02";
    state->dataSurfaceGeometry->SurfaceTmp(5).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(5).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(6).Name = "Fene-03";
    state->dataSurfaceGeometry->SurfaceTmp(6).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(6).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(7).Name = "Fene-07";
    state->dataSurfaceGeometry->SurfaceTmp(7).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(7).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(8).Name = "Fene-08";
    state->dataSurfaceGeometry->SurfaceTmp(8).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(8).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(9).Name = "Fene-09";
    state->dataSurfaceGeometry->SurfaceTmp(9).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(9).ExtBoundCond = ExternalEnvironment;

    bool Err = false;

    int surfNum = 1;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 2);
    EXPECT_FALSE(Err);

    surfNum = 2;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 2);
    EXPECT_FALSE(Err);

    surfNum = 3;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 2);
    EXPECT_FALSE(Err);

    surfNum = 4;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 1);
    EXPECT_FALSE(Err);

    surfNum = 5;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 1);
    EXPECT_FALSE(Err);

    surfNum = 6;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 1);
    EXPECT_FALSE(Err);

    surfNum = 7;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 2);
    EXPECT_FALSE(Err);

    surfNum = 8;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 3);
    EXPECT_FALSE(Err);

    surfNum = 9;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).activeWindowShadingControl, 3);
    EXPECT_FALSE(Err);
}

TEST_F(EnergyPlusFixture, InitialAssociateWindowShadingControlFenestration_Multi_test)
{
    state->dataSurface->TotWinShadingControl = 3;
    state->dataSurface->WindowShadingControl.allocate(state->dataSurface->TotWinShadingControl);
    int zn = 1;

    state->dataSurface->WindowShadingControl(1).Name = "WSC1";
    state->dataSurface->WindowShadingControl(1).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(1).SequenceNumber = 2;
    state->dataSurface->WindowShadingControl(1).multiSurfaceControl = MultiSurfaceControl::Group;
    state->dataSurface->WindowShadingControl(1).FenestrationCount = 3;
    state->dataSurface->WindowShadingControl(1).FenestrationName.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationName(1) = "Fene-01";
    state->dataSurface->WindowShadingControl(1).FenestrationName(2) = "Fene-02";
    state->dataSurface->WindowShadingControl(1).FenestrationName(3) = "Fene-03";

    state->dataSurface->WindowShadingControl(2).Name = "WSC2";
    state->dataSurface->WindowShadingControl(2).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(2).SequenceNumber = 3;
    state->dataSurface->WindowShadingControl(2).multiSurfaceControl = MultiSurfaceControl::Sequential;
    state->dataSurface->WindowShadingControl(2).FenestrationCount = 4;
    state->dataSurface->WindowShadingControl(2).FenestrationName.allocate(state->dataSurface->WindowShadingControl(2).FenestrationCount);
    state->dataSurface->WindowShadingControl(2).FenestrationName(1) = "Fene-02";
    state->dataSurface->WindowShadingControl(2).FenestrationName(2) = "Fene-03";
    state->dataSurface->WindowShadingControl(2).FenestrationName(3) = "Fene-04";
    state->dataSurface->WindowShadingControl(2).FenestrationName(4) = "Fene-05";

    state->dataSurface->WindowShadingControl(3).Name = "WSC3";
    state->dataSurface->WindowShadingControl(3).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(3).SequenceNumber = 1;
    state->dataSurface->WindowShadingControl(3).multiSurfaceControl = MultiSurfaceControl::Group;
    state->dataSurface->WindowShadingControl(3).FenestrationCount = 2;
    state->dataSurface->WindowShadingControl(3).FenestrationName.allocate(state->dataSurface->WindowShadingControl(3).FenestrationCount);
    state->dataSurface->WindowShadingControl(3).FenestrationName(1) = "Fene-03";
    state->dataSurface->WindowShadingControl(3).FenestrationName(2) = "Fene-05";

    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).WindowTypeEQL = false;

    state->dataSurfaceGeometry->SurfaceTmp.allocate(5);

    state->dataSurfaceGeometry->SurfaceTmp(1).Name = "Fene-01";
    state->dataSurfaceGeometry->SurfaceTmp(1).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(1).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(2).Name = "Fene-02";
    state->dataSurfaceGeometry->SurfaceTmp(2).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(2).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(3).Name = "Fene-03";
    state->dataSurfaceGeometry->SurfaceTmp(3).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(3).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(4).Name = "Fene-04";
    state->dataSurfaceGeometry->SurfaceTmp(4).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(4).ExtBoundCond = ExternalEnvironment;

    state->dataSurfaceGeometry->SurfaceTmp(5).Name = "Fene-05";
    state->dataSurfaceGeometry->SurfaceTmp(5).Construction = 1;
    state->dataSurfaceGeometry->SurfaceTmp(5).ExtBoundCond = ExternalEnvironment;

    bool Err = false;

    int surfNum = 1;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList.size(), 1u);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[0], 1);
    EXPECT_FALSE(Err);

    surfNum = 2;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList.size(), 2u);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[0], 1);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[1], 2);
    EXPECT_FALSE(Err);

    surfNum = 3;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList.size(), 3u);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[0], 1);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[1], 2);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[2], 3);
    EXPECT_FALSE(Err);

    surfNum = 4;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList.size(), 1u);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[0], 2);
    EXPECT_FALSE(Err);

    surfNum = 5;
    InitialAssociateWindowShadingControlFenestration(*state, Err, surfNum);
    EXPECT_TRUE(state->dataSurfaceGeometry->SurfaceTmp(surfNum).HasShadeControl);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList.size(), 2u);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[0], 2);
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(surfNum).windowShadingControlList[1], 3);
    EXPECT_FALSE(Err);
}

TEST_F(EnergyPlusFixture, FinalAssociateWindowShadingControlFenestration_test)
{
    state->dataSurface->TotWinShadingControl = 3;
    state->dataSurface->WindowShadingControl.allocate(state->dataSurface->TotWinShadingControl);
    int zn = 1;

    state->dataSurface->WindowShadingControl(1).Name = "WSC1";
    state->dataSurface->WindowShadingControl(1).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(1).SequenceNumber = 2;
    state->dataSurface->WindowShadingControl(1).multiSurfaceControl = MultiSurfaceControl::Group;
    state->dataSurface->WindowShadingControl(1).FenestrationCount = 3;
    state->dataSurface->WindowShadingControl(1).FenestrationName.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationIndex.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationName(1) = "Fene-01";
    state->dataSurface->WindowShadingControl(1).FenestrationName(2) = "Fene-02";
    state->dataSurface->WindowShadingControl(1).FenestrationName(3) = "Fene-03";

    state->dataSurface->WindowShadingControl(2).Name = "WSC2";
    state->dataSurface->WindowShadingControl(2).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(2).SequenceNumber = 3;
    state->dataSurface->WindowShadingControl(2).multiSurfaceControl = MultiSurfaceControl::Sequential;
    state->dataSurface->WindowShadingControl(2).FenestrationCount = 4;
    state->dataSurface->WindowShadingControl(2).FenestrationName.allocate(state->dataSurface->WindowShadingControl(2).FenestrationCount);
    state->dataSurface->WindowShadingControl(2).FenestrationIndex.allocate(state->dataSurface->WindowShadingControl(2).FenestrationCount);
    state->dataSurface->WindowShadingControl(2).FenestrationName(1) = "Fene-04";
    state->dataSurface->WindowShadingControl(2).FenestrationName(2) = "Fene-05";
    state->dataSurface->WindowShadingControl(2).FenestrationName(3) = "Fene-06";
    state->dataSurface->WindowShadingControl(2).FenestrationName(4) = "Fene-07";

    state->dataSurface->WindowShadingControl(3).Name = "WSC3";
    state->dataSurface->WindowShadingControl(3).ZoneIndex = zn;
    state->dataSurface->WindowShadingControl(3).SequenceNumber = 1;
    state->dataSurface->WindowShadingControl(3).multiSurfaceControl = MultiSurfaceControl::Group;
    state->dataSurface->WindowShadingControl(3).FenestrationCount = 2;
    state->dataSurface->WindowShadingControl(3).FenestrationName.allocate(state->dataSurface->WindowShadingControl(3).FenestrationCount);
    state->dataSurface->WindowShadingControl(3).FenestrationIndex.allocate(state->dataSurface->WindowShadingControl(3).FenestrationCount);
    state->dataSurface->WindowShadingControl(3).FenestrationName(1) = "Fene-08";
    state->dataSurface->WindowShadingControl(3).FenestrationName(2) = "Fene-09";

    state->dataSurface->TotSurfaces = 12;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    state->dataSurface->Surface(1).Name = "Fene-07";
    state->dataSurface->Surface(1).windowShadingControlList.push_back(2);

    state->dataSurface->Surface(2).Name = "Fene-01";
    state->dataSurface->Surface(2).windowShadingControlList.push_back(1);

    state->dataSurface->Surface(3).Name = "Fene-08";
    state->dataSurface->Surface(3).windowShadingControlList.push_back(3);

    state->dataSurface->Surface(4).Name = "Fene-02";
    state->dataSurface->Surface(4).windowShadingControlList.push_back(1);

    state->dataSurface->Surface(5).Name = "Fene-10";
    state->dataSurface->Surface(5).windowShadingControlList.push_back(0);

    state->dataSurface->Surface(6).Name = "Fene-03";
    state->dataSurface->Surface(6).windowShadingControlList.push_back(1);

    state->dataSurface->Surface(7).Name = "Fene-09";
    state->dataSurface->Surface(7).windowShadingControlList.push_back(3);

    state->dataSurface->Surface(8).Name = "Fene-04";
    state->dataSurface->Surface(8).windowShadingControlList.push_back(2);

    state->dataSurface->Surface(9).Name = "Fene-10";
    state->dataSurface->Surface(9).windowShadingControlList.push_back(0);

    state->dataSurface->Surface(10).Name = "Fene-05";
    state->dataSurface->Surface(10).windowShadingControlList.push_back(2);

    state->dataSurface->Surface(11).Name = "Fene-11";
    state->dataSurface->Surface(11).windowShadingControlList.push_back(0);

    state->dataSurface->Surface(12).Name = "Fene-06";
    state->dataSurface->Surface(12).windowShadingControlList.push_back(2);

    bool Err = false;

    FinalAssociateWindowShadingControlFenestration(*state, Err);
    EXPECT_FALSE(Err);

    EXPECT_EQ(state->dataSurface->WindowShadingControl(1).FenestrationIndex(1), 2);
    EXPECT_EQ(state->dataSurface->WindowShadingControl(1).FenestrationIndex(2), 4);
    EXPECT_EQ(state->dataSurface->WindowShadingControl(1).FenestrationIndex(3), 6);

    EXPECT_EQ(state->dataSurface->WindowShadingControl(2).FenestrationIndex(1), 8);
    EXPECT_EQ(state->dataSurface->WindowShadingControl(2).FenestrationIndex(2), 10);
    EXPECT_EQ(state->dataSurface->WindowShadingControl(2).FenestrationIndex(3), 12);
    EXPECT_EQ(state->dataSurface->WindowShadingControl(2).FenestrationIndex(4), 1);

    EXPECT_EQ(state->dataSurface->WindowShadingControl(3).FenestrationIndex(1), 3);
    EXPECT_EQ(state->dataSurface->WindowShadingControl(3).FenestrationIndex(2), 7);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_isWindowShadingControlSimilar_Test)
{
    state->dataSurface->WindowShadingControl.allocate(2);

    state->dataSurface->WindowShadingControl(1).Name = "TheShadingControl";
    state->dataSurface->WindowShadingControl(1).ZoneIndex = 57;
    state->dataSurface->WindowShadingControl(1).SequenceNumber = 3;
    state->dataSurface->WindowShadingControl(1).ShadingType = WinShadingType::ExtShade;
    state->dataSurface->WindowShadingControl(1).ShadingDevice = 17;
    state->dataSurface->WindowShadingControl(1).shadingControlType = WindowShadingControlType::OnIfScheduled;
    state->dataSurface->WindowShadingControl(1).sched = nullptr;
    state->dataSurface->WindowShadingControl(1).SetPoint = 200;
    state->dataSurface->WindowShadingControl(1).SetPoint2 = 170;
    state->dataSurface->WindowShadingControl(1).ShadingControlIsScheduled = true;
    state->dataSurface->WindowShadingControl(1).GlareControlIsActive = false;
    state->dataSurface->WindowShadingControl(1).slatAngleSched = nullptr;
    state->dataSurface->WindowShadingControl(1).slatAngleControl = SlatAngleControl::BlockBeamSolar;
    state->dataSurface->WindowShadingControl(1).DaylightingControlName = "TheDaylightingControl";
    state->dataSurface->WindowShadingControl(1).DaylightControlIndex = 7;
    state->dataSurface->WindowShadingControl(1).multiSurfaceControl = MultiSurfaceControl::Sequential;

    state->dataSurface->WindowShadingControl(1).FenestrationCount = 3;
    state->dataSurface->WindowShadingControl(1).FenestrationName.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationName(1) = "Fene-01";
    state->dataSurface->WindowShadingControl(1).FenestrationName(2) = "Fene-02";
    state->dataSurface->WindowShadingControl(1).FenestrationName(3) = "Fene-03";
    state->dataSurface->WindowShadingControl(1).FenestrationIndex.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationIndex(1) = 11;
    state->dataSurface->WindowShadingControl(1).FenestrationIndex(2) = 12;
    state->dataSurface->WindowShadingControl(1).FenestrationIndex(3) = 13;

    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    // no changes
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));

    // changes to portions of struct that are not "similar"
    // these should not impact similarity so changes are ignored

    state->dataSurface->WindowShadingControl(2).Name = "Different";
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).SequenceNumber = 9;
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).ShadingDevice = 21;
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).sched = Sched::GetScheduleAlwaysOn(*state);
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).slatAngleSched = Sched::GetScheduleAlwaysOff(*state);
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).FenestrationCount = 4;
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).FenestrationName(3) = "Fene-Different";
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).FenestrationIndex(3) = 17;
    EXPECT_TRUE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    // changes to portions of struct that are "similar"
    // these are important so they should be shown as false

    state->dataSurface->WindowShadingControl(2).ZoneIndex = 83;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).ShadingType = WinShadingType::BGBlind;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).shadingControlType = WindowShadingControlType::OffNight_OnDay_HiSolarWindow;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).SetPoint = 140;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).SetPoint2 = 169;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).ShadingControlIsScheduled = false;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).GlareControlIsActive = true;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).slatAngleControl = SlatAngleControl::Fixed;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).DaylightingControlName = "Different";
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).DaylightControlIndex = 12;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);

    state->dataSurface->WindowShadingControl(2).multiSurfaceControl = MultiSurfaceControl::Group;
    EXPECT_FALSE(isWindowShadingControlSimilar(*state, 1, 2));
    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_CheckWindowShadingControlSimilarForWindow_Test)
{
    state->dataSurface->Surface.allocate(1);

    state->dataSurface->Surface(1).HasShadeControl = true;
    state->dataSurface->Surface(1).windowShadingControlList.push_back(1);
    state->dataSurface->Surface(1).windowShadingControlList.push_back(2);
    state->dataSurface->Surface(1).windowShadingControlList.push_back(3);

    state->dataSurface->WindowShadingControl.allocate(3);

    state->dataSurface->WindowShadingControl(1).Name = "TheShadingControl";
    state->dataSurface->WindowShadingControl(1).ZoneIndex = 57;
    state->dataSurface->WindowShadingControl(1).SequenceNumber = 3;
    state->dataSurface->WindowShadingControl(1).ShadingType = WinShadingType::ExtShade;
    state->dataSurface->WindowShadingControl(1).ShadingDevice = 17;
    state->dataSurface->WindowShadingControl(1).shadingControlType = WindowShadingControlType::OnIfScheduled;
    state->dataSurface->WindowShadingControl(1).sched = nullptr;
    state->dataSurface->WindowShadingControl(1).SetPoint = 200;
    state->dataSurface->WindowShadingControl(1).SetPoint2 = 170;
    state->dataSurface->WindowShadingControl(1).ShadingControlIsScheduled = true;
    state->dataSurface->WindowShadingControl(1).GlareControlIsActive = false;
    state->dataSurface->WindowShadingControl(1).slatAngleSched = nullptr;
    state->dataSurface->WindowShadingControl(1).slatAngleControl = SlatAngleControl::BlockBeamSolar;
    state->dataSurface->WindowShadingControl(1).DaylightingControlName = "TheDaylightingControl";
    state->dataSurface->WindowShadingControl(1).DaylightControlIndex = 7;
    state->dataSurface->WindowShadingControl(1).multiSurfaceControl = MultiSurfaceControl::Sequential;

    state->dataSurface->WindowShadingControl(1).FenestrationCount = 3;
    state->dataSurface->WindowShadingControl(1).FenestrationName.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationName(1) = "Fene-01";
    state->dataSurface->WindowShadingControl(1).FenestrationName(2) = "Fene-02";
    state->dataSurface->WindowShadingControl(1).FenestrationName(3) = "Fene-03";
    state->dataSurface->WindowShadingControl(1).FenestrationIndex.allocate(state->dataSurface->WindowShadingControl(1).FenestrationCount);
    state->dataSurface->WindowShadingControl(1).FenestrationIndex(1) = 11;
    state->dataSurface->WindowShadingControl(1).FenestrationIndex(2) = 12;
    state->dataSurface->WindowShadingControl(1).FenestrationIndex(3) = 13;

    state->dataSurface->WindowShadingControl(2) = state->dataSurface->WindowShadingControl(1);
    state->dataSurface->WindowShadingControl(3) = state->dataSurface->WindowShadingControl(1);

    bool errorsOccurred = false;

    CheckWindowShadingControlSimilarForWindow(*state, errorsOccurred);
    EXPECT_FALSE(errorsOccurred);

    state->dataSurface->WindowShadingControl(2).SetPoint = 140;
    CheckWindowShadingControlSimilarForWindow(*state, errorsOccurred);
    EXPECT_TRUE(errorsOccurred);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_createAirMaterialFromDistance_Test)
{
    auto &s_mat = state->dataMaterial;
    createAirMaterialFromDistance(*state, 0.008, "test_air_");
    auto const *mat1 = dynamic_cast<Material::MaterialGasMix const *>(s_mat->materials(1));
    EXPECT_EQ(mat1->Name, "test_air_8MM");
    EXPECT_EQ(mat1->Thickness, 0.008);
    EXPECT_EQ(mat1->gases[0].con.c0, 2.873e-3);
    EXPECT_EQ(mat1->gases[0].con.c1, 7.760e-5);

    createAirMaterialFromDistance(*state, 0.012, "test_air_");
    auto const *mat2 = dynamic_cast<Material::MaterialGasMix const *>(s_mat->materials(2));
    EXPECT_EQ(mat2->Name, "test_air_12MM");
    EXPECT_EQ(mat2->Thickness, 0.012);

    createAirMaterialFromDistance(*state, 0.008, "test_air_");
    EXPECT_EQ(s_mat->materials.isize(), 2);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_createConstructionWithStorm_Test)
{
    auto &s_mat = state->dataMaterial;
    state->dataHeatBal->TotConstructs = 1;
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);

    auto *matStorm = new Material::MaterialGlass;
    s_mat->materials.push_back(matStorm);
    matStorm->AbsorpThermalFront = 0.11;

    auto *matGap = new Material::MaterialGasMix;
    s_mat->materials.push_back(matGap);

    auto *mat3 = new Material::MaterialGlass;
    s_mat->materials.push_back(mat3);
    auto *mat4 = new Material::MaterialGlass;
    s_mat->materials.push_back(mat4);
    auto *mat5 = new Material::MaterialGlass;
    s_mat->materials.push_back(mat5);

    // Case 1a: Constructs with regular materials are a reverse of each other--material layers match in reverse (should get a "false" answer)
    state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).TotLayers = 3;
    state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(1) = 3;
    state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(2) = 4;
    state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(3) = 5;

    createConstructionWithStorm(*state, 1, "construction_A", 1, 2);
    EXPECT_EQ(state->dataHeatBal->TotConstructs, 2);
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).Name, "construction_A");
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(1), 1);
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(2), 2);
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(3), 3);
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(4), 4);
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).LayerPoint(5), 5);
    EXPECT_EQ(state->dataConstruction->Construct(state->dataHeatBal->TotConstructs).OutsideAbsorpThermal, 0.11);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_HeatTransferAlgorithmTest)
{
    // Test surface heat transfer algorithms and heat balance surface lists
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Timestep, 20;"
        "  HeatBalanceAlgorithm,",
        "  MoisturePenetrationDepthConductionTransferFunction; !- Algorithm",
        "Material,",
        "    Gypsum Board,            !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0159,                  !- Thickness {m}",
        "    0.16,                    !- Conductivity {W/m-K}",
        "    800,                     !- Density {kg/m3}",
        "    1090,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Material,",
        "    InsulationBatt,        !- Name",
        "    VeryRough,               !- Roughness",
        "    0.3048,                  !- Thickness {m}",
        "    0.05,                    !- Conductivity {W/m-K}",
        "    19,                      !- Density {kg/m3}",
        "    960,                     !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Material,",
        "    InfiniteRPCM23C,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Project semi-exposed ceiling,  !- Name",
        "    InsulationBatt,        !- Outside Layer",
        "    InfiniteRPCM23C,         !- Layer 2",
        "    Gypsum Board;            !- Layer 3",

        "Construction,",
        "    Project semi-exposed ceiling_Rev,  !- Name",
        "    Gypsum Board,            !- Outside Layer",
        "    InfiniteRPCM23C,         !- Layer 2",
        "    InsulationBatt;        !- Layer 3",

        "BuildingSurface:Detailed,",
        "    DATATELCOM_Ceiling_1_0_0,  !- Name",
        "    Ceiling,                 !- Surface Type",
        "    Project semi-exposed ceiling,  !- Construction Name",
        "    DATATELCOM,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1_Floor_4_0_10000,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    9.64595244,              !- Vertex 1 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 1 Y-coordinate {m}",
        "    3.499104,                !- Vertex 1 Z-coordinate {m}",
        "    13.20708687,             !- Vertex 2 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 2 Y-coordinate {m}",
        "    3.499104,                !- Vertex 2 Z-coordinate {m}",
        "    13.20708687,             !- Vertex 3 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 3 Y-coordinate {m}",
        "    3.499104,                !- Vertex 3 Z-coordinate {m}",
        "    9.64595244,              !- Vertex 4 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 4 Y-coordinate {m}",
        "    3.499104;                !- Vertex 4 Z-coordinate {m}",

        "BuildingSurface:Detailed,",
        "    Zone1_Floor_4_0_10000,  !- Name",
        "    Floor,                   !- Surface Type",
        "    Project semi-exposed ceiling_Rev,  !- Construction Name",
        "    ZONE1,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    DATATELCOM_Ceiling_1_0_0,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.20708687,             !- Vertex 1 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 1 Y-coordinate {m}",
        "    3.499104,                !- Vertex 1 Z-coordinate {m}",
        "    9.64595244,              !- Vertex 2 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 2 Y-coordinate {m}",
        "    3.499104,                !- Vertex 2 Z-coordinate {m}",
        "    9.64595244,              !- Vertex 3 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 3 Y-coordinate {m}",
        "    3.499104,                !- Vertex 3 Z-coordinate {m}",
        "    13.20708687,             !- Vertex 4 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 4 Y-coordinate {m}",
        "    3.499104;                !- Vertex 4 Z-coordinate {m}",

        "BuildingSurface:Detailed,",
        "    Zone1_Floor_4_0_20000,  !- Name",
        "    Floor,                   !- Surface Type",
        "    Project semi-exposed ceiling_Rev,  !- Construction Name",
        "    ZONE1,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    23.20708687,             !- Vertex 1 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 1 Y-coordinate {m}",
        "    3.499104,                !- Vertex 1 Z-coordinate {m}",
        "    19.64595244,              !- Vertex 2 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 2 Y-coordinate {m}",
        "    3.499104,                !- Vertex 2 Z-coordinate {m}",
        "    19.64595244,              !- Vertex 3 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 3 Y-coordinate {m}",
        "    3.499104,                !- Vertex 3 Z-coordinate {m}",
        "    23.20708687,             !- Vertex 4 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 4 Y-coordinate {m}",
        "    3.499104;                !- Vertex 4 Z-coordinate {m}",

        "BuildingSurface:Detailed,",
        "    Zone1_Floor_4_0_30000,  !- Name",
        "    Floor,                   !- Surface Type",
        "    Project semi-exposed ceiling_Rev,  !- Construction Name",
        "    ZONE1,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    23.20708687,             !- Vertex 1 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 1 Y-coordinate {m}",
        "    3.499104,                !- Vertex 1 Z-coordinate {m}",
        "    19.64595244,              !- Vertex 2 X-coordinate {m}",
        "    8.1545602599,            !- Vertex 2 Y-coordinate {m}",
        "    3.499104,                !- Vertex 2 Z-coordinate {m}",
        "    19.64595244,              !- Vertex 3 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 3 Y-coordinate {m}",
        "    3.499104,                !- Vertex 3 Z-coordinate {m}",
        "    23.20708687,             !- Vertex 4 X-coordinate {m}",
        "    10.0470868899,           !- Vertex 4 Y-coordinate {m}",
        "    3.499104;                !- Vertex 4 Z-coordinate {m}",

        "SurfaceProperty:HeatTransferAlgorithm:Construction,",
        "    Ceilings CondFD,               !- Name",
        "    ConductionFiniteDifference,    !- Algorithm",
        "    Project semi-exposed ceiling;  !- Construction Name",

        "SurfaceProperty:HeatTransferAlgorithm,",
        "    Zone1_Floor_4_0_20000,       !- Surface Name",
        "    CombinedHeatAndMoistureFiniteElement;  !- Algorithm",

        "SurfaceProperty:HeatTransferAlgorithm,",
        "    Zone1_Floor_4_0_30000,       !- Surface Name",
        "    ConductionTransferFunction;  !- Algorithm",

        "Zone,",
        "    DATATELCOM,       !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    23.5824,                 !- Volume {m3}",
        "    6.7395,                  !- Floor Area {m2}",
        "    TARP;                    !- Zone Inside Convection Algorithm",
        "Zone,",
        "    ZONE1,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    999.6521,                !- Volume {m3}",
        "    454.1032,                !- Floor Area {m2}",
        "    TARP;                    !- Zone Inside Convection Algorithm",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                  // expect no errors

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosZoneRelNorth(2) = state->dataSurfaceGeometry->CosZoneRelNorth(1);
    state->dataSurfaceGeometry->SinZoneRelNorth(2) = state->dataSurfaceGeometry->SinZoneRelNorth(1);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data
    EXPECT_FALSE(ErrorsFound);           // expect no errors

    int surfNum = Util::FindItemInList("DATATELCOM_CEILING_1_0_0", state->dataSurface->Surface);
    EXPECT_ENUM_EQ(DataSurfaces::HeatTransferModel::CondFD, state->dataSurface->Surface(surfNum).HeatTransferAlgorithm);
    EXPECT_TRUE(state->dataHeatBal->AnyCondFD);

    surfNum = Util::FindItemInList("ZONE1_FLOOR_4_0_10000", state->dataSurface->Surface);
    EXPECT_ENUM_EQ(DataSurfaces::HeatTransferModel::CondFD, state->dataSurface->Surface(surfNum).HeatTransferAlgorithm);
    EXPECT_TRUE(state->dataHeatBal->AnyEMPD); // input as EMPD but then later overridden to CondFD - see error message below

    surfNum = Util::FindItemInList("ZONE1_FLOOR_4_0_20000", state->dataSurface->Surface);
    EXPECT_ENUM_EQ(DataSurfaces::HeatTransferModel::HAMT, state->dataSurface->Surface(surfNum).HeatTransferAlgorithm);
    EXPECT_TRUE(state->dataHeatBal->AnyHAMT);

    surfNum = Util::FindItemInList("ZONE1_FLOOR_4_0_30000", state->dataSurface->Surface);
    EXPECT_ENUM_EQ(DataSurfaces::HeatTransferModel::CTF, state->dataSurface->Surface(surfNum).HeatTransferAlgorithm);
    EXPECT_TRUE(state->dataHeatBal->AnyCTF);

    std::string const error_string = delimited_string({
        "   ** Warning ** GetSurfaceData: Entered Zone Floor Area(s) differ more than 5% from the sum of the Space Floor Area(s).",
        "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual zones.",
        "   ** Warning ** The moisture penetration depth conduction transfer function algorithm is used but the input file includes no "
        "MaterialProperty:MoisturePenetrationDepth:Settings objects.",
        "   ** Warning ** The combined heat and moisture finite element algorithm is used but the input file includes no "
        "MaterialProperty:HeatAndMoistureTransfer:* objects.",
        "   **   ~~~   ** Certain materials objects are necessary to achieve proper results with the heat transfer algorithm(s) selected.",
        "   ** Warning ** An interior surface is defined as two surfaces with reverse constructions. The HeatTransferAlgorithm in both constructions "
        "should be same.",
        "   **   ~~~   ** The HeatTransferAlgorithm of Surface: DATATELCOM_CEILING_1_0_0, is CondFD - ConductionFiniteDifference",
        "   **   ~~~   ** The HeatTransferAlgorithm of Surface: ZONE1_FLOOR_4_0_10000, is EMPD - MoisturePenetrationDepthConductionTransferFunction",
        "   **   ~~~   ** The HeatTransferAlgorithm of Surface: ZONE1_FLOOR_4_0_10000, is assigned to CondFD - ConductionFiniteDifference. "
        "Simulation continues.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Check heat balance surface lists
    // Remember that ZoneHTSurfaceList includes all HT surfaces in the zone PLUS any adjacent interzone surfaces - same for ZoneIZSurfaceList
    EXPECT_EQ(state->dataSurface->AllHTSurfaceList.size(), 4u);
    EXPECT_EQ(state->dataSurface->AllIZSurfaceList.size(), 2u);

    int zoneNum = Util::FindItemInList("DATATELCOM", state->dataHeatBal->Zone);
    EXPECT_EQ(state->dataHeatBal->Zone(zoneNum).ZoneHTSurfaceList.size(), 2u);
    EXPECT_EQ(state->dataHeatBal->Zone(zoneNum).ZoneIZSurfaceList.size(), 2u);

    zoneNum = Util::FindItemInList("ZONE1", state->dataHeatBal->Zone);
    EXPECT_EQ(state->dataHeatBal->Zone(zoneNum).ZoneHTSurfaceList.size(), 4u);
    EXPECT_EQ(state->dataHeatBal->Zone(zoneNum).ZoneIZSurfaceList.size(), 2u);
}

// Test for #7071: if a Surface references an outside boundary surface that cannot be found, we handle it gracefully with an error message
// instead of throwing an assert 'contains' error
TEST_F(EnergyPlusFixture, SurfaceGeometry_SurfaceReferencesNonExistingSurface)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Material,",
        "  8 in.Concrete Block Basement Wall,      !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2032,                                 !- Thickness{ m }",
        "  1.326,                                  !- Conductivity{ W / m - K }",
        "  1841.99999999999,                       !- Density{ kg / m3 }",
        "  911.999999999999,                       !- Specific Heat{ J / kg - K }",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",
        "",
        "Construction,",
        "   Typical,   !- Name",
        "   8 in.Concrete Block Basement Wall;     !- Layer 1",

        "BuildingSurface:Detailed,                                   ",
        "  Surface A,               !- Name",
        "  Wall,                    !- Surface Type",
        "  Typical,                 !- Construction Name",
        "  ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Surface,                 !- Outside Boundary Condition",
        "  Surface B,               !- Outside Boundary Condition Object", // Surface B doesn't exist!
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  ,                        !- View Factor to Ground",
        "  ,                        !- Number of Vertices",
        "  251.4600375, 3.5052, 0, !- X, Y, Z Vertex 1 {m}",
        "  251.4600375, 0, 0, !- X, Y, Z Vertex 2 {m}",
        "  249.9571375, 0, 0, !- X, Y, Z Vertex 3 {m}",
        "  248.5215375, 1.0, 0, !- X, Y, Z Vertex 4 {m}",
        "  248.5215375, 3.5052, 0;                 !- X, Y, Z Vertex 5 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Read Material and Construction, and expect no errors
    GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(2).Name = "ZONE 2";
    state->dataSurfaceGeometry->SurfaceTmp.allocate(1);
    int SurfNum = 0;
    int TotHTSurfs = 1;
    Array1D_string const BaseSurfCls(3, {"WALL", "FLOOR", "ROOF"});
    Array1D<DataSurfaces::SurfaceClass> const BaseSurfIDs(
        3, {DataSurfaces::SurfaceClass::Wall, DataSurfaces::SurfaceClass::Floor, DataSurfaces::SurfaceClass::Roof});
    int NeedToAddSurfaces;

    GetGeometryParameters(*state, ErrorsFound);
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;

    GetHTSurfaceData(*state, ErrorsFound, SurfNum, TotHTSurfs, 0, 0, 0, BaseSurfCls, BaseSurfIDs, NeedToAddSurfaces);

    // We expect one surface, but an error since Surface B cannot be located
    EXPECT_EQ(1, SurfNum);
    EXPECT_TRUE(ErrorsFound);

    std::string const error_string = delimited_string(
        {"   ** Severe  ** RoofCeiling:Detailed=\"SURFACE A\" references an outside boundary surface that cannot be found:SURFACE B"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_InternalMassSurfacesCount)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    G SW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G NW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    Office,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G NE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G N1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G N2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G S1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G S2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M SW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M NW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M SE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M NE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M N1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M N2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M S1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M S2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T SW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T NW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T SE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T NE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T N1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T N2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T S1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T S2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T Corridor,              !- Name",
        "    ,                        !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    7.61962839691078,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G Corridor,              !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    7.61962839691078,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M Corridor,              !- Name",
        "    ,                        !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    7.61962839691078,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Construction,",
        "    InteriorFurnishings,     !- Name",
        "    Std Wood 6inch;          !- Outside Layer",

        "  Material,",
        "    Std Wood 6inch,          !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.15,                    !- Thickness {m}",
        "    0.12,                    !- Conductivity {W/m-K}",
        "    540.0000,                !- Density {kg/m3}",
        "    1210,                    !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  InternalMass,",
        "    GFloorZonesIntMass,      !- Name",
        "    InteriorFurnishings,     !- Construction Name",
        "    IntMassZoneList_GF,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  ZoneList,",
        "    IntMassZoneList_GF,      !- Name",
        "    G SW Apartment,          !- Zone 1 Name",
        "    G NW Apartment,          !- Zone 2 Name",
        "    Office,                  !- Zone 3 Name",
        "    G NE Apartment,          !- Zone 4 Name",
        "    G N1 Apartment,          !- Zone 5 Name",
        "    G N2 Apartment,          !- Zone 6 Name",
        "    G S1 Apartment,          !- Zone 7 Name",
        "    G S2 Apartment;          !- Zone 8 Name",

        "  InternalMass,",
        "    MFloorZonesIntMass,      !- Name",
        "    InteriorFurnishings,     !- Construction Name",
        "    IntMassZoneList_MF,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    176.498545342438;        !- Surface Area {m2}",

        "  ZoneList,",
        "    IntMassZoneList_MF,      !- Name",
        "    M SW Apartment,          !- Zone 1 Name",
        "    M NW Apartment,          !- Zone 2 Name",
        "    M SE Apartment,          !- Zone 3 Name",
        "    M NE Apartment,          !- Zone 4 Name",
        "    M N1 Apartment,          !- Zone 5 Name",
        "    M N2 Apartment,          !- Zone 6 Name",
        "    M S1 Apartment,          !- Zone 7 Name",
        "    M S2 Apartment;          !- Zone 8 Name",

        "  InternalMass,",
        "    TFloorZonesIntMass,      !- Name",
        "    InteriorFurnishings,     !- Construction Name",
        "    IntMassZoneList_TF,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  ZoneList,",
        "    IntMassZoneList_TF,      !- Name",
        "    T SW Apartment,          !- Zone 1 Name",
        "    T NW Apartment,          !- Zone 2 Name",
        "    T SE Apartment,          !- Zone 3 Name",
        "    T NE Apartment,          !- Zone 4 Name",
        "    T N1 Apartment,          !- Zone 5 Name",
        "    T N2 Apartment,          !- Zone 6 Name",
        "    T S1 Apartment,          !- Zone 7 Name",
        "    T S2 Apartment;          !- Zone 8 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Read Materials
    GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    // Construction
    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    // Read Zones
    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    // Read InternalMass Object Count
    int TotIntMass = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "InternalMass");
    // check the three internal mass objects
    EXPECT_EQ(3, TotIntMass);

    // Read InternalMass Surfaces Count
    int TotalNumOfInternalMassSurfaces = GetNumIntMassSurfaces(*state);
    // check the 24 internal mass surfaces created from the three zoneLists
    EXPECT_EQ(24, TotalNumOfInternalMassSurfaces);
}
TEST_F(EnergyPlusFixture, SurfaceGeometry_CreateInternalMassSurfaces)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    G SW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G NW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    Office,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G NE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G N1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G N2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G S1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G S2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M SW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M NW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M SE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M NE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M N1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M N2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M S1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M S2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T SW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T NW Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T SE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T NE Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    34.7455054899131,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T N1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T N2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    9.29594664423115,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T S1 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    11.5818351633044,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T S2 Apartment,          !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    23.1636703266088,        !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    T Corridor,              !- Name",
        "    ,                        !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    7.61962839691078,        !- Y Origin {m}",
        "    9.14355407629293,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    G Corridor,              !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    7.61962839691078,        !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    M Corridor,              !- Name",
        "    ,                        !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    7.61962839691078,        !- Y Origin {m}",
        "    3.04785135876431,        !- Z Origin {m}",
        "    1,                       !- Type",
        "    1;                       !- Multiplier",

        "  Construction,",
        "    InteriorFurnishings,     !- Name",
        "    Std Wood 6inch;          !- Outside Layer",

        "  Material,",
        "    Std Wood 6inch,          !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.15,                    !- Thickness {m}",
        "    0.12,                    !- Conductivity {W/m-K}",
        "    540.0000,                !- Density {kg/m3}",
        "    1210,                    !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  InternalMass,",
        "    GFloorZonesIntMass,      !- Name",
        "    InteriorFurnishings,     !- Construction Name",
        "    IntMassZoneList_GF,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  ZoneList,",
        "    IntMassZoneList_GF,      !- Name",
        "    G SW Apartment,          !- Zone 1 Name",
        "    G NW Apartment,          !- Zone 2 Name",
        "    Office,                  !- Zone 3 Name",
        "    G NE Apartment,          !- Zone 4 Name",
        "    G N1 Apartment,          !- Zone 5 Name",
        "    G N2 Apartment,          !- Zone 6 Name",
        "    G S1 Apartment,          !- Zone 7 Name",
        "    G S2 Apartment;          !- Zone 8 Name",

        "  InternalMass,",
        "    MFloorZonesIntMass,      !- Name",
        "    InteriorFurnishings,     !- Construction Name",
        "    IntMassZoneList_MF,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    176.498545342438;        !- Surface Area {m2}",

        "  ZoneList,",
        "    IntMassZoneList_MF,      !- Name",
        "    M SW Apartment,          !- Zone 1 Name",
        "    M NW Apartment,          !- Zone 2 Name",
        "    M SE Apartment,          !- Zone 3 Name",
        "    M NE Apartment,          !- Zone 4 Name",
        "    M N1 Apartment,          !- Zone 5 Name",
        "    M N2 Apartment,          !- Zone 6 Name",
        "    M S1 Apartment,          !- Zone 7 Name",
        "    M S2 Apartment;          !- Zone 8 Name",

        "  InternalMass,",
        "    TFloorZonesIntMass,      !- Name",
        "    InteriorFurnishings,     !- Construction Name",
        "    IntMassZoneList_TF,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  ZoneList,",
        "    IntMassZoneList_TF,      !- Name",
        "    T SW Apartment,          !- Zone 1 Name",
        "    T NW Apartment,          !- Zone 2 Name",
        "    T SE Apartment,          !- Zone 3 Name",
        "    T NE Apartment,          !- Zone 4 Name",
        "    T N1 Apartment,          !- Zone 5 Name",
        "    T N2 Apartment,          !- Zone 6 Name",
        "    T S1 Apartment,          !- Zone 7 Name",
        "    T S2 Apartment;          !- Zone 8 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Read Materials
    GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    // Construction
    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    // Read Zones
    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    // Read InternalMass Object Count
    int TotIntMass = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "InternalMass");
    EXPECT_EQ(3, TotIntMass);

    // Read InternalMass Surfaces Count
    int TotalNumOfInternalMassSurfaces = GetNumIntMassSurfaces(*state);
    EXPECT_EQ(24, TotalNumOfInternalMassSurfaces);

    state->dataSurface->TotSurfaces = TotalNumOfInternalMassSurfaces;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);

    int SurfNum = 0;
    GetIntMassSurfaceData(*state, ErrorsFound, SurfNum);
    ASSERT_FALSE(ErrorsFound);

    // check internal mass surface count and object names
    EXPECT_EQ(8, state->dataSurface->IntMassObjects(1).NumOfZones);
    EXPECT_EQ("GFLOORZONESINTMASS", state->dataSurface->IntMassObjects(1).Name);
    EXPECT_EQ(8, state->dataSurface->IntMassObjects(2).NumOfZones);
    EXPECT_EQ("MFLOORZONESINTMASS", state->dataSurface->IntMassObjects(2).Name);
    EXPECT_EQ(8, state->dataSurface->IntMassObjects(3).NumOfZones);
    EXPECT_EQ("TFLOORZONESINTMASS", state->dataSurface->IntMassObjects(3).Name);
    // check total count of internal surfaces created
    EXPECT_EQ(24, state->dataSurface->TotSurfaces);

    // check unique internal surface name created created from a combination
    // of zone name and internal mass object name represented in the zone
    // first zone in the ground floor ZoneList
    EXPECT_EQ("G SW APARTMENT", state->dataHeatBal->Zone(1).Name);
    EXPECT_EQ("GFLOORZONESINTMASS", state->dataSurface->IntMassObjects(1).Name);
    EXPECT_EQ("G SW APARTMENT GFLOORZONESINTMASS", state->dataSurfaceGeometry->SurfaceTmp(1).Name);
    // first zone in the middle floor ZoneList
    EXPECT_EQ("M SW APARTMENT", state->dataHeatBal->Zone(9).Name);
    EXPECT_EQ("MFLOORZONESINTMASS", state->dataSurface->IntMassObjects(2).Name);
    EXPECT_EQ("M SW APARTMENT MFLOORZONESINTMASS", state->dataSurfaceGeometry->SurfaceTmp(9).Name);
    // first zone in the top floor ZoneList
    EXPECT_EQ("T SW APARTMENT", state->dataHeatBal->Zone(17).Name);
    EXPECT_EQ("TFLOORZONESINTMASS", state->dataSurface->IntMassObjects(3).Name);
    EXPECT_EQ("T SW APARTMENT TFLOORZONESINTMASS", state->dataSurfaceGeometry->SurfaceTmp(17).Name);
}

TEST_F(EnergyPlusFixture, WorldCoord_with_RelativeRectSurfCoord_test1)
{
    // Case 1) NOT world coordinate system (Relative) - No error

    std::string const idf_objects = delimited_string({

        "GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    Relative,                !- Coordinate System",
        "    Relative,                !- Daylighting Reference Point Coordinate System",
        "    Relative;                !- Rectangular Surface Coordinate System",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(1).OriginX = 0;
    state->dataHeatBal->Zone(1).OriginY = 0;
    state->dataHeatBal->Zone(1).OriginZ = 0;

    GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(has_err_output(true));
}

TEST_F(EnergyPlusFixture, WorldCoord_with_RelativeRectSurfCoord_test2)
{
    // Case 2) World coordinate system & All zero zone origins - No error

    std::string const idf_objects = delimited_string({

        "GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World,                   !- Coordinate System",
        "    Relative,                !- Daylighting Reference Point Coordinate System",
        "    Relative;                !- Rectangular Surface Coordinate System",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(1).OriginX = 0;
    state->dataHeatBal->Zone(1).OriginY = 0;
    state->dataHeatBal->Zone(1).OriginZ = 0;

    GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(has_err_output(true));
}

TEST_F(EnergyPlusFixture, WorldCoord_with_RelativeRectSurfCoord_test3)
{
    // Case 3) World coordinate system & Relative Rect. surf, coordinate system & Non-zero zone origin

    std::string const idf_objects = delimited_string({

        "GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World,                   !- Coordinate System",
        "    Relative,                !- Daylighting Reference Point Coordinate System",
        "    Relative;                !- Rectangular Surface Coordinate System",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(1).OriginX = 6;
    state->dataHeatBal->Zone(1).OriginY = 6;
    state->dataHeatBal->Zone(1).OriginZ = 0;

    GetGeometryParameters(*state, ErrorsFound);
    EXPECT_TRUE(has_err_output(false));

    std::string error_string = delimited_string({
        "   ** Warning ** GlobalGeometryRules: Potential mismatch of coordinate specifications. Note that the rectangular surfaces are relying on "
        "the default SurfaceGeometry for 'Relative to zone' coordinate.",
        "   **   ~~~   ** Coordinate System=\"WORLD\"; while ",
        "   **   ~~~   ** Rectangular Surface Coordinate System=\"RELATIVE\".",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, WorldCoord_with_RelativeRectSurfCoord_test4)
{
    // Case 4) World coordinate system & Default Rect. surf, coordinate system & Non-zero zone origin

    std::string const idf_objects = delimited_string({

        "GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World,                   !- Coordinate System",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(1).OriginX = 6;
    state->dataHeatBal->Zone(1).OriginY = 6;
    state->dataHeatBal->Zone(1).OriginZ = 0;

    GetGeometryParameters(*state, ErrorsFound);
    EXPECT_TRUE(has_err_output(false));

    std::string error_string = delimited_string({
        "   ** Warning ** GlobalGeometryRules: Potential mismatch of coordinate specifications. Note that the rectangular surfaces are relying on "
        "the default SurfaceGeometry for 'Relative to zone' coordinate.",
        "   **   ~~~   ** Coordinate System=\"WORLD\"; while ",
        "   **   ~~~   ** Rectangular Surface Coordinate System=\"RELATIVE\".",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_CheckForReversedLayers)
{
    auto &s_mat = state->dataMaterial;
    bool RevLayerDiffs;
    state->dataConstruction->Construct.allocate(6);

    // Case 1a: Constructs with regular materials are a reverse of each other--material layers match in reverse (should get a "false" answer)
    state->dataConstruction->Construct(1).TotLayers = 3;
    state->dataConstruction->Construct(1).LayerPoint(1) = 1;
    state->dataConstruction->Construct(1).LayerPoint(2) = 2;
    state->dataConstruction->Construct(1).LayerPoint(3) = 3;
    state->dataConstruction->Construct(2).TotLayers = 3;
    state->dataConstruction->Construct(2).LayerPoint(1) = 3;
    state->dataConstruction->Construct(2).LayerPoint(2) = 2;
    state->dataConstruction->Construct(2).LayerPoint(3) = 1;
    RevLayerDiffs = true;
    // ExpectResult = false;
    CheckForReversedLayers(*state, RevLayerDiffs, 1, 2, 3);
    EXPECT_FALSE(RevLayerDiffs);

    // Case 1a: Constructs with regular materials are not reverse of each other--material layers do not match in reverse (should get a "true" answer)
    state->dataConstruction->Construct(2).LayerPoint(1) = 1;
    state->dataConstruction->Construct(2).LayerPoint(3) = 3;

    auto *mat1 = new Material::MaterialBase;
    mat1->group = Material::Group::Regular;
    s_mat->materials.push_back(mat1);

    auto *mat2 = new Material::MaterialBase;
    mat2->group = Material::Group::Regular;
    s_mat->materials.push_back(mat2);

    auto *mat3 = new Material::MaterialBase;
    mat3->group = Material::Group::Regular;
    s_mat->materials.push_back(mat3);

    RevLayerDiffs = false;
    // ExpectResult = true;
    CheckForReversedLayers(*state, RevLayerDiffs, 1, 2, 3);
    EXPECT_TRUE(RevLayerDiffs);

    // Case 2a: Constructs are reverse of each other using WindowGlass, front/back properties properly switched (should get a "false" answer)
    state->dataConstruction->Construct(3).TotLayers = 3;
    state->dataConstruction->Construct(3).LayerPoint(1) = 4;
    state->dataConstruction->Construct(3).LayerPoint(2) = 2;
    state->dataConstruction->Construct(3).LayerPoint(3) = 5;
    state->dataConstruction->Construct(4).TotLayers = 3;
    state->dataConstruction->Construct(4).LayerPoint(1) = 4;
    state->dataConstruction->Construct(4).LayerPoint(2) = 2;
    state->dataConstruction->Construct(4).LayerPoint(3) = 5;

    auto *mat4 = new Material::MaterialGlass;
    mat4->group = Material::Group::Glass;
    s_mat->materials.push_back(mat4);

    mat4->Thickness = 0.15;
    mat4->ReflectSolBeamFront = 0.35;
    mat4->ReflectSolBeamBack = 0.25;
    mat4->TransVis = 0.45;
    mat4->ReflectVisBeamFront = 0.34;
    mat4->ReflectVisBeamBack = 0.24;
    mat4->TransThermal = 0.44;
    mat4->AbsorpThermalFront = 0.33;
    mat4->AbsorpThermalBack = 0.23;
    mat4->Conductivity = 0.43;
    mat4->GlassTransDirtFactor = 0.67;
    mat4->SolarDiffusing = true;
    mat4->YoungModulus = 0.89;
    mat4->PoissonsRatio = 1.11;

    auto *mat5 = new Material::MaterialGlass;
    mat5->group = Material::Group::Glass;
    s_mat->materials.push_back(mat5);

    mat5->Thickness = 0.15;
    mat5->ReflectSolBeamFront = 0.25;
    mat5->ReflectSolBeamBack = 0.35;
    mat5->TransVis = 0.45;
    mat5->ReflectVisBeamFront = 0.24;
    mat5->ReflectVisBeamBack = 0.34;
    mat5->TransThermal = 0.44;
    mat5->AbsorpThermalFront = 0.23;
    mat5->AbsorpThermalBack = 0.33;
    mat5->Conductivity = 0.43;
    mat5->GlassTransDirtFactor = 0.67;
    mat5->SolarDiffusing = true;
    mat5->YoungModulus = 0.89;
    mat5->PoissonsRatio = 1.11;
    RevLayerDiffs = true;
    // ExpectResult = false;
    CheckForReversedLayers(*state, RevLayerDiffs, 3, 4, 3);
    EXPECT_FALSE(RevLayerDiffs);

    // Case 2b: Constructs are reverse of each other using WindowGlass, front/back properties NOT properly switched (should get a "true" answer)
    mat5->ReflectVisBeamFront = 0.34; // correct would be 0.24
    mat5->ReflectVisBeamBack = 0.24;  // correct would be 0.34
    RevLayerDiffs = false;
    // ExpectResult = true;
    CheckForReversedLayers(*state, RevLayerDiffs, 3, 4, 3);
    EXPECT_TRUE(RevLayerDiffs);

    // Case 3a: Single layer constructs using Equivalent Glass, front/back properties properly switched (should get a "false" answer)
    state->dataConstruction->Construct(5).TotLayers = 1;
    state->dataConstruction->Construct(5).LayerPoint(1) = 6;
    state->dataConstruction->Construct(6).TotLayers = 1;
    state->dataConstruction->Construct(6).LayerPoint(1) = 7;

    auto *mat6 = new Material::MaterialGlassEQL;
    mat6->group = Material::Group::GlassEQL;
    s_mat->materials.push_back(mat6);
    mat6->TAR.Sol.Ft.Bm[0].BmTra = 0.39;
    mat6->TAR.Sol.Bk.Bm[0].BmTra = 0.29;
    mat6->TAR.Sol.Ft.Bm[0].BmRef = 0.38;
    mat6->TAR.Sol.Bk.Bm[0].BmRef = 0.28;
    mat6->TAR.Vis.Ft.Bm[0].BmTra = 0.37;
    mat6->TAR.Vis.Bk.Bm[0].BmTra = 0.27;
    mat6->TAR.Vis.Ft.Bm[0].BmRef = 0.36;
    mat6->TAR.Vis.Bk.Bm[0].BmRef = 0.26;
    mat6->TAR.Sol.Ft.Bm[0].DfTra = 0.35;
    mat6->TAR.Sol.Bk.Bm[0].DfTra = 0.25;
    mat6->TAR.Sol.Ft.Bm[0].DfRef = 0.34;
    mat6->TAR.Sol.Bk.Bm[0].DfRef = 0.24;
    mat6->TAR.Vis.Ft.Bm[0].DfTra = 0.33;
    mat6->TAR.Vis.Bk.Bm[0].DfTra = 0.23;
    mat6->TAR.Vis.Ft.Bm[0].DfRef = 0.32;
    mat6->TAR.Vis.Bk.Bm[0].DfRef = 0.22;
    mat6->TAR.Sol.Ft.Df.Tra = 0.456;
    mat6->TAR.Sol.Ft.Df.Ref = 0.31;
    mat6->TAR.Sol.Bk.Df.Ref = 0.21;
    mat6->TAR.Vis.Ft.Df.Tra = 0.345;
    mat6->TAR.Vis.Ft.Df.Ref = 0.30;
    mat6->TAR.Vis.Bk.Df.Ref = 0.20;
    mat6->TAR.IR.Ft.Tra = 0.234;
    mat6->TAR.IR.Ft.Emi = 0.888;
    mat6->TAR.IR.Bk.Emi = 0.777;
    mat6->Resistance = 1.234;

    auto *mat7 = new Material::MaterialGlassEQL;
    mat7->group = Material::Group::GlassEQL;
    s_mat->materials.push_back(mat7);

    mat7->TAR.Sol.Ft.Bm[0].BmTra = 0.29;
    mat7->TAR.Sol.Bk.Bm[0].BmTra = 0.39;
    mat7->TAR.Sol.Ft.Bm[0].BmRef = 0.28;
    mat7->TAR.Sol.Bk.Bm[0].BmRef = 0.38;
    mat7->TAR.Vis.Ft.Bm[0].BmTra = 0.27;
    mat7->TAR.Vis.Bk.Bm[0].BmTra = 0.37;
    mat7->TAR.Vis.Ft.Bm[0].BmRef = 0.26;
    mat7->TAR.Vis.Bk.Bm[0].BmRef = 0.36;
    mat7->TAR.Sol.Ft.Bm[0].DfTra = 0.25;
    mat7->TAR.Sol.Bk.Bm[0].DfTra = 0.35;
    mat7->TAR.Sol.Ft.Bm[0].DfRef = 0.24;
    mat7->TAR.Sol.Bk.Bm[0].DfRef = 0.34;
    mat7->TAR.Vis.Ft.Bm[0].DfTra = 0.23;
    mat7->TAR.Vis.Bk.Bm[0].DfTra = 0.33;
    mat7->TAR.Vis.Ft.Bm[0].DfRef = 0.22;
    mat7->TAR.Vis.Bk.Bm[0].DfRef = 0.32;
    mat7->TAR.Sol.Ft.Df.Tra = 0.456;
    mat7->TAR.Sol.Ft.Df.Ref = 0.21;
    mat7->TAR.Sol.Bk.Df.Ref = 0.31;
    mat7->TAR.Vis.Ft.Df.Tra = 0.345;
    mat7->TAR.Vis.Ft.Df.Ref = 0.20;
    mat7->TAR.Vis.Bk.Df.Ref = 0.30;
    mat7->TAR.IR.Ft.Tra = 0.234;
    mat7->TAR.IR.Ft.Emi = 0.777;
    mat7->TAR.IR.Bk.Emi = 0.888;
    mat7->Resistance = 1.234;
    RevLayerDiffs = true;
    // ExpectResult = false;
    CheckForReversedLayers(*state, RevLayerDiffs, 5, 6, 1);
    EXPECT_FALSE(RevLayerDiffs);

    // Case 3a: Single layer constructs using Equivalent Glass, front/back properties NOT properly switched (should get a "true" answer)
    mat7->TAR.IR.Ft.Emi = 0.888;
    RevLayerDiffs = false;
    // ExpectResult = true;
    CheckForReversedLayers(*state, RevLayerDiffs, 5, 6, 1);
    EXPECT_TRUE(RevLayerDiffs);
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_SetupEnclosuresNoAirBoundaries)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Space,",
        "Space 1,             !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 2,             !- Name",
        "Zone 2;             !- Zone Name",

        "Space,",
        "Space 3,             !- Name",
        "Zone 3;             !- Zone Name",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 3);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).Name, "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).Name, "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(3).Name, "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[0], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(3).spaceNames[0], "Space 3"));
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 3);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 3);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).Name, "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).Name, "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(3).Name, "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[0], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(3).spaceNames[0], "Space 3"));
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).solarEnclosureNum, 3);
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_SetupEnclosuresWithAirBoundaries1)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Space,",
        "Space 1,             !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 2,             !- Name",
        "Zone 2;             !- Zone Name",

        "Space,",
        "Space 3,             !- Name",
        "Zone 3;             !- Zone Name",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",
        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone2-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone3-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 1);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).Name, "Radiant Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[1], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[2], "Space 3"));
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 1);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 1);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).Name, "Solar Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[1], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[2], "Space 3"));
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(3).solarEnclosureNum, 1);
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_SetupEnclosuresWithAirBoundaries2)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Space,",
        "Space 1,             !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 2,             !- Name",
        "Zone 2;             !- Zone Name",

        "Space,",
        "Space 3,             !- Name",
        "Zone 3;             !- Zone Name",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",

        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone2-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone3-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors
    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    HeatBalanceIntRadExchange::InitInteriorRadExchange(*state);

    ErrorsFound = false;

    // std::string const error_string = delimited_string({
    //"   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Zone 6\" did not find a matching radiant or solar
    // enclosure name."
    //    });
    // EXPECT_TRUE(compare_err_stream(error_string, true));

    // For this test case, Zones 1 and 3 share radiant and solar enclosures

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 2);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).Name, "Radiant Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[1], "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).Name, "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[0], "Space 2"));
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 1);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 2);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).Name, "Solar Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[1], "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).Name, "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[0], "Space 2"));
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).solarEnclosureNum, 1);

    // Check surface order
    int Zone1Surface1 = Util::FindItemInList(Util::makeUPPER("Zone1-Surface1"), state->dataSurface->Surface);
    int Zone1Surface2 = Util::FindItemInList(Util::makeUPPER("Zone1-Surface2"), state->dataSurface->Surface);
    int Zone2Surface1 = Util::FindItemInList(Util::makeUPPER("Zone2-Surface1"), state->dataSurface->Surface);
    int Zone3Surface1 = Util::FindItemInList(Util::makeUPPER("Zone3-Surface1"), state->dataSurface->Surface);
    int Zone1Floor = Util::FindItemInList(Util::makeUPPER("Zone1-Floor"), state->dataSurface->Surface);
    int Zone2Floor = Util::FindItemInList(Util::makeUPPER("Zone2-Floor"), state->dataSurface->Surface);
    int Zone3Floor = Util::FindItemInList(Util::makeUPPER("Zone3-Floor"), state->dataSurface->Surface);

    EXPECT_EQ(state->dataHeatBal->Zone(1).AllSurfaceFirst, Zone1Surface2);     // air boundary surface
    EXPECT_EQ(state->dataHeatBal->Zone(1).AllSurfaceFirst + 1, Zone1Surface1); // air boundary surface
    EXPECT_EQ(state->dataHeatBal->Zone(2).AllSurfaceFirst, Zone2Surface1);     // no air boundary surfaces in Zone 2
    EXPECT_EQ(state->dataHeatBal->Zone(3).AllSurfaceFirst, Zone3Surface1);     // air boundary surface
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceFirst, Zone1Surface1);     // first non-air boundary surface
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceFirst, Zone2Surface1);     // first non-air boundary surface
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceFirst, Zone3Floor);        // first non-air boundary surface
    EXPECT_EQ(state->dataHeatBal->Zone(1).AllSurfaceLast, Zone1Floor);
    EXPECT_EQ(state->dataHeatBal->Zone(2).AllSurfaceLast, Zone2Floor);
    EXPECT_EQ(state->dataHeatBal->Zone(3).AllSurfaceLast, Zone3Floor);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceLast, Zone1Floor);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceLast, Zone2Floor);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceLast, Zone3Floor);

    // Check MRT calculations
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(3);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(3);
    state->dataHeatBalSurf->SurfTempIn.allocate(7);
    auto &zoneHB1 = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1);
    auto &zoneHB2 = state->dataZoneTempPredictorCorrector->zoneHeatBalance(2);
    auto &zoneHB3 = state->dataZoneTempPredictorCorrector->zoneHeatBalance(3);
    auto &spaceHB1 = state->dataZoneTempPredictorCorrector->spaceHeatBalance(1);
    auto &spaceHB2 = state->dataZoneTempPredictorCorrector->spaceHeatBalance(2);
    auto &spaceHB3 = state->dataZoneTempPredictorCorrector->spaceHeatBalance(3);
    auto &encl1 = state->dataViewFactor->EnclRadInfo(1);
    auto &encl2 = state->dataViewFactor->EnclRadInfo(2);

    // Case 1 - all surfaces the same temperature
    state->dataHeatBalSurf->SurfTempIn(Zone1Surface1) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(Zone2Surface1) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(Zone1Floor) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(Zone2Floor) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(Zone3Floor) = 10.0;

    HeatBalanceSurfaceManager::CalculateZoneMRT(*state);
    EXPECT_NEAR(zoneHB1.MRT, 10.0, 0.001);
    EXPECT_NEAR(zoneHB2.MRT, 10.0, 0.001);
    EXPECT_NEAR(zoneHB3.MRT, 10.0, 0.001);
    EXPECT_NEAR(encl1.MRT, 10.0, 0.001);
    EXPECT_NEAR(encl2.MRT, 10.0, 0.001);
    EXPECT_EQ(spaceHB1.MRT, encl1.MRT);
    EXPECT_EQ(spaceHB2.MRT, encl2.MRT);
    EXPECT_EQ(spaceHB3.MRT, encl1.MRT);

    // Case 2 - all surfaces in each zone same temperature
    state->dataHeatBalSurf->SurfTempIn(Zone1Surface1) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(Zone2Surface1) = 20.0;
    state->dataHeatBalSurf->SurfTempIn(Zone1Floor) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(Zone2Floor) = 20.0;
    state->dataHeatBalSurf->SurfTempIn(Zone3Floor) = 30.0;

    HeatBalanceSurfaceManager::CalculateZoneMRT(*state);
    EXPECT_NEAR(zoneHB1.MRT, 10.0, 0.001);
    EXPECT_NEAR(zoneHB2.MRT, 20.0, 0.001);
    EXPECT_NEAR(zoneHB3.MRT, 30.0, 0.001);
    EXPECT_NEAR(encl1.MRT, 16.667, 0.001);
    EXPECT_NEAR(encl2.MRT, 20.0, 0.001);
    EXPECT_EQ(spaceHB1.MRT, encl1.MRT);
    EXPECT_EQ(spaceHB2.MRT, encl2.MRT);
    EXPECT_EQ(spaceHB3.MRT, encl1.MRT);

    // Case 3 - surface AEs sum to zero - so revert to MATs
    state->dataSurface->Surface(Zone1Surface1).Area = 0.0;
    state->dataSurface->Surface(Zone2Surface1).Area = 0.0;
    state->dataSurface->Surface(Zone1Floor).Area = 0.0;
    state->dataSurface->Surface(Zone2Floor).Area = 0.0;
    state->dataSurface->Surface(Zone3Floor).Area = 0.0;

    spaceHB1.MAT = 15.0;
    spaceHB2.MAT = 22.0;
    spaceHB3.MAT = 28.0;
    zoneHB1.MAT = 20.0;
    zoneHB2.MAT = 25.0;
    zoneHB3.MAT = 28.0;
    state->dataHeatBalSurfMgr->CalculateZoneMRTfirstTime = true;
    HeatBalanceSurfaceManager::CalculateZoneMRT(*state);
    EXPECT_NEAR(zoneHB1.MRT, 20.0, 0.001);
    EXPECT_NEAR(zoneHB2.MRT, 25.0, 0.001);
    EXPECT_NEAR(zoneHB3.MRT, 28.0, 0.001);
    EXPECT_NEAR(encl1.MRT, 21.5, 0.001);
    EXPECT_NEAR(encl2.MRT, 22.0, 0.001);
    EXPECT_EQ(spaceHB1.MRT, encl1.MRT);
    EXPECT_EQ(spaceHB2.MRT, encl2.MRT);
    EXPECT_EQ(spaceHB3.MRT, encl1.MRT);
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_SetupEnclosuresWithAirBoundaries3)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Zone,",
        "Zone 4;             !- Name",

        "Zone,",
        "Zone 5;             !- Name",

        "Space,",
        "Space 1,             !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 2,             !- Name",
        "Zone 2;             !- Zone Name",

        "Space,",
        "Space 3,             !- Name",
        "Zone 3;             !- Zone Name",

        "Space,",
        "Space 4,             !- Name",
        "Zone 4;             !- Zone Name",

        "Space,",
        "Space 5,             !- Name",
        "Zone 5;             !- Zone Name",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",
        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone4-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone4-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 4,       !- Zone Name",
        "    Space 4,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone4-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 4,       !- Zone Name",
        "    Space 4,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone5-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone5-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 5,       !- Zone Name",
        "    Space 5,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone4-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface3,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone5-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone5-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 5,       !- Zone Name",
        "    Space 5,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface3,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone2-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone3-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone4-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 4,       !- Zone Name",
        "    Space 4,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone5-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 5,       !- Zone Name",
        "    Space 5,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors

    // For this test case, there are air boundaries
    // Between Zones 1 and 2
    // None between Zones 2 and 3
    // Between Zones 3 and 4
    // Between Zones 4 and 5
    // Between Zones 1 and 5
    // This should trigger the enclosure merging and all five zones should share a radiant and solar enclosure

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 1);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).Name, "Radiant Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[1], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[2], "Space 5"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[3], "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[4], "Space 4"));
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(4).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(5).radiantEnclosureNum, 1);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 1);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).Name, "Solar Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[1], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[2], "Space 5"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[3], "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[4], "Space 4"));
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(3).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(4).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(5).solarEnclosureNum, 1);
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_SetupEnclosuresWithAirBoundaries4)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Zone,",
        "Zone 4;             !- Name",

        "Zone,",
        "Zone 5;             !- Name",

        "Zone,",
        "Zone 6;             !- Name",

        "Zone,",
        "Zone 7;             !- Name",

        "Zone,",
        "Zone 8;             !- Name",

        "Zone,",
        "Zone 9;             !- Name",

        "Zone,",
        "Zone 10;             !- Name",

        "Space,",
        "Space 1,             !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 2,             !- Name",
        "Zone 2;             !- Zone Name",

        "Space,",
        "Space 3,             !- Name",
        "Zone 3;             !- Zone Name",

        "Space,",
        "Space 4,             !- Name",
        "Zone 4;             !- Zone Name",

        "Space,",
        "Space 5,             !- Name",
        "Zone 5;             !- Zone Name",

        "Space,",
        "Space 6,             !- Name",
        "Zone 6;             !- Zone Name",

        "Space,",
        "Space 7,             !- Name",
        "Zone 7;             !- Zone Name",

        "Space,",
        "Space 8,             !- Name",
        "Zone 8;             !- Zone Name",

        "Space,",
        "Space 9,             !- Name",
        "Zone 9;             !- Zone Name",

        "Space,",
        "Space 10,             !- Name",
        "Zone 10;             !- Zone Name",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",
        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone4-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone4-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone2-Surface3,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone9-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone5-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone4-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 4,       !- Zone Name",
        "    Space 4,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone5-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 5,       !- Zone Name",
        "    Space 5,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone6-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 6,       !- Zone Name",
        "    Space 6,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone7-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone7-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 7,       !- Zone Name",
        "    Space 7,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone6-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone7-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 7,       !- Zone Name",
        "    Space 7,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone10-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone9-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 9,       !- Zone Name",
        "    Space 9,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone2-Surface3,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone9-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 9,       !- Zone Name",
        "    Space 9,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone10-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone10-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 10,       !- Zone Name",
        "    Space 10,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone9-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone2-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone3-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone4-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 4,       !- Zone Name",
        "    Space 4,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone5-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 5,       !- Zone Name",
        "    Space 5,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone6-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 6,       !- Zone Name",
        "    Space 6,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone7-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 7,       !- Zone Name",
        "    Space 7,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone8-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 8,       !- Zone Name",
        "    Space 8,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone9-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 9,       !- Zone Name",
        "    Space 9,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone10-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 10,       !- Zone Name",
        "    Space 10,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors

    // This test is designed to reproduce issue #9218 Crash with many airwalls collapsing into a single enclosure.
    // That proved difficult to reproduce, however the root cause of the crash is space enclosure nums that are not consistent
    // with the enclosures that hold them. This test demonstrates that failure.
    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 3);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).Name, "Radiant Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[0], "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[1], "Space 5"));

    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).Name, "Radiant Enclosure 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[0], "Space 6"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[1], "Space 7"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[2], "Space 10"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[3], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[4], "Space 4"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[5], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[6], "Space 9"));

    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(3).Name, "Zone 8"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(3).spaceNames[0], "Space 8"));

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 3);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).Name, "Solar Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[0], "Space 3"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[1], "Space 5"));

    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).Name, "Solar Enclosure 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[0], "Space 6"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[1], "Space 7"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[2], "Space 10"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[3], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[4], "Space 4"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[5], "Space 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[6], "Space 9"));

    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(3).Name, "Zone 8"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(3).spaceNames[0], "Space 8"));

    // Loop through all spaces on all enclosures and check that space radiantEnlosureNum matches the enclosure number
    // Before the fix, Space 3 and Space 5 have incorrect radiantEnclosureNum = 2 (should be 1)
    // And all of the spaces in enclousre 2 have incorrect radiantEnclosureNum = 3
    for (int enclNum = 1; enclNum <= state->dataViewFactor->NumOfRadiantEnclosures; ++enclNum) {
        for (int spaceNum : state->dataViewFactor->EnclRadInfo(enclNum).spaceNums) {
            EXPECT_EQ(state->dataHeatBal->space(spaceNum).radiantEnclosureNum, enclNum);
        }
    }

    // Check solar enclosures
    for (int enclNum = 1; enclNum <= state->dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        for (int spaceNum : state->dataViewFactor->EnclSolInfo(enclNum).spaceNums) {
            EXPECT_EQ(state->dataHeatBal->space(spaceNum).solarEnclosureNum, enclNum);
        }
    }
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_SetupEnclosuresWithAirBoundaries5_SameSpace)
{

    // Test for issue #9233 Enclosure area incorrect when air boundary surface has the same space on both sides
    // If a Construction:AirBoundary surface has the same Space on both sides, the enclosure space list will include that space twice, and the floor
    // area and other attributes of the enclosure will double-count that Space.

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Space,",
        "Space 1,             !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 2,             !- Name",
        "Zone 2;             !- Zone Name",

        "Space,",
        "Space 3,             !- Name",
        "Zone 3;             !- Zone Name",

        "  Construction,",
        "    Dbl Clr 3mm/6mm Air,     !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 6MM,                 !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    AIR,                     !- Gas Type",
        "    0.006;                   !- Thickness {m}",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",
        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface1b,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface1b,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Surface2,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone3-Surface1,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone3-Surface1,  !- Name",
        "    Wall,                 !- Surface Type",
        "    Grouped Air Boundary,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone1-Surface2,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 1",
        "    0,1,1,              !- Vertex 1",
        "    0,0,1;              !- Vertex 1",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    SunExposed,                   !- Sun Exposure",
        "    WindExposed,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "FenestrationSurface:Detailed,",
        "WF - 1,            !-Name ",
        " WINDOW, !-Surface Type",
        " Dbl Clr 3mm/6mm Air,                !-Construction Name ",
        "Zone1-Floor, !-Building Surface Name",
        ",        !-Outside Boundary Condition Object ",
        "autocalculate,                 !- View Factor to Ground",
        ", !-Frame and Divider Name",
        "1,                       !- Multiplier",
        "4,                       !- Number of Vertices",
        "    0.1,0.1,0,              !- Vertex 1",
        "    0.1,0.9,0,              !- Vertex 2",
        "    0.9,0.9,0,              !- Vertex 3",
        "    0.9,0.1,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor2,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    SunExposed,                   !- Sun Exposure",
        "    WindExposed,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone2-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone3-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 2);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).Name, "Radiant Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(1).spaceNames[1], "Space 3"));
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 1);
    Real64 enclArea = state->dataHeatBal->space(1).FloorArea + state->dataHeatBal->space(3).FloorArea;
    EXPECT_EQ(state->dataViewFactor->EnclRadInfo(1).FloorArea, enclArea);

    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).Name, "Zone 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclRadInfo(2).spaceNames[0], "Space 2"));
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 2);
    enclArea = state->dataHeatBal->space(2).FloorArea;
    EXPECT_EQ(state->dataViewFactor->EnclRadInfo(2).FloorArea, enclArea);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 2);
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).Name, "Solar Enclosure 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[0], "Space 1"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(1).spaceNames[1], "Space 3"));
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(3).solarEnclosureNum, 1);
    enclArea = state->dataHeatBal->space(1).FloorArea + state->dataHeatBal->space(3).FloorArea;
    Real64 enclExtWindowArea = state->dataHeatBal->space(1).extWindowArea + state->dataHeatBal->space(3).extWindowArea;
    EXPECT_EQ(state->dataViewFactor->EnclSolInfo(1).ExtWindowArea, enclExtWindowArea);
    Real64 enclTotSurfArea = state->dataHeatBal->space(1).totalSurfArea + state->dataHeatBal->space(3).totalSurfArea;
    EXPECT_EQ(state->dataViewFactor->EnclSolInfo(1).TotalSurfArea, enclTotSurfArea);

    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).Name, "Zone 2"));
    EXPECT_TRUE(Util::SameString(state->dataViewFactor->EnclSolInfo(2).spaceNames[0], "Space 2"));
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 2);
    enclArea = state->dataHeatBal->space(2).FloorArea;
    EXPECT_EQ(state->dataViewFactor->EnclSolInfo(2).FloorArea, enclArea);
    enclExtWindowArea = state->dataHeatBal->space(2).extWindowArea;
    EXPECT_EQ(state->dataViewFactor->EnclSolInfo(2).ExtWindowArea, enclExtWindowArea);
    enclTotSurfArea = state->dataHeatBal->space(2).totalSurfArea;
    EXPECT_EQ(state->dataViewFactor->EnclSolInfo(2).TotalSurfArea, enclTotSurfArea);
}

TEST_F(EnergyPlusFixture, TwoZones_With_AirDoor)
{
    // Two 10x10m zones. A door between both that has a Construction:AirBoundary assigned (its base surface is a regular interior wall)
    std::string const idf_objects = delimited_string({

        "Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "Material,",
        "  8IN Concrete HW,                        !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2033,                                 !- Thickness {m}",
        "  1.72959999999999,                       !- Conductivity {W/m-K}",
        "  2242.99999999999,                       !- Density {kg/m3}",
        "  836.999999999999,                       !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.65,                                   !- Solar Absorptance",
        "  0.65;                                   !- Visible Absorptance",

        "Construction,",
        "  Regular Construction,                   !- Name",
        "  8IN Concrete HW;                        !- Layer 1",

        "Construction:AirBoundary,",
        "  Air Wall,                               !- Name",
        "  SimpleMixing,                           !- Air Exchange Method",
        "  0.5,                                    !- Simple Mixing Air Changes per Hour {1/hr}",
        "  ;                                       !- Simple Mixing Schedule Name",

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

        "BuildingSurface:Detailed,",
        "  Space1-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.4384;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Surface,                                !- Outside Boundary Condition",
        "  Space2-InteriorWall,                    !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorDoor,                    !- Name",
        "  Door,                                   !- Surface Type",
        "  Air Wall,                               !- Construction Name",
        "  Space1-InteriorWall,                    !- Building Surface Name",
        "  Space2-InteriorDoor,                    !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-West,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 0;                                !- X,Y,Z Vertex 4 {m}",

        "Zone,",
        "  Zone2,                                  !- Name",
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

        "BuildingSurface:Detailed,",
        "  Space2-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Surface,                                !- Outside Boundary Condition",
        "  Space1-InteriorWall,                    !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space2-InteriorDoor,                    !- Name",
        "  Door,                                   !- Surface Type",
        "  Air Wall,                               !- Construction Name",
        "  Space2-InteriorWall,                    !- Building Surface Name",
        "  Space1-InteriorDoor,                    !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.95, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.05, 2;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-East,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    // This is ill-named, but it will shut the "No Ground Temperature" warning when set to false
    state->dataSurfaceGeometry->NoGroundTempObjWarning = false;

    // I don't do ASSERT_NO_THROW because I want to see the err stream to show original defect. But I ASSERT there's no err_stream because we can't
    // continue if it did throw
    EXPECT_NO_THROW(SetupZoneGeometry(*state, ErrorsFound));
    ASSERT_TRUE(compare_err_stream(""));

    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 1);
    EXPECT_EQ("Radiant Enclosure 1", state->dataViewFactor->EnclRadInfo(1).Name);
    EXPECT_EQ("ZONE1", state->dataViewFactor->EnclRadInfo(1).spaceNames[0]);
    EXPECT_EQ("ZONE2", state->dataViewFactor->EnclRadInfo(1).spaceNames[1]);
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 1);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 1);
    EXPECT_EQ("Solar Enclosure 1", state->dataViewFactor->EnclSolInfo(1).Name);
    EXPECT_EQ("ZONE1", state->dataViewFactor->EnclSolInfo(1).spaceNames[0]);
    EXPECT_EQ("ZONE2", state->dataViewFactor->EnclSolInfo(1).spaceNames[1]);
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 1);
}

TEST_F(EnergyPlusFixture, TwoZones_With_AirWindow)
{
    // Two 10x10m zones. A Window between both that has a Construction:AirBoundary assigned (its base surface is a regular interior wall)
    std::string const idf_objects = delimited_string({

        "Material,",
        "  8IN Concrete HW,                        !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2033,                                 !- Thickness {m}",
        "  1.72959999999999,                       !- Conductivity {W/m-K}",
        "  2242.99999999999,                       !- Density {kg/m3}",
        "  836.999999999999,                       !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.65,                                   !- Solar Absorptance",
        "  0.65;                                   !- Visible Absorptance",

        "Construction,",
        "  Regular Construction,                   !- Name",
        "  8IN Concrete HW;                        !- Layer 1",

        "Construction:AirBoundary,",
        "  Air Wall,                               !- Name",
        "  SimpleMixing,                           !- Air Exchange Method",
        "  0.5,                                    !- Simple Mixing Air Changes per Hour {1/hr}",
        "  ;                                       !- Simple Mixing Schedule Name",

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

        "BuildingSurface:Detailed,",
        "  Space1-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.4384;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Surface,                                !- Outside Boundary Condition",
        "  Space2-InteriorWall,                    !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorWindow,                  !- Name",
        "  Window,                                 !- Surface Type",
        "  Air Wall,                               !- Construction Name",
        "  Space1-InteriorWall,                    !- Building Surface Name",
        "  Space2-InteriorWindow,                  !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 3.5, 1.8,                           !- X,Y,Z Vertex 1 {m}",
        "  10, 3.5, 0.8,                           !- X,Y,Z Vertex 2 {m}",
        "  10, 5.5, 0.8,                           !- X,Y,Z Vertex 3 {m}",
        "  10, 5.5, 1.8;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-West,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 0;                                !- X,Y,Z Vertex 4 {m}",

        "Zone,",
        "  Zone2,                                  !- Name",
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

        "BuildingSurface:Detailed,",
        "  Space2-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Surface,                                !- Outside Boundary Condition",
        "  Space1-InteriorWall,                    !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space2-InteriorWindow,                  !- Name",
        "  Window,                                 !- Surface Type",
        "  Air Wall,                               !- Construction Name",
        "  Space2-InteriorWall,                    !- Building Surface Name",
        "  Space1-InteriorWindow,                  !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 5.5, 1.8,                           !- X,Y,Z Vertex 1 {m}",
        "  10, 5.5, 0.8,                           !- X,Y,Z Vertex 2 {m}",
        "  10, 3.5, 0.8,                           !- X,Y,Z Vertex 3 {m}",
        "  10, 3.5, 1.8;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-East,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

    });
    // This is ill-named, but it will shut the "No Ground Temperature" warning when set to false
    state->dataSurfaceGeometry->NoGroundTempObjWarning = false;

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    // I don't do ASSERT_NO_THROW because I want to see the err stream to show original defect. But I ASSERT there's no err_stream because we can't
    // continue if it did throw
    EXPECT_NO_THROW(SetupZoneGeometry(*state, ErrorsFound));
    ASSERT_TRUE(compare_err_stream(""));

    // SetupZoneGeometry calls SurfaceGeometry::GetSurfaceData
    // SetupZoneGeometry calls SurfaceGeometry::SetupSolarEnclosuresAndAirBoundaries
    // SetupZoneGeometry calls SurfaceGeometry::SetupRadiantEnclosuresAndAirBoundaries
    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 1);
    EXPECT_EQ("Radiant Enclosure 1", state->dataViewFactor->EnclRadInfo(1).Name);
    EXPECT_EQ("ZONE1", state->dataViewFactor->EnclRadInfo(1).spaceNames[0]);
    EXPECT_EQ("ZONE2", state->dataViewFactor->EnclRadInfo(1).spaceNames[1]);
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 1);

    EXPECT_EQ(state->dataViewFactor->NumOfSolarEnclosures, 1);
    EXPECT_EQ("Solar Enclosure 1", state->dataViewFactor->EnclSolInfo(1).Name);
    EXPECT_EQ("ZONE1", state->dataViewFactor->EnclSolInfo(1).spaceNames[0]);
    EXPECT_EQ("ZONE2", state->dataViewFactor->EnclSolInfo(1).spaceNames[1]);
    EXPECT_EQ(state->dataHeatBal->space(1).solarEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).solarEnclosureNum, 1);
}

TEST_F(EnergyPlusFixture, GetSurfaceData_SurfaceOrder)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    AIR,                     !- Gas Type",
        "    0.006;                   !- Thickness {m}",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    INTERIORWall,            !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN02,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:GARAGE,            !- Name",
        "    CC03;                    !- Outside Layer",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Construction,",
        "    EXTWALL:GARAGE,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11;                    !- Layer 2",

        "  Construction,",
        "    CEILING:LIVING,          !- Name",
        "    IN05,                    !- Outside Layer",
        "    GP01;                    !- Layer 2",

        "  Construction,",
        "    reverseCEILING:LIVING,   !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN05;                    !- Layer 2",

        "  Construction,",
        "    GABLE,                   !- Name",
        "    PW03;                    !- Outside Layer",

        "  Construction,",
        "    Dbl Clr 3mm/6mm Air,     !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 6MM,                 !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "  Construction,",
        "    Garage:SteelDoor,        !- Name",
        "    HF-A3;                   !- Outside Layer",

        "  Construction,",
        "    CEILING:Garage,          !- Name",
        "    GP01;                    !- Outside Layer",

        "  Zone,",
        "    LIVING ZONE;             !- Name",

        "  Zone,",
        "    GARAGE ZONE;             !- Name",

        "  Zone,",
        "    ATTIC ZONE;              !- Name",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  WindowMaterial:Glazing,",
        "    Clear Acrylic Plastic,   !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.92,                    !- Solar Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.92,                    !- Visible Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",

        "  WindowMaterial:Glazing,",
        "    Diffusing Acrylic Plastic,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.0022,                  !- Thickness {m}",
        "    0.90,                    !- Solar Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.90,                    !- Visible Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",
        "",
        "  Material,",
        "    Very High Reflectivity Surface,  !- Name",
        "    Smooth,                  !- Roughness",
        "    0.0005,                  !- Thickness {m}",
        "    237,                     !- Conductivity {W/m-K}",
        "    2702,                    !- Density {kg/m3}",
        "    903,                     !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.05,                    !- Solar Absorptance",
        "    0.05;                    !- Visible Absorptance",
        "  Construction,",
        "    TDD Pipe,                !- Name",
        "    Very High Reflectivity Surface;  !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Dome,                !- Name",
        "    Clear Acrylic Plastic;   !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Diffuser,            !- Name",
        "    Diffusing Acrylic Plastic;  !- Outside Layer",

        "  DaylightingDevice:Tubular,",
        "    Pipe1,                   !- Name",
        "    TubularDaylightingDome1,                   !- Dome Name",
        "    TubularDaylightingDiffuser1,               !- Diffuser Name",
        "    TDD Pipe,                !- Construction Name",
        "    0.3556,                  !- Diameter {m}",
        "    1.4,                     !- Total Length {m}",
        "    0.28,                    !- Effective Thermal Resistance {m2-K/W}",
        "    ATTIC ZONE,              !- Transition Zone 1 Name",
        "    1.1;                     !- Transition Zone 1 Length {m}",

        "  FenestrationSurface:Detailed,",
        "    TubularDaylightingDome1,                   !- Name",
        "    TubularDaylightDome,     !- Surface Type",
        "    TDD Dome,                !- Construction Name",
        "    NorthRoof1,              !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,6.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,6.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    TubularDaylightingDiffuser1,               !- Name",
        "    TubularDaylightDiffuser, !- Surface Type",
        "    TDD Diffuser,            !- Construction Name",
        "    Living:Ceiling,          !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.3425,3.1575,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.3425,2.8425,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2.6575,2.8425,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2.6575,3.1575,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4383,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Interior,         !- Name",
        "    WALL,                    !- Surface Type",
        "    INTERIORWall,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Interior,         !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Interior,         !- Name",
        "    WALL,                    !- Surface Type",
        "    INTERIORWall,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Interior,         !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic:LivingFloor,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:LivingFloor,       !- Name",
        "    FLOOR,                   !- Surface Type",
        "    reverseCEILING:LIVING,   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof1,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SouthRoof,               !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,5.389000,4.683800,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,2.438400,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.24200,0.000000,2.438400,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.24200,5.389000,4.683800;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof2,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.332,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,7.3172,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof3,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof4,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0.0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    WestGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    0.0,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WestRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,16.876,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthGable,        !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:EastWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:WestWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  InternalMass,",
        "    EVChargingStation,      !- Name",
        "    Garage:SteelDoor,     !- Construction Name",
        "    GARAGE ZONE,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  BuildingSurface:Detailed,",
        "    Garage:FrontDoor,        !- Name",
        "    WALL,                    !- Surface Type",
        "    Garage:SteelDoor,        !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:GarageFloor,       !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CEILING:Garage,          !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:Garage,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic:GarageFloor,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:GARAGE,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    NorthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:North,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    6.572,10.778,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    6.572,10.778,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2,10.778,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2,10.778,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    EastWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:East,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    17.242,2,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,2,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,6.572,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,6.572,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    SouthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:South,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2,0,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2,0,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.572,0,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.572,0,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,6.572,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,6.572,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,2,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,2,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestDoor,              !- Name",
        "    Door,                  !- Surface Type",
        "    Garage:SteelDoor,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,9.0,2.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,9.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,8.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,8.0,2.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    AtticSkylight,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    EastRoof,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    14.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    14.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    15.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    15.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Shading:Zone:Detailed,",
        "    ZoneShade:Living:South:Shade001,  !- Name",
        "    Living:South,        !- Base Surface Name",
        "    ,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Building:Detailed,",
        "  BuildingShade:TiltedShadeSurface,             !- Name",
        "  ,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  -40.0,2.0,20.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  -40.0,0.00,20.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  -45.0,0.00,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  -45.0,2.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Site:Detailed,",
        "  SiteShade:FlatShadeSurface,             !- Name",
        "  ,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  40.0,2.0,10.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  40.0,0.00,10.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  45.0,0.00,10.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  45.0,2.0,10.0;  !- X,Y,Z ==> Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    // compare_err_stream( "" ); // just for debugging

    // This tests the sorted order of surfaces in two ways:
    //
    // Simulation Order:
    //   All shading surfaces (including mirrored surfaces)
    //      Shading:Site
    //      Shading:Building
    //      Shading:Zone (and variants)
    //
    //   By zone (in the order the zones appear in the idf):
    //      Walls
    //      Floors
    //      Roofs
    //      Internal Mass
    //      Doors
    //      Windows
    //      TDD_Domes
    //
    // Reporting Order (preserving the old surface order scheme)
    //
    //   All shading surfaces
    //   By zone (in the order the zones appear in the idf):
    //      Walls
    //          Subsurfaces follow each base surface in the order they appear in the idf
    //      Floors
    //          Subsurfaces follow each base surface in the order they appear in the idf
    //      Roofs/Ceilings
    //          Subsurfaces follow each base surface in the order they appear in the idf
    //      Internal Mass
    //
    // Special cases:
    //      TubularDaylightDome is treated as a separate surface type
    //      TubularDaylightDiffuser is treated as a window subsurface

    // For this test, the order should be
    // Simulation Order (1-based):
    //  SHADING SURFACES:
    //      1. TiltedShadeSurface
    //      2. Mir-TiltedShadeSurface (every shading surface is mirrored by default)
    //      3. FlatShadeSurface
    //      4. Mir-FlatShadeSurface
    //      5. Living:South:Shade001
    //      6. Mir-Living:South:Shade001
    //  LIVING ZONE:
    //      7. Living:North (wall)
    //      8. Living:East (wall)
    //      9. Living:South (wall)
    //      10. Living:West (wall)
    //      11. Living:Interior (wall)
    //      12. Living:Ceiling (ceiling)
    //      13. Living:Floor (floor)
    //      14. WestDoor (door)
    //      15. NorthWindow (window)
    //      16. EastWindow (window)
    //      17. SouthWindow (window)
    //      18. WestWindow (window)
    //      19. TubularDaylightingDiffuser1 ("window")
    //  GARAGE ZONE:
    //      20. Garage:Interior (wall)
    //      21. Garage:EastWall (wall)
    //      22. Garage:WestWall (wall)
    //      23. Garage:FrontDoor (wall)
    //      24. Garage:Floor (floor)
    //      25. Garage:Ceiling (ceiling)
    //      26. EVChargingStation (internal mass)
    //
    //  ATTIC ZONE:
    //      27. EastGable (wall)
    //      28. WestGable (wall)
    //      29. NorthGable (wall)
    //      30. Attic:LivingFloor (floor)
    //      31. Attic:GarageFloor (floor)
    //      32. NorthRoof1 (roof)
    //      33. SouthRoof (roof)
    //      34. NorthRoof2 (roof)
    //      35. NorthRoof3 (roof)
    //      36. NorthRoof4 (roof)
    //      37. EastRoof (roof)
    //      38. WestRoof (roof)
    //      39. AtticSkylight (window)
    //      40. TubularDaylightingDome1 (not a window)

    // For this test, the order should be
    // Reporting (legacy) Order (zero-based):
    //  SHADING SURFACES:
    //      0. TiltedShadeSurface
    //      1. Mir-TiltedShadeSurface (every shading surface is mirrored by default)
    //      2. FlatShadeSurface
    //      3. Mir-FlatShadeSurface
    //      4. Living:South:Shade001
    //      5. Mir-Living:South:Shade001
    //  LIVING ZONE:
    //      6. Living:North (wall)
    //      7.   NorthWindow (window)
    //      8. Living:East (wall)
    //      9.  EastWindow (window)
    //      10. Living:South (wall)
    //      11.   SouthWindow (window)
    //      12. Living:West (wall)
    //      13.   WestWindow (window)
    //      14.   WestDoor (door)
    //      15. Living:Interior (wall)
    //      16. Living:Ceiling (ceiling)
    //      17.   TubularDaylightingDiffuser1 ("window")
    //      18. Living:Floor (floor)
    //  GARAGE ZONE:
    //      19. Garage:Interior (wall)
    //      20. Garage:EastWall (wall)
    //      21. Garage:WestWall (wall)
    //      22. Garage:FrontDoor (wall)
    //      23. Garage:Floor (floor)
    //      24. Garage:Ceiling (ceiling)
    //      25. EVChargingStation (internal mass)
    //
    //  ATTIC ZONE:
    //      26. EastGable (wall)
    //      27. WestGable (wall)
    //      28. NorthGable (wall)
    //      29. Attic:LivingFloor (floor)
    //      30. Attic:GarageFloor (floor)
    //      31. NorthRoof1 (roof)

    //      32. SouthRoof (roof)
    //      33. NorthRoof2 (roof)
    //      34. NorthRoof3 (roof)
    //      35. NorthRoof4 (roof)
    //      36. EastRoof (roof)
    //      37. WestRoof (roof)
    //      38.   AtticSkylight (window)
    //      39.   TubularDaylightingDome1 (not a window)

    // Simulation Order (1-based):
    //  SHADING SURFACES:
    int siteShadeShadeFlatShadeSurface = Util::FindItemInList(Util::makeUPPER("SiteShade:FlatShadeSurface"), state->dataSurface->Surface);
    int mirSiteShadeFlatShadeSurface = Util::FindItemInList("Mir-" + Util::makeUPPER("SiteShade:FlatShadeSurface"), state->dataSurface->Surface);
    int buildingShadeTiltedShadeSurface = Util::FindItemInList(Util::makeUPPER("BuildingShade:TiltedShadeSurface"), state->dataSurface->Surface);
    int mirBuildingShadeTiltedShadeSurface =
        Util::FindItemInList("Mir-" + Util::makeUPPER("BuildingShade:TiltedShadeSurface"), state->dataSurface->Surface);
    int zoneShadeLivingSouthShade001 = Util::FindItemInList(Util::makeUPPER("ZoneShade:Living:South:Shade001"), state->dataSurface->Surface);
    int mirZoneShadeLivingSouthShade001 =
        Util::FindItemInList("Mir-" + Util::makeUPPER("ZoneShade:Living:South:Shade001"), state->dataSurface->Surface);
    EXPECT_EQ(siteShadeShadeFlatShadeSurface, 1);
    EXPECT_EQ(mirSiteShadeFlatShadeSurface, 2);
    EXPECT_EQ(buildingShadeTiltedShadeSurface, 3);
    EXPECT_EQ(mirBuildingShadeTiltedShadeSurface, 4);
    EXPECT_EQ(zoneShadeLivingSouthShade001, 5);
    EXPECT_EQ(mirZoneShadeLivingSouthShade001, 6);

    //  LIVING ZONE:
    int wallLivingNorth = Util::FindItemInList(Util::makeUPPER("Living:North"), state->dataSurface->Surface);
    int wallLivingEast = Util::FindItemInList(Util::makeUPPER("Living:East"), state->dataSurface->Surface);
    int wallLivingSouth = Util::FindItemInList(Util::makeUPPER("Living:South"), state->dataSurface->Surface);
    int wallLivingWest = Util::FindItemInList(Util::makeUPPER("Living:West"), state->dataSurface->Surface);
    int wallLivingInterior = Util::FindItemInList(Util::makeUPPER("Living:Interior"), state->dataSurface->Surface);
    int floorLivingFloor = Util::FindItemInList(Util::makeUPPER("Living:Floor"), state->dataSurface->Surface);
    int ceilingLivingCeiling = Util::FindItemInList(Util::makeUPPER("Living:Ceiling"), state->dataSurface->Surface);
    int doorWestDoor = Util::FindItemInList(Util::makeUPPER("WestDoor"), state->dataSurface->Surface);
    int windowTubularDaylightingDiffuser1 = Util::FindItemInList(Util::makeUPPER("TubularDaylightingDiffuser1"), state->dataSurface->Surface);
    int windowNorthWindow = Util::FindItemInList(Util::makeUPPER("NorthWindow"), state->dataSurface->Surface);
    int windowEastWindow = Util::FindItemInList(Util::makeUPPER("EastWindow"), state->dataSurface->Surface);
    int windowSouthWindow = Util::FindItemInList(Util::makeUPPER("SouthWindow"), state->dataSurface->Surface);
    int windowWestWindow = Util::FindItemInList(Util::makeUPPER("WestWindow"), state->dataSurface->Surface);

    EXPECT_EQ(wallLivingNorth, 7);
    EXPECT_EQ(wallLivingEast, 8);
    EXPECT_EQ(wallLivingSouth, 9);
    EXPECT_EQ(wallLivingWest, 10);
    EXPECT_EQ(wallLivingInterior, 11);
    EXPECT_EQ(floorLivingFloor, 12);
    EXPECT_EQ(ceilingLivingCeiling, 13);
    EXPECT_EQ(doorWestDoor, 14);
    EXPECT_EQ(windowNorthWindow, 15);
    EXPECT_EQ(windowEastWindow, 16);
    EXPECT_EQ(windowSouthWindow, 17);
    EXPECT_EQ(windowWestWindow, 18);
    EXPECT_EQ(windowTubularDaylightingDiffuser1, 19);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceFirst, 7);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceLast, 19);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst, 7);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast, 14);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceFirst, 15);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceLast, 19);

    //  GARAGE ZONE:
    int wallGarageInterior = Util::FindItemInList(Util::makeUPPER("Garage:Interior"), state->dataSurface->Surface);
    int wallGarageEast = Util::FindItemInList(Util::makeUPPER("Garage:EastWall"), state->dataSurface->Surface);
    int wallGarageWest = Util::FindItemInList(Util::makeUPPER("Garage:WestWall"), state->dataSurface->Surface);
    int wallGarageFrontDoor = Util::FindItemInList(Util::makeUPPER("Garage:FrontDoor"), state->dataSurface->Surface);
    int floorGarageFloor = Util::FindItemInList(Util::makeUPPER("Garage:Floor"), state->dataSurface->Surface);
    int ceilingGarageInterior = Util::FindItemInList(Util::makeUPPER("Garage:Ceiling"), state->dataSurface->Surface);
    int intmassEVChargingStation = Util::FindItemInList(Util::makeUPPER("EVChargingStation"), state->dataSurface->Surface);

    EXPECT_EQ(wallGarageInterior, 20);
    EXPECT_EQ(wallGarageEast, 21);
    EXPECT_EQ(wallGarageWest, 22);
    EXPECT_EQ(wallGarageFrontDoor, 23);
    EXPECT_EQ(floorGarageFloor, 24);
    EXPECT_EQ(ceilingGarageInterior, 25);
    EXPECT_EQ(intmassEVChargingStation, 26);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceFirst, 20);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceLast, 26);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrIntMassSurfaceFirst, 20);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrIntMassSurfaceLast, 26);
    EXPECT_EQ(state->dataHeatBal->space(2).WindowSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(2).WindowSurfaceLast, -1);

    //  ATTIC ZONE:
    int wallEastGable = Util::FindItemInList(Util::makeUPPER("EastGable"), state->dataSurface->Surface);
    int wallWestGable = Util::FindItemInList(Util::makeUPPER("WestGable"), state->dataSurface->Surface);
    int wallNorthGable = Util::FindItemInList(Util::makeUPPER("NorthGable"), state->dataSurface->Surface);
    int floorAtticLivingFloor = Util::FindItemInList(Util::makeUPPER("Attic:LivingFloor"), state->dataSurface->Surface);
    int floorAtticGarageFloor = Util::FindItemInList(Util::makeUPPER("Attic:GarageFloor"), state->dataSurface->Surface);
    int roofNorthRoof1 = Util::FindItemInList(Util::makeUPPER("NorthRoof1"), state->dataSurface->Surface);
    int roofSouthRoof = Util::FindItemInList(Util::makeUPPER("SouthRoof"), state->dataSurface->Surface);
    int roofNorthRoof2 = Util::FindItemInList(Util::makeUPPER("NorthRoof2"), state->dataSurface->Surface);
    int roofNorthRoof3 = Util::FindItemInList(Util::makeUPPER("NorthRoof3"), state->dataSurface->Surface);
    int roofNorthRoof4 = Util::FindItemInList(Util::makeUPPER("NorthRoof4"), state->dataSurface->Surface);
    int roofEastRoof = Util::FindItemInList(Util::makeUPPER("EastRoof"), state->dataSurface->Surface);
    int roofWestRoof = Util::FindItemInList(Util::makeUPPER("WestRoof"), state->dataSurface->Surface);
    int nonwindowTubularDaylightingDome1 = Util::FindItemInList(Util::makeUPPER("TubularDaylightingDome1"), state->dataSurface->Surface);
    int windowAtticSkylight = Util::FindItemInList(Util::makeUPPER("AtticSkylight"), state->dataSurface->Surface);

    EXPECT_EQ(wallEastGable, 27);
    EXPECT_EQ(wallWestGable, 28);
    EXPECT_EQ(wallNorthGable, 29);
    EXPECT_EQ(floorAtticLivingFloor, 30);
    EXPECT_EQ(floorAtticGarageFloor, 31);
    EXPECT_EQ(roofNorthRoof1, 32);
    EXPECT_EQ(roofSouthRoof, 33);
    EXPECT_EQ(roofNorthRoof2, 34);
    EXPECT_EQ(roofNorthRoof3, 35);
    EXPECT_EQ(roofNorthRoof4, 36);
    EXPECT_EQ(roofEastRoof, 37);
    EXPECT_EQ(roofWestRoof, 38);
    EXPECT_EQ(windowAtticSkylight, 39);
    EXPECT_EQ(nonwindowTubularDaylightingDome1, 40);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceLast, nonwindowTubularDaylightingDome1);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceLast, roofWestRoof);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceFirst, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceLast, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceLast, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeFirst, nonwindowTubularDaylightingDome1);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeLast, nonwindowTubularDaylightingDome1);

    // Reporting (legacy) Order (zero-based)
    //  SHADING SURFACES:
    EXPECT_EQ(siteShadeShadeFlatShadeSurface, state->dataSurface->AllSurfaceListReportOrder[0]);
    EXPECT_EQ(mirSiteShadeFlatShadeSurface, state->dataSurface->AllSurfaceListReportOrder[1]);
    EXPECT_EQ(buildingShadeTiltedShadeSurface, state->dataSurface->AllSurfaceListReportOrder[2]);
    EXPECT_EQ(mirBuildingShadeTiltedShadeSurface, state->dataSurface->AllSurfaceListReportOrder[3]);
    EXPECT_EQ(zoneShadeLivingSouthShade001, state->dataSurface->AllSurfaceListReportOrder[4]);
    EXPECT_EQ(mirZoneShadeLivingSouthShade001, state->dataSurface->AllSurfaceListReportOrder[5]);

    //  LIVING ZONE:
    EXPECT_EQ(wallLivingNorth, state->dataSurface->AllSurfaceListReportOrder[6]);
    EXPECT_EQ(windowNorthWindow, state->dataSurface->AllSurfaceListReportOrder[7]);
    EXPECT_EQ(wallLivingEast, state->dataSurface->AllSurfaceListReportOrder[8]);
    EXPECT_EQ(windowEastWindow, state->dataSurface->AllSurfaceListReportOrder[9]);
    EXPECT_EQ(wallLivingSouth, state->dataSurface->AllSurfaceListReportOrder[10]);
    EXPECT_EQ(windowSouthWindow, state->dataSurface->AllSurfaceListReportOrder[11]);
    EXPECT_EQ(wallLivingWest, state->dataSurface->AllSurfaceListReportOrder[12]);
    EXPECT_EQ(windowWestWindow, state->dataSurface->AllSurfaceListReportOrder[13]);
    EXPECT_EQ(doorWestDoor, state->dataSurface->AllSurfaceListReportOrder[14]);
    EXPECT_EQ(wallLivingInterior, state->dataSurface->AllSurfaceListReportOrder[15]);
    EXPECT_EQ(floorLivingFloor, state->dataSurface->AllSurfaceListReportOrder[16]);
    EXPECT_EQ(ceilingLivingCeiling, state->dataSurface->AllSurfaceListReportOrder[17]);
    EXPECT_EQ(windowTubularDaylightingDiffuser1, state->dataSurface->AllSurfaceListReportOrder[18]);

    //  GARAGE ZONE:
    EXPECT_EQ(wallGarageInterior, state->dataSurface->AllSurfaceListReportOrder[19]);
    EXPECT_EQ(wallGarageEast, state->dataSurface->AllSurfaceListReportOrder[20]);
    EXPECT_EQ(wallGarageWest, state->dataSurface->AllSurfaceListReportOrder[21]);
    EXPECT_EQ(wallGarageFrontDoor, state->dataSurface->AllSurfaceListReportOrder[22]);
    EXPECT_EQ(floorGarageFloor, state->dataSurface->AllSurfaceListReportOrder[23]);
    EXPECT_EQ(ceilingGarageInterior, state->dataSurface->AllSurfaceListReportOrder[24]);
    EXPECT_EQ(intmassEVChargingStation, state->dataSurface->AllSurfaceListReportOrder[25]);

    //  ATTIC ZONE:
    EXPECT_EQ(wallEastGable, state->dataSurface->AllSurfaceListReportOrder[26]);
    EXPECT_EQ(wallWestGable, state->dataSurface->AllSurfaceListReportOrder[27]);
    EXPECT_EQ(wallNorthGable, state->dataSurface->AllSurfaceListReportOrder[28]);
    EXPECT_EQ(floorAtticLivingFloor, state->dataSurface->AllSurfaceListReportOrder[29]);
    EXPECT_EQ(floorAtticGarageFloor, state->dataSurface->AllSurfaceListReportOrder[30]);
    EXPECT_EQ(roofNorthRoof1, state->dataSurface->AllSurfaceListReportOrder[31]);
    EXPECT_EQ(nonwindowTubularDaylightingDome1, state->dataSurface->AllSurfaceListReportOrder[32]);
    EXPECT_EQ(roofSouthRoof, state->dataSurface->AllSurfaceListReportOrder[33]);
    EXPECT_EQ(roofNorthRoof2, state->dataSurface->AllSurfaceListReportOrder[34]);
    EXPECT_EQ(roofNorthRoof3, state->dataSurface->AllSurfaceListReportOrder[35]);
    EXPECT_EQ(roofNorthRoof4, state->dataSurface->AllSurfaceListReportOrder[36]);
    EXPECT_EQ(roofEastRoof, state->dataSurface->AllSurfaceListReportOrder[37]);
    EXPECT_EQ(windowAtticSkylight, state->dataSurface->AllSurfaceListReportOrder[38]);
    EXPECT_EQ(roofWestRoof, state->dataSurface->AllSurfaceListReportOrder[39]);

    // Extend test to check other surface lists
    auto &HTSurfaces = state->dataSurface->AllHTSurfaceList;
    auto &ExtSolarSurfaces = state->dataSurface->AllExtSolarSurfaceList;
    auto &ExtSolAndShadingSurfaces = state->dataSurface->AllExtSolAndShadingSurfaceList;
    auto &ShadowPossObstrSurfaces = state->dataSurface->AllShadowPossObstrSurfaceList;
    auto &IZSurfaces = state->dataSurface->AllIZSurfaceList;
    auto &HTNonWindowSurfaces = state->dataSurface->AllHTNonWindowSurfaceList;
    auto &HTWindowSurfaces = state->dataSurface->AllHTWindowSurfaceList;
    auto &ExtSolWindowSurfaces = state->dataSurface->AllExtSolWindowSurfaceList;
    auto &ExtSolWinWithFrameSurfaces = state->dataSurface->AllExtSolWinWithFrameSurfaceList;
    auto &HTKivaSurfaces = state->dataSurface->AllHTKivaSurfaceList;

    int thisSurface = siteShadeShadeFlatShadeSurface;
    EXPECT_FALSE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_FALSE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = wallEastGable;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = doorWestDoor;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = wallGarageInterior;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_TRUE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = windowSouthWindow;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_FALSE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_TRUE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = windowTubularDaylightingDiffuser1;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_FALSE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_TRUE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = nonwindowTubularDaylightingDome1;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());
}

TEST_F(EnergyPlusFixture, GetSurfaceData_SurfaceOrder2)
{
    // First zone, LIVING ZONE, has no surfaces

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    AIR,                     !- Gas Type",
        "    0.006;                   !- Thickness {m}",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    INTERIORWall,            !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN02,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:GARAGE,            !- Name",
        "    CC03;                    !- Outside Layer",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Construction,",
        "    EXTWALL:GARAGE,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11;                    !- Layer 2",

        "  Construction,",
        "    CEILING:LIVING,          !- Name",
        "    IN05,                    !- Outside Layer",
        "    GP01;                    !- Layer 2",

        "  Construction,",
        "    reverseCEILING:LIVING,   !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN05;                    !- Layer 2",

        "  Construction,",
        "    GABLE,                   !- Name",
        "    PW03;                    !- Outside Layer",

        "  Construction,",
        "    Dbl Clr 3mm/6mm Air,     !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 6MM,                 !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "  Construction,",
        "    Garage:SteelDoor,        !- Name",
        "    HF-A3;                   !- Outside Layer",

        "  Construction,",
        "    CEILING:Garage,          !- Name",
        "    GP01;                    !- Outside Layer",

        "  Zone,",
        "    LIVING ZONE;             !- Name",

        "  Zone,",
        "    GARAGE ZONE;             !- Name",

        "  Zone,",
        "    ATTIC ZONE;              !- Name",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  WindowMaterial:Glazing,",
        "    Clear Acrylic Plastic,   !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.92,                    !- Solar Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.92,                    !- Visible Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",

        "  WindowMaterial:Glazing,",
        "    Diffusing Acrylic Plastic,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.0022,                  !- Thickness {m}",
        "    0.90,                    !- Solar Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.90,                    !- Visible Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",
        "",
        "  Material,",
        "    Very High Reflectivity Surface,  !- Name",
        "    Smooth,                  !- Roughness",
        "    0.0005,                  !- Thickness {m}",
        "    237,                     !- Conductivity {W/m-K}",
        "    2702,                    !- Density {kg/m3}",
        "    903,                     !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.05,                    !- Solar Absorptance",
        "    0.05;                    !- Visible Absorptance",
        "  Construction,",
        "    TDD Pipe,                !- Name",
        "    Very High Reflectivity Surface;  !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Dome,                !- Name",
        "    Clear Acrylic Plastic;   !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Diffuser,            !- Name",
        "    Diffusing Acrylic Plastic;  !- Outside Layer",

        "  DaylightingDevice:Tubular,",
        "    Pipe1,                   !- Name",
        "    TubularDaylightingDome1,                   !- Dome Name",
        "    TubularDaylightingDiffuser1,               !- Diffuser Name",
        "    TDD Pipe,                !- Construction Name",
        "    0.3556,                  !- Diameter {m}",
        "    1.4,                     !- Total Length {m}",
        "    0.28,                    !- Effective Thermal Resistance {m2-K/W}",
        "    ATTIC ZONE,              !- Transition Zone 1 Name",
        "    1.1;                     !- Transition Zone 1 Length {m}",

        "  FenestrationSurface:Detailed,",
        "    TubularDaylightingDome1,                   !- Name",
        "    TubularDaylightDome,     !- Surface Type",
        "    TDD Dome,                !- Construction Name",
        "    NorthRoof1,              !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,6.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,6.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof1,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SouthRoof,               !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,5.389000,4.683800,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,2.438400,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.24200,0.000000,2.438400,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.24200,5.389000,4.683800;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof2,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.332,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,7.3172,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof3,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof4,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0.0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    WestGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    0.0,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WestRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,16.876,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthGable,        !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:EastWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:WestWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  InternalMass,",
        "    EVChargingStation,      !- Name",
        "    Garage:SteelDoor,     !- Construction Name",
        "    GARAGE ZONE,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  BuildingSurface:Detailed,",
        "    Garage:FrontDoor,        !- Name",
        "    WALL,                    !- Surface Type",
        "    Garage:SteelDoor,        !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:GarageFloor,       !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CEILING:Garage,          !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:Garage,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic:GarageFloor,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:GARAGE,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    AtticSkylight,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    EastRoof,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    14.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    14.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    15.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    15.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                  // expect no errors

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    // compare_err_stream( "" ); // just for debugging

    // This tests the space surface first/last values after surfaces are sorted:
    //
    // Simulation Order:
    //   All shading surfaces (including mirrored surfaces)
    //      Shading:Site
    //      Shading:Building
    //      Shading:Zone (and variants)
    //
    //   By zone (in the order the zones appear in the idf):
    //      Walls
    //      Floors
    //      Roofs
    //      Internal Mass
    //      Doors
    //      Windows
    //      TDD_Domes
    //
    // Special cases:
    //      TubularDaylightDome is treated as a separate surface type
    //      TubularDaylightDiffuser is treated as a window subsurface

    // For this test, the order should be
    // Simulation Order (1-based):
    //  GARAGE ZONE:
    //      1. Garage:EastWall (wall)
    //      2. Garage:WestWall (wall)
    //      3. Garage:FrontDoor (wall)
    //      4. Garage:Floor (floor)
    //      5. Garage:Ceiling (ceiling)
    //      6. EVChargingStation (internal mass)
    //
    //  ATTIC ZONE:
    //      7. EastGable (wall)
    //      8. WestGable (wall)
    //      9. NorthGable (wall)
    //      10. Attic:GarageFloor (floor)
    //      11. NorthRoof1 (roof)
    //      12. SouthRoof (roof)
    //      13. NorthRoof2 (roof)
    //      14. NorthRoof3 (roof)
    //      15. NorthRoof4 (roof)
    //      16. EastRoof (roof)
    //      17. WestRoof (roof)
    //      18. AtticSkylight (window)
    //      19. TubularDaylightingDome1 (not a window)

    // Simulation Order (1-based):

    //  LIVING ZONE - no surfaces
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceFirst, -0);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrWinSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrWinSurfaceLast, -1);

    //  GARAGE ZONE:
    int wallGarageEast = Util::FindItemInList(Util::makeUPPER("Garage:EastWall"), state->dataSurface->Surface);
    int wallGarageWest = Util::FindItemInList(Util::makeUPPER("Garage:WestWall"), state->dataSurface->Surface);
    int wallGarageFrontDoor = Util::FindItemInList(Util::makeUPPER("Garage:FrontDoor"), state->dataSurface->Surface);
    int floorGarageFloor = Util::FindItemInList(Util::makeUPPER("Garage:Floor"), state->dataSurface->Surface);
    int ceilingGarageInterior = Util::FindItemInList(Util::makeUPPER("Garage:Ceiling"), state->dataSurface->Surface);
    int intmassEVChargingStation = Util::FindItemInList(Util::makeUPPER("EVChargingStation"), state->dataSurface->Surface);

    EXPECT_EQ(wallGarageEast, 1);
    EXPECT_EQ(wallGarageWest, 2);
    EXPECT_EQ(wallGarageFrontDoor, 3);
    EXPECT_EQ(floorGarageFloor, 4);
    EXPECT_EQ(ceilingGarageInterior, 5);
    EXPECT_EQ(intmassEVChargingStation, 6);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceFirst, wallGarageEast);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceLast, intmassEVChargingStation);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrIntMassSurfaceFirst, wallGarageEast);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrIntMassSurfaceLast, intmassEVChargingStation);
    EXPECT_EQ(state->dataHeatBal->space(2).WindowSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(2).WindowSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrWinSurfaceFirst, wallGarageEast);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrWinSurfaceLast, intmassEVChargingStation);

    //  ATTIC ZONE:
    int wallEastGable = Util::FindItemInList(Util::makeUPPER("EastGable"), state->dataSurface->Surface);
    int wallWestGable = Util::FindItemInList(Util::makeUPPER("WestGable"), state->dataSurface->Surface);
    int wallNorthGable = Util::FindItemInList(Util::makeUPPER("NorthGable"), state->dataSurface->Surface);
    int floorAtticGarageFloor = Util::FindItemInList(Util::makeUPPER("Attic:GarageFloor"), state->dataSurface->Surface);
    int roofNorthRoof1 = Util::FindItemInList(Util::makeUPPER("NorthRoof1"), state->dataSurface->Surface);
    int roofSouthRoof = Util::FindItemInList(Util::makeUPPER("SouthRoof"), state->dataSurface->Surface);
    int roofNorthRoof2 = Util::FindItemInList(Util::makeUPPER("NorthRoof2"), state->dataSurface->Surface);
    int roofNorthRoof3 = Util::FindItemInList(Util::makeUPPER("NorthRoof3"), state->dataSurface->Surface);
    int roofNorthRoof4 = Util::FindItemInList(Util::makeUPPER("NorthRoof4"), state->dataSurface->Surface);
    int roofEastRoof = Util::FindItemInList(Util::makeUPPER("EastRoof"), state->dataSurface->Surface);
    int roofWestRoof = Util::FindItemInList(Util::makeUPPER("WestRoof"), state->dataSurface->Surface);
    int nonwindowTubularDaylightingDome1 = Util::FindItemInList(Util::makeUPPER("TubularDaylightingDome1"), state->dataSurface->Surface);
    int windowAtticSkylight = Util::FindItemInList(Util::makeUPPER("AtticSkylight"), state->dataSurface->Surface);

    EXPECT_EQ(wallEastGable, 7);
    EXPECT_EQ(wallWestGable, 8);
    EXPECT_EQ(wallNorthGable, 9);
    EXPECT_EQ(floorAtticGarageFloor, 10);
    EXPECT_EQ(roofNorthRoof1, 11);
    EXPECT_EQ(roofSouthRoof, 12);
    EXPECT_EQ(roofNorthRoof2, 13);
    EXPECT_EQ(roofNorthRoof3, 14);
    EXPECT_EQ(roofNorthRoof4, 15);
    EXPECT_EQ(roofEastRoof, 16);
    EXPECT_EQ(roofWestRoof, 17);
    EXPECT_EQ(windowAtticSkylight, 18);
    EXPECT_EQ(nonwindowTubularDaylightingDome1, 19);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceLast, nonwindowTubularDaylightingDome1);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceLast, roofWestRoof);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceFirst, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceLast, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceLast, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeFirst, nonwindowTubularDaylightingDome1);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeLast, nonwindowTubularDaylightingDome1);

    // Extend test to check other surface lists
    auto &HTSurfaces = state->dataSurface->AllHTSurfaceList;
    auto &ExtSolarSurfaces = state->dataSurface->AllExtSolarSurfaceList;
    auto &ExtSolAndShadingSurfaces = state->dataSurface->AllExtSolAndShadingSurfaceList;
    auto &ShadowPossObstrSurfaces = state->dataSurface->AllShadowPossObstrSurfaceList;
    auto &IZSurfaces = state->dataSurface->AllIZSurfaceList;
    auto &HTNonWindowSurfaces = state->dataSurface->AllHTNonWindowSurfaceList;
    auto &HTWindowSurfaces = state->dataSurface->AllHTWindowSurfaceList;
    auto &ExtSolWindowSurfaces = state->dataSurface->AllExtSolWindowSurfaceList;
    auto &ExtSolWinWithFrameSurfaces = state->dataSurface->AllExtSolWinWithFrameSurfaceList;
    auto &HTKivaSurfaces = state->dataSurface->AllHTKivaSurfaceList;

    int thisSurface = wallEastGable;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = nonwindowTubularDaylightingDome1;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());
}

TEST_F(EnergyPlusFixture, GetSurfaceData_SurfaceOrder3)
{

    // Middle zone, GARAGE ZONE, has no surfaces

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    AIR,                     !- Gas Type",
        "    0.006;                   !- Thickness {m}",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    INTERIORWall,            !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN02,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:GARAGE,            !- Name",
        "    CC03;                    !- Outside Layer",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Construction,",
        "    EXTWALL:GARAGE,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11;                    !- Layer 2",

        "  Construction,",
        "    CEILING:LIVING,          !- Name",
        "    IN05,                    !- Outside Layer",
        "    GP01;                    !- Layer 2",

        "  Construction,",
        "    reverseCEILING:LIVING,   !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN05;                    !- Layer 2",

        "  Construction,",
        "    GABLE,                   !- Name",
        "    PW03;                    !- Outside Layer",

        "  Construction,",
        "    Dbl Clr 3mm/6mm Air,     !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 6MM,                 !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "  Construction,",
        "    Garage:SteelDoor,        !- Name",
        "    HF-A3;                   !- Outside Layer",

        "  Construction,",
        "    CEILING:Garage,          !- Name",
        "    GP01;                    !- Outside Layer",

        "  Zone,",
        "    LIVING ZONE;             !- Name",

        "  Zone,",
        "    GARAGE ZONE;             !- Name",

        "  Zone,",
        "    ATTIC ZONE;              !- Name",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  WindowMaterial:Glazing,",
        "    Clear Acrylic Plastic,   !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.92,                    !- Solar Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.92,                    !- Visible Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",

        "  WindowMaterial:Glazing,",
        "    Diffusing Acrylic Plastic,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.0022,                  !- Thickness {m}",
        "    0.90,                    !- Solar Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.90,                    !- Visible Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",
        "",
        "  Material,",
        "    Very High Reflectivity Surface,  !- Name",
        "    Smooth,                  !- Roughness",
        "    0.0005,                  !- Thickness {m}",
        "    237,                     !- Conductivity {W/m-K}",
        "    2702,                    !- Density {kg/m3}",
        "    903,                     !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.05,                    !- Solar Absorptance",
        "    0.05;                    !- Visible Absorptance",
        "  Construction,",
        "    TDD Pipe,                !- Name",
        "    Very High Reflectivity Surface;  !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Dome,                !- Name",
        "    Clear Acrylic Plastic;   !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Diffuser,            !- Name",
        "    Diffusing Acrylic Plastic;  !- Outside Layer",

        "  DaylightingDevice:Tubular,",
        "    Pipe1,                   !- Name",
        "    TubularDaylightingDome1,                   !- Dome Name",
        "    TubularDaylightingDiffuser1,               !- Diffuser Name",
        "    TDD Pipe,                !- Construction Name",
        "    0.3556,                  !- Diameter {m}",
        "    1.4,                     !- Total Length {m}",
        "    0.28,                    !- Effective Thermal Resistance {m2-K/W}",
        "    ATTIC ZONE,              !- Transition Zone 1 Name",
        "    1.1;                     !- Transition Zone 1 Length {m}",

        "  FenestrationSurface:Detailed,",
        "    TubularDaylightingDome1,                   !- Name",
        "    TubularDaylightDome,     !- Surface Type",
        "    TDD Dome,                !- Construction Name",
        "    NorthRoof1,              !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,6.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,6.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    TubularDaylightingDiffuser1,               !- Name",
        "    TubularDaylightDiffuser, !- Surface Type",
        "    TDD Diffuser,            !- Construction Name",
        "    Living:Ceiling,          !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.3425,3.1575,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.3425,2.8425,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2.6575,2.8425,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2.6575,3.1575,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4383,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic:LivingFloor,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:LivingFloor,       !- Name",
        "    FLOOR,                   !- Surface Type",
        "    reverseCEILING:LIVING,   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof1,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SouthRoof,               !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,5.389000,4.683800,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,2.438400,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.24200,0.000000,2.438400,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.24200,5.389000,4.683800;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof2,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.332,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,7.3172,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof3,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof4,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0.0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    WestGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    0.0,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WestRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,16.876,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthGable,        !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  FenestrationSurface:Detailed,",
        "    NorthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:North,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    6.572,10.778,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    6.572,10.778,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2,10.778,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2,10.778,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    EastWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:East,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    17.242,2,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,2,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,6.572,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,6.572,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    SouthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:South,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2,0,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2,0,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.572,0,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.572,0,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,6.572,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,6.572,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,2,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,2,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestDoor,              !- Name",
        "    Door,                  !- Surface Type",
        "    Garage:SteelDoor,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,9.0,2.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,9.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,8.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,8.0,2.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    AtticSkylight,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    EastRoof,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    14.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    14.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    15.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    15.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                  // expect no errors

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    // compare_err_stream( "" ); // just for debugging

    // This tests the space surface first/last values after surfaces are sorted:
    //
    // Simulation Order:
    //   All shading surfaces (including mirrored surfaces)
    //      Shading:Site
    //      Shading:Building
    //      Shading:Zone (and variants)
    //
    //   By zone (in the order the zones appear in the idf):
    //      Walls
    //      Floors
    //      Roofs
    //      Internal Mass
    //      Doors
    //      Windows
    //      TDD_Domes
    //
    // Special cases:
    //      TubularDaylightDome is treated as a separate surface type
    //      TubularDaylightDiffuser is treated as a window subsurface

    // For this test, the order should be
    // Simulation Order (1-based):
    //  LIVING ZONE:
    //      1. Living:North (wall)
    //      2. Living:East (wall)
    //      3. Living:South (wall)
    //      4. Living:West (wall)
    //      5. Living:Ceiling (ceiling)
    //      6. Living:Floor (floor)
    //      7. WestDoor (door)
    //      8. NorthWindow (window)
    //      9. EastWindow (window)
    //      10. SouthWindow (window)
    //      11. WestWindow (window)
    //      12. TubularDaylightingDiffuser1 ("window")
    //
    //  ATTIC ZONE:
    //      13. EastGable (wall)
    //      14. WestGable (wall)
    //      15. NorthGable (wall)
    //      16. Attic:LivingFloor (floor)
    //      17. NorthRoof1 (roof)
    //      18. SouthRoof (roof)
    //      19. NorthRoof2 (roof)
    //      20. NorthRoof3 (roof)
    //      21. NorthRoof4 (roof)
    //      22. EastRoof (roof)
    //      23. WestRoof (roof)
    //      24. AtticSkylight (window)
    //      25. TubularDaylightingDome1 (not a window)

    // Simulation Order (1-based):

    //  LIVING ZONE:
    int wallLivingNorth = Util::FindItemInList(Util::makeUPPER("Living:North"), state->dataSurface->Surface);
    int wallLivingEast = Util::FindItemInList(Util::makeUPPER("Living:East"), state->dataSurface->Surface);
    int wallLivingSouth = Util::FindItemInList(Util::makeUPPER("Living:South"), state->dataSurface->Surface);
    int wallLivingWest = Util::FindItemInList(Util::makeUPPER("Living:West"), state->dataSurface->Surface);
    int floorLivingFloor = Util::FindItemInList(Util::makeUPPER("Living:Floor"), state->dataSurface->Surface);
    int ceilingLivingCeiling = Util::FindItemInList(Util::makeUPPER("Living:Ceiling"), state->dataSurface->Surface);
    int doorWestDoor = Util::FindItemInList(Util::makeUPPER("WestDoor"), state->dataSurface->Surface);
    int windowTubularDaylightingDiffuser1 = Util::FindItemInList(Util::makeUPPER("TubularDaylightingDiffuser1"), state->dataSurface->Surface);
    int windowNorthWindow = Util::FindItemInList(Util::makeUPPER("NorthWindow"), state->dataSurface->Surface);
    int windowEastWindow = Util::FindItemInList(Util::makeUPPER("EastWindow"), state->dataSurface->Surface);
    int windowSouthWindow = Util::FindItemInList(Util::makeUPPER("SouthWindow"), state->dataSurface->Surface);
    int windowWestWindow = Util::FindItemInList(Util::makeUPPER("WestWindow"), state->dataSurface->Surface);

    EXPECT_EQ(wallLivingNorth, 1);
    EXPECT_EQ(wallLivingEast, 2);
    EXPECT_EQ(wallLivingSouth, 3);
    EXPECT_EQ(wallLivingWest, 4);
    EXPECT_EQ(floorLivingFloor, 5);
    EXPECT_EQ(ceilingLivingCeiling, 6);
    EXPECT_EQ(doorWestDoor, 7);
    EXPECT_EQ(windowNorthWindow, 8);
    EXPECT_EQ(windowEastWindow, 9);
    EXPECT_EQ(windowSouthWindow, 10);
    EXPECT_EQ(windowWestWindow, 11);
    EXPECT_EQ(windowTubularDaylightingDiffuser1, 12);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceFirst, 1);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceLast, 12);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst, 1);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast, 7);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceFirst, 8);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceLast, 12);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrWinSurfaceFirst, 1);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrWinSurfaceLast, 12);

    //  ATTIC ZONE:
    int wallEastGable = Util::FindItemInList(Util::makeUPPER("EastGable"), state->dataSurface->Surface);
    int wallWestGable = Util::FindItemInList(Util::makeUPPER("WestGable"), state->dataSurface->Surface);
    int wallNorthGable = Util::FindItemInList(Util::makeUPPER("NorthGable"), state->dataSurface->Surface);
    int floorAtticLivingFloor = Util::FindItemInList(Util::makeUPPER("Attic:LivingFloor"), state->dataSurface->Surface);
    int roofNorthRoof1 = Util::FindItemInList(Util::makeUPPER("NorthRoof1"), state->dataSurface->Surface);
    int roofSouthRoof = Util::FindItemInList(Util::makeUPPER("SouthRoof"), state->dataSurface->Surface);
    int roofNorthRoof2 = Util::FindItemInList(Util::makeUPPER("NorthRoof2"), state->dataSurface->Surface);
    int roofNorthRoof3 = Util::FindItemInList(Util::makeUPPER("NorthRoof3"), state->dataSurface->Surface);
    int roofNorthRoof4 = Util::FindItemInList(Util::makeUPPER("NorthRoof4"), state->dataSurface->Surface);
    int roofEastRoof = Util::FindItemInList(Util::makeUPPER("EastRoof"), state->dataSurface->Surface);
    int roofWestRoof = Util::FindItemInList(Util::makeUPPER("WestRoof"), state->dataSurface->Surface);
    int nonwindowTubularDaylightingDome1 = Util::FindItemInList(Util::makeUPPER("TubularDaylightingDome1"), state->dataSurface->Surface);
    int windowAtticSkylight = Util::FindItemInList(Util::makeUPPER("AtticSkylight"), state->dataSurface->Surface);

    EXPECT_EQ(wallEastGable, 13);
    EXPECT_EQ(wallWestGable, 14);
    EXPECT_EQ(wallNorthGable, 15);
    EXPECT_EQ(floorAtticLivingFloor, 16);
    EXPECT_EQ(roofNorthRoof1, 17);
    EXPECT_EQ(roofSouthRoof, 18);
    EXPECT_EQ(roofNorthRoof2, 19);
    EXPECT_EQ(roofNorthRoof3, 20);
    EXPECT_EQ(roofNorthRoof4, 21);
    EXPECT_EQ(roofEastRoof, 22);
    EXPECT_EQ(roofWestRoof, 23);
    EXPECT_EQ(windowAtticSkylight, 24);
    EXPECT_EQ(nonwindowTubularDaylightingDome1, 25);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceLast, nonwindowTubularDaylightingDome1);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceLast, roofWestRoof);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceFirst, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceLast, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceFirst, wallEastGable);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceLast, windowAtticSkylight);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeFirst, nonwindowTubularDaylightingDome1);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeLast, nonwindowTubularDaylightingDome1);

    // Extend test to check other surface lists
    auto &HTSurfaces = state->dataSurface->AllHTSurfaceList;
    auto &ExtSolarSurfaces = state->dataSurface->AllExtSolarSurfaceList;
    auto &ExtSolAndShadingSurfaces = state->dataSurface->AllExtSolAndShadingSurfaceList;
    auto &ShadowPossObstrSurfaces = state->dataSurface->AllShadowPossObstrSurfaceList;
    auto &IZSurfaces = state->dataSurface->AllIZSurfaceList;
    auto &HTNonWindowSurfaces = state->dataSurface->AllHTNonWindowSurfaceList;
    auto &HTWindowSurfaces = state->dataSurface->AllHTWindowSurfaceList;
    auto &ExtSolWindowSurfaces = state->dataSurface->AllExtSolWindowSurfaceList;
    auto &ExtSolWinWithFrameSurfaces = state->dataSurface->AllExtSolWinWithFrameSurfaceList;
    auto &HTKivaSurfaces = state->dataSurface->AllHTKivaSurfaceList;

    int thisSurface = wallEastGable;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = doorWestDoor;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = windowSouthWindow;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_FALSE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_TRUE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = windowTubularDaylightingDiffuser1;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_FALSE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_TRUE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = nonwindowTubularDaylightingDome1;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_TRUE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());
}

TEST_F(EnergyPlusFixture, GetSurfaceData_SurfaceOrder4)
{

    // Last zone, ATTIC ZONE, has no surfaces

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    AIR,                     !- Gas Type",
        "    0.006;                   !- Thickness {m}",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    INTERIORWall,            !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN02,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:GARAGE,            !- Name",
        "    CC03;                    !- Outside Layer",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Construction,",
        "    EXTWALL:GARAGE,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11;                    !- Layer 2",

        "  Construction,",
        "    CEILING:LIVING,          !- Name",
        "    IN05,                    !- Outside Layer",
        "    GP01;                    !- Layer 2",

        "  Construction,",
        "    reverseCEILING:LIVING,   !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN05;                    !- Layer 2",

        "  Construction,",
        "    GABLE,                   !- Name",
        "    PW03;                    !- Outside Layer",

        "  Construction,",
        "    Dbl Clr 3mm/6mm Air,     !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 6MM,                 !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "  Construction,",
        "    Garage:SteelDoor,        !- Name",
        "    HF-A3;                   !- Outside Layer",

        "  Construction,",
        "    CEILING:Garage,          !- Name",
        "    GP01;                    !- Outside Layer",

        "  Zone,",
        "    LIVING ZONE;             !- Name",

        "  Zone,",
        "    GARAGE ZONE;             !- Name",

        "  Zone,",
        "    ATTIC ZONE;              !- Name",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  WindowMaterial:Glazing,",
        "    Clear Acrylic Plastic,   !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.92,                    !- Solar Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.92,                    !- Visible Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",

        "  WindowMaterial:Glazing,",
        "    Diffusing Acrylic Plastic,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.0022,                  !- Thickness {m}",
        "    0.90,                    !- Solar Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.90,                    !- Visible Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",
        "",
        "  Material,",
        "    Very High Reflectivity Surface,  !- Name",
        "    Smooth,                  !- Roughness",
        "    0.0005,                  !- Thickness {m}",
        "    237,                     !- Conductivity {W/m-K}",
        "    2702,                    !- Density {kg/m3}",
        "    903,                     !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.05,                    !- Solar Absorptance",
        "    0.05;                    !- Visible Absorptance",
        "  Construction,",
        "    TDD Pipe,                !- Name",
        "    Very High Reflectivity Surface;  !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Dome,                !- Name",
        "    Clear Acrylic Plastic;   !- Outside Layer",
        "",
        "  Construction,",
        "    TDD Diffuser,            !- Name",
        "    Diffusing Acrylic Plastic;  !- Outside Layer",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4383,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Interior,         !- Name",
        "    WALL,                    !- Surface Type",
        "    INTERIORWall,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Interior,         !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Interior,         !- Name",
        "    WALL,                    !- Surface Type",
        "    INTERIORWall,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Interior,         !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:EastWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:WestWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  InternalMass,",
        "    EVChargingStation,      !- Name",
        "    Garage:SteelDoor,     !- Construction Name",
        "    GARAGE ZONE,      !- Zone or ZoneList Name",
        "    ,                        !- Space Name",
        "    88.249272671219;         !- Surface Area {m2}",

        "  BuildingSurface:Detailed,",
        "    Garage:FrontDoor,        !- Name",
        "    WALL,                    !- Surface Type",
        "    Garage:SteelDoor,        !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:GARAGE,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    NorthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:North,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    6.572,10.778,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    6.572,10.778,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2,10.778,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2,10.778,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    EastWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:East,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    17.242,2,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,2,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,6.572,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,6.572,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    SouthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:South,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2,0,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2,0,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.572,0,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.572,0,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,6.572,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,6.572,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,2,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,2,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestDoor,              !- Name",
        "    Door,                  !- Surface Type",
        "    Garage:SteelDoor,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,9.0,2.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,9.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,8.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,8.0,2.0;  !- X,Y,Z ==> Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                  // expect no errors

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors, just warnings
    std::string const error_string = delimited_string(
        {"   ** Warning ** createSpaceSurfaceLists: Space=ATTIC ZONE has no surfaces.",
         "   ** Warning ** No floor exists in Zone=\"ATTIC ZONE\", zone floor area is zero. All values for this zone that are entered per floor "
         "area will be zero.",
         "   ** Warning ** Indicated Zone Volume <= 0.0 for Zone=ATTIC ZONE",
         "   **   ~~~   ** The calculated Zone Volume was=0.00",
         "   **   ~~~   ** The simulation will continue with the Zone Volume set to 10.0 m3. ",
         "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual zones.",
         "   ** Warning ** CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; "});

    compare_err_stream(error_string);

    // This tests the space surface first/last values after surfaces are sorted:
    //
    // Simulation Order:
    //   All shading surfaces (including mirrored surfaces)
    //      Shading:Site
    //      Shading:Building
    //      Shading:Zone (and variants)
    //
    //   By zone (in the order the zones appear in the idf):
    //      Walls
    //      Floors
    //      Roofs
    //      Internal Mass
    //      Doors
    //      Windows
    //      TDD_Domes
    //
    // Special cases:
    //      TubularDaylightDome is treated as a separate surface type
    //      TubularDaylightDiffuser is treated as a window subsurface

    // For this test, the order should be
    // Simulation Order (1-based):
    //  LIVING ZONE:
    //      1. Living:North (wall)
    //      2. Living:East (wall)
    //      3. Living:South (wall)
    //      4. Living:West (wall)
    //      5. Living:Interior (wall)
    //      6. Living:Ceiling (ceiling)
    //      7. Living:Floor (floor)
    //      8. WestDoor (door)
    //      9. NorthWindow (window)
    //      10. EastWindow (window)
    //      11. SouthWindow (window)
    //      12. WestWindow (window)
    //  GARAGE ZONE:
    //      13. Garage:Interior (wall)
    //      14. Garage:EastWall (wall)
    //      15. Garage:WestWall (wall)
    //      16. Garage:FrontDoor (wall)
    //      17. Garage:Floor (floor)
    //      18. EVChargingStation (internal mass)
    //
    // Simulation Order (1-based):

    //  LIVING ZONE:
    int wallLivingNorth = Util::FindItemInList(Util::makeUPPER("Living:North"), state->dataSurface->Surface);
    int wallLivingEast = Util::FindItemInList(Util::makeUPPER("Living:East"), state->dataSurface->Surface);
    int wallLivingSouth = Util::FindItemInList(Util::makeUPPER("Living:South"), state->dataSurface->Surface);
    int wallLivingWest = Util::FindItemInList(Util::makeUPPER("Living:West"), state->dataSurface->Surface);
    int wallLivingInterior = Util::FindItemInList(Util::makeUPPER("Living:Interior"), state->dataSurface->Surface);
    int floorLivingFloor = Util::FindItemInList(Util::makeUPPER("Living:Floor"), state->dataSurface->Surface);
    int ceilingLivingCeiling = Util::FindItemInList(Util::makeUPPER("Living:Ceiling"), state->dataSurface->Surface);
    int doorWestDoor = Util::FindItemInList(Util::makeUPPER("WestDoor"), state->dataSurface->Surface);
    int windowNorthWindow = Util::FindItemInList(Util::makeUPPER("NorthWindow"), state->dataSurface->Surface);
    int windowEastWindow = Util::FindItemInList(Util::makeUPPER("EastWindow"), state->dataSurface->Surface);
    int windowSouthWindow = Util::FindItemInList(Util::makeUPPER("SouthWindow"), state->dataSurface->Surface);
    int windowWestWindow = Util::FindItemInList(Util::makeUPPER("WestWindow"), state->dataSurface->Surface);

    EXPECT_EQ(wallLivingNorth, 1);
    EXPECT_EQ(wallLivingEast, 2);
    EXPECT_EQ(wallLivingSouth, 3);
    EXPECT_EQ(wallLivingWest, 4);
    EXPECT_EQ(wallLivingInterior, 5);
    EXPECT_EQ(floorLivingFloor, 6);
    EXPECT_EQ(ceilingLivingCeiling, 7);
    EXPECT_EQ(doorWestDoor, 8);
    EXPECT_EQ(windowNorthWindow, 9);
    EXPECT_EQ(windowEastWindow, 10);
    EXPECT_EQ(windowSouthWindow, 11);
    EXPECT_EQ(windowWestWindow, 12);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceFirst, 1);
    EXPECT_EQ(state->dataHeatBal->space(1).HTSurfaceLast, 12);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst, 1);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast, 8);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceFirst, 9);
    EXPECT_EQ(state->dataHeatBal->space(1).WindowSurfaceLast, 12);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrWinSurfaceFirst, 1);
    EXPECT_EQ(state->dataHeatBal->space(1).OpaqOrWinSurfaceLast, 12);

    //  GARAGE ZONE:
    int wallGarageInterior = Util::FindItemInList(Util::makeUPPER("Garage:Interior"), state->dataSurface->Surface);
    int wallGarageEast = Util::FindItemInList(Util::makeUPPER("Garage:EastWall"), state->dataSurface->Surface);
    int wallGarageWest = Util::FindItemInList(Util::makeUPPER("Garage:WestWall"), state->dataSurface->Surface);
    int wallGarageFrontDoor = Util::FindItemInList(Util::makeUPPER("Garage:FrontDoor"), state->dataSurface->Surface);
    int floorGarageFloor = Util::FindItemInList(Util::makeUPPER("Garage:Floor"), state->dataSurface->Surface);
    int intmassEVChargingStation = Util::FindItemInList(Util::makeUPPER("EVChargingStation"), state->dataSurface->Surface);

    EXPECT_EQ(wallGarageInterior, 13);
    EXPECT_EQ(wallGarageEast, 14);
    EXPECT_EQ(wallGarageWest, 15);
    EXPECT_EQ(wallGarageFrontDoor, 16);
    EXPECT_EQ(floorGarageFloor, 17);
    EXPECT_EQ(intmassEVChargingStation, 18);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceFirst, 13);
    EXPECT_EQ(state->dataHeatBal->space(2).HTSurfaceLast, 18);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrIntMassSurfaceFirst, 13);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrIntMassSurfaceLast, 18);
    EXPECT_EQ(state->dataHeatBal->space(2).WindowSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(2).WindowSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrWinSurfaceFirst, 13);
    EXPECT_EQ(state->dataHeatBal->space(2).OpaqOrWinSurfaceLast, 18);

    //  ATTIC ZONE:
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(3).HTSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrIntMassSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(3).WindowSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(3).OpaqOrWinSurfaceLast, -1);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeFirst, 0);
    EXPECT_EQ(state->dataHeatBal->space(3).TDDDomeLast, -1);

    // Extend test to check other surface lists
    auto &HTSurfaces = state->dataSurface->AllHTSurfaceList;
    auto &ExtSolarSurfaces = state->dataSurface->AllExtSolarSurfaceList;
    auto &ExtSolAndShadingSurfaces = state->dataSurface->AllExtSolAndShadingSurfaceList;
    auto &ShadowPossObstrSurfaces = state->dataSurface->AllShadowPossObstrSurfaceList;
    auto &IZSurfaces = state->dataSurface->AllIZSurfaceList;
    auto &HTNonWindowSurfaces = state->dataSurface->AllHTNonWindowSurfaceList;
    auto &HTWindowSurfaces = state->dataSurface->AllHTWindowSurfaceList;
    auto &ExtSolWindowSurfaces = state->dataSurface->AllExtSolWindowSurfaceList;
    auto &ExtSolWinWithFrameSurfaces = state->dataSurface->AllExtSolWinWithFrameSurfaceList;
    auto &HTKivaSurfaces = state->dataSurface->AllHTKivaSurfaceList;

    int thisSurface = doorWestDoor;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = wallGarageInterior;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_TRUE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_TRUE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_FALSE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());

    thisSurface = windowSouthWindow;
    EXPECT_TRUE(std::find(HTSurfaces.begin(), HTSurfaces.end(), thisSurface) != HTSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolarSurfaces.begin(), ExtSolarSurfaces.end(), thisSurface) != ExtSolarSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolAndShadingSurfaces.begin(), ExtSolAndShadingSurfaces.end(), thisSurface) != ExtSolAndShadingSurfaces.end());
    EXPECT_FALSE(std::find(ShadowPossObstrSurfaces.begin(), ShadowPossObstrSurfaces.end(), thisSurface) != ShadowPossObstrSurfaces.end());
    EXPECT_FALSE(std::find(IZSurfaces.begin(), IZSurfaces.end(), thisSurface) != IZSurfaces.end());
    EXPECT_FALSE(std::find(HTNonWindowSurfaces.begin(), HTNonWindowSurfaces.end(), thisSurface) != HTNonWindowSurfaces.end());
    EXPECT_TRUE(std::find(HTWindowSurfaces.begin(), HTWindowSurfaces.end(), thisSurface) != HTWindowSurfaces.end());
    EXPECT_TRUE(std::find(ExtSolWindowSurfaces.begin(), ExtSolWindowSurfaces.end(), thisSurface) != ExtSolWindowSurfaces.end());
    EXPECT_FALSE(std::find(ExtSolWinWithFrameSurfaces.begin(), ExtSolWinWithFrameSurfaces.end(), thisSurface) != ExtSolWinWithFrameSurfaces.end());
    EXPECT_FALSE(std::find(HTKivaSurfaces.begin(), HTKivaSurfaces.end(), thisSurface) != HTKivaSurfaces.end());
}

TEST_F(EnergyPlusFixture, Use_Gross_Roof_Area_for_Averge_Height)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

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
        "0.7500000;               !-Visible Absorptance",

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

        "BuildingSurface:Detailed,",
        "    00_Floor,                !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,               !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000, 0, 0.0,                   !- X,Y,Z  1 {m}",
        "    0, 4, 0.0,                          !- X,Y,Z  2 {m}",
        "    8, 4, 0,                            !- X,Y,Z  3 {m}",
        "    8, 0, 0;                            !- X,Y,Z  4 {m}",

        "    BuildingSurface:Detailed,",
        "    01Wall_E,                !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    8, 0, 2.5,                          !- X,Y,Z  1 {m}",
        "    8, 0, 0,                            !- X,Y,Z  2 {m}",
        "    8, 4, 0,                            !- X,Y,Z  3 {m}",
        "    8, 4, 2.5;                          !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "02Wall_S, !-Name",
        "Wall, !-Surface Type",
        "R13WALL, !-Construction Name",
        "ZONE ONE, !-Zone Name",
        ",         !- Space Name",
        "Outdoors, !-Outside Boundary Condition",
        ", !-Outside Boundary Condition Object",
        "SunExposed, !-Sun Exposure",
        "WindExposed, !-Wind Exposure",
        "0.5000000, !-View Factor to Ground",
        "6, !-Number of Vertices",
        "0, 0, 2.5, !-X, Y, Z 1{m}",
        "0, 0, 0, !-X, Y, Z 2{m}",
        "8, 0, 0, !-X, Y, Z 3{m}",
        "8, 0, 2.5, !-X, Y, Z 4{m}",
        "6, 0, 4, !-X, Y, Z 5{m}",
        "2, 0, 4; !-X, Y, Z 6{m}",

        "BuildingSurface:Detailed,",
        "    03Wall_W,                !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0, 4, 2.5,                          !- X,Y,Z  1 {m}",
        "    0, 4, 0,                            !- X,Y,Z  2 {m}",
        "    0, 0, 0,                            !- X,Y,Z  3 {m}",
        "    0, 0, 2.5;                          !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    04Wall_N,                !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    6,                       !- Number of Vertices",
        "    8, 4, 2.5,                          !- X,Y,Z  1 {m}",
        "    8, 4, 0,                            !- X,Y,Z  2 {m}",
        "    0, 4, 0,                            !- X,Y,Z  3 {m}",
        "    0, 4, 2.5,                          !- X,Y,Z  4 {m}",
        "    2, 4, 4,                         !- X,Y,Z  5 {m}",
        "    6, 4, 4;                         !- X,Y,Z  6 {m}",

        "BuildingSurface:Detailed,",
        "    05Roof_C,                !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    2, 4, 4,                         !- X,Y,Z  1 {m}",
        "    2, 0, 4,                         !- X,Y,Z  2 {m}",
        "    6, 0, 4,                         !- X,Y,Z  3 {m}",
        "    6, 4, 4;                         !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "06Roof_E,                !- Name",
        "Roof,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "autocalculate,           !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "6, 0, 4,                         !- X,Y,Z  1 {m}",
        "8, 0, 2.5,                          !- X,Y,Z  2 {m}",
        "8, 4, 2.5,                          !- X,Y,Z  3 {m}",
        "6, 4, 4;                         !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "07Roof_W,                !- Name",
        "Roof,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "autocalculate,           !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "2, 4, 4,                         !- X,Y,Z  1 {m}",
        "0, 4, 2.5,                          !- X,Y,Z  2 {m}",
        "0, 0, 2.5,                          !- X,Y,Z  3 {m}",
        "2, 0, 4;                         !- X,Y,Z  4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetSurfaceData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // Center
    EXPECT_NEAR(state->dataSurface->Surface(6).GrossArea, 16.0, 1e-6);
    EXPECT_NEAR(state->dataSurface->Surface(6).Area, 16.0, 1e-6);

    // East
    EXPECT_NEAR(state->dataSurface->Surface(7).GrossArea, 10.0, 1e-6);
    EXPECT_NEAR(state->dataSurface->Surface(7).Area, 10.0, 1e-6);

    // West
    EXPECT_NEAR(state->dataSurface->Surface(8).GrossArea, 10.0, 1e-6);
    EXPECT_NEAR(state->dataSurface->Surface(8).Area, 10.0, 1e-6);

    Real64 totalGrossCeilingArea = 0.0;
    totalGrossCeilingArea =
        state->dataSurface->Surface(6).GrossArea + state->dataSurface->Surface(7).GrossArea + state->dataSurface->Surface(8).GrossArea;

    Real64 totalNetCeilingArea = 0.0;
    totalNetCeilingArea = state->dataSurface->Surface(6).Area + state->dataSurface->Surface(7).Area + state->dataSurface->Surface(8).Area;

    Real64 ceilingHeight_expected = 0.0;
    ceilingHeight_expected = 3.25 * (state->dataSurface->Surface(7).GrossArea + state->dataSurface->Surface(8).GrossArea) / totalGrossCeilingArea +
                             4.0 * state->dataSurface->Surface(6).GrossArea / totalGrossCeilingArea;

    EXPECT_NEAR(state->dataHeatBal->Zone(1).CeilingHeight, ceilingHeight_expected, 1e-6);
}

TEST_F(EnergyPlusFixture, Use_Gross_Roof_Area_for_Averge_Height_with_Window)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0, 0, 0,                            !- X,Y,Z  {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

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
        "0.7500000;               !-Visible Absorptance",

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

        "WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "WindowMaterial:Glazing,",
        "    GREY 3MM,                !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.626,                   !- Solar Transmittance at Normal Incidence",
        "    0.061,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.061,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.611,                   !- Visible Transmittance at Normal Incidence",
        "    0.061,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.061,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "WindowMaterial:Gas,",
        "    AIR 13MM,                !- Name",
        "    Air,                     !- Gas Type",
        "    0.0127;                  !- Thickness {m}",

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
        "    Dbl Clr 3mm/13mm Air,    !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 13MM,                !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "BuildingSurface:Detailed,",
        "    00_Floor,                !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,               !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000, 0, 0.0,                   !- X,Y,Z  1 {m}",
        "    0, 4, 0.0,                          !- X,Y,Z  2 {m}",
        "    8, 4, 0,                            !- X,Y,Z  3 {m}",
        "    8, 0, 0;                            !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    01Wall_E,                !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    8, 0, 2.5,                          !- X,Y,Z  1 {m}",
        "    8, 0, 0,                            !- X,Y,Z  2 {m}",
        "    8, 4, 0,                            !- X,Y,Z  3 {m}",
        "    8, 4, 2.5;                          !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "02Wall_S, !-Name",
        "Wall, !-Surface Type",
        "R13WALL, !-Construction Name",
        "ZONE ONE, !-Zone Name",
        ",         !- Space Name",
        "Outdoors, !-Outside Boundary Condition",
        ", !-Outside Boundary Condition Object",
        "SunExposed, !-Sun Exposure",
        "WindExposed, !-Wind Exposure",
        "0.5000000, !-View Factor to Ground",
        "6, !-Number of Vertices",
        "0, 0, 2.5, !-X, Y, Z 1{m}",
        "0, 0, 0, !-X, Y, Z 2{m}",
        "8, 0, 0, !-X, Y, Z 3{m}",
        "8, 0, 2.5, !-X, Y, Z 4{m}",
        "6, 0, 4, !-X, Y, Z 5{m}",
        "2, 0, 4; !-X, Y, Z 6{m}",

        "BuildingSurface:Detailed,",
        "    03Wall_W,                !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0, 4, 2.5,                          !- X,Y,Z  1 {m}",
        "    0, 4, 0,                            !- X,Y,Z  2 {m}",
        "    0, 0, 0,                            !- X,Y,Z  3 {m}",
        "    0, 0, 2.5;                          !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "    04Wall_N,                !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    6,                       !- Number of Vertices",
        "    8, 4, 2.5,                          !- X,Y,Z  1 {m}",
        "    8, 4, 0,                            !- X,Y,Z  2 {m}",
        "    0, 4, 0,                            !- X,Y,Z  3 {m}",
        "    0, 4, 2.5,                          !- X,Y,Z  4 {m}",
        "    2, 4, 4,                         !- X,Y,Z  5 {m}",
        "    6, 4, 4;                         !- X,Y,Z  6 {m}",

        "BuildingSurface:Detailed,",
        "    05Roof_C,                !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    2, 4, 4,                         !- X,Y,Z  1 {m}",
        "    2, 0, 4,                         !- X,Y,Z  2 {m}",
        "    6, 0, 4,                         !- X,Y,Z  3 {m}",
        "    6, 4, 4;                         !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "06Roof_E,                !- Name",
        "Roof,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "autocalculate,           !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "6, 0, 4,                         !- X,Y,Z  1 {m}",
        "8, 0, 2.5,                          !- X,Y,Z  2 {m}",
        "8, 4, 2.5,                          !- X,Y,Z  3 {m}",
        "6, 4, 4;                         !- X,Y,Z  4 {m}",

        "BuildingSurface:Detailed,",
        "07Roof_W,                !- Name",
        "Roof,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "autocalculate,           !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "2, 4, 4,                         !- X,Y,Z  1 {m}",
        "0, 4, 2.5,                          !- X,Y,Z  2 {m}",
        "0, 0, 2.5,                          !- X,Y,Z  3 {m}",
        "2, 0, 4;                         !- X,Y,Z  4 {m}",

        "FenestrationSurface:Detailed,",
        "    Skylight_C,              !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Dbl Clr 3mm/13mm Air,    !- Construction Name",
        "    05Roof_C,                !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    autocalculate,           !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.2, 0.2, 4,                         !- X,Y,Z  1 {m}",
        "    5.8, 0.2, 4,                         !- X,Y,Z  2 {m}",
        "    5.8, 3.8, 4,                         !- X,Y,Z  3 {m}",
        "    2.2, 3.8, 4;                         !- X,Y,Z  4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetSurfaceData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // Center
    EXPECT_NEAR(state->dataSurface->Surface(6).GrossArea, 16.0, 1e-6);
    EXPECT_NEAR(state->dataSurface->Surface(6).Area, 3.04, 1e-6);

    // East
    EXPECT_NEAR(state->dataSurface->Surface(7).GrossArea, 10.0, 1e-6);
    EXPECT_NEAR(state->dataSurface->Surface(7).Area, 10.0, 1e-6);

    // West
    EXPECT_NEAR(state->dataSurface->Surface(8).GrossArea, 10.0, 1e-6);
    EXPECT_NEAR(state->dataSurface->Surface(8).Area, 10.0, 1e-6);

    Real64 totalGrossCeilingArea = 0.0;
    totalGrossCeilingArea =
        state->dataSurface->Surface(6).GrossArea + state->dataSurface->Surface(7).GrossArea + state->dataSurface->Surface(8).GrossArea;

    Real64 totalNetCeilingArea = 0.0;
    totalNetCeilingArea = state->dataSurface->Surface(6).Area + state->dataSurface->Surface(7).Area + state->dataSurface->Surface(8).Area;

    Real64 ceilingHeight_expected = 0.0;
    ceilingHeight_expected = 3.25 * (state->dataSurface->Surface(7).GrossArea + state->dataSurface->Surface(8).GrossArea) / totalGrossCeilingArea +
                             4.0 * state->dataSurface->Surface(6).GrossArea / totalGrossCeilingArea;

    Real64 ceilingHeight_old = 0.0;
    ceilingHeight_old = 3.25 * (state->dataSurface->Surface(7).Area + state->dataSurface->Surface(8).Area) / totalNetCeilingArea +
                        4.0 * state->dataSurface->Surface(6).Area / totalNetCeilingArea;

    EXPECT_NEAR(state->dataHeatBal->Zone(1).CeilingHeight, ceilingHeight_expected, 1e-6);
    EXPECT_NE(state->dataHeatBal->Zone(1).CeilingHeight, ceilingHeight_old);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_GetKivaFoundationTest)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Foundation:Kiva:Settings,",
        "  1.8,                     !- Soil Conductivity {W / m - K}",
        "  3200,                    !- Soil Density {kg / m3}",
        "  836,                     !- Soil Specific Heat {J / kg - K}",
        "  0.9,                     !- Ground Solar Absorptivity {dimensionless}",
        "  0.9,                     !- Ground Thermal Absorptivity {dimensionless}",
        "  0.03,                    !- Ground Surface Roughness {m}",
        "  40,                      !- Far - Field Width {m}",
        "  ZeroFlux,                !- Deep - Ground Boundary Condition",
        "  AutoCalculate;           !- Deep - Ground Depth",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetFoundationData(*state, ErrorsFound);
    std::string const error_string = delimited_string({
        "   ** Severe  ** Foundation:Kiva:Settings, Deep-Ground Depth should not be set to Autocalculate unless Deep-Ground Boundary Condition is "
        "set to Autoselect",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_GetKivaFoundationTest2)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Foundation:Kiva:Settings,",
        "  1.8,                     !- Soil Conductivity {W / m - K}",
        "  3200,                    !- Soil Density {kg / m3}",
        "  836,                     !- Soil Specific Heat {J / kg - K}",
        "  0.9,                     !- Ground Solar Absorptivity {dimensionless}",
        "  0.9,                     !- Ground Thermal Absorptivity {dimensionless}",
        "  0.03,                    !- Ground Surface Roughness {m}",
        "  40,                      !- Far-Field Width {m}",
        "  Autoselect,              !- Deep-Ground Boundary Condition",
        "  20;                      !- Deep-Ground Depth",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->Elevation = 600.;

    GetFoundationData(*state, ErrorsFound);
    std::string const error_string =
        delimited_string({"   ** Warning ** Foundation:Kiva:Settings, when Deep-Ground Boundary Condition is Autoselect,\n"
                          "   **   ~~~   ** the user-specified Deep-Ground Depth (20.0 m)\n"
                          "   **   ~~~   ** will be overridden with the Autoselected depth (40.0 m)"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}
TEST_F(EnergyPlusFixture, SurfaceGeometry_GetKivaFoundationTest3)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Material,",
        "  exterior vertical ins,                  !- Name",
        "  Rough,                                  !- Roughness",
        "  0.04611624,                             !- Thickness {m}",
        "  0.029427,                               !- Conductivity {W/m-K}",
        "  32.04,                                  !- Density {kg/m3}",
        "  1214.23,                                !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Foundation:Kiva,",
        "  Foundation Kiva 1,                      !- Name",
        "  20,                                     !- Initial Indoor Air Temperature {C}",
        "  ,                                       !- Interior Horizontal Insulation Material Name",
        "  ,                                       !- Interior Horizontal Insulation Depth {m}",
        "  ,                                       !- Interior Horizontal Insulation Width {m}",
        "  ,                                       !- Interior Vertical Insulation Material Name",
        "  ,                                       !- Interior Vertical Insulation Depth {m}",
        "  ,                                       !- Exterior Horizontal Insulation Material Name",
        "  ,                                       !- Exterior Horizontal Insulation Depth {m}",
        "  ,                                       !- Exterior Horizontal Insulation Width {m}",
        "  ,                                       !- Exterior Vertical Insulation Material Name",
        "  ,                                       !- Exterior Vertical Insulation Depth {m}",
        "  0.3048,                                 !- Wall Height Above Grade {m}",
        "  0.2032,                                 !- Wall Depth Below Slab {m}",
        "  ,                                       !- Footing Wall Construction Name",
        "  ,                                       !- Footing Material Name",
        "  ,                                       !- Footing Depth {m}",
        "  exterior vertical ins,                  !- Custom Block Material Name 1",
        "  2.4384,                                 !- Custom Block Depth 1 {m}",
        "  0.2159,                                 !- Custom Block X Position 1 {m}",
        "  0;                                      !- Custom Block Z Position 1 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataEnvrn->Elevation = 600.;

    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetFoundationData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
}
TEST_F(EnergyPlusFixture, SurfaceGeometry_ZoneAndSpaceAreas)
{

    // Test for issue #9302 and beyond - User input Zone Floor Area not used for Space Floor Area

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1,             !- Name",
        "    90,                      !- Direction of Relative North {deg}",
        "    16.12975,                !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    ,                        !- Type",
        "    1,                       !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    ,                        !- Volume {m3}",
        "    30.0;                     !- Floor Area {m2}",

        "Zone,",
        "Zone 2,             !- Name",
        "    90,                      !- Direction of Relative North {deg}",
        "    16.12975,                !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    ,                        !- Type",
        "    1,                       !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    ,                        !- Volume {m3}",
        "    20.0;                     !- Floor Area {m2}",

        "Zone,",
        "Zone 3;             !- Name",

        "Space,",
        "Space 1a,            !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 1b,            !- Name",
        "Zone 1;             !- Zone Name",

        "Space,",
        "Space 3,            !- Name",
        "Zone 3,             !- Zone Name",
        ",                   !- Ceiling Height {m}",
        ",                   !- Volume {m3}",
        "5.0;                !- Floor Area {m2}",

        "Material,",
        "    Some Material,         !- Name",
        "    VeryRough,               !- Roughness",
        "    0.006,                   !- Thickness {m}",
        "    0.815,                   !- Conductivity {W/m-K}",
        "    929,                     !- Density {kg/m3}",
        "    3140,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",
        "Construction,",
        "    Some Construction,  !- Name",
        "    Some Material;        !- Outside Layer",
        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1a,                 !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    SunExposed,                   !- Sun Exposure",
        "    WindExposed,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone1-Floor2,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 1,       !- Zone Name",
        "    Space 1b,                 !- Space Name",
        "    Outdoors,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    SunExposed,                   !- Sun Exposure",
        "    WindExposed,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,2,0,              !- Vertex 2",
        "    1,2,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone2-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 2,       !- Zone Name",
        "    ,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

        "BuildingSurface:Detailed,",
        "    Zone3-Floor,  !- Name",
        "    Floor,                 !- Surface Type",
        "    Some Construction,  !- Construction Name",
        "    Zone 3,       !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Ground,                 !- Outside Boundary Condition",
        "    ,  !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,              !- Vertex 1",
        "    0,1,0,              !- Vertex 2",
        "    1,1,0,              !- Vertex 3",
        "    1,0,0;              !- Vertex 4",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    // Zone 1 with user-entered floor area of 30.0 consists of Space 1 and Space 2
    // Space 1 has a floor surface of area 1.0, user-entered floor area is blank
    // Space 2 has a floor surface of area 2.0, user-entered floor area is blank
    EXPECT_EQ(state->dataHeatBal->space(1).Name, "SPACE 1A");
    EXPECT_NEAR(state->dataHeatBal->space(1).userEnteredFloorArea, Constant::AutoCalculate, 0.001);
    EXPECT_NEAR(state->dataHeatBal->space(1).FloorArea, 10.0, 0.001);

    EXPECT_EQ(state->dataHeatBal->space(2).Name, "SPACE 1B");
    EXPECT_NEAR(state->dataHeatBal->space(2).userEnteredFloorArea, Constant::AutoCalculate, 0.001);
    EXPECT_NEAR(state->dataHeatBal->space(2).FloorArea, 20.0, 0.001);

    EXPECT_EQ(state->dataHeatBal->Zone(1).Name, "ZONE 1");
    EXPECT_NEAR(state->dataHeatBal->Zone(1).UserEnteredFloorArea, 30.0, 0.001);
    EXPECT_NEAR(state->dataHeatBal->Zone(1).FloorArea, 30.0, 0.001);
    Real64 zone1Area = state->dataHeatBal->space(1).FloorArea + state->dataHeatBal->space(2).FloorArea;
    EXPECT_NEAR(state->dataHeatBal->Zone(1).FloorArea, zone1Area, 0.001);

    // Zone 3 consists of Space 3, user-entered floor area is blank
    // Space 3 has a floor surface of area 1.0, user-entered floor is 5.0
    EXPECT_EQ(state->dataHeatBal->Zone(3).Name, "ZONE 3");
    EXPECT_NEAR(state->dataHeatBal->Zone(3).UserEnteredFloorArea, Constant::AutoCalculate, 0.001);
    EXPECT_NEAR(state->dataHeatBal->Zone(3).FloorArea, 5.0, 0.001);
    EXPECT_EQ(state->dataHeatBal->space(3).Name, "SPACE 3");
    EXPECT_NEAR(state->dataHeatBal->space(3).userEnteredFloorArea, 5.0, 0.001);
    EXPECT_NEAR(state->dataHeatBal->space(3).FloorArea, 5.0, 0.001);

    // Zone 2 consists of auto-generated Space 4, user-entered floor area is 20.0
    // Space 4 has a floor surface of area 1.0, user-entered floor is blank
    EXPECT_EQ(state->dataHeatBal->Zone(2).Name, "ZONE 2");
    EXPECT_NEAR(state->dataHeatBal->Zone(2).UserEnteredFloorArea, 20.0, 0.001);
    EXPECT_NEAR(state->dataHeatBal->Zone(2).FloorArea, 20.0, 0.001);
    EXPECT_EQ(state->dataHeatBal->space(4).Name, "ZONE 2");
    EXPECT_NEAR(state->dataHeatBal->space(4).userEnteredFloorArea, Constant::AutoCalculate, 0.001);
    EXPECT_NEAR(state->dataHeatBal->space(4).FloorArea, 20.0, 0.001);
}

TEST_F(EnergyPlusFixture, ZoneFloorAreaTest)
{
    // Issue 9515
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "FenestrationSurface:Detailed,                                   ",
        "	Surface-1-Rectangle,     !- Name                             ",
        "	Window,                  !- Surface Type                     ",
        "	SINGLE PANE HW WINDOW,   !- Construction Name                ",
        "	WallExample,             !- Building Surface Name            ",
        "	,                        !- Outside Boundary Condition Object",
        "	0.50000,                 !- View Factor to Ground            ",
        "	,                        !- Frame and Divider Name           ",
        "	1,                       !- Multiplier                       ",
        "	4,                       !- Number of Vertices               ",
        "	0.0, 11.4, 2.1,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 11.4, 0.9,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 3.8,  0.9,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 3.8,  2.1;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "                                                                ",
        "FenestrationSurface:Detailed,                                   ",
        "	Surface-2-Trapzoid,      !- Name                             ",
        "	Window,                  !- Surface Type                     ",
        "	SINGLE PANE HW WINDOW,   !- Construction Name                ",
        "	WallExample,             !- Building Surface Name            ",
        "	,                        !- Outside Boundary Condition Object",
        "	0.50000,                 !- View Factor to Ground            ",
        "	,                        !- Frame and Divider Name           ",
        "	1,                       !- Multiplier                       ",
        "	4,                       !- Number of Vertices               ",
        "	0.0, 11.2, 2.1,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 11.6, 0.9,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 3.6,  0.9,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 4.0,  2.1;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "                                                                ",
        "FenestrationSurface:Detailed,                                   ",
        "	Surface-3-parallelogram, !- Name                             ",
        "	Window,                  !- Surface Type                     ",
        "	SINGLE PANE HW WINDOW,   !- Construction Name                ",
        "	WallExample,             !- Building Surface Name            ",
        "	,                        !- Outside Boundary Condition Object",
        "	0.50000,                 !- View Factor to Ground            ",
        "	,                        !- Frame and Divider Name           ",
        "	1,                       !- Multiplier                       ",
        "	4,                       !- Number of Vertices               ",
        "	0.0, 11.4, 2.1,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 12.4, 0.9,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 4.8,  0.9,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 3.8,  2.1;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "                                                                ",
        "BuildingSurface:Detailed,                                       ",
        "	WallExample,   !- Name                                       ",
        "	Wall,                    !- Surface Type                     ",
        "	ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name     ",
        "	ZoneExample,             !- Zone Name                        ",
        "   ,                        !- Space Name",
        "	Outdoors,                !- Outside Boundary Condition       ",
        "	,                        !- Outside Boundary Condition Object",
        "	NoSun,                   !- Sun Exposure                     ",
        "	NoWind,                  !- Wind Exposure                    ",
        "	,                        !- View Factor to Ground            ",
        "	,                        !- Number of Vertices               ",
        "	0.0, 15.2, 2.4,          !- X,Y,Z ==> Vertex 1 {m}           ",
        "	0.0, 15.2, 0.0,          !- X,Y,Z ==> Vertex 2 {m}           ",
        "	0.0, 0.0,  0.0,          !- X,Y,Z ==> Vertex 3 {m}           ",
        "	0.0, 0.0,  2.4;          !- X,Y,Z ==> Vertex 4 {m}           ",
        "	                                                             ",
        "Floor:Adiabatic,                                       ",
        "	FloorExample,            !- Name                             ",
        "	ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name     ",
        "	ZoneExample,             !- Zone Name                        ",
        "   ,                        !- Space Name",
        "	0,                       !- Azimuth Angle {deg} ",
        "       180,                     !- Tilt Angle {deg}    ",
        "       0,                       !- Starting X Coordinate {m}    ",
        "       74,                      !- Starting Y Coordinate {m}    ",
        "       0,                       !- Starting Z Coordinate {m}    ",
        "       1,                       !- Length {m}    ",
        "       1;                       !- Width {m}    ",
        "                                                                ",
        "Zone,                                                           ",
        "	ZoneExample,             !- Name                             ",
        "	0,                       !- Direction of Relative North {deg}",
        "	0.0,                     !- X Origin {m}                     ",
        "	0.0,                     !- Y Origin {m}                     ",
        "	0.0,                     !- Z Origin {m}                     ",
        "	,                        !- Type                             ",
        "	,                        !- Multiplier                       ",
        "	,                        !- Ceiling Height {m}               ",
        "	,                        !- Volume {m3}                      ",
        "	0.0;                        !- Floor Area {m2}                  ",
        "                                                                ",
        "Construction,                                                   ",
        "	ExtSlabCarpet 4in ClimateZone 1-8,  !- Name                  ",
        "	MAT-CC05 4 HW CONCRETE,  !- Outside Layer                    ",
        "	CP02 CARPET PAD;         !- Layer 2                          ",
        "                                                                ",
        "Construction,                                                   ",
        "	SINGLE PANE HW WINDOW,   !- Name                             ",
        "	CLEAR 3MM;  !- Outside Layer                                 ",
        "                                                                ",
        "Material,                                                       ",
        "	MAT-CC05 4 HW CONCRETE,  !- Name                             ",
        "	Rough,                   !- Roughness                        ",
        "	0.1016,                  !- Thickness {m}                    ",
        "	1.311,                   !- Conductivity {W/m-K}             ",
        "	2240,                    !- Density {kg/m3}                  ",
        "	836.800000000001,        !- Specific Heat {J/kg-K}           ",
        "	0.9,                     !- Thermal Absorptance              ",
        "	0.85,                    !- Solar Absorptance                ",
        "	0.85;                    !- Visible Absorptance              ",
        "                                                                ",
        "Material:NoMass,                                                ",
        "	CP02 CARPET PAD,         !- Name                             ",
        "	Smooth,                  !- Roughness                        ",
        "	0.1,                     !- Thermal Resistance {m2-K/W}      ",
        "	0.9,                     !- Thermal Absorptance              ",
        "	0.8,                     !- Solar Absorptance                ",
        "	0.8;                     !- Visible Absorptance              ",
        "                                                                ",
        "WindowMaterial:Glazing,                                                          ",
        "	CLEAR 3MM,               !- Name                                              ",
        "	SpectralAverage,         !- Optical Data Type                                 ",
        "	,                        !- Window Glass Spectral Data Set Name               ",
        "	0.003,                   !- Thickness {m}                                     ",
        "	0.837,                   !- Solar Transmittance at Normal Incidence           ",
        "	0.075,                   !- Front Side Solar Reflectance at Normal Incidence  ",
        "	0.075,                   !- Back Side Solar Reflectance at Normal Incidence   ",
        "	0.898,                   !- Visible Transmittance at Normal Incidence         ",
        "	0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "	0.081,                   !- Back Side Visible Reflectance at Normal Incidence ",
        "	0.0,                     !- Infrared Transmittance at Normal Incidence        ",
        "	0.84,                    !- Front Side Infrared Hemispherical Emissivity      ",
        "	0.84,                    !- Back Side Infrared Hemispherical Emissivity       ",
        "	0.9;                     !- Conductivity {W/m-K}                              ",
    });

    // Prepare data for the test
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);
    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);
    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);
    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data
    EXPECT_FALSE(ErrorsFound);           // expect no errors

    EXPECT_NEAR(state->dataHeatBal->Zone(1).FloorArea, 1.0, 0.001);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_GetSurfaceGroundSurfsTest)
{
    bool ErrorsFound(false);
    std::string const idf_objects =
        delimited_string({"  Material,",
                          "    Concrete Block,               !- Name",
                          "    MediumRough,                  !- Roughness",
                          "    0.1014984,                    !- Thickness {m}",
                          "    0.3805070,                    !- Conductivity {W/m-K}",
                          "    608.7016,                     !- Density {kg/m3}",
                          "    836.8000;                     !- Specific Heat {J/kg-K}",

                          "  Construction,",
                          "    WallConstruction,             !- Name",
                          "    Concrete Block;               !- Outside Layer",

                          "  WindowMaterial:SimpleGlazingSystem,",
                          "    WindowMaterial,               !- Name",
                          "    5.778,                        !- U-Factor {W/m2-K}",
                          "    0.819,                        !- Solar Heat Gain Coefficient",
                          "    0.881;                        !- Visible Transmittance",

                          "  Construction,",
                          "    WindowConstruction,           !- Name",
                          "    WindowMaterial;               !- Outside Layer",

                          "  WindowProperty:FrameAndDivider,",
                          "    WindowFrame,                  !- Name",
                          "    0.05,                         !- Frame Width {m}",
                          "    0.00,                         !- Frame Outside Projection {m}",
                          "    0.00,                         !- Frame Inside Projection {m}",
                          "    5.0,                          !- Frame Conductance {W/m2-K}",
                          "    1.2,                          !- Ratio of Frame-Edge Glass Conductance to Center-Of-Glass Conductance",
                          "    0.8,                          !- Frame Solar Absorptance",
                          "    0.8,                          !- Frame Visible Absorptance",
                          "    0.9,                          !- Frame Thermal Hemispherical Emissivity",
                          "    DividedLite,                  !- Divider Type",
                          "    0.02,                         !- Divider Width {m}",
                          "    2,                            !- Number of Horizontal Dividers",
                          "    2,                            !- Number of Vertical Dividers",
                          "    0.00,                         !- Divider Outside Projection {m}",
                          "    0.00,                         !- Divider Inside Projection {m}",
                          "    5.0,                          !- Divider Conductance {W/m2-K}",
                          "    1.2,                          !- Ratio of Divider-Edge Glass Conductance to Center-Of-Glass Conductance",
                          "    0.8,                          !- Divider Solar Absorptance",
                          "    0.8,                          !- Divider Visible Absorptance",
                          "    0.9;                          !- Divider Thermal Hemispherical Emissivity",

                          "  FenestrationSurface:Detailed,",
                          "    FenestrationSurface,          !- Name",
                          "    Window,                       !- Surface Type",
                          "    WindowConstruction,           !- Construction Name",
                          "    Wall,                         !- Building Surface Name",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    0.5000000,                    !- View Factor to Ground",
                          "    WindowFrame,                  !- Frame and Divider Name",
                          "    1.0,                          !- Multiplier",
                          "    4,                            !- Number of Vertices",
                          "    0.200000,0.0,9.900000,        !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.200000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 2 {m}",
                          "    9.900000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 3 {m}",
                          "    9.900000,0.0,9.900000;        !- X,Y,Z ==> Vertex 4 {m}",

                          "  SurfaceProperty:LocalEnvironment,",
                          "    LocEnv:FenestrationSurface,   !- Name",
                          "    FenestrationSurface,          !- Exterior Surface Name",
                          "    ,                             !- External Shading Fraction Schedule Name",
                          "    SrdSurfs:FenesSurface,        !- Surrounding Surfaces Object Name",
                          "    ,                             !- Outdoor Air Node Name",
                          "    GndSurfs:FenesSurface;        !- Ground Surfaces Object Name",

                          "  SurfaceProperty:SurroundingSurfaces,",
                          "    SrdSurfs:FenesSurface,        !- Name",
                          "    0.5,                          !- Sky View Factor",
                          "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
                          "    ,                             !- Ground View Factor",
                          "    ,                             !- Ground Temperature Schedule Name",
                          "    SrdSurfs:Surface 1,           !- Surrounding Surface 1 Name",
                          "    0.1,                          !- Surrounding Surface 1 View Factor",
                          "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

                          "  Schedule:Compact,",
                          "    Surrounding Temp Sch 1,       !- Name",
                          "    Any Number,                   !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 15.0;           !- Field 3",

                          "  SurfaceProperty:GroundSurfaces,",
                          "    GndSurfs:FenesSurface,        !-Name",
                          "    GndSurfs GrassArea,           !-Ground Surface 1 Name",
                          "    0.2,                          !-Ground Surface 1 View Factor",
                          "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
                          "    ,                             !-Ground Surface 1 Reflectance Schedule Name",
                          "    GndSurfs ParkingArea,         !-Ground Surface 2 Name",
                          "    0.1,                          !-Ground Surface 2 View Factor",
                          "    Ground Temp Sch,              !-Ground Surface 2 Temperature Schedule Name",
                          "    ,                             !-Ground Surface 2 Reflectance Schedule Name",
                          "    GndSurfs LakeArea,            !-Ground Surface 3 Name",
                          "    0.1,                          !-Ground Surface 3 View Factor",
                          "    Ground Temp Sch,              !-Ground Surface 3 Temperature Schedule Name",
                          "    ;                             !-Ground Surface 3 Reflectance Schedule Name",

                          "  Schedule:Compact,",
                          "    Ground Temp Sch,              !- Name",
                          "    Any Number,                   !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 15.0;           !- Field 3",

                          "  BuildingSurface:Detailed,",
                          "    Wall,                         !- Name",
                          "    Wall,                         !- Surface Type",
                          "    WallConstruction,             !- Construction Name",
                          "    Zone,                         !- Zone Name",
                          "    ,                             !- Space Name",
                          "    Outdoors,                     !- Outside Boundary Condition",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    SunExposed,                   !- Sun Exposure",
                          "    WindExposed,                  !- Wind Exposure",
                          "    0.5000000,                    !- View Factor to Ground",
                          "    4,                            !- Number of Vertices",
                          "    0.0,0.000000,10.00000,        !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.0,0.000000,0.0,             !- X,Y,Z ==> Vertex 2 {m}",
                          "    10.00000,0.0,0.0,             !- X,Y,Z ==> Vertex 3 {m}",
                          "    10.00000,0.0,10.00000;        !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,"
                          "    Floor,                        !- Name",
                          "    Floor,                        !- Surface Type",
                          "    WallConstruction,             !- Construction Name",
                          "    Zone,                         !- Zone Name",
                          "    ,                             !- Space Name",
                          "    Outdoors,                     !- Outside Boundary Condition",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    NoSun,                        !- Sun Exposure",
                          "    NoWind,                       !- Wind Exposure",
                          "    1.0,                          !- View Factor to Ground",
                          "    4,                            !- Number of Vertices",
                          "    0.000000,0.000000,0,          !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.000000,10.000000,0,         !- X,Y,Z ==> Vertex 2 {m}",
                          "    10.00000,10.000000,0,         !- X,Y,Z ==> Vertex 3 {m}",
                          "    10.00000,0.000000,0;          !- X,Y,Z ==> Vertex 4 {m}",

                          "  Zone,"
                          "    Zone,                         !- Name",
                          "    0,                            !- Direction of Relative North {deg}",
                          "    6.000000,                     !- X Origin {m}",
                          "    6.000000,                     !- Y Origin {m}",
                          "    0,                            !- Z Origin {m}",
                          "    1,                            !- Type",
                          "    1,                            !- Multiplier",
                          "    autocalculate,                !- Ceiling Height {m}",
                          "    autocalculate;                !- Volume {m3}"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBal->ZoneIntGain.allocate(1);

    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    HeatBalanceManager::GetFrameAndDividerData(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    HeatBalanceManager::GetBuildingData(*state, ErrorsFound);

    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;
    state->dataGlobal->TimeStepZoneSec = 3600.0;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->OutBaroPress = 100000;

    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);
    // test surface property object inputs
    int SrdSurfsNum = Util::FindItemInList("SRDSURFS:FENESSURFACE", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(SrdSurfsNum).SurroundingSurfsPtr);
    int GndSurfsNum = Util::FindItemInList("GNDSURFS:FENESSURFACE", state->dataSurface->GroundSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(GndSurfsNum).GroundSurfsPtr);
    // set local derived data vars
    int SurfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    SrdSurfsNum = state->dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    GndSurfsNum = state->dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.5, SrdSurfsProperty.SkyViewFactor);
    EXPECT_EQ(DataSizing::AutoSize, SrdSurfsProperty.GroundViewFactor);
    // check surrounding surfaces view factors
    EXPECT_DOUBLE_EQ(0.1, SrdSurfsProperty.SurroundingSurfs(1).ViewFactor);
    // check ground surfaces view factors
    EXPECT_EQ("GNDSURFS GRASSAREA", GndSurfsProperty.GndSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.2, GndSurfsProperty.GndSurfs(1).ViewFactor);
    EXPECT_EQ("GNDSURFS PARKINGAREA", GndSurfsProperty.GndSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty.GndSurfs(2).ViewFactor);
    EXPECT_EQ("GNDSURFS LAKEAREA", GndSurfsProperty.GndSurfs(3).Name);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty.GndSurfs(3).ViewFactor);
    EXPECT_DOUBLE_EQ(0.4, GndSurfsProperty.SurfsViewFactorSum);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_GetVerticesDropDuplicates)
{
    // Test for #9123 - We expect the point marked "x" to be popped.
    // Once it is popped, point "y" is still below tolerance with the "#" point:
    //  * Floor: originally 6 but now 5
    //  * Ceiling: 1
    //
    //   ▲                       ▲
    //   │                       │
    // 4 o────────o 5          3 o────────o 2
    //   │        │              │        │
    //   ╵        ╵              ╵        ╵
    //   ╵ Floor  ╵              ╵Ceiling ╵
    //   ╵        ╵              ╵        ╵
    // 3 │     2  │            4 │     5  │
    //   o─────y  │              o─────y  │
    //   │     │  │              │     │  │
    //   └─────x──#───►          └─────x──#───►
    //        1    6                  6    1

    std::string const idf_objects = delimited_string({
        "BuildingSurface:Detailed,",
        "  Zn002:Flr002,            !- Name",
        "  Floor,                   !- Surface Type",
        "  FLOOR,                   !- Construction Name",
        "  ZONE 2,                  !- Zone Name",
        "  ,                        !- Space Name",
        "  Surface,                 !- Outside Boundary Condition",
        "  Zn001:Ceiling002,        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  ,                        !- View Factor to Ground",
        "  ,                        !- Number of Vertices",
        "  54.373, -28.887, 3.7,    !- X,Y,Z Vertex 1 {m}",
        "  54.373, -28.881, 3.7,    !- X,Y,Z Vertex 2 {m}",
        "  54.36,  -28.881, 3.7,    !- X,Y,Z Vertex 3 {m}",
        "  54.36,  -23.003, 3.7,    !- X,Y,Z Vertex 4 {m}",
        "  54.379, -23.003, 3.7,    !- X,Y,Z Vertex 5 {m}",
        "  54.379, -28.887, 3.7;    !- X,Y,Z Vertex 6 {m}",

        "BuildingSurface:Detailed,",
        "  Zn001:Ceiling002,        !- Name",
        "  Ceiling,                 !- Surface Type",
        "  FLOOR,                   !- Construction Name",
        "  ZONE 1,                  !- Zone Name",
        "  ,                        !- Space Name",
        "  Surface,                 !- Outside Boundary Condition",
        "  Zn002:Flr002,            !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  ,                        !- View Factor to Ground",
        "  ,                        !- Number of Vertices",
        "  54.379, -28.887, 3.7,    !- X,Y,Z Vertex 1 {m}",
        "  54.379, -23.003, 3.7,    !- X,Y,Z Vertex 2 {m}",
        "  54.36,  -23.003, 3.7,    !- X,Y,Z Vertex 3 {m}",
        "  54.36,  -28.881, 3.7,    !- X,Y,Z Vertex 4 {m}",
        "  54.373, -28.881, 3.7,    !- X,Y,Z Vertex 5 {m}",
        "  54.373, -28.887, 3.7;    !- X,Y,Z Vertex 6 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(2).Name = "ZONE 2";
    state->dataSurfaceGeometry->SurfaceTmp.allocate(2);
    int SurfNum = 0;
    int TotHTSurfs = 2;
    Array1D_string const BaseSurfCls(3, {"WALL", "FLOOR", "ROOF"});
    Array1D<DataSurfaces::SurfaceClass> const BaseSurfIDs(
        3, {DataSurfaces::SurfaceClass::Wall, DataSurfaces::SurfaceClass::Floor, DataSurfaces::SurfaceClass::Roof});
    int NeedToAddSurfaces;

    bool ErrorsFound(false);
    GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);

    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;

    state->dataHeatBal->TotConstructs = 1;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).Name = "FLOOR";

    state->dataGlobal->DisplayExtraWarnings = true;

    GetHTSurfaceData(*state, ErrorsFound, SurfNum, TotHTSurfs, 0, 0, 0, BaseSurfCls, BaseSurfIDs, NeedToAddSurfaces);
    EXPECT_FALSE(ErrorsFound);

    EXPECT_EQ(2, SurfNum);
    auto const error_string = delimited_string({
        "   ** Warning ** GetVertices: Distance between two vertices < .01, possibly coincident. for Surface=ZN002:FLR002, in Zone=ZONE 2",
        "   **   ~~~   ** Vertex [1]=(54.37,-28.89,3.70)",
        "   **   ~~~   ** Vertex [2]=(54.37,-28.88,3.70)",
        "   **   ~~~   ** Dropping Vertex [1].",
        "   ** Warning ** GetVertices: Distance between two vertices < .01, possibly coincident. for Surface=ZN002:FLR002, in Zone=ZONE 2",
        "   **   ~~~   ** Vertex [5]=(54.38,-28.89,3.70)",
        "   **   ~~~   ** Vertex [1]=(54.37,-28.88,3.70)",
        "   **   ~~~   ** Dropping Vertex [1].",
        "   ** Warning ** GetVertices: Distance between two vertices < .01, possibly coincident. for Surface=ZN001:CEILING002, in Zone=ZONE 1",
        "   **   ~~~   ** Vertex [6]=(54.37,-28.89,3.70)",
        "   **   ~~~   ** Vertex [1]=(54.38,-28.89,3.70)",
        "   **   ~~~   ** Dropping Vertex [6].",
        "   ** Warning ** GetVertices: Distance between two vertices < .01, possibly coincident. for Surface=ZN001:CEILING002, in Zone=ZONE 1",
        "   **   ~~~   ** Vertex [5]=(54.37,-28.88,3.70)",
        "   **   ~~~   ** Vertex [1]=(54.38,-28.89,3.70)",
        "   **   ~~~   ** Dropping Vertex [5].",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    const auto &sf_temps = state->dataSurfaceGeometry->SurfaceTmp;
    EXPECT_EQ(2, sf_temps.size());
    EXPECT_EQ("ZN002:FLR002", sf_temps(1).Name);
    EXPECT_EQ("ZN001:CEILING002", sf_temps(2).Name);

    EXPECT_EQ(4, sf_temps(1).Sides);
    EXPECT_EQ(4, sf_temps(1).Vertex.size());

    EXPECT_EQ(4, sf_temps(2).Sides);
    EXPECT_EQ(4, sf_temps(2).Vertex.size());

    EXPECT_NEAR(11.80, sf_temps(1).Perimeter, 0.02);
    EXPECT_NEAR(11.80, sf_temps(2).Perimeter, 0.02);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_GetVerticesDropDuplicates_Once)
{
    // Test for #9873 - We expect the point marked "x" to be popped. Once it is popped, there are no distances that are below tolerance.
    //   ▲                       ▲
    //   │                       │
    // 2 o────────o 3          5 o────────o 4
    //   │        │              │        │
    //   ┤        │              │        │
    //   │ Floor  │              │Ceiling │
    //   ┤        │              │        │
    //   │ 5      │              │ 2      │
    //   ┤  o─────o 4            │  o─────o 3
    //   │  │                    │  │
    //   o──x──┬──┬───►          o──x─────────►
    //  1    6          x       6    1

    constexpr double offset = 0.01;

    constexpr double min_x = -41.28;
    constexpr double max_x = min_x + 0.03;
    constexpr double off_x = min_x + offset;

    constexpr double min_y = -37.05;
    constexpr double max_y = min_y + 0.04;
    constexpr double off_y = min_y + offset;

    constexpr double perimeter = 2 * ((max_x - min_x) + (max_y - min_y));

    std::string const idf_objects = fmt::format(R"idf(
  BuildingSurface:Detailed,
    Zn002:Flr002,            !- Name
    Floor,                   !- Surface Type
    FLOOR,                   !- Construction Name
    ZONE 2,                  !- Zone Name
    ,                        !- Space Name
    Surface,                 !- Outside Boundary Condition
    Zn001:Ceiling002,        !- Outside Boundary Condition Object
    NoSun,                   !- Sun Exposure
    NoWind,                  !- Wind Exposure
    ,                        !- View Factor to Ground
    ,                        !- Number of Vertices
    {min_x:.2f}, {min_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 1
    {min_x:.2f}, {max_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 2
    {max_x:.2f}, {max_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 3
    {max_x:.2f}, {off_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 4
    {off_x:.2f}, {off_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 5
    {off_x:.2f}, {min_y:.2f}, 0.00;    !- X,Y,Z ==> Vertex 6

  BuildingSurface:Detailed,
    Zn001:Ceiling002,        !- Name
    Ceiling,                 !- Surface Type
    FLOOR,                   !- Construction Name
    ZONE 1,                  !- Zone Name
    ,                        !- Space Name
    Surface,                 !- Outside Boundary Condition
    Zn002:Flr002,            !- Outside Boundary Condition Object
    NoSun,                   !- Sun Exposure
    NoWind,                  !- Wind Exposure
    ,                        !- View Factor to Ground
    ,                        !- Number of Vertices
    {off_x:.2f}, {min_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 1
    {off_x:.2f}, {off_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 2
    {max_x:.2f}, {off_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 3
    {max_x:.2f}, {max_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 4
    {min_x:.2f}, {max_y:.2f}, 0.00,    !- X,Y,Z ==> Vertex 5
    {min_x:.2f}, {min_y:.2f}, 0.00;    !- X,Y,Z ==> Vertex 6
    )idf",
                                                fmt::arg("min_x", min_x),
                                                fmt::arg("max_x", max_x),
                                                fmt::arg("off_x", off_x),
                                                fmt::arg("min_y", min_y),
                                                fmt::arg("max_y", max_y),
                                                fmt::arg("off_y", off_y));
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataHeatBal->Zone(2).Name = "ZONE 2";
    state->dataSurfaceGeometry->SurfaceTmp.allocate(2);
    int SurfNum = 0;
    int TotHTSurfs = 2;
    Array1D_string const BaseSurfCls(3, {"WALL", "FLOOR", "ROOF"});
    Array1D<DataSurfaces::SurfaceClass> const BaseSurfIDs(
        3, {DataSurfaces::SurfaceClass::Wall, DataSurfaces::SurfaceClass::Floor, DataSurfaces::SurfaceClass::Roof});
    int NeedToAddSurfaces;

    bool ErrorsFound(false);
    GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);

    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;

    state->dataHeatBal->TotConstructs = 1;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).Name = "FLOOR";

    state->dataGlobal->DisplayExtraWarnings = true;

    GetHTSurfaceData(*state, ErrorsFound, SurfNum, TotHTSurfs, 0, 0, 0, BaseSurfCls, BaseSurfIDs, NeedToAddSurfaces);
    EXPECT_FALSE(ErrorsFound);

    EXPECT_EQ(2, SurfNum);
    auto const error_string = delimited_string({
        "   ** Warning ** GetVertices: Distance between two vertices < .01, possibly coincident. for Surface=ZN002:FLR002, in Zone=ZONE 2",
        fmt::format("   **   ~~~   ** Vertex [6]=({:.2f},{:.2f},0.00)", off_x, min_y),
        fmt::format("   **   ~~~   ** Vertex [1]=({:.2f},{:.2f},0.00)", min_x, min_y),
        "   **   ~~~   ** Dropping Vertex [6].",
        "   ** Warning ** GetVertices: Distance between two vertices < .01, possibly coincident. for Surface=ZN001:CEILING002, in Zone=ZONE 1",
        fmt::format("   **   ~~~   ** Vertex [1]=({:.2f},{:.2f},0.00)", off_x, min_y),
        fmt::format("   **   ~~~   ** Vertex [2]=({:.2f},{:.2f},0.00)", off_x, off_y),
        "   **   ~~~   ** Dropping Vertex [1].",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    const auto &sf_temps = state->dataSurfaceGeometry->SurfaceTmp;
    EXPECT_EQ(2, sf_temps.size());
    EXPECT_EQ("ZN002:FLR002", sf_temps(1).Name);
    EXPECT_EQ("ZN001:CEILING002", sf_temps(2).Name);

    EXPECT_EQ(5, sf_temps(1).Sides);
    EXPECT_EQ(5, sf_temps(1).Vertex.size());

    EXPECT_EQ(5, sf_temps(2).Sides);
    EXPECT_EQ(5, sf_temps(2).Vertex.size());

    EXPECT_NEAR(perimeter, sf_temps(1).Perimeter, 0.02);
    EXPECT_NEAR(perimeter, sf_temps(2).Perimeter, 0.02);
}

TEST_F(EnergyPlusFixture, Wrong_Window_Construction)
{
    // Test for #9331 - Crash in debug when wrong construction name is used for a Window

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        " FenestrationSurface:Detailed,",
        "    Surface 8 - TriangularWindow,    !- Name",
        "    Window,                  !- Surface Type",
        "    WRONG CONSTRUCTION,      !- Construction Name", // <------- HERE: doesn't exist
        "    Surface 3 - Rectangle,   !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    Autocalculate,           !- Number of Vertices",
        "    0.05, 0.0, 0.05,         !- X,Y,Z ==> Vertex 1 {m}",
        "    0.15, 0.0, 0.05,         !- X,Y,Z ==> Vertex 2 {m}",
        "    0.10, 0.0, 0.15;         !- X,Y,Z ==> Vertex 3 {m}",

        " BuildingSurface:Detailed,",
        "    Surface 3 - Rectangle,   !- Name",
        "    Wall,                    !- Surface Type",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name",
        "    Zone1,                   !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    ,                        !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    1.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 2 {m}",
        "    1.0, 0.0, 1.0,           !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 0.0, 1.0;           !- X,Y,Z ==> Vertex 4 {m}",

        " Zone,",
        "    Zone1,                   !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0.0,                     !- X Origin {m}",
        "    0.0,                     !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    ,                        !- Type",
        "    ,                        !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    ,                        !- Volume {m3}",
        "    ,                        !- Floor Area {m2}",
        "    ,                        !- Zone Inside Convection Algorithm",
        "    ,                        !- Zone Outside Convection Algorithm",
        "    No;                      !- Part of Total Floor Area",

        " Construction,",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Name",
        "    MAT-CC05 4 HW CONCRETE;  !- Outside Layer",

        " Material,",
        "    MAT-CC05 4 HW CONCRETE,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1016,                  !- Thickness {m}",
        "    1.311,                   !- Conductivity {W/m-K}",
        "    2240,                    !- Density {kg/m3}",
        "    836.800000000001,        !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.85,                    !- Solar Absorptance",
        "    0.85;                    !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                  // expect no errors

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    EXPECT_THROW(GetSurfaceData(*state, ErrorsFound), std::runtime_error);
    EXPECT_TRUE(ErrorsFound);

    std::string const error_string = delimited_string({
        "   ** Severe  ** FenestrationSurface:Detailed=\"SURFACE 8 - TRIANGULARWINDOW\", invalid Construction Name=\"WRONG CONSTRUCTION\".",
        "   **  Fatal  ** GetSurfaceData: Errors discovered, program terminates.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=FenestrationSurface:Detailed=\"SURFACE 8 - TRIANGULARWINDOW\", invalid Construction Name=\"WRONG CONSTRUCTION\".",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}
TEST_F(EnergyPlusFixture, CalculateZoneVolume_WithAirBoundaries)
{
    // Test floor area and volume with air boundary surfaces
    std::string_view constexpr idf_objects = R"IDF(
        Zone,
        Zone 1;             !- Name

        Zone,
        Zone 2;             !- Name

        Zone,
        Zone 3;             !- Name

        Space,
        Space 1,             !- Name
        Zone 1;             !- Zone Name

        Space,
        Space 2,             !- Name
        Zone 2;             !- Zone Name

        Space,
        Space 3,             !- Name
        Zone 3;             !- Zone Name

        Material,
            Some Material,         !- Name
            VeryRough,               !- Roughness
            0.006,                   !- Thickness {m}
            0.815,                   !- Conductivity {W/m-K}
            929,                     !- Density {kg/m3}
            3140,                    !- Specific Heat {J/kg-K}
            0.9,                     !- Thermal Absorptance
            0.7,                     !- Solar Absorptance
            0.7;                     !- Visible Absorptance
        Construction,
            Some Construction,  !- Name
            Some Material;        !- Outside Layer
        Construction:AirBoundary,
        Grouped Air Boundary, !- Name
        None;                    !- Air Exchange Method

        BuildingSurface:Detailed,
            Zone1-Surface1,  !- Name
            Wall,                 !- Surface Type
            Grouped Air Boundary,  !- Construction Name
            Zone 1,       !- Zone Name
            Space 1,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone2-Surface1,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone2-Surface1,  !- Name
            Wall,                 !- Surface Type
            Grouped Air Boundary,  !- Construction Name
            Zone 2,       !- Zone Name
            Space 2,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone1-Surface1,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone1-Surface2,  !- Name
            Wall,                 !- Surface Type
            Grouped Air Boundary,  !- Construction Name
            Zone 1,       !- Zone Name
            Space 1,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone3-Surface1,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone3-Surface1,  !- Name
            Wall,                 !- Surface Type
            Grouped Air Boundary,  !- Construction Name
            Zone 3,       !- Zone Name
            Space 3,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone1-Surface2,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone1-Floor,  !- Name
            Floor,                 !- Surface Type
            Grouped Air Boundary,  !- Construction Name
            Zone 1,       !- Zone Name
            Space 1,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone2-Ceiling,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 2
            1,1,0,              !- Vertex 3
            1,0,0;              !- Vertex 4

        BuildingSurface:Detailed,
            Zone2-Ceiling,  !- Name
            Ceiling,                 !- Surface Type
            Grouped Air Boundary,  !- Construction Name
            Zone 2,       !- Zone Name
            Space 2,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone1-Floor,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,2,              !- Vertex 1
            1,0,2,              !- Vertex 4
            1,1,2,              !- Vertex 3
            0,1,2;              !- Vertex 2

        BuildingSurface:Detailed,
            Zone2-Floor,  !- Name
            Floor,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 2,       !- Zone Name
            Space 2,                 !- Space Name
            Ground,                 !- Outside Boundary Condition
            ,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 2
            1,1,0,              !- Vertex 3
            1,0,0;              !- Vertex 4

        BuildingSurface:Detailed,
            Zone3-Floor,  !- Name
            Floor,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 3,       !- Zone Name
            Space 3,                 !- Space Name
            Ground,                 !- Outside Boundary Condition
            ,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,2,0,              !- Vertex 2
            2,2,0,              !- Vertex 3
            2,0,0;              !- Vertex 4

    )IDF";
    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    auto const &zone1 = state->dataHeatBal->Zone(1);
    auto const &zone2 = state->dataHeatBal->Zone(2);
    auto const &zone3 = state->dataHeatBal->Zone(3);

    EXPECT_EQ(zone1.UserEnteredFloorArea, -99999.0);
    EXPECT_EQ(zone1.FloorArea, 0.0);
    EXPECT_EQ(zone1.geometricFloorArea, 1.0);
    EXPECT_FALSE(zone1.HasFloor);
    EXPECT_EQ(zone1.CeilingArea, 0.0);
    EXPECT_EQ(zone1.geometricCeilingArea, 0.0);
    EXPECT_FALSE(zone1.ceilingHeightEntered);
    EXPECT_EQ(zone1.CeilingHeight, 2.0);
    EXPECT_EQ(zone1.Volume, 2.0);

    EXPECT_EQ(zone2.UserEnteredFloorArea, -99999.0);
    EXPECT_EQ(zone2.FloorArea, 1.0);
    EXPECT_EQ(zone2.geometricFloorArea, 1.0);
    EXPECT_TRUE(zone2.HasFloor);
    EXPECT_EQ(zone2.CeilingArea, 0.0);
    EXPECT_EQ(zone2.geometricCeilingArea, 1.0);
    EXPECT_FALSE(zone2.ceilingHeightEntered);
    EXPECT_EQ(zone2.CeilingHeight, 2.0);
    EXPECT_EQ(zone2.Volume, 2.0);

    EXPECT_EQ(zone3.UserEnteredFloorArea, -99999.0);
    EXPECT_EQ(zone3.FloorArea, 4.0);
    EXPECT_EQ(zone3.geometricFloorArea, 4.0);
    EXPECT_TRUE(zone3.HasFloor);
    EXPECT_EQ(zone3.CeilingArea, 0.0);
    EXPECT_EQ(zone3.geometricCeilingArea, 0.0);
    EXPECT_FALSE(zone3.ceilingHeightEntered);
    EXPECT_EQ(zone3.CeilingHeight, 2.0);
    EXPECT_EQ(zone3.Volume, 8.0);
}

TEST_F(EnergyPlusFixture, CalculateZoneVolume_IgnoresSameZonePartitionSurfaces)
{
    // A single zone made of three adjacent spaces. One partition is a regular wall pair and one is an air-boundary pair.
    // The outer shell is fully enclosed, so the zone volume check should ignore the same-zone partitions and not warn.
    std::string_view constexpr idf_objects = R"IDF(
        Zone,
        Zone 1;             !- Name

        Space,
        Space 1,            !- Name
        Zone 1;             !- Zone Name

        Space,
        Space 2,            !- Name
        Zone 1;             !- Zone Name

        Space,
        Space 3,            !- Name
        Zone 1;             !- Zone Name

        Material,
            Some Material,         !- Name
            VeryRough,             !- Roughness
            0.006,                 !- Thickness {m}
            0.815,                 !- Conductivity {W/m-K}
            929,                   !- Density {kg/m3}
            3140,                  !- Specific Heat {J/kg-K}
            0.9,                   !- Thermal Absorptance
            0.7,                   !- Solar Absorptance
            0.7;                   !- Visible Absorptance

        Construction,
            Some Construction,     !- Name
            Some Material;         !- Outside Layer

        Construction:AirBoundary,
            Air Boundary,          !- Name
            None;                  !- Air Exchange Method

        BuildingSurface:Detailed,
            Space1-Floor,          !- Name
            Floor,                 !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 1,               !- Space Name
            Ground,                !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            0,0,0,
            0,1,0,
            1,1,0,
            1,0,0;

        BuildingSurface:Detailed,
            Space2-Floor,          !- Name
            Floor,                 !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 2,               !- Space Name
            Ground,                !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            1,0,0,
            1,1,0,
            2,1,0,
            2,0,0;

        BuildingSurface:Detailed,
            Space3-Floor,          !- Name
            Floor,                 !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 3,               !- Space Name
            Ground,                !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            2,0,0,
            2,1,0,
            3,1,0,
            3,0,0;

        BuildingSurface:Detailed,
            Space1-Roof,           !- Name
            Roof,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 1,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            1,0,2,
            1,1,2,
            0,1,2,
            0,0,2;

        BuildingSurface:Detailed,
            Space2-Roof,           !- Name
            Roof,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 2,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            2,0,2,
            2,1,2,
            1,1,2,
            1,0,2;

        BuildingSurface:Detailed,
            Space3-Roof,           !- Name
            Roof,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 3,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            3,0,2,
            3,1,2,
            2,1,2,
            2,0,2;

        BuildingSurface:Detailed,
            Space1-South,          !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 1,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            1,0,2,
            0,0,2,
            0,0,0,
            1,0,0;

        BuildingSurface:Detailed,
            Space2-South,          !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 2,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            2,0,2,
            1,0,2,
            1,0,0,
            2,0,0;

        BuildingSurface:Detailed,
            Space3-South,          !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 3,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            3,0,2,
            2,0,2,
            2,0,0,
            3,0,0;

        BuildingSurface:Detailed,
            Space1-North,          !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 1,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            0,1,2,
            1,1,2,
            1,1,0,
            0,1,0;

        BuildingSurface:Detailed,
            Space2-North,          !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 2,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            1,1,2,
            2,1,2,
            2,1,0,
            1,1,0;

        BuildingSurface:Detailed,
            Space3-North,          !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 3,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            2,1,2,
            3,1,2,
            3,1,0,
            2,1,0;

        BuildingSurface:Detailed,
            Space1-West,           !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 1,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            0,0,2,
            0,1,2,
            0,1,0,
            0,0,0;

        BuildingSurface:Detailed,
            Space3-East,           !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 3,               !- Space Name
            Outdoors,              !- Outside Boundary Condition
            ,                      !- Outside Boundary Condition Object
            SunExposed,            !- Sun Exposure
            WindExposed,           !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            3,1,2,
            3,0,2,
            3,0,0,
            3,1,0;

        BuildingSurface:Detailed,
            Space1-Partition,      !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 1,               !- Space Name
            Surface,               !- Outside Boundary Condition
            Space2-Partition,      !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            1,1,2,
            1,0,2,
            1,0,0,
            1,1,0;

        BuildingSurface:Detailed,
            Space2-Partition,      !- Name
            Wall,                  !- Surface Type
            Some Construction,     !- Construction Name
            Zone 1,                !- Zone Name
            Space 2,               !- Space Name
            Surface,               !- Outside Boundary Condition
            Space1-Partition,      !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            1,0,2,
            1,1,2,
            1,1,0,
            1,0,0;

        BuildingSurface:Detailed,
            Space2-AirBoundary,    !- Name
            Wall,                  !- Surface Type
            Air Boundary,          !- Construction Name
            Zone 1,                !- Zone Name
            Space 2,               !- Space Name
            Surface,               !- Outside Boundary Condition
            Space3-AirBoundary,    !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            2,1,2,
            2,0,2,
            2,0,0,
            2,1,0;

        BuildingSurface:Detailed,
            Space3-AirBoundary,    !- Name
            Wall,                  !- Surface Type
            Air Boundary,          !- Construction Name
            Zone 1,                !- Zone Name
            Space 3,               !- Space Name
            Surface,               !- Outside Boundary Condition
            Space2-AirBoundary,    !- Outside Boundary Condition Object
            NoSun,                 !- Sun Exposure
            NoWind,                !- Wind Exposure
            ,                      !- View Factor to Ground
            4,                     !- Number of Vertices
            2,0,2,
            2,1,2,
            2,1,0,
            2,0,0;
    )IDF";

    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->NoGroundTempObjWarning = false;

    EXPECT_NO_THROW(SetupZoneGeometry(*state, ErrorsFound));
    EXPECT_TRUE(compare_err_stream(""));
    EXPECT_FALSE(ErrorsFound);

    auto const &zone1 = state->dataHeatBal->Zone(1);
    EXPECT_EQ(zone1.geometricFloorArea, 3.0);
    EXPECT_EQ(zone1.CeilingHeight, 2.0);
    EXPECT_EQ(zone1.Volume, 6.0);

    EXPECT_EQ(state->dataHeatBal->space(1).Volume, 2.0);
    EXPECT_EQ(state->dataHeatBal->space(2).Volume, 2.0);
    EXPECT_EQ(state->dataHeatBal->space(3).Volume, 2.0);
}

TEST_F(EnergyPlusFixture, CalculatZoneVolume_WithoutAirBoundaries)
{
    // Test floor area and volume without air boundary surfaces
    std::string_view constexpr idf_objects = R"IDF(
        Zone,
        Zone 1;             !- Name

        Zone,
        Zone 2;             !- Name

        Zone,
        Zone 3;             !- Name

        Space,
        Space 1,             !- Name
        Zone 1;             !- Zone Name

        Space,
        Space 2,             !- Name
        Zone 2;             !- Zone Name

        Space,
        Space 3,             !- Name
        Zone 3;             !- Zone Name

        Material,
            Some Material,         !- Name
            VeryRough,               !- Roughness
            0.006,                   !- Thickness {m}
            0.815,                   !- Conductivity {W/m-K}
            929,                     !- Density {kg/m3}
            3140,                    !- Specific Heat {J/kg-K}
            0.9,                     !- Thermal Absorptance
            0.7,                     !- Solar Absorptance
            0.7;                     !- Visible Absorptance
        Construction,
            Some Construction,  !- Name
            Some Material;        !- Outside Layer

        BuildingSurface:Detailed,
            Zone1-Surface1,  !- Name
            Wall,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 1,       !- Zone Name
            Space 1,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone2-Surface1,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone2-Surface1,  !- Name
            Wall,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 2,       !- Zone Name
            Space 2,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone1-Surface1,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone1-Surface2,  !- Name
            Wall,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 1,       !- Zone Name
            Space 1,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone3-Surface1,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone3-Surface1,  !- Name
            Wall,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 3,       !- Zone Name
            Space 3,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone1-Surface2,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 1
            0,1,2,              !- Vertex 1
            0,0,2;              !- Vertex 1

        BuildingSurface:Detailed,
            Zone1-Floor,  !- Name
            Floor,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 1,       !- Zone Name
            Space 1,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone2-Ceiling,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 2
            1,1,0,              !- Vertex 3
            1,0,0;              !- Vertex 4

        BuildingSurface:Detailed,
            Zone2-Ceiling,  !- Name
            Ceiling,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 2,       !- Zone Name
            Space 2,                 !- Space Name
            Surface,                 !- Outside Boundary Condition
            Zone1-Floor,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,2,              !- Vertex 1
            1,0,2,              !- Vertex 4
            1,1,2,              !- Vertex 3
            0,1,2;              !- Vertex 2

        BuildingSurface:Detailed,
            Zone2-Floor,  !- Name
            Floor,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 2,       !- Zone Name
            Space 2,                 !- Space Name
            Ground,                 !- Outside Boundary Condition
            ,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,1,0,              !- Vertex 2
            1,1,0,              !- Vertex 3
            1,0,0;              !- Vertex 4

        BuildingSurface:Detailed,
            Zone3-Floor,  !- Name
            Floor,                 !- Surface Type
            Some Construction,  !- Construction Name
            Zone 3,       !- Zone Name
            Space 3,                 !- Space Name
            Ground,                 !- Outside Boundary Condition
            ,  !- Outside Boundary Condition Object
            NoSun,                   !- Sun Exposure
            NoWind,                  !- Wind Exposure
            ,                        !- View Factor to Ground
            4,                       !- Number of Vertices
            0,0,0,              !- Vertex 1
            0,2,0,              !- Vertex 2
            2,2,0,              !- Vertex 3
            2,0,0;              !- Vertex 4

    )IDF";
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    auto const &zone1 = state->dataHeatBal->Zone(1);
    auto const &zone2 = state->dataHeatBal->Zone(2);
    auto const &zone3 = state->dataHeatBal->Zone(3);

    EXPECT_EQ(zone1.UserEnteredFloorArea, -99999.0);
    EXPECT_EQ(zone1.FloorArea, 1.0);
    EXPECT_EQ(zone1.geometricFloorArea, 1.0);
    EXPECT_TRUE(zone1.HasFloor);
    EXPECT_EQ(zone1.CeilingArea, 0.0);
    EXPECT_EQ(zone1.geometricCeilingArea, 0.0);
    EXPECT_FALSE(zone1.ceilingHeightEntered);
    EXPECT_EQ(zone1.CeilingHeight, 2.0);
    EXPECT_EQ(zone1.Volume, 2.0);

    EXPECT_EQ(zone2.UserEnteredFloorArea, -99999.0);
    EXPECT_EQ(zone2.FloorArea, 1.0);
    EXPECT_EQ(zone2.geometricFloorArea, 1.0);
    EXPECT_TRUE(zone2.HasFloor);
    EXPECT_EQ(zone2.CeilingArea, 1.0);
    EXPECT_EQ(zone2.geometricCeilingArea, 1.0);
    EXPECT_FALSE(zone2.ceilingHeightEntered);
    EXPECT_EQ(zone2.CeilingHeight, 2.0);
    EXPECT_EQ(zone2.Volume, 2.0);

    EXPECT_EQ(zone3.UserEnteredFloorArea, -99999.0);
    EXPECT_EQ(zone3.FloorArea, 4.0);
    EXPECT_EQ(zone3.geometricFloorArea, 4.0);
    EXPECT_TRUE(zone3.HasFloor);
    EXPECT_EQ(zone3.CeilingArea, 0.0);
    EXPECT_EQ(zone3.geometricCeilingArea, 0.0);
    EXPECT_FALSE(zone3.ceilingHeightEntered);
    EXPECT_EQ(zone3.CeilingHeight, 2.0);
    EXPECT_EQ(zone3.Volume, 8.0);
}

TEST_F(EnergyPlusFixture, Test_Rotational_Azimuth_Diffs)
{
    // Test Pull Request 9920 that fixes Issue 9910
    SurfaceData BaseSurface;
    SurfaceData SubSurface;
    bool surfaceError;

    // Base surface: Northeast
    surfaceError = false;
    BaseSurface.Azimuth = 30.0;
    BaseSurface.Tilt = 90.0;
    BaseSurface.NewellSurfaceNormalVector = Vectors::VecNormalize(DataVectorTypes::Vector(1, std::sqrt(3.0), 0));

    // Sub surface: Northwest
    // should be no error message and no surfaceError
    SubSurface.Azimuth = 330.0;
    SubSurface.Tilt = 90.0;
    SubSurface.NewellSurfaceNormalVector = Vectors::VecNormalize(DataVectorTypes::Vector(-1, std::sqrt(3.0), 0));

    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);

    // This test would fail due to a severe error if without RP 9920 fix
    EXPECT_FALSE(surfaceError);

    // This should be true due to a warning error (but not due to sever surface error)
    EXPECT_TRUE(has_err_output());
}
TEST_F(EnergyPlusFixture, GetSurfaceData_ShadingSurfaceScheduleChecks)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Timestep, 6;",
        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  Construction,",
        "    Walls,          !- Name",
        "    CB11;        !- Outside Layer",

        "  Construction,",
        "    Windows,            !- Name",
        "    CLEAR 3MM;                    !- Outside Layer",

        "  Zone,",
        "    LIVING ZONE;             !- Name",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4383,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    Ceiling,                 !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Ceiling,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    Walls,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    EastWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Windows,     !- Construction Name",
        "    Living:East,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    17.242,2,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,2,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,6.572,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,6.572,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    SouthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Windows,     !- Construction Name",
        "    Living:South,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2,0,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2,0,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.572,0,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.572,0,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ScheduleTypeLimits, AnyNumber;",

        "  Schedule:Constant, AlwaysZero, AnyNumber, 0.0;",
        "  Schedule:Constant, AlwaysOne, AnyNumber, 1.0;",
        "  Schedule:Constant, AlwaysPoint8, AnyNumber, 0.8;",
        "  Schedule:Compact, VaryWithinRange, AnyNumber, Through: 12/31, For: AllDays, ",
        "  Until: 10:00, 0.2, Until: 12:00, 0.4, Until 14:00, 1.0, Until 24:00, 0.0;"
        "  Schedule:Compact, OutofRange, AnyNumber, Through: 12/31, For: AllDays, ",
        "  Until: 10:00, -1.0, Until: 12:00, 0.4, Until 14:00, 1.5, Until 24:00, 0.0;"

        "  Shading:Zone:Detailed,",
        "    ZoneShade:Living:South:Shade001,  !- Name",
        "    Living:South,        !- Base Surface Name",
        "    AlwaysZero,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Shading:Zone:Detailed,",
        "    ZoneShade:Living:South:Shade002,  !- Name",
        "    Living:South,        !- Base Surface Name",
        "    AlwaysOne,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Shading:Zone:Detailed,",
        "    ZoneShade:Living:South:Shade003,  !- Name",
        "    Living:South,        !- Base Surface Name",
        "    AlwaysPoint8,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Shading:Zone:Detailed,",
        "    ZoneShade:Living:South:Shade004,  !- Name",
        "    Living:South,        !- Base Surface Name",
        "    VaryWithinRange,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Building:Detailed,",
        "  BuildingShade:TiltedShadeSurface,             !- Name",
        "  AlwaysZero,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  -40.0,2.0,20.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  -40.0,0.00,20.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  -45.0,0.00,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  -45.0,2.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Site:Detailed,",
        "  SiteShade:FlatShadeSurface,             !- Name",
        "  AlwaysOne,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  40.0,2.0,10.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  40.0,0.00,10.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  45.0,0.00,10.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  45.0,2.0,10.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Building:Detailed,",
        "  BuildingShade:TiltedShadeSurface2,             !- Name",
        "  AlwaysPoint8,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  -40.0,2.0,20.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  -40.0,0.00,20.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  -45.0,0.00,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  -45.0,2.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Site:Detailed,",
        "  SiteShade:FlatShadeSurface2,             !- Name",
        "  VaryWithinRange,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  40.0,2.0,10.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  40.0,0.00,10.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  45.0,0.00,10.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  45.0,2.0,10.0;  !- X,Y,Z ==> Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    SetupZoneGeometry(*state, ErrorsFound);
    SolarShading::checkShadingSurfaceSchedules(*state);
    std::string const error_string = delimited_string(
        {"   ** Warning ** Shading Surface=\"SITESHADE:FLATSHADESURFACE\", Transmittance Schedule Name=\"ALWAYSONE\", is always transparent.",
         "   **   ~~~   ** This shading surface will be ignored.",
         "   ** Warning ** Shading Surface=\"ZONESHADE:LIVING:SOUTH:SHADE002\", Transmittance Schedule Name=\"ALWAYSONE\", is always transparent.",
         "   **   ~~~   ** This shading surface will be ignored."});

    compare_err_stream(error_string);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    static constexpr std::array<std::string_view, 8> surfacenames{"ZoneShade:Living:South:Shade001",
                                                                  "ZoneShade:Living:South:Shade002",
                                                                  "ZoneShade:Living:South:Shade003",
                                                                  "ZoneShade:Living:South:Shade004",
                                                                  "BuildingShade:TiltedShadeSurface",
                                                                  "SiteShade:FlatShadeSurface",
                                                                  "BuildingShade:TiltedShadeSurface2",
                                                                  "SiteShade:FlatShadeSurface2"};
    static constexpr std::array<bool, 8> isTransparent{false, true, false, false, false, true, false, false};

    for (int surf = 0; surf < 8; ++surf) {
        int surfNum = Util::FindItemInList(Util::makeUPPER(surfacenames[surf]), state->dataSurface->Surface);
        EXPECT_EQ(state->dataSurface->Surface(surfNum).IsTransparent, isTransparent[surf]);
    }
    EXPECT_TRUE(state->dataSolarShading->anyScheduledShadingSurface);
    EXPECT_TRUE(state->dataSurface->ShadingTransmittanceVaries);

    // compare_err_stream( "" ); // just for debugging
}
TEST_F(EnergyPlusFixture, GetSurfaceData_ShadingSurfaceScheduleOutOfRange)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Timestep, 6;",
        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  Construction,",
        "    Walls,          !- Name",
        "    CB11;        !- Outside Layer",

        "  Construction,",
        "    Windows,            !- Name",
        "    CLEAR 3MM;                    !- Outside Layer",

        "  Zone,",
        "    LIVING ZONE;             !- Name",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4383,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    Ceiling,                 !- Surface Type",
        "    Walls,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Ceiling,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    Walls,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    EastWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Windows,     !- Construction Name",
        "    Living:East,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    17.242,2,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,2,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,6.572,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,6.572,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    SouthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Windows,     !- Construction Name",
        "    Living:South,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2,0,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2,0,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.572,0,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.572,0,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ScheduleTypeLimits, AnyNumber;",

        "  Schedule:Compact, OutofRange, AnyNumber, Through: 12/31, For: AllDays, ",
        "  Until: 10:00, -1.0, Until: 12:00, 0.4, Until 14:00, 1.5, Until 24:00, 0.0;"

        "  Shading:Zone:Detailed,",
        "    ZoneShade:Living:South:Shade001,  !- Name",
        "    Living:South,        !- Base Surface Name",
        "    OutofRange,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Building:Detailed,",
        "  BuildingShade:TiltedShadeSurface,             !- Name",
        "  OutofRange,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  -40.0,2.0,20.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  -40.0,0.00,20.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  -45.0,0.00,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  -45.0,2.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    EXPECT_THROW(SetupZoneGeometry(*state, ErrorsFound), std::runtime_error);
    // EXPECT_THROW(GetSurfaceData(*state, ErrorsFound), std::runtime_error);
    EXPECT_TRUE(ErrorsFound);
    std::string const error_string =
        delimited_string({"   ** Severe  ** GetDetShdSurfaceData: Shading:Building:Detailed = BUILDINGSHADE:TILTEDSHADESURFACE",
                          "   **   ~~~   ** Transmittance Schedule Name = OUTOFRANGE, schedule contains values that are < 0 and/or > 1",
                          "   ** Severe  ** GetAttShdSurfaceData: Shading:Zone:Detailed = ZONESHADE:LIVING:SOUTH:SHADE001",
                          "   **   ~~~   ** Transmittance Schedule Name = OUTOFRANGE, schedule contains values that are < 0 and/or > 1",
                          "   **  Fatal  ** GetSurfaceData: Errors discovered, program terminates.",
                          "   ...Summary of Errors that led to program termination:",
                          "   ..... Reference severe error count=2",
                          "   ..... Last severe error=GetAttShdSurfaceData: Shading:Zone:Detailed = ZONESHADE:LIVING:SOUTH:SHADE001"});

    compare_err_stream(error_string);
    // compare_err_stream( "" ); // just for debugging
}

TEST_F(EnergyPlusFixture, Fix_checkSubSurfAzTiltNorm_Test)
{
    // Unit Test for Pull Request 9905 that fixes a few potential problems described Issue 9893
    SurfaceData BaseSurface;
    SurfaceData SubSurface;
    bool surfaceError;

    // Test Base surf and subsurf normal vectors assignment
    surfaceError = false;

    BaseSurface.Vertex.dimension(4);

    BaseSurface.Vertex = {
        DataVectorTypes::Vector(0, 0, 0), DataVectorTypes::Vector(1, 0, 0), DataVectorTypes::Vector(1, 1, 0), DataVectorTypes::Vector(0, 1, 0)};
    Vectors::CreateNewellSurfaceNormalVector(BaseSurface.Vertex, BaseSurface.Vertex.size(), BaseSurface.NewellSurfaceNormalVector);
    Vectors::DetermineAzimuthAndTilt(BaseSurface.Vertex,
                                     BaseSurface.Azimuth,
                                     BaseSurface.Tilt,
                                     BaseSurface.lcsx,
                                     BaseSurface.lcsy,
                                     BaseSurface.lcsz,
                                     BaseSurface.NewellSurfaceNormalVector);

    SubSurface.Vertex.dimension(4);

    SubSurface.Vertex = {DataVectorTypes::Vector(0, 0, 0),
                         DataVectorTypes::Vector(1, 0, 0),
                         DataVectorTypes::Vector(1, 1, 0.0003),
                         DataVectorTypes::Vector(0, 1, 0.0003)};
    Vectors::CreateNewellSurfaceNormalVector(SubSurface.Vertex, SubSurface.Vertex.size(), SubSurface.NewellSurfaceNormalVector);
    Vectors::DetermineAzimuthAndTilt(SubSurface.Vertex,
                                     SubSurface.Azimuth,
                                     SubSurface.Tilt,
                                     SubSurface.lcsx,
                                     SubSurface.lcsy,
                                     SubSurface.lcsz,
                                     SubSurface.NewellSurfaceNormalVector);

    bool sameSurfNormal(false);

    // This is the sameSurfNormal test used in checkSubSurfAzTiltNorm()
    Vectors::CompareTwoVectors(BaseSurface.NewellSurfaceNormalVector, SubSurface.NewellSurfaceNormalVector, sameSurfNormal, 0.001);

    // The surface normals are not exactly the same
    EXPECT_GE(std::abs(BaseSurface.NewellSurfaceNormalVector.y - SubSurface.NewellSurfaceNormalVector.y), 1e-5);
    EXPECT_GE(std::abs(BaseSurface.NewellSurfaceNormalVector.z - SubSurface.NewellSurfaceNormalVector.z), 1e-10);

    // But should pass the sameSurfNormal test
    EXPECT_TRUE(sameSurfNormal);

    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);

    // These should pass
    EXPECT_FALSE(surfaceError);
    EXPECT_FALSE(has_err_output());

    // Without the fix in PR 9905 the following would fail since they are not assigned to be exactly the same
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.z, SubSurface.lcsz.z);
    // The following two lines might pass in the original code, but probably not a consistent outcome
    // In the new code (after PR 9905 fix, they should be consistently passing the tests
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.y, SubSurface.lcsz.y);
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.x, SubSurface.lcsz.x);
}

TEST_F(EnergyPlusFixture, SurfaceGeometry_SurroundingSurfacesViewFactorTest)
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
        Concrete Block,               !- Name
        MediumRough,                  !- Roughness
        0.1014984,                    !- Thickness {m}
        0.3805070,                    !- Conductivity {W/m-K}
        608.7016,                     !- Density {kg/m3}
        836.8000;                     !- Specific Heat {J/kg-K}

      Construction,
        WallConstruction,             !- Name
        Concrete Block;               !- Outside Layer

      BuildingSurface:Detailed,
        Floor,                        !- Name
        Floor,                        !- Surface Type
        WallConstruction,             !- Construction Name
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

      BuildingSurface:Detailed,
        North-Wall,                   !- Name
        Wall,                         !- Surface Type
        WallConstruction,             !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        SunExposed,                   !- Sun Exposure
        WindExposed,                  !- Wind Exposure
        0.1000000,                    !- View Factor to Ground
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
        0.2,                          !- Surrounding Surface 1 View Factor
        Surrounding Temp Sch 1,       !- Surrounding Surface 1 Temperature Schedule Name
        SrdSurfs:Surface 2,           !- Surrounding Surface 2 Name
        0.2,                          !- Surrounding Surface 2 View Factor
        Surrounding Temp Sch 2,       !- Surrounding Surface 2 Temperature Schedule Name
        SrdSurfs:Surface 3,           !- Surrounding Surface 3 Name
        0.1,                          !- Surrounding Surface 3 View Factor
        Surrounding Temp Sch 3;       !- Surrounding Surface 3 Temperature Schedule Name

      BuildingSurface:Detailed,
        East-Wall,                    !- Name
        Wall,                         !- Surface Type
        WallConstruction,             !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        SunExposed,                   !- Sun Exposure
        WindExposed,                  !- Wind Exposure
        0.2000000,                    !- View Factor to Ground
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
        0.5,                          !- Sky View Factor
        Sky Temp Sch,                 !- Sky Temperature Schedule Name
        ,                             !- Ground View Factor
        ,                             !- Ground Temperature Schedule Name
        SrdSurfs:Surface 1,           !- Surrounding Surface 1 Name
        0.1,                          !- Surrounding Surface 1 View Factor
        Surrounding Temp Sch 1,       !- Surrounding Surface 1 Temperature Schedule Name
        SrdSurfs:Surface 2,           !- Surrounding Surface 2 Name
        0.1,                          !- Surrounding Surface 2 View Factor
        Surrounding Temp Sch 2,       !- Surrounding Surface 2 Temperature Schedule Name
        SrdSurfs:Surface 3,           !- Surrounding Surface 3 Name
        0.1,                          !- Surrounding Surface 3 View Factor
        Surrounding Temp Sch 3;       !- Surrounding Surface 3 Temperature Schedule Name

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
        Until: 24:00, 15.0;           !- Field 3

    )IDF";

    bool ErrorsFound = false;
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBal->ZoneIntGain.allocate(1);
    createFacilityElectricPowerServiceObject(*state);
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

    // reset sky and ground view factors
    HeatBalanceSurfaceManager::InitSurfacePropertyViewFactors(*state);

    int surfNum = 0;
    int srdSurfsNum = 0;
    int srdSurfsPropNum = 0;
    // test 1: exterior north wall
    srdSurfsPropNum = Util::FindItemInList("SRDSURFS:NORTH-WALL", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(srdSurfsPropNum).SurroundingSurfsPtr);
    surfNum = Util::FindItemInList("NORTH-WALL", state->dataSurface->Surface);
    auto &surface_north_wall = state->dataSurface->Surface(surfNum);
    srdSurfsNum = state->dataSurface->Surface(surfNum).SurfSurroundingSurfacesNum;
    auto &srdSurfsProperty_north = state->dataSurface->SurroundingSurfsProperty(srdSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.4, srdSurfsProperty_north.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.1, srdSurfsProperty_north.GroundViewFactor);
    // check surrounding surfaces view factors
    EXPECT_EQ("SRDSURFS:SURFACE 1", srdSurfsProperty_north.SurroundingSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.2, srdSurfsProperty_north.SurroundingSurfs(1).ViewFactor);
    EXPECT_EQ("SRDSURFS:SURFACE 2", srdSurfsProperty_north.SurroundingSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.2, srdSurfsProperty_north.SurroundingSurfs(2).ViewFactor);
    EXPECT_EQ("SRDSURFS:SURFACE 3", srdSurfsProperty_north.SurroundingSurfs(3).Name);
    EXPECT_DOUBLE_EQ(0.1, srdSurfsProperty_north.SurroundingSurfs(3).ViewFactor);
    // check surrounding surfaces view factors sum (viewed by an exterior surface)
    Real64 srdSurfacesViewFactorSum_result1 = srdSurfsProperty_north.SurroundingSurfs(1).ViewFactor +
                                              srdSurfsProperty_north.SurroundingSurfs(2).ViewFactor +
                                              srdSurfsProperty_north.SurroundingSurfs(3).ViewFactor;
    EXPECT_DOUBLE_EQ(0.5, srdSurfacesViewFactorSum_result1);
    // check the view factors at the exterior north wall
    EXPECT_DOUBLE_EQ(0.5, surface_north_wall.ViewFactorSrdSurfs);
    EXPECT_DOUBLE_EQ(0.4, surface_north_wall.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.1, surface_north_wall.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(1.0, surface_north_wall.ViewFactorSrdSurfs + surface_north_wall.ViewFactorSkyIR + surface_north_wall.ViewFactorGroundIR);

    // test 2: exterior east wall
    srdSurfsPropNum = Util::FindItemInList("SRDSURFS:EAST-WALL", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(2, state->dataSurface->SurfLocalEnvironment(srdSurfsPropNum).SurroundingSurfsPtr);
    surfNum = Util::FindItemInList("EAST-WALL", state->dataSurface->Surface);
    auto &surface_east_wall = state->dataSurface->Surface(surfNum);
    srdSurfsNum = state->dataSurface->Surface(surfNum).SurfSurroundingSurfacesNum;
    auto &srdSurfsProperty_east = state->dataSurface->SurroundingSurfsProperty(srdSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.5, srdSurfsProperty_east.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.2, srdSurfsProperty_east.GroundViewFactor);
    // check surrounding surfaces view factors
    EXPECT_EQ("SRDSURFS:SURFACE 1", srdSurfsProperty_east.SurroundingSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.1, srdSurfsProperty_east.SurroundingSurfs(1).ViewFactor);
    EXPECT_EQ("SRDSURFS:SURFACE 2", srdSurfsProperty_east.SurroundingSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.1, srdSurfsProperty_east.SurroundingSurfs(2).ViewFactor);
    EXPECT_EQ("SRDSURFS:SURFACE 3", srdSurfsProperty_east.SurroundingSurfs(3).Name);
    EXPECT_DOUBLE_EQ(0.1, srdSurfsProperty_east.SurroundingSurfs(3).ViewFactor);
    // check surrounding surfaces view factors sum (viewed by an exterior surface)
    Real64 srdSurfacesViewFactorSum_result2 = srdSurfsProperty_east.SurroundingSurfs(1).ViewFactor +
                                              srdSurfsProperty_east.SurroundingSurfs(2).ViewFactor +
                                              srdSurfsProperty_east.SurroundingSurfs(3).ViewFactor;
    EXPECT_DOUBLE_EQ(0.3, srdSurfacesViewFactorSum_result2);
    // check the view factors at the exterior east wall
    EXPECT_DOUBLE_EQ(0.3, surface_east_wall.ViewFactorSrdSurfs);
    EXPECT_DOUBLE_EQ(0.5, surface_east_wall.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.2, surface_east_wall.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(1.0, surface_east_wall.ViewFactorSrdSurfs + surface_east_wall.ViewFactorSkyIR + surface_east_wall.ViewFactorGroundIR);
}

TEST_F(EnergyPlusFixture, Fix_checkSubSurfAzTiltNorm_Horizontal_Surf_Random)
{
    // Unit Test for Pull Request 10104 that addresses a potential illy functioned (or redundant) `if` condition
    SurfaceData BaseSurface;
    SurfaceData SubSurface;
    SurfaceData SubSurface_Same;
    bool surfaceError;

    // Test Base surf and subsurf normal vectors assignment
    surfaceError = false;

    BaseSurface.Vertex.dimension(4);

    BaseSurface.Vertex = {
        DataVectorTypes::Vector(0, 0, 1), DataVectorTypes::Vector(1, 0, 1), DataVectorTypes::Vector(1, 1, 1), DataVectorTypes::Vector(0, 1, 1)};
    Vectors::CreateNewellSurfaceNormalVector(BaseSurface.Vertex, BaseSurface.Vertex.size(), BaseSurface.NewellSurfaceNormalVector);
    Vectors::DetermineAzimuthAndTilt(BaseSurface.Vertex,
                                     BaseSurface.Azimuth,
                                     BaseSurface.Tilt,
                                     BaseSurface.lcsx,
                                     BaseSurface.lcsy,
                                     BaseSurface.lcsz,
                                     BaseSurface.NewellSurfaceNormalVector);

    SubSurface.Vertex.dimension(4);

    SubSurface.Vertex = {DataVectorTypes::Vector(0, 0, 1),
                         DataVectorTypes::Vector(1, 0, 1),
                         DataVectorTypes::Vector(1, 1, 1.0003),
                         DataVectorTypes::Vector(0, 1, 1.0003)};
    Vectors::CreateNewellSurfaceNormalVector(SubSurface.Vertex, SubSurface.Vertex.size(), SubSurface.NewellSurfaceNormalVector);
    Vectors::DetermineAzimuthAndTilt(SubSurface.Vertex,
                                     SubSurface.Azimuth,
                                     SubSurface.Tilt,
                                     SubSurface.lcsx,
                                     SubSurface.lcsy,
                                     SubSurface.lcsz,
                                     SubSurface.NewellSurfaceNormalVector);

    bool sameSurfNormal(false);

    // This is the sameSurfNormal test used in checkSubSurfAzTiltNorm()
    Vectors::CompareTwoVectors(BaseSurface.NewellSurfaceNormalVector, SubSurface.NewellSurfaceNormalVector, sameSurfNormal, 0.001);

    // The surface normals are not exactly the same
    EXPECT_GE(std::abs(BaseSurface.NewellSurfaceNormalVector.y - SubSurface.NewellSurfaceNormalVector.y), 1e-5);
    EXPECT_GE(std::abs(BaseSurface.NewellSurfaceNormalVector.z - SubSurface.NewellSurfaceNormalVector.z), 1e-10);

    // But should pass the sameSurfNormal test
    EXPECT_TRUE(sameSurfNormal);

    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface, surfaceError);

    // These should pass
    EXPECT_FALSE(surfaceError);
    EXPECT_FALSE(has_err_output());

    // The base and the sub surfaces now should be adjusted to be exactly the same
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.z, SubSurface.lcsz.z);
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.y, SubSurface.lcsz.y);
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.x, SubSurface.lcsz.x);

    // Now do a test with the same SubSurface but with slightly different (but still valid) vertices input order
    // Then the same test should pass all the same with the PR 10104 fix
    // But it would expect to fail in the original develop branch without RP 10104 fix
    SubSurface_Same.Vertex.dimension(4);
    SubSurface_Same.Vertex = {DataVectorTypes::Vector(1, 0, 1),
                              DataVectorTypes::Vector(1, 1, 1.0003),
                              DataVectorTypes::Vector(0, 1, 1.0003),
                              DataVectorTypes::Vector(0, 0, 1)};
    Vectors::CreateNewellSurfaceNormalVector(SubSurface_Same.Vertex, SubSurface_Same.Vertex.size(), SubSurface_Same.NewellSurfaceNormalVector);
    Vectors::DetermineAzimuthAndTilt(SubSurface_Same.Vertex,
                                     SubSurface_Same.Azimuth,
                                     SubSurface_Same.Tilt,
                                     SubSurface_Same.lcsx,
                                     SubSurface_Same.lcsy,
                                     SubSurface_Same.lcsz,
                                     SubSurface_Same.NewellSurfaceNormalVector);

    sameSurfNormal = false;

    // This is the sameSurfNormal test used in checkSubSurfAzTiltNorm()
    Vectors::CompareTwoVectors(BaseSurface.NewellSurfaceNormalVector, SubSurface_Same.NewellSurfaceNormalVector, sameSurfNormal, 0.001);

    // The surface normals are not exactly the same
    EXPECT_GE(std::abs(BaseSurface.NewellSurfaceNormalVector.y - SubSurface_Same.NewellSurfaceNormalVector.y), 1e-5);
    EXPECT_GE(std::abs(BaseSurface.NewellSurfaceNormalVector.z - SubSurface_Same.NewellSurfaceNormalVector.z), 1e-10);

    // But should pass the sameSurfNormal test
    EXPECT_TRUE(sameSurfNormal);

    checkSubSurfAzTiltNorm(*state, BaseSurface, SubSurface_Same, surfaceError);

    // These should pass
    EXPECT_FALSE(surfaceError);
    EXPECT_FALSE(has_err_output());

    // At least one of the following tests are expected to fail in the original develop branch without PR 10104 fix
    // But with PR 10104 fix they should all pass
    // The base and the sub surfaces now should be adjusted to be exactly the same
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.z, SubSurface_Same.lcsz.z);
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.y, SubSurface_Same.lcsz.y);
    EXPECT_DOUBLE_EQ(BaseSurface.lcsz.x, SubSurface_Same.lcsz.x);
}

TEST_F(EnergyPlusFixture, ExtSolarForShadingTest)
{
    // Unit test added as part of the fix for Defect #5949 (certain output variables not being produced for shading elements
    bool ErrorsFound = false;
    std::string const idf_objects = delimited_string({
        "Shading:Building:Detailed,",
        "  ShadeTest1BuildingDetailed, !- Name",
        "  ,                           !- Transmittance Schedule Name",
        "  4,                          !- Number of Vertices",
        "  -20.0,4.0,10.0,             !- X,Y,Z ==> Vertex 1 {m}",
        "  -20.0,0.00,10.0,            !- X,Y,Z ==> Vertex 2 {m}",
        "  -55.0,0.00,0.0,             !- X,Y,Z ==> Vertex 3 {m}",
        "  -55.0,4.0,0.0;              !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Site:Detailed,",
        "  ShadeTest2SiteDetailed, !- Name",
        "  ,                       !- Transmittance Schedule Name",
        "  4,                      !- Number of Vertices",
        "  20.0,14.0,23.0,         !- X,Y,Z ==> Vertex 1 {m}",
        "  20.0,0.00,23.0,         !- X,Y,Z ==> Vertex 2 {m}",
        "  55.0,0.00,23.0,         !- X,Y,Z ==> Vertex 3 {m}",
        "  55.0,14.0,23.0;         !- X,Y,Z ==> Vertex 4 {m}",

        "Shading:Building,",
        "  ShadeTest3Building, !- Name",
        "  123.4,              !- Azimuth Angle {deg}",
        "  45,                 !- Tilt Angle {deg}",
        "  0.1,                !- Starting X Coordinate {m}",
        "  2.3,                !- Starting Y Coordinate {m}",
        "  4.5,                !- Starting Z Coordinate {m}",
        "  6.7,                !- Length {m}",
        "  8.9;                !- Height {m}",

        "Shading:Site,",
        "  ShadeTest4Site, !- Name",
        "  154.1,          !- Azimuth Angle {deg}",
        "  45,             !- Tilt Angle {deg}",
        "  -8.5,           !- Starting X Coordinate {m}",
        "  2.5,            !- Starting Y Coordinate {m}",
        "  6.3,            !- Starting Z Coordinate {m}",
        "  3.6,            !- Length {m}",
        "  1.5;            !- Height {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int TotSurfaces = 8;                                          // Need to double the number of surfaces because E+ will add mirrored surfaces
    state->dataSurfaceGeometry->SurfaceTmp.allocate(TotSurfaces); // Allocate the Surface derived type appropriately
    state->dataSurface->Corner = LowerLeftCorner;
    state->dataSurface->WorldCoordSystem = true;

    int NumSurfs = 0;
    int TotDetachedFixed = 1;
    int TotDetachedBldg = 1;
    int TotRectDetachedFixed = 1;
    int TotRectDetachedBldg = 1;
    SurfaceGeometry::GetDetShdSurfaceData(*state, ErrorsFound, NumSurfs, TotDetachedFixed, TotDetachedBldg);
    SurfaceGeometry::GetRectDetShdSurfaceData(*state, ErrorsFound, NumSurfs, TotRectDetachedFixed, TotRectDetachedBldg);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    // Check set-up of select variables for all four detached shading types
    auto &thisSG = state->dataSurfaceGeometry;

    // First processed surface will be the detailed site shading surface
    EXPECT_EQ(thisSG->SurfaceTmp(1).Name, "SHADETEST2SITEDETAILED");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(1).Class, SurfaceClass::Detached_F);
    EXPECT_FALSE(thisSG->SurfaceTmp(1).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(1).ExtSolar);
    EXPECT_EQ(thisSG->SurfaceTmp(2).Name, "Mir-SHADETEST2SITEDETAILED");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(2).Class, SurfaceClass::Detached_F);
    EXPECT_FALSE(thisSG->SurfaceTmp(2).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(2).ExtSolar);

    // Second processed surface will be the detailed building shading surface
    EXPECT_EQ(thisSG->SurfaceTmp(3).Name, "SHADETEST1BUILDINGDETAILED");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(3).Class, SurfaceClass::Detached_B);
    EXPECT_FALSE(thisSG->SurfaceTmp(3).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(3).ExtSolar);
    EXPECT_EQ(thisSG->SurfaceTmp(4).Name, "Mir-SHADETEST1BUILDINGDETAILED");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(4).Class, SurfaceClass::Detached_B);
    EXPECT_FALSE(thisSG->SurfaceTmp(4).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(4).ExtSolar);

    // Third processed surface will be the site (simple) shading surface
    EXPECT_EQ(thisSG->SurfaceTmp(5).Name, "SHADETEST4SITE");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(5).Class, SurfaceClass::Detached_F);
    EXPECT_FALSE(thisSG->SurfaceTmp(5).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(5).ExtSolar);
    EXPECT_EQ(thisSG->SurfaceTmp(6).Name, "Mir-SHADETEST4SITE");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(6).Class, SurfaceClass::Detached_F);
    EXPECT_FALSE(thisSG->SurfaceTmp(6).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(6).ExtSolar);

    // Fourth processed surface will be the building (simple) shading surface
    EXPECT_EQ(thisSG->SurfaceTmp(7).Name, "SHADETEST3BUILDING");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(7).Class, SurfaceClass::Detached_B);
    EXPECT_FALSE(thisSG->SurfaceTmp(7).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(7).ExtSolar);
    EXPECT_EQ(thisSG->SurfaceTmp(8).Name, "Mir-SHADETEST3BUILDING");
    EXPECT_ENUM_EQ(thisSG->SurfaceTmp(8).Class, SurfaceClass::Detached_B);
    EXPECT_FALSE(thisSG->SurfaceTmp(8).HeatTransSurf);
    EXPECT_TRUE(thisSG->SurfaceTmp(8).ExtSolar);
}
TEST_F(EnergyPlusFixture, SurfaceGeometry_ZoneOutsideBC_SpacesNotInput)
{
    // Two 10x10m zones. A door between both that has a Construction:AirBoundary assigned (its base surface is a regular interior wall)
    std::string const idf_objects = delimited_string({

        "Material,",
        "  8IN Concrete HW,                        !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2033,                                 !- Thickness {m}",
        "  1.72959999999999,                       !- Conductivity {W/m-K}",
        "  2242.99999999999,                       !- Density {kg/m3}",
        "  836.999999999999,                       !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.65,                                   !- Solar Absorptance",
        "  0.65;                                   !- Visible Absorptance",

        "Material,",
        "    Gypsum Board,            !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0159,                  !- Thickness {m}",
        "    0.16,                    !- Conductivity {W/m-K}",
        "    800,                     !- Density {kg/m3}",
        "    1090,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",

        "Construction,",
        "  Regular Construction,                   !- Name",
        "  8IN Concrete HW,                        !- Layer 1",
        "  Gypsum Board;                           !- Layer 2",

        "Zone,",
        "  Zone1;                                  !- Name",

        "Zone,",
        "  Zone2;                                  !- Name",

        "Zone,",
        "  Zone3;                                  !- Name",

        "BuildingSurface:Detailed,",
        "  Space1-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.4384;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Zone,                                   !- Outside Boundary Condition",
        "  Zone2,                                  !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorDoor,                    !- Name",
        "  Door,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Space1-InteriorWall,                    !- Building Surface Name",
        "  ,                                       !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-West,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 0;                                !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-East,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Ceiling,                          !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Floor,                            !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-East,                        !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-North,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-South,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall2,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Zone,                                   !- Outside Boundary Condition",
        "  Zone3,                                  !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorWindow,                  !- Name",
        "  Window,                                 !- Surface Type",
        "  SINGLE PANE HW WINDOW,                  !- Construction Name",
        "  Space1-InteriorWall2,                   !- Building Surface Name",
        "  ,                                       !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        " Construction,",
        "    SINGLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",

        " WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    6.0000001E-03,           !- Thickness {m}",
        "    0.7750000,               !- Solar Transmittance at Normal Incidence",
        "    7.1000002E-02,           !- Front Side Solar Reflectance at Normal Incidence",
        "    7.1000002E-02,           !- Back Side Solar Reflectance at Normal Incidence",
        "    0.8810000,               !- Visible Transmittance at Normal Incidence",
        "    7.9999998E-02,           !- Front Side Visible Reflectance at Normal Incidence",
        "    7.9999998E-02,           !- Back Side Visible Reflectance at Normal Incidence",
        "    0,                       !- Infrared Transmittance at Normal Incidence",
        "    0.8400000,               !- Front Side Infrared Hemispherical Emissivity",
        "    0.8400000,               !- Back Side Infrared Hemispherical Emissivity",
        "    0.9000000;               !- Conductivity {W/m-K}",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    // This is ill-named, but it will shut the "No Ground Temperature" warning when set to false
    state->dataSurfaceGeometry->NoGroundTempObjWarning = false;

    // I don't do ASSERT_NO_THROW because I want to see the err stream to show original defect. But I ASSERT there's no err_stream because we can't
    // continue if it did throw
    EXPECT_NO_THROW(SetupZoneGeometry(*state, ErrorsFound));
    ASSERT_TRUE(compare_err_stream(
        "   ** Warning ** CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; \n"));

    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 3);
    EXPECT_EQ("ZONE1", state->dataViewFactor->EnclRadInfo(1).Name);
    EXPECT_EQ("ZONE2", state->dataViewFactor->EnclRadInfo(2).Name);
    EXPECT_EQ("ZONE3", state->dataViewFactor->EnclRadInfo(3).Name);
    EXPECT_EQ("ZONE1", state->dataViewFactor->EnclRadInfo(1).spaceNames[0]);
    EXPECT_EQ("ZONE2", state->dataViewFactor->EnclRadInfo(2).spaceNames[0]);
    EXPECT_EQ("ZONE3", state->dataViewFactor->EnclRadInfo(3).spaceNames[0]);
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 3);
    EXPECT_EQ(state->dataHeatBal->space(1).surfaces.size(), 9);
    EXPECT_EQ(state->dataHeatBal->space(2).surfaces.size(), 7);
    EXPECT_EQ(state->dataHeatBal->space(3).surfaces.size(), 7);

    int originalIntWallNum = Util::FindItemInList("SPACE1-INTERIORWALL", state->dataSurface->Surface);
    int originalIntDoorNum = Util::FindItemInList("SPACE1-INTERIORDOOR", state->dataSurface->Surface);
    int originalIntWall2Num = Util::FindItemInList("SPACE1-INTERIORWALL2", state->dataSurface->Surface);
    int originalIntWinNum = Util::FindItemInList("SPACE1-INTERIORWINDOW", state->dataSurface->Surface);
    int izIntWallNum = Util::FindItemInList("iz-SPACE1-INTERIORWALL", state->dataSurface->Surface);
    int izIntDoorNum = Util::FindItemInList("iz-SPACE1-INTERIORDOOR", state->dataSurface->Surface);
    int izIntWall2Num = Util::FindItemInList("iz-SPACE1-INTERIORWALL2", state->dataSurface->Surface);
    int izIntWinNum = Util::FindItemInList("iz-SPACE1-INTERIORWINDOW", state->dataSurface->Surface);
    auto &originalIntWall = state->dataSurface->Surface(originalIntWallNum);
    auto &originalIntDoor = state->dataSurface->Surface(originalIntDoorNum);
    auto &originalIntWall2 = state->dataSurface->Surface(originalIntWall2Num);
    auto &originalIntWin = state->dataSurface->Surface(originalIntWinNum);
    auto &izIntWall = state->dataSurface->Surface(izIntWallNum);
    auto &izIntDoor = state->dataSurface->Surface(izIntDoorNum);
    auto &izIntWall2 = state->dataSurface->Surface(izIntWall2Num);
    auto &izIntWin = state->dataSurface->Surface(izIntWinNum);

    // Original wall input with Zone outside boundary condition
    // Now points to auto-generated wall in Zone2
    EXPECT_EQ(originalIntWall.Zone, 1);
    EXPECT_EQ(originalIntWall.spaceNum, 1);
    EXPECT_EQ(originalIntWall.ExtBoundCond, izIntWallNum);
    EXPECT_EQ(originalIntWall.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWall.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated wall in Zone2, points to original wall in Zone1
    EXPECT_EQ(izIntWall.Zone, 2);
    EXPECT_EQ(izIntWall.spaceNum, 2);
    EXPECT_EQ(izIntWall.ExtBoundCond, originalIntWallNum);
    EXPECT_EQ(izIntWall.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(izIntWall.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original wall input with Zone outside boundary condition
    // Now points to auto-generated wall in Zone3, Space3
    EXPECT_EQ(originalIntWall2.Zone, 1);
    EXPECT_EQ(originalIntWall2.spaceNum, 1);
    EXPECT_EQ(originalIntWall2.ExtBoundCond, izIntWall2Num);
    EXPECT_EQ(originalIntWall2.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWall2.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated wall in Zone3, Space3, points to original wall in Zone1
    EXPECT_EQ(izIntWall2.Zone, 3);
    EXPECT_EQ(izIntWall2.spaceNum, 3);
    EXPECT_EQ(izIntWall2.ExtBoundCond, originalIntWall2Num);
    EXPECT_EQ(izIntWall2.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(izIntWall2.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original Door input with blank outside boundary condition, inherits base surface boundary condition
    // Now points to auto-generated door in Zone2
    EXPECT_EQ(originalIntDoor.Zone, 1);
    EXPECT_EQ(originalIntDoor.spaceNum, 1);
    EXPECT_EQ(originalIntDoor.ExtBoundCond, izIntDoorNum);
    EXPECT_EQ(originalIntDoor.Class, DataSurfaces::SurfaceClass::Door);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntDoor.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated door in Zone2, points to original door in Zone1
    EXPECT_EQ(izIntDoor.Zone, 2);
    EXPECT_EQ(izIntDoor.spaceNum, 2);
    EXPECT_EQ(izIntDoor.ExtBoundCond, originalIntDoorNum);
    EXPECT_EQ(izIntDoor.Class, DataSurfaces::SurfaceClass::Door);
    EXPECT_EQ(state->dataConstruction->Construct(izIntDoor.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original Window input with blank outside boundary condition, inherits base surface boundary condition
    // Now points to auto-generated window in Zone3, Space3
    EXPECT_EQ(originalIntWin.Zone, 1);
    EXPECT_EQ(originalIntWin.spaceNum, 1);
    EXPECT_EQ(originalIntWin.ExtBoundCond, izIntWinNum);
    EXPECT_EQ(originalIntWin.Class, DataSurfaces::SurfaceClass::Window);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWin.Construction).Name, "SINGLE PANE HW WINDOW");
    // Auto-generated window in Zone3, points to original window in Zone1
    EXPECT_EQ(izIntWin.Zone, 3);
    EXPECT_EQ(izIntWin.spaceNum, 3);
    EXPECT_EQ(izIntWin.ExtBoundCond, originalIntWinNum);
    EXPECT_EQ(izIntWin.Class, DataSurfaces::SurfaceClass::Window);
    // SINGLE PANE HW WINDOW is a single-layer construction, so the same construction works for both sides
    EXPECT_EQ(state->dataConstruction->Construct(izIntWin.Construction).Name, "SINGLE PANE HW WINDOW");
}
TEST_F(EnergyPlusFixture, SurfaceGeometry_ZoneOutsideBC_SpacesInput)
{
    // Two 10x10m zones. A door between both that has a Construction:AirBoundary assigned (its base surface is a regular interior wall)
    std::string const idf_objects = delimited_string({

        "Material,",
        "  8IN Concrete HW,                        !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2033,                                 !- Thickness {m}",
        "  1.72959999999999,                       !- Conductivity {W/m-K}",
        "  2242.99999999999,                       !- Density {kg/m3}",
        "  836.999999999999,                       !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.65,                                   !- Solar Absorptance",
        "  0.65;                                   !- Visible Absorptance",

        "Material,",
        "    Gypsum Board,            !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0159,                  !- Thickness {m}",
        "    0.16,                    !- Conductivity {W/m-K}",
        "    800,                     !- Density {kg/m3}",
        "    1090,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",

        "Construction,",
        "  Regular Construction,                   !- Name",
        "  8IN Concrete HW,                        !- Layer 1",
        "  Gypsum Board;                           !- Layer 2",

        "Zone,",
        "  Zone1;                                  !- Name",

        "Zone,",
        "  Zone2;                                  !- Name",

        "Zone,",
        "  Zone3;                                  !- Name",

        "Space,",
        "  Space1,                                 !- Name",
        "  Zone1;                                  !- Zone Name",

        "Space,",
        "  Space2,                                 !- Name",
        "  Zone2;                                  !- Zone Name",

        "Space,",
        "  Space3a,                                 !- Name",
        "  Zone3;                                  !- Zone Name",

        "Space,",
        "  Space3b,                                 !- Name",
        "  Zone3;                                  !- Zone Name",

        "BuildingSurface:Detailed,",
        "  Space1-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.4384;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Zone,                                   !- Outside Boundary Condition",
        "  Zone2,                                  !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorDoor,                    !- Name",
        "  Door,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Space1-InteriorWall,                    !- Building Surface Name",
        "  ,                                       !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-West,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 0;                                !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-East,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Ceiling,                          !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3a,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Floor,                            !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3a,                                !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-East,                        !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3b,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-North,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3b,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-South,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3a,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall2,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Zone,                                   !- Outside Boundary Condition",
        "  Zone3,                                  !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorWindow,                  !- Name",
        "  Window,                                 !- Surface Type",
        "  SINGLE PANE HW WINDOW,                  !- Construction Name",
        "  Space1-InteriorWall2,                   !- Building Surface Name",
        "  ,                                       !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        " Construction,",
        "    SINGLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",

        " WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    6.0000001E-03,           !- Thickness {m}",
        "    0.7750000,               !- Solar Transmittance at Normal Incidence",
        "    7.1000002E-02,           !- Front Side Solar Reflectance at Normal Incidence",
        "    7.1000002E-02,           !- Back Side Solar Reflectance at Normal Incidence",
        "    0.8810000,               !- Visible Transmittance at Normal Incidence",
        "    7.9999998E-02,           !- Front Side Visible Reflectance at Normal Incidence",
        "    7.9999998E-02,           !- Back Side Visible Reflectance at Normal Incidence",
        "    0,                       !- Infrared Transmittance at Normal Incidence",
        "    0.8400000,               !- Front Side Infrared Hemispherical Emissivity",
        "    0.8400000,               !- Back Side Infrared Hemispherical Emissivity",
        "    0.9000000;               !- Conductivity {W/m-K}",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    // This is ill-named, but it will shut the "No Ground Temperature" warning when set to false
    state->dataSurfaceGeometry->NoGroundTempObjWarning = false;

    // I don't do ASSERT_NO_THROW because I want to see the err stream to show original defect. But I ASSERT there's no err_stream because we can't
    // continue if it did throw
    EXPECT_NO_THROW(SetupZoneGeometry(*state, ErrorsFound));

    std::string const error_string = delimited_string(
        {"   ** Warning ** CreateMissingSpaces: Surface=\"SPACE1-INTERIORWALL2\" has Outside Boundary Condition=Zone, but Zone=\"ZONE3\" has more "
         "than 1 Space.",
         "   **   ~~~   ** Auto-generated surface=\"iz-SPACE1-INTERIORWALL2\" will be assigned to Space=\"SPACE3B\"",
         "   **   ~~~   ** Use Outside Boundary Condition = Space to specify the exact Space for the outside boundary.",
         "   ** Warning ** CreateMissingSpaces: Surface=\"SPACE1-INTERIORWINDOW\" has Outside Boundary Condition=Zone, but Zone=\"ZONE3\" has more "
         "than 1 Space.",
         "   **   ~~~   ** Auto-generated surface=\"iz-SPACE1-INTERIORWINDOW\" will be assigned to Space=\"SPACE3B\"",
         "   **   ~~~   ** Use Outside Boundary Condition = Space to specify the exact Space for the outside boundary.",
         "   ** Warning ** CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; "});

    ASSERT_TRUE(compare_err_stream(error_string));

    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 4);
    EXPECT_EQ("SPACE1", state->dataViewFactor->EnclRadInfo(1).Name);
    EXPECT_EQ("SPACE2", state->dataViewFactor->EnclRadInfo(2).Name);
    EXPECT_EQ("SPACE3A", state->dataViewFactor->EnclRadInfo(3).Name);
    EXPECT_EQ("SPACE3B", state->dataViewFactor->EnclRadInfo(4).Name);
    EXPECT_EQ("SPACE1", state->dataViewFactor->EnclRadInfo(1).spaceNames[0]);
    EXPECT_EQ("SPACE2", state->dataViewFactor->EnclRadInfo(2).spaceNames[0]);
    EXPECT_EQ("SPACE3A", state->dataViewFactor->EnclRadInfo(3).spaceNames[0]);
    EXPECT_EQ("SPACE3B", state->dataViewFactor->EnclRadInfo(4).spaceNames[0]);
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 3);
    EXPECT_EQ(state->dataHeatBal->space(4).radiantEnclosureNum, 4);
    EXPECT_EQ(state->dataHeatBal->space(1).surfaces.size(), 9);
    EXPECT_EQ(state->dataHeatBal->space(2).surfaces.size(), 7);
    EXPECT_EQ(state->dataHeatBal->space(3).surfaces.size(), 3);
    EXPECT_EQ(state->dataHeatBal->space(4).surfaces.size(), 4);

    int originalIntWallNum = Util::FindItemInList("SPACE1-INTERIORWALL", state->dataSurface->Surface);
    int originalIntDoorNum = Util::FindItemInList("SPACE1-INTERIORDOOR", state->dataSurface->Surface);
    int originalIntWall2Num = Util::FindItemInList("SPACE1-INTERIORWALL2", state->dataSurface->Surface);
    int originalIntWinNum = Util::FindItemInList("SPACE1-INTERIORWINDOW", state->dataSurface->Surface);
    int izIntWallNum = Util::FindItemInList("iz-SPACE1-INTERIORWALL", state->dataSurface->Surface);
    int izIntDoorNum = Util::FindItemInList("iz-SPACE1-INTERIORDOOR", state->dataSurface->Surface);
    int izIntWall2Num = Util::FindItemInList("iz-SPACE1-INTERIORWALL2", state->dataSurface->Surface);
    int izIntWinNum = Util::FindItemInList("iz-SPACE1-INTERIORWINDOW", state->dataSurface->Surface);
    auto &originalIntWall = state->dataSurface->Surface(originalIntWallNum);
    auto &originalIntDoor = state->dataSurface->Surface(originalIntDoorNum);
    auto &originalIntWall2 = state->dataSurface->Surface(originalIntWall2Num);
    auto &originalIntWin = state->dataSurface->Surface(originalIntWinNum);
    auto &izIntWall = state->dataSurface->Surface(izIntWallNum);
    auto &izIntDoor = state->dataSurface->Surface(izIntDoorNum);
    auto &izIntWall2 = state->dataSurface->Surface(izIntWall2Num);
    auto &izIntWin = state->dataSurface->Surface(izIntWinNum);
    // Original wall input with Zone outside boundary condition
    // Now points to auto-generated wall in Zone2
    EXPECT_EQ(originalIntWall.Zone, 1);
    EXPECT_EQ(originalIntWall.spaceNum, 1);
    EXPECT_EQ(originalIntWall.ExtBoundCond, izIntWallNum);
    EXPECT_EQ(originalIntWall.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWall.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated wall in Zone2, points to original wall in Zone1
    EXPECT_EQ(izIntWall.Zone, 2);
    EXPECT_EQ(izIntWall.spaceNum, 2);
    EXPECT_EQ(izIntWall.ExtBoundCond, originalIntWallNum);
    EXPECT_EQ(izIntWall.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(izIntWall.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original wall input with Zone outside boundary condition
    // Now points to auto-generated wall in Zone3, Space3B
    EXPECT_EQ(originalIntWall2.Zone, 1);
    EXPECT_EQ(originalIntWall2.spaceNum, 1);
    EXPECT_EQ(originalIntWall2.ExtBoundCond, izIntWall2Num);
    EXPECT_EQ(originalIntWall2.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWall2.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated wall in Zone3, Space3B, points to original wall in Zone1
    EXPECT_EQ(izIntWall2.Zone, 3);
    EXPECT_EQ(izIntWall2.spaceNum, 4);
    EXPECT_EQ(izIntWall2.ExtBoundCond, originalIntWall2Num);
    EXPECT_EQ(izIntWall2.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(izIntWall2.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original Door input with blank outside boundary condition, inherits base surface boundary condition
    // Now points to auto-generated door in Zone2
    EXPECT_EQ(originalIntDoor.Zone, 1);
    EXPECT_EQ(originalIntDoor.spaceNum, 1);
    EXPECT_EQ(originalIntDoor.ExtBoundCond, izIntDoorNum);
    EXPECT_EQ(originalIntDoor.Class, DataSurfaces::SurfaceClass::Door);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntDoor.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated door in Zone2, points to original door in Zone1
    EXPECT_EQ(izIntDoor.Zone, 2);
    EXPECT_EQ(izIntDoor.spaceNum, 2);
    EXPECT_EQ(izIntDoor.ExtBoundCond, originalIntDoorNum);
    EXPECT_EQ(izIntDoor.Class, DataSurfaces::SurfaceClass::Door);
    EXPECT_EQ(state->dataConstruction->Construct(izIntDoor.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original Window input with blank outside boundary condition, inherits base surface boundary condition
    // Now points to auto-generated window in Zone3, Space3B
    EXPECT_EQ(originalIntWin.Zone, 1);
    EXPECT_EQ(originalIntWin.spaceNum, 1);
    EXPECT_EQ(originalIntWin.ExtBoundCond, izIntWinNum);
    EXPECT_EQ(originalIntWin.Class, DataSurfaces::SurfaceClass::Window);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWin.Construction).Name, "SINGLE PANE HW WINDOW");
    // Auto-generated window in Zone3, Space3B, points to original window in Zone1
    EXPECT_EQ(izIntWin.Zone, 3);
    EXPECT_EQ(izIntWin.spaceNum, 4);
    EXPECT_EQ(izIntWin.ExtBoundCond, originalIntWinNum);
    EXPECT_EQ(izIntWin.Class, DataSurfaces::SurfaceClass::Window);
    // SINGLE PANE HW WINDOW is a single-layer construction, so the same construction works for both sides
    EXPECT_EQ(state->dataConstruction->Construct(izIntWin.Construction).Name, "SINGLE PANE HW WINDOW");
}
TEST_F(EnergyPlusFixture, SurfaceGeometry_SpaceOutsideBC_SpacesInput)
{
    // Two 10x10m zones. A door between both that has a Construction:AirBoundary assigned (its base surface is a regular interior wall)
    std::string const idf_objects = delimited_string({

        "Material,",
        "  8IN Concrete HW,                        !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2033,                                 !- Thickness {m}",
        "  1.72959999999999,                       !- Conductivity {W/m-K}",
        "  2242.99999999999,                       !- Density {kg/m3}",
        "  836.999999999999,                       !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.65,                                   !- Solar Absorptance",
        "  0.65;                                   !- Visible Absorptance",

        "Material,",
        "    Gypsum Board,            !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0159,                  !- Thickness {m}",
        "    0.16,                    !- Conductivity {W/m-K}",
        "    800,                     !- Density {kg/m3}",
        "    1090,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    0.7;                     !- Visible Absorptance",

        "Construction,",
        "  Regular Construction,                   !- Name",
        "  8IN Concrete HW,                        !- Layer 1",
        "  Gypsum Board;                           !- Layer 2",

        "Zone,",
        "  Zone1;                                  !- Name",

        "Zone,",
        "  Zone2;                                  !- Name",

        "Zone,",
        "  Zone3;                                  !- Name",

        "Space,",
        "  Space1,                                 !- Name",
        "  Zone1;                                  !- Zone Name",

        "Space,",
        "  Space2,                                 !- Name",
        "  Zone2;                                  !- Zone Name",

        "Space,",
        "  Space3a,                                 !- Name",
        "  Zone3;                                  !- Zone Name",

        "Space,",
        "  Space3b,                                 !- Name",
        "  Zone3;                                  !- Zone Name",

        "BuildingSurface:Detailed,",
        "  Space1-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.4384;                           !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Space,                                  !- Outside Boundary Condition",
        "  Space2,                                 !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorDoor,                    !- Name",
        "  Door,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Space1-InteriorWall,                    !- Building Surface Name",
        "  ,                                       !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-Wall-West,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.4384,                           !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 0;                                !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Ceiling,                         !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Floor,                           !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-East,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-North,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space2-Wall-South,                      !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone2,                                  !- Zone Name",
        "  Space2,                                 !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Ceiling,                          !- Name",
        "  Roof,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3a,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.4384;                          !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Floor,                            !- Name",
        "  Floor,                                  !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3a,                                !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 0,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-East,                        !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3b,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-North,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3b,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.4384,                         !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Zone3-Wall-South,                       !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone3,                                  !- Zone Name",
        "  Space3a,                                !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.4384,                          !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Space1-InteriorWall2,                    !- Name",
        "  Wall,                                   !- Surface Type",
        "  Regular Construction,                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  Space1,                                 !- Space Name",
        "  Space,                                  !- Outside Boundary Condition",
        "  Space3a,                                !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.4384,                         !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 2.4384,                          !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 0;                              !- X,Y,Z Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "  Space1-InteriorWindow,                  !- Name",
        "  Window,                                 !- Surface Type",
        "  SINGLE PANE HW WINDOW,                  !- Construction Name",
        "  Space1-InteriorWall2,                   !- Building Surface Name",
        "  ,                                       !- Outside Boundary Condition Object",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Frame and Divider Name",
        "  ,                                       !- Multiplier",
        "  ,                                       !- Number of Vertices",
        "  10, 7.05, 2,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 7.05, 0,                            !- X,Y,Z Vertex 2 {m}",
        "  10, 7.95, 0,                            !- X,Y,Z Vertex 3 {m}",
        "  10, 7.95, 2;                            !- X,Y,Z Vertex 4 {m}",

        " Construction,",
        "    SINGLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",

        " WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    6.0000001E-03,           !- Thickness {m}",
        "    0.7750000,               !- Solar Transmittance at Normal Incidence",
        "    7.1000002E-02,           !- Front Side Solar Reflectance at Normal Incidence",
        "    7.1000002E-02,           !- Back Side Solar Reflectance at Normal Incidence",
        "    0.8810000,               !- Visible Transmittance at Normal Incidence",
        "    7.9999998E-02,           !- Front Side Visible Reflectance at Normal Incidence",
        "    7.9999998E-02,           !- Back Side Visible Reflectance at Normal Incidence",
        "    0,                       !- Infrared Transmittance at Normal Incidence",
        "    0.8400000,               !- Front Side Infrared Hemispherical Emissivity",
        "    0.8400000,               !- Back Side Infrared Hemispherical Emissivity",
        "    0.9000000;               !- Conductivity {W/m-K}",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);            // expect no errors

    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    // This is ill-named, but it will shut the "No Ground Temperature" warning when set to false
    state->dataSurfaceGeometry->NoGroundTempObjWarning = false;

    // I don't do ASSERT_NO_THROW because I want to see the err stream to show original defect. But I ASSERT there's no err_stream because we can't
    // continue if it did throw
    EXPECT_NO_THROW(SetupZoneGeometry(*state, ErrorsFound));
    ASSERT_TRUE(compare_err_stream(
        "   ** Warning ** CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; \n"));

    EXPECT_FALSE(ErrorsFound); // expect no errors

    EXPECT_EQ(state->dataViewFactor->NumOfRadiantEnclosures, 4);
    EXPECT_EQ("SPACE1", state->dataViewFactor->EnclRadInfo(1).Name);
    EXPECT_EQ("SPACE2", state->dataViewFactor->EnclRadInfo(2).Name);
    EXPECT_EQ("SPACE3A", state->dataViewFactor->EnclRadInfo(3).Name);
    EXPECT_EQ("SPACE3B", state->dataViewFactor->EnclRadInfo(4).Name);
    EXPECT_EQ("SPACE1", state->dataViewFactor->EnclRadInfo(1).spaceNames[0]);
    EXPECT_EQ("SPACE2", state->dataViewFactor->EnclRadInfo(2).spaceNames[0]);
    EXPECT_EQ("SPACE3A", state->dataViewFactor->EnclRadInfo(3).spaceNames[0]);
    EXPECT_EQ("SPACE3B", state->dataViewFactor->EnclRadInfo(4).spaceNames[0]);
    EXPECT_EQ(state->dataHeatBal->space(1).radiantEnclosureNum, 1);
    EXPECT_EQ(state->dataHeatBal->space(2).radiantEnclosureNum, 2);
    EXPECT_EQ(state->dataHeatBal->space(3).radiantEnclosureNum, 3);
    EXPECT_EQ(state->dataHeatBal->space(4).radiantEnclosureNum, 4);
    EXPECT_EQ(state->dataHeatBal->space(1).surfaces.size(), 9);
    EXPECT_EQ(state->dataHeatBal->space(2).surfaces.size(), 7);
    EXPECT_EQ(state->dataHeatBal->space(3).surfaces.size(), 5);
    EXPECT_EQ(state->dataHeatBal->space(4).surfaces.size(), 2);

    int originalIntWallNum = Util::FindItemInList("SPACE1-INTERIORWALL", state->dataSurface->Surface);
    int originalIntDoorNum = Util::FindItemInList("SPACE1-INTERIORDOOR", state->dataSurface->Surface);
    int originalIntWall2Num = Util::FindItemInList("SPACE1-INTERIORWALL2", state->dataSurface->Surface);
    int originalIntWinNum = Util::FindItemInList("SPACE1-INTERIORWINDOW", state->dataSurface->Surface);
    int izIntWallNum = Util::FindItemInList("iz-SPACE1-INTERIORWALL", state->dataSurface->Surface);
    int izIntDoorNum = Util::FindItemInList("iz-SPACE1-INTERIORDOOR", state->dataSurface->Surface);
    int izIntWall2Num = Util::FindItemInList("iz-SPACE1-INTERIORWALL2", state->dataSurface->Surface);
    int izIntWinNum = Util::FindItemInList("iz-SPACE1-INTERIORWINDOW", state->dataSurface->Surface);
    auto &originalIntWall = state->dataSurface->Surface(originalIntWallNum);
    auto &originalIntDoor = state->dataSurface->Surface(originalIntDoorNum);
    auto &originalIntWall2 = state->dataSurface->Surface(originalIntWall2Num);
    auto &originalIntWin = state->dataSurface->Surface(originalIntWinNum);
    auto &izIntWall = state->dataSurface->Surface(izIntWallNum);
    auto &izIntDoor = state->dataSurface->Surface(izIntDoorNum);
    auto &izIntWall2 = state->dataSurface->Surface(izIntWall2Num);
    auto &izIntWin = state->dataSurface->Surface(izIntWinNum);
    // Original wall input with Zone outside boundary condition
    // Now points to auto-generated wall in Zone2
    EXPECT_EQ(originalIntWall.Zone, 1);
    EXPECT_EQ(originalIntWall.spaceNum, 1);
    EXPECT_EQ(originalIntWall.ExtBoundCond, izIntWallNum);
    EXPECT_EQ(originalIntWall.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWall.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated wall in Zone2, points to original wall in Zone1
    EXPECT_EQ(izIntWall.Zone, 2);
    EXPECT_EQ(izIntWall.spaceNum, 2);
    EXPECT_EQ(izIntWall.ExtBoundCond, originalIntWallNum);
    EXPECT_EQ(izIntWall.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(izIntWall.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original wall input with Zone outside boundary condition
    // Now points to auto-generated wall in Zone3, Space3B
    EXPECT_EQ(originalIntWall2.Zone, 1);
    EXPECT_EQ(originalIntWall2.spaceNum, 1);
    EXPECT_EQ(originalIntWall2.ExtBoundCond, izIntWall2Num);
    EXPECT_EQ(originalIntWall2.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWall2.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated wall in Zone3, Space3A, points to original wall in Zone1
    EXPECT_EQ(izIntWall2.Zone, 3);
    EXPECT_EQ(izIntWall2.spaceNum, 3);
    EXPECT_EQ(izIntWall2.ExtBoundCond, originalIntWall2Num);
    EXPECT_EQ(izIntWall2.Class, DataSurfaces::SurfaceClass::Wall);
    EXPECT_EQ(state->dataConstruction->Construct(izIntWall2.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original Door input with blank outside boundary condition, inherits base surface boundary condition
    // Now points to auto-generated door in Zone2
    EXPECT_EQ(originalIntDoor.Zone, 1);
    EXPECT_EQ(originalIntDoor.spaceNum, 1);
    EXPECT_EQ(originalIntDoor.ExtBoundCond, izIntDoorNum);
    EXPECT_EQ(originalIntDoor.Class, DataSurfaces::SurfaceClass::Door);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntDoor.Construction).Name, "REGULAR CONSTRUCTION");
    // Auto-generated door in Zone2, points to original door in Zone1
    EXPECT_EQ(izIntDoor.Zone, 2);
    EXPECT_EQ(izIntDoor.spaceNum, 2);
    EXPECT_EQ(izIntDoor.ExtBoundCond, originalIntDoorNum);
    EXPECT_EQ(izIntDoor.Class, DataSurfaces::SurfaceClass::Door);
    EXPECT_EQ(state->dataConstruction->Construct(izIntDoor.Construction).Name, "iz-REGULAR CONSTRUCTION");
    // Original Window input with blank outside boundary condition, inherits base surface boundary condition
    // Now points to auto-generated window in Zone3, Space3B
    EXPECT_EQ(originalIntWin.Zone, 1);
    EXPECT_EQ(originalIntWin.spaceNum, 1);
    EXPECT_EQ(originalIntWin.ExtBoundCond, izIntWinNum);
    EXPECT_EQ(originalIntWin.Class, DataSurfaces::SurfaceClass::Window);
    EXPECT_EQ(state->dataConstruction->Construct(originalIntWin.Construction).Name, "SINGLE PANE HW WINDOW");
    // Auto-generated window in Zone3, Space3a points to original window in Zone1
    EXPECT_EQ(izIntWin.Zone, 3);
    EXPECT_EQ(izIntWin.spaceNum, 3);
    EXPECT_EQ(izIntWin.ExtBoundCond, originalIntWinNum);
    EXPECT_EQ(izIntWin.Class, DataSurfaces::SurfaceClass::Window);
    // SINGLE PANE HW WINDOW is a single-layer construction, so the same construction works for both sides
    EXPECT_EQ(state->dataConstruction->Construct(izIntWin.Construction).Name, "SINGLE PANE HW WINDOW");
}
