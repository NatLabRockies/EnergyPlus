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

// EnergyPlus::FuelCellElectricGenerator Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// C++ Headers
#include <memory>
#include <vector>

// EnergyPlus Headers
#include "EnergyPlus/CurveManager.hh"
#include "EnergyPlus/DataGenerators.hh"
#include "EnergyPlus/Plant/Loop.hh"
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/FuelCellElectricGenerator.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, FuelCellTest)
{
    std::string const idf_objects = delimited_string({

        "Material,",
        "  8 in.Concrete Block Basement Wall,      !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.2032,                                 !- Thickness{ m }",
        "  1.326,                                  !- Conductivity{ W / m - K }",
        "  1841.99999999999,                       !- Density{ kg / m3 }",
        "  911.999999999999,                       !- Specific Heat{ J / kg - K }",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Construction,",
        "   Typical,   !- Name",
        "   8 in.Concrete Block Basement Wall;     !- Layer 1",

        "Zone,",
        "  Thermal Zone 1,                         !- Name",
        "  0,                                      !- Direction of Relative North {deg}",
        "  0,                                      !- X Origin {m}",
        "  0,                                      !- Y Origin {m}",
        "  0,                                      !- Z Origin {m}",
        "  ,                                       !- Type",
        "  1,                                      !- Multiplier",
        "  ,                                       !- Ceiling Height {m}",
        "  ,                                       !- Volume {m3}",
        "  ,                                       !- Floor Area {m2}",
        "  ,                                       !- Zone Inside Convection Algorithm",
        "  ,                                       !- Zone Outside Convection Algorithm",
        "  Yes;                                    !- Part of Total Floor Area",

        "BuildingSurface:Detailed,",
        "  Floor,                                  !- Name",
        "  Floor,                                  !- Surface Type",
        "  Typical,                                !- Construction Name",
        "  Thermal Zone 1,                         !- Zone Name",
        "    ,                        !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Wall 1,                                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  Typical,                                !- Construction Name",
        "  Thermal Zone 1,                         !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 3,                               !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 3;                                !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Wall 2,                                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  Typical,                                !- Construction Name",
        "  Thermal Zone 1,                         !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 3,                              !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 3;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Wall 3,                                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  Typical,                                !- Construction Name",
        "  Thermal Zone 1,                         !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 3,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 3;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Wall 4,                                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  Typical,                                !- Construction Name",
        "  Thermal Zone 1,                         !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 3,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 3;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Roof,                                   !- Name",
        "  Roof,                                   !- Surface Type",
        "  Typical,                                !- Construction Name",
        "  Thermal Zone 1,                         !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 3,                               !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 3,                              !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 3,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 3;                                !- X,Y,Z Vertex 4 {m}",

        "ScheduleTypeLimits,",
        "  OnOff,                                  !- Name",
        "  0,                                      !- Lower Limit Value {BasedOnField A3}",
        "  1,                                      !- Upper Limit Value {BasedOnField A3}",
        "  Discrete,                               !- Numeric Type",
        "  availability;                           !- Unit Type",

        "ScheduleTypeLimits,",
        "  Temperature,                            !- Name",
        "  0,                                      !- Lower Limit Value {BasedOnField A3}",
        "  100,                                    !- Upper Limit Value {BasedOnField A3}",
        "  Continuous,                             !- Numeric Type",
        "  temperature;                            !- Unit Type",

        "Schedule:Constant,",
        "  Always On Discrete,                     !- Name",
        "  OnOff,                                  !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  Fuel Temperature,                       !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  20;                                     !- Hourly Value",

        "Schedule:Constant,",
        "  Hot_Water_Temperature,                  !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  55;                                     !- Hourly Value",

        "Schedule:Constant,",
        "  Water Temperature,                      !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  20;                                     !- Hourly Value",

        "Schedule:Constant,",
        "  Ambient Temperature 22C,                !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  22;                                     !- Hourly Value",

        "OutdoorAir:Node,",
        "  Model Outdoor Air Node;                 !- Name",

        "PlantLoop,",
        "  SHW Loop,                               !- Name",
        "  Water,                                  !- Fluid Type",
        "  ,                                       !- User Defined Fluid Type",
        "  SHW Loop Operation Schemes,             !- Plant Equipment Operation Scheme Name",
        "  Node 3,                                 !- Loop Temperature Setpoint Node Name",
        "  100,                                    !- Maximum Loop Temperature {C}",
        "  0,                                      !- Minimum Loop Temperature {C}",
        "  Autosize,                               !- Maximum Loop Flow Rate {m3/s}",
        "  0,                                      !- Minimum Loop Flow Rate {m3/s}",
        "  Autocalculate,                          !- Plant Loop Volume {m3}",
        "  Node 2,                                 !- Plant Side Inlet Node Name",
        "  Node 3,                                 !- Plant Side Outlet Node Name",
        "  SHW Loop Supply Branches,               !- Plant Side Branch List Name",
        "  SHW Loop Supply Connector List,         !- Plant Side Connector List Name",
        "  Node 5,                                 !- Demand Side Inlet Node Name",
        "  Node 6,                                 !- Demand Side Outlet Node Name",
        "  SHW Loop Demand Branches,               !- Demand Side Branch List Name",
        "  SHW Loop Demand Connector List,         !- Demand Side Connector List Name",
        "  Optimal,                                !- Load Distribution Scheme",
        "  ,                                       !- Availability Manager List Name",
        "  SingleSetpoint,                         !- Plant Loop Demand Calculation Scheme",
        "  ;                                       !- Common Pipe Simulation",

        "Sizing:Plant,",
        "  SHW Loop,                               !- Plant or Condenser Loop Name",
        "  Heating,                                !- Loop Type",
        "  82,                                     !- Design Loop Exit Temperature {C}",
        "  11,                                     !- Loop Design Temperature Difference {deltaC}",
        "  NonCoincident,                          !- Sizing Option",
        "  1,                                      !- Zone Timesteps in Averaging Window",
        "  None;                                   !- Coincident Sizing Factor Mode",

        "BranchList,",
        "  SHW Loop Supply Branches,               !- Name",
        "  SHW Loop Supply Inlet Branch,           !- Branch Name 1",
        "  SHW Loop Supply Branch 2,               !- Branch Name 3",
        "  SHW Loop Supply Outlet Branch;          !- Branch Name 4",

        "ConnectorList,",
        "  SHW Loop Supply Connector List,         !- Name",
        "  Connector:Splitter,                     !- Connector Object Type 1",
        "  SHW Loop Supply Splitter,               !- Connector Name 1",
        "  Connector:Mixer,                        !- Connector Object Type 2",
        "  SHW Loop Supply Mixer;                  !- Connector Name 2",

        "Connector:Splitter,",
        "  SHW Loop Supply Splitter,               !- Name",
        "  SHW Loop Supply Inlet Branch,           !- Inlet Branch Name",
        "  SHW Loop Supply Branch 2;               !- Outlet Branch Name 2",

        "Connector:Mixer,",
        "  SHW Loop Supply Mixer,                  !- Name",
        "  SHW Loop Supply Outlet Branch,          !- Outlet Branch Name",
        "  SHW Loop Supply Branch 2;               !- Inlet Branch Name 2",

        "Branch,",
        "  SHW Loop Supply Inlet Branch,           !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pump:ConstantSpeed,                     !- Component Object Type 1",
        "  Pump Constant Speed 1,                  !- Component Name 1",
        "  Node 2,                                 !- Component Inlet Node Name 1",
        "  Node 8;                                 !- Component Outlet Node Name 1",

        "Pump:ConstantSpeed,",
        "  Pump Constant Speed 1,                  !- Name",
        "  Node 2,                                 !- Inlet Node Name",
        "  Node 8,                                 !- Outlet Node Name",
        "  Autosize,                               !- Design Flow Rate {m3/s}",
        "  179352,                                 !- Design Pump Head {Pa}",
        "  Autosize,                               !- Design Power Consumption {W}",
        "  0.9,                                    !- Motor Efficiency",
        "  0,                                      !- Fraction of Motor Inefficiencies to Fluid Stream",
        "  Intermittent,                           !- Pump Control Type",
        "  ,                                       !- Pump Flow Rate Schedule Name",
        "  ,                                       !- Pump Curve Name",
        "  ,                                       !- Impeller Diameter {m}",
        "  ,                                       !- Rotational Speed {rev/min}",
        "  ,                                       !- Zone Name",
        "  ,                                       !- Skin Loss Radiative Fraction",
        "  PowerPerFlowPerPressure,                !- Design Power Sizing Method",
        "  348701.1,                               !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "  1.282051282,                            !- Design Shaft Power per Unit Flow Rate per Unit Head {W-s/m3-Pa}",
        "  General;                                !- End-Use Subcategory",

        "Branch,",
        "  SHW Loop Supply Branch 2,           !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Generator:FuelCell:ExhaustGasToWaterHeatExchanger, !- Component Object Type 1",
        "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1, !- Component Name 1",
        "  Node 10,                                !- Component Inlet Node Name 1",
        "  Node 11;                                !- Component Outlet Node Name 1",

        "Generator:FuelCell:ExhaustGasToWaterHeatExchanger,",
        "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1, !- Name",
        "  Node 10,                                !- Heat Recovery Water Inlet Node Name",
        "  Node 11,                                !- Heat Recovery Water Outlet Node Name",
        "  0.0004,                                 !- Heat Recovery Water Maximum Flow Rate {m3/s}",
        "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1 Exhaust Outlet Air Node, !- Exhaust Outlet Air Node Name",
        "  Condensing,                             !- Heat Exchanger Calculation Method",
        "  ,                                       !- Method 1 Heat Exchanger Effectiveness",
        "  83.1,                                   !- Method 2 Parameter hxs0",
        "  4798,                                   !- Method 2 Parameter hxs1",
        "  -138000,                                !- Method 2 Parameter hxs2",
        "  -353800,                                !- Method 2 Parameter hxs3",
        "  515000000,                              !- Method 2 Parameter hxs4",
        "  ,                                       !- Method 3 h0Gas Coefficient",
        "  ,                                       !- Method 3 NdotGasRef Coefficient",
        "  ,                                       !- Method 3 n Coefficient",
        "  ,                                       !- Method 3 Gas Area {m2}",
        "  ,                                       !- Method 3 h0 Water Coefficient",
        "  ,                                       !- Method 3 N dot Water ref Coefficient",
        "  ,                                       !- Method 3 m Coefficient",
        "  ,                                       !- Method 3 Water Area {m2}",
        "  ,                                       !- Method 3 F Adjustment Factor",
        "  0.0031,                                 !- Method 4 hxl1 Coefficient",
        "  1,                                      !- Method 4 hxl2 Coefficient",
        "  35;                                     !- Method 4 Condensation Threshold {C}",

        "Generator:FuelCell,",
        "  Generator Fuel Cell 1,                  !- Name",
        "  Generator Fuel Cell Power Module 1,     !- Power Module Name",
        "  Generator Fuel Cell Air Supply 1,       !- Air Supply Name",
        "  NATURALGAS,                             !- Fuel Supply Name",
        "  Generator Fuel Cell Water Supply 1,     !- Water Supply Name",
        "  Generator Fuel Cell Auxiliary Heater 1, !- Auxiliary Heater Name",
        "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1, !- Heat Exchanger Name",
        "  Generator Fuel Cell Electrical Storage 1, !- Electrical Storage Name",
        "  Generator Fuel Cell Inverter 1;         !- Inverter Name",

        "Generator:FuelCell:PowerModule,",
        "  Generator Fuel Cell Power Module 1,     !- Name",
        "  Annex42,                                !- Efficiency Curve Mode",
        "  Power Module Efficiency Curve,          !- Efficiency Curve Name",
        "  1,                                      !- Nominal Efficiency",
        "  3400,                                   !- Nominal Electrical Power {W}",
        "  10,                                     !- Number of Stops at Start of Simulation",
        "  0,                                      !- Cycling Performance Degradation Coefficient",
        "  0,                                      !- Number of Run Hours at Beginning of Simulation {hr}",
        "  0,                                      !- Accumulated Run Time Degradation Coefficient",
        "  10000,                                  !- Run Time Degradation Initiation Time Threshold {hr}",
        "  1.4,                                    !- Power Up Transient Limit {W/s}",
        "  0.2,                                    !- Power Down Transient Limit {W/s}",
        "  0,                                      !- Start Up Time {s}",
        "  0.2,                                    !- Start Up Fuel {kmol}",
        "  0,                                      !- Start Up Electricity Consumption {J}",
        "  0,                                      !- Start Up Electricity Produced {J}",
        "  0,                                      !- Shut Down Time {s}",
        "  0.2,                                    !- Shut Down Fuel {kmol}",
        "  0,                                      !- Shut Down Electricity Consumption {J}",
        "  0,                                      !- Ancillary Electricity Constant Term",
        "  0,                                      !- Ancillary Electricity Linear Term",
        "  ConstantRate,                           !- Skin Loss Calculation Mode",
        "  Thermal Zone 1,                         !- Zone Name",
        "  0.6392,                                 !- Skin Loss Radiative Fraction",
        "  729,                                    !- Constant Skin Loss Rate {W}",
        "  0,                                      !- Skin Loss U-Factor Times Area Term {W/K}",
        "  Skin Loss Curve,                        !- Skin Loss Quadratic Curve Name",
        "  0.006156,                               !- Dilution Air Flow Rate {kmol/s}",
        "  2307,                                   !- Stack Heat loss to Dilution Air {W}",
        "  Generator Fuel Cell Power Module 1 OA Node, !- Dilution Inlet Air Node Name",
        "  Generator Fuel Cell Power Module 1 Dilution Outlet Air Node, !- Dilution Outlet Air Node Name",
        "  3010,                                   !- Minimum Operating Point {W}",
        "  3728;                                   !- Maximum Operating Point {W}",

        "OutdoorAir:NodeList,",
        "  Generator Fuel Cell Power Module 1 OA Node; !- Node or NodeList Name 1",

        "Generator:FuelCell:AirSupply,",
        "  Generator Fuel Cell Air Supply 1,       !- Name",
        "  Generator Fuel Cell Air Supply 1 OA Node, !- Air Inlet Node Name",
        "  Blower Power Curve,                     !- Blower Power Curve Name",
        "  1,                                      !- Blower Heat Loss Factor",
        "  AirRatiobyStoics,                       !- Air Supply Rate Calculation Mode",
        "  1,                                      !- Stoichiometric Ratio",
        "  Air Rate Function of Electric Power Curve, !- Air Rate Function of Electric Power Curve Name",
        "  0.00283,                                !- Air Rate Air Temperature Coefficient",
        "  ,                                       !- Air Rate Function of Fuel Rate Curve Name",
        "  NoRecovery,                             !- Air Intake Heat Recovery Mode",
        "  AmbientAir,                             !- Air Supply Constituent Mode",
        "  0;                                      !- Number of UserDefined Constituents",

        "OutdoorAir:NodeList,",
        "  Generator Fuel Cell Air Supply 1 OA Node; !- Node or NodeList Name 1",

        "Generator:FuelSupply,",
        "  NATURALGAS,                             !- Name",
        "  Scheduled,                              !- Fuel Temperature Modeling Mode",
        "  ,                                       !- Fuel Temperature Reference Node Name",
        "  Fuel Temperature,                       !- Fuel Temperature Schedule Name",
        "  Compressor Power Multiplier Function of FuelRate Curve, !- Compressor Power Multiplier Function of Fuel Rate Curve Name",
        "  1,                                      !- Compressor Heat Loss Factor",
        "  GaseousConstituents,                    !- Fuel Type",
        "  ,                                       !- Liquid Generic Fuel Lower Heating Value {kJ/kg}",
        "  ,                                       !- Liquid Generic Fuel Higher Heating Value {kJ/kg}",
        "  ,                                       !- Liquid Generic Fuel Molecular Weight {g/mol}",
        "  ,                                       !- Liquid Generic Fuel CO2 Emission Factor",
        "  8,                                      !- Number of Constituents in Gaseous Constituent Fuel Supply",
        "  METHANE,                                !- Constituent Name 1",
        "  0.949,                                  !- Constituent Molar Fraction 1",
        "  CarbonDioxide,                          !- Constituent Name 2",
        "  0.007,                                  !- Constituent Molar Fraction 2",
        "  NITROGEN,                               !- Constituent Name 3",
        "  0.016,                                  !- Constituent Molar Fraction 3",
        "  ETHANE,                                 !- Constituent Name 4",
        "  0.025,                                  !- Constituent Molar Fraction 4",
        "  PROPANE,                                !- Constituent Name 5",
        "  0.002,                                  !- Constituent Molar Fraction 5",
        "  BUTANE,                                 !- Constituent Name 6",
        "  0.0006,                                 !- Constituent Molar Fraction 6",
        "  PENTANE,                                !- Constituent Name 7",
        "  0.0002,                                 !- Constituent Molar Fraction 7",
        "  OXYGEN,                                 !- Constituent Name 8",
        "  0.0002;                                 !- Constituent Molar Fraction 8",

        "Generator:FuelCell:WaterSupply,",
        "  Generator Fuel Cell Water Supply 1,     !- Name",
        "  Reformer Water FlowRate Function of FuelRate Curve, !- Reformer Water Flow Rate Function of Fuel Rate Curve Name",
        "  Reformer Water Pump Power Function of FuelRate Curve, !- Reformer Water Pump Power Function of Fuel Rate Curve Name",
        "  0,                                      !- Pump Heat Loss Factor",
        "  TemperatureFromSchedule,                !- Water Temperature Modeling Mode",
        "  ,                                       !- Water Temperature Reference Node Name",
        "  Water Temperature;                      !- Water Temperature Schedule Name",

        "Generator:FuelCell:AuxiliaryHeater,",
        "  Generator Fuel Cell Auxiliary Heater 1, !- Name",
        "  0,                                      !- Excess Air Ratio",
        "  0,                                      !- Ancillary Power Constant Term",
        "  0,                                      !- Ancillary Power Linear Term",
        "  0.5,                                    !- Skin Loss U-Factor Times Area Value {W/K}",
        "  AirInletForFuelCell,                    !- Skin Loss Destination",
        "  ,                                       !- Zone Name to Receive Skin Losses",
        "  Watts,                                  !- Heating Capacity Units",
        "  0,                                      !- Maximum Heating Capacity in Watts {W}",
        "  0,                                      !- Minimum Heating Capacity in Watts {W}",
        "  0,                                      !- Maximum Heating Capacity in Kmol per Second {kmol/s}",
        "  0;                                      !- Minimum Heating Capacity in Kmol per Second {kmol/s}",

        "Generator:FuelCell:ElectricalStorage,",
        "  Generator Fuel Cell Electrical Storage 1, !- Name",
        "  SimpleEfficiencyWithConstraints,        !- Choice of Model",
        "  1,                                      !- Nominal Charging Energetic Efficiency",
        "  1,                                      !- Nominal Discharging Energetic Efficiency",
        "  0,                                      !- Simple Maximum Capacity {J}",
        "  0,                                      !- Simple Maximum Power Draw {W}",
        "  0,                                      !- Simple Maximum Power Store {W}",
        "  0;                                      !- Initial Charge State {J}",

        "Generator:FuelCell:Inverter,",
        "  Generator Fuel Cell Inverter 1,         !- Name",
        "  Constant,                               !- Inverter Efficiency Calculation Mode",
        "  1,                                      !- Inverter Efficiency",
        "  Efficiency Function of DC Power Curve;  !- Efficiency Function of DC Power Curve Name",

        "Curve:Cubic,",
        "  Blower Power Curve,                     !- Name",
        "  0,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 x**3",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Cubic,",
        "  Compressor Power Multiplier Function of FuelRate Curve, !- Name",
        "  0,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 x**3",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Cubic,",
        "  Reformer Water Pump Power Function of FuelRate Curve, !- Name",
        "  0,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 x**3",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Air Rate Function of Electric Power Curve, !- Name",
        "  0.00150976,                             !- Coefficient1 Constant",
        "  -7.76656e-07,                           !- Coefficient2 x",
        "  1.30317e-10,                            !- Coefficient3 x**2",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Air Rate Function of Fuel Rate Curve,   !- Name",
        "  0,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Efficiency Function of DC Power Curve,  !- Name",
        "  0.560717,                               !- Coefficient1 Constant",
        "  0.00012401,                             !- Coefficient2 x",
        "  -2.01648e-08,                           !- Coefficient3 x**2",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Power Module Efficiency Curve,          !- Name",
        "  0.642388,                               !- Coefficient1 Constant",
        "  -0.0001619,                             !- Coefficient2 x",
        "  2.26e-08,                               !- Coefficient3 x**2",
        "  0,                                      !- Minimum Value of x {BasedOnField A2}",
        "  10000;                                  !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Reformer Water FlowRate Function of FuelRate Curve, !- Name",
        "  0,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Skin Loss Curve,                        !- Name",
        "  0,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
        "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",

        "Branch,",
        "  SHW Loop Supply Outlet Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  SHW Loop Supply Outlet Pipe,            !- Component Name 1",
        "  SHW Loop Supply Outlet Pipe Node,       !- Component Inlet Node Name 1",
        "  Node 3;                                 !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  SHW Loop Supply Outlet Pipe,            !- Name",
        "  SHW Loop Supply Outlet Pipe Node,       !- Inlet Node Name",
        "  Node 3;                                 !- Outlet Node Name",

        "BranchList,",
        "  SHW Loop Demand Branches,               !- Name",
        "  SHW Loop Demand Inlet Branch,           !- Branch Name 1",
        "  SHW Loop Demand Branch 1,               !- Branch Name 2",
        "  SHW Loop Demand Bypass Branch,          !- Branch Name 3",
        "  SHW Loop Demand Outlet Branch;          !- Branch Name 4",

        "ConnectorList,",
        "  SHW Loop Demand Connector List,         !- Name",
        "  Connector:Splitter,                     !- Connector Object Type 1",
        "  SHW Loop Demand Splitter,               !- Connector Name 1",
        "  Connector:Mixer,                        !- Connector Object Type 2",
        "  SHW Loop Demand Mixer;                  !- Connector Name 2",

        "Connector:Splitter,",
        "  SHW Loop Demand Splitter,               !- Name",
        "  SHW Loop Demand Inlet Branch,           !- Inlet Branch Name",
        "  SHW Loop Demand Branch 1,               !- Outlet Branch Name 1",
        "  SHW Loop Demand Bypass Branch;          !- Outlet Branch Name 2",

        "Connector:Mixer,",
        "  SHW Loop Demand Mixer,                  !- Name",
        "  SHW Loop Demand Outlet Branch,          !- Outlet Branch Name",
        "  SHW Loop Demand Branch 1,               !- Inlet Branch Name 1",
        "  SHW Loop Demand Bypass Branch;          !- Inlet Branch Name 2",

        "Branch,",
        "  SHW Loop Demand Inlet Branch,           !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  SHW Loop Demand Inlet Pipe,             !- Component Name 1",
        "  Node 5,                                 !- Component Inlet Node Name 1",
        "  SHW Loop Demand Inlet Pipe Node;        !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  SHW Loop Demand Inlet Pipe,             !- Name",
        "  Node 5,                                 !- Inlet Node Name",
        "  SHW Loop Demand Inlet Pipe Node;        !- Outlet Node Name",

        "Branch,",
        "  SHW Loop Demand Branch 1,               !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  WaterUse:Connections,                   !- Component Object Type 1",
        "  Water Use Connections 1,                !- Component Name 1",
        "  Node 7,                                 !- Component Inlet Node Name 1",
        "  Node 12;                                !- Component Outlet Node Name 1",

        "WaterUse:Connections,",
        "  Water Use Connections 1,                !- Name",
        "  Node 7,                                 !- Inlet Node Name",
        "  Node 12,                                !- Outlet Node Name",
        "  ,                                       !- Supply Water Storage Tank Name",
        "  ,                                       !- Reclamation Water Storage Tank Name",
        "  ,                                       !- Hot Water Supply Temperature Schedule Name",
        "  ,                                       !- Cold Water Supply Temperature Schedule Name",
        "  None,                                   !- Drain Water Heat Exchanger Type",
        "  Plant,                                  !- Drain Water Heat Exchanger Destination",
        "  ,                                       !- Drain Water Heat Exchanger U-Factor Times Area {W/K}",
        "  Water Use Equipment 1;                  !- Water Use Equipment Name 1",

        "WaterUse:Equipment,",
        "  Water Use Equipment 1,                  !- Name",
        "  General,                                !- End-Use Subcategory",
        "  1.0,                                    !- Peak Flow Rate {m3/s}",
        "  Always On Discrete,                     !- Flow Rate Fraction Schedule Name",
        "  Hot_Water_Temperature;                  !- Target Temperature Schedule Name",

        "Branch,",
        "  SHW Loop Demand Bypass Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  SHW Loop Demand Bypass Pipe,            !- Component Name 1",
        "  SHW Loop Demand Bypass Pipe Inlet Node, !- Component Inlet Node Name 1",
        "  SHW Loop Demand Bypass Pipe Outlet Node; !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  SHW Loop Demand Bypass Pipe,            !- Name",
        "  SHW Loop Demand Bypass Pipe Inlet Node, !- Inlet Node Name",
        "  SHW Loop Demand Bypass Pipe Outlet Node; !- Outlet Node Name",

        "Branch,",
        "  SHW Loop Demand Outlet Branch,          !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Pipe:Adiabatic,                         !- Component Object Type 1",
        "  SHW Loop Demand Outlet Pipe,            !- Component Name 1",
        "  SHW Loop Demand Outlet Pipe Node,       !- Component Inlet Node Name 1",
        "  Node 6;                                 !- Component Outlet Node Name 1",

        "Pipe:Adiabatic,",
        "  SHW Loop Demand Outlet Pipe,            !- Name",
        "  SHW Loop Demand Outlet Pipe Node,       !- Inlet Node Name",
        "  Node 6;                                 !- Outlet Node Name",

        "PlantEquipmentOperationSchemes,",
        "  SHW Loop Operation Schemes,             !- Name",
        "  PlantEquipmentOperation:HeatingLoad,    !- Control Scheme Object Type 1",
        "  SHW Loop Heating Operation Scheme,      !- Control Scheme Name 1",
        "  Always On Discrete;                     !- Control Scheme Schedule Name 1",

        "PlantEquipmentOperation:HeatingLoad,",
        "  SHW Loop Heating Operation Scheme,      !- Name",
        "  0,                                      !- Load Range Lower Limit 1 {W}",
        "  1000000000,                             !- Load Range Upper Limit 1 {W}",
        "  SHW Loop Heating Equipment List;        !- Range Equipment List Name 1",

        "PlantEquipmentList,",
        "  SHW Loop Heating Equipment List,        !- Name",
        "  Generator:FuelCell:ExhaustGasToWaterHeatExchanger, !- Equipment Object Type 1",
        "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1; !- Equipment Name 1",

        "SetpointManager:Scheduled,",
        "  SHW LWT SPM,                            !- Name",
        "  Temperature,                            !- Control Variable",
        "  Hot_Water_Temperature,                  !- Schedule Name",
        "  Node 3;                                 !- Setpoint Node or NodeList Name",

        "ElectricLoadCenter:Distribution,",
        "  Electric Load Center Distribution 1,    !- Name",
        "  Electric Load Center Distribution 1 Generators, !- Generator List Name",
        "  Baseload,                               !- Generator Operation Scheme Type",
        "  ,                                       !- Generator Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "  ,                                       !- Generator Track Schedule Name Scheme Schedule Name",
        "  ,                                       !- Generator Track Meter Scheme Meter Name",
        "  AlternatingCurrent;                     !- Electrical Buss Type",

        "ElectricLoadCenter:Generators,",
        "  Electric Load Center Distribution 1 Generators, !- Name",
        "  Generator Fuel Cell 1,                  !- Generator Name 1",
        "  Generator:FuelCell,                     !- Generator Object Type 1",
        "  3400,                                   !- Generator Rated Electric Power Output 1 {W}",
        "  Always On Discrete,                     !- Generator Availability Schedule Name 1",
        "  ;                                       !- Generator Rated Thermal to Electrical Power Ratio 1",

        // Fake a load
        "Exterior:Lights,",
        "  Exterior Facade Lighting,!- Name",
        "  Always On Discrete,      !- Schedule Name",
        "  10000.00,                 !- Design Level {W}",
        "  ScheduleNameOnly,        !- Control Option",
        "  Exterior Facade Lighting;!- End-Use Subcategory",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    Yes,                     !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

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
        "    1.0;                     !- Sky Clearness",

        "Timestep,4;",

        // Obviously need to add the sqlite output since we query it...
        "Output:SQLite,",
        "  SimpleAndTabular;                       !- Option Type",

        // Need at least one meter for the query below to succeed
        "Output:Meter,",
        "  NaturalGas:Facility,                    !- Key Name",
        "  Timestep;                               !- Reporting Frequency",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());
    state->init_state(*state);

    SimulationManager::ManageSimulation(*state);
    EXPECT_TRUE(has_err_output(true));

    auto &generatorController = state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0];
    EXPECT_ENUM_EQ(GeneratorType::FuelCell, generatorController->generatorType);
    EXPECT_EQ("GENERATOR FUEL CELL 1", generatorController->name);
    EXPECT_EQ("GENERATOR:FUELCELL", generatorTypeNamesUC[(int)generatorController->generatorType]);

    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::Generator_FCExhaust, generatorController->compPlantType);

    // Note: plantInfoFound (and cogenLocation) are only set when mode is FollowThermal or FollowThermalLimitElectric
    // Here it's 'Baseload'
    // EXPECT_TRUE(generatorController->plantInfoFound);
    // EXPECT_EQ("PLANT LOOP 1", state->dataPlnt->PlantLoop(generatorController->cogenLocation.loopNum).Name);

    EXPECT_EQ("GENERATOR FUEL CELL EXHAUST GAS TO WATER HEAT EXCHANGER 1", generatorController->compPlantName);
    EXPECT_EQ(0, generatorController->generatorIndex);

    EXPECT_EQ("ALWAYS ON DISCRETE", generatorController->availSched->Name);

    EXPECT_EQ(0, generatorController->nominalThermElectRatio);

    EXPECT_TRUE(generatorController->onThisTimestep);

    // FC has 3400 of power per input, and operates in Baseload mode
    EXPECT_EQ(3400.0, generatorController->maxPowerOut);
    EXPECT_EQ(3400.0, generatorController->powerRequestThisTimestep);
    EXPECT_EQ(3400.0, generatorController->electProdRate);
    EXPECT_EQ(3400.0 * 15 * 60, generatorController->electricityProd); // Timestep = 4, so 15min
    EXPECT_EQ(0, generatorController->dCElectricityProd);
    EXPECT_EQ(0, generatorController->dCElectProdRate);

    auto thisFCcompPtr = FuelCellElectricGenerator::FCDataStruct::factory(*state, generatorController->name);
    auto thisFC = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFCcompPtr);
    // Power Module
    EXPECT_EQ("GENERATOR FUEL CELL POWER MODULE 1", thisFC->NameFCPM);
    auto fCPM = thisFC->FCPM;
    EXPECT_EQ(fCPM.Name, thisFC->NameFCPM);
    // Annex42 = Direct
    EXPECT_ENUM_EQ(DataGenerators::CurveMode::Direct, fCPM.EffMode);
    ASSERT_GT(fCPM.EffCurve->Num, 0);
    EXPECT_EQ("POWER MODULE EFFICIENCY CURVE", fCPM.EffCurve->Name);
    EXPECT_EQ(1.0, fCPM.NomEff);
    EXPECT_EQ(3400.0, fCPM.NomPel);
    // At beginning of simulation, 10. Then it's ALWAYS ON (baseload), so it doesn't cycle, so still should be 10
    EXPECT_EQ(10, fCPM.NumCycles);
    EXPECT_EQ(0.0, fCPM.CyclingDegradRat);
    EXPECT_EQ(1.0, fCPM.NomEff);
    EXPECT_EQ(3400.0, fCPM.NomPel);
    EXPECT_EQ(0.0, fCPM.CyclingDegradRat);
    EXPECT_EQ(0.0, fCPM.NumRunHours);
    EXPECT_EQ(0.0, fCPM.OperateDegradRat);
    EXPECT_EQ(10000.0, fCPM.ThreshRunHours);
    EXPECT_EQ(1.4, fCPM.UpTranLimit);
    EXPECT_EQ(0.2, fCPM.DownTranLimit);
    EXPECT_EQ(0.0, fCPM.StartUpTime);
    EXPECT_EQ(0.2, fCPM.StartUpFuel);
    EXPECT_EQ(0.0, fCPM.StartUpElectConsum);
    EXPECT_EQ(0.0, fCPM.StartUpElectProd);
    EXPECT_EQ(0.0, fCPM.ShutDownTime);
    EXPECT_EQ(0.2, fCPM.ShutDownFuel);
    EXPECT_EQ(0.0, fCPM.ShutDownElectConsum);
    EXPECT_EQ(0.0, fCPM.ANC0);
    EXPECT_EQ(0.0, fCPM.ANC1);

    EXPECT_ENUM_EQ(DataGenerators::SkinLoss::ConstantRate, fCPM.SkinLossMode);

    EXPECT_EQ("THERMAL ZONE 1", fCPM.ZoneName);
    EXPECT_EQ(1, fCPM.ZoneID);

    EXPECT_EQ(0.6392, fCPM.RadiativeFract);

    EXPECT_EQ(729, fCPM.QdotSkin);

    EXPECT_EQ(0.0, fCPM.UAskin);
    EXPECT_GT(fCPM.SkinLossCurve->Num, 0);
    EXPECT_EQ(0.006156, fCPM.NdotDilutionAir);
    EXPECT_EQ(2307, fCPM.StackHeatLossToDilution);
    EXPECT_EQ("GENERATOR FUEL CELL POWER MODULE 1 OA NODE", fCPM.DilutionInletNodeName);
    EXPECT_EQ("GENERATOR FUEL CELL POWER MODULE 1 DILUTION OUTLET AIR NODE", fCPM.DilutionExhaustNodeName);
    EXPECT_EQ(3010, fCPM.PelMin);
    EXPECT_EQ(3728, fCPM.PelMax);

    // Air Supply
    EXPECT_EQ("GENERATOR FUEL CELL AIR SUPPLY 1", thisFC->NameFCAirSup);
    auto airSup = thisFC->AirSup;
    EXPECT_EQ(airSup.Name, thisFC->NameFCAirSup);
    EXPECT_EQ("GENERATOR FUEL CELL AIR SUPPLY 1 OA NODE", airSup.NodeName);
    ASSERT_GT(airSup.BlowerPowerCurve->Num, 0);
    EXPECT_EQ("BLOWER POWER CURVE", airSup.BlowerPowerCurve->Name);
    EXPECT_EQ(1.0, airSup.BlowerHeatLossFactor);
    EXPECT_ENUM_EQ(DataGenerators::AirSupRateMode::ConstantStoicsAirRat, airSup.AirSupRateMode);

    // Note: as mentioned in the IO/ref, Stoics ratio is the input + 1.0
    EXPECT_EQ(2.0, airSup.Stoics);

    ASSERT_GT(airSup.AirFuncPelCurve->Num, 0);
    EXPECT_EQ("AIR RATE FUNCTION OF ELECTRIC POWER CURVE", airSup.AirFuncPelCurve->Name);
    EXPECT_EQ(0.00283, airSup.AirTempCoeff);
    EXPECT_EQ(nullptr, airSup.AirFuncNdotCurve);

    EXPECT_ENUM_EQ(DataGenerators::RecoverMode::NoRecoveryOnAirIntake, airSup.IntakeRecoveryMode);

    EXPECT_ENUM_EQ(DataGenerators::ConstituentMode::RegularAir, airSup.ConstituentMode);
    // Regular air has 5 constituents
    EXPECT_EQ(5, airSup.NumConstituents);

    // Fuel Supply
    EXPECT_EQ("NATURALGAS", thisFC->NameFCFuelSup);

    // Water Supply
    EXPECT_EQ("GENERATOR FUEL CELL WATER SUPPLY 1", thisFC->NameFCWaterSup);
    auto waterSup = thisFC->WaterSup;
    EXPECT_EQ(waterSup.Name, thisFC->NameFCWaterSup);

    ASSERT_GT(waterSup.WaterSupRateCurve->Num, 0);
    EXPECT_EQ("REFORMER WATER FLOWRATE FUNCTION OF FUELRATE CURVE", waterSup.WaterSupRateCurve->Name);

    ASSERT_GT(waterSup.PmpPowerCurve->Num, 0);
    EXPECT_EQ("REFORMER WATER PUMP POWER FUNCTION OF FUELRATE CURVE", waterSup.PmpPowerCurve->Name);

    EXPECT_EQ(0.0, waterSup.PmpPowerLossFactor);

    EXPECT_EQ(0, waterSup.NodeNum);

    EXPECT_ENUM_EQ(DataGenerators::WaterTempMode::Schedule, waterSup.waterTempMode);

    ASSERT_NE(waterSup.sched, nullptr);

    // Auxiliary Heater
    EXPECT_EQ("GENERATOR FUEL CELL AUXILIARY HEATER 1", thisFC->NameFCAuxilHeat);
    auto auxilHeat = thisFC->AuxilHeat;
    EXPECT_EQ(auxilHeat.Name, thisFC->NameFCAuxilHeat);
    EXPECT_EQ(0.0, auxilHeat.ExcessAirRAT);
    EXPECT_EQ(0.0, auxilHeat.ANC0);
    EXPECT_EQ(0.0, auxilHeat.ANC1);
    EXPECT_EQ(0.5, auxilHeat.UASkin);

    EXPECT_ENUM_EQ(DataGenerators::LossDestination::AirInletForFC, auxilHeat.SkinLossDestination);

    EXPECT_EQ(0, auxilHeat.ZoneID);
    EXPECT_TRUE(auxilHeat.ZoneName.empty());

    EXPECT_EQ(0.0, auxilHeat.MaxPowerW);
    EXPECT_EQ(0.0, auxilHeat.MinPowerW);
    EXPECT_EQ(0.0, auxilHeat.MaxPowerkmolperSec);
    EXPECT_EQ(0.0, auxilHeat.MinPowerkmolperSec);

    // Exhaust HX
    EXPECT_EQ("GENERATOR FUEL CELL EXHAUST GAS TO WATER HEAT EXCHANGER 1", thisFC->NameExhaustHX);
    auto exhaustHX = thisFC->ExhaustHX;
    EXPECT_EQ(exhaustHX.Name, thisFC->NameExhaustHX);

    EXPECT_EQ("NODE 10", exhaustHX.WaterInNodeName);
    EXPECT_GT(exhaustHX.WaterInNode, 0);

    EXPECT_EQ("NODE 11", exhaustHX.WaterOutNodeName);
    EXPECT_GT(exhaustHX.WaterOutNode, 0);

    EXPECT_EQ(0.0004, exhaustHX.WaterVolumeFlowMax);

    EXPECT_EQ("GENERATOR FUEL CELL EXHAUST GAS TO WATER HEAT EXCHANGER 1 EXHAUST OUTLET AIR NODE", exhaustHX.ExhaustOutNodeName);
    EXPECT_GT(exhaustHX.ExhaustOutNode, 0);

    EXPECT_ENUM_EQ(DataGenerators::ExhaustGasHX::Condensing, thisFC->ExhaustHX.HXmodelMode);

    EXPECT_EQ(83.1, exhaustHX.hxs0);
    EXPECT_EQ(4798.0, exhaustHX.hxs1);
    EXPECT_EQ(-138000.0, exhaustHX.hxs2);
    EXPECT_EQ(-353800.0, exhaustHX.hxs3);
    EXPECT_EQ(515000000.0, exhaustHX.hxs4);

    EXPECT_EQ(0.0, exhaustHX.HXEffect);
    EXPECT_EQ(0.0, exhaustHX.NdotGasRef);
    EXPECT_EQ(0.0, exhaustHX.nCoeff);
    EXPECT_EQ(0.0, exhaustHX.AreaGas);
    EXPECT_EQ(0.0, exhaustHX.h0Water);
    EXPECT_EQ(0.0, exhaustHX.NdotWaterRef);
    EXPECT_EQ(0.0, exhaustHX.mCoeff);
    EXPECT_EQ(0.0, exhaustHX.AreaWater);
    EXPECT_EQ(0.0, exhaustHX.Fadjust);
    EXPECT_EQ(0.0031, exhaustHX.l1Coeff);
    EXPECT_EQ(1.0, exhaustHX.l2Coeff);
    EXPECT_EQ(35.0, exhaustHX.CondensationThresholdTemp);

    // Electrical Storage
    EXPECT_EQ("GENERATOR FUEL CELL ELECTRICAL STORAGE 1", thisFC->NameElecStorage);
    auto elecStorage = thisFC->ElecStorage;
    EXPECT_EQ(elecStorage.Name, thisFC->NameElecStorage);
    EXPECT_ENUM_EQ(DataGenerators::ElectricalStorage::SimpleEffConstraints, elecStorage.StorageModelMode);
    EXPECT_EQ(1.0, elecStorage.EnergeticEfficCharge);
    EXPECT_EQ(1.0, elecStorage.EnergeticEfficDischarge);
    EXPECT_EQ(0.0, elecStorage.NominalEnergyCapacity);
    EXPECT_EQ(0.0, elecStorage.MaxPowerDraw);
    EXPECT_EQ(0.0, elecStorage.MaxPowerStore);
    EXPECT_EQ(0.0, elecStorage.StartingEnergyStored);

    // Inverter
    EXPECT_EQ("GENERATOR FUEL CELL INVERTER 1", thisFC->NameInverter);
    auto inverter = thisFC->Inverter;
    EXPECT_ENUM_EQ(DataGenerators::InverterEfficiencyMode::Constant, inverter.EffMode);
    EXPECT_EQ(1.0, inverter.ConstEff);
    ASSERT_GT(inverter.EffQuadraticCurve->Num, 0);
    EXPECT_EQ("EFFICIENCY FUNCTION OF DC POWER CURVE", inverter.EffQuadraticCurve->Name);

    // StackCooler: not included

    // other checks
    EXPECT_EQ(1, thisFC->CWPlantLoc.loopNum);

    auto report = thisFC->Report;
    EXPECT_EQ(exhaustHX.qHX, report.qHX);
    EXPECT_EQ(report.ACPowerGen, generatorController->electProdRate);
    EXPECT_EQ(report.ACEnergyGen, generatorController->electricityProd);

    EXPECT_TRUE(generatorController->electProdRate > 1.15 * generatorController->thermProdRate) << "Power to Heat Ratio appears too low";
    EXPECT_DOUBLE_EQ(exhaustHX.qHX, generatorController->thermProdRate);
    EXPECT_DOUBLE_EQ(generatorController->thermProdRate * 15 * 60, generatorController->thermalProd);
}

// This was marked with DISABLE_ and skipped for a long time.  I enabled it, and messed with it, but
// didn't figure out the issue, and want to move on.  It's odd...it runs locally, and if I log into the
// CI machine, it passes there as well.  But during Decent runs, it regularly fails with an SQLite
// error.  Anyway, commenting it out but would love to get it running again.
// Modified from previous GaseousConstituents fuel supply model to use LiquidGeneric fuel type
// TEST_F(EnergyPlusFixture, FuelCellTest_Zero_Cp_Fix)
// {
//     // state->clear_state();
//     std::string const idf_objects = delimited_string({
//
//         "Material,",
//         "  8 in.Concrete Block Basement Wall,      !- Name",
//         "  MediumRough,                            !- Roughness",
//         "  0.2032,                                 !- Thickness{ m }",
//         "  1.326,                                  !- Conductivity{ W / m - K }",
//         "  1841.99999999999,                       !- Density{ kg / m3 }",
//         "  911.999999999999,                       !- Specific Heat{ J / kg - K }",
//         "  0.9,                                    !- Thermal Absorptance",
//         "  0.7,                                    !- Solar Absorptance",
//         "  0.7;                                    !- Visible Absorptance",
//
//         "Construction,",
//         "   Typical,   !- Name",
//         "   8 in.Concrete Block Basement Wall;     !- Layer 1",
//
//         "Zone,",
//         "  Thermal Zone 1,                         !- Name",
//         "  0,                                      !- Direction of Relative North {deg}",
//         "  0,                                      !- X Origin {m}",
//         "  0,                                      !- Y Origin {m}",
//         "  0,                                      !- Z Origin {m}",
//         "  ,                                       !- Type",
//         "  1,                                      !- Multiplier",
//         "  ,                                       !- Ceiling Height {m}",
//         "  ,                                       !- Volume {m3}",
//         "  ,                                       !- Floor Area {m2}",
//         "  ,                                       !- Zone Inside Convection Algorithm",
//         "  ,                                       !- Zone Outside Convection Algorithm",
//         "  Yes;                                    !- Part of Total Floor Area",
//
//         "BuildingSurface:Detailed,",
//         "  Floor,                                  !- Name",
//         "  Floor,                                  !- Surface Type",
//         "  Typical,                                !- Construction Name",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "    ,                        !- Space Name",
//         "  Ground,                                 !- Outside Boundary Condition",
//         "  ,                                       !- Outside Boundary Condition Object",
//         "  NoSun,                                  !- Sun Exposure",
//         "  NoWind,                                 !- Wind Exposure",
//         "  ,                                       !- View Factor to Ground",
//         "  ,                                       !- Number of Vertices",
//         "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
//         "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
//         "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
//         "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",
//
//         "BuildingSurface:Detailed,",
//         "  Wall 1,                                 !- Name",
//         "  Wall,                                   !- Surface Type",
//         "  Typical,                                !- Construction Name",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "    ,                        !- Space Name",
//         "  Outdoors,                               !- Outside Boundary Condition",
//         "  ,                                       !- Outside Boundary Condition Object",
//         "  SunExposed,                             !- Sun Exposure",
//         "  WindExposed,                            !- Wind Exposure",
//         "  ,                                       !- View Factor to Ground",
//         "  ,                                       !- Number of Vertices",
//         "  0, 10, 3,                               !- X,Y,Z Vertex 1 {m}",
//         "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
//         "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
//         "  0, 0, 3;                                !- X,Y,Z Vertex 4 {m}",
//
//         "BuildingSurface:Detailed,",
//         "  Wall 2,                                 !- Name",
//         "  Wall,                                   !- Surface Type",
//         "  Typical,                                !- Construction Name",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "    ,                        !- Space Name",
//         "  Outdoors,                               !- Outside Boundary Condition",
//         "  ,                                       !- Outside Boundary Condition Object",
//         "  SunExposed,                             !- Sun Exposure",
//         "  WindExposed,                            !- Wind Exposure",
//         "  ,                                       !- View Factor to Ground",
//         "  ,                                       !- Number of Vertices",
//         "  10, 10, 3,                              !- X,Y,Z Vertex 1 {m}",
//         "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
//         "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
//         "  0, 10, 3;                               !- X,Y,Z Vertex 4 {m}",
//
//         "BuildingSurface:Detailed,",
//         "  Wall 3,                                 !- Name",
//         "  Wall,                                   !- Surface Type",
//         "  Typical,                                !- Construction Name",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "    ,                        !- Space Name",
//         "  Outdoors,                               !- Outside Boundary Condition",
//         "  ,                                       !- Outside Boundary Condition Object",
//         "  SunExposed,                             !- Sun Exposure",
//         "  WindExposed,                            !- Wind Exposure",
//         "  ,                                       !- View Factor to Ground",
//         "  ,                                       !- Number of Vertices",
//         "  10, 0, 3,                               !- X,Y,Z Vertex 1 {m}",
//         "  10, 0, 0,                               !- X,Y,Z Vertex 2 {m}",
//         "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
//         "  10, 10, 3;                              !- X,Y,Z Vertex 4 {m}",
//
//         "BuildingSurface:Detailed,",
//         "  Wall 4,                                 !- Name",
//         "  Wall,                                   !- Surface Type",
//         "  Typical,                                !- Construction Name",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "    ,                        !- Space Name",
//         "  Outdoors,                               !- Outside Boundary Condition",
//         "  ,                                       !- Outside Boundary Condition Object",
//         "  SunExposed,                             !- Sun Exposure",
//         "  WindExposed,                            !- Wind Exposure",
//         "  ,                                       !- View Factor to Ground",
//         "  ,                                       !- Number of Vertices",
//         "  0, 0, 3,                                !- X,Y,Z Vertex 1 {m}",
//         "  0, 0, 0,                                !- X,Y,Z Vertex 2 {m}",
//         "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
//         "  10, 0, 3;                               !- X,Y,Z Vertex 4 {m}",
//
//         "BuildingSurface:Detailed,",
//         "  Roof,                                   !- Name",
//         "  Roof,                                   !- Surface Type",
//         "  Typical,                                !- Construction Name",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "    ,                        !- Space Name",
//         "  Outdoors,                               !- Outside Boundary Condition",
//         "  ,                                       !- Outside Boundary Condition Object",
//         "  SunExposed,                             !- Sun Exposure",
//         "  WindExposed,                            !- Wind Exposure",
//         "  ,                                       !- View Factor to Ground",
//         "  ,                                       !- Number of Vertices",
//         "  10, 0, 3,                               !- X,Y,Z Vertex 1 {m}",
//         "  10, 10, 3,                              !- X,Y,Z Vertex 2 {m}",
//         "  0, 10, 3,                               !- X,Y,Z Vertex 3 {m}",
//         "  0, 0, 3;                                !- X,Y,Z Vertex 4 {m}",
//
//         "ScheduleTypeLimits,",
//         "  OnOff,                                  !- Name",
//         "  0,                                      !- Lower Limit Value {BasedOnField A3}",
//         "  1,                                      !- Upper Limit Value {BasedOnField A3}",
//         "  Discrete,                               !- Numeric Type",
//         "  availability;                           !- Unit Type",
//
//         "ScheduleTypeLimits,",
//         "  Temperature,                            !- Name",
//         "  0,                                      !- Lower Limit Value {BasedOnField A3}",
//         "  100,                                    !- Upper Limit Value {BasedOnField A3}",
//         "  Continuous,                             !- Numeric Type",
//         "  temperature;                            !- Unit Type",
//
//         "Schedule:Constant,",
//         "  Always On Discrete,                     !- Name",
//         "  OnOff,                                  !- Schedule Type Limits Name",
//         "  1;                                      !- Hourly Value",
//
//         "Schedule:Constant,",
//         "  Fuel Temperature,                       !- Name",
//         "  Temperature,                            !- Schedule Type Limits Name",
//         "  20;                                     !- Hourly Value",
//
//         "Schedule:Constant,",
//         "  Hot_Water_Temperature,                  !- Name",
//         "  Temperature,                            !- Schedule Type Limits Name",
//         "  55;                                     !- Hourly Value",
//
//         "Schedule:Constant,",
//         "  Water Temperature,                      !- Name",
//         "  Temperature,                            !- Schedule Type Limits Name",
//         "  20;                                     !- Hourly Value",
//
//         "Schedule:Constant,",
//         "  Ambient Temperature 22C,                !- Name",
//         "  Temperature,                            !- Schedule Type Limits Name",
//         "  22;                                     !- Hourly Value",
//
//         "OutdoorAir:Node,",
//         "  Model Outdoor Air Node;                 !- Name",
//
//         "PlantLoop,",
//         "  SHW Loop,                               !- Name",
//         "  Water,                                  !- Fluid Type",
//         "  ,                                       !- User Defined Fluid Type",
//         "  SHW Loop Operation Schemes,             !- Plant Equipment Operation Scheme Name",
//         "  Node 3,                                 !- Loop Temperature Setpoint Node Name",
//         "  100,                                    !- Maximum Loop Temperature {C}",
//         "  0,                                      !- Minimum Loop Temperature {C}",
//         "  Autosize,                               !- Maximum Loop Flow Rate {m3/s}",
//         "  0,                                      !- Minimum Loop Flow Rate {m3/s}",
//         "  Autocalculate,                          !- Plant Loop Volume {m3}",
//         "  Node 2,                                 !- Plant Side Inlet Node Name",
//         "  Node 3,                                 !- Plant Side Outlet Node Name",
//         "  SHW Loop Supply Branches,               !- Plant Side Branch List Name",
//         "  SHW Loop Supply Connector List,         !- Plant Side Connector List Name",
//         "  Node 5,                                 !- Demand Side Inlet Node Name",
//         "  Node 6,                                 !- Demand Side Outlet Node Name",
//         "  SHW Loop Demand Branches,               !- Demand Side Branch List Name",
//         "  SHW Loop Demand Connector List,         !- Demand Side Connector List Name",
//         "  Optimal,                                !- Load Distribution Scheme",
//         "  ,                                       !- Availability Manager List Name",
//         "  SingleSetpoint,                         !- Plant Loop Demand Calculation Scheme",
//         "  ;                                       !- Common Pipe Simulation",
//
//         "Sizing:Plant,",
//         "  SHW Loop,                               !- Plant or Condenser Loop Name",
//         "  Heating,                                !- Loop Type",
//         "  82,                                     !- Design Loop Exit Temperature {C}",
//         "  11,                                     !- Loop Design Temperature Difference {deltaC}",
//         "  NonCoincident,                          !- Sizing Option",
//         "  1,                                      !- Zone Timesteps in Averaging Window",
//         "  None;                                   !- Coincident Sizing Factor Mode",
//
//         "BranchList,",
//         "  SHW Loop Supply Branches,               !- Name",
//         "  SHW Loop Supply Inlet Branch,           !- Branch Name 1",
//         "  SHW Loop Supply Branch 2,               !- Branch Name 3",
//         "  SHW Loop Supply Outlet Branch;          !- Branch Name 4",
//
//         "ConnectorList,",
//         "  SHW Loop Supply Connector List,         !- Name",
//         "  Connector:Splitter,                     !- Connector Object Type 1",
//         "  SHW Loop Supply Splitter,               !- Connector Name 1",
//         "  Connector:Mixer,                        !- Connector Object Type 2",
//         "  SHW Loop Supply Mixer;                  !- Connector Name 2",
//
//         "Connector:Splitter,",
//         "  SHW Loop Supply Splitter,               !- Name",
//         "  SHW Loop Supply Inlet Branch,           !- Inlet Branch Name",
//         "  SHW Loop Supply Branch 2;               !- Outlet Branch Name 2",
//
//         "Connector:Mixer,",
//         "  SHW Loop Supply Mixer,                  !- Name",
//         "  SHW Loop Supply Outlet Branch,          !- Outlet Branch Name",
//         "  SHW Loop Supply Branch 2;               !- Inlet Branch Name 2",
//
//         "Branch,",
//         "  SHW Loop Supply Inlet Branch,           !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pump:ConstantSpeed,                     !- Component Object Type 1",
//         "  Pump Constant Speed 1,                  !- Component Name 1",
//         "  Node 2,                                 !- Component Inlet Node Name 1",
//         "  Node 8;                                 !- Component Outlet Node Name 1",
//
//         "Pump:ConstantSpeed,",
//         "  Pump Constant Speed 1,                  !- Name",
//         "  Node 2,                                 !- Inlet Node Name",
//         "  Node 8,                                 !- Outlet Node Name",
//         "  Autosize,                               !- Design Flow Rate {m3/s}",
//         "  179352,                                 !- Design Pump Head {Pa}",
//         "  Autosize,                               !- Design Power Consumption {W}",
//         "  0.9,                                    !- Motor Efficiency",
//         "  0,                                      !- Fraction of Motor Inefficiencies to Fluid Stream",
//         "  Intermittent,                           !- Pump Control Type",
//         "  ,                                       !- Pump Flow Rate Schedule Name",
//         "  ,                                       !- Pump Curve Name",
//         "  ,                                       !- Impeller Diameter {m}",
//         "  ,                                       !- Rotational Speed {rev/min}",
//         "  ,                                       !- Zone Name",
//         "  ,                                       !- Skin Loss Radiative Fraction",
//         "  PowerPerFlowPerPressure,                !- Design Power Sizing Method",
//         "  348701.1,                               !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
//         "  1.282051282,                            !- Design Shaft Power per Unit Flow Rate per Unit Head {W-s/m3-Pa}",
//         "  General;                                !- End-Use Subcategory",
//
//         "Branch,",
//         "  SHW Loop Supply Branch 2,           !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Generator:FuelCell:ExhaustGasToWaterHeatExchanger, !- Component Object Type 1",
//         "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1, !- Component Name 1",
//         "  Node 10,                                !- Component Inlet Node Name 1",
//         "  Node 11;                                !- Component Outlet Node Name 1",
//
//         "Generator:FuelCell:ExhaustGasToWaterHeatExchanger,",
//         "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1, !- Name",
//         "  Node 10,                                !- Heat Recovery Water Inlet Node Name",
//         "  Node 11,                                !- Heat Recovery Water Outlet Node Name",
//         "  0.0004,                                 !- Heat Recovery Water Maximum Flow Rate {m3/s}",
//         "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1 Exhaust Outlet Air Node, !- Exhaust Outlet Air Node Name",
//         "  Condensing,                             !- Heat Exchanger Calculation Method",
//         "  ,                                       !- Method 1 Heat Exchanger Effectiveness",
//         "  83.1,                                   !- Method 2 Parameter hxs0",
//         "  4798,                                   !- Method 2 Parameter hxs1",
//         "  -138000,                                !- Method 2 Parameter hxs2",
//         "  -353800,                                !- Method 2 Parameter hxs3",
//         "  515000000,                              !- Method 2 Parameter hxs4",
//         "  ,                                       !- Method 3 h0Gas Coefficient",
//         "  ,                                       !- Method 3 NdotGasRef Coefficient",
//         "  ,                                       !- Method 3 n Coefficient",
//         "  ,                                       !- Method 3 Gas Area {m2}",
//         "  ,                                       !- Method 3 h0 Water Coefficient",
//         "  ,                                       !- Method 3 N dot Water ref Coefficient",
//         "  ,                                       !- Method 3 m Coefficient",
//         "  ,                                       !- Method 3 Water Area {m2}",
//         "  ,                                       !- Method 3 F Adjustment Factor",
//         "  0.0031,                                 !- Method 4 hxl1 Coefficient",
//         "  1,                                      !- Method 4 hxl2 Coefficient",
//         "  35;                                     !- Method 4 Condensation Threshold {C}",
//
//         "Generator:FuelCell,",
//         "  Generator Fuel Cell 1,                  !- Name",
//         "  Generator Fuel Cell Power Module 1,     !- Power Module Name",
//         "  Generator Fuel Cell Air Supply 1,       !- Air Supply Name",
//         "  NATURALGAS,                             !- Fuel Supply Name",
//         "  Generator Fuel Cell Water Supply 1,     !- Water Supply Name",
//         "  Generator Fuel Cell Auxiliary Heater 1, !- Auxiliary Heater Name",
//         "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1, !- Heat Exchanger Name",
//         "  Generator Fuel Cell Electrical Storage 1, !- Electrical Storage Name",
//         "  Generator Fuel Cell Inverter 1;         !- Inverter Name",
//
//         "Generator:FuelCell:PowerModule,",
//         "  Generator Fuel Cell Power Module 1,     !- Name",
//         "  Annex42,                                !- Efficiency Curve Mode",
//         "  Power Module Efficiency Curve,          !- Efficiency Curve Name",
//         "  1,                                      !- Nominal Efficiency",
//         "  3400,                                   !- Nominal Electrical Power {W}",
//         "  10,                                     !- Number of Stops at Start of Simulation",
//         "  0,                                      !- Cycling Performance Degradation Coefficient",
//         "  0,                                      !- Number of Run Hours at Beginning of Simulation {hr}",
//         "  0,                                      !- Accumulated Run Time Degradation Coefficient",
//         "  10000,                                  !- Run Time Degradation Initiation Time Threshold {hr}",
//         "  1.4,                                    !- Power Up Transient Limit {W/s}",
//         "  0.2,                                    !- Power Down Transient Limit {W/s}",
//         "  0,                                      !- Start Up Time {s}",
//         "  0.2,                                    !- Start Up Fuel {kmol}",
//         "  0,                                      !- Start Up Electricity Consumption {J}",
//         "  0,                                      !- Start Up Electricity Produced {J}",
//         "  0,                                      !- Shut Down Time {s}",
//         "  0.2,                                    !- Shut Down Fuel {kmol}",
//         "  0,                                      !- Shut Down Electricity Consumption {J}",
//         "  0,                                      !- Ancillary Electricity Constant Term",
//         "  0,                                      !- Ancillary Electricity Linear Term",
//         "  ConstantRate,                           !- Skin Loss Calculation Mode",
//         "  Thermal Zone 1,                         !- Zone Name",
//         "  0.6392,                                 !- Skin Loss Radiative Fraction",
//         "  729,                                    !- Constant Skin Loss Rate {W}",
//         "  0,                                      !- Skin Loss U-Factor Times Area Term {W/K}",
//         "  Skin Loss Curve,                        !- Skin Loss Quadratic Curve Name",
//         "  0.006156,                               !- Dilution Air Flow Rate {kmol/s}",
//         "  2307,                                   !- Stack Heat loss to Dilution Air {W}",
//         "  Generator Fuel Cell Power Module 1 OA Node, !- Dilution Inlet Air Node Name",
//         "  Generator Fuel Cell Power Module 1 Dilution Outlet Air Node, !- Dilution Outlet Air Node Name",
//         "  3010,                                   !- Minimum Operating Point {W}",
//         "  3728;                                   !- Maximum Operating Point {W}",
//
//         "OutdoorAir:NodeList,",
//         "  Generator Fuel Cell Power Module 1 OA Node; !- Node or NodeList Name 1",
//
//         "Generator:FuelCell:AirSupply,",
//         "  Generator Fuel Cell Air Supply 1,       !- Name",
//         "  Generator Fuel Cell Air Supply 1 OA Node, !- Air Inlet Node Name",
//         "  Blower Power Curve,                     !- Blower Power Curve Name",
//         "  1,                                      !- Blower Heat Loss Factor",
//         "  QuadraticFunctionofElectricPower,                       !- Air Supply Rate Calculation Mode",
//         "  1,                                      !- Stoichiometric Ratio",
//         "  Air Rate Function of Electric Power Curve, !- Air Rate Function of Electric Power Curve Name",
//         "  0.00283,                                !- Air Rate Air Temperature Coefficient",
//         "  ,                                       !- Air Rate Function of Fuel Rate Curve Name",
//         "  NoRecovery,                             !- Air Intake Heat Recovery Mode",
//         "  AmbientAir,                             !- Air Supply Constituent Mode",
//         "  0;                                      !- Number of UserDefined Constituents",
//
//         "OutdoorAir:NodeList,",
//         "  Generator Fuel Cell Air Supply 1 OA Node; !- Node or NodeList Name 1",
//
//         "Generator:FuelSupply,",
//         "  NATURALGAS,                             !- Name",
//         "  Scheduled,                              !- Fuel Temperature Modeling Mode",
//         "  ,                                       !- Fuel Temperature Reference Node Name",
//         "  Fuel Temperature,                       !- Fuel Temperature Schedule Name",
//         "  Compressor Power Multiplier Function of FuelRate Curve, !- Compressor Power Multiplier Function of Fuel Rate Curve Name",
//         "  1,                                      !- Compressor Heat Loss Factor",
//         "  LiquidGeneric,                          !- Fuel Type",
//         "  43100,                                  !- Liquid Generic Fuel Lower Heating Value {kJ/kg}",
//         "  46200,                                  !- Liquid Generic Fuel Higher Heating Value {kJ/kg}",
//         "  200,                                    !- Liquid Generic Fuel Molecular Weight {g/mol}",
//         "  ,                                       !- Liquid Generic Fuel CO2 Emission Factor",
//         "  0;                                      !- Number of Constituents in Gaseous Constituent Fuel Supply",
//         "! METHANE,                                !- Constituent Name 1",
//         "! 0.949,                                  !- Constituent Molar Fraction 1",
//         "! CarbonDioxide,                          !- Constituent Name 2",
//         "! 0.007,                                  !- Constituent Molar Fraction 2",
//         "! NITROGEN,                               !- Constituent Name 3",
//         "! 0.016,                                  !- Constituent Molar Fraction 3",
//         "! ETHANE,                                 !- Constituent Name 4",
//         "! 0.025,                                  !- Constituent Molar Fraction 4",
//         "! PROPANE,                                !- Constituent Name 5",
//         "! 0.002,                                  !- Constituent Molar Fraction 5",
//         "! BUTANE,                                 !- Constituent Name 6",
//         "! 0.0006,                                 !- Constituent Molar Fraction 6",
//         "! PENTANE,                                !- Constituent Name 7",
//         "! 0.0002,                                 !- Constituent Molar Fraction 7",
//         "! OXYGEN,                                 !- Constituent Name 8",
//         "! 0.0002;                                 !- Constituent Molar Fraction 8",
//
//         "Generator:FuelCell:WaterSupply,",
//         "  Generator Fuel Cell Water Supply 1,     !- Name",
//         "  Reformer Water FlowRate Function of FuelRate Curve, !- Reformer Water Flow Rate Function of Fuel Rate Curve Name",
//         "  Reformer Water Pump Power Function of FuelRate Curve, !- Reformer Water Pump Power Function of Fuel Rate Curve Name",
//         "  0,                                      !- Pump Heat Loss Factor",
//         "  TemperatureFromSchedule,                !- Water Temperature Modeling Mode",
//         "  ,                                       !- Water Temperature Reference Node Name",
//         "  Water Temperature;                      !- Water Temperature Schedule Name",
//
//         "Generator:FuelCell:AuxiliaryHeater,",
//         "  Generator Fuel Cell Auxiliary Heater 1, !- Name",
//         "  0,                                      !- Excess Air Ratio",
//         "  0,                                      !- Ancillary Power Constant Term",
//         "  0,                                      !- Ancillary Power Linear Term",
//         "  0.5,                                    !- Skin Loss U-Factor Times Area Value {W/K}",
//         "  AirInletForFuelCell,                    !- Skin Loss Destination",
//         "  ,                                       !- Zone Name to Receive Skin Losses",
//         "  Watts,                                  !- Heating Capacity Units",
//         "  0,                                      !- Maximum Heating Capacity in Watts {W}",
//         "  0,                                      !- Minimum Heating Capacity in Watts {W}",
//         "  0,                                      !- Maximum Heating Capacity in Kmol per Second {kmol/s}",
//         "  0;                                      !- Minimum Heating Capacity in Kmol per Second {kmol/s}",
//
//         "Generator:FuelCell:ElectricalStorage,",
//         "  Generator Fuel Cell Electrical Storage 1, !- Name",
//         "  SimpleEfficiencyWithConstraints,        !- Choice of Model",
//         "  1,                                      !- Nominal Charging Energetic Efficiency",
//         "  1,                                      !- Nominal Discharging Energetic Efficiency",
//         "  0,                                      !- Simple Maximum Capacity {J}",
//         "  0,                                      !- Simple Maximum Power Draw {W}",
//         "  0,                                      !- Simple Maximum Power Store {W}",
//         "  0;                                      !- Initial Charge State {J}",
//
//         "Generator:FuelCell:Inverter,",
//         "  Generator Fuel Cell Inverter 1,         !- Name",
//         "  Constant,                               !- Inverter Efficiency Calculation Mode",
//         "  1,                                      !- Inverter Efficiency",
//         "  Efficiency Function of DC Power Curve;  !- Efficiency Function of DC Power Curve Name",
//
//         "Curve:Cubic,",
//         "  Blower Power Curve,                     !- Name",
//         "  0,                                      !- Coefficient1 Constant",
//         "  0,                                      !- Coefficient2 x",
//         "  0,                                      !- Coefficient3 x**2",
//         "  0,                                      !- Coefficient4 x**3",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Cubic,",
//         "  Compressor Power Multiplier Function of FuelRate Curve, !- Name",
//         "  0,                                      !- Coefficient1 Constant",
//         "  0,                                      !- Coefficient2 x",
//         "  0,                                      !- Coefficient3 x**2",
//         "  0,                                      !- Coefficient4 x**3",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Cubic,",
//         "  Reformer Water Pump Power Function of FuelRate Curve, !- Name",
//         "  0,                                      !- Coefficient1 Constant",
//         "  0,                                      !- Coefficient2 x",
//         "  0,                                      !- Coefficient3 x**2",
//         "  0,                                      !- Coefficient4 x**3",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Quadratic,",
//         "  Air Rate Function of Electric Power Curve, !- Name",
//         "  0.00150976,                             !- Coefficient1 Constant",
//         "  -7.76656e-07,                           !- Coefficient2 x",
//         "  1.30317e-10,                            !- Coefficient3 x**2",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Quadratic,",
//         "  Air Rate Function of Fuel Rate Curve,   !- Name",
//         "  0,                                      !- Coefficient1 Constant",
//         "  0,                                      !- Coefficient2 x",
//         "  0,                                      !- Coefficient3 x**2",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Quadratic,",
//         "  Efficiency Function of DC Power Curve,  !- Name",
//         "  0.560717,                               !- Coefficient1 Constant",
//         "  0.00012401,                             !- Coefficient2 x",
//         "  -2.01648e-08,                           !- Coefficient3 x**2",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Quadratic,",
//         "  Power Module Efficiency Curve,          !- Name",
//         "  0.642388,                               !- Coefficient1 Constant",
//         "  -0.0001619,                             !- Coefficient2 x",
//         "  2.26e-08,                               !- Coefficient3 x**2",
//         "  0,                                      !- Minimum Value of x {BasedOnField A2}",
//         "  10000;                                  !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Quadratic,",
//         "  Reformer Water FlowRate Function of FuelRate Curve, !- Name",
//         "  0,                                      !- Coefficient1 Constant",
//         "  0,                                      !- Coefficient2 x",
//         "  0,                                      !- Coefficient3 x**2",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Curve:Quadratic,",
//         "  Skin Loss Curve,                        !- Name",
//         "  0,                                      !- Coefficient1 Constant",
//         "  0,                                      !- Coefficient2 x",
//         "  0,                                      !- Coefficient3 x**2",
//         "  -10000000000,                           !- Minimum Value of x {BasedOnField A2}",
//         "  10000000000;                            !- Maximum Value of x {BasedOnField A2}",
//
//         "Branch,",
//         "  SHW Loop Supply Outlet Branch,          !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  SHW Loop Supply Outlet Pipe,            !- Component Name 1",
//         "  SHW Loop Supply Outlet Pipe Node,       !- Component Inlet Node Name 1",
//         "  Node 3;                                 !- Component Outlet Node Name 1",
//
//         "Pipe:Adiabatic,",
//         "  SHW Loop Supply Outlet Pipe,            !- Name",
//         "  SHW Loop Supply Outlet Pipe Node,       !- Inlet Node Name",
//         "  Node 3;                                 !- Outlet Node Name",
//
//         "BranchList,",
//         "  SHW Loop Demand Branches,               !- Name",
//         "  SHW Loop Demand Inlet Branch,           !- Branch Name 1",
//         "  SHW Loop Demand Branch 1,               !- Branch Name 2",
//         "  SHW Loop Demand Bypass Branch,          !- Branch Name 3",
//         "  SHW Loop Demand Outlet Branch;          !- Branch Name 4",
//
//         "ConnectorList,",
//         "  SHW Loop Demand Connector List,         !- Name",
//         "  Connector:Splitter,                     !- Connector Object Type 1",
//         "  SHW Loop Demand Splitter,               !- Connector Name 1",
//         "  Connector:Mixer,                        !- Connector Object Type 2",
//         "  SHW Loop Demand Mixer;                  !- Connector Name 2",
//
//         "Connector:Splitter,",
//         "  SHW Loop Demand Splitter,               !- Name",
//         "  SHW Loop Demand Inlet Branch,           !- Inlet Branch Name",
//         "  SHW Loop Demand Branch 1,               !- Outlet Branch Name 1",
//         "  SHW Loop Demand Bypass Branch;          !- Outlet Branch Name 2",
//
//         "Connector:Mixer,",
//         "  SHW Loop Demand Mixer,                  !- Name",
//         "  SHW Loop Demand Outlet Branch,          !- Outlet Branch Name",
//         "  SHW Loop Demand Branch 1,               !- Inlet Branch Name 1",
//         "  SHW Loop Demand Bypass Branch;          !- Inlet Branch Name 2",
//
//         "Branch,",
//         "  SHW Loop Demand Inlet Branch,           !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  SHW Loop Demand Inlet Pipe,             !- Component Name 1",
//         "  Node 5,                                 !- Component Inlet Node Name 1",
//         "  SHW Loop Demand Inlet Pipe Node;        !- Component Outlet Node Name 1",
//
//         "Pipe:Adiabatic,",
//         "  SHW Loop Demand Inlet Pipe,             !- Name",
//         "  Node 5,                                 !- Inlet Node Name",
//         "  SHW Loop Demand Inlet Pipe Node;        !- Outlet Node Name",
//
//         "Branch,",
//         "  SHW Loop Demand Branch 1,               !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  WaterUse:Connections,                   !- Component Object Type 1",
//         "  Water Use Connections 1,                !- Component Name 1",
//         "  Node 7,                                 !- Component Inlet Node Name 1",
//         "  Node 12;                                !- Component Outlet Node Name 1",
//
//         "WaterUse:Connections,",
//         "  Water Use Connections 1,                !- Name",
//         "  Node 7,                                 !- Inlet Node Name",
//         "  Node 12,                                !- Outlet Node Name",
//         "  ,                                       !- Supply Water Storage Tank Name",
//         "  ,                                       !- Reclamation Water Storage Tank Name",
//         "  ,                                       !- Hot Water Supply Temperature Schedule Name",
//         "  ,                                       !- Cold Water Supply Temperature Schedule Name",
//         "  None,                                   !- Drain Water Heat Exchanger Type",
//         "  Plant,                                  !- Drain Water Heat Exchanger Destination",
//         "  ,                                       !- Drain Water Heat Exchanger U-Factor Times Area {W/K}",
//         "  Water Use Equipment 1;                  !- Water Use Equipment Name 1",
//
//         "WaterUse:Equipment,",
//         "  Water Use Equipment 1,                  !- Name",
//         "  General,                                !- End-Use Subcategory",
//         "  1.0,                                    !- Peak Flow Rate {m3/s}",
//         "  Always On Discrete,                     !- Flow Rate Fraction Schedule Name",
//         "  Hot_Water_Temperature;                  !- Target Temperature Schedule Name",
//
//         "Branch,",
//         "  SHW Loop Demand Bypass Branch,          !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  SHW Loop Demand Bypass Pipe,            !- Component Name 1",
//         "  SHW Loop Demand Bypass Pipe Inlet Node, !- Component Inlet Node Name 1",
//         "  SHW Loop Demand Bypass Pipe Outlet Node; !- Component Outlet Node Name 1",
//
//         "Pipe:Adiabatic,",
//         "  SHW Loop Demand Bypass Pipe,            !- Name",
//         "  SHW Loop Demand Bypass Pipe Inlet Node, !- Inlet Node Name",
//         "  SHW Loop Demand Bypass Pipe Outlet Node; !- Outlet Node Name",
//
//         "Branch,",
//         "  SHW Loop Demand Outlet Branch,          !- Name",
//         "  ,                                       !- Pressure Drop Curve Name",
//         "  Pipe:Adiabatic,                         !- Component Object Type 1",
//         "  SHW Loop Demand Outlet Pipe,            !- Component Name 1",
//         "  SHW Loop Demand Outlet Pipe Node,       !- Component Inlet Node Name 1",
//         "  Node 6;                                 !- Component Outlet Node Name 1",
//
//         "Pipe:Adiabatic,",
//         "  SHW Loop Demand Outlet Pipe,            !- Name",
//         "  SHW Loop Demand Outlet Pipe Node,       !- Inlet Node Name",
//         "  Node 6;                                 !- Outlet Node Name",
//
//         "PlantEquipmentOperationSchemes,",
//         "  SHW Loop Operation Schemes,             !- Name",
//         "  PlantEquipmentOperation:HeatingLoad,    !- Control Scheme Object Type 1",
//         "  SHW Loop Heating Operation Scheme,      !- Control Scheme Name 1",
//         "  Always On Discrete;                     !- Control Scheme Schedule Name 1",
//
//         "PlantEquipmentOperation:HeatingLoad,",
//         "  SHW Loop Heating Operation Scheme,      !- Name",
//         "  0,                                      !- Load Range Lower Limit 1 {W}",
//         "  1000000000,                             !- Load Range Upper Limit 1 {W}",
//         "  SHW Loop Heating Equipment List;        !- Range Equipment List Name 1",
//
//         "PlantEquipmentList,",
//         "  SHW Loop Heating Equipment List,        !- Name",
//         "  Generator:FuelCell:ExhaustGasToWaterHeatExchanger, !- Equipment Object Type 1",
//         "  Generator Fuel Cell Exhaust Gas To Water Heat Exchanger 1; !- Equipment Name 1",
//
//         "SetpointManager:Scheduled,",
//         "  SHW LWT SPM,                            !- Name",
//         "  Temperature,                            !- Control Variable",
//         "  Hot_Water_Temperature,                  !- Schedule Name",
//         "  Node 3;                                 !- Setpoint Node or NodeList Name",
//
//         "ElectricLoadCenter:Distribution,",
//         "  Electric Load Center Distribution 1,    !- Name",
//         "  Electric Load Center Distribution 1 Generators, !- Generator List Name",
//         "  Baseload,                               !- Generator Operation Scheme Type",
//         "  ,                                       !- Generator Demand Limit Scheme Purchased Electric Demand Limit {W}",
//         "  ,                                       !- Generator Track Schedule Name Scheme Schedule Name",
//         "  ,                                       !- Generator Track Meter Scheme Meter Name",
//         "  AlternatingCurrent;                     !- Electrical Buss Type",
//
//         "ElectricLoadCenter:Generators,",
//         "  Electric Load Center Distribution 1 Generators, !- Name",
//         "  Generator Fuel Cell 1,                  !- Generator Name 1",
//         "  Generator:FuelCell,                     !- Generator Object Type 1",
//         "  3400,                                   !- Generator Rated Electric Power Output 1 {W}",
//         "  Always On Discrete,                     !- Generator Availability Schedule Name 1",
//         "  ;                                       !- Generator Rated Thermal to Electrical Power Ratio 1",
//
//         // Fake a load
//         "Exterior:Lights,",
//         "  Exterior Facade Lighting,!- Name",
//         "  Always On Discrete,      !- Schedule Name",
//         "  10000.00,                 !- Design Level {W}",
//         "  ScheduleNameOnly,        !- Control Option",
//         "  Exterior Facade Lighting;!- End-Use Subcategory",
//
//         "  SimulationControl,",
//         "    No,                      !- Do Zone Sizing Calculation",
//         "    No,                      !- Do System Sizing Calculation",
//         "    Yes,                     !- Do Plant Sizing Calculation",
//         "    Yes,                     !- Run Simulation for Sizing Periods",
//         "    No;                      !- Run Simulation for Weather File Run Periods",
//
//         "  Site:Location,",
//         "    CHICAGO_IL_USA TMY2-94846,  !- Name",
//         "    41.78,                   !- Latitude {deg}",
//         "    -87.75,                  !- Longitude {deg}",
//         "    -6.00,                   !- Time Zone {hr}",
//         "    190.00;                  !- Elevation {m}",
//
//         "  SizingPeriod:DesignDay,",
//         "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
//         "    1,                       !- Month",
//         "    21,                      !- Day of Month",
//         "    WinterDesignDay,         !- Day Type",
//         "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
//         "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
//         "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
//         "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
//         "    Wetbulb,                 !- Humidity Condition Type",
//         "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
//         "    ,                        !- Humidity Condition Day Schedule Name",
//         "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
//         "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
//         "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
//         "    99063.,                  !- Barometric Pressure {Pa}",
//         "    4.9,                     !- Wind Speed {m/s}",
//         "    270,                     !- Wind Direction {deg}",
//         "    No,                      !- Rain Indicator",
//         "    No,                      !- Snow Indicator",
//         "    No,                      !- Daylight Saving Time Indicator",
//         "    ASHRAEClearSky,          !- Solar Model Indicator",
//         "    ,                        !- Beam Solar Day Schedule Name",
//         "    ,                        !- Diffuse Solar Day Schedule Name",
//         "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
//         "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
//         "    1.0;                     !- Sky Clearness",
//
//         "Timestep,4;",
//
//         // Obviously need to add the sqlite output since we query it...
//         "Output:SQLite,",
//         "  SimpleAndTabular;                       !- Option Type",
//
//         // Need at least one meter for the query below to succeed
//         "Output:Meter,",
//         "  NaturalGas:Facility,                    !- Key Name",
//         "  Timestep;                               !- Reporting Frequency",
//
//     });
//
//     ASSERT_TRUE(process_idf(idf_objects));
//     bool process_err(false);
//     process_err = has_err_output(true);
//     EXPECT_FALSE(process_err);
//
//     state->init_state(*state);
//
//     SimulationManager::ManageSimulation(*state);
//     bool simulation_err(false);
//     simulation_err = has_err_output(false);
//     EXPECT_TRUE(simulation_err);
//
//     auto &generatorController = state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0];
//     EXPECT_ENUM_EQ(GeneratorType::FuelCell, generatorController->generatorType);
//     EXPECT_EQ("GENERATOR FUEL CELL 1", generatorController->name);
//     EXPECT_EQ("GENERATOR:FUELCELL", GeneratorTypeNamesUC[static_cast<int>(generatorController->generatorType)]);
//
//     EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::Generator_FCExhaust, generatorController->compPlantType);
//
//     // Note: plantInfoFound (and cogenLocation) are only set when mode is FollowThermal or FollowThermalLimitElectric
//     // Here it's 'Baseload'
//     // EXPECT_TRUE(generatorController->plantInfoFound);
//     // EXPECT_EQ("PLANT LOOP 1", state->dataPlnt->PlantLoop(generatorController->cogenLocation.loopNum).Name);
//
//     EXPECT_EQ("GENERATOR FUEL CELL EXHAUST GAS TO WATER HEAT EXCHANGER 1", generatorController->compPlantName);
//     EXPECT_EQ(0, generatorController->generatorIndex);
//
//     EXPECT_EQ("ALWAYS ON DISCRETE", generatorController->availSched->Name);
//
//     EXPECT_EQ(0, generatorController->nominalThermElectRatio);
//
//     EXPECT_TRUE(generatorController->onThisTimestep);
//
//     // FC has 3400 of power per input, and operates in Baseload mode
//     EXPECT_EQ(3400.0, generatorController->maxPowerOut);
//     EXPECT_EQ(3400.0, generatorController->powerRequestThisTimestep);
//     EXPECT_EQ(3400.0, generatorController->electProdRate);
//     EXPECT_EQ(3400.0 * 15 * 60, generatorController->electricityProd); // Timestep = 4, so 15min
//     EXPECT_EQ(0, generatorController->dCElectricityProd);
//     EXPECT_EQ(0, generatorController->dCElectProdRate);
//
//     auto thisFCcompPtr = FuelCellElectricGenerator::FCDataStruct::factory(*state, generatorController->name);
//     auto thisFC = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFCcompPtr);
//     // Power Module
//     EXPECT_EQ("GENERATOR FUEL CELL POWER MODULE 1", thisFC->NameFCPM);
//     auto fCPM = thisFC->FCPM;
//     EXPECT_EQ(fCPM.Name, thisFC->NameFCPM);
//     // Annex42 = Direct
//     EXPECT_ENUM_EQ(DataGenerators::CurveMode::Direct, fCPM.EffMode);
//     ASSERT_GT(fCPM.EffCurveID, 0);
//     EXPECT_EQ("POWER MODULE EFFICIENCY CURVE", state->dataCurveManager->PerfCurve(fCPM.EffCurveID)->Name);
//     EXPECT_EQ(1.0, fCPM.NomEff);
//     EXPECT_EQ(3400.0, fCPM.NomPel);
//     // At beginning of simulation, 10. Then it's ALWAYS ON (baseload), so it doesn't cycle, so still should be 10
//     EXPECT_EQ(10, fCPM.NumCycles);
//     EXPECT_EQ(0.0, fCPM.CyclingDegradRat);
//     EXPECT_EQ(1.0, fCPM.NomEff);
//     EXPECT_EQ(3400.0, fCPM.NomPel);
//     EXPECT_EQ(0.0, fCPM.CyclingDegradRat);
//     EXPECT_EQ(0.0, fCPM.NumRunHours);
//     EXPECT_EQ(0.0, fCPM.OperateDegradRat);
//     EXPECT_EQ(10000.0, fCPM.ThreshRunHours);
//     EXPECT_EQ(1.4, fCPM.UpTranLimit);
//     EXPECT_EQ(0.2, fCPM.DownTranLimit);
//     EXPECT_EQ(0.0, fCPM.StartUpTime);
//     EXPECT_EQ(0.2, fCPM.StartUpFuel);
//     EXPECT_EQ(0.0, fCPM.StartUpElectConsum);
//     EXPECT_EQ(0.0, fCPM.StartUpElectProd);
//     EXPECT_EQ(0.0, fCPM.ShutDownTime);
//     EXPECT_EQ(0.2, fCPM.ShutDownFuel);
//     EXPECT_EQ(0.0, fCPM.ShutDownElectConsum);
//     EXPECT_EQ(0.0, fCPM.ANC0);
//     EXPECT_EQ(0.0, fCPM.ANC1);
//
//     EXPECT_ENUM_EQ(DataGenerators::SkinLoss::ConstantRate, fCPM.SkinLossMode);
//
//     EXPECT_EQ("THERMAL ZONE 1", fCPM.ZoneName);
//     EXPECT_EQ(1, fCPM.ZoneID);
//
//     EXPECT_EQ(0.6392, fCPM.RadiativeFract);
//
//     EXPECT_EQ(729, fCPM.QdotSkin);
//
//     EXPECT_EQ(0.0, fCPM.UAskin);
//     EXPECT_GT(fCPM.SkinLossCurveID, 0);
//     EXPECT_EQ(0.006156, fCPM.NdotDilutionAir);
//     EXPECT_EQ(2307, fCPM.StackHeatLossToDilution);
//     EXPECT_EQ("GENERATOR FUEL CELL POWER MODULE 1 OA NODE", fCPM.DilutionInletNodeName);
//     EXPECT_EQ("GENERATOR FUEL CELL POWER MODULE 1 DILUTION OUTLET AIR NODE", fCPM.DilutionExhaustNodeName);
//     EXPECT_EQ(3010, fCPM.PelMin);
//     EXPECT_EQ(3728, fCPM.PelMax);
//
//     // Air Supply
//     EXPECT_EQ("GENERATOR FUEL CELL AIR SUPPLY 1", thisFC->NameFCAirSup);
//     auto airSup = thisFC->AirSup;
//     EXPECT_EQ(airSup.Name, thisFC->NameFCAirSup);
//     EXPECT_EQ("GENERATOR FUEL CELL AIR SUPPLY 1 OA NODE", airSup.NodeName);
//     ASSERT_GT(airSup.BlowerPowerCurveID, 0);
//     EXPECT_EQ("BLOWER POWER CURVE", state->dataCurveManager->PerfCurve(airSup.BlowerPowerCurveID)->Name);
//     EXPECT_EQ(1.0, airSup.BlowerHeatLossFactor);
//     EXPECT_ENUM_EQ(DataGenerators::AirSupRateMode::QuadraticFuncofPel, airSup.AirSupRateMode);
//
//     // Note: as mentioned in the IO/ref, Stoics ratio is the input + 1.0
//     EXPECT_EQ(2.0, airSup.Stoics);
//
//     ASSERT_GT(airSup.AirFuncPelCurveID, 0);
//     EXPECT_EQ("AIR RATE FUNCTION OF ELECTRIC POWER CURVE", state->dataCurveManager->PerfCurve(airSup.AirFuncPelCurveID)->Name);
//     EXPECT_EQ(0.00283, airSup.AirTempCoeff);
//     EXPECT_EQ(0, airSup.AirFuncNdotCurveID);
//
//     EXPECT_ENUM_EQ(DataGenerators::RecoverMode::NoRecoveryOnAirIntake, airSup.IntakeRecoveryMode);
//
//     EXPECT_ENUM_EQ(DataGenerators::ConstituentMode::RegularAir, airSup.ConstituentMode);
//     // Regular air has 5 constituents
//     EXPECT_EQ(5, airSup.NumConstituents);
//
//     // Fuel Supply
//     EXPECT_EQ("NATURALGAS", thisFC->NameFCFuelSup);
//
//     // Water Supply
//     EXPECT_EQ("GENERATOR FUEL CELL WATER SUPPLY 1", thisFC->NameFCWaterSup);
//     auto waterSup = thisFC->WaterSup;
//     EXPECT_EQ(waterSup.Name, thisFC->NameFCWaterSup);
//
//     ASSERT_GT(waterSup.WaterSupRateCurveID, 0);
//     EXPECT_EQ("REFORMER WATER FLOWRATE FUNCTION OF FUELRATE CURVE", state->dataCurveManager->PerfCurve(waterSup.WaterSupRateCurveID)->Name);
//
//     ASSERT_GT(waterSup.PmpPowerCurveID, 0);
//     EXPECT_EQ("REFORMER WATER PUMP POWER FUNCTION OF FUELRATE CURVE", state->dataCurveManager->PerfCurve(waterSup.PmpPowerCurveID)->Name);
//
//     EXPECT_EQ(0.0, waterSup.PmpPowerLossFactor);
//
//     EXPECT_EQ(0, waterSup.NodeNum);
//
//     EXPECT_ENUM_EQ(DataGenerators::WaterTemperatureMode::WaterInReformSchedule, waterSup.WaterTempMode);
//
//     ASSERT_NE(waterSup.sched, nullptr);
//
//     // Auxiliary Heater
//     EXPECT_EQ("GENERATOR FUEL CELL AUXILIARY HEATER 1", thisFC->NameFCAuxilHeat);
//     auto auxilHeat = thisFC->AuxilHeat;
//     EXPECT_EQ(auxilHeat.Name, thisFC->NameFCAuxilHeat);
//     EXPECT_EQ(0.0, auxilHeat.ExcessAirRAT);
//     EXPECT_EQ(0.0, auxilHeat.ANC0);
//     EXPECT_EQ(0.0, auxilHeat.ANC1);
//     EXPECT_EQ(0.5, auxilHeat.UASkin);
//
//     EXPECT_ENUM_EQ(DataGenerators::LossDestination::AirInletForFC, auxilHeat.SkinLossDestination);
//
//     EXPECT_EQ(0, auxilHeat.ZoneID);
//     EXPECT_TRUE(auxilHeat.ZoneName.empty());
//
//     EXPECT_EQ(0.0, auxilHeat.MaxPowerW);
//     EXPECT_EQ(0.0, auxilHeat.MinPowerW);
//     EXPECT_EQ(0.0, auxilHeat.MaxPowerkmolperSec);
//     EXPECT_EQ(0.0, auxilHeat.MinPowerkmolperSec);
//
//     // Exhaust HX
//     EXPECT_EQ("GENERATOR FUEL CELL EXHAUST GAS TO WATER HEAT EXCHANGER 1", thisFC->NameExhaustHX);
//     auto exhaustHX = thisFC->ExhaustHX;
//     EXPECT_EQ(exhaustHX.Name, thisFC->NameExhaustHX);
//
//     EXPECT_EQ("NODE 10", exhaustHX.WaterInNodeName);
//     EXPECT_GT(exhaustHX.WaterInNode, 0);
//
//     EXPECT_EQ("NODE 11", exhaustHX.WaterOutNodeName);
//     EXPECT_GT(exhaustHX.WaterOutNode, 0);
//
//     EXPECT_EQ(0.0004, exhaustHX.WaterVolumeFlowMax);
//
//     EXPECT_EQ("GENERATOR FUEL CELL EXHAUST GAS TO WATER HEAT EXCHANGER 1 EXHAUST OUTLET AIR NODE", exhaustHX.ExhaustOutNodeName);
//     EXPECT_GT(exhaustHX.ExhaustOutNode, 0);
//
//     EXPECT_ENUM_EQ(DataGenerators::ExhaustGasHX::Condensing, thisFC->ExhaustHX.HXmodelMode);
//
//     EXPECT_EQ(83.1, exhaustHX.hxs0);
//     EXPECT_EQ(4798.0, exhaustHX.hxs1);
//     EXPECT_EQ(-138000.0, exhaustHX.hxs2);
//     EXPECT_EQ(-353800.0, exhaustHX.hxs3);
//     EXPECT_EQ(515000000.0, exhaustHX.hxs4);
//
//     EXPECT_EQ(0.0, exhaustHX.HXEffect);
//     EXPECT_EQ(0.0, exhaustHX.NdotGasRef);
//     EXPECT_EQ(0.0, exhaustHX.nCoeff);
//     EXPECT_EQ(0.0, exhaustHX.AreaGas);
//     EXPECT_EQ(0.0, exhaustHX.h0Water);
//     EXPECT_EQ(0.0, exhaustHX.NdotWaterRef);
//     EXPECT_EQ(0.0, exhaustHX.mCoeff);
//     EXPECT_EQ(0.0, exhaustHX.AreaWater);
//     EXPECT_EQ(0.0, exhaustHX.Fadjust);
//     EXPECT_EQ(0.0031, exhaustHX.l1Coeff);
//     EXPECT_EQ(1.0, exhaustHX.l2Coeff);
//     EXPECT_EQ(35.0, exhaustHX.CondensationThresholdTemp);
//
//     // Electrical Storage
//     EXPECT_EQ("GENERATOR FUEL CELL ELECTRICAL STORAGE 1", thisFC->NameElecStorage);
//     auto elecStorage = thisFC->ElecStorage;
//     EXPECT_EQ(elecStorage.Name, thisFC->NameElecStorage);
//     EXPECT_ENUM_EQ(DataGenerators::ElectricalStorage::SimpleEffConstraints, elecStorage.StorageModelMode);
//     EXPECT_EQ(1.0, elecStorage.EnergeticEfficCharge);
//     EXPECT_EQ(1.0, elecStorage.EnergeticEfficDischarge);
//     EXPECT_EQ(0.0, elecStorage.NominalEnergyCapacity);
//     EXPECT_EQ(0.0, elecStorage.MaxPowerDraw);
//     EXPECT_EQ(0.0, elecStorage.MaxPowerStore);
//     EXPECT_EQ(0.0, elecStorage.StartingEnergyStored);
//
//     // Inverter
//     EXPECT_EQ("GENERATOR FUEL CELL INVERTER 1", thisFC->NameInverter);
//     auto inverter = thisFC->Inverter;
//     EXPECT_ENUM_EQ(DataGenerators::InverterEfficiencyMode::Constant, inverter.EffMode);
//     EXPECT_EQ(1.0, inverter.ConstEff);
//     ASSERT_GT(inverter.EffQuadraticCurveID, 0);
//     EXPECT_EQ("EFFICIENCY FUNCTION OF DC POWER CURVE", state->dataCurveManager->PerfCurve(inverter.EffQuadraticCurveID)->Name);
//
//     // StackCooler: not included
//
//     // other checks
//     EXPECT_EQ(1, thisFC->CWPlantLoc.loopNum);
//
//     auto report = thisFC->Report;
//     EXPECT_EQ(exhaustHX.qHX, report.qHX);
//     EXPECT_EQ(report.ACPowerGen, generatorController->electProdRate);
//     EXPECT_EQ(report.ACEnergyGen, generatorController->electricityProd);
//
//     EXPECT_TRUE(generatorController->electProdRate > 1.15 * generatorController->thermProdRate) << "Power to Heat Ratio appears too low";
//     EXPECT_DOUBLE_EQ(exhaustHX.qHX, generatorController->thermProdRate);
//     EXPECT_DOUBLE_EQ(generatorController->thermProdRate * 15 * 60, generatorController->thermalProd);
// }
