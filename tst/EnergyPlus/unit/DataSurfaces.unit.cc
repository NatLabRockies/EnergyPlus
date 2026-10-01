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

// EnergyPlus::DataSurfaces Unit Tests

// C++ Headers
#include <cmath>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/Vectors.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::SurfaceGeometry;
using DataVectorTypes::Vector;

TEST_F(EnergyPlusFixture, DataSurfaces_SetSurfaceOutBulbTempAtTest)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({

        "	BuildingSurface:Detailed,",
        "    T3-RF1 - Floor:n,        !- Name",
        "    Floor,                   !- Surface Type",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Construction Name",
        "    T3-RF1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                        !- View Factor to Ground",
        "    ,                        !- Number of Vertices",
        "    -73.4395447868102,       !- Vertex 1 X-coordinate {m}",
        "    115.81641271866,         !- Vertex 1 Y-coordinate {m}",
        "    25000,                   !- Vertex 1 Z-coordinate {m}",
        "    -58.0249751030646,       !- Vertex 2 X-coordinate {m}",
        "    93.1706338416311,        !- Vertex 2 Y-coordinate {m}",
        "    25000,                   !- Vertex 2 Z-coordinate {m}",
        "    -68.9295447868101,       !- Vertex 3 X-coordinate {m}",
        "    74.3054685889134,        !- Vertex 3 Y-coordinate {m}",
        "    25000,                   !- Vertex 3 Z-coordinate {m}",
        "    -58.0345461881513,       !- Vertex 4 X-coordinate {m}",
        "    93.1761597101821,        !- Vertex 4 Y-coordinate {m}",
        "    25000;                   !- Vertex 4 Z-coordinate {m}",

        "Zone,",
        "    T3-RF1,                  !- Name",
        "    60,                      !- Direction of Relative North {deg}",
        "    234.651324196041,        !- X Origin {m}",
        "    -132.406575100608,       !- Y Origin {m}",
        "    14.8000000000003,        !- Z Origin {m}",
        "    ,                        !- Type",
        "    ,                        !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    ,                        !- Volume {m3}",
        "    ,                        !- Floor Area {m2}",
        "    ,                        !- Zone Inside Convection Algorithm",
        "    ,                        !- Zone Outside Convection Algorithm",
        "    No;                      !- Part of Total Floor Area",

        "Construction,",
        "    ExtSlabCarpet 4in ClimateZone 1-8,  !- Name",
        "    MAT-CC05 4 HW CONCRETE,  !- Outside Layer",
        "    CP02 CARPET PAD;         !- Layer 2",

        "Material,",
        "    MAT-CC05 4 HW CONCRETE,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1016,                  !- Thickness {m}",
        "    1.311,                   !- Conductivity {W/m-K}",
        "    2240,                    !- Density {kg/m3}",
        "    836.800000000001,        !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.85,                    !- Solar Absorptance",
        "    0.85;                    !- Visible Absorptance",

        "Material:NoMass,",
        "    CP02 CARPET PAD,         !- Name",
        "    Smooth,                  !- Roughness",
        "    0.1,                     !- Thermal Resistance {m2-K/W}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.8,                     !- Solar Absorptance",
        "    0.8;                     !- Visible Absorptance",

        "SurfaceConvectionAlgorithm:Inside,TARP;",

        "SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "HeatBalanceAlgorithm,ConductionTransferFunction;",

        "ZoneAirHeatBalanceAlgorithm,",
        "    AnalyticalSolution;      !- Algorithm",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                  // expect no errors

    ErrorsFound = false;
    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    ErrorsFound = false;
    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    ErrorsFound = false;
    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    ErrorsFound = false;
    GetSurfaceData(*state, ErrorsFound); // setup zone geometry and get zone data
    EXPECT_FALSE(ErrorsFound);           // expect no errors
    SetSurfaceOutBulbTempAt(*state);
    EXPECT_EQ("T3-RF1 - FLOOR:N", state->dataSurface->Surface(1).Name);
    EXPECT_GT(state->dataSurface->Surface(1).Centroid.z, 20000.0); // this condition is fatal
    EXPECT_LT(state->dataSurface->SurfOutDryBulbTemp(1), -100.0);  // this condition is fatal
    EXPECT_LT(state->dataSurface->SurfOutWetBulbTemp(1), -100.0);  // this condition is fatal
}

TEST_F(EnergyPlusFixture, SurfaceTest_Plane)
{
    {
        SurfaceData s;
        s.Vertex.dimension(3);
        s.Vertex = {Vector(1, 1, 1), Vector(-1, 1, 0), Vector(2, 0, 3)};
        s.Shape = SurfaceShape::Triangle;
        s.set_computed_geometry();

        EXPECT_DOUBLE_EQ(-1.0, s.plane.x);
        EXPECT_DOUBLE_EQ(3.0, s.plane.y);
        EXPECT_DOUBLE_EQ(2.0, s.plane.z);
        EXPECT_DOUBLE_EQ(-4.0, s.plane.w);
    }
    {
        SurfaceData s;
        s.Vertex.dimension(3);
        s.Vertex = {Vector(2, 1, -1), Vector(0, -2, 0), Vector(1, -1, 2)};
        s.Shape = SurfaceShape::Triangle;
        s.set_computed_geometry();

        EXPECT_DOUBLE_EQ(-7.0, s.plane.x);
        EXPECT_DOUBLE_EQ(5.0, s.plane.y);
        EXPECT_DOUBLE_EQ(1.0, s.plane.z);
        EXPECT_DOUBLE_EQ(10.0, s.plane.w);
    }
}

TEST_F(EnergyPlusFixture, SurfaceTest_Surface2D)
{
    {
        using Vector2D = Surface2D::Vector2D;
        SurfaceData s;
        s.Vertex.dimension(4);
        s.Vertex = {Vector(0, 0, 0), Vector(1, 0, 0), Vector(1, 1, 0), Vector(0, 1, 0)};
        s.Shape = SurfaceShape::Rectangle;
        s.set_computed_geometry();

        Surface2D const &s2d(s.surface2d);
        EXPECT_EQ(2, s2d.axis); // Projection along z axis
        EXPECT_EQ(Vector2D(0, 0), s2d.vertices[0]);
        EXPECT_EQ(Vector2D(1, 0), s2d.vertices[1]);
        EXPECT_EQ(Vector2D(1, 1), s2d.vertices[2]);
        EXPECT_EQ(Vector2D(0, 1), s2d.vertices[3]);
        EXPECT_DOUBLE_EQ(0.0, s2d.vl.x);
        EXPECT_DOUBLE_EQ(0.0, s2d.vl.y);
        EXPECT_DOUBLE_EQ(1.0, s2d.vu.x);
        EXPECT_DOUBLE_EQ(1.0, s2d.vu.y);
    }
}

TEST_F(EnergyPlusFixture, SurfaceTest_Surface2D_bigVertices)
{
    using Vector2D = Surface2D::Vector2D;
    state->dataSurface->TotSurfaces = 1;
    constexpr int surfNum = 1;
    int nVertices = 22;
    state->dataSurface->MaxVerticesPerSurface = nVertices;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);
    auto &s = state->dataSurfaceGeometry->SurfaceTmp(surfNum);
    s.Vertex.dimension(nVertices);
    s.Sides = nVertices;

    // convex, >nVerticesBig, simple shape
    s.Vertex = {Vector(0, 0, 0),     Vector(0.1, 1.0, 0), Vector(0.2, 1.9, 0), Vector(0.3, 2.7, 0), Vector(0.4, 3.4, 0), Vector(0.5, 4.0, 0),
                Vector(0.6, 4.5, 0), Vector(0.7, 4.9, 0), Vector(0.8, 5.2, 0), Vector(0.9, 5.4, 0), Vector(1.0, 5.5, 0), Vector(1.1, 5.4, 0),
                Vector(1.2, 5.2, 0), Vector(1.3, 4.9, 0), Vector(1.4, 4.5, 0), Vector(1.5, 4.0, 0), Vector(1.6, 3.4, 0), Vector(1.7, 2.7, 0),
                Vector(1.8, 1.9, 0), Vector(1.9, 1.0, 0), Vector(2.0, 0.0, 0), Vector(2.0, -1, 0)};
    s.Shape = SurfaceShape::Polygonal;
    CheckConvexity(*state, 1, s.Sides);
    EXPECT_TRUE(s.IsConvex);
    s.set_computed_geometry();

    Surface2D const &s2d(s.surface2d);
    EXPECT_EQ(2, s2d.axis); // Projection along z axis
    EXPECT_EQ(Vector2D(0.0, 0.0), s2d.vertices[0]);
    EXPECT_EQ(Vector2D(2.0, -1.0), s2d.vertices[1]);
    EXPECT_EQ(Vector2D(2.0, 0.0), s2d.vertices[2]);
    EXPECT_EQ(Vector2D(1.9, 1.0), s2d.vertices[3]);
    EXPECT_EQ(Vector2D(1.8, 1.9), s2d.vertices[4]);
    EXPECT_EQ(Vector2D(1.7, 2.7), s2d.vertices[5]);
    EXPECT_EQ(Vector2D(1.6, 3.4), s2d.vertices[6]);
    EXPECT_EQ(Vector2D(1.5, 4.0), s2d.vertices[7]);
    EXPECT_EQ(Vector2D(1.4, 4.5), s2d.vertices[8]);
    EXPECT_EQ(Vector2D(1.3, 4.9), s2d.vertices[9]);
    EXPECT_EQ(Vector2D(1.2, 5.2), s2d.vertices[10]);
    EXPECT_EQ(Vector2D(1.1, 5.4), s2d.vertices[11]);
    EXPECT_EQ(Vector2D(1.0, 5.5), s2d.vertices[12]);
    EXPECT_EQ(Vector2D(0.9, 5.4), s2d.vertices[13]);
    EXPECT_EQ(Vector2D(0.8, 5.2), s2d.vertices[14]);
    EXPECT_EQ(Vector2D(0.7, 4.9), s2d.vertices[15]);
    EXPECT_EQ(Vector2D(0.6, 4.5), s2d.vertices[16]);
    EXPECT_EQ(Vector2D(0.5, 4.0), s2d.vertices[17]);
    EXPECT_EQ(Vector2D(0.4, 3.4), s2d.vertices[18]);
    EXPECT_EQ(Vector2D(0.3, 2.7), s2d.vertices[19]);
    EXPECT_EQ(Vector2D(0.2, 1.9), s2d.vertices[20]);
    EXPECT_EQ(Vector2D(0.1, 1.0), s2d.vertices[21]);
    EXPECT_DOUBLE_EQ(0.0, s2d.vl.x);
    EXPECT_DOUBLE_EQ(-1.0, s2d.vl.y);
    EXPECT_DOUBLE_EQ(2.0, s2d.vu.x);
    EXPECT_DOUBLE_EQ(5.5, s2d.vu.y);
    EXPECT_EQ(11u, s2d.slabs.size());
    // Slabs here all have 2 edges
    EXPECT_EQ(2u, s2d.slabs[0].edges.size());
    EXPECT_EQ(2u, s2d.slabs[1].edges.size());
    EXPECT_EQ(2u, s2d.slabs[2].edges.size());
    EXPECT_EQ(2u, s2d.slabs[3].edges.size());
    EXPECT_EQ(2u, s2d.slabs[4].edges.size());
    EXPECT_EQ(2u, s2d.slabs[5].edges.size());
    EXPECT_EQ(2u, s2d.slabs[6].edges.size());
    EXPECT_EQ(2u, s2d.slabs[7].edges.size());
    EXPECT_EQ(2u, s2d.slabs[8].edges.size());
    EXPECT_EQ(2u, s2d.slabs[9].edges.size());
    EXPECT_EQ(2u, s2d.slabs[10].edges.size());
}

TEST_F(EnergyPlusFixture, SurfaceTest_Surface2D_bigVertices2)
{
    using Vector2D = Surface2D::Vector2D;
    state->dataSurface->TotSurfaces = 1;
    constexpr int surfNum = 1;
    int nVertices = 24;
    state->dataSurface->MaxVerticesPerSurface = nVertices;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(state->dataSurface->TotSurfaces);
    auto &s = state->dataSurfaceGeometry->SurfaceTmp(surfNum);
    s.Vertex.dimension(nVertices);
    s.Sides = nVertices;

    // nonconvex, >nVerticesBig, Issue 10490 defect file failed surface
    s.Vertex = {Vector(4.5047023, 14.8653133, 35.35), Vector(6.5689151, 13.4862441, 35.35), Vector(6.1242243, 12.8206238, 35.35),
                Vector(7.8836902, 11.6451513, 35.35), Vector(8.2156112, 12.1419759, 35.35), Vector(8.7993282, 11.7520035, 35.35),
                Vector(8.9659831, 12.0014552, 35.35), Vector(9.2241656, 11.8289674, 35.35), Vector(9.4352618, 12.1449395, 35.35),
                Vector(8.5933623, 12.7073998, 35.35), Vector(9.6032909, 14.219077, 35.35),  Vector(9.5550636, 14.251297, 35.35),
                Vector(8.5268029, 12.71218, 35.35),   Vector(9.1313075, 12.3083197, 35.35), Vector(8.7902205, 11.7977752, 35.35),
                Vector(8.1857159, 12.2016356, 35.35), Vector(7.8676828, 11.7255986, 35.35), Vector(6.2046715, 12.8366312, 35.35),
                Vector(6.6740828, 13.5392534, 35.35), Vector(4.646872, 14.8936022, 35.35),  Vector(5.8684524, 16.7220831, 35.35),
                Vector(7.8474358, 15.3999543, 35.35), Vector(7.8796558, 15.4481816, 35.35), Vector(5.815443, 16.8272508, 35.35)};
    s.Shape = SurfaceShape::Polygonal;
    CheckConvexity(*state, 1, s.Sides);
    EXPECT_FALSE(s.IsConvex);
    s.set_computed_geometry();

    Surface2D const &s2d(s.surface2d);
    EXPECT_EQ(2, s2d.axis); // Projection along z axis
    EXPECT_EQ(Vector2D(4.5047023, 14.8653133), s2d.vertices[0]);
    EXPECT_EQ(Vector2D(6.5689151, 13.4862441), s2d.vertices[1]);
    EXPECT_EQ(Vector2D(6.1242243, 12.8206238), s2d.vertices[2]);
    EXPECT_EQ(Vector2D(7.8836902, 11.6451513), s2d.vertices[3]);
    EXPECT_EQ(Vector2D(8.2156112, 12.1419759), s2d.vertices[4]);
    EXPECT_EQ(Vector2D(8.7993282, 11.7520035), s2d.vertices[5]);
    EXPECT_EQ(Vector2D(8.9659831, 12.0014552), s2d.vertices[6]);
    EXPECT_EQ(Vector2D(9.2241656, 11.8289674), s2d.vertices[7]);
    EXPECT_EQ(Vector2D(9.4352618, 12.1449395), s2d.vertices[8]);
    EXPECT_EQ(Vector2D(8.5933623, 12.7073998), s2d.vertices[9]);
    EXPECT_EQ(Vector2D(9.6032909, 14.219077), s2d.vertices[10]);
    EXPECT_EQ(Vector2D(9.5550636, 14.251297), s2d.vertices[11]);
    EXPECT_EQ(Vector2D(8.5268029, 12.71218), s2d.vertices[12]);
    EXPECT_EQ(Vector2D(9.1313075, 12.3083197), s2d.vertices[13]);
    EXPECT_EQ(Vector2D(8.7902205, 11.7977752), s2d.vertices[14]);
    EXPECT_EQ(Vector2D(8.1857159, 12.2016356), s2d.vertices[15]);
    EXPECT_EQ(Vector2D(7.8676828, 11.7255986), s2d.vertices[16]);
    EXPECT_EQ(Vector2D(6.2046715, 12.8366312), s2d.vertices[17]);
    EXPECT_EQ(Vector2D(6.6740828, 13.5392534), s2d.vertices[18]);
    EXPECT_EQ(Vector2D(4.646872, 14.8936022), s2d.vertices[19]);
    EXPECT_EQ(Vector2D(5.8684524, 16.7220831), s2d.vertices[20]);
    EXPECT_EQ(Vector2D(7.8474358, 15.3999543), s2d.vertices[21]);
    EXPECT_EQ(Vector2D(7.8796558, 15.4481816), s2d.vertices[22]);
    EXPECT_EQ(Vector2D(5.815443, 16.8272508), s2d.vertices[23]);
    EXPECT_DOUBLE_EQ(4.5047023, s2d.vl.x);
    EXPECT_DOUBLE_EQ(11.6451513, s2d.vl.y);
    EXPECT_DOUBLE_EQ(9.6032909, s2d.vu.x);
    EXPECT_DOUBLE_EQ(16.8272508, s2d.vu.y);
    EXPECT_EQ(23u, s2d.slabs.size());
    // Slabs here have anywhere from 2 to 10 edges
    EXPECT_EQ(2u, s2d.slabs[0].edges.size());
    EXPECT_EQ(4u, s2d.slabs[1].edges.size());
    EXPECT_EQ(6u, s2d.slabs[2].edges.size());
    EXPECT_EQ(8u, s2d.slabs[3].edges.size());
    EXPECT_EQ(10u, s2d.slabs[4].edges.size());
    EXPECT_EQ(8u, s2d.slabs[5].edges.size());
    EXPECT_EQ(6u, s2d.slabs[6].edges.size());
    EXPECT_EQ(6u, s2d.slabs[7].edges.size());
    EXPECT_EQ(4u, s2d.slabs[8].edges.size());
    EXPECT_EQ(4u, s2d.slabs[9].edges.size());
    EXPECT_EQ(4u, s2d.slabs[10].edges.size());
    EXPECT_EQ(4u, s2d.slabs[11].edges.size());
    EXPECT_EQ(4u, s2d.slabs[12].edges.size());
    EXPECT_EQ(4u, s2d.slabs[13].edges.size());
    EXPECT_EQ(4u, s2d.slabs[14].edges.size());
    EXPECT_EQ(4u, s2d.slabs[15].edges.size());
    EXPECT_EQ(4u, s2d.slabs[16].edges.size());
    EXPECT_EQ(2u, s2d.slabs[17].edges.size());
    EXPECT_EQ(2u, s2d.slabs[18].edges.size());
    EXPECT_EQ(2u, s2d.slabs[19].edges.size());
    EXPECT_EQ(4u, s2d.slabs[20].edges.size());
    EXPECT_EQ(4u, s2d.slabs[21].edges.size());
    EXPECT_EQ(2u, s2d.slabs[22].edges.size());
}

TEST_F(EnergyPlusFixture, SurfaceTest_AverageHeightRectangle)
{
    {
        SurfaceData s;
        s.Vertex.dimension(4);
        s.Shape = SurfaceShape::Rectangle;

        s.Vertex = {Vector(0, 0, 0), Vector(1, 0, 0), Vector(1, 1, 0), Vector(0, 1, 0)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 0.0);

        s.Vertex = {Vector(0, 0, 0), Vector(1, 1, 0), Vector(1, 1, 1), Vector(0, 0, 1)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 1.0);

        s.Vertex = {Vector(0, 0, 0), Vector(1, 0, 0), Vector(1, 1, 1), Vector(0, 1, 1)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 1.0 / s.SinTilt);

        s.Vertex = {Vector(0, 0, 0), Vector(0, 1, 0), Vector(0, 1, 1), Vector(0, 0, 1)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 1.0);

        s.Vertex = {Vector(1, -1, 0), Vector(1, -1, -1), Vector(0, 0, -1), Vector(0, 0, 0)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 1.0);
    }
}

TEST_F(EnergyPlusFixture, SurfaceTest_AverageHeightTriangle)
{
    {
        SurfaceData s;
        s.Vertex.dimension(3);
        s.Shape = SurfaceShape::Triangle;

        s.Vertex = {Vector(0, 0, 0), Vector(1, 0, 0), Vector(1, 0, 1)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 0.5);

        s.Vertex = {Vector(0, 0, 0), Vector(0, 0, 1), Vector(1, 0, 0)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 0.5);
    }
}

TEST_F(EnergyPlusFixture, SurfaceTest_AverageHeightL)
{
    {
        SurfaceData s;
        s.Vertex.dimension(6);
        s.Shape = SurfaceShape::Polygonal;

        s.Vertex = {Vector(0, 0, 0), Vector(0, 0, 1), Vector(0.5, 0, 1), Vector(0.5, 0, 0.5), Vector(1, 0, 0.5), Vector(1, 0, 0)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 0.75);

        s.Vertex = {Vector(0, 0, 0), Vector(0, 0, 1), Vector(1, 0, 1), Vector(1, 0, 0.5), Vector(0.5, 0, 0.5), Vector(0.5, 0, 0)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);
        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_DOUBLE_EQ(s.get_average_height(*state), 0.75);
    }
}

TEST_F(EnergyPlusFixture, SurfaceTest_HashMap)
{
    int numSurfs = state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(numSurfs);
    state->dataSurface->SurfTAirRef.dimension(numSurfs, 0);
    state->dataSurface->surfIntConv.allocate(numSurfs);
    std::fill(state->dataSurface->surfIntConv.begin(), state->dataSurface->surfIntConv.end(), SurfIntConv());
    state->dataSurface->surfExtConv.allocate(numSurfs);
    std::fill(state->dataSurface->surfExtConv.begin(), state->dataSurface->surfExtConv.end(), SurfExtConv());
    state->dataSurface->SurfWinStormWinConstr.dimension(numSurfs, 0);
    state->dataSurface->intMovInsuls.allocate(numSurfs);
    state->dataSurface->extMovInsuls.allocate(numSurfs);

    for (int SurfNum = 1; SurfNum <= numSurfs; SurfNum++) {
        state->dataSurface->Surface(SurfNum).set_representative_surface(*state, SurfNum);
    }

    EXPECT_EQ(state->dataSurface->RepresentativeSurfaceMap.size(), 1);
    EXPECT_EQ(state->dataSurface->Surface(1).RepresentativeCalcSurfNum, 1);
    EXPECT_EQ(state->dataSurface->Surface(2).RepresentativeCalcSurfNum, 1);
    EXPECT_EQ(state->dataSurface->Surface(3).RepresentativeCalcSurfNum, 1);
    EXPECT_EQ(state->dataSurface->Surface(4).RepresentativeCalcSurfNum, 1);

    state->dataSurface->RepresentativeSurfaceMap.clear();

    state->dataSurface->Surface(1).Area = 20.0;
    state->dataSurface->Surface(2).Azimuth = 180.0;
    state->dataSurface->Surface(3).Azimuth = 180.04;

    for (int SurfNum = 1; SurfNum <= numSurfs; SurfNum++) {
        state->dataSurface->Surface(SurfNum).set_representative_surface(*state, SurfNum);
    }

    EXPECT_EQ(state->dataSurface->RepresentativeSurfaceMap.size(), 2);
    EXPECT_EQ(state->dataSurface->Surface(1).RepresentativeCalcSurfNum, 1);
    EXPECT_EQ(state->dataSurface->Surface(2).RepresentativeCalcSurfNum, 2);
    EXPECT_EQ(state->dataSurface->Surface(3).RepresentativeCalcSurfNum, 2);
    EXPECT_EQ(state->dataSurface->Surface(4).RepresentativeCalcSurfNum, 1);

    state->dataSurface->RepresentativeSurfaceMap.clear();

    state->dataSurface->Surface(3).Azimuth = 180.05;

    for (int SurfNum = 1; SurfNum <= numSurfs; SurfNum++) {
        state->dataSurface->Surface(SurfNum).set_representative_surface(*state, SurfNum);
    }

    EXPECT_EQ(state->dataSurface->RepresentativeSurfaceMap.size(), 3);
    EXPECT_EQ(state->dataSurface->Surface(1).RepresentativeCalcSurfNum, 1);
    EXPECT_EQ(state->dataSurface->Surface(2).RepresentativeCalcSurfNum, 2);
    EXPECT_EQ(state->dataSurface->Surface(3).RepresentativeCalcSurfNum, 3);
    EXPECT_EQ(state->dataSurface->Surface(4).RepresentativeCalcSurfNum, 1);
}

TEST_F(EnergyPlusFixture, SurfaceTest_Azimuth_non_conv)
{
    // Unit test for PR 9907 to fix Issue 9906 incorrect Azimuth angle calculation for some non-convex surfaces
    {
        SurfaceData s;
        s.Vertex.dimension(6);
        s.Shape = SurfaceShape::Polygonal;

        s.Vertex = {Vector(0, 0, 0), Vector(1, 0, 0), Vector(1, 0, -1), Vector(2, 0, -1), Vector(2, 0, 1), Vector(0, 0, 1)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);

        EXPECT_DOUBLE_EQ(s.Azimuth, 180.0); // Original code without PR 9907 fix would fail this one by getting an s.Azimuth of 0.0
        EXPECT_DOUBLE_EQ(s.Tilt, 90.0);

        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_NEAR(s.SinAzim, 0.0, 1e-15);
        EXPECT_DOUBLE_EQ(s.CosAzim, -1.0);
        EXPECT_DOUBLE_EQ(s.SinTilt, 1.0);
    }

    // Test an "ordinary" scenario to make sure the new code would not break anything
    {
        SurfaceData s;
        s.Vertex.dimension(5);
        s.Shape = SurfaceShape::Polygonal;

        s.Vertex = {Vector(0, 0, 0), Vector(1, 0, -1), Vector(2, 0, -1), Vector(2, 0, 1), Vector(0, 0, 1)};
        Vectors::CreateNewellSurfaceNormalVector(s.Vertex, s.Vertex.size(), s.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(s.Vertex, s.Azimuth, s.Tilt, s.lcsx, s.lcsy, s.lcsz, s.NewellSurfaceNormalVector);

        // Original code should get the same result in this case
        EXPECT_DOUBLE_EQ(s.Azimuth, 180.0);
        EXPECT_DOUBLE_EQ(s.Tilt, 90.0);

        s.SinAzim = std::sin(s.Azimuth * Constant::DegToRad);
        s.CosAzim = std::cos(s.Azimuth * Constant::DegToRad);
        s.SinTilt = std::sin(s.Tilt * Constant::DegToRad);

        EXPECT_NEAR(s.SinAzim, 0.0, 1e-15);
        EXPECT_DOUBLE_EQ(s.CosAzim, -1.0);
        EXPECT_DOUBLE_EQ(s.SinTilt, 1.0);
    }
}

TEST_F(EnergyPlusFixture, DataSurfaces_SurfaceClassHelpers)
{
    // SurfaceClassIsWindow: true for the four window types only
    EXPECT_TRUE(SurfaceClassIsWindow(SurfaceClass::Window));
    EXPECT_TRUE(SurfaceClassIsWindow(SurfaceClass::FixedWindow));
    EXPECT_TRUE(SurfaceClassIsWindow(SurfaceClass::OperableWindow));
    EXPECT_TRUE(SurfaceClassIsWindow(SurfaceClass::Skylight));
    EXPECT_FALSE(SurfaceClassIsWindow(SurfaceClass::GlassDoor));
    EXPECT_FALSE(SurfaceClassIsWindow(SurfaceClass::Door));
    EXPECT_FALSE(SurfaceClassIsWindow(SurfaceClass::OverheadDoor));
    EXPECT_FALSE(SurfaceClassIsWindow(SurfaceClass::Wall));

    // SurfaceClassIsGlazed: window types plus GlassDoor
    EXPECT_TRUE(SurfaceClassIsGlazed(SurfaceClass::Window));
    EXPECT_TRUE(SurfaceClassIsGlazed(SurfaceClass::FixedWindow));
    EXPECT_TRUE(SurfaceClassIsGlazed(SurfaceClass::OperableWindow));
    EXPECT_TRUE(SurfaceClassIsGlazed(SurfaceClass::Skylight));
    EXPECT_TRUE(SurfaceClassIsGlazed(SurfaceClass::GlassDoor));
    EXPECT_FALSE(SurfaceClassIsGlazed(SurfaceClass::Door));
    EXPECT_FALSE(SurfaceClassIsGlazed(SurfaceClass::OverheadDoor));
    EXPECT_FALSE(SurfaceClassIsGlazed(SurfaceClass::Wall));

    // SurfaceClassIsDoor: Door, OverheadDoor, GlassDoor
    EXPECT_TRUE(SurfaceClassIsDoor(SurfaceClass::Door));
    EXPECT_TRUE(SurfaceClassIsDoor(SurfaceClass::OverheadDoor));
    EXPECT_TRUE(SurfaceClassIsDoor(SurfaceClass::GlassDoor));
    EXPECT_FALSE(SurfaceClassIsDoor(SurfaceClass::Window));
    EXPECT_FALSE(SurfaceClassIsDoor(SurfaceClass::FixedWindow));
    EXPECT_FALSE(SurfaceClassIsDoor(SurfaceClass::OperableWindow));
    EXPECT_FALSE(SurfaceClassIsDoor(SurfaceClass::Skylight));
    EXPECT_FALSE(SurfaceClassIsDoor(SurfaceClass::Wall));
}
