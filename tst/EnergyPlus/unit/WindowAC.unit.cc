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
// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WindowAC.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, WindowAC_VStest1)
{
    // this unit test runs the window air conditioner with a Coil:Cooling:DX:VariableSpeed coil
    // set up minimal zone, zone equipment, and ZoneHVAC:WindowAirConditioner, check input processing, check sizing, check simulation results
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "   Yes;                     !- Run Simulation for Weather File Run Periods",

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
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneHVAC:WindowAirConditioner,",
        "    Zone1WindAC,             !- Name",
        "    ,   !- Availability Schedule Name",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    Zone1WindACAirInletNode, !- Air Inlet Node Name",
        "    Zone1WindACAirOutletNode,!- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    Zone1WindACOAMixer,      !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    Zone1WindACFan,          !- Supply Air Fan Name",
        "    Coil:Cooling:DX:VariableSpeed,  !- Cooling Coil Object Type",
        "    Zone1WindAC_VS_DXCoil,       !- DX Cooling Coil Name",
        "    ,           !- Supply Air Fan Operating Mode Schedule Name",
        "    BlowThrough,             !- Fan Placement",
        "    0.001;                   !- Cooling Convergence Tolerance",

        "  Schedule:Compact,",
        "    CYCLINGFANSCH,           !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,0.00;       !- Field 3",

        "  OutdoorAir:Mixer,",
        "    Zone1WindACOAMixer,      !- Name",
        "    Zone1WindACOAMixerOutletNode,  !- Mixed Air Node Name",
        "    Zone1WindACOAInNode,     !- Outdoor Air Stream Node Name",
        "    Zone1WindACExhNode,      !- Relief Air Stream Node Name",
        "    Zone1WindACAirInletNode; !- Return Air Stream Node Name",

        "  Fan:OnOff,",
        "    Zone1WindACFan,          !- Name",
        "    ,    !- Availability Schedule Name",
        "    0.5,                     !- Fan Total Efficiency",
        "    75.0,                    !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    Zone1WindACOAMixerOutletNode,  !- Air Inlet Node Name",
        "    Zone1WindACFanOutletNode;!- Air Outlet Node Name",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    Zone1WindAC_VS_DXCoil,    !- Name",
        "    ,                         !- Availability Schedule Name",
        "    Zone1WindACFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1WindACAirOutletNode,  !- Indoor Air Outlet Node Name",
        "    1,                       !- Number of Speeds {dimensionless}",
        "    1,                       !- Nominal Speed Level {dimensionless}",
        "    AUTOSIZE,                !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
        "    AUTOSIZE,                !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    0.0,                     !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    36991.44197,             !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {w}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.866381837,             !- Speed 1 Reference Unit Gross Rated Cooling COP {dimensionless}",
        "    3.776,                   !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    10.62,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp4,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp4,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF;                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "     Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    1.0,                    !- Coefficient1 Constant",
        "    0.0,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x  ",

        "  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp4,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp4,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    Zone1Exhausts,           !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:WindowAirConditioner,  !- Zone Equipment 1 Object Type",
        "    Zone1WindAC,             !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  NodeList,",
        "    Zone1Exhausts,           !- Name",
        "    Zone1WindACAirInletNode; !- Node 1 Name",

        "  OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Node or NodeList Name 1",

        "  NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    Zone1WindACOAInNode;     !- Node 1 Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1WindACAirOutletNode;!- Node 1 Name",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:SingleHeating,  !- Control 1 Object Type",
        "    Heating Setpoint with SB,!- Control 1 Name",
        "    ThermostatSetpoint:SingleCooling,  !- Control 2 Object Type",
        "    Cooling Setpoint with SB;!- Control 2 Name",

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

        "  ThermostatSetpoint:SingleHeating,",
        "    Heating Setpoint with SB,!- Name",
        "    Heating Setpoints;       !- Setpoint Temperature Schedule Name",

        "  ThermostatSetpoint:SingleCooling,",
        "    Cooling Setpoint with SB,!- Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    West Zone,               !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    0.0,                     !- Zone Heating Sizing Factor",
        "    0.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ;                        !- Heating Maximum Air Flow Fraction",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

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
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

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

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 6;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 10; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);
    // OutputProcessor::TimeValue.allocate(2);
    state->dataGlobal->DDOnlySimulation = true;

    SimulationManager::GetProjectData(*state);
    OutputReportPredefined::SetPredefinedTables(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state); // establish array bounds for constructions early

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->ZoneSizingCalc = true;
    EnergyPlus::createFacilityElectricPowerServiceObject(*state);

    SizingManager::ManageSizing(*state);

    SimulationManager::SetupSimulation(*state, errorsFound);
    //

    Real64 qDotMet(0.0);    // Watts total cap
    Real64 lDotProvid(0.0); // latent removal kg/s
    int compIndex(0);
    WindowAC::SimWindowAC(*state, "ZONE1WINDAC", 1, true, qDotMet, lDotProvid, compIndex);
    // check input processing
    EXPECT_EQ(compIndex, 1);

    // MixTemp = 24.00, MixHumRat = 0.008, SupTemp = 12, SupHumRat = 0.008
    double constexpr expected_full_airflow = 0.041484382187390034;
    double constexpr expected_full_cap = 622.50474573886743;

    auto const &windowAC = state->dataWindowAC->WindAC(compIndex);

    ASSERT_GT(windowAC.FanIndex, 0);
    auto const &fan = state->dataFans->fans(windowAC.FanIndex);

    auto const &finalZoneSizing = state->dataSize->FinalZoneSizing(1);

    EXPECT_NEAR(expected_full_airflow, finalZoneSizing.DesCoolVolFlow, 0.0001);

    EXPECT_EQ(windowAC.HVACSizingIndex, 0);

    EXPECT_EQ(windowAC.DXCoilType_Num, HVAC::Coil_CoolingAirToAirVariableSpeed);
    ASSERT_GT(windowAC.DXCoilIndex, 0);
    auto const &varSpeedCoil = state->dataVariableSpeedCoils->VarSpeedCoil(windowAC.DXCoilIndex);

    // check Sizing
    EXPECT_NEAR(expected_full_airflow, windowAC.MaxAirVolFlow, 0.0001);
    EXPECT_NEAR(expected_full_airflow, fan->maxAirFlowRate, 0.0001);
    EXPECT_NEAR(expected_full_cap, varSpeedCoil.RatedCapCoolTotal, 0.001);

    // VSD Sizing forces it back to Catalog data based on Nominal Speed Level
    double constexpr vsd_coil_norm_speed_tot_cap = 36991.44197; // Speed 1 Reference Unit Gross Rated Total Cooling Capacity {w}
    double constexpr vsd_coil_norm_speed_airflow = 3.776;       // Speed 1 Reference Unit Gross Rated Cooling COP {dimensionless}
    double constexpr vsd_coil_norm_speed_airflow_per_cap = vsd_coil_norm_speed_airflow / vsd_coil_norm_speed_tot_cap;
    EXPECT_NEAR(vsd_coil_norm_speed_airflow_per_cap, varSpeedCoil.MSRatedAirVolFlowPerRatedTotCap(1), 0.0001);
    EXPECT_NEAR(expected_full_cap * vsd_coil_norm_speed_airflow_per_cap, varSpeedCoil.RatedAirVolFlowRate, 0.0001);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -295.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;

    WindowAC::SimWindowAC(*state, "ZONE1WINDAC", 1, true, qDotMet, lDotProvid, compIndex);
    // check output
    EXPECT_NEAR(qDotMet, -295.0, 0.1);

    // #8124 Retrieve zero value data without heating loads
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "WEST ZONE"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "WEST ZONE"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "WEST ZONE"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "WEST ZONE"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "WEST ZONE"));
}

TEST_F(EnergyPlusFixture, WindowAC_DesignSpecificationZoneHVACSizing)
{
    // this unit test runs the window air conditioner with a Coil:Cooling:DX:VariableSpeed coil
    // set up minimal zone, zone equipment, and ZoneHVAC:WindowAirConditioner, check input processing, check sizing, check simulation results
    //  This adds a DesignSpecification:ZoneHVAC:Sizing too

    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "   Yes;                     !- Run Simulation for Weather File Run Periods",

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
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneHVAC:WindowAirConditioner,",
        "    Zone1WindAC,             !- Name",
        "    ,   !- Availability Schedule Name",
        "    autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "    autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    Zone1WindACAirInletNode, !- Air Inlet Node Name",
        "    Zone1WindACAirOutletNode,!- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    Zone1WindACOAMixer,      !- Outdoor Air Mixer Name",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    Zone1WindACFan,          !- Supply Air Fan Name",
        "    Coil:Cooling:DX:VariableSpeed,  !- Cooling Coil Object Type",
        "    Zone1WindAC_VS_DXCoil,       !- DX Cooling Coil Name",
        "    ,           !- Supply Air Fan Operating Mode Schedule Name",
        "    BlowThrough,             !- Fan Placement",
        "    0.001,                   !- Cooling Convergence Tolerance",
        "    ,                        !- Availability Manager List Name",
        "    WindowACSizing;          !- Design Specification ZoneHVAC Sizing Object Name",

        // NOTE: Only difference here is that I'm setting a DesignSpecification:ZoneHVAC:Sizing
        "  DesignSpecification:ZoneHVAC:Sizing,",
        "    WindowACSizing,          !- Name",
        "    FractionOfAutosizedCoolingAirflow,  !- Cooling Supply Air Flow Rate Method",
        "    ,                        !- Cooling Supply Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    0.5,                     !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}",
        "    FractionOfAutosizedCoolingAirflow,  !- No Load Supply Air Flow Rate Method",
        "    ,                        !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    0.5,                     !- No Load Fraction of Cooling Supply Air Flow Rate",
        "    ,                        !- No Load Fraction of Heating Supply Air Flow Rate",
        "    None,                    !- Heating Supply Air Flow Rate Method",
        "    ,                        !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                        !- Heating Fraction of Heating Supply Air Flow Rate",
        "    ,                        !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}",
        "    FractionOfAutosizedCoolingCapacity,  !- Cooling Design Capacity Method",
        "    ,                        !- Cooling Design Capacity {W}",
        "    ,                        !- Cooling Design Capacity Per Floor Area {W/m2}",
        "    0.5;                     !- Fraction of Autosized Cooling Design Capacity",

        "  Schedule:Compact,",
        "    CYCLINGFANSCH,           !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,0.00;       !- Field 3",

        "  OutdoorAir:Mixer,",
        "    Zone1WindACOAMixer,      !- Name",
        "    Zone1WindACOAMixerOutletNode,  !- Mixed Air Node Name",
        "    Zone1WindACOAInNode,     !- Outdoor Air Stream Node Name",
        "    Zone1WindACExhNode,      !- Relief Air Stream Node Name",
        "    Zone1WindACAirInletNode; !- Return Air Stream Node Name",

        "  Fan:OnOff,",
        "    Zone1WindACFan,          !- Name",
        "    ,    !- Availability Schedule Name",
        "    0.5,                     !- Fan Total Efficiency",
        "    75.0,                    !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    Zone1WindACOAMixerOutletNode,  !- Air Inlet Node Name",
        "    Zone1WindACFanOutletNode;!- Air Outlet Node Name",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    Zone1WindAC_VS_DXCoil,    !- Name",
        "    ,                         !- Availability Schedule Name",
        "    Zone1WindACFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1WindACAirOutletNode,  !- Indoor Air Outlet Node Name",
        "    1,                       !- Number of Speeds {dimensionless}",
        "    1,                       !- Nominal Speed Level {dimensionless}",
        "    AUTOSIZE,                !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
        "    AUTOSIZE,                !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    0.0,                     !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    36991.44197,             !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {w}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.866381837,             !- Speed 1 Reference Unit Gross Rated Cooling COP {dimensionless}",
        "    3.776,                   !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    10.62,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp4,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp4,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF;                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "     Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    1.0,                    !- Coefficient1 Constant",
        "    0.0,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x  ",

        "  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp4,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp4,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    Zone1Exhausts,           !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:WindowAirConditioner,  !- Zone Equipment 1 Object Type",
        "    Zone1WindAC,             !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  NodeList,",
        "    Zone1Exhausts,           !- Name",
        "    Zone1WindACAirInletNode; !- Node 1 Name",

        "  OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Node or NodeList Name 1",

        "  NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    Zone1WindACOAInNode;     !- Node 1 Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1WindACAirOutletNode;!- Node 1 Name",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:SingleHeating,  !- Control 1 Object Type",
        "    Heating Setpoint with SB,!- Control 1 Name",
        "    ThermostatSetpoint:SingleCooling,  !- Control 2 Object Type",
        "    Cooling Setpoint with SB;!- Control 2 Name",

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

        "  ThermostatSetpoint:SingleHeating,",
        "    Heating Setpoint with SB,!- Name",
        "    Heating Setpoints;       !- Setpoint Temperature Schedule Name",

        "  ThermostatSetpoint:SingleCooling,",
        "    Cooling Setpoint with SB,!- Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    West Zone,               !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    0.0,                     !- Zone Heating Sizing Factor",
        "    0.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ;                        !- Heating Maximum Air Flow Fraction",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

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
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

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

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state); // must initialize this to get schedules initialized

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);
    // OutputProcessor::TimeValue.allocate(2);
    state->dataGlobal->DDOnlySimulation = true;

    SimulationManager::GetProjectData(*state);
    OutputReportPredefined::SetPredefinedTables(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state); // establish array bounds for constructions early

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->ZoneSizingCalc = true;
    EnergyPlus::createFacilityElectricPowerServiceObject(*state);

    SizingManager::ManageSizing(*state);

    SimulationManager::SetupSimulation(*state, errorsFound);
    //

    Real64 qDotMet(0.0);    // Watts total cap
    Real64 lDotProvid(0.0); // latent removal kg/s
    int compIndex(0);
    WindowAC::SimWindowAC(*state, "ZONE1WINDAC", 1, true, qDotMet, lDotProvid, compIndex);
    // check input processing
    EXPECT_EQ(compIndex, 1);

    double constexpr expected_full_airflow = 0.041484382187390034;
    double constexpr scaler_cooling_saf = 0.5;
    double constexpr scaler_cooling_cap = 0.5;
    double constexpr expected_full_cap = 622.50474573886743;

    auto const &windowAC = state->dataWindowAC->WindAC(compIndex);

    ASSERT_GT(windowAC.FanIndex, 0);
    auto const &fan = state->dataFans->fans(windowAC.FanIndex);

    auto const &finalZoneSizing = state->dataSize->FinalZoneSizing(1);

    EXPECT_NEAR(expected_full_airflow, finalZoneSizing.DesCoolVolFlow, 0.0001);

    ASSERT_GT(windowAC.HVACSizingIndex, 0);
    auto const &zoneHVACSizing = state->dataSize->ZoneHVACSizing(windowAC.HVACSizingIndex);

    EXPECT_EQ(DataSizing::FractionOfAutosizedCoolingAirflow, zoneHVACSizing.CoolingSAFMethod);
    EXPECT_EQ(scaler_cooling_saf, zoneHVACSizing.MaxCoolAirVolFlow);
    EXPECT_EQ(DataSizing::FractionOfAutosizedCoolingCapacity, zoneHVACSizing.CoolingCapMethod);
    EXPECT_EQ(scaler_cooling_cap, zoneHVACSizing.ScaledCoolingCapacity);

    EXPECT_EQ(windowAC.DXCoilType_Num, HVAC::Coil_CoolingAirToAirVariableSpeed);
    ASSERT_GT(windowAC.DXCoilIndex, 0);
    auto const &varSpeedCoil = state->dataVariableSpeedCoils->VarSpeedCoil(windowAC.DXCoilIndex);

    // check Sizing
    EXPECT_NEAR(expected_full_airflow * scaler_cooling_saf, windowAC.MaxAirVolFlow, 0.0001);
    EXPECT_NEAR(expected_full_airflow * scaler_cooling_saf, fan->maxAirFlowRate, 0.0001);

    EXPECT_NEAR(622.505, varSpeedCoil.RatedCapCoolTotal, 0.001);

    // VSD Sizing forces it back to Catalog data based on Nominal Speed Level
    double constexpr vsd_coil_norm_speed_tot_cap = 36991.44197; // Speed 1 Reference Unit Gross Rated Total Cooling Capacity {w}
    double constexpr vsd_coil_norm_speed_airflow = 3.776;       // Speed 1 Reference Unit Gross Rated Cooling COP {dimensionless}
    double constexpr vsd_coil_norm_speed_airflow_per_cap = vsd_coil_norm_speed_airflow / vsd_coil_norm_speed_tot_cap;
    EXPECT_NEAR(vsd_coil_norm_speed_airflow_per_cap, varSpeedCoil.MSRatedAirVolFlowPerRatedTotCap(1), 0.0001);
    EXPECT_NEAR(expected_full_cap * vsd_coil_norm_speed_airflow_per_cap, varSpeedCoil.RatedAirVolFlowRate, 0.0001);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -295.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;

    WindowAC::SimWindowAC(*state, "ZONE1WINDAC", 1, true, qDotMet, lDotProvid, compIndex);
    // check output
    EXPECT_NEAR(-295.0, qDotMet, 0.1);

    // #8124 Retrieve zero value data without heating loads
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "WEST ZONE"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "WEST ZONE"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "WEST ZONE"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "WEST ZONE"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "WEST ZONE"));
}
