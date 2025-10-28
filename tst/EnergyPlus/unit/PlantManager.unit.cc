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

// EnergyPlus::PlantManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/UserDefinedComponents.hh>

namespace EnergyPlus {
namespace PlantManager {
    using namespace DataPlant;
    using namespace DataLoopNode;
    using namespace DataSizing;
    using namespace SetPointManager;

    TEST_F(EnergyPlusFixture, PlantManager_SizePlantLoopTest)
    {
        state->init_state(*state);
        state->dataPlnt->PlantLoop.allocate(1);
        state->dataPlnt->PlantLoop(1).VolumeWasAutoSized = true;
        state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 5;
        state->dataPlnt->PlantLoop(1).CirculationTime = 2;
        state->dataPlnt->PlantLoop(1).FluidType = DataLoopNode::NodeFluidType::Water;
        state->dataPlnt->PlantLoop(1).TypeOfLoop = LoopType::Plant;
        state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
        SizePlantLoop(*state, 1, true);
        int TestVolume = 600;
        EXPECT_EQ(TestVolume, state->dataPlnt->PlantLoop(1).Volume);
    }

    TEST_F(EnergyPlusFixture, PlantManager_TwoWayCommonPipeSetPointManagerTest)
    {
        // issue 6069
        bool ErrorsFound = false;

        std::string const idf_objects = delimited_string({

            "  PlantLoop,",
            "    Chilled Water Loop,      !- Name",
            "    Water,                   !- Fluid Type",
            "    ,                        !- User Defined Fluid Type",
            "    Chilled Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
            "    Chilled Water Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
            "    98,                      !- Maximum Loop Temperature {C}",
            "    1,                       !- Minimum Loop Temperature {C}",
            "    0.12396E-02,             !- Maximum Loop Flow Rate {m3/s}",
            "    0,                       !- Minimum Loop Flow Rate {m3/s}",
            "    autocalculate,           !- Plant Loop Volume {m3}",
            "    Chilled Water Loop Supply Inlet,  !- Plant Side Inlet Node Name",
            "    Chilled Water Loop Supply Outlet,  !- Plant Side Outlet Node Name",
            "    Chilled Water Loop Supply Side Branches,  !- Plant Side Branch List Name",
            "    Chilled Water Loop Supply Side Connectors,  !- Plant Side Connector List Name",
            "    Chilled Water Loop Demand Inlet,  !- Demand Side Inlet Node Name",
            "    Chilled Water Loop Demand Outlet,  !- Demand Side Outlet Node Name",
            "    Chilled Water Loop Demand Side Branches,  !- Demand Side Branch List Name",
            "    Chilled Water Loop Demand Side Connectors,  !- Demand Side Connector List Name",
            "    SequentialLoad,          !- Load Distribution Scheme",
            "    ,                        !- Availability Manager List Name",
            "    SingleSetPoint,          !- Plant Loop Demand Calculation Scheme",
            "    TwoWayCommonPipe;        !- Common Pipe Simulation",

            "  BranchList,",
            "    Chilled Water Loop Supply Side Branches,  !- Name",
            "    Chilled Water Loop Supply Inlet Branch,  !- Branch 1 Name",
            "    Main Chiller ChW Branch, !- Branch 2 Name",
            "    Chilled Water Loop Supply Bypass Branch,  !- Branch 3 Name",
            "    Chilled Water Loop Supply Outlet Branch;  !- Branch 4 Name",

            "  Branch,",
            "    Chilled Water Loop Supply Inlet Branch,  !- Name",
            "    ,                        !- Pressure Drop Curve Name",
            "    Pump:ConstantSpeed,      !- Component 1 Object Type",
            "    Chilled Water Loop Pri Supply Pump,  !- Component 1 Name",
            "    Chilled Water Loop Supply Inlet,  !- Component 1 Inlet Node Name",
            "    Chilled Water Loop Pri Pump Outlet;  !- Component 1 Outlet Node Name",

            "  Branch,",
            "    Chilled Water Loop Supply Outlet Branch,  !- Name",
            "    ,                        !- Pressure Drop Curve Name",
            "    Pipe:Adiabatic,          !- Component 1 Object Type",
            "    Chilled Water Loop Supply Outlet Pipe,  !- Component 1 Name",
            "    Chilled Water Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
            "    Chilled Water Loop Supply Outlet;  !- Component 1 Outlet Node Name",

            "  Pipe:Adiabatic,",
            "    Chilled Water Loop Supply Outlet Pipe,  !- Name",
            "    Chilled Water Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
            "    Chilled Water Loop Supply Outlet;  !- Outlet Node Name",

            "  BranchList,",
            "    Chilled Water Loop Demand Side Branches,  !- Name",
            "    Chilled Water Loop Demand Inlet Branch,  !- Branch 1 Name",
            "    VAV Sys 1 Cooling Coil ChW Branch,  !- Branch 2 Name",
            "    Chilled Water Loop Demand Bypass Branch,  !- Branch 3 Name",
            "    Chilled Water Loop Demand Outlet Branch;  !- Branch 4 Name",

            "  Branch,",
            "    Chilled Water Loop Demand Inlet Branch,  !- Name",
            "    ,                        !- Pressure Drop Curve Name",
            "    Pump:VariableSpeed,      !- Component 1 Object Type",
            "    Chilled Water Loop Demand Pump,  !- Component 1 Name",
            "    Chilled Water Loop Demand Inlet,  !- Component 1 Inlet Node Name",
            "    Chilled Water Loop Demand Pump Outlet;  !- Component 1 Outlet Node Name",

            "  Branch,",
            "    Chilled Water Loop Demand Outlet Branch,  !- Name",
            "    ,                        !- Pressure Drop Curve Name",
            "    Pipe:Adiabatic,          !- Component 1 Object Type",
            "    Chilled Water Loop Demand Outlet Pipe,  !- Component 1 Name",
            "    Chilled Water Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
            "    Chilled Water Loop Demand Outlet;  !- Component 1 Outlet Node Name",

            "  Pipe:Adiabatic,",
            "    Chilled Water Loop Demand Outlet Pipe,  !- Name",
            "    Chilled Water Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
            "    Chilled Water Loop Demand Outlet;  !- Outlet Node Name",

            "  Schedule:Compact,",
            "    COMPACT HVAC-ALWAYS 5.0, !- Name",
            "    COMPACT HVAC Any Number, !- Schedule Type Limits Name",
            "    Through: 12/31,          !- Field 1",
            "    For: AllDays,            !- Field 2",
            "    Until: 24:00,5.0;        !- Field 3",

            "  SetpointManager:Scheduled,",
            "    Chilled Water Primary Loop Setpoint Manager,  !- Name",
            "    Temperature,             !- Control Variable",
            "    COMPACT HVAC-ALWAYS 5.0, !- Schedule Name",
            "    Chilled Water Loop Supply Outlet;  !- Setpoint Node or NodeList Name",

            "  Schedule:Compact,",
            "    COMPACT HVAC-ALWAYS 8.00,!- Name",
            "    COMPACT HVAC Any Number, !- Schedule Type Limits Name",
            "    Through: 12/31,          !- Field 1",
            "    For: AllDays,            !- Field 2",
            "    Until: 24:00,8.00;       !- Field 3",

            " SetpointManager:OutdoorAirReset,",
            "    Chilled Water Secondary Loop Setpoint Manager,  !- Name",
            "    Temperature,             !- Control Variable",
            "    11.11,                   !- Setpoint at Outdoor Low Temperature {C}",
            "    7.22,                    !- Outdoor Low Temperature {C}",
            "    7.22,                    !- Setpoint at Outdoor High Temperature {C}",
            "    29.44,                   !- Outdoor High Temperature {C}",
            "    Chilled Water Loop Supply inlet;  !- Setpoint Node or NodeList Name",
        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        // get input and checks if there are two setpointmanagers
        // for a TwoWayCommonPipe and one of them setpoints can be
        // a SetpointManager:OutdoorAirReset type.
        GetPlantLoopData(*state);
        ASSERT_FALSE(ErrorsFound);
        // there two setpoint managers in the loop
        EXPECT_EQ(2, state->dataSetPointManager->spms.size()); // SetpointManager:Scheduled

        // Schedule Setpoint Manager assigned at a plant loop supply outlet node
        EXPECT_EQ((int)state->dataSetPointManager->spms(1)->ctrlVar, (int)HVAC::CtrlVarType::Temp);
        EXPECT_EQ(state->dataLoopNodes->NodeID(state->dataSetPointManager->spms(1)->ctrlNodeNums[0]), "CHILLED WATER LOOP SUPPLY OUTLET");
        // OAReset Setpoint Manager assigned at a plant loop supply inlet node
        EXPECT_EQ((int)state->dataSetPointManager->spms(2)->ctrlVar, (int)HVAC::CtrlVarType::Temp);
        EXPECT_EQ(state->dataLoopNodes->NodeID(state->dataSetPointManager->spms(2)->ctrlNodeNums[0]), "CHILLED WATER LOOP SUPPLY INLET");
    }

    TEST_F(EnergyPlusFixture, PlantManager_SizePlantLoop_CorrectTempReportTest)
    {
        int loopNum = 1;
        bool okToFinish = false;

        state->dataPlnt->PlantLoop.allocate(1);
        state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
        state->dataPlantMgr->GetCompSizFac = false;
        state->dataSize->PlantSizData.allocate(1);
        state->dataPlnt->PlantLoop(1).LoopSide(LoopSideLocation::Demand).TotalBranches = 0;
        state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 0.0;
        state->dataPlnt->PlantLoop(1).MaxVolFlowRateWasAutoSized = false;
        state->dataPlnt->PlantLoop(1).VolumeWasAutoSized = true;
        state->dataPlnt->PlantLoop(1).CirculationTime = 1.0;
        state->dataPlnt->PlantFinalSizesOkayToReport = true;
        state->dataPlnt->PlantLoop(1).MinVolFlowRate = 0.0;
        state->dataPlnt->PlantLoop(1).FluidType = DataLoopNode::NodeFluidType::Steam;
        state->dataSize->PlantSizData(1).DesVolFlowRate = 1.0;
        state->dataSize->PlantSizData(1).DeltaT = 5.0;
        state->dataSize->PlantSizData(1).ExitTemp = 25.0;
        state->dataPlnt->PlantLoop(1).Name = "A LONG TIME AGO";

        // Test 1: Plant/Heating
        state->dataPlnt->PlantLoop(1).TypeOfLoop = LoopType::Plant;
        state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Heating;
        state->dataPlnt->PlantLoop(1).Name = "HOTH";
        SizePlantLoop(*state, loopNum, okToFinish);
        const std::string heating_eio_output = "PlantLoop, HOTH, Design Return Temperature [C], 20.0";
        compare_eio_stream_substring(heating_eio_output, true);

        // Test 2: Plant/Cooling
        state->dataPlnt->PlantLoop(1).TypeOfLoop = LoopType::Plant;
        state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Cooling;
        state->dataPlnt->PlantLoop(1).Name = "MUSTAFAR";
        SizePlantLoop(*state, loopNum, okToFinish);
        const std::string cooling_eio_output = "PlantLoop, MUSTAFAR, Design Return Temperature [C], 30.0";
        compare_eio_stream_substring(cooling_eio_output, true);

        // Test 2: Condenser
        state->dataPlnt->PlantLoop(1).TypeOfLoop = LoopType::Condenser;
        state->dataPlnt->PlantLoop(1).Name = "KAMINO";
        SizePlantLoop(*state, loopNum, okToFinish);
        const std::string condenser_eio_output = "CondenserLoop, KAMINO, Design Return Temperature [C], 30.0";
        compare_eio_stream_substring(condenser_eio_output, true);
    }

    TEST_F(EnergyPlusFixture, PlantManager_CheckPlantEquipmentCtrlType)
    {
        // Check size and alignment of DataPlant::PlantEquipmentCtrlType
        // If this unit test fails, it likely means that a new equipment type was added to DataPlant::PlantEquipmentType (in Enums.hh) but
        // a new element was not added in the corresponding place in array DataPlant::PlantEquipmentCtrlType (in Plant/Component.hh)

        // Check size first - this will likely never fail since the size is set using DataPlant::PlantEquipmentType::Num
        EXPECT_EQ(static_cast<int>(DataPlant::PlantEquipmentCtrlType.size()), static_cast<int>(DataPlant::PlantEquipmentType::Num));

        // Check first equipment type
        DataPlant::CtrlType ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::Boiler_Simple)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::HeatingOp);
        // Check last dew equipment types
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::CoolingPanel_Simple)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::Invalid);
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::HeatPumpEIRCooling)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::CoolingOp);
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::HeatPumpEIRHeating)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::HeatingOp);
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::PurchSteam)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::HeatingOp);
        // Check random other types
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::FluidCooler_SingleSpd)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::CoolingOp);
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::GrndHtExchgSlinky)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::DualOp);
        ctrlType = DataPlant::PlantEquipmentCtrlType[static_cast<int>(DataPlant::PlantEquipmentType::PurchHotWater)];
        EXPECT_EQ(ctrlType, DataPlant::CtrlType::HeatingOp);
    }

} // namespace PlantManager

namespace UserDefinedComponents {

    TEST_F(EnergyPlusFixture, Fix_CoilUserDefined_Test)
    {
        bool ErrorsFound = false;

        std::string const idf_objects = delimited_string({
            "  Coil:UserDefined,",
            "    CoilUserDef_1, !-Name",
            "    , !-Overall Model Simulation Program Calling Manager Name",
            "    Test Program Calling Manager, !-Model Setup and Sizing Program Calling Manager Name",
            "    2, !-Number of Air Connections",
            "    Primary_Inlet_Node, !-Air Connection 1 Inlet Node Name",
            "    Primary_Outlet_Node, !-Air Connection 1 Outlet Node Name",
            "    Secondary_Inlet_Node, !-Air Connection 2 Inlet Node Name",
            "    Secondary_Outlet_Node, !-Air Connection 2 Outlet Node Name",
            "    No, !-Plant Connection is Used",
            "    , !-Plant Connection Inlet Node Name",
            "    ; !-Plant Connection Outlet Node Name",

            "  OutdoorAir:Node,",
            "    Test_OA_Node;",

            "  EnergyManagementSystem:Actuator,",
            "    TempSetpointLo,          !- Name",
            "    Test_OA_Node,  !- Actuated Component Unique Name",
            "    System Node Setpoint,    !- Actuated Component Type",
            "    Temperature Minimum Setpoint;    !- Actuated Component Control Type",

            "  EnergyManagementSystem:Actuator,",
            "    TempSetpointHi,          !- Name",
            "    Test_OA_Node,  !- Actuated Component Unique Name",
            "    System Node Setpoint,    !- Actuated Component Type",
            "    Temperature Maximum Setpoint;    !- Actuated Component Control Type",

            "  EnergyManagementSystem:ProgramCallingManager,",
            "    Test Program Calling Manager,  !- Name",
            "    BeginNewEnvironment,  !- EnergyPlus Model Calling Point",
            "    DualSetpointTestControl;  !- Program Name 1",

            "  EnergyManagementSystem:Program,",
            "    DualSetpointTestControl,",
            "    Set TempSetpointLo = 16.0,",
            "    Set TempSetpointHi  = 20.0;",

        });

        ASSERT_TRUE(process_idf(idf_objects));
        state->init_state(*state);

        OutAirNodeManager::SetOutAirNodes(*state);
        EMSManager::CheckIfAnyEMS(*state);
        state->dataEMSMgr->FinishProcessingUserInput = true;

        bool anyRan;
        EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
        EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

        GetUserDefinedPlantComponents(*state);
        ASSERT_FALSE(ErrorsFound);

        // Five Node Connections
        EXPECT_EQ(5, state->dataBranchNodeConnections->NumOfNodeConnections);

        // OA Node
        EXPECT_ENUM_EQ(NodeInputManager::CompFluidStream::Primary, state->dataBranchNodeConnections->NodeConnections(1).FluidStream);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionObjectType::OutdoorAirNode, state->dataBranchNodeConnections->NodeConnections(1).ObjectType);
        EXPECT_EQ("OutdoorAir:Node", state->dataBranchNodeConnections->NodeConnections(1).ObjectName);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionType::OutsideAir, state->dataBranchNodeConnections->NodeConnections(1).ConnectionType);
        EXPECT_EQ("TEST_OA_NODE", state->dataBranchNodeConnections->NodeConnections(1).NodeName);

        // Coil Air Primiary Inlet
        EXPECT_ENUM_EQ(NodeInputManager::CompFluidStream::Primary, state->dataBranchNodeConnections->NodeConnections(2).FluidStream);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionObjectType::CoilUserDefined, state->dataBranchNodeConnections->NodeConnections(2).ObjectType);
        EXPECT_EQ("COILUSERDEF_1", state->dataBranchNodeConnections->NodeConnections(2).ObjectName);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionType::Inlet, state->dataBranchNodeConnections->NodeConnections(2).ConnectionType);
        EXPECT_EQ("PRIMARY_INLET_NODE", state->dataBranchNodeConnections->NodeConnections(2).NodeName);

        // Coil Air Primiary Outlet
        EXPECT_ENUM_EQ(NodeInputManager::CompFluidStream::Primary, state->dataBranchNodeConnections->NodeConnections(3).FluidStream);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionObjectType::CoilUserDefined, state->dataBranchNodeConnections->NodeConnections(3).ObjectType);
        EXPECT_EQ("COILUSERDEF_1", state->dataBranchNodeConnections->NodeConnections(3).ObjectName);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionType::Outlet, state->dataBranchNodeConnections->NodeConnections(3).ConnectionType);
        EXPECT_EQ("PRIMARY_OUTLET_NODE", state->dataBranchNodeConnections->NodeConnections(3).NodeName);

        // Coil Air Secondary Inlet
        EXPECT_ENUM_EQ(NodeInputManager::CompFluidStream::Secondary, state->dataBranchNodeConnections->NodeConnections(4).FluidStream);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionObjectType::CoilUserDefined, state->dataBranchNodeConnections->NodeConnections(4).ObjectType);
        EXPECT_EQ("COILUSERDEF_1", state->dataBranchNodeConnections->NodeConnections(4).ObjectName);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionType::Inlet, state->dataBranchNodeConnections->NodeConnections(4).ConnectionType);
        EXPECT_EQ("SECONDARY_INLET_NODE", state->dataBranchNodeConnections->NodeConnections(4).NodeName);

        // Coil Air Secondary Outlet
        EXPECT_ENUM_EQ(NodeInputManager::CompFluidStream::Secondary, state->dataBranchNodeConnections->NodeConnections(5).FluidStream);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionObjectType::CoilUserDefined, state->dataBranchNodeConnections->NodeConnections(5).ObjectType);
        EXPECT_EQ("COILUSERDEF_1", state->dataBranchNodeConnections->NodeConnections(5).ObjectName);
        EXPECT_ENUM_EQ(DataLoopNode::ConnectionType::Outlet, state->dataBranchNodeConnections->NodeConnections(5).ConnectionType);
        EXPECT_EQ("SECONDARY_OUTLET_NODE", state->dataBranchNodeConnections->NodeConnections(5).NodeName);
    }
} // namespace UserDefinedComponents

} // namespace EnergyPlus
