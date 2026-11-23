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

// EnergyPlus::WindowEquivalentLayer unit tests

// C++ Headers
#include <iostream>

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>
#include <EnergyPlus/WindowManager.hh>

// Fixtures
#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::HeatBalanceManager;

using namespace EnergyPlus::WindowEquivalentLayer;

TEST_F(EnergyPlusFixture, WindowEquivalentLayer_GetInput)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Construction:WindowEquivalentLayer,",
        "  CLR CLR VB,                !- Name",
        "  GLZCLR,                    !- Outside Layer",
        "  Air GAP SealedOut 20mm,    !- Layer 2",
        "  GLZCLR,                    !- Layer 3",
        "  Air GAP SealedIndoor 20mm, !- Layer 4",
        "  VBU8D6+45SW1;              !- Layer 5",

        "WindowMaterial:Glazing:EquivalentLayer,",
        "  GLZCLR,                    !-  Name",
        "  SpectralAverage,           !-  Optical Data Type",
        "  ,                          !-  Window Glass Spectral Data Set Name",
        "  0.83,                      !-  Front Side Beam-Beam Solar Transmittance",
        "  0.83,                      !-  Back Side Beam-Beam Solar Transmittance",
        "  0.08,                      !-  Front Side Beam-Beam Solar Reflectance",
        "  0.08,                      !-  Back Side Beam-Beam Solar Reflectance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Reflectance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Reflectance",
        "  0.0,                       !-  Front Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !-  Back Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !-  Front Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !-  Back Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !-  Front Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !-  Back Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !-  Front Side Beam-Diffuse Visible Reflectance",
        "  0.0,                       !-  Back Side Beam-Diffuse Visible Reflectance",
        "  0.76,                      !-  Diffuse-Diffuse Solar Transmittance",
        "  0.14,                      !-  Front Side Diffuse-Diffuse Solar Reflectance",
        "  0.14,                      !-  Back Side Diffuse-Diffuse Solar Reflectance",
        "  0.0,                       !-  Diffuse-Diffuse Visible Transmittance",
        "  0.0,                       !-  Front Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Back Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Infrared Transmittance (front and back)",
        "  0.84,                      !-  Front Side Infrared Emissivity",
        "  0.84;                      !-  Back Side Infrared Emissivity",

        "WindowMaterial:Blind:EquivalentLayer,",
        "  VBU8D6+45SW1,           ! - Name",
        "  Horizontal,             ! - Slat Orientation",
        "  0.025,                  ! - Slat Width",
        "  0.025,                  ! - Slat Separation",
        "  0.0,                    ! - Slat Crown",
        "  45.0,                   ! - Slat Angle",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Solar Transmittance",
        "  0.80,                   ! - Front Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.60,                   ! - Back Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Visible Transmittance",
        "  0.0,                    ! - Front Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Back Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Slat Infrared Transmittance",
        "  0.90,                   ! - Front Side Slat Infrared Emissivity",
        "  0.90,                   ! - Back Side Slat Infrared Emissivity",
        "  BlockBeamSolar;         ! - Slat Angle Control",

        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedOut 20mm,    !- Name",
        "  Air,                       !- Gas Type",
        "  0.0200,                    !- Thickness",
        "  Sealed;                    !- Gap Vent Type",

        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedIndoor 20mm, !- Name",
        "  Air,                       !- Gas Type",
        "  0.020,                     !- Thickness",
        "  Sealed;                    !- Gap Vent Type ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);

    EXPECT_EQ(1, state->dataMaterial->NumEQLBlinds);
    int VBMatNum = Material::GetMaterialNum(*state, "VBU8D6+45SW1");

    auto const *mat = state->dataMaterial->materials(VBMatNum);
    EXPECT_ENUM_EQ(mat->group, Material::Group::BlindEQL);

    auto const *matVenetBlind = dynamic_cast<Material::MaterialBlindEQL const *>(mat);
    EXPECT_EQ(static_cast<int>(matVenetBlind->slatAngleType), state->dataWindowEquivalentLayer->lscVBNOBM);

    int ConstrNum = 1;
    int EQLNum = 0;
    InitEquivalentLayerWindowCalculations(*state);
    EQLNum = state->dataConstruction->Construct(ConstrNum).EQLConsPtr;
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).L(state->dataWindowEquivLayer->CFS(EQLNum).VBLayerPtr).CNTRL,
              state->dataWindowEquivalentLayer->lscVBNOBM);
}

TEST_F(EnergyPlusFixture, WindowEquivalentLayer_VBMaximizeBeamSolar)
{
    // GitHub issue 5750
    int SurfNum(0);
    Real64 ProfAngVer(0);
    static Array2D<Real64> AbsSolBeam(2, CFSMAXNL + 1);

    std::string const idf_objects = delimited_string({

        "  Timestep,1;",

        "  Building,",
        "    Simple One Zone w Windows,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.004,                   !- Temperature Convergence Tolerance Value {deltaC}",
        "    MinimalShadowing,        !- Solar Distribution",
        "    30,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    Denver Stapleton Intl Arpt Ann Clg 1% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.6,                    !- Maximum Dry-Bulb Temperature {C}",
        "    15.2,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    15.6,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R31LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    5.456,                   !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    C5 - 4 IN HW CONCRETE,   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    FLOOR,                   !- Name",
        "    C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    R31LAYER;                !- Outside Layer",

        "  Site:GroundTemperature:BuildingSurface,18.89,18.92,19.02,19.12,19.21,19.23,19.07,19.32,19.09,19.21,19.13,18.96;",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
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
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    CLR CLR VB,              !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.548000,0,2.5000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.548000,0,0.5000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    5.548000,0,0.5000,       !- X,Y,Z ==> Vertex 3 {m}",
        "    5.548000,0,2.5000;       !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
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
        "    15.24000,0,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
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
        "    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 2 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    0,15.24000,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
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
        "    0,15.24000,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,4.572000;            !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0.000000,0.0,   !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    0.000000,15.24000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,0.0;   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
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
        "    0.000000,15.24000,4.572, !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,4.572, !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0.000000,4.572, !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572; !- X,Y,Z ==> Vertex 4 {m}",

        "  Construction:WindowEquivalentLayer,",
        "  CLR CLR VB,                !- Name",
        "  GLZCLR,                    !- Outside Layer",
        "  Air GAP SealedOut 20mm,    !- Layer 2",
        "  GLZCLR,                    !- Layer 3",
        "  Air GAP SealedIndoor 20mm, !- Layer 4",
        "  VBU8D6+45SW1;              !- Layer 5",

        "WindowMaterial:Glazing:EquivalentLayer,",
        "  GLZCLR,                    !-  Name",
        "  SpectralAverage,           !-  Optical Data Type",
        "  ,                          !-  Window Glass Spectral Data Set Name",
        "  0.83,                      !-  Front Side Beam-Beam Solar Transmittance",
        "  0.83,                      !-  Back Side Beam-Beam Solar Transmittance",
        "  0.08,                      !-  Front Side Beam-Beam Solar Reflectance",
        "  0.08,                      !-  Back Side Beam-Beam Solar Reflectance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Reflectance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Reflectance",
        "  0.0,                       !- Front Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !- Back Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !- Front Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !- Back Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !- Front Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !- Back Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !- Front Side Beam-Diffuse Visible Reflectance",
        "  0.0,                       !- Back Side Beam-Diffuse Visible Reflectance",
        "  0.76,                      !-  Diffuse-Diffuse Solar Transmittance",
        "  0.14,                      !-  Front Side Diffuse-Diffuse Solar Reflectance",
        "  0.14,                      !-  Back Side Diffuse-Diffuse Solar Reflectance",
        "  0.0,                       !-  Diffuse-Diffuse Visible Transmittance",
        "  0.0,                       !-  Front Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Back Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Infrared Transmittance (front and back)",
        "  0.84,                      !-  Front Side Infrared Emissivity",
        "  0.84;                      !-  Back Side Infrared Emissivity",

        "WindowMaterial:Blind:EquivalentLayer,",
        "  VBU8D6+45SW1,           ! - Name",
        "  Horizontal,             ! - Slat Orientation",
        "  0.025,                  ! - Slat Width",
        "  0.025,                  ! - Slat Separation",
        "  0.0,                    ! - Slat Crown",
        "  45.0,                   ! - Slat Angle",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Solar Transmittance",
        "  0.80,                   ! - Front Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.60,                   ! - Back Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Visible Transmittance",
        "  0.0,                    ! - Front Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Back Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Slat Infrared Transmittance",
        "  0.90,                   ! - Front Side Slat Infrared Emissivity",
        "  0.90,                   ! - Back Side Slat Infrared Emissivity",
        "  MaximizeSolar;          ! - Slat Angle Control",

        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedOut 20mm,    !- Name",
        "  Air,                       !- Gas Type",
        "  0.0200,                    !- Thickness",
        "  Sealed;                    !- Gap Vent Type",

        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedIndoor 20mm, !- Name",
        "  Air,                       !- Gas Type",
        "  0.020,                     !- Thickness",
        "  Sealed;                    !- Gap Vent Type ",

    });
    ASSERT_TRUE(process_idf(idf_objects));

    // OutputProcessor::TimeValue.allocate(2); //
    SimulationManager::ManageSimulation(*state);
    // re-set the hour of the day to mid-day
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 12;
    state->dataGlobal->CurrentTime = 12.0;
    Weather::DetermineSunUpDown(*state, state->dataEnvrn->SOLCOS);
    // get window surface index
    for (int iSurf = 1; iSurf <= state->dataSurface->TotSurfaces; iSurf++) {
        if (state->dataSurface->SurfWinWindowModelType(iSurf) == DataSurfaces::WindowModel::EQL) {
            SurfNum = iSurf;
            break;
        }
    }

    // get venetian blind material index
    int VBMatNum = Material::GetMaterialNum(*state, "VBU8D6+45SW1");
    auto const *matVenetBlind = dynamic_cast<Material::MaterialBlindEQL const *>(state->dataMaterial->materials(VBMatNum));
    // get equivalent layer window optical properties
    CalcEQLOpticalProperty(*state, SurfNum, DataWindowEquivalentLayer::SolarArrays::BEAM, AbsSolBeam);
    // check that the slat angle control type is set to MaximizeSolar
    EXPECT_EQ(static_cast<int>(matVenetBlind->slatAngleType), state->dataWindowEquivalentLayer->lscVBPROF);
    // check the slat angle
    EXPECT_NEAR(-71.0772, state->dataSurface->surfShades(SurfNum).blind.slatAngDeg, 0.0001);
    // check that for MaximizeSolar slat angle control, the slat angle = -ve vertical profile angle
    ProfAngVer = Dayltg::ProfileAngle(*state, SurfNum, state->dataEnvrn->SOLCOS, DataWindowEquivalentLayer::Orientation::Horizontal);
    EXPECT_NEAR(-Constant::RadToDeg * ProfAngVer, state->dataSurface->surfShades(SurfNum).blind.slatAngDeg, 0.0001);
}

TEST_F(EnergyPlusFixture, WindowEquivalentLayer_VBBlockBeamSolar)
{
    // GitHub issue 5750
    int SurfNum(0);
    Real64 ProfAngVer(0);
    static Array2D<Real64> AbsSolBeam(2, CFSMAXNL + 1);

    std::string const idf_objects = delimited_string({

        "  Timestep,1;",

        "  Building,",
        "    Simple One Zone w Windows,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.004,                   !- Temperature Convergence Tolerance Value {deltaC}",
        "    MinimalShadowing,        !- Solar Distribution",
        "    30,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    Denver Stapleton Intl Arpt Ann Clg 1% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.6,                    !- Maximum Dry-Bulb Temperature {C}",
        "    15.2,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    15.6,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R31LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    5.456,                   !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    C5 - 4 IN HW CONCRETE,   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    FLOOR,                   !- Name",
        "    C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    R31LAYER;                !- Outside Layer",

        "  Site:GroundTemperature:BuildingSurface,18.89,18.92,19.02,19.12,19.21,19.23,19.07,19.32,19.09,19.21,19.13,18.96;",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
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
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    CLR CLR VB,              !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.548000,0,2.5000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.548000,0,0.5000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    5.548000,0,0.5000,       !- X,Y,Z ==> Vertex 3 {m}",
        "    5.548000,0,2.5000;       !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
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
        "    15.24000,0,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
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
        "    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 2 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    0,15.24000,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
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
        "    0,15.24000,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,4.572000;            !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0.000000,0.0,   !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    0.000000,15.24000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,0.0;   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
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
        "    0.000000,15.24000,4.572, !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,4.572, !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0.000000,4.572, !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572; !- X,Y,Z ==> Vertex 4 {m}",

        "  Construction:WindowEquivalentLayer,",
        "  CLR CLR VB,                !- Name",
        "  GLZCLR,                    !- Outside Layer",
        "  Air GAP SealedOut 20mm,    !- Layer 2",
        "  GLZCLR,                    !- Layer 3",
        "  Air GAP SealedIndoor 20mm, !- Layer 4",
        "  VBU8D6+45SW1;              !- Layer 5",

        "WindowMaterial:Glazing:EquivalentLayer,",
        "  GLZCLR,                    !-  Name",
        "  SpectralAverage,           !-  Optical Data Type",
        "  ,                          !-  Window Glass Spectral Data Set Name",
        "  0.83,                      !-  Front Side Beam-Beam Solar Transmittance",
        "  0.83,                      !-  Back Side Beam-Beam Solar Transmittance",
        "  0.08,                      !-  Front Side Beam-Beam Solar Reflectance",
        "  0.08,                      !-  Back Side Beam-Beam Solar Reflectance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Reflectance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Reflectance",
        "  0.0,                       !- Front Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !- Back Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !- Front Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !- Back Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !- Front Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !- Back Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !- Front Side Beam-Diffuse Visible Reflectance",
        "  0.0,                       !- Back Side Beam-Diffuse Visible Reflectance",
        "  0.76,                      !-  Diffuse-Diffuse Solar Transmittance",
        "  0.14,                      !-  Front Side Diffuse-Diffuse Solar Reflectance",
        "  0.14,                      !-  Back Side Diffuse-Diffuse Solar Reflectance",
        "  0.0,                       !-  Diffuse-Diffuse Visible Transmittance",
        "  0.0,                       !-  Front Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Back Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Infrared Transmittance (front and back)",
        "  0.84,                      !-  Front Side Infrared Emissivity",
        "  0.84;                      !-  Back Side Infrared Emissivity",

        "WindowMaterial:Blind:EquivalentLayer,",
        "  VBU8D6+45SW1,           ! - Name",
        "  Horizontal,             ! - Slat Orientation",
        "  0.025,                  ! - Slat Width",
        "  0.025,                  ! - Slat Separation",
        "  0.0,                    ! - Slat Crown",
        "  45.0,                   ! - Slat Angle",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Solar Transmittance",
        "  0.80,                   ! - Front Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.60,                   ! - Back Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Visible Transmittance",
        "  0.0,                    ! - Front Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Back Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Slat Infrared Transmittance",
        "  0.90,                   ! - Front Side Slat Infrared Emissivity",
        "  0.90,                   ! - Back Side Slat Infrared Emissivity",
        "  BlockBeamSolar;         ! - Slat Angle Control",

        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedOut 20mm,    !- Name",
        "  Air,                       !- Gas Type",
        "  0.0200,                    !- Thickness",
        "  Sealed;                    !- Gap Vent Type",

        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedIndoor 20mm, !- Name",
        "  Air,                       !- Gas Type",
        "  0.020,                     !- Thickness",
        "  Sealed;                    !- Gap Vent Type ",

    });
    ASSERT_TRUE(process_idf(idf_objects));

    // OutputProcessor::TimeValue.allocate(2);
    SimulationManager::ManageSimulation(*state);
    // re-set the hour of the day to noon
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 12;
    state->dataGlobal->CurrentTime = 12.0;
    Weather::DetermineSunUpDown(*state, state->dataEnvrn->SOLCOS);
    // get equivalent layer window surface index
    for (int iSurf = 1; iSurf <= state->dataSurface->TotSurfaces; iSurf++) {
        if (state->dataSurface->SurfWinWindowModelType(iSurf) == DataSurfaces::WindowModel::EQL) {
            SurfNum = iSurf;
            break;
        }
    }
    // get venetian blind material index
    int VBMatNum = Material::GetMaterialNum(*state, "VBU8D6+45SW1");
    auto const *matVenetBlind = dynamic_cast<Material::MaterialBlindEQL const *>(state->dataMaterial->materials(VBMatNum));
    // calc window optical property
    CalcEQLOpticalProperty(*state, SurfNum, DataWindowEquivalentLayer::SolarArrays::BEAM, AbsSolBeam);
    // check VB slat angle for BlockBeamSolar slat angle control
    EXPECT_EQ(static_cast<int>(matVenetBlind->slatAngleType), state->dataWindowEquivalentLayer->lscVBNOBM);
    // check the VB slat angle
    EXPECT_NEAR(18.9228, state->dataSurface->surfShades(SurfNum).blind.slatAngDeg, 0.0001);
    // check that for BlockBeamSolar slat angle control, the slat angle = 90 - ProfAngVer
    ProfAngVer = Dayltg::ProfileAngle(*state, SurfNum, state->dataEnvrn->SOLCOS, DataWindowEquivalentLayer::Orientation::Horizontal);
    EXPECT_NEAR(90.0 - Constant::RadToDeg * ProfAngVer, state->dataSurface->surfShades(SurfNum).blind.slatAngDeg, 0.0001);
    // get the slat angle from profile angle
    Real64 SlateAngleBlockBeamSolar = VB_CriticalSlatAngle(Constant::RadToDeg * ProfAngVer);
    EXPECT_NEAR(SlateAngleBlockBeamSolar, state->dataSurface->surfShades(SurfNum).blind.slatAngDeg, 0.0001);
}

TEST_F(EnergyPlusFixture, WindowEquivalentLayer_InvalidLayerTest)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({

        "   WindowMaterial:SimpleGlazingSystem,",
        "     Simple Glazing System Layer,   !- Name",
        "     2.8,                           !- U-Factor {W/m2-K}",
        "     0.7;                           !- Solar Heat Gain Coefficient",

        "   Construction:WindowEquivalentLayer,",
        "     Simple Equivalent Layer Window,!- Name",
        "     Simple Glazing System Layer;   !- Outside Layer",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_ENUM_EQ(state->dataMaterial->materials(1)->group, Material::Group::GlassSimple);
    // get construction returns error forund true due to invalid layer
    GetConstructData(*state, ErrorsFound);
    EXPECT_EQ(1, state->dataHeatBal->TotConstructs);
    EXPECT_EQ(1, state->dataWindowEquivLayer->TotWinEquivLayerConstructs);
    EXPECT_TRUE(state->dataConstruction->Construct(1).TypeIsWindow);
    EXPECT_TRUE(state->dataConstruction->Construct(1).WindowTypeEQL);
    EXPECT_TRUE(ErrorsFound); // error found due to invalid layer
}

TEST_F(EnergyPlusFixture, WindowEquivalentLayer_AirGapOutdoorVentedTest)
{
    // GitHub issue 7345
    std::string const idf_objects = delimited_string({

        "  Timestep,1;",

        "  Building,",
        "    Simple One Zone w Windows,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.004,                   !- Temperature Convergence Tolerance Value {deltaC}",
        "    MinimalShadowing,        !- Solar Distribution",
        "    30,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    Denver Stapleton Intl Arpt Ann Clg 1% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.6,                    !- Maximum Dry-Bulb Temperature {C}",
        "    15.2,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    15.6,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R31LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    5.456,                   !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    C5 - 4 IN HW CONCRETE,   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    FLOOR,                   !- Name",
        "    C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    R31LAYER;                !- Outside Layer",

        "  Site:GroundTemperature:BuildingSurface,18.89,18.92,19.02,19.12,19.21,19.23,19.07,19.32,19.09,19.21,19.13,18.96;",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
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
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
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
        "    15.24000,0,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
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
        "    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 2 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    0,15.24000,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
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
        "    0,15.24000,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,4.572000;            !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0.000000,0.0,   !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    0.000000,15.24000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,0.0;   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
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
        "    0.000000,15.24000,4.572, !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,4.572, !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0.000000,4.572, !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572; !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    CLR AIRGAP CLR,          !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.548000,0,2.5000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.548000,0,0.5000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    5.548000,0,0.5000,       !- X,Y,Z ==> Vertex 3 {m}",
        "    5.548000,0,2.5000;       !- X,Y,Z ==> Vertex 4 {m}",

        "  Construction:WindowEquivalentLayer,",
        "    CLR AIRGAP CLR,          !- Name",
        "    GLZCLR,                  !- Outside Layer",
        "    Air GAP 12mm,            !- Layer 2",
        "    GLZCLR;                  !- Layer 3",

        "  WindowMaterial:Glazing:EquivalentLayer,",
        "    GLZCLR,                  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.77,                    !- Front Side Beam-Beam Solar Transmittance {dimensionless}",
        "    0.77,                    !- Back Side Beam-Beam Solar Transmittance {dimensionless}",
        "    0.07,                    !- Front Side Beam-Beam Solar Reflectance {dimensionless}",
        "    0.07,                    !- Back Side Beam-Beam Solar Reflectance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.695,                   !- Diffuse-Diffuse Solar Transmittance {dimensionless}",
        "    0.16,                    !- Front Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "    0.16,                    !- Back Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "    0.0,                     !- Diffuse-Diffuse Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Infrared Transmittance (applies to front and back) {dimensionless}",
        "    0.84,                    !- Front Side Infrared Emissivity {dimensionless}",
        "    0.84;                    !- Back Side Infrared Emissivity {dimensionless}",

        "  WindowMaterial:Gap:EquivalentLayer,",
        "   Air GAP 12mm,            !- Name",
        "   Air,                     !- Gas Type",
        "   0.0120,                  !- Thickness {m}",
        "   VentedOutdoor;           !- Gap Vent Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::ManageSimulation(*state);

    int EQLNum(1);
    Array1D<Real64> T({1, CFSMAXNL}, 0.0);
    Array1D<Real64> Q({0, CFSMAXNL}, 0.0);
    Array1D<Real64> JB({0, CFSMAXNL}, 0.0);
    Array1D<Real64> QOCF({1, CFSMAXNL}, 0.0);
    Array1D<Real64> H({0, CFSMAXNL + 1}, 0.0);
    Array1D<Real64> JF({1, CFSMAXNL + 1}, 0.0);
    Array1D<Real64> Source({1, CFSMAXNL + 1}, 0.0);

    Real64 HcIn = 1.5;
    Real64 HcOut = 6.0;
    Real64 TOL = 0.001;
    Real64 TIN = 301.5;
    Real64 Tout = 310.0;
    Real64 TRMIN = 301.5;
    Real64 TRMOUT = 308.0;
    Real64 QOCFRoom = 0.0;
    H(0) = HcOut;
    H(1) = 0.0;
    H(2) = HcIn;

    // check the window air gap vent type: vented to outdoor
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).G(1).GTYPE, state->dataWindowEquivalentLayer->gtyOPENout);
    // zero solar absorbed on glazing layers or no solar input
    Source = 0.0;
    ASHWAT_ThermalCalc(
        *state, state->dataWindowEquivLayer->CFS(EQLNum), TIN, Tout, HcIn, HcOut, TRMOUT, TRMIN, Source, TOL, QOCF, QOCFRoom, T, Q, JF, JB, H);
    EXPECT_NEAR(T(1), 308.610, 0.001);
    EXPECT_NEAR(T(2), 306.231, 0.001);

    // with solar absorbed on glazing layers
    Source(1) = 100.0; // outside glass layer
    Source(2) = 50.0;  // inside glass layer
    ASHWAT_ThermalCalc(
        *state, state->dataWindowEquivLayer->CFS(EQLNum), TIN, Tout, HcIn, HcOut, TRMOUT, TRMIN, Source, TOL, QOCF, QOCFRoom, T, Q, JF, JB, H);
    EXPECT_NEAR(T(1), 313.886, 0.001);
    EXPECT_NEAR(T(2), 310.559, 0.001);
}
TEST_F(EnergyPlusFixture, WindowEquivalentLayer_AirGapIndoorVentedTest)
{
    // GitHub issue 7345
    std::string const idf_objects = delimited_string({

        "  Timestep,1;",

        "  Building,",
        "    Simple One Zone w Windows,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.004,                   !- Temperature Convergence Tolerance Value {deltaC}",
        "    MinimalShadowing,        !- Solar Distribution",
        "    30,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    Denver Stapleton Intl Arpt Ann Clg 1% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.6,                    !- Maximum Dry-Bulb Temperature {C}",
        "    15.2,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    15.6,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R31LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    5.456,                   !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    C5 - 4 IN HW CONCRETE,   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    FLOOR,                   !- Name",
        "    C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    R31LAYER;                !- Outside Layer",

        "  Site:GroundTemperature:BuildingSurface,18.89,18.92,19.02,19.12,19.21,19.23,19.07,19.32,19.09,19.21,19.13,18.96;",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
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
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
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
        "    15.24000,0,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
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
        "    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 2 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    0,15.24000,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
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
        "    0,15.24000,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,4.572000;            !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0.000000,0.0,   !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    0.000000,15.24000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,0.0;   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
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
        "    0.000000,15.24000,4.572, !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,4.572, !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0.000000,4.572, !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572; !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    CLR AIRGAP CLR,          !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.548000,0,2.5000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.548000,0,0.5000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    5.548000,0,0.5000,       !- X,Y,Z ==> Vertex 3 {m}",
        "    5.548000,0,2.5000;       !- X,Y,Z ==> Vertex 4 {m}",

        "  Construction:WindowEquivalentLayer,",
        "    CLR AIRGAP CLR,          !- Name",
        "    GLZCLR,                  !- Outside Layer",
        "    Air GAP 12mm,            !- Layer 2",
        "    GLZCLR;                  !- Layer 3",

        "  WindowMaterial:Glazing:EquivalentLayer,",
        "    GLZCLR,                  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.77,                    !- Front Side Beam-Beam Solar Transmittance {dimensionless}",
        "    0.77,                    !- Back Side Beam-Beam Solar Transmittance {dimensionless}",
        "    0.07,                    !- Front Side Beam-Beam Solar Reflectance {dimensionless}",
        "    0.07,                    !- Back Side Beam-Beam Solar Reflectance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.695,                   !- Diffuse-Diffuse Solar Transmittance {dimensionless}",
        "    0.16,                    !- Front Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "    0.16,                    !- Back Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "    0.0,                     !- Diffuse-Diffuse Visible Solar Transmittance {dimensionless}",
        "    0.0,                     !- Front Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Back Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "    0.0,                     !- Infrared Transmittance (applies to front and back) {dimensionless}",
        "    0.84,                    !- Front Side Infrared Emissivity {dimensionless}",
        "    0.84;                    !- Back Side Infrared Emissivity {dimensionless}",

        "  WindowMaterial:Gap:EquivalentLayer,",
        "   Air GAP 12mm,            !- Name",
        "   Air,                     !- Gas Type",
        "   0.0120,                  !- Thickness {m}",
        "   VentedIndoor;            !- Gap Vent Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::ManageSimulation(*state);

    int EQLNum(1);
    Array1D<Real64> T({1, CFSMAXNL}, 0.0);
    Array1D<Real64> Q({0, CFSMAXNL}, 0.0);
    Array1D<Real64> JB({0, CFSMAXNL}, 0.0);
    Array1D<Real64> QOCF({1, CFSMAXNL}, 0.0);
    Array1D<Real64> H({0, CFSMAXNL + 1}, 0.0);
    Array1D<Real64> JF({1, CFSMAXNL + 1}, 0.0);
    Array1D<Real64> Source({1, CFSMAXNL + 1}, 0.0);

    Real64 HcIn = 1.5;
    Real64 HcOut = 6.0;
    Real64 TOL = 0.001;
    Real64 TIN = 301.5;
    Real64 Tout = 310.0;
    Real64 TRMIN = 301.5;
    Real64 TRMOUT = 308.0;
    Real64 QOCFRoom = 0.0;
    H(0) = HcOut;
    H(1) = 0.0;
    H(2) = HcIn;

    // check the window air gap vent type: vented to outdoor
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).G(1).GTYPE, state->dataWindowEquivalentLayer->gtyOPENin);
    // zero solar absorbed on glazing layers or no solar input
    Source = 0.0;
    ASHWAT_ThermalCalc(
        *state, state->dataWindowEquivLayer->CFS(EQLNum), TIN, Tout, HcIn, HcOut, TRMOUT, TRMIN, Source, TOL, QOCF, QOCFRoom, T, Q, JF, JB, H);
    EXPECT_NEAR(T(1), 307.054, 0.001);
    EXPECT_NEAR(T(2), 304.197, 0.001);

    // with solar absorbed on glazing layers
    Source(1) = 100.0; // outside glass layer
    Source(2) = 50.0;  // inside glass layer
    ASHWAT_ThermalCalc(
        *state, state->dataWindowEquivLayer->CFS(EQLNum), TIN, Tout, HcIn, HcOut, TRMOUT, TRMIN, Source, TOL, QOCF, QOCFRoom, T, Q, JF, JB, H);
    EXPECT_NEAR(T(1), 314.666, 0.001);
    EXPECT_NEAR(T(2), 311.282, 0.001);
}
TEST_F(EnergyPlusFixture, WindowEquivalentLayer_VBEffectiveEmissivityTest)
{
    // GitHub issue 7345
    std::string const idf_objects = delimited_string({

        "  Timestep,1;",

        "  Building,",
        "    Simple One Zone w Windows,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.004,                   !- Temperature Convergence Tolerance Value {deltaC}",
        "    MinimalShadowing,        !- Solar Distribution",
        "    30,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    Denver Stapleton Intl Arpt Ann Clg 1% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.6,                    !- Maximum Dry-Bulb Temperature {C}",
        "    15.2,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    15.6,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R31LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    5.456,                   !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    C5 - 4 IN HW CONCRETE,   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    FLOOR,                   !- Name",
        "    C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    R31LAYER;                !- Outside Layer",

        "  Site:GroundTemperature:BuildingSurface,18.89,18.92,19.02,19.12,19.21,19.23,19.07,19.32,19.09,19.21,19.13,18.96;",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
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
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
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
        "    15.24000,0,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
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
        "    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 2 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    0,15.24000,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
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
        "    0,15.24000,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,4.572000;            !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0.000000,0.0,   !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    0.000000,15.24000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,0.0;   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
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
        "    0.000000,15.24000,4.572, !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,4.572, !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0.000000,4.572, !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572; !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    CON_WIN_EQL,             !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.548000,0,2.5000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.548000,0,0.5000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    5.548000,0,0.5000,       !- X,Y,Z ==> Vertex 3 {m}",
        "    5.548000,0,2.5000;       !- X,Y,Z ==> Vertex 4 {m}",

        "    Construction:WindowEquivalentLayer,",
        "      CON_WIN_EQL,             !- Name",
        "      WMTEUQL_Glss_SC,         !- Outside Layer",
        "      WMTEUQL_Gap_ARGON_16MM,  !- Layer 2",
        "      WMTEUQL_Glss_Clr,        !- Layer 3",
        "      WMTEUQL_Gap_AIR_65MM_VENTINDR,  !- Layer 4",
        "      WMTEUQL_BLND_KINDV_RF80_T02_A18_Rb45;  !- Layer 5",

        "    WindowMaterial:Glazing:EquivalentLayer,",
        "      WMTEUQL_Glss_SC,         !- Name",
        "      SpectralAverage,         !- Optical Data Type",
        "      ,                        !- Window Glass Spectral Data Set Name",
        "      0.300379,                !- Front Side Beam-Beam Solar Transmittance {dimensionless}",
        "      0.300379,                !- Back Side Beam-Beam Solar Transmittance {dimensionless}",
        "      4.448762e-001,           !- Front Side Beam-Beam Solar Reflectance {dimensionless}",
        "      5.449085e-001,           !- Back Side Beam-Beam Solar Reflectance {dimensionless}",
        "      0,                       !- Front Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "      0,                       !- Back Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "      0,                       !- Front Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "      0,                       !- Back Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.263,                   !- Diffuse-Diffuse Solar Transmittance {dimensionless}",
        "      0.470,                   !- Front Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "      0.564,                   !- Back Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "      0.0,                     !- Diffuse-Diffuse Visible Solar Transmittance {dimensionless}",
        "      0.0,                     !- Front Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Back Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Infrared Transmittance (applies to front and back) {dimensionless}",
        "      0.840000,                !- Front Side Infrared Emissivity {dimensionless}",
        "      0.018410;                !- Back Side Infrared Emissivity {dimensionless}",

        "    WindowMaterial:Gap:EquivalentLayer,",
        "      WMTEUQL_Gap_ARGON_16MM,  !- Name",
        "      Argon,                   !- Gas Type",
        "      0.0160,                  !- Thickness {m}",
        "      Sealed;                  !- Gap Vent Type",

        "    WindowMaterial:Glazing:EquivalentLayer,",
        "      WMTEUQL_Glss_Clr,        !- Name",
        "      SpectralAverage,         !- Optical Data Type",
        "      ,                        !- Window Glass Spectral Data Set Name",
        "      0.770675,                !- Front Side Beam-Beam Solar Transmittance {dimensionless}",
        "      0.770675,                !- Back Side Beam-Beam Solar Transmittance {dimensionless}",
        "      6.997562e-002,           !- Front Side Beam-Beam Solar Reflectance {dimensionless}",
        "      7.023712e-002,           !- Back Side Beam-Beam Solar Reflectance {dimensionless}",
        "      0,                       !- Front Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "      0,                       !- Back Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "      0,                       !- Front Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "      0,                       !- Back Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "      0.0,                     !- Front Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Back Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.689,                   !- Diffuse-Diffuse Solar Transmittance {dimensionless}",
        "      0.128,                   !- Front Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "      0.128,                   !- Back Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "      0.0,                     !- Diffuse-Diffuse Visible Solar Transmittance {dimensionless}",
        "      0.0,                     !- Front Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Back Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "      0.0,                     !- Infrared Transmittance (applies to front and back) {dimensionless}",
        "      0.840000,                !- Front Side Infrared Emissivity {dimensionless}",
        "      0.840000;                !- Back Side Infrared Emissivity {dimensionless}",

        "    WindowMaterial:Gap:EquivalentLayer,",
        "      WMTEUQL_Gap_AIR_65MM_VENTINDR,  !- Name",
        "      Air,                     !- Gas Type",
        "      0.065,                   !- Thickness {m}",
        "      VentedIndoor;            !- Gap Vent Type",

        "    WindowMaterial:Blind:EquivalentLayer,",
        "      WMTEUQL_BLND_KINDV_RF80_T02_A18_Rb45,  !- Name",
        "      Vertical,                !- Slat Orientation",
        "      0.02,                    !- Slat Width {m}",
        "      0.02,                    !- Slat Separation {m}",
        "      0.0000,                  !- Slat Crown {m}",
        "      45,                      !- Slat Angle {deg}",
        "      0.02,                    !- Front Side Slat Beam-Diffuse Solar Transmittance",
        "      0.02,                    !- Back Side Slat Beam-Diffuse Solar Transmittance {dimensionless}",
        "      0.80,                    !- Front Side Slat Beam-Diffuse Solar Reflectance {dimensionless}",
        "      0.45,                    !- Back Side Slat Beam-Diffuse Solar Reflectance {dimensionless}",
        "      ,                        !- Front Side Slat Beam-Diffuse Visible Transmittance {dimensionless}",
        "      ,                        !- Back Side Slat Beam-Diffuse Visible Transmittance {dimensionless}",
        "      ,                        !- Front Side Slat Beam-Diffuse Visible Reflectance {dimensionless}",
        "      ,                        !- Back Side Slat Beam-Diffuse Visible Reflectance {dimensionless}",
        "      0.02,                    !- Slat Diffuse-Diffuse Solar Transmittance {dimensionless}",
        "      0.80,                    !- Front Side Slat Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "      0.45,                    !- Back Side Slat Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "      ,                        !- Slat Diffuse-Diffuse Visible Transmittance",
        "      ,                        !- Front Side Slat Diffuse-Diffuse Visible Reflectance {dimensionless}",
        "      ,                        !- Back Side Slat Diffuse-Diffuse Visible Reflectance {dimensionless}",
        "      ,                        !- Slat Infrared Transmittance",
        "      0.9,                     !- Front Side Slat Infrared Emissivity {dimensionless}",
        "      0.9,                     !- Back Side Slat Infrared Emissivity {dimensionless}",
        "      FixedSlatAngle;          !- Slat Angle Control",

    });
    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::ManageSimulation(*state);

    int EQLNum(0);
    int SurfNum(0);
    int ConstrNum(0);

    for (int iSurf = 1; iSurf <= state->dataSurface->TotSurfaces; iSurf++) {
        if (state->dataSurface->SurfWinWindowModelType(iSurf) == DataSurfaces::WindowModel::EQL) {
            SurfNum = iSurf;
            break;
        }
    }
    // get venetian blind material index
    int VBMatNum = Material::GetMaterialNum(*state, "WMTEUQL_BLND_KINDV_RF80_T02_A18_Rb45");
    auto const *matVenetBlind = dynamic_cast<Material::MaterialBlindEQL const *>(state->dataMaterial->materials(VBMatNum));
    // get equivalent layer window construction index
    for (int ConstrPtr = 1; ConstrPtr <= state->dataHeatBal->TotConstructs; ++ConstrPtr) {
        if (state->dataConstruction->Construct(ConstrPtr).WindowTypeEQL) {
            ConstrNum = ConstrPtr;
        }
    }
    // check VB slat angle control for FixedSlatAngle
    EXPECT_EQ(static_cast<int>(matVenetBlind->slatAngleType), state->dataWindowEquivalentLayer->lscNONE);

    EQLNum = state->dataConstruction->Construct(ConstrNum).EQLConsPtr;
    // check number of solid layers
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).NL, 3);
    // check optical and thermal property of the VB layer (Inside Layer)
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).L(3).Name, "WMTEUQL_BLND_KINDV_RF80_T02_A18_RB45");
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).L(3).LWP_MAT.TAUL, 0.0);
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).L(3).LWP_MAT.EPSLF, 0.90);
    EXPECT_EQ(state->dataWindowEquivLayer->CFS(EQLNum).L(3).LWP_MAT.EPSLB, 0.90);
    // check inside face effective emissivity
    EXPECT_NEAR(state->dataConstruction->Construct(ConstrNum).InsideAbsorpThermal, 0.91024, 0.00001);
    // for fixed slate angle the emissivity remains the same
    EXPECT_NEAR(EQLWindowInsideEffectiveEmiss(*state, ConstrNum), 0.91024, 0.00001);
}
