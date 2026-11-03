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
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/WaterUse.hh>

using namespace EnergyPlus;
using namespace WaterUse;

TEST_F(EnergyPlusFixture, WaterUse_WaterTempWarnings)
{
    // This unit test checks warnings/errors associated with unreasonable temperatures in WaterUse:Equipment
    bool ErrorsFound(false);

    std::string_view constexpr idf_objects = R"IDF(
  Zone,
    Core_ZN,                 !- Name
    0.0000,                  !- Direction of Relative North {deg}
    0.0000,                  !- X Origin {m}
    0.0000,                  !- Y Origin {m}
    0.0000,                  !- Z Origin {m}
    1,                       !- Type
    1,                       !- Multiplier
    ,                        !- Ceiling Height {m}
    ,                        !- Volume {m3}
    autocalculate,           !- Floor Area {m2}
    ,                        !- Zone Inside Convection Algorithm
    ,                        !- Zone Outside Convection Algorithm
    Yes;                     !- Part of Total Floor Area

  Sizing:Plant,
    SWHSys1,                 !- Plant or Condenser Loop Name
    Heating,                 !- Loop Type
    60,                      !- Design Loop Exit Temperature {C}
    5.0;                     !- Loop Design Temperature Difference {deltaC}

  SetpointManager:Scheduled,
    SWHSys1 Loop Setpoint Manager,  !- Name
    Temperature,             !- Control Variable
    SWHSys1-Loop-Temp-Schedule,  !- Schedule Name
    SWHSys1 Supply Outlet Node;  !- Setpoint Node or NodeList Name

  PlantEquipmentOperationSchemes,
    SWHSys1 Loop Operation Scheme List,  !- Name
    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type
    SWHSys1 Operation Scheme,!- Control Scheme 1 Name
    ALWAYS_ON;               !- Control Scheme 1 Schedule Name

  PlantEquipmentOperation:HeatingLoad,
    SWHSys1 Operation Scheme,!- Name
    0.0,                     !- Load Range 1 Lower Limit {W}
    1000000000000000,        !- Load Range 1 Upper Limit {W}
    SWHSys1 Equipment List;  !- Range 1 Equipment List Name

! ***SWH LOOP***

  PlantLoop,
    SWHSys1,                 !- Name
    Water,                   !- Fluid Type
    ,                        !- User Defined Fluid Type
    SWHSys1 Loop Operation Scheme List,  !- Plant Equipment Operation Scheme Name
    SWHSys1 Supply Outlet Node,  !- Loop Temperature Setpoint Node Name
    60.0,                    !- Maximum Loop Temperature {C}
    10.0,                    !- Minimum Loop Temperature {C}
    AUTOSIZE,                !- Maximum Loop Flow Rate {m3/s}
    0.0,                     !- Minimum Loop Flow Rate {m3/s}
    AUTOSIZE,                !- Plant Loop Volume {m3}
    SWHSys1 Supply Inlet Node,  !- Plant Side Inlet Node Name
    SWHSys1 Supply Outlet Node,  !- Plant Side Outlet Node Name
    SWHSys1 Supply Branches, !- Plant Side Branch List Name
    SWHSys1 Supply Connectors,  !- Plant Side Connector List Name
    SWHSys1 Demand Inlet Node,  !- Demand Side Inlet Node Name
    SWHSys1 Demand Outlet Node,  !- Demand Side Outlet Node Name
    SWHSys1 Demand Branches, !- Demand Side Branch List Name
    SWHSys1 Demand Connectors,  !- Demand Side Connector List Name
    Optimal;                 !- Load Distribution Scheme

! ***SWH CONNECTIONS***

  BranchList,
    SWHSys1 Demand Branches, !- Name
    SWHSys1 Demand Inlet Branch,  !- Branch 1 Name
    SWHSys1 Demand Load Branch 1,  !- Branch 2 Name
    SWHSys1 Demand Bypass Branch,  !- Branch 3 Name
    SWHSys1 Demand Outlet Branch;  !- Branch 4 Name

  BranchList,
    SWHSys1 Supply Branches, !- Name
    SWHSys1 Supply Inlet Branch,  !- Branch 1 Name
    SWHSys1 Supply Equipment Branch,  !- Branch 2 Name
    SWHSys1 Supply Equipment Bypass Branch,  !- Branch 3 Name
    SWHSys1 Supply Outlet Branch;  !- Branch 4 Name

  Branch,
    SWHSys1 Demand Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Bypass Pipe,  !- Component 1 Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;

  Branch,
    SWHSys1 Demand Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Inlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;

  Branch,
    SWHSys1 Demand Load Branch 1,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterUse:Connections,    !- Component 1 Object Type
    Core_ZN Water Equipment, !- Component 1 Name
    Core_ZN Water Equipment Water Inlet Node,  !- Component 1 Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node;

  Branch,
    SWHSys1 Demand Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Outlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterHeater:Mixed,       !- Component 1 Object Type
    SWHSys1 Water Heater,    !- Component 1 Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Equipment Bypass Pipe,  !- Component 1 Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;

  Branch,
    SWHSys1 Supply Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pump:ConstantSpeed,      !- Component 1 Object Type
    SWHSys1 Pump,            !- Component 1 Name
    SWHSys1 Supply Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector;

  Branch,
    SWHSys1 Supply Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Outlet Pipe,  !- Component 1 Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Outlet Node;

  ConnectorList,
    SWHSys1 Demand Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Demand Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Demand Mixer;    !- Connector 2 Name

  ConnectorList,
    SWHSys1 Supply Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Supply Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Supply Mixer;    !- Connector 2 Name

  Connector:Splitter,
    SWHSys1 Demand Splitter, !- Name
    SWHSys1 Demand Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Outlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Splitter,
    SWHSys1 Supply Splitter, !- Name
    SWHSys1 Supply Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Outlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Demand Mixer,    !- Name
    SWHSys1 Demand Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Inlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Inlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Supply Mixer,    !- Name
    SWHSys1 Supply Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Inlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Inlet Branch 2 Name

  Pipe:Adiabatic,
    SWHSys1 Demand Bypass Pipe,  !- Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Inlet Pipe,  !- Name
    SWHSys1 Demand Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Outlet Pipe,  !- Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Demand Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Equipment Bypass Pipe,  !- Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Outlet Pipe,  !- Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Supply Outlet Node;  !- Outlet Node Name

! ***SWH SCHEDULES***

  ScheduleTypeLimits,
    Fraction,                !- Name
    0.0,                     !- Lower Limit Value
    1.0,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  ScheduleTypeLimits,
    Temperature,             !- Name
    -60,                     !- Lower Limit Value
    200,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  Schedule:Compact,
    Water Equipment Latent fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.05;       !- Field 3

  Schedule:Compact,
    Water Equipment Sensible fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.2;        !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,22.0;       !- Field 3

  Schedule:Compact,
    Water Equipment Temp Sched,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  Schedule:Compact,
    SWHSys1-Loop-Temp-Schedule,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  PlantEquipmentList,
    SWHSys1 Equipment List,  !- Name
    WaterHeater:Mixed,       !- Equipment 1 Object Type
    SWHSys1 Water Heater;    !- Equipment 1 Name

  WaterHeater:Mixed,
    SWHSys1 Water Heater,    !- Name
    0.1514,                  !- Tank Volume {m3}
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Setpoint Temperature Schedule Name
    2.0,                     !- Deadband Temperature Difference {deltaC}
    82.2222,                 !- Maximum Temperature Limit {C}
    Cycle,                   !- Heater Control Type
    845000,                  !- Heater Maximum Capacity {W}
    ,                        !- Heater Minimum Capacity {W}
    ,                        !- Heater Ignition Minimum Flow Rate {m3/s}
    ,                        !- Heater Ignition Delay {s}
    NATURALGAS,              !- Heater Fuel Type
    0.8,                     !- Heater Thermal Efficiency
    ,                        !- Part Load Factor Curve Name
    20,                      !- Off Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- Off Cycle Parasitic Fuel Type
    0.8,                     !- Off Cycle Parasitic Heat Fraction to Tank
    ,                        !- On Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- On Cycle Parasitic Fuel Type
    ,                        !- On Cycle Parasitic Heat Fraction to Tank
    SCHEDULE,                !- Ambient Temperature Indicator
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Ambient Temperature Schedule Name
    ,                        !- Ambient Temperature Zone Name
    ,                        !- Ambient Temperature Outdoor Air Node Name
    6.0,                     !- Off Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- Off Cycle Loss Fraction to Zone
    6.0,                     !- On Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- On Cycle Loss Fraction to Zone
    ,                        !- Peak Use Flow Rate {m3/s}
    ,                        !- Use Flow Rate Fraction Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Use Side Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node,  !- Use Side Outlet Node Name
    1.0,                     !- Use Side Effectiveness
    ,                        !- Source Side Inlet Node Name
    ,                        !- Source Side Outlet Node Name
    1.0,                     !- Source Side Effectiveness
    AUTOSIZE,                !- Use Side Design Flow Rate {m3/s}
    AUTOSIZE,                !- Source Side Design Flow Rate {m3/s}
    1.5;                     !- Indirect Water Heating Recovery Time {hr}

  Pump:ConstantSpeed,
    SWHSys1 Pump,            !- Name
    SWHSys1 Supply Inlet Node,  !- Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector,  !- Outlet Node Name
    AUTOSIZE,                !- Rated Flow Rate {m3/s}
    0.001,                   !- Rated Pump Head {Pa}
    AUTOSIZE,                !- Rated Power Consumption {W}
    1,                       !- Motor Efficiency
    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream
    Intermittent,            !- Pump Control Type
    ;                        !- Pump Flow Rate Schedule Name

  WaterUse:Connections,
    Core_ZN Water Equipment, !- Name
    Core_ZN Water Equipment Water Inlet Node,  !- Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node,  !- Outlet Node Name
    ,                        !- Supply Water Storage Tank Name
    ,                        !- Reclamation Water Storage Tank Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    ,                        !- Drain Water Heat Exchanger Type
    ,                        !- Drain Water Heat Exchanger Destination
    ,                        !- Drain Water Heat Exchanger U-Factor Times Area {W/K}
    Core_ZN Water Equipment; !- Water Use Equipment 1 Name

  WaterUse:Equipment,
    Core_ZN Water Equipment, !- Name
    ,                        !- End-Use Subcategory
    1.0e-005,                !- Peak Flow Rate {m3/s}
    ,                        !- Flow Rate Fraction Schedule Name
    Water Equipment Temp Sched,  !- Target Temperature Schedule Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    Core_ZN,                 !- Zone Name
    Water Equipment Sensible fract sched,  !- Sensible Fraction Schedule Name
    Water Equipment Latent fract sched;  !- Latent Fraction Schedule Name
    )IDF";

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->CurrentTime = 0.25;
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, 101325.0, 20.0, 0.0);
    Sched::UpdateScheduleVals(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    InternalHeatGains::GetInternalHeatGainsInput(*state);
    Real64 WaterConnNum = 1;
    GetWaterUseInput(*state);
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->WaterMainsTemp = 15;

    // Set plant loop temperature to 10C, below hot water temperature to trigger warning
    state->dataLoopNodes->Node(1).Temp = 10;
    Real64 WaterEquipNum = 1;
    auto &thisWaterConnections = state->dataWaterUse->WaterConnections(WaterConnNum);
    auto &thisWaterEquipment = state->dataWaterUse->WaterEquipment(WaterEquipNum);
    thisWaterConnections.InitConnections(*state);
    thisWaterEquipment.WaterEquipmentType::CalcEquipmentFlowRates(*state);

    std::string const error_string1 = delimited_string({
        "   ** Warning ** CalcEquipmentFlowRates: \"CORE_ZN WATER EQUIPMENT\" - Hot water temperature is less than the cold water temperature by "
        "(5.00 C)",
        "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:15",
        "   **   ~~~   ** ...hot water temperature        = 10.00 C",
        "   **   ~~~   ** ...cold water temperature       = 15.00 C",
        "   **   ~~~   ** ...Hot water temperature should be greater than or equal to the cold water temperature. Verify temperature setpoints and "
        "schedules.",
    });

    EXPECT_TRUE(compare_err_stream(error_string1, true));

    // configuration allows hot water mixing. A target temp schedule exists with either a hot temp schedule or a connections object
    EXPECT_TRUE(thisWaterEquipment.allowHotControl);
    EXPECT_NE(thisWaterEquipment.targetTempSched, nullptr);
    EXPECT_TRUE(thisWaterEquipment.hotTempSched != nullptr || thisWaterEquipment.Connections);
    EXPECT_GT(thisWaterEquipment.HotMassFlowRate, 0.0);
    EXPECT_NEAR(thisWaterEquipment.ColdMassFlowRate + thisWaterEquipment.HotMassFlowRate, thisWaterEquipment.TotalMassFlowRate, 0.00000001);

    // Reset hot water temperature to 43.3C
    state->dataLoopNodes->Node(1).Temp = 43.3;
    thisWaterConnections.InitConnections(*state);

    // Set target temperature to 50C, above hot water temperature to trigger warning
    Sched::GetSchedule(*state, "WATER EQUIPMENT TEMP SCHED")->currentVal = 50;
    thisWaterEquipment.WaterEquipmentType::CalcEquipmentFlowRates(*state);

    std::string const error_string2 = delimited_string({
        "   ** Warning ** CalcEquipmentFlowRates: \"CORE_ZN WATER EQUIPMENT\" - Target water temperature is greater than the hot water temperature "
        "by (6.70 C)",
        "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:15",
        "   **   ~~~   ** ...target water temperature     = 50.00 C",
        "   **   ~~~   ** ...hot water temperature        = 43.30 C",
        "   **   ~~~   ** ...Target water temperature should be less than or equal to the hot water temperature. Verify temperature setpoints and "
        "schedules.",
    });

    EXPECT_TRUE(compare_err_stream(error_string2, true));
    EXPECT_GT(thisWaterEquipment.HotMassFlowRate, 0.0);
    EXPECT_NEAR(thisWaterEquipment.ColdMassFlowRate + thisWaterEquipment.HotMassFlowRate, thisWaterEquipment.TotalMassFlowRate, 0.00000001);

    // Set target temperature to 0C, below cold water temperature to trigger warning
    Sched::GetSchedule(*state, "WATER EQUIPMENT TEMP SCHED")->currentVal = 0;
    thisWaterEquipment.WaterEquipmentType::CalcEquipmentFlowRates(*state);

    std::string const error_string3 = delimited_string({
        "   ** Warning ** CalcEquipmentFlowRates: \"CORE_ZN WATER EQUIPMENT\" - Target water temperature is less than the cold water temperature "
        "by (15.00 C)",
        "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:15",
        "   **   ~~~   ** ...target water temperature     = 0.00 C",
        "   **   ~~~   ** ...cold water temperature       = 15.00 C",
        "   **   ~~~   ** ...Target water temperature should be greater than or equal to the cold water temperature. Verify temperature setpoints "
        "and schedules.",
    });

    EXPECT_TRUE(compare_err_stream(error_string3, true));
    EXPECT_GT(thisWaterEquipment.ColdMassFlowRate, 0.0);
    EXPECT_NEAR(thisWaterEquipment.ColdMassFlowRate + thisWaterEquipment.HotMassFlowRate, thisWaterEquipment.TotalMassFlowRate, 0.00000001);
}

TEST_F(EnergyPlusFixture, WaterUse_BranchFlowRequest_PipeUpstreamOfConnections)
{
    // Reproduces https://github.com/NatLabRockies/EnergyPlus/issues/10165
    //
    // Same SWHSys1 plant loop as WaterUse_WaterTempWarnings, except the demand load branch
    // now has a Pipe:Adiabatic UPSTREAM of the WaterUse:Connections component, both on the
    // same branch (mirroring the reported Pipe:Outdoor-before-WaterUse:Connections scenario;
    // any passive pipe type hits the same code path). Before the fix, the branch's flow
    // request was read from the branch's first-component (the pipe's) inlet node, which never
    // receives a flow request, so BranchData::DetermineBranchFlowRequest returned ~0 for this
    // branch and splitter/mixer flow resolution sent essentially all flow through the parallel
    // bypass branch instead. After the fix, the request is taken from the branch's Active
    // component (the WaterUse:Connections), regardless of its position on the branch.
    bool ErrorsFound(false);

    std::string_view constexpr idf_objects = R"IDF(
  Zone,
    Core_ZN,                 !- Name
    0.0000,                  !- Direction of Relative North {deg}
    0.0000,                  !- X Origin {m}
    0.0000,                  !- Y Origin {m}
    0.0000,                  !- Z Origin {m}
    1,                       !- Type
    1,                       !- Multiplier
    ,                        !- Ceiling Height {m}
    ,                        !- Volume {m3}
    autocalculate,           !- Floor Area {m2}
    ,                        !- Zone Inside Convection Algorithm
    ,                        !- Zone Outside Convection Algorithm
    Yes;                     !- Part of Total Floor Area

  Sizing:Plant,
    SWHSys1,                 !- Plant or Condenser Loop Name
    Heating,                 !- Loop Type
    60,                      !- Design Loop Exit Temperature {C}
    5.0;                     !- Loop Design Temperature Difference {deltaC}

  SetpointManager:Scheduled,
    SWHSys1 Loop Setpoint Manager,  !- Name
    Temperature,             !- Control Variable
    SWHSys1-Loop-Temp-Schedule,  !- Schedule Name
    SWHSys1 Supply Outlet Node;  !- Setpoint Node or NodeList Name

  PlantEquipmentOperationSchemes,
    SWHSys1 Loop Operation Scheme List,  !- Name
    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type
    SWHSys1 Operation Scheme,!- Control Scheme 1 Name
    ALWAYS_ON;               !- Control Scheme 1 Schedule Name

  PlantEquipmentOperation:HeatingLoad,
    SWHSys1 Operation Scheme,!- Name
    0.0,                     !- Load Range 1 Lower Limit {W}
    1000000000000000,        !- Load Range 1 Upper Limit {W}
    SWHSys1 Equipment List;  !- Range 1 Equipment List Name

! ***SWH LOOP***

  PlantLoop,
    SWHSys1,                 !- Name
    Water,                   !- Fluid Type
    ,                        !- User Defined Fluid Type
    SWHSys1 Loop Operation Scheme List,  !- Plant Equipment Operation Scheme Name
    SWHSys1 Supply Outlet Node,  !- Loop Temperature Setpoint Node Name
    60.0,                    !- Maximum Loop Temperature {C}
    10.0,                    !- Minimum Loop Temperature {C}
    AUTOSIZE,                !- Maximum Loop Flow Rate {m3/s}
    0.0,                     !- Minimum Loop Flow Rate {m3/s}
    AUTOSIZE,                !- Plant Loop Volume {m3}
    SWHSys1 Supply Inlet Node,  !- Plant Side Inlet Node Name
    SWHSys1 Supply Outlet Node,  !- Plant Side Outlet Node Name
    SWHSys1 Supply Branches, !- Plant Side Branch List Name
    SWHSys1 Supply Connectors,  !- Plant Side Connector List Name
    SWHSys1 Demand Inlet Node,  !- Demand Side Inlet Node Name
    SWHSys1 Demand Outlet Node,  !- Demand Side Outlet Node Name
    SWHSys1 Demand Branches, !- Demand Side Branch List Name
    SWHSys1 Demand Connectors,  !- Demand Side Connector List Name
    Optimal;                 !- Load Distribution Scheme

! ***SWH CONNECTIONS***

  BranchList,
    SWHSys1 Demand Branches, !- Name
    SWHSys1 Demand Inlet Branch,  !- Branch 1 Name
    SWHSys1 Demand Load Branch 1,  !- Branch 2 Name
    SWHSys1 Demand Bypass Branch,  !- Branch 3 Name
    SWHSys1 Demand Outlet Branch;  !- Branch 4 Name

  BranchList,
    SWHSys1 Supply Branches, !- Name
    SWHSys1 Supply Inlet Branch,  !- Branch 1 Name
    SWHSys1 Supply Equipment Branch,  !- Branch 2 Name
    SWHSys1 Supply Equipment Bypass Branch,  !- Branch 3 Name
    SWHSys1 Supply Outlet Branch;  !- Branch 4 Name

  Branch,
    SWHSys1 Demand Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Bypass Pipe,  !- Component 1 Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;

  Branch,
    SWHSys1 Demand Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Inlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;

  Branch,
    SWHSys1 Demand Load Branch 1,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    Core_ZN Water Equipment Inlet Pipe,  !- Component 1 Name
    Core_ZN Water Equipment Water Inlet Node,  !- Component 1 Inlet Node Name
    Core_ZN Water Equipment Pipe Outlet Node,  !- Component 1 Outlet Node Name
    WaterUse:Connections,    !- Component 2 Object Type
    Core_ZN Water Equipment, !- Component 2 Name
    Core_ZN Water Equipment Pipe Outlet Node,  !- Component 2 Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node;  !- Component 2 Outlet Node Name

  Branch,
    SWHSys1 Demand Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Outlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterHeater:Mixed,       !- Component 1 Object Type
    SWHSys1 Water Heater,    !- Component 1 Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Equipment Bypass Pipe,  !- Component 1 Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;

  Branch,
    SWHSys1 Supply Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pump:ConstantSpeed,      !- Component 1 Object Type
    SWHSys1 Pump,            !- Component 1 Name
    SWHSys1 Supply Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector;

  Branch,
    SWHSys1 Supply Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Outlet Pipe,  !- Component 1 Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Outlet Node;

  ConnectorList,
    SWHSys1 Demand Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Demand Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Demand Mixer;    !- Connector 2 Name

  ConnectorList,
    SWHSys1 Supply Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Supply Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Supply Mixer;    !- Connector 2 Name

  Connector:Splitter,
    SWHSys1 Demand Splitter, !- Name
    SWHSys1 Demand Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Outlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Splitter,
    SWHSys1 Supply Splitter, !- Name
    SWHSys1 Supply Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Outlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Demand Mixer,    !- Name
    SWHSys1 Demand Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Inlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Inlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Supply Mixer,    !- Name
    SWHSys1 Supply Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Inlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Inlet Branch 2 Name

  Pipe:Adiabatic,
    SWHSys1 Demand Bypass Pipe,  !- Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Inlet Pipe,  !- Name
    SWHSys1 Demand Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;  !- Outlet Node Name

  Pipe:Adiabatic,
    Core_ZN Water Equipment Inlet Pipe,  !- Name
    Core_ZN Water Equipment Water Inlet Node,  !- Inlet Node Name
    Core_ZN Water Equipment Pipe Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Outlet Pipe,  !- Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Demand Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Equipment Bypass Pipe,  !- Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Outlet Pipe,  !- Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Supply Outlet Node;  !- Outlet Node Name

! ***SWH SCHEDULES***

  ScheduleTypeLimits,
    Fraction,                !- Name
    0.0,                     !- Lower Limit Value
    1.0,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  ScheduleTypeLimits,
    Temperature,             !- Name
    -60,                     !- Lower Limit Value
    200,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  Schedule:Compact,
    Water Equipment Latent fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.05;       !- Field 3

  Schedule:Compact,
    Water Equipment Sensible fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.2;        !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,22.0;       !- Field 3

  Schedule:Compact,
    Water Equipment Temp Sched,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  Schedule:Compact,
    SWHSys1-Loop-Temp-Schedule,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  PlantEquipmentList,
    SWHSys1 Equipment List,  !- Name
    WaterHeater:Mixed,       !- Equipment 1 Object Type
    SWHSys1 Water Heater;    !- Equipment 1 Name

  WaterHeater:Mixed,
    SWHSys1 Water Heater,    !- Name
    0.1514,                  !- Tank Volume {m3}
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Setpoint Temperature Schedule Name
    2.0,                     !- Deadband Temperature Difference {deltaC}
    82.2222,                 !- Maximum Temperature Limit {C}
    Cycle,                   !- Heater Control Type
    845000,                  !- Heater Maximum Capacity {W}
    ,                        !- Heater Minimum Capacity {W}
    ,                        !- Heater Ignition Minimum Flow Rate {m3/s}
    ,                        !- Heater Ignition Delay {s}
    NATURALGAS,              !- Heater Fuel Type
    0.8,                     !- Heater Thermal Efficiency
    ,                        !- Part Load Factor Curve Name
    20,                      !- Off Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- Off Cycle Parasitic Fuel Type
    0.8,                     !- Off Cycle Parasitic Heat Fraction to Tank
    ,                        !- On Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- On Cycle Parasitic Fuel Type
    ,                        !- On Cycle Parasitic Heat Fraction to Tank
    SCHEDULE,                !- Ambient Temperature Indicator
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Ambient Temperature Schedule Name
    ,                        !- Ambient Temperature Zone Name
    ,                        !- Ambient Temperature Outdoor Air Node Name
    6.0,                     !- Off Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- Off Cycle Loss Fraction to Zone
    6.0,                     !- On Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- On Cycle Loss Fraction to Zone
    ,                        !- Peak Use Flow Rate {m3/s}
    ,                        !- Use Flow Rate Fraction Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Use Side Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node,  !- Use Side Outlet Node Name
    1.0,                     !- Use Side Effectiveness
    ,                        !- Source Side Inlet Node Name
    ,                        !- Source Side Outlet Node Name
    1.0,                     !- Source Side Effectiveness
    AUTOSIZE,                !- Use Side Design Flow Rate {m3/s}
    AUTOSIZE,                !- Source Side Design Flow Rate {m3/s}
    1.5;                     !- Indirect Water Heating Recovery Time {hr}

  Pump:ConstantSpeed,
    SWHSys1 Pump,            !- Name
    SWHSys1 Supply Inlet Node,  !- Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector,  !- Outlet Node Name
    AUTOSIZE,                !- Rated Flow Rate {m3/s}
    0.001,                   !- Rated Pump Head {Pa}
    AUTOSIZE,                !- Rated Power Consumption {W}
    1,                       !- Motor Efficiency
    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream
    Intermittent,            !- Pump Control Type
    ;                        !- Pump Flow Rate Schedule Name

  WaterUse:Connections,
    Core_ZN Water Equipment, !- Name
    Core_ZN Water Equipment Pipe Outlet Node,  !- Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node,  !- Outlet Node Name
    ,                        !- Supply Water Storage Tank Name
    ,                        !- Reclamation Water Storage Tank Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    ,                        !- Drain Water Heat Exchanger Type
    ,                        !- Drain Water Heat Exchanger Destination
    ,                        !- Drain Water Heat Exchanger U-Factor Times Area {W/K}
    Core_ZN Water Equipment; !- Water Use Equipment 1 Name

  WaterUse:Equipment,
    Core_ZN Water Equipment, !- Name
    ,                        !- End-Use Subcategory
    1.0e-005,                !- Peak Flow Rate {m3/s}
    ,                        !- Flow Rate Fraction Schedule Name
    Water Equipment Temp Sched,  !- Target Temperature Schedule Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    Core_ZN,                 !- Zone Name
    Water Equipment Sensible fract sched,  !- Sensible Fraction Schedule Name
    Water Equipment Latent fract sched;  !- Latent Fraction Schedule Name
    )IDF";

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->CurrentTime = 0.25;
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, 101325.0, 20.0, 0.0);
    Sched::UpdateScheduleVals(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    InternalHeatGains::GetInternalHeatGainsInput(*state);

    // Run the real plant "get input" and branch-control-type setup pipeline, in the same
    // order HVACManager::SimHVAC does it (see HVACManager.cc), so the branch's controlType
    // is determined by the actual production logic rather than being hand-set.
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    GetWaterUseInput(*state);
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->WaterMainsTemp = 15;

    Real64 WaterConnNum = 1;
    auto &thisWaterConnections = state->dataWaterUse->WaterConnections(WaterConnNum);

    // Scans the plant loops for this WaterUse:Connections object to fill in thisWaterConnections.plantLoc,
    // which SetComponentFlowRate (called from CalcConnectionsFlowRates below) needs to find its branch/component.
    thisWaterConnections.oneTimeInit_new(*state);

    auto &demandLoadBranch = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(2);
    ASSERT_EQ(demandLoadBranch.TotalComponents, 2);

    // The pipe is passive and the WaterUse:Connections is the single Active component on the
    // branch, so the branch as a whole should be flagged Active (this is the real
    // SetupBranchControlTypes-computed control type, not a hand-set one).
    EXPECT_EQ(demandLoadBranch.Comp(1).FlowCtrl, DataBranchAirLoopPlant::ControlType::Passive);
    EXPECT_EQ(demandLoadBranch.Comp(2).FlowCtrl, DataBranchAirLoopPlant::ControlType::Active);
    EXPECT_EQ(demandLoadBranch.controlType, DataBranchAirLoopPlant::ControlType::Active);

    // Set a hot water temperature so CalcEquipmentFlowRates below computes a nonzero flow request
    state->dataLoopNodes->Node(thisWaterConnections.InletNode).Temp = 43.3;

    thisWaterConnections.InitConnections(*state);
    thisWaterConnections.CalcConnectionsFlowRates(*state);

    // The WaterUse:Connections component registered a nonzero flow request on its own inlet
    // node (the branch's SECOND node, downstream of the passive pipe) via SetComponentFlowRate.
    EXPECT_GT(thisWaterConnections.TotalMassFlowRate, 0.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisWaterConnections.InletNode).MassFlowRateRequest, thisWaterConnections.TotalMassFlowRate, 0.00000001);

    // The passive pipe never registers a request of its own, so the branch's first-component
    // (pipe) inlet node request stays at 0 -- this is exactly what used to fool
    // DetermineBranchFlowRequest into starving this branch in favor of the bypass branch.
    EXPECT_NEAR(state->dataLoopNodes->Node(demandLoadBranch.NodeNumIn).MassFlowRateRequest, 0.0, 0.00000001);

    // The fix: DetermineBranchFlowRequest must find the Active component's request rather than
    // trusting the branch's (pipe) inlet node.
    Real64 const branchFlowRequest = demandLoadBranch.DetermineBranchFlowRequest(*state);
    EXPECT_NEAR(branchFlowRequest, thisWaterConnections.TotalMassFlowRate, 0.00000001);
    EXPECT_GT(branchFlowRequest, 0.0);
}

TEST_F(EnergyPlusFixture, WaterUse_Default_Target_Temperature_Test1)
{
    // This is a unit test that checks the default target temperature associated with WaterUse:Equipment
    // This one should default to the hot water temperature
    bool ErrorsFound(false);

    std::string_view constexpr idf_objects = R"IDF(
  Zone,
    Core_ZN,                 !- Name
    0.0000,                  !- Direction of Relative North {deg}
    0.0000,                  !- X Origin {m}
    0.0000,                  !- Y Origin {m}
    0.0000,                  !- Z Origin {m}
    1,                       !- Type
    1,                       !- Multiplier
    ,                        !- Ceiling Height {m}
    ,                        !- Volume {m3}
    autocalculate,           !- Floor Area {m2}
    ,                        !- Zone Inside Convection Algorithm
    ,                        !- Zone Outside Convection Algorithm
    Yes;                     !- Part of Total Floor Area

  Sizing:Plant,
    SWHSys1,                 !- Plant or Condenser Loop Name
    Heating,                 !- Loop Type
    60,                      !- Design Loop Exit Temperature {C}
    5.0;                     !- Loop Design Temperature Difference {deltaC}

  SetpointManager:Scheduled,
    SWHSys1 Loop Setpoint Manager,  !- Name
    Temperature,             !- Control Variable
    SWHSys1-Loop-Temp-Schedule,  !- Schedule Name
    SWHSys1 Supply Outlet Node;  !- Setpoint Node or NodeList Name

  PlantEquipmentOperationSchemes,
    SWHSys1 Loop Operation Scheme List,  !- Name
    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type
    SWHSys1 Operation Scheme,!- Control Scheme 1 Name
    ALWAYS_ON;               !- Control Scheme 1 Schedule Name

  PlantEquipmentOperation:HeatingLoad,
    SWHSys1 Operation Scheme,!- Name
    0.0,                     !- Load Range 1 Lower Limit {W}
    1000000000000000,        !- Load Range 1 Upper Limit {W}
    SWHSys1 Equipment List;  !- Range 1 Equipment List Name

! ***SWH LOOP***

  PlantLoop,
    SWHSys1,                 !- Name
    Water,                   !- Fluid Type
    ,                        !- User Defined Fluid Type
    SWHSys1 Loop Operation Scheme List,  !- Plant Equipment Operation Scheme Name
    SWHSys1 Supply Outlet Node,  !- Loop Temperature Setpoint Node Name
    60.0,                    !- Maximum Loop Temperature {C}
    10.0,                    !- Minimum Loop Temperature {C}
    AUTOSIZE,                !- Maximum Loop Flow Rate {m3/s}
    0.0,                     !- Minimum Loop Flow Rate {m3/s}
    AUTOSIZE,                !- Plant Loop Volume {m3}
    SWHSys1 Supply Inlet Node,  !- Plant Side Inlet Node Name
    SWHSys1 Supply Outlet Node,  !- Plant Side Outlet Node Name
    SWHSys1 Supply Branches, !- Plant Side Branch List Name
    SWHSys1 Supply Connectors,  !- Plant Side Connector List Name
    SWHSys1 Demand Inlet Node,  !- Demand Side Inlet Node Name
    SWHSys1 Demand Outlet Node,  !- Demand Side Outlet Node Name
    SWHSys1 Demand Branches, !- Demand Side Branch List Name
    SWHSys1 Demand Connectors,  !- Demand Side Connector List Name
    Optimal;                 !- Load Distribution Scheme

! ***SWH CONNECTIONS***

  BranchList,
    SWHSys1 Demand Branches, !- Name
    SWHSys1 Demand Inlet Branch,  !- Branch 1 Name
    SWHSys1 Demand Load Branch 1,  !- Branch 2 Name
    SWHSys1 Demand Bypass Branch,  !- Branch 3 Name
    SWHSys1 Demand Outlet Branch;  !- Branch 4 Name

  BranchList,
    SWHSys1 Supply Branches, !- Name
    SWHSys1 Supply Inlet Branch,  !- Branch 1 Name
    SWHSys1 Supply Equipment Branch,  !- Branch 2 Name
    SWHSys1 Supply Equipment Bypass Branch,  !- Branch 3 Name
    SWHSys1 Supply Outlet Branch;  !- Branch 4 Name

  Branch,
    SWHSys1 Demand Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Bypass Pipe,  !- Component 1 Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;

  Branch,
    SWHSys1 Demand Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Inlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;

  Branch,
    SWHSys1 Demand Load Branch 1,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterUse:Connections,    !- Component 1 Object Type
    Core_ZN Water Equipment, !- Component 1 Name
    Core_ZN Water Equipment Water Inlet Node,  !- Component 1 Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node;

  Branch,
    SWHSys1 Demand Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Outlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterHeater:Mixed,       !- Component 1 Object Type
    SWHSys1 Water Heater,    !- Component 1 Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Equipment Bypass Pipe,  !- Component 1 Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;

  Branch,
    SWHSys1 Supply Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pump:ConstantSpeed,      !- Component 1 Object Type
    SWHSys1 Pump,            !- Component 1 Name
    SWHSys1 Supply Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector;

  Branch,
    SWHSys1 Supply Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Outlet Pipe,  !- Component 1 Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Outlet Node;

  ConnectorList,
    SWHSys1 Demand Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Demand Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Demand Mixer;    !- Connector 2 Name

  ConnectorList,
    SWHSys1 Supply Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Supply Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Supply Mixer;    !- Connector 2 Name

  Connector:Splitter,
    SWHSys1 Demand Splitter, !- Name
    SWHSys1 Demand Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Outlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Splitter,
    SWHSys1 Supply Splitter, !- Name
    SWHSys1 Supply Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Outlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Demand Mixer,    !- Name
    SWHSys1 Demand Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Inlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Inlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Supply Mixer,    !- Name
    SWHSys1 Supply Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Inlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Inlet Branch 2 Name

  Pipe:Adiabatic,
    SWHSys1 Demand Bypass Pipe,  !- Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Inlet Pipe,  !- Name
    SWHSys1 Demand Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Outlet Pipe,  !- Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Demand Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Equipment Bypass Pipe,  !- Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Outlet Pipe,  !- Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Supply Outlet Node;  !- Outlet Node Name

! ***SWH SCHEDULES***

  ScheduleTypeLimits,
    Fraction,                !- Name
    0.0,                     !- Lower Limit Value
    1.0,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  ScheduleTypeLimits,
    Temperature,             !- Name
    -60,                     !- Lower Limit Value
    200,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  Schedule:Compact,
    Water Equipment Latent fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.05;       !- Field 3

  Schedule:Compact,
    Water Equipment Sensible fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.2;        !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,22.0;       !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  Schedule:Compact,
    SWHSys1-Loop-Temp-Schedule,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  PlantEquipmentList,
    SWHSys1 Equipment List,  !- Name
    WaterHeater:Mixed,       !- Equipment 1 Object Type
    SWHSys1 Water Heater;    !- Equipment 1 Name

  WaterHeater:Mixed,
    SWHSys1 Water Heater,    !- Name
    0.1514,                  !- Tank Volume {m3}
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Setpoint Temperature Schedule Name
    2.0,                     !- Deadband Temperature Difference {deltaC}
    82.2222,                 !- Maximum Temperature Limit {C}
    Cycle,                   !- Heater Control Type
    845000,                  !- Heater Maximum Capacity {W}
    ,                        !- Heater Minimum Capacity {W}
    ,                        !- Heater Ignition Minimum Flow Rate {m3/s}
    ,                        !- Heater Ignition Delay {s}
    NATURALGAS,              !- Heater Fuel Type
    0.8,                     !- Heater Thermal Efficiency
    ,                        !- Part Load Factor Curve Name
    20,                      !- Off Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- Off Cycle Parasitic Fuel Type
    0.8,                     !- Off Cycle Parasitic Heat Fraction to Tank
    ,                        !- On Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- On Cycle Parasitic Fuel Type
    ,                        !- On Cycle Parasitic Heat Fraction to Tank
    SCHEDULE,                !- Ambient Temperature Indicator
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Ambient Temperature Schedule Name
    ,                        !- Ambient Temperature Zone Name
    ,                        !- Ambient Temperature Outdoor Air Node Name
    6.0,                     !- Off Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- Off Cycle Loss Fraction to Zone
    6.0,                     !- On Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- On Cycle Loss Fraction to Zone
    ,                        !- Peak Use Flow Rate {m3/s}
    ,                        !- Use Flow Rate Fraction Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Use Side Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node,  !- Use Side Outlet Node Name
    1.0,                     !- Use Side Effectiveness
    ,                        !- Source Side Inlet Node Name
    ,                        !- Source Side Outlet Node Name
    1.0,                     !- Source Side Effectiveness
    AUTOSIZE,                !- Use Side Design Flow Rate {m3/s}
    AUTOSIZE,                !- Source Side Design Flow Rate {m3/s}
    1.5;                     !- Indirect Water Heating Recovery Time {hr}

  Pump:ConstantSpeed,
    SWHSys1 Pump,            !- Name
    SWHSys1 Supply Inlet Node,  !- Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector,  !- Outlet Node Name
    AUTOSIZE,                !- Rated Flow Rate {m3/s}
    0.001,                   !- Rated Pump Head {Pa}
    AUTOSIZE,                !- Rated Power Consumption {W}
    1,                       !- Motor Efficiency
    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream
    Intermittent,            !- Pump Control Type
    ;                        !- Pump Flow Rate Schedule Name

  WaterUse:Connections,
    Core_ZN Water Equipment, !- Name
    Core_ZN Water Equipment Water Inlet Node,  !- Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node,  !- Outlet Node Name
    ,                        !- Supply Water Storage Tank Name
    ,                        !- Reclamation Water Storage Tank Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    ,                        !- Drain Water Heat Exchanger Type
    ,                        !- Drain Water Heat Exchanger Destination
    ,                        !- Drain Water Heat Exchanger U-Factor Times Area {W/K}
    Core_ZN Water Equipment; !- Water Use Equipment 1 Name

  WaterUse:Equipment,
    Core_ZN Water Equipment, !- Name
    ,                        !- End-Use Subcategory
    1.0e-005,                !- Peak Flow Rate {m3/s}
    ,                        !- Flow Rate Fraction Schedule Name
    ,                        !- Target Temperature Schedule Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    Core_ZN,                 !- Zone Name
    Water Equipment Sensible fract sched,  !- Sensible Fraction Schedule Name
    Water Equipment Latent fract sched;  !- Latent Fraction Schedule Name
    )IDF";

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, 101325.0, 20.0, 0.0);
    Sched::UpdateScheduleVals(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    InternalHeatGains::GetInternalHeatGainsInput(*state);
    Real64 WaterConnNum = 1;
    GetWaterUseInput(*state);
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->WaterMainsTemp = 15;

    // Set plant loop temperature to 45C
    state->dataLoopNodes->Node(1).Temp = 45;
    Real64 WaterEquipNum = 1;
    auto &thisWaterConnections = state->dataWaterUse->WaterConnections(WaterConnNum);
    auto &thisWaterEquipment = state->dataWaterUse->WaterEquipment(WaterEquipNum);
    thisWaterConnections.InitConnections(*state);
    thisWaterEquipment.WaterEquipmentType::CalcEquipmentFlowRates(*state);

    // The target temp will default to the hot water temperature if there exists either a hot temp schedule or a water use connections object.
    EXPECT_TRUE(thisWaterEquipment.allowHotControl);
    EXPECT_EQ(thisWaterEquipment.targetTempSched, nullptr);
    EXPECT_NEAR(thisWaterEquipment.TargetTemp, 45, 1e-5);

    EXPECT_TRUE(thisWaterEquipment.hotTempSched != nullptr || thisWaterEquipment.Connections);
    EXPECT_GT(thisWaterEquipment.HotMassFlowRate, 0.0);
    EXPECT_NEAR(thisWaterEquipment.ColdMassFlowRate, 0.0, 1e-5);
    EXPECT_NEAR(thisWaterEquipment.ColdMassFlowRate + thisWaterEquipment.HotMassFlowRate, thisWaterEquipment.TotalMassFlowRate, 1e-8);
}

TEST_F(EnergyPlusFixture, WaterUse_Default_Target_Temperature_Test2)
{
    // This is another unit test that checks the default target temperature associated with WaterUse:Equipment
    // This time defaults to the cold water temperature
    bool ErrorsFound(false);

    std::string_view constexpr idf_objects = R"IDF(
  Zone,
    Core_ZN,                 !- Name
    0.0000,                  !- Direction of Relative North {deg}
    0.0000,                  !- X Origin {m}
    0.0000,                  !- Y Origin {m}
    0.0000,                  !- Z Origin {m}
    1,                       !- Type
    1,                       !- Multiplier
    ,                        !- Ceiling Height {m}
    ,                        !- Volume {m3}
    autocalculate,           !- Floor Area {m2}
    ,                        !- Zone Inside Convection Algorithm
    ,                        !- Zone Outside Convection Algorithm
    Yes;                     !- Part of Total Floor Area

  Sizing:Plant,
    SWHSys1,                 !- Plant or Condenser Loop Name
    Heating,                 !- Loop Type
    60,                      !- Design Loop Exit Temperature {C}
    5.0;                     !- Loop Design Temperature Difference {deltaC}

  SetpointManager:Scheduled,
    SWHSys1 Loop Setpoint Manager,  !- Name
    Temperature,             !- Control Variable
    SWHSys1-Loop-Temp-Schedule,  !- Schedule Name
    SWHSys1 Supply Outlet Node;  !- Setpoint Node or NodeList Name

  PlantEquipmentOperationSchemes,
    SWHSys1 Loop Operation Scheme List,  !- Name
    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type
    SWHSys1 Operation Scheme,!- Control Scheme 1 Name
    ALWAYS_ON;               !- Control Scheme 1 Schedule Name

  PlantEquipmentOperation:HeatingLoad,
    SWHSys1 Operation Scheme,!- Name
    0.0,                     !- Load Range 1 Lower Limit {W}
    1000000000000000,        !- Load Range 1 Upper Limit {W}
    SWHSys1 Equipment List;  !- Range 1 Equipment List Name

! ***SWH LOOP***

  PlantLoop,
    SWHSys1,                 !- Name
    Water,                   !- Fluid Type
    ,                        !- User Defined Fluid Type
    SWHSys1 Loop Operation Scheme List,  !- Plant Equipment Operation Scheme Name
    SWHSys1 Supply Outlet Node,  !- Loop Temperature Setpoint Node Name
    60.0,                    !- Maximum Loop Temperature {C}
    10.0,                    !- Minimum Loop Temperature {C}
    AUTOSIZE,                !- Maximum Loop Flow Rate {m3/s}
    0.0,                     !- Minimum Loop Flow Rate {m3/s}
    AUTOSIZE,                !- Plant Loop Volume {m3}
    SWHSys1 Supply Inlet Node,  !- Plant Side Inlet Node Name
    SWHSys1 Supply Outlet Node,  !- Plant Side Outlet Node Name
    SWHSys1 Supply Branches, !- Plant Side Branch List Name
    SWHSys1 Supply Connectors,  !- Plant Side Connector List Name
    SWHSys1 Demand Inlet Node,  !- Demand Side Inlet Node Name
    SWHSys1 Demand Outlet Node,  !- Demand Side Outlet Node Name
    SWHSys1 Demand Branches, !- Demand Side Branch List Name
    SWHSys1 Demand Connectors,  !- Demand Side Connector List Name
    Optimal;                 !- Load Distribution Scheme

! ***SWH CONNECTIONS***

  BranchList,
    SWHSys1 Demand Branches, !- Name
    SWHSys1 Demand Inlet Branch,  !- Branch 1 Name
    SWHSys1 Demand Load Branch 1,  !- Branch 2 Name
    SWHSys1 Demand Bypass Branch,  !- Branch 3 Name
    SWHSys1 Demand Outlet Branch;  !- Branch 4 Name

  BranchList,
    SWHSys1 Supply Branches, !- Name
    SWHSys1 Supply Inlet Branch,  !- Branch 1 Name
    SWHSys1 Supply Equipment Branch,  !- Branch 2 Name
    SWHSys1 Supply Equipment Bypass Branch,  !- Branch 3 Name
    SWHSys1 Supply Outlet Branch;  !- Branch 4 Name

  Branch,
    SWHSys1 Demand Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Bypass Pipe,  !- Component 1 Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;

  Branch,
    SWHSys1 Demand Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Inlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;

  Branch,
    SWHSys1 Demand Load Branch 1,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterUse:Connections,    !- Component 1 Object Type
    Core_ZN Water Equipment, !- Component 1 Name
    Core_ZN Water Equipment Water Inlet Node,  !- Component 1 Inlet Node Name
    Core_ZN Water Equipment Water Outlet Node;

  Branch,
    SWHSys1 Demand Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Demand Outlet Pipe,  !- Component 1 Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Demand Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    WaterHeater:Mixed,       !- Component 1 Object Type
    SWHSys1 Water Heater,    !- Component 1 Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node;

  Branch,
    SWHSys1 Supply Equipment Bypass Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Equipment Bypass Pipe,  !- Component 1 Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;

  Branch,
    SWHSys1 Supply Inlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pump:ConstantSpeed,      !- Component 1 Object Type
    SWHSys1 Pump,            !- Component 1 Name
    SWHSys1 Supply Inlet Node,  !- Component 1 Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector;

  Branch,
    SWHSys1 Supply Outlet Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    Pipe:Adiabatic,          !- Component 1 Object Type
    SWHSys1 Supply Outlet Pipe,  !- Component 1 Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Component 1 Inlet Node Name
    SWHSys1 Supply Outlet Node;

  ConnectorList,
    SWHSys1 Demand Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Demand Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Demand Mixer;    !- Connector 2 Name

  ConnectorList,
    SWHSys1 Supply Connectors,  !- Name
    Connector:Splitter,      !- Connector 1 Object Type
    SWHSys1 Supply Splitter, !- Connector 1 Name
    Connector:Mixer,         !- Connector 2 Object Type
    SWHSys1 Supply Mixer;    !- Connector 2 Name

  Connector:Splitter,
    SWHSys1 Demand Splitter, !- Name
    SWHSys1 Demand Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Outlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Splitter,
    SWHSys1 Supply Splitter, !- Name
    SWHSys1 Supply Inlet Branch,  !- Inlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Outlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Outlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Demand Mixer,    !- Name
    SWHSys1 Demand Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Demand Load Branch 1,  !- Inlet Branch 1 Name
    SWHSys1 Demand Bypass Branch;  !- Inlet Branch 2 Name

  Connector:Mixer,
    SWHSys1 Supply Mixer,    !- Name
    SWHSys1 Supply Outlet Branch,  !- Outlet Branch Name
    SWHSys1 Supply Equipment Branch,  !- Inlet Branch 1 Name
    SWHSys1 Supply Equipment Bypass Branch;  !- Inlet Branch 2 Name

  Pipe:Adiabatic,
    SWHSys1 Demand Bypass Pipe,  !- Name
    SWHSys1 Demand Bypass Pipe Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Bypass Pipe Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Inlet Pipe,  !- Name
    SWHSys1 Demand Inlet Node,  !- Inlet Node Name
    SWHSys1 Demand Inlet Pipe-SWHSys1 Demand Mixer;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Demand Outlet Pipe,  !- Name
    SWHSys1 Demand Mixer-SWHSys1 Demand Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Demand Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Equipment Bypass Pipe,  !- Name
    SWHSys1 Supply Equip Bypass Inlet Node,  !- Inlet Node Name
    SWHSys1 Supply Equip Bypass Outlet Node;  !- Outlet Node Name

  Pipe:Adiabatic,
    SWHSys1 Supply Outlet Pipe,  !- Name
    SWHSys1 Supply Mixer-SWHSys1 Supply Outlet Pipe,  !- Inlet Node Name
    SWHSys1 Supply Outlet Node;  !- Outlet Node Name

! ***SWH SCHEDULES***

  ScheduleTypeLimits,
    Fraction,                !- Name
    0.0,                     !- Lower Limit Value
    1.0,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  ScheduleTypeLimits,
    Temperature,             !- Name
    -60,                     !- Lower Limit Value
    200,                     !- Upper Limit Value
    CONTINUOUS;              !- Numeric Type

  Schedule:Compact,
    Water Equipment Latent fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.05;       !- Field 3

  Schedule:Compact,
    Water Equipment Sensible fract sched,  !- Name
    Fraction,                !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,0.2;        !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,22.0;       !- Field 3

  Schedule:Compact,
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  Schedule:Compact,
    SWHSys1-Loop-Temp-Schedule,  !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,43.3;       !- Field 3

  PlantEquipmentList,
    SWHSys1 Equipment List,  !- Name
    WaterHeater:Mixed,       !- Equipment 1 Object Type
    SWHSys1 Water Heater;    !- Equipment 1 Name

  WaterHeater:Mixed,
    SWHSys1 Water Heater,    !- Name
    0.1514,                  !- Tank Volume {m3}
    SWHSys1 Water Heater Setpoint Temperature Schedule Name,  !- Setpoint Temperature Schedule Name
    2.0,                     !- Deadband Temperature Difference {deltaC}
    82.2222,                 !- Maximum Temperature Limit {C}
    Cycle,                   !- Heater Control Type
    845000,                  !- Heater Maximum Capacity {W}
    ,                        !- Heater Minimum Capacity {W}
    ,                        !- Heater Ignition Minimum Flow Rate {m3/s}
    ,                        !- Heater Ignition Delay {s}
    NATURALGAS,              !- Heater Fuel Type
    0.8,                     !- Heater Thermal Efficiency
    ,                        !- Part Load Factor Curve Name
    20,                      !- Off Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- Off Cycle Parasitic Fuel Type
    0.8,                     !- Off Cycle Parasitic Heat Fraction to Tank
    ,                        !- On Cycle Parasitic Fuel Consumption Rate {W}
    NATURALGAS,              !- On Cycle Parasitic Fuel Type
    ,                        !- On Cycle Parasitic Heat Fraction to Tank
    SCHEDULE,                !- Ambient Temperature Indicator
    SWHSys1 Water Heater Ambient Temperature Schedule Name,  !- Ambient Temperature Schedule Name
    ,                        !- Ambient Temperature Zone Name
    ,                        !- Ambient Temperature Outdoor Air Node Name
    6.0,                     !- Off Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- Off Cycle Loss Fraction to Zone
    6.0,                     !- On Cycle Loss Coefficient to Ambient Temperature {W/K}
    ,                        !- On Cycle Loss Fraction to Zone
    ,                        !- Peak Use Flow Rate {m3/s}
    ,                        !- Use Flow Rate Fraction Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    SWHSys1 Pump-SWHSys1 Water HeaterNode,  !- Use Side Inlet Node Name
    SWHSys1 Supply Equipment Outlet Node,  !- Use Side Outlet Node Name
    1.0,                     !- Use Side Effectiveness
    ,                        !- Source Side Inlet Node Name
    ,                        !- Source Side Outlet Node Name
    1.0,                     !- Source Side Effectiveness
    AUTOSIZE,                !- Use Side Design Flow Rate {m3/s}
    AUTOSIZE,                !- Source Side Design Flow Rate {m3/s}
    1.5;                     !- Indirect Water Heating Recovery Time {hr}

  Pump:ConstantSpeed,
    SWHSys1 Pump,            !- Name
    SWHSys1 Supply Inlet Node,  !- Inlet Node Name
    SWHSys1 Pump-SWHSys1 Water HeaterNodeviaConnector,  !- Outlet Node Name
    AUTOSIZE,                !- Rated Flow Rate {m3/s}
    0.001,                   !- Rated Pump Head {Pa}
    AUTOSIZE,                !- Rated Power Consumption {W}
    1,                       !- Motor Efficiency
    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream
    Intermittent,            !- Pump Control Type
    ;                        !- Pump Flow Rate Schedule Name

  WaterUse:Equipment,
    Core_ZN Water Equipment, !- Name
    ,                        !- End-Use Subcategory
    1.0e-005,                !- Peak Flow Rate {m3/s}
    ,                        !- Flow Rate Fraction Schedule Name
    ,                        !- Target Temperature Schedule Name
    ,                        !- Hot Water Supply Temperature Schedule Name
    ,                        !- Cold Water Supply Temperature Schedule Name
    Core_ZN,                 !- Zone Name
    Water Equipment Sensible fract sched,  !- Sensible Fraction Schedule Name
    Water Equipment Latent fract sched;  !- Latent Fraction Schedule Name
    )IDF";

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, 101325.0, 20.0, 0.0);
    Sched::UpdateScheduleVals(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    InternalHeatGains::GetInternalHeatGainsInput(*state);
    GetWaterUseInput(*state);
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->WaterMainsTemp = 15;

    Real64 WaterEquipNum = 1;
    auto &thisWaterEquipment = state->dataWaterUse->WaterEquipment(WaterEquipNum);
    thisWaterEquipment.WaterEquipmentType::CalcEquipmentFlowRates(*state);

    // A target temp will default to cold water temperature is there is neither a hot temp schedule nor a water use connections object.
    EXPECT_FALSE(thisWaterEquipment.allowHotControl);
    EXPECT_EQ(thisWaterEquipment.targetTempSched, nullptr);
    EXPECT_NEAR(thisWaterEquipment.TargetTemp, 15, 1e-5);

    EXPECT_FALSE((thisWaterEquipment.targetTempSched != nullptr && thisWaterEquipment.hotTempSched != nullptr) || thisWaterEquipment.Connections);
    EXPECT_GT(thisWaterEquipment.ColdMassFlowRate, 0.0);
    EXPECT_NEAR(thisWaterEquipment.HotMassFlowRate, 0.0, 1e-5);

    EXPECT_NEAR(thisWaterEquipment.ColdMassFlowRate + thisWaterEquipment.HotMassFlowRate, thisWaterEquipment.TotalMassFlowRate, 1e-8);
}

TEST_F(EnergyPlusFixture, WaterUse_calcH2ODensity_Test)
{
    Real64 functionResult;
    Real64 expectedResult;
    Real64 allowedTolerance = 0.001;

    state->init_state(*state);
    // Test 1: Set the flag to false means don't recalculate.  This means it wasn't set so this should be the default value.
    state->dataWaterUse->calcRhoH2O = false;
    expectedResult = 1000.0;
    functionResult = calcH2ODensity(*state);
    EXPECT_NEAR(functionResult, expectedResult, allowedTolerance);
    EXPECT_FALSE(state->dataWaterUse->calcRhoH2O);

    // Test 2: Now set the flag to true so that an actual value is calculated and flag should come back false.
    state->dataWaterUse->calcRhoH2O = true;
    expectedResult = 999.898;
    functionResult = calcH2ODensity(*state);
    EXPECT_NEAR(functionResult, expectedResult, allowedTolerance);
    EXPECT_FALSE(state->dataWaterUse->calcRhoH2O);

    // Test 2: Now the flag is still false.  The actual value should have been preserved and returned; flag should come back false.
    expectedResult = 999.898;
    functionResult = calcH2ODensity(*state);
    EXPECT_NEAR(functionResult, expectedResult, allowedTolerance);
    EXPECT_FALSE(state->dataWaterUse->calcRhoH2O);
}

namespace {

Real64 constexpr lockedLoopColdTemp = 10.0;
Real64 constexpr lockedLoopHotTemp = 55.0;
Real64 constexpr lockedLoopPeakVolFlowRate = 1.0e-4;

// Builds one WaterUse:Connections on a locked plant demand branch, with one WaterUse:Equipment (no zone) per target temperature.
// On a locked loop, SetComponentFlowRate returns the inlet node's mass flow rate, so that node flow is the hot water the plant delivers.
void setupLockedLoopWaterConnection(EnergyPlusData &state, std::vector<Real64> const &targetTemps)
{
    state.init_state(state);

    state.dataEnvrn->WaterMainsTemp = lockedLoopColdTemp;
    state.dataLoopNodes->Node.allocate(2);
    state.dataLoopNodes->Node(1).MassFlowRateMax = 1.0;
    state.dataLoopNodes->Node(1).Temp = lockedLoopHotTemp;

    state.dataPlnt->TotNumLoops = 1;
    state.dataPlnt->PlantLoop.allocate(1);
    auto &demandSide = state.dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand);
    demandSide.TotalBranches = 1;
    demandSide.Branch.allocate(1);
    demandSide.Branch(1).TotalComponents = 1;
    demandSide.Branch(1).Comp.allocate(1);
    demandSide.Branch(1).Comp(1).CurOpSchemeType = DataPlant::OpScheme::Demand;
    demandSide.FlowLock = DataPlant::FlowLock::Locked;

    int const numEquip = static_cast<int>(targetTemps.size());
    state.dataWaterUse->numWaterEquipment = numEquip;
    state.dataWaterUse->WaterEquipment.allocate(numEquip);
    state.dataWaterUse->numWaterConnections = 1;
    state.dataWaterUse->WaterConnections.allocate(1);
    auto &waterConnections = state.dataWaterUse->WaterConnections(1);
    waterConnections.InletNode = 1;
    waterConnections.OutletNode = 2;
    waterConnections.NumWaterEquipment = numEquip;
    waterConnections.myWaterEquipArr.allocate(numEquip);
    waterConnections.ColdTemp = lockedLoopColdTemp;
    waterConnections.HotTemp = lockedLoopHotTemp;
    waterConnections.plantLoc = PlantLocation(1, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantUtilities::SetPlantLocationLinks(state, waterConnections.plantLoc);

    for (int i = 1; i <= numEquip; ++i) {
        auto &waterEquipment = state.dataWaterUse->WaterEquipment(i);
        waterEquipment.Name = std::format("WATER EQUIPMENT {}", i);
        waterEquipment.Connections = 1;
        waterEquipment.PeakVolFlowRate = lockedLoopPeakVolFlowRate;
        waterEquipment.targetTempSched = Sched::AddScheduleConstant(state, std::format("TARGET TEMP {}", i), targetTemps[i - 1]);
        waterEquipment.allowHotControl = true;
        waterEquipment.setupMyOutputVars = false;
        waterConnections.myWaterEquipArr(i) = i;
    }
}

// Checks that each equipment's and the connection's hot + cold flow equals its total, the equipment flows sum to the connection
// flows, and each equipment's mixed temperature matches its hot/cold split
void expectConsistentWaterConnectionFlows(EnergyPlusData &state)
{
    auto const &waterConnections = state.dataWaterUse->WaterConnections(1);
    Real64 sumHot = 0.0;
    Real64 sumCold = 0.0;
    for (auto const &waterEquipment : state.dataWaterUse->WaterEquipment) {
        EXPECT_GE(waterEquipment.HotMassFlowRate, 0.0);
        EXPECT_GE(waterEquipment.ColdMassFlowRate, 0.0);
        EXPECT_NEAR(waterEquipment.HotMassFlowRate + waterEquipment.ColdMassFlowRate, waterEquipment.TotalMassFlowRate, 1.0e-10);
        EXPECT_NEAR(waterEquipment.MixedTemp,
                    (waterEquipment.HotMassFlowRate * lockedLoopHotTemp + waterEquipment.ColdMassFlowRate * lockedLoopColdTemp) /
                        waterEquipment.TotalMassFlowRate,
                    1.0e-8);
        sumHot += waterEquipment.HotMassFlowRate;
        sumCold += waterEquipment.ColdMassFlowRate;
    }
    EXPECT_GE(waterConnections.ColdMassFlowRate, 0.0);
    EXPECT_NEAR(waterConnections.HotMassFlowRate + waterConnections.ColdMassFlowRate, waterConnections.TotalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(sumHot, waterConnections.HotMassFlowRate, 1.0e-10);
    EXPECT_NEAR(sumCold, waterConnections.ColdMassFlowRate, 1.0e-10);
}

} // namespace

// Verifies that WaterUse:Connections reports the hot-water flow actually delivered by a locked plant loop, while cold-water makeup preserves
// the requested total fixture flow. The first HVAC iteration used to keep the delivered hot flow without adding cold makeup or rescaling the
// equipment flows, and later iterations rescaled the equipment flows without storing the delivered flow on the connection.
TEST_F(EnergyPlusFixture, WaterUse_ConnectionsHotFlowWhenPlantLimitsFlow)
{
    setupLockedLoopWaterConnection(*state, {40.0});
    auto &waterEquipment = state->dataWaterUse->WaterEquipment(1);
    auto &waterConnections = state->dataWaterUse->WaterConnections(1);

    Real64 const totalMassFlowRate = lockedLoopPeakVolFlowRate * calcH2ODensity(*state);
    Real64 const requestedHotMassFlowRate = totalMassFlowRate * (40.0 - 10.0) / (55.0 - 10.0);

    // Locked plant flow delivers only half of the requested hot water
    Real64 const deliveredHotMassFlowRate = 0.5 * requestedHotMassFlowRate;
    state->dataLoopNodes->Node(1).MassFlowRate = deliveredHotMassFlowRate;

    for (bool const firstHVACIteration : {true, false}) {
        SCOPED_TRACE(std::format("FirstHVACIteration = {}", firstHVACIteration));
        Real64 curLoad = 0.0;
        waterConnections.simulate(*state, waterConnections.plantLoc, firstHVACIteration, curLoad, true);

        EXPECT_NEAR(waterConnections.TotalMassFlowRate, totalMassFlowRate, 1.0e-10);
        EXPECT_NEAR(waterConnections.HotMassFlowRate, deliveredHotMassFlowRate, 1.0e-10);
        EXPECT_NEAR(waterConnections.ColdMassFlowRate, totalMassFlowRate - deliveredHotMassFlowRate, 1.0e-10);
        EXPECT_NEAR(waterEquipment.TotalMassFlowRate, totalMassFlowRate, 1.0e-10);
        EXPECT_NEAR(waterEquipment.HotMassFlowRate, deliveredHotMassFlowRate, 1.0e-10);
        EXPECT_NEAR(waterEquipment.ColdMassFlowRate, totalMassFlowRate - deliveredHotMassFlowRate, 1.0e-10);
        EXPECT_LT(waterEquipment.MixedTemp, 40.0);
        expectConsistentWaterConnectionFlows(*state);

        // Reported volume flows follow the rescaled mass flows
        EXPECT_NEAR(waterConnections.HotVolFlowRate, deliveredHotMassFlowRate / calcH2ODensity(*state), 1.0e-12);
        EXPECT_NEAR(waterEquipment.HotVolFlowRate, deliveredHotMassFlowRate / calcH2ODensity(*state), 1.0e-12);
    }
}

// A locked loop can deliver more hot water than was requested; the surplus replaces cold water at each equipment, and anything beyond
// the total fixture draw is ignored so that cold flow never goes negative
TEST_F(EnergyPlusFixture, WaterUse_ConnectionsHotFlowWhenPlantDeliversMoreThanRequested)
{
    // Equipment 1 mixes to 40 C (2/3 hot); equipment 2 targets the cold water temperature (all cold)
    setupLockedLoopWaterConnection(*state, {40.0, 10.0});
    auto &waterEquipment1 = state->dataWaterUse->WaterEquipment(1);
    auto &waterEquipment2 = state->dataWaterUse->WaterEquipment(2);
    auto &waterConnections = state->dataWaterUse->WaterConnections(1);

    Real64 const equipMassFlowRate = lockedLoopPeakVolFlowRate * calcH2ODensity(*state);
    Real64 const totalMassFlowRate = 2.0 * equipMassFlowRate;
    Real64 const requestedHotMassFlowRate = equipMassFlowRate * (40.0 - 10.0) / (55.0 - 10.0);

    // Deliver half of the cold flow as surplus hot water
    Real64 const deliveredHotMassFlowRate = requestedHotMassFlowRate + 0.5 * (totalMassFlowRate - requestedHotMassFlowRate);
    state->dataLoopNodes->Node(1).MassFlowRate = deliveredHotMassFlowRate;

    Real64 curLoad = 0.0;
    waterConnections.simulate(*state, waterConnections.plantLoc, true, curLoad, true);

    EXPECT_NEAR(waterConnections.TotalMassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.HotMassFlowRate, deliveredHotMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.PlantMassFlowRate, deliveredHotMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterEquipment1.HotMassFlowRate, requestedHotMassFlowRate + 0.5 * (equipMassFlowRate - requestedHotMassFlowRate), 1.0e-10);
    EXPECT_NEAR(waterEquipment2.HotMassFlowRate, 0.5 * equipMassFlowRate, 1.0e-10);
    EXPECT_GT(waterEquipment1.MixedTemp, 40.0);
    expectConsistentWaterConnectionFlows(*state);

    // Deliver three times the total fixture draw: everything is served hot and no cold water is used
    Real64 const plantMassFlowRate = 3.0 * totalMassFlowRate;
    state->dataLoopNodes->Node(1).MassFlowRate = plantMassFlowRate;

    waterConnections.simulate(*state, waterConnections.plantLoc, false, curLoad, true);

    EXPECT_NEAR(waterConnections.TotalMassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.HotMassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.PlantMassFlowRate, plantMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.ColdMassFlowRate, 0.0, 1.0e-10);
    EXPECT_NEAR(waterEquipment1.MixedTemp, lockedLoopHotTemp, 1.0e-8);
    EXPECT_NEAR(waterEquipment2.MixedTemp, lockedLoopHotTemp, 1.0e-8);
    expectConsistentWaterConnectionFlows(*state);

    // The unused plant flow bypasses the fixtures at the inlet temperature, so the plant outlet and reported load conserve energy
    Real64 const expectedPlantReturnTemp =
        (totalMassFlowRate * lockedLoopColdTemp + (plantMassFlowRate - totalMassFlowRate) * lockedLoopHotTemp) / plantMassFlowRate;
    EXPECT_NEAR(waterConnections.ReturnTemp, lockedLoopColdTemp, 1.0e-10);
    EXPECT_NEAR(waterConnections.PlantReturnTemp, expectedPlantReturnTemp, 1.0e-8);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).MassFlowRate, plantMassFlowRate, 1.0e-10);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).Temp, expectedPlantReturnTemp, 1.0e-8);
    EXPECT_NEAR(waterConnections.Power, waterEquipment1.Power + waterEquipment2.Power, 1.0e-8);

    // With no fixture draw, all locked plant flow bypasses at the inlet temperature and creates no plant load
    waterEquipment1.PeakVolFlowRate = 0.0;
    waterEquipment2.PeakVolFlowRate = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRate = totalMassFlowRate;

    waterConnections.simulate(*state, waterConnections.plantLoc, false, curLoad, true);

    EXPECT_EQ(waterConnections.TotalMassFlowRate, 0.0);
    EXPECT_EQ(waterConnections.HotMassFlowRate, 0.0);
    EXPECT_NEAR(waterConnections.PlantMassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.ReturnTemp, lockedLoopColdTemp, 1.0e-8);
    EXPECT_NEAR(waterConnections.PlantReturnTemp, lockedLoopHotTemp, 1.0e-8);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).MassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).Temp, lockedLoopHotTemp, 1.0e-8);
    EXPECT_NEAR(waterConnections.Power, 0.0, 1.0e-8);
}

// With drain water heat recovery to the plant, no hot water flow leaves the heat exchanger with zero capacity on the recovery side;
// the CrossFlow effectiveness used to divide by that zero capacity ratio and turn the recovery results into NaN
TEST_F(EnergyPlusFixture, WaterUse_ConnectionsCrossFlowHeatRecoveryWithNoHotFlow)
{
    setupLockedLoopWaterConnection(*state, {40.0});
    auto &waterEquipment = state->dataWaterUse->WaterEquipment(1);
    auto &waterConnections = state->dataWaterUse->WaterConnections(1);
    waterConnections.HeatRecovery = true;
    waterConnections.HeatRecoveryHX = HeatRecovHX::CrossFlow;
    waterConnections.HeatRecoveryConfig = HeatRecovConfig::Plant;
    waterConnections.HXUA = 1500.0;

    Real64 const totalMassFlowRate = lockedLoopPeakVolFlowRate * calcH2ODensity(*state);

    // The locked loop delivers no hot water
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;

    Real64 curLoad = 0.0;
    waterConnections.simulate(*state, waterConnections.plantLoc, false, curLoad, true);

    EXPECT_NEAR(waterConnections.TotalMassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterConnections.HotMassFlowRate, 0.0, 1.0e-10);
    EXPECT_NEAR(waterConnections.ColdMassFlowRate, totalMassFlowRate, 1.0e-10);
    EXPECT_NEAR(waterEquipment.MixedTemp, lockedLoopColdTemp, 1.0e-8);
    expectConsistentWaterConnectionFlows(*state);

    EXPECT_NEAR(waterConnections.RecoveryMassFlowRate, 0.0, 1.0e-10);
    EXPECT_EQ(waterConnections.Effectiveness, 0.0);
    EXPECT_EQ(waterConnections.RecoveryRate, 0.0);
    EXPECT_EQ(waterConnections.RecoveryEnergy, 0.0);
    EXPECT_NEAR(waterConnections.RecoveryTemp, lockedLoopColdTemp, 1.0e-10);
    EXPECT_NEAR(waterConnections.ReturnTemp, lockedLoopColdTemp, 1.0e-10);
    EXPECT_NEAR(waterConnections.WasteTemp, waterConnections.DrainTemp, 1.0e-10);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).Temp, lockedLoopColdTemp, 1.0e-10);
}

// With PlantAndEquipment destination and no latent load, the recovery and drain flows are equal, but round-off in the summed flows can leave
// the capacity ratio one ulp from 1.0; the general CounterFlow formula then cancels to a wrong effectiveness, which made the heat recovery
// temperature iteration oscillate without converging
TEST_F(EnergyPlusFixture, WaterUse_ConnectionsCounterFlowHeatRecoveryBalancedFlows)
{
    state->init_state(*state);
    state->dataWaterUse->WaterConnections.allocate(1);
    auto &waterConnections = state->dataWaterUse->WaterConnections(1);
    waterConnections.HeatRecovery = true;
    waterConnections.HeatRecoveryHX = HeatRecovHX::CounterFlow;
    waterConnections.HeatRecoveryConfig = HeatRecovConfig::PlantAndEquip;
    waterConnections.HXUA = 250.0;
    waterConnections.ColdSupplyTemp = 18.8;
    waterConnections.DrainTemp = 31.8;

    Real64 constexpr totalMassFlowRate = 0.05;
    waterConnections.TotalMassFlowRate = totalMassFlowRate;
    waterConnections.HotMassFlowRate = 0.02;
    waterConnections.ColdMassFlowRate = totalMassFlowRate - waterConnections.HotMassFlowRate;

    Real64 const NTU = waterConnections.HXUA / (Psychrometrics::CPHW(Constant::InitConvTemp) * totalMassFlowRate);
    Real64 const balancedEffectiveness = NTU / (1.0 + NTU);

    for (Real64 const drainMassFlowRate : {std::nextafter(totalMassFlowRate, 1.0), std::nextafter(totalMassFlowRate, 0.0)}) {
        waterConnections.DrainMassFlowRate = drainMassFlowRate;
        waterConnections.ColdTemp = waterConnections.ColdSupplyTemp;

        waterConnections.CalcConnectionsHeatRecovery(*state);

        EXPECT_NEAR(waterConnections.Effectiveness, balancedEffectiveness, 1.0e-9);
        EXPECT_NEAR(waterConnections.RecoveryTemp,
                    waterConnections.ColdSupplyTemp + balancedEffectiveness * (waterConnections.DrainTemp - waterConnections.ColdSupplyTemp),
                    1.0e-8);
    }
}

// When the fixtures stop drawing water, the heat recovery results must reset instead of reporting the last nonzero recovery flow
TEST_F(EnergyPlusFixture, WaterUse_ConnectionsHeatRecoveryResetsAtZeroFlow)
{
    setupLockedLoopWaterConnection(*state, {40.0});
    auto &waterEquipment = state->dataWaterUse->WaterEquipment(1);
    auto &waterConnections = state->dataWaterUse->WaterConnections(1);
    waterConnections.HeatRecovery = true;
    waterConnections.HeatRecoveryHX = HeatRecovHX::CounterFlow;
    waterConnections.HeatRecoveryConfig = HeatRecovConfig::Plant;
    waterConnections.HXUA = 250.0;

    // The plant delivers the requested hot water, so heat is recovered into it
    Real64 const totalMassFlowRate = lockedLoopPeakVolFlowRate * calcH2ODensity(*state);
    Real64 const requestedHotMassFlowRate = totalMassFlowRate * (40.0 - 10.0) / (55.0 - 10.0);
    state->dataLoopNodes->Node(1).MassFlowRate = requestedHotMassFlowRate;

    Real64 curLoad = 0.0;
    waterConnections.simulate(*state, waterConnections.plantLoc, false, curLoad, true);

    EXPECT_NEAR(waterConnections.RecoveryMassFlowRate, requestedHotMassFlowRate, 1.0e-10);
    EXPECT_GT(waterConnections.Effectiveness, 0.0);

    // The fixtures stop drawing water right after a timestep whose heat recovery temperature did not converge
    waterEquipment.PeakVolFlowRate = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;
    waterConnections.TempError = 1.0;
    state->dataGlobal->WarmupFlag = false;

    waterConnections.simulate(*state, waterConnections.plantLoc, false, curLoad, true);

    EXPECT_EQ(waterConnections.TotalMassFlowRate, 0.0);
    EXPECT_EQ(waterConnections.RecoveryMassFlowRate, 0.0);
    EXPECT_EQ(waterConnections.Effectiveness, 0.0);
    EXPECT_EQ(waterConnections.RecoveryRate, 0.0);
    EXPECT_NEAR(waterConnections.ReturnTemp, lockedLoopColdTemp, 1.0e-10);
    // With no flow there is nothing to iterate on, so the stale error must not trigger a non-convergence warning
    EXPECT_EQ(waterConnections.TempError, 0.0);
    EXPECT_EQ(waterConnections.MaxIterationsErrorIndex, 0);
}

// The recovered heat raises only the stream that passes through the heat exchanger (hot makeup for Plant, cold supply for Equipment, both
// for PlantAndEquipment) and lowers only the drain water, so each outlet temperature must carry the full recovery rate on its own flow
TEST_F(EnergyPlusFixture, WaterUse_ConnectionsHeatRecoveryEnergyBalance)
{
    state->init_state(*state);
    state->dataWaterUse->WaterConnections.allocate(1);
    auto &waterConnections = state->dataWaterUse->WaterConnections(1);
    waterConnections.HeatRecovery = true;
    waterConnections.HeatRecoveryHX = HeatRecovHX::CounterFlow;
    waterConnections.HXUA = 250.0;
    waterConnections.ColdSupplyTemp = 10.0;
    waterConnections.DrainTemp = 32.0;
    waterConnections.TotalMassFlowRate = 0.05;
    waterConnections.HotMassFlowRate = 0.02;
    waterConnections.ColdMassFlowRate = 0.03;
    waterConnections.DrainMassFlowRate = 0.048; // some water evaporated to the zone

    Real64 const cp = Psychrometrics::CPHW(Constant::InitConvTemp);

    for (auto const &[config, recoveryMassFlowRate] :
         {std::pair{HeatRecovConfig::Plant, 0.02}, std::pair{HeatRecovConfig::Equipment, 0.03}, std::pair{HeatRecovConfig::PlantAndEquip, 0.05}}) {
        waterConnections.HeatRecoveryConfig = config;
        waterConnections.ColdTemp = waterConnections.ColdSupplyTemp;

        waterConnections.CalcConnectionsHeatRecovery(*state);

        EXPECT_NEAR(waterConnections.RecoveryMassFlowRate, recoveryMassFlowRate, 1.0e-12);
        EXPECT_GT(waterConnections.RecoveryRate, 0.0);
        EXPECT_NEAR(
            recoveryMassFlowRate * cp * (waterConnections.RecoveryTemp - waterConnections.ColdSupplyTemp), waterConnections.RecoveryRate, 1.0e-8);
        EXPECT_NEAR(waterConnections.DrainMassFlowRate * cp * (waterConnections.DrainTemp - waterConnections.WasteTemp),
                    waterConnections.RecoveryRate,
                    1.0e-8);
        EXPECT_LE(waterConnections.RecoveryTemp, waterConnections.DrainTemp);
        if (config == HeatRecovConfig::Equipment) {
            EXPECT_NEAR(waterConnections.ReturnTemp, waterConnections.ColdSupplyTemp, 1.0e-12);
        } else {
            EXPECT_NEAR(waterConnections.ReturnTemp, waterConnections.RecoveryTemp, 1.0e-12);
        }
    }
}
