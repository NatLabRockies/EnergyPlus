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

// EnergyPlus::Standalone ERV Unit Tests

#include <fstream>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

using namespace EnergyPlus;
using namespace SimulationManager;
using namespace DataSizing;

TEST_F(EnergyPlusFixture, ParallelPIUTest1)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                               !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                               !- Zone Name",
        "  SPACE2-1 Equipment,                     !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,                       !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,                  !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,                      !- Zone Air Node Name",
        "  SPACE2-1 Return Node;                   !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                     !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,                           !- Zone Equipment 1 Name",
        "  1,                                      !- Zone Equipment 1 Cooling Sequence",
        "  1;                                      !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,                           !- Name",
        "  SPACE2-1 In Node,                       !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:ParallelPIU:Reheat,  !- Air Terminal Object Type",
        "  SPACE2-1 Parallel PIU Reheat;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:ParallelPIU:Reheat,",
        "  SPACE2-1 Parallel PIU Reheat,           !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  0.1,                                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.05,                                   !- Maximum Secondary Air Flow Rate {m3/s}",
        "  0.2,                                    !- Minimum Primary Air Flow Fraction",
        "  0.1,                                    !- Fan On Flow Fraction",
        "  SPACE2-1 ATU In Node,                   !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,                  !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,                       !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,                     !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,                       !- Fan Name",
        "  Coil:Heating:Electric,                  !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,                     !- Reheat Coil Name",
        "  0.0,                                    !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                                    !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001;                                 !- Convergence Tolerance",

        "Fan:ConstantVolume,",
        "  SPACE2-1 PIU Fan,                       !- Name",
        "  AlwaysOff,                              !- Availability Schedule Name",
        "  0.5,                                    !- Fan Total Efficiency",
        "  50.0,                                   !- Pressure Rise {Pa}",
        "  0.05,                                   !- Maximum Flow Rate {m3/s}",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Airstream Fraction",
        "  SPACE2-1 ATU Sec Node,                  !- Air Inlet Node Name",
        "  SPACE2-1 ATU Fan Outlet Node;           !- Air Outlet Node Name",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,                     !- Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,                   !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Fan Outlet Node;           !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,                     !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  1.0,                                    !- Efficiency",
        "  1000,                                   !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Inlet Node Name",
        "  SPACE2-1 In Node;                       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  AlwaysOn,                               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);
    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    // node number table
    //  1   SPACE2-1 Air Node
    //  2   SPACE2-1 Return Node
    //  3   SPACE2-1 In Node
    //  4   SPACE2-1 ATU Sec Node
    //  5   SPACE2-1 ATU Fan Outlet Node
    //  6   SPACE2-1 ATU In Node
    //  7   SPACE2-1 Zone Coil Air In Node

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup for Zone 1 VAV No Reheat
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    int SecNodeNum = state->dataPowerInductionUnits->PIU(SysNum).SecAirInNode;
    int PriNodeNum = state->dataPowerInductionUnits->PIU(SysNum).PriAirInNode;
    bool FirstHVACIteration = true;
    Real64 SecMaxMassFlow = 0.05 * state->dataEnvrn->StdRhoAir; // From inputs

    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    state->dataFans->fans(1)->init(*state);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;

    // Note that the fan schedule is always off, so the PIU fan should only run if the night cycle turn on flag is true

    // First test - Heating load, TurnZoneFansOn is false, no primary flow - expecting no secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = false;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Second test - Heating load, TurnZoneFansOn is true, no primary flow - expecting secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Third test - Cooling load TurnZoneFansOn is true, no primary flow - expecting no secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Fourth test - Cooling load TurnFansOn is true, no primary flow - expecting no secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Fifth test - Heating load TurnFansOn is true, no primary flow - expecting secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Sixth test - Heating load TurnFansOn is true, yes primary flow, deadbandorsetback is true - expecting secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = state->dataPowerInductionUnits->PIU(SysNum).MinPriAirMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = true;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.2, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Seventh test - Heating load TurnFansOn is true, yes primary flow - expecting secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = state->dataPowerInductionUnits->PIU(SysNum).MinPriAirMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.2, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Eighth test - Cooling load TurnFansOn is true, yes primary flow - expecting secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = state->dataPowerInductionUnits->PIU(SysNum).MinPriAirMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2000.0; // Cooling load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(1.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Cleanup
    state->dataHeatBalFanSys->TempControlType.deallocate();
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.deallocate();
}

TEST_F(EnergyPlusFixture, SeriesPIUTest1)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                !- Zone Name",
        "  SPACE2-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,        !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,   !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,       !- Zone Air Node Name",
        "  SPACE2-1 Return Node;    !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,      !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,            !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,            !- Name",
        "  SPACE2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:SeriesPIU:Reheat,  !- Air Terminal Object Type",
        "  SPACE2-1 Series PIU Reheat;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
        "  SPACE2-1 Series PIU Reheat,     !- Name",
        "  AlwaysOn,    !- Availability Schedule Name",
        "  0.15,                !- Maximum Air Flow Rate {m3/s}",
        "  0.05,                !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.2,                !- Minimum Primary Air Flow Fraction",
        "  SPACE2-1 ATU In Node,    !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,   !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,        !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,  !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,      !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,        !- Fan Name",
        "  Coil:Heating:Electric,      !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,      !- Reheat Coil Name",
        "  0.0,                !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                     !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001;                  !- Convergence Tolerance",

        "Fan:ConstantVolume,",
        "  SPACE2-1 PIU Fan,        !- Name",
        "  AlwaysOff,           !- Availability Schedule Name",
        "  0.5,                     !- Fan Total Efficiency",
        "  50.0,                    !- Pressure Rise {Pa}",
        "  0.05,                !- Maximum Flow Rate {m3/s}",
        "  0.9,                     !- Motor Efficiency",
        "  1.0,                     !- Motor In Airstream Fraction",
        "  SPACE2-1 ATU Fan Inlet Node,   !- Air Inlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node;  !- Air Outlet Node Name",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,      !- Name",
        "  SPACE2-1 ATU Fan Inlet Node,  !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,    !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Sec Node;  !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,      !- Name",
        "  AlwaysOn,    !- Availability Schedule Name",
        "  1.0,                     !- Efficiency",
        "  1000,                !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,  !- Air Inlet Node Name",
        "  SPACE2-1 In Node;       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,               !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  0;                       !- Hourly Value",
        "Schedule:Constant,",
        "  AlwaysOn,               !- Name",
        "  ,                        !- Schedule Type Limits Name",
        "  1;                       !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);

    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    // node number table
    //  1   SPACE2-1 Air Node
    //  2   SPACE2-1 Return Node
    //  3   SPACE2-1 In Node
    //  4   SPACE2-1 ATU Sec Node
    //  5   SPACE2-1 ATU Fan Outlet Node
    //  6   SPACE2-1 ATU In Node
    //  7   SPACE2-1 Zone Coil Air In Node

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup for Zone 1 VAV No Reheat
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    int SecNodeNum = state->dataPowerInductionUnits->PIU(SysNum).SecAirInNode;
    int PriNodeNum = state->dataPowerInductionUnits->PIU(SysNum).PriAirInNode;
    bool FirstHVACIteration = true;

    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    state->dataFans->fans(1)->init(*state);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;

    // From inputs
    Real64 SecMaxMassFlow = state->dataPowerInductionUnits->PIU(SysNum).MaxTotAirMassFlow;
    Real64 PriMaxMassFlow = state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow;
    Real64 PriMinMassFlow =
        state->dataPowerInductionUnits->PIU(SysNum).MaxPriAirMassFlow * state->dataPowerInductionUnits->PIU(SysNum).MinPriAirFlowFrac;
    Real64 SecMassFlowAtPrimMin = state->dataPowerInductionUnits->PIU(SysNum).MaxTotAirMassFlow - PriMinMassFlow;
    Real64 SecMassFlowAtPrimMax = state->dataPowerInductionUnits->PIU(SysNum).MaxTotAirMassFlow - PriMaxMassFlow;

    // Note that the fan schedule is always off, so the PIU fan should only run if the night cycle turn on flag is true

    // First test - Heating load, TurnZoneFansOn is false, no primary flow - expecting no secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = false;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Second test - Heating load, TurnZoneFansOn is true, no primary flow - expecting max secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Third test - Cooling load TurnZoneFansOn is true, no primary flow - expecting no secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Fourth test - Cooling load TurnFansOn is true, no primary flow - expecting no secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Fifth test - Heating load TurnFansOn is true, no primary flow - expecting max secondary flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Sixth test - Heating load TurnFansOn is true, yes min primary flow, deadbandorsetback is true - expecting secondary flow at primary min flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = true;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMassFlowAtPrimMin, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(1.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Seventh test - Heating load TurnFansOn is true, yes min primary flow - expecting secondary flow at primary min flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMassFlowAtPrimMin, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(1.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Eighth test - Cooling load TurnFansOn is true, yes primary flow at max - expecting secondary flow at primary max flow
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2000.0; // Heating load - expect min flow rate
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHVACGlobal->TurnFansOn = true;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_EQ(SecMassFlowAtPrimMax, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(1.0, state->dataPowerInductionUnits->PIU(SysNum).PriDamperPosition);

    // Cleanup
    state->dataHeatBalFanSys->TempControlType.deallocate();
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.deallocate();
}

// cf: https://github.com/NREL/EnergyPlus/issues/7183
TEST_F(EnergyPlusFixture, PIUArrayOutOfBounds)
{

    state->dataPowerInductionUnits->NumSeriesPIUs = 1;
    state->dataPowerInductionUnits->NumPIUs = 1;
    state->dataPowerInductionUnits->PIU.allocate(1);
    int PIUNum = 1;
    state->dataPowerInductionUnits->PIU(PIUNum).Name = "Series PIU";
    state->dataPowerInductionUnits->PIU(PIUNum).UnitType_Num = DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat;
    state->dataPowerInductionUnits->PIU(PIUNum).HCoilType = PoweredInductionUnits::HtgCoilType::Electric;

    // Go into all of the autosize blocks (aside from Heating/Steam coils)
    state->dataPowerInductionUnits->PIU(PIUNum).MaxPriAirVolFlow = AutoSize;
    state->dataPowerInductionUnits->PIU(PIUNum).MaxTotAirVolFlow = AutoSize;
    state->dataPowerInductionUnits->PIU(PIUNum).MaxSecAirVolFlow = AutoSize;
    state->dataPowerInductionUnits->PIU(PIUNum).MinPriAirFlowFrac = AutoSize;
    state->dataPowerInductionUnits->PIU(PIUNum).FanOnFlowFrac = AutoSize;
    state->dataPowerInductionUnits->PIU(PIUNum).MaxVolHotWaterFlow = AutoSize;
    state->dataPowerInductionUnits->PIU(PIUNum).MaxVolHotSteamFlow = AutoSize;

    state->dataSize->CurSysNum = 0;
    state->dataSize->SysSizingRunDone = false;
    state->dataSize->ZoneSizingRunDone = true;

    // Test array out of bounds error. Notice that CurZoneEqNum is 2, while CurTermUnitSizingNum is 1
    // CurZoneEqNum = Current Zone Equipment index (0 if not simulating ZoneEq)
    // CurTermUnitSizingNum = Current terminal unit sizing index for TermUnitSizing and TermUnitFinalZoneSizing
    state->dataSize->CurZoneEqNum = 2;
    state->dataSize->FinalZoneSizing.allocate(2);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 10.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.006;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.008;

    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).AirVolFlow = 1.0;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).MinPriFlowFrac = 0.5;
    state->dataSize->TermUnitSingDuct = true;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));

    // Call the sizing routine now
    PoweredInductionUnits::SizePIU(*state, PIUNum);

    EXPECT_TRUE(compare_err_stream(""));
}

TEST_F(EnergyPlusFixture, SeriesPIUZoneOAVolumeFlowRateTest)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                !- Zone Name",
        "  SPACE2-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,        !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,   !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,       !- Zone Air Node Name",
        "  SPACE2-1 Return Node;    !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,      !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,            !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,            !- Name",
        "  SPACE2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:SeriesPIU:Reheat,  !- Air Terminal Object Type",
        "  SPACE2-1 Series PIU Reheat;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
        "  SPACE2-1 Series PIU Reheat,     !- Name",
        "  ,                        !- Availability Schedule Name",
        "  0.15,                    !- Maximum Air Flow Rate {m3/s}",
        "  0.05,                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.2,                     !- Minimum Primary Air Flow Fraction",
        "  SPACE2-1 ATU In Node,    !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,   !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,        !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,  !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,      !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,        !- Fan Name",
        "  Coil:Heating:Electric,      !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,      !- Reheat Coil Name",
        "  0.0,                     !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                     !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001;                  !- Convergence Tolerance",

        "Fan:ConstantVolume,",
        "  SPACE2-1 PIU Fan,        !- Name",
        "  ,                        !- Availability Schedule Name",
        "  0.5,                     !- Fan Total Efficiency",
        "  50.0,                    !- Pressure Rise {Pa}",
        "  0.05,                    !- Maximum Flow Rate {m3/s}",
        "  0.9,                     !- Motor Efficiency",
        "  1.0,                     !- Motor In Airstream Fraction",
        "  SPACE2-1 ATU Fan Inlet Node,   !- Air Inlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node;  !- Air Outlet Node Name",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,      !- Name",
        "  SPACE2-1 ATU Fan Inlet Node,  !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,    !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Sec Node;   !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,      !- Name",
        "  ,                        !- Availability Schedule Name",
        "  1.0,                     !- Efficiency",
        "  2000,                    !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,  !- Air Inlet Node Name",
        "  SPACE2-1 In Node;        !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);
    EXPECT_TRUE(compare_err_stream(""));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;

    // Setup for Zone 1 series PIU Reheat air terminal
    int ZoneNum = 1;
    int PIUNum = 1;
    int ZoneNodeNum = 1;

    auto &thisSeriesAT = state->dataPowerInductionUnits->PIU(PIUNum);
    int SecNodeNum = thisSeriesAT.SecAirInNode;
    int PriNodeNum = thisSeriesAT.PriAirInNode;
    bool FirstHVACIteration = true;

    state->dataGlobal->BeginEnvrnFlag = true;
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, PIUNum, FirstHVACIteration);
    state->dataFans->fans(1)->init(*state);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    state->dataHVACGlobal->TurnFansOn = true;

    // From inputs
    Real64 SecMaxMassFlow = thisSeriesAT.MaxTotAirMassFlow;
    Real64 PriMaxMassFlow = thisSeriesAT.MaxPriAirMassFlow;
    Real64 PriMinMassFlow = thisSeriesAT.MaxPriAirMassFlow * thisSeriesAT.MinPriAirFlowFrac;
    Real64 SecMassFlowAtPrimMin = thisSeriesAT.MaxTotAirMassFlow - PriMinMassFlow;
    Real64 SecMassFlowAtPrimMax = thisSeriesAT.MaxTotAirMassFlow - PriMaxMassFlow;

    // Needs an airloop, assume 20% outdoor air
    Real64 constexpr AirLoopOAFraction = 0.20;
    thisSeriesAT.AirLoopNum = 1;
    state->dataAirLoop->AirLoopFlow.allocate(1);
    state->dataAirLoop->AirLoopFlow(thisSeriesAT.AirLoopNum).OAFrac = AirLoopOAFraction;

    state->dataZoneEquip->ZoneEquipConfig(thisSeriesAT.CtrlZoneNum).InletNodeAirLoopNum(thisSeriesAT.ctrlZoneInNodeIndex) = 1;
    // set heating zone and AT unit inlet conditions
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.005;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = state->dataLoopNodes->Node(ZoneNodeNum).Temp;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = state->dataLoopNodes->Node(ZoneNodeNum).HumRat;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy = state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy;
    state->dataLoopNodes->Node(PriNodeNum).Temp = 5.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.006;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);

    // test 1:  Heating load, at 0.0 primary air flow rate
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, PIUNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    PoweredInductionUnits::ReportPIU(*state, PIUNum);
    Real64 expect_OutdoorAirFlowRate = (0.0 / state->dataEnvrn->StdRhoAir) * AirLoopOAFraction;
    EXPECT_EQ(SecMaxMassFlow, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(0.0, state->dataLoopNodes->Node(PriNodeNum).MassFlowRate);
    EXPECT_EQ(expect_OutdoorAirFlowRate, thisSeriesAT.OutdoorAirFlowRate);

    // test 2:  Heating load, at minimum primary flow rate
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, PIUNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    PoweredInductionUnits::ReportPIU(*state, PIUNum);
    expect_OutdoorAirFlowRate = (PriMinMassFlow / state->dataEnvrn->StdRhoAir) * AirLoopOAFraction;
    EXPECT_EQ(SecMassFlowAtPrimMin, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(PriMinMassFlow, state->dataLoopNodes->Node(PriNodeNum).MassFlowRate);
    EXPECT_EQ(expect_OutdoorAirFlowRate, thisSeriesAT.OutdoorAirFlowRate);

    // test 3: - Cooling load, at maximum primary air flow rate
    // set cooling zone and AT unit inlet conditions
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 24.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0080;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = state->dataLoopNodes->Node(ZoneNodeNum).Temp;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = state->dataLoopNodes->Node(ZoneNodeNum).HumRat;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy = state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy;
    state->dataLoopNodes->Node(PriNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0075;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);

    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -3000.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, PIUNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    PoweredInductionUnits::ReportPIU(*state, PIUNum);
    expect_OutdoorAirFlowRate = (PriMaxMassFlow / state->dataEnvrn->StdRhoAir) * AirLoopOAFraction;
    EXPECT_EQ(SecMassFlowAtPrimMax, state->dataLoopNodes->Node(SecNodeNum).MassFlowRate);
    EXPECT_EQ(PriMaxMassFlow, state->dataLoopNodes->Node(PriNodeNum).MassFlowRate);
    EXPECT_EQ(expect_OutdoorAirFlowRate, thisSeriesAT.OutdoorAirFlowRate);
}

TEST_F(EnergyPlusFixture, PIU_InducedAir_Plenums)
{
    // Test for #9831 - We need to have both a supply plenum and a return plenums, and the return plenums should have the induced air node filled

    std::string const idf_objects = delimited_string({
        "Timestep,",
        "  4;                                      !- Number of Timesteps per Hour",

        "ScheduleTypeLimits,",
        "  Any Number;                             !- Name",

        "Building,",
        "  Building 1,                             !- Name",
        "  0,                                      !- North Axis {deg}",
        "  ,                                       !- Terrain",
        "  ,                                       !- Loads Convergence Tolerance Value {W}",
        "  ,                                       !- Temperature Convergence Tolerance Value {deltaC}",
        "  ,                                       !- Solar Distribution",
        "  ,                                       !- Maximum Number of Warmup Days",
        "  ;                                       !- Minimum Number of Warmup Days",

        "Zone,",
        "  ReturnPlenum,                           !- Name",
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
        "  No;                                     !- Part of Total Floor Area",

        "BuildingSurface:Detailed,",
        "  FLOOR 2,                                !- Name",
        "  Floor,                                  !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.5,                              !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 2.5,                            !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 2.5;                             !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  RETURNPLENUM - 1-SOUTH,                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 3,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 2.5,                              !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 2.5,                             !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 3;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  RETURNPLENUM - 2-WEST,                  !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 3,                               !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 2.5,                              !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 3;                                !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  RETURNPLENUM - 3-EAST,                  !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 3,                               !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 2.5,                             !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 2.5,                            !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 3;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  RETURNPLENUM - 4-NORTH,                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 3,                              !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.5,                            !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 3;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ROOF 2,                                 !- Name",
        "  Roof,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 3,                               !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 3,                              !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 3,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 3;                                !- X,Y,Z Vertex 4 {m}",

        "Zone,",
        "  SupplyPlenum,                           !- Name",
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
        "  No;                                     !- Part of Total Floor Area",

        "BuildingSurface:Detailed,",
        "  FLOOR,                                  !- Name",
        "  Floor,                                  !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 3,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 3,                               !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 3,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 3;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ROOF,                                   !- Name",
        "  Roof,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 3.5,                             !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 3.5,                            !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 3.5,                             !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 3.5;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  SUPPLYPLENUM - 1-SOUTH,                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 3.5,                              !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 3,                                !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 3,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 3.5;                             !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  SUPPLYPLENUM - 2-WEST,                  !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 3.5,                             !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 3,                               !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 3,                                !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 3.5;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  SUPPLYPLENUM - 3-EAST,                  !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 3.5,                             !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 3,                               !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 3,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 3.5;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  SUPPLYPLENUM - 4-NORTH,                 !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 3.5,                            !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 3,                              !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 3,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 3.5;                             !- X,Y,Z Vertex 4 {m}",

        "Zone,",
        "  Zone1,                                  !- Name",
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
        "  FLOOR 1,                                !- Name",
        "  Floor,                                  !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ROOF 1,                                 !- Name",
        "  Roof,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.5,                             !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 2.5,                            !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.5;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ZONE1 - 1-SOUTH,                        !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.5,                              !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 2 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  20, 0, 2.5;                             !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ZONE1 - 2-WEST,                         !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.5;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ZONE1 - 3-EAST,                         !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 0, 2.5,                             !- X,Y,Z Vertex 1 {m}",
        "  20, 0, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  20, 10, 2.5;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ZONE1 - 4-NORTH,                        !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  20, 10, 2.5,                            !- X,Y,Z Vertex 1 {m}",
        "  20, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 2.5;                             !- X,Y,Z Vertex 4 {m}",

        "ZoneControl:Thermostat,",
        "  Zone1 Thermostat,                       !- Name",
        "  Zone1,                                  !- Zone or ZoneList Name",
        "  Zone1 Thermostat Schedule,              !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,        !- Control 1 Object Type",
        "  Thermostat Setpoint Dual Setpoint 2,    !- Control 1 Name",
        "  ,                                       !- Control 2 Object Type",
        "  ,                                       !- Control 2 Name",
        "  ,                                       !- Control 3 Object Type",
        "  ,                                       !- Control 3 Name",
        "  ,                                       !- Control 4 Object Type",
        "  ,                                       !- Control 4 Name",
        "  0;                                      !- Temperature Difference Between Cutout And Setpoint {deltaC}",

        "Schedule:Compact,",
        "  Zone1 Thermostat Schedule,              !- Name",
        "  Zone1 Thermostat Schedule Type Limits,  !- Schedule Type Limits Name",
        "  Through: 12/31,                         !- Field 1",
        "  For: AllDays,                           !- Field 2",
        "  Until: 24:00,                           !- Field 3",
        "  4;                                      !- Field 4",

        "ScheduleTypeLimits,",
        "  Zone1 Thermostat Schedule Type Limits,  !- Name",
        "  0,                                      !- Lower Limit Value {BasedOnField A3}",
        "  4,                                      !- Upper Limit Value {BasedOnField A3}",
        "  DISCRETE;                               !- Numeric Type",

        "ThermostatSetpoint:DualSetpoint,",
        "  Thermostat Setpoint Dual Setpoint 2,    !- Name",
        "  Schedule Constant 14,                   !- Heating Setpoint Temperature Schedule Name",
        "  Schedule Constant 13;                   !- Cooling Setpoint Temperature Schedule Name",

        "Schedule:Constant,",
        "  Schedule Constant 14,                   !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  19;                                     !- Hourly Value",

        "Schedule:Constant,",
        "  Schedule Constant 13,                   !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  26;                                     !- Hourly Value",

        "ZoneHVAC:EquipmentConnections,",
        "  Zone1,                                  !- Zone Name",
        "  Zone1 Equipment List,                   !- Zone Conditioning Equipment List Name",
        "  Zone1 Inlet Node List,                  !- Zone Air Inlet Node or NodeList Name",
        "  Zone1 Exhaust Node List,                !- Zone Air Exhaust Node or NodeList Name",
        "  Zone1 Zone Air Node,                    !- Zone Air Node Name",
        "  Zone1 Return Node List;                 !- Zone Return Air Node or NodeList Name",

        "NodeList,",
        "  Zone1 Inlet Node List,                  !- Name",
        "  SeriesPIU Outlet Node;                  !- Node Name 1",

        "NodeList,",
        "  Zone1 Exhaust Node List,                !- Name",
        "  SeriesPIU Secondary Air Inlet Node;     !- Node Name 1",

        "NodeList,",
        "  Zone1 Return Node List,                 !- Name",
        "  Zone1 Return Air Node;                  !- Node Name 1",

        "ZoneHVAC:AirDistributionUnit,",
        "  Air Terminal Single Duct Series PIU Reheat 1 Air Distribution Unit, !- Name",
        "  SeriesPIU Outlet Node,                  !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:SeriesPIU:Reheat, !- Air Terminal Object Type",
        "  Air Terminal Single Duct Series PIU Reheat 1; !- Air Terminal Name",

        "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
        "  Air Terminal Single Duct Series PIU Reheat 1, !- Name",
        "  ,                                       !- Availability Schedule Name",
        "  Autosize,                               !- Maximum Air Flow Rate {m3/s}",
        "  Autosize,                               !- Maximum Primary Air Flow Rate {m3/s}",
        "  Autosize,                               !- Minimum Primary Air Flow Fraction",
        "  SeriesPIU Supply Air Inlet Node,        !- Supply Air Inlet Node Name",
        "  SeriesPIU Secondary Air Inlet Node,     !- Secondary Air Inlet Node Name",
        "  SeriesPIU Outlet Node,                  !- Outlet Node Name",
        "  Air Terminal Single Duct Series PIU Reheat 1 Fan Outlet, !- Reheat Coil Air Inlet Node Name",
        "  Air Terminal Single Duct Series PIU Reheat 1 Mixer, !- Zone Mixer Name",
        "  Fan System Model 1,                     !- Fan Name",
        "  Coil:Heating:Electric,                  !- Reheat Coil Object Type",
        "  Coil Heating Electric 2,                !- Reheat Coil Name",
        "  Autosize,                               !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0,                                      !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.001;                                  !- Convergence Tolerance",

        "Coil:Heating:Electric,",
        "  Coil Heating Electric 2,                !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  1,                                      !- Efficiency",
        "  Autosize,                               !- Nominal Capacity {W}",
        "  Air Terminal Single Duct Series PIU Reheat 1 Fan Outlet, !- Air Inlet Node Name",
        "  SeriesPIU Outlet Node;                  !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  Always On Discrete,                     !- Name",
        "  OnOff,                                  !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

        "Fan:SystemModel,",
        "  Fan System Model 1,                     !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  Air Terminal Single Duct Series PIU Reheat 1 Mixer Outlet, !- Air Inlet Node Name",
        "  Air Terminal Single Duct Series PIU Reheat 1 Fan Outlet, !- Air Outlet Node Name",
        "  Autosize,                               !- Design Maximum Air Flow Rate {m3/s}",
        "  Discrete,                               !- Speed Control Method",
        "  0.2,                                    !- Electric Power Minimum Flow Rate Fraction",
        "  500,                                    !- Design Pressure Rise {Pa}",
        "  0.9,                                    !- Motor Efficiency",
        "  1,                                      !- Motor In Air Stream Fraction",
        "  Autosize,                               !- Design Electric Power Consumption {W}",
        "  PowerPerFlowPerPressure,                !- Design Power Sizing Method",
        "  840,                                    !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  1.66667,                                !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.7,                                    !- Fan Total Efficiency",
        "  ,                                       !- Electric Power Function of Flow Fraction Curve Name",
        "  ,                                       !- Night Ventilation Mode Pressure Rise {Pa}",
        "  ,                                       !- Night Ventilation Mode Flow Fraction",
        "  ,                                       !- Motor Loss Zone Name",
        "  0,                                      !- Motor Loss Radiative Fraction",
        "  General,                                !- End-Use Subcategory",
        "  1;                                      !- Number of Speeds",

        "AirLoopHVAC:ZoneMixer,",
        "  Air Terminal Single Duct Series PIU Reheat 1 Mixer, !- Name",
        "  Air Terminal Single Duct Series PIU Reheat 1 Mixer Outlet, !- Outlet Node Name",
        "  SeriesPIU Secondary Air Inlet Node,     !- Inlet Node Name 1",
        "  SeriesPIU Supply Air Inlet Node;        !- Inlet Node Name 2",

        "ZoneHVAC:EquipmentList,",
        "  Zone1 Equipment List,                   !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment Object Type 1",
        "  Air Terminal Single Duct Series PIU Reheat 1 Air Distribution Unit, !- Zone Equipment Name 1",
        "  1,                                      !- Zone Equipment Cooling Sequence 1",
        "  1,                                      !- Zone Equipment Heating or No-Load Sequence 1",
        "  ,                                       !- Zone Equipment Sequential Cooling Fraction Schedule Name 1",
        "  ;                                       !- Zone Equipment Sequential Heating Fraction Schedule Name 1",

        "Sizing:Zone,",
        "  Zone1,                                  !- Zone or ZoneList Name",
        "  SupplyAirTemperature,                   !- Zone Cooling Design Supply Air Temperature Input Method",
        "  14,                                     !- Zone Cooling Design Supply Air Temperature {C}",
        "  11.11,                                  !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "  SupplyAirTemperature,                   !- Zone Heating Design Supply Air Temperature Input Method",
        "  40,                                     !- Zone Heating Design Supply Air Temperature {C}",
        "  11.11,                                  !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "  0.0085,                                 !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "  0.008,                                  !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "  ,                                       !- Design Specification Outdoor Air Object Name",
        "  ,                                       !- Zone Heating Sizing Factor",
        "  ,                                       !- Zone Cooling Sizing Factor",
        "  DesignDay,                              !- Cooling Design Air Flow Method",
        "  0,                                      !- Cooling Design Air Flow Rate {m3/s}",
        "  0.000762,                               !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "  0,                                      !- Cooling Minimum Air Flow {m3/s}",
        "  0,                                      !- Cooling Minimum Air Flow Fraction",
        "  DesignDay,                              !- Heating Design Air Flow Method",
        "  0,                                      !- Heating Design Air Flow Rate {m3/s}",
        "  0.002032,                               !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "  0.1415762,                              !- Heating Maximum Air Flow {m3/s}",
        "  0.3,                                    !- Heating Maximum Air Flow Fraction",
        "  ,                                       !- Design Specification Zone Air Distribution Object Name",
        "  No,                                     !- Account for Dedicated Outdoor Air System",
        "  ,                                       !- Dedicated Outdoor Air System Control Strategy",
        "  ,                                       !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "  ,                                       !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "  Sensible Load Only No Latent Load,      !- Zone Load Sizing Method",
        "  HumidityRatioDifference,                !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "  ,                                       !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "  0.005,                                  !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "  HumidityRatioDifference,                !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "  ,                                       !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "  0.005;                                  !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",

        "Controller:MechanicalVentilation,",
        "  Controller Mechanical Ventilation 1,    !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  No,                                     !- Demand Controlled Ventilation",
        "  ZoneSum,                                !- System Outdoor Air Method",
        "  ,                                       !- Zone Maximum Outdoor Air Fraction {dimensionless}",
        "  Zone1,                                  !- Zone or ZoneList Name 1",
        "  ,                                       !- Design Specification Outdoor Air Object Name 1",
        "  ;                                       !- Design Specification Zone Air Distribution Object Name 1",

        "SimulationControl,",
        "  Yes,                                    !- Do Zone Sizing Calculation",
        "  Yes,                                    !- Do System Sizing Calculation",
        "  No,                                     !- Do Plant Sizing Calculation",
        "  Yes,                                    !- Run Simulation for Sizing Periods",
        "  No,                                     !- Run Simulation for Weather File Run Periods",
        "  ,                                       !- Do HVAC Sizing Simulation for Sizing Periods",
        "  ;                                       !- Maximum Number of HVAC Sizing Simulation Passes",

        "Sizing:Parameters,",
        "  1.25,                                   !- Heating Sizing Factor",
        "  1.15;                                   !- Cooling Sizing Factor",

        "RunPeriod,",
        "  Run Period 1,                           !- Name",
        "  1,                                      !- Begin Month",
        "  1,                                      !- Begin Day of Month",
        "  2009,                                   !- Begin Year",
        "  12,                                     !- End Month",
        "  31,                                     !- End Day of Month",
        "  2009,                                   !- End Year",
        "  Thursday,                               !- Day of Week for Start Day",
        "  No,                                     !- Use Weather File Holidays and Special Days",
        "  No,                                     !- Use Weather File Daylight Saving Period",
        "  No,                                     !- Apply Weekend Holiday Rule",
        "  Yes,                                    !- Use Weather File Rain Indicators",
        "  Yes;                                    !- Use Weather File Snow Indicators",

        "Output:Table:SummaryReports,",
        "  AllSummary;                             !- Report Name 1",

        "GlobalGeometryRules,",
        "  UpperLeftCorner,                        !- Starting Vertex Position",
        "  Counterclockwise,                       !- Vertex Entry Direction",
        "  Relative,                               !- Coordinate System",
        "  Relative,                               !- Daylighting Reference Point Coordinate System",
        "  Relative;                               !- Rectangular Surface Coordinate System",

        "Material:NoMass,",
        "  R13-IP,                                 !- Name",
        "  Smooth,                                 !- Roughness",
        "  2.28943238786998,                       !- Thermal Resistance {m2-K/W}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Material,",
        "  C5 - 4 IN HW CONCRETE,                  !- Name",
        "  MediumRough,                            !- Roughness",
        "  0.1014984,                              !- Thickness {m}",
        "  1.729577,                               !- Conductivity {W/m-K}",
        "  2242.585,                               !- Density {kg/m3}",
        "  836.8000,                               !- Specific Heat {J/kg-K}",
        "  0.9000000,                              !- Thermal Absorptance",
        "  0.6500000,                              !- Solar Absorptance",
        "  0.6500000;                              !- Visible Absorptance",

        "Construction,",
        "  R13 Construction,                       !- Name",
        "  R13-IP,                                 !- Layer 1",
        "  C5 - 4 IN HW CONCRETE;                  !- Layer 2",

        "ScheduleTypeLimits,",
        "  OnOff,                                  !- Name",
        "  0,                                      !- Lower Limit Value {BasedOnField A3}",
        "  1,                                      !- Upper Limit Value {BasedOnField A3}",
        "  Discrete,                               !- Numeric Type",
        "  availability;                           !- Unit Type",

        "ScheduleTypeLimits,",
        "  Temperature,                            !- Name",
        "  ,                                       !- Lower Limit Value {BasedOnField A3}",
        "  ,                                       !- Upper Limit Value {BasedOnField A3}",
        "  Continuous,                             !- Numeric Type",
        "  temperature;                            !- Unit Type",

        "  Schedule:Day:Interval,",
        "  Deck_Temperature_Default,               !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  No,                                     !- Interpolate to Timestep",
        "  24:00,                                  !- Time 1 {hh:mm}",
        "  12.8;                                   !- Value Until Time 1",

        "Schedule:Day:Interval,",
        "  Deck_Temperature_Summer_Design_Day,     !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  No,                                     !- Interpolate to Timestep",
        "  24:00,                                  !- Time 1 {hh:mm}",
        "  12.8;                                   !- Value Until Time 1",

        "Schedule:Day:Interval,",
        "  Deck_Temperature_Winter_Design_Day,     !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  No,                                     !- Interpolate to Timestep",
        "  24:00,                                  !- Time 1 {hh:mm}",
        "  12.8;                                   !- Value Until Time 1",

        "Schedule:Week:Daily,",
        "  Deck_Temperature Week Rule - Jan1-Dec31, !- Name",
        "  Deck_Temperature_Default,               !- Sunday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Monday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Tuesday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Wednesday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Thursday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Friday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Saturday Schedule:Day Name",
        "  Deck_Temperature_Default,               !- Holiday Schedule:Day Name",
        "  Deck_Temperature_Summer_Design_Day,     !- SummerDesignDay Schedule:Day Name",
        "  Deck_Temperature_Winter_Design_Day,     !- WinterDesignDay Schedule:Day Name",
        "  Deck_Temperature_Default,               !- CustomDay1 Schedule:Day Name",
        "  Deck_Temperature_Default;               !- CustomDay2 Schedule:Day Name",

        "Schedule:Year,",
        "  Deck_Temperature,                       !- Name",
        "  Temperature,                            !- Schedule Type Limits Name",
        "  Deck_Temperature Week Rule - Jan1-Dec31, !- Schedule:Week Name 1",
        "  1,                                      !- Start Month 1",
        "  1,                                      !- Start Day 1",
        "  12,                                     !- End Month 1",
        "  31;                                     !- End Day 1",

        "Schedule:Constant,",
        "  Always Off Discrete,                    !- Name",
        "  OnOff,                                  !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  Always On Continuous,                   !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

        "OutdoorAir:Node,",
        "  Model Outdoor Air Node;                 !- Name",

        "AirLoopHVAC,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat, !- Name",
        "  ,                                       !- Controller List Name",
        "  Packaged Rooftop VAV with PFP Boxes and ReheatAvailability Manager List, !- Availability Manager List Name",
        "  AutoSize,                               !- Design Supply Air Flow Rate {m3/s}",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Supply Branches, !- Branch List Name",
        "  ,                                       !- Connector List Name",
        "  Node 1,                                 !- Supply Side Inlet Node Name",
        "  Node 4,                                 !- Demand Side Outlet Node Name",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Demand Inlet Nodes, !- Demand Side Inlet Node Names",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Supply Outlet Nodes, !- Supply Side Outlet Node Names",
        "  1;                                      !- Design Return Air Flow Fraction of Supply Air Flow",

        "NodeList,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Supply Outlet Nodes, !- Name",
        "  Node 2;                                 !- Node Name 1",

        "NodeList,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Demand Inlet Nodes, !- Name",
        "  Node 3;                                 !- Node Name 1",

        "Sizing:System,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat, !- AirLoop Name",
        "  Sensible,                               !- Type of Load to Size On",
        "  Autosize,                               !- Design Outdoor Air Flow Rate {m3/s}",
        "  0.3,                                    !- Central Heating Maximum System Air Flow Ratio",
        "  7,                                      !- Preheat Design Temperature {C}",
        "  0.008,                                  !- Preheat Design Humidity Ratio {kgWater/kgDryAir}",
        "  12.8,                                   !- Precool Design Temperature {C}",
        "  0.008,                                  !- Precool Design Humidity Ratio {kgWater/kgDryAir}",
        "  12.8,                                   !- Central Cooling Design Supply Air Temperature {C}",
        "  16.7,                                   !- Central Heating Design Supply Air Temperature {C}",
        "  NonCoincident,                          !- Type of Zone Sum to Use",
        "  Yes,                                    !- 100% Outdoor Air in Cooling",
        "  Yes,                                    !- 100% Outdoor Air in Heating",
        "  0.0085,                                 !- Central Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "  0.008,                                  !- Central Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "  DesignDay,                              !- Cooling Supply Air Flow Rate Method",
        "  0,                                      !- Cooling Supply Air Flow Rate {m3/s}",
        "  0.0099676501,                           !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "  1,                                      !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "  3.9475456e-05,                          !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}",
        "  DesignDay,                              !- Heating Supply Air Flow Rate Method",
        "  0,                                      !- Heating Supply Air Flow Rate {m3/s}",
        "  0.0099676501,                           !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "  1,                                      !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "  1,                                      !- Heating Fraction of Autosized Cooling Supply Air Flow Rate",
        "  3.1588213e-05,                          !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}",
        "  ZoneSum,                                !- System Outdoor Air Method",
        "  1,                                      !- Zone Maximum Outdoor Air Fraction {dimensionless}",
        "  CoolingDesignCapacity,                  !- Cooling Design Capacity Method",
        "  Autosize,                               !- Cooling Design Capacity {W}",
        "  234.7,                                  !- Cooling Design Capacity Per Floor Area {W/m2}",
        "  1,                                      !- Fraction of Autosized Cooling Design Capacity",
        "  HeatingDesignCapacity,                  !- Heating Design Capacity Method",
        "  Autosize,                               !- Heating Design Capacity {W}",
        "  157,                                    !- Heating Design Capacity Per Floor Area {W/m2}",
        "  1,                                      !- Fraction of Autosized Heating Design Capacity",
        "  OnOff,                                  !- Central Cooling Capacity Control Method",
        "  Autosize;                               !- Occupant Diversity",

        "AvailabilityManagerAssignmentList,",
        "  Packaged Rooftop VAV with PFP Boxes and ReheatAvailability Manager List, !- Name",
        "  AvailabilityManager:Scheduled,          !- Availability Manager Object Type 1",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Availability Manager; !- Availability Manager Name 1",

        "AvailabilityManager:Scheduled,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Availability Manager, !- Name",
        "  Always On Discrete;                     !- Schedule Name",

        "BranchList,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Supply Branches, !- Name",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Main Branch; !- Branch Name 1",

        "Branch,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Main Branch, !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  AirLoopHVAC:OutdoorAirSystem,           !- Component Object Type 1",
        "  Air Loop HVAC Outdoor Air System 1,     !- Component Name 1",
        "  Node 1,                                 !- Component Inlet Node Name 1",
        "  Node 8,                                 !- Component Outlet Node Name 1",
        "  CoilSystem:Cooling:DX,                  !- Component Object Type 2",
        "  Coil Cooling DX Two Speed 1 CoilSystem, !- Component Name 2",
        "  Node 8,                                 !- Component Inlet Node Name 2",
        "  Node 9,                                 !- Component Outlet Node Name 2",
        "  Coil:Heating:Electric,                  !- Component Object Type 3",
        "  Coil Heating Electric 1,                !- Component Name 3",
        "  Node 9,                                 !- Component Inlet Node Name 3",
        "  Node 10,                                !- Component Outlet Node Name 3",
        "  Fan:VariableVolume,                     !- Component Object Type 4",
        "  Fan Variable Volume 1,                  !- Component Name 4",
        "  Node 10,                                !- Component Inlet Node Name 4",
        "  Node 2;                                 !- Component Outlet Node Name 4",

        "AirLoopHVAC:OutdoorAirSystem,",
        "  Air Loop HVAC Outdoor Air System 1,     !- Name",
        "  Air Loop HVAC Outdoor Air System 1 Controller List, !- Controller List Name",
        "  Air Loop HVAC Outdoor Air System 1 Equipment List; !- Outdoor Air Equipment List Name",

        "AirLoopHVAC:ControllerList,",
        "  Air Loop HVAC Outdoor Air System 1 Controller List, !- Name",
        "  Controller:OutdoorAir,                  !- Controller Object Type 1",
        "  Controller Outdoor Air 1;               !- Controller Name 1",

        "Controller:OutdoorAir,",
        "  Controller Outdoor Air 1,               !- Name",
        "  Node 7,                                 !- Relief Air Outlet Node Name",
        "  Node 1,                                 !- Return Air Node Name",
        "  Node 8,                                 !- Mixed Air Node Name",
        "  Node 6,                                 !- Actuator Node Name",
        "  0,                                      !- Minimum Outdoor Air Flow Rate {m3/s}",
        "  Autosize,                               !- Maximum Outdoor Air Flow Rate {m3/s}",
        "  NoEconomizer,                           !- Economizer Control Type",
        "  ModulateFlow,                           !- Economizer Control Action Type",
        "  28,                                     !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "  64000,                                  !- Economizer Maximum Limit Enthalpy {J/kg}",
        "  ,                                       !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "  ,                                       !- Electronic Enthalpy Limit Curve Name",
        "  -100,                                   !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "  NoLockout,                              !- Lockout Type",
        "  FixedMinimum,                           !- Minimum Limit Type",
        "  ,                                       !- Minimum Outdoor Air Schedule Name",
        "  ,                                       !- Minimum Fraction of Outdoor Air Schedule Name",
        "  ,                                       !- Maximum Fraction of Outdoor Air Schedule Name",
        "  Controller Mechanical Ventilation 1,    !- Mechanical Ventilation Controller Name",
        "  ,                                       !- Time of Day Economizer Control Schedule Name",
        "  No,                                     !- High Humidity Control",
        "  ,                                       !- Humidistat Control Zone Name",
        "  ,                                       !- High Humidity Outdoor Air Flow Ratio",
        "  Yes,                                    !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
        "  BypassWhenWithinEconomizerLimits;       !- Heat Recovery Bypass Control Type",

        "AvailabilityManagerAssignmentList,",
        "  Air Loop HVAC Outdoor Air System 1 Availability Manager List, !- Name",
        "  AvailabilityManager:Scheduled,          !- Availability Manager Object Type 1",
        "  Air Loop HVAC Outdoor Air System 1 Availability Manager; !- Availability Manager Name 1",

        "AvailabilityManager:Scheduled,",
        "  Air Loop HVAC Outdoor Air System 1 Availability Manager, !- Name",
        "  Always On Discrete;                     !- Schedule Name",

        "OutdoorAir:NodeList,",
        "  Node 6;                                 !- Node or NodeList Name 1",

        "AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "  Air Loop HVAC Outdoor Air System 1 Equipment List, !- Name",
        "  OutdoorAir:Mixer,                       !- Component Object Type 1",
        "  Air Loop HVAC Outdoor Air System 1 Outdoor Air Mixer; !- Component Name 1",

        "OutdoorAir:Mixer,",
        "  Air Loop HVAC Outdoor Air System 1 Outdoor Air Mixer, !- Name",
        "  Node 8,                                 !- Mixed Air Node Name",
        "  Node 6,                                 !- Outdoor Air Stream Node Name",
        "  Node 7,                                 !- Relief Air Stream Node Name",
        "  Node 1;                                 !- Return Air Stream Node Name",

        "SetpointManager:MixedAir,",
        "  Node 8 OS Default SPM,                  !- Name",
        "  Temperature,                            !- Control Variable",
        "  Node 2,                                 !- Reference Setpoint Node Name",
        "  Node 10,                                !- Fan Inlet Node Name",
        "  Node 2,                                 !- Fan Outlet Node Name",
        "  Node 8;                                 !- Setpoint Node or NodeList Name",

        "CoilSystem:Cooling:DX,",
        "  Coil Cooling DX Two Speed 1 CoilSystem, !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  Node 8,                                 !- DX Cooling Coil System Inlet Node Name",
        "  Node 9,                                 !- DX Cooling Coil System Outlet Node Name",
        "  Node 9,                                 !- DX Cooling Coil System Sensor Node Name",
        "  Coil:Cooling:DX:TwoSpeed,               !- Cooling Coil Object Type",
        "  Coil Cooling DX Two Speed 1;            !- Cooling Coil Name",

        "Coil:Cooling:DX:TwoSpeed,",
        "  Coil Cooling DX Two Speed 1,            !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  Autosize,                               !- High Speed Gross Rated Total Cooling Capacity {W}",
        "  Autosize,                               !- High Speed Rated Sensible Heat Ratio",
        "  3,                                      !- High Speed Gross Rated Cooling COP {W/W}",
        "  Autosize,                               !- High Speed Rated Air Flow Rate {m3/s}",
        "    ,",
        "    ,",
        "  773.3,                                  !- Unit Internal Static Air Pressure {Pa}",
        "  Node 8,                                 !- Air Inlet Node Name",
        "  Node 9,                                 !- Air Outlet Node Name",
        "  Curve Biquadratic 1,                    !- Total Cooling Capacity Function of Temperature Curve Name",
        "  Curve Quadratic 1,                      !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Curve Biquadratic 2,                    !- Energy Input Ratio Function of Temperature Curve Name",
        "  Curve Quadratic 2,                      !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Curve Quadratic 3,                      !- Part Load Fraction Correlation Curve Name",
        "  Autosize,                               !- Low Speed Gross Rated Total Cooling Capacity {W}",
        "  0.69,                                   !- Low Speed Gross Rated Sensible Heat Ratio",
        "  3,                                      !- Low Speed Gross Rated Cooling COP {W/W}",
        "  Autosize,                               !- Low Speed Rated Air Flow Rate {m3/s}",
        "    ,",
        "    ,",
        "  Curve Biquadratic 3,                    !- Low Speed Total Cooling Capacity Function of Temperature Curve Name",
        "  Curve Biquadratic 4,                    !- Low Speed Energy Input Ratio Function of Temperature Curve Name",
        "  ,                                       !- Condenser Air Inlet Node Name",
        "  AirCooled,                              !- Condenser Type",
        "  -25,                                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  0.9,                                    !- High Speed Evaporative Condenser Effectiveness {dimensionless}",
        "  Autosize,                               !- High Speed Evaporative Condenser Air Flow Rate {m3/s}",
        "  Autosize,                               !- High Speed Evaporative Condenser Pump Rated Power Consumption {W}",
        "  0.9,                                    !- Low Speed Evaporative Condenser Effectiveness {dimensionless}",
        "  Autosize,                               !- Low Speed Evaporative Condenser Air Flow Rate {m3/s}",
        "  Autosize,                               !- Low Speed Evaporative Condenser Pump Rated Power Consumption {W}",
        "  ,                                       !- Supply Water Storage Tank Name",
        "  ,                                       !- Condensate Collection Water Storage Tank Name",
        "  0,                                      !- Basin Heater Capacity {W/K}",
        "  2;                                      !- Basin Heater Setpoint Temperature {C}",

        "SetpointManager:MixedAir,",
        "  Node 9 OS Default SPM,                  !- Name",
        "  Temperature,                            !- Control Variable",
        "  Node 2,                                 !- Reference Setpoint Node Name",
        "  Node 10,                                !- Fan Inlet Node Name",
        "  Node 2,                                 !- Fan Outlet Node Name",
        "  Node 9;                                 !- Setpoint Node or NodeList Name",

        "Curve:Biquadratic,",
        "  Curve Biquadratic 1,                    !- Name",
        "  0.42415,                                !- Coefficient1 Constant",
        "  0.04426,                                !- Coefficient2 x",
        "  -0.00042,                               !- Coefficient3 x**2",
        "  0.00333,                                !- Coefficient4 y",
        "  -8e-05,                                 !- Coefficient5 y**2",
        "  -0.00021,                               !- Coefficient6 x*y",
        "  17,                                     !- Minimum Value of x {BasedOnField A2}",
        "  22,                                     !- Maximum Value of x {BasedOnField A2}",
        "  13,                                     !- Minimum Value of y {BasedOnField A3}",
        "  46;                                     !- Maximum Value of y {BasedOnField A3}",

        "Curve:Quadratic,",
        "  Curve Quadratic 1,                      !- Name",
        "  0.77136,                                !- Coefficient1 Constant",
        "  0.34053,                                !- Coefficient2 x",
        "  -0.11088,                               !- Coefficient3 x**2",
        "  0.75918,                                !- Minimum Value of x {BasedOnField A2}",
        "  1.13877;                                !- Maximum Value of x {BasedOnField A2}",

        "Curve:Biquadratic,",
        "  Curve Biquadratic 2,                    !- Name",
        "  1.23649,                                !- Coefficient1 Constant",
        "  -0.02431,                               !- Coefficient2 x",
        "  0.00057,                                !- Coefficient3 x**2",
        "  -0.01434,                               !- Coefficient4 y",
        "  0.00063,                                !- Coefficient5 y**2",
        "  -0.00038,                               !- Coefficient6 x*y",
        "  17,                                     !- Minimum Value of x {BasedOnField A2}",
        "  22,                                     !- Maximum Value of x {BasedOnField A2}",
        "  13,                                     !- Minimum Value of y {BasedOnField A3}",
        "  46;                                     !- Maximum Value of y {BasedOnField A3}",

        "Curve:Quadratic,",
        "  Curve Quadratic 2,                      !- Name",
        "  1.2055,                                 !- Coefficient1 Constant",
        "  -0.32953,                               !- Coefficient2 x",
        "  0.12308,                                !- Coefficient3 x**2",
        "  0.75918,                                !- Minimum Value of x {BasedOnField A2}",
        "  1.13877;                                !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  Curve Quadratic 3,                      !- Name",
        "  0.771,                                  !- Coefficient1 Constant",
        "  0.229,                                  !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Minimum Value of x {BasedOnField A2}",
        "  1;                                      !- Maximum Value of x {BasedOnField A2}",

        "Curve:Biquadratic,",
        "  Curve Biquadratic 3,                    !- Name",
        "  0.42415,                                !- Coefficient1 Constant",
        "  0.04426,                                !- Coefficient2 x",
        "  -0.00042,                               !- Coefficient3 x**2",
        "  0.00333,                                !- Coefficient4 y",
        "  -8e-05,                                 !- Coefficient5 y**2",
        "  -0.00021,                               !- Coefficient6 x*y",
        "  17,                                     !- Minimum Value of x {BasedOnField A2}",
        "  22,                                     !- Maximum Value of x {BasedOnField A2}",
        "  13,                                     !- Minimum Value of y {BasedOnField A3}",
        "  46;                                     !- Maximum Value of y {BasedOnField A3}",

        "Curve:Biquadratic,",
        "  Curve Biquadratic 4,                    !- Name",
        "  1.23649,                                !- Coefficient1 Constant",
        "  -0.02431,                               !- Coefficient2 x",
        "  0.00057,                                !- Coefficient3 x**2",
        "  -0.01434,                               !- Coefficient4 y",
        "  0.00063,                                !- Coefficient5 y**2",
        "  -0.00038,                               !- Coefficient6 x*y",
        "  17,                                     !- Minimum Value of x {BasedOnField A2}",
        "  22,                                     !- Maximum Value of x {BasedOnField A2}",
        "  13,                                     !- Minimum Value of y {BasedOnField A3}",
        "  46;                                     !- Maximum Value of y {BasedOnField A3}",

        "Coil:Heating:Electric,",
        "  Coil Heating Electric 1,                !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  1,                                      !- Efficiency",
        "  Autosize,                               !- Nominal Capacity {W}",
        "  Node 9,                                 !- Air Inlet Node Name",
        "  Node 10,                                !- Air Outlet Node Name",
        "  Node 10;                                !- Temperature Setpoint Node Name",

        "SetpointManager:MixedAir,",
        "  Node 10 OS Default SPM,                 !- Name",
        "  Temperature,                            !- Control Variable",
        "  Node 2,                                 !- Reference Setpoint Node Name",
        "  Node 10,                                !- Fan Inlet Node Name",
        "  Node 2,                                 !- Fan Outlet Node Name",
        "  Node 10;                                !- Setpoint Node or NodeList Name",

        "Fan:VariableVolume,",
        "  Fan Variable Volume 1,                  !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  0.6045,                                 !- Fan Total Efficiency",
        "  500,                                    !- Pressure Rise {Pa}",
        "  AutoSize,                               !- Maximum Flow Rate {m3/s}",
        "  FixedFlowRate,                          !- Fan Power Minimum Flow Rate Input Method",
        "  0,                                      !- Fan Power Minimum Flow Fraction",
        "  0,                                      !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.93,                                   !- Motor Efficiency",
        "  1,                                      !- Motor In Airstream Fraction",
        "  0.040759894,                            !- Fan Power Coefficient 1",
        "  0.08804497,                             !- Fan Power Coefficient 2",
        "  -0.07292612,                            !- Fan Power Coefficient 3",
        "  0.943739823,                            !- Fan Power Coefficient 4",
        "  0,                                      !- Fan Power Coefficient 5",
        "  Node 10,                                !- Air Inlet Node Name",
        "  Node 2,                                 !- Air Outlet Node Name",
        "  General;                                !- End-Use Subcategory",

        "SetpointManager:Scheduled,",
        "  Setpoint Manager Scheduled 1,           !- Name",
        "  Temperature,                            !- Control Variable",
        "  Deck_Temperature,                       !- Schedule Name",
        "  Node 2;                                 !- Setpoint Node or NodeList Name",

        "AirLoopHVAC:SupplyPath,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Node 3 Supply Path, !- Name",
        "  Node 3,                                 !- Supply Air Path Inlet Node Name",
        "  AirLoopHVAC:ZoneSplitter,               !- Component Object Type 1",
        "  Air Loop HVAC Zone Splitter 1,          !- Component Name 1",
        "  AirLoopHVAC:SupplyPlenum,               !- Component Object Type 2",
        "  Air Loop HVAC Supply Plenum 1;          !- Component Name 2",

        "AirLoopHVAC:ZoneSplitter,",
        "  Air Loop HVAC Zone Splitter 1,          !- Name",
        "  Node 3,                                 !- Inlet Node Name",
        "  Node 12;                                !- Outlet Node Name 1",

        "AirLoopHVAC:SupplyPlenum,",
        "  Air Loop HVAC Supply Plenum 1,          !- Name",
        "  SupplyPlenum,                           !- Zone Name",
        "  SupplyPlenum Zone Air Node,             !- Zone Node Name",
        "  Node 12,                                !- Inlet Node Name",
        "  SeriesPIU Supply Air Inlet Node;        !- Outlet Node Name 1",

        "AirLoopHVAC:ReturnPath,",
        "  Packaged Rooftop VAV with PFP Boxes and Reheat Return Path, !- Name",
        "  Node 4,                                 !- Return Air Path Outlet Node Name",
        "  AirLoopHVAC:ReturnPlenum,               !- Component Object Type 1",
        "  Air Loop HVAC Return Plenum 1,          !- Component Name 1",
        "  AirLoopHVAC:ZoneMixer,                  !- Component Object Type 2",
        "  Air Loop HVAC Zone Mixer 1;             !- Component Name 2",

        "AirLoopHVAC:ReturnPlenum,",
        "  Air Loop HVAC Return Plenum 1,          !- Name",
        "  ReturnPlenum,                           !- Zone Name",
        "  ReturnPlenum Zone Air Node,             !- Zone Node Name",
        "  Plenum Outlet Node,                     !- Outlet Node Name",
        "  SeriesPIU Secondary Air Inlet Node,     !- Induced Air Outlet Node or NodeList Name",
        "  Zone1 Return Air Node;                  !- Inlet Node Name 1",

        "AirLoopHVAC:ZoneMixer,",
        "  Air Loop HVAC Zone Mixer 1,             !- Name",
        "  Node 4,                                 !- Outlet Node Name",
        "  Plenum Outlet Node;                     !- Inlet Node Name 1",

        "Site:Location,",
        "  USA IL-CHICAGO-OHARE,                   !- Name",
        "  41.77,                                  !- Latitude {deg}",
        "  -87.75,                                 !- Longitude {deg}",
        "  -6.00,                                  !- Time Zone {hr}",
        "  190;                                    !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        "  Chicago Ohare Intl Ap Ann Clg .4% Condns DB=>MWB, !- Name",
        "  7,                                      !- Month",
        "  21,                                     !- Day of Month",
        "  SummerDesignDay,                        !- Day Type",
        "  33.3,                                   !- Maximum Dry-Bulb Temperature {C}",
        "  10.5,                                   !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  DefaultMultipliers,                     !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                                       !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                                !- Humidity Condition Type",
        "  23.7,                                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                                       !- Humidity Condition Day Schedule Name",
        "  ,                                       !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                                       !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  98934,                                  !- Barometric Pressure {Pa}",
        "  5.2,                                    !- Wind Speed {m/s}",
        "  230,                                    !- Wind Direction {deg}",
        "  No,                                     !- Rain Indicator",
        "  No,                                     !- Snow Indicator",
        "  No,                                     !- Daylight Saving Time Indicator",
        "  ASHRAETau,                              !- Solar Model Indicator",
        "  ,                                       !- Beam Solar Day Schedule Name",
        "  ,                                       !- Diffuse Solar Day Schedule Name",
        "  0.455,                                  !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  2.05,                                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  ,                                       !- Sky Clearness",
        "  ,                                       !- Maximum Number Warmup Days",
        "  FullResetAtBeginEnvironment;            !- Begin Environment Reset Mode",

        "SizingPeriod:DesignDay,",
        "  Chicago Ohare Intl Ap Ann Htg 99.6% Condns DB, !- Name",
        "  1,                                      !- Month",
        "  21,                                     !- Day of Month",
        "  WinterDesignDay,                        !- Day Type",
        "  -20,                                    !- Maximum Dry-Bulb Temperature {C}",
        "  0,                                      !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  DefaultMultipliers,                     !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                                       !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                                !- Humidity Condition Type",
        "  -20,                                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                                       !- Humidity Condition Day Schedule Name",
        "  ,                                       !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                                       !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  98934,                                  !- Barometric Pressure {Pa}",
        "  4.9,                                    !- Wind Speed {m/s}",
        "  270,                                    !- Wind Direction {deg}",
        "  No,                                     !- Rain Indicator",
        "  No,                                     !- Snow Indicator",
        "  No,                                     !- Daylight Saving Time Indicator",
        "  ASHRAEClearSky,                         !- Solar Model Indicator",
        "  ,                                       !- Beam Solar Day Schedule Name",
        "  ,                                       !- Diffuse Solar Day Schedule Name",
        "  ,                                       !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  ,                                       !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  0,                                      !- Sky Clearness",
        "  ,                                       !- Maximum Number Warmup Days",
        "  FullResetAtBeginEnvironment;            !- Begin Environment Reset Mode",

        "Site:GroundTemperature:BuildingSurface,19.527,19.502,19.536,19.598,20.002,21.640,22.225,22.375,21.449,20.121,19.802,19.633;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // What we're testing for here is an initialization order issue, and this is why we rely on calling a high-level function such as ManageSizing
    // and not lower level ones
    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);

    OutputReportPredefined::SetPredefinedTables(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state); // establish array bounds for constructions early
    // OutputProcessor::TimeValue.allocate(2);
    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);
    PlantManager::CheckIfAnyPlant(*state);
    EnergyPlus::createFacilityElectricPowerServiceObject(*state);
    BranchInputManager::ManageBranchInput(*state); // just gets input and returns.
    state->dataGlobal->DoingSizing = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->ZoneSizingCalc = true;
    EXPECT_FALSE(has_err_output(true));
    EXPECT_NO_THROW(SizingManager::ManageSizing(*state));

    std::string const expectedError = delimited_string({
        "   ************* Beginning Zone Sizing Calculations",
        "   ************* Beginning System Sizing Calculations",
    });
    EXPECT_TRUE(compare_err_stream(expectedError, true));
}

TEST_F(EnergyPlusFixture, VSParallelPIUStagedHeat)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                               !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                               !- Zone Name",
        "  SPACE2-1 Equipment,                     !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,                       !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,                  !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,                      !- Zone Air Node Name",
        "  SPACE2-1 Return Node;                   !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                     !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,                           !- Zone Equipment 1 Name",
        "  1,                                      !- Zone Equipment 1 Cooling Sequence",
        "  1;                                      !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,                           !- Name",
        "  SPACE2-1 In Node,                       !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:ParallelPIU:Reheat,  !- Air Terminal Object Type",
        "  SPACE2-1 Parallel PIU Reheat;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:ParallelPIU:Reheat,",
        "  SPACE2-1 Parallel PIU Reheat,           !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  0.1,                                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.05,                                   !- Maximum Secondary Air Flow Rate {m3/s}",
        "  0.2,                                    !- Minimum Primary Air Flow Fraction",
        "  0.1,                                    !- Fan On Flow Fraction",
        "  SPACE2-1 ATU In Node,                   !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,                  !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,                       !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,                     !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,                       !- Fan Name",
        "  Coil:Heating:Electric,                  !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,                     !- Reheat Coil Name",
        "  0.0,                                    !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                                    !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001,                                 !- Convergence Tolerance",
        "  VariableSpeed,                          !- Fan Control Type",
        "  0.3,                                    !- Minimum Fan Turn Down Ratio",
        "  Staged;                                 !- Heating Control Type",

        "Fan:SystemModel,",
        "  SPACE2-1 PIU Fan,                       !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  SPACE2-1 ATU Sec Node,                  !- Air Inlet Node Name",
        "  SPACE2-1 ATU Fan Outlet Node,           !- Air Outlet Node Name",
        "  0.05,                               !- Design Maximum Air Flow Rate {m3/s}",
        "  Continuous,                             !- Speed Control Method",
        "  0.0,                                    !- Electric Power Minimum Flow Rate Fraction",
        "  50.0,                                   !- Design Pressure Rise {Pa}",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Air Stream Fraction",
        "  AUTOSIZE,                               !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,             !- Design Power Sizing Method",
        "  ,                                       !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                                       !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.50,                                   !- Fan Total Efficiency",
        "  CombinedPowerAndFanEff;                 !- Electric Power Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    CombinedPowerAndFanEff,  !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    0.027411,                !- Coefficient2 x",
        "    0.008740,                !- Coefficient3 x**2",
        "    0.969563,                !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,                     !- Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,                   !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Fan Outlet Node;           !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,                     !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  1.0,                                    !- Efficiency",
        "  1000,                                   !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Inlet Node Name",
        "  SPACE2-1 In Node;                       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  AlwaysOn,                               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);
    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    bool FirstHVACIteration = true;
    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    auto &thisPIU = state->dataPowerInductionUnits->PIU(1);
    state->dataFans->fans(thisPIU.Fan_Index)->simulate(*state, FirstHVACIteration, _, _);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    int SecNodeNum = thisPIU.SecAirInNode;
    int PriNodeNum = thisPIU.PriAirInNode;
    Real64 PriMaxMassFlow = thisPIU.MaxPriAirMassFlow;
    Real64 PriMinMassFlow = thisPIU.MinPriAirMassFlow;

    // first stage heating, expects:
    // - reheat: no
    // - primary air flow rate: minimum value
    // - secondary air flow rate: modulating between minimum and maximum value
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 500.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNodeNum) = false;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 26.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 16.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMaxAvail = thisPIU.MaxSecAirMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMinAvail = thisPIU.MinSecAirMassFlow;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataLoopNodes->Node(7).MassFlowRateMax = thisPIU.MaxSecAirMassFlow; // Fan node
    state->dataLoopNodes->Node(7).MassFlowRateMin = thisPIU.MinSecAirMassFlow;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::StagedHeatFirstStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_GT(thisPIU.SecMassFlowRate, thisPIU.MinSecAirMassFlow);
    EXPECT_EQ(thisPIU.DischargeAirTemp, state->dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp); // no reheat

    // second stage heating, expects:
    // - reheat: yes
    // - primary air flow rate: minimum value
    // - secondary air flow rate: maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 1000.0;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::StagedHeatSecondStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_EQ(thisPIU.SecMassFlowRate, thisPIU.MaxSecAirMassFlow);
}

TEST_F(EnergyPlusFixture, VSParallelPIUModulatedHeat)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                               !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                               !- Zone Name",
        "  SPACE2-1 Equipment,                     !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,                       !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,                  !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,                      !- Zone Air Node Name",
        "  SPACE2-1 Return Node;                   !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                     !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,                           !- Zone Equipment 1 Name",
        "  1,                                      !- Zone Equipment 1 Cooling Sequence",
        "  1;                                      !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,                           !- Name",
        "  SPACE2-1 In Node,                       !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:ParallelPIU:Reheat,  !- Air Terminal Object Type",
        "  SPACE2-1 Parallel PIU Reheat;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:ParallelPIU:Reheat,",
        "  SPACE2-1 Parallel PIU Reheat,           !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  0.1,                                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.05,                                   !- Maximum Secondary Air Flow Rate {m3/s}",
        "  0.2,                                    !- Minimum Primary Air Flow Fraction",
        "  0.1,                                    !- Fan On Flow Fraction",
        "  SPACE2-1 ATU In Node,                   !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,                  !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,                       !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,                     !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,                       !- Fan Name",
        "  Coil:Heating:Electric,                  !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,                     !- Reheat Coil Name",
        "  0.0,                                    !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                                    !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001,                                 !- Convergence Tolerance",
        "  VariableSpeed,                          !- Fan Control Type",
        "  0.3,                                    !- Minimum Fan Turn Down Ratio",
        "  Modulated;                              !- Heating Control Type",

        "Fan:SystemModel,",
        "  SPACE2-1 PIU Fan,                       !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  SPACE2-1 ATU Sec Node,                  !- Air Inlet Node Name",
        "  SPACE2-1 ATU Fan Outlet Node,           !- Air Outlet Node Name",
        "  0.05,                               !- Design Maximum Air Flow Rate {m3/s}",
        "  Continuous,                             !- Speed Control Method",
        "  0.0,                                    !- Electric Power Minimum Flow Rate Fraction",
        "  50.0,                                   !- Design Pressure Rise {Pa}",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Air Stream Fraction",
        "  AUTOSIZE,                               !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,             !- Design Power Sizing Method",
        "  ,                                       !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                                       !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.50,                                   !- Fan Total Efficiency",
        "  CombinedPowerAndFanEff;                 !- Electric Power Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    CombinedPowerAndFanEff,  !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    0.027411,                !- Coefficient2 x",
        "    0.008740,                !- Coefficient3 x**2",
        "    0.969563,                !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,                     !- Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,                   !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Fan Outlet Node;           !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,                     !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  1.0,                                    !- Efficiency",
        "  2500,                                   !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Inlet Node Name",
        "  SPACE2-1 In Node;                       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  AlwaysOn,                               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);
    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    bool FirstHVACIteration = true;
    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    auto &thisPIU = state->dataPowerInductionUnits->PIU(1);
    state->dataFans->fans(thisPIU.Fan_Index)->simulate(*state, false, _, _);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    int SecNodeNum = thisPIU.SecAirInNode;
    int PriNodeNum = thisPIU.PriAirInNode;
    Real64 PriMaxMassFlow = thisPIU.MaxPriAirMassFlow;
    Real64 PriMinMassFlow = thisPIU.MinPriAirMassFlow;

    // first stage heating, expects:
    // - reheat: yes
    // - discharge air temperature: DAT below design heating DAT
    // - primary air flow rate: minimum value
    // - secondary air flow rate: minmum value
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 500.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNodeNum) = false;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 26.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 16.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMaxAvail = thisPIU.MaxSecAirMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMinAvail = thisPIU.MinSecAirMassFlow;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataLoopNodes->Node(7).MassFlowRateMax = thisPIU.MaxSecAirMassFlow; // Fan node
    state->dataLoopNodes->Node(7).MassFlowRateMin = thisPIU.MinSecAirMassFlow;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatFirstStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_EQ(thisPIU.SecMassFlowRate, thisPIU.MinSecAirMassFlow);
    EXPECT_LT(thisPIU.DischargeAirTemp, thisPIU.designHeatingDAT);

    // second stage heating, expects:
    // - reheat: yes
    // - discharge air temperature: design heating DAT
    // - primary air flow rate: minimum value
    // - secondary air flow rate: modulating between minimum and maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 1000.0;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatSecondStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_LT(thisPIU.SecMassFlowRate, thisPIU.MaxSecAirMassFlow);
    EXPECT_GT(thisPIU.SecMassFlowRate, thisPIU.MinSecAirMassFlow);
    EXPECT_NEAR(thisPIU.DischargeAirTemp, thisPIU.designHeatingDAT, 0.0001);

    // third stage heating, expects:
    // - reheat: yes
    // - discharge air temperature: modulating between design heating DAT and high limit DAT
    // - primary air flow rate: minimum value
    // - secondary air flow rate: maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 1500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 1500.0;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatThirdStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_EQ(thisPIU.SecMassFlowRate, thisPIU.MaxSecAirMassFlow);
    EXPECT_LT(thisPIU.DischargeAirTemp, thisPIU.highLimitDAT);
    EXPECT_GT(thisPIU.DischargeAirTemp, thisPIU.designHeatingDAT);

    // check that high limit DAT is not exceeded
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 16.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMaxAvail = thisPIU.MaxSecAirMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMinAvail = thisPIU.MinSecAirMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 2000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 2000.0;
    PoweredInductionUnits::CalcParallelPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatThirdStage);
    EXPECT_NEAR(thisPIU.DischargeAirTemp, thisPIU.highLimitDAT, 0.0001);
}

TEST_F(EnergyPlusFixture, VSSeriesPIUStagedHeat)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                               !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                               !- Zone Name",
        "  SPACE2-1 Equipment,                     !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,                       !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,                  !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,                      !- Zone Air Node Name",
        "  SPACE2-1 Return Node;                   !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                     !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,                           !- Zone Equipment 1 Name",
        "  1,                                      !- Zone Equipment 1 Cooling Sequence",
        "  1;                                      !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,                           !- Name",
        "  SPACE2-1 In Node,                       !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:SeriesPIU:Reheat,  !- Air Terminal Object Type",
        "  SPACE2-1 Series PIU Reheat;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
        "  SPACE2-1 Series PIU Reheat,              !- Name",
        "  AlwaysOn,                                !- Availability Schedule Name",
        "  0.15,                                    !- Maximum Air Flow Rate {m3/s}",
        "  0.05,                                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.2,                                     !- Minimum Primary Air Flow Fraction",
        "  SPACE2-1 ATU In Node,                    !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,                   !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,                        !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,          !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,                      !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,                        !- Fan Name",
        "  Coil:Heating:Electric,                   !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,                      !- Reheat Coil Name",
        "  0.0,                                     !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                                     !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001,                                  !- Convergence Tolerance",
        "  VariableSpeed,                           !- Fan Control Type",
        "  0.3,                                     !- Minimum Fan Turn Down Ratio",
        "  Staged;                                  !- Heating Control Type",

        "Fan:SystemModel,",
        "  SPACE2-1 PIU Fan,                       !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  SPACE2-1 ATU Fan Inlet Node,            !- Air Inlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Outlet Node Name",
        "  0.15,                                   !- Design Maximum Air Flow Rate {m3/s}",
        "  Continuous,                             !- Speed Control Method",
        "  0.0,                                    !- Electric Power Minimum Flow Rate Fraction",
        "  50.0,                                   !- Design Pressure Rise {Pa}",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Air Stream Fraction",
        "  AUTOSIZE,                               !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,             !- Design Power Sizing Method",
        "  ,                                       !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                                       !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.50,                                   !- Fan Total Efficiency",
        "  CombinedPowerAndFanEff;                 !- Electric Power Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    CombinedPowerAndFanEff,  !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    0.027411,                !- Coefficient2 x",
        "    0.008740,                !- Coefficient3 x**2",
        "    0.969563,                !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,      !- Name",
        "  SPACE2-1 ATU Fan Inlet Node,  !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,    !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Sec Node;  !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,                     !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  1.0,                                    !- Efficiency",
        "  2500,                                   !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Inlet Node Name",
        "  SPACE2-1 In Node;                       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  AlwaysOn,                               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    state->dataFans->GetFanInputFlag = false;
    Fans::GetFanInput(*state);
    PoweredInductionUnits::GetPIUs(*state);
    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    bool FirstHVACIteration = true;
    // Real64 SecMaxMassFlow = 0.05 * state->dataEnvrn->StdRhoAir;
    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    MixerComponent::GetMixerInput(*state);
    auto &thisPIU = state->dataPowerInductionUnits->PIU(1);
    state->dataFans->fans(thisPIU.Fan_Index)->simulate(*state, false, _, _);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    int SecNodeNum = thisPIU.SecAirInNode;
    int PriNodeNum = thisPIU.PriAirInNode;
    Real64 PriMaxMassFlow = thisPIU.MaxPriAirMassFlow;
    Real64 PriMinMassFlow = thisPIU.MinPriAirMassFlow;

    // first stage heating, expects:
    // - reheat: no
    // - primary air flow rate: minimum value
    // - secondary air flow rate: modulating between minimum and maximum value
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 500.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNodeNum) = false;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 26.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 16.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMaxAvail = thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMinAvail = thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataLoopNodes->Node(7).MassFlowRateMax = thisPIU.MaxTotAirMassFlow; // Fan node
    state->dataLoopNodes->Node(7).MassFlowRateMin = thisPIU.MinTotAirMassFlow;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::StagedHeatFirstStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_LT(thisPIU.SecMassFlowRate, thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow);
    EXPECT_GT(thisPIU.SecMassFlowRate, thisPIU.MinSecAirMassFlow);
    EXPECT_EQ(thisPIU.DischargeAirTemp, state->dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp); // no reheat

    // second stage heating, expects:
    // - reheat: yes
    // - primary air flow rate: minimum value
    // - secondary air flow rate: maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 2500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 2500.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::StagedHeatSecondStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_EQ(thisPIU.SecMassFlowRate, thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow);
    EXPECT_GT(thisPIU.DischargeAirTemp, state->dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp); // reheat
    EXPECT_LT(thisPIU.DischargeAirTemp, thisPIU.highLimitDAT);
}

TEST_F(EnergyPlusFixture, VSSeriesPIUModulatedHeat)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                               !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                               !- Zone Name",
        "  SPACE2-1 Equipment,                     !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,                       !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,                  !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,                      !- Zone Air Node Name",
        "  SPACE2-1 Return Node;                   !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                     !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,                           !- Zone Equipment 1 Name",
        "  1,                                      !- Zone Equipment 1 Cooling Sequence",
        "  1;                                      !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,                           !- Name",
        "  SPACE2-1 In Node,                       !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:SeriesPIU:Reheat,!- Air Terminal Object Type",
        "  SPACE2-1 Series PIU Reheat;             !- Air Terminal Name",

        "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
        "  SPACE2-1 Series PIU Reheat,              !- Name",
        "  AlwaysOn,                                !- Availability Schedule Name",
        "  0.15,                                    !- Maximum Air Flow Rate {m3/s}",
        "  0.05,                                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.2,                                     !- Minimum Primary Air Flow Fraction",
        "  SPACE2-1 ATU In Node,                    !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,                   !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,                        !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,          !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,                      !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,                        !- Fan Name",
        "  Coil:Heating:Electric,                   !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,                      !- Reheat Coil Name",
        "  0.0,                                     !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                                     !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001,                                  !- Convergence Tolerance",
        "  VariableSpeed,                           !- Fan Control Type",
        "  0.3,                                     !- Minimum Fan Turn Down Ratio",
        "  Modulated;                               !- Heating Control Type",

        "Fan:SystemModel,",
        "  SPACE2-1 PIU Fan,                       !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  SPACE2-1 ATU Fan Inlet Node,            !- Air Inlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Outlet Node Name",
        "  0.15,                                   !- Design Maximum Air Flow Rate {m3/s}",
        "  Continuous,                             !- Speed Control Method",
        "  0.0,                                    !- Electric Power Minimum Flow Rate Fraction",
        "  50.0,                                   !- Design Pressure Rise {Pa}",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Air Stream Fraction",
        "  AUTOSIZE,                               !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,             !- Design Power Sizing Method",
        "  ,                                       !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                                       !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.50,                                   !- Fan Total Efficiency",
        "  CombinedPowerAndFanEff;                 !- Electric Power Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    CombinedPowerAndFanEff,  !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    0.027411,                !- Coefficient2 x",
        "    0.008740,                !- Coefficient3 x**2",
        "    0.969563,                !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,           !- Name",
        "  SPACE2-1 ATU Fan Inlet Node,  !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,         !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Sec Node;        !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,                     !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  1.0,                                    !- Efficiency",
        "  5000,                                   !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Inlet Node Name",
        "  SPACE2-1 In Node;                       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  AlwaysOn,                               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);
    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    bool FirstHVACIteration = true;
    // Real64 SecMaxMassFlow = 0.05 * state->dataEnvrn->StdRhoAir;
    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    MixerComponent::GetMixerInput(*state);
    auto &thisPIU = state->dataPowerInductionUnits->PIU(1);
    state->dataFans->fans(thisPIU.Fan_Index)->simulate(*state, false, _, _);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    int SecNodeNum = thisPIU.SecAirInNode;
    int PriNodeNum = thisPIU.PriAirInNode;
    Real64 PriMaxMassFlow = thisPIU.MaxPriAirMassFlow;
    Real64 PriMinMassFlow = thisPIU.MinPriAirMassFlow;

    // first stage heating, expects:
    // - reheat: yes
    // - discharge air temperature: DAT below design heating DAT
    // - primary air flow rate: minimum value
    // - secondary air flow rate: minmum value
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 500.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNodeNum) = false;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 16.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMaxAvail = thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMinAvail = thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataLoopNodes->Node(7).MassFlowRateMax = thisPIU.MaxTotAirMassFlow; // Fan node
    state->dataLoopNodes->Node(7).MassFlowRateMin = thisPIU.MinTotAirMassFlow;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatFirstStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_EQ(thisPIU.SecMassFlowRate, thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow);
    EXPECT_LT(thisPIU.DischargeAirTemp, thisPIU.designHeatingDAT);

    // second stage heating, expects:
    // - reheat: yes
    // - discharge air temperature: design heating DAT
    // - primary air flow rate: minimum value
    // - secondary air flow rate: modulating between minimum and maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 1500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 1500.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatSecondStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_LT(thisPIU.SecMassFlowRate, thisPIU.MaxSecAirMassFlow);
    EXPECT_GT(thisPIU.SecMassFlowRate, thisPIU.MinSecAirMassFlow);
    EXPECT_NEAR(thisPIU.DischargeAirTemp, thisPIU.designHeatingDAT, 0.0001);

    // third stage heating, expects:
    // - reheat: yes
    // - discharge air temperature: modulating between design heating DAT and high limit DAT
    // - primary air flow rate: minimum value
    // - secondary air flow rate: maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 3300.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 3300.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatThirdStage);
    EXPECT_EQ(thisPIU.PriMassFlowRate, PriMinMassFlow);
    EXPECT_EQ(state->dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate, thisPIU.MaxTotAirMassFlow);
    EXPECT_LT(thisPIU.DischargeAirTemp, thisPIU.highLimitDAT);
    EXPECT_GT(thisPIU.DischargeAirTemp, thisPIU.designHeatingDAT);

    // check that high limit DAT is not exceeded
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 16.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = 4800.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToHeatSP = 4800.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.heatingOperatingMode, PoweredInductionUnits::HeatOpModeType::ModulatedHeatThirdStage);
    EXPECT_NEAR(thisPIU.DischargeAirTemp, thisPIU.highLimitDAT, 0.0001);
}

TEST_F(EnergyPlusFixture, VSSeriesPIUCool)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  SPACE2-1;                               !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                               !- Zone Name",
        "  SPACE2-1 Equipment,                     !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 In Node,                       !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 ATU Sec Node,                  !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Air Node,                      !- Zone Air Node Name",
        "  SPACE2-1 Return Node;                   !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                     !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment 1 Object Type",
        "  SPACE2-1 ADU,                           !- Zone Equipment 1 Name",
        "  1,                                      !- Zone Equipment 1 Cooling Sequence",
        "  1;                                      !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  SPACE2-1 ADU,                           !- Name",
        "  SPACE2-1 In Node,                       !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:SeriesPIU:Reheat,!- Air Terminal Object Type",
        "  SPACE2-1 Series PIU Reheat;             !- Air Terminal Name",

        "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
        "  SPACE2-1 Series PIU Reheat,              !- Name",
        "  AlwaysOn,                                !- Availability Schedule Name",
        "  0.15,                                    !- Maximum Air Flow Rate {m3/s}",
        "  0.05,                                    !- Maximum Primary Air Flow Rate {m3/s}",
        "  0.2,                                     !- Minimum Primary Air Flow Fraction",
        "  SPACE2-1 ATU In Node,                    !- Supply Air Inlet Node Name",
        "  SPACE2-1 ATU Sec Node,                   !- Secondary Air Inlet Node Name",
        "  SPACE2-1 In Node,                        !- Outlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,          !- Reheat Coil Air Inlet Node Name",
        "  SPACE2-1 PIU Mixer,                      !- Zone Mixer Name",
        "  SPACE2-1 PIU Fan,                        !- Fan Name",
        "  Coil:Heating:Electric,                   !- Reheat Coil Object Type",
        "  SPACE2-1 Zone Coil,                      !- Reheat Coil Name",
        "  0.0,                                     !- Maximum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0,                                     !- Minimum Hot Water or Steam Flow Rate {m3/s}",
        "  0.0001,                                  !- Convergence Tolerance",
        "  VariableSpeed,                           !- Fan Control Type",
        "  0.3,                                     !- Minimum Fan Turn Down Ratio",
        "  Modulated;                               !- Heating Control Type",

        "Fan:SystemModel,",
        "  SPACE2-1 PIU Fan,                       !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  SPACE2-1 ATU Fan Inlet Node,            !- Air Inlet Node Name",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Outlet Node Name",
        "  0.15,                                   !- Design Maximum Air Flow Rate {m3/s}",
        "  Continuous,                             !- Speed Control Method",
        "  0.0,                                    !- Electric Power Minimum Flow Rate Fraction",
        "  50.0,                                   !- Design Pressure Rise {Pa}",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Air Stream Fraction",
        "  AUTOSIZE,                               !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,             !- Design Power Sizing Method",
        "  ,                                       !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                                       !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.50,                                   !- Fan Total Efficiency",
        "  CombinedPowerAndFanEff;                 !- Electric Power Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    CombinedPowerAndFanEff,  !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    0.027411,                !- Coefficient2 x",
        "    0.008740,                !- Coefficient3 x**2",
        "    0.969563,                !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "AirLoopHVAC:ZoneMixer,",
        "  SPACE2-1 PIU Mixer,           !- Name",
        "  SPACE2-1 ATU Fan Inlet Node,  !- Outlet Node Name",
        "  SPACE2-1 ATU In Node,         !- Inlet 1 Node Name",
        "  SPACE2-1 ATU Sec Node;        !- Inlet 2 Node Name",

        "Coil:Heating:Electric,",
        "  SPACE2-1 Zone Coil,                     !- Name",
        "  AlwaysOn,                               !- Availability Schedule Name",
        "  1.0,                                    !- Efficiency",
        "  5000,                                   !- Nominal Capacity",
        "  SPACE2-1 Zone Coil Air In Node,         !- Air Inlet Node Name",
        "  SPACE2-1 In Node;                       !- Air Outlet Node Name",

        "Schedule:Constant,",
        "  AlwaysOff,                              !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  0;                                      !- Hourly Value",

        "Schedule:Constant,",
        "  AlwaysOn,                               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));
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
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    DataZoneEquipment::GetZoneEquipmentData(*state);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    state->dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
    Fans::GetFanInput(*state);
    state->dataFans->GetFanInputFlag = false;
    PoweredInductionUnits::GetPIUs(*state);
    std::string error_string = delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSOFF",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated.",
                                                 "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYSON",
                                                 "   **   ~~~   ** Schedule Type Limits Name is empty.",
                                                 "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string));
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);

    // Setup
    int ZoneNum = 1;
    int SysNum = 1;
    int ZoneNodeNum = 1;
    bool FirstHVACIteration = true;
    // Real64 SecMaxMassFlow = 0.05 * state->dataEnvrn->StdRhoAir;
    state->dataGlobal->BeginEnvrnFlag = true; // Must be true for initial pass thru InitPIU for this terminal unit
    FirstHVACIteration = true;
    PoweredInductionUnits::InitPIU(*state, SysNum, FirstHVACIteration); // Run thru init once with FirstHVACIteration set to true
    MixerComponent::GetMixerInput(*state);
    auto &thisPIU = state->dataPowerInductionUnits->PIU(1);
    state->dataFans->fans(thisPIU.Fan_Index)->simulate(*state, false, _, _);
    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    int SecNodeNum = thisPIU.SecAirInNode;
    int PriNodeNum = thisPIU.PriAirInNode;
    Real64 PriMaxMassFlow = thisPIU.MaxPriAirMassFlow;
    Real64 PriMinMassFlow = thisPIU.MinPriAirMassFlow;

    // first stage cooling, expects:
    // - total flow rate: modulating between minimum and maxium value
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRate = PriMinMassFlow;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = -400.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToCoolSP = -400.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNodeNum) = false;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 19.0;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataLoopNodes->Node(ZoneNodeNum).HumRat);
    state->dataLoopNodes->Node(SecNodeNum).Temp = 19.0;
    state->dataLoopNodes->Node(SecNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(SecNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(SecNodeNum).Temp, state->dataLoopNodes->Node(SecNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).Temp = 12.0;
    state->dataLoopNodes->Node(PriNodeNum).HumRat = 0.0085;
    state->dataLoopNodes->Node(PriNodeNum).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(PriNodeNum).Temp, state->dataLoopNodes->Node(PriNodeNum).HumRat);
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMaxAvail = PriMaxMassFlow;
    state->dataLoopNodes->Node(PriNodeNum).MassFlowRateMinAvail = PriMinMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMaxAvail = thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow;
    state->dataLoopNodes->Node(SecNodeNum).MassFlowRateMinAvail = thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataLoopNodes->Node(7).MassFlowRateMax = thisPIU.MaxTotAirMassFlow; // Fan node
    state->dataLoopNodes->Node(7).MassFlowRateMin = thisPIU.MinTotAirMassFlow;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.coolingOperatingMode, PoweredInductionUnits::CoolOpModeType::CoolFirstStage);
    EXPECT_LT(state->dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate, thisPIU.MaxTotAirMassFlow);

    // second stage cooling, expects:
    // - total flow rate: maximum value
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputRequired = -800.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNodeNum).RemainingOutputReqToCoolSP = -800.0;
    PoweredInductionUnits::CalcSeriesPIU(*state, SysNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
    EXPECT_ENUM_EQ(thisPIU.coolingOperatingMode, PoweredInductionUnits::CoolOpModeType::CoolSecondStage);
    EXPECT_EQ(state->dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate, thisPIU.MaxTotAirMassFlow);
}

TEST_F(EnergyPlusFixture, PIU_reportTerminalUnit)
{
    using namespace EnergyPlus::OutputReportPredefined;
    auto &orp = *state->dataOutRptPredefined;

    SetPredefinedTables(*state);

    Sched::AddScheduleConstant(*state, "SCHA");
    Sched::AddScheduleConstant(*state, "SCHB");

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

    auto &piu = state->dataPowerInductionUnits->PIU;
    piu.allocate(2);
    piu(1).ADUNum = 1;
    piu(1).UnitType = "AirTerminal:SingleDuct:SeriesPIU:Reheat";
    piu(1).MaxPriAirVolFlow = 0.30;
    piu(1).MaxSecAirVolFlow = 0.25;
    piu(1).HCoilType = PoweredInductionUnits::HtgCoilType::Electric;
    piu(1).fanType = HVAC::FanType::Constant;
    piu(1).FanName = "FanA";

    piu(1).reportTerminalUnit(*state);

    EXPECT_EQ("0.15", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlow, "ADU a"));
    EXPECT_EQ("0.05", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOutdoorFlow, "ADU a"));
    EXPECT_EQ("12.50", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupCoolingSP, "ADU a"));
    EXPECT_EQ("40.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupHeatingSP, "ADU a"));
    EXPECT_EQ("2000.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatingCap, "ADU a"));
    EXPECT_EQ("3000.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolingCap, "ADU a"));
    EXPECT_EQ("AirTerminal:SingleDuct:SeriesPIU:Reheat", RetrievePreDefTableEntry(*state, orp.pdchAirTermTypeInp, "ADU a"));
    EXPECT_EQ("0.30", RetrievePreDefTableEntry(*state, orp.pdchAirTermPrimFlow, "ADU a"));
    EXPECT_EQ("0.25", RetrievePreDefTableEntry(*state, orp.pdchAirTermSecdFlow, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlowSch, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMaxFlowReh, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOAflowSch, "ADU a"));
    EXPECT_EQ("COIL:HEATING:ELECTRIC", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatCoilType, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolCoilType, "ADU a"));
    EXPECT_EQ("Fan:ConstantVolume", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanType, "ADU a"));
    EXPECT_EQ("FanA", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanName, "ADU a"));
}
