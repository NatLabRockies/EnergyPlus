// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::ThermalChimney Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/ThermalChimney.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::InternalHeatGains;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataHeatBalSurface;
using namespace SimulationManager;

TEST_F(EnergyPlusFixture, ThermalChimney_EMSAirflow_Test)
{

    std::string const idf_objects = delimited_string({

        "  Version," + DataStringGlobals::MatchVersion + ";",

        "  SimulationControl,",
        "    NO,                      !- Do Zone Sizing Calculation",
        "    NO,                      !- Do System Sizing Calculation",
        "    NO,                      !- Do Plant Sizing Calculation",
        "    YES,                     !- Run Simulation for Sizing Periods",
        "    NO;                      !- Run Simulation for Weather File Run Periods",

        "  Building,",
        "    BUILDING #1,             !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value {W}",
        "    0.2500000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  Timestep,4;",

        "  Site:Location,",
        "    CEDAR_CITY_UT_USA_WMO_724755,  !- Name",
        "    37.70,                   !- Latitude {deg}",
        "    -113.08,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1712.00;                 !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CEDAR_CITY Ann Clg 1% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.9,                    !- Maximum Dry-Bulb Temperature {C}",
        "    16.3,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    14.8,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    82379.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    200,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CEDAR_CITY Ann Htg 99% Condns DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -12.8,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -12.8,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    82379.,                  !- Barometric Pressure {Pa}",
        "    1.2,                     !- Wind Speed {m/s}",
        "    130,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.00;                    !- Sky Clearness",

        "  RunPeriod,",
        "    Run Period 1,            !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  Site:GroundTemperature:BuildingSurface,19.77,19.81,19.91,19.89,19.85,19.60,19.38,19.56,19.66,19.74,19.82,19.76;",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  Schedule:Compact,",
        "    ThermalChimneyAvail,     !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Always 1,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    OFF,                     !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0;        !- Field 3",

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
        "    B12 - 3 IN DENSE INSULATION,  !- Name",
        "    VeryRough,               !- Roughness",
        "    7.6200001E-02,           !- Thickness {m}",
        "    4.3239430E-02,           !- Conductivity {W/m-K}",
        "    91.30524,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    C7 - 8 IN LW CONCRETE BLOCK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.2033016,               !- Thickness {m}",
        "    0.5707605,               !- Conductivity {W/m-K}",
        "    608.7016,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    E8 - 5 / 8 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5880080E-02,           !- Thickness {m}",
        "    0.1601589,               !- Conductivity {W/m-K}",
        "    784.9047,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    B3 - 2 IN INSULATION,    !- Name",
        "    VeryRough,               !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    4.3239430E-02,           !- Conductivity {W/m-K}",
        "    32.03693,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    C2 - 4 IN LW CONCRETE BLOCK,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.3805070,               !- Conductivity {W/m-K}",
        "    608.7016,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

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
        "    TABOR SOLAR ABSORBER,    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.6000000E-03,           !- Thickness {m}",
        "    392.6100,                !- Conductivity {W/m-K}",
        "    8906.260,                !- Density {kg/m3}",
        "    370.0000,                !- Specific Heat {J/kg-K}",
        "    0.0500000,               !- Thermal Absorptance",
        "    0.8500000,               !- Solar Absorptance",
        "    0.8500000;               !- Visible Absorptance",

        "  Material,",
        "    CONCRETE - SAND AND GRAVEL 4 IN,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6000000,               !- Solar Absorptance",
        "    0.6000000;               !- Visible Absorptance",

        "  Material,",
        "    C10 - 8 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2033016,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    ROOFING - WOOD SHINGLES, !- Name",
        "    VeryRough,               !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.1141521,               !- Conductivity {W/m-K}",
        "    720.8308,                !- Density {kg/m3}",
        "    1255.200,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    BLBD - SHEATHING NAIL BASE 1 / 2 IN,  !- Name",
        "    MediumRough,             !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    6.3475482E-02,           !- Conductivity {W/m-K}",
        "    400.4616,                !- Density {kg/m3}",
        "    1297.040,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material,",
        "    E6 - 1 / 2 IN GYP SHEATHING BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    0.1601589,               !- Conductivity {W/m-K}",
        "    784.9047,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR SHEET 1 / 8 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    3.0000000E-03,           !- Thickness {m}",
        "    0.8370000,               !- Solar Transmittance at Normal Incidence",
        "    7.5000003E-02,           !- Front Side Solar Reflectance at Normal Incidence",
        "    7.5000003E-02,           !- Back Side Solar Reflectance at Normal Incidence",
        "    0.8980000,               !- Visible Transmittance at Normal Incidence",
        "    8.1000000E-02,           !- Front Side Visible Reflectance at Normal Incidence",
        "    8.1000000E-02,           !- Back Side Visible Reflectance at Normal Incidence",
        "    0,                       !- Infrared Transmittance at Normal Incidence",
        "    0.8400000,               !- Front Side Infrared Hemispherical Emissivity",
        "    0.8400000,               !- Back Side Infrared Hemispherical Emissivity",
        "    0.9000000;               !- Conductivity {W/m-K}",

        "  WindowMaterial:Glazing,",
        "    GLASS - LOW IRON 3 MM,   !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.0030000E+00,           !- Thickness {m}",
        "    0.8990000,               !- Solar Transmittance at Normal Incidence",
        "    0.0790000,               !- Front Side Solar Reflectance at Normal Incidence",
        "    0.0790000,               !- Back Side Solar Reflectance at Normal Incidence",
        "    0.9130000,               !- Visible Transmittance at Normal Incidence",
        "    0.0820000,               !- Front Side Visible Reflectance at Normal Incidence",
        "    0.0820000,               !- Back Side Visible Reflectance at Normal Incidence",
        "    0,                       !- Infrared Transmittance at Normal Incidence",
        "    0.8400000,               !- Front Side Infrared Hemispherical Emissivity",
        "    0.8400000,               !- Back Side Infrared Hemispherical Emissivity",
        "    0.9000000E+00;           !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    WinAirB1 - AIRSPACE RESISTANCE,  !- Name",
        "    AIR,                     !- Gas Type",
        "    1.3000000E-02;           !- Thickness {m}",

        "  Construction,",
        "    EXTERIOR,                !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 2",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 3",
        "    C7 - 8 IN LW CONCRETE BLOCK,  !- Layer 4",
        "    E8 - 5 / 8 IN PLASTER OR GYP BOARD;  !- Layer 5",

        "  Construction,",
        "    DOUBLE PANE WINDOW,      !- Name",
        "    GLASS - CLEAR SHEET 1 / 8 IN,  !- Outside Layer",
        "    WinAirB1 - AIRSPACE RESISTANCE,  !- Layer 2",
        "    GLASS - CLEAR SHEET 1 / 8 IN;  !- Layer 3",

        "  Construction,",
        "    ThermalChimney IN,       !- Name",
        "    TABOR SOLAR ABSORBER,    !- Outside Layer",
        "    A1 - 1 IN STUCCO,        !- Layer 2",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 3",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 4",
        "    B3 - 2 IN INSULATION,    !- Layer 5",
        "    B3 - 2 IN INSULATION,    !- Layer 6",
        "    C7 - 8 IN LW CONCRETE BLOCK,  !- Layer 7",
        "    E8 - 5 / 8 IN PLASTER OR GYP BOARD;  !- Layer 8",

        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    CONCRETE - SAND AND GRAVEL 4 IN;  !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    ROOFING - WOOD SHINGLES, !- Outside Layer",
        "    BLBD - SHEATHING NAIL BASE 1 / 2 IN,  !- Layer 2",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 3",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 4",
        "    B3 - 2 IN INSULATION,    !- Layer 5",
        "    B3 - 2 IN INSULATION,    !- Layer 6",
        "    E6 - 1 / 2 IN GYP SHEATHING BOARD;  !- Layer 7",

        "  Construction,",
        "    SGL CLR LOW IRON 3 MM,   !- Name",
        "    GLASS - LOW IRON 3 MM;   !- Outside Layer",

        "  Construction,",
        "    ThermalChimney OUT,      !- Name",
        "    E8 - 5 / 8 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    C7 - 8 IN LW CONCRETE BLOCK,  !- Layer 2",
        "    B3 - 2 IN INSULATION,    !- Layer 3",
        "    B3 - 2 IN INSULATION,    !- Layer 4",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 5",
        "    B12 - 3 IN DENSE INSULATION,  !- Layer 6",
        "    A1 - 1 IN STUCCO,        !- Layer 7",
        "    TABOR SOLAR ABSORBER;    !- Layer 8",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World,                   !- Coordinate System",
        "    ,                        !- Daylighting Reference Point Coordinate System",
        "    World;                   !- Rectangular Surface Coordinate System",

        "  Zone,",
        "    ZONE 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    10.00000,                !- X Origin {m}",
        "    15.00000,                !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Zone,",
        "    ZONE 2,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    10.00000,                !- X Origin {m}",
        "    14.7,                    !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate,           !- Volume {m3}",
        "    autocalculate,           !- Floor Area {m2}",
        "    TARP;                    !- Zone Inside Convection Algorithm",

        "  Zone,",
        "    ZONE 3,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    40.00000,                !- X Origin {m}",
        "    15.00000,                !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Zone,",
        "    ZONE 4,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    40,                      !- X Origin {m}",
        "    14.7,                    !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate,           !- Volume {m3}",
        "    autocalculate,           !- Floor Area {m2}",
        "    TARP;                    !- Zone Inside Convection Algorithm",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.00000,25.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,25.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,25.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.00000,25.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,25.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,25.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.00000,25.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,25.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,15.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
        "    Wall,                    !- Surface Type",
        "    ThermalChimney IN,       !- Construction Name",
        "    ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn002:Wall004,           !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,15.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,15.00000,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,25.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,25.00000,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE 1,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.00000,25.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,25.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,14.7,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.00000,14.7,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,14.7,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Wall002,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.00000,14.7,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,15.00000,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Wall003,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.00000,15.00000,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,14.7,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,14.7,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Wall004,           !- Name",
        "    Wall,                    !- Surface Type",
        "    ThermalChimney OUT,      !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Wall004,           !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,15.00000,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,15.00000,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Wall005,           !- Name",
        "    Wall,                    !- Surface Type",
        "    ThermalChimney OUT,      !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,15.00000,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.00000,15.00000,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,15.00000,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,15.00000,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn002:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.00000,14.7,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,14.7,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,15.00000,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn002:Roof001,           !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE 2,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.00000,15.00000,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.00000,15.00000,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn003:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 3,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    60.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    60.00000,25.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,25.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn003:Wall002,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 3,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,25.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    60.00000,25.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,25.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    40.00000,25.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn003:Wall003,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 3,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    40.00000,25.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,25.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    40.00000,15.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn003:Wall004,           !- Name",
        "    Wall,                    !- Surface Type",
        "    ThermalChimney IN,       !- Construction Name",
        "    ZONE 3,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn004:Wall004,           !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    40.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    60.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,15.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn003:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE 3,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn003:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,15.00000,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,25.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,25.00000,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn003:Roof001,           !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE 3,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    40.00000,25.00000,3.000000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    60.00000,15.00000,3.000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,25.00000,3.000000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    40.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,14.7,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    60.00000,14.7,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,14.7,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Wall002,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    60.00000,14.7,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    60.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,15.00000,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Wall003,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTERIOR,                !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    40.00000,15.00000,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,14.7,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    40.00000,14.7,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Wall004,           !- Name",
        "    Wall,                    !- Surface Type",
        "    ThermalChimney OUT,      !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn003:Wall004,           !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,15.00000,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    60.00000,15.00000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    40.00000,15.00000,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Wall005,           !- Name",
        "    Wall,                    !- Surface Type",
        "    ThermalChimney OUT,      !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,15.00000,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    60.00000,15.00000,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,15.00000,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    40.00000,15.00000,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn004:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    60.00000,14.7,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,14.7,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    40.00000,15.00000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,15.00000,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn004:Roof001,           !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE 4,                  !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    40.00000,15.00000,6.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    60.00000,14.7,6.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    60.00000,15.00000,6.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall002:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE WINDOW,      !- Construction Name",
        "    Zn001:Wall002,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.000000,                !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    24.00000,25.00000,2.500000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    24.00000,25.00000,0.5000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    16.00000,25.00000,0.5000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    16.00000,25.00000,2.500000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn002:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    SGL CLR LOW IRON 3 MM,   !- Construction Name",
        "    Zn002:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.000000,                !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    10.10000,14.7,5.9,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.10000,14.7,0.1000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    29.90000,14.7,0.1000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    29.90000,14.7,5.9;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn003:Wall002:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE WINDOW,      !- Construction Name",
        "    Zn003:Wall002,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.000000,                !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    54.00000,25.00000,2.500000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    54.00000,25.00000,0.5000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    46.00000,25.00000,0.5000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    46.00000,25.00000,2.500000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn004:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    SGL CLR LOW IRON 3 MM,   !- Construction Name",
        "    Zn004:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.000000,                !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    40.10000,14.7,5.9,  !- X,Y,Z ==> Vertex 1 {m}",
        "    40.10000,14.7,0.1000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    59.90000,14.7,0.1000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    59.90000,14.7,5.9;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ZoneInfiltration:DesignFlowRate,",
        "    Zone1Infiltration,       !- Name",
        "    ZONE 1,                  !- Zone or ZoneList Name",
        "    Always 1,                !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.01,                    !- Design Flow Rate {m3/s}",
        "    0,                       !- Flow per Zone Floor Area {m3/s-m2}",
        "    0,                       !- Flow per Exterior Surface Area {m3/s-m2}",
        "    0,                       !- Air Changes per Hour {1/hr}",
        "    1,                       !- Constant Term Coefficient",
        "    ,                        !- Temperature Term Coefficient",
        "    ,                        !- Velocity Term Coefficient",
        "    ;                        !- Velocity Squared Term Coefficient",

        "  ZoneInfiltration:DesignFlowRate,",
        "    Zone3Infiltration,       !- Name",
        "    ZONE 3,                  !- Zone or ZoneList Name",
        "    Always 1,                !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.01,                    !- Design Flow Rate {m3/s}",
        "    0,                       !- Flow per Zone Floor Area {m3/s-m2}",
        "    0,                       !- Flow per Exterior Surface Area {m3/s-m2}",
        "    0,                       !- Air Changes per Hour {1/hr}",
        "    1,                       !- Constant Term Coefficient",
        "    ,                        !- Temperature Term Coefficient",
        "    ,                        !- Velocity Term Coefficient",
        "    ;                        !- Velocity Squared Term Coefficient",

        "  ZoneThermalChimney,",
        "    ThermalChimney 1,        !- Name",
        "    ZONE 2,                  !- Zone Name",
        "    ThermalChimneyAvail,     !- Availability Schedule Name",
        "    10.0,                    !- Width of the Absorber Wall {m}",
        "    0.08,                    !- Cross Sectional Area of Air Channel Outlet {m2}",
        "    0.8,                     !- Discharge Coefficient",
        "    ZONE 1,                  !- Zone 1 Name",
        "    6.0,                     !- Distance from Top of Thermal Chimney to Inlet 1 {m}",
        "    1.0,                     !- Relative Ratios of Air Flow Rates Passing through Zone 1",
        "    0.035;                   !- Cross Sectional Areas of Air Channel Inlet 1 {m2}",

        "  ZoneThermalChimney,",
        "    ThermalChimney 2,        !- Name",
        "    ZONE 4,                  !- Zone Name",
        "    OFF,                     !- Availability Schedule Name",
        "    10.0,                    !- Width of the Absorber Wall {m}",
        "    0.08,                    !- Cross Sectional Area of Air Channel Outlet {m2}",
        "    0.8,                     !- Discharge Coefficient",
        "    ZONE 3,                  !- Zone 1 Name",
        "    6.0,                     !- Distance from Top of Thermal Chimney to Inlet 1 {m}",
        "    1.0,                     !- Relative Ratios of Air Flow Rates Passing through Zone 1",
        "    0.035;                   !- Cross Sectional Areas of Air Channel Inlet 1 {m2}",

        "  EnergyManagementSystem:Sensor,",
        "    Zone1Temp,                  !- Name",
        "    Zone 1,             !- Output:Variable or Output:Meter Index Key Name",
        "    Zone Air Temperature ;  !- Output:Variable or Output:Meter Name",

        "  EnergyManagementSystem:Actuator,",
        "    MyChimney,            !- Name",
        "    ThermalChimney 1,    !- Actuated Component Unique Name",
        "    Zone Thermal Chimney,  !- Actuated Component Type",
        "    Air Exchange Flow Rate;  !- Actuated Component Control Type",

        "  EnergyManagementSystem:ProgramCallingManager,",
        "    ThermalChimneyCalling,  !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    ThermalChimneyController;    !- Program Name 1",

        "  EnergyManagementSystem:Program,",
        "    ThermalChimneyController,    !- Name",
        "    IF Zone1Temp < 34.0,          !- Program Line 1",
        "    SET MyChimney = NULL,   !- Program Line 2",
        "    ELSE,                    !- <none>",
        "    SET MyChimney = 0.01,  !- <none>",
        "    ENDIF;                   !- <none>",

        "  Output:EnergyManagementSystem,",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataIPShortCut->lNumericFieldBlanks.allocate(1000);
    state->dataIPShortCut->lAlphaFieldBlanks.allocate(1000);
    state->dataIPShortCut->cAlphaFieldNames.allocate(1000);
    state->dataIPShortCut->cNumericFieldNames.allocate(1000);
    state->dataIPShortCut->cAlphaArgs.allocate(1000);
    state->dataIPShortCut->rNumericArgs.allocate(1000);
    state->dataIPShortCut->lNumericFieldBlanks = false;
    state->dataIPShortCut->lAlphaFieldBlanks = false;
    state->dataIPShortCut->cAlphaFieldNames = " ";
    state->dataIPShortCut->cNumericFieldNames = " ";
    state->dataIPShortCut->cAlphaArgs = " ";
    state->dataIPShortCut->rNumericArgs = 0.0;

    bool localErrorsFound = false;
    // Read objects
    HeatBalanceManager::GetProjectControlData(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);
    HeatBalanceManager::GetZoneData(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);
    Material::GetWindowGlassSpectralData(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);
    Material::GetMaterialData(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);
    HeatBalanceManager::GetConstructData(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::GetSurfaceData(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);

    state->dataHeatBal->Zone(2).HasWindow = true;
    state->dataHeatBal->Zone(4).HasWindow = true;
    state->dataHeatBalSurf->SurfTempIn.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfHConvInt.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfHConvInt = 0.1;
    state->dataHeatBalSurf->SurfTempIn = 25.00;
    int surfNum = Util::FindItemInList("ZN002:WALL001", state->dataSurface->Surface);
    state->dataHeatBalSurf->SurfTempIn(surfNum) = 25.92;
    surfNum = Util::FindItemInList("ZN002:WALL001:WIN001", state->dataSurface->Surface);
    state->dataHeatBalSurf->SurfTempIn(surfNum) = 25.92;
    surfNum = Util::FindItemInList("ZN002:WALL004", state->dataSurface->Surface);
    state->dataHeatBalSurf->SurfTempIn(surfNum) = 26.99;
    surfNum = Util::FindItemInList("ZN004:WALL001:WIN001", state->dataSurface->Surface);
    state->dataHeatBalSurf->SurfTempIn(surfNum) = 22.99;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    for (auto &thisZoneHB : state->dataZoneTempPredictorCorrector->zoneHeatBalance) {
        thisZoneHB.MAT = 23.0;
        thisZoneHB.airHumRat = 0.01;
    }
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    Sched::GetSchedule(*state, "THERMALCHIMNEYAVAIL")->currentVal = 1.0;
    state->dataHeatBal->ZnAirRpt.allocate(state->dataGlobal->NumOfZones);
    // No EMS
    ThermalChimney::GetThermalChimney(*state, localErrorsFound);
    EXPECT_FALSE(localErrorsFound);
    ThermalChimney::CalcThermalChimney(*state);
    EXPECT_NEAR(state->dataThermalChimneys->ThermalChimneyReport(1).OverallTCVolumeFlow, 0.015668, 0.0001);
    // EMS Override
    state->dataThermalChimneys->ThermalChimneySys(1).EMSOverrideOn = true;
    state->dataThermalChimneys->ThermalChimneySys(1).EMSAirFlowRateValue = 0.01;
    ThermalChimney::CalcThermalChimney(*state);
    EXPECT_NEAR(state->dataThermalChimneys->ThermalChimneyReport(1).OverallTCVolumeFlow, 0.01, 0.0001);
}
