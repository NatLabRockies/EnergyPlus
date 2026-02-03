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

// EnergyPlus::Standalone ERV Unit Tests

#include <fstream>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DualDuct.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

using namespace EnergyPlus;
using namespace DualDuct;

TEST_F(EnergyPlusFixture, TestDualDuctOAMassFlowRateUsingStdRhoAir)
{

    // AUTHOR: L. Gu, FSEC
    // DATE WRITTEN: Jul. 2016
    // TEST: #5769

    Real64 SAMassFlow;
    Real64 AirLoopOAFrac;
    Real64 OAMassFlow;

    int numOfdd_airterminals = 2;

    state->init_state(*state);

    state->dataHeatBal->Zone.allocate(1);
    state->dataSize->OARequirements.allocate(1);
    state->dataAirLoop->AirLoopControlInfo.allocate(1);
    state->dataHeatBal->ZoneIntGain.allocate(1);

    state->dataHeatBal->Zone(1).FloorArea = 10.0;

    state->dataDualDuct->dd_airterminal.allocate(numOfdd_airterminals);
    state->dataDualDuct->dd_airterminal(1).CtrlZoneNum = 1;
    state->dataDualDuct->dd_airterminal(1).OARequirementsPtr = 1;
    state->dataDualDuct->dd_airterminal(1).NoOAFlowInputFromUser = false;
    state->dataDualDuct->dd_airterminal(1).AirLoopNum = 1;
    state->dataDualDuct->dd_airterminal(2).CtrlZoneNum = 1;
    state->dataDualDuct->dd_airterminal(2).NoOAFlowInputFromUser = false;
    state->dataDualDuct->dd_airterminal(2).OARequirementsPtr = 1;
    state->dataDualDuct->dd_airterminal(2).AirLoopNum = 1;

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1) = 1;

    state->dataAirLoop->AirLoopFlow.allocate(1);
    state->dataAirLoop->AirLoopFlow(1).OAFrac = 0.5;
    state->dataAirLoop->AirLoopControlInfo(1).AirLoopDCVFlag = true;

    state->dataSize->OARequirements(1).Name = "CM DSOA WEST ZONE";
    state->dataSize->OARequirements(1).OAFlowMethod = DataSizing::OAFlowCalcMethod::Sum;
    state->dataSize->OARequirements(1).OAFlowPerPerson = 0.003149;
    state->dataSize->OARequirements(1).OAFlowPerArea = 0.000407;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 0.1;

    state->dataDualDuct->dd_airterminal(1).CalcOAMassFlow(*state, SAMassFlow, AirLoopOAFrac);
    EXPECT_NEAR(0.01052376, SAMassFlow, 0.00001);
    EXPECT_NEAR(0.5, AirLoopOAFrac, 0.00001);

    state->dataDualDuct->dd_airterminal(2).CalcOAOnlyMassFlow(*state, OAMassFlow);
    EXPECT_NEAR(0.004884, OAMassFlow, 0.00001);

    // Cleanup
    state->dataHeatBal->Zone.deallocate();
    state->dataSize->OARequirements.deallocate();
    state->dataAirLoop->AirLoopControlInfo.deallocate();
    state->dataHeatBal->ZoneIntGain.deallocate();

    state->dataDualDuct->dd_airterminal.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataAirLoop->AirLoopFlow.deallocate();
}

// TEST_F( EnergyPlusFixture, AirTerminalDualDuct_GetInputTest ) {
//
//		bool ErrorsFound( false );
//
//		std::string const idf_objects = delimited_string( {
//			// Dual Duct CV
//			"  ZoneHVAC:AirDistributionUnit,",
//			"    ADU DualDuctCV,                   !- Name",
//			"    Zone DualDuctCV Inlet Node,       !- Air Distribution Unit Outlet Node Name",
//			"    AirTerminal:DualDuct:ConstantVolume,  !- Air Terminal Object Type",
//			"    Air Terminal Dual Duct Constant Volume 1;  !- Air Terminal Name",
//
//			"  AirTerminal:DualDuct:ConstantVolume,",
//			"    ATU DualDuctCV,                   !- Name",
//			"    ,                                 !- Availability Schedule Name",
//			"    Zone DualDuctCV Inlet Node,       !- Air Outlet Node Name",
//			"    DualDuctCV Hot Air Inlet Node,    !- Hot Air Inlet Node Name",
//			"    DualDuctCV Cold Air Inlet Node,   !- Cold Air Inlet Node Name",
//			"    Autosize;                         !- Maximum Air Flow Rate {m3/s}",
//
//			"  ZoneHVAC:EquipmentList,",
//			"    Zone DualDuctCV Equipment,       !- Name",
//			"    SequentialLoad,                  !- Load Distribution Scheme",
//			"    ZoneHVAC:AirDistributionUnit,    !- Zone Equipment 1 Object Type",
//			"    ADU DualDuctVAV,                 !- Zone Equipment 1 Name",
//			"    1,                               !- Zone Equipment 1 Cooling Sequence",
//			"    1;                               !- Zone Equipment 1 Heating or No-Load Sequence",
//
//			"  Zone,",
//			"    Zone DualDuctCV,         !- Name",
//			"    0,                       !- Direction of Relative North {deg}",
//			"    0,                       !- X Origin {m}",
//			"    0,                       !- Y Origin {m}",
//			"    0,                       !- Z Origin {m}",
//			"    1,                       !- Type",
//			"    1,                       !- Multiplier",
//			"    2,                       !- Ceiling Height {m}",
//			"    400;                     !- Volume {m3}",
//
//			"  ZoneHVAC:EquipmentConnections,",
//			"    Zone DualDuctCV,                 !- Zone Name",
//			"    Zone DualDuctCV Equipment,       !- Zone Conditioning Equipment List Name",
//			"    Zone DualDuctCV Inlet Node,      !- Zone Air Inlet Node or NodeList Name",
//			"    ,    !- Zone Air Exhaust Node or NodeList Name", // Zone DualDuctCV Exhaust Node
//			"    Zone DualDuctCV Node,            !- Zone Air Node Name",
//			"    Zone DualDuctCV Outlet Node;     !- Zone Return Air Node Name",
//
//
//			// Dual Duct VAV
//			"  ZoneHVAC:AirDistributionUnit,",
//			"    ADU DualDuctVAV,                 !- Name",
//			"    Zone DualDuctVAV Inlet Node,     !- Air Distribution Unit Outlet Node Name",
//			"    AirTerminal:DualDuct:VAV,        !- Air Terminal Object Type",
//			"    Air Terminal Dual Duct VAV 1;    !- Air Terminal Name",
//
//			"  AirTerminal:DualDuct:VAV,",
//			"    ATU DualDuctVAV,                 !- Name",
//			"    ,                                !- Availability Schedule Name",
//			"    Zone DualDuctVAV Inlet Node,     !- Air Outlet Node Name",
//			"    DualDuctVAV Hot Air Inlet Node,  !- Hot Air Inlet Node Name",
//			"    DualDuctVAV Cold Air Inlet Node, !- Cold Air Inlet Node Name",
//			"    Autosize,                        !- Maximum dd_airterminal Air Flow Rate {m3/s}",
//			"    0.3;                             !- Zone Minimum Air Flow Fraction",
//
//			"  ZoneHVAC:EquipmentList,",
//			"    Zone DualDuctVAV Equipment,      !- Name",
//			"    SequentialLoad,                  !- Load Distribution Scheme",
//			"    ZoneHVAC:AirDistributionUnit,    !- Zone Equipment 1 Object Type",
//			"    ADU DualDuctVAV,                 !- Zone Equipment 1 Name",
//			"    1,                               !- Zone Equipment 1 Cooling Sequence",
//			"    1;                               !- Zone Equipment 1 Heating or No-Load Sequence",
//
//			"  Zone,",
//			"    Zone DualDuctVAV,        !- Name",
//			"    0,                       !- Direction of Relative North {deg}",
//			"    10,                      !- X Origin {m}",
//			"    0,                       !- Y Origin {m}",
//			"    0,                       !- Z Origin {m}",
//			"    1,                       !- Type",
//			"    1,                       !- Multiplier",
//			"    2,                       !- Ceiling Height {m}",
//			"    400;                     !- Volume {m3}",
//
//			"  ZoneHVAC:EquipmentConnections,",
//			"    Zone DualDuctVAV,                  !- Zone Name",
//			"    Zone DualDuctVAV Equipment,        !- Zone Conditioning Equipment List Name",
//			"    Zone DualDuctVAV Inlet Node,       !- Zone Air Inlet Node or NodeList Name",
//			"    ,     !- Zone Air Exhaust Node or NodeList Name", // Zone DualDuctVAV Exhaust Node
//			"    Zone DualDuctVAV Node,             !- Zone Air Node Name",
//			"    Zone DualDuctVAV Outlet Node;      !- Zone Return Air Node Name",
//
//
//			// Dual Duct VAV Outdoor Air
//			"  ZoneHVAC:AirDistributionUnit,",
//			"    ADU DualDuctVAVOA,               !- Name",
//			"    Zone DualDuctVAVOA Inlet Node,   !- Air Distribution Unit Outlet Node Name",
//			"    AirTerminal:DualDuct:VAV:OutdoorAir,  !- Air Terminal Object Type",
//			"    Air Terminal Dual Duct VAV Outdoor Air 1;  !- Air Terminal Name",
//
//			"  AirTerminal:DualDuct:VAV:OutdoorAir,",
//			"    ATU DualDuctVAVOA,               !- Name",
//			"    ,                                !- Availability Schedule Name",
//			"    Zone DualDuctVAVOA Inlet Node,   !- Air Outlet Node Name",
//			"    DualDuctVAVOA OA Inlet Node,     !- Outdoor Air Inlet Node Name",
//			"    DualDuctVAVOA Recirc Inlet Node, !- Recirculated Air Inlet Node Name",
//			"    Autosize,                !- Maximum Terminal Air Flow Rate {m3/s}",
//			"    Baseline Model OA,       !- Design Specification Outdoor Air Object Name",
//			"    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",
//
//			"  DesignSpecification:OutdoorAir,",
//			"    Baseline Model OA,       !- Name",
//			"    Sum,                     !- Outdoor Air Method",
//			"    0.009438948864,          !- Outdoor Air Flow per Person {m3/s-person}",
//			"    0,                       !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
//			"    0,                       !- Outdoor Air Flow per Zone {m3/s}",
//			"    0;                       !- Outdoor Air Flow Air Changes per Hour {1/hr}",
//
//			"  ZoneHVAC:EquipmentList,",
//			"    Zone DualDuctVAVOA Equipment,    !- Name",
//			"    SequentialLoad,                  !- Load Distribution Scheme",
//			"    ZoneHVAC:AirDistributionUnit,    !- Zone Equipment 1 Object Type",
//			"    ADU DualDuctVAVOA,               !- Zone Equipment 1 Name",
//			"    1,                               !- Zone Equipment 1 Cooling Sequence",
//			"    1;                               !- Zone Equipment 1 Heating or No-Load Sequence",
//
//			"  Zone,",
//			"    Zone DualDuctVAVOA,      !- Name",
//			"    0,                       !- Direction of Relative North {deg}",
//			"    10,                      !- X Origin {m}",
//			"    10,                      !- Y Origin {m}",
//			"    0,                       !- Z Origin {m}",
//			"    1,                       !- Type",
//			"    1,                       !- Multiplier",
//			"    2,                       !- Ceiling Height {m}",
//			"    400;                     !- Volume {m3}",
//
//			"  ZoneHVAC:EquipmentConnections,",
//			"    Zone DualDuctVAVOA,                 !- Zone Name",
//			"    Zone DualDuctVAVOA Equipment,       !- Zone Conditioning Equipment List Name",
//			"    Zone DualDuctVAVOA Inlet Node,      !- Zone Air Inlet Node or NodeList Name",
//			//"    Zone DualDuctVAVOA Exhaust Node,    !- Zone Air Exhaust Node or NodeList Name",
//
//			"    ,                                   !- Zone Air Exhaust Node or NodeList Name",
//			"    Zone DualDuctVAVOA Node,            !- Zone Air Node Name",
//			"    Zone DualDuctVAVOA Outlet Node;     !- Zone Return Air Node Name",
//	} );
//
//		ASSERT_FALSE( process_idf( idf_objects ) );
//
//		DataGlobals::TimeStepsInHour = 1; // must initialize this to get schedules initialized
//		DataGlobals::MinutesInTimeStep = 60; // must initialize this to get schedules initialized
//		state->init_state(*state);
//
//		HeatBalanceManager::GetZoneData(*state,  ErrorsFound );
//		ASSERT_FALSE( ErrorsFound );
//
//		DataZoneEquipment::GetZoneEquipmentData1();
//		ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment();
//		DualDuct::GetDualDuctInput(*state);
//
//		EXPECT_EQ(3u, dd_airterminal.size());
//		EXPECT_ENUM_EQ( DualDuct::DualDuct_ConstantVolume, dd_airterminal( 1 ).dd_airterminalType );
//		EXPECT_ENUM_EQ( DualDuct::DualDuct_VariableVolume, dd_airterminal( 2 ).dd_airterminalType ); // AT SD VAV HeatCool Reheat
// Type 		EXPECT_ENUM_EQ( DualDuct::DualDuct_OutdoorAir, dd_airterminal( 3 ).dd_airterminalType ); // AT SD VAV HeatCool
// Reheat Type
//
//
//		for (size_t i = 1; i <= DualDuct::dd_airterminal.size(); ++i) {
//			EXPECT_GT(0, DualDuct::dd_airterminal(i).ADUNum);
//		}
//}

TEST_F(EnergyPlusFixture, DualDuctVAVAirTerminals_GetInputs)
{
    std::string const idf_objects = delimited_string({

        "   ZoneHVAC:AirDistributionUnit,",
        "     ADU Dual Duct AT,        !- Name",
        "     DualDuct Outlet,         !- Air Distribution Unit Outlet Node Name",
        "     AirTerminal:DualDuct:VAV,!- Air Terminal Object Type",
        "     VAV Dual Duct AT;        !- Air Terminal Name",

        "   AirTerminal:DualDuct:VAV,",
        "     VAV Dual Duct AT,        !- Name",
        "     ,                        !- Availability Schedule Name",
        "     DualDuct Outlet,         !- Air Outlet Node Name",
        "     DualDuct Hot Inlet,      !- Hot Air Inlet Node Name",
        "     DualDuct Cold Inlet,     !- Cold Air Inlet Node Name",
        "     0.47,                    !- Maximum Damper Air Flow Rate {m3/s}",
        "     0.3,                     !- Zone Minimum Air Flow Fraction",
        "     ,                        !- Design Specification Outdoor Air Object Name",
        "     TurndownMinAirFlowSch;   !- Minimum Air Flow Turndown Schedule Name",

        "   Schedule:Compact,",
        "     TurndownMinAirFlowSch,   !- Name",
        "     Fraction,                !- Schedule Type Limits Name",
        "     Through: 12/31,          !- Field 1",
        "     For: Weekdays,           !- Field 2",
        "     Until: 7:00,0.50,        !- Field 3",
        "     Until: 17:00,0.75,       !- Field 4",
        "     Until: 24:00,0.50,       !- Field 5",
        "     For: SummerDesignDay WinterDesignDay, !- Field 6",
        "     Until: 24:00,1.0,        !- Field 7",
        "     For: Weekends Holidays CustomDay1 CustomDay2, !- Field 8",
        "     Until: 24:00,0.25;       !- Field 9",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    DualDuct::GetDualDuctInput(*state);

    // dual duct  VAV air terminal get input test
    EXPECT_ENUM_EQ(state->dataDualDuct->dd_airterminal(1).DamperType, DualDuctDamper::VariableVolume); // dual duct VAV Type
    EXPECT_EQ(state->dataDualDuct->dd_airterminal(1).Name, "VAV DUAL DUCT AT");                        // dual duct VAV Name
    EXPECT_TRUE(state->dataDualDuct->dd_airterminal(1).zoneTurndownMinAirFracSched != nullptr);        // turndown schedule exists
    EXPECT_EQ(state->dataDualDuct->dd_airterminal(1).ZoneTurndownMinAirFrac, 1.0);                     // turndown fraction initialized to 1.0
    EXPECT_EQ(state->dataDualDuct->dd_airterminal(1).ZoneMinAirFracDes, 0.3);                          // design minimum flow fraction
}

TEST_F(EnergyPlusFixture, DualDuctVAVAirTerminals_MinFlowTurnDownTest)
{
    std::string const idf_objects = delimited_string({
        "   Zone,",
        "    Thermal Zone;               !- Name",

        "   ZoneHVAC:EquipmentConnections,",
        "     Thermal Zone,              !- Zone Name",
        "     Thermal Zone Equipment,    !- Zone Conditioning Equipment List Name",
        "     DualDuct Outlet,           !- Zone Air Inlet Node or NodeList Name",
        "     ,                          !- Zone Air Exhaust Node or NodeList Name",
        "     Zone 1 Air Node,           !- Zone Air Node Name",
        "     Zone 1 Return Node;        !- Zone Return Air Node Name",

        "   ZoneHVAC:EquipmentList,",
        "     Thermal Zone Equipment,    !- Name",
        "     SequentialLoad,            !- Load Distribution Scheme",
        "     ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "     ADU Dual Duct AT,          !- Zone Equipment 1 Name",
        "     1,                         !- Zone Equipment 1 Cooling Sequence",
        "     1;                         !- Zone Equipment 1 Heating or No-Load Sequence",

        "   ZoneHVAC:AirDistributionUnit,",
        "     ADU Dual Duct AT,        !- Name",
        "     DualDuct Outlet,         !- Air Distribution Unit Outlet Node Name",
        "     AirTerminal:DualDuct:VAV,!- Air Terminal Object Type",
        "     VAV Dual Duct AT;        !- Air Terminal Name",

        "   AirTerminal:DualDuct:VAV,",
        "     VAV Dual Duct AT,        !- Name",
        "     ,                        !- Availability Schedule Name",
        "     DualDuct Outlet,         !- Air Outlet Node Name",
        "     DualDuct Hot Inlet,      !- Hot Air Inlet Node Name",
        "     DualDuct Cold Inlet,     !- Cold Air Inlet Node Name",
        "     1.0,                     !- Maximum Damper Air Flow Rate {m3/s}",
        "     0.3,                     !- Zone Minimum Air Flow Fraction",
        "     ,                        !- Design Specification Outdoor Air Object Name",
        "     TurndownMinAirFlowSch1;  !- Minimum Air Flow Turndown Schedule Name",

        "   Schedule:Compact,",
        "     TurndownMinAirFlowSch1,     !- Name",
        "     Fraction,                   !- Schedule Type Limits Name",
        "     Through: 12/31,             !- Field 1",
        "     For: AllDays,               !- Field 2",
        "     Until: 24:00, 1.0;          !- Field 3",

        "   Schedule:Compact,",
        "     TurndownMinAirFlowSch2,     !- Name",
        "     Fraction,                   !- Schedule Type Limits Name",
        "     Through: 12/31,             !- Field 1",
        "     For: AllDays,               !- Field 2",
        "     Until: 24:00, 0.5;          !- Field 3",

        "   ScheduleTypeLimits,",
        "     Fraction,                   !- Name",
        "     0,                          !- Lower Limit Value",
        "     1,                          !- Upper Limit Value",
        "     CONTINUOUS;                 !- Numeric Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // setup variables for dual duct VAV air terminal
    int DDNum = 1;
    int ZoneNum = 1;
    int ZoneNodeNum = 1;
    bool ErrorsFound = false;
    bool FirstHVACIteration = true;

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
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    DualDuct::GetDualDuctInput(*state);

    auto &thisDDAirTerminal = state->dataDualDuct->dd_airterminal(DDNum);

    // dual duct  VAV air terminal get input test
    EXPECT_ENUM_EQ(thisDDAirTerminal.DamperType, DualDuctDamper::VariableVolume); // dual duct VAV Type
    EXPECT_EQ(thisDDAirTerminal.Name, "VAV DUAL DUCT AT");                        // dual duct VAV Name
    EXPECT_TRUE(thisDDAirTerminal.zoneTurndownMinAirFracSched != nullptr);        // turndown schedule exists
    EXPECT_EQ(thisDDAirTerminal.ZoneTurndownMinAirFrac, 1.0);                     // turndown fraction initialized to 1.0
    EXPECT_EQ(thisDDAirTerminal.ZoneMinAirFracDes, 0.3);                          // design minimum flow fraction

    int OutNode = thisDDAirTerminal.OutletNodeNum;
    int HotInNode = thisDDAirTerminal.HotAirInletNodeNum;
    int ColdInNode = thisDDAirTerminal.ColdAirInletNodeNum;

    // calculate mass flow rates
    Real64 SysMinMassFlowRes = 1.0 * state->dataEnvrn->StdRhoAir * 0.30 * 1.0; // min flow rate at 1.0 turndown fraction
    Real64 SysMaxMassFlowRes = 1.0 * state->dataEnvrn->StdRhoAir;              // inputs from dual duct VAV AT

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired = 2000.0;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(HotInNode).Temp = 35.0;
    state->dataLoopNodes->Node(HotInNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(HotInNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(HotInNode).Temp, state->dataLoopNodes->Node(HotInNode).HumRat);

    // test with heating load and turndown fraction schedule value set 1.0
    state->dataDualDuct->dd_airterminal(DDNum).zoneTurndownMinAirFracSched = Sched::GetSchedule(*state, "TURNDOWNMINAIRFLOWSCH1"); //
    state->dataLoopNodes->Node(OutNode).MassFlowRate = SysMaxMassFlowRes;
    state->dataLoopNodes->Node(HotInNode).MassFlowRate = SysMaxMassFlowRes;
    state->dataLoopNodes->Node(HotInNode).MassFlowRateMaxAvail = SysMaxMassFlowRes;
    state->dataGlobal->BeginEnvrnFlag = true;
    FirstHVACIteration = true;
    state->dataDualDuct->dd_airterminal(DDNum).InitDualDuct(*state, FirstHVACIteration);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    thisDDAirTerminal.InitDualDuct(*state, FirstHVACIteration);
    thisDDAirTerminal.SimDualDuctVarVol(*state, ZoneNum, ZoneNodeNum);
    // check inputs and calculated values for turndown fraction set to 1.0
    EXPECT_EQ(0.3, thisDDAirTerminal.ZoneMinAirFracDes);
    EXPECT_EQ(1.0, thisDDAirTerminal.ZoneTurndownMinAirFrac);
    EXPECT_EQ(0.3, thisDDAirTerminal.ZoneMinAirFracDes * thisDDAirTerminal.ZoneTurndownMinAirFrac);
    EXPECT_EQ(0.3, thisDDAirTerminal.ZoneMinAirFrac);
    EXPECT_EQ(SysMinMassFlowRes, thisDDAirTerminal.dd_airterminalOutlet.AirMassFlowRate);
    EXPECT_EQ(SysMinMassFlowRes, thisDDAirTerminal.dd_airterminalOutlet.AirMassFlowRateMinAvail);
    EXPECT_EQ(SysMinMassFlowRes, thisDDAirTerminal.dd_airterminalHotAirInlet.AirMassFlowRateMax * thisDDAirTerminal.ZoneMinAirFrac);
    EXPECT_EQ(SysMinMassFlowRes,
              thisDDAirTerminal.dd_airterminalHotAirInlet.AirMassFlowRateMax * thisDDAirTerminal.ZoneMinAirFracDes *
                  thisDDAirTerminal.ZoneTurndownMinAirFrac);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(ColdInNode).MassFlowRate);

    // test with heating load and turndown fraction schedule value set 0.5
    state->dataDualDuct->dd_airterminal(DDNum).zoneTurndownMinAirFracSched = Sched::GetSchedule(*state, "TURNDOWNMINAIRFLOWSCH2");
    SysMinMassFlowRes = 1.0 * state->dataEnvrn->StdRhoAir * 0.30 * 0.5; // min flow rate at 0.5 turndown fraction
    state->dataLoopNodes->Node(OutNode).MassFlowRate = SysMaxMassFlowRes;
    state->dataLoopNodes->Node(HotInNode).MassFlowRate = SysMaxMassFlowRes;
    state->dataLoopNodes->Node(HotInNode).MassFlowRateMaxAvail = SysMaxMassFlowRes;
    state->dataGlobal->BeginEnvrnFlag = true;
    FirstHVACIteration = true;
    state->dataDualDuct->dd_airterminal(DDNum).InitDualDuct(*state, FirstHVACIteration);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    thisDDAirTerminal.InitDualDuct(*state, FirstHVACIteration);
    thisDDAirTerminal.SimDualDuctVarVol(*state, ZoneNum, ZoneNodeNum);
    // check inputs and calculated values for turndown fraction set to 0.5
    EXPECT_EQ(0.3, thisDDAirTerminal.ZoneMinAirFracDes);
    EXPECT_EQ(0.5, thisDDAirTerminal.ZoneTurndownMinAirFrac);
    EXPECT_EQ(0.15, thisDDAirTerminal.ZoneMinAirFracDes * thisDDAirTerminal.ZoneTurndownMinAirFrac);
    EXPECT_EQ(0.15, thisDDAirTerminal.ZoneMinAirFrac);
    EXPECT_EQ(SysMinMassFlowRes, thisDDAirTerminal.dd_airterminalOutlet.AirMassFlowRate);
    EXPECT_EQ(SysMinMassFlowRes, thisDDAirTerminal.dd_airterminalOutlet.AirMassFlowRateMinAvail);
    EXPECT_EQ(SysMinMassFlowRes, thisDDAirTerminal.dd_airterminalHotAirInlet.AirMassFlowRateMax * thisDDAirTerminal.ZoneMinAirFrac);
    EXPECT_EQ(SysMinMassFlowRes,
              thisDDAirTerminal.dd_airterminalHotAirInlet.AirMassFlowRateMax * thisDDAirTerminal.ZoneMinAirFracDes *
                  thisDDAirTerminal.ZoneTurndownMinAirFrac);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(ColdInNode).MassFlowRate);
}

TEST_F(EnergyPlusFixture, DualDuctAirTerminal_reportTerminalUnit)
{
    using namespace EnergyPlus::OutputReportPredefined;
    state->init_state(*state);

    auto &orp = *state->dataOutRptPredefined;

    SetPredefinedTables(*state);

    auto *schedA = Sched::AddScheduleConstant(*state, "schA");
    auto *schedB = Sched::AddScheduleConstant(*state, "schB");

    auto &adu = state->dataDefineEquipment->AirDistUnit;
    adu.allocate(2);
    adu(1).Name = "ADU a";
    adu(1).TermUnitSizingNum = 1;

    auto &siz = state->dataSize->TermUnitFinalZoneSizing;
    siz.allocate(2);
    siz(1).DesCoolVolFlowMin = 0.15;
    siz(1).MinOA = 0.05;
    siz(1).CoolDesTemp = 12.5;
    siz(1).HeatDesTemp = 40.0;
    siz(1).DesHeatLoad = 2000.0;
    siz(1).DesCoolLoad = 3000.0;

    auto &ddat = state->dataDualDuct->dd_airterminal;
    ddat.allocate(2);
    ddat(1).ADUNum = 1;
    ddat(1).DamperType = DualDuctDamper::ConstantVolume;
    ddat(1).MaxAirVolFlowRate = 0.30;
    ddat(1).zoneTurndownMinAirFracSched = schedA;
    ddat(1).OARequirementsPtr = 0;

    ddat(1).reportTerminalUnit(*state);

    EXPECT_EQ("0.15", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlow, "ADU a"));
    EXPECT_EQ("0.05", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOutdoorFlow, "ADU a"));
    EXPECT_EQ("12.50", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupCoolingSP, "ADU a"));
    EXPECT_EQ("40.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupHeatingSP, "ADU a"));
    EXPECT_EQ("2000.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatingCap, "ADU a"));
    EXPECT_EQ("3000.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolingCap, "ADU a"));
    EXPECT_EQ("ConstantVolume", RetrievePreDefTableEntry(*state, orp.pdchAirTermTypeInp, "ADU a"));
    EXPECT_EQ("0.30", RetrievePreDefTableEntry(*state, orp.pdchAirTermPrimFlow, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermSecdFlow, "ADU a"));
    EXPECT_EQ("schA", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlowSch, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMaxFlowReh, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOAflowSch, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatCoilType, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolCoilType, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanType, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanName, "ADU a"));

    adu(2).Name = "ADU b";
    adu(2).TermUnitSizingNum = 2;

    siz(2).DesCoolVolFlowMin = 0.16;
    siz(2).MinOA = 0.06;
    siz(2).CoolDesTemp = 12.6;
    siz(2).HeatDesTemp = 41.0;
    siz(2).DesHeatLoad = 2100.0;
    siz(2).DesCoolLoad = 3100.0;

    ddat(2).ADUNum = 2;
    ddat(2).DamperType = DualDuctDamper::VariableVolume;
    ddat(2).MaxAirVolFlowRate = 0.31;
    ddat(2).zoneTurndownMinAirFracSched = nullptr;
    ddat(2).OARequirementsPtr = 1;

    auto &oa = state->dataSize->OARequirements;
    oa.allocate(1);
    oa(1).oaFlowFracSched = schedB;

    ddat(2).reportTerminalUnit(*state);

    EXPECT_EQ("0.16", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlow, "ADU b"));
    EXPECT_EQ("0.06", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOutdoorFlow, "ADU b"));
    EXPECT_EQ("12.60", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupCoolingSP, "ADU b"));
    EXPECT_EQ("41.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupHeatingSP, "ADU b"));
    EXPECT_EQ("2100.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatingCap, "ADU b"));
    EXPECT_EQ("3100.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolingCap, "ADU b"));
    EXPECT_EQ("VariableVolume", RetrievePreDefTableEntry(*state, orp.pdchAirTermTypeInp, "ADU b"));
    EXPECT_EQ("0.31", RetrievePreDefTableEntry(*state, orp.pdchAirTermPrimFlow, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermSecdFlow, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlowSch, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMaxFlowReh, "ADU b"));
    EXPECT_EQ("schB", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOAflowSch, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatCoilType, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolCoilType, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanType, "ADU b"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanName, "ADU b"));
}
