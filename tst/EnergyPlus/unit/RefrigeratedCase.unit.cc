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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/DataZoneEquipment.hh>

// I should not have to import these, but I do, the headers don't declare them
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/ScheduleManager.hh>

#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/RefrigeratedCase.hh>

// Standard headers
#include <string>
#include <string_view>

using namespace EnergyPlus;

static std::string_view constexpr oneZoneBuildingWithIdealLoads = R"IDF(
ScheduleTypeLimits,
  Any Number;                             !- Name

Schedule:Constant,
  Always On,                              !- Name
  Any Number,                             !- Schedule Type Limits Name
  1;                                      !- Hourly Value

Schedule:Constant,
  Always Off,                             !- Name
  Any Number,                             !- Schedule Type Limits Name
  0;                                      !- Hourly Value

Zone,
  Zone_1,                                 !- Name
  ,                                       !- Direction of Relative North {deg}
  0,                                      !- X Origin {m}
  0,                                      !- Y Origin {m}
  0,                                      !- Z Origin {m}
  ,                                       !- Type
  1,                                      !- Multiplier
  ,                                       !- Ceiling Height {m}
  ,                                       !- Volume {m3}
  ,                                       !- Floor Area {m2}
  ,                                       !- Zone Inside Convection Algorithm
  ,                                       !- Zone Outside Convection Algorithm
  Yes;                                    !- Part of Total Floor Area

Space,
  Space_1,                                !- Name
  Zone_1,                                 !- Zone Name
  ,                                       !- Ceiling Height {m}
  ,                                       !- Volume {m3}
  ;                                       !- Floor Area {m2}

BuildingSurface:Detailed,
  Space_1 Floor,                          !- Name
  Floor,                                  !- Surface Type
  Construction 1,                         !- Construction Name
  Zone_1,                                 !- Zone Name
  Space_1,                                !- Space Name
  Ground,                                 !- Outside Boundary Condition
  ,                                       !- Outside Boundary Condition Object
  NoSun,                                  !- Sun Exposure
  NoWind,                                 !- Wind Exposure
  ,                                       !- View Factor to Ground
  ,                                       !- Number of Vertices
  10, 5, 0,                               !- X,Y,Z Vertex 1 {m}
  10, -5, 0,                              !- X,Y,Z Vertex 2 {m}
  -10, -5, 0,                             !- X,Y,Z Vertex 3 {m}
  -10, 5, 0;                              !- X,Y,Z Vertex 4 {m}

BuildingSurface:Detailed,
  Space_1 RoofCeiling,                    !- Name
  Roof,                                   !- Surface Type
  Construction 1,                         !- Construction Name
  Zone_1,                                 !- Zone Name
  Space_1,                                !- Space Name
  Outdoors,                               !- Outside Boundary Condition
  ,                                       !- Outside Boundary Condition Object
  SunExposed,                             !- Sun Exposure
  WindExposed,                            !- Wind Exposure
  ,                                       !- View Factor to Ground
  ,                                       !- Number of Vertices
  -10, 5, 3,                              !- X,Y,Z Vertex 1 {m}
  -10, -5, 3,                             !- X,Y,Z Vertex 2 {m}
  10, -5, 3,                              !- X,Y,Z Vertex 3 {m}
  10, 5, 3;                               !- X,Y,Z Vertex 4 {m}

BuildingSurface:Detailed,
  Space_1 Wall 1,                         !- Name
  Wall,                                   !- Surface Type
  Construction 1,                         !- Construction Name
  Zone_1,                                 !- Zone Name
  Space_1,                                !- Space Name
  Outdoors,                               !- Outside Boundary Condition
  ,                                       !- Outside Boundary Condition Object
  SunExposed,                             !- Sun Exposure
  WindExposed,                            !- Wind Exposure
  ,                                       !- View Factor to Ground
  ,                                       !- Number of Vertices
  10, 5, 3,                               !- X,Y,Z Vertex 1 {m}
  10, -5, 3,                              !- X,Y,Z Vertex 2 {m}
  10, -5, 0,                              !- X,Y,Z Vertex 3 {m}
  10, 5, 0;                               !- X,Y,Z Vertex 4 {m}

BuildingSurface:Detailed,
  Space_1 Wall 2,                         !- Name
  Wall,                                   !- Surface Type
  Construction 1,                         !- Construction Name
  Zone_1,                                 !- Zone Name
  Space_1,                                !- Space Name
  Outdoors,                               !- Outside Boundary Condition
  ,                                       !- Outside Boundary Condition Object
  SunExposed,                             !- Sun Exposure
  WindExposed,                            !- Wind Exposure
  ,                                       !- View Factor to Ground
  ,                                       !- Number of Vertices
  10, -5, 3,                              !- X,Y,Z Vertex 1 {m}
  -10, -5, 3,                             !- X,Y,Z Vertex 2 {m}
  -10, -5, 0,                             !- X,Y,Z Vertex 3 {m}
  10, -5, 0;                              !- X,Y,Z Vertex 4 {m}

BuildingSurface:Detailed,
  Space_1 Wall 3,                         !- Name
  Wall,                                   !- Surface Type
  Construction 1,                         !- Construction Name
  Zone_1,                                 !- Zone Name
  Space_1,                                !- Space Name
  Outdoors,                               !- Outside Boundary Condition
  ,                                       !- Outside Boundary Condition Object
  SunExposed,                             !- Sun Exposure
  WindExposed,                            !- Wind Exposure
  ,                                       !- View Factor to Ground
  ,                                       !- Number of Vertices
  -10, -5, 3,                             !- X,Y,Z Vertex 1 {m}
  -10, 5, 3,                              !- X,Y,Z Vertex 2 {m}
  -10, 5, 0,                              !- X,Y,Z Vertex 3 {m}
  -10, -5, 0;                             !- X,Y,Z Vertex 4 {m}

BuildingSurface:Detailed,
  Space_1 Wall 4,                         !- Name
  Wall,                                   !- Surface Type
  Construction 1,                         !- Construction Name
  Zone_1,                                 !- Zone Name
  Space_1,                                !- Space Name
  Outdoors,                               !- Outside Boundary Condition
  ,                                       !- Outside Boundary Condition Object
  SunExposed,                             !- Sun Exposure
  WindExposed,                            !- Wind Exposure
  ,                                       !- View Factor to Ground
  ,                                       !- Number of Vertices
  -10, 5, 3,                              !- X,Y,Z Vertex 1 {m}
  10, 5, 3,                               !- X,Y,Z Vertex 2 {m}
  10, 5, 0,                               !- X,Y,Z Vertex 3 {m}
  -10, 5, 0;                              !- X,Y,Z Vertex 4 {m}

Material,
  Material 1,                             !- Name
  Smooth,                                 !- Roughness
  0.1,                                    !- Thickness {m}
  0.1,                                    !- Conductivity {W/m-K}
  0.1,                                    !- Density {kg/m3}
  1400,                                   !- Specific Heat {J/kg-K}
  0.9,                                    !- Thermal Absorptance
  0.7,                                    !- Solar Absorptance
  0.7;                                    !- Visible Absorptance

Construction,
  Construction 1,                         !- Name
  Material 1;                             !- Layer 1

ZoneHVAC:EquipmentConnections,
  Zone_1,                                 !- Zone Name
  Zone_1 Equipment List,                  !- Zone Conditioning Equipment List Name
  Zone_1 Inlet Node List,                 !- Zone Air Inlet Node or NodeList Name
  Zone_1 Exhaust Node List,               !- Zone Air Exhaust Node or NodeList Name
  Node 1;                                 !- Zone Air Node Name

NodeList,
  Zone_1 Inlet Node List,                 !- Name
  Node 3;                                 !- Node Name 1

NodeList,
  Zone_1 Exhaust Node List,               !- Name
  Node 2;                                 !- Node Name 1

ZoneHVAC:IdealLoadsAirSystem,
  Zone HVAC Ideal Loads Air System 1,     !- Name
  ,                                       !- Availability Schedule Name
  Node 3,                                 !- Zone Supply Air Node Name
  Node 2,                                 !- Zone Exhaust Air Node Name
  ,                                       !- System Inlet Air Node Name
  ,                                       !- Maximum Heating Supply Air Temperature {C}
  ,                                       !- Minimum Cooling Supply Air Temperature {C}
  ,                                       !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Heating Limit
  ,                                       !- Maximum Heating Air Flow Rate {m3/s}
  ,                                       !- Maximum Sensible Heating Capacity {W}
  ,                                       !- Cooling Limit
  ,                                       !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                       !- Maximum Total Cooling Capacity {W}
  ,                                       !- Heating Availability Schedule Name
  ,                                       !- Cooling Availability Schedule Name
  ,                                       !- Dehumidification Control Type
  ,                                       !- Cooling Sensible Heat Ratio {dimensionless}
  ,                                       !- Humidification Control Type
  ,                                       !- Design Specification Outdoor Air Object Name
  ,                                       !- Outdoor Air Inlet Node Name
  ,                                       !- Demand Controlled Ventilation Type
  ,                                       !- Outdoor Air Economizer Type
  ,                                       !- Heat Recovery Type
  ,                                       !- Sensible Heat Recovery Effectiveness {dimensionless}
  ;                                       !- Latent Heat Recovery Effectiveness {dimensionless}

ZoneHVAC:EquipmentList,
  Zone_1 Equipment List,                  !- Name
  SequentialLoad,                         !- Load Distribution Scheme
  ZoneHVAC:IdealLoadsAirSystem,           !- Zone Equipment Object Type 1
  Zone HVAC Ideal Loads Air System 1,     !- Zone Equipment Name 1
  1,                                      !- Zone Equipment Cooling Sequence 1
  1,                                      !- Zone Equipment Heating or No-Load Sequence 1
  ,                                       !- Zone Equipment Sequential Cooling Fraction Schedule Name 1
  ;                                       !- Zone Equipment Sequential Heating Fraction Schedule Name 1
)IDF";

TEST_F(EnergyPlusFixture, RefrigeratedRackWithCaseInZone)
{
    std::string_view constexpr idf_objects = R"IDF(
Refrigeration:CompressorRack,
  SelfContainedDisplay,    !- Name
  Zone,                    !- Heat Rejection Location
  4.0,                     !- Design Compressor Rack COP {W/W}
  RackCOPfTCurve2,         !- Compressor Rack COP Function of Temperature Curve Name
  175.0,                   !- Design Condenser Fan Power {W}
  ,                        !- Condenser Fan Power Function of Temperature Curve Name
  AirCooled,               !- Condenser Type
  ,                        !- Water-Cooled Condenser Inlet Node Name
  ,                        !- Water-Cooled Condenser Outlet Node Name
  ,                        !- Water-Cooled Loop Flow Type
  ,                        !- Water-Cooled Condenser Outlet Temperature Schedule Name
  ,                        !- Water-Cooled Condenser Design Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Water Outlet Temperature {C}
  ,                        !- Water-Cooled Condenser Minimum Water Inlet Temperature {C}
  ,                        !- Evaporative Condenser Availability Schedule Name
  ,                        !- Evaporative Condenser Effectiveness {dimensionless}
  ,                        !- Evaporative Condenser Air Flow Rate {m3/s}
  ,                        !- Basin Heater Capacity {W/K}
  ,                        !- Basin Heater Setpoint Temperature {C}
  ,                        !- Design Evaporative Condenser Water Pump Power {W}
  ,                        !- Evaporative Water Supply Tank Name
  ,                        !- Condenser Air Inlet Node Name
  ,                        !- End-Use Subcategory
  SelfContainedDisplayCase,!- Refrigeration Case Name or WalkIn Name or CaseAndWalkInList Name
  ZONE_1;                  !- Heat Rejection Zone Name


Curve:Quadratic,
  RackCOPfTCurve2,         !- Name
  1.0,                     !- Coefficient1 Constant
  0.0,                     !- Coefficient2 x
  0.0,                     !- Coefficient3 x**2
  0.0,                     !- Minimum Value of x
  50.0,                    !- Maximum Value of x
  ,                        !- Minimum Curve Output
  ,                        !- Maximum Curve Output
  Temperature,             !- Input Unit Type for X
  Dimensionless;           !- Output Unit Type

Refrigeration:Case,
  SelfContainedDisplayCase,!- Name
  ,                        !- Availability Schedule Name
  ZONE_1,                  !- Zone Name
  23.88,                   !- Rated Ambient Temperature {C}
  55.0,                    !- Rated Ambient Relative Humidity {percent}
  1000.0,                  !- Rated Total Cooling Capacity per Unit Length {W/m}
  0.08,                    !- Rated Latent Heat Ratio
  0.85,                    !- Rated Runtime Fraction
  10.0,                    !- Case Length {m}
  13.0,                    !- Case Operating Temperature {C}
  CaseTemperatureMethod,   !- Latent Case Credit Curve Type
  MultiShelfVertical_LatentEnergyMult,  !- Latent Case Credit Curve Name
  40.0,                    !- Standard Case Fan Power per Unit Length {W/m}
  40.0,                    !- Operating Case Fan Power per Unit Length {W/m}
  75.0,                    !- Standard Case Lighting Power per Unit Length {W/m}
  ,                        !- Installed Case Lighting Power per Unit Length {W/m}
  Always On,               !- Case Lighting Schedule Name
  0.9,                     !- Fraction of Lighting Energy to Case
  0.0,                     !- Case Anti-Sweat Heater Power per Unit Length {W/m}
  ,                        !- Minimum Anti-Sweat Heater Power per Unit Length {W/m}
  None,                    !- Anti-Sweat Heater Control Type
  0.0,                     !- Humidity at Zero Anti-Sweat Heater Energy {percent}
  0.0,                     !- Case Height {m}
  0.0,                     !- Fraction of Anti-Sweat Heater Energy to Case
  0.0,                     !- Case Defrost Power per Unit Length {W/m}
  None,                    !- Case Defrost Type
  ,                        !- Case Defrost Schedule Name
  ,                        !- Case Defrost Drip-Down Schedule Name
  ,                        !- Defrost Energy Correction Curve Type
  ,                        !- Defrost Energy Correction Curve Name
  0.0,                     !- Under Case HVAC Return Air Fraction
  SelfContainedCaseStockingSched;  !- Refrigerated Case Restocking Schedule Name

Curve:Cubic,
  MultiShelfVertical_LatentEnergyMult,  !- Name
  0.026526281,             !- Coefficient1 Constant
  0.001078032,             !- Coefficient2 x
  -0.0000602558,           !- Coefficient3 x**2
  0.00000123732,           !- Coefficient4 x**3
  -35.0,                   !- Minimum Value of x
  20.0;                    !- Maximum Value of x

Schedule:Compact,
  SelfContainedCaseStockingSched,  !- Name
  AnyNumber,               !- Schedule Type Limits Name
  Through: 12/31,          !- Field 1
  For: AllDays,            !- Field 2
  Until: 6:00,0.0,         !- Field 3
  Until: 7:00,50.0,        !- Field 5
  Until: 9:00,70.0,        !- Field 7
  Until: 10:00,80.0,       !- Field 9
  Until: 11:00,70.0,       !- Field 11
  Until: 13:00,50.0,       !- Field 13
  Until: 14:00,80.0,       !- Field 15
  Until: 15:00,90.0,       !- Field 17
  Until: 16:00,80.0,       !- Field 19
  Until: 24:00,0.0;        !- Field 21
)IDF";

    ASSERT_TRUE(process_idf(std::format("{}\n{}", oneZoneBuildingWithIdealLoads, idf_objects))); // read idf objects
    state->init_state(*state);

    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);

    // ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    InternalHeatGains::ManageInternalHeatGains(*state, true);
    RefrigeratedCase::ManageRefrigeratedCaseRacks(*state);
}

TEST_F(EnergyPlusFixture, RefrigeratedRackWithWalkInInZone)
{

    std::string_view constexpr idf_objects = R"IDF(
Refrigeration:CompressorRack,
  SelfContainedDisplay,    !- Name
  Zone,                    !- Heat Rejection Location
  4.0,                     !- Design Compressor Rack COP {W/W}
  RackCOPfTCurve2,         !- Compressor Rack COP Function of Temperature Curve Name
  175.0,                   !- Design Condenser Fan Power {W}
  ,                        !- Condenser Fan Power Function of Temperature Curve Name
  AirCooled,               !- Condenser Type
  ,                        !- Water-Cooled Condenser Inlet Node Name
  ,                        !- Water-Cooled Condenser Outlet Node Name
  ,                        !- Water-Cooled Loop Flow Type
  ,                        !- Water-Cooled Condenser Outlet Temperature Schedule Name
  ,                        !- Water-Cooled Condenser Design Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Water Outlet Temperature {C}
  ,                        !- Water-Cooled Condenser Minimum Water Inlet Temperature {C}
  ,                        !- Evaporative Condenser Availability Schedule Name
  ,                        !- Evaporative Condenser Effectiveness {dimensionless}
  ,                        !- Evaporative Condenser Air Flow Rate {m3/s}
  ,                        !- Basin Heater Capacity {W/K}
  ,                        !- Basin Heater Setpoint Temperature {C}
  ,                        !- Design Evaporative Condenser Water Pump Power {W}
  ,                        !- Evaporative Water Supply Tank Name
  ,                        !- Condenser Air Inlet Node Name
  ,                        !- End-Use Subcategory
  RefrigerationWalkIn,     !- Refrigeration Case Name or WalkIn Name or CaseAndWalkInList Name
  ZONE_1;                  !- Heat Rejection Zone Name


Curve:Quadratic,
  RackCOPfTCurve2,         !- Name
  1.0,                     !- Coefficient1 Constant
  0.0,                     !- Coefficient2 x
  0.0,                     !- Coefficient3 x**2
  0.0,                     !- Minimum Value of x
  50.0,                    !- Maximum Value of x
  ,                        !- Minimum Curve Output
  ,                        !- Maximum Curve Output
  Temperature,             !- Input Unit Type for X
  Dimensionless;           !- Output Unit Type

Refrigeration:WalkIn,
  RefrigerationWalkIn,     !- Name
  Always On,               !- Availability Schedule Name
  5,                       !- Rated Coil Cooling Capacity {W}
  1.66666666666667,        !- Operating Temperature {C}
  -6.11111111111111,       !- Rated Cooling Source Temperature {C}
  0.0,                     !- Rated Total Heating Power {W}
  Always On,               !- Heating Power Schedule Name
  5,                       !- Rated Cooling Coil Fan Power {W}
  0.0,                     !- Rated Circulation Fan Power {W}
  5,                       !- Rated Total Lighting Power {W}
  Always On,               !- Lighting Schedule Name
  Electric,                !- Defrost Type
  TimeSchedule,            !- Defrost Control Type
  Always Off,              !- Defrost Schedule Name
  ,                        !- Defrost Drip-Down Schedule Name
  0.001,                   !- Defrost Power {W}
  ,                        !- Temperature Termination Defrost Fraction to Ice {dimensionless}
  ,                        !- Restocking Schedule Name
  ,                        !- Average Refrigerant Charge Inventory {kg}
  1,                       !- Insulated Floor Surface Area {m2}
  0.17744571875,           !- Insulated Floor U-Value {W/m2-K}
  ZONE_1,                  !- Zone 1 Name
  6.504,                   !- Total Insulated Surface Area Facing Zone 1 {m2}
  0.17744571875,           !- Insulated Surface U-Value Facing Zone 1 {W/m2-K}
  ,                        !- Area of Glass Reach In Doors Facing Zone 1 {m2}
  ,                        !- Height of Glass Reach In Doors Facing Zone 1 {m}
  ,                        !- Glass Reach In Door U Value Facing Zone 1 {W/m2-K}
  ,                        !- Glass Reach In Door Opening Schedule Name Facing Zone 1
  1.216,                   !- Area of Stocking Doors Facing Zone 1 {m2}
  1.651,                   !- Height of Stocking Doors Facing Zone 1 {m}
  ,                        !- Stocking Door U Value Facing Zone 1 {W/m2-K}
  ,                        !- Stocking Door Opening Schedule Name Facing Zone 1
  None;                    !- Stocking Door Opening Protection Type Facing Zone 1
)IDF";

    ASSERT_TRUE(process_idf(std::format("{}\n{}", oneZoneBuildingWithIdealLoads, idf_objects))); // read idf objects
    state->init_state(*state);

    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    // ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    InternalHeatGains::ManageInternalHeatGains(*state, true);
    RefrigeratedCase::ManageRefrigeratedCaseRacks(*state);
}

TEST_F(EnergyPlusFixture, RefrigeratedRackWithWalkInInZone_CaseAndWalkinList)
{

    std::string_view constexpr idf_objects = R"IDF(
Refrigeration:CompressorRack,
  SelfContainedDisplay,    !- Name
  Zone,                    !- Heat Rejection Location
  4.0,                     !- Design Compressor Rack COP {W/W}
  RackCOPfTCurve2,         !- Compressor Rack COP Function of Temperature Curve Name
  175.0,                   !- Design Condenser Fan Power {W}
  ,                        !- Condenser Fan Power Function of Temperature Curve Name
  AirCooled,               !- Condenser Type
  ,                        !- Water-Cooled Condenser Inlet Node Name
  ,                        !- Water-Cooled Condenser Outlet Node Name
  ,                        !- Water-Cooled Loop Flow Type
  ,                        !- Water-Cooled Condenser Outlet Temperature Schedule Name
  ,                        !- Water-Cooled Condenser Design Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Water Outlet Temperature {C}
  ,                        !- Water-Cooled Condenser Minimum Water Inlet Temperature {C}
  ,                        !- Evaporative Condenser Availability Schedule Name
  ,                        !- Evaporative Condenser Effectiveness {dimensionless}
  ,                        !- Evaporative Condenser Air Flow Rate {m3/s}
  ,                        !- Basin Heater Capacity {W/K}
  ,                        !- Basin Heater Setpoint Temperature {C}
  ,                        !- Design Evaporative Condenser Water Pump Power {W}
  ,                        !- Evaporative Water Supply Tank Name
  ,                        !- Condenser Air Inlet Node Name
  ,                        !- End-Use Subcategory
  CompressorRack with Case and Walkin List,     !- Refrigeration Case Name or WalkIn Name or CaseAndWalkInList Name
  ZONE_1;                  !- Heat Rejection Zone Name

Curve:Quadratic,
  RackCOPfTCurve2,         !- Name
  1.0,                     !- Coefficient1 Constant
  0.0,                     !- Coefficient2 x
  0.0,                     !- Coefficient3 x**2
  0.0,                     !- Minimum Value of x
  50.0,                    !- Maximum Value of x
  ,                        !- Minimum Curve Output
  ,                        !- Maximum Curve Output
  Temperature,             !- Input Unit Type for X
  Dimensionless;           !- Output Unit Type

Refrigeration:CaseAndWalkInList,
  CompressorRack with Case and Walkin List, !- Name
  RefrigerationWalkIn;                      !- Case or WalkIn Name 1

Refrigeration:WalkIn,
  RefrigerationWalkIn,     !- Name
  Always On,               !- Availability Schedule Name
  5,                       !- Rated Coil Cooling Capacity {W}
  1.66666666666667,        !- Operating Temperature {C}
  -6.11111111111111,       !- Rated Cooling Source Temperature {C}
  0.0,                     !- Rated Total Heating Power {W}
  Always On,               !- Heating Power Schedule Name
  5,                       !- Rated Cooling Coil Fan Power {W}
  0.0,                     !- Rated Circulation Fan Power {W}
  5,                       !- Rated Total Lighting Power {W}
  Always On,               !- Lighting Schedule Name
  Electric,                !- Defrost Type
  TimeSchedule,            !- Defrost Control Type
  Always Off,              !- Defrost Schedule Name
  ,                        !- Defrost Drip-Down Schedule Name
  0.001,                   !- Defrost Power {W}
  ,                        !- Temperature Termination Defrost Fraction to Ice {dimensionless}
  ,                        !- Restocking Schedule Name
  ,                        !- Average Refrigerant Charge Inventory {kg}
  1,                       !- Insulated Floor Surface Area {m2}
  0.17744571875,           !- Insulated Floor U-Value {W/m2-K}
  ZONE_1,                  !- Zone 1 Name
  6.504,                   !- Total Insulated Surface Area Facing Zone 1 {m2}
  0.17744571875,           !- Insulated Surface U-Value Facing Zone 1 {W/m2-K}
  ,                        !- Area of Glass Reach In Doors Facing Zone 1 {m2}
  ,                        !- Height of Glass Reach In Doors Facing Zone 1 {m}
  ,                        !- Glass Reach In Door U Value Facing Zone 1 {W/m2-K}
  ,                        !- Glass Reach In Door Opening Schedule Name Facing Zone 1
  1.216,                   !- Area of Stocking Doors Facing Zone 1 {m2}
  1.651,                   !- Height of Stocking Doors Facing Zone 1 {m}
  ,                        !- Stocking Door U Value Facing Zone 1 {W/m2-K}
  ,                        !- Stocking Door Opening Schedule Name Facing Zone 1
  None;                    !- Stocking Door Opening Protection Type Facing Zone 1
)IDF";

    ASSERT_TRUE(process_idf(std::format("{}\n{}", oneZoneBuildingWithIdealLoads, idf_objects))); // read idf objects
    state->init_state(*state);

    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    // ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    InternalHeatGains::ManageInternalHeatGains(*state, true);
    RefrigeratedCase::ManageRefrigeratedCaseRacks(*state);
}

TEST_F(EnergyPlusFixture, RefrigeratedRackWithBothInZone_CaseAndWalkinList)
{

    std::string_view constexpr idf_objects = R"IDF(
Refrigeration:CompressorRack,
  SelfContainedDisplay,    !- Name
  Zone,                    !- Heat Rejection Location
  4.0,                     !- Design Compressor Rack COP {W/W}
  RackCOPfTCurve2,         !- Compressor Rack COP Function of Temperature Curve Name
  175.0,                   !- Design Condenser Fan Power {W}
  ,                        !- Condenser Fan Power Function of Temperature Curve Name
  AirCooled,               !- Condenser Type
  ,                        !- Water-Cooled Condenser Inlet Node Name
  ,                        !- Water-Cooled Condenser Outlet Node Name
  ,                        !- Water-Cooled Loop Flow Type
  ,                        !- Water-Cooled Condenser Outlet Temperature Schedule Name
  ,                        !- Water-Cooled Condenser Design Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Flow Rate {m3/s}
  ,                        !- Water-Cooled Condenser Maximum Water Outlet Temperature {C}
  ,                        !- Water-Cooled Condenser Minimum Water Inlet Temperature {C}
  ,                        !- Evaporative Condenser Availability Schedule Name
  ,                        !- Evaporative Condenser Effectiveness {dimensionless}
  ,                        !- Evaporative Condenser Air Flow Rate {m3/s}
  ,                        !- Basin Heater Capacity {W/K}
  ,                        !- Basin Heater Setpoint Temperature {C}
  ,                        !- Design Evaporative Condenser Water Pump Power {W}
  ,                        !- Evaporative Water Supply Tank Name
  ,                        !- Condenser Air Inlet Node Name
  ,                        !- End-Use Subcategory
  CompressorRack with Case and Walkin List,     !- Refrigeration Case Name or WalkIn Name or CaseAndWalkInList Name
  ZONE_1;                  !- Heat Rejection Zone Name

Curve:Quadratic,
  RackCOPfTCurve2,         !- Name
  1.0,                     !- Coefficient1 Constant
  0.0,                     !- Coefficient2 x
  0.0,                     !- Coefficient3 x**2
  0.0,                     !- Minimum Value of x
  50.0,                    !- Maximum Value of x
  ,                        !- Minimum Curve Output
  ,                        !- Maximum Curve Output
  Temperature,             !- Input Unit Type for X
  Dimensionless;           !- Output Unit Type

Refrigeration:CaseAndWalkInList,
  CompressorRack with Case and Walkin List, !- Name
  RefrigerationWalkIn,                      !- Case or WalkIn Name 1
  SelfContainedDisplayCase;                 !- Case or WalkIn Name 2

Refrigeration:WalkIn,
  RefrigerationWalkIn,     !- Name
  Always On,               !- Availability Schedule Name
  5,                       !- Rated Coil Cooling Capacity {W}
  1.66666666666667,        !- Operating Temperature {C}
  -6.11111111111111,       !- Rated Cooling Source Temperature {C}
  0.0,                     !- Rated Total Heating Power {W}
  Always On,               !- Heating Power Schedule Name
  5,                       !- Rated Cooling Coil Fan Power {W}
  0.0,                     !- Rated Circulation Fan Power {W}
  5,                       !- Rated Total Lighting Power {W}
  Always On,               !- Lighting Schedule Name
  Electric,                !- Defrost Type
  TimeSchedule,            !- Defrost Control Type
  Always Off,              !- Defrost Schedule Name
  ,                        !- Defrost Drip-Down Schedule Name
  0.001,                   !- Defrost Power {W}
  ,                        !- Temperature Termination Defrost Fraction to Ice {dimensionless}
  ,                        !- Restocking Schedule Name
  ,                        !- Average Refrigerant Charge Inventory {kg}
  1,                       !- Insulated Floor Surface Area {m2}
  0.17744571875,           !- Insulated Floor U-Value {W/m2-K}
  ZONE_1,                  !- Zone 1 Name
  6.504,                   !- Total Insulated Surface Area Facing Zone 1 {m2}
  0.17744571875,           !- Insulated Surface U-Value Facing Zone 1 {W/m2-K}
  ,                        !- Area of Glass Reach In Doors Facing Zone 1 {m2}
  ,                        !- Height of Glass Reach In Doors Facing Zone 1 {m}
  ,                        !- Glass Reach In Door U Value Facing Zone 1 {W/m2-K}
  ,                        !- Glass Reach In Door Opening Schedule Name Facing Zone 1
  1.216,                   !- Area of Stocking Doors Facing Zone 1 {m2}
  1.651,                   !- Height of Stocking Doors Facing Zone 1 {m}
  ,                        !- Stocking Door U Value Facing Zone 1 {W/m2-K}
  ,                        !- Stocking Door Opening Schedule Name Facing Zone 1
  None;                    !- Stocking Door Opening Protection Type Facing Zone 1

Refrigeration:Case,
  SelfContainedDisplayCase,!- Name
  ,                        !- Availability Schedule Name
  ZONE_1,                  !- Zone Name
  23.88,                   !- Rated Ambient Temperature {C}
  55.0,                    !- Rated Ambient Relative Humidity {percent}
  1000.0,                  !- Rated Total Cooling Capacity per Unit Length {W/m}
  0.08,                    !- Rated Latent Heat Ratio
  0.85,                    !- Rated Runtime Fraction
  10.0,                    !- Case Length {m}
  13.0,                    !- Case Operating Temperature {C}
  CaseTemperatureMethod,   !- Latent Case Credit Curve Type
  MultiShelfVertical_LatentEnergyMult,  !- Latent Case Credit Curve Name
  40.0,                    !- Standard Case Fan Power per Unit Length {W/m}
  40.0,                    !- Operating Case Fan Power per Unit Length {W/m}
  75.0,                    !- Standard Case Lighting Power per Unit Length {W/m}
  ,                        !- Installed Case Lighting Power per Unit Length {W/m}
  Always On,               !- Case Lighting Schedule Name
  0.9,                     !- Fraction of Lighting Energy to Case
  0.0,                     !- Case Anti-Sweat Heater Power per Unit Length {W/m}
  ,                        !- Minimum Anti-Sweat Heater Power per Unit Length {W/m}
  None,                    !- Anti-Sweat Heater Control Type
  0.0,                     !- Humidity at Zero Anti-Sweat Heater Energy {percent}
  0.0,                     !- Case Height {m}
  0.0,                     !- Fraction of Anti-Sweat Heater Energy to Case
  0.0,                     !- Case Defrost Power per Unit Length {W/m}
  None,                    !- Case Defrost Type
  ,                        !- Case Defrost Schedule Name
  ,                        !- Case Defrost Drip-Down Schedule Name
  ,                        !- Defrost Energy Correction Curve Type
  ,                        !- Defrost Energy Correction Curve Name
  0.0,                     !- Under Case HVAC Return Air Fraction
  SelfContainedCaseStockingSched;  !- Refrigerated Case Restocking Schedule Name

Curve:Cubic,
  MultiShelfVertical_LatentEnergyMult,  !- Name
  0.026526281,             !- Coefficient1 Constant
  0.001078032,             !- Coefficient2 x
  -0.0000602558,           !- Coefficient3 x**2
  0.00000123732,           !- Coefficient4 x**3
  -35.0,                   !- Minimum Value of x
  20.0;                    !- Maximum Value of x

Schedule:Compact,
  SelfContainedCaseStockingSched,  !- Name
  AnyNumber,               !- Schedule Type Limits Name
  Through: 12/31,          !- Field 1
  For: AllDays,            !- Field 2
  Until: 6:00,0.0,         !- Field 3
  Until: 7:00,50.0,        !- Field 5
  Until: 9:00,70.0,        !- Field 7
  Until: 10:00,80.0,       !- Field 9
  Until: 11:00,70.0,       !- Field 11
  Until: 13:00,50.0,       !- Field 13
  Until: 14:00,80.0,       !- Field 15
  Until: 15:00,90.0,       !- Field 17
  Until: 16:00,80.0,       !- Field 19
  Until: 24:00,0.0;        !- Field 21
)IDF";

    ASSERT_TRUE(process_idf(std::format("{}\n{}", oneZoneBuildingWithIdealLoads, idf_objects))); // read idf objects
    state->init_state(*state);

    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    // ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    InternalHeatGains::ManageInternalHeatGains(*state, true);
    RefrigeratedCase::ManageRefrigeratedCaseRacks(*state);
}

TEST_F(EnergyPlusFixture, DesuperheaterRefrigeration)
{
    std::string const idf_objects = delimited_string(
        {"  Zone,",
         "    Basic Zone,                                               !- Name",
         "    ,                                                         !- Direction of Relative North {deg}",
         "    0,                                                        !- X Origin {m}",
         "    0,                                                        !- Y Origin {m}",
         "    0,                                                        !- Z Origin {m}",
         "    ,                                                         !- Type",
         "    1,                                                        !- Multiplier",
         "    ,                                                         !- Ceiling Height {m}",
         "    ,                                                         !- Volume {m3}",
         "    ,                                                         !- Floor Area {m2}",
         "    ,                                                         !- Zone Inside Convection Algorithm",
         "    ,                                                         !- Zone Outside Convection Algorithm",
         "    Yes;                                                      !- Part of Total Floor Area",
         "  ZoneHVAC:EquipmentConnections,",
         "    Basic Zone,                                               !- Zone Name",
         "    Basic Zone Equipment List,                                !- Zone Conditioning Equipment List Name",
         "    Basic Zone Inlet Node,                                    !- Zone Air Inlet Node or NodeList Name",
         "    Basic Zone Exhaust Node,                                  !- Zone Air Exhaust Node or NodeList Name",
         "    Basic Zone Air Node;                                      !- Zone Air Node Name",
         "  ZoneHVAC:EquipmentList,",
         "    Basic Zone Equipment List,                                !- Name",
         "    SequentialLoad,                                           !- Load Distribution Scheme",
         "    ZoneHVAC:PackagedTerminalAirConditioner,                  !- Zone Equipment 1 Object Type",
         "    Basic Zone PTAC,                                          !- Zone Equipment 1 Name",
         "    1,                                                        !- Zone Equipment 1 Cooling Sequence",
         "    1,                                                        !- Zone Equipment 1 Heating or No-Load Sequence",
         "    ,                                                         !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
         "    ;                                                         !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
         "  ZoneHVAC:PackagedTerminalAirConditioner,",
         "    Basic Zone PTAC,                                          !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Basic Zone PTAC Inlet Node,                               !- Air Inlet Node Name",
         "    Basic Zone Inlet Node,                                    !- Air Outlet Node Name",
         "    OutdoorAir:Mixer,                                         !- Outdoor Air Mixer Object Type",
         "    Basic Zone PTAC OA Mixer,                                 !- Outdoor Air Mixer Name",
         "    Autosize,                                                 !- Cooling Supply Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Heating Supply Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- No Load Supply Air Flow Rate {m3/s}",
         "    No,                                                       !- No Load Supply Air Flow Rate Control Set To Low Speed",
         "    Autosize,                                                 !- Cooling Outdoor Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Heating Outdoor Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- No Load Outdoor Air Flow Rate {m3/s}",
         "    Fan:OnOff,                                                !- Supply Air Fan Object Type",
         "    Basic Zone PTAC Fan,                                      !- Supply Air Fan Name",
         "    Coil:Heating:Fuel,                                        !- Heating Coil Object Type",
         "    Basic Zone PTAC Heating Coil,                             !- Heating Coil Name",
         "    Coil:Cooling:DX:SingleSpeed,                              !- Cooling Coil Object Type",
         "    Basic Zone PTAC Cooling Coil,                             !- Cooling Coil Name",
         "    DrawThrough,                                              !- Fan Placement",
         "    ;                                                         !- Supply Air Fan Operating Mode Schedule Name",
         "  OutdoorAir:Mixer,",
         "    Basic Zone PTAC OA Mixer,                                 !- Name",
         "    Basic Zone PTAC Mixed Air Node,                           !- Mixed Air Node Name",
         "    Basic Zone PTAC OA Node,                                  !- Outdoor Air Stream Node Name",
         "    Basic Zone PTAC Relief Air Node,                          !- Relief Air Stream Node Name",
         "    Basic Zone PTAC Inlet Node;                               !- Return Air Stream Node Name",
         "  OutdoorAir:NodeList,",
         "    Basic Zone PTAC OA Node;                                  !- Node or NodeList Name 1",
         "  Fan:OnOff,",
         "    Basic Zone PTAC Fan,                                      !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.6,                                                      !- Fan Total Efficiency",
         "    250,                                                      !- Pressure Rise {Pa}",
         "    Autosize,                                                 !- Maximum Flow Rate {m3/s}",
         "    0.8,                                                      !- Motor Efficiency",
         "    1,                                                        !- Motor In Airstream Fraction",
         "    Basic Zone PTAC Heating Coil Outlet Node,                 !- Air Inlet Node Name",
         "    Basic Zone Inlet Node,                                    !- Air Outlet Node Name",
         "    Generic Curve,                                            !- Fan Power Ratio Function of Speed Ratio Curve Name",
         "    Generic Curve,                                            !- Fan Efficiency Ratio Function of Speed Ratio Curve Name",
         "    General;                                                  !- End-Use Subcategory",
         "  Coil:Heating:Fuel,",
         "    Basic Zone PTAC Heating Coil,                             !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    NaturalGas,                                               !- Fuel Type",
         "    0.8,                                                      !- Burner Efficiency",
         "    1,                                                        !- Nominal Capacity {W}",
         "    Basic Zone PTAC Cooling Coil Outlet Node,                 !- Air Inlet Node Name",
         "    Basic Zone PTAC Heating Coil Outlet Node,                 !- Air Outlet Node Name",
         "    ,                                                         !- Temperature Setpoint Node Name",
         "    0,                                                        !- On Cycle Parasitic Electric Load {W}",
         "    ,                                                         !- Part Load Fraction Correlation Curve Name",
         "    0;                                                        !- Off Cycle Parasitic Fuel Load {W}",
         "  Coil:Cooling:DX:SingleSpeed,",
         "    Basic Zone PTAC Cooling Coil,                             !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Autosize,                                                 !- Gross Rated Total Cooling Capacity {W}",
         "    Autosize,                                                 !- Gross Rated Sensible Heat Ratio",
         "    3,                                                        !- Gross Rated Cooling COP {W/W}",
         "    Autosize,                                                 !- Rated Air Flow Rate {m3/s}",
         "    773.3,                                                    !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    934.4,                                                    !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    Basic Zone PTAC Mixed Air Node,                           !- Air Inlet Node Name",
         "    Basic Zone PTAC Cooling Coil Outlet Node,                 !- Air Outlet Node Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Part Load Fraction Correlation Curve Name",
         "    -25,                                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
         "    0,                                                        !- Nominal Time for Condensate Removal to Begin {s}",
         "    0,                                                        !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
         "Capacity {dimensionless}",
         "    0,                                                        !- Maximum Cycling Rate {cycles/hr}",
         "    0,                                                        !- Latent Capacity Time Constant {s}",
         "    ,                                                         !- Condenser Air Inlet Node Name",
         "    AirCooled,                                                !- Condenser Type",
         "    0,                                                        !- Evaporative Condenser Effectiveness {dimensionless}",
         "    Autosize,                                                 !- Evaporative Condenser Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Evaporative Condenser Pump Rated Power Consumption {W}",
         "    0,                                                        !- Crankcase Heater Capacity {W}",
         "    ,                                                         !- Crankcase Heater Capacity Function of Temperature Curve Name",
         "    0,                                                        !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
         "    ,                                                         !- Supply Water Storage Tank Name",
         "    ,                                                         !- Condensate Collection Water Storage Tank Name",
         "    0,                                                        !- Basin Heater Capacity {W/K}",
         "    10;                                                       !- Basin Heater Setpoint Temperature {C}",

         "  Zone,",
         "    Refrig Cond AC Zone,                                      !- Name",
         "    ,                                                         !- Direction of Relative North {deg}",
         "    0,                                                        !- X Origin {m}",
         "    0,                                                        !- Y Origin {m}",
         "    0,                                                        !- Z Origin {m}",
         "    ,                                                         !- Type",
         "    1,                                                        !- Multiplier",
         "    ,                                                         !- Ceiling Height {m}",
         "    ,                                                         !- Volume {m3}",
         "    ,                                                         !- Floor Area {m2}",
         "    ,                                                         !- Zone Inside Convection Algorithm",
         "    ,                                                         !- Zone Outside Convection Algorithm",
         "    Yes;                                                      !- Part of Total Floor Area",
         "  ZoneHVAC:EquipmentConnections,",
         "    Refrig Cond AC Zone,                                      !- Zone Name",
         "    Refrig Cond AC Zone Equipment List,                       !- Zone Conditioning Equipment List Name",
         "    Refrig Cond AC Zone Inlet Node,                           !- Zone Air Inlet Node or NodeList Name",
         "    Refrig Cond AC Zone Exhaust Node,                         !- Zone Air Exhaust Node or NodeList Name",
         "    Refrig Cond AC Zone Air Node,                             !- Zone Air Node Name",
         "    Refrig Cond AC Zone Return Node;                          !- Zone Return Air Node or NodeList Name",
         "  ZoneHVAC:EquipmentList,",
         "    Refrig Cond AC Zone Equipment List,                       !- Name",
         "    SequentialLoad,                                           !- Load Distribution Scheme",
         "    ZoneHVAC:AirDistributionUnit,                             !- Zone Equipment 1 Object Type",
         "    Refrig Cond AC ADU,                                       !- Zone Equipment 1 Name",
         "    1,                                                        !- Zone Equipment 1 Cooling Sequence",
         "    1,                                                        !- Zone Equipment 1 Heating or No-Load Sequence",
         "    ,                                                         !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
         "    ;                                                         !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
         "  ZoneHVAC:AirDistributionUnit,",
         "    Refrig Cond AC ADU,                                       !- Name",
         "    Refrig Cond AC Zone Inlet Node,                           !- Air Distribution Unit Outlet Node Name",
         "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,           !- Air Terminal Object Type",
         "    Refrig Cond AC Air Terminal;                              !- Air Terminal Name",
         "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
         "    Refrig Cond AC Air Terminal,                              !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Cond AC Air Terminal Inlet Node,                   !- Air Inlet Node Name",
         "    Refrig Cond AC Zone Inlet Node,                           !- Air Outlet Node Name",
         "    AutoSize;                                                 !- Maximum Air Flow Rate {m3/s}",
         "  AirLoopHVAC,",
         "    Refrig Cond AC PTAC,                                      !- Name",
         "    ,                                                         !- Controller List Name",
         "    ,                                                         !- Availability Manager List Name",
         "    AutoSize,                                                 !- Design Supply Air Flow Rate {m3/s}",
         "    Refrig Cond AC PTAC Supply Branch List,                   !- Branch List Name",
         "    ,                                                         !- Connector List Name",
         "    Refrig Cond AC PTAC Supply Side Inlet Node,               !- Supply Side Inlet Node Name",
         "    Refrig Cond AC PTAC Demand Side Outlet Node,              !- Demand Side Outlet Node Name",
         "    Refrig Cond AC PTAC Demand Side Inlet Node,               !- Demand Side Inlet Node Names",
         "    Refrig Cond AC PTAC Supply Side Outlet Node,              !- Supply Side Outlet Node Names",
         "    1;                                                        !- Design Return Air Flow Fraction of Supply Air Flow",
         "  BranchList,",
         "    Refrig Cond AC PTAC Supply Branch List,                   !- Name",
         "    Refrig Cond AC PTAC Branch;                               !- Branch 1 Name",
         "  Branch,",
         "    Refrig Cond AC PTAC Branch,                               !- Name",
         "    ,                                                         !- Pressure Drop Curve Name",
         "    AirLoopHVAC:OutdoorAirSystem,                             !- Component 1 Object Type",
         "    Refrig Cond AC PTAC OA System,                            !- Component 1 Name",
         "    Refrig Cond AC PTAC Supply Side Inlet Node,               !- Component 1 Inlet Node Name",
         "    Refrig Cond AC PTAC Mixed Air Node,                       !- Component 1 Outlet Node Name",
         "    Coil:Cooling:DX:SingleSpeed,                              !- Component 2 Object Type",
         "    Refrig Cond AC PTAC Cooling Coil,                         !- Component 2 Name",
         "    Refrig Cond AC PTAC Mixed Air Node,                       !- Component 2 Inlet Node Name",
         "    Refrig Cond AC PTAC Desuperheater Inlet Node,             !- Component 2 Outlet Node Name",
         "    Coil:Heating:Desuperheater,                               !- Component 3 Object Type",
         "    Refrig Cond AC PTAC Desuperheater,                        !- Component 3 Name",
         "    Refrig Cond AC PTAC Desuperheater Inlet Node,             !- Component 3 Inlet Node Name",
         "    Refrig Cond AC PTAC Desuperheater Outlet Node,            !- Component 3 Outlet Node Name",
         "    Coil:Heating:Fuel,                                        !- Component 4 Object Type",
         "    Refrig Cond AC PTAC Heating Coil,                         !- Component 4 Name",
         "    Refrig Cond AC PTAC Desuperheater Outlet Node,            !- Component 4 Inlet Node Name",
         "    Refrig Cond AC PTAC Heating Coil Outlet Node,             !- Component 4 Outlet Node Name",
         "    Fan:ConstantVolume,                                       !- Component 5 Object Type",
         "    Refrig Cond AC PTAC Fan,                                  !- Component 5 Name",
         "    Refrig Cond AC PTAC Heating Coil Outlet Node,             !- Component 5 Inlet Node Name",
         "    Refrig Cond AC PTAC Supply Side Outlet Node;              !- Component 5 Outlet Node Name",
         "  AirLoopHVAC:OutdoorAirSystem,",
         "    Refrig Cond AC PTAC OA System,                            !- Name",
         "    Refrig Cond AC PTAC OA System Controller List,            !- Controller List Name",
         "    Refrig Cond AC PTAC OA System Equipment List;             !- Outdoor Air Equipment List Name",
         "  AirLoopHVAC:ControllerList,",
         "    Refrig Cond AC PTAC OA System Controller List,            !- Name",
         "    Controller:OutdoorAir,                                    !- Controller 1 Object Type",
         "    Refrig Cond AC PTAC OA Controller;                        !- Controller 1 Name",
         "  Controller:OutdoorAir,",
         "    Refrig Cond AC PTAC OA Controller,                        !- Name",
         "    Refrig Cond AC PTAC Relief Node,                          !- Relief Air Outlet Node Name",
         "    Refrig Cond AC PTAC Supply Side Inlet Node,               !- Return Air Node Name",
         "    Refrig Cond AC PTAC Mixed Air Node,                       !- Mixed Air Node Name",
         "    Refrig Cond AC PTAC OA Node,                              !- Actuator Node Name",
         "    0,                                                        !- Minimum Outdoor Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Maximum Outdoor Air Flow Rate {m3/s}",
         "    NoEconomizer,                                             !- Economizer Control Type",
         "    ModulateFlow,                                             !- Economizer Control Action Type",
         "    28,                                                       !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
         "    64000,                                                    !- Economizer Maximum Limit Enthalpy {J/kg}",
         "    ,                                                         !- Economizer Maximum Limit Dewpoint Temperature {C}",
         "    ,                                                         !- Electronic Enthalpy Limit Curve Name",
         "    -100,                                                     !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
         "    NoLockout,                                                !- Lockout Type",
         "    FixedMinimum,                                             !- Minimum Limit Type",
         "    ,                                                         !- Minimum Outdoor Air Schedule Name",
         "    ,                                                         !- Minimum Fraction of Outdoor Air Schedule Name",
         "    ,                                                         !- Maximum Fraction of Outdoor Air Schedule Name",
         "    Refrig Cond AC PTAC MV Controller,                        !- Mechanical Ventilation Controller Name",
         "    ,                                                         !- Time of Day Economizer Control Schedule Name",
         "    No,                                                       !- High Humidity Control",
         "    ,                                                         !- Humidistat Control Zone Name",
         "    ,                                                         !- High Humidity Outdoor Air Flow Ratio",
         "    Yes,                                                      !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
         "    BypassWhenWithinEconomizerLimits,                         !- Heat Recovery Bypass Control Type",
         "    InterlockedWithMechanicalCooling;                         !- Economizer Operation Staging",
         "  OutdoorAir:NodeList,",
         "    Refrig Cond AC PTAC OA Node;                              !- Node or NodeList Name 1",
         "  Controller:MechanicalVentilation,",
         "    Refrig Cond AC PTAC MV Controller,                        !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    No,                                                       !- Demand Controlled Ventilation",
         "    ZoneSum,                                                  !- System Outdoor Air Method",
         "    ,                                                         !- Zone Maximum Outdoor Air Fraction {dimensionless}",
         "    Refrig Cond AC Zone,                                      !- Zone or ZoneList 1 Name",
         "    ,                                                         !- Design Specification Outdoor Air Object Name 1",
         "    ;                                                         !- Design Specification Zone Air Distribution Object Name 1",
         "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
         "    Refrig Cond AC PTAC OA System Equipment List,             !- Name",
         "    OutdoorAir:Mixer,                                         !- Component 1 Object Type",
         "    Refrig Cond AC PTAC OA System Outdoor Air Mixer;          !- Component 1 Name",
         "  OutdoorAir:Mixer,",
         "    Refrig Cond AC PTAC OA System Outdoor Air Mixer,          !- Name",
         "    Refrig Cond AC PTAC Mixed Air Node,                       !- Mixed Air Node Name",
         "    Refrig Cond AC PTAC OA Node,                              !- Outdoor Air Stream Node Name",
         "    Refrig Cond AC PTAC Relief Node,                          !- Relief Air Stream Node Name",
         "    Refrig Cond AC PTAC Supply Side Inlet Node;               !- Return Air Stream Node Name",
         "  Coil:Cooling:DX:SingleSpeed,",
         "    Refrig Cond AC PTAC Cooling Coil,                         !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Autosize,                                                 !- Gross Rated Total Cooling Capacity {W}",
         "    Autosize,                                                 !- Gross Rated Sensible Heat Ratio",
         "    3,                                                        !- Gross Rated Cooling COP {W/W}",
         "    Autosize,                                                 !- Rated Air Flow Rate {m3/s}",
         "    773.3,                                                    !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    934.4,                                                    !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    Refrig Cond AC PTAC Mixed Air Node,                       !- Air Inlet Node Name",
         "    Refrig Cond AC PTAC Desuperheater Inlet Node,             !- Air Outlet Node Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Part Load Fraction Correlation Curve Name",
         "    -25,                                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
         "    0,                                                        !- Nominal Time for Condensate Removal to Begin {s}",
         "    0,                                                        !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
         "Capacity {dimensionless}",
         "    0,                                                        !- Maximum Cycling Rate {cycles/hr}",
         "    0,                                                        !- Latent Capacity Time Constant {s}",
         "    ,                                                         !- Condenser Air Inlet Node Name",
         "    AirCooled,                                                !- Condenser Type",
         "    0.9,                                                      !- Evaporative Condenser Effectiveness {dimensionless}",
         "    Autosize,                                                 !- Evaporative Condenser Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Evaporative Condenser Pump Rated Power Consumption {W}",
         "    0,                                                        !- Crankcase Heater Capacity {W}",
         "    ,                                                         !- Crankcase Heater Capacity Function of Temperature Curve Name",
         "    10,                                                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
         "    ,                                                         !- Supply Water Storage Tank Name",
         "    ,                                                         !- Condensate Collection Water Storage Tank Name",
         "    0,                                                        !- Basin Heater Capacity {W/K}",
         "    2;                                                        !- Basin Heater Setpoint Temperature {C}",
         "  Coil:Heating:Desuperheater,",
         "    Refrig Cond AC PTAC Desuperheater,                        !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.25,                                                     !- Refrig Cond AC PTAC Desuperheater Recovery Efficiency",
         "    Refrig Cond AC PTAC Desuperheater Inlet Node,             !- Air Inlet Node Name",
         "    Refrig Cond AC PTAC Desuperheater Outlet Node,            !- Air Outlet Node Name",
         "    Refrigeration:Condenser:AirCooled,                        !- Heating Source Object Type",
         "    Refrig Cond AC Refrig Condenser,                          !- Heating Source Name",
         "    Refrig Cond AC PTAC Desuperheater Outlet Node;            !- Temperature Setpoint Node Name",
         "  Coil:Heating:Fuel,",
         "    Refrig Cond AC PTAC Heating Coil,                         !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    NaturalGas,                                               !- Fuel Type",
         "    0.9,                                                      !- Burner Efficiency",
         "    AutoSize,                                                 !- Nominal Capacity {W}",
         "    Refrig Cond AC PTAC Desuperheater Outlet Node,            !- Air Inlet Node Name",
         "    Refrig Cond AC PTAC Heating Coil Outlet Node,             !- Air Outlet Node Name",
         "    Refrig Cond AC PTAC Heating Coil Outlet Node,             !- Temperature Setpoint Node Name",
         "    0,                                                        !- On Cycle Parasitic Electric Load {W}",
         "    ,                                                         !- Part Load Fraction Correlation Curve Name",
         "    0;                                                        !- Off Cycle Parasitic Fuel Load {W}",
         "  Fan:ConstantVolume,",
         "    Refrig Cond AC PTAC Fan,                                  !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.7,                                                      !- Fan Total Efficiency",
         "    500,                                                      !- Pressure Rise {Pa}",
         "    AutoSize,                                                 !- Maximum Flow Rate {m3/s}",
         "    0.9,                                                      !- Motor Efficiency",
         "    1,                                                        !- Motor In Airstream Fraction",
         "    Refrig Cond AC PTAC Heating Coil Outlet Node,             !- Air Inlet Node Name",
         "    Refrig Cond AC PTAC Supply Side Outlet Node;              !- Air Outlet Node Name",
         "  Refrigeration:System,",
         "    Refrig Cond AC Refrig System,                             !- Name",
         "    Refrig Cond AC CaseAndWalkInList,                         !- Refrigerated Case or Walkin or CaseAndWalkInList Name",
         "    ,                                                         !- Refrigeration Transfer Load or TransferLoad List Name",
         "    Refrig Cond AC Refrig Condenser,                          !- Refrigeration Condenser Name",
         "    Refrig Cond AC Refrig Compressor,                         !- Compressor or CompressorList Name",
         "    20,                                                       !- Minimum Condensing Temperature {C}",
         "    R407a,                                                    !- Refrigeration System Working Fluid Type",
         "    ConstantSuctionTemperature,                               !- Suction Temperature Control Type",
         "    ,                                                         !- Mechanical Subcooler Name",
         "    ,                                                         !- Liquid Suction Heat Exchanger Subcooler Name",
         "    0,                                                        !- Sum UA Suction Piping {W/K}",
         "    Refrig Cond AC Zone,                                      !- Suction Piping Zone Name",
         "    General,                                                  !- End-Use Subcategory",
         "    1,                                                        !- Number of Compressor Stages",
         "    None,                                                     !- Intercooler Type",
         "    0.8;                                                      !- Shell-and-Coil Intercooler Effectiveness",
         "  Refrigeration:CaseAndWalkInList,",
         "    Refrig Cond AC CaseAndWalkInList,                         !- Name",
         "    Refrig Cond AC Case,                                      !- Case or Refrig Cond AC WalkIn Name",
         "    Refrig Cond AC WalkIn;                                    !- Case or WalkIn 2 Name",
         "  Refrigeration:Case,",
         "    Refrig Cond AC Case,                                      !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Cond AC Zone,                                      !- Zone Name",
         "    24,                                                       !- Rated Ambient Temperature {C}",
         "    55,                                                       !- Rated Ambient Relative Humidity {percent}",
         "    1406,                                                     !- Rated Total Cooling Capacity per Unit Length {W/m}",
         "    0.3,                                                      !- Rated Latent Heat Ratio",
         "    0.85,                                                     !- Rated Runtime Fraction",
         "    2.19,                                                     !- Case Length {m}",
         "    4,                                                        !- Case Operating Temperature {C}",
         "    CaseTemperatureMethod,                                    !- Latent Case Credit Curve Type",
         "    Generic Curve,                                            !- Latent Case Credit Curve Name",
         "    30,                                                       !- Standard Case Fan Power per Unit Length {W/m}",
         "    30,                                                       !- Operating Case Fan Power per Unit Length {W/m}",
         "    20,                                                       !- Standard Case Lighting Power per Unit Length {W/m}",
         "    20,                                                       !- Installed Case Lighting Power per Unit Length {W/m}",
         "    ,                                                         !- Case Lighting Schedule Name",
         "    1,                                                        !- Fraction of Lighting Energy to Case",
         "    0,                                                        !- Case Anti-Sweat Heater Power per Unit Length {W/m}",
         "    0,                                                        !- Minimum Anti-Sweat Heater Power per Unit Length {W/m}",
         "    None,                                                     !- Anti-Sweat Heater Control Type",
         "    -10,                                                      !- Humidity at Zero Anti-Sweat Heater Energy {percent}",
         "    1.5,                                                      !- Case Height {m}",
         "    1,                                                        !- Fraction of Anti-Sweat Heater Energy to Case",
         "    0,                                                        !- Case Defrost Power per Unit Length {W/m}",
         "    OffCycle,                                                 !- Case Defrost Type",
         "    AlwaysOn,                                                 !- Case Defrost Schedule Name",
         "    ,                                                         !- Case Defrost Drip-Down Schedule Name",
         "    None,                                                     !- Defrost Energy Correction Curve Type",
         "    ,                                                         !- Defrost Energy Correction Curve Name",
         "    0,                                                        !- Under Case HVAC Return Air Fraction",
         "    ,                                                         !- Refrigerated Case Restocking Schedule Name",
         "    ,                                                         !- Case Credit Fraction Schedule Name",
         "    -7,                                                       !- Design Evaporator Temperature or Brine Inlet Temperature {C}",
         "    0;                                                        !- Average Refrigerant Charge Inventory {kg/m}",
         "  Refrigeration:WalkIn,",
         "    Refrig Cond AC WalkIn,                                    !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    4744,                                                     !- Rated Coil Cooling Capacity {W}",
         "    4,                                                        !- Operating Temperature {C}",
         "    -6,                                                       !- Rated Cooling Source Temperature {C}",
         "    0,                                                        !- Rated Total Heating Power {W}",
         "    ,                                                         !- Heating Power Schedule Name",
         "    308,                                                      !- Rated Cooling Coil Fan Power {W}",
         "    0,                                                        !- Rated Circulation Fan Power {W}",
         "    100,                                                      !- Rated Total Lighting Power {W}",
         "    ,                                                         !- Lighting Schedule Name",
         "    Electric,                                                 !- Defrost Type",
         "    TimeSchedule,                                             !- Defrost Control Type",
         "    AlwaysOn,                                                 !- Defrost Schedule Name",
         "    ,                                                         !- Defrost Drip-Down Schedule Name",
         "    2400,                                                     !- Defrost Power {W}",
         "    ,                                                         !- Temperature Termination Defrost Fraction to Ice {dimensionless}",
         "    ,                                                         !- Restocking Schedule Name",
         "    0,                                                        !- Average Refrigerant Charge Inventory {kg}",
         "    13,                                                       !- Insulated Floor Surface Area {m2}",
         "    0.25,                                                     !- Insulated Floor U-Value {W/m2-K}",
         "    Refrig Cond AC Zone,                                      !- Zone 1 Name",
         "    55,                                                       !- Total Insulated Surface Area Facing Zone 1 {m2}",
         "    0.3,                                                      !- Insulated Surface U-Value Facing Zone 1 {W/m2-K}",
         "    0,                                                        !- Area of Glass Reach In Doors Facing Zone 1 {m2}",
         "    1.5,                                                      !- Height of Glass Reach In Doors Facing Zone 1 {m}",
         "    1.136,                                                    !- Glass Reach In Door U Value Facing Zone 1 {W/m2-K}",
         "    ,                                                         !- Glass Reach In Door Opening Schedule Name Facing Zone 1",
         "    2,                                                        !- Area of Stocking Doors Facing Zone 1 {m2}",
         "    2,                                                        !- Height of Stocking Doors Facing Zone 1 {m}",
         "    0.3785,                                                   !- Stocking Door U Value Facing Zone 1 {W/m2-K}",
         "    ,                                                         !- Stocking Door Opening Schedule Name Facing Zone 1",
         "    None;                                                     !- Stocking Door Opening Protection Type Facing Zone 1",
         "  Refrigeration:Condenser:AirCooled,",
         "    Refrig Cond AC Refrig Condenser,                          !- Name",
         "    Condenser Curve,                                          !- Rated Effective Total Heat Rejection Rate Curve Name",
         "    0,                                                        !- Rated Subcooling Temperature Difference {deltaC}",
         "    VariableSpeed,                                            !- Condenser Fan Speed Control Type",
         "    2000,                                                     !- Rated Fan Power {W}",
         "    ,                                                         !- Minimum Fan Air Flow Ratio {dimensionless}",
         "    Refrig Cond AC Outdoor Air Node,                          !- Air Inlet Node Name or Zone Name",
         "    General,                                                  !- End-Use Subcategory",
         "    0,                                                        !- Condenser Refrigerant Operating Charge Inventory {kg}",
         "    0,                                                        !- Condensate Receiver Refrigerant Inventory {kg}",
         "    0;                                                        !- Condensate Piping Refrigerant Inventory {kg}",
         "  OutdoorAir:Node,",
         "    Refrig Cond AC Outdoor Air Node;                          !- Name",
         "  Refrigeration:Compressor,",
         "    Refrig Cond AC Refrig Compressor,                         !- Name",
         "    Generic Curve,                                            !- Refrigeration Compressor Power Curve Name",
         "    Capacity Curve,                                           !- Refrigeration Compressor Capacity Curve Name",
         "    ,                                                         !- Rated Superheat {deltaC}",
         "    18.3,                                                     !- Rated Return Gas Temperature {C}",
         "    ,                                                         !- Rated Liquid Temperature {C}",
         "    0,                                                        !- Rated Subcooling {deltaC}",
         "    General,                                                  !- End-Use Subcategory",
         "    Subcritical;                                              !- Mode of Operation",

         "  Zone,",
         "    Refrig Cond WC Zone,                                      !- Name",
         "    ,                                                         !- Direction of Relative North {deg}",
         "    0,                                                        !- X Origin {m}",
         "    0,                                                        !- Y Origin {m}",
         "    0,                                                        !- Z Origin {m}",
         "    ,                                                         !- Type",
         "    1,                                                        !- Multiplier",
         "    ,                                                         !- Ceiling Height {m}",
         "    ,                                                         !- Volume {m3}",
         "    ,                                                         !- Floor Area {m2}",
         "    ,                                                         !- Zone Inside Convection Algorithm",
         "    ,                                                         !- Zone Outside Convection Algorithm",
         "    Yes;                                                      !- Part of Total Floor Area",
         "  ZoneHVAC:EquipmentConnections,",
         "    Refrig Cond WC Zone,                                      !- Zone Name",
         "    Refrig Cond WC Zone Equipment List,                       !- Zone Conditioning Equipment List Name",
         "    Refrig Cond WC Zone Inlet Node,                           !- Zone Air Inlet Node or NodeList Name",
         "    Refrig Cond WC Zone Exhaust Node,                         !- Zone Air Exhaust Node or NodeList Name",
         "    Refrig Cond WC Zone Air Node,                             !- Zone Air Node Name",
         "    Refrig Cond WC Zone Return Node;                          !- Zone Return Air Node or NodeList Name",
         "  ZoneHVAC:EquipmentList,",
         "    Refrig Cond WC Zone Equipment List,                       !- Name",
         "    SequentialLoad,                                           !- Load Distribution Scheme",
         "    ZoneHVAC:AirDistributionUnit,                             !- Zone Equipment 1 Object Type",
         "    Refrig Cond WC ADU,                                       !- Zone Equipment 1 Name",
         "    1,                                                        !- Zone Equipment 1 Cooling Sequence",
         "    1,                                                        !- Zone Equipment 1 Heating or No-Load Sequence",
         "    ,                                                         !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
         "    ;                                                         !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
         "  ZoneHVAC:AirDistributionUnit,",
         "    Refrig Cond WC ADU,                                       !- Name",
         "    Refrig Cond WC Zone Inlet Node,                           !- Air Distribution Unit Outlet Node Name",
         "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,           !- Air Terminal Object Type",
         "    Refrig Cond WC Air Terminal;                              !- Air Terminal Name",
         "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
         "    Refrig Cond WC Air Terminal,                              !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Cond WC Air Terminal Inlet Node,                   !- Air Inlet Node Name",
         "    Refrig Cond WC Zone Inlet Node,                           !- Air Outlet Node Name",
         "    AutoSize;                                                 !- Maximum Air Flow Rate {m3/s}",
         "  AirLoopHVAC,",
         "    Refrig Cond WC PTAC,                                      !- Name",
         "    ,                                                         !- Controller List Name",
         "    ,                                                         !- Availability Manager List Name",
         "    AutoSize,                                                 !- Design Supply Air Flow Rate {m3/s}",
         "    Refrig Cond WC PTAC Supply Branch List,                   !- Branch List Name",
         "    ,                                                         !- Connector List Name",
         "    Refrig Cond WC PTAC Supply Side Inlet Node,               !- Supply Side Inlet Node Name",
         "    Refrig Cond WC PTAC Demand Side Outlet Node,              !- Demand Side Outlet Node Name",
         "    Refrig Cond WC PTAC Demand Side Inlet Node,               !- Demand Side Inlet Node Names",
         "    Refrig Cond WC PTAC Supply Side Outlet Node,              !- Supply Side Outlet Node Names",
         "    1;                                                        !- Design Return Air Flow Fraction of Supply Air Flow",
         "  BranchList,",
         "    Refrig Cond WC PTAC Supply Branch List,                   !- Name",
         "    Refrig Cond WC PTAC Branch;                               !- Branch 1 Name",
         "  Branch,",
         "    Refrig Cond WC PTAC Branch,                               !- Name",
         "    ,                                                         !- Pressure Drop Curve Name",
         "    AirLoopHVAC:OutdoorAirSystem,                             !- Component 1 Object Type",
         "    Refrig Cond WC PTAC OA System,                            !- Component 1 Name",
         "    Refrig Cond WC PTAC Supply Side Inlet Node,               !- Component 1 Inlet Node Name",
         "    Refrig Cond WC PTAC Mixed Air Node,                       !- Component 1 Outlet Node Name",
         "    Coil:Cooling:DX:SingleSpeed,                              !- Component 2 Object Type",
         "    Refrig Cond WC PTAC Cooling Coil,                         !- Component 2 Name",
         "    Refrig Cond WC PTAC Mixed Air Node,                       !- Component 2 Inlet Node Name",
         "    Refrig Cond WC PTAC Desuperheater Inlet Node,             !- Component 2 Outlet Node Name",
         "    Coil:Heating:Desuperheater,                               !- Component 3 Object Type",
         "    Refrig Cond WC PTAC Desuperheater,                        !- Component 3 Name",
         "    Refrig Cond WC PTAC Desuperheater Inlet Node,             !- Component 3 Inlet Node Name",
         "    Refrig Cond WC PTAC Desuperheater Outlet Node,            !- Component 3 Outlet Node Name",
         "    Coil:Heating:Fuel,                                        !- Component 4 Object Type",
         "    Refrig Cond WC PTAC Heating Coil,                         !- Component 4 Name",
         "    Refrig Cond WC PTAC Desuperheater Outlet Node,            !- Component 4 Inlet Node Name",
         "    Refrig Cond WC PTAC Heating Coil Outlet Node,             !- Component 4 Outlet Node Name",
         "    Fan:ConstantVolume,                                       !- Component 5 Object Type",
         "    Refrig Cond WC PTAC Fan,                                  !- Component 5 Name",
         "    Refrig Cond WC PTAC Heating Coil Outlet Node,             !- Component 5 Inlet Node Name",
         "    Refrig Cond WC PTAC Supply Side Outlet Node;              !- Component 5 Outlet Node Name",
         "  AirLoopHVAC:OutdoorAirSystem,",
         "    Refrig Cond WC PTAC OA System,                            !- Name",
         "    Refrig Cond WC PTAC OA System Controller List,            !- Controller List Name",
         "    Refrig Cond WC PTAC OA System Equipment List;             !- Outdoor Air Equipment List Name",
         "  AirLoopHVAC:ControllerList,",
         "    Refrig Cond WC PTAC OA System Controller List,            !- Name",
         "    Controller:OutdoorAir,                                    !- Controller 1 Object Type",
         "    Refrig Cond WC PTAC OA Controller;                        !- Controller 1 Name",
         "  Controller:OutdoorAir,",
         "    Refrig Cond WC PTAC OA Controller,                        !- Name",
         "    Refrig Cond WC PTAC Relief Node,                          !- Relief Air Outlet Node Name",
         "    Refrig Cond WC PTAC Supply Side Inlet Node,               !- Return Air Node Name",
         "    Refrig Cond WC PTAC Mixed Air Node,                       !- Mixed Air Node Name",
         "    Refrig Cond WC PTAC OA Node,                              !- Actuator Node Name",
         "    0,                                                        !- Minimum Outdoor Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Maximum Outdoor Air Flow Rate {m3/s}",
         "    NoEconomizer,                                             !- Economizer Control Type",
         "    ModulateFlow,                                             !- Economizer Control Action Type",
         "    28,                                                       !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
         "    64000,                                                    !- Economizer Maximum Limit Enthalpy {J/kg}",
         "    ,                                                         !- Economizer Maximum Limit Dewpoint Temperature {C}",
         "    ,                                                         !- Electronic Enthalpy Limit Curve Name",
         "    -100,                                                     !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
         "    NoLockout,                                                !- Lockout Type",
         "    FixedMinimum,                                             !- Minimum Limit Type",
         "    ,                                                         !- Minimum Outdoor Air Schedule Name",
         "    ,                                                         !- Minimum Fraction of Outdoor Air Schedule Name",
         "    ,                                                         !- Maximum Fraction of Outdoor Air Schedule Name",
         "    Refrig Cond WC PTAC MV Controller,                        !- Mechanical Ventilation Controller Name",
         "    ,                                                         !- Time of Day Economizer Control Schedule Name",
         "    No,                                                       !- High Humidity Control",
         "    ,                                                         !- Humidistat Control Zone Name",
         "    ,                                                         !- High Humidity Outdoor Air Flow Ratio",
         "    Yes,                                                      !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
         "    BypassWhenWithinEconomizerLimits,                         !- Heat Recovery Bypass Control Type",
         "    InterlockedWithMechanicalCooling;                         !- Economizer Operation Staging",
         "  OutdoorAir:NodeList,",
         "    Refrig Cond WC PTAC OA Node;                              !- Node or NodeList Name 1",
         "  Controller:MechanicalVentilation,",
         "    Refrig Cond WC PTAC MV Controller,                        !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    No,                                                       !- Demand Controlled Ventilation",
         "    ZoneSum,                                                  !- System Outdoor Air Method",
         "    ,                                                         !- Zone Maximum Outdoor Air Fraction {dimensionless}",
         "    Refrig Cond WC Zone,                                      !- Zone or ZoneList 1 Name",
         "    ,                                                         !- Design Specification Outdoor Air Object Name 1",
         "    ;                                                         !- Design Specification Zone Air Distribution Object Name 1",
         "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
         "    Refrig Cond WC PTAC OA System Equipment List,             !- Name",
         "    OutdoorAir:Mixer,                                         !- Component 1 Object Type",
         "    Refrig Cond WC PTAC OA System Outdoor Air Mixer;          !- Component 1 Name",
         "  OutdoorAir:Mixer,",
         "    Refrig Cond WC PTAC OA System Outdoor Air Mixer,          !- Name",
         "    Refrig Cond WC PTAC Mixed Air Node,                       !- Mixed Air Node Name",
         "    Refrig Cond WC PTAC OA Node,                              !- Outdoor Air Stream Node Name",
         "    Refrig Cond WC PTAC Relief Node,                          !- Relief Air Stream Node Name",
         "    Refrig Cond WC PTAC Supply Side Inlet Node;               !- Return Air Stream Node Name",
         "  Coil:Cooling:DX:SingleSpeed,",
         "    Refrig Cond WC PTAC Cooling Coil,                         !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Autosize,                                                 !- Gross Rated Total Cooling Capacity {W}",
         "    Autosize,                                                 !- Gross Rated Sensible Heat Ratio",
         "    3,                                                        !- Gross Rated Cooling COP {W/W}",
         "    Autosize,                                                 !- Rated Air Flow Rate {m3/s}",
         "    773.3,                                                    !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    934.4,                                                    !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    Refrig Cond WC PTAC Mixed Air Node,                       !- Air Inlet Node Name",
         "    Refrig Cond WC PTAC Desuperheater Inlet Node,             !- Air Outlet Node Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Part Load Fraction Correlation Curve Name",
         "    -25,                                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
         "    0,                                                        !- Nominal Time for Condensate Removal to Begin {s}",
         "    0,                                                        !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
         "Capacity {dimensionless}",
         "    0,                                                        !- Maximum Cycling Rate {cycles/hr}",
         "    0,                                                        !- Latent Capacity Time Constant {s}",
         "    ,                                                         !- Condenser Air Inlet Node Name",
         "    AirCooled,                                                !- Condenser Type",
         "    0.9,                                                      !- Evaporative Condenser Effectiveness {dimensionless}",
         "    Autosize,                                                 !- Evaporative Condenser Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Evaporative Condenser Pump Rated Power Consumption {W}",
         "    0,                                                        !- Crankcase Heater Capacity {W}",
         "    ,                                                         !- Crankcase Heater Capacity Function of Temperature Curve Name",
         "    10,                                                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
         "    ,                                                         !- Supply Water Storage Tank Name",
         "    ,                                                         !- Condensate Collection Water Storage Tank Name",
         "    0,                                                        !- Basin Heater Capacity {W/K}",
         "    2;                                                        !- Basin Heater Setpoint Temperature {C}",
         "  Coil:Heating:Desuperheater,",
         "    Refrig Cond WC PTAC Desuperheater,                        !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.25,                                                     !- Refrig Cond WC PTAC Desuperheater Recovery Efficiency",
         "    Refrig Cond WC PTAC Desuperheater Inlet Node,             !- Air Inlet Node Name",
         "    Refrig Cond WC PTAC Desuperheater Outlet Node,            !- Air Outlet Node Name",
         "    Refrigeration:Condenser:WaterCooled,                      !- Heating Source Object Type",
         "    Refrig Cond WC Refrig Condenser,                          !- Heating Source Name",
         "    Refrig Cond WC PTAC Desuperheater Outlet Node;            !- Temperature Setpoint Node Name",
         "  Coil:Heating:Fuel,",
         "    Refrig Cond WC PTAC Heating Coil,                         !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    NaturalGas,                                               !- Fuel Type",
         "    0.9,                                                      !- Burner Efficiency",
         "    AutoSize,                                                 !- Nominal Capacity {W}",
         "    Refrig Cond WC PTAC Desuperheater Outlet Node,            !- Air Inlet Node Name",
         "    Refrig Cond WC PTAC Heating Coil Outlet Node,             !- Air Outlet Node Name",
         "    Refrig Cond WC PTAC Heating Coil Outlet Node,             !- Temperature Setpoint Node Name",
         "    0,                                                        !- On Cycle Parasitic Electric Load {W}",
         "    ,                                                         !- Part Load Fraction Correlation Curve Name",
         "    0;                                                        !- Off Cycle Parasitic Fuel Load {W}",
         "  Fan:ConstantVolume,",
         "    Refrig Cond WC PTAC Fan,                                  !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.7,                                                      !- Fan Total Efficiency",
         "    500,                                                      !- Pressure Rise {Pa}",
         "    AutoSize,                                                 !- Maximum Flow Rate {m3/s}",
         "    0.9,                                                      !- Motor Efficiency",
         "    1,                                                        !- Motor In Airstream Fraction",
         "    Refrig Cond WC PTAC Heating Coil Outlet Node,             !- Air Inlet Node Name",
         "    Refrig Cond WC PTAC Supply Side Outlet Node;              !- Air Outlet Node Name",
         "  Refrigeration:System,",
         "    Refrig Cond WC Refrig System,                             !- Name",
         "    Refrig Cond WC CaseAndWalkInList,                         !- Refrigerated Case or Walkin or CaseAndWalkInList Name",
         "    ,                                                         !- Refrigeration Transfer Load or TransferLoad List Name",
         "    Refrig Cond WC Refrig Condenser,                          !- Refrigeration Condenser Name",
         "    Refrig Cond WC Refrig Compressor,                         !- Compressor or CompressorList Name",
         "    20,                                                       !- Minimum Condensing Temperature {C}",
         "    R407a,                                                    !- Refrigeration System Working Fluid Type",
         "    ConstantSuctionTemperature,                               !- Suction Temperature Control Type",
         "    ,                                                         !- Mechanical Subcooler Name",
         "    ,                                                         !- Liquid Suction Heat Exchanger Subcooler Name",
         "    0,                                                        !- Sum UA Suction Piping {W/K}",
         "    Refrig Cond WC Zone,                                      !- Suction Piping Zone Name",
         "    General,                                                  !- End-Use Subcategory",
         "    1,                                                        !- Number of Compressor Stages",
         "    None,                                                     !- Intercooler Type",
         "    0.8;                                                      !- Shell-and-Coil Intercooler Effectiveness",
         "  Refrigeration:CaseAndWalkInList,",
         "    Refrig Cond WC CaseAndWalkInList,                         !- Name",
         "    Refrig Cond WC Case,                                      !- Case or Refrig Cond WC WalkIn Name",
         "    Refrig Cond WC WalkIn;                                    !- Case or WalkIn 2 Name",
         "  Refrigeration:Case,",
         "    Refrig Cond WC Case,                                      !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Cond WC Zone,                                      !- Zone Name",
         "    24,                                                       !- Rated Ambient Temperature {C}",
         "    55,                                                       !- Rated Ambient Relative Humidity {percent}",
         "    1406,                                                     !- Rated Total Cooling Capacity per Unit Length {W/m}",
         "    0.3,                                                      !- Rated Latent Heat Ratio",
         "    0.85,                                                     !- Rated Runtime Fraction",
         "    2.19,                                                     !- Case Length {m}",
         "    4,                                                        !- Case Operating Temperature {C}",
         "    CaseTemperatureMethod,                                    !- Latent Case Credit Curve Type",
         "    Generic Curve,                                            !- Latent Case Credit Curve Name",
         "    30,                                                       !- Standard Case Fan Power per Unit Length {W/m}",
         "    30,                                                       !- Operating Case Fan Power per Unit Length {W/m}",
         "    20,                                                       !- Standard Case Lighting Power per Unit Length {W/m}",
         "    20,                                                       !- Installed Case Lighting Power per Unit Length {W/m}",
         "    ,                                                         !- Case Lighting Schedule Name",
         "    1,                                                        !- Fraction of Lighting Energy to Case",
         "    0,                                                        !- Case Anti-Sweat Heater Power per Unit Length {W/m}",
         "    0,                                                        !- Minimum Anti-Sweat Heater Power per Unit Length {W/m}",
         "    None,                                                     !- Anti-Sweat Heater Control Type",
         "    -10,                                                      !- Humidity at Zero Anti-Sweat Heater Energy {percent}",
         "    1.5,                                                      !- Case Height {m}",
         "    1,                                                        !- Fraction of Anti-Sweat Heater Energy to Case",
         "    0,                                                        !- Case Defrost Power per Unit Length {W/m}",
         "    OffCycle,                                                 !- Case Defrost Type",
         "    AlwaysOn,                                                 !- Case Defrost Schedule Name",
         "    ,                                                         !- Case Defrost Drip-Down Schedule Name",
         "    None,                                                     !- Defrost Energy Correction Curve Type",
         "    ,                                                         !- Defrost Energy Correction Curve Name",
         "    0,                                                        !- Under Case HVAC Return Air Fraction",
         "    ,                                                         !- Refrigerated Case Restocking Schedule Name",
         "    ,                                                         !- Case Credit Fraction Schedule Name",
         "    -7,                                                       !- Design Evaporator Temperature or Brine Inlet Temperature {C}",
         "    0;                                                        !- Average Refrigerant Charge Inventory {kg/m}",
         "  Refrigeration:WalkIn,",
         "    Refrig Cond WC WalkIn,                                    !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    4744,                                                     !- Rated Coil Cooling Capacity {W}",
         "    4,                                                        !- Operating Temperature {C}",
         "    -6,                                                       !- Rated Cooling Source Temperature {C}",
         "    0,                                                        !- Rated Total Heating Power {W}",
         "    ,                                                         !- Heating Power Schedule Name",
         "    308,                                                      !- Rated Cooling Coil Fan Power {W}",
         "    0,                                                        !- Rated Circulation Fan Power {W}",
         "    100,                                                      !- Rated Total Lighting Power {W}",
         "    ,                                                         !- Lighting Schedule Name",
         "    Electric,                                                 !- Defrost Type",
         "    TimeSchedule,                                             !- Defrost Control Type",
         "    AlwaysOn,                                                 !- Defrost Schedule Name",
         "    ,                                                         !- Defrost Drip-Down Schedule Name",
         "    2400,                                                     !- Defrost Power {W}",
         "    ,                                                         !- Temperature Termination Defrost Fraction to Ice {dimensionless}",
         "    ,                                                         !- Restocking Schedule Name",
         "    0,                                                        !- Average Refrigerant Charge Inventory {kg}",
         "    13,                                                       !- Insulated Floor Surface Area {m2}",
         "    0.25,                                                     !- Insulated Floor U-Value {W/m2-K}",
         "    Refrig Cond WC Zone,                                      !- Zone 1 Name",
         "    55,                                                       !- Total Insulated Surface Area Facing Zone 1 {m2}",
         "    0.3,                                                      !- Insulated Surface U-Value Facing Zone 1 {W/m2-K}",
         "    0,                                                        !- Area of Glass Reach In Doors Facing Zone 1 {m2}",
         "    1.5,                                                      !- Height of Glass Reach In Doors Facing Zone 1 {m}",
         "    1.136,                                                    !- Glass Reach In Door U Value Facing Zone 1 {W/m2-K}",
         "    ,                                                         !- Glass Reach In Door Opening Schedule Name Facing Zone 1",
         "    2,                                                        !- Area of Stocking Doors Facing Zone 1 {m2}",
         "    2,                                                        !- Height of Stocking Doors Facing Zone 1 {m}",
         "    0.3785,                                                   !- Stocking Door U Value Facing Zone 1 {W/m2-K}",
         "    ,                                                         !- Stocking Door Opening Schedule Name Facing Zone 1",
         "    None;                                                     !- Stocking Door Opening Protection Type Facing Zone 1",
         "  Refrigeration:Condenser:WaterCooled,",
         "    Refrig Cond WC Refrig Condenser,                          !- Name",
         "    58000.,                                                   !- Rated Effective Total Heat Rejection Rate {W}",
         "    29.4,                                                     !- Rated Condensing Temperature {C}",
         "    0.0,                                                      !- Rated Subcooling Temperature Difference {deltaC}",
         "    10.0,                                                     !- Rated Water Inlet Temperature {C}",
         "    Refrig Cond WC Water Inlet Node,                          !- Water Inlet Node Name",
         "    Refrig Cond WC Water Outlet Node,                         !- Water Outlet Node Name",
         "    ConstantFlow,                                             !- Water-Cooled Loop Flow Type",
         "    ,                                                         !- Water Outlet Temperature Schedule Name",
         "    0.003,                                                    !- Water Design Flow Rate {m3/s}",
         "    0.003,                                                    !- Water Maximum Flow Rate {m3/s}",
         "    55.,                                                      !- Water Maximum Water Outlet Temperature {C}",
         "    ,                                                         !- Water Minimum Water Inlet Temperature {C}",
         "    ,                                                         !- End-Use Subcategory",
         "    30.,                                                      !- Condenser Refrigerant Operating Charge Inventory {kg}",
         "    65.,                                                      !- Condensate Receiver Refrigerant Inventory {kg}",
         "    20.;                                                      !- Condensate Piping Refrigerant Inventory {kg}",
         "  OutdoorAir:Node,",
         "    Refrig Cond WC Outdoor Air Node;                          !- Name",
         "  Refrigeration:Compressor,",
         "    Refrig Cond WC Refrig Compressor,                         !- Name",
         "    Generic Curve,                                            !- Refrigeration Compressor Power Curve Name",
         "    Capacity Curve,                                           !- Refrigeration Compressor Capacity Curve Name",
         "    ,                                                         !- Rated Superheat {deltaC}",
         "    18.3,                                                     !- Rated Return Gas Temperature {C}",
         "    ,                                                         !- Rated Liquid Temperature {C}",
         "    0,                                                        !- Rated Subcooling {deltaC}",
         "    General,                                                  !- End-Use Subcategory",
         "    Subcritical;                                              !- Mode of Operation",

         "  Zone,",
         "    Refrig Cond EC Zone,                                      !- Name",
         "    ,                                                         !- Direction of Relative North {deg}",
         "    0,                                                        !- X Origin {m}",
         "    0,                                                        !- Y Origin {m}",
         "    0,                                                        !- Z Origin {m}",
         "    ,                                                         !- Type",
         "    1,                                                        !- Multiplier",
         "    ,                                                         !- Ceiling Height {m}",
         "    ,                                                         !- Volume {m3}",
         "    ,                                                         !- Floor Area {m2}",
         "    ,                                                         !- Zone Inside Convection Algorithm",
         "    ,                                                         !- Zone Outside Convection Algorithm",
         "    Yes;                                                      !- Part of Total Floor Area",
         "  ZoneHVAC:EquipmentConnections,",
         "    Refrig Cond EC Zone,                                      !- Zone Name",
         "    Refrig Cond EC Zone Equipment List,                       !- Zone Conditioning Equipment List Name",
         "    Refrig Cond EC Zone Inlet Node,                           !- Zone Air Inlet Node or NodeList Name",
         "    Refrig Cond EC Zone Exhaust Node,                         !- Zone Air Exhaust Node or NodeList Name",
         "    Refrig Cond EC Zone Air Node,                             !- Zone Air Node Name",
         "    Refrig Cond EC Zone Return Node;                          !- Zone Return Air Node or NodeList Name",
         "  ZoneHVAC:EquipmentList,",
         "    Refrig Cond EC Zone Equipment List,                       !- Name",
         "    SequentialLoad,                                           !- Load Distribution Scheme",
         "    ZoneHVAC:AirDistributionUnit,                             !- Zone Equipment 1 Object Type",
         "    Refrig Cond EC ADU,                                       !- Zone Equipment 1 Name",
         "    1,                                                        !- Zone Equipment 1 Cooling Sequence",
         "    1,                                                        !- Zone Equipment 1 Heating or No-Load Sequence",
         "    ,                                                         !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
         "    ;                                                         !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
         "  ZoneHVAC:AirDistributionUnit,",
         "    Refrig Cond EC ADU,                                       !- Name",
         "    Refrig Cond EC Zone Inlet Node,                           !- Air Distribution Unit Outlet Node Name",
         "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,           !- Air Terminal Object Type",
         "    Refrig Cond EC Air Terminal;                              !- Air Terminal Name",
         "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
         "    Refrig Cond EC Air Terminal,                              !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Cond EC Air Terminal Inlet Node,                   !- Air Inlet Node Name",
         "    Refrig Cond EC Zone Inlet Node,                           !- Air Outlet Node Name",
         "    AutoSize;                                                 !- Maximum Air Flow Rate {m3/s}",
         "  AirLoopHVAC,",
         "    Refrig Cond EC PTAC,                                      !- Name",
         "    ,                                                         !- Controller List Name",
         "    ,                                                         !- Availability Manager List Name",
         "    AutoSize,                                                 !- Design Supply Air Flow Rate {m3/s}",
         "    Refrig Cond EC PTAC Supply Branch List,                   !- Branch List Name",
         "    ,                                                         !- Connector List Name",
         "    Refrig Cond EC PTAC Supply Side Inlet Node,               !- Supply Side Inlet Node Name",
         "    Refrig Cond EC PTAC Demand Side Outlet Node,              !- Demand Side Outlet Node Name",
         "    Refrig Cond EC PTAC Demand Side Inlet Node,               !- Demand Side Inlet Node Names",
         "    Refrig Cond EC PTAC Supply Side Outlet Node,              !- Supply Side Outlet Node Names",
         "    1;                                                        !- Design Return Air Flow Fraction of Supply Air Flow",
         "  BranchList,",
         "    Refrig Cond EC PTAC Supply Branch List,                   !- Name",
         "    Refrig Cond EC PTAC Branch;                               !- Branch 1 Name",
         "  Branch,",
         "    Refrig Cond EC PTAC Branch,                               !- Name",
         "    ,                                                         !- Pressure Drop Curve Name",
         "    AirLoopHVAC:OutdoorAirSystem,                             !- Component 1 Object Type",
         "    Refrig Cond EC PTAC OA System,                            !- Component 1 Name",
         "    Refrig Cond EC PTAC Supply Side Inlet Node,               !- Component 1 Inlet Node Name",
         "    Refrig Cond EC PTAC Mixed Air Node,                       !- Component 1 Outlet Node Name",
         "    Coil:Cooling:DX:SingleSpeed,                              !- Component 2 Object Type",
         "    Refrig Cond EC PTAC Cooling Coil,                         !- Component 2 Name",
         "    Refrig Cond EC PTAC Mixed Air Node,                       !- Component 2 Inlet Node Name",
         "    Refrig Cond EC PTAC Desuperheater Inlet Node,             !- Component 2 Outlet Node Name",
         "    Coil:Heating:Desuperheater,                               !- Component 3 Object Type",
         "    Refrig Cond EC PTAC Desuperheater,                        !- Component 3 Name",
         "    Refrig Cond EC PTAC Desuperheater Inlet Node,             !- Component 3 Inlet Node Name",
         "    Refrig Cond EC PTAC Desuperheater Outlet Node,            !- Component 3 Outlet Node Name",
         "    Coil:Heating:Fuel,                                        !- Component 4 Object Type",
         "    Refrig Cond EC PTAC Heating Coil,                         !- Component 4 Name",
         "    Refrig Cond EC PTAC Desuperheater Outlet Node,            !- Component 4 Inlet Node Name",
         "    Refrig Cond EC PTAC Heating Coil Outlet Node,             !- Component 4 Outlet Node Name",
         "    Fan:ConstantVolume,                                       !- Component 5 Object Type",
         "    Refrig Cond EC PTAC Fan,                                  !- Component 5 Name",
         "    Refrig Cond EC PTAC Heating Coil Outlet Node,             !- Component 5 Inlet Node Name",
         "    Refrig Cond EC PTAC Supply Side Outlet Node;              !- Component 5 Outlet Node Name",
         "  AirLoopHVAC:OutdoorAirSystem,",
         "    Refrig Cond EC PTAC OA System,                            !- Name",
         "    Refrig Cond EC PTAC OA System Controller List,            !- Controller List Name",
         "    Refrig Cond EC PTAC OA System Equipment List;             !- Outdoor Air Equipment List Name",
         "  AirLoopHVAC:ControllerList,",
         "    Refrig Cond EC PTAC OA System Controller List,            !- Name",
         "    Controller:OutdoorAir,                                    !- Controller 1 Object Type",
         "    Refrig Cond EC PTAC OA Controller;                        !- Controller 1 Name",
         "  Controller:OutdoorAir,",
         "    Refrig Cond EC PTAC OA Controller,                        !- Name",
         "    Refrig Cond EC PTAC Relief Node,                          !- Relief Air Outlet Node Name",
         "    Refrig Cond EC PTAC Supply Side Inlet Node,               !- Return Air Node Name",
         "    Refrig Cond EC PTAC Mixed Air Node,                       !- Mixed Air Node Name",
         "    Refrig Cond EC PTAC OA Node,                              !- Actuator Node Name",
         "    0,                                                        !- Minimum Outdoor Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Maximum Outdoor Air Flow Rate {m3/s}",
         "    NoEconomizer,                                             !- Economizer Control Type",
         "    ModulateFlow,                                             !- Economizer Control Action Type",
         "    28,                                                       !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
         "    64000,                                                    !- Economizer Maximum Limit Enthalpy {J/kg}",
         "    ,                                                         !- Economizer Maximum Limit Dewpoint Temperature {C}",
         "    ,                                                         !- Electronic Enthalpy Limit Curve Name",
         "    -100,                                                     !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
         "    NoLockout,                                                !- Lockout Type",
         "    FixedMinimum,                                             !- Minimum Limit Type",
         "    ,                                                         !- Minimum Outdoor Air Schedule Name",
         "    ,                                                         !- Minimum Fraction of Outdoor Air Schedule Name",
         "    ,                                                         !- Maximum Fraction of Outdoor Air Schedule Name",
         "    Refrig Cond EC PTAC MV Controller,                        !- Mechanical Ventilation Controller Name",
         "    ,                                                         !- Time of Day Economizer Control Schedule Name",
         "    No,                                                       !- High Humidity Control",
         "    ,                                                         !- Humidistat Control Zone Name",
         "    ,                                                         !- High Humidity Outdoor Air Flow Ratio",
         "    Yes,                                                      !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
         "    BypassWhenWithinEconomizerLimits,                         !- Heat Recovery Bypass Control Type",
         "    InterlockedWithMechanicalCooling;                         !- Economizer Operation Staging",
         "  OutdoorAir:NodeList,",
         "    Refrig Cond EC PTAC OA Node;                              !- Node or NodeList Name 1",
         "  Controller:MechanicalVentilation,",
         "    Refrig Cond EC PTAC MV Controller,                        !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    No,                                                       !- Demand Controlled Ventilation",
         "    ZoneSum,                                                  !- System Outdoor Air Method",
         "    ,                                                         !- Zone Maximum Outdoor Air Fraction {dimensionless}",
         "    Refrig Cond EC Zone,                                      !- Zone or ZoneList 1 Name",
         "    ,                                                         !- Design Specification Outdoor Air Object Name 1",
         "    ;                                                         !- Design Specification Zone Air Distribution Object Name 1",
         "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
         "    Refrig Cond EC PTAC OA System Equipment List,             !- Name",
         "    OutdoorAir:Mixer,                                         !- Component 1 Object Type",
         "    Refrig Cond EC PTAC OA System Outdoor Air Mixer;          !- Component 1 Name",
         "  OutdoorAir:Mixer,",
         "    Refrig Cond EC PTAC OA System Outdoor Air Mixer,          !- Name",
         "    Refrig Cond EC PTAC Mixed Air Node,                       !- Mixed Air Node Name",
         "    Refrig Cond EC PTAC OA Node,                              !- Outdoor Air Stream Node Name",
         "    Refrig Cond EC PTAC Relief Node,                          !- Relief Air Stream Node Name",
         "    Refrig Cond EC PTAC Supply Side Inlet Node;               !- Return Air Stream Node Name",
         "  Coil:Cooling:DX:SingleSpeed,",
         "    Refrig Cond EC PTAC Cooling Coil,                         !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Autosize,                                                 !- Gross Rated Total Cooling Capacity {W}",
         "    Autosize,                                                 !- Gross Rated Sensible Heat Ratio",
         "    3,                                                        !- Gross Rated Cooling COP {W/W}",
         "    Autosize,                                                 !- Rated Air Flow Rate {m3/s}",
         "    773.3,                                                    !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    934.4,                                                    !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    Refrig Cond EC PTAC Mixed Air Node,                       !- Air Inlet Node Name",
         "    Refrig Cond EC PTAC Desuperheater Inlet Node,             !- Air Outlet Node Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Part Load Fraction Correlation Curve Name",
         "    -25,                                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
         "    0,                                                        !- Nominal Time for Condensate Removal to Begin {s}",
         "    0,                                                        !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
         "Capacity {dimensionless}",
         "    0,                                                        !- Maximum Cycling Rate {cycles/hr}",
         "    0,                                                        !- Latent Capacity Time Constant {s}",
         "    ,                                                         !- Condenser Air Inlet Node Name",
         "    AirCooled,                                                !- Condenser Type",
         "    0.9,                                                      !- Evaporative Condenser Effectiveness {dimensionless}",
         "    Autosize,                                                 !- Evaporative Condenser Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Evaporative Condenser Pump Rated Power Consumption {W}",
         "    0,                                                        !- Crankcase Heater Capacity {W}",
         "    ,                                                         !- Crankcase Heater Capacity Function of Temperature Curve Name",
         "    10,                                                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
         "    ,                                                         !- Supply Water Storage Tank Name",
         "    ,                                                         !- Condensate Collection Water Storage Tank Name",
         "    0,                                                        !- Basin Heater Capacity {W/K}",
         "    2;                                                        !- Basin Heater Setpoint Temperature {C}",
         "  Coil:Heating:Desuperheater,",
         "    Refrig Cond EC PTAC Desuperheater,                        !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.25,                                                     !- Refrig Cond EC PTAC Desuperheater Recovery Efficiency",
         "    Refrig Cond EC PTAC Desuperheater Inlet Node,             !- Air Inlet Node Name",
         "    Refrig Cond EC PTAC Desuperheater Outlet Node,            !- Air Outlet Node Name",
         "    Refrigeration:Condenser:EvaporativeCooled,                !- Heating Source Object Type",
         "    Refrig Cond EC Refrig Condenser,                          !- Heating Source Name",
         "    Refrig Cond EC PTAC Desuperheater Outlet Node;            !- Temperature Setpoint Node Name",
         "  Coil:Heating:Fuel,",
         "    Refrig Cond EC PTAC Heating Coil,                         !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    NaturalGas,                                               !- Fuel Type",
         "    0.9,                                                      !- Burner Efficiency",
         "    AutoSize,                                                 !- Nominal Capacity {W}",
         "    Refrig Cond EC PTAC Desuperheater Outlet Node,            !- Air Inlet Node Name",
         "    Refrig Cond EC PTAC Heating Coil Outlet Node,             !- Air Outlet Node Name",
         "    Refrig Cond EC PTAC Heating Coil Outlet Node,             !- Temperature Setpoint Node Name",
         "    0,                                                        !- On Cycle Parasitic Electric Load {W}",
         "    ,                                                         !- Part Load Fraction Correlation Curve Name",
         "    0;                                                        !- Off Cycle Parasitic Fuel Load {W}",
         "  Fan:ConstantVolume,",
         "    Refrig Cond EC PTAC Fan,                                  !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.7,                                                      !- Fan Total Efficiency",
         "    500,                                                      !- Pressure Rise {Pa}",
         "    AutoSize,                                                 !- Maximum Flow Rate {m3/s}",
         "    0.9,                                                      !- Motor Efficiency",
         "    1,                                                        !- Motor In Airstream Fraction",
         "    Refrig Cond EC PTAC Heating Coil Outlet Node,             !- Air Inlet Node Name",
         "    Refrig Cond EC PTAC Supply Side Outlet Node;              !- Air Outlet Node Name",
         "  Refrigeration:System,",
         "    Refrig Cond EC Refrig System,                             !- Name",
         "    Refrig Cond EC CaseAndWalkInList,                         !- Refrigerated Case or Walkin or CaseAndWalkInList Name",
         "    ,                                                         !- Refrigeration Transfer Load or TransferLoad List Name",
         "    Refrig Cond EC Refrig Condenser,                          !- Refrigeration Condenser Name",
         "    Refrig Cond EC Refrig Compressor,                         !- Compressor or CompressorList Name",
         "    20,                                                       !- Minimum Condensing Temperature {C}",
         "    R407a,                                                    !- Refrigeration System Working Fluid Type",
         "    ConstantSuctionTemperature,                               !- Suction Temperature Control Type",
         "    ,                                                         !- Mechanical Subcooler Name",
         "    ,                                                         !- Liquid Suction Heat Exchanger Subcooler Name",
         "    0,                                                        !- Sum UA Suction Piping {W/K}",
         "    Refrig Cond EC Zone,                                      !- Suction Piping Zone Name",
         "    General,                                                  !- End-Use Subcategory",
         "    1,                                                        !- Number of Compressor Stages",
         "    None,                                                     !- Intercooler Type",
         "    0.8;                                                      !- Shell-and-Coil Intercooler Effectiveness",
         "  Refrigeration:CaseAndWalkInList,",
         "    Refrig Cond EC CaseAndWalkInList,                         !- Name",
         "    Refrig Cond EC Case,                                      !- Case or Refrig Cond EC WalkIn Name",
         "    Refrig Cond EC WalkIn;                                    !- Case or WalkIn 2 Name",
         "  Refrigeration:Case,",
         "    Refrig Cond EC Case,                                      !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Cond EC Zone,                                      !- Zone Name",
         "    24,                                                       !- Rated Ambient Temperature {C}",
         "    55,                                                       !- Rated Ambient Relative Humidity {percent}",
         "    1406,                                                     !- Rated Total Cooling Capacity per Unit Length {W/m}",
         "    0.3,                                                      !- Rated Latent Heat Ratio",
         "    0.85,                                                     !- Rated Runtime Fraction",
         "    2.19,                                                     !- Case Length {m}",
         "    4,                                                        !- Case Operating Temperature {C}",
         "    CaseTemperatureMethod,                                    !- Latent Case Credit Curve Type",
         "    Generic Curve,                                            !- Latent Case Credit Curve Name",
         "    30,                                                       !- Standard Case Fan Power per Unit Length {W/m}",
         "    30,                                                       !- Operating Case Fan Power per Unit Length {W/m}",
         "    20,                                                       !- Standard Case Lighting Power per Unit Length {W/m}",
         "    20,                                                       !- Installed Case Lighting Power per Unit Length {W/m}",
         "    ,                                                         !- Case Lighting Schedule Name",
         "    1,                                                        !- Fraction of Lighting Energy to Case",
         "    0,                                                        !- Case Anti-Sweat Heater Power per Unit Length {W/m}",
         "    0,                                                        !- Minimum Anti-Sweat Heater Power per Unit Length {W/m}",
         "    None,                                                     !- Anti-Sweat Heater Control Type",
         "    -10,                                                      !- Humidity at Zero Anti-Sweat Heater Energy {percent}",
         "    1.5,                                                      !- Case Height {m}",
         "    1,                                                        !- Fraction of Anti-Sweat Heater Energy to Case",
         "    0,                                                        !- Case Defrost Power per Unit Length {W/m}",
         "    OffCycle,                                                 !- Case Defrost Type",
         "    AlwaysOn,                                                 !- Case Defrost Schedule Name",
         "    ,                                                         !- Case Defrost Drip-Down Schedule Name",
         "    None,                                                     !- Defrost Energy Correction Curve Type",
         "    ,                                                         !- Defrost Energy Correction Curve Name",
         "    0,                                                        !- Under Case HVAC Return Air Fraction",
         "    ,                                                         !- Refrigerated Case Restocking Schedule Name",
         "    ,                                                         !- Case Credit Fraction Schedule Name",
         "    -7,                                                       !- Design Evaporator Temperature or Brine Inlet Temperature {C}",
         "    0;                                                        !- Average Refrigerant Charge Inventory {kg/m}",
         "  Refrigeration:WalkIn,",
         "    Refrig Cond EC WalkIn,                                    !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    4744,                                                     !- Rated Coil Cooling Capacity {W}",
         "    4,                                                        !- Operating Temperature {C}",
         "    -6,                                                       !- Rated Cooling Source Temperature {C}",
         "    0,                                                        !- Rated Total Heating Power {W}",
         "    ,                                                         !- Heating Power Schedule Name",
         "    308,                                                      !- Rated Cooling Coil Fan Power {W}",
         "    0,                                                        !- Rated Circulation Fan Power {W}",
         "    100,                                                      !- Rated Total Lighting Power {W}",
         "    ,                                                         !- Lighting Schedule Name",
         "    Electric,                                                 !- Defrost Type",
         "    TimeSchedule,                                             !- Defrost Control Type",
         "    AlwaysOn,                                                 !- Defrost Schedule Name",
         "    ,                                                         !- Defrost Drip-Down Schedule Name",
         "    2400,                                                     !- Defrost Power {W}",
         "    ,                                                         !- Temperature Termination Defrost Fraction to Ice {dimensionless}",
         "    ,                                                         !- Restocking Schedule Name",
         "    0,                                                        !- Average Refrigerant Charge Inventory {kg}",
         "    13,                                                       !- Insulated Floor Surface Area {m2}",
         "    0.25,                                                     !- Insulated Floor U-Value {W/m2-K}",
         "    Refrig Cond EC Zone,                                      !- Zone 1 Name",
         "    55,                                                       !- Total Insulated Surface Area Facing Zone 1 {m2}",
         "    0.3,                                                      !- Insulated Surface U-Value Facing Zone 1 {W/m2-K}",
         "    0,                                                        !- Area of Glass Reach In Doors Facing Zone 1 {m2}",
         "    1.5,                                                      !- Height of Glass Reach In Doors Facing Zone 1 {m}",
         "    1.136,                                                    !- Glass Reach In Door U Value Facing Zone 1 {W/m2-K}",
         "    ,                                                         !- Glass Reach In Door Opening Schedule Name Facing Zone 1",
         "    2,                                                        !- Area of Stocking Doors Facing Zone 1 {m2}",
         "    2,                                                        !- Height of Stocking Doors Facing Zone 1 {m}",
         "    0.3785,                                                   !- Stocking Door U Value Facing Zone 1 {W/m2-K}",
         "    ,                                                         !- Stocking Door Opening Schedule Name Facing Zone 1",
         "    None;                                                     !- Stocking Door Opening Protection Type Facing Zone 1",
         "  Refrigeration:Condenser:EvaporativeCooled,",
         "    Refrig Cond EC Refrig Condenser,                          !- Name",
         "    107900.,                                                  !- Rated Effective Total Heat Rejection Rate {W}",
         "    0.,                                                       !- Rated Subcooling Temperature Difference {deltaC}",
         "    VariableSpeed,                                            !- Fan Speed Control Type",
         "    1119.,                                                    !- Rated Fan Power {W}",
         "    ,                                                         !- Minimum Fan Air Flow Ratio {dimensionless}",
         "    6.63,                                                     !- Approach Temperature Constant Term {C}",
         "    0.468,                                                    !- Approach Temperature Coefficient 2 {C}",
         "    17.93,                                                    !- Approach Temperature Coefficient 3 {C}",
         "    -0.322,                                                   !- Approach Temperature Coefficient 4 {dimensionless}",
         "    0.6,                                                      !- Minimum Capacity Factor {dimensionless}",
         "    4.8,                                                      !- Maximum Capacity Factor {dimensionless}",
         "    Refrig Cond EC Outdoor Air Node,                          !- Air Inlet Node Name",
         "    3.16,                                                     !- Rated Air Flow Rate {m3/s}",
         "    66.7,                                                     !- Basin Heater Capacity {W/K}",
         "    2.0,                                                      !- Basin Heater Setpoint Temperature {C}",
         "    250.,                                                     !- Rated Water Pump Power {W}",
         "    ,                                                         !- Evaporative Water Supply Tank Name",
         "    ,                                                         !- Evaporative Condenser Availability Schedule Name",
         "    ,                                                         !- End-Use Subcategory",
         "    38.5,                                                     !- Condenser Refrigerant Operating Charge Inventory {kg}",
         "    20.,                                                      !- Condensate Receiver Refrigerant Inventory {kg}",
         "    25.;                                                      !- Condensate Piping Refrigerant Inventory {kg}",
         "  OutdoorAir:Node,",
         "    Refrig Cond EC Outdoor Air Node;                          !- Name",
         "  Refrigeration:Compressor,",
         "    Refrig Cond EC Refrig Compressor,                         !- Name",
         "    Generic Curve,                                            !- Refrigeration Compressor Power Curve Name",
         "    Capacity Curve,                                           !- Refrigeration Compressor Capacity Curve Name",
         "    ,                                                         !- Rated Superheat {deltaC}",
         "    18.3,                                                     !- Rated Return Gas Temperature {C}",
         "    ,                                                         !- Rated Liquid Temperature {C}",
         "    0,                                                        !- Rated Subcooling {deltaC}",
         "    General,                                                  !- End-Use Subcategory",
         "    Subcritical;                                              !- Mode of Operation",

         "  Zone,",
         "    Refrig Comp Rack Zone,                                    !- Name",
         "    ,                                                         !- Direction of Relative North {deg}",
         "    0,                                                        !- X Origin {m}",
         "    0,                                                        !- Y Origin {m}",
         "    0,                                                        !- Z Origin {m}",
         "    ,                                                         !- Type",
         "    1,                                                        !- Multiplier",
         "    ,                                                         !- Ceiling Height {m}",
         "    ,                                                         !- Volume {m3}",
         "    ,                                                         !- Floor Area {m2}",
         "    ,                                                         !- Zone Inside Convection Algorithm",
         "    ,                                                         !- Zone Outside Convection Algorithm",
         "    Yes;                                                      !- Part of Total Floor Area",
         "  ZoneHVAC:EquipmentConnections,",
         "    Refrig Comp Rack Zone,                                    !- Zone Name",
         "    Refrig Comp Rack Zone Equipment List,                     !- Zone Conditioning Equipment List Name",
         "    Refrig Comp Rack Zone Inlet Node,                         !- Zone Air Inlet Node or NodeList Name",
         "    Refrig Comp Rack Zone Exhaust Node,                       !- Zone Air Exhaust Node or NodeList Name",
         "    Refrig Comp Rack Zone Air Node,                           !- Zone Air Node Name",
         "    Refrig Comp Rack Zone Return Node;                        !- Zone Return Air Node or NodeList Name",
         "  ZoneHVAC:EquipmentList,",
         "    Refrig Comp Rack Zone Equipment List,                     !- Name",
         "    SequentialLoad,                                           !- Load Distribution Scheme",
         "    ZoneHVAC:AirDistributionUnit,                             !- Zone Equipment 1 Object Type",
         "    Refrig Comp Rack ADU,                                     !- Zone Equipment 1 Name",
         "    1,                                                        !- Zone Equipment 1 Cooling Sequence",
         "    1,                                                        !- Zone Equipment 1 Heating or No-Load Sequence",
         "    ,                                                         !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
         "    ;                                                         !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
         "  ZoneHVAC:AirDistributionUnit,",
         "    Refrig Comp Rack ADU,                                     !- Name",
         "    Refrig Comp Rack Zone Inlet Node,                         !- Air Distribution Unit Outlet Node Name",
         "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,           !- Air Terminal Object Type",
         "    Refrig Comp Rack Air Terminal;                            !- Air Terminal Name",
         "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
         "    Refrig Comp Rack Air Terminal,                            !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Comp Rack Air Terminal Inlet Node,                 !- Air Inlet Node Name",
         "    Refrig Comp Rack Zone Inlet Node,                         !- Air Outlet Node Name",
         "    AutoSize;                                                 !- Maximum Air Flow Rate {m3/s}",
         "  AirLoopHVAC,",
         "    Refrig Comp Rack PTAC,                                    !- Name",
         "    ,                                                         !- Controller List Name",
         "    ,                                                         !- Availability Manager List Name",
         "    AutoSize,                                                 !- Design Supply Air Flow Rate {m3/s}",
         "    Refrig Comp Rack PTAC Supply Branch List,                 !- Branch List Name",
         "    ,                                                         !- Connector List Name",
         "    Refrig Comp Rack PTAC Supply Side Inlet Node,             !- Supply Side Inlet Node Name",
         "    Refrig Comp Rack PTAC Demand Side Outlet Node,            !- Demand Side Outlet Node Name",
         "    Refrig Comp Rack PTAC Demand Side Inlet Node,             !- Demand Side Inlet Node Names",
         "    Refrig Comp Rack PTAC Supply Side Outlet Node,            !- Supply Side Outlet Node Names",
         "    1;                                                        !- Design Return Air Flow Fraction of Supply Air Flow",
         "  BranchList,",
         "    Refrig Comp Rack PTAC Supply Branch List,                 !- Name",
         "    Refrig Comp Rack PTAC Branch;                             !- Branch 1 Name",
         "  Branch,",
         "    Refrig Comp Rack PTAC Branch,                             !- Name",
         "    ,                                                         !- Pressure Drop Curve Name",
         "    AirLoopHVAC:OutdoorAirSystem,                             !- Component 1 Object Type",
         "    Refrig Comp Rack PTAC OA System,                          !- Component 1 Name",
         "    Refrig Comp Rack PTAC Supply Side Inlet Node,             !- Component 1 Inlet Node Name",
         "    Refrig Comp Rack PTAC Mixed Air Node,                     !- Component 1 Outlet Node Name",
         "    Coil:Cooling:DX:SingleSpeed,                              !- Component 2 Object Type",
         "    Refrig Comp Rack PTAC Cooling Coil,                       !- Component 2 Name",
         "    Refrig Comp Rack PTAC Mixed Air Node,                     !- Component 2 Inlet Node Name",
         "    Refrig Comp Rack PTAC Desuperheater Inlet Node,           !- Component 2 Outlet Node Name",
         "    Coil:Heating:Desuperheater,                               !- Component 3 Object Type",
         "    Refrig Comp Rack PTAC Desuperheater,                      !- Component 3 Name",
         "    Refrig Comp Rack PTAC Desuperheater Inlet Node,           !- Component 3 Inlet Node Name",
         "    Refrig Comp Rack PTAC Desuperheater Outlet Node,          !- Component 3 Outlet Node Name",
         "    Coil:Heating:Fuel,                                        !- Component 4 Object Type",
         "    Refrig Comp Rack PTAC Heating Coil,                       !- Component 4 Name",
         "    Refrig Comp Rack PTAC Desuperheater Outlet Node,          !- Component 4 Inlet Node Name",
         "    Refrig Comp Rack PTAC Heating Coil Outlet Node,           !- Component 4 Outlet Node Name",
         "    Fan:ConstantVolume,                                       !- Component 5 Object Type",
         "    Refrig Comp Rack PTAC Fan,                                !- Component 5 Name",
         "    Refrig Comp Rack PTAC Heating Coil Outlet Node,           !- Component 5 Inlet Node Name",
         "    Refrig Comp Rack PTAC Supply Side Outlet Node;            !- Component 5 Outlet Node Name",
         "  AirLoopHVAC:OutdoorAirSystem,",
         "    Refrig Comp Rack PTAC OA System,                          !- Name",
         "    Refrig Comp Rack PTAC OA System Controller List,          !- Controller List Name",
         "    Refrig Comp Rack PTAC OA System Equipment List;           !- Outdoor Air Equipment List Name",
         "  AirLoopHVAC:ControllerList,",
         "    Refrig Comp Rack PTAC OA System Controller List,          !- Name",
         "    Controller:OutdoorAir,                                    !- Controller 1 Object Type",
         "    Refrig Comp Rack PTAC OA Controller;                      !- Controller 1 Name",
         "  Controller:OutdoorAir,",
         "    Refrig Comp Rack PTAC OA Controller,                      !- Name",
         "    Refrig Comp Rack PTAC Relief Node,                        !- Relief Air Outlet Node Name",
         "    Refrig Comp Rack PTAC Supply Side Inlet Node,             !- Return Air Node Name",
         "    Refrig Comp Rack PTAC Mixed Air Node,                     !- Mixed Air Node Name",
         "    Refrig Comp Rack PTAC OA Node,                            !- Actuator Node Name",
         "    0,                                                        !- Minimum Outdoor Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Maximum Outdoor Air Flow Rate {m3/s}",
         "    NoEconomizer,                                             !- Economizer Control Type",
         "    ModulateFlow,                                             !- Economizer Control Action Type",
         "    28,                                                       !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
         "    64000,                                                    !- Economizer Maximum Limit Enthalpy {J/kg}",
         "    ,                                                         !- Economizer Maximum Limit Dewpoint Temperature {C}",
         "    ,                                                         !- Electronic Enthalpy Limit Curve Name",
         "    -100,                                                     !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
         "    NoLockout,                                                !- Lockout Type",
         "    FixedMinimum,                                             !- Minimum Limit Type",
         "    ,                                                         !- Minimum Outdoor Air Schedule Name",
         "    ,                                                         !- Minimum Fraction of Outdoor Air Schedule Name",
         "    ,                                                         !- Maximum Fraction of Outdoor Air Schedule Name",
         "    Refrig Comp Rack PTAC MV Controller,                      !- Mechanical Ventilation Controller Name",
         "    ,                                                         !- Time of Day Economizer Control Schedule Name",
         "    No,                                                       !- High Humidity Control",
         "    ,                                                         !- Humidistat Control Zone Name",
         "    ,                                                         !- High Humidity Outdoor Air Flow Ratio",
         "    Yes,                                                      !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
         "    BypassWhenWithinEconomizerLimits,                         !- Heat Recovery Bypass Control Type",
         "    InterlockedWithMechanicalCooling;                         !- Economizer Operation Staging",
         "  OutdoorAir:NodeList,",
         "    Refrig Comp Rack PTAC OA Node;                            !- Node or NodeList Name 1",
         "  Controller:MechanicalVentilation,",
         "    Refrig Comp Rack PTAC MV Controller,                      !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    No,                                                       !- Demand Controlled Ventilation",
         "    ZoneSum,                                                  !- System Outdoor Air Method",
         "    ,                                                         !- Zone Maximum Outdoor Air Fraction {dimensionless}",
         "    Refrig Comp Rack Zone,                                    !- Zone or ZoneList 1 Name",
         "    ,                                                         !- Design Specification Outdoor Air Object Name 1",
         "    ;                                                         !- Design Specification Zone Air Distribution Object Name 1",
         "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
         "    Refrig Comp Rack PTAC OA System Equipment List,           !- Name",
         "    OutdoorAir:Mixer,                                         !- Component 1 Object Type",
         "    Refrig Comp Rack PTAC OA System Outdoor Air Mixer;        !- Component 1 Name",
         "  OutdoorAir:Mixer,",
         "    Refrig Comp Rack PTAC OA System Outdoor Air Mixer,        !- Name",
         "    Refrig Comp Rack PTAC Mixed Air Node,                     !- Mixed Air Node Name",
         "    Refrig Comp Rack PTAC OA Node,                            !- Outdoor Air Stream Node Name",
         "    Refrig Comp Rack PTAC Relief Node,                        !- Relief Air Stream Node Name",
         "    Refrig Comp Rack PTAC Supply Side Inlet Node;             !- Return Air Stream Node Name",
         "  Coil:Cooling:DX:SingleSpeed,",
         "    Refrig Comp Rack PTAC Cooling Coil,                       !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Autosize,                                                 !- Gross Rated Total Cooling Capacity {W}",
         "    Autosize,                                                 !- Gross Rated Sensible Heat Ratio",
         "    3,                                                        !- Gross Rated Cooling COP {W/W}",
         "    Autosize,                                                 !- Rated Air Flow Rate {m3/s}",
         "    773.3,                                                    !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    934.4,                                                    !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
         "    Refrig Comp Rack PTAC Mixed Air Node,                     !- Air Inlet Node Name",
         "    Refrig Comp Rack PTAC Desuperheater Inlet Node,           !- Air Outlet Node Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Total Cooling Capacity Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Temperature Curve Name",
         "    Generic Curve,                                            !- Energy Input Ratio Function of Flow Fraction Curve Name",
         "    Generic Curve,                                            !- Part Load Fraction Correlation Curve Name",
         "    -25,                                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
         "    0,                                                        !- Nominal Time for Condensate Removal to Begin {s}",
         "    0,                                                        !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
         "Capacity {dimensionless}",
         "    0,                                                        !- Maximum Cycling Rate {cycles/hr}",
         "    0,                                                        !- Latent Capacity Time Constant {s}",
         "    ,                                                         !- Condenser Air Inlet Node Name",
         "    AirCooled,                                                !- Condenser Type",
         "    0.9,                                                      !- Evaporative Condenser Effectiveness {dimensionless}",
         "    Autosize,                                                 !- Evaporative Condenser Air Flow Rate {m3/s}",
         "    Autosize,                                                 !- Evaporative Condenser Pump Rated Power Consumption {W}",
         "    0,                                                        !- Crankcase Heater Capacity {W}",
         "    ,                                                         !- Crankcase Heater Capacity Function of Temperature Curve Name",
         "    10,                                                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
         "    ,                                                         !- Supply Water Storage Tank Name",
         "    ,                                                         !- Condensate Collection Water Storage Tank Name",
         "    0,                                                        !- Basin Heater Capacity {W/K}",
         "    2;                                                        !- Basin Heater Setpoint Temperature {C}",
         "  Coil:Heating:Desuperheater,",
         "    Refrig Comp Rack PTAC Desuperheater,                      !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.25,                                                     !- Refrig Comp Rack PTAC Desuperheater Recovery Efficiency",
         "    Refrig Comp Rack PTAC Desuperheater Inlet Node,           !- Air Inlet Node Name",
         "    Refrig Comp Rack PTAC Desuperheater Outlet Node,          !- Air Outlet Node Name",
         "    Refrigeration:CompressorRack,                             !- Heating Source Object Type",
         "    Refrig Comp Rack Refrig Rack,                             !- Heating Source Name",
         "    Refrig Comp Rack PTAC Desuperheater Outlet Node;          !- Temperature Setpoint Node Name",
         "  Coil:Heating:Fuel,",
         "    Refrig Comp Rack PTAC Heating Coil,                       !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    NaturalGas,                                               !- Fuel Type",
         "    0.9,                                                      !- Burner Efficiency",
         "    AutoSize,                                                 !- Nominal Capacity {W}",
         "    Refrig Comp Rack PTAC Desuperheater Outlet Node,          !- Air Inlet Node Name",
         "    Refrig Comp Rack PTAC Heating Coil Outlet Node,           !- Air Outlet Node Name",
         "    Refrig Comp Rack PTAC Heating Coil Outlet Node,           !- Temperature Setpoint Node Name",
         "    0,                                                        !- On Cycle Parasitic Electric Load {W}",
         "    ,                                                         !- Part Load Fraction Correlation Curve Name",
         "    0;                                                        !- Off Cycle Parasitic Fuel Load {W}",
         "  Fan:ConstantVolume,",
         "    Refrig Comp Rack PTAC Fan,                                !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    0.7,                                                      !- Fan Total Efficiency",
         "    500,                                                      !- Pressure Rise {Pa}",
         "    AutoSize,                                                 !- Maximum Flow Rate {m3/s}",
         "    0.9,                                                      !- Motor Efficiency",
         "    1,                                                        !- Motor In Airstream Fraction",
         "    Refrig Comp Rack PTAC Heating Coil Outlet Node,           !- Air Inlet Node Name",
         "    Refrig Comp Rack PTAC Supply Side Outlet Node;            !- Air Outlet Node Name",
         "  Refrigeration:CompressorRack,",
         "    Refrig Comp Rack Refrig Rack,                             !- Name",
         "    Zone,                                                     !- Heat Rejection Location",
         "    4.0,                                                      !- Design Compressor Rack COP {W/W}",
         "    Condenser Curve,                                          !- Compressor Rack COP Function of Temperature Curve Name",
         "    175.0,                                                    !- Design Condenser Fan Power {W}",
         "    ,                                                         !- Condenser Fan Power Function of Temperature Curve Name",
         "    AirCooled,                                                !- Condenser Type",
         "    ,                                                         !- Water-Cooled Condenser Inlet Node Name",
         "    ,                                                         !- Water-Cooled Condenser Outlet Node Name",
         "    ,                                                         !- Water-Cooled Loop Flow Type",
         "    ,                                                         !- Water-Cooled Condenser Outlet Temperature Schedule Name",
         "    ,                                                         !- Water-Cooled Condenser Design Flow Rate {m3/s}",
         "    ,                                                         !- Water-Cooled Condenser Maximum Flow Rate {m3/s}",
         "    ,                                                         !- Water-Cooled Condenser Maximum Water Outlet Temperature {C}",
         "    ,                                                         !- Water-Cooled Condenser Minimum Water Inlet Temperature {C}",
         "    ,                                                         !- Evaporative Condenser Availability Schedule Name",
         "    ,                                                         !- Evaporative Condenser Effectiveness {dimensionless}",
         "    ,                                                         !- Evaporative Condenser Air Flow Rate {m3/s}",
         "    ,                                                         !- Basin Heater Capacity {W/K}",
         "    ,                                                         !- Basin Heater Setpoint Temperature {C}",
         "    ,                                                         !- Design Evaporative Condenser Water Pump Power {W}",
         "    ,                                                         !- Evaporative Water Supply Tank Name",
         "    ,                                                         !- Condenser Air Inlet Node Name",
         "    ,                                                         !- End-Use Subcategory",
         "    Refrig Comp Rack Case;                                    !- Refrigeration Case Name or WalkIn Name or CaseAndWalkInList Name",
         "  Refrigeration:Case,",
         "    Refrig Comp Rack Case,                                    !- Name",
         "    ,                                                         !- Availability Schedule Name",
         "    Refrig Comp Rack Zone,                                    !- Zone Name",
         "    24,                                                       !- Rated Ambient Temperature {C}",
         "    55,                                                       !- Rated Ambient Relative Humidity {percent}",
         "    1406,                                                     !- Rated Total Cooling Capacity per Unit Length {W/m}",
         "    0.3,                                                      !- Rated Latent Heat Ratio",
         "    0.85,                                                     !- Rated Runtime Fraction",
         "    2.19,                                                     !- Case Length {m}",
         "    4,                                                        !- Case Operating Temperature {C}",
         "    CaseTemperatureMethod,                                    !- Latent Case Credit Curve Type",
         "    Generic Curve,                                            !- Latent Case Credit Curve Name",
         "    30,                                                       !- Standard Case Fan Power per Unit Length {W/m}",
         "    30,                                                       !- Operating Case Fan Power per Unit Length {W/m}",
         "    20,                                                       !- Standard Case Lighting Power per Unit Length {W/m}",
         "    20,                                                       !- Installed Case Lighting Power per Unit Length {W/m}",
         "    ,                                                         !- Case Lighting Schedule Name",
         "    1,                                                        !- Fraction of Lighting Energy to Case",
         "    0,                                                        !- Case Anti-Sweat Heater Power per Unit Length {W/m}",
         "    0,                                                        !- Minimum Anti-Sweat Heater Power per Unit Length {W/m}",
         "    None,                                                     !- Anti-Sweat Heater Control Type",
         "    -10,                                                      !- Humidity at Zero Anti-Sweat Heater Energy {percent}",
         "    1.5,                                                      !- Case Height {m}",
         "    1,                                                        !- Fraction of Anti-Sweat Heater Energy to Case",
         "    0,                                                        !- Case Defrost Power per Unit Length {W/m}",
         "    OffCycle,                                                 !- Case Defrost Type",
         "    AlwaysOn,                                                 !- Case Defrost Schedule Name",
         "    ,                                                         !- Case Defrost Drip-Down Schedule Name",
         "    None,                                                     !- Defrost Energy Correction Curve Type",
         "    ,                                                         !- Defrost Energy Correction Curve Name",
         "    0,                                                        !- Under Case HVAC Return Air Fraction",
         "    ,                                                         !- Refrigerated Case Restocking Schedule Name",
         "    ,                                                         !- Case Credit Fraction Schedule Name",
         "    -7,                                                       !- Design Evaporator Temperature or Brine Inlet Temperature {C}",
         "    0;                                                        !- Average Refrigerant Charge Inventory {kg/m}",

         "  ScheduleTypeLimits,",
         "    OnOff,                                                    !- Name",
         "    0,                                                        !- Lower Limit Value",
         "    1,                                                        !- Upper Limit Value",
         "    Discrete,                                                 !- Numeric Type",
         "    Availability;                                             !- Unit Type",
         "  Schedule:Constant,",
         "    AlwaysOn,                                                 !- Name",
         "    OnOff,                                                    !- Schedule Type Limits Name",
         "    1;                                                        !- Hourly Value",
         "  Curve:Cubic,",
         "    Generic Curve,                                            !- Name",
         "    1,                                                        !- Coefficient1 Constant",
         "    0,                                                        !- Coefficient2 x",
         "    0,                                                        !- Coefficient3 x**2",
         "    0,                                                        !- Coefficient4 x**3",
         "    0,                                                        !- Minimum Value of x",
         "    1;                                                        !- Maximum Value of x",
         "  Curve:Linear,",
         "    Condenser Curve,                                          !- Name",
         "    0,                                                        !- Coefficient1 Constant",
         "    14704,                                                    !- Coefficient2 x",
         "    5,                                                        !- Minimum Value of x",
         "    22.2;                                                     !- Maximum Value of x",
         "  Curve:Bicubic,",
         "    Capacity Curve,                                           !- Name",
         "    22520,                                                    !- Coefficient1 Constant",
         "    755.9,                                                    !- Coefficient2 x",
         "    9.047,                                                    !- Coefficient3 x**2",
         "    -69.75,                                                   !- Coefficient4 y",
         "    -2.849,                                                   !- Coefficient5 y**2",
         "    -3.813,                                                   !- Coefficient6 x*y",
         "    0.02657,                                                  !- Coefficient7 x**3",
         "    0.01137,                                                  !- Coefficient8 y**3",
         "    -0.06655,                                                 !- Coefficient9 x**2*y",
         "    -0.04638,                                                 !- Coefficient10 x*y**2",
         "    -17.8,                                                    !- Minimum Value of x",
         "    4.4,                                                      !- Maximum Value of x",
         "    10,                                                       !- Minimum Value of y",
         "    48.9,                                                     !- Maximum Value of y",
         "    ,                                                         !- Minimum Curve Output",
         "    ,                                                         !- Maximum Curve Output",
         "    Temperature,                                              !- Input Unit Type for X",
         "    Temperature,                                              !- Input Unit Type for Y",
         "    Capacity;                                                 !- Output Unit Type",
         "  FluidProperties:Name,",
         "    R407a,                       !- Fluid Name",
         "    Refrigerant;                 !- Fluid Type",
         "  FluidProperties:Temperatures,",
         "    R407aSaturatedTemperatures,  !- Name",
         "    -45,-40,-35,-30,-25,-20,-15,",
         "    -10,-5,0,5,10,15,20,",
         "    25,30,35,40,45,50,55,",
         "    60,65,70,75;",
         " FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    Pressure,                !- Fluid Property Type",
         "    FluidGas,                !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    7.3063E+4,9.4480E+4,1.2060E+5,1.5212E+5,1.8976E+5,2.3430E+5,2.8655E+5,",
         "    3.4737E+5,4.1766E+5,4.9834E+5,5.9040E+5,6.9485E+5,8.1275E+5,9.4522E+5,",
         "    1.0934E+6,1.2586E+6,1.4420E+6,1.6451E+6,1.8693E+6,2.1165E+6,2.3884E+6,",
         "    2.6873E+6,3.0160E+6,3.3784E+6,3.7806E+6;",
         "  FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    Fluid,                   !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    1.3999E+5,1.4644E+5,1.5293E+5,1.5947E+5,1.6607E+5,1.7272E+5,1.7943E+5,",
         "    1.8621E+5,1.9306E+5,2.0000E+5,2.0703E+5,2.1415E+5,2.2138E+5,2.2874E+5,",
         "    2.3623E+5,2.4386E+5,2.5167E+5,2.5968E+5,2.6790E+5,2.7640E+5,2.8523E+5,",
         "    2.9449E+5,3.0434E+5,3.1509E+5,3.2749E+5;",
         "  FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    FluidGas,                !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    3.7182E+5,3.7473E+5,3.7761E+5,3.8044E+5,3.8322E+5,3.8595E+5,3.8861E+5,",
         "    3.9121E+5,3.9372E+5,3.9615E+5,3.9847E+5,4.0069E+5,4.0278E+5,4.0473E+5,",
         "    4.0651E+5,4.0811E+5,4.0948E+5,4.1060E+5,4.1141E+5,4.1185E+5,4.1183E+5,",
         "    4.1121E+5,4.0977E+5,4.0711E+5,4.0230E+5;",
         "  FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    Fluid,                   !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    1.4033E+3,1.3874E+3,1.3712E+3,1.3548E+3,1.3381E+3,1.3211E+3,1.3037E+3,",
         "    1.2859E+3,1.2677E+3,1.2489E+3,1.2296E+3,1.2097E+3,1.1890E+3,1.1675E+3,",
         "    1.1451E+3,1.1216E+3,1.0969E+3,1.0707E+3,1.0427E+3,1.0125E+3,9.7962E+2,",
         "    9.4304E+2,9.0120E+2,8.5099E+2,7.8510E+2;",
         "  FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    FluidGas,                !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    3.5889E+0,4.5710E+0,5.7544E+0,7.1675E+0,8.8412E+0,1.0810E+1,1.3110E+1,",
         "    1.5784E+1,1.8877E+1,2.2441E+1,2.6535E+1,3.1226E+1,3.6594E+1,4.2731E+1,",
         "    4.9749E+1,5.7782E+1,6.7001E+1,7.7620E+1,8.9921E+1,1.0429E+2,1.2126E+2,",
         "    1.4166E+2,1.6682E+2,1.9924E+2,2.4458E+2;",
         "  FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    SpecificHeat,            !- Fluid Property Type",
         "    Fluid,                   !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    1.2846E+3,1.2924E+3,1.3011E+3,1.3107E+3,1.3214E+3,1.3331E+3,1.3461E+3,",
         "    1.3604E+3,1.3762E+3,1.3937E+3,1.4133E+3,1.4352E+3,1.4598E+3,1.4879E+3,",
         "    1.5200E+3,1.5572E+3,1.6010E+3,1.6536E+3,1.7182E+3,1.8006E+3,1.9107E+3,",
         "    2.0682E+3,2.317E+3,2.7759E+3,3.9105E+3;",
         "  FluidProperties:Saturated,",
         "    R407a,                   !- Fluid Name",
         "    SpecificHeat,            !- Fluid Property Type",
         "    FluidGas,                !- Fluid Phase",
         "    R407aSaturatedTemperatures,  !- Temperature Values Name",
         "    7.5047E+2,7.6821E+2,7.8690E+2,8.0660E+2,8.2736E+2,8.4927E+2,8.7244E+2,",
         "    8.9701E+2,9.2314E+2,9.5103E+2,9.8096E+2,1.0133E+3,1.0487E+3,1.0879E+3,",
         "    1.1322E+3,1.1831E+3,1.2428E+3,1.3144E+3,1.4025E+3,1.5141E+3,1.6614E+3,",
         "    1.8666E+3,2.1760E+3,2.7055E+3,3.8467E+3;",
         "  FluidProperties:Temperatures,",
         "    R407aSuperHeatTemperatures,  !- Name",
         "    -45,-40,-35,-30,-25,-20,-15,",
         "    -10,-5,0,5,10,15,20,",
         "    25,30,35,40,45,50,55,",
         "    60,65,70,75,85,90,95,",
         "    100,105,110,115,120,125,130,",
         "    135,140,145,150,155,160,165,",
         "    170,175,180,185,190,195,200,",
         "    205;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.0000E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,3.7837E+5,3.8222E+5,3.8608E+5,3.8995E+5,3.9384E+5,",
         "    3.9776E+5,4.0170E+5,4.0566E+5,4.0966E+5,4.1369E+5,4.1775E+5,4.2184E+5,",
         "    4.2597E+5,4.3013E+5,4.3433E+5,4.3857E+5,4.4283E+5,4.4714E+5,4.5148E+5,",
         "    4.5586E+5,4.6028E+5,4.6473E+5,4.6922E+5,4.7831E+5,4.8291E+5,4.8754E+5,",
         "    4.9222E+5,4.9693E+5,5.0167E+5,5.0646E+5,5.1128E+5,5.1613E+5,5.2103E+5,",
         "    5.2595E+5,5.3092E+5,5.3592E+5,5.4095E+5,5.4602E+5,5.5112E+5,5.5626E+5,",
         "    5.6144E+5,5.6664E+5,5.7189E+5,5.7716E+5,5.8247E+5,5.8781E+5,5.9319E+5,",
         "    5.9860E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.0000E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,4.7301E+0,4.6173E+0,4.5111E+0,4.4106E+0,4.3152E+0,",
         "    4.2246E+0,4.1382E+0,4.0557E+0,3.9768E+0,3.9013E+0,3.8288E+0,3.7593E+0,",
         "    3.6924E+0,3.6281E+0,3.5661E+0,3.5063E+0,3.4486E+0,3.3929E+0,3.3391E+0,",
         "    3.2870E+0,3.2366E+0,3.1878E+0,3.1405E+0,3.0502E+0,3.0070E+0,2.9651E+0,",
         "    2.9243E+0,2.8847E+0,2.8462E+0,2.8087E+0,2.7722E+0,2.7367E+0,2.7021E+0,",
         "    2.6684E+0,2.6355E+0,2.6035E+0,2.5722E+0,2.5417E+0,2.5119E+0,2.4828E+0,",
         "    2.4544E+0,2.4267E+0,2.3995E+0,2.3730E+0,2.3471E+0,2.3217E+0,2.2969E+0,",
         "    2.2726E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.2589E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,3.8135E+5,3.8529E+5,3.8922E+5,3.9317E+5,",
         "    3.9713E+5,4.0111E+5,4.0511E+5,4.0914E+5,4.1320E+5,4.1729E+5,4.2141E+5,",
         "    4.2556E+5,4.2974E+5,4.3396E+5,4.3821E+5,4.4249E+5,4.4681E+5,4.5117E+5,",
         "    4.5556E+5,4.5999E+5,4.6445E+5,4.6895E+5,4.7806E+5,4.8267E+5,4.8731E+5,",
         "    4.9199E+5,4.9671E+5,5.0147E+5,5.0626E+5,5.1108E+5,5.1594E+5,5.2084E+5,",
         "    5.2578E+5,5.3074E+5,5.3575E+5,5.4079E+5,5.4586E+5,5.5097E+5,5.5611E+5,",
         "    5.6129E+5,5.6650E+5,5.7175E+5,5.7703E+5,5.8234E+5,5.8768E+5,5.9306E+5,",
         "    5.9847E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.2589E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,5.8705E+0,5.7301E+0,5.5981E+0,5.4734E+0,",
         "    5.3552E+0,5.2430E+0,5.1361E+0,5.0341E+0,4.9366E+0,4.8432E+0,4.7537E+0,",
         "    4.6678E+0,4.5853E+0,4.5059E+0,4.4294E+0,4.3556E+0,4.2845E+0,4.2158E+0,",
         "    4.1494E+0,4.0852E+0,4.0230E+0,3.9628E+0,3.8479E+0,3.7931E+0,3.7398E+0,",
         "    3.6881E+0,3.6378E+0,3.5889E+0,3.5414E+0,3.4952E+0,3.4501E+0,3.4063E+0,",
         "    3.3636E+0,3.3220E+0,3.2814E+0,3.2418E+0,3.2032E+0,3.1655E+0,3.1287E+0,",
         "    3.0928E+0,3.0577E+0,3.0234E+0,2.9899E+0,2.9571E+0,2.9251E+0,2.8937E+0,",
         "    2.8630E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.5849E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.8426E+5,3.8828E+5,3.9229E+5,",
         "    3.9632E+5,4.0035E+5,4.0440E+5,4.0848E+5,4.1257E+5,4.1670E+5,4.2085E+5,",
         "    4.2503E+5,4.2924E+5,4.3348E+5,4.3775E+5,4.4206E+5,4.4640E+5,4.5077E+5,",
         "    4.5518E+5,4.5962E+5,4.6410E+5,4.6861E+5,4.7775E+5,4.8237E+5,4.8702E+5,",
         "    4.9171E+5,4.9644E+5,5.0120E+5,5.0600E+5,5.1084E+5,5.1571E+5,5.2061E+5,",
         "    5.2555E+5,5.3053E+5,5.3554E+5,5.4058E+5,5.4566E+5,5.5077E+5,5.5592E+5,",
         "    5.6111E+5,5.6632E+5,5.7157E+5,5.7686E+5,5.8217E+5,5.8752E+5,5.9290E+5,",
         "    5.9832E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.5849E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,7.2985E+0,7.1227E+0,6.9577E+0,",
         "    6.8020E+0,6.6548E+0,6.5151E+0,6.3821E+0,6.2554E+0,6.1344E+0,6.0186E+0,",
         "    5.9077E+0,5.8013E+0,5.6991E+0,5.6007E+0,5.5061E+0,5.4148E+0,5.3268E+0,",
         "    5.2419E+0,5.1598E+0,5.0804E+0,5.0035E+0,4.8570E+0,4.7871E+0,4.7193E+0,",
         "    4.6535E+0,4.5896E+0,4.5275E+0,4.4671E+0,4.4083E+0,4.3512E+0,4.2955E+0,",
         "    4.2413E+0,4.1886E+0,4.1371E+0,4.0869E+0,4.0380E+0,3.9903E+0,3.9437E+0,",
         "    3.8982E+0,3.8537E+0,3.8103E+0,3.7679E+0,3.7265E+0,3.6859E+0,3.6463E+0,",
         "    3.6075E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.9953E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.8704E+5,3.9116E+5,",
         "    3.9527E+5,3.9938E+5,4.0350E+5,4.0763E+5,4.1178E+5,4.1594E+5,4.2014E+5,",
         "    4.2435E+5,4.2860E+5,4.3287E+5,4.3717E+5,4.4150E+5,4.4587E+5,4.5027E+5,",
         "    4.5470E+5,4.5916E+5,4.6365E+5,4.6819E+5,4.7735E+5,4.8198E+5,4.8665E+5,",
         "    4.9136E+5,4.9610E+5,5.0087E+5,5.0568E+5,5.1052E+5,5.1540E+5,5.2032E+5,",
         "    5.2527E+5,5.3025E+5,5.3527E+5,5.4032E+5,5.4541E+5,5.5053E+5,5.5568E+5,",
         "    5.6087E+5,5.6609E+5,5.7135E+5,5.7664E+5,5.8196E+5,5.8732E+5,5.9270E+5,",
         "    5.9812E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.9953E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,9.0925E+0,8.8705E+0,",
         "    8.6627E+0,8.4671E+0,8.2825E+0,8.1076E+0,7.9414E+0,7.7832E+0,7.6322E+0,",
         "    7.4880E+0,7.3499E+0,7.2175E+0,7.0904E+0,6.9682E+0,6.8506E+0,6.7373E+0,",
         "    6.6281E+0,6.5227E+0,6.4209E+0,6.3225E+0,6.1351E+0,6.0458E+0,5.9592E+0,",
         "    5.8752E+0,5.7937E+0,5.7145E+0,5.6376E+0,5.5628E+0,5.4901E+0,5.4193E+0,",
         "    5.3504E+0,5.2833E+0,5.2180E+0,5.1543E+0,5.0922E+0,5.0316E+0,4.9725E+0,",
         "    4.9148E+0,4.8585E+0,4.8034E+0,4.7497E+0,4.6972E+0,4.6458E+0,4.5956E+0,",
         "    4.5465E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    2.5119E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.8968E+5,",
         "    3.9391E+5,3.9813E+5,4.0233E+5,4.0654E+5,4.1075E+5,4.1498E+5,4.1923E+5,",
         "    4.2349E+5,4.2778E+5,4.3209E+5,4.3643E+5,4.4080E+5,4.4520E+5,4.4962E+5,",
         "    4.5408E+5,4.5857E+5,4.6309E+5,4.6764E+5,4.7685E+5,4.8150E+5,4.8619E+5,",
         "    4.9091E+5,4.9566E+5,5.0045E+5,5.0527E+5,5.1013E+5,5.1502E+5,5.1995E+5,",
         "    5.2491E+5,5.2990E+5,5.3493E+5,5.3999E+5,5.4509E+5,5.5022E+5,5.5538E+5,",
         "    5.6058E+5,5.6581E+5,5.7107E+5,5.7637E+5,5.8170E+5,5.8706E+5,5.9245E+5,",
         "    5.9788E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    2.5119E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,1.1355E+1,",
         "    1.1072E+1,1.0808E+1,1.0560E+1,1.0327E+1,1.0107E+1,9.8974E+0,9.6986E+0,",
         "    9.5093E+0,9.3285E+0,9.1557E+0,8.9902E+0,8.8314E+0,8.6789E+0,8.5323E+0,",
         "    8.3911E+0,8.2551E+0,8.1239E+0,7.9972E+0,7.7564E+0,7.6418E+0,7.5308E+0,",
         "    7.4232E+0,7.3189E+0,7.2177E+0,7.1194E+0,7.0240E+0,6.9312E+0,6.8409E+0,",
         "    6.7531E+0,6.6677E+0,6.5844E+0,6.5034E+0,6.4244E+0,6.3473E+0,6.2722E+0,",
         "    6.1989E+0,6.1274E+0,6.0575E+0,5.9893E+0,5.9227E+0,5.8575E+0,5.7938E+0,",
         "    5.7316E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.1623E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    3.9211E+5,3.9648E+5,4.0081E+5,4.0512E+5,4.0943E+5,4.1374E+5,4.1806E+5,",
         "    4.2239E+5,4.2674E+5,4.3110E+5,4.3549E+5,4.3991E+5,4.4434E+5,4.4881E+5,",
         "    4.5330E+5,4.5782E+5,4.6237E+5,4.6696E+5,4.7621E+5,4.8089E+5,4.8560E+5,",
         "    4.9034E+5,4.9511E+5,4.9992E+5,5.0476E+5,5.0963E+5,5.1454E+5,5.1948E+5,",
         "    5.2446E+5,5.2946E+5,5.3450E+5,5.3958E+5,5.4469E+5,5.4983E+5,5.5500E+5,",
         "    5.6021E+5,5.6545E+5,5.7072E+5,5.7602E+5,5.8136E+5,5.8673E+5,5.9213E+5,",
         "    5.9757E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.1623E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    1.4223E+1,1.3858E+1,1.3519E+1,1.3202E+1,1.2904E+1,1.2624E+1,1.2359E+1,",
         "    1.2107E+1,1.1868E+1,1.1640E+1,1.1422E+1,1.1214E+1,1.1015E+1,1.0823E+1,",
         "    1.0640E+1,1.0463E+1,1.0293E+1,1.01290E+1,9.8174E+0,9.6697E+0,9.5267E+0,",
         "    9.3884E+0,9.2543E+0,9.1244E+0,8.9983E+0,8.8760E+0,8.7571E+0,8.6417E+0,",
         "    8.5294E+0,8.4202E+0,8.3139E+0,8.2105E+0,8.1097E+0,8.0115E+0,7.9158E+0,",
         "    7.8224E+0,7.7313E+0,7.6425E+0,7.5557E+0,7.4709E+0,7.3881E+0,7.3072E+0,",
         "    7.2282E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.9811E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,3.9427E+5,3.9879E+5,4.0326E+5,4.0770E+5,4.1213E+5,4.1654E+5,",
         "    4.2097E+5,4.2539E+5,4.2983E+5,4.3429E+5,4.3876E+5,4.4325E+5,4.4777E+5,",
         "    4.5231E+5,4.5687E+5,4.6146E+5,4.6608E+5,4.7540E+5,4.8011E+5,4.8485E+5,",
         "    4.8962E+5,4.9442E+5,4.9925E+5,5.0411E+5,5.0900E+5,5.1393E+5,5.1889E+5,",
         "    5.2388E+5,5.2891E+5,5.3397E+5,5.3906E+5,5.4418E+5,5.4933E+5,5.5452E+5,",
         "    5.5974E+5,5.6499E+5,5.7028E+5,5.7559E+5,5.8094E+5,5.8632E+5,5.9173E+5,",
         "    5.9718E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.9811E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,1.7882E+1,1.7403E+1,1.6961E+1,1.6551E+1,1.6167E+1,1.5807E+1,",
         "    1.5467E+1,1.5146E+1,1.4841E+1,1.4551E+1,1.4275E+1,1.4012E+1,1.3760E+1,",
         "    1.3518E+1,1.3286E+1,1.3064E+1,1.2850E+1,1.2445E+1,1.2253E+1,1.2068E+1,",
         "    1.1889E+1,1.1715E+1,1.1548E+1,1.1385E+1,1.1228E+1,1.1075E+1,1.0926E+1,",
         "    1.0782E+1,1.0642E+1,1.0506E+1,1.0373E+1,1.0244E+1,1.0119E+1,9.9965E+0,",
         "    9.8773E+0,9.7610E+0,9.6476E+0,9.5369E+0,9.4289E+0,9.3234E+0,9.2203E+0,",
         "    9.1197E+0;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    5.0119E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,4.0078E+5,4.0542E+5,4.1001E+5,4.1457E+5,",
         "    4.1911E+5,4.2365E+5,4.2819E+5,4.3273E+5,4.3728E+5,4.4185E+5,4.4643E+5,",
         "    4.5103E+5,4.5565E+5,4.6030E+5,4.6496E+5,4.7438E+5,4.7912E+5,4.8390E+5,",
         "    4.8870E+5,4.9353E+5,4.9839E+5,5.0328E+5,5.0821E+5,5.1316E+5,5.1814E+5,",
         "    5.2316E+5,5.2820E+5,5.3328E+5,5.3839E+5,5.4353E+5,5.4871E+5,5.5391E+5,",
         "    5.5915E+5,5.6442E+5,5.6972E+5,5.7505E+5,5.8041E+5,5.8580E+5,5.9123E+5,",
         "    5.9668E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    5.0119E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,2.1949E+1,2.1364E+1,2.0824E+1,2.0322E+1,",
         "    1.9852E+1,1.9412E+1,1.8997E+1,1.8605E+1,1.8233E+1,1.7879E+1,1.7542E+1,",
         "    1.7221E+1,1.6914E+1,1.6620E+1,1.6337E+1,1.5806E+1,1.5555E+1,1.5313E+1,",
         "    1.5079E+1,1.4854E+1,1.4636E+1,1.4425E+1,1.4221E+1,1.4023E+1,1.3831E+1,",
         "    1.3645E+1,1.3465E+1,1.3289E+1,1.3119E+1,1.2953E+1,1.2792E+1,1.2635E+1,",
         "    1.2482E+1,1.2333E+1,1.2188E+1,1.2046E+1,1.1908E+1,1.1773E+1,1.1641E+1,",
         "    1.1513E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    6.3096E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,4.0232E+5,4.0717E+5,4.1194E+5,",
         "    4.1667E+5,4.2136E+5,4.2603E+5,4.3070E+5,4.3536E+5,4.4003E+5,4.4470E+5,",
         "    4.4939E+5,4.5408E+5,4.5880E+5,4.6353E+5,4.7306E+5,4.7786E+5,4.8268E+5,",
         "    4.8753E+5,4.9240E+5,4.9730E+5,5.0223E+5,5.0719E+5,5.1218E+5,5.1719E+5,",
         "    5.2224E+5,5.2731E+5,5.3242E+5,5.3755E+5,5.4272E+5,5.4792E+5,5.5314E+5,",
         "    5.5840E+5,5.6369E+5,5.6901E+5,5.7436E+5,5.7974E+5,5.8515E+5,5.9059E+5,",
         "    5.9606E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    6.3096E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,2.7837E+1,2.7044E+1,2.6318E+1,",
         "    2.5649E+1,2.5028E+1,2.4448E+1,2.3904E+1,2.3393E+1,2.2909E+1,2.2452E+1,",
         "    2.2017E+1,2.1603E+1,2.1209E+1,2.0832E+1,2.0125E+1,1.9793E+1,1.9474E+1,",
         "    1.9166E+1,1.8870E+1,1.8585E+1,1.8309E+1,1.8042E+1,1.7785E+1,1.7535E+1,",
         "    1.7293E+1,1.7059E+1,1.6832E+1,1.6611E+1,1.6397E+1,1.6189E+1,1.5986E+1,",
         "    1.5789E+1,1.5597E+1,1.5410E+1,1.5228E+1,1.5051E+1,1.4878E+1,1.4709E+1,",
         "    1.4544E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    7.9433E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,4.0325E+5,4.0837E+5,",
         "    4.1338E+5,4.1831E+5,4.2319E+5,4.2803E+5,4.3285E+5,4.3765E+5,4.4245E+5,",
         "    4.4725E+5,4.5205E+5,4.5686E+5,4.6168E+5,4.7137E+5,4.7623E+5,4.8112E+5,",
         "    4.8603E+5,4.9096E+5,4.9591E+5,5.0089E+5,5.0590E+5,5.1093E+5,5.1598E+5,",
         "    5.2107E+5,5.2618E+5,5.3132E+5,5.3649E+5,5.4168E+5,5.4691E+5,5.5217E+5,",
         "    5.5745E+5,5.6277E+5,5.6811E+5,5.7348E+5,5.7888E+5,5.8432E+5,5.8978E+5,",
         "    5.9527E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    7.9433E+5,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.5574E+1,3.4462E+1,",
         "    3.3461E+1,3.2547E+1,3.1707E+1,3.0928E+1,3.0203E+1,2.9525E+1,2.8887E+1,",
         "    2.8286E+1,2.7717E+1,2.7178E+1,2.6666E+1,2.5711E+1,2.5265E+1,2.4838E+1,",
         "    2.4429E+1,2.4035E+1,2.3657E+1,2.3292E+1,2.2941E+1,2.2602E+1,2.2274E+1,",
         "    2.1957E+1,2.1651E+1,2.1354E+1,2.1066E+1,2.0787E+1,2.0517E+1,2.0254E+1,",
         "    1.9998E+1,1.9750E+1,1.9508E+1,1.9273E+1,1.9044E+1,1.8821E+1,1.8603E+1,",
         "    1.8391E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.0000E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    4.0880E+5,4.1412E+5,4.1932E+5,4.2444E+5,4.2949E+5,4.3451E+5,4.3949E+5,",
         "    4.4445E+5,4.4940E+5,4.5434E+5,4.5928E+5,4.6918E+5,4.7414E+5,4.7911E+5,",
         "    4.8410E+5,4.8911E+5,4.9413E+5,4.9917E+5,5.0424E+5,5.0933E+5,5.1444E+5,",
         "    5.1957E+5,5.2474E+5,5.2992E+5,5.3513E+5,5.4037E+5,5.4564E+5,5.5093E+5,",
         "    5.5625E+5,5.6160E+5,5.6697E+5,5.7238E+5,5.7781E+5,5.8327E+5,5.8876E+5,",
         "    5.9427E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.0000E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    4.4326E+1,4.2889E+1,4.1599E+1,4.0428E+1,3.9355E+1,3.8365E+1,3.7445E+1,",
         "    3.6588E+1,3.5784E+1,3.5028E+1,3.4314E+1,3.2997E+1,3.2387E+1,3.1806E+1,",
         "    3.1251E+1,3.0720E+1,3.0211E+1,2.9722E+1,2.9253E+1,2.8801E+1,2.8366E+1,",
         "    2.7946E+1,2.7541E+1,2.7149E+1,2.6771E+1,2.6404E+1,2.6049E+1,2.5705E+1,",
         "    2.5371E+1,2.5047E+1,2.4732E+1,2.4426E+1,2.4129E+1,2.3839E+1,2.3557E+1,",
         "    2.3283E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.2589E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,4.1388E+5,4.1946E+5,4.2490E+5,4.3024E+5,4.3550E+5,",
         "    4.4071E+5,4.4587E+5,4.5101E+5,4.5612E+5,4.6632E+5,4.7141E+5,4.7651E+5,",
         "    4.8160E+5,4.8671E+5,4.9183E+5,4.9696E+5,5.0211E+5,5.0728E+5,5.1246E+5,",
         "    5.1766E+5,5.2289E+5,5.2814E+5,5.3340E+5,5.3870E+5,5.4401E+5,5.4935E+5,",
         "    5.5472E+5,5.6011E+5,5.6553E+5,5.7097E+5,5.7644E+5,5.8194E+5,5.8746E+5,",
         "    5.9301E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.2589E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,5.5617E+1,5.3706E+1,5.2005E+1,5.0472E+1,4.9077E+1,",
         "    4.7795E+1,4.6611E+1,4.5511E+1,4.4484E+1,4.2613E+1,4.1757E+1,4.0947E+1,",
         "    4.0177E+1,3.9445E+1,3.8747E+1,3.8080E+1,3.7442E+1,3.6831E+1,3.6244E+1,",
         "    3.5680E+1,3.5137E+1,3.4614E+1,3.4109E+1,3.3623E+1,3.3152E+1,3.2697E+1,",
         "    3.2256E+1,3.1829E+1,3.1415E+1,3.1014E+1,3.0624E+1,3.0245E+1,2.9877E+1,",
         "    2.9519E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.5849E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,4.1214E+5,4.1832E+5,4.2424E+5,4.2998E+5,",
         "    4.3559E+5,4.4109E+5,4.4653E+5,4.5191E+5,4.6254E+5,4.6782E+5,4.7309E+5,",
         "    4.7834E+5,4.8359E+5,4.8885E+5,4.9410E+5,4.9936E+5,5.0463E+5,5.0991E+5,",
         "    5.1521E+5,5.2052E+5,5.2585E+5,5.3119E+5,5.3655E+5,5.4194E+5,5.4734E+5,",
         "    5.5277E+5,5.5822E+5,5.6369E+5,5.6919E+5,5.7471E+5,5.8025E+5,5.8582E+5,",
         "    5.9141E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.5849E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,7.3448E+1,7.0389E+1,6.7761E+1,6.5454E+1,",
         "    6.3396E+1,6.1539E+1,5.9847E+1,5.8293E+1,5.5523E+1,5.4277E+1,5.3108E+1,",
         "    5.2009E+1,5.0971E+1,4.9988E+1,4.9056E+1,4.8169E+1,4.7324E+1,4.6516E+1,",
         "    4.5744E+1,4.5004E+1,4.4293E+1,4.3611E+1,4.2954E+1,4.2321E+1,4.1711E+1,",
         "    4.1122E+1,4.0553E+1,4.0003E+1,3.9470E+1,3.8954E+1,3.8453E+1,3.7968E+1,",
         "    3.7496E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.9953E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,4.1510E+5,4.2185E+5,",
         "    4.2823E+5,4.3436E+5,4.4030E+5,4.4611E+5,4.5745E+5,4.6302E+5,4.6854E+5,",
         "    4.7402E+5,4.7948E+5,4.8492E+5,4.9035E+5,4.9577E+5,5.0119E+5,5.0661E+5,",
         "    5.1203E+5,5.1746E+5,5.2289E+5,5.2834E+5,5.3380E+5,5.3928E+5,5.4477E+5,",
         "    5.5028E+5,5.5580E+5,5.6135E+5,5.6691E+5,5.7250E+5,5.7811E+5,5.8373E+5,",
         "    5.8938E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    1.9953E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,9.4602E+1,9.0089E+1,",
         "    8.6317E+1,8.3071E+1,8.0222E+1,7.7683E+1,7.3307E+1,7.1394E+1,6.9628E+1,",
         "    6.7987E+1,6.6457E+1,6.5023E+1,6.3675E+1,6.2404E+1,6.1201E+1,6.0061E+1,",
         "    5.8976E+1,5.7943E+1,5.6957E+1,5.6014E+1,5.5111E+1,5.4245E+1,5.3413E+1,",
         "    5.2613E+1,5.1842E+1,5.1100E+1,5.0383E+1,4.9690E+1,4.9021E+1,4.8373E+1,",
         "    4.7745E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    2.5119E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    4.1645E+5,4.2407E+5,4.3111E+5,4.3776E+5,4.5035E+5,4.5640E+5,4.6233E+5,",
         "    4.6818E+5,4.7396E+5,4.7969E+5,4.8538E+5,4.9103E+5,4.9666E+5,5.0228E+5,",
         "    5.0788E+5,5.1347E+5,5.1906E+5,5.2465E+5,5.3025E+5,5.3585E+5,5.4146E+5,",
         "    5.4708E+5,5.5270E+5,5.5835E+5,5.6400E+5,5.6968E+5,5.7537E+5,5.8107E+5,",
         "    5.868E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    2.5119E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    1.2428E+2,1.1709E+2,1.1135E+2,1.0656E+2,9.8867E+1,9.5672E+1,9.2798E+1,",
         "    9.0187E+1,8.7796E+1,8.5592E+1,8.3550E+1,8.1647E+1,7.9868E+1,7.8197E+1,",
         "    7.6623E+1,7.5136E+1,7.3727E+1,7.2389E+1,7.1116E+1,6.9901E+1,6.8741E+1,",
         "    6.7631E+1,6.6567E+1,6.5545E+1,6.4564E+1,6.3619E+1,6.2708E+1,6.1830E+1,",
         "    6.0982E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.1623E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,4.1517E+5,4.2434E+5,4.3982E+5,4.4682E+5,4.5353E+5,",
         "    4.6002E+5,4.6634E+5,4.7254E+5,4.7864E+5,4.8466E+5,4.9061E+5,4.9652E+5,",
         "    5.0239E+5,5.0823E+5,5.1404E+5,5.1984E+5,5.2562E+5,5.3140E+5,5.3717E+5,",
         "    5.4294E+5,5.4871E+5,5.5449E+5,5.6027E+5,5.6606E+5,5.7187E+5,5.7768E+5,",
         "    5.8350E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.1623E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,1.6856E+2,1.5561E+2,1.3863E+2,1.3241E+2,1.2711E+2,",
         "    1.2249E+2,1.1840E+2,1.1473E+2,1.1141E+2,1.0838E+2,1.0560E+2,1.0302E+2,",
         "    1.0063E+2,9.8397E+1,9.6305E+1,9.4337E+1,9.2480E+1,9.0724E+1,8.9059E+1,",
         "    8.7476E+1,8.5969E+1,8.4530E+1,8.3155E+1,8.1838E+1,8.0575E+1,7.9362E+1,",
         "    7.8196E+1;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.9811E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,4.2155E+5,4.3139E+5,4.4000E+5,",
         "    4.4789E+5,4.5530E+5,4.6238E+5,4.6921E+5,4.7585E+5,4.8234E+5,4.8872E+5,",
         "    4.9501E+5,5.0122E+5,5.0737E+5,5.1347E+5,5.1953E+5,5.2557E+5,5.3157E+5,",
         "    5.3756E+5,5.4353E+5,5.4950E+5,5.5546E+5,5.6141E+5,5.6737E+5,5.7333E+5,",
         "    5.7929E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    3.9811E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,2.1400E+2,1.9640E+2,1.8365E+2,",
         "    1.7365E+2,1.6543E+2,1.5847E+2,1.5244E+2,1.4713E+2,1.4240E+2,1.3813E+2,",
         "    1.3425E+2,1.3069E+2,1.2741E+2,1.2437E+2,1.2154E+2,1.1890E+2,1.1642E+2,",
         "    1.1408E+2,1.1187E+2,1.0978E+2,1.0780E+2,1.0592E+2,1.0412E+2,1.0241E+2,",
         "    1.0077E+2;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    5.0119E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.5170E+5,3.9388E+5,4.1434E+5,",
         "    4.2730E+5,4.3775E+5,4.4692E+5,4.5530E+5,4.6317E+5,4.7066E+5,4.7787E+5,",
         "    4.8486E+5,4.9168E+5,4.9837E+5,5.0495E+5,5.1144E+5,5.1785E+5,5.2421E+5,",
         "    5.3052E+5,5.3678E+5,5.4302E+5,5.4922E+5,5.5541E+5,5.6158E+5,5.6774E+5,",
         "    5.7389E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    5.0119E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,6.7001E+2,3.8944E+2,3.0817E+2,",
         "    2.7168E+2,2.4841E+2,2.3144E+2,2.1816E+2,2.0730E+2,1.9814E+2,1.9025E+2,",
         "    1.8332E+2,1.7718E+2,1.7165E+2,1.6665E+2,1.6208E+2,1.5788E+2,1.5400E+2,",
         "    1.5039E+2,1.4703E+2,1.4389E+2,1.4093E+2,1.3814E+2,1.3551E+2,1.3302E+2,",
         "    1.3065E+2;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    6.3096E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.3728E+5,3.5011E+5,3.6603E+5,",
         "    3.8611E+5,4.0560E+5,4.2092E+5,4.3330E+5,4.4394E+5,4.5349E+5,4.6229E+5,",
         "    4.7058E+5,4.7846E+5,4.8605E+5,4.9341E+5,5.0057E+5,5.0758E+5,5.1447E+5,",
         "    5.2125E+5,5.2795E+5,5.3457E+5,5.4114E+5,5.4765E+5,5.5413E+5,5.6057E+5,",
         "    5.6698E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    6.3096E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,8.1728E+2,7.4948E+2,6.5660E+2,",
         "    5.3778E+2,4.3912E+2,3.7826E+2,3.3897E+2,3.1111E+2,2.8993E+2,2.7305E+2,",
         "    2.5912E+2,2.4734E+2,2.3717E+2,2.2826E+2,2.2036E+2,2.1327E+2,2.0685E+2,",
         "    2.0101E+2,1.9565E+2,1.9070E+2,1.8612E+2,1.8185E+2,1.7787E+2,1.7413E+2,",
         "    1.7061E+2;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Enthalpy,                !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    7.9433E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,3.3158E+5,3.4154E+5,3.5219E+5,",
         "    3.6374E+5,3.7633E+5,3.8986E+5,4.0375E+5,4.1711E+5,4.2940E+5,4.4060E+5,",
         "    4.5087E+5,4.6042E+5,4.6941E+5,4.7795E+5,4.8613E+5,4.9403E+5,5.0169E+5,",
         "    5.0917E+5,5.1648E+5,5.2367E+5,5.3074E+5,5.3771E+5,5.4461E+5,5.5143E+5,",
         "    5.5820E+5;",
         "  FluidProperties:Superheated,",
         "    R407a,                   !- Fluid Name",
         "    Density,                 !- Fluid Property Type",
         "    R407aSuperHeatTemperatures,  !- Temperature Values Name",
         "    7.9433E+6,               !- Pressure {Pa}",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,",
         "    0.0000E+0,0.0000E+0,0.0000E+0,0.0000E+0,8.8181E+2,8.4036E+2,7.9328E+2,",
         "    7.3939E+2,6.7834E+2,6.1234E+2,5.4754E+2,4.9096E+2,4.4521E+2,4.0899E+2,",
         "    3.8004E+2,3.5646E+2,3.3685E+2,3.2023E+2,3.0593E+2,2.9344E+2,2.8242E+2,",
         "    2.7258E+2,2.6373E+2,2.5571E+2,2.4838E+2,2.4166E+2,2.3546E+2,2.2971E+2,",
         "    2.2436E+2;"});

    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    bool ErrorsFound(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);

    // ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    InternalHeatGains::ManageInternalHeatGains(*state, true);
    RefrigeratedCase::ManageRefrigeratedCaseRacks(*state);

    compare_err_stream("", true);

    EXPECT_EQ(state->dataRefrigCase->NumRefrigCondensers, 3);
    EXPECT_EQ(state->dataRefrigCase->NumRefrigeratedRacks, 1);
    EXPECT_EQ(state->dataRefrigCase->NumSimulationCases, 4);
    EXPECT_EQ(state->dataRefrigCase->NumSimulationWalkIns, 3);

    for (auto refrigCase : state->dataRefrigCase->RefrigCase) {
        EXPECT_GT(refrigCase.ActualZoneNum, 0);
        EXPECT_GT(refrigCase.ZoneNodeNum, 0);
    }
    for (auto refrigWalkIn : state->dataRefrigCase->WalkIn) {
        for (int zoneNum : refrigWalkIn.ZoneNum) {
            EXPECT_GT(zoneNum, 0);
        }
        for (int zoneNodeNum : refrigWalkIn.ZoneNodeNum) {
            EXPECT_GT(zoneNodeNum, 0);
        }
    }
}

TEST_F(EnergyPlusFixture, TranscriticalSystem_CapacityCorrectionUsesPostSubcoolerEnthalpy)
{
    constexpr Real64 hCompInHP = 460000.0;
    constexpr Real64 hGasCoolerOut = 300000.0;
    constexpr Real64 delHSubcoolerDis = 15000.0; // positive on discharge side in this convention
    constexpr Real64 caseEnthalpyChangeRatedMT = 120000.0;
    constexpr Real64 massCorrectionMT = 0.92;

    const Real64 totalHDeltaWithSubcooler = RefrigeratedCase::CalcTransMTActualEnthalpyChange(hCompInHP, hGasCoolerOut, delHSubcoolerDis);
    const Real64 totalHDeltaNoSubcooler = RefrigeratedCase::CalcTransMTActualEnthalpyChange(hCompInHP, hGasCoolerOut, 0.0);

    EXPECT_NEAR(totalHDeltaWithSubcooler, 145000.0, 1e-9);
    EXPECT_NEAR(totalHDeltaNoSubcooler, 160000.0, 1e-9);
    EXPECT_LT(totalHDeltaWithSubcooler, totalHDeltaNoSubcooler);

    const Real64 capCorrWithSubcooler = massCorrectionMT * totalHDeltaWithSubcooler / caseEnthalpyChangeRatedMT;
    const Real64 capCorrNoSubcooler = massCorrectionMT * totalHDeltaNoSubcooler / caseEnthalpyChangeRatedMT;

    // Expected direction: adding subcooler term reduces capacity correction
    EXPECT_LT(capCorrWithSubcooler, capCorrNoSubcooler);
}
