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

// EnergyPlus::DataHeatBalance unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataRuntimeLanguage;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::EMSManager;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::SimulationManager;
using namespace EnergyPlus::SurfaceGeometry;

// using DataVectorTypes::Vector;

TEST_F(EnergyPlusFixture, DataHeatBalance_CheckConstructLayers)
{
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Building,",
        "    NONE,                    !- Name",
        "    0.0000000E+00,           !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    0.4000000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
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

        "  RunPeriod,",
        "    RP1,                     !- Name",
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
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
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
        "    SINGLEPANE,              !- Name",
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

        "  WindowMaterial:Blind,",
        "    BLIND,                   !- Name",
        "    HORIZONTAL,              !- Slat Orientation",
        "    0.025,                   !- Slat Width {m}",
        "    0.01875,                 !- Slat Separation {m}",
        "    0.001,                   !- Slat Thickness {m}",
        "    45.0,                    !- Slat Angle {deg}",
        "    0.1,                     !- Slat Conductivity {W/m-K}",
        "    0.0,                     !- Slat Beam Solar Transmittance",
        "    0.7,                     !- Front Side Slat Beam Solar Reflectance",
        "    0.7,                     !- Back Side Slat Beam Solar Reflectance",
        "    0.0,                     !- Slat Diffuse Solar Transmittance",
        "    0.7,                     !- Front Side Slat Diffuse Solar Reflectance",
        "    0.7,                     !- Back Side Slat Diffuse Solar Reflectance",
        "    0.0,                     !- Slat Beam Visible Transmittance",
        "    0.5,                     !- Front Side Slat Beam Visible Reflectance",
        "    0.5,                     !- Back Side Slat Beam Visible Reflectance",
        "    0.0,                     !- Slat Diffuse Visible Transmittance",
        "    0.5,                     !- Front Side Slat Diffuse Visible Reflectance",
        "    0.5,                     !- Back Side Slat Diffuse Visible Reflectance",
        "    0.0,                     !- Slat Infrared Hemispherical Transmittance",
        "    0.9,                     !- Front Side Slat Infrared Hemispherical Emissivity",
        "    0.9,                     !- Back Side Slat Infrared Hemispherical Emissivity",
        "    0.050,                   !- Blind to Glass Distance {m}",
        "    0.5,                     !- Blind Top Opening Multiplier",
        "    0.5,                     !- Blind Bottom Opening Multiplier",
        "    0.0,                     !- Blind Left Side Opening Multiplier",
        "    0.0,                     !- Blind Right Side Opening Multiplier",
        "    0,                       !- Minimum Slat Angle {deg}",
        "    180;                     !- Maximum Slat Angle {deg}",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    C4 - 4 IN COMMON BRICK;  !- Layer 2",

        "  Construction,",
        "    FLOOR SLAB 8 IN,         !- Name",
        "    C10 - 8 IN HW CONCRETE;  !- Outside Layer",

        "  Construction,",
        "    ROOF34,                  !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",

        "  Construction,",
        "    WIN-CON-DOUBLEPANE,      !- Name",
        "    SINGLEPANE,              !- Outside Layer",
        "    WinGas,                  !- Layer 2",
        "    SINGLEPANE;              !- Layer 3",

        "  WindowMaterial:Gas,",
        "    WinGas,                  !- Name",
        "    Air,                     !- Gas Type",
        "    0.013;                   !- Thickness {m}",

        "  WindowShadingControl,",
        "    INCIDENT SOLAR ON BLIND, !- Name",
        "    West Zone,               !- Zone Name",
        "    1,                       !- Shading Control Sequence Number ",
        "    SwitchableGlazing,       !- Shading Type",
        "    WIN-CON-DOUBLEPANE,      !- Construction with Shading Name",
        "    OnIfHighSolarOnWindow,   !- Shading Control Type",
        "    ,                        !- Schedule Name",
        "    20,                      !- Setpoint {W/m2, W or deg C}",
        "    No,                      !- Shading Control Is Scheduled",
        "    No,                      !- Glare Control Is Active",
        "    ,                        !- Shading Device Material Name",
        "    FixedSlatAngle,          !- Type of Slat Angle Control for Blinds",
        "    ,                        !- Slat Angle Schedule Name",
        "    ,                        !- Setpoint 2",
        "    ,                        !- Daylighting Control Object Name",
        "    ,                        !- Multiple Surface Control Type",
        "    Zn001:Wall001:Win001;    !- Fenestration Surface 1 Name",

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

        "  Schedule:Compact,",
        "    Activity Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,131.80;     !- Field 3",

        "  Schedule:Compact,",
        "    Work Eff Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,0.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Clothing Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Air Velo Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,0.137;      !- Field 3",

        "  Schedule:Compact,",
        "    Office Occupancy,        !- Name",
        "    ANY NUMBER,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays SummerDesignDay WinterDesignDay, !- Field 2",
        "    Until: 6:00,0.00,        !- Field 3",
        "    Until: 7:00,0.10,        !- Field 5",
        "    Until: 8:00,0.50,        !- Field 7",
        "    Until: 12:00,1.00,       !- Field 9",
        "    Until: 13:00,0.50,       !- Field 11",
        "    Until: 16:00,1.00,       !- Field 13",
        "    Until: 17:00,0.50,       !- Field 15",
        "    Until: 18:00,0.10,       !- Field 17",
        "    Until: 24:00,0.00,       !- Field 19",
        "    For: Weekends Holidays CustomDay1 CustomDay2, !- Field 21",
        "    Until: 24:00,0.00;       !- Field 22",

        "  Schedule:Compact,",
        "    Intermittent,            !- Name",
        "    ANY NUMBER,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays SummerDesignDay WinterDesignDay, !- Field 2",
        "    Until: 8:00,0.00,        !- Field 3",
        "    Until: 18:00,1.00,       !- Field 5",
        "    Until: 24:00,0.00,       !- Field 7",
        "    For: Weekends Holidays CustomDay1 CustomDay2, !- Field 9",
        "    Until: 24:00,0.00;       !- Field 10",

        "  Schedule:Compact,",
        "    Office Lighting,         !- Name",
        "    ANY NUMBER,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays SummerDesignDay WinterDesignDay, !- Field 2",
        "    Until: 6:00,5.00E-002,   !- Field 3",
        "    Until: 7:00,0.20,        !- Field 5",
        "    Until: 17:00,1.00,       !- Field 7",
        "    Until: 18:00,0.50,       !- Field 9",
        "    Until: 24:00,5.00E-002,  !- Field 11",
        "    For: Weekends Holidays CustomDay1 CustomDay2, !- Field 13",
        "    Until: 24:00,5.00E-002;  !- Field 14",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00;      !- Field 7",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 20:00,23.00,      !- Field 5",
        "    Until: 24:00,30.00;      !- Field 7",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    CONTROL TYPE,            !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1,          !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: Alldays,            !- Field 6",
        "    Until: 24:00,2,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,1;          !- Field 11",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0.0000000E+00,           !- Direction of Relative North {deg}",
        "    0.0000000E+00,           !- X Origin {m}",
        "    0.0000000E+00,           !- Y Origin {m}",
        "    0.0000000E+00,           !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  People,",
        "    West Zone,               !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    Office Occupancy,        !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    3.000000,                !- Number of People",
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
        "    Intermittent,            !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    2928.751,                !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000000E+00,           !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0.0000000E+00;           !- Fraction Lost",

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
        "    0.00000,0.000000,3.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.00000,0.000000,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,0.000000,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,0.000000,3.0000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    WIN-CON-DOUBLEPANE,      !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.54800,0.00000,2.5000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.54800,0.00000,0.5000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.54800,0.00000,0.5000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.54800,0.00000,2.5000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Glass Conductance",
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
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-Glass Conductance",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",

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
        "    0.0000000E+00,10.0000,3.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0000000E+00,10.0000,0.0000000E+00,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0000000E+00,0.0000000E+00,0.0000000E+00,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0000000E+00,0.0000000E+00,3.0000;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    10.0000,10.0000,3.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.0000,10.0000,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.00000,10.0000,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.00000,10.0000,3.0000;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    10.0000,0.00000,3.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.0000,0.00000,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,10.0000,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,10.0000,3.0000;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    0.00000,0.00000,0.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.00000,10.0000,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,10.0000,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,0.00000,0.0000;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    0.0000000E+00,           !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.00000,10.0000,3.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.00000,0.00000,3.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.0000,0.00000,3.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.0000,10.0000,3.0000;  !- X,Y,Z ==> Vertex 4 {m}",

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

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    NODE_4,                  !- Zone Air Node Name",
        "    NODE_5;                  !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type",
        "    Zone1Air,                !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:IdealLoadsAirSystem,",
        "    Zone1Air,                !- Name",
        "    ,                        !- Availability Schedule Name",
        "    NODE_1,                  !- Zone Supply Air Node Name",
        "    ,                        !- Zone Exhaust Air Node Name",
        "    ,                        !- System Inlet Air Node Name",
        "    50,                      !- Maximum Heating Supply Air Temperature {C}",
        "    13,                      !- Minimum Cooling Supply Air Temperature {C}",
        "    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    NoLimit,                 !- Heating Limit",
        "    autosize,                !- Maximum Heating Air Flow Rate {m3/s}",
        "    ,                        !- Maximum Sensible Heating Capacity {W}",
        "    NoLimit,                 !- Cooling Limit",
        "    autosize,                !- Maximum Cooling Air Flow Rate {m3/s}",
        "    ,                        !- Maximum Total Cooling Capacity {W}",
        "    ,                        !- Heating Availability Schedule Name",
        "    ,                        !- Cooling Availability Schedule Name",
        "    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type",
        "    ,                        !- Cooling Sensible Heat Ratio {dimensionless}",
        "    ConstantSupplyHumidityRatio,  !- Humidification Control Type",
        "    ,                        !- Design Specification Outdoor Air Object Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    ,                        !- Demand Controlled Ventilation Type",
        "    ,                        !- Outdoor Air Economizer Type",
        "    ,                        !- Heat Recovery Type",
        "    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}",
        "    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    NODE_1;                  !- Node 1 Name",

        "  Output:EnergyManagementSystem,",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level",

        "  EnergyManagementSystem:Sensor,",
        "    Solar_Beam_Incident_Cos, !- Name",
        "    Zn001:Wall001:Win001,    !- Output:Variable or Output:Meter Index Key Name",
        "    Surface Outside Face Beam Solar Incident Angle Cosine Value;  !- Output:Variable or Output:Meter Name",

        "  Output:Variable,Zn001:Wall001:Win001,Surface Outside Face Beam Solar Incident Angle Cosine Value,Timestep;",

        "  EnergyManagementSystem:Sensor,",
        "    Zone_Sensible_Cool_Rate, !- Name",
        "    WEST ZONE,               !- Output:Variable or Output:Meter Index Key Name",
        "    Zone Air System Sensible Cooling Rate;  !- Output:Variable or Output:Meter Name",

        "  EnergyManagementSystem:ProgramCallingManager,",
        "    Window Shading Device EMS Controller,  !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    Set_Shade_Control_State; !- Program Name 1",

        "  EnergyManagementSystem:Actuator,",
        "    Zn001_Wall001_Win001_Shading_Deploy_Status,  !- Name",
        "    Zn001:Wall001:Win001,    !- Actuated Component Unique Name",
        "    Window Shading Control,  !- Actuated Component Type",
        "    Control Status;          !- Actuated Component Control Type",

        "  EnergyManagementSystem:Program,",
        "    Set_Shade_Control_State, !- Name",
        "    Set IncidentAngleRad = @ArcCos Solar_Beam_Incident_Cos,  !- Program Line 1",
        "    Set IncidentAngle   = @RadToDeg IncidentAngleRad,  !- Program Line 2",
        "    IF IncidentAngle < 45,   !- <none>",
        "    Set Zn001_Wall001_Win001_Shading_Deploy_Status = Shade_Status_Interior_Blind_On,  !- <none>",
        "    ELSEIF Zone_Sensible_Cool_Rate > 20,  !- <none>",
        "    Set Zn001_Wall001_Win001_Shading_Deploy_Status = Shade_Status_Interior_Blind_On,  !- <none>",
        "    Else,                    !- <none>",
        "    Set Zn001_Wall001_Win001_Shading_Deploy_Status = Shade_Status_Off,  !- <none>",
        "    ENDIF;                   !- <none>",

        "  EnergyManagementSystem:OutputVariable,",
        "    Erl Shading Control Status,  !- Name",
        "    Zn001_Wall001_Win001_Shading_Deploy_Status,  !- EMS Variable Name",
        "    Averaged,                !- Type of Data in Variable",
        "    ZoneTimeStep,            !- Update Frequency",
        "    ,                        !- EMS Program or Subroutine Name",
        "    ;                        !- Units",

        "  EnergyManagementSystem:OutputVariable,",
        "    Erl Zn001:Wall001:Win001 Incident Angle,  !- Name",
        "    IncidentAngle,           !- EMS Variable Name",
        "    Averaged,                !- Type of Data in Variable",
        "    ZoneTimeStep,            !- Update Frequency",
        "    ,                        !- EMS Program or Subroutine Name",
        "    deg;                     !- Units",

        "  EnergyManagementSystem:GlobalVariable,",
        "    IncidentAngle;           !- Erl Variable 1 Name",

        "  Output:Variable,*,Erl Shading Control Status,Timestep;",

        "  Output:Variable,*,Erl Zn001:Wall001:Win001 Incident Angle,Timestep;",

        "  EnergyManagementSystem:ProgramCallingManager,",
        "    Init Window Shading Device Control Constants,  !- Name",
        "    BeginNewEnvironment,     !- EnergyPlus Model Calling Point",
        "    InitializeShadeControlFlags;  !- Program Name 1",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_None;       !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Off;        !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Interior_Shade_On;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Switchable_Dark;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Exterior_Shade_On;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Interior_Blind_On;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Exterior_Blind_On;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Between_Glass_Shade_On;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:GlobalVariable,",
        "    Shade_Status_Between_Glass_Blind_On;  !- Erl Variable 1 Name",

        "  EnergyManagementSystem:Program,",
        "    InitializeShadeControlFlags,  !- Name",
        "    Set Shade_Status_None = 0.0 - 1.0,  !- Program Line 1",
        "    Set Shade_Status_Off = 0.0,  !- Program Line 2",
        "    Set Shade_Status_Interior_Shade_On = 1.0,  !- <none>",
        "    Set Shade_Status_Switchable_Dark = 2.0,  !- <none>",
        "    Set Shade_Status_Exterior_Shade_On = 3.0,  !- <none>",
        "    Set Shade_Status_Interior_Blind_On = 6.0,  !- <none>",
        "    Set Shade_Status_Exterior_Blind_On = 7.0,  !- <none>",
        "    Set Shade_Status_Between_Glass_Shade_On = 8.0,  !- <none>",
        "    Set Shade_Status_Between_Glass_Blind_On = 9.0;  !- <none>",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    // OutputProcessor::TimeValue.allocate(2);
    state->init_state(*state);

    ErrorsFound = false;
    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    ErrorsFound = false;
    GetFrameAndDividerData(*state);

    SetPreConstructionInputParameters(*state);

    ErrorsFound = false;
    GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);             // expect no errors

    ErrorsFound = false;
    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);        // expect no errors

    ErrorsFound = false;
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    ErrorsFound = false;
    SurfaceGeometry::SetupZoneGeometry(*state,
                                       ErrorsFound); // this calls GetSurfaceData() and SetFlagForWindowConstructionWithShadeOrBlindLayer(*state)
    EXPECT_FALSE(ErrorsFound);

    EXPECT_EQ(state->dataConstruction->Construct(4).Name, "WIN-CON-DOUBLEPANE"); // glass, air gap, glass
    EXPECT_EQ(state->dataConstruction->Construct(4).TotLayers, 3);               //  outer glass, air gap, inner glass
    EXPECT_EQ(state->dataConstruction->Construct(4).TotGlassLayers, 2);          // outer glass, inner glass
    EXPECT_EQ(state->dataConstruction->Construct(4).TotSolidLayers, 2);          // outer glass, inner glass

    EXPECT_EQ(state->dataMaterial->materials(4)->Name, "SINGLEPANE"); // single pane glass
    EXPECT_EQ(state->dataMaterial->materials(5)->Name, "WINGAS");     // air gap
    EXPECT_EQ(state->dataMaterial->materials(6)->Name, "BLIND");      // window blind

    // construction layer material pointers. this construction has no blind
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(1), 4); // glass, outer layer
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(2), 5); // air gap
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(3), 4); // glass, inner layer

    int windowSurfNum = Util::FindItemInList("ZN001:WALL001:WIN001", state->dataSurface->Surface);

    EXPECT_FALSE(state->dataSurface->SurfWinHasShadeOrBlindLayer(windowSurfNum)); // the window construction has no blind
    // check if the construction has a blind material layer
    SetFlagForWindowConstructionWithShadeOrBlindLayer(*state);
    EXPECT_FALSE(state->dataSurface->SurfWinHasShadeOrBlindLayer(windowSurfNum)); // the window construction has no blind

    GetEMSInput(*state);
    // check if EMS actuator is not setup because there is no blind/shade layer
    SetupWindowShadingControlActuators(*state);

    // init_state() checks for EMS so there will be actuators for schedules and materials already
    EXPECT_EQ(state->dataRuntimeLang->numEMSActuatorsAvailable, 19);

    // add a blind layer in between glass
    state->dataConstruction->Construct(4).TotLayers = 5;
    state->dataConstruction->Construct(4).TotGlassLayers = 2;
    state->dataConstruction->Construct(4).TotSolidLayers = 3;
    state->dataConstruction->Construct(4).LayerPoint(1) = 4; // glass
    state->dataConstruction->Construct(4).LayerPoint(2) = 5; // air gap
    state->dataConstruction->Construct(4).LayerPoint(3) = 6; // window blind
    state->dataConstruction->Construct(4).LayerPoint(4) = 5; // air gap
    state->dataConstruction->Construct(4).LayerPoint(5) = 4; // glass
    // updated construction and material layers data
    EXPECT_EQ(state->dataConstruction->Construct(4).TotLayers, 5);      // outer glass, air gap, blind, air gap, inner glass
    EXPECT_EQ(state->dataConstruction->Construct(4).TotGlassLayers, 2); // outer glass, inner glass
    EXPECT_EQ(state->dataConstruction->Construct(4).TotSolidLayers, 3); // glass, blind, glass
    // construction layer material pointers. this construction has blind
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(1), 4); // glass, outer layer
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(2), 5); // air gap
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(3), 6); // blind
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(4), 5); // air gap
    EXPECT_EQ(state->dataConstruction->Construct(4).LayerPoint(5), 4); // glass, inner layer

    // check if the construction has a blind material layer
    SetFlagForWindowConstructionWithShadeOrBlindLayer(*state);
    EXPECT_TRUE(state->dataSurface->SurfWinHasShadeOrBlindLayer(windowSurfNum)); // the window construction has blind
    // set the blind to movable
    state->dataSurface->surfShades(windowSurfNum).blind.movableSlats = true;
    // check if EMS actuator is available when blind layer is added
    SetupWindowShadingControlActuators(*state);
    EXPECT_EQ(state->dataRuntimeLang->numEMSActuatorsAvailable, 21);
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorAvailable(20).ComponentTypeName, "Window Shading Control");
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorAvailable(20).ControlTypeName, "Control Status");
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorAvailable(20).Units, "[ShadeStatus]");
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorAvailable(21).ComponentTypeName, "Window Shading Control");
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorAvailable(21).ControlTypeName, "Slat Angle");
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorAvailable(21).Units, "[degrees]");
}

TEST_F(EnergyPlusFixture, DataHeatBalance_setUserTemperatureLocationPerpendicular)
{
    state->init_state(*state);

    Real64 userInputValue;
    Real64 expectedReturnValue;
    Real64 actualReturnValue;

    state->dataConstruction->Construct.allocate(1);
    auto &thisConstruct(state->dataConstruction->Construct(1));
    thisConstruct.Name = "RadiantSystem1";

    // Test 1: User value is less than zero--should be reset to zero
    userInputValue = -0.25;
    expectedReturnValue = 0.0;
    actualReturnValue = thisConstruct.setUserTemperatureLocationPerpendicular(*state, userInputValue);
    EXPECT_EQ(actualReturnValue, expectedReturnValue);

    // Test 2: User value is greater than unity--should be reset to 1.0
    userInputValue = 1.23456;
    expectedReturnValue = 1.0;
    actualReturnValue = thisConstruct.setUserTemperatureLocationPerpendicular(*state, userInputValue);
    EXPECT_EQ(actualReturnValue, expectedReturnValue);

    // Test 3: User value is valid (between 0 and 1)--returned value should be equal to user input
    userInputValue = 0.234567;
    expectedReturnValue = 0.234567;
    actualReturnValue = thisConstruct.setUserTemperatureLocationPerpendicular(*state, userInputValue);
    EXPECT_EQ(actualReturnValue, expectedReturnValue);
}

TEST_F(EnergyPlusFixture, DataHeatBalance_setNodeSourceAndUserTemp)
{
    state->init_state(*state);
    int expectedNodeNumberAtSource;
    int expectedNodeNumberAtUserSpecifiedLocation;
    state->dataConstruction->Construct.allocate(1);
    auto &thisConstruct(state->dataConstruction->Construct(1));
    thisConstruct.NumOfPerpendNodes = 4;

    // Data common to all tests
    Array1D_int nodePerLayer(Construction::MaxLayersInConstruct);
    nodePerLayer(1) = 5;
    nodePerLayer(2) = 6;
    nodePerLayer(3) = 7;
    nodePerLayer(4) = 8;
    nodePerLayer(5) = 9;

    // Test 1: Not a construction with an internal source--both results should be zero
    thisConstruct.SourceSinkPresent = false;
    expectedNodeNumberAtSource = 0;
    expectedNodeNumberAtUserSpecifiedLocation = 0;
    thisConstruct.setNodeSourceAndUserTemp(nodePerLayer);
    EXPECT_EQ(expectedNodeNumberAtSource, thisConstruct.NodeSource);
    EXPECT_EQ(expectedNodeNumberAtUserSpecifiedLocation, thisConstruct.NodeUserTemp);

    // Test 2: Construction with Internal Source but 1-D
    thisConstruct.SourceSinkPresent = true;
    thisConstruct.SourceAfterLayer = 2;
    thisConstruct.TempAfterLayer = 3;
    thisConstruct.SolutionDimensions = 1;
    expectedNodeNumberAtSource = 11;
    expectedNodeNumberAtUserSpecifiedLocation = 18;
    thisConstruct.setNodeSourceAndUserTemp(nodePerLayer);
    EXPECT_EQ(expectedNodeNumberAtSource, thisConstruct.NodeSource);
    EXPECT_EQ(expectedNodeNumberAtUserSpecifiedLocation, thisConstruct.NodeUserTemp);

    // Test 3a: Construction with Internal Source using 2-D Solution
    //          First sub-test--user location in line with source
    thisConstruct.SourceAfterLayer = 2;
    thisConstruct.TempAfterLayer = 3;
    thisConstruct.SolutionDimensions = 2;
    thisConstruct.userTemperatureLocationPerpendicular = 0.0;
    expectedNodeNumberAtSource = 41;
    expectedNodeNumberAtUserSpecifiedLocation = 69;
    thisConstruct.setNodeSourceAndUserTemp(nodePerLayer);
    EXPECT_EQ(expectedNodeNumberAtSource, thisConstruct.NodeSource);
    EXPECT_EQ(expectedNodeNumberAtUserSpecifiedLocation, thisConstruct.NodeUserTemp);

    // Test 3b: Construction with Internal Source using 2-D Solution
    //          First sub-test--user location at mid-point between tubes
    thisConstruct.SourceAfterLayer = 3;
    thisConstruct.TempAfterLayer = 4;
    thisConstruct.SolutionDimensions = 2;
    thisConstruct.userTemperatureLocationPerpendicular = 1.0;
    expectedNodeNumberAtSource = 69;
    expectedNodeNumberAtUserSpecifiedLocation = 104;
    thisConstruct.setNodeSourceAndUserTemp(nodePerLayer);
    EXPECT_EQ(expectedNodeNumberAtSource, thisConstruct.NodeSource);
    EXPECT_EQ(expectedNodeNumberAtUserSpecifiedLocation, thisConstruct.NodeUserTemp);
}

TEST_F(EnergyPlusFixture, DataHeatBalance_AssignReverseConstructionNumberTest)
{
    state->init_state(*state);
    int ConstrNum;
    int expectedResultRevConstrNum;
    int functionResultRevConstrNum;
    bool ErrorsFound = false;

    state->dataHeatBal->TotConstructs = 2;
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataConstruction->LayerPoint.allocate(Construction::MaxLayersInConstruct);

    auto &thisConstruct(state->dataConstruction->Construct(1));
    auto &otherConstruct(state->dataConstruction->Construct(2));

    // Test: For an interzone surface that uses the zone as the other side, a new
    //       interzone surface for that zone has to be created.  That interzone has
    //       to have the reversed construction.  If that reversed construction is
    //       not used for any defined surface, the variable IsUsed is false.  However,
    //       since it now is being used, IsUsed has to be set to true.  Prior to
    //       work on Defect #8919, when using CondFD and a zone other side condition
    //       for an interzone surface (implied or E+ internally created other side)
    //       for a construction that was reversed in the input file but not used
    //       for other defined surfaces, IsUsed was set to false and this construction
    //       was then skipped in the CondFD routine that calculates the number of
    //       nodes.  This led to a hard crash.  This test simply makes sure that
    //       IsUsed is set to true when the reversed construction already exists.
    ConstrNum = 1;
    thisConstruct.IsUsed = true;
    thisConstruct.TotLayers = 2;
    thisConstruct.LayerPoint.allocate(Construction::MaxLayersInConstruct);
    thisConstruct.LayerPoint = 0;
    thisConstruct.LayerPoint(1) = 10;
    thisConstruct.LayerPoint(2) = 12;
    otherConstruct.IsUsed = false;
    otherConstruct.TotLayers = 2;
    otherConstruct.LayerPoint.allocate(Construction::MaxLayersInConstruct);
    otherConstruct.LayerPoint = 0;
    otherConstruct.LayerPoint(1) = 12;
    otherConstruct.LayerPoint(2) = 10;
    expectedResultRevConstrNum = 2;

    functionResultRevConstrNum = AssignReverseConstructionNumber(*state, ConstrNum, ErrorsFound);
    EXPECT_EQ(expectedResultRevConstrNum, functionResultRevConstrNum);
    EXPECT_TRUE(otherConstruct.IsUsed);
    EXPECT_FALSE(ErrorsFound);
}

TEST_F(EnergyPlusFixture, DataHeatBalance_setThicknessPerpendicularTest)
{
    state->init_state(*state);
    Real64 userInputValue;
    Real64 expectedReturnValue;
    Real64 actualReturnValue;

    state->dataConstruction->Construct.allocate(1);
    auto &thisConstruct(state->dataConstruction->Construct(1));
    thisConstruct.Name = "TestThisConstruction";

    std::string const error_string0 = delimited_string(
        {format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
         "   ** Warning ** ConstructionProperty:InternalHeatSource has a tube spacing that is less than 2 mm.  This is not allowed.",
         "   **   ~~~   ** Construction=TestThisConstruction has this problem.  The tube spacing has been reset to 0.15m (~6 "
         "inches) for this construction.",
         "   **   ~~~   ** As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing."});
    std::string const error_string1 =
        delimited_string({"   ** Warning ** ConstructionProperty:InternalHeatSource has a tube spacing that is less than 2 mm.  This is not allowed.",
                          "   **   ~~~   ** Construction=TestThisConstruction has this problem.  The tube spacing has been reset to 0.15m (~6 "
                          "inches) for this construction.",
                          "   **   ~~~   ** As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing."});
    std::string const error_string2 = delimited_string(
        {"   ** Warning ** ConstructionProperty:InternalHeatSource has a tube spacing that is less than 1 cm (0.4 inch).",
         "   **   ~~~   ** Construction=TestThisConstruction has this concern.  Please check this construction to make sure it is correct.",
         "   **   ~~~   ** As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing."});
    std::string const error_string3 = delimited_string(
        {"   ** Warning ** ConstructionProperty:InternalHeatSource has a tube spacing that is greater than 1 meter (39.4 inches).",
         "   **   ~~~   ** Construction=TestThisConstruction has this concern.  Please check this construction to make sure it is correct.",
         "   **   ~~~   ** As per the Input Output Reference, tube spacing is only used for 2-D solutions and autosizing."});

    // Test 1: User value is less than zero--should be reset to "default" value (warning messages produced)
    userInputValue = -0.01;
    expectedReturnValue = 0.075;
    actualReturnValue = thisConstruct.setThicknessPerpendicular(*state, userInputValue);
    EXPECT_NEAR(expectedReturnValue, actualReturnValue, 0.0001);
    EXPECT_TRUE(compare_err_stream(error_string0, true));

    // Test 2: User value is greater than zero but still too small--should be reset to the "default" value (warning messages produced)
    userInputValue = 0.0001;
    actualReturnValue = thisConstruct.setThicknessPerpendicular(*state, userInputValue);
    EXPECT_NEAR(expectedReturnValue, actualReturnValue, 0.0001);
    EXPECT_TRUE(compare_err_stream(error_string1, true));

    // Test 3: User value is greater than minimum allowed but smaller than "typical"--no resetting (warning messages produced)
    userInputValue = 0.008;
    expectedReturnValue = 0.004;
    actualReturnValue = thisConstruct.setThicknessPerpendicular(*state, userInputValue);
    EXPECT_NEAR(expectedReturnValue, actualReturnValue, 0.0001);
    EXPECT_TRUE(compare_err_stream(error_string2, true));

    // Test 4: User value is great than a typical range--no resetting (warning message produced)
    userInputValue = 2.0;
    expectedReturnValue = 1.0;
    actualReturnValue = thisConstruct.setThicknessPerpendicular(*state, userInputValue);
    EXPECT_NEAR(expectedReturnValue, actualReturnValue, 0.0001);
    EXPECT_TRUE(compare_err_stream(error_string3, true));

    // Test 5: User value is within the typical range--no resetting, no warning messages
    userInputValue = 0.2;
    expectedReturnValue = 0.1;
    actualReturnValue = thisConstruct.setThicknessPerpendicular(*state, userInputValue);
    EXPECT_NEAR(expectedReturnValue, actualReturnValue, 0.0001);
}

TEST_F(EnergyPlusFixture, DataHeatBalance_ComputeNominalUwithConvCoeffsTest)
{
    state->init_state(*state);
    Real64 expectedAnswer;
    Real64 actualAnswer;
    Real64 allowableTolerance = 0.00001;
    bool isWithConvCoefValid;

    state->dataSurface->Surface.allocate(1);
    state->dataHeatBal->NominalU.allocate(1);
    auto &thisSurf(state->dataSurface->Surface(1));
    auto &thisUval(state->dataHeatBal->NominalU(1));

    // Test 1a: Exterior Wall Surface with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.869797;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1b: Wall Surface in contact with Ground with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::Ground;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.893053;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1c: Wall Surface in contact with OSC NoCalcExt with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::OtherSideCoefNoCalcExt;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.893053;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1d: Wall Surface in contact with OSC CondModeledExt with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::OtherSideCondModeledExt;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.893053;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1e: Wall Surface in contact with GroundFCfactorMethod with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::GroundFCfactorMethod;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.893053;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1f: Wall Surface in contact with KivaFoundation with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::KivaFoundation;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.893053;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1g: Surface is an interior wall with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = 0.806771;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1h: Surface is an interior wall using Other Side Coefficients (OSC)
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::OtherSideCoefCalcExt;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    thisSurf.OSCPtr = 1;
    state->dataSurface->OSC.allocate(1);
    state->dataSurface->OSC(1).SurfFilmCoef = 0.5;
    expectedAnswer = 0.617377;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 1i: Surface is an interior wall with an invalid U-value
    thisUval = -1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::Wall;
    thisSurf.Construction = 1;
    expectedAnswer = -1.0;
    isWithConvCoefValid = true;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_FALSE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 2: Surface is an interior floor with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::Floor;
    thisSurf.Construction = 1;
    expectedAnswer = 0.755263;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 3: Surface is an interior ceiling (roof) with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::Roof;
    thisSurf.Construction = 1;
    expectedAnswer = 0.823144;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 4: Surface is internal mass with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::IntMass;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 5: Surface is detached shading (B) with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::Detached_B;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 6: Surface is detached shading (F) with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::Detached_F;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 7: Surface is a window with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::Window;
    thisSurf.Construction = 1;
    expectedAnswer = 0.806771;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 8: Surface is a glass door with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::GlassDoor;
    thisSurf.Construction = 1;
    expectedAnswer = 0.806771;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 9: Surface is a door with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = 1;
    thisSurf.Class = DataSurfaces::SurfaceClass::Door;
    thisSurf.Construction = 1;
    expectedAnswer = 0.806771;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 10: Surface is shading with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::Shading;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 11: Surface is an overhang with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::Overhang;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 12: Surface is a fin with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::Fin;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 13: Surface is a TDD dome with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::TDD_Dome;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Test 14: Surface is a TDD diffuser with a valid U-value
    thisUval = 1.0;
    thisSurf.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    thisSurf.Class = DataSurfaces::SurfaceClass::TDD_Diffuser;
    thisSurf.Construction = 1;
    expectedAnswer = 1.0;
    isWithConvCoefValid = false;
    actualAnswer = ComputeNominalUwithConvCoeffs(*state, 1, isWithConvCoefValid);
    EXPECT_TRUE(isWithConvCoefValid);
    EXPECT_NEAR(expectedAnswer, actualAnswer, allowableTolerance);

    // Check to see if there have been any additions to SurfaceClass--this will fail is something got added
    // If something does get added to SurfaceClass, changes also need to be made to ComputeNominalUwithConvCoeffs
    // in DataHeatBalance.cc
    int numTypesSurfaceClass = static_cast<int>(DataSurfaces::SurfaceClass::Num);
    bool surfClassOK = false;
    if (numTypesSurfaceClass == 15) {
        surfClassOK = true;
    }
    EXPECT_TRUE(surfClassOK);
}
