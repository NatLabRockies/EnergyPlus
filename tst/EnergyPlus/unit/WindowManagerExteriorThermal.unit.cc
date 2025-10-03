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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WindowManager.hh>

// Windows library headers
#include <WCEMultiLayerOptics.hpp>
#include <WCETarcog.hpp>

// EnergyPlus headers
#include <EnergyPlus/WindowManagerExteriorThermal.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::Window;

TEST_F(EnergyPlusFixture, test_overallUfactorFromFilmsAndCond)
{
    auto &s_mat = state->dataMaterial;
    // set up for using CWCEHeatTransferFactory
    int numSurf = 1;
    state->dataSurface->Surface.allocate(numSurf);
    state->dataSurface->SurfaceWindow.allocate(numSurf);
    int numCons = 1;
    state->dataConstruction->Construct.allocate(numCons);
    state->dataSurface->Surface(numSurf).Construction = numCons;
    int numLayers = 2;
    state->dataConstruction->Construct(numCons).LayerPoint.allocate(numLayers);
    int materialOutside = 1;
    int materialInside = 2;
    state->dataConstruction->Construct(numCons).TotLayers = numLayers;
    state->dataConstruction->Construct(numCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(numCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(numCons).AbsDiff.allocate(2);

    s_mat->materials.push_back(new Material::MaterialGlass);
    s_mat->materials.push_back(new Material::MaterialGlass);

    auto aFactory = CWCEHeatTransferFactory(*state, state->dataSurface->Surface(numSurf), numSurf, numCons);

    double hIntConvCoeff = 0.;
    double hExtConvCoeff = 0.;
    double conductance = 0.;

    double uvalue = aFactory.overallUfactorFromFilmsAndCond(conductance, hIntConvCoeff, hExtConvCoeff);
    EXPECT_NEAR(uvalue, 0., 0.0001);

    conductance = 0.5;
    uvalue = aFactory.overallUfactorFromFilmsAndCond(conductance, hIntConvCoeff, hExtConvCoeff);
    EXPECT_NEAR(uvalue, 0., 0.0001);

    hIntConvCoeff = 1.;
    hExtConvCoeff = 1.;
    conductance = 1.;
    uvalue = aFactory.overallUfactorFromFilmsAndCond(conductance, hIntConvCoeff, hExtConvCoeff);
    EXPECT_NEAR(uvalue, 0.33333, 0.0001);

    hIntConvCoeff = 8.;
    hExtConvCoeff = 30.;
    conductance = 2.326112;
    uvalue = aFactory.overallUfactorFromFilmsAndCond(conductance, hIntConvCoeff, hExtConvCoeff);
    EXPECT_NEAR(uvalue, 1.700, 0.001);

    hIntConvCoeff = 30.;
    hExtConvCoeff = 8.;
    conductance = 3.543645;
    uvalue = aFactory.overallUfactorFromFilmsAndCond(conductance, hIntConvCoeff, hExtConvCoeff);
    EXPECT_NEAR(uvalue, 2.270, 0.001);
}

TEST_F(EnergyPlusFixture, test_getOutdoorNfrc)
{
    auto &s_mat = state->dataMaterial;
    // set up for using CWCEHeatTransferFactory
    int numSurf = 1;
    state->dataSurface->Surface.allocate(numSurf);
    state->dataSurface->SurfaceWindow.allocate(numSurf);
    int numCons = 1;
    state->dataConstruction->Construct.allocate(numCons);
    state->dataSurface->Surface(numSurf).Construction = numCons;
    int numLayers = 2;
    state->dataConstruction->Construct(numCons).LayerPoint.allocate(numLayers);
    int materialOutside = 1;
    int materialInside = 2;
    state->dataConstruction->Construct(numCons).TotLayers = numLayers;
    state->dataConstruction->Construct(numCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(numCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(numCons).AbsDiff.allocate(2);

    auto *matOutside = new Material::MaterialGlass;
    s_mat->materials.push_back(matOutside);
    auto *matInside = new Material::MaterialGlass;
    s_mat->materials.push_back(matInside);

    auto aFactory = CWCEHeatTransferFactory(*state, state->dataSurface->Surface(numSurf), numSurf, numCons);

    auto indoor = aFactory.getOutdoorNfrc(true);
    EXPECT_NEAR(indoor->getAirTemperature(), 305.15, 0.01);
    EXPECT_NEAR(indoor->getDirectSolarRadiation(), 783.0, 0.01);

    indoor = aFactory.getOutdoorNfrc(false);
    EXPECT_NEAR(indoor->getAirTemperature(), 255.15, 0.01);
    EXPECT_NEAR(indoor->getDirectSolarRadiation(), 0.0, 0.01);
}

TEST_F(EnergyPlusFixture, test_getIndoorNfrc)
{
    auto &s_mat = state->dataMaterial;
    // set up for using CWCEHeatTransferFactory
    int numSurf = 1;
    state->dataSurface->Surface.allocate(numSurf);
    state->dataSurface->SurfaceWindow.allocate(numSurf);
    int numCons = 1;
    state->dataConstruction->Construct.allocate(numCons);
    state->dataSurface->Surface(numSurf).Construction = numCons;
    int numLayers = 2;
    state->dataConstruction->Construct(numCons).LayerPoint.allocate(numLayers);
    int materialOutside = 1;
    int materialInside = 2;
    state->dataConstruction->Construct(numCons).TotLayers = numLayers;
    state->dataConstruction->Construct(numCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(numCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(numCons).AbsDiff.allocate(2);
    s_mat->materials.push_back(new Material::MaterialGlass);
    s_mat->materials.push_back(new Material::MaterialGlass);

    auto aFactory = CWCEHeatTransferFactory(*state, state->dataSurface->Surface(numSurf), numSurf, numCons);

    auto indoor = aFactory.getIndoorNfrc(true);
    EXPECT_NEAR(indoor->getAirTemperature(), 297.15, 0.01);

    indoor = aFactory.getIndoorNfrc(false);
    EXPECT_NEAR(indoor->getAirTemperature(), 294.15, 0.01);
}

TEST_F(EnergyPlusFixture, test_getShadeType)
{
    auto &s_mat = state->dataMaterial;
    // set up for using CWCEHeatTransferFactory
    int numSurf = 1;
    state->dataSurface->Surface.allocate(numSurf);
    state->dataSurface->SurfaceWindow.allocate(numSurf);
    int numCons = 2;
    int simpleCons = 1;
    state->dataConstruction->Construct.allocate(numCons);
    state->dataSurface->Surface(numSurf).Construction = simpleCons;
    int numLayers = 3;
    state->dataConstruction->Construct(simpleCons).LayerPoint.allocate(numLayers);
    int materialOutside = 1;
    int materialInside = 2;
    state->dataConstruction->Construct(simpleCons).TotLayers = numLayers;
    state->dataConstruction->Construct(simpleCons).TotGlassLayers = numLayers - 1;
    state->dataConstruction->Construct(simpleCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(simpleCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(simpleCons).AbsDiff.allocate(2);
    s_mat->materials.push_back(new Material::MaterialGlass);
    s_mat->materials.push_back(new Material::MaterialGlass);
    auto aFactory = CWCEHeatTransferFactory(*state, state->dataSurface->Surface(numSurf), numSurf, simpleCons);

    // outside
    auto typeOfShade = aFactory.getShadeType(*state, simpleCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::NoShade);

    delete s_mat->materials(materialOutside);
    s_mat->materials(materialOutside) = new Material::MaterialShade;
    typeOfShade = aFactory.getShadeType(*state, simpleCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::ExtShade);

    delete s_mat->materials(materialOutside);
    s_mat->materials(materialOutside) = new Material::MaterialBlind;
    typeOfShade = aFactory.getShadeType(*state, simpleCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::ExtBlind);

    // reset the outside to glass
    delete s_mat->materials(materialOutside);
    s_mat->materials(materialOutside) = new Material::MaterialGlass;

    // inside
    delete s_mat->materials(materialInside);
    s_mat->materials(materialInside) = new Material::MaterialShade;
    typeOfShade = aFactory.getShadeType(*state, simpleCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::IntShade);

    delete s_mat->materials(materialInside);
    s_mat->materials(materialInside) = new Material::MaterialBlind;
    typeOfShade = aFactory.getShadeType(*state, simpleCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::IntBlind);

    // reset the outside to glass
    delete s_mat->materials(materialInside);
    s_mat->materials(materialInside) = new Material::MaterialGlass;

    // between glass - double pane
    int betweenCons = 2;
    state->dataSurface->Surface(numSurf).Construction = betweenCons;
    numLayers = 4;
    int shadeLayer = 3;
    state->dataConstruction->Construct(betweenCons).LayerPoint.allocate(numLayers);
    int materialShade = 3;
    state->dataConstruction->Construct(betweenCons).TotLayers = numLayers;
    state->dataConstruction->Construct(betweenCons).TotGlassLayers = 2;
    state->dataConstruction->Construct(betweenCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(betweenCons).LayerPoint(shadeLayer) = materialShade;
    state->dataConstruction->Construct(betweenCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(betweenCons).AbsDiff.allocate(2);

    s_mat->materials.push_back(new Material::MaterialShade);
    typeOfShade = aFactory.getShadeType(*state, betweenCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::BGShade);

    delete s_mat->materials(materialShade);
    s_mat->materials(materialShade) = new Material::MaterialBlind;
    typeOfShade = aFactory.getShadeType(*state, betweenCons);
    EXPECT_ENUM_EQ(typeOfShade, DataSurfaces::WinShadingType::BGBlind);
}

TEST_F(EnergyPlusFixture, test_getActiveConstructionNumber)
{
    auto &s_mat = state->dataMaterial;
    // set up for using CWCEHeatTransferFactory
    int numSurf = 1;
    state->dataSurface->Surface.allocate(numSurf);
    state->dataSurface->SurfaceWindow.allocate(numSurf);
    state->dataSurface->SurfWinShadingFlag.allocate(numSurf);
    state->dataSurface->SurfWinActiveShadedConstruction.allocate(numSurf);

    int numCons = 2;
    state->dataConstruction->Construct.allocate(numCons);
    state->dataSurface->Surface(numSurf).Construction = numCons;
    int numLayers = 2;
    state->dataConstruction->Construct(numCons).LayerPoint.allocate(numLayers);
    int materialOutside = 1;
    int materialInside = 2;
    state->dataConstruction->Construct(numCons).TotLayers = numLayers;
    state->dataConstruction->Construct(numCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(numCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(numCons).AbsDiff.allocate(2);

    s_mat->materials.push_back(new Material::MaterialGlass);
    s_mat->materials.push_back(new Material::MaterialGlass);

    auto aFactory = CWCEHeatTransferFactory(*state, state->dataSurface->Surface(numSurf), numSurf, numCons);

    auto &surface(state->dataSurface->Surface(numSurf));
    state->dataSurface->SurfWinShadingFlag(numSurf) = DataSurfaces::WinShadingType::ExtBlind;
    state->dataSurface->SurfWinActiveShadedConstruction(numSurf) = 7;

    int consSelected = aFactory.getActiveConstructionNumber(*state, surface, numSurf);
    EXPECT_EQ(consSelected, 7);

    state->dataSurface->SurfWinShadingFlag(numSurf) = DataSurfaces::WinShadingType::NoShade;

    consSelected = aFactory.getActiveConstructionNumber(*state, surface, numSurf);
    EXPECT_EQ(consSelected, numCons);
}

TEST_F(EnergyPlusFixture, test_getIGU)
{
    auto &s_mat = state->dataMaterial;
    // set up for using CWCEHeatTransferFactory
    int numSurf = 1;
    state->dataSurface->Surface.allocate(numSurf);
    state->dataSurface->SurfaceWindow.allocate(numSurf);
    int numCons = 1;
    state->dataConstruction->Construct.allocate(numCons);
    state->dataSurface->Surface(numSurf).Construction = numCons;
    int numLayers = 2;
    state->dataConstruction->Construct(numCons).LayerPoint.allocate(numLayers);
    int materialOutside = 1;
    int materialInside = 2;
    state->dataConstruction->Construct(numCons).TotLayers = numLayers;
    state->dataConstruction->Construct(numCons).LayerPoint(1) = materialOutside;
    state->dataConstruction->Construct(numCons).LayerPoint(numLayers) = materialInside;
    state->dataConstruction->Construct(numCons).AbsDiff.allocate(2);

    s_mat->materials.push_back(new Material::MaterialGlass);
    s_mat->materials.push_back(new Material::MaterialGlass);

    auto aFactory = CWCEHeatTransferFactory(*state, state->dataSurface->Surface(numSurf), numSurf, numCons);

    double width = 10.;
    double height = 15.;
    double tilt = 90.;

    auto igu = aFactory.getIGU(width, height, tilt);
    EXPECT_NEAR(igu.getTilt(), 90., 0.01);
    EXPECT_NEAR(igu.getHeight(), 15., 0.01);
    EXPECT_NEAR(igu.getWidth(), 10., 0.01);
}

TEST_F(EnergyPlusFixture, test_GetWindowAssemblyNfrcForReport_withIDF)
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",
        "  Timestep,6;",
        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",
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
        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  Construction,",
        "    INTERIOR,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",
        "  Construction,",
        "    PARTITION02,             !- Name",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE,  !- Layer 4",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",
        "  Construction,",
        "    single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
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
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 9;

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::InitSolarCalculations(*state);
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;

    double uValueRep{0.};
    double shgcRep{0.};
    double vtRep{0.};

    int windowSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    EXPECT_TRUE(windowSurfNum > 0);
    int constructNum = Util::FindItemInList("DOUBLE PANE HW WINDOW", state->dataConstruction->Construct);
    EXPECT_TRUE(constructNum > 0);

    GetWindowAssemblyNfrcForReport(
        *state, windowSurfNum, constructNum, 1.0, 0.5, DataSurfaces::NfrcVisionType::DualVertical, uValueRep, shgcRep, vtRep);

    EXPECT_NEAR(uValueRep, 3.24, 0.01);
    EXPECT_NEAR(shgcRep, 0.029, 0.001);
    EXPECT_NEAR(vtRep, 0.0, 0.1);

    GetWindowAssemblyNfrcForReport(
        *state, windowSurfNum, constructNum, 1.0, 0.5, DataSurfaces::NfrcVisionType::DualHorizontal, uValueRep, shgcRep, vtRep);

    EXPECT_NEAR(uValueRep, 3.07, 0.01);
    EXPECT_NEAR(shgcRep, 0.024, 0.001);
    EXPECT_NEAR(vtRep, 0.0, 0.1);

    GetWindowAssemblyNfrcForReport(*state, windowSurfNum, constructNum, 1.0, 0.5, DataSurfaces::NfrcVisionType::Single, uValueRep, shgcRep, vtRep);

    EXPECT_NEAR(uValueRep, 3.11, 0.01);
    EXPECT_NEAR(shgcRep, 0.021, 0.001);
    EXPECT_NEAR(vtRep, 0.0, 0.1);
}
