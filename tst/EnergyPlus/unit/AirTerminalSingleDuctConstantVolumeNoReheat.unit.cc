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

// EnergyPlus::AirTerminal SingleDuct Unit Tests
// AirTerminal:SingleDuct:ConstantVolume:NoReheat

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

// EnergyPlus Headers
using namespace EnergyPlus::DataDefineEquip;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SingleDuct;
using namespace EnergyPlus::ZoneAirLoopEquipmentManager;

using namespace EnergyPlus::EMSManager;
using namespace EnergyPlus::DataRuntimeLanguage;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_GetInput)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SDCVNoReheatAT1,         !- Name",
        "    AvailSchedule,           !- Availability Schedule Name",
        "    Zone1NoReheatAirInletNode,   !- Air Inlet Node Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Outlet Node Name",
        "    0.50;                    !- Maximum Air Flow Rate {m3/s}",

        "  Schedule:Compact,",
        "    AvailSchedule,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SDCVNoReheatADU1,        !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SDCVNoReheatADU1,        !- Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SDCVNoReheatAT1;         !- Air Terminal Name",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1NoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    EXPECT_EQ("AirTerminal:SingleDuct:ConstantVolume:NoReheat",
              state->dataSingleDuct->sd_airterminal(1).sysType);                    // AT SD constant volume no reheat object type
    EXPECT_EQ("SDCVNOREHEATAT1", state->dataSingleDuct->sd_airterminal(1).SysName); // AT SD constant volume no reheat name
    EXPECT_EQ("AVAILSCHEDULE",
              state->dataSingleDuct->sd_airterminal(1).availSched->Name);        // AT SD constant volume no reheat availability schedule name
    EXPECT_EQ(0.50, state->dataSingleDuct->sd_airterminal(1).MaxAirVolFlowRate); // maximum volume flow Rate
    ASSERT_TRUE(state->dataSingleDuct->sd_airterminal(1).NoOAFlowInputFromUser); // no OA flow input from user
    EXPECT_EQ(DataZoneEquipment::PerPersonVentRateMode::DCVByCurrentLevel,
              state->dataSingleDuct->sd_airterminal(1).OAPerPersonMode); // default value when A6 input field is blank
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_SimConstVolNoReheat)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SDCVNoReheatAT1,         !- Name",
        "    AvailSchedule,           !- Availability Schedule Name",
        "    Zone1NoReheatAirInletNode,   !- Air Inlet Node Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Outlet Node Name",
        "    1.0;                    !- Maximum Air Flow Rate {m3/s}",

        "  Schedule:Compact,",
        "    AvailSchedule,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SDCVNoReheatADU1,        !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SDCVNoReheatADU1,        !- Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SDCVNoReheatAT1;         !- Air Terminal Name",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1NoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);
    state->dataEnvrn->StdRhoAir = 1.0;
    int constexpr SysNum(1);
    Real64 MassFlowRateMaxAvail = state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate = MassFlowRateMaxAvail;
    state->dataSingleDuct->sd_airterminal(SysNum).availSched->currentVal = 1.0; // unit is always available
    // run SimConstVolNoReheat() function
    state->dataSingleDuct->sd_airterminal(SysNum).SimConstVolNoReheat(*state);
    // check the TA outlet air mass flow rate
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_Sim)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SDCVNoReheatAT1,         !- Name",
        "    AvailSchedule,           !- Availability Schedule Name",
        "    Zone1NoReheatAirInletNode,   !- Air Inlet Node Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Outlet Node Name",
        "    1.0;                    !- Maximum Air Flow Rate {m3/s}",

        "  Schedule:Compact,",
        "    AvailSchedule,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SDCVNoReheatADU1,        !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SDCVNoReheatADU1,        !- Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SDCVNoReheatAT1;         !- Air Terminal Name",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1NoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    int constexpr SysNum(1);
    int const InletNode = state->dataSingleDuct->sd_airterminal(SysNum).InletNodeNum;
    int const ZonePtr = state->dataSingleDuct->sd_airterminal(SysNum).CtrlZoneNum;
    int const ZoneAirNodeNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).ZoneNode;
    state->dataSingleDuct->sd_airterminal(SysNum).availSched->currentVal = 1.0; // unit is always available

    // design maximum air mass flow rate
    Real64 MassFlowRateMaxAvail = state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    EXPECT_EQ(1.0, state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate);
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);

    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 50.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    ;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.0075;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);

    // set inlet mass flow rate to zero
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
    FirstHVACIteration = true;
    state->dataSingleDuct->GetInputFlag = false;
    // run SimulateSingleDuct(state, ) function
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).AirMassFlowRateMax);         // design maximum mass flow rate
    EXPECT_EQ(0.0, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRateMaxAvail); // maximum available mass flow rate
    EXPECT_EQ(0.0, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);         // outlet mass flow rate is zero
    EXPECT_EQ(0.0, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);        // outlet mass flow rate is zero

    FirstHVACIteration = false;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);
    // run SimulateSingleDuct(*state, ) function
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);
    // outlet and inlet nodes air conditions must match exactly
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirTemp,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirTemp);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirHumRat,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirHumRat);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirEnthalpy,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirEnthalpy);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_OASpecification)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SDCVNoReheatAT1,         !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Zone1NoReheatAirInletNode,   !- Air Inlet Node Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Outlet Node Name",
        "    3.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Zone 1 Ventilation,      !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "DesignSpecification:OutdoorAir,",
        "    Zone 1 Ventilation,      !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.1000,                  !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0000,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.5,                     !- Outdoor Air Flow per Zone {m3/s}",
        "    0,                       !- Outdoor Air Flow Air Changes per Hour {1/hr}",
        "    VentSchedule;            !- Outdoor Air Schedule Name",

        "  Schedule:Compact,",
        "    VentSchedule,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 18:00,1.0,        !- Field 3",
        "    Until: 24:00,0.0;        !- Field 4",

        "  People,",
        "    West Zone People,        !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    OFFICE OCCUPANCY,        !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    3.000000,                !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3000000,               !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    Activity Sch;            !- Activity Level Schedule Name",

        "  Schedule:Compact,",
        "    OFFICE OCCUPANCY,        !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays,           !- Field 2",
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
        "    Activity Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,131.8;      !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SDCVNoReheatADU1,        !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SDCVNoReheatADU1,        !- Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SDCVNoReheatAT1;         !- Air Terminal Name",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1NoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    SizingManager::GetOARequirements(*state);
    InternalHeatGains::GetInternalHeatGainsInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    int constexpr SysNum(1);
    int const InletNode = state->dataSingleDuct->sd_airterminal(SysNum).InletNodeNum;
    int const ZonePtr = state->dataSingleDuct->sd_airterminal(SysNum).CtrlZoneNum;
    int const ZoneAirNodeNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).ZoneNode;

    // design maximum air mass flow rate
    Real64 MassFlowRateMaxAvail = state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    EXPECT_EQ(3.0, state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate);
    EXPECT_EQ(3.0, MassFlowRateMaxAvail);

    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 50.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.0075;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);

    // set inlet mass flow rate to zero
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
    FirstHVACIteration = true;
    state->dataSingleDuct->GetInputFlag = false;
    // run SimulateSingleDuct(state, ) function
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).AirMassFlowRateMax);         // design maximum mass flow rate
    EXPECT_EQ(0.0, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRateMaxAvail); // maximum available mass flow rate
    EXPECT_EQ(0.0, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);         // outlet mass flow rate is zero
    EXPECT_EQ(0.0, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);        // outlet mass flow rate is zero

    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    // Needs an airloop, assume 100% OA
    state->dataSingleDuct->sd_airterminal(SysNum).AirLoopNum = 1;
    state->dataAirLoop->AirLoopFlow.allocate(1);
    state->dataAirLoop->AirLoopFlow(state->dataSingleDuct->sd_airterminal(SysNum).AirLoopNum).OAFrac = 1.0;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    EXPECT_EQ(3.0, MassFlowRateMaxAvail);

    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfYear_Schedule = 1;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;

    // Full occupancy 3 people, OA/person = 0.1, OA/zone = 0.5, OA Sched = 1.0
    state->dataGlobal->HourOfDay = 12;
    Sched::UpdateScheduleVals(*state);
    // Just set number of people directly, too many other things that have to be in place to call ManagerInternalHeatGains()
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 3.0;
    Real64 expectedMassFlow = 1.0 * ((3.0 * 0.1) + 0.5);

    // run SimulateSingleDuct() function
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(expectedMassFlow, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(expectedMassFlow, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);

    // 50% occupancy 1.5 people, OA/person = 0.1, OA/zone = 0.5, OA Sched = 1.0
    state->dataGlobal->HourOfDay = 12;
    Sched::UpdateScheduleVals(*state);
    // Just set number of people directly, too many other things that have to be in place to call ManagerInternalHeatGains()
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 1.5;
    expectedMassFlow = 1.0 * ((1.5 * 0.1) + 0.5);
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(expectedMassFlow, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(expectedMassFlow, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);

    // Nighttime OA Sched = 0.0
    state->dataGlobal->HourOfDay = 24;
    Sched::UpdateScheduleVals(*state);
    // Just set number of people directly, too many other things that have to be in place to call ManagerInternalHeatGains()
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 1.5;
    expectedMassFlow = 0.0 * ((1.5 * 0.1) + 0.5);
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(expectedMassFlow, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(expectedMassFlow, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_EMSOverrideAirFlow)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SDCVNoReheatAT1,         !- Name",
        "    AvailSchedule,           !- Availability Schedule Name",
        "    Zone1NoReheatAirInletNode,   !- Air Inlet Node Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Outlet Node Name",
        "    1.0;                    !- Maximum Air Flow Rate {m3/s}",

        "  Schedule:Compact,",
        "    AvailSchedule,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SDCVNoReheatADU1,        !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SDCVNoReheatADU1,        !- Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SDCVNoReheatAT1;         !- Air Terminal Name",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Zone1Inlets,             !- Name",
        "    Zone1NoReheatAirOutletNode;   !- Node 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    int constexpr SysNum(1);
    int const InletNode = state->dataSingleDuct->sd_airterminal(SysNum).InletNodeNum;
    int const ZonePtr = state->dataSingleDuct->sd_airterminal(SysNum).CtrlZoneNum;
    int const ZoneAirNodeNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).ZoneNode;
    state->dataSingleDuct->sd_airterminal(SysNum).availSched->currentVal = 1.0; // unit is always available
    // design maximum air mass flow rate
    Real64 MassFlowRateMaxAvail = state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    EXPECT_EQ(1.0, state->dataSingleDuct->sd_airterminal(SysNum).MaxAirVolFlowRate);
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);

    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 50.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    ;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.0075;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);
    ;
    state->dataSingleDuct->GetInputFlag = false;
    FirstHVACIteration = false;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);
    // run SimulateSingleDuct(*state, ) function
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(MassFlowRateMaxAvail, state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);
    // outlet and inlet nodes air conditions must match exactly
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirTemp,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirTemp);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirHumRat,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirHumRat);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirEnthalpy,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirEnthalpy);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    // sets EMS actuators
    state->dataSingleDuct->sd_airterminal(SysNum).EMSOverrideAirFlow = true;
    state->dataSingleDuct->sd_airterminal(SysNum).EMSMassFlowRateValue = 0.5;
    // run SimulateSingleDuct(state, ) function
    SimulateSingleDuct(*state,
                       state->dataDefineEquipment->AirDistUnit(1).EquipName(1),
                       FirstHVACIteration,
                       ZonePtr,
                       ZoneAirNodeNum,
                       state->dataDefineEquipment->AirDistUnit(1).EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).EMSMassFlowRateValue,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(state->dataSingleDuct->sd_airterminal(SysNum).EMSMassFlowRateValue,
              state->dataSingleDuct->sd_airterminal(SysNum).sd_airterminalOutlet.AirMassFlowRate);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_OAVolumeFlowRateReporting)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SDCVNoReheatATU,         !- Name",
        "    ,                        !- Availability Schedule Name",
        "    WZoneNoReheatAirInletNode,   !- Air Inlet Node Name",
        "    WZoneNoReheatAirOutletNode,  !- Air Outlet Node Name",
        "    3.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Ventilation Spec,        !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "DesignSpecification:OutdoorAir,",
        "    Ventilation Spec,        !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.1000,                  !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0000,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.5,                     !- Outdoor Air Flow per Zone {m3/s}",
        "    0,                       !- Outdoor Air Flow Air Changes per Hour {1/hr}",
        "    VentSchedule;            !- Outdoor Air Schedule Name",

        "  Schedule:Compact,",
        "    VentSchedule,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 06:00,0.0,        !- Field 3",
        "    Until: 18:00,1.0,        !- Field 4",
        "    Until: 24:00,0.0;        !- Field 6",

        "  People,",
        "    West Zone People,        !- Name",
        "    West Zone,               !- Zone or ZoneList Name",
        "    Office Occupancy,        !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    3.000000,                !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3000000,               !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    Activity Sch;            !- Activity Level Schedule Name",

        "  Schedule:Compact,",
        "    Office Occupancy,        !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays,           !- Field 2",
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
        "    Activity Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,131.8;      !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    WestZoneEquipment,       !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SDCVNoReheatADU,         !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SDCVNoReheatADU,         !- Name",
        "    WZoneNoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SDCVNoReheatATU;         !- Air Terminal Name",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    WestZoneEquipment,       !- Zone Conditioning Equipment List Name",
        "    West Zone Inlet Nodes,   !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    West Zone Air Node,      !- Zone Air Node Name",
        "    West Zone Outlet Node;   !- Zone Return Air Node Name",

        "  NodeList,",
        "    West Zone Inlet Nodes,   !- Name",
        "    WZoneNoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    SizingManager::GetOARequirements(*state);
    InternalHeatGains::GetInternalHeatGainsInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataAirLoop->AirLoopFlow.allocate(1);
    int constexpr SysNum(1);
    auto &thisAirTerminal = state->dataSingleDuct->sd_airterminal(SysNum);
    auto &thisAirTerminalInlet = thisAirTerminal.sd_airterminalInlet;
    auto &thisAirTerminalOutlet = thisAirTerminal.sd_airterminalOutlet;
    auto &thisAirDisUnit = state->dataDefineEquipment->AirDistUnit(1);
    auto &thisAirLoop = state->dataAirLoop->AirLoopFlow(1);

    int const InletNode = thisAirTerminal.InletNodeNum;
    int const ZonePtr = thisAirTerminal.CtrlZoneNum;
    int const ZoneAirNodeNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).ZoneNode;
    state->dataZoneEquip->ZoneEquipConfig(ZonePtr).InletNodeAirLoopNum(1) = 1;
    thisAirTerminal.AirLoopNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).InletNodeAirLoopNum(1);

    // design maximum air mass flow rate
    Real64 MassFlowRateMaxAvail = thisAirTerminal.MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    EXPECT_EQ(3.0, thisAirTerminal.MaxAirVolFlowRate);
    EXPECT_EQ(3.0, MassFlowRateMaxAvail);

    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 50.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.0075;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);

    // set inlet mass flow rate to zero
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
    FirstHVACIteration = true;
    state->dataSingleDuct->GetInputFlag = false;
    // run SimulateSingleDuct(state, ) function
    SimulateSingleDuct(*state, thisAirDisUnit.EquipName(1), FirstHVACIteration, ZonePtr, ZoneAirNodeNum, thisAirDisUnit.EquipIndex(1));
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.AirMassFlowRateMax); // design maximum mass flow rate
    EXPECT_EQ(0.0, thisAirTerminalInlet.AirMassFlowRateMaxAvail);        // maximum available mass flow rate
    EXPECT_EQ(0.0, thisAirTerminalOutlet.AirMassFlowRate);               // outlet mass flow rate is zero
    EXPECT_EQ(0.0, thisAirTerminalOutlet.AirMassFlowRate);               // outlet mass flow rate is zero
    EXPECT_EQ(0.0, thisAirTerminal.OutdoorAirFlowRate);                  // OA volume flow rate is zero

    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    // Needs an airloop, assume 100% OA
    thisAirLoop.OAFrac = 1.0;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    EXPECT_EQ(3.0, MassFlowRateMaxAvail);

    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfYear_Schedule = 1;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;

    // Full occupancy 3 people, OA/person = 0.1, OA/zone = 0.5, OA Sched = 1.0
    state->dataGlobal->HourOfDay = 12;
    Sched::UpdateScheduleVals(*state);
    // Just set number of people directly, too many other things that have to be in place to call ManagerInternalHeatGains()
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 3.0;
    Real64 expectedMassFlow = 1.0 * ((3.0 * 0.1) + 0.5);

    // run SimulateSingleDuct(*state, ) function
    SimulateSingleDuct(*state, thisAirDisUnit.EquipName(1), FirstHVACIteration, ZonePtr, ZoneAirNodeNum, thisAirDisUnit.EquipIndex(1));
    Real64 expected_OAVolFlowRate = thisAirTerminalOutlet.AirMassFlowRate * thisAirLoop.OAFrac / state->dataEnvrn->StdRhoAir;
    // check AT air mass flow rates
    EXPECT_EQ(expectedMassFlow, thisAirTerminalInlet.AirMassFlowRate);
    EXPECT_EQ(expectedMassFlow, thisAirTerminalOutlet.AirMassFlowRate);
    EXPECT_EQ(expected_OAVolFlowRate, thisAirTerminal.OutdoorAirFlowRate); // OA volume flow rate

    // 50% occupancy 1.5 people, OA/person = 0.1, OA/zone = 0.5, OA Sched = 1.0
    state->dataGlobal->HourOfDay = 12;
    Sched::UpdateScheduleVals(*state);
    // Just set number of people directly, too many other things that have to be in place to call ManagerInternalHeatGains()
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 1.5;
    expectedMassFlow = 1.0 * ((1.5 * 0.1) + 0.5);
    SimulateSingleDuct(*state, thisAirDisUnit.EquipName(1), FirstHVACIteration, ZonePtr, ZoneAirNodeNum, thisAirDisUnit.EquipIndex(1));
    expected_OAVolFlowRate = thisAirTerminalOutlet.AirMassFlowRate * thisAirLoop.OAFrac / state->dataEnvrn->StdRhoAir;
    // check AT air mass flow rates
    EXPECT_EQ(expectedMassFlow, thisAirTerminalInlet.AirMassFlowRate);
    EXPECT_EQ(expectedMassFlow, thisAirTerminalOutlet.AirMassFlowRate);
    EXPECT_EQ(expected_OAVolFlowRate, thisAirTerminal.OutdoorAirFlowRate); // OA volume flow rate

    // Nighttime OA Sched = 0.0
    state->dataGlobal->HourOfDay = 24;
    Sched::UpdateScheduleVals(*state);
    // Just set number of people directly, too many other things that have to be in place to call ManagerInternalHeatGains()
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 1.5;
    expectedMassFlow = 0.0 * ((1.5 * 0.1) + 0.5);
    SimulateSingleDuct(*state, thisAirDisUnit.EquipName(1), FirstHVACIteration, ZonePtr, ZoneAirNodeNum, thisAirDisUnit.EquipIndex(1));
    expected_OAVolFlowRate = thisAirTerminalOutlet.AirMassFlowRate * thisAirLoop.OAFrac / state->dataEnvrn->StdRhoAir;
    // check AT air mass flow rates
    EXPECT_EQ(expectedMassFlow, thisAirTerminalInlet.AirMassFlowRate);
    EXPECT_EQ(expectedMassFlow, thisAirTerminalOutlet.AirMassFlowRate);
    EXPECT_EQ(expected_OAVolFlowRate, thisAirTerminal.OutdoorAirFlowRate); // OA volume flow rate is zero
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_SimSensibleOutPutTest)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    CVNoReheatATU,           !- Name",
        "    AvailSchedule,           !- Availability Schedule Name",
        "    NoReheatAirInletNode,    !- Air Inlet Node Name",
        "    NoReheatAirOutletNode,   !- Air Outlet Node Name",
        "    1.0;                     !- Maximum Air Flow Rate {m3/s}",

        "  Schedule:Compact,",
        "    AvailSchedule,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    ZoneEquipment,           !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    NoReheatADU,             !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    NoReheatADU,             !- Name",
        "    NoReheatAirOutletNode,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    CVNoReheatATU;           !- Air Terminal Name",

        "  Zone,",
        "    Zone One,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Zone One,                !- Zone Name",
        "    ZoneEquipment,           !- Zone Conditioning Equipment List Name",
        "    ZoneInlets,              !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone Air Node,           !- Zone Air Node Name",
        "    Zone Return Air Node;    !- Zone Return Air Node Name",

        "  NodeList,",
        "    ZoneInlets,              !- Name",
        "    NoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    int constexpr AirDistUnitNum(1);
    int constexpr AirTerminalNum(1);

    auto &thisAirTerminal(state->dataSingleDuct->sd_airterminal(AirTerminalNum));

    int const InletNode = thisAirTerminal.InletNodeNum;
    int const OutletNode = thisAirTerminal.OutletNodeNum;
    int const ZonePtr = thisAirTerminal.CtrlZoneNum;
    int const ZoneAirNodeNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).ZoneNode;

    thisAirTerminal.availSched->currentVal = 1.0; // unit is always available
    ;
    // design maximum air mass flow rate
    Real64 MassFlowRateMaxAvail = thisAirTerminal.MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    EXPECT_EQ(1.0, thisAirTerminal.MaxAirVolFlowRate);
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);

    // test 1: heating mode test
    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 35.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    state->dataLoopNodes->Node(OutletNode).Temp = state->dataLoopNodes->Node(InletNode).Temp;
    state->dataLoopNodes->Node(OutletNode).HumRat = state->dataLoopNodes->Node(InletNode).HumRat;
    state->dataLoopNodes->Node(OutletNode).Enthalpy = state->dataLoopNodes->Node(InletNode).Enthalpy;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.005;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);
    // set inlet mass flow rate to zero
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
    FirstHVACIteration = true;
    state->dataSingleDuct->GetInputFlag = false;
    Real64 SysOutputProvided = 0.0;
    Real64 NonAirSysOutput = 0.0;
    Real64 LatOutputProvided = 0.0;
    // run single duct simulation
    SimZoneAirLoopEquipment(*state, AirDistUnitNum, SysOutputProvided, NonAirSysOutput, LatOutputProvided, FirstHVACIteration, ZonePtr);
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.AirMassFlowRateMax);         // design maximum mass flow rate
    EXPECT_EQ(0.0, thisAirTerminal.sd_airterminalInlet.AirMassFlowRateMaxAvail); // maximum available mass flow rate
    EXPECT_EQ(0.0, thisAirTerminal.sd_airterminalInlet.AirMassFlowRate);         // inlet mass flow rate is zero
    EXPECT_EQ(0.0, thisAirTerminal.sd_airterminalOutlet.AirMassFlowRate);        // outlet mass flow rate is zero
    EXPECT_EQ(0.0, SysOutputProvided);                                           // delivered sensible heating is zero
                                                                                 // reset mass flow rate to the maximum available
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);
    // calculate sensible output provided by the air terminal unit
    Real64 CpAir = PsyCpAirFnW(min(state->dataLoopNodes->Node(OutletNode).HumRat, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat));
    Real64 SensHeatRateProvided =
        MassFlowRateMaxAvail * CpAir * (state->dataLoopNodes->Node(OutletNode).Temp - state->dataLoopNodes->Node(ZoneAirNodeNum).Temp);
    // run SimulateSingleDuct() function
    SimZoneAirLoopEquipment(*state, AirDistUnitNum, SysOutputProvided, NonAirSysOutput, LatOutputProvided, FirstHVACIteration, ZonePtr);
    // check air terminal unit air mass flow rates and delivered sensible heating rate
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.sd_airterminalOutlet.AirMassFlowRate);
    EXPECT_NEAR(SensHeatRateProvided, SysOutputProvided, 0.001);
    // outlet and inlet nodes air conditions must match exactly
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirTemp, thisAirTerminal.sd_airterminalInlet.AirTemp);
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirHumRat, thisAirTerminal.sd_airterminalInlet.AirHumRat);
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirEnthalpy, thisAirTerminal.sd_airterminalInlet.AirEnthalpy);
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirMassFlowRate, thisAirTerminal.sd_airterminalInlet.AirMassFlowRate);
    ;
    // test 2: cooling mode test
    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 15.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0085;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    state->dataLoopNodes->Node(OutletNode).Temp = state->dataLoopNodes->Node(InletNode).Temp;
    state->dataLoopNodes->Node(OutletNode).HumRat = state->dataLoopNodes->Node(InletNode).HumRat;
    state->dataLoopNodes->Node(OutletNode).Enthalpy = state->dataLoopNodes->Node(InletNode).Enthalpy;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 24.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.00975;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);
    // set inlet mass flow rate to zero
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
    FirstHVACIteration = true;
    SysOutputProvided = 0.0;
    NonAirSysOutput = 0.0;
    LatOutputProvided = 0.0;
    // run single duct simulation
    SimZoneAirLoopEquipment(*state, AirDistUnitNum, SysOutputProvided, NonAirSysOutput, LatOutputProvided, FirstHVACIteration, ZonePtr);
    // check AT air mass flow rates
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.AirMassFlowRateMax);         // design maximum mass flow rate
    EXPECT_EQ(0.0, thisAirTerminal.sd_airterminalInlet.AirMassFlowRateMaxAvail); // maximum available mass flow rate
    EXPECT_EQ(0.0, thisAirTerminal.sd_airterminalInlet.AirMassFlowRate);         // inlet mass flow rate is zero
    EXPECT_EQ(0.0, thisAirTerminal.sd_airterminalOutlet.AirMassFlowRate);        // outlet mass flow rate is zero
    EXPECT_EQ(0.0, SysOutputProvided);                                           // delivered sensible cooling is zero
    ;
    // reset mass flow rate to the maximum available
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);
    // calculate sensible output provided by the air terminal unit
    CpAir = PsyCpAirFnW(min(state->dataLoopNodes->Node(OutletNode).HumRat, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat));
    Real64 SensCoolRateProvided =
        MassFlowRateMaxAvail * CpAir * (state->dataLoopNodes->Node(OutletNode).Temp - state->dataLoopNodes->Node(ZoneAirNodeNum).Temp);
    // run SimulateSingleDuct() function
    SimZoneAirLoopEquipment(*state, AirDistUnitNum, SysOutputProvided, NonAirSysOutput, LatOutputProvided, FirstHVACIteration, ZonePtr);
    // check air terminal unit air mass flow rates and delivered sensible cooling rate
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.sd_airterminalInlet.AirMassFlowRate);
    EXPECT_EQ(MassFlowRateMaxAvail, thisAirTerminal.sd_airterminalOutlet.AirMassFlowRate);
    EXPECT_NEAR(SensCoolRateProvided, SysOutputProvided, 0.001);
    // outlet and inlet nodes air conditions must match exactly
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirTemp, thisAirTerminal.sd_airterminalInlet.AirTemp);
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirHumRat, thisAirTerminal.sd_airterminalInlet.AirHumRat);
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirEnthalpy, thisAirTerminal.sd_airterminalInlet.AirEnthalpy);
    EXPECT_EQ(thisAirTerminal.sd_airterminalOutlet.AirMassFlowRate, thisAirTerminal.sd_airterminalInlet.AirMassFlowRate);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctCVNoReheat_DownstreamLeakTest)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    CVNoReheatATU,           !- Name",
        "    AvailSchedule,           !- Availability Schedule Name",
        "    NoReheatAirInletNode,    !- Air Inlet Node Name",
        "    NoReheatAirOutletNode,   !- Air Outlet Node Name",
        "    1.0;                     !- Maximum Air Flow Rate {m3/s}",

        "  Schedule:Compact,",
        "    AvailSchedule,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  ZoneHVAC:EquipmentList,",
        "    ZoneEquipment,           !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    NoReheatADU,             !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    NoReheatADU,             !- Name",
        "    NoReheatAirOutletNode,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    CVNoReheatATU,           !- Air Terminal Name",
        "    0.0,                     !- Nominal Upstream Leakage Fraction",
        "    0.29;                    !- Constant Downstream Leakage Fraction",

        "  Zone,",
        "    Zone One,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.40,                    !- Ceiling Height {m}",
        "    240.0;                   !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Zone One,                !- Zone Name",
        "    ZoneEquipment,           !- Zone Conditioning Equipment List Name",
        "    ZoneInlets,              !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone Air Node,           !- Zone Air Node Name",
        "    Zone Return Air Node;    !- Zone Return Air Node Name",

        "  NodeList,",
        "    ZoneInlets,              !- Name",
        "    NoReheatAirOutletNode;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetSysInput(*state);

    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    int constexpr AirDistUnitNum(1);
    int constexpr AirTerminalNum(1);

    auto &thisAirTerminal(state->dataSingleDuct->sd_airterminal(AirTerminalNum));

    int const InletNode = thisAirTerminal.InletNodeNum;
    int const OutletNode = thisAirTerminal.OutletNodeNum;
    int const ZonePtr = thisAirTerminal.CtrlZoneNum;
    int const ZoneAirNodeNum = state->dataZoneEquip->ZoneEquipConfig(ZonePtr).ZoneNode;

    thisAirTerminal.availSched->currentVal = 1.0; // unit is always available
    ;
    // design maximum air mass flow rate
    Real64 MassFlowRateMaxAvail = thisAirTerminal.MaxAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    EXPECT_EQ(1.0, thisAirTerminal.MaxAirVolFlowRate);
    EXPECT_EQ(1.0, MassFlowRateMaxAvail);

    // heating mode test
    // set air inlet node properties
    state->dataLoopNodes->Node(InletNode).Temp = 35.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);
    state->dataLoopNodes->Node(OutletNode).Temp = state->dataLoopNodes->Node(InletNode).Temp;
    state->dataLoopNodes->Node(OutletNode).HumRat = state->dataLoopNodes->Node(InletNode).HumRat;
    state->dataLoopNodes->Node(OutletNode).Enthalpy = state->dataLoopNodes->Node(InletNode).Enthalpy;
    // set zone air node properties
    state->dataLoopNodes->Node(ZoneAirNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat = 0.005;
    state->dataLoopNodes->Node(ZoneAirNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneAirNodeNum).Temp, state->dataLoopNodes->Node(ZoneAirNodeNum).HumRat);
    // set inlet mass flow rate to zero
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
    FirstHVACIteration = true;
    state->dataSingleDuct->GetInputFlag = false;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
    Real64 SysOutputProvided = 0.0;
    Real64 NonAirSysOutput = 0.0;
    Real64 LatOutputProvided = 0.0;
    SimZoneAirLoopEquipment(*state, AirDistUnitNum, SysOutputProvided, NonAirSysOutput, LatOutputProvided, FirstHVACIteration, ZonePtr);
    EXPECT_EQ(MassFlowRateMaxAvail * (1 - 0.29), state->dataLoopNodes->Node(thisAirTerminal.OutletNodeNum).MassFlowRate);
}

} // namespace EnergyPlus
