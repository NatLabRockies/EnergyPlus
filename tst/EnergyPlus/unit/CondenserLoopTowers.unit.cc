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

// EnergyPlus::CondenserLoopTowers unit tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/CondenserLoopTowers.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, CondenserLoopTowers_MerkelNoCooling)
{
    std::string const idf_objects = delimited_string(
        {"  Site:Location,",
         "    USA IL-CHICAGO-OHARE,    !- Name",
         "    41.77,                   !- Latitude {deg}",
         "    -87.75,                  !- Longitude {deg}",
         "    -6.00,                   !- Time Zone {hr}",
         "    190;                     !- Elevation {m}",

         "  SizingPeriod:DesignDay,",
         "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
         "    1,                       !- Month",
         "    21,                      !- Day of Month",
         "    WinterDesignDay,         !- Day Type",
         "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
         "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
         "    Wetbulb,                 !- Humidity Condition Type",
         "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

         "  SizingPeriod:DesignDay,",
         "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
         "    7,                       !- Month",
         "    21,                      !- Day of Month",
         "    SummerDesignDay,         !- Day Type",
         "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
         "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
         "    Wetbulb,                 !- Humidity Condition Type",
         "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
         "    1.00;                    !- Sky Clearness",

         "    SimulationControl,",
         "    no,                     !- Do Zone Sizing Calculation",
         "    no,                     !- Do System Sizing Calculation",
         "    no,                     !- Do Plant Sizing Calculation",
         "    Yes,                     !- Run Simulation for Sizing Periods",
         "    no;                     !- Run Simulation for Weather File Run Periods",

         "  Timestep,6;",

         "  ScheduleTypeLimits,",
         "    Any Number;              !- Name",

         "  CoolingTower:VariableSpeed:Merkel,",
         "    TowerWaterSys CoolTower1,!- Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
         "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
         "    NominalCapacity,         !- Performance Input Method",
         "    1.25,                    !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
         "    10000,                !- Nominal Capacity {W}",
         "    autocalculate,           !- Free Convection Nominal Capacity {W}",
         "    0.1,                     !- Free Convection Nominal Capacity Sizing Factor",
         "    autocalculate,           !- Design Water Flow Rate {m3/s}",
         "    5.382E-8,                !- Design Water Flow Rate per Unit of Nominal Capacity {m3/s-W}",
         "    autocalculate,           !- Design Air Flow Rate {m3/s}",
         "    2.76316E-5,              !- Design Air Flow Rate Per Unit of Nominal Capacity {m3/s-W}",
         "    0.2,                     !- Minimum Air Flow Rate Ratio",
         "    autocalculate,           !- Design Fan Power {W}",
         "    0.0105,                  !- Design Fan Power Per Unit of Nominal Capacity {dimensionless}",
         "    VS tower fan power mod func air flow ratio,  !- Fan Power Modifier Function of Air Flow Rate Ratio Curve Name",
         "    autocalculate,           !- Free Convection Regime Air Flow Rate {m3/s}",
         "    0.1,                     !- Free Convection Regime Air Flow Rate Sizing Factor",
         "    ,                        !- Design Air Flow Rate U-Factor Times Area Value {W/K}",
         "    ,                        !- Free Convection Regime U-Factor Times Area Value {W/K}",
         "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
         "    VS tower UA mod func air flow ratio,  !- U-Factor Times Area Modifier Function of Air Flow Ratio Curve Name",
         "    VS tower UA mod func wetbulb difference,  !- U-Factor Times Area Modifier Function of Wetbulb Temperature Difference Curve Name",
         "    VS tower UA mod func water flow ratio,  !- U-Factor Times Area Modifier Function of Water Flow Ratio Curve Name",
         "    , 						  !- Design Inlet Air Dry - Bulb Temperature",
         "    ,                        !- Design Inlet Air Wet-Bulb Temperature",
         "    ,                        !- Design Approach Temperature",
         "    ,                        !- Design Range Temperature",
         "    ,                        !- Basin Heater Capacity {W/K}",
         "    ,                        !- Basin Heater Setpoint Temperature {C}",
         "    ,                        !- Basin Heater Operating Schedule Name",
         "    ,                        !- Evaporation Loss Mode",
         "    ,                        !- Evaporation Loss Factor {percent/K}",
         "    ,                        !- Drift Loss Percent {percent}",
         "    ,                        !- Blowdown Calculation Mode",
         "    ,                        !- Blowdown Concentration Ratio",
         "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
         "    ,                        !- Supply Water Storage Tank Name",
         "    ,                        !- Outdoor Air Inlet Node Name",
         "    ,                        !- Number of Cells",
         "    ,                        !- Cell Control",
         "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
         "    ,                        !- Cell Maximum Water Flow Rate Fraction",
         "    0.5;                     !- Sizing Factor",

         "  Curve:Cubic,",
         "    VS tower fan power mod func air flow ratio,  !- Name",
         "    0.02,                    !- Coefficient1 Constant",
         "    0.0,                     !- Coefficient2 x",
         "    0.0,                     !- Coefficient3 x**2",
         "    0.98,                    !- Coefficient4 x**3",
         "    0.2,                     !- Minimum Value of x",
         "    1.0,                     !- Maximum Value of x",
         "    0.0,                     !- Minimum Curve Output",
         "    1.0,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Curve:Quadratic,",
         "    VS tower UA mod func air flow ratio,  !- Name",
         "    0.0,                     !- Coefficient1 Constant",
         "    1.3,                     !- Coefficient2 x",
         "    -0.3,                    !- Coefficient3 x**2",
         "    0.2,                     !- Minimum Value of x",
         "    1.0,                     !- Maximum Value of x",
         "    0.0,                     !- Minimum Curve Output",
         "    1.0,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Curve:Linear,",
         "    VS tower UA mod func wetbulb difference,  !- Name",
         "    1.0,                     !- Coefficient1 Constant",
         "    0.0081,                  !- Coefficient2 x",
         "    -10,                     !- Minimum Value of x",
         "    25.0,                    !- Maximum Value of x",
         "    0.85,                    !- Minimum Curve Output",
         "    1.3,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Curve:Quadratic,",
         "    VS tower UA mod func water flow ratio,  !- Name",
         "    0.1082,                  !- Coefficient1 Constant",
         "    1.667,                   !- Coefficient2 x",
         "    -0.7713,                 !- Coefficient3 x**2",
         "    0.3,                     !- Minimum Value of x",
         "    1.0,                     !- Maximum Value of x",
         "    0.5,                     !- Minimum Curve Output",
         "    1.1,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Pump:ConstantSpeed,",
         "    TowerWaterSys Pump,      !- Name",
         "    TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
         "    0.002,                !- Rated Flow Rate {m3/s}",
         "    0.1,                  !- Rated Pump Head {Pa}",
         "    200.0,                !- Rated Power Consumption {W}",
         "    0.87,                    !- Motor Efficiency",
         "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
         "    Intermittent;            !- Pump Control Type",

         "  CondenserEquipmentList,",
         "    TowerWaterSys Equipment List,  !- Name",
         "    CoolingTower:VariableSpeed:Merkel,  !- Equipment 1 Object Type",
         "    TowerWaterSys CoolTower1;!- Equipment 1 Name",

         "  CondenserLoop,",
         "    TowerWaterSys,           !- Name",
         "    Water,                   !- Fluid Type",
         "    ,                        !- User Defined Fluid Type",
         "    TowerWaterSys Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
         "    TowerWaterSys Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
         "    80.0,                    !- Maximum Loop Temperature {C}",
         "    5.0,                     !- Minimum Loop Temperature {C}",
         "    0.002,                !- Maximum Loop Flow Rate {m3/s}",
         "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
         "    0.1,                !- Condenser Loop Volume {m3}",
         "    TowerWaterSys Supply Inlet Node,  !- Condenser Side Inlet Node Name",
         "    TowerWaterSys Supply Outlet Node,  !- Condenser Side Outlet Node Name",
         "    TowerWaterSys Supply Branches,  !- Condenser Side Branch List Name",
         "    TowerWaterSys Supply Connectors,  !- Condenser Side Connector List Name",
         "    TowerWaterSys Demand Inlet Node,  !- Demand Side Inlet Node Name",
         "    TowerWaterSys Demand Outlet Node,  !- Demand Side Outlet Node Name",
         "    TowerWaterSys Demand Branches,  !- Condenser Demand Side Branch List Name",
         "    TowerWaterSys Demand Connectors,  !- Condenser Demand Side Connector List Name",
         "    SequentialLoad,          !- Load Distribution Scheme",
         "    ,                        !- Pressure Simulation Type",
         "    2.0;                     !- Loop Circulation Time {minutes}",

         "  CondenserEquipmentOperationSchemes,",
         "    TowerWaterSys Loop Operation Scheme List,  !- Name",
         "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
         "    TowerWaterSys Operation Scheme,  !- Control Scheme 1 Name",
         "    CONSTANT-1.0;               !- Control Scheme 1 Schedule Name",

         "  PlantEquipmentOperation:CoolingLoad,",
         "    TowerWaterSys Operation Scheme,  !- Name",
         "    0.0,                     !- Load Range 1 Lower Limit {W}",
         "    1000000000000,           !- Load Range 1 Upper Limit {W}",
         "    TowerWaterSys Equipment List;  !- Range 1 Equipment List Name",

         "  SetpointManager:Scheduled,",
         "    TowerWaterSys Setpoint Manager,  !- Name",
         "    Temperature,             !- Control Variable",
         "    TowerWaterSys Temp Sch,      !- Schedule Name",
         "    TowerWaterSys Supply Outlet Node;  !- Setpoint Node or NodeList Name",

         "  Schedule:Compact,",
         "    TowerWaterSys Temp Sch,      !- Name",
         "    Any Number,              !- Schedule Type Limits Name",
         "    THROUGH: 12/31,          !- Field 1",
         "    FOR: AllDays,            !- Field 2",
         "    UNTIL: 24:00,30.0;       !- Field 3",

         "  BranchList,",
         "    TowerWaterSys Demand Branches,  !- Name",
         "    TowerWaterSys Demand Inlet Branch,  !- Branch 1 Name",
         "    TowerWaterSys Demand Load Branch 1,  !- Branch 2 Name",
         "    TowerWaterSys Demand Bypass Branch,  !- Branch 4 Name",
         "    TowerWaterSys Demand Outlet Branch;  !- Branch 5 Name",

         "  BranchList,",
         "    TowerWaterSys Supply Branches,  !- Name",
         "    TowerWaterSys Supply Inlet Branch,  !- Branch 1 Name",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Branch 2 Name",
         "    TowerWaterSys Supply Equipment Bypass Branch,  !- Branch 4 Name",
         "    TowerWaterSys Supply Outlet Branch;  !- Branch 5 Name",

         "  Branch,",
         "    TowerWaterSys Demand Bypass Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
         "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Demand Inlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Demand Inlet Pipe,  !- Component 1 Name",
         "    TowerWaterSys Demand Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Demand Load Branch 1,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    LoadProfile:Plant,  !- Component 1 Object Type",
         "    Load Profile 1,      !- Component 1 Name",
         "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
         "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

         "  LoadProfile:Plant,",
         "    Load Profile 1,          !- Name",
         "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
         "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
         "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
         "    0.002,                   !- Peak Flow Rate {m3/s}",
         "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

         "  Schedule:Compact,",
         "    Load Profile 1 Load Schedule,  !- Name",
         "    Any Number,              !- Schedule Type Limits Name",
         "    THROUGH: 12/31,          !- Field 1",
         "    FOR: AllDays,            !- Field 2",
         "    UNTIL: 24:00,0.0;        !- Field 3",

         "  Schedule:Compact,",
         "    Load Profile 1 Flow Frac Schedule,  !- Name",
         "    Any Number,              !- Schedule Type Limits Name",
         "    THROUGH: 12/31,          !- Field 1",
         "    FOR: AllDays,            !- Field 2",
         "    UNTIL: 24:00,1.0;        !- Field 3",

         "  Branch,",
         "    TowerWaterSys Demand Outlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Demand Outlet Pipe,  !- Component 1 Name",
         "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Demand Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    CoolingTower:VariableSpeed:Merkel,  !- Component 1 Object Type",
         "    TowerWaterSys CoolTower1,!- Component 1 Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Supply Equipment Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Equipment Bypass Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Component 1 Name",
         "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Inlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pump:ConstantSpeed,      !- Component 1 Object Type",
         "    TowerWaterSys Pump,      !- Component 1 Name",
         "    TowerWaterSys Supply Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Outlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Supply Outlet Pipe,  !- Component 1 Name",
         "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Supply Outlet Node;  !- Component 1 Outlet Node Name",

         "  OutdoorAir:Node,",
         "    TowerWaterSys CoolTower OA ref Node;  !- Name",

         "  ConnectorList,",
         "    TowerWaterSys Demand Connectors,  !- Name",
         "    Connector:Splitter,      !- Connector 1 Object Type",
         "    TowerWaterSys Demand Splitter,  !- Connector 1 Name",
         "    Connector:Mixer,         !- Connector 2 Object Type",
         "    TowerWaterSys Demand Mixer;  !- Connector 2 Name",

         "  ConnectorList,",
         "    TowerWaterSys Supply Connectors,  !- Name",
         "    Connector:Splitter,      !- Connector 1 Object Type",
         "    TowerWaterSys Supply Splitter,  !- Connector 1 Name",
         "    Connector:Mixer,         !- Connector 2 Object Type",
         "    TowerWaterSys Supply Mixer;  !- Connector 2 Name",

         "  Connector:Splitter,",
         "    TowerWaterSys Demand Splitter,  !- Name",
         "    TowerWaterSys Demand Inlet Branch,  !- Inlet Branch Name",
         "    TowerWaterSys Demand Load Branch 1,  !- Outlet Branch 1 Name",
         "    TowerWaterSys Demand Bypass Branch;  !- Outlet Branch 3 Name",

         "  Connector:Splitter,",
         "    TowerWaterSys Supply Splitter,  !- Name",
         "    TowerWaterSys Supply Inlet Branch,  !- Inlet Branch Name",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
         "    TowerWaterSys Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

         "  Connector:Mixer,",
         "    TowerWaterSys Demand Mixer,  !- Name",
         "    TowerWaterSys Demand Outlet Branch,  !- Outlet Branch Name",
         "    TowerWaterSys Demand Load Branch 1,  !- Inlet Branch 1 Name",
         "    TowerWaterSys Demand Bypass Branch;  !- Inlet Branch 3 Name",

         "  Connector:Mixer,",
         "    TowerWaterSys Supply Mixer,  !- Name",
         "    TowerWaterSys Supply Outlet Branch,  !- Outlet Branch Name",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
         "    TowerWaterSys Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Demand Bypass Pipe,  !- Name",
         "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Demand Inlet Pipe,  !- Name",
         "    TowerWaterSys Demand Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Demand Outlet Pipe,  !- Name",
         "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Inlet Node Name",
         "    TowerWaterSys Demand Outlet Node;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Name",
         "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Supply Outlet Pipe,  !- Name",
         "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Inlet Node Name",
         "    TowerWaterSys Supply Outlet Node;  !- Outlet Node Name"

        });
    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    state->init_state(*state);

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    CondenserLoopTowers::GetTowerInput(*state);

    state->dataCondenserLoopTowers->towers(1).initialize(*state);
    state->dataCondenserLoopTowers->towers(1).SizeVSMerkelTower(*state);
    state->dataCondenserLoopTowers->towers(1).initialize(*state);
    Real64 MyLoad = 0.0;
    bool RunFlag = false;
    state->dataCondenserLoopTowers->towers(1).calculateMerkelVariableSpeedTower(*state, MyLoad, RunFlag);
    state->dataCondenserLoopTowers->towers(1).update(*state);
    state->dataCondenserLoopTowers->towers(1).report(*state, true);

    // test that tower is really not cooling with no load so temp in and out is the same issue #4927
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(9).Temp, state->dataLoopNodes->Node(10).Temp);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_SingleSpeedSizing)
{
    std::string const idf_objects =
        delimited_string({"  Site:Location,",
                          "    USA IL-CHICAGO-OHARE,    !- Name",
                          "    41.77,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190;                     !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
                          "    1.00;                    !- Sky Clearness",

                          "    SimulationControl,",
                          "    no,                     !- Do Zone Sizing Calculation",
                          "    no,                     !- Do System Sizing Calculation",
                          "    no,                     !- Do Plant Sizing Calculation",
                          "    Yes,                     !- Run Simulation for Sizing Periods",
                          "    no;                     !- Run Simulation for Weather File Run Periods",

                          "  Timestep,6;",

                          "  ScheduleTypeLimits,",
                          "    Any Number;              !- Name",

                          "  CoolingTower:SingleSpeed,",
                          "    TowerWaterSys CoolTower1,!- Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
                          "    ,                        !- Design Water Flow Rate {m3/s}",
                          "    autosize,                !- Design Air Flow Rate {m3/s}",
                          "    autosize,                !- Design Fan Power {W}",
                          "    ,                        !- Design U-Factor Times Area Value {W/K}",
                          "    autocalculate,           !- Free Convection Air Flow Rate {m3/s}",
                          "    ,                        !- Free Convection Air Flow Rate Sizing Factor",
                          "    ,                        !- Free Convection U-Factor Times Area Value {W/K}",
                          "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
                          "    NominalCapacity,         !- Performance Input Method",
                          "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
                          "    1E+25,                   !- Nominal Capacity {W}",
                          "    1E+15,                   !- Free Convection Capacity {W}",
                          "    ,                        !- Free Convection Nominal Capacity Sizing Factor",
                          "    ,                        !- Design Inlet Air Dry-Bulb Temperature",
                          "    ,                        !- Design Inlet Air Wet-Bulb Temperature",
                          "    ,                        !- Design Approach Temperature",
                          "    ,                        !- Design Range Temperature",
                          "    ,                        !- Basin Heater Capacity {W/K}",
                          "    ,                        !- Basin Heater Setpoint Temperature {C}",
                          "    ,                        !- Basin Heater Operating Schedule Name",
                          "    ,                        !- Evaporation Loss Mode",
                          "    ,                        !- Evaporation Loss Factor {percent/K}",
                          "    ,                        !- Drift Loss Percent {percent}",
                          "    ,                        !- Blowdown Calculation Mode",
                          "    ,                        !- Blowdown Concentration Ratio",
                          "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
                          "    ,                        !- Supply Water Storage Tank Name",
                          "    ,                        !- Outdoor Air Inlet Node Name",
                          "    FluidBypass;             !- Capacity Control",

                          "  Pump:ConstantSpeed,",
                          "    TowerWaterSys Pump,      !- Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
                          "    0.002,                !- Rated Flow Rate {m3/s}",
                          "    0.1,                  !- Rated Pump Head {Pa}",
                          "    200.0,                !- Rated Power Consumption {W}",
                          "    0.87,                    !- Motor Efficiency",
                          "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "  CondenserEquipmentList,",
                          "    TowerWaterSys Equipment List,  !- Name",
                          "    CoolingTower:SingleSpeed,  !- Equipment 1 Object Type",
                          "    TowerWaterSys CoolTower1;!- Equipment 1 Name",

                          "  CondenserLoop,",
                          "    TowerWaterSys,           !- Name",
                          "    Water,                   !- Fluid Type",
                          "    ,                        !- User Defined Fluid Type",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
                          "    TowerWaterSys Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
                          "    80.0,                    !- Maximum Loop Temperature {C}",
                          "    5.0,                     !- Minimum Loop Temperature {C}",
                          "    0.002,                !- Maximum Loop Flow Rate {m3/s}",
                          "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
                          "    0.1,                !- Condenser Loop Volume {m3}",
                          "    TowerWaterSys Supply Inlet Node,  !- Condenser Side Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node,  !- Condenser Side Outlet Node Name",
                          "    TowerWaterSys Supply Branches,  !- Condenser Side Branch List Name",
                          "    TowerWaterSys Supply Connectors,  !- Condenser Side Connector List Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Demand Side Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node,  !- Demand Side Outlet Node Name",
                          "    TowerWaterSys Demand Branches,  !- Condenser Demand Side Branch List Name",
                          "    TowerWaterSys Demand Connectors,  !- Condenser Demand Side Connector List Name",
                          "    SequentialLoad,          !- Load Distribution Scheme",
                          "    ,                        !- Pressure Simulation Type",
                          "    2.0;                     !- Loop Circulation Time {minutes}",

                          "  CondenserEquipmentOperationSchemes,",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    TowerWaterSys Operation Scheme,  !- Control Scheme 1 Name",
                          "    CONSTANT-1.0;               !- Control Scheme 1 Schedule Name",

                          "  PlantEquipmentOperation:CoolingLoad,",
                          "    TowerWaterSys Operation Scheme,  !- Name",
                          "    0.0,                     !- Load Range 1 Lower Limit {W}",
                          "    1000000000000,           !- Load Range 1 Upper Limit {W}",
                          "    TowerWaterSys Equipment List;  !- Range 1 Equipment List Name",

                          "  SetpointManager:Scheduled,",
                          "    TowerWaterSys Setpoint Manager,  !- Name",
                          "    Temperature,             !- Control Variable",
                          "    TowerWaterSys Temp Sch,      !- Schedule Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Setpoint Node or NodeList Name",

                          "  Schedule:Compact,",
                          "    TowerWaterSys Temp Sch,      !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,30.0;       !- Field 3",

                          "  BranchList,",
                          "    TowerWaterSys Demand Branches,  !- Name",
                          "    TowerWaterSys Demand Inlet Branch,  !- Branch 1 Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Demand Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Demand Outlet Branch;  !- Branch 5 Name",

                          "  BranchList,",
                          "    TowerWaterSys Supply Branches,  !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Supply Outlet Branch;  !- Branch 5 Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Load Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    LoadProfile:Plant,  !- Component 1 Object Type",
                          "    Load Profile 1,      !- Component 1 Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

                          "  LoadProfile:Plant,",
                          "    Load Profile 1,          !- Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
                          "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
                          "    0.002,                   !- Peak Flow Rate {m3/s}",
                          "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Load Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,0.0;        !- Field 3",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Flow Frac Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,1.0;        !- Field 3",

                          "  Branch,",
                          "    TowerWaterSys Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    CoolingTower:SingleSpeed,  !- Component 1 Object Type",
                          "    TowerWaterSys CoolTower1,!- Component 1 Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    TowerWaterSys Pump,      !- Component 1 Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Component 1 Outlet Node Name",

                          "  OutdoorAir:Node,",
                          "    TowerWaterSys CoolTower OA ref Node;  !- Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Demand Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Demand Mixer;  !- Connector 2 Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Supply Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Supply Mixer;  !- Connector 2 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Demand Splitter,  !- Name",
                          "    TowerWaterSys Demand Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Supply Splitter,  !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Demand Mixer,  !- Name",
                          "    TowerWaterSys Demand Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Supply Mixer,  !- Name",
                          "    TowerWaterSys Supply Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Outlet Node Name"

        });
    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    state->init_state(*state);

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    CondenserLoopTowers::GetTowerInput(*state);

    Real64 MyLoad = 0.0;
    bool RunFlag = false;
    state->dataCondenserLoopTowers->towers(1).initialize(*state);
    state->dataCondenserLoopTowers->towers(1).SizeTower(*state);
    state->dataCondenserLoopTowers->towers(1).initialize(*state);
    state->dataCondenserLoopTowers->towers(1).calculateSingleSpeedTower(*state, MyLoad, RunFlag);
    state->dataCondenserLoopTowers->towers(1).update(*state);
    state->dataCondenserLoopTowers->towers(1).report(*state, true);

    // test that tower outlet temperature = set point temperature
    int inletNodeIndex = 0;
    int outletNodeIndex = 0;
    auto inletNode =
        std::find(state->dataLoopNodes->NodeID.begin(), state->dataLoopNodes->NodeID.end(), "TOWERWATERSYS PUMP-TOWERWATERSYS COOLTOWERNODE");
    ASSERT_TRUE(inletNode != state->dataLoopNodes->NodeID.end());
    if (inletNode != state->dataLoopNodes->NodeID.end()) {
        inletNodeIndex = std::distance(state->dataLoopNodes->NodeID.begin(), inletNode);
    }
    auto outletNode =
        std::find(state->dataLoopNodes->NodeID.begin(), state->dataLoopNodes->NodeID.end(), "TOWERWATERSYS SUPPLY EQUIPMENT OUTLET NODE");
    ASSERT_TRUE(outletNode != state->dataLoopNodes->NodeID.end());
    if (outletNode != state->dataLoopNodes->NodeID.end()) {
        outletNodeIndex = std::distance(state->dataLoopNodes->NodeID.begin(), outletNode);
    }
    EXPECT_GT(state->dataLoopNodes->Node(inletNodeIndex).Temp, 30.0);                                                    // inlet node temperature
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(inletNodeIndex).Temp, state->dataLoopNodes->Node(outletNodeIndex).Temp); // outlet node temperature

    // input not needed for sizing (WasAutoSized = false) using NominalCapacity method but this variable should still size
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUAWasAutoSized);
    EXPECT_GT(state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUA,
              10000000.0); // nominal capacity input was huge at 1E+25 so all sized variables referencing capacity are very large

    // input not needed for sizing (WasAutoSized = false) using NominalCapacity method but this variable should still size
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRateWasAutoSized);
    EXPECT_GT(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate, 10000000.0);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate,
                     5.382e-8 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRateWasAutoSized);
    EXPECT_GT(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate, 10000000.0);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower * 0.5 * (101325.0 / state->dataEnvrn->StdBaroPress) / 190.0);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPowerWasAutoSized);
    EXPECT_GT(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower, 10000000.0);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower,
                     0.0105 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autocalculate input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRateWasAutoSized);
    EXPECT_GT(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRate, 10000000.0);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRateSizingFactor *
                         state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_SingleSpeedUserInputTowerSizing)
{
    std::string const idf_objects =
        delimited_string({"  Site:Location,",
                          "    USA IL-CHICAGO-OHARE,    !- Name",
                          "    41.77,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190;                     !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
                          "    1.00;                    !- Sky Clearness",

                          "    SimulationControl,",
                          "    no,                     !- Do Zone Sizing Calculation",
                          "    no,                     !- Do System Sizing Calculation",
                          "    Yes,                    !- Do Plant Sizing Calculation",
                          "    Yes,                    !- Run Simulation for Sizing Periods",
                          "    no;                     !- Run Simulation for Weather File Run Periods",

                          "  Timestep,6;",

                          "  ScheduleTypeLimits,",
                          "    Any Number;              !- Name",

                          "  CoolingTower:SingleSpeed,",
                          "    TowerWaterSys CoolTower1,!- Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
                          "    ,                        !- Design Water Flow Rate {m3/s}",
                          "    autosize,                !- Design Air Flow Rate {m3/s}",
                          "    autosize,                !- Design Fan Power {W}",
                          "    ,                        !- Design U-Factor Times Area Value {W/K}",
                          "    autocalculate,           !- Free Convection Air Flow Rate {m3/s}",
                          "    ,                        !- Free Convection Air Flow Rate Sizing Factor",
                          "    ,                        !- Free Convection U-Factor Times Area Value {W/K}",
                          "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
                          "    NominalCapacity,         !- Performance Input Method",
                          "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
                          "    1E+5,                    !- Nominal Capacity {W}",
                          "    1E+3,                    !- Free Convection Capacity {W}",
                          "    ,                        !- Free Convection Nominal Capacity Sizing Factor",
                          "    35.0,					  !- Design Inlet Air Dry-Bulb Temperature",
                          "    25.6,					  !- Design Inlet Air Wet-Bulb Temperature",
                          "    3.9,                     !- Design Approach Temperature",
                          "    5.5,                     !- Design Range Temperature",
                          "    ,                        !- Basin Heater Capacity {W/K}",
                          "    ,                        !- Basin Heater Setpoint Temperature {C}",
                          "    ,                        !- Basin Heater Operating Schedule Name",
                          "    ,                        !- Evaporation Loss Mode",
                          "    ,                        !- Evaporation Loss Factor {percent/K}",
                          "    ,                        !- Drift Loss Percent {percent}",
                          "    ,                        !- Blowdown Calculation Mode",
                          "    ,                        !- Blowdown Concentration Ratio",
                          "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
                          "    ,                        !- Supply Water Storage Tank Name",
                          "    ,                        !- Outdoor Air Inlet Node Name",
                          "    FluidBypass;             !- Capacity Control",

                          "  Pump:ConstantSpeed,",
                          "    TowerWaterSys Pump,      !- Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
                          "    0.002,                !- Rated Flow Rate {m3/s}",
                          "    0.1,                  !- Rated Pump Head {Pa}",
                          "    200.0,                !- Rated Power Consumption {W}",
                          "    0.87,                    !- Motor Efficiency",
                          "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "  CondenserEquipmentList,",
                          "    TowerWaterSys Equipment List,  !- Name",
                          "    CoolingTower:SingleSpeed,  !- Equipment 1 Object Type",
                          "    TowerWaterSys CoolTower1;!- Equipment 1 Name",

                          "  CondenserLoop,",
                          "    TowerWaterSys,           !- Name",
                          "    Water,                   !- Fluid Type",
                          "    ,                        !- User Defined Fluid Type",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
                          "    TowerWaterSys Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
                          "    80.0,                    !- Maximum Loop Temperature {C}",
                          "    5.0,                     !- Minimum Loop Temperature {C}",
                          "    0.002,                   !- Maximum Loop Flow Rate {m3/s}",
                          "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
                          "    0.1,                     !- Condenser Loop Volume {m3}",
                          "    TowerWaterSys Supply Inlet Node,  !- Condenser Side Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node, !- Condenser Side Outlet Node Name",
                          "    TowerWaterSys Supply Branches,    !- Condenser Side Branch List Name",
                          "    TowerWaterSys Supply Connectors,  !- Condenser Side Connector List Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Demand Side Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node, !- Demand Side Outlet Node Name",
                          "    TowerWaterSys Demand Branches,    !- Condenser Demand Side Branch List Name",
                          "    TowerWaterSys Demand Connectors,  !- Condenser Demand Side Connector List Name",
                          "    SequentialLoad,                   !- Load Distribution Scheme",
                          "    ,;",

                          "  CondenserEquipmentOperationSchemes,",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    TowerWaterSys Operation Scheme,  !- Control Scheme 1 Name",
                          "    CONSTANT-1.0;                       !- Control Scheme 1 Schedule Name",

                          "  PlantEquipmentOperation:CoolingLoad,",
                          "    TowerWaterSys Operation Scheme,  !- Name",
                          "    0.0,                             !- Load Range 1 Lower Limit {W}",
                          "    1000000000000,                   !- Load Range 1 Upper Limit {W}",
                          "    TowerWaterSys Equipment List;    !- Range 1 Equipment List Name",

                          "  SetpointManager:Scheduled,",
                          "    TowerWaterSys Setpoint Manager,  !- Name",
                          "    Temperature,                     !- Control Variable",
                          "    TowerWaterSys Temp Sch,          !- Schedule Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Setpoint Node or NodeList Name",

                          "  Schedule:Compact,",
                          "    TowerWaterSys Temp Sch,      !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,30.0;       !- Field 3",

                          "  BranchList,",
                          "    TowerWaterSys Demand Branches,       !- Name",
                          "    TowerWaterSys Demand Inlet Branch,   !- Branch 1 Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Demand Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Demand Outlet Branch;  !- Branch 5 Name",

                          "  BranchList,",
                          "    TowerWaterSys Supply Branches,      !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Supply Outlet Branch;  !- Branch 5 Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Load Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    LoadProfile:Plant,   !- Component 1 Object Type",
                          "    Load Profile 1,      !- Component 1 Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

                          "  LoadProfile:Plant,",
                          "    Load Profile 1,          !- Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
                          "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
                          "    0.002,                   !- Peak Flow Rate {m3/s}",
                          "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Load Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,0.0;        !- Field 3",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Flow Frac Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,1.0;        !- Field 3",

                          "  Branch,",
                          "    TowerWaterSys Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    CoolingTower:SingleSpeed,  !- Component 1 Object Type",
                          "    TowerWaterSys CoolTower1,!- Component 1 Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    TowerWaterSys Pump,      !- Component 1 Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Component 1 Outlet Node Name",

                          "  OutdoorAir:Node,",
                          "    TowerWaterSys CoolTower OA ref Node;  !- Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Demand Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Demand Mixer;  !- Connector 2 Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Supply Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Supply Mixer;  !- Connector 2 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Demand Splitter,  !- Name",
                          "    TowerWaterSys Demand Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Supply Splitter,  !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Demand Mixer,  !- Name",
                          "    TowerWaterSys Demand Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Supply Mixer,  !- Name",
                          "    TowerWaterSys Supply Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Outlet Node Name"

        });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    CondenserLoopTowers::GetTowerInput(*state);

    // sized using user inputs in cooling tower instead of plant sizing object
    state->dataCondenserLoopTowers->towers(1).SizeTower(*state);

    // input not needed for sizing
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUAWasAutoSized);
    EXPECT_NEAR(
        state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUA, 9595.0, 1.0); // nominal capacity input was 100 kW, approach, 3.9K, range 5.5K

    // input not needed for sizing
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate, 0.005382, 0.00001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate,
                     5.382e-8 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate, 2.8262, 0.0001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower * 0.5 * (101325.0 / state->dataEnvrn->StdBaroPress) / 190.0);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPowerWasAutoSized);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower, 1050);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower,
                     0.0105 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autocalculate input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRate, 0.28262, 0.00001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRateSizingFactor *
                         state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_TwoSpeedUserInputTowerSizing)
{
    std::string const idf_objects =
        delimited_string({"  Site:Location,",
                          "    USA IL-CHICAGO-OHARE,    !- Name",
                          "    41.77,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190;                     !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
                          "    1.00;                    !- Sky Clearness",

                          "    SimulationControl,",
                          "    no,                     !- Do Zone Sizing Calculation",
                          "    no,                     !- Do System Sizing Calculation",
                          "    Yes,                    !- Do Plant Sizing Calculation",
                          "    Yes,                    !- Run Simulation for Sizing Periods",
                          "    no;                     !- Run Simulation for Weather File Run Periods",

                          "  Timestep,6;",

                          "  ScheduleTypeLimits,",
                          "    Any Number;              !- Name",

                          "  CoolingTower:TwoSpeed,",
                          "    TowerWaterSys CoolTower1,!- Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
                          "    ,                        !- Design Water Flow Rate {m3/s}",
                          "    AUTOSIZE,                !- High Fan Speed Air Flow Rate {m3/s}",
                          "    AUTOSIZE,                !- High Fan Speed Fan Power {W}",
                          "    ,                        !- High Fan Speed U-Factor Times Area Value {W/K}",
                          "    autocalculate,           !- Low Fan Speed Air Flow Rate {m3/s}",
                          "    ,                        !- Low Fan Speed Air Flow Rate Sizing Factor",
                          "    AUTOSIZE,                !- Low Fan Speed Fan Power {W}",
                          "    ,                        !- Low Fan Speed Fan Power Sizing Factor",
                          "    AUTOSIZE,                !- Low Fan Speed U-Factor Times Area Value {W/K}",
                          "    ,                        !- Low Fan Speed U-Factor Times Area Sizing Factor",
                          "    0.9,                     !- Free Convection Regime Air Flow Rate {m3/s}",
                          "    ,                        !- Free Convection Regime Air Flow Rate Sizing Factor",
                          "    autocalculate,           !- Free Convection Regime U-Factor Times Area Value {W/K}",
                          "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
                          "    NominalCapacity,         !- Performance Input Method",
                          "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
                          "    1E+5,                    !- High Speed Nominal Capacity {W}",
                          "    1E+4,                    !- Low Speed Nominal Capacity {W}",
                          "    ,                        !- Low Speed Nominal Capacity Sizing Factor",
                          "    5E+3,                    !- Free Convection Nominal Capacity {W}",
                          "    ,                        !- Free Convection Nominal Capacity Sizing Factor",
                          "    35.0,					  !- Design Inlet Air Dry-Bulb Temperature",
                          "    25.6,					  !- Design Inlet Air Wet-Bulb Temperature",
                          "    3.9,                     !- Design Approach Temperature",
                          "    5.5,                     !- Design Range Temperature",
                          "    ,                        !- Basin Heater Capacity {W/K}",
                          "    ,                        !- Basin Heater Setpoint Temperature {C}",
                          "    ,                        !- Basin Heater Operating Schedule Name",
                          "    ,                        !- Evaporation Loss Mode",
                          "    ,                        !- Evaporation Loss Factor {percent/K}",
                          "    ,                        !- Drift Loss Percent {percent}",
                          "    ,                        !- Blowdown Calculation Mode",
                          "    ,                        !- Blowdown Concentration Ratio",
                          "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
                          "    ,                        !- Supply Water Storage Tank Name",
                          "    ;                        !- Outdoor Air Inlet Node Name",

                          "  Pump:ConstantSpeed,",
                          "    TowerWaterSys Pump,      !- Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
                          "    0.002,                !- Rated Flow Rate {m3/s}",
                          "    0.1,                  !- Rated Pump Head {Pa}",
                          "    200.0,                !- Rated Power Consumption {W}",
                          "    0.87,                    !- Motor Efficiency",
                          "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "  CondenserEquipmentList,",
                          "    TowerWaterSys Equipment List,  !- Name",
                          "    CoolingTower:TwoSpeed,   !- Equipment 1 Object Type",
                          "    TowerWaterSys CoolTower1;!- Equipment 1 Name",

                          "  CondenserLoop,",
                          "    TowerWaterSys,           !- Name",
                          "    Water,                   !- Fluid Type",
                          "    ,                        !- User Defined Fluid Type",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
                          "    TowerWaterSys Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
                          "    80.0,                    !- Maximum Loop Temperature {C}",
                          "    5.0,                     !- Minimum Loop Temperature {C}",
                          "    0.002,                   !- Maximum Loop Flow Rate {m3/s}",
                          "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
                          "    0.1,                     !- Condenser Loop Volume {m3}",
                          "    TowerWaterSys Supply Inlet Node,  !- Condenser Side Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node, !- Condenser Side Outlet Node Name",
                          "    TowerWaterSys Supply Branches,    !- Condenser Side Branch List Name",
                          "    TowerWaterSys Supply Connectors,  !- Condenser Side Connector List Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Demand Side Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node, !- Demand Side Outlet Node Name",
                          "    TowerWaterSys Demand Branches,    !- Condenser Demand Side Branch List Name",
                          "    TowerWaterSys Demand Connectors,  !- Condenser Demand Side Connector List Name",
                          "    SequentialLoad,,;                   !- Load Distribution Scheme",

                          "  CondenserEquipmentOperationSchemes,",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    TowerWaterSys Operation Scheme,  !- Control Scheme 1 Name",
                          "    CONSTANT-1.0;                       !- Control Scheme 1 Schedule Name",

                          "  PlantEquipmentOperation:CoolingLoad,",
                          "    TowerWaterSys Operation Scheme,  !- Name",
                          "    0.0,                             !- Load Range 1 Lower Limit {W}",
                          "    1000000000000,                   !- Load Range 1 Upper Limit {W}",
                          "    TowerWaterSys Equipment List;    !- Range 1 Equipment List Name",

                          "  SetpointManager:Scheduled,",
                          "    TowerWaterSys Setpoint Manager,  !- Name",
                          "    Temperature,                     !- Control Variable",
                          "    TowerWaterSys Temp Sch,          !- Schedule Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Setpoint Node or NodeList Name",

                          "  Schedule:Compact,",
                          "    TowerWaterSys Temp Sch,      !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,30.0;       !- Field 3",

                          "  BranchList,",
                          "    TowerWaterSys Demand Branches,       !- Name",
                          "    TowerWaterSys Demand Inlet Branch,   !- Branch 1 Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Demand Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Demand Outlet Branch;  !- Branch 5 Name",

                          "  BranchList,",
                          "    TowerWaterSys Supply Branches,      !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Supply Outlet Branch;  !- Branch 5 Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Load Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    LoadProfile:Plant,   !- Component 1 Object Type",
                          "    Load Profile 1,      !- Component 1 Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

                          "  LoadProfile:Plant,",
                          "    Load Profile 1,          !- Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
                          "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
                          "    0.002,                   !- Peak Flow Rate {m3/s}",
                          "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Load Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,0.0;        !- Field 3",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Flow Frac Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,1.0;        !- Field 3",

                          "  Branch,",
                          "    TowerWaterSys Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    CoolingTower:TwoSpeed,  !- Component 1 Object Type",
                          "    TowerWaterSys CoolTower1,!- Component 1 Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    TowerWaterSys Pump,      !- Component 1 Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Component 1 Outlet Node Name",

                          "  OutdoorAir:Node,",
                          "    TowerWaterSys CoolTower OA ref Node;  !- Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Demand Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Demand Mixer;  !- Connector 2 Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Supply Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Supply Mixer;  !- Connector 2 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Demand Splitter,  !- Name",
                          "    TowerWaterSys Demand Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Supply Splitter,  !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Demand Mixer,  !- Name",
                          "    TowerWaterSys Demand Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Supply Mixer,  !- Name",
                          "    TowerWaterSys Supply Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Outlet Node Name"

        });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    CondenserLoopTowers::GetTowerInput(*state);

    // sized using user inputs in cooling tower instead of plant sizing object
    state->dataCondenserLoopTowers->towers(1).SizeTower(*state);

    // input not needed for sizing (NOT WasAutoSized)
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUAWasAutoSized);
    EXPECT_NEAR(
        state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUA, 9595.55, 1.0); // nominal capacity input was 100 kW, approach, 3.9K, range 5.5K

    // input not needed for sizing (NOT WasAutoSized)
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate, 0.005382, 0.00001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate,
                     5.382e-8 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate, 2.8262, 0.0001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower * 0.5 * (101325.0 / state->dataEnvrn->StdBaroPress) / 190.0);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPowerWasAutoSized);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower, 1050);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower,
                     0.0105 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).LowSpeedAirFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).LowSpeedAirFlowRate, 1.4131, 0.0001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).LowSpeedAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate *
                         state->dataCondenserLoopTowers->towers(1).LowSpeedAirFlowRateSizingFactor);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).LowSpeedTowerUAWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).LowSpeedTowerUA, 346.0, 1.0);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).LowSpeedFanPowerWasAutoSized);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).LowSpeedFanPower, 168);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).LowSpeedFanPower,
                     state->dataCondenserLoopTowers->towers(1).LowSpeedFanPowerSizingFactor *
                         state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).FreeConvTowerUAWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).FreeConvTowerUA, 168.0, 1.0);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_MerkelUserInputTowerSizing)
{
    std::string const idf_objects = delimited_string(
        {"  Site:Location,",
         "    USA IL-CHICAGO-OHARE,    !- Name",
         "    41.77,                   !- Latitude {deg}",
         "    -87.75,                  !- Longitude {deg}",
         "    -6.00,                   !- Time Zone {hr}",
         "    190;                     !- Elevation {m}",

         "  SizingPeriod:DesignDay,",
         "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
         "    1,                       !- Month",
         "    21,                      !- Day of Month",
         "    WinterDesignDay,         !- Day Type",
         "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
         "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
         "    Wetbulb,                 !- Humidity Condition Type",
         "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

         "  SizingPeriod:DesignDay,",
         "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
         "    7,                       !- Month",
         "    21,                      !- Day of Month",
         "    SummerDesignDay,         !- Day Type",
         "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
         "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
         "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
         "    Wetbulb,                 !- Humidity Condition Type",
         "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
         "    1.00;                    !- Sky Clearness",

         "    SimulationControl,",
         "    no,                     !- Do Zone Sizing Calculation",
         "    no,                     !- Do System Sizing Calculation",
         "    no,                     !- Do Plant Sizing Calculation",
         "    Yes,                     !- Run Simulation for Sizing Periods",
         "    no;                     !- Run Simulation for Weather File Run Periods",

         "  Timestep,6;",

         "  ScheduleTypeLimits,",
         "    Any Number;              !- Name",

         "  CoolingTower:VariableSpeed:Merkel,",
         "    TowerWaterSys CoolTower1,!- Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
         "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
         "    NominalCapacity,         !- Performance Input Method",
         "    1.25,                    !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
         "    1E+5,                    !- Nominal Capacity {W}",
         "    autocalculate,           !- Free Convection Nominal Capacity {W}",
         "    0.1,                     !- Free Convection Nominal Capacity Sizing Factor",
         "    autocalculate,           !- Design Water Flow Rate {m3/s}",
         "    5.382E-8,                !- Design Water Flow Rate per Unit of Nominal Capacity {m3/s-W}",
         "    autocalculate,           !- Design Air Flow Rate {m3/s}",
         "    2.76316E-5,              !- Design Air Flow Rate Per Unit of Nominal Capacity {m3/s-W}",
         "    0.2,                     !- Minimum Air Flow Rate Ratio",
         "    autocalculate,           !- Design Fan Power {W}",
         "    0.0105,                  !- Design Fan Power Per Unit of Nominal Capacity {dimensionless}",
         "    VS tower fan power mod func air flow ratio,  !- Fan Power Modifier Function of Air Flow Rate Ratio Curve Name",
         "    autocalculate,           !- Free Convection Regime Air Flow Rate {m3/s}",
         "    0.1,                     !- Free Convection Regime Air Flow Rate Sizing Factor",
         "    ,                        !- Design Air Flow Rate U-Factor Times Area Value {W/K}",
         "    ,                        !- Free Convection Regime U-Factor Times Area Value {W/K}",
         "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
         "    VS tower UA mod func air flow ratio,  !- U-Factor Times Area Modifier Function of Air Flow Ratio Curve Name",
         "    VS tower UA mod func wetbulb difference,  !- U-Factor Times Area Modifier Function of Wetbulb Temperature Difference Curve Name",
         "    VS tower UA mod func water flow ratio,  !- U-Factor Times Area Modifier Function of Water Flow Ratio Curve Name",
         "    35.0,					  !- Design Inlet Air Dry-Bulb Temperature",
         "    25.6,					  !- Design Inlet Air Wet-Bulb Temperature",
         "    3.9,                     !- Design Approach Temperature",
         "    5.5,                     !- Design Range Temperature",
         "    ,                        !- Basin Heater Capacity {W/K}",
         "    ,                        !- Basin Heater Setpoint Temperature {C}",
         "    ,                        !- Basin Heater Operating Schedule Name",
         "    ,                        !- Evaporation Loss Mode",
         "    ,                        !- Evaporation Loss Factor {percent/K}",
         "    ,                        !- Drift Loss Percent {percent}",
         "    ,                        !- Blowdown Calculation Mode",
         "    ,                        !- Blowdown Concentration Ratio",
         "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
         "    ,                        !- Supply Water Storage Tank Name",
         "    ,                        !- Outdoor Air Inlet Node Name",
         "    ,                        !- Number of Cells",
         "    ,                        !- Cell Control",
         "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
         "    ,                        !- Cell Maximum Water Flow Rate Fraction",
         "    0.5;                     !- Sizing Factor",

         "  Curve:Cubic,",
         "    VS tower fan power mod func air flow ratio,  !- Name",
         "    0.02,                    !- Coefficient1 Constant",
         "    0.0,                     !- Coefficient2 x",
         "    0.0,                     !- Coefficient3 x**2",
         "    0.98,                    !- Coefficient4 x**3",
         "    0.2,                     !- Minimum Value of x",
         "    1.0,                     !- Maximum Value of x",
         "    0.0,                     !- Minimum Curve Output",
         "    1.0,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Curve:Quadratic,",
         "    VS tower UA mod func air flow ratio,  !- Name",
         "    0.0,                     !- Coefficient1 Constant",
         "    1.3,                     !- Coefficient2 x",
         "    -0.3,                    !- Coefficient3 x**2",
         "    0.2,                     !- Minimum Value of x",
         "    1.0,                     !- Maximum Value of x",
         "    0.0,                     !- Minimum Curve Output",
         "    1.0,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Curve:Linear,",
         "    VS tower UA mod func wetbulb difference,  !- Name",
         "    1.0,                     !- Coefficient1 Constant",
         "    0.0081,                  !- Coefficient2 x",
         "    -10,                     !- Minimum Value of x",
         "    25.0,                    !- Maximum Value of x",
         "    0.85,                    !- Minimum Curve Output",
         "    1.3,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Curve:Quadratic,",
         "    VS tower UA mod func water flow ratio,  !- Name",
         "    0.1082,                  !- Coefficient1 Constant",
         "    1.667,                   !- Coefficient2 x",
         "    -0.7713,                 !- Coefficient3 x**2",
         "    0.3,                     !- Minimum Value of x",
         "    1.0,                     !- Maximum Value of x",
         "    0.5,                     !- Minimum Curve Output",
         "    1.1,                     !- Maximum Curve Output",
         "    Dimensionless,           !- Input Unit Type for X",
         "    Dimensionless;           !- Output Unit Type",

         "  Pump:ConstantSpeed,",
         "    TowerWaterSys Pump,      !- Name",
         "    TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
         "    0.002,                !- Rated Flow Rate {m3/s}",
         "    0.1,                  !- Rated Pump Head {Pa}",
         "    200.0,                !- Rated Power Consumption {W}",
         "    0.87,                    !- Motor Efficiency",
         "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
         "    Intermittent;            !- Pump Control Type",

         "  CondenserEquipmentList,",
         "    TowerWaterSys Equipment List,  !- Name",
         "    CoolingTower:VariableSpeed:Merkel,  !- Equipment 1 Object Type",
         "    TowerWaterSys CoolTower1;!- Equipment 1 Name",

         "  CondenserLoop,",
         "    TowerWaterSys,           !- Name",
         "    Water,                   !- Fluid Type",
         "    ,                        !- User Defined Fluid Type",
         "    TowerWaterSys Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
         "    TowerWaterSys Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
         "    80.0,                    !- Maximum Loop Temperature {C}",
         "    5.0,                     !- Minimum Loop Temperature {C}",
         "    0.002,                !- Maximum Loop Flow Rate {m3/s}",
         "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
         "    0.1,                !- Condenser Loop Volume {m3}",
         "    TowerWaterSys Supply Inlet Node,  !- Condenser Side Inlet Node Name",
         "    TowerWaterSys Supply Outlet Node,  !- Condenser Side Outlet Node Name",
         "    TowerWaterSys Supply Branches,  !- Condenser Side Branch List Name",
         "    TowerWaterSys Supply Connectors,  !- Condenser Side Connector List Name",
         "    TowerWaterSys Demand Inlet Node,  !- Demand Side Inlet Node Name",
         "    TowerWaterSys Demand Outlet Node,  !- Demand Side Outlet Node Name",
         "    TowerWaterSys Demand Branches,  !- Condenser Demand Side Branch List Name",
         "    TowerWaterSys Demand Connectors,  !- Condenser Demand Side Connector List Name",
         "    SequentialLoad,          !- Load Distribution Scheme",
         "    ,                        !- Pressure Simulation Type",
         "    ;                        !- Loop Circulation Time",

         "  CondenserEquipmentOperationSchemes,",
         "    TowerWaterSys Loop Operation Scheme List,  !- Name",
         "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
         "    TowerWaterSys Operation Scheme,  !- Control Scheme 1 Name",
         "    CONSTANT-1.0;               !- Control Scheme 1 Schedule Name",

         "  PlantEquipmentOperation:CoolingLoad,",
         "    TowerWaterSys Operation Scheme,  !- Name",
         "    0.0,                     !- Load Range 1 Lower Limit {W}",
         "    1000000000000,           !- Load Range 1 Upper Limit {W}",
         "    TowerWaterSys Equipment List;  !- Range 1 Equipment List Name",

         "  SetpointManager:Scheduled,",
         "    TowerWaterSys Setpoint Manager,  !- Name",
         "    Temperature,             !- Control Variable",
         "    TowerWaterSys Temp Sch,      !- Schedule Name",
         "    TowerWaterSys Supply Outlet Node;  !- Setpoint Node or NodeList Name",

         "  Schedule:Compact,",
         "    TowerWaterSys Temp Sch,      !- Name",
         "    Any Number,              !- Schedule Type Limits Name",
         "    THROUGH: 12/31,          !- Field 1",
         "    FOR: AllDays,            !- Field 2",
         "    UNTIL: 24:00,30.0;       !- Field 3",

         "  BranchList,",
         "    TowerWaterSys Demand Branches,  !- Name",
         "    TowerWaterSys Demand Inlet Branch,  !- Branch 1 Name",
         "    TowerWaterSys Demand Load Branch 1,  !- Branch 2 Name",
         "    TowerWaterSys Demand Bypass Branch,  !- Branch 4 Name",
         "    TowerWaterSys Demand Outlet Branch;  !- Branch 5 Name",

         "  BranchList,",
         "    TowerWaterSys Supply Branches,  !- Name",
         "    TowerWaterSys Supply Inlet Branch,  !- Branch 1 Name",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Branch 2 Name",
         "    TowerWaterSys Supply Equipment Bypass Branch,  !- Branch 4 Name",
         "    TowerWaterSys Supply Outlet Branch;  !- Branch 5 Name",

         "  Branch,",
         "    TowerWaterSys Demand Bypass Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
         "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Demand Inlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Demand Inlet Pipe,  !- Component 1 Name",
         "    TowerWaterSys Demand Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Demand Load Branch 1,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    LoadProfile:Plant,  !- Component 1 Object Type",
         "    Load Profile 1,      !- Component 1 Name",
         "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
         "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

         "  LoadProfile:Plant,",
         "    Load Profile 1,          !- Name",
         "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
         "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
         "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
         "    0.002,                   !- Peak Flow Rate {m3/s}",
         "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

         "  Schedule:Compact,",
         "    Load Profile 1 Load Schedule,  !- Name",
         "    Any Number,              !- Schedule Type Limits Name",
         "    THROUGH: 12/31,          !- Field 1",
         "    FOR: AllDays,            !- Field 2",
         "    UNTIL: 24:00,0.0;        !- Field 3",

         "  Schedule:Compact,",
         "    Load Profile 1 Flow Frac Schedule,  !- Name",
         "    Any Number,              !- Schedule Type Limits Name",
         "    THROUGH: 12/31,          !- Field 1",
         "    FOR: AllDays,            !- Field 2",
         "    UNTIL: 24:00,1.0;        !- Field 3",

         "  Branch,",
         "    TowerWaterSys Demand Outlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Demand Outlet Pipe,  !- Component 1 Name",
         "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Demand Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    CoolingTower:VariableSpeed:Merkel,  !- Component 1 Object Type",
         "    TowerWaterSys CoolTower1,!- Component 1 Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Supply Equipment Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Equipment Bypass Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Component 1 Name",
         "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Inlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pump:ConstantSpeed,      !- Component 1 Object Type",
         "    TowerWaterSys Pump,      !- Component 1 Name",
         "    TowerWaterSys Supply Inlet Node,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

         "  Branch,",
         "    TowerWaterSys Supply Outlet Branch,  !- Name",
         "    ,                        !- Pressure Drop Curve Name",
         "    Pipe:Adiabatic,          !- Component 1 Object Type",
         "    TowerWaterSys Supply Outlet Pipe,  !- Component 1 Name",
         "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
         "    TowerWaterSys Supply Outlet Node;  !- Component 1 Outlet Node Name",

         "  OutdoorAir:Node,",
         "    TowerWaterSys CoolTower OA ref Node;  !- Name",

         "  ConnectorList,",
         "    TowerWaterSys Demand Connectors,  !- Name",
         "    Connector:Splitter,      !- Connector 1 Object Type",
         "    TowerWaterSys Demand Splitter,  !- Connector 1 Name",
         "    Connector:Mixer,         !- Connector 2 Object Type",
         "    TowerWaterSys Demand Mixer;  !- Connector 2 Name",

         "  ConnectorList,",
         "    TowerWaterSys Supply Connectors,  !- Name",
         "    Connector:Splitter,      !- Connector 1 Object Type",
         "    TowerWaterSys Supply Splitter,  !- Connector 1 Name",
         "    Connector:Mixer,         !- Connector 2 Object Type",
         "    TowerWaterSys Supply Mixer;  !- Connector 2 Name",

         "  Connector:Splitter,",
         "    TowerWaterSys Demand Splitter,  !- Name",
         "    TowerWaterSys Demand Inlet Branch,  !- Inlet Branch Name",
         "    TowerWaterSys Demand Load Branch 1,  !- Outlet Branch 1 Name",
         "    TowerWaterSys Demand Bypass Branch;  !- Outlet Branch 3 Name",

         "  Connector:Splitter,",
         "    TowerWaterSys Supply Splitter,  !- Name",
         "    TowerWaterSys Supply Inlet Branch,  !- Inlet Branch Name",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
         "    TowerWaterSys Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

         "  Connector:Mixer,",
         "    TowerWaterSys Demand Mixer,  !- Name",
         "    TowerWaterSys Demand Outlet Branch,  !- Outlet Branch Name",
         "    TowerWaterSys Demand Load Branch 1,  !- Inlet Branch 1 Name",
         "    TowerWaterSys Demand Bypass Branch;  !- Inlet Branch 3 Name",

         "  Connector:Mixer,",
         "    TowerWaterSys Supply Mixer,  !- Name",
         "    TowerWaterSys Supply Outlet Branch,  !- Outlet Branch Name",
         "    TowerWaterSys Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
         "    TowerWaterSys Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Demand Bypass Pipe,  !- Name",
         "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Demand Inlet Pipe,  !- Name",
         "    TowerWaterSys Demand Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Demand Outlet Pipe,  !- Name",
         "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Inlet Node Name",
         "    TowerWaterSys Demand Outlet Node;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Name",
         "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
         "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

         "  Pipe:Adiabatic,",
         "    TowerWaterSys Supply Outlet Pipe,  !- Name",
         "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Inlet Node Name",
         "    TowerWaterSys Supply Outlet Node;  !- Outlet Node Name"

        });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);
    CondenserLoopTowers::GetTowerInput(*state);

    // sized using user inputs in cooling tower instead of plant sizing object
    state->dataCondenserLoopTowers->towers(1).SizeVSMerkelTower(*state);

    // input not needed for sizing (NOT WasAutoSized)
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUAWasAutoSized);
    EXPECT_NEAR(
        state->dataCondenserLoopTowers->towers(1).HighSpeedTowerUA, 9770.0, 1.0); // nominal capacity input was 100 kW, approach, 3.9K, range 5.5K

    // input not needed for sizing (NOT WasAutoSized)
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate, 0.005382, 0.00001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate,
                     5.382e-8 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate, 2.7632, 0.0001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity *
                         state->dataCondenserLoopTowers->towers(1).DesignAirFlowPerUnitNomCap);

    // autosized input
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPowerWasAutoSized);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower, 1050);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).HighSpeedFanPower,
                     0.0105 * state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity);

    // input not needed for sizing (NOT WasAutoSized)
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRateWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRate, 0.27632, 0.00001);
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRate,
                     state->dataCondenserLoopTowers->towers(1).FreeConvAirFlowRateSizingFactor *
                         state->dataCondenserLoopTowers->towers(1).HighSpeedAirFlowRate);

    // autosized input
    EXPECT_FALSE(state->dataCondenserLoopTowers->towers(1).FreeConvTowerUAWasAutoSized);
    EXPECT_NEAR(state->dataCondenserLoopTowers->towers(1).FreeConvTowerUA, 590.0, 1.0);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_TwoSpeedTowerLowSpeedNomCapSizing)
{

    Real64 LowSpeedCoolTowerNomCap(0.0);

    std::string const idf_objects =
        delimited_string({"  Site:Location,",
                          "    USA IL-CHICAGO-OHARE,    !- Name",
                          "    41.77,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190;                     !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
                          "    1.00;                    !- Sky Clearness",

                          "    SimulationControl,",
                          "    no,                     !- Do Zone Sizing Calculation",
                          "    no,                     !- Do System Sizing Calculation",
                          "    Yes,                    !- Do Plant Sizing Calculation",
                          "    Yes,                    !- Run Simulation for Sizing Periods",
                          "    no;                     !- Run Simulation for Weather File Run Periods",

                          "  Timestep,6;",

                          "  ScheduleTypeLimits,",
                          "    Any Number;              !- Name",

                          "  CoolingTower:TwoSpeed,",
                          "    TowerWaterSys CoolTower1,!- Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
                          "    ,                        !- Design Water Flow Rate {m3/s}",
                          "    autosize,                !- High Fan Speed Air Flow Rate {m3/s}",
                          "    autosize,                !- High Fan Speed Fan Power {W}",
                          "    ,                        !- High Fan Speed U-Factor Times Area Value {W/K}",
                          "    autocalculate,           !- Low Fan Speed Air Flow Rate {m3/s}",
                          "    0.5000,                  !- Low Fan Speed Air Flow Rate Sizing Factor",
                          "    autocalculate,           !- Low Fan Speed Fan Power {W}",
                          "    0.1600,                  !- Low Fan Speed Fan Power Sizing Factor",
                          "    ,                        !- Low Fan Speed U-Factor Times Area Value {W/K}",
                          "    0.6000,                  !- Low Fan Speed U-Factor Times Area Sizing Factor",
                          "    autocalculate,           !- Free Convection Regime Air Flow Rate {m3/s}",
                          "    0.1000,                  !- Free Convection Regime Air Flow Rate Sizing Factor",
                          "    ,                        !- Free Convection Regime U-Factor Times Area Value {W/K}",
                          "    0.1000,                  !- Free Convection U-Factor Times Area Value Sizing Factor",
                          "    NominalCapacity,         !- Performance Input Method",
                          "    1.2500,                  !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
                          "    100000.0000,             !- High Speed Nominal Capacity {W}",
                          "    autocalculate,           !- Low Speed Nominal Capacity {W}",
                          "    0.5000,                  !- Low Speed Nominal Capacity Sizing Factor",
                          "    10000.0000,              !- Free Convection Nominal Capacity {W}",
                          "    0.1000,                  !- Free Convection Nominal Capacity Sizing Factor",
                          "    ,                        !- Design Inlet Air Dry-Bulb Temperature {C}",
                          "    ,                        !- Design Inlet Air Wet-Bulb Temperature {C}",
                          "    ,                        !- Design Approach Temperature {deltaC}",
                          "    ,                        !- Design Range Temperature {deltaC}",
                          "    0.00,                    !- Basin Heater Capacity {W/K}",
                          "    2.00,                    !- Basin Heater Setpoint Temperature {C}",
                          "    On 24/7,                 !- Basin Heater Operating Schedule Name",
                          "    SaturatedExit,           !- Evaporation Loss Mode",
                          "    0.2000,                  !- Evaporation Loss Factor {percent/K}",
                          "    0,                       !- Drift Loss Percent {percent}",
                          "    ConcentrationRatio,      !- Blowdown Calculation Mode",
                          "    3.00,                    !- Blowdown Concentration Ratio",
                          "    On 24/7,                 !- Blowdown Makeup Water Usage Schedule Name",
                          "    ,                        !- Supply Water Storage Tank Name",
                          "    ,                        !- Outdoor Air Inlet Node Name",
                          "    ,                        !- Number of Cells",
                          "    MinimalCell,             !- Cell Control",
                          "    0.3300,                  !- Cell Minimum  Water Flow Rate Fraction",
                          "    2.5000,                  !- Cell Maximum Water Flow Rate Fraction",
                          "    1.00;                    !- Sizing Factor",

                          "  Pump:ConstantSpeed,",
                          "    TowerWaterSys Pump,      !- Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
                          "    0.002,                !- Rated Flow Rate {m3/s}",
                          "    0.1,                  !- Rated Pump Head {Pa}",
                          "    200.0,                !- Rated Power Consumption {W}",
                          "    0.87,                    !- Motor Efficiency",
                          "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "  CondenserEquipmentList,",
                          "    TowerWaterSys Equipment List,  !- Name",
                          "    CoolingTower:TwoSpeed,   !- Equipment 1 Object Type",
                          "    TowerWaterSys CoolTower1;!- Equipment 1 Name",

                          "  CondenserLoop,",
                          "    TowerWaterSys,           !- Name",
                          "    Water,                   !- Fluid Type",
                          "    ,                        !- User Defined Fluid Type",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
                          "    TowerWaterSys Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
                          "    80.0,                    !- Maximum Loop Temperature {C}",
                          "    5.0,                     !- Minimum Loop Temperature {C}",
                          "    0.002,                   !- Maximum Loop Flow Rate {m3/s}",
                          "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
                          "    0.1,                     !- Condenser Loop Volume {m3}",
                          "    TowerWaterSys Supply Inlet Node,  !- Condenser Side Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node, !- Condenser Side Outlet Node Name",
                          "    TowerWaterSys Supply Branches,    !- Condenser Side Branch List Name",
                          "    TowerWaterSys Supply Connectors,  !- Condenser Side Connector List Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Demand Side Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node, !- Demand Side Outlet Node Name",
                          "    TowerWaterSys Demand Branches,    !- Condenser Demand Side Branch List Name",
                          "    TowerWaterSys Demand Connectors,  !- Condenser Demand Side Connector List Name",
                          "    SequentialLoad,,;                   !- Load Distribution Scheme",

                          "  CondenserEquipmentOperationSchemes,",
                          "    TowerWaterSys Loop Operation Scheme List,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    TowerWaterSys Operation Scheme,  !- Control Scheme 1 Name",
                          "    CONSTANT-1.0;                       !- Control Scheme 1 Schedule Name",

                          "  PlantEquipmentOperation:CoolingLoad,",
                          "    TowerWaterSys Operation Scheme,  !- Name",
                          "    0.0,                             !- Load Range 1 Lower Limit {W}",
                          "    1000000000000,                   !- Load Range 1 Upper Limit {W}",
                          "    TowerWaterSys Equipment List;    !- Range 1 Equipment List Name",

                          "  SetpointManager:Scheduled,",
                          "    TowerWaterSys Setpoint Manager,  !- Name",
                          "    Temperature,                     !- Control Variable",
                          "    TowerWaterSys Temp Sch,          !- Schedule Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Setpoint Node or NodeList Name",

                          "  Schedule:Compact,",
                          "    TowerWaterSys Temp Sch,      !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,30.0;       !- Field 3",

                          "  BranchList,",
                          "    TowerWaterSys Demand Branches,       !- Name",
                          "    TowerWaterSys Demand Inlet Branch,   !- Branch 1 Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Demand Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Demand Outlet Branch;  !- Branch 5 Name",

                          "  BranchList,",
                          "    TowerWaterSys Supply Branches,      !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Branch 2 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Branch 4 Name",
                          "    TowerWaterSys Supply Outlet Branch;  !- Branch 5 Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Demand Load Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    LoadProfile:Plant,   !- Component 1 Object Type",
                          "    Load Profile 1,      !- Component 1 Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

                          "  LoadProfile:Plant,",
                          "    Load Profile 1,          !- Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
                          "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
                          "    0.002,                   !- Peak Flow Rate {m3/s}",
                          "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Load Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,0.0;        !- Field 3",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Flow Frac Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,1.0;        !- Field 3",

                          "  Branch,",
                          "    TowerWaterSys Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    CoolingTower:TwoSpeed,  !- Component 1 Object Type",
                          "    TowerWaterSys CoolTower1,!- Component 1 Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equipment Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Equipment Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    TowerWaterSys Pump,      !- Component 1 Name",
                          "    TowerWaterSys Supply Inlet Node,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    TowerWaterSys Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Component 1 Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Component 1 Outlet Node Name",

                          "  OutdoorAir:Node,",
                          "    TowerWaterSys CoolTower OA ref Node;  !- Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Demand Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Demand Mixer;  !- Connector 2 Name",

                          "  ConnectorList,",
                          "    TowerWaterSys Supply Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    TowerWaterSys Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    TowerWaterSys Supply Mixer;  !- Connector 2 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Demand Splitter,  !- Name",
                          "    TowerWaterSys Demand Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Splitter,",
                          "    TowerWaterSys Supply Splitter,  !- Name",
                          "    TowerWaterSys Supply Inlet Branch,  !- Inlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Demand Mixer,  !- Name",
                          "    TowerWaterSys Demand Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Demand Load Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Demand Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    TowerWaterSys Supply Mixer,  !- Name",
                          "    TowerWaterSys Supply Outlet Branch,  !- Outlet Branch Name",
                          "    TowerWaterSys Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
                          "    TowerWaterSys Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Bypass Pipe,  !- Name",
                          "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Inlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Inlet Pipe-TowerWaterSys Demand Mixer;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Demand Outlet Pipe,  !- Name",
                          "    TowerWaterSys Demand Mixer-TowerWaterSys Demand Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Demand Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Equipment Bypass Pipe,  !- Name",
                          "    TowerWaterSys Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    TowerWaterSys Supply Outlet Pipe,  !- Name",
                          "    TowerWaterSys Supply Mixer-TowerWaterSys Supply Outlet Pipe,  !- Inlet Node Name",
                          "    TowerWaterSys Supply Outlet Node;  !- Outlet Node Name"

        });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    bool ErrorsFound = false;

    state->dataGlobal->BeginSimFlag = true;

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    Weather::ResetEnvironmentCounter(*state);
    SimulationManager::SetupSimulation(*state, ErrorsFound);

    // get inputs of cooling tower object
    CondenserLoopTowers::GetTowerInput(*state);
    // check the low speed nominal capacity field is autosized
    EXPECT_TRUE(state->dataCondenserLoopTowers->towers(1).TowerLowSpeedNomCapWasAutoSized);
    // check user input value for high speed nominal capacity
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity, 100000.0);

    // autosized other input fields of cooling tower
    state->dataCondenserLoopTowers->towers(1).SizeTower(*state);
    // size low speed nominal capacity
    LowSpeedCoolTowerNomCap =
        state->dataCondenserLoopTowers->towers(1).TowerNominalCapacity * state->dataCondenserLoopTowers->towers(1).TowerLowSpeedNomCapSizingFactor;
    EXPECT_DOUBLE_EQ(state->dataCondenserLoopTowers->towers(1).TowerLowSpeedNomCap, LowSpeedCoolTowerNomCap);
    // check the low speed nominal capacity is higher than that of free convection nominal capacity
    EXPECT_GT(state->dataCondenserLoopTowers->towers(1).TowerLowSpeedNomCap, state->dataCondenserLoopTowers->towers(1).TowerFreeConvNomCap);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_SingleSpeedUser_SizingError_SizingPlant)
{
    std::string const idf_objects = delimited_string({

        // General Stuff
        "Timestep, 4;",

        "Site:Location,",
        "  Chicago Ohare Intl Ap,                  !- Name",
        "  41.98,                                  !- Latitude {deg}",
        "  -87.92,                                 !- Longitude {deg}",
        "  -6,                                     !- Time Zone {hr}",
        "  201;                                    !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        "  CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
        "  7,                       !- Month",
        "  21,                      !- Day of Month",
        "  SummerDesignDay,         !- Day Type",
        "  31.2,                    !- Maximum Dry-Bulb Temperature {C}",
        "  10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                 !- Humidity Condition Type",
        "  25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                        !- Humidity Condition Day Schedule Name",
        "  ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  99063.,                  !- Barometric Pressure {Pa}",
        "  5.3,                     !- Wind Speed {m/s}",
        "  230,                     !- Wind Direction {deg}",
        "  No,                      !- Rain Indicator",
        "  No,                      !- Snow Indicator",
        "  No,                      !- Daylight Saving Time Indicator",
        "  ASHRAEClearSky,          !- Solar Model Indicator",
        "  ,                        !- Beam Solar Day Schedule Name",
        "  ,                        !- Diffuse Solar Day Schedule Name",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  1.00;                    !- Sky Clearness",

        "SimulationControl,",
        "  No,                                    !- Do Zone Sizing Calculation",
        "  No,                                    !- Do System Sizing Calculation",
        "  Yes,                                   !- Do Plant Sizing Calculation",
        "  Yes,                                   !- Run Simulation for Sizing Periods",
        "  No;                                    !- Run Simulation for Weather File Run Periods",

        "ScheduleTypeLimits, Any Number;",

        // Define a condenser loop, with a CT Single Speed on the supply, and a load profile on the demand side
        // We set the Sizing:Plant design loop exit temperature at 25C, and the Cooling Tower Design Inlet Air WB Temp to 25.6C (78F)
        // in order to trigger an error

        "PlantLoop,",
        "  CndW Loop,                              !- Name",
        "  Water,                                  !- Fluid Type",
        "  ,                                       !- User Defined Fluid Type",
        "  CndW Loop Operation Schemes,            !- Plant Equipment Operation Scheme Name",
        "  CndW Loop Supply Outlet Node,           !- Loop Temperature Setpoint Node Name",
        "  100,                                    !- Maximum Loop Temperature {C}",
        "  0,                                      !- Minimum Loop Temperature {C}",
        "  Autosize,                               !- Maximum Loop Flow Rate {m3/s}",
        "  0,                                      !- Minimum Loop Flow Rate {m3/s}",
        "  Autocalculate,                          !- Plant Loop Volume {m3}",
        "  CndW Loop Supply Inlet Node,            !- Plant Side Inlet Node Name",
        "  CndW Loop Supply Outlet Node,           !- Plant Side Outlet Node Name",
        "  CndW Loop Supply Branches,              !- Plant Side Branch List Name",
        "  CndW Loop Supply Connector List,        !- Plant Side Connector List Name",
        "  CndW Loop Demand Inlet Node,            !- Demand Side Inlet Node Name",
        "  CndW Loop Demand Outlet Node,           !- Demand Side Outlet Node Name",
        "  CndW Loop Demand Branches,              !- Demand Side Branch List Name",
        "  CndW Loop Demand Connector List,        !- Demand Side Connector List Name",
        "  Optimal,                                !- Load Distribution Scheme",
        "  ,                                       !- Availability Manager List Name",
        "  SingleSetpoint,                         !- Plant Loop Demand Calculation Scheme",
        "  ;                                       !- Common Pipe Simulation",

        "Sizing:Plant,",
        "  CndW Loop,                              !- Plant or Condenser Loop Name",
        "  Condenser,                              !- Loop Type",
        "  25,                                     !- Design Loop Exit Temperature {C}",
        "  7,                                      !- Loop Design Temperature Difference {deltaC}",
        "  NonCoincident,                          !- Sizing Option",
        "  1,                                      !- Zone Timesteps in Averaging Window",
        "  None;                                   !- Coincident Sizing Factor Mode",

        "BranchList,",
        "  CndW Loop Supply Branches,              !- Name",
        "  CndW Loop Supply Inlet Branch,          !- Branch Name 1",
        "  CndW Loop Supply CT Branch,              !- Branch Name 2",
        "  CndW Loop Supply Bypass Branch,              !- Branch Name 3",
        "  CndW Loop Supply Outlet Branch;         !- Branch Name 4",

        "ConnectorList,",
        "  CndW Loop Supply Connector List,        !- Name",
        "  Connector:Splitter,                     !- Connector Object Type 1",
        "  CndW Loop Supply Splitter,              !- Connector Name 1",
        "  Connector:Mixer,                        !- Connector Object Type 2",
        "  CndW Loop Supply Mixer;                 !- Connector Name 2",

        "Connector:Splitter,",
        "  CndW Loop Supply Splitter,              !- Name",
        "  CndW Loop Supply Inlet Branch,          !- Inlet Branch Name",
        "  CndW Loop Supply CT Branch,              !- Outlet Branch Name 1",
        "  CndW Loop Supply Bypass Branch;              !- Outlet Branch Name 2",

        "Connector:Mixer,",
        "  CndW Loop Supply Mixer,                 !- Name",
        "  CndW Loop Supply Outlet Branch,         !- Outlet Branch Name",
        "  CndW Loop Supply CT Branch,              !- Inlet Branch Name 1",
        "  CndW Loop Supply Bypass Branch;              !- Inlet Branch Name 2",

        "Branch,",
        "  CndW Loop Supply Inlet Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pump:VariableSpeed,                     !- Component Object Type 1",
        "  VSP Pump,                               !- Component Name 1",
        "  CndW Loop Supply Inlet Node,            !- Component Inlet Node Name 1",
        "  CndW Loop Pump Outlet Node;             !- Component Outlet Node Name 1",

        "Pump:VariableSpeed,",
        "  VSP Pump,                               !- Name",
        "  CndW Loop Supply Inlet Node,            !- Inlet Node Name",
        "  CndW Loop Pump Outlet Node,             !- Outlet Node Name",
        "  Autosize,                               !- Design Maximum Flow Rate {m3/s}",
        "  179352,                                 !- Design Pump Head {Pa}",
        "  Autosize,                               !- Design Power Consumption {W}",
        "  0.9,                                    !- Motor Efficiency",
        "  0,                                      !- Fraction of Motor Inefficiencies to Fluid Stream",
        "  0,                                      !- Coefficient 1 of the Part Load Performance Curve",
        "  1,                                      !- Coefficient 2 of the Part Load Performance Curve",
        "  0,                                      !- Coefficient 3 of the Part Load Performance Curve",
        "  0,                                      !- Coefficient 4 of the Part Load Performance Curve",
        "  0,                                      !- Design Minimum Flow Rate {m3/s}",
        "  Intermittent,                           !- Pump Control Type",
        "  ,                                       !- Pump Flow Rate Schedule Name",
        "  ,                                       !- Pump Curve Name",
        "  ,                                       !- Impeller Diameter {m}",
        "  ,                                       !- VFD Control Type",
        "  ,                                       !- Pump rpm Schedule Name",
        "  ,                                       !- Minimum Pressure Schedule {Pa}",
        "  ,                                       !- Maximum Pressure Schedule {Pa}",
        "  ,                                       !- Minimum RPM Schedule {Rotations Per Minute}",
        "  ,                                       !- Maximum RPM Schedule {Rotations Per Minute}",
        "  ,                                       !- Zone Name",
        "  0.5,                                    !- Skin Loss Radiative Fraction",
        "  PowerPerFlowPerPressure,                !- Design Power Sizing Method",
        "  348701.1,                               !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "  1.282051282,                            !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "  0;                                      !- Design Minimum Flow Rate Fraction",

        "Branch,",
        "  CndW Loop Supply CT Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  CoolingTower:SingleSpeed,               !- Component Object Type 1",
        "  CT Single Speed,                        !- Component Name 1",
        "  CT Single Speed Inlet Node,             !- Component Inlet Node Name 1",
        "  CT Single Speed Outlet Node;            !- Component Outlet Node Name 1",

        "CoolingTower:SingleSpeed,",
        "  CT Single Speed,                        !- Name",
        "  CT Single Speed Inlet Node,             !- Water Inlet Node Name",
        "  CT Single Speed Outlet Node,            !- Water Outlet Node Name",
        "  Autosize,                               !- Design Water Flow Rate {m3/s}",
        "  Autosize,                               !- Design Air Flow Rate {m3/s}",
        "  Autosize,                               !- Design Fan Power {W}",
        "  Autosize,                               !- Design U-Factor Times Area Value {W/K}",
        "  Autosize,                               !- Free Convection Air Flow Rate {m3/s}",
        "  0.1,                                    !- Free Convection Air Flow Rate Sizing Factor",
        "  Autosize,                               !- Free Convection U-Factor Times Area Value {W/K}",
        "  0.1,                                    !- Free Convection U-Factor Times Area Value Sizing Factor",
        "  UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "  1.25,                                   !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "  ,                                       !- Nominal Capacity {W}",
        "  0,                                      !- Free Convection Capacity {W}",
        "  0.1,                                    !- Free Convection Nominal Capacity Sizing Factor",
        "  25.6,                                   !- Design Inlet Air Dry-Bulb Temperature {C}",
        "  25.6,                                   !- Design Inlet Air Wet-Bulb Temperature {C}",
        "  Autosize,                               !- Design Approach Temperature {deltaC}",
        "  Autosize,                               !- Design Range Temperature {deltaC}",
        "  0,                                      !- Basin Heater Capacity {W/K}",
        "  2,                                      !- Basin Heater Setpoint Temperature {C}",
        "  ,                                       !- Basin Heater Operating Schedule Name",
        "  LossFactor,                             !- Evaporation Loss Mode",
        "  0.2,                                    !- Evaporation Loss Factor {percent/K}",
        "  0.008,                                  !- Drift Loss Percent {percent}",
        "  ConcentrationRatio,                     !- Blowdown Calculation Mode",
        "  3,                                      !- Blowdown Concentration Ratio",
        "  ,                                       !- Blowdown Makeup Water Usage Schedule Name",
        "  ,                                       !- Supply Water Storage Tank Name",
        "  ,                                       !- Outdoor Air Inlet Node Name",
        "  FanCycling,                             !- Capacity Control",
        "  1,                                      !- Number of Cells",
        "  MinimalCell,                            !- Cell Control",
        "  0.33,                                   !- Cell Minimum  Water Flow Rate Fraction",
        "  2.5,                                    !- Cell Maximum Water Flow Rate Fraction",
        "  1,                                      !- Sizing Factor",
        "  General;                                !- End-Use Subcategory",

        "Branch,",
        "  CndW Loop Supply Bypass Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Supply Bypass Pipe,                     !- Component Name 1",
        "  Supply Bypass Pipe Inlet Node,          !- Component Inlet Node Name 1",
        "  Supply Bypass Pipe Outlet Node;         !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Supply Bypass Pipe,                     !- Name",
        "  Supply Bypass Pipe Inlet Node,          !- Inlet Node Name",
        "  Supply Bypass Pipe Outlet Node;         !- Outlet Node Name",

        "Branch,",
        "  CndW Loop Supply Outlet Branch,         !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Supply Outlet Pipe,                     !- Component Name 1",
        "  Supply Outlet Pipe Inlet Node,          !- Component Inlet Node Name 1",
        "  CndW Loop Supply Outlet Node;           !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Supply Outlet Pipe,                     !- Name",
        "  Supply Outlet Pipe Inlet Node,          !- Inlet Node Name",
        "  CndW Loop Supply Outlet Node;           !- Outlet Node Name",

        "SetpointManager:FollowOutdoorAirTemperature,",
        "  SPM Follow OAT 7F,                      !- Name",
        "  Temperature,                            !- Control Variable",
        "  OutdoorAirWetBulb,                      !- Reference Temperature Type",
        "  3.88888888888889,                       !- Offset Temperature Difference {deltaC}",
        "  80,                                     !- Maximum Setpoint Temperature {C}",
        "  6,                                      !- Minimum Setpoint Temperature {C}",
        "  CndW Loop Supply Outlet Node;           !- Setpoint Node or NodeList Name",

        "BranchList,",
        "  CndW Loop Demand Branches,              !- Name",
        "  CndW Loop Demand Inlet Branch,          !- Branch Name 1",
        "  CndW Loop Demand Load Profile Branch,              !- Branch Name 2",
        "  CndW Loop Demand Bypass Branch,         !- Branch Name 3",
        "  CndW Loop Demand Outlet Branch;         !- Branch Name 4",

        "ConnectorList,",
        "  CndW Loop Demand Connector List,        !- Name",
        "  Connector:Splitter,                     !- Connector Object Type 1",
        "  CndW Loop Demand Splitter,              !- Connector Name 1",
        "  Connector:Mixer,                        !- Connector Object Type 2",
        "  CndW Loop Demand Mixer;                 !- Connector Name 2",

        "Connector:Splitter,",
        "  CndW Loop Demand Splitter,              !- Name",
        "  CndW Loop Demand Inlet Branch,          !- Inlet Branch Name",
        "  CndW Loop Demand Load Profile Branch,              !- Outlet Branch Name 1",
        "  CndW Loop Demand Bypass Branch;         !- Outlet Branch Name 2",

        "Connector:Mixer,",
        "  CndW Loop Demand Mixer,                 !- Name",
        "  CndW Loop Demand Outlet Branch,         !- Outlet Branch Name",
        "  CndW Loop Demand Load Profile Branch,              !- Inlet Branch Name 1",
        "  CndW Loop Demand Bypass Branch;         !- Inlet Branch Name 2",

        "Branch,",
        "  CndW Loop Demand Inlet Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Demand Inlet Pipe,                      !- Component Name 1",
        "  CndW Loop Demand Inlet Node,            !- Component Inlet Node Name 1",
        "  Demand Inlet Pipe Outlet Node;          !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Demand Inlet Pipe,                      !- Name",
        "  CndW Loop Demand Inlet Node,            !- Inlet Node Name",
        "  Demand Inlet Pipe Outlet Node;          !- Outlet Node Name",

        "Branch,",
        "  CndW Loop Demand Load Profile Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  LoadProfile:Plant,                      !- Component Object Type 1",
        "  Load Profile,                           !- Component Name 1",
        "  Load Profile Inlet Node,                !- Component Inlet Node Name 1",
        "  Load Profile Outlet Node;               !- Component Outlet Node Name 1",

        "LoadProfile:Plant,",
        "  Load Profile,                           !- Name",
        "  Load Profile Inlet Node,                !- Inlet Node Name",
        "  Load Profile Outlet Node,               !- Outlet Node Name",
        "  Load Profile Load Schedule,             !- Load Schedule Name",
        "  2.0,                                    !- Peak Flow Rate {m3/s}",
        "  Load Profile Flow Frac Schedule;        !- Flow Rate Fraction Schedule Name",

        "Schedule:Compact,",
        "  Load Profile Load Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  THROUGH: 12/31,          !- Field 1",
        "  FOR: AllDays,            !- Field 2",
        "  UNTIL: 24:00,-10000.0;   !- Field 3",

        "Schedule:Compact,",
        "  Load Profile Flow Frac Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  THROUGH: 12/31,          !- Field 1",
        "  FOR: AllDays,            !- Field 2",
        "  UNTIL: 24:00,1.0;        !- Field 3",

        "Branch,",
        "  CndW Loop Demand Bypass Branch,         !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  CndW Loop Demand Bypass Pipe,           !- Component Name 1",
        "  CndW Loop Demand Bypass Pipe Inlet Node, !- Component Inlet Node Name 1",
        "  CndW Loop Demand Bypass Pipe Outlet Node; !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  CndW Loop Demand Bypass Pipe,           !- Name",
        "  CndW Loop Demand Bypass Pipe Inlet Node, !- Inlet Node Name",
        "  CndW Loop Demand Bypass Pipe Outlet Node; !- Outlet Node Name",

        "Branch,",
        "  CndW Loop Demand Outlet Branch,         !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Demand Outlet Pipe,                     !- Component Name 1",
        "  Demand Outlet Pipe Inlet Node,          !- Component Inlet Node Name 1",
        "  CndW Loop Demand Outlet Node;           !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Demand Outlet Pipe,                     !- Name",
        "  Demand Outlet Pipe Inlet Node,          !- Inlet Node Name",
        "  CndW Loop Demand Outlet Node;           !- Outlet Node Name",

        "PlantEquipmentOperationSchemes,",
        "  CndW Loop Operation Schemes,            !- Name",
        "  PlantEquipmentOperation:CoolingLoad,    !- Control Scheme Object Type 1",
        "  CndW Loop Cooling Operation Scheme,     !- Control Scheme Name 1",
        "  Always On Discrete;                     !- Control Scheme Schedule Name 1",

        "PlantEquipmentOperation:CoolingLoad,",
        "  CndW Loop Cooling Operation Scheme,     !- Name",
        "  0,                                      !- Load Range Lower Limit 1 {W}",
        "  1000000000,                             !- Load Range Upper Limit 1 {W}",
        "  CndW Loop Cooling Equipment List;       !- Range Equipment List Name 1",

        "PlantEquipmentList,",
        "  CndW Loop Cooling Equipment List,       !- Name",
        "  CoolingTower:SingleSpeed,               !- Equipment Object Type 1",
        "  CT Single Speed;                        !- Equipment Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    state->dataGlobal->BeginSimFlag = true;

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);

    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.
    // Get plant loop data
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    SizingManager::GetPlantSizingInput(*state);
    PlantManager::InitOneTimePlantSizingInfo(*state, 1);
    PlantManager::SizePlantLoop(*state, 1, true);

    // Fake having more than small load
    state->dataSize->PlantSizData(1).DesVolFlowRate = 1000.0;

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    // autosized other input fields of cooling tower. Tt throws, so we catch that so we can compare the error
    ASSERT_THROW(state->dataCondenserLoopTowers->towers(1).SizeTower(*state), std::runtime_error);

    std::string const error_string = delimited_string({

        "   ** Severe  ** Error when autosizing the UA value for cooling tower = CT SINGLE SPEED. Design Loop Exit Temperature must be greater than "
        "25.60 C when autosizing the tower UA.",
        "   **   ~~~   ** The Design Loop Exit Temperature specified in Sizing:Plant object = CNDW LOOP (25.00 C)",
        "   **   ~~~   ** is less than or equal to the design inlet air wet-bulb temperature of 25.60 C.",
        "   **   ~~~   ** If using HVACTemplate:Plant:ChilledWaterLoop, then check that input field Condenser Water Design Setpoint must be > 25.60 "
        "C if autosizing the cooling tower.",
        "   **  Fatal  ** Autosizing of cooling tower fails for tower = CT SINGLE SPEED.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Error when autosizing the UA value for cooling tower = CT SINGLE SPEED. Design Loop Exit Temperature must be "
        "greater than 25.60 C when autosizing the tower UA."});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_SingleSpeedUser_SizingError_UserSpecified)
{
    std::string const idf_objects = delimited_string({

        // General Stuff
        "Timestep, 4;",

        "Site:Location,",
        "  Chicago Ohare Intl Ap,                  !- Name",
        "  41.98,                                  !- Latitude {deg}",
        "  -87.92,                                 !- Longitude {deg}",
        "  -6,                                     !- Time Zone {hr}",
        "  201;                                    !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        "  CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
        "  7,                       !- Month",
        "  21,                      !- Day of Month",
        "  SummerDesignDay,         !- Day Type",
        "  31.2,                    !- Maximum Dry-Bulb Temperature {C}",
        "  10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                 !- Humidity Condition Type",
        "  25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                        !- Humidity Condition Day Schedule Name",
        "  ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  99063.,                  !- Barometric Pressure {Pa}",
        "  5.3,                     !- Wind Speed {m/s}",
        "  230,                     !- Wind Direction {deg}",
        "  No,                      !- Rain Indicator",
        "  No,                      !- Snow Indicator",
        "  No,                      !- Daylight Saving Time Indicator",
        "  ASHRAEClearSky,          !- Solar Model Indicator",
        "  ,                        !- Beam Solar Day Schedule Name",
        "  ,                        !- Diffuse Solar Day Schedule Name",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  1.00;                    !- Sky Clearness",

        "SimulationControl,",
        "  No,                                    !- Do Zone Sizing Calculation",
        "  No,                                    !- Do System Sizing Calculation",
        "  Yes,                                   !- Do Plant Sizing Calculation",
        "  Yes,                                   !- Run Simulation for Sizing Periods",
        "  No;                                    !- Run Simulation for Weather File Run Periods",

        "ScheduleTypeLimits, Any Number;",

        // Define a condenser loop, with a CT Single Speed on the supply, and a load profile on the demand side
        // We set the Sizing:Plant design loop exit temperature at 25C, and the Cooling Tower Design Inlet Air WB Temp to 25.6C (78F)
        // in order to trigger an error

        "PlantLoop,",
        "  CndW Loop,                              !- Name",
        "  Water,                                  !- Fluid Type",
        "  ,                                       !- User Defined Fluid Type",
        "  CndW Loop Operation Schemes,            !- Plant Equipment Operation Scheme Name",
        "  CndW Loop Supply Outlet Node,           !- Loop Temperature Setpoint Node Name",
        "  100,                                    !- Maximum Loop Temperature {C}",
        "  0,                                      !- Minimum Loop Temperature {C}",
        "  Autosize,                               !- Maximum Loop Flow Rate {m3/s}",
        "  0,                                      !- Minimum Loop Flow Rate {m3/s}",
        "  Autocalculate,                          !- Plant Loop Volume {m3}",
        "  CndW Loop Supply Inlet Node,            !- Plant Side Inlet Node Name",
        "  CndW Loop Supply Outlet Node,           !- Plant Side Outlet Node Name",
        "  CndW Loop Supply Branches,              !- Plant Side Branch List Name",
        "  CndW Loop Supply Connector List,        !- Plant Side Connector List Name",
        "  CndW Loop Demand Inlet Node,            !- Demand Side Inlet Node Name",
        "  CndW Loop Demand Outlet Node,           !- Demand Side Outlet Node Name",
        "  CndW Loop Demand Branches,              !- Demand Side Branch List Name",
        "  CndW Loop Demand Connector List,        !- Demand Side Connector List Name",
        "  Optimal,                                !- Load Distribution Scheme",
        "  ,                                       !- Availability Manager List Name",
        "  SingleSetpoint,                         !- Plant Loop Demand Calculation Scheme",
        "  ;                                       !- Common Pipe Simulation",

        // No Sizing:Plant

        "BranchList,",
        "  CndW Loop Supply Branches,              !- Name",
        "  CndW Loop Supply Inlet Branch,          !- Branch Name 1",
        "  CndW Loop Supply CT Branch,              !- Branch Name 2",
        "  CndW Loop Supply Bypass Branch,              !- Branch Name 3",
        "  CndW Loop Supply Outlet Branch;         !- Branch Name 4",

        "ConnectorList,",
        "  CndW Loop Supply Connector List,        !- Name",
        "  Connector:Splitter,                     !- Connector Object Type 1",
        "  CndW Loop Supply Splitter,              !- Connector Name 1",
        "  Connector:Mixer,                        !- Connector Object Type 2",
        "  CndW Loop Supply Mixer;                 !- Connector Name 2",

        "Connector:Splitter,",
        "  CndW Loop Supply Splitter,              !- Name",
        "  CndW Loop Supply Inlet Branch,          !- Inlet Branch Name",
        "  CndW Loop Supply CT Branch,              !- Outlet Branch Name 1",
        "  CndW Loop Supply Bypass Branch;              !- Outlet Branch Name 2",

        "Connector:Mixer,",
        "  CndW Loop Supply Mixer,                 !- Name",
        "  CndW Loop Supply Outlet Branch,         !- Outlet Branch Name",
        "  CndW Loop Supply CT Branch,              !- Inlet Branch Name 1",
        "  CndW Loop Supply Bypass Branch;              !- Inlet Branch Name 2",

        "Branch,",
        "  CndW Loop Supply Inlet Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pump:VariableSpeed,                     !- Component Object Type 1",
        "  VSP Pump,                               !- Component Name 1",
        "  CndW Loop Supply Inlet Node,            !- Component Inlet Node Name 1",
        "  CndW Loop Pump Outlet Node;             !- Component Outlet Node Name 1",

        "Pump:VariableSpeed,",
        "  VSP Pump,                               !- Name",
        "  CndW Loop Supply Inlet Node,            !- Inlet Node Name",
        "  CndW Loop Pump Outlet Node,             !- Outlet Node Name",
        "  Autosize,                               !- Design Maximum Flow Rate {m3/s}",
        "  179352,                                 !- Design Pump Head {Pa}",
        "  Autosize,                               !- Design Power Consumption {W}",
        "  0.9,                                    !- Motor Efficiency",
        "  0,                                      !- Fraction of Motor Inefficiencies to Fluid Stream",
        "  0,                                      !- Coefficient 1 of the Part Load Performance Curve",
        "  1,                                      !- Coefficient 2 of the Part Load Performance Curve",
        "  0,                                      !- Coefficient 3 of the Part Load Performance Curve",
        "  0,                                      !- Coefficient 4 of the Part Load Performance Curve",
        "  0,                                      !- Design Minimum Flow Rate {m3/s}",
        "  Intermittent,                           !- Pump Control Type",
        "  ,                                       !- Pump Flow Rate Schedule Name",
        "  ,                                       !- Pump Curve Name",
        "  ,                                       !- Impeller Diameter {m}",
        "  ,                                       !- VFD Control Type",
        "  ,                                       !- Pump rpm Schedule Name",
        "  ,                                       !- Minimum Pressure Schedule {Pa}",
        "  ,                                       !- Maximum Pressure Schedule {Pa}",
        "  ,                                       !- Minimum RPM Schedule {Rotations Per Minute}",
        "  ,                                       !- Maximum RPM Schedule {Rotations Per Minute}",
        "  ,                                       !- Zone Name",
        "  0.5,                                    !- Skin Loss Radiative Fraction",
        "  PowerPerFlowPerPressure,                !- Design Power Sizing Method",
        "  348701.1,                               !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "  1.282051282,                            !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "  0;                                      !- Design Minimum Flow Rate Fraction",

        "Branch,",
        "  CndW Loop Supply CT Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  CoolingTower:SingleSpeed,               !- Component Object Type 1",
        "  CT Single Speed,                        !- Component Name 1",
        "  CT Single Speed Inlet Node,             !- Component Inlet Node Name 1",
        "  CT Single Speed Outlet Node;            !- Component Outlet Node Name 1",

        "CoolingTower:SingleSpeed,",
        "  CT Single Speed,                        !- Name",
        "  CT Single Speed Inlet Node,             !- Water Inlet Node Name",
        "  CT Single Speed Outlet Node,            !- Water Outlet Node Name",
        "  Autosize,                               !- Design Water Flow Rate {m3/s}",
        "  Autosize,                               !- Design Air Flow Rate {m3/s}",
        "  Autosize,                               !- Design Fan Power {W}",
        "  Autosize,                               !- Design U-Factor Times Area Value {W/K}",
        "  Autosize,                               !- Free Convection Air Flow Rate {m3/s}",
        "  0.1,                                    !- Free Convection Air Flow Rate Sizing Factor",
        "  Autosize,                               !- Free Convection U-Factor Times Area Value {W/K}",
        "  0.1,                                    !- Free Convection U-Factor Times Area Value Sizing Factor",
        "  UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "  1.25,                                   !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "  ,                                       !- Nominal Capacity {W}",
        "  0,                                      !- Free Convection Capacity {W}",
        "  0.1,                                    !- Free Convection Nominal Capacity Sizing Factor",
        "  25.6,                                   !- Design Inlet Air Dry-Bulb Temperature {C}",
        "  25.6,                                   !- Design Inlet Air Wet-Bulb Temperature {C}",
        "  Autosize,                               !- Design Approach Temperature {deltaC}",
        "  Autosize,                               !- Design Range Temperature {deltaC}",
        "  0,                                      !- Basin Heater Capacity {W/K}",
        "  2,                                      !- Basin Heater Setpoint Temperature {C}",
        "  ,                                       !- Basin Heater Operating Schedule Name",
        "  LossFactor,                             !- Evaporation Loss Mode",
        "  0.2,                                    !- Evaporation Loss Factor {percent/K}",
        "  0.008,                                  !- Drift Loss Percent {percent}",
        "  ConcentrationRatio,                     !- Blowdown Calculation Mode",
        "  3,                                      !- Blowdown Concentration Ratio",
        "  ,                                       !- Blowdown Makeup Water Usage Schedule Name",
        "  ,                                       !- Supply Water Storage Tank Name",
        "  ,                                       !- Outdoor Air Inlet Node Name",
        "  FanCycling,                             !- Capacity Control",
        "  1,                                      !- Number of Cells",
        "  MinimalCell,                            !- Cell Control",
        "  0.33,                                   !- Cell Minimum  Water Flow Rate Fraction",
        "  2.5,                                    !- Cell Maximum Water Flow Rate Fraction",
        "  1,                                      !- Sizing Factor",
        "  General;                                !- End-Use Subcategory",

        "Branch,",
        "  CndW Loop Supply Bypass Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Supply Bypass Pipe,                     !- Component Name 1",
        "  Supply Bypass Pipe Inlet Node,          !- Component Inlet Node Name 1",
        "  Supply Bypass Pipe Outlet Node;         !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Supply Bypass Pipe,                     !- Name",
        "  Supply Bypass Pipe Inlet Node,          !- Inlet Node Name",
        "  Supply Bypass Pipe Outlet Node;         !- Outlet Node Name",

        "Branch,",
        "  CndW Loop Supply Outlet Branch,         !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Supply Outlet Pipe,                     !- Component Name 1",
        "  Supply Outlet Pipe Inlet Node,          !- Component Inlet Node Name 1",
        "  CndW Loop Supply Outlet Node;           !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Supply Outlet Pipe,                     !- Name",
        "  Supply Outlet Pipe Inlet Node,          !- Inlet Node Name",
        "  CndW Loop Supply Outlet Node;           !- Outlet Node Name",

        "SetpointManager:FollowOutdoorAirTemperature,",
        "  SPM Follow OAT 7F,                      !- Name",
        "  Temperature,                            !- Control Variable",
        "  OutdoorAirWetBulb,                      !- Reference Temperature Type",
        "  3.88888888888889,                       !- Offset Temperature Difference {deltaC}",
        "  80,                                     !- Maximum Setpoint Temperature {C}",
        "  6,                                      !- Minimum Setpoint Temperature {C}",
        "  CndW Loop Supply Outlet Node;           !- Setpoint Node or NodeList Name",

        "BranchList,",
        "  CndW Loop Demand Branches,              !- Name",
        "  CndW Loop Demand Inlet Branch,          !- Branch Name 1",
        "  CndW Loop Demand Load Profile Branch,              !- Branch Name 2",
        "  CndW Loop Demand Bypass Branch,         !- Branch Name 3",
        "  CndW Loop Demand Outlet Branch;         !- Branch Name 4",

        "ConnectorList,",
        "  CndW Loop Demand Connector List,        !- Name",
        "  Connector:Splitter,                     !- Connector Object Type 1",
        "  CndW Loop Demand Splitter,              !- Connector Name 1",
        "  Connector:Mixer,                        !- Connector Object Type 2",
        "  CndW Loop Demand Mixer;                 !- Connector Name 2",

        "Connector:Splitter,",
        "  CndW Loop Demand Splitter,              !- Name",
        "  CndW Loop Demand Inlet Branch,          !- Inlet Branch Name",
        "  CndW Loop Demand Load Profile Branch,              !- Outlet Branch Name 1",
        "  CndW Loop Demand Bypass Branch;         !- Outlet Branch Name 2",

        "Connector:Mixer,",
        "  CndW Loop Demand Mixer,                 !- Name",
        "  CndW Loop Demand Outlet Branch,         !- Outlet Branch Name",
        "  CndW Loop Demand Load Profile Branch,              !- Inlet Branch Name 1",
        "  CndW Loop Demand Bypass Branch;         !- Inlet Branch Name 2",

        "Branch,",
        "  CndW Loop Demand Inlet Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Demand Inlet Pipe,                      !- Component Name 1",
        "  CndW Loop Demand Inlet Node,            !- Component Inlet Node Name 1",
        "  Demand Inlet Pipe Outlet Node;          !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Demand Inlet Pipe,                      !- Name",
        "  CndW Loop Demand Inlet Node,            !- Inlet Node Name",
        "  Demand Inlet Pipe Outlet Node;          !- Outlet Node Name",

        "Branch,",
        "  CndW Loop Demand Load Profile Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  LoadProfile:Plant,                      !- Component Object Type 1",
        "  Load Profile,                           !- Component Name 1",
        "  Load Profile Inlet Node,                !- Component Inlet Node Name 1",
        "  Load Profile Outlet Node;               !- Component Outlet Node Name 1",

        "LoadProfile:Plant,",
        "  Load Profile,                           !- Name",
        "  Load Profile Inlet Node,                !- Inlet Node Name",
        "  Load Profile Outlet Node,               !- Outlet Node Name",
        "  Load Profile Load Schedule,             !- Load Schedule Name",
        "  2.0,                                    !- Peak Flow Rate {m3/s}",
        "  Load Profile Flow Frac Schedule;        !- Flow Rate Fraction Schedule Name",

        "Schedule:Compact,",
        "  Load Profile Load Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  THROUGH: 12/31,          !- Field 1",
        "  FOR: AllDays,            !- Field 2",
        "  UNTIL: 24:00,-10000.0;   !- Field 3",

        "Schedule:Compact,",
        "  Load Profile Flow Frac Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  THROUGH: 12/31,          !- Field 1",
        "  FOR: AllDays,            !- Field 2",
        "  UNTIL: 24:00,1.0;        !- Field 3",

        "Branch,",
        "  CndW Loop Demand Bypass Branch,         !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  CndW Loop Demand Bypass Pipe,           !- Component Name 1",
        "  CndW Loop Demand Bypass Pipe Inlet Node, !- Component Inlet Node Name 1",
        "  CndW Loop Demand Bypass Pipe Outlet Node; !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  CndW Loop Demand Bypass Pipe,           !- Name",
        "  CndW Loop Demand Bypass Pipe Inlet Node, !- Inlet Node Name",
        "  CndW Loop Demand Bypass Pipe Outlet Node; !- Outlet Node Name",

        "Branch,",
        "  CndW Loop Demand Outlet Branch,         !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  Demand Outlet Pipe,                     !- Component Name 1",
        "  Demand Outlet Pipe Inlet Node,          !- Component Inlet Node Name 1",
        "  CndW Loop Demand Outlet Node;           !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  Demand Outlet Pipe,                     !- Name",
        "  Demand Outlet Pipe Inlet Node,          !- Inlet Node Name",
        "  CndW Loop Demand Outlet Node;           !- Outlet Node Name",

        "PlantEquipmentOperationSchemes,",
        "  CndW Loop Operation Schemes,            !- Name",
        "  PlantEquipmentOperation:CoolingLoad,    !- Control Scheme Object Type 1",
        "  CndW Loop Cooling Operation Scheme,     !- Control Scheme Name 1",
        "  Always On Discrete;                     !- Control Scheme Schedule Name 1",

        "PlantEquipmentOperation:CoolingLoad,",
        "  CndW Loop Cooling Operation Scheme,     !- Name",
        "  0,                                      !- Load Range Lower Limit 1 {W}",
        "  1000000000,                             !- Load Range Upper Limit 1 {W}",
        "  CndW Loop Cooling Equipment List;       !- Range Equipment List Name 1",

        "PlantEquipmentList,",
        "  CndW Loop Cooling Equipment List,       !- Name",
        "  CoolingTower:SingleSpeed,               !- Equipment Object Type 1",
        "  CT Single Speed;                        !- Equipment Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    state->dataGlobal->BeginSimFlag = true;

    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);

    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.
    // Get plant loop data
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    SizingManager::GetPlantSizingInput(*state);
    PlantManager::InitOneTimePlantSizingInfo(*state, 1);
    PlantManager::SizePlantLoop(*state, 1, true);

    // Fake having more than small load

    // SizingManager::ManageSizing();

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    // get inputs of cooling tower object
    CondenserLoopTowers::GetTowerInput(*state);

    state->dataCondenserLoopTowers->towers(1).initialize(*state);

    // Fake a flow
    state->dataCondenserLoopTowers->towers(1).DesignWaterFlowRate = 1000.0;

    // autosized other input fields of cooling tower. Tt throws, so we catch that so we can compare the error
    ASSERT_THROW(state->dataCondenserLoopTowers->towers(1).SizeTower(*state), std::runtime_error);

    std::string const error_string = delimited_string({

        "   ** Severe  ** Error when autosizing the UA value for cooling tower = CT SINGLE SPEED. Design Tower Exit Temperature must be greater than "
        "25.60 C when autosizing the tower UA.",
        "   **   ~~~   ** The User-specified Design Loop Exit Temperature=21.00",
        "   **   ~~~   ** is less than or equal to the design inlet air wet-bulb temperature of 25.60 C.",
        "   **   ~~~   ** Because you did not specify the Design Approach Temperature, and you do not have a Sizing:Plant object, it was defaulted "
        "to 21.00 C.",
        "   **   ~~~   ** If using HVACTemplate:Plant:ChilledWaterLoop, then check that input field Condenser Water Design Setpoint must be > 25.60 "
        "C if autosizing the cooling tower.",
        "   **  Fatal  ** Autosizing of cooling tower fails for tower = CT SINGLE SPEED.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Error when autosizing the UA value for cooling tower = CT SINGLE SPEED. Design Tower Exit Temperature must be "
        "greater than 25.60 C when autosizing the tower UA.",

    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, VSCoolingTowers_WaterOutletTempTest)
{
    std::string const idf_objects =
        delimited_string({"  Site:Location,",
                          "    USA IL-CHICAGO-OHARE,    !- Name",
                          "    41.77,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190;                     !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
                          "    1.00;                    !- Sky Clearness",

                          "    SimulationControl,",
                          "    no,                     !- Do Zone Sizing Calculation",
                          "    no,                     !- Do System Sizing Calculation",
                          "    no,                     !- Do Plant Sizing Calculation",
                          "    Yes,                    !- Run Simulation for Sizing Periods",
                          "    no;                     !- Run Simulation for Weather File Run Periods",

                          "  Timestep,6;",

                          "  ScheduleTypeLimits,",
                          "    Any Number;              !- Name",

                          "  CoolingTower:VariableSpeed,",
                          "    CoolingTower Variable Speed,  !- Name",
                          "    Tower Inlet Node,        !- Water Inlet Node Name",
                          "    Tower Outlet Node,       !- Water Outlet Node Name",
                          "    CoolToolsCrossFlow,      !- Model Type",
                          "    ,                        !- Model Coefficient Name",
                          "    23.25,                   !- Design Inlet Air Wet-Bulb Temperature {C}",
                          "    3.88888888888889,        !- Design Approach Temperature {deltaC}",
                          "    5.55555555555556,        !- Design Range Temperature {deltaC}",
                          "    0.02,                    !- Design Water Flow Rate {m3/s}",
                          "    10.0,                    !- Design Air Flow Rate {m3/s}",
                          "    1000.0,                  !- Design Fan Power {W}",
                          "    CoolingTower Variable Speed Fan Power Ratio Curve,  !- Fan Power Ratio Function of Air Flow Rate Ratio Curve Name",
                          "    0.2,                     !- Minimum Air Flow Rate Ratio",
                          "    0.125,                   !- Fraction of Tower Capacity in Free Convection Regime",
                          "    0,                       !- Basin Heater Capacity {W/K}",
                          "    2,                       !- Basin Heater Setpoint Temperature {C}",
                          "    ,                        !- Basin Heater Operating Schedule Name",
                          "    SaturatedExit,           !- Evaporation Loss Mode",
                          "    0.2,                     !- Evaporation Loss Factor {percent/K}",
                          "    0.008,                   !- Drift Loss Percent {percent}",
                          "    ConcentrationRatio,      !- Blowdown Calculation Mode",
                          "    3,                       !- Blowdown Concentration Ratio",
                          "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
                          "    ,                        !- Supply Water Storage Tank Name",
                          "    ,                        !- Outdoor Air Inlet Node Name",
                          "    ,                        !- Number of Cells",
                          "    ,                        !- Cell Control",
                          "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
                          "    ,                        !- Cell Maximum Water Flow Rate Fraction",
                          "    2.0,                     !- Sizing Factor",
                          "    General;                 !- End-Use Subcategory",

                          "  Curve:Cubic,",
                          "    CoolingTower Variable Speed Fan Power Ratio Curve,  !- Name",
                          "    -0.0093,                 !- Coefficient1 Constant",
                          "    0.0512,                  !- Coefficient2 x",
                          "    -0.0838,                 !- Coefficient3 x**2",
                          "    1.0419,                  !- Coefficient4 x**3",
                          "    0.15,                    !- Minimum Value of x",
                          "    1;                       !- Maximum Value of x",

                          "  Pump:VariableSpeed,",
                          "    CoolingTower Pump,         !- Name",
                          "    CoolingTower Supply Inlet Node,  !- Inlet Node Name",
                          "    CoolingTower Pump-CoolingTower CoolTowerNodeviaConnector,  !- Outlet Node Name",
                          "      0.03,                    !- Design Maximum Flow Rate {m3/s}",
                          "      134508,                  !- Design Pump Head {Pa}",
                          "      20000.0,                 !- Design Power Consumption {W}",
                          "      0.9,                     !- Motor Efficiency",
                          "      0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "      0,                       !- Coefficient 1 of the Part Load Performance Curve",
                          "      0.0216,                  !- Coefficient 2 of the Part Load Performance Curve",
                          "      -0.0325,                 !- Coefficient 3 of the Part Load Performance Curve",
                          "      1.0095,                  !- Coefficient 4 of the Part Load Performance Curve",
                          "      0,                       !- Design Minimum Flow Rate {m3/s}",
                          "      Intermittent,            !- Pump Control Type",
                          "      ,                        !- Pump Flow Rate Schedule Name",
                          "      ,                        !- Pump Curve Name",
                          "      ,                        !- Impeller Diameter {m}",
                          "      ,                        !- VFD Control Type",
                          "      ,                        !- Pump RPM Schedule Name",
                          "      ,                        !- Minimum Pressure Schedule",
                          "      ,                        !- Maximum Pressure Schedule",
                          "      ,                        !- Minimum RPM Schedule",
                          "      ,                        !- Maximum RPM Schedule",
                          "      ,                        !- Zone Name",
                          "      0.5,                     !- Skin Loss Radiative Fraction",
                          "      PowerPerFlowPerPressure, !- Design Power Sizing Method",
                          "      348701.1,                !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
                          "      1.282051282,             !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
                          "      0,                       !- Design Minimum Flow Rate Fraction",
                          "      General;                 !- End-Use Subcategory",

                          "  PlantEquipmentList,",
                          "    CoolingTower Equipment List,  !- Name",
                          "    CoolingTower:VariableSpeed,    !- Equipment 1 Object Type",
                          "    CoolingTower Variable Speed;  !- Equipment 1 Name",

                          "  PlantLoop,",
                          "    CoolingTower Loop,       !- Name",
                          "    Water,                   !- Fluid Type",
                          "    ,                        !- User Defined Fluid Type",
                          "    CoolingTower Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
                          "    CoolingTower Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
                          "    80.0,                    !- Maximum Loop Temperature {C}",
                          "    5.0,                     !- Minimum Loop Temperature {C}",
                          "    0.03,                    !- Maximum Loop Flow Rate {m3/s}",
                          "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
                          "    Autocalculate,                     !- Condenser Loop Volume {m3}",
                          "    CoolingTower Supply Inlet Node,  !- Condenser Side Inlet Node Name",
                          "    CoolingTower Supply Outlet Node,  !- Condenser Side Outlet Node Name",
                          "    CoolingTower Supply Branches,  !- Condenser Side Branch List Name",
                          "    CoolingTower Supply Connectors,  !- Condenser Side Connector List Name",
                          "    CoolingTower Demand Inlet Node,  !- Demand Side Inlet Node Name",
                          "    CoolingTower Demand Outlet Node,  !- Demand Side Outlet Node Name",
                          "    CoolingTower Demand Branches,  !- Condenser Demand Side Branch List Name",
                          "    CoolingTower Demand Connectors,  !- Condenser Demand Side Connector List Name",
                          "    Optimal,                 !- Load Distribution Scheme",
                          "    ,                        !- Availability Manager List Name",
                          "    SingleSetpoint;          !- Plant Loop Demand Calculation Scheme",

                          "  Sizing:Plant,",
                          "    CoolingTower Loop,       !- Plant or Condenser Loop Name",
                          "    Condenser,               !- Loop Type",
                          "    30,                      !- Design Loop Exit Temperature {C}",
                          "    5.55555555555556,        !- Loop Design Temperature Difference {deltaC}",
                          "    NonCoincident,           !- Sizing Option",
                          "    1,                       !- Zone Timesteps in Averaging Window",
                          "    None;                    !- Coincident Sizing Factor Mode",

                          "  PlantEquipmentOperationSchemes,",
                          "    CoolingTower Loop Operation Scheme List,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    CoolingTower Operation Scheme,  !- Control Scheme 1 Name",
                          "    CONSTANT-1.0,                      !- Control Scheme 1 Schedule Name",
                          "    PlantEquipmentOperation:ComponentSetpoint,  !- Control Scheme 3 Object Type",
                          "    CoolingTower Loop Setpoint Operation Scheme,  !- Control Scheme 3 Name",
                          "    CONSTANT-1.0;      !- Control Scheme 3 Schedule Name",

                          "  PlantEquipmentOperation:ComponentSetpoint,",
                          "    CoolingTower Loop Setpoint Operation Scheme,  !- Name",
                          "    CoolingTower:VariableSpeed,    !- Equipment 1 Object Type",
                          "    CoolingTower Variable Speed,   !- Equipment 1 Name",
                          "    CoolingTower Inlet Node,       !- Demand Calculation 1 Node Name",
                          "    CoolingTower Outlet Node,      !- Setpoint 1 Node Name",
                          "    0.03,                          !- Component 1 Flow Rate {m3/s}",
                          "    Cooling;                       !- Operation 1 Type",

                          "  PlantEquipmentOperation:CoolingLoad,",
                          "    CoolingTower Operation Scheme, !- Name",
                          "    0.0,                           !- Load Range 1 Lower Limit {W}",
                          "    1000000000000,                 !- Load Range 1 Upper Limit {W}",
                          "    CoolingTower Equipment List;   !- Range 1 Equipment List Name",

                          "  SetpointManager:Scheduled,",
                          "    CoolingTower Setpoint Manager,  !- Name",
                          "    Temperature,                 !- Control Variable",
                          "    CoolingTower Temp Sch,       !- Schedule Name",
                          "    CoolingTower Control Node List;   !- Setpoint Node or NodeList Name",

                          "  NodeList,",
                          "    CoolingTower Control Node List,   !- Name",
                          "    CoolingTower Outlet Node,         !- Node 1 Name",
                          "    CoolingTower Supply Outlet Node;  !- Node 2 Name",

                          "  Schedule:Compact,",
                          "    CoolingTower Temp Sch,   !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,30.0;       !- Field 3",

                          "  BranchList,",
                          "    CoolingTower Demand Branches,  !- Name",
                          "    CoolingTower Demand Inlet Branch,  !- Branch 1 Name",
                          "    CoolingTower Demand Load Branch 1,  !- Branch 2 Name",
                          "    CoolingTower Demand Bypass Branch,  !- Branch 4 Name",
                          "    CoolingTower Demand Outlet Branch;  !- Branch 5 Name",

                          "  BranchList,",
                          "    CoolingTower Supply Branches,  !- Name",
                          "    CoolingTower Supply Inlet Branch,  !- Branch 1 Name",
                          "    CoolingTower Supply Equipment Branch 1,  !- Branch 2 Name",
                          "    CoolingTower Supply Equipment Bypass Branch,  !- Branch 4 Name",
                          "    CoolingTower Supply Outlet Branch;  !- Branch 5 Name",

                          "  Branch,",
                          "    CoolingTower Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    CoolingTower Demand Bypass Pipe,  !- Component 1 Name",
                          "    CoolingTower Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
                          "    CoolingTower Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    CoolingTower Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    CoolingTower Demand Inlet Pipe,  !- Component 1 Name",
                          "    CoolingTower Demand Inlet Node,  !- Component 1 Inlet Node Name",
                          "    CoolingTower Demand Inlet Pipe-CoolingTower Demand Mixer;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    CoolingTower Demand Load Branch 1,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    LoadProfile:Plant,  !- Component 1 Object Type",
                          "    Load Profile 1,      !- Component 1 Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

                          "  LoadProfile:Plant,",
                          "    Load Profile 1,          !- Name",
                          "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
                          "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
                          "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
                          "    0.010,                    !- Peak Flow Rate {m3/s}",
                          "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Load Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,0.0;        !- Field 3",

                          "  Schedule:Compact,",
                          "    Load Profile 1 Flow Frac Schedule,  !- Name",
                          "    Any Number,              !- Schedule Type Limits Name",
                          "    THROUGH: 12/31,          !- Field 1",
                          "    FOR: AllDays,            !- Field 2",
                          "    UNTIL: 24:00,1.0;        !- Field 3",

                          "  Branch,",
                          "    CoolingTower Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    CoolingTower Demand Outlet Pipe,  !- Component 1 Name",
                          "    CoolingTower Demand Mixer-CoolingTower Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    CoolingTower Demand Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    CoolingTower Supply Equipment Branch 1,  !- Name",
                          "    ,                            !- Pressure Drop Curve Name",
                          "    CoolingTower:VariableSpeed,  !- Component 1 Object Type",
                          "    CoolingTower Variable Speed, !- Component 1 Name",
                          "    CoolingTower Inlet Node,     !- Component 1 Inlet Node Name",
                          "    CoolingTower Outlet Node;    !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    CoolingTower Supply Equipment Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    CoolingTower Supply Equipment Bypass Pipe,  !- Component 1 Name",
                          "    CoolingTower Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
                          "    CoolingTower Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    CoolingTower Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:VariableSpeed,      !- Component 1 Object Type",
                          "    CoolingTower Pump,       !- Component 1 Name",
                          "    CoolingTower Supply Inlet Node,  !- Component 1 Inlet Node Name",
                          "    CoolingTower Pump-CoolingTower CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

                          "  Branch,",
                          "    CoolingTower Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    CoolingTower Supply Outlet Pipe,  !- Component 1 Name",
                          "    CoolingTower Supply Mixer-CoolingTower Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
                          "    CoolingTower Supply Outlet Node;  !- Component 1 Outlet Node Name",

                          "  OutdoorAir:Node,",
                          "    CoolingTower CoolTower OA ref Node;  !- Name",

                          "  ConnectorList,",
                          "    CoolingTower Demand Connectors,  !- Name",
                          "    Connector:Splitter,          !- Connector 1 Object Type",
                          "    CoolingTower Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,             !- Connector 2 Object Type",
                          "    CoolingTower Demand Mixer;  !- Connector 2 Name",

                          "  ConnectorList,",
                          "    CoolingTower Supply Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    CoolingTower Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    CoolingTower Supply Mixer;  !- Connector 2 Name",

                          "  Connector:Splitter,",
                          "    CoolingTower Demand Splitter,  !- Name",
                          "    CoolingTower Demand Inlet Branch,  !- Inlet Branch Name",
                          "    CoolingTower Demand Load Branch 1,  !- Outlet Branch 1 Name",
                          "    CoolingTower Demand Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Splitter,",
                          "    CoolingTower Supply Splitter,  !- Name",
                          "    CoolingTower Supply Inlet Branch,  !- Inlet Branch Name",
                          "    CoolingTower Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
                          "    CoolingTower Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    CoolingTower Demand Mixer,  !- Name",
                          "    CoolingTower Demand Outlet Branch,  !- Outlet Branch Name",
                          "    CoolingTower Demand Load Branch 1,  !- Inlet Branch 1 Name",
                          "    CoolingTower Demand Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Connector:Mixer,",
                          "    CoolingTower Supply Mixer,  !- Name",
                          "    CoolingTower Supply Outlet Branch,  !- Outlet Branch Name",
                          "    CoolingTower Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
                          "    CoolingTower Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

                          "  Pipe:Adiabatic,",
                          "    CoolingTower Demand Bypass Pipe,  !- Name",
                          "    CoolingTower Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
                          "    CoolingTower Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    CoolingTower Demand Inlet Pipe,  !- Name",
                          "    CoolingTower Demand Inlet Node,  !- Inlet Node Name",
                          "    CoolingTower Demand Inlet Pipe-CoolingTower Demand Mixer;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    CoolingTower Demand Outlet Pipe,  !- Name",
                          "    CoolingTower Demand Mixer-CoolingTower Demand Outlet Pipe,  !- Inlet Node Name",
                          "    CoolingTower Demand Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    CoolingTower Supply Equipment Bypass Pipe,  !- Name",
                          "    CoolingTower Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
                          "    CoolingTower Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

                          "  Pipe:Adiabatic,",
                          "    CoolingTower Supply Outlet Pipe,  !- Name",
                          "    CoolingTower Supply Mixer-CoolingTower Supply Outlet Pipe,  !- Inlet Node Name",
                          "    CoolingTower Supply Outlet Node;  !- Outlet Node Name"

        });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    state->dataGlobal->BeginSimFlag = true;

    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone);
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state);

    // Get plant loop data
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    SizingManager::GetPlantSizingInput(*state);
    PlantManager::InitOneTimePlantSizingInfo(*state, 1);
    PlantManager::SizePlantLoop(*state, 1, true);

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    CondenserLoopTowers::GetTowerInput(*state);
    auto &VSTower = state->dataCondenserLoopTowers->towers(1);

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataGlobal->BeginEnvrnFlag = true;

    // test case 1:
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutWetBulbTemp = 26.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat =
        Psychrometrics::PsyWFnTdbTwbPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutWetBulbTemp, state->dataEnvrn->OutBaroPress);
    state->dataLoopNodes->Node(VSTower.WaterInletNodeNum).Temp = 35.0;

    VSTower.initialize(*state);
    state->dataGlobal->BeginEnvrnFlag = false;
    VSTower.SizeTower(*state);
    VSTower.initialize(*state);

    VSTower.InletWaterTemp = 35.0;
    Real64 WaterFlowRateRatio = 0.75;
    Real64 AirWetBulbTemp = state->dataEnvrn->OutWetBulbTemp;

    state->dataPlnt->PlantLoop(VSTower.plantLoc.loopNum).LoopSide(VSTower.plantLoc.loopSideNum).FlowLock = DataPlant::FlowLock::Locked;
    state->dataPlnt->PlantLoop(VSTower.plantLoc.loopNum).LoopSide(VSTower.plantLoc.loopSideNum).TempSetPoint = 30.0;
    VSTower.WaterMassFlowRate = VSTower.DesWaterMassFlowRate * WaterFlowRateRatio;

    Real64 myLoad = -1000.0;
    bool RunFlag = true;
    VSTower.calculateVariableSpeedTower(*state, myLoad, RunFlag);
    EXPECT_DOUBLE_EQ(30.0, VSTower.OutletWaterTemp);
    EXPECT_DOUBLE_EQ(1.0, VSTower.FanCyclingRatio);
    Real64 TowerOutletWaterTemp = VSTower.calculateVariableTowerOutletTemp(*state, WaterFlowRateRatio, VSTower.airFlowRateRatio, AirWetBulbTemp);
    EXPECT_NEAR(30.0, TowerOutletWaterTemp, 0.0001);

    // test case 2:
    state->dataEnvrn->OutDryBulbTemp = 15.0;
    state->dataEnvrn->OutWetBulbTemp = 10.0;
    state->dataEnvrn->OutHumRat =
        Psychrometrics::PsyWFnTdbTwbPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutWetBulbTemp, state->dataEnvrn->OutBaroPress);
    AirWetBulbTemp = state->dataEnvrn->OutWetBulbTemp;
    VSTower.WaterMassFlowRate = VSTower.DesWaterMassFlowRate * WaterFlowRateRatio;

    VSTower.AirTemp = state->dataEnvrn->OutDryBulbTemp;
    VSTower.AirWetBulb = state->dataEnvrn->OutWetBulbTemp;
    VSTower.AirHumRat = state->dataEnvrn->OutHumRat;

    VSTower.calculateVariableSpeedTower(*state, myLoad, RunFlag);
    EXPECT_DOUBLE_EQ(30.0, VSTower.OutletWaterTemp);
    EXPECT_NEAR(0.5424, VSTower.FanCyclingRatio, 0.0001);
    // outside air condition is favorable that fan only needs to cycle at min flow to meet the setpoint
    TowerOutletWaterTemp = VSTower.calculateVariableTowerOutletTemp(*state, WaterFlowRateRatio, VSTower.MinimumVSAirFlowFrac, AirWetBulbTemp);
    // this is tower outlet temperature at continuous minimum fan flow
    EXPECT_NEAR(26.9537, TowerOutletWaterTemp, 0.0001);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_CoolingTowersDefaultValuesTest)
{
    std::string const idf_objects = delimited_string({

        "  CoolingTower:SingleSpeed,",
        "    SingleSpeed CoolingTower,!- Name",
        "    CoolTower InletNode,     !- Water Inlet Node Name",
        "    CoolTower Outlet Node,   !- Water Outlet Node Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Fan Power {W}",
        "    autosize,                !- Design U-Factor Times Area Value {W/K}",
        "    0.0,                     !- Free Convection Air Flow Rate {m3/s}",
        "    0.1,                     !- Free Convection Air Flow Rate Sizing Factor",
        "    0.0,                     !- Free Convection U-Factor Times Area Value {W/K}",
        "    0.1,                     !- Free Convection U-Factor Times Area Value Sizing Factor",
        "    ,                        !- Performance Input Method",
        "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "    1E+5,                    !- Nominal Capacity {W}",
        "    1E+3,                    !- Free Convection Capacity {W}",
        "    ,                        !- Free Convection Nominal Capacity Sizing Factor",
        "    35.0,                    !- Design Inlet Air Dry-Bulb Temperature",
        "    25.6,                    !- Design Inlet Air Wet-Bulb Temperature",
        "    autosize,                !- Design Approach Temperature",
        "    autosize,                !- Design Range Temperature",
        "    0.0,                     !- Basin Heater Capacity {W/K}",
        "    2.0,                     !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    ,                        !- Evaporation Loss Mode",
        "    ,                        !- Evaporation Loss Factor {percent/K}",
        "    ,                        !- Drift Loss Percent {percent}",
        "    ,                        !- Blowdown Calculation Mode",
        "    ,                        !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    ,                        !- Capacity Control",
        "    ,                        !- Number of Cells",
        "    ,                        !- Cell Control",
        "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
        "    ,                        !- Cell Maximum Water Flow Rate Fraction",
        "    ,                        !- Sizing Factor",
        "    ;                        !- End-Use Subcategory",

        "  CoolingTower:TwoSpeed,",
        "    TwoSpeed CoolingTower,   !- Name",
        "    CoolTower InletNode,     !- Water Inlet Node Name",
        "    CoolTower Outlet Node,   !- Water Outlet Node Name",
        "    ,                        !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- High Fan Speed Air Flow Rate {m3/s}",
        "    autosize,                !- High Fan Speed Fan Power {W}",
        "    ,                        !- High Fan Speed U-Factor Times Area Value {W/K}",
        "    autocalculate,           !- Low Fan Speed Air Flow Rate {m3/s}",
        "    ,                        !- Low Fan Speed Air Flow Rate Sizing Factor",
        "    autosize,                !- Low Fan Speed Fan Power {W}",
        "    ,                        !- Low Fan Speed Fan Power Sizing Factor",
        "    autosize,                !- Low Fan Speed U-Factor Times Area Value {W/K}",
        "    ,                        !- Low Fan Speed U-Factor Times Area Sizing Factor",
        "    0.9,                     !- Free Convection Regime Air Flow Rate {m3/s}",
        "    ,                        !- Free Convection Regime Air Flow Rate Sizing Factor",
        "    autocalculate,           !- Free Convection Regime U-Factor Times Area Value {W/K}",
        "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
        "    NominalCapacity,         !- Performance Input Method",
        "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "    1E+5,                    !- High Speed Nominal Capacity {W}",
        "    1E+4,                    !- Low Speed Nominal Capacity {W}",
        "    ,                        !- Low Speed Nominal Capacity Sizing Factor",
        "    5E+3,                    !- Free Convection Nominal Capacity {W}",
        "    ,                        !- Free Convection Nominal Capacity Sizing Factor",
        "    35.0,					  !- Design Inlet Air Dry-Bulb Temperature",
        "    25.6,					  !- Design Inlet Air Wet-Bulb Temperature",
        "    autosize,                !- Design Approach Temperature",
        "    autosize,                !- Design Range Temperature",
        "    0.0,                     !- Basin Heater Capacity {W/K}",
        "    2.0,                     !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    ,                        !- Evaporation Loss Mode",
        "    ,                        !- Evaporation Loss Factor {percent/K}",
        "    ,                        !- Drift Loss Percent {percent}",
        "    ,                        !- Blowdown Calculation Mode",
        "    ,                        !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    ,                        !- Number of Cells",
        "    ,                        !- Cell Control",
        "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
        "    ,                        !- Cell Maximum Water Flow Rate Fraction",
        "    ,                        !- Sizing Factor",
        "    ;                        !- End-Use Subcategory",

        "  CoolingTower:VariableSpeed:Merkel,",
        "    VSMerkel CoolingTower,   !- Name",
        "    CoolTower InletNode,     !- Water Inlet Node Name",
        "    CoolTower Outlet Node,   !- Water Outlet Node Name",
        "    ,                        !- Performance Input Method",
        "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "    1E+5,                    !- Nominal Capacity {W}",
        "    autocalculate,           !- Free Convection Nominal Capacity {W}",
        "    0.1,                     !- Free Convection Nominal Capacity Sizing Factor",
        "    autocalculate,           !- Design Water Flow Rate {m3/s}",
        "    5.382E-8,                !- Design Water Flow Rate per Unit of Nominal Capacity {m3/s-W}",
        "    autocalculate,           !- Design Air Flow Rate {m3/s}",
        "    2.76316E-5,              !- Design Air Flow Rate Per Unit of Nominal Capacity {m3/s-W}",
        "    0.2,                     !- Minimum Air Flow Rate Ratio",
        "    autocalculate,           !- Design Fan Power {W}",
        "    0.0105,                  !- Design Fan Power Per Unit of Nominal Capacity {dimensionless}",
        "    VSTowerFanPowerFFF,      !- Fan Power Modifier Function of Air Flow Rate Ratio Curve Name",
        "    autocalculate,           !- Free Convection Regime Air Flow Rate {m3/s}",
        "    0.1,                     !- Free Convection Regime Air Flow Rate Sizing Factor",
        "    ,                        !- Design Air Flow Rate U-Factor Times Area Value {W/K}",
        "    ,                        !- Free Convection Regime U-Factor Times Area Value {W/K}",
        "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
        "    VSTowerUAModFFF,         !- U-Factor Times Area Modifier Function of Air Flow Ratio Curve Name",
        "    VSTowerUAModFWBT,        !- U-Factor Times Area Modifier Function of Wetbulb Temperature Difference Curve Name",
        "    VSTowerUAModFWFF,        !- U-Factor Times Area Modifier Function of Water Flow Ratio Curve Name",
        "    35.0,					  !- Design Inlet Air Dry-Bulb Temperature",
        "    25.6,					  !- Design Inlet Air Wet-Bulb Temperature",
        "    autosize,                !- Design Approach Temperature",
        "    autosize,                !- Design Range Temperature",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    ,                        !- Evaporation Loss Mode",
        "    ,                        !- Evaporation Loss Factor {percent/K}",
        "    ,                        !- Drift Loss Percent {percent}",
        "    ,                        !- Blowdown Calculation Mode",
        "    ,                        !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    ,                        !- Number of Cells",
        "    ,                        !- Cell Control",
        "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
        "    ,                        !- Cell Maximum Water Flow Rate Fraction",
        "    ,                        !- Sizing Factor",
        "    ;                        !- End-Use Subcategory",

        "  Curve:Cubic, VSTowerFanPowerFFF, 0.02, 0.0, 0.0, 0.98, 0.2, 1.0, 0.0, 1.0, Dimensionless, Dimensionless;",
        "  Curve:Quadratic, VSTowerUAModFFF, 0.0, 1.3, -0.3, 0.2, 1.0, 0.0, 1.0, Dimensionless, Dimensionless;",
        "  Curve:Linear, VSTowerUAModFWBT, 1.0, 0.0081, -10, 25.0, 0.85, 1.3, Dimensionless, Dimensionless;",
        "  Curve:Quadratic, VSTowerUAModFWFF, 0.1082, 1.667, -0.7713, 0.3, 1.0, 0.5, 1.1, Dimensionless, Dimensionless;",

        "  CoolingTower:VariableSpeed,",
        "    VarSpeed CoolingTower,   !- Name",
        "    CoolTower InletNode,     !- Water Inlet Node Name",
        "    CoolTower Outlet Node,   !- Water Outlet Node Name",
        "    CoolToolsCrossFlow,      !- Model Type",
        "    ,                        !- Model Coefficient Name",
        "    23.25,                   !- Design Inlet Air Wet-Bulb Temperature {C}",
        "    ,                        !- Design Approach Temperature {deltaC}",
        "    ,                        !- Design Range Temperature {deltaC}",
        "    0.02,                    !- Design Water Flow Rate {m3/s}",
        "    10.0,                    !- Design Air Flow Rate {m3/s}",
        "    1000.0,                  !- Design Fan Power {W}",
        "    VSTowerFanPowerFFF,      !- Fan Power Ratio Function of Air Flow Rate Ratio Curve Name",
        "    ,                        !- Minimum Air Flow Rate Ratio",
        "    ,                        !- Fraction of Tower Capacity in Free Convection Regime",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    ,                        !- Evaporation Loss Mode",
        "    ,                        !- Evaporation Loss Factor {percent/K}",
        "    ,                        !- Drift Loss Percent {percent}",
        "    ,                        !- Blowdown Calculation Mode",
        "    ,                        !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    ,                        !- Number of Cells",
        "    ,                        !- Cell Control",
        "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
        "    ,                        !- Cell Maximum Water Flow Rate Fraction",
        "    ,                        !- Sizing Factor",
        "    ;                        !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // cooling towers get input
    CondenserLoopTowers::GetTowerInput(*state);

    int index; // index to cooling tower
    index = Util::FindItemInList("SINGLESPEED COOLINGTOWER", state->dataCondenserLoopTowers->towers);
    auto &singleSpd_tower = state->dataCondenserLoopTowers->towers(index);
    // some inputs and default values checks
    EXPECT_EQ(singleSpd_tower.Name, "SINGLESPEED COOLINGTOWER");
    EXPECT_EQ(singleSpd_tower.TowerType, DataPlant::PlantEquipmentType::CoolingTower_SingleSpd);
    EXPECT_EQ(singleSpd_tower.DesignWaterFlowRate, DataSizing::AutoSize);
    EXPECT_EQ(singleSpd_tower.HighSpeedAirFlowRate, DataSizing::AutoSize);
    EXPECT_EQ(singleSpd_tower.HighSpeedFanPower, DataSizing::AutoSize);
    EXPECT_EQ(singleSpd_tower.HighSpeedTowerUA, DataSizing::AutoSize);
    EXPECT_EQ(singleSpd_tower.FreeConvAirFlowRate, 0.0);
    EXPECT_EQ(singleSpd_tower.FreeConvAirFlowRateSizingFactor, 0.1);
    EXPECT_EQ(singleSpd_tower.FreeConvTowerUA, 0.0);
    EXPECT_EQ(singleSpd_tower.FreeConvTowerUASizingFactor, 0.1);
    EXPECT_EQ(singleSpd_tower.PerformanceInputMethod_Num, CondenserLoopTowers::PIM::UFactor);
    EXPECT_EQ(singleSpd_tower.HeatRejectCapNomCapSizingRatio, 1.25);
    EXPECT_EQ(singleSpd_tower.EvapLossMode, CondenserLoopTowers::EvapLoss::MoistTheory);
    EXPECT_EQ(singleSpd_tower.BlowdownMode, CondenserLoopTowers::Blowdown::Concentration);
    EXPECT_EQ(singleSpd_tower.DriftLossFraction, 0.008 / 100);
    EXPECT_EQ(singleSpd_tower.ConcentrationRatio, 3.0);
    EXPECT_EQ(singleSpd_tower.CapacityControl, CondenserLoopTowers::CapacityCtrl::FanCycling);
    EXPECT_EQ(singleSpd_tower.NumCell, 1);
    EXPECT_EQ(singleSpd_tower.MinFracFlowRate, 0.33);
    EXPECT_EQ(singleSpd_tower.MaxFracFlowRate, 2.50);
    EXPECT_EQ(singleSpd_tower.cellCtrl, CondenserLoopTowers::CellCtrl::MaxCell);
    EXPECT_EQ(singleSpd_tower.EndUseSubcategory, "General");

    index = Util::FindItemInList("TWOSPEED COOLINGTOWER", state->dataCondenserLoopTowers->towers);
    auto &twoSpd_tower = state->dataCondenserLoopTowers->towers(index);
    // some inputs and default values checks
    EXPECT_EQ(twoSpd_tower.Name, "TWOSPEED COOLINGTOWER");
    EXPECT_EQ(twoSpd_tower.TowerType, DataPlant::PlantEquipmentType::CoolingTower_TwoSpd);
    EXPECT_EQ(twoSpd_tower.EvapLossMode, CondenserLoopTowers::EvapLoss::MoistTheory);
    EXPECT_EQ(twoSpd_tower.BlowdownMode, CondenserLoopTowers::Blowdown::Concentration);
    EXPECT_EQ(twoSpd_tower.DriftLossFraction, 0.008 / 100);
    EXPECT_EQ(twoSpd_tower.ConcentrationRatio, 3.0);
    EXPECT_EQ(twoSpd_tower.NumCell, 1);
    EXPECT_EQ(twoSpd_tower.MinFracFlowRate, 0.33);
    EXPECT_EQ(twoSpd_tower.MaxFracFlowRate, 2.50);
    EXPECT_EQ(twoSpd_tower.cellCtrl, CondenserLoopTowers::CellCtrl::MaxCell);
    EXPECT_EQ(twoSpd_tower.EndUseSubcategory, "General");

    index = Util::FindItemInList("VARSPEED COOLINGTOWER", state->dataCondenserLoopTowers->towers);
    auto &varSpd_tower = state->dataCondenserLoopTowers->towers(index);
    // some inputs and default values checks
    EXPECT_EQ(varSpd_tower.Name, "VARSPEED COOLINGTOWER");
    EXPECT_EQ(varSpd_tower.TowerType, DataPlant::PlantEquipmentType::CoolingTower_VarSpd);
    EXPECT_EQ(varSpd_tower.EvapLossMode, CondenserLoopTowers::EvapLoss::MoistTheory);
    EXPECT_EQ(varSpd_tower.BlowdownMode, CondenserLoopTowers::Blowdown::Concentration);
    EXPECT_EQ(varSpd_tower.DriftLossFraction, 0.008 / 100);
    EXPECT_EQ(varSpd_tower.ConcentrationRatio, 3.0);
    EXPECT_EQ(varSpd_tower.NumCell, 1);
    EXPECT_EQ(varSpd_tower.MinFracFlowRate, 0.33);
    EXPECT_EQ(varSpd_tower.MaxFracFlowRate, 2.50);
    EXPECT_EQ(varSpd_tower.cellCtrl, CondenserLoopTowers::CellCtrl::MaxCell);
    EXPECT_EQ(varSpd_tower.EndUseSubcategory, "General");
    EXPECT_EQ(varSpd_tower.PerformanceInputMethod_Num, CondenserLoopTowers::PIM::UFactor); // hard coded
    EXPECT_EQ(varSpd_tower.HeatRejectCapNomCapSizingRatio, 1.25);

    index = Util::FindItemInList("VSMERKEL COOLINGTOWER", state->dataCondenserLoopTowers->towers);
    auto &vSpdMerkel_tower = state->dataCondenserLoopTowers->towers(index);
    // some inputs and default values checks
    EXPECT_EQ(vSpdMerkel_tower.Name, "VSMERKEL COOLINGTOWER");
    EXPECT_EQ(vSpdMerkel_tower.TowerType, DataPlant::PlantEquipmentType::CoolingTower_VarSpdMerkel);
    EXPECT_EQ(vSpdMerkel_tower.EvapLossMode, CondenserLoopTowers::EvapLoss::MoistTheory);
    EXPECT_EQ(vSpdMerkel_tower.BlowdownMode, CondenserLoopTowers::Blowdown::Concentration);
    EXPECT_EQ(vSpdMerkel_tower.DriftLossFraction, 0.008 / 100);
    EXPECT_EQ(vSpdMerkel_tower.ConcentrationRatio, 3.0);
    EXPECT_EQ(vSpdMerkel_tower.NumCell, 1);
    EXPECT_EQ(vSpdMerkel_tower.MinFracFlowRate, 0.33);
    EXPECT_EQ(vSpdMerkel_tower.MaxFracFlowRate, 2.50);
    EXPECT_EQ(vSpdMerkel_tower.cellCtrl, CondenserLoopTowers::CellCtrl::MaxCell);
    EXPECT_EQ(vSpdMerkel_tower.EndUseSubcategory, "General");
    EXPECT_EQ(vSpdMerkel_tower.PerformanceInputMethod_Num, CondenserLoopTowers::PIM::NominalCapacity);
    EXPECT_EQ(vSpdMerkel_tower.HeatRejectCapNomCapSizingRatio, 1.25);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_CalculateVariableTowerOutletTemp)
{
    std::string const idf_objects = delimited_string({
        "  CoolingTower:VariableSpeed,",
        "    CoolingTower-plant_loops-170,  !- Name",
        "    Node 40,                 !- Water Inlet Node Name",
        "    Node 41,                 !- Water Outlet Node Name",
        "    YorkCalc,                !- Model Type",
        "    ,                        !- Model Coefficient Name",
        "    25,                      !- Design Inlet Air Wet-Bulb Temperature {C}",
        "    4.44,                    !- Design Approach Temperature {deltaC}",
        "    5.56,                    !- Design Range Temperature {deltaC}",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    Autosize,                !- Design Air Flow Rate {m3/s}",
        "    3544.15691546004,        !- Design Fan Power {W}",
        "    Variable Speed Cooling Tower Fan Power Ratio Function of Air Flow Rate,  !- Fan Power Ratio Function of Air Flow Rate Ratio Curve Name",
        "    0.25,                    !- Minimum Air Flow Rate Ratio",
        "    0.125,                   !- Fraction of Tower Capacity in Free Convection Regime",
        "    0,                       !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    LossFactor,              !- Evaporation Loss Mode",
        "    0.2,                     !- Evaporation Loss Factor {percent/K}",
        "    0.008,                   !- Drift Loss Percent {percent}",
        "    ConcentrationRatio,      !- Blowdown Calculation Mode",
        "    3,                       !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    2,                       !- Number of Cells",
        "    MaximalCell,             !- Cell Control",
        "    0.25,                    !- Cell Minimum  Water Flow Rate Fraction",
        "    2.5,                     !- Cell Maximum Water Flow Rate Fraction",
        "    1,                       !- Sizing Factor",
        "    General;                 !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // cooling tower get input
    CondenserLoopTowers::GetTowerInput(*state);
    int index;
    index = Util::FindItemInList("COOLINGTOWER-PLANT_LOOPS-170", state->dataCondenserLoopTowers->towers);
    auto &tower = state->dataCondenserLoopTowers->towers(index);

    // set node temperature
    state->dataLoopNodes->Node(tower.WaterInletNodeNum).Temp = 40;
    tower.WaterTemp = 40;

    // calculate outlet temperature
    Real64 tOutlet = tower.calculateVariableTowerOutletTemp(*state, 0.25, 1.0, 14.1);
    EXPECT_NEAR(tower.WaterTemp - 22.2222, tOutlet, 0.01);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_checkMassFlowAndLoadTest)
{
    bool flagToReturn;
    bool runFlag;
    Real64 myLoad;
    Real64 constexpr allowedTolerance = 0.0001;
    Real64 expectedPower;
    Real64 expectedTemp;

    state->dataCondenserLoopTowers->towers.allocate(1);
    auto &tower = state->dataCondenserLoopTowers->towers(1);
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataLoopNodes->Node.allocate(2);
    tower.WaterInletNodeNum = 1;
    tower.WaterOutletNodeNum = 2;

    tower.plantLoc.loopNum = 1;
    tower.plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    tower.plantLoc.branchNum = 1;
    tower.plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, tower.plantLoc);

    // Test 1: Mass flow rate is low but myLoad is ok--flag should be set to true
    flagToReturn = false;
    runFlag = true;
    myLoad = -1000.0;
    state->dataEnvrn->OutDryBulbTemp = 27.0;
    tower.WaterMassFlowRate = 0.0;
    tower.BasinHeaterPowerFTempDiff = 1.0;
    tower.basinHeaterSched = nullptr;
    tower.BasinHeaterSetPointTemp = 26.0;
    tower.BasinHeaterPower = 1.0;
    expectedPower = 0.0;
    tower.checkMassFlowAndLoad(*state, myLoad, runFlag, flagToReturn);
    EXPECT_TRUE(flagToReturn);
    EXPECT_NEAR(expectedPower, tower.BasinHeaterPower, allowedTolerance);

    // Test 2: Mass flow rate is ok but myLoad is zero--flag should be set to true
    flagToReturn = false;
    runFlag = false;
    myLoad = 0.0;
    state->dataEnvrn->OutDryBulbTemp = 27.0;
    tower.WaterMassFlowRate = 0.5;
    tower.BasinHeaterPowerFTempDiff = 1.0;
    tower.basinHeaterSched = nullptr;
    tower.BasinHeaterSetPointTemp = 25.0;
    tower.BasinHeaterPower = 2.0;
    tower.FanPower = 1.0;
    tower.airFlowRateRatio = 1.0;
    tower.Qactual = 1.0;
    expectedPower = 0.0;
    expectedTemp = 23.0;
    state->dataLoopNodes->Node(tower.WaterInletNodeNum).Temp = 23.0;
    tower.checkMassFlowAndLoad(*state, myLoad, runFlag, flagToReturn);
    EXPECT_TRUE(flagToReturn);
    EXPECT_NEAR(expectedPower, tower.BasinHeaterPower, allowedTolerance);
    EXPECT_NEAR(expectedTemp, tower.OutletWaterTemp, allowedTolerance);
    EXPECT_NEAR(0.0, tower.FanPower, allowedTolerance);
    EXPECT_NEAR(0.0, tower.airFlowRateRatio, allowedTolerance);
    EXPECT_NEAR(0.0, tower.Qactual, allowedTolerance);

    // Test 3: Mass flow rate and myLoad are both ok--nothing changes, power does not get calculated here
    flagToReturn = false;
    runFlag = true;
    myLoad = -1000.0;
    state->dataEnvrn->OutDryBulbTemp = 27.0;
    tower.WaterMassFlowRate = 0.5;
    tower.BasinHeaterPowerFTempDiff = 1.0;
    tower.basinHeaterSched = nullptr;
    tower.BasinHeaterSetPointTemp = 25.0;
    tower.BasinHeaterPower = 3.0;
    expectedPower = 3.0;
    tower.checkMassFlowAndLoad(*state, myLoad, runFlag, flagToReturn);
    EXPECT_FALSE(flagToReturn);
    EXPECT_NEAR(expectedPower, tower.BasinHeaterPower, allowedTolerance);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_checkTowerIndices)
{
    std::string const idf_objects = delimited_string({
        "  CoolingTower:VariableSpeed,",
        "    CoolingTower-plant_loops-170,  !- Name",
        "    Node 40,                 !- Water Inlet Node Name",
        "    Node 41,                 !- Water Outlet Node Name",
        "    YorkCalc,                !- Model Type",
        "    ,                        !- Model Coefficient Name",
        "    25,                      !- Design Inlet Air Wet-Bulb Temperature {C}",
        "    4.44,                    !- Design Approach Temperature {deltaC}",
        "    5.56,                    !- Design Range Temperature {deltaC}",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    Autosize,                !- Design Air Flow Rate {m3/s}",
        "    3544.15691546004,        !- Design Fan Power {W}",
        "    Variable Speed Cooling Tower Fan Power Ratio Function of Air Flow Rate,  !- Fan Power Ratio Function of Air Flow Rate Ratio Curve Name",
        "    0.25,                    !- Minimum Air Flow Rate Ratio",
        "    0.125,                   !- Fraction of Tower Capacity in Free Convection Regime",
        "    0,                       !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    LossFactor,              !- Evaporation Loss Mode",
        "    0.2,                     !- Evaporation Loss Factor {percent/K}",
        "    0.008,                   !- Drift Loss Percent {percent}",
        "    ConcentrationRatio,      !- Blowdown Calculation Mode",
        "    3,                       !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    2,                       !- Number of Cells",
        "    MaximalCell,             !- Cell Control",
        "    0.25,                    !- Cell Minimum  Water Flow Rate Fraction",
        "    2.5,                     !- Cell Maximum Water Flow Rate Fraction",
        "    1,                       !- Sizing Factor",
        "    General;                 !- End-Use Subcategory",
        "  CoolingTower:TwoSpeed,",
        "    TowerWaterSys CoolTower1,!- Name",
        "    TowerWaterSys Pump-TowerWaterSys CoolTowerNode,  !- Water Inlet Node Name",
        "    TowerWaterSys Supply Equipment Outlet Node,  !- Water Outlet Node Name",
        "    ,                        !- Design Water Flow Rate {m3/s}",
        "    AUTOSIZE,                !- High Fan Speed Air Flow Rate {m3/s}",
        "    AUTOSIZE,                !- High Fan Speed Fan Power {W}",
        "    ,                        !- High Fan Speed U-Factor Times Area Value {W/K}",
        "    autocalculate,           !- Low Fan Speed Air Flow Rate {m3/s}",
        "    ,                        !- Low Fan Speed Air Flow Rate Sizing Factor",
        "    AUTOSIZE,                !- Low Fan Speed Fan Power {W}",
        "    ,                        !- Low Fan Speed Fan Power Sizing Factor",
        "    AUTOSIZE,                !- Low Fan Speed U-Factor Times Area Value {W/K}",
        "    ,                        !- Low Fan Speed U-Factor Times Area Sizing Factor",
        "    0.9,                     !- Free Convection Regime Air Flow Rate {m3/s}",
        "    ,                        !- Free Convection Regime Air Flow Rate Sizing Factor",
        "    autocalculate,           !- Free Convection Regime U-Factor Times Area Value {W/K}",
        "    ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
        "    NominalCapacity,         !- Performance Input Method",
        "    ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "    1E+5,                    !- High Speed Nominal Capacity {W}",
        "    1E+4,                    !- Low Speed Nominal Capacity {W}",
        "    ,                        !- Low Speed Nominal Capacity Sizing Factor",
        "    5E+3,                    !- Free Convection Nominal Capacity {W}",
        "    ,                        !- Free Convection Nominal Capacity Sizing Factor",
        "    35.0,					  !- Design Inlet Air Dry-Bulb Temperature",
        "    25.6,					  !- Design Inlet Air Wet-Bulb Temperature",
        "    3.9,                     !- Design Approach Temperature",
        "    5.5,                     !- Design Range Temperature",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    ,                        !- Evaporation Loss Mode",
        "    ,                        !- Evaporation Loss Factor {percent/K}",
        "    ,                        !- Drift Loss Percent {percent}",
        "    ,                        !- Blowdown Calculation Mode",
        "    ,                        !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ;                        !- Outdoor Air Inlet Node Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    // cooling tower get input
    CondenserLoopTowers::GetTowerInput(*state);
    int index;
    index = Util::FindItemInList("COOLINGTOWER-PLANT_LOOPS-170", state->dataCondenserLoopTowers->towers);
    EXPECT_TRUE(index == 2);

    index = Util::FindItemInList("TOWERWATERSYS COOLTOWER1", state->dataCondenserLoopTowers->towers);
    EXPECT_TRUE(index == 1);
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_VSCoolingTower_OutputReport)
{

    std::string const idf_objects = delimited_string({
        "  Site:Location,",
        "    USA IL-CHICAGO-OHARE,    !- Name",
        "    41.77,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190;                     !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

        "  SizingPeriod:DesignDay,",
        "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
        "    1.00;                    !- Sky Clearness",

        "    SimulationControl,",
        "    no,                     !- Do Zone Sizing Calculation",
        "    no,                     !- Do System Sizing Calculation",
        "    no,                     !- Do Plant Sizing Calculation",
        "    Yes,                    !- Run Simulation for Sizing Periods",
        "    no;                     !- Run Simulation for Weather File Run Periods",

        "  Timestep,6;",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  Schedule:Compact,",
        "    ALWAYS_ON,               !- Name",
        "    On/Off,                  !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 3",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  CoolingTower:VariableSpeed,",
        "    CoolingTower Variable Speed,  !- Name",
        "    Tower Inlet Node,        !- Water Inlet Node Name",
        "    Tower Outlet Node,       !- Water Outlet Node Name",
        "    CoolToolsCrossFlow,      !- Model Type",
        "    ,                        !- Model Coefficient Name",
        "    23.28,                   !- Design Inlet Air Wet-Bulb Temperature {C}",
        "    4.56,                    !- Design Approach Temperature {deltaC}",
        "    6.14,                    !- Design Range Temperature {deltaC}",
        "    0.02,                    !- Design Water Flow Rate {m3/s}",
        "    10.0,                    !- Design Air Flow Rate {m3/s}",
        "    1000.0,                  !- Design Fan Power {W}",
        "    CoolingTower Variable Speed Fan Power Ratio Curve,  !- Fan Power Ratio Function of Air Flow Rate Ratio Curve Name",
        "    0.2,                     !- Minimum Air Flow Rate Ratio",
        "    0.125,                   !- Fraction of Tower Capacity in Free Convection Regime",
        "    0,                       !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    SaturatedExit,           !- Evaporation Loss Mode",
        "    0.2,                     !- Evaporation Loss Factor {percent/K}",
        "    0.008,                   !- Drift Loss Percent {percent}",
        "    ConcentrationRatio,      !- Blowdown Calculation Mode",
        "    3,                       !- Blowdown Concentration Ratio",
        "    ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Outdoor Air Inlet Node Name",
        "    ,                        !- Number of Cells",
        "    ,                        !- Cell Control",
        "    ,                        !- Cell Minimum  Water Flow Rate Fraction",
        "    ,                        !- Cell Maximum Water Flow Rate Fraction",
        "    2.0,                     !- Sizing Factor",
        "    General;                 !- End-Use Subcategory",

        "  Curve:Cubic,",
        "    CoolingTower Variable Speed Fan Power Ratio Curve,  !- Name",
        "    -0.0093,                 !- Coefficient1 Constant",
        "    0.0512,                  !- Coefficient2 x",
        "    -0.0838,                 !- Coefficient3 x**2",
        "    1.0419,                  !- Coefficient4 x**3",
        "    0.15,                    !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "  Pump:VariableSpeed,",
        "    CoolingTower Pump,         !- Name",
        "    CoolingTower Supply Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Pump-CoolingTower CoolTowerNodeviaConnector,  !- Outlet Node Name",
        "      0.03,                    !- Design Maximum Flow Rate {m3/s}",
        "      134508,                  !- Design Pump Head {Pa}",
        "      20000.0,                 !- Design Power Consumption {W}",
        "      0.9,                     !- Motor Efficiency",
        "      0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "      0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "      0.0216,                  !- Coefficient 2 of the Part Load Performance Curve",
        "      -0.0325,                 !- Coefficient 3 of the Part Load Performance Curve",
        "      1.0095,                  !- Coefficient 4 of the Part Load Performance Curve",
        "      0,                       !- Design Minimum Flow Rate {m3/s}",
        "      Intermittent,            !- Pump Control Type",
        "      ,                        !- Pump Flow Rate Schedule Name",
        "      ,                        !- Pump Curve Name",
        "      ,                        !- Impeller Diameter {m}",
        "      ,                        !- VFD Control Type",
        "      ,                        !- Pump RPM Schedule Name",
        "      ,                        !- Minimum Pressure Schedule",
        "      ,                        !- Maximum Pressure Schedule",
        "      ,                        !- Minimum RPM Schedule",
        "      ,                        !- Maximum RPM Schedule",
        "      ,                        !- Zone Name",
        "      0.5,                     !- Skin Loss Radiative Fraction",
        "      PowerPerFlowPerPressure, !- Design Power Sizing Method",
        "      348701.1,                !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "      1.282051282,             !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
        "      0,                       !- Design Minimum Flow Rate Fraction",
        "      General;                 !- End-Use Subcategory",

        "  PlantEquipmentList,",
        "    CoolingTower Equipment List,  !- Name",
        "    CoolingTower:VariableSpeed,    !- Equipment 1 Object Type",
        "    CoolingTower Variable Speed;  !- Equipment 1 Name",

        "  PlantLoop,",
        "    CoolingTower Loop,       !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    CoolingTower Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
        "    CoolingTower Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
        "    80.0,                    !- Maximum Loop Temperature {C}",
        "    5.0,                     !- Minimum Loop Temperature {C}",
        "    0.03,                    !- Maximum Loop Flow Rate {m3/s}",
        "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
        "    Autocalculate,                     !- Condenser Loop Volume {m3}",
        "    CoolingTower Supply Inlet Node,  !- Condenser Side Inlet Node Name",
        "    CoolingTower Supply Outlet Node,  !- Condenser Side Outlet Node Name",
        "    CoolingTower Supply Branches,  !- Condenser Side Branch List Name",
        "    CoolingTower Supply Connectors,  !- Condenser Side Connector List Name",
        "    CoolingTower Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    CoolingTower Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    CoolingTower Demand Branches,  !- Condenser Demand Side Branch List Name",
        "    CoolingTower Demand Connectors,  !- Condenser Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    SingleSetpoint;          !- Plant Loop Demand Calculation Scheme",

        "  Sizing:Plant,",
        "    CoolingTower Loop,       !- Plant or Condenser Loop Name",
        "    Condenser,               !- Loop Type",
        "    30,                      !- Design Loop Exit Temperature {C}",
        "    5.55555555555556,        !- Loop Design Temperature Difference {deltaC}",
        "    NonCoincident,           !- Sizing Option",
        "    1,                       !- Zone Timesteps in Averaging Window",
        "    None;                    !- Coincident Sizing Factor Mode",

        "  PlantEquipmentOperationSchemes,",
        "    CoolingTower Loop Operation Scheme List,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    CoolingTower Operation Scheme,  !- Control Scheme 1 Name",
        "    ALWAYS_ON,                      !- Control Scheme 1 Schedule Name",
        "    PlantEquipmentOperation:ComponentSetpoint,  !- Control Scheme 3 Object Type",
        "    CoolingTower Loop Setpoint Operation Scheme,  !- Control Scheme 3 Name",
        "    ALWAYS_ON;      !- Control Scheme 3 Schedule Name",

        "  PlantEquipmentOperation:ComponentSetpoint,",
        "    CoolingTower Loop Setpoint Operation Scheme,  !- Name",
        "    CoolingTower:VariableSpeed,    !- Equipment 1 Object Type",
        "    CoolingTower Variable Speed,   !- Equipment 1 Name",
        "    CoolingTower Inlet Node,       !- Demand Calculation 1 Node Name",
        "    CoolingTower Outlet Node,      !- Setpoint 1 Node Name",
        "    0.03,                          !- Component 1 Flow Rate {m3/s}",
        "    Cooling;                       !- Operation 1 Type",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    CoolingTower Operation Scheme, !- Name",
        "    0.0,                           !- Load Range 1 Lower Limit {W}",
        "    1000000000000,                 !- Load Range 1 Upper Limit {W}",
        "    CoolingTower Equipment List;   !- Range 1 Equipment List Name",

        "  SetpointManager:Scheduled,",
        "    CoolingTower Setpoint Manager,  !- Name",
        "    Temperature,                 !- Control Variable",
        "    CoolingTower Temp Sch,       !- Schedule Name",
        "    CoolingTower Control Node List;   !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    CoolingTower Control Node List,   !- Name",
        "    CoolingTower Outlet Node,         !- Node 1 Name",
        "    CoolingTower Supply Outlet Node;  !- Node 2 Name",

        "  Schedule:Compact,",
        "    CoolingTower Temp Sch,   !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,30.0;       !- Field 3",

        "  BranchList,",
        "    CoolingTower Demand Branches,  !- Name",
        "    CoolingTower Demand Inlet Branch,  !- Branch 1 Name",
        "    CoolingTower Demand Load Branch 1,  !- Branch 2 Name",
        "    CoolingTower Demand Bypass Branch,  !- Branch 4 Name",
        "    CoolingTower Demand Outlet Branch;  !- Branch 5 Name",

        "  BranchList,",
        "    CoolingTower Supply Branches,  !- Name",
        "    CoolingTower Supply Inlet Branch,  !- Branch 1 Name",
        "    CoolingTower Supply Equipment Branch 1,  !- Branch 2 Name",
        "    CoolingTower Supply Equipment Bypass Branch,  !- Branch 4 Name",
        "    CoolingTower Supply Outlet Branch;  !- Branch 5 Name",

        "  Branch,",
        "    CoolingTower Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Demand Bypass Pipe,  !- Component 1 Name",
        "    CoolingTower Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Demand Inlet Pipe,  !- Component 1 Name",
        "    CoolingTower Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Demand Inlet Pipe-CoolingTower Demand Mixer;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Demand Load Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    LoadProfile:Plant,  !- Component 1 Object Type",
        "    Load Profile 1,      !- Component 1 Name",
        "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
        "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

        "  LoadProfile:Plant,",
        "    Load Profile 1,          !- Name",
        "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
        "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
        "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
        "    0.010,                    !- Peak Flow Rate {m3/s}",
        "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

        "  Schedule:Compact,",
        "    Load Profile 1 Load Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,0.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Load Profile 1 Flow Frac Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,1.0;        !- Field 3",

        "  Branch,",
        "    CoolingTower Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Demand Outlet Pipe,  !- Component 1 Name",
        "    CoolingTower Demand Mixer-CoolingTower Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
        "    CoolingTower Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Equipment Branch 1,  !- Name",
        "    ,                            !- Pressure Drop Curve Name",
        "    CoolingTower:VariableSpeed,  !- Component 1 Object Type",
        "    CoolingTower Variable Speed, !- Component 1 Name",
        "    CoolingTower Inlet Node,     !- Component 1 Inlet Node Name",
        "    CoolingTower Outlet Node;    !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Equipment Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Supply Equipment Bypass Pipe,  !- Component 1 Name",
        "    CoolingTower Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    CoolingTower Pump,       !- Component 1 Name",
        "    CoolingTower Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Pump-CoolingTower CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Supply Outlet Pipe,  !- Component 1 Name",
        "    CoolingTower Supply Mixer-CoolingTower Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
        "    CoolingTower Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  OutdoorAir:Node,",
        "    CoolingTower CoolTower OA ref Node;  !- Name",

        "  ConnectorList,",
        "    CoolingTower Demand Connectors,  !- Name",
        "    Connector:Splitter,          !- Connector 1 Object Type",
        "    CoolingTower Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,             !- Connector 2 Object Type",
        "    CoolingTower Demand Mixer;  !- Connector 2 Name",

        "  ConnectorList,",
        "    CoolingTower Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CoolingTower Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CoolingTower Supply Mixer;  !- Connector 2 Name",

        "  Connector:Splitter,",
        "    CoolingTower Demand Splitter,  !- Name",
        "    CoolingTower Demand Inlet Branch,  !- Inlet Branch Name",
        "    CoolingTower Demand Load Branch 1,  !- Outlet Branch 1 Name",
        "    CoolingTower Demand Bypass Branch;  !- Outlet Branch 3 Name",

        "  Connector:Splitter,",
        "    CoolingTower Supply Splitter,  !- Name",
        "    CoolingTower Supply Inlet Branch,  !- Inlet Branch Name",
        "    CoolingTower Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
        "    CoolingTower Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

        "  Connector:Mixer,",
        "    CoolingTower Demand Mixer,  !- Name",
        "    CoolingTower Demand Outlet Branch,  !- Outlet Branch Name",
        "    CoolingTower Demand Load Branch 1,  !- Inlet Branch 1 Name",
        "    CoolingTower Demand Bypass Branch;  !- Inlet Branch 3 Name",

        "  Connector:Mixer,",
        "    CoolingTower Supply Mixer,  !- Name",
        "    CoolingTower Supply Outlet Branch,  !- Outlet Branch Name",
        "    CoolingTower Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
        "    CoolingTower Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Demand Bypass Pipe,  !- Name",
        "    CoolingTower Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Demand Inlet Pipe,  !- Name",
        "    CoolingTower Demand Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Demand Inlet Pipe-CoolingTower Demand Mixer;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Demand Outlet Pipe,  !- Name",
        "    CoolingTower Demand Mixer-CoolingTower Demand Outlet Pipe,  !- Inlet Node Name",
        "    CoolingTower Demand Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Supply Equipment Bypass Pipe,  !- Name",
        "    CoolingTower Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Supply Outlet Pipe,  !- Name",
        "    CoolingTower Supply Mixer-CoolingTower Supply Outlet Pipe,  !- Inlet Node Name",
        "    CoolingTower Supply Outlet Node;  !- Outlet Node Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::PostIPProcessing(*state);

    state->init_state(*state); // need to process schedules once they have been input processed

    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);

    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone);
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state);

    // Get plant loop data
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    SizingManager::GetPlantSizingInput(*state);
    PlantManager::InitOneTimePlantSizingInfo(*state, 1);
    PlantManager::SizePlantLoop(*state, 1, true);

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    constexpr Real64 expectedDesignInletWB = 23.28;
    constexpr Real64 expectedDesingApproach = 4.56;
    constexpr Real64 expectedDesingRange = 6.14;
    constexpr Real64 expectedDesOutletWaterTemp = expectedDesignInletWB + expectedDesingApproach;
    constexpr Real64 expectedDesInletWaterTemp = expectedDesOutletWaterTemp + expectedDesingRange;

    CondenserLoopTowers::GetTowerInput(*state);
    auto &VSTower = state->dataCondenserLoopTowers->towers(1);

    EXPECT_EQ(expectedDesignInletWB, VSTower.DesignInletWB);
    EXPECT_EQ(expectedDesingApproach, VSTower.DesignApproach);
    EXPECT_EQ(expectedDesingRange, VSTower.DesignRange);
    EXPECT_EQ(expectedDesOutletWaterTemp, VSTower.DesOutletWaterTemp);
    EXPECT_EQ(expectedDesInletWaterTemp, VSTower.DesInletWaterTemp);

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataGlobal->BeginEnvrnFlag = true;

    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutWetBulbTemp = 26.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat =
        Psychrometrics::PsyWFnTdbTwbPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutWetBulbTemp, state->dataEnvrn->OutBaroPress);
    state->dataLoopNodes->Node(VSTower.WaterInletNodeNum).Temp = 35.0;

    EXPECT_TRUE(state->dataPlnt->PlantFinalSizesOkayToReport = true);

    VSTower.initialize(*state);
    state->dataGlobal->BeginEnvrnFlag = false;
    VSTower.SizeTower(*state); // This calls PreDefTableEntry when state.dataPlnt->PlantFinalSizesOkayToReport == true

    auto &orp = *state->dataOutRptPredefined;
    std::string const TowerName = VSTower.Name;
    EXPECT_EQ("CoolingTower:VariableSpeed", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCType, TowerName));
    EXPECT_EQ("WATER", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCFluidType, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesingRange), OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCRange, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesingApproach),
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCApproach, TowerName));
    EXPECT_EQ("1000.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCDesFanPwr, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesignInletWB),
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCDesInletAirWBT, TowerName));
    EXPECT_EQ("0.020000", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCDesWaterFlowRate, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesOutletWaterTemp),
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCLevWaterSPTemp, TowerName));
    EXPECT_EQ("COOLINGTOWER LOOP", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCCondLoopName, TowerName));
    EXPECT_EQ("COOLINGTOWER SUPPLY EQUIPMENT BRANCH 1",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCCondLoopBranchName, TowerName));
}

TEST_F(EnergyPlusFixture, CondenserLoopTowers_VSMerkelCoolingTower_OutputReport)
{

    std::string const idf_objects = delimited_string({
        "  Site:Location,",
        "    USA IL-CHICAGO-OHARE,    !- Name",
        "    41.77,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190;                     !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO Ann Htg 99.6% Condns DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -20.6,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -20.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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

        "  SizingPeriod:DesignDay,",
        "    CHICAGO Ann Clg .4% Condns WB=>MDB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.2,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    25.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
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
        "    1.00;                    !- Sky Clearness",

        "    SimulationControl,",
        "    no,                     !- Do Zone Sizing Calculation",
        "    no,                     !- Do System Sizing Calculation",
        "    no,                     !- Do Plant Sizing Calculation",
        "    Yes,                    !- Run Simulation for Sizing Periods",
        "    no;                     !- Run Simulation for Weather File Run Periods",

        "  Timestep,6;",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  Schedule:Compact,",
        "    ALWAYS_ON,               !- Name",
        "    On/Off,                  !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 3",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "CoolingTower:VariableSpeed:Merkel,",
        "  CoolingTower Variable Speed Merkel, !- Name",
        "  CoolingTower Inlet Node, !- Water Inlet Node Name",
        "  CoolingTower Outlet Node,!- Water Outlet Node Name",
        "  NominalCapacity,         !- Performance Input Method",
        "  1.25,                    !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "  autosize,                !- Nominal Capacity {W}",
        "  autocalculate,           !- Free Convection Nominal Capacity {W}",
        "  0.1,                     !- Free Convection Nominal Capacity Sizing Factor",
        "  0.02,                    !- Design Water Flow Rate {m3/s}",
        "  5.382E-8,                !- Design Water Flow Rate per Unit of Nominal Capacity {m3/s-W}",
        "  autocalculate,           !- Design Air Flow Rate {m3/s}",
        "  2.76316E-5,              !- Design Air Flow Rate Per Unit of Nominal Capacity {m3/s-W}",
        "  0.2,                     !- Minimum Air Flow Rate Ratio",
        "  4000,                    !- Design Fan Power {W}",
        "  0.0105,                  !- Design Fan Power Per Unit of Nominal Capacity {dimensionless}",
        "  VS tower fan power mod func air flow ratio,  !- Fan Power Modifier Function of Air Flow Rate Ratio Curve Name",
        "  autocalculate,           !- Free Convection Regime Air Flow Rate {m3/s}",
        "  0.1,                     !- Free Convection Regime Air Flow Rate Sizing Factor",
        "  ,                        !- Design Air Flow Rate U-Factor Times Area Value {W/K}",
        "  ,                        !- Free Convection Regime U-Factor Times Area Value {W/K}",
        "  ,                        !- Free Convection U-Factor Times Area Value Sizing Factor",
        "  VS tower UA mod func air flow ratio,  !- U-Factor Times Area Modifier Function of Air Flow Ratio Curve Name",
        "  VS tower UA mod func wetbulb difference,  !- U-Factor Times Area Modifier Function of Wetbulb Temperature Difference Curve Name",
        "  VS tower UA mod func water flow ratio,  !- U-Factor Times Area Modifier Function of Water Flow Ratio Curve Name",
        "  35.0,                    !- Design Inlet Air Dry-Bulb Temperature {C}",
        "  25.7,                    !- Design Inlet Air Wet-Bulb Temperature {C}",
        "  4.56,                    !- Design Approach Temperature {deltaC}",
        "  6.14,                    !- Design Range Temperature {deltaC}",
        "  0.0,                     !- Basin Heater Capacity {W/K}",
        "  2.0,                     !- Basin Heater Setpoint Temperature {C}",
        "  ,                        !- Basin Heater Operating Schedule Name",
        "  SaturatedExit,           !- Evaporation Loss Mode",
        "  0.2,                     !- Evaporation Loss Factor {percent/K}",
        "  0.008,                   !- Drift Loss Percent {percent}",
        "  ConcentrationRatio,      !- Blowdown Calculation Mode",
        "  3.0,                     !- Blowdown Concentration Ratio",
        "  ,                        !- Blowdown Makeup Water Usage Schedule Name",
        "  ,                        !- Supply Water Storage Tank Name",
        "  ,                        !- Outdoor Air Inlet Node Name",
        "  1,                       !- Number of Cells",
        "  MaximalCell,             !- Cell Control",
        "  0.33,                    !- Cell Minimum  Water Flow Rate Fraction",
        "  2.5,                     !- Cell Maximum Water Flow Rate Fraction",
        "  1.0,                     !- Sizing Factor",
        "  General;                 !- End-Use Subcategory",

        "  Curve:Cubic,",
        "    VS tower fan power mod func air flow ratio,  !- Name",
        "    0.02,                    !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.98,                    !- Coefficient4 x**3",
        "    0.2,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.0,                     !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    VS tower UA mod func air flow ratio,  !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.3,                     !- Coefficient2 x",
        "    -0.3,                    !- Coefficient3 x**2",
        "    0.2,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.0,                     !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Linear,",
        "    VS tower UA mod func wetbulb difference,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0081,                  !- Coefficient2 x",
        "    -10,                     !- Minimum Value of x",
        "    25.0,                    !- Maximum Value of x",
        "    0.85,                    !- Minimum Curve Output",
        "    1.3,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    VS tower UA mod func water flow ratio,  !- Name",
        "    0.1082,                  !- Coefficient1 Constant",
        "    1.667,                   !- Coefficient2 x",
        "    -0.7713,                 !- Coefficient3 x**2",
        "    0.3,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.5,                     !- Minimum Curve Output",
        "    1.1,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Pump:VariableSpeed,",
        "    CoolingTower Pump,         !- Name",
        "    CoolingTower Supply Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Pump-CoolingTower CoolTowerNodeviaConnector,  !- Outlet Node Name",
        "      0.03,                    !- Design Maximum Flow Rate {m3/s}",
        "      134508,                  !- Design Pump Head {Pa}",
        "      20000.0,                 !- Design Power Consumption {W}",
        "      0.9,                     !- Motor Efficiency",
        "      0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "      0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "      0.0216,                  !- Coefficient 2 of the Part Load Performance Curve",
        "      -0.0325,                 !- Coefficient 3 of the Part Load Performance Curve",
        "      1.0095,                  !- Coefficient 4 of the Part Load Performance Curve",
        "      0,                       !- Design Minimum Flow Rate {m3/s}",
        "      Intermittent,            !- Pump Control Type",
        "      ,                        !- Pump Flow Rate Schedule Name",
        "      ,                        !- Pump Curve Name",
        "      ,                        !- Impeller Diameter {m}",
        "      ,                        !- VFD Control Type",
        "      ,                        !- Pump RPM Schedule Name",
        "      ,                        !- Minimum Pressure Schedule",
        "      ,                        !- Maximum Pressure Schedule",
        "      ,                        !- Minimum RPM Schedule",
        "      ,                        !- Maximum RPM Schedule",
        "      ,                        !- Zone Name",
        "      0.5,                     !- Skin Loss Radiative Fraction",
        "      PowerPerFlowPerPressure, !- Design Power Sizing Method",
        "      348701.1,                !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "      1.282051282,             !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
        "      0,                       !- Design Minimum Flow Rate Fraction",
        "      General;                 !- End-Use Subcategory",

        "  PlantEquipmentList,",
        "    CoolingTower Equipment List,  !- Name",
        "    CoolingTower:VariableSpeed:Merkel,  !- Equipment 1 Object Type",
        "    CoolingTower Variable Speed Merkel;  !- Equipment 1 Name",

        "  PlantLoop,",
        "    CoolingTower Loop,       !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    CoolingTower Loop Operation Scheme List,  !- Condenser Equipment Operation Scheme Name",
        "    CoolingTower Supply Outlet Node,  !- Condenser Loop Temperature Setpoint Node Name",
        "    80.0,                    !- Maximum Loop Temperature {C}",
        "    5.0,                     !- Minimum Loop Temperature {C}",
        "    0.03,                    !- Maximum Loop Flow Rate {m3/s}",
        "    0.0,                     !- Minimum Loop Flow Rate {m3/s}",
        "    Autocalculate,                     !- Condenser Loop Volume {m3}",
        "    CoolingTower Supply Inlet Node,  !- Condenser Side Inlet Node Name",
        "    CoolingTower Supply Outlet Node,  !- Condenser Side Outlet Node Name",
        "    CoolingTower Supply Branches,  !- Condenser Side Branch List Name",
        "    CoolingTower Supply Connectors,  !- Condenser Side Connector List Name",
        "    CoolingTower Demand Inlet Node,  !- Demand Side Inlet Node Name",
        "    CoolingTower Demand Outlet Node,  !- Demand Side Outlet Node Name",
        "    CoolingTower Demand Branches,  !- Condenser Demand Side Branch List Name",
        "    CoolingTower Demand Connectors,  !- Condenser Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    SingleSetpoint;          !- Plant Loop Demand Calculation Scheme",

        "  Sizing:Plant,",
        "    CoolingTower Loop,       !- Plant or Condenser Loop Name",
        "    Condenser,               !- Loop Type",
        "    30.25,                   !- Design Loop Exit Temperature {C}",
        "    6.14,                    !- Loop Design Temperature Difference {deltaC}",
        "    NonCoincident,           !- Sizing Option",
        "    1,                       !- Zone Timesteps in Averaging Window",
        "    None;                    !- Coincident Sizing Factor Mode",

        "  PlantEquipmentOperationSchemes,",
        "    CoolingTower Loop Operation Scheme List,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    CoolingTower Operation Scheme,  !- Control Scheme 1 Name",
        "    ALWAYS_ON,                      !- Control Scheme 1 Schedule Name",
        "    PlantEquipmentOperation:ComponentSetpoint,  !- Control Scheme 3 Object Type",
        "    CoolingTower Loop Setpoint Operation Scheme,  !- Control Scheme 3 Name",
        "    ALWAYS_ON;      !- Control Scheme 3 Schedule Name",

        "  PlantEquipmentOperation:ComponentSetpoint,",
        "    CoolingTower Loop Setpoint Operation Scheme,  !- Name",
        "    CoolingTower:VariableSpeed:Merkel,  !- Equipment 1 Object Type",
        "    CoolingTower Variable Speed Merkel, !- Equipment 1 Name",
        "    CoolingTower Inlet Node,       !- Demand Calculation 1 Node Name",
        "    CoolingTower Outlet Node,      !- Setpoint 1 Node Name",
        "    0.03,                          !- Component 1 Flow Rate {m3/s}",
        "    Cooling;                       !- Operation 1 Type",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    CoolingTower Operation Scheme, !- Name",
        "    0.0,                           !- Load Range 1 Lower Limit {W}",
        "    1000000000000,                 !- Load Range 1 Upper Limit {W}",
        "    CoolingTower Equipment List;   !- Range 1 Equipment List Name",

        "  SetpointManager:Scheduled,",
        "    CoolingTower Setpoint Manager,  !- Name",
        "    Temperature,                 !- Control Variable",
        "    CoolingTower Temp Sch,       !- Schedule Name",
        "    CoolingTower Control Node List;   !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    CoolingTower Control Node List,   !- Name",
        "    CoolingTower Outlet Node,         !- Node 1 Name",
        "    CoolingTower Supply Outlet Node;  !- Node 2 Name",

        "  Schedule:Compact,",
        "    CoolingTower Temp Sch,   !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,30.0;       !- Field 3",

        "  BranchList,",
        "    CoolingTower Demand Branches,  !- Name",
        "    CoolingTower Demand Inlet Branch,  !- Branch 1 Name",
        "    CoolingTower Demand Load Branch 1,  !- Branch 2 Name",
        "    CoolingTower Demand Bypass Branch,  !- Branch 4 Name",
        "    CoolingTower Demand Outlet Branch;  !- Branch 5 Name",

        "  BranchList,",
        "    CoolingTower Supply Branches,  !- Name",
        "    CoolingTower Supply Inlet Branch,  !- Branch 1 Name",
        "    CoolingTower Supply Equipment Branch 1,  !- Branch 2 Name",
        "    CoolingTower Supply Equipment Bypass Branch,  !- Branch 4 Name",
        "    CoolingTower Supply Outlet Branch;  !- Branch 5 Name",

        "  Branch,",
        "    CoolingTower Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Demand Bypass Pipe,  !- Component 1 Name",
        "    CoolingTower Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Demand Inlet Pipe,  !- Component 1 Name",
        "    CoolingTower Demand Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Demand Inlet Pipe-CoolingTower Demand Mixer;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Demand Load Branch 1,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    LoadProfile:Plant,  !- Component 1 Object Type",
        "    Load Profile 1,      !- Component 1 Name",
        "    Demand Load Profile 1 Inlet Node,  !- Component 1 Inlet Node Name",
        "    Demand Load Profile 1 Outlet Node;  !- Component 1 Outlet Node Name",

        "  LoadProfile:Plant,",
        "    Load Profile 1,          !- Name",
        "    Demand Load Profile 1 Inlet Node,  !- Inlet Node Name",
        "    Demand Load Profile 1 Outlet Node,  !- Outlet Node Name",
        "    Load Profile 1 Load Schedule,  !- Load Schedule Name",
        "    0.010,                    !- Peak Flow Rate {m3/s}",
        "    Load Profile 1 Flow Frac Schedule;  !- Flow Rate Fraction Schedule Name",

        "  Schedule:Compact,",
        "    Load Profile 1 Load Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,0.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Load Profile 1 Flow Frac Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    THROUGH: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    UNTIL: 24:00,1.0;        !- Field 3",

        "  Branch,",
        "    CoolingTower Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Demand Outlet Pipe,  !- Component 1 Name",
        "    CoolingTower Demand Mixer-CoolingTower Demand Outlet Pipe,  !- Component 1 Inlet Node Name",
        "    CoolingTower Demand Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Equipment Branch 1,  !- Name",
        "    ,                            !- Pressure Drop Curve Name",
        "    CoolingTower:VariableSpeed:Merkel,  !- Component 1 Object Type",
        "    CoolingTower Variable Speed Merkel, !- Component 1 Name",
        "    CoolingTower Inlet Node,     !- Component 1 Inlet Node Name",
        "    CoolingTower Outlet Node;    !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Equipment Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Supply Equipment Bypass Pipe,  !- Component 1 Name",
        "    CoolingTower Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Supply Equip Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    CoolingTower Pump,       !- Component 1 Name",
        "    CoolingTower Supply Inlet Node,  !- Component 1 Inlet Node Name",
        "    CoolingTower Pump-CoolingTower CoolTowerNodeviaConnector;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    CoolingTower Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CoolingTower Supply Outlet Pipe,  !- Component 1 Name",
        "    CoolingTower Supply Mixer-CoolingTower Supply Outlet Pipe,  !- Component 1 Inlet Node Name",
        "    CoolingTower Supply Outlet Node;  !- Component 1 Outlet Node Name",

        "  OutdoorAir:Node,",
        "    CoolingTower CoolTower OA ref Node;  !- Name",

        "  ConnectorList,",
        "    CoolingTower Demand Connectors,  !- Name",
        "    Connector:Splitter,          !- Connector 1 Object Type",
        "    CoolingTower Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,             !- Connector 2 Object Type",
        "    CoolingTower Demand Mixer;  !- Connector 2 Name",

        "  ConnectorList,",
        "    CoolingTower Supply Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CoolingTower Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CoolingTower Supply Mixer;  !- Connector 2 Name",

        "  Connector:Splitter,",
        "    CoolingTower Demand Splitter,  !- Name",
        "    CoolingTower Demand Inlet Branch,  !- Inlet Branch Name",
        "    CoolingTower Demand Load Branch 1,  !- Outlet Branch 1 Name",
        "    CoolingTower Demand Bypass Branch;  !- Outlet Branch 3 Name",

        "  Connector:Splitter,",
        "    CoolingTower Supply Splitter,  !- Name",
        "    CoolingTower Supply Inlet Branch,  !- Inlet Branch Name",
        "    CoolingTower Supply Equipment Branch 1,  !- Outlet Branch 1 Name",
        "    CoolingTower Supply Equipment Bypass Branch;  !- Outlet Branch 3 Name",

        "  Connector:Mixer,",
        "    CoolingTower Demand Mixer,  !- Name",
        "    CoolingTower Demand Outlet Branch,  !- Outlet Branch Name",
        "    CoolingTower Demand Load Branch 1,  !- Inlet Branch 1 Name",
        "    CoolingTower Demand Bypass Branch;  !- Inlet Branch 3 Name",

        "  Connector:Mixer,",
        "    CoolingTower Supply Mixer,  !- Name",
        "    CoolingTower Supply Outlet Branch,  !- Outlet Branch Name",
        "    CoolingTower Supply Equipment Branch 1,  !- Inlet Branch 1 Name",
        "    CoolingTower Supply Equipment Bypass Branch;  !- Inlet Branch 3 Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Demand Bypass Pipe,  !- Name",
        "    CoolingTower Demand Bypass Pipe Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Demand Bypass Pipe Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Demand Inlet Pipe,  !- Name",
        "    CoolingTower Demand Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Demand Inlet Pipe-CoolingTower Demand Mixer;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Demand Outlet Pipe,  !- Name",
        "    CoolingTower Demand Mixer-CoolingTower Demand Outlet Pipe,  !- Inlet Node Name",
        "    CoolingTower Demand Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Supply Equipment Bypass Pipe,  !- Name",
        "    CoolingTower Supply Equip Bypass Inlet Node,  !- Inlet Node Name",
        "    CoolingTower Supply Equip Bypass Outlet Node;  !- Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CoolingTower Supply Outlet Pipe,  !- Name",
        "    CoolingTower Supply Mixer-CoolingTower Supply Outlet Pipe,  !- Inlet Node Name",
        "    CoolingTower Supply Outlet Node;  !- Outlet Node Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::PostIPProcessing(*state);

    state->init_state(*state); // need to process schedules once they have been input processed

    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);

    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone);
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    createFacilityElectricPowerServiceObject(*state);
    OutputProcessor::GetReportVariableInput(*state);
    PlantManager::CheckIfAnyPlant(*state);
    BranchInputManager::ManageBranchInput(*state);

    // Get plant loop data
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    SizingManager::GetPlantSizingInput(*state);
    PlantManager::InitOneTimePlantSizingInfo(*state, 1);
    PlantManager::SizePlantLoop(*state, 1, true);

    // Fake having more than small load
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.02;

    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KickOffSimulation = true;

    constexpr Real64 expectedDesignInletWB = 25.7;
    constexpr Real64 expectedDesingApproach = 4.56;
    constexpr Real64 expectedDesingRange = 6.14;
    constexpr Real64 expectedDesOutletWaterTemp = expectedDesignInletWB + expectedDesingApproach;
    constexpr Real64 expectedDesInletWaterTemp = expectedDesOutletWaterTemp + expectedDesingRange;

    CondenserLoopTowers::GetTowerInput(*state);
    auto &VSTower = state->dataCondenserLoopTowers->towers(1);

    EXPECT_EQ(expectedDesignInletWB, VSTower.DesignInletWB);
    EXPECT_EQ(expectedDesingApproach, VSTower.DesignApproach);
    EXPECT_EQ(expectedDesingRange, VSTower.DesignRange);
    EXPECT_EQ(expectedDesOutletWaterTemp, VSTower.DesOutletWaterTemp);
    EXPECT_EQ(expectedDesInletWaterTemp, VSTower.DesInletWaterTemp);

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataGlobal->BeginEnvrnFlag = true;

    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutWetBulbTemp = 26.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat =
        Psychrometrics::PsyWFnTdbTwbPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutWetBulbTemp, state->dataEnvrn->OutBaroPress);
    state->dataLoopNodes->Node(VSTower.WaterInletNodeNum).Temp = 35.0;

    EXPECT_TRUE(state->dataPlnt->PlantFinalSizesOkayToReport = true);

    VSTower.initialize(*state);
    state->dataGlobal->BeginEnvrnFlag = false;
    VSTower.SizeVSMerkelTower(*state); // This calls PreDefTableEntry when state.dataPlnt->PlantFinalSizesOkayToReport == true

    auto &orp = *state->dataOutRptPredefined;
    std::string const TowerName = VSTower.Name;
    EXPECT_EQ("CoolingTower:VariableSpeed:Merkel", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCType, TowerName));
    EXPECT_EQ("WATER", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCFluidType, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesingRange), OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCRange, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesingApproach),
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCApproach, TowerName));
    EXPECT_EQ("4000.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCDesFanPwr, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesignInletWB),
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCDesInletAirWBT, TowerName));
    EXPECT_EQ("0.020000", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCDesWaterFlowRate, TowerName));
    EXPECT_EQ(std::format("{:.2f}", expectedDesOutletWaterTemp),
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCLevWaterSPTemp, TowerName));
    EXPECT_EQ("COOLINGTOWER LOOP", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCCondLoopName, TowerName));
    EXPECT_EQ("COOLINGTOWER SUPPLY EQUIPMENT BRANCH 1",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchCTFCCondLoopBranchName, TowerName));
}

} // namespace EnergyPlus
