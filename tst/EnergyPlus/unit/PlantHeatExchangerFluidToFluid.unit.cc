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

// EnergyPlus::PlantHeatExchangerFluidToFluid Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantHeatExchangerFluidToFluid.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, PlantHXModulatedDualDeadDefectFileHi)
{
    // this unit test was devised for issue #5258 which involves control logic related to plant HX not controlling well when the setpoint cannot be
    // met this test has complete IDF input to set up a system of four plant loops taken from the PlantLoopChain* integration tests.  This test checks
    // that the HX will attempt to meet setpoint of 19 when the conditioning fluid is 20 and cannot really make it to 19.  The HX still cools to down
    // to 20.

    std::string const idf_objects = delimited_string({
        "Building,",
        "Plant Load Profile Example,  !- Name",
        "    0.0,                     !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.04,                    !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",

        "  Site:Location,",
        "    CHICAGO_IL_USA_WMO_725300,  !- Name",
        "    42.00,                   !- Latitude {deg}",
        "    -87.88,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO Ann Htg 99% Condns DB,  !- Name",
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
        "    0.00;                    !- Sky Clearness",

        "  RunPeriod,",
        "    Jan,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    2,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  SimulationControl,",
        "    NO,                      !- Do Zone Sizing Calculation",
        "    NO,                      !- Do System Sizing Calculation",
        "    NO,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  PlantLoop,",
        "    Use Heat Loop,           !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Use Heat Loop Operation, !- Plant Equipment Operation Scheme Name",
        "    Use Heat Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    Use Heat Supply Inlet Node,  !- Plant Side Inlet Node Name",
        "    Use Heat Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    Use Heat Supply Branches,!- Plant Side Branch List Name",
        "    Use Heat Supply Connectors,  !- Plant Side Connector List Name",
        "    Use Heat Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    Use Heat Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    Use Heat Demand Branches,!- Demand Side Branch List Name",
        "    Use Heat Demand Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Use Heat Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Use Heat Loop Temp Sch,  !- Schedule Name",
        "    Use Heat Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Use Heat Loop Setpoint Node List,  !- Name",
        "    Use Heat Supply Outlet Node;  !- Node 1 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Use Heat Loop Operation, !- Name",
        "    PlantEquipmentOperation:Uncontrolled,  !- Control Scheme 1 Object Type",
        "    USE Heat operation,      !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:Uncontrolled,",
        "    USE Heat operation,      !- Name",
        "    TRANSFER to USE Heat HX equip;  !- Equipment List Name",

        "  PlantEquipmentList,",
        "    TRANSFER to USE Heat HX equip,  !- Name",
        "    HeatExchanger:FluidToFluid,  !- Equipment 1 Object Type",
        "    TRANSFER to USE Heat HX; !- Equipment 1 Name",

        "  BranchList,",
        "    USE Heat Supply Branches,!- Name",
        "    USE Heat Supply Inlet Branch,  !- Branch 1 Name",
        "    USE Heat Heating Branch, !- Branch 2 Name",
        "    USE Heat Supply Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Heat Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Heat Supply Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Heat Supply Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Heat Supply Splitter,!- Name",
        "    USE Heat Supply Inlet Branch,  !- Inlet Branch Name",
        "    USE Heat Heating Branch; !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Heat Supply Mixer,   !- Name",
        "    USE Heat Supply Outlet Branch,  !- Outlet Branch Name",
        "    USE Heat Heating Branch; !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Heat Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    USE Heat Pump,           !- Component 1 Name",
        "    USE Heat Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Supply Pump-Heating Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    USE Heat Pump,           !- Name",
        "    USE Heat Supply Inlet Node,  !- Inlet Node Name",
        "    USE Heat Supply Pump-Heating Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    USE Heat Heating Branch, !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE Heat HX, !- Component 1 Name",
        "    USE Heat Supply Heating Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Supply Heating Outlet Node;  !- Component 1 Outlet Node Name",

        "  HeatExchanger:FluidToFluid,",
        "    TRANSFER to USE Heat HX, !- Name",
        "    AlwaysOnSchedule,        !- Availability Schedule Name",
        "    TRANSFER Heat Demand HX Inlet Node,  !- Loop Demand Side Inlet Node Name",
        "    TRANSFER Heat Demand HX Outlet Node,  !- Loop Demand Side Outlet Node Name",
        "    0.003,                   !- Loop Demand Side Design Flow Rate {m3/s}",
        "    USE Heat Supply Heating Inlet Node,  !- Loop Supply Side Inlet Node Name",
        "    USE Heat Supply Heating Outlet Node,  !- Loop Supply Side Outlet Node Name",
        "    0.003,                   !- Loop Supply Side Design Flow Rate {m3/s}",
        "    CrossFlowBothUnMixed,    !- Heat Exchange Model Type",
        "    15000,                   !- Heat Exchanger U-Factor Times Area Value {W/k}",
        "    HeatingSetpointModulated,!- Control Type",
        "    Use heat Supply Outlet Node,  !- Heat Exchanger Setpoint Node Name",
        "    0.2,                     !- Minimum Temperature Difference to Activate Heat Exchanger {deltaC}",
        "   LoopToLoop;              !- Heat Transfer Metering End Use Type",

        "  Branch,",
        "    USE Heat Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Heat Supply Outlet Pipe,  !- Component 1 Name",
        "    USE Heat Supply Heating-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Heat Supply Outlet Pipe,  !- Name",
        "    USE Heat Supply Heating-Pipe Node,  !- Inlet Node Name",
        "    USE Heat Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    USE Heat Demand Branches,!- Name",
        "    USE Heat Demand Inlet Branch,  !- Branch 1 Name",
        "    USE Heat Load Profile Branch 1,  !- Branch 2 Name",
        "    USE Heat Demand Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Heat Demand Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Heat Demand Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Heat Demand Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Heat Demand Splitter,!- ",
        "    USE Heat Demand Inlet Branch,  !- Inlet Branch Name",
        "    USE Heat Load Profile Branch 1;  !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Heat Demand Mixer,   !- Name",
        "    USE Heat Demand Outlet Branch,  !- Outlet Branch Name",
        "    USE Heat Load Profile Branch 1;  !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Heat Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Heat Demand Inlet Pipe,  !- Component 1 Name",
        "    USE Heat Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Demand Pipe-Load Profile Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Heat Demand Inlet Pipe,  !- Name",
        "    USE Heat Demand Inlet Node,  !- Inlet Node Name",
        "    USE Heat Demand Pipe-Load Profile Node;  !- Outlet Node Name",

        "  Branch,",
        "    USE Heat Load Profile Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    LoadProfile:Plant,       !- Component 1 Object Type",
        "    Heat Load Profile 1,     !- Component 1 Name",
        "    Heat Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
        "    Heat Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

        "  LoadProfile:Plant,",
        "    Heat Load Profile 1,     !- Name",
        "    Heat Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
        "    Heat Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
        "    Heat Load Profile 1 Load Schedule,  !- Load Schedule Name",
        "    0.002,                   !- Peak Flow Rate {m3/s}",
        "    Heat Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

        "  Branch,",
        "    USE Heat Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Heat Demand Outlet Pipe,  !- Component 1 Name",
        "    USE Heat Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Heat Demand Outlet Pipe,  !- Name",
        "    USE Heat Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    USE Heat Demand Outlet Node;  !- Outlet Node Name",

        "  PlantLoop,",
        "    Use Cool Loop,           !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Use Cool Loop Operation, !- Plant Equipment Operation Scheme Name",
        "    Use Cool Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    Use Cool Supply Inlet Node,  !- Plant Side Inlet Node Name",
        "    Use Cool Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    Use Cool Supply Branches,!- Plant Side Branch List Name",
        "    Use Cool Supply Connectors,  !- Plant Side Connector List Name",
        "    Use Cool Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    Use Cool Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    Use Cool Demand Branches,!- Demand Side Branch List Name",
        "    Use Cool Demand Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Use Cool Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Use Cool Loop Temp Sch,  !- Schedule Name",
        "    Use Cool Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Use Cool Loop Setpoint Node List,  !- Name",
        "    Use Cool Supply Outlet Node;  !- Node 1 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Use Cool Loop Operation, !- Name",
        "    PlantEquipmentOperation:Uncontrolled,  !- Control Scheme 1 Object Type",
        "    USE Cool operation,      !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:Uncontrolled,",
        "    USE Cool operation,      !- Name",
        "    TRANSFER to USE Cool HX equip;  !- Equipment List Name",

        "  PlantEquipmentList,",
        "    TRANSFER to USE Cool HX equip,  !- Name",
        "    HeatExchanger:FluidToFluid,  !- Equipment 1 Object Type",
        "    TRANSFER to USE Cool HX; !- Equipment 1 Name",

        "  BranchList,",
        "    USE Cool Supply Branches,!- Name",
        "    USE Cool Supply Inlet Branch,  !- Branch 1 Name",
        "    USE Cooling Branch,      !- Branch 2 Name",
        "    USE Cool Supply Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Cool Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Cool Supply Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Cool Supply Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Cool Supply Splitter,!- Name",
        "    USE Cool Supply Inlet Branch,  !- Inlet Branch Name",
        "    USE Cooling Branch;      !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Cool Supply Mixer,   !- Name",
        "    USE Cool Supply Outlet Branch,  !- Outlet Branch Name",
        "    USE Cooling Branch;      !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Cool Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    USE Cool Pump,           !- Component 1 Name",
        "    USE Cool Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Supply Pump-Heating Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    USE Cool Pump,           !- Name",
        "    USE Cool Supply Inlet Node,  !- Inlet Node Name",
        "    USE Cool Supply Pump-Heating Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    USE Cooling Branch,      !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE Cool HX, !- Component 1 Name",
        "    USE Cool Supply Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Supply Cooling Outlet Node;  !- Component 1 Outlet Node Name",

        "  HeatExchanger:FluidToFluid,",
        "    TRANSFER to USE Cool HX, !- Name",
        "    AlwaysOnSchedule,        !- Availability Schedule Name",
        "    TRANSFER Demand HX Cool Inlet Node,  !- Loop Demand Side Inlet Node Name",
        "    TRANSFER Demand HX Cool Outlet Node,  !- Loop Demand Side Outlet Node Name",
        "    0.003,                   !- Loop Demand Side Design Flow Rate {m3/s}",
        "    USE Cool Supply Cooling Inlet Node,  !- Loop Supply Side Inlet Node Name",
        "    USE Cool Supply Cooling Outlet Node,  !- Loop Supply Side Outlet Node Name",
        "    0.003,                   !- Loop Supply Side Design Flow Rate {m3/s}",
        "    CrossFlowBothUnMixed,    !- Heat Exchange Model Type",
        "    15000,                   !- Heat Exchanger U-Factor Times Area Value {W/k}",
        "    CoolingSetpointModulated,!- Control Type",
        "    Use Cool Supply Outlet Node,  !- Heat Exchanger Setpoint Node Name",
        "    0.2,                     !- Minimum Temperature Difference to Activate Heat Exchanger {deltaC}",
        "    LoopToLoop;              !- Heat Transfer Metering End Use Type",

        "  Branch,",
        "    USE Cool Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Cool Supply Outlet Pipe,  !- Component 1 Name",
        "    USE Cool Supply Heating-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Cool Supply Outlet Pipe,  !- Name",
        "    USE Cool Supply Heating-Pipe Node,  !- Inlet Node Name",
        "    USE Cool Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    USE Cool Demand Branches,!- Name",
        "    USE Cool Demand Inlet Branch,  !- Branch 1 Name",
        "    USE Cool Load Profile Branch 1,  !- Branch 2 Name",
        "    USE Cool Demand Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Cool Demand Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Cool Demand Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Cool Demand Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Cool Demand Splitter,!- Name",
        "    USE Cool Demand Inlet Branch,  !- Inlet Branch Name",
        "    USE Cool Load Profile Branch 1;  !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Cool Demand Mixer,   !- Name",
        "    USE Cool Demand Outlet Branch,  !- Outlet Branch Name",
        "    USE Cool Load Profile Branch 1;  !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Cool Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Cool Demand Inlet Pipe,  !- Component 1 Name",
        "    USE Cool Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Demand Pipe-Load Profile Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Cool Demand Inlet Pipe,  !- Name",
        "    USE Cool Demand Inlet Node,  !- Inlet Node Name",
        "    USE Cool Demand Pipe-Load Profile Node;  !- Outlet Node Name",

        "  Branch,",
        "    USE Cool Load Profile Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    LoadProfile:Plant,       !- Component 1 Object Type",
        "    Cool Load Profile 1,     !- Component 1 Name",
        "    Demand Cool Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
        "    Demand Cool Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

        "  LoadProfile:Plant,",
        "    Cool Load Profile 1,     !- Name",
        "    Demand Cool Load Profile 1 Inlet Node,  !- Inlet Node Name",
        "    Demand Cool Load Profile 1 Outlet Node,  !- Outlet Node Name",
        "    Cool Load Profile 1 Load Schedule,  !- Load Schedule Name",
        "    0.002,                   !- Peak Flow Rate {m3/s}",
        "    Cool Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

        "  Branch,",
        "    USE Cool Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Cool Demand Outlet Pipe,  !- Component 1 Name",
        "    USE Cool Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Cool Demand Outlet Pipe,  !- Name",
        "    USE Cool Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    USE Cool Demand Outlet Node;  !- Outlet Node Name",

        "  PlantLoop,",
        "    TRANSFER Loop,           !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    TRANSFER Loop Operation, !- Plant Equipment Operation Scheme Name",
        "    TRANSFER Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    TRANSFER Supply Inlet Node,  !- Plant Side Inlet Node Name",
        "    TRANSFER Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    TRANSFER Supply Branches,!- Plant Side Branch List Name",
        "    TRANSFER Supply Connectors,  !- Plant Side Connector List Name",
        "    TRANSFER Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    TRANSFER Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    TRANSFER Demand Branches,!- Demand Side Branch List Name",
        "    TRANSFER Demand Connectors,  !- Demand Side Connector List Name",
        "    OPTIMAL,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetpointDeadband,    !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled:dualSetpoint,",
        "    TRANSFER Loop Dual Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    TRANSFER Dual Loop Hi Temp Sch,  !- High Setpoint Schedule Name",
        "    TRANSFER Dual Loop Lo Temp Sch,  !- Low Setpoint Schedule Name",
        "    TRANSFER Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    TRANSFER Loop Setpoint Node List,  !- Name",
        "    TRANSFER Supply Outlet Node;  !- Node 1 Name",

        "  PlantEquipmentOperationSchemes,",
        "    TRANSFER Loop Operation, !- Name",
        "    PlantEquipmentOperation:Uncontrolled,  !- Control Scheme 1 Object Type",
        "    TRANSFER HX,             !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:Uncontrolled,",
        "    TRANSFER HX,             !- Name",
        "    TRANSFER Plant;          !- Equipment List Name",

        "  PlantEquipmentList,",
        "    TRANSFER Plant,          !- Name",
        "    HeatExchanger:FluidToFluid,  !- Equipment 1 Object Type",
        "    SOURCE to TRANSFER HX;   !- Equipment 1 Name",

        "  BranchList,",
        "    TRANSFER Supply Branches,!- Name",
        "    TRANSFER Supply Inlet Branch,  !- Branch 1 Name",
        "    TRANSFER HX Branch,      !- Branch 2 Name",
        "    TRANSFER Supply Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    TRANSFER Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    TRANSFER Supply Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    TRANSFER Supply Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    TRANSFER Supply Splitter,!- Name",
        "    TRANSFER Supply Inlet Branch,  !- Inlet Branch Name",
        "    TRANSFER HX Branch;      !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    TRANSFER Supply Mixer,   !- Name",
        "    TRANSFER Supply Outlet Branch,  !- Outlet Branch Name",
        "    TRANSFER HX Branch;      !- Inlet Branch 1 Name",

        "  Branch,",
        "    TRANSFER Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    TRANSFER Pump,           !- Component 1 Name",
        "    TRANSFER Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Supply Pump-Heating Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    TRANSFER Pump,           !- Name",
        "    TRANSFER Supply Inlet Node,  !- Inlet Node Name",
        "    TRANSFER Supply Pump-Heating Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    TRANSFER HX Branch,      !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    SOURCE to TRANSFER HX,   !- Component 1 Name",
        "    TRANSFER HX Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER HX Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  HeatExchanger:FluidToFluid,",
        "    SOURCE to TRANSFER HX,   !- Name",
        "    AlwaysOnSchedule,        !- Availability Schedule Name",
        "    SOURCE Demand HX Inlet Node,  !- Loop Demand Side Inlet Node Name",
        "    SOURCE Demand HX Outlet Node,  !- Loop Demand Side Outlet Node Name",
        "    0.003,                   !- Loop Demand Side Design Flow Rate {m3/s}",
        "    TRANSFER HX Supply Inlet Node,  !- Loop Supply Side Inlet Node Name",
        "    TRANSFER HX Supply Outlet Node,  !- Loop Supply Side Outlet Node Name",
        "    0.003,                   !- Loop Supply Side Design Flow Rate {m3/s}",
        "    CrossFlowBothUnMixed,    !- Heat Exchange Model Type",
        "    15000,                   !- Heat Exchanger U-Factor Times Area Value {W/k}",
        "    DualDeadbandSetpointModulated,  !- Control Type",
        "    TRANSFER Supply Outlet Node,  !- Heat Exchanger Setpoint Node Name",
        "    0.2,                     !- Minimum Temperature Difference to Activate Heat Exchanger {deltaC}",
        "    LoopToLoop;              !- Heat Transfer Metering End Use Type",

        "  Branch,",
        "    TRANSFER Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TRANSFER Supply Outlet Pipe,  !- Component 1 Name",
        "    TRANSFER Supply HX-Pipe Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    TRANSFER Supply Outlet Pipe,  !- Name",
        "    TRANSFER Supply HX-Pipe Node,  !- Inlet Node Name",
        "    TRANSFER Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    TRANSFER Demand Branches,!- Name",
        "    TRANSFER Demand Inlet Branch,  !- Branch 1 Name",
        "    TRANSFER Demand HX Branch 1,  !- Branch 2 Name",
        "    TRANSFER Demand HX Branch 2,  !- Branch 3 Name",
        "    TRANSFER Demand Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    TRANSFER Demand Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    TRANSFER Demand Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    TRANSFER Demand Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    TRANSFER Demand Splitter,!- Name",
        "    TRANSFER Demand Inlet Branch,  !- Inlet Branch Name",
        "    TRANSFER Demand HX Branch 1,  !- Outlet Branch 1 Name",
        "    TRANSFER Demand HX Branch 2;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    TRANSFER Demand Mixer,   !- Name",
        "    TRANSFER Demand Outlet Branch,  !- Outlet Branch Name",
        "    TRANSFER Demand HX Branch 1,  !- Inlet Branch 1 Name",
        "    TRANSFER Demand HX Branch 2;  !- Inlet Branch 2 Name",

        "  Branch,",
        "    TRANSFER Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TRANSFER Demand Inlet Pipe,  !- Component 1 Name",
        "    TRANSFER Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Demand Pipe-Load Profile Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    TRANSFER Demand Inlet Pipe,  !- Name",
        "    TRANSFER Demand Inlet Node,  !- Inlet Node Name",
        "    TRANSFER Demand Pipe-Load Profile Node;  !- Outlet Node Name",

        "  Branch,",
        "    TRANSFER Demand HX Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE Heat HX, !- Component 1 Name",
        "    TRANSFER heat Demand HX Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER heat Demand HX Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    TRANSFER Demand HX Branch 2,  !- Name",
        "    ,                        !- Pressure Drop Curve ",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE cool HX, !- Component 1 Name",
        "    TRANSFER Demand HX Cool Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Demand HX cool Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    TRANSFER Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TRANSFER Demand Outlet Pipe,  !- Component 1 Name",
        "    TRANSFER Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    TRANSFER Demand Outlet Pipe,  !- Name",
        "    TRANSFER Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    TRANSFER Demand Outlet Node;  !- Outlet Node Name",

        "  PlantLoop,",
        "    SOURCE Loop,             !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    SOURCE Loop Operation,   !- Plant Equipment Operation Scheme Name",
        "    SOURCE Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    SOURCE Supply Inlet Node,!- Plant Side Inlet Node Name",
        "    SOURCE Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    SOURCE Supply Branches,  !- Plant Side Branch List Name",
        "    SOURCE Supply Connectors,!- Plant Side Connector List Name",
        "    SOURCE Demand Inlet Node,!- Demand Side Inlet Node Name",
        "    SOURCE Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    SOURCE Demand Branches,  !- Demand Side Branch List Name",
        "    SOURCE Demand Connectors,!- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    SOURCE Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    SOURCE Loop Temp Sch,    !- Schedule Name",
        "    SOURCE Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    SOURCE Loop Setpoint Node List,  !- Name",
        "    SOURCE Supply Outlet Node,  !- Node 1 Name",
        "    SOURCE Supply Heating Outlet Node,  !- Node 2 Name",
        "    SOURCE Supply Cooling Outlet Node;  !- Node 3 Name",

        "  PlantEquipmentOperationSchemes,",
        "    SOURCE Loop Operation,   !- Name",
        "    PlantEquipmentOperation:ComponentSetpoint,  !- Control Scheme 1 Object Type",
        "    SOURCE Purchased Only,   !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:ComponentSetpoint,",
        "    SOURCE Purchased Only,   !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    SOURCE Purchased Cooling,!- Equipment 1 Name",
        "    SOURCE Supply Cooling Inlet Node,  !- Demand Calculation 1 Node Name",
        "    SOURCE Supply Cooling Outlet Node,  !- Setpoint 1 Node Name",
        "    0.003,                   !- Component 1 Flow Rate {m3/s}",
        "    Cooling,                 !- Operation 1 Type",
        "    DistrictHeating:Water,         !- Equipment 2 Object Type",
        "    SOURCE Purchased Heating,!- Equipment 2 Name",
        "    SOURCE Supply Heating Inlet Node,  !- Demand Calculation 2 Node Name",
        "    SOURCE Supply Heating Outlet Node,  !- Setpoint 2 Node Name",
        "    0.003,                   !- Component 2 Flow Rate {m3/s}",
        "    HEATINg;                 !- Operation 2 Type",

        "  PlantEquipmentList,",
        "    SOURCE cooling Plant,    !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    SOURCE Purchased Cooling;!- Equipment 1 Name",

        "  BranchList,",
        "    SOURCE Supply Branches,  !- Name",
        "    SOURCE Supply Inlet Branch,  !- Branch 1 Name",
        "    SOURCE Cooling Branch,   !- Branch 2 Name",
        "    SOURCE Heating Branch,   !- Branch 3 Name",
        "    SOURCE Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    SOURCE Supply Connectors,!- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    SOURCE Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    SOURCE Supply Mixer;     !- Connector 2 Name",

        "  Connector:Splitter,",
        "    SOURCE Supply Splitter,  !- Name",
        "    SOURCE Supply Inlet Branch,  !- Inlet Branch Name",
        "    SOURCE Cooling Branch,   !- Outlet Branch 1 Name",
        "    SOURCE Heating Branch;   !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    SOURCE Supply Mixer,     !- Name",
        "    SOURCE Supply Outlet Branch,  !- Outlet Branch Name",
        "    SOURCE Cooling Branch,   !- Inlet Branch 1 Name",
        "    SOURCE Heating Branch;   !- Inlet Branch 2 Name",

        "  Branch,",
        "    SOURCE Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    SOURCE Pump,             !- Component 1 Name",
        "    SOURCE Supply Inlet Node,!- Component 1 Inlet Node Name",
        "    SOURCE Supply Pump-district Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    SOURCE Pump,             !- Name",
        "    SOURCE Supply Inlet Node,!- Inlet Node Name",
        "    SOURCE Supply Pump-district Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance ",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    SOURCE Cooling Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictCooling,         !- Component 1 Object Type",
        "    SOURCE Purchased Cooling,!- Component 1 Name",
        "    SOURCE Supply Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Supply Cooling Outlet Node;  !- Component 1 Outlet Node Name",

        "  DistrictCooling,",
        "    SOURCE Purchased Cooling,!- Name",
        "    SOURCE Supply Cooling Inlet Node,  !- Chilled Water Inlet Node Name",
        "    SOURCE Supply Cooling Outlet Node,  !- Chilled Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Branch,",
        "    SOURCE Heating Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    SOURCE Purchased Heating,!- Component 1 Name",
        "    SOURCE Supply Heating Inlet Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Supply Heating Outlet Node;  !- Component 1 Outlet Node Name",

        "  DistrictHeating:Water,",
        "    SOURCE Purchased Heating,!- Name",
        "    SOURCE Supply Heating Inlet Node,  !- Hot Water Inlet Node Name",
        "    SOURCE Supply Heating Outlet Node,  !- Hot Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Branch,",
        "    SOURCE Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    SOURCE Supply Outlet Pipe,  !- Component 1 Name",
        "    SOURCE Supply Heating-Pipe Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    SOURCE Supply Outlet Pipe,  !- Name",
        "    SOURCE Supply Heating-Pipe Node,  !- Inlet Node Name",
        "    SOURCE Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    SOURCE Demand Branches,  !- Name",
        "    SOURCE Demand Inlet Branch,  !- Branch 1 Name",
        "    SOURCE Demand HX Branch, !- Branch 2 Name",
        "    SOURCE Demand Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    SOURCE Demand Connectors,!- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    SOURCE Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    SOURCE Demand Mixer;     !- Connector 2 Name",

        "  Connector:Splitter,",
        "    SOURCE Demand Splitter,  !- Nam",
        "    SOURCE Demand Inlet Branch,  !- Inlet Branch Name",
        "    SOURCE Demand HX Branch; !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    SOURCE Demand Mixer,     !- Name",
        "    SOURCE Demand Outlet Branch,  !- Outlet Branch Name",
        "    SOURCE Demand HX Branch; !- Inlet Branch 1 Name",

        "  Branch,",
        "    SOURCE Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    SOURCE Demand Inlet Pipe,!- Component 1 Name",
        "    SOURCE Demand Inlet Node,!- Component 1 Inlet Node Name",
        "    SOURCE Demand Pipe-HX Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    SOURCE Demand Inlet Pipe,!- Name",
        "    SOURCE Demand Inlet Node,!- Inlet Node Name",
        "    SOURCE Demand Pipe-HX Node;  !- Outlet Node Name",

        "  Branch,",
        "    SOURCE Demand HX Branch, !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    SOURCE to TRANSFER HX,   !- Component 1 Name",
        "    SOURCE Demand HX Inlet Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Demand HX Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    SOURCE Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    SOURCE Demand Outlet Pipe,  !- Component 1 Name",
        "    SOURCE Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    SOURCE Demand Outlet Pipe,  !- Name",
        "    SOURCE Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    SOURCE Demand Outlet Node;  !- Outlet Node Name",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  Schedule:Compact,",
        "    Use Cool Loop Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,25.0;       !- Field 3",

        "  Schedule:Compact,",
        "    Use Heat Loop Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,15.0;       !- Field 3",

        "  Schedule:Compact,",
        "    TRANSFER Dual Loop Hi Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,19.0;       !- Field 3",

        "  Schedule:Compact,",
        "    TRANSFER Dual Loop Lo Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,17.5;       !- Field 3",

        "  Schedule:Compact,",
        "    SOURCE Loop Temp Sch,    !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,20.0;       !- Field 3",

        "  Schedule:Compact,",
        "    AlwaysOnSchedule,        !- Name",
        "    On/Off,                  !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,1;          !- Field 3",

        "  Schedule:Compact,",
        "    Heat Load Profile 1 Load Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 12:00,0,          !- Field 3",
        "    UNTIL: 14:00,0,       !- Field 5",
        "    UNTIL: 16:00,0,       !- Field 7",
        "    UNTIL: 17:00,0,          !- Field 9",
        "    UNTIL: 20:00,0,       !- Field 11",
        "    UNTIL: 24:00,0;      !- Field 13",

        "  Schedule:Compact,",
        "    Heat Load Profile 1 Flow Frac Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,0.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Cool Load Profile 1 Load Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 2:00,-8000,       !- Field 3",
        "    UNTIL: 4:00,-8000,       !- Field 5",
        "    UNTIL: 5:00,-8000,           !- Field 7",
        "    UNTIL: 8:00,-8000,       !- Field 9",
        "    UNTIL: 12:00,-8000,     !- Field 11",
        "    UNTIL: 24:00,-8000;          !- Field 13",

        "  Schedule:Compact,",
        "    cool Load Profile 1 Flow Frac Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,1.0;        !- Field 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    OutputReportPredefined::SetPredefinedTables(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state); // establish array bounds for constructions early
    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    PlantManager::CheckIfAnyPlant(*state);
    createFacilityElectricPowerServiceObject(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.
    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    state->dataGlobal->KickOffSimulation = false;

    int EnvCount = 0;
    state->dataGlobal->WarmupFlag = true;
    bool Available(true);

    while (Available) {

        Weather::GetNextEnvironment(*state, Available, ErrorsFound);

        if (!Available) {
            break;
        }
        if (ErrorsFound) {
            break;
        }

        ++EnvCount;

        state->dataGlobal->BeginEnvrnFlag = true;
        state->dataGlobal->EndEnvrnFlag = false;
        state->dataEnvrn->EndMonthFlag = false;
        state->dataGlobal->WarmupFlag = true;
        state->dataGlobal->DayOfSim = 0;
        state->dataGlobal->DayOfSimChr = "0";

        while ((state->dataGlobal->DayOfSim < state->dataGlobal->NumOfDayInEnvrn) || (state->dataGlobal->WarmupFlag)) { // Begin day loop ...

            ++state->dataGlobal->DayOfSim;

            if (!state->dataGlobal->WarmupFlag) {
                ++state->dataEnvrn->CurrentOverallSimDay;
            }
            state->dataGlobal->BeginDayFlag = true;
            state->dataGlobal->EndDayFlag = false;

            for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...

                state->dataGlobal->BeginHourFlag = true;
                state->dataGlobal->EndHourFlag = false;

                for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour;
                     ++state->dataGlobal->TimeStep) {

                    state->dataGlobal->BeginTimeStepFlag = true;

                    // Set the End__Flag variables to true if necessary.  Note that
                    // each flag builds on the previous level.  EndDayFlag cannot be
                    // .TRUE. unless EndHourFlag is also .TRUE., etc.  Note that the
                    // EndEnvrnFlag and the EndSimFlag cannot be set during warmup.
                    // Note also that BeginTimeStepFlag, EndTimeStepFlag, and the
                    // SubTimeStepFlags can/will be set/reset in the HVAC Manager.

                    if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
                        state->dataGlobal->EndHourFlag = true;
                        if (state->dataGlobal->HourOfDay == 24) {
                            state->dataGlobal->EndDayFlag = true;
                            if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                                state->dataGlobal->EndEnvrnFlag = true;
                            }
                        }
                    }

                    Weather::ManageWeather(*state);

                    HeatBalanceManager::ManageHeatBalance(*state);

                    //  After the first iteration of HeatBalance, all the 'input' has been gotten

                    state->dataGlobal->BeginHourFlag = false;
                    state->dataGlobal->BeginDayFlag = false;
                    state->dataGlobal->BeginEnvrnFlag = false;
                    state->dataGlobal->BeginSimFlag = false;

                } // TimeStep loop

                state->dataGlobal->PreviousHour = state->dataGlobal->HourOfDay;

            } // ... End hour loop.

        } // ... End day loop.

    } // ... End environment loop.

    EXPECT_NEAR(state->dataLoopNodes->Node(4).Temp, 20.0, 0.01);
}

TEST_F(EnergyPlusFixture, PlantHXModulatedDualDeadDefectFileLo)
{

    // this unit test was devised for issue #5258 which involves control logic related to plant HX not controlling well when the setpoint cannot be
    // met this test has complete IDF input to set up a system of four plant loops taken from the PlantLoopChain* integration tests.
    // This test checks that the HX will attempt to meet setpoint of 21 when the conditioning fluid is 20 and cannot really make it to 21.  The HX
    // still heats up to 20.

    std::string const idf_objects = delimited_string({
        "Building,",
        "Plant Load Profile Example,  !- Name",
        "    0.0,                     !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.04,                    !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",

        "  Site:Location,",
        "    CHICAGO_IL_USA_WMO_725300,  !- Name",
        "    42.00,                   !- Latitude {deg}",
        "    -87.88,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO Ann Htg 99% Condns DB,  !- Name",
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
        "    0.00;                    !- Sky Clearness",

        "  RunPeriod,",
        "    Jan,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    2,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  SimulationControl,",
        "    NO,                      !- Do Zone Sizing Calculation",
        "    NO,                      !- Do System Sizing Calculation",
        "    NO,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  PlantLoop,",
        "    Use Heat Loop,           !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Use Heat Loop Operation, !- Plant Equipment Operation Scheme Name",
        "    Use Heat Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    Use Heat Supply Inlet Node,  !- Plant Side Inlet Node Name",
        "    Use Heat Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    Use Heat Supply Branches,!- Plant Side Branch List Name",
        "    Use Heat Supply Connectors,  !- Plant Side Connector List Name",
        "    Use Heat Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    Use Heat Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    Use Heat Demand Branches,!- Demand Side Branch List Name",
        "    Use Heat Demand Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Use Heat Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Use Heat Loop Temp Sch,  !- Schedule Name",
        "    Use Heat Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Use Heat Loop Setpoint Node List,  !- Name",
        "    Use Heat Supply Outlet Node;  !- Node 1 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Use Heat Loop Operation, !- Name",
        "    PlantEquipmentOperation:Uncontrolled,  !- Control Scheme 1 Object Type",
        "    USE Heat operation,      !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:Uncontrolled,",
        "    USE Heat operation,      !- Name",
        "    TRANSFER to USE Heat HX equip;  !- Equipment List Name",

        "  PlantEquipmentList,",
        "    TRANSFER to USE Heat HX equip,  !- Name",
        "    HeatExchanger:FluidToFluid,  !- Equipment 1 Object Type",
        "    TRANSFER to USE Heat HX; !- Equipment 1 Name",

        "  BranchList,",
        "    USE Heat Supply Branches,!- Name",
        "    USE Heat Supply Inlet Branch,  !- Branch 1 Name",
        "    USE Heat Heating Branch, !- Branch 2 Name",
        "    USE Heat Supply Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Heat Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Heat Supply Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Heat Supply Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Heat Supply Splitter,!- Name",
        "    USE Heat Supply Inlet Branch,  !- Inlet Branch Name",
        "    USE Heat Heating Branch; !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Heat Supply Mixer,   !- Name",
        "    USE Heat Supply Outlet Branch,  !- Outlet Branch Name",
        "    USE Heat Heating Branch; !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Heat Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    USE Heat Pump,           !- Component 1 Name",
        "    USE Heat Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Supply Pump-Heating Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    USE Heat Pump,           !- Name",
        "    USE Heat Supply Inlet Node,  !- Inlet Node Name",
        "    USE Heat Supply Pump-Heating Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    USE Heat Heating Branch, !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE Heat HX, !- Component 1 Name",
        "    USE Heat Supply Heating Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Supply Heating Outlet Node;  !- Component 1 Outlet Node Name",

        "  HeatExchanger:FluidToFluid,",
        "    TRANSFER to USE Heat HX, !- Name",
        "    AlwaysOnSchedule,        !- Availability Schedule Name",
        "    TRANSFER Heat Demand HX Inlet Node,  !- Loop Demand Side Inlet Node Name",
        "    TRANSFER Heat Demand HX Outlet Node,  !- Loop Demand Side Outlet Node Name",
        "    0.003,                   !- Loop Demand Side Design Flow Rate {m3/s}",
        "    USE Heat Supply Heating Inlet Node,  !- Loop Supply Side Inlet Node Name",
        "    USE Heat Supply Heating Outlet Node,  !- Loop Supply Side Outlet Node Name",
        "    0.003,                   !- Loop Supply Side Design Flow Rate {m3/s}",
        "    CrossFlowBothUnMixed,    !- Heat Exchange Model Type",
        "    15000,                   !- Heat Exchanger U-Factor Times Area Value {W/k}",
        "    HeatingSetpointModulated,!- Control Type",
        "    Use heat Supply Outlet Node,  !- Heat Exchanger Setpoint Node Name",
        "    0.2,                     !- Minimum Temperature Difference to Activate Heat Exchanger {deltaC}",
        "   LoopToLoop;              !- Heat Transfer Metering End Use Type",

        "  Branch,",
        "    USE Heat Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Heat Supply Outlet Pipe,  !- Component 1 Name",
        "    USE Heat Supply Heating-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Heat Supply Outlet Pipe,  !- Name",
        "    USE Heat Supply Heating-Pipe Node,  !- Inlet Node Name",
        "    USE Heat Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    USE Heat Demand Branches,!- Name",
        "    USE Heat Demand Inlet Branch,  !- Branch 1 Name",
        "    USE Heat Load Profile Branch 1,  !- Branch 2 Name",
        "    USE Heat Demand Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Heat Demand Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Heat Demand Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Heat Demand Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Heat Demand Splitter,!- ",
        "    USE Heat Demand Inlet Branch,  !- Inlet Branch Name",
        "    USE Heat Load Profile Branch 1;  !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Heat Demand Mixer,   !- Name",
        "    USE Heat Demand Outlet Branch,  !- Outlet Branch Name",
        "    USE Heat Load Profile Branch 1;  !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Heat Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Heat Demand Inlet Pipe,  !- Component 1 Name",
        "    USE Heat Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Demand Pipe-Load Profile Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Heat Demand Inlet Pipe,  !- Name",
        "    USE Heat Demand Inlet Node,  !- Inlet Node Name",
        "    USE Heat Demand Pipe-Load Profile Node;  !- Outlet Node Name",

        "  Branch,",
        "    USE Heat Load Profile Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    LoadProfile:Plant,       !- Component 1 Object Type",
        "    Heat Load Profile 1,     !- Component 1 Name",
        "    Heat Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
        "    Heat Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

        "  LoadProfile:Plant,",
        "    Heat Load Profile 1,     !- Name",
        "    Heat Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
        "    Heat Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
        "    Heat Load Profile 1 Load Schedule,  !- Load Schedule Name",
        "    0.002,                   !- Peak Flow Rate {m3/s}",
        "    Heat Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

        "  Branch,",
        "    USE Heat Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Heat Demand Outlet Pipe,  !- Component 1 Name",
        "    USE Heat Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Heat Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Heat Demand Outlet Pipe,  !- Name",
        "    USE Heat Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    USE Heat Demand Outlet Node;  !- Outlet Node Name",

        "  PlantLoop,",
        "    Use Cool Loop,           !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Use Cool Loop Operation, !- Plant Equipment Operation Scheme Name",
        "    Use Cool Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    Use Cool Supply Inlet Node,  !- Plant Side Inlet Node Name",
        "    Use Cool Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    Use Cool Supply Branches,!- Plant Side Branch List Name",
        "    Use Cool Supply Connectors,  !- Plant Side Connector List Name",
        "    Use Cool Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    Use Cool Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    Use Cool Demand Branches,!- Demand Side Branch List Name",
        "    Use Cool Demand Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Use Cool Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Use Cool Loop Temp Sch,  !- Schedule Name",
        "    Use Cool Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Use Cool Loop Setpoint Node List,  !- Name",
        "    Use Cool Supply Outlet Node;  !- Node 1 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Use Cool Loop Operation, !- Name",
        "    PlantEquipmentOperation:Uncontrolled,  !- Control Scheme 1 Object Type",
        "    USE Cool operation,      !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:Uncontrolled,",
        "    USE Cool operation,      !- Name",
        "    TRANSFER to USE Cool HX equip;  !- Equipment List Name",

        "  PlantEquipmentList,",
        "    TRANSFER to USE Cool HX equip,  !- Name",
        "    HeatExchanger:FluidToFluid,  !- Equipment 1 Object Type",
        "    TRANSFER to USE Cool HX; !- Equipment 1 Name",

        "  BranchList,",
        "    USE Cool Supply Branches,!- Name",
        "    USE Cool Supply Inlet Branch,  !- Branch 1 Name",
        "    USE Cooling Branch,      !- Branch 2 Name",
        "    USE Cool Supply Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Cool Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Cool Supply Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Cool Supply Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Cool Supply Splitter,!- Name",
        "    USE Cool Supply Inlet Branch,  !- Inlet Branch Name",
        "    USE Cooling Branch;      !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Cool Supply Mixer,   !- Name",
        "    USE Cool Supply Outlet Branch,  !- Outlet Branch Name",
        "    USE Cooling Branch;      !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Cool Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    USE Cool Pump,           !- Component 1 Name",
        "    USE Cool Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Supply Pump-Heating Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    USE Cool Pump,           !- Name",
        "    USE Cool Supply Inlet Node,  !- Inlet Node Name",
        "    USE Cool Supply Pump-Heating Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    USE Cooling Branch,      !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE Cool HX, !- Component 1 Name",
        "    USE Cool Supply Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Supply Cooling Outlet Node;  !- Component 1 Outlet Node Name",

        "  HeatExchanger:FluidToFluid,",
        "    TRANSFER to USE Cool HX, !- Name",
        "    AlwaysOnSchedule,        !- Availability Schedule Name",
        "    TRANSFER Demand HX Cool Inlet Node,  !- Loop Demand Side Inlet Node Name",
        "    TRANSFER Demand HX Cool Outlet Node,  !- Loop Demand Side Outlet Node Name",
        "    0.003,                   !- Loop Demand Side Design Flow Rate {m3/s}",
        "    USE Cool Supply Cooling Inlet Node,  !- Loop Supply Side Inlet Node Name",
        "    USE Cool Supply Cooling Outlet Node,  !- Loop Supply Side Outlet Node Name",
        "    0.003,                   !- Loop Supply Side Design Flow Rate {m3/s}",
        "    CrossFlowBothUnMixed,    !- Heat Exchange Model Type",
        "    15000,                   !- Heat Exchanger U-Factor Times Area Value {W/k}",
        "    CoolingSetpointModulated,!- Control Type",
        "    Use Cool Supply Outlet Node,  !- Heat Exchanger Setpoint Node Name",
        "    0.2,                     !- Minimum Temperature Difference to Activate Heat Exchanger {deltaC}",
        "    LoopToLoop;              !- Heat Transfer Metering End Use Type",

        "  Branch,",
        "    USE Cool Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Cool Supply Outlet Pipe,  !- Component 1 Name",
        "    USE Cool Supply Heating-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Cool Supply Outlet Pipe,  !- Name",
        "    USE Cool Supply Heating-Pipe Node,  !- Inlet Node Name",
        "    USE Cool Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    USE Cool Demand Branches,!- Name",
        "    USE Cool Demand Inlet Branch,  !- Branch 1 Name",
        "    USE Cool Load Profile Branch 1,  !- Branch 2 Name",
        "    USE Cool Demand Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    USE Cool Demand Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    USE Cool Demand Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    USE Cool Demand Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    USE Cool Demand Splitter,!- Name",
        "    USE Cool Demand Inlet Branch,  !- Inlet Branch Name",
        "    USE Cool Load Profile Branch 1;  !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    USE Cool Demand Mixer,   !- Name",
        "    USE Cool Demand Outlet Branch,  !- Outlet Branch Name",
        "    USE Cool Load Profile Branch 1;  !- Inlet Branch 1 Name",

        "  Branch,",
        "    USE Cool Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Cool Demand Inlet Pipe,  !- Component 1 Name",
        "    USE Cool Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Demand Pipe-Load Profile Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Cool Demand Inlet Pipe,  !- Name",
        "    USE Cool Demand Inlet Node,  !- Inlet Node Name",
        "    USE Cool Demand Pipe-Load Profile Node;  !- Outlet Node Name",

        "  Branch,",
        "    USE Cool Load Profile Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    LoadProfile:Plant,       !- Component 1 Object Type",
        "    Cool Load Profile 1,     !- Component 1 Name",
        "    Demand Cool Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
        "    Demand Cool Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

        "  LoadProfile:Plant,",
        "    Cool Load Profile 1,     !- Name",
        "    Demand Cool Load Profile 1 Inlet Node,  !- Inlet Node Name",
        "    Demand Cool Load Profile 1 Outlet Node,  !- Outlet Node Name",
        "    Cool Load Profile 1 Load Schedule,  !- Load Schedule Name",
        "    0.002,                   !- Peak Flow Rate {m3/s}",
        "    Cool Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

        "  Branch,",
        "    USE Cool Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    USE Cool Demand Outlet Pipe,  !- Component 1 Name",
        "    USE Cool Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    USE Cool Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    USE Cool Demand Outlet Pipe,  !- Name",
        "    USE Cool Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    USE Cool Demand Outlet Node;  !- Outlet Node Name",

        "  PlantLoop,",
        "    TRANSFER Loop,           !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    TRANSFER Loop Operation, !- Plant Equipment Operation Scheme Name",
        "    TRANSFER Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    TRANSFER Supply Inlet Node,  !- Plant Side Inlet Node Name",
        "    TRANSFER Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    TRANSFER Supply Branches,!- Plant Side Branch List Name",
        "    TRANSFER Supply Connectors,  !- Plant Side Connector List Name",
        "    TRANSFER Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    TRANSFER Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    TRANSFER Demand Branches,!- Demand Side Branch List Name",
        "    TRANSFER Demand Connectors,  !- Demand Side Connector List Name",
        "    OPTIMAL,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetpointDeadband,    !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled:dualSetpoint,",
        "    TRANSFER Loop Dual Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    TRANSFER Dual Loop Hi Temp Sch,  !- High Setpoint Schedule Name",
        "    TRANSFER Dual Loop Lo Temp Sch,  !- Low Setpoint Schedule Name",
        "    TRANSFER Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    TRANSFER Loop Setpoint Node List,  !- Name",
        "    TRANSFER Supply Outlet Node;  !- Node 1 Name",

        "  PlantEquipmentOperationSchemes,",
        "    TRANSFER Loop Operation, !- Name",
        "    PlantEquipmentOperation:Uncontrolled,  !- Control Scheme 1 Object Type",
        "    TRANSFER HX,             !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:Uncontrolled,",
        "    TRANSFER HX,             !- Name",
        "    TRANSFER Plant;          !- Equipment List Name",

        "  PlantEquipmentList,",
        "    TRANSFER Plant,          !- Name",
        "    HeatExchanger:FluidToFluid,  !- Equipment 1 Object Type",
        "    SOURCE to TRANSFER HX;   !- Equipment 1 Name",

        "  BranchList,",
        "    TRANSFER Supply Branches,!- Name",
        "    TRANSFER Supply Inlet Branch,  !- Branch 1 Name",
        "    TRANSFER HX Branch,      !- Branch 2 Name",
        "    TRANSFER Supply Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    TRANSFER Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    TRANSFER Supply Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    TRANSFER Supply Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    TRANSFER Supply Splitter,!- Name",
        "    TRANSFER Supply Inlet Branch,  !- Inlet Branch Name",
        "    TRANSFER HX Branch;      !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    TRANSFER Supply Mixer,   !- Name",
        "    TRANSFER Supply Outlet Branch,  !- Outlet Branch Name",
        "    TRANSFER HX Branch;      !- Inlet Branch 1 Name",

        "  Branch,",
        "    TRANSFER Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    TRANSFER Pump,           !- Component 1 Name",
        "    TRANSFER Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Supply Pump-Heating Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    TRANSFER Pump,           !- Name",
        "    TRANSFER Supply Inlet Node,  !- Inlet Node Name",
        "    TRANSFER Supply Pump-Heating Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    TRANSFER HX Branch,      !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    SOURCE to TRANSFER HX,   !- Component 1 Name",
        "    TRANSFER HX Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER HX Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  HeatExchanger:FluidToFluid,",
        "    SOURCE to TRANSFER HX,   !- Name",
        "    AlwaysOnSchedule,        !- Availability Schedule Name",
        "    SOURCE Demand HX Inlet Node,  !- Loop Demand Side Inlet Node Name",
        "    SOURCE Demand HX Outlet Node,  !- Loop Demand Side Outlet Node Name",
        "    0.003,                   !- Loop Demand Side Design Flow Rate {m3/s}",
        "    TRANSFER HX Supply Inlet Node,  !- Loop Supply Side Inlet Node Name",
        "    TRANSFER HX Supply Outlet Node,  !- Loop Supply Side Outlet Node Name",
        "    0.003,                   !- Loop Supply Side Design Flow Rate {m3/s}",
        "    CrossFlowBothUnMixed,    !- Heat Exchange Model Type",
        "    15000,                   !- Heat Exchanger U-Factor Times Area Value {W/k}",
        "    DualDeadbandSetpointModulated,  !- Control Type",
        "    TRANSFER Supply Outlet Node,  !- Heat Exchanger Setpoint Node Name",
        "    0.2,                     !- Minimum Temperature Difference to Activate Heat Exchanger {deltaC}",
        "    LoopToLoop;              !- Heat Transfer Metering End Use Type",

        "  Branch,",
        "    TRANSFER Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TRANSFER Supply Outlet Pipe,  !- Component 1 Name",
        "    TRANSFER Supply HX-Pipe Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    TRANSFER Supply Outlet Pipe,  !- Name",
        "    TRANSFER Supply HX-Pipe Node,  !- Inlet Node Name",
        "    TRANSFER Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    TRANSFER Demand Branches,!- Name",
        "    TRANSFER Demand Inlet Branch,  !- Branch 1 Name",
        "    TRANSFER Demand HX Branch 1,  !- Branch 2 Name",
        "    TRANSFER Demand HX Branch 2,  !- Branch 3 Name",
        "    TRANSFER Demand Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    TRANSFER Demand Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    TRANSFER Demand Splitter,!- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    TRANSFER Demand Mixer;   !- Connector 2 Name",

        "  Connector:Splitter,",
        "    TRANSFER Demand Splitter,!- Name",
        "    TRANSFER Demand Inlet Branch,  !- Inlet Branch Name",
        "    TRANSFER Demand HX Branch 1,  !- Outlet Branch 1 Name",
        "    TRANSFER Demand HX Branch 2;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    TRANSFER Demand Mixer,   !- Name",
        "    TRANSFER Demand Outlet Branch,  !- Outlet Branch Name",
        "    TRANSFER Demand HX Branch 1,  !- Inlet Branch 1 Name",
        "    TRANSFER Demand HX Branch 2;  !- Inlet Branch 2 Name",

        "  Branch,",
        "    TRANSFER Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TRANSFER Demand Inlet Pipe,  !- Component 1 Name",
        "    TRANSFER Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Demand Pipe-Load Profile Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    TRANSFER Demand Inlet Pipe,  !- Name",
        "    TRANSFER Demand Inlet Node,  !- Inlet Node Name",
        "    TRANSFER Demand Pipe-Load Profile Node;  !- Outlet Node Name",

        "  Branch,",
        "    TRANSFER Demand HX Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE Heat HX, !- Component 1 Name",
        "    TRANSFER heat Demand HX Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER heat Demand HX Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    TRANSFER Demand HX Branch 2,  !- Name",
        "    ,                        !- Pressure Drop Curve ",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    TRANSFER to USE cool HX, !- Component 1 Name",
        "    TRANSFER Demand HX Cool Inlet Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Demand HX cool Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    TRANSFER Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TRANSFER Demand Outlet Pipe,  !- Component 1 Name",
        "    TRANSFER Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    TRANSFER Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    TRANSFER Demand Outlet Pipe,  !- Name",
        "    TRANSFER Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    TRANSFER Demand Outlet Node;  !- Outlet Node Name",

        "  PlantLoop,",
        "    SOURCE Loop,             !- Name",
        "    WATER,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    SOURCE Loop Operation,   !- Plant Equipment Operation Scheme Name",
        "    SOURCE Supply Outlet Node,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    3,                       !- Minimum Loop Temperature {C}",
        "    0.003,                   !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    1.0,                     !- Plant Loop Volume {m3}",
        "    SOURCE Supply Inlet Node,!- Plant Side Inlet Node Name",
        "    SOURCE Supply Outlet Node,  !- Plant Side Outlet Node Name",
        "    SOURCE Supply Branches,  !- Plant Side Branch List Name",
        "    SOURCE Supply Connectors,!- Plant Side Connector List Name",
        "    SOURCE Demand Inlet Node,!- Demand Side Inlet Node Name",
        "    SOURCE Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    SOURCE Demand Branches,  !- Demand Side Branch List Name",
        "    SOURCE Demand Connectors,!- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    SOURCE Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    SOURCE Loop Temp Sch,    !- Schedule Name",
        "    SOURCE Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    SOURCE Loop Setpoint Node List,  !- Name",
        "    SOURCE Supply Outlet Node,  !- Node 1 Name",
        "    SOURCE Supply Heating Outlet Node,  !- Node 2 Name",
        "    SOURCE Supply Cooling Outlet Node;  !- Node 3 Name",

        "  PlantEquipmentOperationSchemes,",
        "    SOURCE Loop Operation,   !- Name",
        "    PlantEquipmentOperation:ComponentSetpoint,  !- Control Scheme 1 Object Type",
        "    SOURCE Purchased Only,   !- Control Scheme 1 Name",
        "    AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:ComponentSetpoint,",
        "    SOURCE Purchased Only,   !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    SOURCE Purchased Cooling,!- Equipment 1 Name",
        "    SOURCE Supply Cooling Inlet Node,  !- Demand Calculation 1 Node Name",
        "    SOURCE Supply Cooling Outlet Node,  !- Setpoint 1 Node Name",
        "    0.003,                   !- Component 1 Flow Rate {m3/s}",
        "    Cooling,                 !- Operation 1 Type",
        "    DistrictHeating:Water,         !- Equipment 2 Object Type",
        "    SOURCE Purchased Heating,!- Equipment 2 Name",
        "    SOURCE Supply Heating Inlet Node,  !- Demand Calculation 2 Node Name",
        "    SOURCE Supply Heating Outlet Node,  !- Setpoint 2 Node Name",
        "    0.003,                   !- Component 2 Flow Rate {m3/s}",
        "    HEATINg;                 !- Operation 2 Type",

        "  PlantEquipmentList,",
        "    SOURCE cooling Plant,    !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    SOURCE Purchased Cooling;!- Equipment 1 Name",

        "  BranchList,",
        "    SOURCE Supply Branches,  !- Name",
        "    SOURCE Supply Inlet Branch,  !- Branch 1 Name",
        "    SOURCE Cooling Branch,   !- Branch 2 Name",
        "    SOURCE Heating Branch,   !- Branch 3 Name",
        "    SOURCE Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    SOURCE Supply Connectors,!- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    SOURCE Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    SOURCE Supply Mixer;     !- Connector 2 Name",

        "  Connector:Splitter,",
        "    SOURCE Supply Splitter,  !- Name",
        "    SOURCE Supply Inlet Branch,  !- Inlet Branch Name",
        "    SOURCE Cooling Branch,   !- Outlet Branch 1 Name",
        "    SOURCE Heating Branch;   !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    SOURCE Supply Mixer,     !- Name",
        "    SOURCE Supply Outlet Branch,  !- Outlet Branch Name",
        "    SOURCE Cooling Branch,   !- Inlet Branch 1 Name",
        "    SOURCE Heating Branch;   !- Inlet Branch 2 Name",

        "  Branch,",
        "    SOURCE Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    SOURCE Pump,             !- Component 1 Name",
        "    SOURCE Supply Inlet Node,!- Component 1 Inlet Node Name",
        "    SOURCE Supply Pump-district Node;  !- Component 1 Outlet Node Name",

        "  Pump:VariableSpeed,",
        "    SOURCE Pump,             !- Name",
        "    SOURCE Supply Inlet Node,!- Inlet Node Name",
        "    SOURCE Supply Pump-district Node,  !- Outlet Node Name",
        "    0.005,                   !- Rated Flow Rate {m3/s}",
        "    1,                       !- Rated Pump Head {Pa}",
        "    0.01,                    !- Rated Power Consumption {W}",
        "    0.87,                    !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance ",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  Branch,",
        "    SOURCE Cooling Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictCooling,         !- Component 1 Object Type",
        "    SOURCE Purchased Cooling,!- Component 1 Name",
        "    SOURCE Supply Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Supply Cooling Outlet Node;  !- Component 1 Outlet Node Name",

        "  DistrictCooling,",
        "    SOURCE Purchased Cooling,!- Name",
        "    SOURCE Supply Cooling Inlet Node,  !- Chilled Water Inlet Node Name",
        "    SOURCE Supply Cooling Outlet Node,  !- Chilled Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Branch,",
        "    SOURCE Heating Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    SOURCE Purchased Heating,!- Component 1 Name",
        "    SOURCE Supply Heating Inlet Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Supply Heating Outlet Node;  !- Component 1 Outlet Node Name",

        "  DistrictHeating:Water,",
        "    SOURCE Purchased Heating,!- Name",
        "    SOURCE Supply Heating Inlet Node,  !- Hot Water Inlet Node Name",
        "    SOURCE Supply Heating Outlet Node,  !- Hot Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Branch,",
        "    SOURCE Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    SOURCE Supply Outlet Pipe,  !- Component 1 Name",
        "    SOURCE Supply Heating-Pipe Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    SOURCE Supply Outlet Pipe,  !- Name",
        "    SOURCE Supply Heating-Pipe Node,  !- Inlet Node Name",
        "    SOURCE Supply Outlet Node;  !- Outlet Node Name",

        "  BranchList,",
        "    SOURCE Demand Branches,  !- Name",
        "    SOURCE Demand Inlet Branch,  !- Branch 1 Name",
        "    SOURCE Demand HX Branch, !- Branch 2 Name",
        "    SOURCE Demand Outlet Branch;  !- Branch 3 Name",

        "  ConnectorList,",
        "    SOURCE Demand Connectors,!- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    SOURCE Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    SOURCE Demand Mixer;     !- Connector 2 Name",

        "  Connector:Splitter,",
        "    SOURCE Demand Splitter,  !- Nam",
        "    SOURCE Demand Inlet Branch,  !- Inlet Branch Name",
        "    SOURCE Demand HX Branch; !- Outlet Branch 1 Name",

        "  Connector:Mixer,",
        "    SOURCE Demand Mixer,     !- Name",
        "    SOURCE Demand Outlet Branch,  !- Outlet Branch Name",
        "    SOURCE Demand HX Branch; !- Inlet Branch 1 Name",

        "  Branch,",
        "    SOURCE Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    SOURCE Demand Inlet Pipe,!- Component 1 Name",
        "    SOURCE Demand Inlet Node,!- Component 1 Inlet Node Name",
        "    SOURCE Demand Pipe-HX Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    SOURCE Demand Inlet Pipe,!- Name",
        "    SOURCE Demand Inlet Node,!- Inlet Node Name",
        "    SOURCE Demand Pipe-HX Node;  !- Outlet Node Name",

        "  Branch,",
        "    SOURCE Demand HX Branch, !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    HeatExchanger:FluidToFluid,  !- Component 1 Object Type",
        "    SOURCE to TRANSFER HX,   !- Component 1 Name",
        "    SOURCE Demand HX Inlet Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Demand HX Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    SOURCE Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    SOURCE Demand Outlet Pipe,  !- Component 1 Name",
        "    SOURCE Demand Load Profile-Pipe Node,  !- Component 1 Inlet Node Name",
        "    SOURCE Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    SOURCE Demand Outlet Pipe,  !- Name",
        "    SOURCE Demand Load Profile-Pipe Node,  !- Inlet Node Name",
        "    SOURCE Demand Outlet Node;  !- Outlet Node Name",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  Schedule:Compact,",
        "    Use Cool Loop Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,25.0;       !- Field 3",

        "  Schedule:Compact,",
        "    Use Heat Loop Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,15.0;       !- Field 3",

        "  Schedule:Compact,",
        "    TRANSFER Dual Loop Hi Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,22.5;       !- Field 3",

        "  Schedule:Compact,",
        "    TRANSFER Dual Loop Lo Temp Sch,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,21.0;       !- Field 3",

        "  Schedule:Compact,",
        "    SOURCE Loop Temp Sch,    !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,20.0;       !- Field 3",

        "  Schedule:Compact,",
        "    AlwaysOnSchedule,        !- Name",
        "    On/Off,                  !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,1;          !- Field 3",

        "  Schedule:Compact,",
        "    Heat Load Profile 1 Load Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 12:00,8000,          !- Field 3",
        "    UNTIL: 14:00,8000,       !- Field 5",
        "    UNTIL: 16:00,8000,       !- Field 7",
        "    UNTIL: 17:00,8000,          !- Field 9",
        "    UNTIL: 20:00,8000,       !- Field 11",
        "    UNTIL: 24:00,8000;      !- Field 13",

        "  Schedule:Compact,",
        "    Heat Load Profile 1 Flow Frac Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Cool Load Profile 1 Load Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 2:00,0.0,       !- Field 3",
        "    UNTIL: 4:00,0.0,       !- Field 5",
        "    UNTIL: 5:00,0.0,           !- Field 7",
        "    UNTIL: 8:00,0.0,       !- Field 9",
        "    UNTIL: 12:00,0.0,     !- Field 11",
        "    UNTIL: 24:00,0.0;          !- Field 13",

        "  Schedule:Compact,",
        "    cool Load Profile 1 Flow Frac Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,0.0;        !- Field 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    OutputReportPredefined::SetPredefinedTables(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state); // establish array bounds for constructions early
    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    PlantManager::CheckIfAnyPlant(*state);
    createFacilityElectricPowerServiceObject(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.
    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    state->dataGlobal->KickOffSimulation = false;

    int EnvCount = 0;
    state->dataGlobal->WarmupFlag = true;
    bool Available(true);

    while (Available) {

        Weather::GetNextEnvironment(*state, Available, ErrorsFound);

        if (!Available) {
            break;
        }
        if (ErrorsFound) {
            break;
        }

        ++EnvCount;

        state->dataGlobal->BeginEnvrnFlag = true;
        state->dataGlobal->EndEnvrnFlag = false;
        state->dataEnvrn->EndMonthFlag = false;
        state->dataGlobal->WarmupFlag = true;
        state->dataGlobal->DayOfSim = 0;
        state->dataGlobal->DayOfSimChr = "0";

        while ((state->dataGlobal->DayOfSim < state->dataGlobal->NumOfDayInEnvrn) || (state->dataGlobal->WarmupFlag)) { // Begin day loop ...

            ++state->dataGlobal->DayOfSim;

            if (!state->dataGlobal->WarmupFlag) {
                ++state->dataEnvrn->CurrentOverallSimDay;
            }
            state->dataGlobal->BeginDayFlag = true;
            state->dataGlobal->EndDayFlag = false;

            for (state->dataGlobal->HourOfDay = 1; state->dataGlobal->HourOfDay <= 24; ++state->dataGlobal->HourOfDay) { // Begin hour loop ...

                state->dataGlobal->BeginHourFlag = true;
                state->dataGlobal->EndHourFlag = false;

                for (state->dataGlobal->TimeStep = 1; state->dataGlobal->TimeStep <= state->dataGlobal->TimeStepsInHour;
                     ++state->dataGlobal->TimeStep) {

                    state->dataGlobal->BeginTimeStepFlag = true;

                    // Set the End__Flag variables to true if necessary.  Note that
                    // each flag builds on the previous level.  EndDayFlag cannot be
                    // .TRUE. unless EndHourFlag is also .TRUE., etc.  Note that the
                    // EndEnvrnFlag and the EndSimFlag cannot be set during warmup.
                    // Note also that BeginTimeStepFlag, EndTimeStepFlag, and the
                    // SubTimeStepFlags can/will be set/reset in the HVAC Manager.

                    if (state->dataGlobal->TimeStep == state->dataGlobal->TimeStepsInHour) {
                        state->dataGlobal->EndHourFlag = true;
                        if (state->dataGlobal->HourOfDay == 24) {
                            state->dataGlobal->EndDayFlag = true;
                            if ((!state->dataGlobal->WarmupFlag) && (state->dataGlobal->DayOfSim == state->dataGlobal->NumOfDayInEnvrn)) {
                                state->dataGlobal->EndEnvrnFlag = true;
                            }
                        }
                    }

                    Weather::ManageWeather(*state);

                    HeatBalanceManager::ManageHeatBalance(*state);

                    //  After the first iteration of HeatBalance, all the 'input' has been gotten

                    state->dataGlobal->BeginHourFlag = false;
                    state->dataGlobal->BeginDayFlag = false;
                    state->dataGlobal->BeginEnvrnFlag = false;
                    state->dataGlobal->BeginSimFlag = false;

                } // TimeStep loop

                state->dataGlobal->PreviousHour = state->dataGlobal->HourOfDay;

            } // ... End hour loop.

        } // ... End day loop.

    } // ... End environment loop.
      //

    // Index 4 corresponds to NodeNumOut of "TRANSFER SUPPLY OUTLET PIPE", which is a component of
    // "TRANSFER SUPPLY OUTLET BRANCH" in "TRANSFER SUPPLY BRANCHES" OF "TRANSFER LOOP" (part of PlantLoop data)
    EXPECT_NEAR(state->dataLoopNodes->Node(4).Temp, 20.0, 0.01);
}

TEST_F(EnergyPlusFixture, PlantHXControlWithFirstHVACIteration)
{
    // get availability schedule to work
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    // this unit test is for issue #4959.  Added FirstHVACIteration to simulate and control routines
    // unit test checks that the change to logic for #4959 does work to affect node mass flow rate.  The conditions are set up such that the demand
    // side inlet is too warm to cool the supply side, so previous behavior would shut down flow.  Now if firstHVACIteration is true is should set
    // flow request at the design max and if it is false set the flow request to 0.0.  The intent is setup enough structures to be able to call the
    // routine ControlFluidHeatExchanger and check its behavior with FirstHVACIteration.

    state->dataPlantHXFluidToFluid->FluidHX.allocate(1);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataPlantHXFluidToFluid->FluidHX(1).availSched = Sched::GetScheduleAlwaysOn(*state);

    // setup four plant nodes for HX
    state->dataLoopNodes->Node.allocate(4);
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.inletNodeNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.outletNodeNum = 3;
    state->dataLoopNodes->Node(1).Temp = 18.0;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 2.0;
    state->dataLoopNodes->Node(3).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(3).MassFlowRateMax = 2.0;

    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.InletTemp = 18.0;

    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.inletNodeNum = 2;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.outletNodeNum = 4;
    state->dataLoopNodes->Node(2).Temp = 19.0;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(2).MassFlowRateMax = 2.0;
    state->dataLoopNodes->Node(4).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(4).MassFlowRateMax = 2.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.InletTemp = 19.0;

    state->dataPlantHXFluidToFluid->FluidHX(1).controlMode = PlantHeatExchangerFluidToFluid::ControlType::CoolingDifferentialOnOff;
    state->dataPlantHXFluidToFluid->FluidHX(1).MinOperationTemp = 10.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).MaxOperationTemp = 30.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).Name = "Test HX";

    // setup two plant loops, need for SetComponenetFlowRate
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataPlnt->PlantLoop(1).Name = "HX supply side loop ";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataPlantHXFluidToFluid->FluidHX(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.inletNodeNum;

    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.loopNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.branchNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop);

    state->dataPlnt->PlantLoop(2).Name = "HX demand side loop ";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataPlantHXFluidToFluid->FluidHX(1).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.inletNodeNum;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.loopNum = 2;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.branchNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.compNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.MassFlowRateMax = 2.0;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop);

    // when FirstHVACIteration is true, mass flow should match design max
    bool testFirstHVACIteration = true;
    state->dataPlantHXFluidToFluid->FluidHX(1).control(*state, -1000.0, testFirstHVACIteration);

    EXPECT_NEAR(state->dataLoopNodes->Node(2).MassFlowRate, state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.MassFlowRateMax, 0.001);

    // when FirstHVACIteration is false, mass flow should be zero
    testFirstHVACIteration = false;
    state->dataPlantHXFluidToFluid->FluidHX(1).control(*state, -1000.0, testFirstHVACIteration);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).MassFlowRate, 0.0, 0.001);
}

TEST_F(EnergyPlusFixture, PlantHXControl_CoolingSetpointOnOffWithComponentOverride)
{
    // this unit test is for issue #5626.  Fixed logic for CoolingSetpointOnOffWithComponentOverride.
    // unit test checks that the change for #5626 adjusts the temperature value used in central plant dispatch routines by the tolerance value.

    // get availability schedule to work
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    state->dataPlantHXFluidToFluid->FluidHX.allocate(1);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataPlantHXFluidToFluid->FluidHX(1).availSched = Sched::GetScheduleAlwaysOn(*state);

    // setup four plant nodes for HX
    state->dataLoopNodes->Node.allocate(6);
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.inletNodeNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.outletNodeNum = 3;
    state->dataPlantHXFluidToFluid->FluidHX(1).SetPointNodeNum = 3;
    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.MassFlowRateMax = 2.0;

    state->dataLoopNodes->Node(1).Temp = 18.0;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 2.0;
    state->dataLoopNodes->Node(3).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(3).MassFlowRateMax = 2.0;

    state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.InletTemp = 18.0;

    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.inletNodeNum = 2;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.outletNodeNum = 4;
    state->dataLoopNodes->Node(2).Temp = 19.0;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(2).MassFlowRateMax = 2.0;
    state->dataLoopNodes->Node(4).MassFlowRateMaxAvail = 2.0;
    state->dataLoopNodes->Node(4).MassFlowRateMax = 2.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.InletTemp = 19.0;

    state->dataPlantHXFluidToFluid->FluidHX(1).controlMode = PlantHeatExchangerFluidToFluid::ControlType::CoolingSetPointOnOffWithComponentOverride;
    state->dataPlantHXFluidToFluid->FluidHX(1).MinOperationTemp = 10.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).MaxOperationTemp = 30.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).Name = "Test HX";

    // setup two plant loops, need for SetComponenetFlowRate
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    // loop 1 is like a chilled water loop, supply side of HX, two branches on supply side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp(1).NodeNumIn = 5;

    // loop 2 is like a condenser loop, demand side of HX
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    state->dataPlnt->PlantLoop(1).Name = "HX supply side loop ";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name =
        state->dataPlantHXFluidToFluid->FluidHX(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn =
        state->dataPlantHXFluidToFluid->FluidHX(1).SupplySideLoop.inletNodeNum;

    state->dataPlnt->PlantLoop(2).Name = "HX demand side loop ";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataPlantHXFluidToFluid->FluidHX(1).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::FluidToFluidPlantHtExchg;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.inletNodeNum;

    state->dataPlantHXFluidToFluid->FluidHX(1).DemandSideLoop.MassFlowRateMax = 2.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).ControlSignalTemp = PlantHeatExchangerFluidToFluid::CtrlTempType::DryBulbTemperature;
    state->dataPlantHXFluidToFluid->FluidHX(1).OtherCompSupplySideLoop.inletNodeNum = 5;
    state->dataPlantHXFluidToFluid->FluidHX(1).OtherCompSupplySideLoop.loopNum = 1;
    state->dataPlantHXFluidToFluid->FluidHX(1).OtherCompSupplySideLoop.loopSideNum = DataPlant::LoopSideLocation::Supply;
    state->dataPlantHXFluidToFluid->FluidHX(1).OtherCompSupplySideLoop.branchNum = 2;
    state->dataPlantHXFluidToFluid->FluidHX(1).OtherCompSupplySideLoop.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantHXFluidToFluid->FluidHX(1).OtherCompSupplySideLoop);

    state->dataPlantHXFluidToFluid->NumberOfPlantFluidHXs = 1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp(1).HowLoadServed = DataPlant::HowMet::ByNominalCap;
    state->dataEnvrn->OutDryBulbTemp = 9.0;
    state->dataPlantHXFluidToFluid->FluidHX(1).TempControlTol = 0.0;
    state->dataLoopNodes->Node(3).TempSetPoint = 11.0;

    // now call the init routine
    state->dataPlantHXFluidToFluid->FluidHX(1).initialize(*state);

    // check value in FreeCoolCntrlMinCntrlTemp
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp(1).FreeCoolCntrlMinCntrlTemp, 11.0, 0.001);

    // change the tolerance and check the result, issue 5626 fix subtracts tolerance
    state->dataPlantHXFluidToFluid->FluidHX(1).TempControlTol = 1.5;
    state->dataPlantHXFluidToFluid->FluidHX(1).initialize(*state);
}

} // namespace EnergyPlus
