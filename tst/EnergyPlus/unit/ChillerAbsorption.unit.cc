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

// EnergyPlus::ChillerAbsorption Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/ChillerAbsorption.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimulationManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::ChillerAbsorption;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::SimulationManager;

TEST_F(EnergyPlusFixture, ChillerAbsorption_Calc)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,4;",

        "  Building,",
        "    NONE,                    !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    0.4000000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

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

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    42.00,                   !- Latitude {deg}",
        "    -87.88,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

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

        "  Material,",
        "    C6 - 8 IN CLAY TILE,     !- Name",
        "    Smooth,                  !- Roughness",
        "    0.2033016,               !- Thickness {m}",
        "    0.5707605,               !- Conductivity {W/m-K}",
        "    1121.292,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.8200000,               !- Solar Absorptance",
        "    0.8200000;               !- Visible Absorptance",

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
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    WIN-LAY-GLASS-LIGHT,     !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.90,                    !- Solar Transmittance at Normal Incidence",
        "    0.031,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.031,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.90,                    !- Visible Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "  Construction,",
        "    FLOOR SLAB 8 IN,         !- Name",
        "    C10 - 8 IN HW CONCRETE;  !- Outside Layer",

        "  Construction,",
        "    ROOF34,                  !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",

        "  Construction,",
        "    WIN-CON-LIGHT,           !- Name",
        "    WIN-LAY-GLASS-LIGHT;     !- Outside Layer",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS,              !- Numeric Type",
        "    Temperature;             !- Unit Type",

        "  ScheduleTypeLimits,",
        "    Control Type,            !- Name",
        "    0,                       !- Lower Limit Value",
        "    4,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  Schedule:Compact,",
        "    Activity Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,131.8;      !- Field 3",

        "  Schedule:Compact,",
        "    Work Eff Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Clothing Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Air Velo Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.137;      !- Field 3",

        "  Schedule:Compact,",
        "    OFFICE OCCUPANCY,        !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay, !- Field 2",
        "    Until: 6:00,0.0,         !- Field 3",
        "    Until: 7:00,0.10,        !- Field 5",
        "    Until: 8:00,0.50,        !- Field 7",
        "    Until: 12:00,1.00,       !- Field 9",
        "    Until: 13:00,0.50,       !- Field 11",
        "    Until: 16:00,1.00,       !- Field 13",
        "    Until: 17:00,0.50,       !- Field 15",
        "    Until: 18:00,0.10,       !- Field 17",
        "    Until: 24:00,0.0,        !- Field 19",
        "    For: AllOtherDays,       !- Field 21",
        "    Until: 24:00,0.0;        !- Field 22",

        "  Schedule:Compact,",
        "    INTERMITTENT,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay, !- Field 2",
        "    Until: 8:00,0.0,         !- Field 3",
        "    Until: 18:00,1.00,       !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: AllOtherDays,       !- Field 9",
        "    Until: 24:00,0.0;        !- Field 10",

        "  Schedule:Compact,",
        "    OFFICE LIGHTING,         !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay, !- Field 2",
        "    Until: 6:00,0.05,        !- Field 3",
        "    Until: 7:00,0.20,        !- Field 5",
        "    Until: 17:00,1.00,       !- Field 7",
        "    Until: 18:00,0.50,       !- Field 9",
        "    Until: 24:00,0.05,       !- Field 11",
        "    For: AllOtherDays,       !- Field 13",
        "    Until: 24:00,0.05;       !- Field 14",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Lights,",
        "    NORTH ZONE Lights 1,     !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    OFFICE LIGHTING,         !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    1200.0,                  !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.2000000,               !- Fraction Radiant",
        "    0.2000000,               !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "  People,",
        "    West Zone,               !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    OFFICE OCCUPANCY,        !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    5.000000,                !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3000000,               !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    Activity Sch,            !- Activity Level Schedule Name",
        "    3.82E-8,                 !- Carbon Dioxide Generation Rate {m3/s-W}",
        "    ,                        !- Enable ASHRAE 55 Comfort Warnings",
        "    EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        "    ,                        !- Surface Name/Angle Factor List Name",
        "    Work Eff Sch,            !- Work Efficiency Schedule Name",
        "    ClothingInsulationSchedule,  !- Clothing Insulation Calculation Method",
        "    ,                        !- Clothing Insulation Calculation Method Schedule Name",
        "    Clothing Sch,            !- Clothing Insulation Schedule Name",
        "    Air Velo Sch,            !- Air Velocity Schedule Name",
        "    FANGER;                  !- Thermal Comfort Model 1 Type",

        "  ElectricEquipment,",
        "    West Zone ElecEq 1,      !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    INTERMITTENT,            !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    2500.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    WIN-CON-LIGHT,           !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.50000,0,2.5000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.50000,0,0.5000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    8.50000,0,0.5000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    8.50000,0,2.5000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.0000,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.0000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.0000,10.0000,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.0000,10.0000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.0000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.0000,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.0000,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.0000,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,10.0000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,10.0000,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR SLAB 8 IN,         !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.0000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,10.0000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF34,                  !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.0000,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,0,3.048000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,10.0000,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Schedule:Compact,",
        "    All Hours,               !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    ON,                      !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Seasonal Reset Supply Air Temp Sch,  !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,16.0,       !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,13.0,       !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,16.0;       !- Field 11",

        "  Schedule:Compact,",
        "    CW Loop Temp Schedule,   !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,6.67;       !- Field 3",

        "  Schedule:Compact,",
        "    HW Loop Temp Schedule,   !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,60;         !- Field 3",

        "  Schedule:Compact,",
        "    FanAndCoilAvailSched,    !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    CoolingCoilAvailSched,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Heating Setpoints,       !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until:  7:00,15.0,       !- Field 3",
        "    Until: 17:00,20.0,       !- Field 5",
        "    Until: 24:00,15.0;       !- Field 7",

        "  Schedule:Compact,",
        "    Cooling Setpoints,       !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until:  7:00,30.0,       !- Field 3",
        "    Until: 17:00,24.0,       !- Field 5",
        "    Until: 24:00,30.0;       !- Field 7",

        "  Schedule:Compact,",
        "    Zone Control Type Sched, !- Name",
        "    Control Type,            !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1,          !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,2,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,1;          !- Field 11",

        "  AirLoopHVAC,",
        "    Typical Terminal Reheat 1,  !- Name",
        "    Reheat System 1 Controllers,  !- Controller List Name",
        "    Reheat System 1 Avail List,  !- Availability Manager List Name",
        "    1.3,                     !- Design Supply Air Flow Rate {m3/s}",
        "    Air Loop Branches,       !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    Air Loop Inlet Node,     !- Supply Side Inlet Node Name",
        "    Return Air Mixer Outlet, !- Demand Side Outlet Node Name",
        "    Zone Equipment Inlet Node,  !- Demand Side Inlet Node Names",
        "    Air Loop Outlet Node;    !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:ControllerList,",
        "    Reheat System 1 Controllers,  !- Name",
        "    Controller:WaterCoil,    !- Controller 1 Object Type",
        "    Main Cooling Coil Controller;  !- Controller 1 Name",

        "  BranchList,",
        "    Air Loop Branches,       !- Name",
        "    Air Loop Main Branch;    !- Branch 1 Name",

        "  Branch,",
        "    Air Loop Main Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:ConstantVolume,      !- Component 1 Object Type",
        "    Supply Fan 1,            !- Component 1 Name",
        "    Air Loop Inlet Node,     !- Component 1 Inlet Node Name",
        "    Cooling Coil Air Inlet Node,  !- Component 1 Outlet Node Name",
        "    Coil:Cooling:Water:DetailedGeometry,  !- Component 2 Object Type",
        "    Detailed Cooling Coil,   !- Component 2 Name",
        "    Cooling Coil Air Inlet Node,  !- Component 2 Inlet Node Name",
        "    Air Loop Outlet Node;    !- Component 2 Outlet Node Name",

        "  AvailabilityManagerAssignmentList,",
        "    Reheat System 1 Avail List,  !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    Reheat System 1 Avail;   !- Availability Manager 1 Name",

        "  AvailabilityManager:Scheduled,",
        "    Reheat System 1 Avail,   !- Name",
        "    FanAndCoilAvailSched;    !- Schedule Name",

        "  PlantLoop,",
        "    Chilled Water Loop,      !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    CW Loop Operation,       !- Plant Equipment Operation Scheme Name",
        "    CW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    98,                      !- Maximum Loop Temperature {C}",
        "    1,                       !- Minimum Loop Temperature {C}",
        "    0.0011,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    CW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    CW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Cooling Supply Side Branches,  !- Plant Side Branch List Name",
        "    Cooling Supply Side Connectors,  !- Plant Side Connector List Name",
        "    CW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    CW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Cooling Demand Side Branches,  !- Demand Side Branch List Name",
        "    Cooling Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal;                 !- Load Distribution Scheme",

        "  SetpointManager:Scheduled,",
        "    Chilled Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    CW Loop Temp Schedule,   !- Schedule Name",
        "    Chilled Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Chilled Water Loop Setpoint Node List,  !- Name",
        "    CW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Cooling Supply Side Branches,  !- Name",
        "    CW Pump Branch,          !- Branch 1 Name",
        "    Big Chiller Branch,      !- Branch 2 Name",
        "    Supply Bypass Branch,    !- Branch 3 Name",
        "    Cooling Supply Outlet;   !- Branch 4 Name",

        "  BranchList,",
        "    Cooling Demand Side Branches,  !- Name",
        "    Cooling Demand Inlet,    !- Branch 1 Name",
        "    Cooling Coil Branch,     !- Branch 2 Name",
        "    Demand Bypass Branch,    !- Branch 3 Name",
        "    Cooling Demand Outlet;   !- Branch 4 Name",

        "  ConnectorList,",
        "    Cooling Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Loop Splitter,        !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Loop Mixer;           !- Connector 2 Name",

        "  ConnectorList,",
        "    Cooling Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Demand Splitter,      !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Demand Mixer;         !- Connector 2 Name",

        "  Branch,",
        "    Cooling Demand Inlet,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Inlet Pipe,  !- Component 1 Name",
        "    CW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Demand Side Inlet Pipe,  !- Name",
        "    CW Demand Inlet Node,    !- Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Cooling Coil Branch,     !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:Water:DetailedGeometry,  !- Component 1 Object Type",
        "    Detailed Cooling Coil,   !- Component 1 Name",
        "    Cooling Coil Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Cooling Coil Water Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Demand Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Bypass,      !- Component 1 Name",
        "    CW Demand Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Demand Side Bypass,      !- Name",
        "    CW Demand Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Cooling Demand Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CW Demand Side Outlet Pipe,  !- Component 1 Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CW Demand Side Outlet Pipe,  !- Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Cooling Supply Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Outlet Pipe, !- Component 1 Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Outlet Pipe, !- Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Supply Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    CW Pump Branch,          !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    Circ Pump,               !- Component 1 Name",
        "    CW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Big Chiller Branch,      !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Chiller:Absorption,      !- Component 1 Object Type",
        "    Big Chiller,             !- Component 1 Name",
        "    Big Chiller Inlet Node,  !- Component 1 Inlet Node Name",
        "    Big Chiller Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Supply Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Bypass,      !- Component 1 Name",
        "    CW Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Bypass,      !- Name",
        "    CW Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Supply Bypass Outlet Node;  !- Outlet Node Name",

        "  Connector:Splitter,",
        "    CW Loop Splitter,        !- Name",
        "    CW Pump Branch,          !- Inlet Branch Name",
        "    Big Chiller Branch,      !- Outlet Branch 1 Name",
        "    Supply Bypass Branch;    !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    CW Loop Mixer,           !- Name",
        "    Cooling Supply Outlet,   !- Outlet Branch Name",
        "    Big Chiller Branch,      !- Inlet Branch 1 Name",
        "    Supply Bypass Branch;    !- Inlet Branch 2 Name",

        "  Connector:Splitter,",
        "    CW Demand Splitter,      !- Name",
        "    Cooling Demand Inlet,    !- Inlet Branch Name",
        "    Demand Bypass Branch,    !- Outlet Branch 1 Name",
        "    Cooling Coil Branch;     !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    CW Demand Mixer,         !- Name",
        "    Cooling Demand Outlet,   !- Outlet Branch Name",
        "    Cooling Coil Branch,     !- Inlet Branch 1 Name",
        "    Demand Bypass Branch;    !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    CW Loop Operation,       !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    All Operation,           !- Control Scheme 1 Name",
        "    All Hours;               !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    All Operation,           !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    9000000,                 !- Load Range 1 Upper Limit {W}",
        "    All Chillers;            !- Range 1 Equipment List Name",

        "  PlantEquipmentList,",
        "    All Chillers,            !- Name",
        "    Chiller:Absorption,      !- Equipment 1 Object Type",
        "    Big Chiller;             !- Equipment 1 Name",

        "  Chiller:Absorption,",
        "    Big Chiller,             !- Name",
        "    100000,                  !- Nominal Capacity {W}",
        "    250,                     !- Nominal Pumping Power {W}",
        "    Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name",
        "    Big Chiller Outlet Node, !- Chilled Water Outlet Node Name",
        "    Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "    Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    0.15,                    !- Minimum Part Load Ratio",
        "    1.0,                     !- Maximum Part Load Ratio",
        "    0.65,                    !- Optimum Part Load Ratio",
        "    35.0,                    !- Design Condenser Inlet Temperature {C}",
        "    0.0011,                  !- Design Chilled Water Flow Rate {m3/s}",
        "    0.0011,                  !- Design Condenser Water Flow Rate {m3/s}",
        "    0.0,                     !- Coefficient 1 of the Hot Water or Steam Use Part Load Ratio Curve",
        "    1.0,                     !- Coefficient 2 of the Hot Water or Steam Use Part Load Ratio Curve",
        "    0.0,                     !- Coefficient 3 of the Hot Water or Steam Use Part Load Ratio Curve",
        "    1.0,                     !- Coefficient 1 of the Pump Electric Use Part Load Ratio Curve",
        "    0,                       !- Coefficient 2 of the Pump Electric Use Part Load Ratio Curve",
        "    0,                       !- Coefficient 3 of the Pump Electric Use Part Load Ratio Curve",
        "    5,                       !- Chilled Water Outlet Temperature Lower Limit {C}",
        "    AbsorberHWInletNode,     !- Generator Inlet Node Name",
        "    AbsorberHWOutletNode,    !- Generator Outlet Node Name",
        "    LeavingSetpointModulated,!- Chiller Flow Mode",
        "    HotWater,                !- Generator Heat Source Type",
        "    Autosize;                !- Design Generator Fluid Flow Rate {m3/s}",

        "  SetpointManager:Scheduled,",
        "    Big Chiller Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    CW Loop Temp Schedule,   !- Schedule Name",
        "    Big Chiller Outlet Node; !- Setpoint Node or NodeList Name",

        "  SetpointManager:Scheduled,",
        "    Generator Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Gen Outlet Temp Schedule,!- Schedule Name",
        "    AbsorberHWOutletNode;    !- Setpoint Node or NodeList Name",

        "  Schedule:Compact,",
        "    Gen Outlet Temp Schedule,!- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,70.0;       !- Field 3",

        "  Pump:VariableSpeed,",
        "    Circ Pump,               !- Name",
        "    CW Supply Inlet Node,    !- Inlet Node Name",
        "    CW Pump Outlet Node,     !- Outlet Node Name",
        "    0.0011,                  !- Design Maximum Flow Rate {m3/s}",
        "    300000,                  !- Design Pump Head {Pa}",
        "    500,                     !- Design Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    1.0,                     !- Coefficient 1 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 2 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 3 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 4 of the Part Load Performance Curve",
        "    0.0,                     !- Design Minimum Flow Rate {m3/s}",
        "    Intermittent;            !- Pump Control Type",

        "  CondenserLoop,",
        "    Chilled Water Condenser Loop,  !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Tower Loop Operation,    !- Condenser Equipment Operation Scheme Name",
        "    Condenser Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
        "    80,                      !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    0.0011,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Condenser Loop Volume {m3}",
        "    Condenser Supply Inlet Node,  !- Condenser Side Inlet Node Name",
        "    Condenser Supply Outlet Node,  !- Condenser Side Outlet Node Name",
        "    Condenser Supply Side Branches,  !- Condenser Side Branch List Name",
        "    Condenser Supply Side Connectors,  !- Condenser Side Connector List Name",
        "    Condenser Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    Condenser Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    Condenser Demand Side Branches,  !- Condenser Demand Side Branch List Name",
        "    Condenser Demand Side Connectors,  !- Condenser Demand Side Connector List Name",
        "    SequentialLoad;          !- Load Distribution Scheme",

        "  SetpointManager:FollowOutdoorAirTemperature,",
        "    MyCondenserControl,      !- Name",
        "    Temperature,             !- Control Variable",
        "    OutdoorAirWetBulb,       !- Reference Temperature Type",
        "    0,                       !- Offset Temperature Difference {deltaC}",
        "    80,                      !- Maximum Setpoint Temperature {C}",
        "    10,                      !- Minimum Setpoint Temperature {C}",
        "    Condenser Supply Outlet Node;  !- Setpoint Node or NodeList Name",

        "  BranchList,",
        "    Condenser Supply Side Branches,  !- Name",
        "    Condenser Supply Inlet Branch,  !- Branch 1 Name",
        "    Condenser Supply Tower Branch,  !- Branch 2 Name",
        "    Condenser Supply Bypass Branch,  !- Branch 3 Name",
        "    Condenser Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    Condenser Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Condenser Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Condenser Supply Mixer;  !- Connector 2 Name",

        "  Branch,",
        "    Condenser Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    Cond Circ Pump,          !- Component 1 Name",
        "    Condenser Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    Condenser Pump Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Condenser Supply Tower Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    CoolingTower:SingleSpeed,!- Component 1 Object Type",
        "    Big Tower,               !- Component 1 Name",
        "    Condenser Tower Inlet Node,  !- Component 1 Inlet Node Name",
        "    Condenser Tower Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Condenser Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Condenser Supply Side Bypass,  !- Component 1 Name",
        "    Cond Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    Cond Supply Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Condenser Supply Side Bypass,  !- Name",
        "    Cond Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    Cond Supply Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Condenser Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Condenser Supply Outlet, !- Component 1 Name",
        "    Condenser Supply Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    Condenser Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Condenser Supply Outlet, !- Name",
        "    Condenser Supply Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    Condenser Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    Condenser Demand Side Branches,  !- Name",
        "    Condenser Demand Inlet Branch,  !- Branch 1 Name",
        "    Big Chiller Condenser Branch,  !- Branch 2 Name",
        "    Condenser Demand Bypass Branch,  !- Branch 3 Name",
        "    Condenser Demand Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    Condenser Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Condenser Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Condenser Demand Mixer;  !- Connector 2 Name",

        "  Branch,",
        "    Condenser Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Condenser Demand Inlet Pipe,  !- Component 1 Name",
        "    Condenser Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    Condenser Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Condenser Demand Inlet Pipe,  !- Name",
        "    Condenser Demand Inlet Node,  !- Inlet Node Name",
        "    Condenser Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Big Chiller Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Chiller:Absorption,      !- Component 1 Object Type",
        "    Big Chiller,             !- Component 1 Name",
        "    Big Chiller Condenser Inlet Node,  !- Component 1 Inlet Node Name",
        "    Big Chiller Condenser Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Condenser Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Condenser Demand Side Bypass,  !- Component 1 Name",
        "    Cond Demand Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    Cond Demand Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Condenser Demand Side Bypass,  !- Name",
        "    Cond Demand Bypass Inlet Node,  !- Inlet Node Name",
        "    Cond Demand Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Condenser Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Condenser Demand Outlet Pipe,  !- Component 1 Name",
        "    Condenser Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    Condenser Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Condenser Demand Outlet Pipe,  !- Name",
        "    Condenser Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    Condenser Demand Outlet Node;  !- Outlet Node Name",

        "  Connector:Splitter,",
        "    Condenser Demand Splitter,  !- Name",
        "    Condenser Demand Inlet Branch,  !- Inlet Branch Name",
        "    Big Chiller Condenser Branch,  !- Outlet Branch 1 Name",
        "    Condenser Demand Bypass Branch;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    Condenser Demand Mixer,  !- Name",
        "    Condenser Demand Outlet Branch,  !- Outlet Branch Name",
        "    Big Chiller Condenser Branch,  !- Inlet Branch 1 Name",
        "    Condenser Demand Bypass Branch;  !- Inlet Branch 2 Name",

        "  Connector:Splitter,",
        "    Condenser Supply Splitter,  !- Name",
        "    Condenser Supply Inlet Branch,  !- Inlet Branch Name",
        "    Condenser Supply Tower Branch,  !- Outlet Branch 1 Name",
        "    Condenser Supply Bypass Branch;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    Condenser Supply Mixer,  !- Name",
        "    Condenser Supply Outlet Branch,  !- Outlet Branch Name",
        "    Condenser Supply Tower Branch,  !- Inlet Branch 1 Name",
        "    Condenser Supply Bypass Branch;  !- Inlet Branch 2 Name",

        "  CondenserEquipmentOperationSchemes,",
        "    Tower Loop Operation,    !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Year Round Tower Operation,  !- Control Scheme 1 Name",
        "    ON;                      !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    Year Round Tower Operation,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    90000000,                !- Load Range 1 Upper Limit {W}",
        "    All Towers;              !- Range 1 Equipment List Name",

        "  CondenserEquipmentList,",
        "    All Towers,              !- Name",
        "    CoolingTower:SingleSpeed,!- Equipment 1 Object Type",
        "    Big Tower;               !- Equipment 1 Name",

        "  CoolingTower:SingleSpeed,",
        "    Big Tower,               !- Name",
        "    Condenser Tower Inlet Node,  !- Water Inlet Node Name",
        "    Condenser Tower Outlet Node,  !- Water Outlet Node Name",
        "    0.0011,                  !- Design Water Flow Rate {m3/s}",
        "    16.0,                    !- Design Air Flow Rate {m3/s}",
        "    1000,                    !- Design Fan Power {W}",
        "    1750.0,                  !- Design U-Factor Times Area Value {W/K}",
        "    0.0,                     !- Free Convection Air Flow Rate {m3/s}",
        "    ,                        !- Free Convection Air Flow Rate Sizing Factor",
        "    0.0,                     !- Free Convection U-Factor Times Area Value {W/K}",
        "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "    ,                        !- Nominal Capacity {W}",
        "    ,                        !- Free Convection Capacity {W}",
        "    ;                        !- Free Convection Nominal Capacity Sizing Factor",

        "  Pump:VariableSpeed,",
        "    Cond Circ Pump,          !- Name",
        "    Condenser Supply Inlet Node,  !- Inlet Node Name",
        "    Condenser Pump Outlet Node,  !- Outlet Node Name",
        "    0.0011,                  !- Design Maximum Flow Rate {m3/s}",
        "    300000,                  !- Design Pump Head {Pa}",
        "    500,                     !- Design Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    1.0,                     !- Coefficient 1 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 2 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 3 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 4 of the Part Load Performance Curve",
        "    0.0,                     !- Design Minimum Flow Rate {m3/s}",
        "    Intermittent;            !- Pump Control Type",

        "  PlantLoop,",
        "    Hot Water Loop,          !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Hot Loop Operation,      !- Plant Equipment Operation Scheme Name",
        "    HW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    100.0000,                !- Maximum Loop Temperature {C}",
        "    10.00000,                !- Minimum Loop Temperature {C}",
        "    0.0043,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    HW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    HW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Heating Supply Side Branches,  !- Plant Side Branch List Name",
        "    Heating Supply Side Connectors,  !- Plant Side Connector List Name",
        "    HW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    HW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Heating Demand Side Branches,  !- Demand Side Branch List Name",
        "    Heating Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal;                 !- Load Distribution Scheme",

        "  SetpointManager:Scheduled,",
        "    Hot Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    HW Loop Temp Schedule,   !- Schedule Name",
        "    Hot Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Hot Water Loop Setpoint Node List,  !- Name",
        "    HW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Heating Supply Side Branches,  !- Name",
        "    Heating Supply Inlet Branch,  !- Branch 1 Name",
        "    Heating Purchased Hot Water Branch,  !- Branch 2 Name",
        "    Heating Supply Bypass Branch,  !- Branch 3 Name",
        "    Heating Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    Heating Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Heating Supply Splitter, !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Heating Supply Mixer;    !- Connector 2 Name",

        "  Branch,",
        "    Heating Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    HW Circ Pump,            !- Component 1 Name",
        "    HW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Purchased Hot Water Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    Purchased Heating,       !- Component 1 Name",
        "    Purchased Heat Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Heat Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Side Bypass,  !- Component 1 Name",
        "    Heating Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Side Bypass,  !- Name",
        "    Heating Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Heating Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Outlet,   !- Component 1 Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Outlet,   !- Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Supply Outlet Node;   !- Outlet Node Name",

        "  BranchList,",
        "    Heating Demand Side Branches,  !- Name",
        "    Reheat Inlet Branch,     !- Branch 1 Name",
        "    Zone 1 Reheat Branch,    !- Branch 2 Name",
        "    Reheat Bypass Branch,    !- Branch 5 Name",
        "    Reheat Outlet Branch;    !- Branch 6 Name",

        "  ConnectorList,",
        "    Heating Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Reheat Splitter,         !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Reheat Mixer;            !- Connector 2 Name",

        "  Branch,",
        "    Reheat Inlet Branch,     !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Inlet Pipe,       !- Component 1 Name",
        "    HW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Inlet Pipe,       !- Name",
        "    HW Demand Inlet Node,    !- Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Reheat Outlet Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Outlet Pipe,      !- Component 1 Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Outlet Pipe,      !- Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Zone 1 Reheat Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:Water,      !- Component 1 Object Type",
        "    Reheat Coil Zone 1,      !- Component 1 Name",
        "    Zone 1 Reheat Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Zone 1 Reheat Water Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Reheat Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Bypass,           !- Component 1 Name",
        "    Reheat Bypass Inlet Node,!- Component 1 Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Bypass,           !- Name",
        "    Reheat Bypass Inlet Node,!- Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Outlet Node Name",

        "  Connector:Splitter,",
        "    Reheat Splitter,         !- Name",
        "    Reheat Inlet Branch,     !- Inlet Branch Name",
        "    Zone 1 Reheat Branch,    !- Outlet Branch 1 Name",
        "    Reheat Bypass Branch;    !- Outlet Branch 4 Name",

        "  Connector:Mixer,",
        "    Reheat Mixer,            !- Name",
        "    Reheat Outlet Branch,    !- Outlet Branch Name",
        "    Zone 1 Reheat Branch,    !- Inlet Branch 1 Name",
        "    Reheat Bypass Branch;    !- Inlet Branch 4 Name",

        "  Connector:Splitter,",
        "    Heating Supply Splitter, !- Name",
        "    Heating Supply Inlet Branch,  !- Inlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Outlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    Heating Supply Mixer,    !- Name",
        "    Heating Supply Outlet Branch,  !- Outlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Inlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Hot Loop Operation,      !- Name",
        "    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "    Purchased Only,          !- Control Scheme 1 Name",
        "    ON;                      !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:HeatingLoad,",
        "    Purchased Only,          !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000,                 !- Load Range 1 Upper Limit {W}",
        "    heating plant;           !- Range 1 Equipment List Name",

        "  PlantEquipmentList,",
        "    heating plant,           !- Name",
        "    DistrictHeating:Water,         !- Equipment 1 Object Type",
        "    Purchased Heating;       !- Equipment 1 Name",

        "  Pump:VariableSpeed,",
        "    HW Circ Pump,            !- Name",
        "    HW Supply Inlet Node,    !- Inlet Node Name",
        "    HW Pump Outlet Node,     !- Outlet Node Name",
        "    0.0043,                  !- Design Maximum Flow Rate {m3/s}",
        "    300000,                  !- Design Pump Head {Pa}",
        "    2000,                    !- Design Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    1.0,                     !- Coefficient 1 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 2 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 3 of the Part Load Performance Curve",
        "    0.0,                     !- Coefficient 4 of the Part Load Performance Curve",
        "    0.0,                     !- Design Minimum Flow Rate {m3/s}",
        "    Intermittent;            !- Pump Control Type",

        "  DistrictHeating:Water,",
        "    Purchased Heating,       !- Name",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:SingleHeating,  !- Control 1 Object Type",
        "    Heating Setpoint with SB,!- Control 1 Name",
        "    ThermostatSetpoint:SingleCooling,  !- Control 2 Object Type",
        "    Cooling Setpoint with SB;!- Control 2 Name",

        "  ThermostatSetpoint:SingleHeating,",
        "    Heating Setpoint with SB,!- Name",
        "    Heating Setpoints;       !- Setpoint Temperature Schedule Name",

        "  ThermostatSetpoint:SingleCooling,",
        "    Cooling Setpoint with SB,!- Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  AirLoopHVAC:SupplyPath,",
        "    TermReheatSupplyPath,    !- Name",
        "    Zone Equipment Inlet Node,  !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;!- Component 1 Name",

        "  AirLoopHVAC:ReturnPath,",
        "    TermReheatReturnPath,    !- Name",
        "    Return Air Mixer Outlet, !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    ,                        !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    Zone1TermReheat,         !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    Zone1TermReheat,         !- Name",
        "    Zone 1 Reheat Air Outlet Node,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:Reheat,  !- Air Terminal Object Type",
        "    Reheat Zone 1;           !- Air Terminal Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone 1 Reheat Air Outlet Node;  !- Node 1 Name",

        "  Fan:ConstantVolume,",
        "    Supply Fan 1,            !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.3,                     !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    Air Loop Inlet Node,     !- Air Inlet Node Name",
        "    Cooling Coil Air Inlet Node;  !- Air Outlet Node Name",

        "  Coil:Cooling:Water:DetailedGeometry,",
        "    Detailed Cooling Coil,   !- Name",
        "    CoolingCoilAvailSched,   !- Availability Schedule Name",
        "    0.0011,                  !- Maximum Water Flow Rate {m3/s}",
        "    6.23816,                 !- Tube Outside Surface Area {m2}",
        "    6.20007018,              !- Total Tube Inside Area {m2}",
        "    101.7158224,             !- Fin Surface Area {m2}",
        "    0.810606367,             !- Minimum Airflow Area {m2}",
        "    0.165097968,             !- Coil Depth {m}",
        "    0.43507152,              !- Fin Diameter {m}",
        "    0.001499982,             !- Fin Thickness {m}",
        "    0.014449958,             !- Tube Inside Diameter {m}",
        "    0.015879775,             !- Tube Outside Diameter {m}",
        "    385.764854,              !- Tube Thermal Conductivity {W/m-K}",
        "    203.882537,              !- Fin Thermal Conductivity {W/m-K}",
        "    0.001814292,             !- Fin Spacing {m}",
        "    0.02589977,              !- Tube Depth Spacing {m}",
        "    6,                       !- Number of Tube Rows",
        "    16,                      !- Number of Tubes per Row",
        "    Cooling Coil Water Inlet Node,  !- Water Inlet Node Name",
        "    Cooling Coil Water Outlet Node,  !- Water Outlet Node Name",
        "    Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Air Loop Outlet Node;    !- Air Outlet Node Name",

        "  AirTerminal:SingleDuct:ConstantVolume:Reheat,",
        "    Reheat Zone 1,           !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Zone 1 Reheat Air Outlet Node,  !- Air Outlet Node Name",
        "    Zone 1 Reheat Air Inlet Node,  !- Air Inlet Node Name",
        "    0.47,                    !- Maximum Air Flow Rate {m3/s}",
        "    Coil:Heating:Water,      !- Reheat Coil Object Type",
        "    Reheat Coil Zone 1,      !- Reheat Coil Name",
        "    0.0013,                  !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "    0.0,                     !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "    0.001;                   !- Convergence Tolerance",

        "  Coil:Heating:Water,",
        "    Reheat Coil Zone 1,      !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    400.0,                   !- U-Factor Times Area Value {W/K}",
        "    0.0013,                  !- Maximum Water Flow Rate {m3/s}",
        "    Zone 1 Reheat Water Inlet Node,  !- Water Inlet Node Name",
        "    Zone 1 Reheat Water Outlet Node,  !- Water Outlet Node Name",
        "    Zone 1 Reheat Air Inlet Node,  !- Air Inlet Node Name",
        "    Zone 1 Reheat Air Outlet Node,  !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    ;                        !- Rated Ratio for Air and Water Convection",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,!- Name",
        "    Zone Equipment Inlet Node,  !- Inlet Node Name",
        "    Zone 1 Reheat Air Inlet Node;  !- Outlet 1 Node Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Return Air Mixer Outlet, !- Outlet Node Name",
        "    Zone 1 Outlet Node;      !- Inlet 1 Node Name",

        "  Controller:WaterCoil,",
        "    Main Cooling Coil Controller,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Reverse,                 !- Action",
        "    FLOW,                    !- Actuator Variable",
        "    Air Loop Outlet Node,    !- Sensor Node Name",
        "    Cooling Coil Water Inlet Node,  !- Actuator Node Name",
        "    0.1,                     !- Controller Convergence Tolerance {deltaC}",
        "    0.0011,                  !- Maximum Actuated Flow {m3/s}",
        "    0.0;                     !- Minimum Actuated Flow {m3/s}",

        "  SetpointManager:Scheduled,",
        "    Supply Air Temp Manager, !- Name",
        "    Temperature,             !- Control Variable",
        "    Seasonal Reset Supply Air Temp Sch,  !- Schedule Name",
        "    Supply Air Temp Nodes;   !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Supply Air Temp Nodes,   !- Name",
        "    Air Loop Outlet Node;    !- Node 1 Name",

        "  Sizing:Plant,",
        "    DH loop,                 !- Plant or Condenser Loop Name",
        "    Heating,                 !- Loop Type",
        "    75.00,                   !- Design Loop Exit Temperature {C}",
        "    10.00;                   !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    DH loop,                 !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    DH loop Operation,       !- Plant Equipment Operation Scheme Name",
        "    DH loop Supply Side Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100.00,                  !- Maximum Loop Temperature {C}",
        "    10.00,                   !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0.000000,                !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    DH loop Supply Side Inlet,  !- Plant Side Inlet Node Name",
        "    DH loop Supply Side Outlet,  !- Plant Side Outlet Node Name",
        "    DH loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    DH loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    DH loop Demand Side Inlet,  !- Demand Side Inlet Node Name",
        "    DH loop Demand Side Outlet,  !- Demand Side Outlet Node Name",
        "    DH loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    DH loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    DH loop AvailabilityManager List,  !- Availability Manager List Name",
        "    SingleSetpoint,          !- Plant Loop Demand Calculation Scheme",
        "    None,                    !- Common Pipe Simulation",
        "    None;                    !- Pressure Simulation Type",

        "  ConnectorList,",
        "    DH loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    DH loop Demand Splitter 1,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    DH loop Demand Mixer 1;  !- Connector 2 Name",

        "  ConnectorList,",
        "    DH loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    DH loop Supply Splitter 1,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    DH loop Supply Mixer 1;  !- Connector 2 Name",

        "  BranchList,",
        "    DH loop Demand Side Branches,  !- Name",
        "    DH loop Demand Side Inlet Branch,  !- Branch 1 Name",
        "    DH loop Demand Side Bypass Branch,  !- Branch 2 Name",
        "    chiller demand side branch,  !- Branch 3 Name",
        "    DH loop Demand Side Outlet Branch;  !- Branch 4 Name",

        "  BranchList,",
        "    DH loop Supply Side Branches,  !- Name",
        "    DH loop Supply Side Inlet Branch,  !- Branch 1 Name",
        "    DH loop Supply Side Bypass Branch,  !- Branch 2 Name",
        "    DH DH loop Supply Side Branch,  !- Branch 3 Name",
        "    DH loop Supply Side Outlet Branch;  !- Branch 4 Name",

        "  Connector:Splitter,",
        "    DH loop Demand Splitter 1,  !- Name",
        "    DH loop Demand Side Inlet Branch,  !- Inlet Branch Name",
        "    DH loop Demand Side Bypass Branch,  !- Outlet Branch 1 Name",
        "    chiller demand side branch;  !- Outlet Branch 2 Name",

        "  Connector:Splitter,",
        "    DH loop Supply Splitter 1,  !- Name",
        "    DH loop Supply Side Inlet Branch,  !- Inlet Branch Name",
        "    DH loop Supply Side Bypass Branch,  !- Outlet Branch 1 Name",
        "    DH DH loop Supply Side Branch;  !- Outlet Branch 2 Name",

        "  Branch,",
        "    DH loop Demand Side Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    DH loop Demand Side Inlet Branch Pipe,  !- Component 1 Name",
        "    DH loop Demand Side Inlet,  !- Component 1 Inlet Node Name",
        "    DH loop Demand Side Inlet Branch Pipe Outlet;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    DH loop Demand Side Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    DH loop Demand Side Outlet Branch Pipe,  !- Component 1 Name",
        "    DH loop Demand Side Outlet Branch Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    DH loop Demand Side Outlet;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    DH loop Supply Side Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    DH loop Supply Pump 1,   !- Component 1 Name",
        "    DH loop Supply Side Inlet,  !- Component 1 Inlet Node Name",
        "    DH loop Supply Pump 1 Water Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    DH loop Supply Side Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    DH loop Supply Side Outlet Branch Pipe,  !- Component 1 Name",
        "    DH loop Supply Side Outlet Branch Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    DH loop Supply Side Outlet;  !- Component 1 Outlet Node Name",

        "  Connector:Mixer,",
        "    DH loop Demand Mixer 1,  !- Name",
        "    DH loop Demand Side Outlet Branch,  !- Outlet Branch Name",
        "    chiller demand side branch,  !- Inlet Branch 1 Name",
        "    DH loop Demand Side Bypass Branch;  !- Inlet Branch 2 Name",

        "  Connector:Mixer,",
        "    DH loop Supply Mixer 1,  !- Name",
        "    DH loop Supply Side Outlet Branch,  !- Outlet Branch Name",
        "    DH DH loop Supply Side Branch,  !- Inlet Branch 1 Name",
        "    DH loop Supply Side Bypass Branch;  !- Inlet Branch 2 Name",

        "  Branch,",
        "    chiller demand side branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Chiller:absorption,      !- Component 1 Object Type",
        "    Big Chiller,             !- Component 1 Name",
        "    AbsorberHWInletNode,     !- Component 1 Inlet Node Name",
        "    AbsorberHWOutletNode;    !- Component 1 Outlet Node Name",

        "  Branch,",
        "    DH loop Demand Side Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    DH loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    DH loop Demand Side Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    DH loop Demand Side Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    DH loop Supply Side Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    DH loop Supply Side Bypass Pipe,  !- Component 1 Name",
        "    DH loop Supply Side Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    DH loop Supply Side Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    DH DH loop Supply Side Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Boiler:HotWater,         !- Component 1 Object Type",
        "    DH,                      !- Component 1 Name",
        "    DH Water Inlet Node,     !- Component 1 Inlet Node Name",
        "    DH Water Outlet Node;    !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    DH loop Demand Side Inlet Branch Pipe,  !- Name",
        "    DH loop Demand Side Inlet,  !- Inlet Node Name",
        "    DH loop Demand Side Inlet Branch Pipe Outlet;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    DH loop Demand Side Bypass Pipe,  !- Name",
        "    DH loop Demand Side Bypass Pipe Inlet Node,  !- Inlet Node Name",
        "    DH loop Demand Side Bypass Pipe Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    DH loop Demand Side Outlet Branch Pipe,  !- Name",
        "    DH loop Demand Side Outlet Branch Pipe Inlet,  !- Inlet Node Name",
        "    DH loop Demand Side Outlet;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    DH loop Supply Side Bypass Pipe,  !- Name",
        "    DH loop Supply Side Bypass Pipe Inlet Node,  !- Inlet Node Name",
        "    DH loop Supply Side Bypass Pipe Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    DH loop Supply Side Outlet Branch Pipe,  !- Name",
        "    DH loop Supply Side Outlet Branch Pipe Inlet,  !- Inlet Node Name",
        "    DH loop Supply Side Outlet;  !- Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    DH loop Supply Pump 1,   !- Name",
        "    DH loop Supply Side Inlet,  !- Inlet Node Name",
        "    DH loop Supply Pump 1 Water Outlet Node,  !- Outlet Node Name",
        "    0.012,                   !- Design Maximum Flow Rate {m3/s}",
        "    244435.00,               !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.90,                    !- Motor Efficiency",
        "    0.00,                    !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0.0000,                  !- Coefficient 1 of the Part Load Performance Curve",
        "    1.0000,                  !- Coefficient 2 of the Part Load Performance Curve",
        "    0.0000,                  !- Coefficient 3 of the Part Load Performance Curve",
        "    0.0000,                  !- Coefficient 4 of the Part Load Performance Curve",
        "    0.000000,                !- Design Minimum Flow Rate {m3/s}",
        "    Intermittent;            !- Pump Control Type",

        "  Boiler:HotWater,",
        "    DH,                      !- Name",
        "    OtherFuel1,              !- Fuel Type",
        "    270000.00,               !- Nominal Capacity {W}",
        "    0.94,                    !- Nominal Thermal Efficiency",
        "    LeavingBoiler,           !- Efficiency Curve Temperature Evaluation Variable",
        "    100% efficient,          !- Normalized Boiler Efficiency Curve Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    0.00,                    !- Minimum Part Load Ratio",
        "    1.00,                    !- Maximum Part Load Ratio",
        "    1.00,                    !- Optimum Part Load Ratio",
        "    DH Water Inlet Node,     !- Boiler Water Inlet Node Name",
        "    DH Water Outlet Node,    !- Boiler Water Outlet Node Name",
        "    100.0,                   !- Water Outlet Upper Temperature Limit {C}",
        "    NotModulated,            !- Boiler Flow Mode",
        "    4770.00,                 !- On Cycle Parasitic Electric Load {W}",
        "    1.00;                    !- Sizing Factor",

        "  Curve:Linear,",
        "    100% efficient,          !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "  PlantEquipmentOperationSchemes,",
        "    DH loop Operation,       !- Name",
        "    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "    DH loop Scheme 1,        !- Control Scheme 1 Name",
        "    On 24/7;                 !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:HeatingLoad,",
        "    DH loop Scheme 1,        !- Name",
        "    0.00,                    !- Load Range 1 Lower Limit {W}",
        "    1000000000000000.00,     !- Load Range 1 Upper Limit {W}",
        "    DH loop Scheme 1 Range 1 Equipment List;  !- Range 1 Equipment List Name",

        "  Schedule:Compact,",
        "    On 24/7,                 !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 3",

        "  PlantEquipmentList,",
        "    DH loop Scheme 1 Range 1 Equipment List,  !- Name",
        "    Boiler:HotWater,         !- Equipment 1 Object Type",
        "    DH;                      !- Equipment 1 Name",

        "  SetpointManager:Scheduled,",
        "    SM DH,                   !- Name",
        "    Temperature,             !- Control Variable",
        "    75C,                     !- Schedule Name",
        "    DH Water Outlet Node;    !- Setpoint Node or NodeList Name",

        "  SetpointManager:Scheduled,",
        "    SM DH 2,                 !- Name",
        "    Temperature,             !- Control Variable",
        "    75C,                     !- Schedule Name",
        "    DH loop Supply Side Outlet;  !- Setpoint Node or NodeList Name",

        "  Schedule:Compact,",
        "    75C,                     !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,75;         !- Field 3",

        "  AvailabilityManagerAssignmentList,",
        "    DH loop AvailabilityManager List,  !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    DH loop Availability;    !- Availability Manager 1 Name",

        "  AvailabilityManager:Scheduled,",
        "    DH loop Availability,    !- Name",
        "    On 24/7;                 !- Schedule Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    SimulationManager::ManageSimulation(*state); // run the design day

    // set conditions for test
    int AbsChillNum = 1;
    DataBranchAirLoopPlant::ControlType EquipFlowCtrl = DataBranchAirLoopPlant::ControlType::Active;
    Real64 AbsChillEvapLoad;
    bool AbsChillRunFlag = true;
    // check chiller inputs
    auto &thisChiller = state->dataChillerAbsorber->absorptionChillers(AbsChillNum);
    EXPECT_EQ(thisChiller.NomCap, 100000.0);
    EXPECT_ENUM_EQ(thisChiller.FlowMode, DataPlant::FlowMode::LeavingSetpointModulated);
    // define local var
    int EvapInletNode = thisChiller.EvapInletNodeNum;
    int EvapOutletNode = thisChiller.EvapOutletNodeNum;
    int GeneratorInletNode = thisChiller.GeneratorInletNodeNum;
    int GeneratorOutletNode = thisChiller.GeneratorOutletNodeNum;
    // set conditions for unit test
    state->dataLoopNodes->Node(EvapInletNode).MassFlowRate = 1.0;
    state->dataLoopNodes->Node(EvapInletNode).Temp = 15.0;
    state->dataLoopNodes->Node(EvapOutletNode).TempSetPoint = 5.0;
    thisChiller.MaxPartLoadRat = 1.0;
    thisChiller.EvapMassFlowRateMax = 2.0;
    thisChiller.GenMassFlowRateMax = 5.0;
    state->dataLoopNodes->Node(GeneratorInletNode).MassFlowRate = thisChiller.GenMassFlowRateMax;
    state->dataLoopNodes->Node(GeneratorInletNode).MassFlowRateMax = thisChiller.GenMassFlowRateMax;
    state->dataLoopNodes->Node(GeneratorInletNode).MassFlowRateMaxAvail = thisChiller.GenMassFlowRateMax;
    // calc evap load
    Real64 CpFluid = 4179.0;
    Real64 EvapMassFlowRate = state->dataLoopNodes->Node(EvapInletNode).MassFlowRate;
    AbsChillEvapLoad =
        EvapMassFlowRate * CpFluid * (state->dataLoopNodes->Node(EvapOutletNode).TempSetPoint - state->dataLoopNodes->Node(EvapInletNode).Temp);
    thisChiller.NomCap = 2.0 * abs(AbsChillEvapLoad);
    // generator hot water mass flow rate is calculated proportional to evap mass flow rate for flow mode = LeavingSetPointModulated
    Real64 GenMassFlowRateTestResult =
        (state->dataLoopNodes->Node(EvapInletNode).MassFlowRate / thisChiller.EvapMassFlowRateMax) * thisChiller.GenMassFlowRateMax;
    // lock the evap flow at test condition specified
    int LoopNum = thisChiller.CWPlantLoc.loopNum;
    DataPlant::LoopSideLocation LoopSideNum = thisChiller.CWPlantLoc.loopSideNum;
    state->dataPlnt->PlantLoop(LoopNum).LoopSide(LoopSideNum).FlowLock = DataPlant::FlowLock::Locked;
    // calc generator flow
    int GenLoopNum = thisChiller.GenPlantLoc.loopNum;
    DataPlant::LoopSideLocation GenLoopSideNum = thisChiller.GenPlantLoc.loopSideNum;
    state->dataPlnt->PlantLoop(GenLoopNum).LoopSide(GenLoopSideNum).FlowLock = DataPlant::FlowLock::Unlocked;
    // run CalcBLASTAbsorberModel
    thisChiller.EquipFlowCtrl = EquipFlowCtrl;
    thisChiller.calculate(*state, AbsChillEvapLoad, AbsChillRunFlag);
    // check generator hot water mass flow rate is proportional to the chilled water flow rate
    EXPECT_EQ(state->dataLoopNodes->Node(GeneratorInletNode).MassFlowRate, GenMassFlowRateTestResult);
    EXPECT_EQ(state->dataLoopNodes->Node(GeneratorOutletNode).MassFlowRate, GenMassFlowRateTestResult);
}

TEST_F(EnergyPlusFixture, ChillerAbsorption_Autosize)
{
    state->dataPlnt->TotNumLoops = 3;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 15;
    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataGlobal->SysSizingCalc = true;

    std::string const idf_objects = delimited_string({
        "Chiller:Absorption,",
        "  Chiller,                 !- Name",
        "  Autosize,                !- Nominal Capacity {W}",
        "  Autosize,                !- Nominal Pumping Power {W}",
        "  Chiller ChW Inlet,       !- Chilled Water Inlet Node Name",
        "  Chiller ChW Outlet,      !- Chilled Water Outlet Node Name",
        "  Chiller Cnd Outlet,      !- Condenser Inlet Node Name",
        "  Chiller Cnd Outlet,      !- Condenser Outlet Node Name",
        "  0.15,                    !- Minimum Part Load Ratio",
        "  1.0,                     !- Maximum Part Load Ratio",
        "  0.65,                    !- Optimum Part Load Ratio",
        "  35.0,                    !- Design Condenser Inlet Temperature {C}",
        "  Autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "  Autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "  0.0,                     !- Coefficient 1 of the Hot Water or Steam Use Part Load Ratio Curve",
        "  0.8,                     !- Coefficient 2 of the Hot Water or Steam Use Part Load Ratio Curve",
        "  0.2,                     !- Coefficient 3 of the Hot Water or Steam Use Part Load Ratio Curve",
        "  0.9,                     !- Coefficient 1 of the Pump Electric Use Part Load Ratio Curve",
        "  0.1,                     !- Coefficient 2 of the Pump Electric Use Part Load Ratio Curve",
        "  0.0,                     !- Coefficient 3 of the Pump Electric Use Part Load Ratio Curve",
        "  5,                       !- Chilled Water Outlet Temperature Lower Limit {C}",
        "  AbsorberHWInletNode,     !- Generator Inlet Node Name",
        "  AbsorberHWOutletNode,    !- Generator Outlet Node Name",
        "  ConstantFlow,            !- Chiller Flow Mode",
        "  HotWater,                !- Generator Heat Source Type",
        "  Autosize,                !- Design Generator Fluid Flow Rate {m3/s}",
        "  2.0,                     !- Degree of Subcooling in Steam Generator",
        "  1.5;                     !- Sizing Factor",
    });
    EXPECT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->NumPltSizInput = state->dataPlnt->TotNumLoops;
    state->dataSize->PlantSizData.allocate(state->dataPlnt->TotNumLoops);

    constexpr int chwLoopNum = 1;
    auto &chwLoop = state->dataPlnt->PlantLoop(1);
    auto &chwLoopSizing = state->dataSize->PlantSizData(1);

    constexpr int cndLoopNum = 2;
    auto &cndLoop = state->dataPlnt->PlantLoop(cndLoopNum);
    auto &cndLoopSizing = state->dataSize->PlantSizData(cndLoopNum);

    constexpr int genLoopNum = 3;
    auto &genLoop = state->dataPlnt->PlantLoop(genLoopNum);
    auto &genLoopSizing = state->dataSize->PlantSizData(genLoopNum);

    ChillerAbsorption::GetBLASTAbsorberInput(*state);
    ASSERT_EQ(1, state->dataChillerAbsorber->absorptionChillers.size());
    auto &thisChiller = state->dataChillerAbsorber->absorptionChillers(1);

    EXPECT_EQ(thisChiller.NomCap, DataSizing::AutoSize);
    EXPECT_EQ(thisChiller.NomPumpPower, DataSizing::AutoSize);
    EXPECT_EQ(thisChiller.EvapVolFlowRate, DataSizing::AutoSize);
    EXPECT_EQ(thisChiller.CondVolFlowRate, DataSizing::AutoSize);
    EXPECT_EQ(thisChiller.GeneratorVolFlowRate, DataSizing::AutoSize);

    EXPECT_TRUE(thisChiller.NomCapWasAutoSized);
    EXPECT_TRUE(thisChiller.NomPumpPowerWasAutoSized);
    EXPECT_GT(thisChiller.EvapInletNodeNum, 0);
    EXPECT_GT(thisChiller.EvapOutletNodeNum, 0);
    EXPECT_GT(thisChiller.CondInletNodeNum, 0);
    EXPECT_GT(thisChiller.CondOutletNodeNum, 0);

    EXPECT_DOUBLE_EQ(0.15, thisChiller.MinPartLoadRat);
    EXPECT_DOUBLE_EQ(1.0, thisChiller.MaxPartLoadRat);
    EXPECT_DOUBLE_EQ(0.65, thisChiller.OptPartLoadRat);
    EXPECT_DOUBLE_EQ(35.0, thisChiller.TempDesCondIn);

    EXPECT_TRUE(thisChiller.EvapVolFlowRateWasAutoSized);
    EXPECT_TRUE(thisChiller.CondVolFlowRateWasAutoSized);

    EXPECT_DOUBLE_EQ(0.0, thisChiller.SteamLoadCoef[0]);
    EXPECT_DOUBLE_EQ(0.8, thisChiller.SteamLoadCoef[1]);
    EXPECT_DOUBLE_EQ(0.2, thisChiller.SteamLoadCoef[2]);

    EXPECT_DOUBLE_EQ(0.9, thisChiller.PumpPowerCoef[0]);
    EXPECT_DOUBLE_EQ(0.1, thisChiller.PumpPowerCoef[1]);
    EXPECT_DOUBLE_EQ(0.0, thisChiller.PumpPowerCoef[2]);

    EXPECT_DOUBLE_EQ(5.0, thisChiller.TempLowLimitEvapOut);

    EXPECT_ENUM_EQ(thisChiller.FlowMode, DataPlant::FlowMode::Constant);
    EXPECT_TRUE(thisChiller.GenInputOutputNodesUsed);
    EXPECT_GT(thisChiller.GeneratorInletNodeNum, 0);
    EXPECT_GT(thisChiller.GeneratorOutletNodeNum, 0);
    EXPECT_ENUM_EQ(thisChiller.GenHeatSourceType, DataLoopNode::NodeFluidType::Water);
    EXPECT_TRUE(thisChiller.GeneratorVolFlowRateWasAutoSized);

    EXPECT_DOUBLE_EQ(2.0, thisChiller.GeneratorSubcool);
    EXPECT_DOUBLE_EQ(1.5, thisChiller.SizFac);

    {
        auto &thisLoop = chwLoop;
        auto &thisLoopSizing = chwLoopSizing;
        thisLoop.Name = "ChilledWaterLoop";
        thisLoop.FluidName = "WATER";
        thisLoop.glycol = Fluid::GetWater(*state);
        thisLoop.PlantSizNum = chwLoopNum;
        auto &loopside = thisLoop.LoopSide(DataPlant::LoopSideLocation::Supply);
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);

        auto &branch = loopside.Branch(1);
        branch.TotalComponents = 1;
        branch.Comp.allocate(1);
        branch.Comp(1).Name = thisChiller.Name;
        branch.Comp(1).Type = DataPlant::PlantEquipmentType::Chiller_Absorption;
        branch.Comp(1).NodeNumIn = thisChiller.EvapInletNodeNum;
        branch.Comp(1).NodeNumOut = thisChiller.EvapOutletNodeNum;

        thisLoopSizing.PlantLoopName = thisLoop.Name;
        thisLoopSizing.LoopType = DataSizing::TypeOfPlantLoop::Cooling;
        thisLoopSizing.ExitTemp = 6.67;
        thisLoopSizing.DesVolFlowRate = 0.01;
        thisLoopSizing.DeltaT = 5.0;
    }

    {
        auto &thisLoop = cndLoop;
        auto &thisLoopSizing = cndLoopSizing;
        thisLoop.Name = "CondenserWaterLoop";
        thisLoop.FluidName = "WATER";
        thisLoop.glycol = Fluid::GetWater(*state);
        thisLoop.PlantSizNum = cndLoopNum;
        auto &loopside = thisLoop.LoopSide(DataPlant::LoopSideLocation::Demand);
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);

        auto &branch = loopside.Branch(1);
        branch.TotalComponents = 1;
        branch.Comp.allocate(1);
        branch.Comp(1).Name = thisChiller.Name;
        branch.Comp(1).Type = DataPlant::PlantEquipmentType::Chiller_Absorption;
        branch.Comp(1).NodeNumIn = thisChiller.CondInletNodeNum;
        branch.Comp(1).NodeNumOut = thisChiller.CondOutletNodeNum;

        thisLoopSizing.PlantLoopName = thisLoop.Name;
        thisLoopSizing.LoopType = DataSizing::TypeOfPlantLoop::Condenser;
        thisLoopSizing.ExitTemp = 29.4;
        thisLoopSizing.DesVolFlowRate = 0.01;
        thisLoopSizing.DeltaT = 5.0;
    }

    {
        auto &thisLoop = genLoop;
        auto &thisLoopSizing = genLoopSizing;
        thisLoop.Name = "Generator HW Loop";
        thisLoop.FluidName = "WATER";
        thisLoop.glycol = Fluid::GetWater(*state);
        thisLoop.PlantSizNum = genLoopNum;
        auto &loopside = thisLoop.LoopSide(DataPlant::LoopSideLocation::Demand);
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);

        auto &branch = loopside.Branch(1);
        branch.TotalComponents = 1;
        branch.Comp.allocate(1);
        branch.Comp(1).Name = thisChiller.Name;
        branch.Comp(1).Type = DataPlant::PlantEquipmentType::Chiller_Absorption;
        branch.Comp(1).NodeNumIn = thisChiller.GeneratorInletNodeNum;
        branch.Comp(1).NodeNumOut = thisChiller.GeneratorOutletNodeNum;

        thisLoopSizing.PlantLoopName = thisLoop.Name;
        thisLoopSizing.LoopType = DataSizing::TypeOfPlantLoop::Heating;
        thisLoopSizing.ExitTemp = 80.0;
        thisLoopSizing.DesVolFlowRate = 0.01;
        thisLoopSizing.DeltaT = 20.0;
    }

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    // Calculate expected values
    Real64 rho_cw = state->dataPlnt->PlantLoop(chwLoopNum).glycol->getDensity(*state, Constant::CWInitConvTemp, "ChillerAbsorption_Autosize_TEST");

    Real64 Cp_evap =
        state->dataPlnt->PlantLoop(chwLoopNum).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, "ChillerAbsorption_Autosize_TEST");

    Real64 const expectedEvapVolFlowRate = state->dataSize->PlantSizData(chwLoopNum).DesVolFlowRate * thisChiller.SizFac;
    Real64 const expectedNomCap = Cp_evap * rho_cw * state->dataSize->PlantSizData(chwLoopNum).DeltaT * expectedEvapVolFlowRate;

    Real64 constexpr nomCapToPumpRatio = 0.0045;

    Real64 const SteamInputRatNom = thisChiller.SteamLoadCoef[0] + thisChiller.SteamLoadCoef[1] + thisChiller.SteamLoadCoef[2];
    EXPECT_DOUBLE_EQ(1.0, SteamInputRatNom);

    Real64 rho_cond = state->dataPlnt->PlantLoop(cndLoopNum).glycol->getDensity(*state, Constant::CWInitConvTemp, "ChillerAbsorption_Autosize_TEST");

    Real64 Cp_cond =
        state->dataPlnt->PlantLoop(cndLoopNum).glycol->getSpecificHeat(*state, thisChiller.TempDesCondIn, "ChillerAbsorption_Autosize_TEST");

    Real64 const expectedCondVolFlowRate =
        expectedNomCap * (1.0 + SteamInputRatNom + nomCapToPumpRatio) / (rho_cond * Cp_cond * state->dataSize->PlantSizData(cndLoopNum).DeltaT);

    Real64 const SteamDeltaT = state->dataSize->PlantSizData(genLoopNum).DeltaT;
    Real64 const Cp_gen = state->dataPlnt->PlantLoop(genLoopNum)
                              .glycol->getSpecificHeat(*state, state->dataSize->PlantSizData(genLoopNum).ExitTemp, "ChillerAbsorption_Autosize_TEST");

    Real64 const rho_gen =
        state->dataPlnt->PlantLoop(genLoopNum)
            .glycol->getDensity(*state, (state->dataSize->PlantSizData(genLoopNum).ExitTemp - SteamDeltaT), "ChillerAbsorption_Autos");
    Real64 const expectedGeneratorVolFlowRate = (expectedNomCap * SteamInputRatNom) / (Cp_gen * rho_gen * SteamDeltaT);

    bool RunFlag(true);
    Real64 MyLoad(-20000.0);

    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.sizeChiller(*state);

    EXPECT_NE(thisChiller.NomCap, DataSizing::AutoSize);
    EXPECT_NE(thisChiller.NomPumpPower, DataSizing::AutoSize);
    EXPECT_NE(thisChiller.EvapVolFlowRate, DataSizing::AutoSize);
    EXPECT_NE(thisChiller.CondVolFlowRate, DataSizing::AutoSize);
    EXPECT_NE(thisChiller.GeneratorVolFlowRate, DataSizing::AutoSize);

    EXPECT_GT(thisChiller.NomCap, 0.0);
    EXPECT_GT(thisChiller.NomPumpPower, 0.0);
    EXPECT_GT(thisChiller.EvapVolFlowRate, 0.0);
    EXPECT_GT(thisChiller.CondVolFlowRate, 0.0);
    EXPECT_GT(thisChiller.GeneratorVolFlowRate, 0.0);

    EXPECT_DOUBLE_EQ(expectedNomCap, thisChiller.NomCap);
    EXPECT_DOUBLE_EQ(expectedNomCap * nomCapToPumpRatio, thisChiller.NomPumpPower);
    EXPECT_DOUBLE_EQ(expectedEvapVolFlowRate, thisChiller.EvapVolFlowRate);
    EXPECT_DOUBLE_EQ(expectedCondVolFlowRate, thisChiller.CondVolFlowRate);
    EXPECT_DOUBLE_EQ(expectedGeneratorVolFlowRate, thisChiller.GeneratorVolFlowRate);
}
