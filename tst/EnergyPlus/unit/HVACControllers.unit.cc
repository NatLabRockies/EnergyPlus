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

// EnergyPlus::HVACControllers Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/WaterCoils.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus::MixedAir;
using namespace EnergyPlus::HVACControllers;
using namespace EnergyPlus::SetPointManager;
using namespace EnergyPlus::WaterCoils;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HVACControllers_ResetHumidityRatioCtrlVarType)
{
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:Water,",
        "	Chilled Water Coil,	!- Name",
        "	AvailSched,			!- Availability Schedule Name",
        "	autosize,			!- Design Water Flow Rate { m3 / s }",
        "	autosize,			!- Design Air Flow Rate { m3 / s }",
        "	autosize,			!- Design Inlet Water Temperature { C }",
        "	autosize,			!- Design Inlet Air Temperature { C }",
        "	autosize,			!- Design Outlet Air Temperature { C }",
        "	autosize,			!- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	autosize,			!- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Water Inlet Node,	!- Water Inlet Node Name",
        "	Water Outlet Node,  !- Water Outlet Node Name",
        "	Air Inlet Node,		!- Air Inlet Node Name",
        "	Air Outlet Node,	!- Air Outlet Node Name",
        "	SimpleAnalysis,		!- Type of Analysis",
        "	CrossFlow;          !- Heat Exchanger Configuration",
        " Controller:WaterCoil,",
        "	CW Coil Controller, !- Name",
        "	HumidityRatio,		!- Control Variable",
        "	Reverse,			!- Action",
        "	FLOW,				!- Actuator Variable",
        "	Air Outlet Node,	!- Sensor Node Name",
        "	Water Inlet Node,	!- Actuator Node Name",
        "	autosize,			!- Controller Convergence Tolerance { deltaC }",
        "	autosize,			!- Maximum Actuated Flow { m3 / s }",
        "	0.0;				!- Minimum Actuated Flow { m3 / s }",
        " SetpointManager:Scheduled,",
        "	HumRatSPManager,	!- Name",
        "	HumidityRatio,		!- Control Variable",
        "	HumRatioSched,		!- Schedule Name",
        "	Air Outlet Node;	!- Setpoint Node or NodeList Name",
        " Schedule:Compact,",
        "   HumRatioSched,		!- Name",
        "	Any Number,			!- Schedule Type Limits Name",
        "	Through: 12/31,		!- Field 1",
        "	For: AllDays,		!- Field 2",
        "	Until: 24:00, 0.015; !- Field 3",
        " Schedule:Compact,",
        "   AvailSched,			!- Name",
        "	Fraction,			!- Schedule Type Limits Name",
        "	Through: 12/31,		!- Field 1",
        "	For: AllDays,		!- Field 2",
        "	Until: 24:00, 1.0;  !- Field 3",
        " AirLoopHVAC:ControllerList,",
        "	CW Coil Controller, !- Name",
        "	Controller:WaterCoil, !- Controller 1 Object Type",
        "	CW Coil Controller; !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetSetPointManagerInputs(*state);
    // check specified control variable type is "HumidityRatio"
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::HumRat, state->dataSetPointManager->spms(1)->ctrlVar);

    GetControllerInput(*state);
    // check control variable type in AllSetPtMgr is reset to "MaximumHumidityRatio"
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::MaxHumRat, state->dataSetPointManager->spms(1)->ctrlVar);

    // ControllerProps always expects the control variable type to be "HumidityRatio"
    state->dataHVACControllers->ControllerProps(1).HumRatCntrlType =
        GetHumidityRatioVariableType(*state, state->dataHVACControllers->ControllerProps(1).SensedNode);

    // There is a mismatch here between SchSetPtMgr(1).ctrlVar and what was previous AllSetPtMgr(1).ctrlVar but which no longer exists
    // Is this a spurious test or is that difference a "feature"?
    // ASSERT_ENUM_EQ(HVAC::CtrlVarType::HumRat, state->dataHVACControllers->ControllerProps(1).HumRatCntrlType);

    ASSERT_EQ(state->dataHVACControllers->ControllerProps.size(), 1u);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MaxVolFlowActuated, DataSizing::AutoSize);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).Offset, DataSizing::AutoSize);
}

TEST_F(EnergyPlusFixture, HVACControllers_TestTempAndHumidityRatioCtrlVarType)
{
    std::string const idf_objects = delimited_string({

        " Coil:Cooling:Water,",
        "	Chilled Water Coil,	!- Name",
        "	AvailSched,			!- Availability Schedule Name",
        "	0.01,				!- Design Water Flow Rate { m3 / s }",
        "	1.0,				!- Design Air Flow Rate { m3 / s }",
        "	7.2,				!- Design Inlet Water Temperature { C }",
        "	32.0,				!- Design Inlet Air Temperature { C }",
        "	12.0,				!- Design Outlet Air Temperature { C }",
        "	0.01,				!- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.07,				!- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Water Inlet Node,	!- Water Inlet Node Name",
        "	Water Outlet Node,  !- Water Outlet Node Name",
        "	Air Inlet Node,		!- Air Inlet Node Name",
        "	Air Outlet Node,	!- Air Outlet Node Name",
        "	SimpleAnalysis,		!- Type of Analysis",
        "	CrossFlow;          !- Heat Exchanger Configuration",
        " Controller:WaterCoil,",
        "	CW Coil Controller, !- Name",
        "	TemperatureAndHumidityRatio,		!- Control Variable",
        "	Reverse,			!- Action",
        "	FLOW,				!- Actuator Variable",
        "	Air Outlet Node,	!- Sensor Node Name",
        "	Water Inlet Node,	!- Actuator Node Name",
        "	0.001,				!- Controller Convergence Tolerance { deltaC }",
        "	0.01,				!- Maximum Actuated Flow { m3 / s }",
        "	0.0;				!- Minimum Actuated Flow { m3 / s }",
        " SetpointManager:Scheduled,",
        "	HumRatSPManager,	!- Name",
        "	MaximumHumidityRatio,  !- Control Variable",
        "	HumRatioSched,		!- Schedule Name",
        "	Air Outlet Node;	!- Setpoint Node or NodeList Name",
        " Schedule:Compact,",
        "   HumRatioSched,		!- Name",
        "	Any Number,			!- Schedule Type Limits Name",
        "	Through: 12/31,		!- Field 1",
        "	For: AllDays,		!- Field 2",
        "	Until: 24:00, 0.015; !- Field 3",
        " Schedule:Compact,",
        "   AvailSched,			!- Name",
        "	Fraction,			!- Schedule Type Limits Name",
        "	Through: 12/31,		!- Field 1",
        "	For: AllDays,		!- Field 2",
        "	Until: 24:00, 1.0;  !- Field 3",
        " AirLoopHVAC:ControllerList,",
        "	CW Coil Controller, !- Name",
        "	Controller:WaterCoil, !- Controller 1 Object Type",
        "	CW Coil Controller; !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetSetPointManagerInputs(*state);
    // check specified control variable type is "HumidityRatio"
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::MaxHumRat, state->dataSetPointManager->spms(1)->ctrlVar);

    GetControllerInput(*state);
    // check control variable type in AllSetPtMgr is reset to "MaximumHumidityRatio"
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::MaxHumRat, state->dataSetPointManager->spms(1)->ctrlVar);

    // ControllerProps expects the control variable type to be "MaximumHumididtyRatio"
    state->dataHVACControllers->ControllerProps(1).HumRatCntrlType =
        GetHumidityRatioVariableType(*state, state->dataHVACControllers->ControllerProps(1).SensedNode);
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::MaxHumRat, state->dataHVACControllers->ControllerProps(1).HumRatCntrlType);

    // test index for air loop controllers
    // before controllers are simulated, AirLoopControllerIndex = 0
    ASSERT_EQ(0, state->dataHVACControllers->ControllerProps(1).AirLoopControllerIndex);

    OutputReportPredefined::SetPredefinedTables(*state);
    state->dataSimAirServingZones->GetAirLoopInputFlag = false;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirLoop->PriAirSysAvailMgr.allocate(1);
    state->dataAirLoop->AirLoopControlInfo.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).NumSupplyNodes = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopSupplyNodeNum.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopSupplyNodeNum(1) = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipSupplyNodeNum.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipSupplyNodeNum(1) = 4;
    state->dataConvergeParams->AirLoopConvergence.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).NumControllers = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex(1) = 0;
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerName.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerName(1) = "CW COIL CONTROLLER";
    state->dataAirSystemsData->PrimaryAirSystems(1).ControlConverged.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNumIn = 4;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNumOut = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalNodes = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNum.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNum(1) = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "CHILLED WATER COIL";
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).CompType_Num = SimAirServingZones::CompType::WaterCoil_Cooling;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->TotNumLoops = 1;

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 3;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "CHILLED WATER COIL";
    bool SimZoneEquipment(false);
    SimAirServingZones::SimAirLoops(*state, true, SimZoneEquipment);

    // after controllers are simulated, AirLoopControllerIndex = index to this controller on this air loop (e.g., n of num controllers on air loop)
    ASSERT_EQ(1, state->dataAirSystemsData->PrimaryAirSystems(1).NumControllers);
    ASSERT_EQ(1, state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex(1));
    ASSERT_EQ(1, state->dataHVACControllers->ControllerProps(1).AirLoopControllerIndex);
}

TEST_F(EnergyPlusFixture, HVACControllers_SchSetPointMgrsOrderTest)
{
    std::string const idf_objects = delimited_string({
        "  Coil:Cooling:Water,",
        "    Main Cooling Coil 1,     !- Name",
        "    CoolingCoilAvailSched,   !- Availability Schedule Name",
        "    autosize,                !- Design Water Flow Rate {m3/s}",
        "    autosize,                !- Design Air Flow Rate {m3/s}",
        "    autosize,                !- Design Inlet Water Temperature {C}",
        "    autosize,                !- Design Inlet Air Temperature {C}",
        "    autosize,                !- Design Outlet Air Temperature {C}",
        "    autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    CCoil Water Inlet Node,  !- Water Inlet Node Name",
        "    CCoil Water Outlet Node, !- Water Outlet Node Name",
        "    Mixed Air Node 1,        !- Air Inlet Node Name",
        "    CCoil Air Outlet Node,   !- Air Outlet Node Name",
        "    SimpleAnalysis,          !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",

        "  Schedule:Compact,",
        "   CoolingCoilAvailSched,	  !- Name",
        "	Fraction,			      !- Schedule Type Limits Name",
        "	Through: 12/31,		      !- Field 1",
        "	For: AllDays,		      !- Field 2",
        "	Until: 24:00, 1.0;        !- Field 3",

        "  Controller:WaterCoil,",
        "    Cooling Coil Controller,  !- Name",
        "    HumidityRatio,           !- Control Variable",
        "    Reverse,                 !- Action",
        "    FLOW,                    !- Actuator Variable",
        "    CCoil Air Outlet Node,   !- Sensor Node Name",
        "    CCoil Water Inlet Node,  !- Actuator Node Name",
        "    autosize,                !- Controller Convergence Tolerance {deltaC}",
        "    autosize,                !- Maximum Actuated Flow {m3/s}",
        "    0.0;                     !- Minimum Actuated Flow {m3/s}",

        "  SetpointManager:Scheduled,",
        "    CCoil Temp Setpoint Mgr, !- Name",
        "    Temperature,             !- Control Variable",
        "    Always 16,               !- Schedule Name",
        "    CCoil Air Outlet Node;   !- Setpoint Node or NodeList Name",

        "  Schedule:Compact,",
        "    Always 16,               !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,16;         !- Field 3",

        "  SetpointManager:Scheduled,",
        "    CCoil Hum Setpoint Mgr,  !- Name",
        "    MaximumHumidityRatio,    !- Control Variable",
        "    HumSetPt,                !- Schedule Name",
        "    CCoil Air Outlet Node;   !- Setpoint Node or NodeList Name",

        "  Schedule:Compact,",
        "    HumSetPt,                !- Name",
        "    AnyNumber,               !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.009;     !- Field 3",

        "  AirLoopHVAC:ControllerList,",
        "	CW Coil Controller,       !- Name",
        "	Controller:WaterCoil,     !- Controller 1 Object Type",
        "	Cooling Coil Controller;   !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetSetPointManagerInputs(*state);
    // There are two setpoint managers and are schedule type
    ASSERT_EQ(2, state->dataSetPointManager->spms.size()); // 2 schedule set point managers
    // check specified control variable types
    // this was a bug waiting to happen, iTemperature is declared as its own int const in HVACControllers.hh
    // and it just happened to have the same value as the iCtrlVarType_Temperature int const in SetPointManager.hh
    // changing it to CtrlVarType::Temp
    int spmCCoilHumNum = SetPointManager::GetSetPointManagerIndex(*state, "CCOIL HUM SETPOINT MGR");
    auto *spmCCoilHum = state->dataSetPointManager->spms(spmCCoilHumNum);

    int spmCCoilTempNum = SetPointManager::GetSetPointManagerIndex(*state, "CCOIL TEMP SETPOINT MGR");
    auto *spmCCoilTemp = state->dataSetPointManager->spms(spmCCoilTempNum);

    ASSERT_ENUM_EQ(HVAC::CtrlVarType::Temp, spmCCoilTemp->ctrlVar);     // is "Temperature"
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::MaxHumRat, spmCCoilHum->ctrlVar); // is "MaximumHumidityRatio"

    GetControllerInput(*state);
    // check ControllerProps control variable is set to "MaximumHumidityRatio"
    state->dataHVACControllers->ControllerProps(1).HumRatCntrlType =
        GetHumidityRatioVariableType(*state, state->dataHVACControllers->ControllerProps(1).SensedNode);
    ASSERT_ENUM_EQ(HVAC::CtrlVarType::MaxHumRat, state->dataHVACControllers->ControllerProps(1).HumRatCntrlType); // MaximumHumidityRatio
}

TEST_F(EnergyPlusFixture, HVACControllers_WaterCoilOnPrimaryLoopCheckTest)
{
    std::string const idf_objects = delimited_string({

        " Coil:Cooling:Water,",
        "	Chilled Water Coil,	!- Name",
        "	,        			!- Availability Schedule Name",
        "	0.01,				!- Design Water Flow Rate { m3 / s }",
        "	1.0,				!- Design Air Flow Rate { m3 / s }",
        "	7.2,				!- Design Inlet Water Temperature { C }",
        "	32.0,				!- Design Inlet Air Temperature { C }",
        "	12.0,				!- Design Outlet Air Temperature { C }",
        "	0.01,				!- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.07,				!- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Water Inlet Node,	!- Water Inlet Node Name",
        "	Water Outlet Node,  !- Water Outlet Node Name",
        "	Air Inlet Node,		!- Air Inlet Node Name",
        "	Air Outlet Node,	!- Air Outlet Node Name",
        "	SimpleAnalysis,		!- Type of Analysis",
        "	CrossFlow;          !- Heat Exchanger Configuration",

        " Controller:WaterCoil,",
        "	CW Coil Controller, !- Name",
        "	TemperatureAndHumidityRatio,!- Control Variable",
        "	Reverse,			!- Action",
        "	FLOW,				!- Actuator Variable",
        "	Air Outlet Node,	!- Sensor Node Name",
        "	Water Inlet Node,	!- Actuator Node Name",
        "	0.001,				!- Controller Convergence Tolerance { deltaC }",
        "	0.01,				!- Maximum Actuated Flow { m3 / s }",
        "	0.0;				!- Minimum Actuated Flow { m3 / s }",

        " AirLoopHVAC:ControllerList,",
        "	CW Coil Controller, !- Name",
        "	Controller:WaterCoil,   !- Controller 1 Object Type",
        "	CW Coil Controller; !- Controller 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetControllerInput(*state);

    ASSERT_EQ(state->dataWaterCoils->WaterCoil(1).Name, "CHILLED WATER COIL");
    ASSERT_EQ(state->dataWaterCoils->WaterCoil(1).WaterCoilType, DataPlant::PlantEquipmentType::CoilWaterCooling);

    OutputReportPredefined::SetPredefinedTables(*state);
    state->dataSimAirServingZones->GetAirLoopInputFlag = false;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).NumControllers = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex(1) = 0;
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerName.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerName(1) = "CW COIL CONTROLLER";
    state->dataAirSystemsData->PrimaryAirSystems(1).ControlConverged.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNumIn = 4;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNumOut = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalNodes = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNum.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNum(1) = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).CompType_Num = SimAirServingZones::CompType::WaterCoil_Cooling;

    bool WaterCoilOnAirLoop = true;
    std::string CompType = std::string{HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater]}; //"Coil:Cooling:Water";
    std::string CompName = "CHILLED WATER COIL";
    SimAirServingZones::CompType CoilTypeNum = SimAirServingZones::CompType::WaterCoil_Cooling;

    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnPrimaryAirLoopBranch(*state, CoilTypeNum, CompName);
    EXPECT_TRUE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = true;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnOASystem(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = true;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilSystemOnAirLoopOrOASystem(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = true;
    SimAirServingZones::CheckWaterCoilIsOnAirLoop(*state, CoilTypeNum, CompType, CompName, WaterCoilOnAirLoop);
    EXPECT_TRUE(WaterCoilOnAirLoop);

    // now test a different water coil type
    CoilTypeNum = SimAirServingZones::CompType::WaterCoil_DetailedCool;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnPrimaryAirLoopBranch(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);
}

TEST_F(EnergyPlusFixture, HVACControllers_WaterCoilOnOutsideAirSystemCheckTest)
{
    std::string const idf_objects = delimited_string({
        "  AirLoopHVAC:ControllerList,",
        "    OA Sys 1 Controllers,    !- Name",
        "    Controller:WaterCoil,    !- Controller 1 Object Type",
        "    Preheat Coil Controller; !- Controller 1 Name",

        "  Coil:Heating:Water,",
        "    OA Preheat HW Coil,      !- Name",
        "    ,                        !- Availability Schedule Name",
        "    autosize,                !- U-Factor Times Area Value {W/K}",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    HWCoil Water InletNode,  !- Zone1UnitHeatHWInletNode,!- Water Inlet Node Name",
        "    HWCoil Water OutletNode, !- Water Outlet Node Name",
        "    Outside Air Inlet Node,  !- Air Inlet Node Name",
        "    HW Coil Air OutletNode,  !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "    autosize,                !- Rated Capacity {W}",
        "    82.2,                    !- Rated Inlet Water Temperature {C}",
        "    16.6,                    !- Rated Inlet Air Temperature {C}",
        "    71.1,                    !- Rated Outlet Water Temperature {C}",
        "    32.2,                    !- Rated Outlet Air Temperature {C}",
        "    ;                        !- Rated Ratio for Air and Water Convection",

        "  Controller:WaterCoil,",
        "    Preheat Coil Controller, !- Name",
        "    Temperature,             !- Control Variable",
        "    Normal,                  !- Action",
        "    Flow,                    !- Actuator Variable",
        "    HW Coil Air OutletNode,  !- Sensor Node Name",
        "    HWCoil Water InletNode,  !- Actuator Node Name",
        "    Autosize,                !- Controller Convergence Tolerance {deltaC}",
        "    Autosize,                !- Maximum Actuated Flow {m3/s}",
        "    0;                       !- Minimum Actuated Flow {m3/s}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetControllerInput(*state);

    ASSERT_EQ(state->dataWaterCoils->WaterCoil(1).Name, "OA PREHEAT HW COIL");
    ASSERT_EQ(state->dataWaterCoils->WaterCoil(1).WaterCoilType, DataPlant::PlantEquipmentType::CoilWaterSimpleHeating);

    OutputReportPredefined::SetPredefinedTables(*state);
    state->dataSimAirServingZones->GetAirLoopInputFlag = false;

    state->dataAirLoop->NumOASystems = 1;
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataAirLoop->OutsideAirSys(1).Name = "AIRLOOP OASYSTEM";
    state->dataAirLoop->OutsideAirSys(1).NumControllers = 1;
    state->dataAirLoop->OutsideAirSys(1).ControllerName.allocate(1);
    state->dataAirLoop->OutsideAirSys(1).ControllerName(1) = "OA CONTROLLER 1";
    state->dataAirLoop->OutsideAirSys(1).NumComponents = 2;
    state->dataAirLoop->OutsideAirSys(1).ComponentType.allocate(2);
    state->dataAirLoop->OutsideAirSys(1).ComponentType(1) = HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater];
    state->dataAirLoop->OutsideAirSys(1).ComponentType(2) = "OutdoorAir:Mixer";
    state->dataAirLoop->OutsideAirSys(1).ComponentName.allocate(2);
    state->dataAirLoop->OutsideAirSys(1).ComponentName(1) = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataAirLoop->OutsideAirSys(1).ComponentName(2) = "OAMixer";
    state->dataAirLoop->OutsideAirSys(1).ComponentTypeEnum.allocate(2);
    state->dataAirLoop->OutsideAirSys(1).ComponentTypeEnum(1) = SimAirServingZones::CompType::WaterCoil_SimpleHeat;
    state->dataAirLoop->OutsideAirSys(1).ComponentTypeEnum(2) = SimAirServingZones::CompType::OAMixer_Num;

    state->dataMixedAir->OAMixer.allocate(1);
    state->dataMixedAir->OAMixer(1).Name = "OAMixer";
    state->dataMixedAir->OAMixer(1).InletNode = 2;

    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Name = "PrimaryAirLoop";
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = state->dataAirLoop->OutsideAirSys(1).Name;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).TypeOf = "AirLoopHVAC:OutdoorAirSystem";

    bool WaterCoilOnAirLoop = true;
    std::string CompType = std::string{HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater]};
    std::string CompName = state->dataWaterCoils->WaterCoil(1).Name;
    SimAirServingZones::CompType CoilTypeNum = SimAirServingZones::CompType::WaterCoil_SimpleHeat;

    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnPrimaryAirLoopBranch(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = false;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnOASystem(*state, CoilTypeNum, CompName);
    EXPECT_TRUE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = false;
    SimAirServingZones::CheckWaterCoilIsOnAirLoop(*state, CoilTypeNum, CompType, CompName, WaterCoilOnAirLoop);
    EXPECT_TRUE(WaterCoilOnAirLoop);

    // test a different water coil type
    CoilTypeNum = SimAirServingZones::CompType::WaterCoil_DetailedCool;
    WaterCoilOnAirLoop = true;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnOASystem(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);
}
TEST_F(EnergyPlusFixture, HVACControllers_CoilSystemCoolingWaterOnOutsideAirSystemCheckTest)
{
    std::string const idf_objects = delimited_string({
        "  AirLoopHVAC:ControllerList,",
        "    OA System Controllers,   !- Name",
        "    Controller:WaterCoil,    !- Controller 1 Object Type",
        "    Detailed WaterCoil Cntrl;!- Controller 1 Name",

        "  Controller:WaterCoil,",
        "    Detailed WaterCoil Cntrl,!- Name",
        "    Temperature,             !- Control Variable",
        "    Reverse,                 !- Action",
        "    FLOW,                    !- Actuator Variable",
        "    Main Cooling Coil 1 Outlet Node,  !- Sensor Node Name",
        "    Main Cooling Coil 1 Water Inlet Node,  !- Actuator Node Name",
        "    0.002,                   !- Controller Convergence Tolerance {deltaC}",
        "    autosize,                !- Maximum Actuated Flow {m3/s}",
        "    0.0;                     !- Minimum Actuated Flow {m3/s}",

        "  Coil:Cooling:Water:DetailedGeometry,",
        "    Detailed Pre Cooling Coil, !- Name",
        "    ,                        !- Availability Schedule Name",
        "    autosize,                !- Maximum Water Flow Rate {m3/s}",
        "    autosize,                !- Tube Outside Surface Area {m2}",
        "    autosize,                !- Total Tube Inside Area {m2}",
        "    autosize,                !- Fin Surface Area {m2}",
        "    autosize,                !- Minimum Airflow Area {m2}",
        "    autosize,                !- Coil Depth {m}",
        "    autosize,                !- Fin Diameter {m}",
        "    ,                        !- Fin Thickness {m}",
        "    ,                        !- Tube Inside Diameter {m}",
        "    ,                        !- Tube Outside Diameter {m}",
        "    ,                        !- Tube Thermal Conductivity {W/m-K}",
        "    ,                        !- Fin Thermal Conductivity {W/m-K}",
        "    ,                        !- Fin Spacing {m}",
        "    ,                        !- Tube Depth Spacing {m}",
        "    ,                        !- Number of Tube Rows",
        "    autosize,                !- Number of Tubes per Row",
        "    Main Cooling Coil 1 Water Inlet Node,  !- Water Inlet Node Name",
        "    Main Cooling Coil 1 Water Outlet Node,  !- Water Outlet Node Name",
        "    Main Cooling Coil 1 Inlet Node,  !- Air Inlet Node Name",
        "    Main Cooling Coil 1 Outlet Node;  !- Air Outlet Node Name",

        "  CoilSystem:Cooling:Water:HeatExchangerAssisted,",
        "    HXAssisting Cooling Coil,  !- Name",
        "    HeatExchanger:AirToAir:FlatPlate,  !- Heat Exchanger Object Type",
        "    HXAssisting Cooling Coil,  !- Heat Exchanger Name",
        "    Coil:Cooling:Water:DetailedGeometry,  !- Cooling Coil Object Type",
        "    Detailed Pre Cooling Coil; !- Cooling Coil Name",

        "  HeatExchanger:AirToAir:FlatPlate,",
        "    HXAssisting Cooling Coil,!- Name",
        "    ,                        !- Availability Schedule Name",
        "    CounterFlow,             !- Flow Arrangement Type",
        "    Yes,                     !- Economizer Lockout",
        "    1.0,                     !- Ratio of Supply to Secondary hA Values",
        "    1.32,                    !- Nominal Supply Air Flow Rate {m3/s}",
        "    24.0,                    !- Nominal Supply Air Inlet Temperature {C}",
        "    21.0,                    !- Nominal Supply Air Outlet Temperature {C}",
        "    1.32,                    !- Nominal Secondary Air Flow Rate {m3/s}",
        "    12.0,                    !- Nominal Secondary Air Inlet Temperature {C}",
        "    100.0,                   !- Nominal Electric Power {W}",
        "    Mixed Air Node 1,        !- Supply Air Inlet Node Name",
        "    Main Cooling Coil 1 Inlet Node,  !- Supply Air Outlet Node Name",
        "    Main Cooling Coil 1 Outlet Node,  !- Secondary Air Inlet Node Name",
        "    Main Heating Coil 1 Inlet Node;  !- Secondary Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetControllerInput(*state);

    ASSERT_EQ(state->dataWaterCoils->WaterCoil(1).Name, "DETAILED PRE COOLING COIL");
    ASSERT_EQ(state->dataWaterCoils->WaterCoil(1).WaterCoilType, DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling);

    OutputReportPredefined::SetPredefinedTables(*state);
    state->dataSimAirServingZones->GetAirLoopInputFlag = false;

    state->dataAirLoop->NumOASystems = 1;
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataAirLoop->OutsideAirSys(1).Name = "AIRLOOP OASYSTEM";
    state->dataAirLoop->OutsideAirSys(1).NumControllers = 1;
    state->dataAirLoop->OutsideAirSys(1).ControllerName.allocate(1);
    state->dataAirLoop->OutsideAirSys(1).ControllerName(1) = "OA CONTROLLER 1";
    state->dataAirLoop->OutsideAirSys(1).NumComponents = 2;
    state->dataAirLoop->OutsideAirSys(1).ComponentType.allocate(2);
    state->dataAirLoop->OutsideAirSys(1).ComponentType(1) = HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWaterHXAssisted];
    state->dataAirLoop->OutsideAirSys(1).ComponentType(2) = "OutdoorAir:Mixer";
    state->dataAirLoop->OutsideAirSys(1).ComponentName.allocate(2);
    state->dataAirLoop->OutsideAirSys(1).ComponentName(1) = "HXAssisting Cooling Coil";
    state->dataAirLoop->OutsideAirSys(1).ComponentName(2) = "OAMixer";
    state->dataAirLoop->OutsideAirSys(1).ComponentTypeEnum.allocate(2);
    state->dataAirLoop->OutsideAirSys(1).ComponentTypeEnum(1) = SimAirServingZones::CompType::WaterCoil_CoolingHXAsst;
    state->dataAirLoop->OutsideAirSys(1).ComponentTypeEnum(2) = SimAirServingZones::CompType::OAMixer_Num;

    state->dataMixedAir->OAMixer.allocate(1);
    state->dataMixedAir->OAMixer(1).Name = "OAMixer";
    state->dataMixedAir->OAMixer(1).InletNode = 2;

    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Name = "PrimaryAirLoop";
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = state->dataAirLoop->OutsideAirSys(1).Name;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).TypeOf = "AirLoopHVAC:OutdoorAirSystem";

    bool WaterCoilOnAirLoop = true;
    std::string CompType = std::string{HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWaterDetailed]};
    std::string CompName = state->dataWaterCoils->WaterCoil(1).Name;
    SimAirServingZones::CompType CoilTypeNum = SimAirServingZones::CompType::WaterCoil_DetailedCool;

    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnPrimaryAirLoopBranch(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = true;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilOnOASystem(*state, CoilTypeNum, CompName);
    EXPECT_FALSE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = false;
    WaterCoilOnAirLoop = SimAirServingZones::CheckWaterCoilSystemOnAirLoopOrOASystem(*state, CoilTypeNum, CompName);
    EXPECT_TRUE(WaterCoilOnAirLoop);

    WaterCoilOnAirLoop = false;
    SimAirServingZones::CheckWaterCoilIsOnAirLoop(*state, CoilTypeNum, CompType, CompName, WaterCoilOnAirLoop);
    EXPECT_TRUE(WaterCoilOnAirLoop);
}
TEST_F(EnergyPlusFixture, HVACControllers_CheckTempAndHumRatCtrl)
{
    state->dataHVACControllers->ControllerProps.allocate(1);
    state->dataHVACControllers->RootFinders.allocate(1);
    bool isConverged = true;
    int constexpr controlNum = 1;
    auto &thisController(state->dataHVACControllers->ControllerProps(1));
    thisController.ControlVar = HVACControllers::CtrlVarType::TemperatureAndHumidityRatio;
    thisController.Offset = 0.0001;
    int sensedNode = 1;
    thisController.SensedNode = sensedNode;
    state->dataLoopNodes->Node.allocate(2);
    state->dataLoopNodes->Node(sensedNode).Temp = 21.2;
    state->dataLoopNodes->Node(sensedNode).HumRatMax = 0.001;
    thisController.ActuatedNode = 2;
    thisController.ActuatedNodePlantLoc = {0, DataPlant::LoopSideLocation::Invalid, 0, 0};

    // Case 1 - not converged yet, no override yet, return untouched
    isConverged = false;
    thisController.HumRatCtrlOverride = false;
    thisController.SetPointValue = 21.1;
    thisController.IsSetPointDefinedFlag = true;
    thisController.NumCalcCalls = 5;
    state->dataLoopNodes->Node(sensedNode).HumRat = 0.0011;

    HVACControllers::CheckTempAndHumRatCtrl(*state, controlNum, isConverged);
    EXPECT_FALSE(isConverged);
    EXPECT_FALSE(thisController.HumRatCtrlOverride);
    EXPECT_NEAR(thisController.SetPointValue, 21.1, 0.0001);
    EXPECT_TRUE(thisController.IsSetPointDefinedFlag);
    EXPECT_EQ(thisController.NumCalcCalls, 5);

    // Case 2 - converged, override true, return untouched
    isConverged = true;
    thisController.HumRatCtrlOverride = true;
    thisController.SetPointValue = 21.1;
    thisController.IsSetPointDefinedFlag = true;
    thisController.NumCalcCalls = 5;
    state->dataLoopNodes->Node(sensedNode).HumRat = 0.0011;

    HVACControllers::CheckTempAndHumRatCtrl(*state, controlNum, isConverged);
    EXPECT_TRUE(isConverged);
    EXPECT_TRUE(thisController.HumRatCtrlOverride);
    EXPECT_NEAR(thisController.SetPointValue, 21.1, 0.0001);
    EXPECT_TRUE(thisController.IsSetPointDefinedFlag);
    EXPECT_EQ(thisController.NumCalcCalls, 5);

    // Case 3 - converged, override false, humrat<humratMax+Offset, return untouched
    isConverged = true;
    thisController.HumRatCtrlOverride = false;
    thisController.SetPointValue = 21.1;
    thisController.IsSetPointDefinedFlag = true;
    thisController.NumCalcCalls = 5;
    state->dataLoopNodes->Node(sensedNode).HumRat = state->dataLoopNodes->Node(sensedNode).HumRatMax - 0.001;

    HVACControllers::CheckTempAndHumRatCtrl(*state, controlNum, isConverged);
    EXPECT_TRUE(isConverged);
    EXPECT_FALSE(thisController.HumRatCtrlOverride);
    EXPECT_NEAR(thisController.SetPointValue, 21.1, 0.0001);
    EXPECT_TRUE(thisController.IsSetPointDefinedFlag);
    EXPECT_EQ(thisController.NumCalcCalls, 5);

    // Case 4 - converged, override false, humrat>humratMax+Offset, return with everything reset
    isConverged = true;
    thisController.HumRatCtrlOverride = false;
    thisController.SetPointValue = 21.1;
    thisController.IsSetPointDefinedFlag = true;
    thisController.NumCalcCalls = 5;
    state->dataLoopNodes->Node(sensedNode).HumRat = state->dataLoopNodes->Node(sensedNode).HumRatMax + 0.002;

    HVACControllers::CheckTempAndHumRatCtrl(*state, controlNum, isConverged);
    EXPECT_FALSE(isConverged);
    EXPECT_TRUE(thisController.HumRatCtrlOverride);
    EXPECT_NEAR(thisController.SetPointValue, 0.0, 0.0001);
    EXPECT_FALSE(thisController.IsSetPointDefinedFlag);
    EXPECT_EQ(thisController.NumCalcCalls, 0);

    // Case 5 - converged, override false, humrat>humratMax+Offset, temp only controller, return untouched
    isConverged = true;
    thisController.HumRatCtrlOverride = false;
    thisController.SetPointValue = 21.1;
    thisController.IsSetPointDefinedFlag = true;
    thisController.NumCalcCalls = 5;
    state->dataLoopNodes->Node(sensedNode).HumRat = state->dataLoopNodes->Node(sensedNode).HumRatMax - 0.001;
    thisController.ControlVar = HVACControllers::CtrlVarType::Temperature;

    HVACControllers::CheckTempAndHumRatCtrl(*state, controlNum, isConverged);
    EXPECT_TRUE(isConverged);
    EXPECT_FALSE(thisController.HumRatCtrlOverride);
    EXPECT_NEAR(thisController.SetPointValue, 21.1, 0.0001);
    EXPECT_TRUE(thisController.IsSetPointDefinedFlag);
    EXPECT_EQ(thisController.NumCalcCalls, 5);
}

TEST_F(EnergyPlusFixture, HVACControllers_BlankAutosized)
{
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:Water,",
        "   Chilled Water Coil, !- Name",
        "   AvailSched,         !- Availability Schedule Name",
        "   autosize,           !- Design Water Flow Rate { m3 / s }",
        "   autosize,           !- Design Air Flow Rate { m3 / s }",
        "   autosize,           !- Design Inlet Water Temperature { C }",
        "   autosize,           !- Design Inlet Air Temperature { C }",
        "   autosize,           !- Design Outlet Air Temperature { C }",
        "   autosize,           !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "   autosize,           !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "   Water Inlet Node,   !- Water Inlet Node Name",
        "   Water Outlet Node,  !- Water Outlet Node Name",
        "   Air Inlet Node,     !- Air Inlet Node Name",
        "   Air Outlet Node,    !- Air Outlet Node Name",
        "   SimpleAnalysis,     !- Type of Analysis",
        "   CrossFlow;          !- Heat Exchanger Configuration",
        " Controller:WaterCoil,",
        "   CW Coil Controller, !- Name",
        "   HumidityRatio,      !- Control Variable",
        "   Reverse,            !- Action",
        "   FLOW,               !- Actuator Variable",
        "   Air Outlet Node,    !- Sensor Node Name",
        "   Water Inlet Node,   !- Actuator Node Name",
        "   ,                   !- Controller Convergence Tolerance { deltaC }",
        "   ,                   !- Maximum Actuated Flow { m3 / s }",
        "   ;                   !- Minimum Actuated Flow { m3 / s }",
        " SetpointManager:Scheduled,",
        "   HumRatSPManager,    !- Name",
        "   HumidityRatio,      !- Control Variable",
        "   HumRatioSched,      !- Schedule Name",
        "   Air Outlet Node;    !- Setpoint Node or NodeList Name",
        " Schedule:Compact,",
        "   HumRatioSched,      !- Name",
        "   Any Number,         !- Schedule Type Limits Name",
        "   Through: 12/31,     !- Field 1",
        "   For: AllDays,       !- Field 2",
        "   Until: 24:00, 0.015; !- Field 3",
        " Schedule:Compact,",
        "   AvailSched,         !- Name",
        "   Fraction,           !- Schedule Type Limits Name",
        "   Through: 12/31,     !- Field 1",
        "   For: AllDays,       !- Field 2",
        "   Until: 24:00, 1.0;  !- Field 3",
        " AirLoopHVAC:ControllerList,",
        "   CW Coil Controller, !- Name",
        "   Controller:WaterCoil, !- Controller 1 Object Type",
        "   CW Coil Controller; !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    GetSetPointManagerInputs(*state);

    GetControllerInput(*state);

    ASSERT_EQ(state->dataHVACControllers->ControllerProps.size(), 1u);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MaxVolFlowActuated, DataSizing::AutoSize);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).Offset, DataSizing::AutoSize);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MinVolFlowActuated, 0.0);
}

TEST_F(EnergyPlusFixture, HVACControllers_MaxFlowZero)
{
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:Water,",
        "   Chilled Water Coil, !- Name",
        "   AvailSched,         !- Availability Schedule Name",
        "   0.00,               !- Design Water Flow Rate { m3 / s }",
        "   1.0,                !- Design Air Flow Rate { m3 / s }",
        "   7.2,                !- Design Inlet Water Temperature { C }",
        "   32.0,               !- Design Inlet Air Temperature { C }",
        "   12.0,               !- Design Outlet Air Temperature { C }",
        "   0.01,               !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "   0.07,               !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "   Water Inlet Node,   !- Water Inlet Node Name",
        "   Water Outlet Node,  !- Water Outlet Node Name",
        "   Air Inlet Node,     !- Air Inlet Node Name",
        "   Air Outlet Node,    !- Air Outlet Node Name",
        "   SimpleAnalysis,     !- Type of Analysis",
        "   CrossFlow;          !- Heat Exchanger Configuration",

        " Controller:WaterCoil,",
        "   CW Coil Controller, !- Name",
        "   HumidityRatio,      !- Control Variable",
        "   Reverse,            !- Action",
        "   FLOW,               !- Actuator Variable",
        "   Air Outlet Node,    !- Sensor Node Name",
        "   Water Inlet Node,   !- Actuator Node Name",
        "   ,                   !- Controller Convergence Tolerance { deltaC }",
        "   ,                   !- Maximum Actuated Flow { m3 / s }",
        "   ;                   !- Minimum Actuated Flow { m3 / s }",

        " SetpointManager:Scheduled,",
        "   HumRatSPManager,    !- Name",
        "   MaximumHumidityRatio, !- Control Variable",
        "   HumRatioSched,      !- Schedule Name",
        "   Air Outlet Node;    !- Setpoint Node or NodeList Name",

        " Schedule:Compact,",
        "   HumRatioSched,      !- Name",
        "   Fraction,           !- Schedule Type Limits Name",
        "   Through: 12/31,     !- Field 1",
        "   For: AllDays,       !- Field 2",
        "   Until: 24:00, 0.015; !- Field 3",

        " Schedule:Compact,",
        "   AvailSched,         !- Name",
        "   Fraction,           !- Schedule Type Limits Name",
        "   Through: 12/31,     !- Field 1",
        "   For: AllDays,       !- Field 2",
        "   Until: 24:00, 1.0;  !- Field 3",

        " ScheduleTypeLimits,",
        "   Fraction,         !- Name",
        "   0.0,              !- Lower Limit Value",
        "   1.0,              !- Upper Limit Value",
        "   CONTINUOUS;       !- Numeric Type",

        " AirLoopHVAC:ControllerList,",
        "   CW Coil Controller, !- Name",
        "   Controller:WaterCoil, !- Controller 1 Object Type",
        "   CW Coil Controller; !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetSetPointManagerInputs(*state);

    GetControllerInput(*state);

    ASSERT_EQ(state->dataHVACControllers->ControllerProps.size(), 1u);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MaxVolFlowActuated, DataSizing::AutoSize);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).Offset, DataSizing::AutoSize);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MinVolFlowActuated, 0.0);

    // test index for air loop controllers
    // before controllers are simulated, AirLoopControllerIndex = 0
    ASSERT_EQ(0, state->dataHVACControllers->ControllerProps(1).AirLoopControllerIndex);

    OutputReportPredefined::SetPredefinedTables(*state);
    state->dataSimAirServingZones->GetAirLoopInputFlag = false;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirLoop->PriAirSysAvailMgr.allocate(1);
    state->dataAirLoop->AirLoopControlInfo.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).NumSupplyNodes = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopSupplyNodeNum.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).AirLoopSupplyNodeNum(1) = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipSupplyNodeNum.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipSupplyNodeNum(1) = 4;
    state->dataConvergeParams->AirLoopConvergence.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).NumControllers = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex(1) = 0;
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerName.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).ControllerName(1) = "CW COIL CONTROLLER";
    state->dataAirSystemsData->PrimaryAirSystems(1).ControlConverged.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNumIn = 4;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNumOut = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalNodes = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNum.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).NodeNum(1) = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "CHILLED WATER COIL";
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).CompType_Num = SimAirServingZones::CompType::WaterCoil_Cooling;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop(1).Name = "CHW LOOP";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 3;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "CHILLED WATER COIL";

    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataSize->PlantSizData(1).ExitTemp = 6.0;
    state->dataSize->PlantSizData(1).PlantLoopName = "CHW LOOP";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Cooling;
    state->dataSize->PlantSizData(1).DesVolFlowRate = 1.0;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    state->dataSize->UnitarySysEqSizing.allocate(1);
    state->dataSize->UnitarySysEqSizing(1).CoolingCapacity = false;
    state->dataSize->UnitarySysEqSizing(1).HeatingCapacity = false;
    state->dataSize->UnitarySysEqSizing.deallocate();

    bool SimZoneEquipment(false);
    // compare_err_stream("", true); // reset error stream

    // This will call ManageController, which calls SizeController which should autosize the controller max actuated flow rate to zero
    // and issue a warning
    SimAirServingZones::SimAirLoops(*state, true, SimZoneEquipment);

    // after controllers are simulated, AirLoopControllerIndex = index to this controller on this air loop (e.g., n of num controllers on air loop)
    ASSERT_EQ(1, state->dataAirSystemsData->PrimaryAirSystems(1).NumControllers);
    ASSERT_EQ(1, state->dataAirSystemsData->PrimaryAirSystems(1).ControllerIndex(1));
    ASSERT_EQ(1, state->dataHVACControllers->ControllerProps(1).AirLoopControllerIndex);

    // This should have been autosized to zero
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MaxVolFlowActuated, 0.0);
    // This should have been reset to zero
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).MinVolFlowActuated, 0.0);

    Real64 expectedOffset = (0.001 / (2100.0 * HVAC::SmallWaterVolFlow)) * (DataConvergParams::HVACEnergyToler / 10.0);
    // do not let the controller tolerance exceed 1/10 of the loop temperature tolerance.
    expectedOffset = min(0.1 * DataConvergParams::HVACTemperatureToler, expectedOffset);
    EXPECT_EQ(expectedOffset, 0.1 * DataConvergParams::HVACTemperatureToler);
    EXPECT_EQ(state->dataHVACControllers->ControllerProps(1).Offset, expectedOffset);

    std::string const error_string = delimited_string({
        "   ** Warning ** InitController: Controller:WaterCoil=\"CW COIL CONTROLLER\", Maximum Actuated Flow is zero.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

} // namespace EnergyPlus
