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

// EnergyPlus::HeatBalanceMovableInsulation Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

#include "Fixtures/EnergyPlusFixture.hh"

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalanceMovableInsulation_EvalOutsideMovableInsulation)
{
    state->init_state(*state);
    auto &s_mat = state->dataMaterial;

    int SurfNum = 1;
    state->dataSurface->Surface.allocate(SurfNum);
    state->dataSurface->extMovInsuls.allocate(SurfNum);
    state->dataSurface->extMovInsuls(SurfNum).sched = Sched::GetScheduleAlwaysOn(*state);
    state->dataSurface->extMovInsuls(SurfNum).matNum = 1;
    state->dataHeatBalSurf->SurfAbsSolarExt.allocate(SurfNum);
    state->dataHeatBalSurf->SurfAbsThermalExt.allocate(SurfNum);
    state->dataHeatBalSurf->SurfRoughnessExt.allocate(SurfNum);
    state->dataSurface->extMovInsuls(SurfNum).present = true;
    state->dataSurface->extMovInsulSurfNums.push_back(SurfNum);

    auto *mat1 = new Material::MaterialShade;
    s_mat->materials.push_back(mat1);
    mat1->Resistance = 1.25;
    mat1->Roughness = Material::SurfaceRoughness::VeryRough;
    mat1->group = Material::Group::Regular;
    mat1->AbsorpSolar = 0.75;
    mat1->AbsorpThermal = 0.75;
    mat1->Trans = 0.25;
    mat1->ReflectSolBeamFront = 0.20;
    state->dataHeatBal->Zone.allocate(1);
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst = 1;
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast = 1;

    state->dataHeatBalSurf->SurfAbsSolarExt(1) = 0.0;
    HeatBalanceSurfaceManager::EvalOutsideMovableInsulation(*state);
    EXPECT_EQ(0.75, state->dataHeatBalSurf->SurfAbsSolarExt(1));
    EXPECT_EQ(0.8, state->dataSurface->extMovInsuls(1).H);
    EXPECT_ENUM_EQ(Material::SurfaceRoughness::VeryRough, state->dataHeatBalSurf->SurfRoughnessExt(1));
    EXPECT_EQ(0.75, state->dataHeatBalSurf->SurfAbsThermalExt(1));

    delete mat1;
    s_mat->materials.clear();

    state->dataHeatBalSurf->SurfAbsSolarExt(1) = 0.0;
    auto *mat2 = new Material::MaterialGlass;
    s_mat->materials.push_back(mat2);
    mat2->Resistance = 1.25;
    mat2->Roughness = Material::SurfaceRoughness::VeryRough;
    mat2->group = Material::Group::Glass;
    mat2->AbsorpSolar = 0.75;
    mat2->AbsorpThermal = 0.75;
    mat2->Trans = 0.25;
    mat2->ReflectSolBeamFront = 0.20;

    HeatBalanceSurfaceManager::EvalOutsideMovableInsulation(*state);
    EXPECT_EQ(0.55, state->dataHeatBalSurf->SurfAbsSolarExt(1));

    delete mat2;
    s_mat->materials.clear();

    state->dataHeatBalSurf->SurfAbsSolarExt(1) = 0.0;

    auto *mat3 = new Material::MaterialGlassEQL;
    s_mat->materials.push_back(mat3);
    mat3->Resistance = 1.25;
    mat3->Roughness = Material::SurfaceRoughness::VeryRough;
    mat3->group = Material::Group::GlassEQL;
    mat3->AbsorpSolar = 0.75;
    mat3->AbsorpThermal = 0.75;
    mat3->Trans = 0.25;
    mat3->ReflectSolBeamFront = 0.20;
    HeatBalanceSurfaceManager::EvalOutsideMovableInsulation(*state);
    EXPECT_EQ(0.55, state->dataHeatBalSurf->SurfAbsSolarExt(1));

    delete mat3;
    s_mat->materials.clear();
}

TEST_F(EnergyPlusFixture, HeatBalanceMovableInsulation_EvalInsideMovableInsulation)
{
    state->init_state(*state);

    int SurfNum = 1;
    state->dataSurface->Surface.allocate(SurfNum);

    state->dataSurface->intMovInsuls.allocate(SurfNum);
    state->dataSurface->intMovInsuls(SurfNum).sched = Sched::GetScheduleAlwaysOn(*state);
    state->dataSurface->intMovInsuls(SurfNum).matNum = 1;
    state->dataHeatBalSurf->SurfAbsSolarInt.allocate(SurfNum);
    state->dataHeatBalSurf->SurfAbsThermalInt.allocate(SurfNum);
    state->dataSurface->intMovInsulSurfNums.push_back(SurfNum);

    auto *mat = new Material::MaterialShade;
    state->dataMaterial->materials.push_back(mat);
    mat->Resistance = 1.25;
    mat->Roughness = Material::SurfaceRoughness::VeryRough;
    mat->group = Material::Group::Regular;
    mat->AbsorpSolar = 0.75;
    mat->AbsorpThermal = 0.75;
    mat->Trans = 0.25;
    mat->ReflectSolBeamFront = 0.20;
    state->dataHeatBal->Zone.allocate(1);
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst = 1;
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast = 1;

    state->dataHeatBalSurf->SurfAbsSolarInt(1) = 0.0;
    HeatBalanceSurfaceManager::EvalInsideMovableInsulation(*state);
    EXPECT_EQ(0.75, state->dataHeatBalSurf->SurfAbsSolarInt(1));
    EXPECT_EQ(0.8, state->dataSurface->intMovInsuls(1).H);
    EXPECT_EQ(true, state->dataSurface->intMovInsuls(1).present);
    EXPECT_EQ(0.75, state->dataHeatBalSurf->SurfAbsThermalInt(1));

    state->dataHeatBalSurf->SurfAbsSolarInt(1) = 0.0;
    mat->group = Material::Group::Glass;
    HeatBalanceSurfaceManager::EvalInsideMovableInsulation(*state);
    EXPECT_EQ(0.55, state->dataHeatBalSurf->SurfAbsSolarInt(1));

    state->dataHeatBalSurf->SurfAbsSolarInt(1) = 0.0;
    mat->group = Material::Group::GlassEQL;
    HeatBalanceSurfaceManager::EvalInsideMovableInsulation(*state);
    EXPECT_EQ(0.55, state->dataHeatBalSurf->SurfAbsSolarInt(1));
}
TEST_F(EnergyPlusFixture, SurfaceControlMovableInsulation_InvalidWindowSimpleGlazingTest)
{

    std::string const idf_objects = delimited_string({

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

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
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  WindowMaterial:SimpleGlazingSystem,",
        "    SimpleGlazingSystem,     !- Name",
        "    2.8,                     !- U-Factor {W/m2-K}",
        "    0.7;                     !- Solar Heat Gain Coefficient",

        "  SurfaceControl:MovableInsulation,",
        "    Outside,                 !- Insulation Type",
        "    Zn001:Wall001,           !- Surface Name",
        "    SimpleGlazingSystem,     !- Material Name",
        "    ON;                      !- Schedule Name",

        "  Schedule:Compact,",
        "    ON,                      !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    // set error to false
    bool ErrorsFound(false);
    // set zone data
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE ONE";

    // get materials data
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ(4, state->dataMaterial->materials.size());
    EXPECT_ENUM_EQ(state->dataMaterial->materials(4)->group, Material::Group::GlassSimple);
    // get construction data
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_EQ(1, state->dataHeatBal->TotConstructs);
    EXPECT_FALSE(ErrorsFound);
    // set relative coordinate
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    // set surface data
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->extMovInsuls.allocate(1);
    state->dataSurface->intMovInsuls.allocate(1);
    state->dataSurface->extMovInsuls(1).matNum = 0;
    state->dataSurface->extMovInsuls(1).sched = nullptr;
    state->dataSurface->intMovInsuls(1).matNum = 0;
    state->dataSurface->intMovInsuls(1).sched = nullptr;
    state->dataSurfaceGeometry->SurfaceTmp.allocate(1);
    int SurfNum = 0;
    int TotHTSurfs = state->dataSurface->TotSurfaces = 1;
    Array1D_string const BaseSurfCls(1, {"WALL"});
    Array1D<DataSurfaces::SurfaceClass> const BaseSurfIDs(1, {DataSurfaces::SurfaceClass::Wall});
    int NeedToAddSurfaces;
    // get heat transfer surface data
    SurfaceGeometry::GetHTSurfaceData(*state, ErrorsFound, SurfNum, TotHTSurfs, 0, 0, 0, BaseSurfCls, BaseSurfIDs, NeedToAddSurfaces);
    // get movable insulation object data
    state->dataSurface->Surface(1) = state->dataSurfaceGeometry->SurfaceTmp(1);
    SurfaceGeometry::GetMovableInsulationData(*state, ErrorsFound);
    // check movable insulation material
    EXPECT_EQ(state->dataSurfaceGeometry->SurfaceTmp(1).BaseSurfName, "ZN001:WALL001");     // base surface name
    EXPECT_EQ(state->dataSurface->extMovInsuls(1).matNum, 4);                               // index to movable insulation material
    EXPECT_EQ(state->dataMaterial->materials(4)->Name, "SIMPLEGLAZINGSYSTEM");              // name of movable insulation material
    EXPECT_ENUM_EQ(state->dataMaterial->materials(4)->group, Material::Group::GlassSimple); // invalid material group type
    EXPECT_TRUE(ErrorsFound);                                                               // error found due to invalid material
}
} // namespace EnergyPlus
