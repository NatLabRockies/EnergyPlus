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

// EnergyPlus::EMSManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantCondLoopOperation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include <algorithm>
#include <array>

using namespace EnergyPlus;
using namespace EnergyPlus::EMSManager;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataRuntimeLanguage;
using namespace EnergyPlus::PlantUtilities;

TEST_F(EnergyPlusFixture, EMSManager_TestForUniqueEMSActuators)
{
    state->init_state(*state);
    state->dataRuntimeLang->EMSActuatorAvailable.allocate(100);

    std::string componentTypeName1("Chiller1");
    std::string componentTypeName2("Chiller2");
    std::string uniqueIDName1("Plant Component Chiller:Electric:ReformulatedEIR");
    std::string controlTypeName1("On/Off Supervisory");
    std::string units1("None");
    bool EMSActuated1(true);
    bool testBoolean1(true);
    bool testBoolean2(true);
    bool testBoolean3(true);

    // calling three times but twice with same names should still result in only two item in the resulting list
    SetupEMSActuator(*state, componentTypeName1, uniqueIDName1, controlTypeName1, units1, EMSActuated1, testBoolean1);
    SetupEMSActuator(*state, componentTypeName1, uniqueIDName1, controlTypeName1, units1, EMSActuated1, testBoolean2);
    SetupEMSActuator(*state, componentTypeName2, uniqueIDName1, controlTypeName1, units1, EMSActuated1, testBoolean3);
    EXPECT_EQ(2, state->dataRuntimeLang->numEMSActuatorsAvailable);

    // repeat with integers
    std::string controlTypeName2("ModeOfSomething");
    int testInt1(7);
    int testInt2(9);
    int testInt3(11);
    SetupEMSActuator(*state, componentTypeName1, uniqueIDName1, controlTypeName2, units1, EMSActuated1, testInt1);
    SetupEMSActuator(*state, componentTypeName1, uniqueIDName1, controlTypeName2, units1, EMSActuated1, testInt2);
    SetupEMSActuator(*state, componentTypeName2, uniqueIDName1, controlTypeName2, units1, EMSActuated1, testInt3);
    EXPECT_EQ(4, state->dataRuntimeLang->numEMSActuatorsAvailable);

    // repeat with reals
    std::string controlTypeName3("ValueOfResults");
    Real64 testReal1(0.123);
    Real64 testReal2(0.456);
    Real64 testReal3(0.789);
    SetupEMSActuator(*state, componentTypeName1, uniqueIDName1, controlTypeName3, units1, EMSActuated1, testReal1);
    SetupEMSActuator(*state, componentTypeName1, uniqueIDName1, controlTypeName3, units1, EMSActuated1, testReal2);
    SetupEMSActuator(*state, componentTypeName2, uniqueIDName1, controlTypeName3, units1, EMSActuated1, testReal3);
    EXPECT_EQ(6, state->dataRuntimeLang->numEMSActuatorsAvailable);

    state->dataRuntimeLang->EMSActuatorAvailable.deallocate();
}

TEST_F(EnergyPlusFixture, Dual_NodeTempSetpoints)
{
    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Actuator,",
        "TempSetpointLo,          !- Name",
        "Test node,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Minimum Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpointHi,          !- Name",
        "Test node,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Maximum Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Dual Setpoint Test Manager,  !- Name",
        "BeginNewEnvironment,  !- EnergyPlus Model Calling Point",
        "DualSetpointTestControl;  !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "DualSetpointTestControl,",
        "Set TempSetpointLo = 16.0,",
        "Set TempSetpointHi  = 20.0;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);

    EMSManager::CheckIfAnyEMS(*state);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());

    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_NEAR(state->dataLoopNodes->Node(1).TempSetPointHi, 20.0, 0.000001);

    EXPECT_NEAR(state->dataLoopNodes->Node(1).TempSetPointLo, 16.0, 0.000001);
}

TEST_F(EnergyPlusFixture, CheckActuatorInit)
{
    // this test checks that new actuators have the Erl variable associated with them set to Null right away, issue #5710
    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Actuator,",
        "TempSetpointLo,          !- Name",
        "Test node,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Minimum Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Dual Setpoint Test Manager,  !- Name",
        "EndOfSystemTimestepBeforeHVACReporting,  !- EnergyPlus Model Calling Point",
        "DualSetpointTestControl;  !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "DualSetpointTestControl,",
        "Set TempSetpointLo = 16.0;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    OutAirNodeManager::SetOutAirNodes(*state);
    EMSManager::GetEMSInput(*state);

    // now check that Erl variable is Null
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlVariable(1).Value.Type, DataRuntimeLanguage::Value::Null);
}

TEST_F(EnergyPlusFixture, SupervisoryControl_PlantComponent_SetActuatedBranchFlowRate)
{
    // test EMS actuator for Plant Component
    // test SetActuatedBranchFlowRate for expected response

    std::string const idf_objects = delimited_string({
        " EnergyManagementSystem:Actuator,",
        "  CoilActuator,          !- Name",
        "  Zone1FanCoilHeatingCoil,  !- Actuated Component Unique Name",
        "  Plant Component Coil:Heating:Water,    !- Actuated Component Type",
        "  On/Off Supervisory;    !- Actuated Component Control Type",

        " EnergyManagementSystem:ProgramCallingManager,",
        "  Supervisory Control Manager,  !- Name",
        "  BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "  HeatCoilController;  !- Program Name 1",

        " EnergyManagementSystem:Program,",
        "  HeatCoilController,",
        "  Set CoilActuator = 0.0;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // sets number of EMS objects
    EMSManager::CheckIfAnyEMS(*state);

    // allows NodeSetpoint and AvailabilityManagers actuators to be setup
    state->dataEMSMgr->FinishProcessingUserInput = true;

    // set up plant loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "MyPlant";
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // create 2 components on a single branch to simulate water flow control for entire branch
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating; // Coil:Heating:Water
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "Zone1FanCoilHeatingCoil";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Type =
        DataPlant::PlantEquipmentType::Pipe; // Pipe:Adiabatic
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Name = "Pipe";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumOut = 3;
    PlantCondLoopOperation::SetupPlantEMSActuators(*state);

    // set flow, max and maxavail on the nodes
    state->dataLoopNodes->Node.allocate(3);
    Real64 NodeMdot(1.5);
    state->dataLoopNodes->Node(1).MassFlowRate = NodeMdot;
    state->dataLoopNodes->Node(1).MassFlowRateMax = NodeMdot;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = NodeMdot;
    state->dataLoopNodes->Node(1).MassFlowRateRequest = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRate = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRateMax = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRateRequest = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRate = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRateMax = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRateMaxAvail = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRateRequest = NodeMdot;

    bool anyRan;
    // set up EMS
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());

    // set dummy EMS value
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue = 1.0;

    // dummy value set above should be zero'd on this call since EMS 0's values on begin environment (whether EMS program runs on this call or not)
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideOn);
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue, 0.0, 0.000001);

    // expect node data to represent full flow
    // SetActuatedBranchFlowRate(*state, CompFlow, ActuatedNode, LoopNum, LoopSideNum, BranchNum, ResetMode )
    PlantLocation plantLoc0{1, DataPlant::LoopSideLocation::Demand, 1, 0};
    PlantUtilities::SetPlantLocationLinks(*state, plantLoc0);

    SetActuatedBranchFlowRate(*state, NodeMdot, 1, plantLoc0, false);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    SetActuatedBranchFlowRate(*state, NodeMdot, 2, plantLoc0, false);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateRequest, NodeMdot);

    // set dummy EMS value
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue = 1.0;

    // dummy value set above should remain on this call since EMS calling manager uses BeginTimestepBeforePredictor as the calling point
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideOn);
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue, 1.0, 0.000001);
    SetActuatedBranchFlowRate(*state, NodeMdot, 1, plantLoc0, false);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    SetActuatedBranchFlowRate(*state, NodeMdot, 2, plantLoc0, false);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateRequest, NodeMdot);

    // dummy value set above should reset to 0 on this call since EMS calling manager uses BeginTimestepBeforePredictor as the calling point
    // override flag should also be true
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideOn);
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue, 0.0, 0.000001);

    // expect node data to represent no flow. Request is also 0's in this function. Max and MaxAvail are not changed
    SetActuatedBranchFlowRate(*state, NodeMdot, 1, plantLoc0, false);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateRequest, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, 0.0);
    SetActuatedBranchFlowRate(*state, NodeMdot, 2, plantLoc0, false);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateRequest, 0.0);
}

TEST_F(EnergyPlusFixture, SupervisoryControl_PlantComponent_SetComponentFlowRate)
{

    // test EMS actuator for Plant Component
    // test SetComponentFlowRate for expected response

    std::string const idf_objects = delimited_string({
        " EnergyManagementSystem:Actuator,",
        "  CoilActuator,          !- Name",
        "  Zone1FanCoilHeatingCoil,  !- Actuated Component Unique Name",
        "  Plant Component Coil:Heating:Water,    !- Actuated Component Type",
        "  On/Off Supervisory;    !- Actuated Component Control Type",

        " EnergyManagementSystem:ProgramCallingManager,",
        "  Supervisory Control Manager,  !- Name",
        "  BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "  HeatCoilController;  !- Program Name 1",

        " EnergyManagementSystem:Program,",
        "  HeatCoilController,",
        "  Set CoilActuator = 0.0;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // sets number of EMS objects
    EMSManager::CheckIfAnyEMS(*state);

    // allows NodeSetpoint and AvailabilityManagers actuators to be setup
    state->dataEMSMgr->FinishProcessingUserInput = true;

    // set up plant loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "MyPlant";
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // create 2 components on a single branch to simulate water flow control for entire branch
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating; // Coil:Heating:Water
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "Zone1FanCoilHeatingCoil";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Type =
        DataPlant::PlantEquipmentType::Pipe; // Pipe:Adiabatic
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Name = "Pipe";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumOut = 3;
    PlantCondLoopOperation::SetupPlantEMSActuators(*state);

    // set flow, max and maxavail on the nodes
    state->dataLoopNodes->Node.allocate(3);
    Real64 NodeMdot(1.5);
    state->dataLoopNodes->Node(1).MassFlowRate = NodeMdot;
    state->dataLoopNodes->Node(1).MassFlowRateMax = NodeMdot;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = NodeMdot;
    state->dataLoopNodes->Node(1).MassFlowRateRequest = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRate = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRateMax = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = NodeMdot;
    state->dataLoopNodes->Node(2).MassFlowRateRequest = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRate = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRateMax = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRateMaxAvail = NodeMdot;
    state->dataLoopNodes->Node(3).MassFlowRateRequest = NodeMdot;

    bool anyRan;
    // set up EMS
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    // set dummy EMS value
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue = 1.0;

    // dummy value set above should be zero'd on this call since EMS 0's values on begin environment (whether EMS program runs on this call or not)
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideOn);
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue, 0.0, 0.000001);

    // expect node data to represent full flow
    // SetComponentFlowRate(*state, CompFlow, InletNode, OutletNode, LoopNum, LoopSideNum, BranchIndex, CompIndex )
    PlantLocation plantLoc1{1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, plantLoc1);

    SetComponentFlowRate(*state, NodeMdot, 1, 2, plantLoc1);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    SetComponentFlowRate(*state, NodeMdot, 2, 3, plantLoc1);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateRequest, NodeMdot);

    // set dummy EMS value
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue = 1.0;

    // dummy value set above should remain on this call since EMS calling manager uses BeginTimestepBeforePredictor as the calling point
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideOn);
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue, 1.0, 0.000001);

    // expect node data to represent full flow
    SetComponentFlowRate(*state, NodeMdot, 1, 2, plantLoc1);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    SetComponentFlowRate(*state, NodeMdot, 2, 3, plantLoc1);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRate, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateRequest, NodeMdot);

    // dummy value set above should reset to 0 on this call since EMS calling manager uses BeginTimestepBeforePredictor as the calling point
    // override flag should also be true
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideOn);
    EXPECT_NEAR(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).EMSLoadOverrideValue, 0.0, 0.000001);
    Real64 tempNodeMdot(NodeMdot);

    // expect node data to represent no flow. Max, MaxAvail, and Request are not changed
    SetComponentFlowRate(*state, tempNodeMdot, 1, 2, plantLoc1);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    tempNodeMdot = NodeMdot;
    SetComponentFlowRate(*state, tempNodeMdot, 2, 3, plantLoc1);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(2).MassFlowRateRequest, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRate, 0.0);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMax, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateMaxAvail, NodeMdot);
    EXPECT_EQ(state->dataLoopNodes->Node(3).MassFlowRateRequest, NodeMdot);
}

TEST_F(EnergyPlusFixture, Test_EMSLogic)
{
    // AUTHOR: R. Raustad, FSEC
    // DATE WRITTEN: Nov 2015
    // Tests to evaluate robustness of EMS programming language math expressions
    // specific issue was use of "-" just after operand, e.g., IF MyVar == -X,

    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node 1;",
        "OutdoorAir:Node, Test node 2;",
        "OutdoorAir:Node, Test node 3;",
        "OutdoorAir:Node, Test node 4;",
        "OutdoorAir:Node, Test node 5;",
        "OutdoorAir:Node, Test node 6;",
        "OutdoorAir:Node, Test node 7;",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint1,          !- Name",
        "Test node 1,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint2,          !- Name",
        "Test node 2,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint3,          !- Name",
        "Test node 3,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint4,          !- Name",
        "Test node 4,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint5,          !- Name",
        "Test node 5,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint6,          !- Name",
        "Test node 6,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint7,          !- Name",
        "Test node 7,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Logic Manager 1,  !- Name",
        "BeginNewEnvironment,  !- EnergyPlus Model Calling Point",
        "LogicTest1;  !- Program Name 1",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Logic Manager 2,  !- Name",
        "BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "LogicTest2;  !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "LogicTest1,",
        "Set MyVar1 = 10,",
        "Set MyVar2 = -10,",
        "Set MyVar3 = -10+3-1-2,",
        "Set MyVar4 = 10,",
        "Set MyVar5 = -PI,",
        "Set MyVar6 = ( 10 - ( ( @MOD 15.5 3 ) * 40 ) ),",
        "Set MyVar7 = ( 9.9 - ( @MOD 15.5 3 ) * 20 ),",

        "IF MyVar1 == 10,",
        "  Set TempSetpoint1 = 11.0,",
        "ELSE,",
        "  Set TempSetpoint1 = 21.0,",
        "ENDIF,",
        "IF MyVar2 == -10,",
        "  Set TempSetpoint2 = 12.0,",
        "ELSE,",
        "  Set TempSetpoint2 = 22.0,",
        "ENDIF,",
        "IF -10 == MyVar3,",
        "  Set TempSetpoint3 = 13.0,",
        "ELSE,",
        "  Set TempSetpoint3 = 23.0,",
        "ENDIF,",
        "IF MyVar4 == -20+30,",
        "  Set TempSetpoint4 = 14.0,",
        "ELSE,",
        "  Set TempSetpoint4 = 24.0,",
        "ENDIF,",
        "IF MyVar5 == -PI,",
        "  Set TempSetpoint5 = 15.0,",
        "ELSE,",
        "  Set TempSetpoint5 = 25.0,",
        "ENDIF,",
        "IF MyVar6 == -10,",
        "  Set TempSetpoint6 = 16.0,",
        "ELSE,",
        "  Set TempSetpoint6 = 26.0,",
        "ENDIF,",
        "IF MyVar7 > -11.0+3-1+8.89,",
        "  Set TempSetpoint7 = 17.0,",
        "ELSE,",
        "  Set TempSetpoint7 = 27.0,",
        "ENDIF;",

        "EnergyManagementSystem:Program,",
        "LogicTest2,",
        "Set MyVar1 = 10,",
        "Set MyVar2 = -10,",
        "Set MyVar3 = -10 + 3 - 1 - 2,",
        "Set MyVar4 = 10,",
        "Set MyVar5 = -PI,",
        "Set MyVar6 = ( 10 - ( ( @MOD 15.5 3 ) * 40 ) ),",
        "Set MyVar7 = ( 9.9 - ( @MOD 15.5 3 ) * 20 ),",

        "IF ( MyVar1 <> 10 ),",
        "  Set TempSetpoint1 = 11.0,",
        "ELSE,",
        "  Set TempSetpoint1 = 21.0,",
        "ENDIF,",
        "IF ( MyVar2 <> -10 ),",
        "  Set TempSetpoint2 = 12.0,",
        "ELSE,",
        "  Set TempSetpoint2 = 22.0,",
        "ENDIF,",
        "IF ( -10 <> MyVar3 ),",
        "  Set TempSetpoint3 = 13.0,",
        "ELSE,",
        "  Set TempSetpoint3 = 23.0,",
        "ENDIF,",
        "IF ( MyVar4 <> ( -20+30 ) ),",
        "  Set TempSetpoint4 = 14.0,",
        "ELSE,",
        "  Set TempSetpoint4 = 24.0,",
        "ENDIF,",
        "IF ( MyVar5 <> -PI ),",
        "  Set TempSetpoint5 = 15.0,",
        "ELSE,",
        "  Set TempSetpoint5 = 25.0,",
        "ENDIF,",
        "IF ( MyVar6 <> -10 ),",
        "  Set TempSetpoint6 = 16.0,",
        "ELSE,",
        "  Set TempSetpoint6 = 26.0,",
        "ENDIF,",
        "IF ( MyVar7 == -0.1 ),",
        "  Set TempSetpoint7 = 17.0,",
        "ELSE,",
        "  Set TempSetpoint7 = 27.0,",
        "ENDIF;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);

    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;
    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_NEAR(state->dataLoopNodes->Node(1).TempSetPoint, 11.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).TempSetPoint, 12.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(3).TempSetPoint, 13.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(4).TempSetPoint, 14.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(5).TempSetPoint, 15.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(6).TempSetPoint, 16.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(7).TempSetPoint, 17.0, 0.0000001);

    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_NEAR(state->dataLoopNodes->Node(1).TempSetPoint, 21.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(2).TempSetPoint, 22.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(3).TempSetPoint, 23.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(4).TempSetPoint, 24.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(5).TempSetPoint, 25.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(6).TempSetPoint, 26.0, 0.0000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(7).TempSetPoint, 27.0, 0.0000001);
}

TEST_F(EnergyPlusFixture, Debug_EMSLogic)
{
    // AUTHOR: R. Raustad, FSEC
    // DATE WRITTEN: Nov 2015
    // Tests to evaluate robustness of EMS programming language math expressions
    // specific issue was use of "-" just after operand, e.g., Set MyVar == -X,
    // This unit test is purposely singular to enable easy debugging
    // If a problem is suspected, change this unit test accordingly and test
    // Much easier to single step this unit test than a more complex version

    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node 1;",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint1,          !- Name",
        "Test node 1,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Logic Manager 1,  !- Name",
        "BeginNewEnvironment,  !- EnergyPlus Model Calling Point",
        "LogicTest1;  !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "LogicTest1,",
        "Set MyVar1 = ( -2 ),",
        "Set MyVar2 = ( -2 ),",
        "Set TempSetpoint1 = MyVar1 / MyVar2;",

        //		"IF MyVar1 == 8,",
        //		"  Set TempSetpoint1 = 11.0,",
        //		"ELSE,",
        //		"  Set TempSetpoint1 = 21.0,",
        //		"ENDIF;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);

    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;
    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

    EXPECT_NEAR(state->dataLoopNodes->Node(1).TempSetPoint, 1.0, 0.0000001);
}

TEST_F(EnergyPlusFixture, TestAnyRanArgument)
{
    // small test to demonstrate new boolean argument.
    // shows a simple way to setup sensor on a node, need to call SetupNodeVarsForReporting()

    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Sensor,",
        "Node_mdot,",
        "Test node,",
        "System Node Mass Flow Rate;",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set dumm1 = Node_mdot;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);
    Node::SetupNodeVarsForReporting(*state);
    EMSManager::CheckIfAnyEMS(*state);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_FALSE(anyRan);

    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_FALSE(anyRan);

    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::HVACIterationLoop, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyRan);
}

TEST_F(EnergyPlusFixture, TestUnInitializedEMSVariable1)
{
    // this tests the new initialized variable added to Erl variable value data structure, for issue #4943
    // this is also what is checked to see if an actuator has been used for issue #4404.
    std::string const idf_objects = delimited_string({

        "EnergyManagementSystem:GlobalVariable,",
        "TempSetpoint1;          !- Name",

        "EnergyManagementSystem:Program,",
        "InitVariableTest,",
        "Set TempSetpoint1 = 21.0;"

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test Program Manager 1,  !- Name",
        "BeginNewEnvironment,  !- EnergyPlus Model Calling Point",
        "InitVariableTest;  !- Program Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;
    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    // Find the variable in the list
    int internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TempSetpoint1", 0);
    ASSERT_GT(internalVarNum, 0);
    // Expect the variable to not yet be initialized
    EXPECT_FALSE(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.initialized);
    // next run a small program that sets the value
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());
    // check that it worked and the value came thru
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 21.0, 0.0000001);
    // check of state to see if now initialized
    EXPECT_TRUE(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.initialized);
}

TEST_F(EnergyPlusFixture, TestUnInitializedEMSVariable2)
{
    // this tests the new initialized variable added to Erl variable value data structure in a slightly different way
    // we call the routine EvaluateExpression and examine the new bool argument for fatal errors.
    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node 1;",

        "EnergyManagementSystem:Actuator,",
        "TempSetpoint1,          !- Name",
        "Test node 1,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Setpoint;    !- Actuated Component Control Type",

        "EnergyManagementSystem:Program,",
        "SetNodeSetpointTest,",
        "Set TempSetpoint1 = testGlobalVar;"

        "EnergyManagementSystem:Program,",
        "SetGlobalValue,",
        "SET testGlobalVar = 21.0;"

        "EnergyManagementSystem:GlobalVariable, ",
        "testGlobalVar;"

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test Program Manager 1,  !- Name",
        "BeginNewEnvironment,  !- EnergyPlus Model Calling Point",
        "SetNodeSetpointTest;  !- Program Name 1",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test Program Manager 2,  !- Name",
        "BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "SetGlobalValue;  !- Program Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);

    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;
    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    // Expect the variable to not yet be initialized, call EvaluateExpression and check argument

    ErlValueType ReturnValue;
    bool seriousErrorFound = false;
    state->dataEMSMgr->FinishProcessingUserInput = false;
    ReturnValue = RuntimeLanguageProcessor::EvaluateExpression(
        *state,
        state->dataRuntimeLang->ErlStack(Util::FindItemInList("SETNODESETPOINTTEST", state->dataRuntimeLang->ErlStack)).Instruction(1).Argument2,
        seriousErrorFound); // we just check the logic and don't throw the fatal errors.
    EXPECT_TRUE(seriousErrorFound);

    // next run a small program that sets the global variable value
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());
    // now check that it worked, should stay false
    seriousErrorFound = false;
    ReturnValue = RuntimeLanguageProcessor::EvaluateExpression(
        *state,
        state->dataRuntimeLang->ErlStack(Util::FindItemInList("SETNODESETPOINTTEST", state->dataRuntimeLang->ErlStack)).Instruction(1).Argument2,
        seriousErrorFound);
    EXPECT_FALSE(seriousErrorFound);
}

TEST_F(EnergyPlusFixture, TestEMSVariableInitAfterRef1)
{
    // test for #11360 - EMS variable initialized after reference, outside of ManageSimulation
    std::string const idf_objects = delimited_string({

        "EnergyManagementSystem:Program,",
        "    ev_discharge_program,                          !- Name",
        "    Set power_mult = site_temp_adj,                !- Program Line 1",
        "    Set site_temp_adj = 0.1;                       !- Program Line 2",

        "EnergyManagementSystem:ProgramCallingManager,",
        "    ev_discharge_pcm,              !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    ev_discharge_program;          !- Program Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "site_temp_adj", 1);
    EXPECT_EQ(internalVarNum, 0);

    bool anyRan;
    EXPECT_TRUE(state->dataEMSMgr->GetEMSUserInput);
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "site_temp_adj", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_FALSE(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.initialized);

    EXPECT_FALSE(state->dataEMSMgr->GetEMSUserInput);
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyRan, ObjexxFCL::Optional_int_const());

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "site_temp_adj", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_FALSE(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.initialized);

    EXPECT_FALSE(state->dataEMSMgr->GetEMSUserInput);
    ASSERT_THROW(EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const()),
                 EnergyPlus::FatalError);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "site_temp_adj", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_FALSE(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.initialized);

    // Expect the variable to not yet be initialized, call EvaluateExpression and check argument
    bool seriousErrorFound = false;
    ErlValueType ReturnValue = RuntimeLanguageProcessor::EvaluateExpression(
        *state,
        state->dataRuntimeLang->ErlStack(Util::FindItemInList("EV_DISCHARGE_PROGRAM", state->dataRuntimeLang->ErlStack)).Instruction(1).Argument2,
        seriousErrorFound);
    EXPECT_TRUE(seriousErrorFound);

    const std::string expected_error = delimited_string({
        "   ** Severe  ** Problem found in EMS EnergyPlus Runtime Language.",
        "   **   ~~~   ** Erl program name: EV_DISCHARGE_PROGRAM",
        "   **   ~~~   ** Erl program line number: 1",
        "   **   ~~~   ** Erl program line text: SET POWER_MULT = SITE_TEMP_ADJ",
        "   **   ~~~   ** Error message:  *** Error: EvaluateExpression: Variable = 'SITE_TEMP_ADJ' used in expression has not been initialized! "
        "*** ",
        "   **   ~~~   **  Environment=, at Simulation time= 00:-15 - 00:00",
        "   **  Fatal  ** Previous EMS error caused program termination.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Problem found in EMS EnergyPlus Runtime Language.",
    });

    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, TestEMSVariableInitAfterRef2)
{
    // test for #11360 - EMS variable initialized after reference, within ManageSimulation
    std::string const idf_objects = delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",

        "RunPeriod,",
        "    Run Period 1,            !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    2007,                    !- Begin Year",
        "    1,                       !- End Month",
        "    1,                       !- End Day of Month",
        "    2007,                    !- End Year",
        "    Monday,                  !- Day of Week for Start Day",
        "    No,                      !- Use Weather File Holidays and Special Days",
        "    No,                      !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    Yes,                     !- Run Simulation for Weather File Run Periods",
        "    ,                        !- Do HVAC Sizing Simulation for Sizing Periods",
        "    ;                        !- Maximum Number of HVAC Sizing Simulation Passes",

        "Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "Material,",
        "    Concrete Block,          !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.3805070,               !- Conductivity {W/m-K}",
        "    608.7016,                !- Density {kg/m3}",
        "    836.8000;                !- Specific Heat {J/kg-K}",

        "Construction,",
        "    ConcConstruction,        !- Name",
        "    Concrete Block;          !- Outside Layer",

        "BuildingSurface:Detailed,"
        "    Wall,                    !- Name",
        "    Wall,                    !- Surface Type",
        "    ConcConstruction,        !- Construction Name",
        "    Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,0.000000,10.00000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,0.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,0.000000,10.00000;  !- X,Y,Z ==> Vertex 4 {m}",

        "Zone,"
        "    Zone,                    !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    6.000000,                !- X Origin {m}",
        "    6.000000,                !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "EnergyManagementSystem:Program,",
        "    ev_discharge_program,                          !- Name",
        "    Set power_mult = site_temp_adj,                !- Program Line 1",
        "    Set site_temp_adj = 0.1;                       !- Program Line 2",

        "EnergyManagementSystem:ProgramCallingManager,",
        "    ev_discharge_pcm,              !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    ev_discharge_program;          !- Program Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "weather/USA_CO_Golden-NREL.724666_TMY3.epw";

    int internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "site_temp_adj", 1);
    EXPECT_EQ(internalVarNum, 0);

    EXPECT_TRUE(state->dataEMSMgr->GetEMSUserInput);
    ASSERT_THROW(SimulationManager::ManageSimulation(*state), EnergyPlus::FatalError);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "site_temp_adj", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_FALSE(state->dataRuntimeLang->ErlVariable(internalVarNum).Value.initialized);
}

TEST_F(EnergyPlusFixture, EMSManager_CheckIfAnyEMS_OutEMS)
{
    std::string const idf_objects = delimited_string({
        "  Output:EnergyManagementSystem,                                                                ",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting                      ",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting             ",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level                         ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    CheckIfAnyEMS(*state);
    EXPECT_TRUE(state->dataGlobal->AnyEnergyManagementSystemInModel);
}

TEST_F(EnergyPlusFixture, EMSManager_TestFuntionCall)
{
    // test to demonstrate accurate function calls.

    std::string const idf_objects = delimited_string({

        "Curve:Quadratic,",
        "  TestCurve,       !- Name",
        "  0.8,             !- Coefficient1 Constant",
        "  0.2,             !- Coefficient2 x",
        "  0.0,             !- Coefficient3 x**2",
        "  0.5,             !- Minimum Value of x",
        "  1.5;             !- Maximum Value of x",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set Var1 = @Round 2.1,",
        "set Var2 = @Mod 7 3,",
        "set Var3 = @Sin 0.785398163,", // 45 degrees
        "set Var4 = @Cos 0.785398163,", // 45 degrees
        "set Var5 = @ArcCos 0.70710678090550916,",
        "set Var6 = @ArcSin 0.70710678090550916,",
        "set Var7 = @DegToRad 45.0,",
        "set Var8 = @RadToDeg 0.785398163,",
        "set Var9 = @Exp 1.0,",
        "set Var10 = @Ln 2.0,",
        "set Var11 = @Max 0.5 1.5,",
        "set Var12 = @Min 0.5 1.5,",
        "set Var13 = @Abs 1.3,", // -3.1 does not work, (-3.1) does not work, "set absNum = -3.1" and use @Abs absNum does work if EMS line of code
                                 // occurs before this line
        "set Var14 = @SEEDRANDOM 2.65,",
        "set Var15 = @RANDOMUNIFORM 0.0 1.0,",
        "set Var16 = @RANDOMNORMAL 1.5 0.5 0.75 2.25,", // negative number does not get processed correctly
        "set Var17 = @RhoAirFnPbTdbW 101325.0 30.0 0.01,",
        "set Var18 = @CpAirFnW 0.01,",
        "set Var19 = @HfgAirFnWTdb 0.01 30.0,",
        "set Var20 = @HgAirFnWTdb 0.01 30.0,",
        "set Var21 = @TdpFnTdbTwbPb 30.0 16.0 101325.0,",
        "set Var22 = @TdpFnWPb 0.01 101325.0,",
        "set Var23 = @HFnTdbW 30.0 0.01,",
        "set Var24 = @HFnTdbRhPb 30.0 0.5 101325.0,",
        "set Var25 = @TdbFnHW 30000.0 0.01,",
        "set Var26 = @RhovFnTdbRh 30.0 0.5,",
        "set Var27 = @RhovFnTdbRhLBnd0C 30.0 0.5,",
        "set Var28 = @RhovFnTdbWPb 30.0 0.01 101325.0,",
        "set Var29 = @RhFnTdbRhov 30.0 0.01,",
        "set Var30 = @RhFnTdbRhovLBnd0C 30.0 0.01,",
        "set Var31 = @RhFnTdbWPb 30.0 0.01 101325.0,",
        "set Var32 = @TwbFnTdbWPb 30.0 0.01 101325.0,",
        "set Var33 = @VFnTdbWPb 30.0 0.01 101325.0,",
        "set Var34 = @WFnTdpPb 16.0 101325.0,",
        "set Var35 = @WFnTdbH 20.0 30000.0,",
        "set Var36 = @WFnTdbTwbPb 30.0 16.0 101325.0,",
        "set Var37 = @WFnTdbRhPb 30.0 0.5 101325.0,", // this function implementation looks suspicious given it takes up to 4 arguments (CalledFrom is
                                                      // 4th)
        "set Var38 = @PsatFnTemp 30.0,",
        "set Var39 = @TsatFnHPb 30000.0 101325.0,",
        "set Var40 = @CpCW 30.0,",
        "set Var41 = @CpHW 60.0,",
        "set Var42 = @RhoH2O 20.0,",
        "set Var43 = @SEVEREWARNEP 0.0,",
        "set Var44 = @WARNEP 0.0,",
        "set Var45 = @TRENDVALUE Variable_Trend1 1,",
        "set Var46 = @TRENDAVERAGE Variable_Trend2 4,",
        "set Var47 = @TRENDMAX Variable_Trend3 4,",
        "set Var48 = @TRENDMIN Variable_Trend4 4,",
        "set Var49 = @TRENDDIRECTION Variable_Trend5 4,",
        "set Var50 = @TRENDSUM Variable_Trend6 4,",
        "set Var51 = @CURVEVALUE 1 0.75,",
        "set absNum = -3.1,",
        "set Var53 = @Abs absNum,", // -3.1 does not work, (-3.1) does not work, "set absNum = -3.1" and use @Abs absNum does work if EMS "set absNum"
                                    // line of code occurs before this line
        "Run Test_Trend_Variables;"

        //			"set Var54 = @TsatFnPb 0.0,", // not public in PsycRoutines so not available to EMS (commented out at line 2397 of
        // RuntimeLanguageProcessor.cc)
        //			"set Var55 = @FATALHALTEP 0.0,", // terminates program, not unit test friendly (but it was also verified here that it
        // halts program)

        "EnergyManagementSystem:GlobalVariable,",
        "  argTrendValue;       !- Erl Variable 1 Name",

        "EnergyManagementSystem:TrendVariable,",
        "  Variable_Trend1,     !- Name",
        "  argTrendValue,       !- EMS Variable Name",
        "  4;                   !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:TrendVariable,",
        "  Variable_Trend2,     !- Name",
        "  argTrendValue,       !- EMS Variable Name",
        "  4;                   !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:TrendVariable,",
        "  Variable_Trend3,     !- Name",
        "  argTrendValue,       !- EMS Variable Name",
        "  4;                   !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:TrendVariable,",
        "  Variable_Trend4,     !- Name",
        "  argTrendValue,       !- EMS Variable Name",
        "  4;                   !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:TrendVariable,",
        "  Variable_Trend5,     !- Name",
        "  argTrendValue,       !- EMS Variable Name",
        "  4;                   !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:TrendVariable,",
        "  Variable_Trend6,     !- Name",
        "  argTrendValue,       !- EMS Variable Name",
        "  4;                   !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:Subroutine,",
        "  Test_Trend_Variables,                             !- Name",
        "  Set TrendVal = @TrendValue Variable_Trend1 1,     !- Program Line 1",
        "  SET argTrendValue = TrendVal,                     !- Program Line 2",
        "  Set TrendVal = @TrendAverage Variable_Trend2 4,   !- Program Line 3",
        "  SET argTrendAvg = TrendVal,                       !- Program Line 4",
        "  Set TrendVal = @TrendMax Variable_Trend3 4,       !- Program Line 5",
        "  SET argTrendMax = TrendVal,                       !- Program Line 6",
        "  Set TrendVal = @TrendMin Variable_Trend4 4,       !- Program Line 7",
        "  SET argTrendMin = TrendVal,                       !- Program Line 8",
        "  Set TrendVal = @TrendDirection Variable_Trend5 4, !- Program Line 9",
        "  SET argTrendDirection = TrendVal,                 !- Program Line 10",
        "  Set TrendVal = @TrendSum Variable_Trend6 4,       !- Program Line 11",
        "  SET argTrendSum = TrendVal;                       !- Program Line 12",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepZone = 0.25;
    state->init_state(*state);

    EMSManager::CheckIfAnyEMS(*state); // get EMS input
    state->dataEMSMgr->FinishProcessingUserInput = true;

    // Does GetCurveInput have to be called after CheckIfAnyEMS?
    // bool ErrorsFound(false);
    // Curve::GetCurveInputData(*state, ErrorsFound); // process curve for use with EMS
    // EXPECT_FALSE(ErrorsFound);

    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::HVACIterationLoop, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyRan);

    for (int i = 1; i <= 6; ++i) {
        state->dataRuntimeLang->TrendVariable(i).TrendValARR(1) = 1.1; // initialize history for trend variables
        state->dataRuntimeLang->TrendVariable(i).TrendValARR(2) = 2.2;
        state->dataRuntimeLang->TrendVariable(i).TrendValARR(3) = 3.3;
        state->dataRuntimeLang->TrendVariable(i).TrendValARR(4) = 4.4;
    }

    EMSManager::ManageEMS(*state,
                          EMSManager::EMSCallFrom::HVACIterationLoop,
                          anyRan,
                          ObjexxFCL::Optional_int_const()); // process trend functions again using above data
    EXPECT_TRUE(anyRan);

    int offset = 28; // first 27 values in ErlExpression() are constant and built-in variables
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(1).Operator, ErlFunc::Round);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(1).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(1).Operand.size(), 1u);
    int index = 1 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR1");
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Value.Number, 2.0); // round(2.1)

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(2).Operator, ErlFunc::Mod);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(2).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(2).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(2).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 2 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR2");
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1.0); // mod( 7, 3 )

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(3).Operator, ErlFunc::Sin);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(3).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(3).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(3).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 3 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR3");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.70710678, 0.00000001); // Sin(45) or Sin(0.7854) = 0.707107

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(4).Operator, ErlFunc::Cos);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(4).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(4).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(4).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 4 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR4");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.70710678, 0.00000001); // Cos(45) or Cos(0.7854) = 0.707107

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(5).Operator, ErlFunc::ArcCos);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(5).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(5).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(5).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 5 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR5");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.78539816, 0.00000001); // ArcCos(Cos(45)) = 0.7854 rad

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(6).Operator, ErlFunc::ArcSin);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(6).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(6).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(6).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 6 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR6");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.78539816, 0.00000001); // ArcSin(Sin(45)) = 0.7854 rad

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(7).Operator, ErlFunc::DegToRad);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(7).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(7).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(7).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 7 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR7");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.78539816, 0.00000001); // DegToRad(45) = 0.7854 rad

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(8).Operator, ErlFunc::RadToDeg);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(8).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(8).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(8).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 8 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR8");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 45.0, 0.0000001); // RadToDeg(0.7854 rad) = 45 deg

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(9).Operator, ErlFunc::Exp);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(9).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(9).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(9).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 9 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR9");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 2.718281828, 0.00000001); // e^1 = 2.71828

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(10).Operator, ErlFunc::Ln);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(10).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(10).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(10).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 10 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR10");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.69314718, 0.00000001); // e^1 = 0.693147

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(11).Operator, ErlFunc::Max);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(11).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(11).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(11).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 11 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR11");
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1.5); // max(0.5, 1.5) = 1.5

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(12).Operator, ErlFunc::Min);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(12).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(12).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(12).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 12 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR12");
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.5); // min(0.5, 1.5) = 0.5

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(13).Operator, ErlFunc::ABS);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(13).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(13).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(13).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 13 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR13");
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1.3); // abs(1.3) = 1.3

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(14).Operator, ErlFunc::RandSeed);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(14).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(14).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(14).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 14 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR14");
    // seed may differ by processor, don't test seed generator
    // EXPECT_EQ( DataRuntimeLanguage::ErlVariable( index ).Value.Number, 2.0 ); // @SeedRandom( 2.65 )

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(15).Operator, ErlFunc::RandU);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(15).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(15).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(15).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 15 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR15");
    // don't test random number generator
    // EXPECT_NEAR( DataRuntimeLanguage::ErlVariable( index ).Value.Number, 0.148876574, 0.00000001 ); // @RANDOMUNIFORM 0.0 1.0

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(16).Operator, ErlFunc::RandG);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(16).NumOperands, 4);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(16).Operand.size(), 4u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(16).Operand(4).Type, Value::Number); // argument was passed to EMS function
    index = 16 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR16");
    // don't test random number generator
    // EXPECT_NEAR( DataRuntimeLanguage::ErlVariable( index ).Value.Number, 1.30797328, 0.00000001 ); // @RANDOMNORMAL 1.5 0.5 0.75 2.25 (mean, std,
    // min, max)

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(17).Operator, ErlFunc::RhoAirFnPbTdbW);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(17).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(17).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(17).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 17 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR17");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1.146173145, 0.00000001); // RhoAirFnPbTdbW 101325.0 30.0 0.01 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(18).Operator, ErlFunc::CpAirFnW);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(18).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(18).Operand.size(), 1u);
    index = 18 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR18");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1023.42949999999, 0.00000001); // CpAirFnW 0.01 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(19).Operator, ErlFunc::HfgAirFnWTdb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(19).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(19).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(19).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 19 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR19");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 2431308.50000000, 0.00000001); // HfgAirFnWTdb 0.01 30.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(20).Operator, ErlFunc::HgAirFnWTdb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(20).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(20).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(20).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 20 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR20");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 2556708.50000000, 0.00000001); // HgAirFnWTdb 0.01 30.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(21).Operator, ErlFunc::TdpFnTdbTwbPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(21).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(21).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(21).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 21 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR21");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 5.573987554, 0.00000001); // TdpFnTdbTwbPb 30.0 16.0 101325 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(22).Operator, ErlFunc::TdpFnWPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(22).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(22).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(22).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 22 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR22");                            // verified at sugartech site as 14.0439
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 14.044515576, 0.00000001); // TdpFnWPb 0.01 101325 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(23).Operator, ErlFunc::HFnTdbW);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(23).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(23).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(23).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 23 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR23");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 55712.28500000, 0.00000001); // HFnTdbW 30.0 0.01 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(24).Operator, ErlFunc::HFnTdbRhPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(24).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(24).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(24).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 24 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR24");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 64177.426349195, 0.00000001); // HFnTdbRhPb 30.0 0.5 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(25).Operator, ErlFunc::TdbFnHW);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(25).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(25).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(25).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 25 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR25");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 4.876349567, 0.00000001); // TdbFnHW 30000.0 0.01 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(26).Operator, ErlFunc::RhovFnTdbRh);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(26).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(26).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(26).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 26 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR26");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.015174171, 0.00000001); // RhovFnTdbRh 30.0 0.5 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(27).Operator,
                   ErlFunc::RhovFnTdbRhLBnd0C); // fails before #5284, returns FuncRhovFnTdbRh( 41 )
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(27).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(27).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(27).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 27 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR27");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.015156240, 0.00000001); // RhovFnTdbRhLBnd0C 30.0 0.5 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(28).Operator, ErlFunc::RhovFnTdbWPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(28).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(28).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(28).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 28 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name,
              "VAR28"); // http://www.gribble.org/cycling/air_density.html 30 C db, 1013.25 hPa, 16 C dp = 0.011565 g/m3
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number,
                0.011459487,
                0.00000001); // RhovFnTdbWPb 30.0 0.01 101325.0 = ** this and previous 2 numbers seem very different **

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(29).Operator, ErlFunc::RhFnTdbRhov);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(29).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(29).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(29).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 29 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR29");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.3295072808, 0.00000001); // RhFnTdbRhov 30.0 0.01 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(30).Operator,
                   ErlFunc::RhFnTdbRhovLBnd0C); // fails before #5284, returns int const FuncRhFnTdbRhov( 44 )
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(30).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(30).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(30).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 30 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR30");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.3298971165, 0.00000001); // RhFnTdbRhovLBnd0C 30.0 0.01 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(31).Operator, ErlFunc::RhFnTdbWPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(31).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(31).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(31).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 31 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR31");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.377598442, 0.00000001); // RhFnTdbWPb 30.0 0.01 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(32).Operator, ErlFunc::TwbFnTdbWPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(32).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(32).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(32).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 32 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name,
              "VAR32"); // verified at sugartech site using 30 C db and 0.01 kg/kg = 19.60536624685125 C
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 19.60933534, 0.00000001); // TwbFnTdbWPb 30.0 0.01 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(33).Operator, ErlFunc::VFnTdbWPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(33).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(33).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(33).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 33 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name,
              "VAR33"); // http://www.sugartech.co.za/psychro/ 30 C db, 14.043895 dp = 0.8432375 m3/kg
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.873152783, 0.00000001); // VFnTdbWPb 30.0 0.01 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(34).Operator, ErlFunc::WFnTdpPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(34).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(34).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(34).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 34 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR34");                            // verified at sugartech site as 0.011366881 kg/kg
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.0113664167, 0.00000001); // WFnTdpPb 16.0 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(35).Operator, ErlFunc::WFnTdbH);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(35).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(35).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(35).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 35 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name,
              "VAR35"); // http://www.sugartech.co.za/psychro/ 20 C db, 0.42830288 C dp, 30000 H = 0.00389466 kg/kg
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.00390178711, 0.00000001); // WFnTdbH 20.0 30000.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(36).Operator, ErlFunc::WFnTdbTwbPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(36).NumOperands, 3);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(36).Operand.size(), 3u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(36).Operand(3).Type, Value::Number); // argument was passed to EMS function
    index = 36 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR36"); // http://www.sugartech.co.za/psychro/ 30 C db, 16 C wb = 0.00559757 kg/kg
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.005624362, 0.00000001); // WFnTdbTwbPb 30.0 16.0 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(37).Operator, ErlFunc::WFnTdbRhPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(37).NumOperands, 4); // why is this 4?
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(37).Operand.size(), 4u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(37).Operand(3).Type, Value::Number); // argument was passed to EMS function
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(37).Operand(4).Type, Value::Null);   // 4th argument not passed to EMS function
    index = 37 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR37"); // http://www.sugartech.co.za/psychro/ 30 C db, 50% rh = 0.01331149 kg/kg
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.0133109528, 0.00000001); // WFnTdbRhPb 30.0 0.5 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(38).Operator, ErlFunc::PsatFnTemp);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(38).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(38).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(38).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 38 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name,
              "VAR38"); // http://www.sugartech.co.za/psychro/ 30 C db, 100% rh = 42.46019 mbar = 4246.019 Pa
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 4246.030243592, 0.00000001); // PsatFnTemp 30.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(39).Operator, ErlFunc::TsatFnHPb);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(39).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(39).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(39).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 39 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR39"); // http://www.sugartech.co.za/psychro/ 10.303 C db gives H = 29999.9999
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 10.318382617, 0.00000001); // TsatFnHPb 30000.0 101325.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(40).Operator, ErlFunc::CpCW);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(40).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(40).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(40).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 40 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR40");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 4180.0, 0.00000001); // CpCW 30.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(41).Operator, ErlFunc::CpHW);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(41).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(41).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(41).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 41 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR41");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 4180.0, 0.00000001); // CpHW 60.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(42).Operator, ErlFunc::RhoH2O);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(42).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(42).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(42).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 42 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR42");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 998.2331862652, 0.00000001); // RhoH2O 60.0 =

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(43).Operator, ErlFunc::SevereWarnEp);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(43).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(43).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(43).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 43 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR43");

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(44).Operator, ErlFunc::WarnEp);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(44).NumOperands, 1);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(44).Operand.size(), 1u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(44).Operand(1).Type, Value::Number); // argument was passed to EMS function
    index = 44 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR44");

    // all trend variables hold 4 values: 1.1, 2.2, 3.3, 4.4
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(45).Operator, ErlFunc::TrendValue);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(45).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(45).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(45).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 45 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR45");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1.1, 0.00000001); // TrendValue Variable_Trend1 1

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(46).Operator, ErlFunc::TrendAverage);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(46).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(46).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(46).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 46 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR46");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 2.75, 0.00000001); // TrendAverage Variable_Trend2 4

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(47).Operator, ErlFunc::TrendMax);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(47).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(47).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(47).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 47 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR47");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 4.4, 0.00000001); // TrendMax Variable_Trend3 4

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(48).Operator, ErlFunc::TrendMin);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(48).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(48).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(48).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 48 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR48");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 1.1, 0.00000001); // TrendMin Variable_Trend4 4

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(49).Operator, ErlFunc::TrendDirection);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(49).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(49).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(49).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 49 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR49");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number,
                -4.4,
                0.00000001); // TrendDirection Variable_Trend5 4 (-1.1 per 0.25 hrs = -4.4/hr)

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(50).Operator, ErlFunc::TrendSum);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(50).NumOperands, 2);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(50).Operand.size(), 2u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(50).Operand(2).Type, Value::Number); // argument was passed to EMS function
    index = 50 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR50");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 11.0, 0.00000001); // TrendSum Variable_Trend6 4

    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(51).Operator, ErlFunc::CurveValue);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(51).NumOperands, 6);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(51).Operand.size(), 6u);
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(51).Operand(2).Type, Value::Number); // argument was passed to EMS function
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(51).Operand(3).Type, Value::Null);   // 3rd argument not passed to EMS function
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(51).Operand(4).Type, Value::Null);   // 4th argument not passed to EMS function
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(51).Operand(5).Type, Value::Null);   // 5th argument not passed to EMS function
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(51).Operand(6).Type, Value::Null);   // 6th argument not passed to EMS function
    index = 51 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR51");
    EXPECT_NEAR(state->dataRuntimeLang->ErlVariable(index).Value.Number, 0.95, 0.00000001); // CurveValue 0.75 = 0.95

    // test these functions as needed to verify results

    // test ABS using negative number
    EXPECT_ENUM_EQ(state->dataRuntimeLang->ErlExpression(53).Operator, ErlFunc::ABS);
    EXPECT_EQ(state->dataRuntimeLang->ErlExpression(53).NumOperands, 1);
    index = 53 + offset;
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Name, "VAR53");
    EXPECT_EQ(state->dataRuntimeLang->ErlVariable(index).Value.Number, 3.1); // set absNum = -3.1, abs(absNum) = 3.1

    //		EXPECT_EQ( DataRuntimeLanguage::ErlExpression( 54 ).Operator, FuncTsatFnPb ); // not public in PsycRoutines so not available to EMS
    //(commented out at line 2397 of RuntimeLanguageProcessor.cc) 		EXPECT_EQ( DataRuntimeLanguage::ErlExpression( 55 ).Operator,
    // FuncFatalHaltEp ); // terminates program, not unit test friendly
}

TEST_F(EnergyPlusFixture, EMSManager_TestOANodeAsActuators)
{
    state->init_state(*state);

    //    EMSActuatorAvailable.allocate(100);
    state->dataLoopNodes->NumOfNodes = 3;
    state->dataRuntimeLang->numActuatorsUsed = 3;
    state->dataLoopNodes->Node.allocate(3);
    state->dataLoopNodes->NodeID.allocate(3);
    state->dataRuntimeLang->EMSActuatorUsed.allocate(3);
    state->dataOutAirNodeMgr->NumOutsideAirNodes = 3;
    state->dataOutAirNodeMgr->OutsideAirNodeList.allocate(3);

    state->dataLoopNodes->NodeID(1) = "Node1";
    state->dataLoopNodes->NodeID(2) = "Node2";
    state->dataLoopNodes->NodeID(3) = "Node3";

    state->dataLoopNodes->Node(1).TempSetPoint = 23.0;
    state->dataLoopNodes->Node(2).TempSetPoint = 23.0;
    state->dataLoopNodes->Node(3).TempSetPoint = 23.0;

    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;

    state->dataOutAirNodeMgr->OutsideAirNodeList(1) = 1;
    state->dataOutAirNodeMgr->OutsideAirNodeList(2) = 2;
    state->dataOutAirNodeMgr->OutsideAirNodeList(3) = 3;
    state->dataRuntimeLang->EMSActuatorUsed(1).ComponentTypeName = "Outdoor Air System Node";
    state->dataRuntimeLang->EMSActuatorUsed(2).ComponentTypeName = "";
    state->dataRuntimeLang->EMSActuatorUsed(3).ComponentTypeName = "Outdoor Air System Node";
    state->dataRuntimeLang->EMSActuatorUsed(1).UniqueIDName = state->dataLoopNodes->NodeID(1);
    state->dataRuntimeLang->EMSActuatorUsed(2).UniqueIDName = state->dataLoopNodes->NodeID(2);
    state->dataRuntimeLang->EMSActuatorUsed(3).UniqueIDName = state->dataLoopNodes->NodeID(3);

    SetupNodeSetPointsAsActuators(*state);

    EXPECT_TRUE(state->dataLoopNodes->Node(1).IsLocalNode);
    EXPECT_FALSE(state->dataLoopNodes->Node(2).IsLocalNode);
    EXPECT_TRUE(state->dataLoopNodes->Node(3).IsLocalNode);
}
TEST_F(EnergyPlusFixture, EMSManager_TestWindowShadingControlExteriorScreenOption)
{
    state->init_state(*state);

    // #7586
    state->dataSurface->Surface.allocate(2);
    state->dataSurface->SurfaceWindow.allocate(2);
    state->dataSurface->surfShades.allocate(2);
    EnergyPlus::SurfaceGeometry::AllocateSurfaceWindows(*state, 2);
    state->dataConstruction->Construct.allocate(1);
    state->dataSurface->WindowShadingControl.allocate(2);
    state->dataDayltg->ZoneDaylight.allocate(1);
    state->dataSurface->Surface(1).Name = "Surface1";
    state->dataSurface->Surface(2).Name = "Surface2";
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(1).ExtBoundCond = DataSurfaces::ExternalEnvironment;
    state->dataSurface->Surface(2).ExtBoundCond = DataSurfaces::ExternalEnvironment;
    state->dataSurface->Surface(1).windowShadingControlList.push_back(1);
    state->dataSurface->Surface(2).windowShadingControlList.push_back(2);
    state->dataSurface->Surface(1).HasShadeControl = true;
    state->dataSurface->Surface(2).HasShadeControl = true;

    state->dataSurface->SurfWinHasShadeOrBlindLayer(1) = false;
    state->dataSurface->SurfWinHasShadeOrBlindLayer(2) = false;
    state->dataSurface->Surface(1).activeShadedConstruction = 1;
    state->dataSurface->Surface(2).activeShadedConstruction = 1;

    state->dataConstruction->Construct(1).Name = "Construction1";

    state->dataSurface->WindowShadingControl(1).ShadingType = DataSurfaces::WinShadingType::NoShade;
    state->dataSurface->WindowShadingControl(2).ShadingType = DataSurfaces::WinShadingType::ExtScreen;

    state->dataSurface->TotSurfaces = 2;

    state->dataSurface->Surface(1).activeWindowShadingControl =
        state->dataSurface->Surface(1).windowShadingControlList[SolarShading::selectActiveWindowShadingControlIndex(*state, 1)];
    state->dataSurface->Surface(2).activeWindowShadingControl =
        state->dataSurface->Surface(1).windowShadingControlList[SolarShading::selectActiveWindowShadingControlIndex(*state, 2)];

    SetupWindowShadingControlActuators(*state);

    EXPECT_FALSE(state->dataSurface->SurfWinShadingFlagEMSOn(2));
    EXPECT_EQ(state->dataSurface->SurfWinShadingFlagEMSValue(2), 0.0);

    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->space(1).WindowSurfaceFirst = 1;
    state->dataHeatBal->space(1).WindowSurfaceLast = 2;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataGlobal->NumOfZones = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataSurface->SurfWinShadingFlagEMSOn(2) = true;
    state->dataSurface->SurfWinShadingFlagEMSValue(2) = 1.0; // WinShadingType::IntShade
    SolarShading::WindowShadingManager(*state);
    EXPECT_ENUM_EQ(state->dataSurface->SurfWinShadingFlag(2), DataSurfaces::WinShadingType::IntShade);
}
TEST_F(EnergyPlusFixture, EMS_WeatherDataActuators)
{
    // GetNextEnvironment Will call ReadUserWeatherInput which calls inputProcessor, so let's use process_idf to create one Environment (Design Day)
    std::string const idf_objects = delimited_string({

        "Site:Location,",
        "   Atlanta Hartsfield Intl Ap_GA_USA Design_Conditions, !- Location Name",
        "     33.63,    !- Latitude {N + S - }",
        "    -84.43,    !- Longitude {W - E + }",
        "     -5.00,    !- Time Zone Relative to GMT {GMT + / -}",
        "    308.00;    !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    Atlanta Made Up Day,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    34.0,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.6,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    13.2,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    8,                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    5.0,                     !- Wind Speed {m/s}",
        "    180.0,                   !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    0.325,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    2.461;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",

        "EnergyManagementSystem:Actuator,",
        "OutDryBulb,              !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Outdoor Dry Bulb;        !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "OutDewPoint,             !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Outdoor Dew Point;       !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "OutRH,                   !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Outdoor Relative Humidity;  !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "DiffuseSolar,            !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Diffuse Solar;           !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "DirectSolar,              !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Direct Solar;        !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "WindSpeed,               !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Wind Speed;              !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "WindDirection,           !- Name",
        "Environment,             !- Actuated Component Unique Name",
        "Weather Data,            !- Actuated Component Type",
        "Wind Direction;          !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Dual Setpoint Test Manager,  !- Name",
        "BeginZoneTimestepBeforeSetCurrentWeather,  !- EnergyPlus Model Calling Point",
        "OverrideWeather;  !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "OverrideWeather,",
        "Set OutDryBulb = 50.0,",
        "Set OutDewPoint = 25.0,",
        "Set OutRH = 50.0,",
        "Set DiffuseSolar = 500.0,",
        "Set DirectSolar = 1000.0,",
        "Set WindSpeed = 5.5,",
        "Set WindDirection = 32.1;",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->LocationGathered = false;

    state->init_state(*state);

    EMSManager::CheckIfAnyEMS(*state);
    bool available = false;
    bool errorsFound = false;
    Weather::GetNextEnvironment(*state, available, errorsFound);
    ASSERT_FALSE(errorsFound);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    // Initialize all sorts of weather stuff
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->BeginDayFlag = true;
    Weather::ManageWeather(*state);

    EXPECT_NEAR(state->dataEnvrn->OutDryBulbTemp, 50.0, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->OutDewPointTemp, 25.0, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->OutRelHum, 50.0, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->DifSolarRad, 0.0, 0.000001);  // Sun is down
    EXPECT_NEAR(state->dataEnvrn->BeamSolarRad, 0.0, 0.000001); // Sun is down
    EXPECT_NEAR(state->dataEnvrn->WindSpeed, 5.5, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->WindDir, 32.1, 0.000001);

    state->dataGlobal->TimeStep = 3;
    state->dataGlobal->HourOfDay = 8;
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataGlobal->BeginDayFlag = false;
    Weather::ManageWeather(*state);

    EXPECT_NEAR(state->dataEnvrn->OutDryBulbTemp, 50.0, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->OutDewPointTemp, 25.0, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->OutRelHum, 50.0, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->DifSolarRad, 500.0, 0.000001);   // Sun is up
    EXPECT_NEAR(state->dataEnvrn->BeamSolarRad, 1000.0, 0.000001); // Sun is up
    EXPECT_NEAR(state->dataEnvrn->WindSpeed, 5.5, 0.000001);
    EXPECT_NEAR(state->dataEnvrn->WindDir, 32.1, 0.000001);
}
TEST_F(EnergyPlusFixture, EMS_TodayTomorrowFunctions)
{
    // GetNextEnvironment Will call ReadUserWeatherInput which calls inputProcessor, so let's use process_idf to create one Environment (Design Day)
    std::string const idf_objects = delimited_string({

        "Site:Location,",
        "   Atlanta Hartsfield Intl Ap_GA_USA Design_Conditions, !- Location Name",
        "     33.63,    !- Latitude {N + S - }",
        "    -84.43,    !- Longitude {W - E + }",
        "     -5.00,    !- Time Zone Relative to GMT {GMT + / -}",
        "    308.00;    !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    Atlanta Made Up Day,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    34.0,                    !- Maximum Dry-Bulb Temperature {C}",
        "    11.6,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    WetBulbProfileDefaultMultipliers,  !- Humidity Condition Type",
        "    13.2,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    8,                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    97620,                   !- Barometric Pressure {Pa}",
        "    5.0,                     !- Wind Speed {m/s}",
        "    180.0,                   !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAETau2017,           !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    0.325,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    2.461;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Dual Setpoint Test Manager,  !- Name",
        "BeginZoneTimestepBeforeSetCurrentWeather,  !- EnergyPlus Model Calling Point",
        "QueryWeather;  !- Program Name 1",

        "EnergyManagementSystem:GlobalVariable,",
        "TodayDryBulb;",

        "EnergyManagementSystem:Program,",
        "QueryWeather,",
        "Set iHour = 5,",
        "Set iTimeStep = 3,",
        "Set TodayRain = @TodayIsRain iHour iTimeStep,",
        "Set TodaySnow = @TodayIsSnow iHour iTimeStep,",
        "Set TodayDryBulb = @TodayOutDryBulbTemp iHour iTimeStep,",
        "Set TodayDewPoint = @TodayOutDewPointTemp iHour iTimeStep,",
        "Set TodayBaroPress = @TodayOutBaroPress iHour iTimeStep,",
        "Set TodayRelHum = @TodayOutRelHum iHour iTimeStep,",
        "Set TodayWindSpd = @TodayWindSpeed iHour iTimeStep,",
        "Set TodayWindDirect = @TodayWindDir iHour iTimeStep,",
        "Set TodaySkyT = @TodaySkyTemp iHour iTimeStep,",
        "Set TodayHorIR = @TodayHorizIRSky iHour iTimeStep,",
        "Set TodayBeamSol = @TodayBeamSolarRad iHour iTimeStep,",
        "Set TodayDifSol = @TodayDifSolarRad iHour iTimeStep,",
        "Set TodayAlb = @TodayAlbedo iHour iTimeStep,",
        "Set TodayPrecip = @TodayLiquidPrecip iHour iTimeStep,",
        "Set TomorrowRain = @TomorrowIsRain iHour iTimeStep,",
        "Set TomorrowSnow = @TomorrowIsSnow iHour iTimeStep,",
        "Set TomorrowDryBulb = @TomorrowOutDryBulbTemp iHour iTimeStep,",
        "Set TomorrowDewPoint = @TomorrowOutDewPointTemp iHour iTimeStep,",
        "Set TomorrowBaroPress = @TomorrowOutBaroPress iHour iTimeStep,",
        "Set TomorrowRelHum = @TomorrowOutRelHum iHour iTimeStep,",
        "Set TomorrowWindSpd = @TomorrowWindSpeed iHour iTimeStep,",
        "Set TomorrowWindDirect = @TomorrowWindDir iHour iTimeStep,",
        "Set TomorrowSkyT = @TomorrowSkyTemp iHour iTimeStep,",
        "Set TomorrowHorIR = @TomorrowHorizIRSky iHour iTimeStep,",
        "Set TomorrowBeamSol = @TomorrowBeamSolarRad iHour iTimeStep,",
        "Set TomorrowDifSol = @TomorrowDifSolarRad iHour iTimeStep,",
        "Set TomorrowAlb = @TomorrowAlbedo iHour iTimeStep,",
        "Set TomorrowPrecip = @TomorrowLiquidPrecip iHour iTimeStep;",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataWeather->LocationGathered = false;
    state->init_state(*state);

    EMSManager::CheckIfAnyEMS(*state);
    bool available = false;
    bool errorsFound = false;
    Weather::GetNextEnvironment(*state, available, errorsFound);
    ASSERT_FALSE(errorsFound);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    // Initialize all sorts of weather stuff
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->BeginDayFlag = true;
    Weather::ManageWeather(*state);

    // Note that operands for these functions are Hour (0:23) then Timestep
    // In the EMS code above, they are all using Hour = 5 and Timestep=3
    // But the arrays are stored as TodayXyz(timestep, hour) where hour is 1:24

    // TodayIsRain and TodayIsSnow are logicals, but the ems functions returns 0 or 1
    int internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayRain", 1);
    ASSERT_GT(internalVarNum, 0);
    bool rainTrueFalse = (state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number > 0.0);
    EXPECT_EQ(state->dataWeather->wvarsHrTsToday(3, 5 + 1).IsRain, rainTrueFalse);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodaySnow", 1);
    ASSERT_GT(internalVarNum, 0);
    bool snowTrueFalse = (state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number > 0.0);
    EXPECT_EQ(state->dataWeather->wvarsHrTsToday(3, 5 + 1).IsSnow, snowTrueFalse);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayDryBulb", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsToday(3, 5 + 1).OutDryBulbTemp, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayDewPoint", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsToday(3, 5 + 1).OutDewPointTemp, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayBaroPress", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsToday(3, 5 + 1).OutBaroPress, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayRelHum", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).OutRelHum, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayWindSpd", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).WindSpeed, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayWindDirect", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).WindDir, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodaySkyT", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).SkyTemp, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayHorIR", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).HorizIRSky, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayBeamSol", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsToday(3, 5 + 1).BeamSolarRad, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayDifSol", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).DifSolarRad, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayAlb", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsToday(3, 5 + 1).Albedo, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TodayPrecip", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsToday(3, 5 + 1).LiquidPrecip, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    // TodayIsRain and TodayIsSnow are logicals, but the ems functions returns 0 or 1
    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowRain", 1);
    ASSERT_GT(internalVarNum, 0);
    rainTrueFalse = (state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number > 0.0);
    EXPECT_EQ(state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).IsRain, rainTrueFalse);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowSnow", 1);
    ASSERT_GT(internalVarNum, 0);
    snowTrueFalse = (state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number > 0.0);
    EXPECT_EQ(state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).IsSnow, snowTrueFalse);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowDryBulb", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).OutDryBulbTemp, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowDewPoint", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).OutDewPointTemp, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowBaroPress", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).OutBaroPress, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowRelHum", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).OutRelHum, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowWindSpd", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).WindSpeed, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowWindDirect", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).WindDir, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowSkyT", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).SkyTemp, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowHorIR", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).HorizIRSky, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowBeamSol", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).BeamSolarRad, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowDifSol", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).DifSolarRad, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowAlb", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).Albedo, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);

    internalVarNum = RuntimeLanguageProcessor::FindEMSVariable(*state, "TomorrowPrecip", 1);
    ASSERT_GT(internalVarNum, 0);
    EXPECT_NEAR(
        state->dataWeather->wvarsHrTsTomorrow(3, 5 + 1).LiquidPrecip, state->dataRuntimeLang->ErlVariable(internalVarNum).Value.Number, 0.000001);
}

TEST_F(EnergyPlusFixture, EMS_ViewFactorToGround)
{
    std::string const idf_objects = delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",

        "SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "SizingPeriod:DesignDay,",
        "    SunnyWinterDay,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    5.0,                    !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    4.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",

        "Material,",
        "    Concrete Block,          !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.3805070,               !- Conductivity {W/m-K}",
        "    608.7016,                !- Density {kg/m3}",
        "    836.8000;                !- Specific Heat {J/kg-K}",

        "Construction,",
        "    ConcConstruction,        !- Name",
        "    Concrete Block;          !- Outside Layer",

        "WindowMaterial:SimpleGlazingSystem,",
        "    WindowMaterial,          !- Name",
        "    1.87374,                 !- U-Factor {W/m2-K}",
        "    0.45;                    !- Solar Heat Gain Coefficient",

        "Construction,",
        "    WindowConstruction 1,    !- Name",
        "    WindowMaterial;          !- Outside Layer",

        "FenestrationSurface:Detailed,",
        "    FenestrationSurface,     !- Name",
        "    Window,                  !- Surface Type",
        "    WindowConstruction 1,      !- Construction Name",
        "    Wall,                    !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.200000,0.000000,9.900000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.200000,0.000000,0.1000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "    9.900000,0.000000,0.1000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "    9.900000,0.000000,9.900000;  !- X,Y,Z ==> Vertex 4 {m}",

        "BuildingSurface:Detailed,"
        "    Wall,                    !- Name",
        "    Wall,                    !- Surface Type",
        "    ConcConstruction,        !- Construction Name",
        "    Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,0.000000,10.00000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,0.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,0.000000,10.00000;  !- X,Y,Z ==> Vertex 4 {m}",

        "BuildingSurface:Detailed,"
        "    Floor,                   !- Name",
        "    Floor,                   !- Surface Type",
        "    ConcConstruction,        !- Construction Name",
        "    Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,10.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.00000,10.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.00000,0.000000,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "Zone,"
        "    Zone,                    !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    6.000000,                !- X Origin {m}",
        "    6.000000,                !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "EnergyManagementSystem:Actuator,",
        "    south_window_view_factor_ground_act,  !- Name",
        "    FenestrationSurface,             !- Actuated Component Unique Name",
        "    Surface,                 !- Actuated Component Type",
        "    View Factor To Ground;   !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "    surface_south_window_view_factor_ground_act,  !- Name",
        "    Wall,     !- Actuated Component Unique Name",
        "    Surface,                 !- Actuated Component Type",
        "    View Factor To Ground;   !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "    window_view_factor_ground calling manager,  !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    window_view_factor_ground_adj;  !- Program Name 1",

        "EnergyManagementSystem:ProgramCallingManager,",
        "    surface_view_factor_ground calling manager,  !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    surface_view_factor_ground_adj;  !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "    window_view_factor_ground_adj,  !- Name",
        "    IF (DayOfYear >= 1) && (DayOfYear <= 120),  !- Program Line 1",
        "    Set south_window_view_factor_ground_act = 0.5 * 0.85,  !- Program Line 2",
        "    ELSEIF (DayOfYear > 120) && (DayOfYear <= 320),  !- A4",
        "    Set south_window_view_factor_ground_act = 0.5 * 0.7,  !- A5",
        "    ELSEIF (DayOfYear > 320) && (DayOfYear <= 365),  !- A6",
        "    Set south_window_view_factor_ground_act = 0.5 * 0.85,  !- A7",
        "    ENDIF;                   !- A8",

        "EnergyManagementSystem:Program,",
        "    surface_view_factor_ground_adj,  !- Name",
        "    IF (DayOfYear >= 1) && (DayOfYear <= 120),  !- Program Line 1",
        "    Set surface_south_window_view_factor_ground_act = 0.5 * 0.85,  !- Program Line 2",
        "    ELSEIF (DayOfYear > 120) && (DayOfYear <= 320),  !- A4",
        "    Set surface_south_window_view_factor_ground_act = 0.5 * 0.7,  !- A5",
        "    ELSEIF (DayOfYear > 320) && (DayOfYear <= 365),  !- A6",
        "    Set surface_south_window_view_factor_ground_act = 0.5 * 0.85,  !- A7",
        "    ENDIF;                   !- A8",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->Year = 2000;
    state->dataEnvrn->EndYear = 2000;

    SimulationManager::ManageSimulation(*state);

    int winSurfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    int wallSurfNum = Util::FindItemInList("WALL", state->dataSurface->Surface);
    bool anyRan;

    // Test 1 - Set day of year to 121
    state->dataGlobal->DayOfSim = 121; // avoid array bounds problem in RecKeepHeatBalance
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 11;
    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataEnvrn->DayOfYear = 121;

    HeatBalanceManager::ManageHeatBalance(*state);
    // For now, must call this twice in order to hit the BeginTimeStepBeforePredictor EMS calling point
    HeatBalanceManager::ManageHeatBalance(*state);

    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_EQ(state->dataSurface->Surface(winSurfNum).ViewFactorGround, 0.35);
    EXPECT_EQ(state->dataSurface->Surface(wallSurfNum).ViewFactorGround, 0.35);

    // Test 2 - Set day of year to 321
    state->dataGlobal->DayOfSim = 321; // avoid array bounds problem in RecKeepHeatBalance
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 11;
    state->dataEnvrn->Month = 11;
    state->dataEnvrn->DayOfMonth = 17;
    state->dataEnvrn->DayOfYear = 321;

    HeatBalanceManager::ManageHeatBalance(*state);
    // For now, must call this twice in order to hit the BeginTimeStepBeforePredictor EMS calling point
    HeatBalanceManager::ManageHeatBalance(*state);

    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());
    EXPECT_EQ(state->dataSurface->Surface(winSurfNum).ViewFactorGround, 0.425);
    EXPECT_EQ(state->dataSurface->Surface(wallSurfNum).ViewFactorGround, 0.425);
}

TEST_F(EnergyPlusFixture, EMSManager_TrendValue_to_Actuator)
{
    // Test for #10279 - Make sure that assigning the result of a TendVariable @TrendValue results in proper actuator behavior
    std::string const idf_objects = delimited_string({
        "Schedule:Constant,",
        "  Actuated Schedule Direct,               !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  18;                                     !- Hourly Value",

        "Schedule:Constant,",
        "  Actuated Schedule Indirect,             !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  18;                                     !- Hourly Value",

        "EnergyManagementSystem:GlobalVariable,",
        "  argTrendValue,                          !- Erl Variable Name 1",
        "  resultValue1;                           !- Erl Variable Name 2",

        "EnergyManagementSystem:TrendVariable,",
        "  Trend_argTrendValue,                    !- Name",
        "  argTrendValue,                          !- EMS Variable Name",
        "  12;                                     !- Number of Timesteps to be Logged",

        "EnergyManagementSystem:Actuator,",
        "  actuator_sch_Direct,                    !- Name",
        "  Actuated Schedule Direct,               !- Actuated Component Unique Name",
        "  Schedule:Constant,                      !- Actuated Component Type",
        "  Schedule Value;                         !- Actuated Component Control Type",

        "EnergyManagementSystem:Actuator,",
        "  actuator_sch_Indirect,                  !- Name",
        "  Actuated Schedule Indirect,             !- Actuated Component Unique Name",
        "  Schedule:Constant,                      !- Actuated Component Type",
        "  Schedule Value;                         !- Actuated Component Control Type",

        "EnergyManagementSystem:Program,",
        "  sch_test,                               !- Name",
        "  SET actuator_sch_Direct   = @TrendValue Trend_argTrendValue 1,  !- Program Line 1",
        "  SET        resultValue1   = @TrendValue Trend_argTrendValue 1,  !- Program Line 2",
        "  SET actuator_sch_Indirect = resultValue1,                       !- Program Line 3",
        "  SET         argTrendValue = MINUTE;                             !- Program Line 4",

        "EnergyManagementSystem:ProgramCallingManager,",
        "  sch_test_pcm,                           !- Name",
        "  BeginTimestepBeforePredictor,           !- EnergyPlus Model Calling Point",
        "  sch_test;                               !- Program Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Apparently this needs to be called before GetScheduleData, i.e., init_state()
    EMSManager::CheckIfAnyEMS(*state); // get EMS input
    EXPECT_TRUE(state->dataGlobal->AnyEnergyManagementSystemInModel);

    state->dataGlobal->TimeStepZone = 0.25;

    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;
    state->init_state(*state);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    EXPECT_EQ(4, state->dataSched->schedules.size());

    auto *schedDirect = Sched::GetSchedule(*state, "ACTUATED SCHEDULE DIRECT");
    auto *schedIndirect = Sched::GetSchedule(*state, "ACTUATED SCHEDULE INDIRECT");
    EXPECT_NE(schedDirect, nullptr);
    EXPECT_NE(schedIndirect, nullptr);

    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 23;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 24;
    state->dataGlobal->CurrentTime = 24.0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::UpdateScheduleVals(*state);

    EXPECT_EQ(18.0, schedDirect->currentVal);
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);
    EXPECT_EQ(18.0, schedIndirect->currentVal);
    EXPECT_FALSE(schedIndirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedIndirect->EMSVal);

    EMSManager::InitEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor);

    constexpr int numBuiltInErlVars = 27; // 27 ErlVariables are constant and built-in variables
    constexpr std::array<std::string_view, numBuiltInErlVars> builtInErlVarNames = {
        "NULL",
        "FALSE",
        "TRUE",
        "OFF",
        "ON",
        "PI",
        "TIMESTEPSPERHOUR",
        "YEAR",
        "CALENDARYEAR",
        "MONTH",
        "DAYOFMONTH",
        "DAYOFWEEK",
        "DAYOFYEAR",
        "HOUR",
        "TIMESTEPNUM",
        "MINUTE",
        "HOLIDAY",
        "DAYLIGHTSAVINGS",
        "CURRENTTIME",
        "SUNISUP",
        "ISRAINING",
        "SYSTEMTIMESTEP",
        "ZONETIMESTEP",
        "CURRENTENVIRONMENT",
        "ACTUALDATEANDTIME",
        "ACTUALTIME",
        "WARMUPFLAG",
    };

    constexpr int expectedUserErlVarNums = 4;
    EXPECT_EQ(numBuiltInErlVars + expectedUserErlVarNums, state->dataRuntimeLang->NumErlVariables);
    EXPECT_EQ(1, state->dataRuntimeLang->NumErlTrendVariables);

    // Actuators are first
    auto &actuatorDirectErlVar = state->dataRuntimeLang->ErlVariable(1);
    EXPECT_EQ("ACTUATOR_SCH_DIRECT", actuatorDirectErlVar.Name);
    EXPECT_FALSE(actuatorDirectErlVar.ReadOnly);
    EXPECT_FALSE(actuatorDirectErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Null, actuatorDirectErlVar.Value.Type); // Note: this is Null for an actuator to begin with
    EXPECT_EQ(0.0, actuatorDirectErlVar.Value.Number);
    EXPECT_TRUE(actuatorDirectErlVar.Value.String.empty());
    EXPECT_EQ(0, actuatorDirectErlVar.Value.Expression);
    EXPECT_FALSE(actuatorDirectErlVar.Value.TrendVariable);
    EXPECT_EQ(0, actuatorDirectErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(actuatorDirectErlVar.Value.Error.empty());
    EXPECT_TRUE(actuatorDirectErlVar.Value.initialized);

    auto &actuatorIndirectErlVar = state->dataRuntimeLang->ErlVariable(2);
    EXPECT_EQ("ACTUATOR_SCH_INDIRECT", actuatorIndirectErlVar.Name);
    EXPECT_FALSE(actuatorIndirectErlVar.ReadOnly);
    EXPECT_FALSE(actuatorIndirectErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Null, actuatorIndirectErlVar.Value.Type); // Note: this is Null for an actuator to begin with
    EXPECT_EQ(0.0, actuatorIndirectErlVar.Value.Number);
    EXPECT_TRUE(actuatorIndirectErlVar.Value.String.empty());
    EXPECT_EQ(0, actuatorIndirectErlVar.Value.Expression);
    EXPECT_FALSE(actuatorIndirectErlVar.Value.TrendVariable);
    EXPECT_EQ(0, actuatorIndirectErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(actuatorIndirectErlVar.Value.Error.empty());
    EXPECT_TRUE(actuatorIndirectErlVar.Value.initialized);

    // Then we have the built in ones
    for (int i = 1; i <= numBuiltInErlVars; ++i) {
        EXPECT_EQ(builtInErlVarNames[i - 1], state->dataRuntimeLang->ErlVariable(2 + i).Name);
    }

    auto &argTrendValueErlVar = state->dataRuntimeLang->ErlVariable(3 + numBuiltInErlVars);
    EXPECT_EQ("ARGTRENDVALUE", argTrendValueErlVar.Name);
    EXPECT_FALSE(argTrendValueErlVar.ReadOnly);
    EXPECT_FALSE(argTrendValueErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, argTrendValueErlVar.Value.Type);
    EXPECT_EQ(0.0, argTrendValueErlVar.Value.Number);
    EXPECT_TRUE(argTrendValueErlVar.Value.String.empty());
    EXPECT_EQ(0, argTrendValueErlVar.Value.Expression);
    EXPECT_TRUE(argTrendValueErlVar.Value.TrendVariable);
    EXPECT_EQ(1, argTrendValueErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(argTrendValueErlVar.Value.Error.empty());
    EXPECT_TRUE(argTrendValueErlVar.Value.initialized);

    auto &trendVar = state->dataRuntimeLang->TrendVariable(1);
    std::array<Real64, 12> trendValues = {45, 30, 15, 60, 45, 30, 15, 60, 45, 30, 15, 60};

    // rotate right is the same as shift right + insert 60 at first pos in our case
    // [60, 45, 30, 15, 60, 45, 30, 15, 60, 45, 30, 15]
    auto trenValuesAfterCallAt60Minute = trendValues;
    std::rotate(trenValuesAfterCallAt60Minute.rbegin(), trenValuesAfterCallAt60Minute.rbegin() + 1, trenValuesAfterCallAt60Minute.rend());
    trendVar.TrendValARR = trendValues;
    EXPECT_EQ("TREND_ARGTRENDVALUE", trendVar.Name);
    EXPECT_EQ(12, trendVar.LogDepth);
    EXPECT_EQ(3 + numBuiltInErlVars, trendVar.ErlVariablePointer);

    auto &resultValueErlVar = state->dataRuntimeLang->ErlVariable(4 + numBuiltInErlVars);
    EXPECT_EQ("RESULTVALUE1", resultValueErlVar.Name);
    EXPECT_FALSE(resultValueErlVar.ReadOnly);
    EXPECT_FALSE(resultValueErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, resultValueErlVar.Value.Type);
    EXPECT_EQ(0.0, resultValueErlVar.Value.Number);
    EXPECT_TRUE(resultValueErlVar.Value.String.empty());
    EXPECT_EQ(0, resultValueErlVar.Value.Expression);
    EXPECT_FALSE(resultValueErlVar.Value.TrendVariable);
    EXPECT_EQ(0, resultValueErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(resultValueErlVar.Value.Error.empty());
    EXPECT_FALSE(resultValueErlVar.Value.initialized); // Note

    bool anyEMSRan = false;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyEMSRan);

    EXPECT_EQ("ACTUATOR_SCH_DIRECT", actuatorDirectErlVar.Name);
    EXPECT_FALSE(actuatorDirectErlVar.ReadOnly);
    EXPECT_FALSE(actuatorDirectErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, actuatorDirectErlVar.Value.Type); // Note: this is Null for an actuator to begin with
    EXPECT_EQ(45.0, actuatorDirectErlVar.Value.Number);
    EXPECT_TRUE(actuatorDirectErlVar.Value.String.empty());
    EXPECT_EQ(0, actuatorDirectErlVar.Value.Expression);
    EXPECT_FALSE(actuatorDirectErlVar.Value.TrendVariable);
    EXPECT_EQ(0, actuatorDirectErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(actuatorDirectErlVar.Value.Error.empty());
    EXPECT_TRUE(actuatorDirectErlVar.Value.initialized);

    EXPECT_EQ("ACTUATOR_SCH_INDIRECT", actuatorIndirectErlVar.Name);
    EXPECT_FALSE(actuatorIndirectErlVar.ReadOnly);
    EXPECT_FALSE(actuatorIndirectErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, actuatorIndirectErlVar.Value.Type); // Note: this is Null for an actuator to begin with
    EXPECT_EQ(45.0, actuatorIndirectErlVar.Value.Number);
    EXPECT_TRUE(actuatorIndirectErlVar.Value.String.empty());
    EXPECT_EQ(0, actuatorIndirectErlVar.Value.Expression);
    EXPECT_FALSE(actuatorIndirectErlVar.Value.TrendVariable);
    EXPECT_EQ(0, actuatorIndirectErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(actuatorIndirectErlVar.Value.Error.empty());
    EXPECT_TRUE(actuatorIndirectErlVar.Value.initialized);

    EXPECT_EQ("ARGTRENDVALUE", argTrendValueErlVar.Name);
    EXPECT_FALSE(argTrendValueErlVar.ReadOnly);
    EXPECT_FALSE(argTrendValueErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, argTrendValueErlVar.Value.Type);
    EXPECT_EQ(60.0, argTrendValueErlVar.Value.Number);
    EXPECT_TRUE(argTrendValueErlVar.Value.String.empty());
    EXPECT_EQ(0, argTrendValueErlVar.Value.Expression);
    EXPECT_TRUE(argTrendValueErlVar.Value.TrendVariable);
    EXPECT_EQ(1, argTrendValueErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(argTrendValueErlVar.Value.Error.empty());
    EXPECT_TRUE(argTrendValueErlVar.Value.initialized);

    EXPECT_EQ("RESULTVALUE1", resultValueErlVar.Name);
    EXPECT_FALSE(resultValueErlVar.ReadOnly);
    EXPECT_FALSE(resultValueErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, resultValueErlVar.Value.Type);
    EXPECT_EQ(45.0, resultValueErlVar.Value.Number);
    EXPECT_TRUE(resultValueErlVar.Value.String.empty());
    EXPECT_EQ(0, resultValueErlVar.Value.Expression);
    EXPECT_FALSE(resultValueErlVar.Value.TrendVariable);
    EXPECT_EQ(0, resultValueErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(resultValueErlVar.Value.Error.empty());
    EXPECT_TRUE(resultValueErlVar.Value.initialized);

    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_TRUE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(45.0, schedDirect->EMSVal);
    EXPECT_EQ(45.0, schedIndirect->currentVal);
    EXPECT_TRUE(schedIndirect->EMSActuatedOn);
    EXPECT_EQ(45.0, schedIndirect->EMSVal);

    EMSManager::UpdateEMSTrendVariables(*state);
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(trenValuesAfterCallAt60Minute[i], trendVar.TrendValARR(i + 1));
    }
    // Now, here's the kicker. Once we get to the Simulation itself again, we go into a different block, because t thinks it's a trend variable
    // BeginEnvrnInitializeRuntimeLanguage is called, which resets the actuators to a Value with Type Null
    // And the assignment for trend variable only sets Number, not Type, so Type stays Null, and the actuator is not actuating
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_FALSE(anyEMSRan);

    trendVar.TrendValARR = trendValues;

    state->dataGlobal->CurrentTime = 24.00;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyEMSRan);

    EMSManager::UpdateEMSTrendVariables(*state);
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(trenValuesAfterCallAt60Minute[i], trendVar.TrendValARR(i + 1));
    }

    EXPECT_EQ("ACTUATOR_SCH_DIRECT", actuatorDirectErlVar.Name);
    EXPECT_FALSE(actuatorDirectErlVar.ReadOnly);
    EXPECT_FALSE(actuatorDirectErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, actuatorDirectErlVar.Value.Type); // Note: this is Null for an actuator to begin with
    EXPECT_EQ(45.0, actuatorDirectErlVar.Value.Number);
    EXPECT_TRUE(actuatorDirectErlVar.Value.String.empty());
    EXPECT_EQ(0, actuatorDirectErlVar.Value.Expression);
    EXPECT_FALSE(actuatorDirectErlVar.Value.TrendVariable);
    EXPECT_EQ(0, actuatorDirectErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(actuatorDirectErlVar.Value.Error.empty());
    EXPECT_TRUE(actuatorDirectErlVar.Value.initialized);

    EXPECT_EQ("ACTUATOR_SCH_INDIRECT", actuatorIndirectErlVar.Name);
    EXPECT_FALSE(actuatorIndirectErlVar.ReadOnly);
    EXPECT_FALSE(actuatorIndirectErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, actuatorIndirectErlVar.Value.Type); // Note: this is Null for an actuator to begin with
    EXPECT_EQ(45.0, actuatorIndirectErlVar.Value.Number);
    EXPECT_TRUE(actuatorIndirectErlVar.Value.String.empty());
    EXPECT_EQ(0, actuatorIndirectErlVar.Value.Expression);
    EXPECT_FALSE(actuatorIndirectErlVar.Value.TrendVariable);
    EXPECT_EQ(0, actuatorIndirectErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(actuatorIndirectErlVar.Value.Error.empty());
    EXPECT_TRUE(actuatorIndirectErlVar.Value.initialized);

    EXPECT_EQ("ARGTRENDVALUE", argTrendValueErlVar.Name);
    EXPECT_FALSE(argTrendValueErlVar.ReadOnly);
    EXPECT_FALSE(argTrendValueErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, argTrendValueErlVar.Value.Type);
    EXPECT_EQ(60.0, argTrendValueErlVar.Value.Number);
    EXPECT_TRUE(argTrendValueErlVar.Value.String.empty());
    EXPECT_EQ(0, argTrendValueErlVar.Value.Expression);
    EXPECT_TRUE(argTrendValueErlVar.Value.TrendVariable);
    EXPECT_EQ(1, argTrendValueErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(argTrendValueErlVar.Value.Error.empty());
    EXPECT_TRUE(argTrendValueErlVar.Value.initialized);

    EXPECT_EQ("RESULTVALUE1", resultValueErlVar.Name);
    EXPECT_FALSE(resultValueErlVar.ReadOnly);
    EXPECT_FALSE(resultValueErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, resultValueErlVar.Value.Type);
    EXPECT_EQ(45.0, resultValueErlVar.Value.Number);
    EXPECT_TRUE(resultValueErlVar.Value.String.empty());
    EXPECT_EQ(0, resultValueErlVar.Value.Expression);
    EXPECT_FALSE(resultValueErlVar.Value.TrendVariable);
    EXPECT_EQ(0, resultValueErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(resultValueErlVar.Value.Error.empty());
    EXPECT_TRUE(resultValueErlVar.Value.initialized);

    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_TRUE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(45.0, schedDirect->EMSVal);
    EXPECT_EQ(45.0, schedIndirect->currentVal);
    EXPECT_TRUE(schedIndirect->EMSActuatedOn);
    EXPECT_EQ(45.0, schedIndirect->EMSVal);
}

TEST_F(EnergyPlusFixture, EMSManager_Sensor_On_ScheduleConstant)
{

    // Test for #10962 - Make sure that assigning a Sensor on a Schedule:Constant does not reset it to zero

    std::string const idf_objects = delimited_string({
        "Schedule:Constant,",
        "  ScheduleConstant,                       !- Name",
        "  ,                                       !- Schedule Type Limits Name",
        "  45.0;                                   !- Hourly Value",

        "EnergyManagementSystem:Sensor,",
        "  ScheduleConstant_Sensor,                !- Name",
        "  ScheduleConstant,                       !- Output:Variable or Output:Meter Index Key Name",
        "  Schedule Value;                         !- Output:Variable or Output:Meter Name",

        "EnergyManagementSystem:Program,",
        "  sch_test,                               !- Name",
        "  SET DummyVal = ScheduleConstant_Sensor; !- Program Line 1",

        "EnergyManagementSystem:ProgramCallingManager,",
        "  sch_test_pcm,                           !- Name",
        "  BeginTimestepBeforePredictor,           !- EnergyPlus Model Calling Point",
        "  sch_test;                               !- Program Name 1",

        "Output:Variable,*,Schedule Value,Timestep;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    constexpr int numBuiltInErlVars = 27; // 27 ErlVariables are constant and built-in variables
    constexpr std::array<std::string_view, numBuiltInErlVars> builtInErlVarNames = {
        "NULL",
        "FALSE",
        "TRUE",
        "OFF",
        "ON",
        "PI",
        "TIMESTEPSPERHOUR",
        "YEAR",
        "CALENDARYEAR",
        "MONTH",
        "DAYOFMONTH",
        "DAYOFWEEK",
        "DAYOFYEAR",
        "HOUR",
        "TIMESTEPNUM",
        "MINUTE",
        "HOLIDAY",
        "DAYLIGHTSAVINGS",
        "CURRENTTIME",
        "SUNISUP",
        "ISRAINING",
        "SYSTEMTIMESTEP",
        "ZONETIMESTEP",
        "CURRENTENVIRONMENT",
        "ACTUALDATEANDTIME",
        "ACTUALTIME",
        "WARMUPFLAG",
    };
    constexpr int expectedUserErlVarNums = 2;

    // Apparently this needs to be called before GetScheduleData, i.e., init_state()
    EMSManager::CheckIfAnyEMS(*state); // get EMS input
    EXPECT_TRUE(state->dataGlobal->AnyEnergyManagementSystemInModel);

    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;
    state->init_state(*state);

    OutputProcessor::SetupTimePointers(
        *state, OutputProcessor::TimeStepType::Zone, state->dataGlobal->TimeStepZone); // Set up Time pointer for HB/Zone Simulation
    OutputProcessor::SetupTimePointers(*state, OutputProcessor::TimeStepType::System, state->dataHVACGlobal->TimeStepSys);

    // Read EMS first, it should NOT find the Schedule Value in the output processor.
    bool anyEMSRan = false;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginZoneTimestepBeforeInitHeatBalance, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_FALSE(anyEMSRan);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    EXPECT_EQ(3, state->dataSched->schedules.size());
    auto *schedDirect = Sched::GetSchedule(*state, "SCHEDULECONSTANT");
    EXPECT_NE(schedDirect, nullptr);

    EXPECT_EQ(numBuiltInErlVars + expectedUserErlVarNums, state->dataRuntimeLang->NumErlVariables);
    EXPECT_EQ(1, state->dataRuntimeLang->NumSensors);

    // Actuators & Sensors are first
    auto &sensorErlVar = state->dataRuntimeLang->ErlVariable(1);
    EXPECT_EQ("SCHEDULECONSTANT_SENSOR", sensorErlVar.Name);
    EXPECT_FALSE(sensorErlVar.ReadOnly);
    EXPECT_FALSE(sensorErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, sensorErlVar.Value.Type);
    EXPECT_EQ(0.0, sensorErlVar.Value.Number);
    EXPECT_TRUE(sensorErlVar.Value.String.empty());
    EXPECT_EQ(0, sensorErlVar.Value.Expression);
    EXPECT_FALSE(sensorErlVar.Value.TrendVariable);
    EXPECT_EQ(0, sensorErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(sensorErlVar.Value.Error.empty());
    EXPECT_TRUE(sensorErlVar.Value.initialized);

    auto const &sensor = state->dataRuntimeLang->Sensor(1);
    EXPECT_EQ("SCHEDULECONSTANT_SENSOR", sensor.Name);
    EXPECT_EQ("SCHEDULECONSTANT", sensor.UniqueKeyName);
    EXPECT_EQ("SCHEDULE VALUE", sensor.OutputVarName);
    EXPECT_EQ(0, sensor.Index);
    EXPECT_EQ(1, sensor.VariableNum);
    // CheckedOkay and sched aren't good yet
    EXPECT_ENUM_EQ(EnergyPlus::OutputProcessor::VariableType::Invalid, sensor.VariableType);
    EXPECT_FALSE(sensor.CheckedOkay);
    EXPECT_EQ(nullptr, sensor.sched);

    // Then we have the built in ones
    for (int i = 1; i <= numBuiltInErlVars; ++i) {
        EXPECT_EQ(builtInErlVarNames[i - 1], state->dataRuntimeLang->ErlVariable(1 + i).Name);
    }

    // FInally we have our DummyVal local
    auto &dummyValErlVar = state->dataRuntimeLang->ErlVariable(2 + numBuiltInErlVars);
    EXPECT_EQ("DUMMYVAL", dummyValErlVar.Name);
    EXPECT_FALSE(dummyValErlVar.ReadOnly);
    EXPECT_FALSE(dummyValErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, dummyValErlVar.Value.Type);
    EXPECT_EQ(0.0, dummyValErlVar.Value.Number);
    EXPECT_TRUE(dummyValErlVar.Value.String.empty());
    EXPECT_EQ(0, dummyValErlVar.Value.Expression);
    EXPECT_FALSE(dummyValErlVar.Value.TrendVariable);
    EXPECT_EQ(0, dummyValErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(dummyValErlVar.Value.Error.empty());
    EXPECT_FALSE(dummyValErlVar.Value.initialized); // Note

    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 23;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 24;
    state->dataGlobal->CurrentTime = 24.0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    state->dataEnvrn->DSTIndicator = 0; // DST IS OFF
    Sched::ReportScheduleVals(*state);  // ReportScheduleVals will take care of adding the Output Variables

    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);

    // Call InitEMS and mostly ProcessEMSInput so that the sched is assigned
    // It will call BeginEnvrnInitializeRuntimeLanguage, which will reset the Sensor to 0...
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_FALSE(anyEMSRan);
    // this stays the same
    EXPECT_EQ("SCHEDULECONSTANT_SENSOR", sensor.Name);
    EXPECT_EQ("SCHEDULECONSTANT", sensor.UniqueKeyName);
    EXPECT_EQ("SCHEDULE VALUE", sensor.OutputVarName);
    EXPECT_EQ(0, sensor.Index);
    EXPECT_EQ(1, sensor.VariableNum);
    // CheckedOkay and sched are ok now
    EXPECT_ENUM_EQ(EnergyPlus::OutputProcessor::VariableType::Real, sensor.VariableType);
    EXPECT_TRUE(sensor.CheckedOkay);
    EXPECT_NE(nullptr, sensor.sched);
    EXPECT_EQ(schedDirect, sensor.sched);

    EXPECT_EQ(0.0, schedDirect->currentVal);
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);
    Sched::ReportScheduleVals(*state); // ReportScheduleVals will take care of adding the Output Variables
    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_EQ(45.0, schedDirect->getCurrentVal());
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);

    anyEMSRan = false;
    // Just to call BeginEnvrnInitializeRuntimeLanguage, which will reset the Sensor to 0...
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginNewEnvironment, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_FALSE(anyEMSRan);

    EXPECT_EQ("SCHEDULECONSTANT_SENSOR", sensorErlVar.Name);
    EXPECT_FALSE(sensorErlVar.ReadOnly);
    EXPECT_FALSE(sensorErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, sensorErlVar.Value.Type);
    EXPECT_EQ(0.0, sensorErlVar.Value.Number);
    EXPECT_TRUE(sensorErlVar.Value.String.empty());
    EXPECT_EQ(0, sensorErlVar.Value.Expression);
    EXPECT_FALSE(sensorErlVar.Value.TrendVariable);
    EXPECT_EQ(0, sensorErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(sensorErlVar.Value.Error.empty());
    EXPECT_TRUE(sensorErlVar.Value.initialized);

    EXPECT_EQ(0.0, schedDirect->currentVal);
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);

    Sched::ReportScheduleVals(*state);
    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_EQ(45.0, schedDirect->getCurrentVal());
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);

    anyEMSRan = false;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyEMSRan, ObjexxFCL::Optional_int_const());
    EXPECT_TRUE(anyEMSRan);

    EXPECT_EQ("SCHEDULECONSTANT_SENSOR", sensorErlVar.Name);
    EXPECT_FALSE(sensorErlVar.ReadOnly);
    EXPECT_FALSE(sensorErlVar.SetByExternalInterface);
    EXPECT_ENUM_EQ(DataRuntimeLanguage::Value::Number, sensorErlVar.Value.Type);
    EXPECT_EQ(45.0, sensorErlVar.Value.Number);
    EXPECT_TRUE(sensorErlVar.Value.String.empty());
    EXPECT_EQ(0, sensorErlVar.Value.Expression);
    EXPECT_FALSE(sensorErlVar.Value.TrendVariable);
    EXPECT_EQ(0, sensorErlVar.Value.TrendVarPointer);
    EXPECT_TRUE(sensorErlVar.Value.Error.empty());
    EXPECT_TRUE(sensorErlVar.Value.initialized);

    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);
    Sched::ReportScheduleVals(*state);
    EXPECT_EQ(45.0, schedDirect->currentVal);
    EXPECT_EQ(45.0, schedDirect->getCurrentVal());
    EXPECT_FALSE(schedDirect->EMSActuatedOn);
    EXPECT_EQ(0.0, schedDirect->EMSVal);
}

TEST_F(EnergyPlusFixture, UnusedActuatorWarning)
{
    // Issue #10944: user declares actuator A1, typos it as Al in the program.
    // Al becomes a fresh local Erl variable; A1 is never assigned.
    // End-of-sim check must warn about unused actuator A1.
    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Actuator,",
        "A1,                              !- Name",
        "Test node,                       !- Actuated Component Unique Name",
        "System Node Setpoint,            !- Actuated Component Type",
        "Temperature Setpoint;            !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Typo Test Manager,               !- Name",
        "BeginTimestepBeforePredictor,    !- EnergyPlus Model Calling Point",
        "TypoProgram;                     !- Program Name 1",

        "EnergyManagementSystem:Program,",
        "TypoProgram,",
        "Set Al = 2;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);

    EMSManager::CheckIfAnyEMS(*state);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());

    EMSManager::checkForUnusedActuatorsAtEnd(*state);

    EXPECT_TRUE(compare_err_stream_substring("Unused EMS Actuator detected", false));
    EXPECT_TRUE(compare_err_stream_substring("A1", true));
}

TEST_F(EnergyPlusFixture, UnusedActuatorWarning_ConditionalNullBranchNotFalsePositive)
{
    // Issue #10944: idiomatic `IF cond SET act = v ELSE SET act = NULL` pattern must NOT
    // trigger the unused-actuator warning just because the condition branch never fires at runtime.
    // Static parse-time reference check sees the SET target, flag flips even if only NULL branch runs.
    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Actuator,",
        "A1,                              !- Name",
        "Test node,                       !- Actuated Component Unique Name",
        "System Node Setpoint,            !- Actuated Component Type",
        "Temperature Setpoint;            !- Actuated Component Control Type",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Cond Null Manager,               !- Name",
        "BeginTimestepBeforePredictor,    !- EnergyPlus Model Calling Point",
        "CondNullProgram;                 !- Program Name 1",

        // Condition always false at runtime -> only NULL branch fires -> old runtime flag would stay false.
        // Static parse check sees `SET A1 = ...` in the IF branch -> flag flips at parse time regardless.
        "EnergyManagementSystem:Program,",
        "CondNullProgram,",
        "IF 0 > 1,",
        "SET A1 = 42,",
        "ELSE,",
        "SET A1 = NULL,",
        "ENDIF;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    OutAirNodeManager::SetOutAirNodes(*state);

    EMSManager::CheckIfAnyEMS(*state);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    bool anyRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan, ObjexxFCL::Optional_int_const());
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::BeginTimestepBeforePredictor, anyRan, ObjexxFCL::Optional_int_const());

    EMSManager::checkForUnusedActuatorsAtEnd(*state);

    EXPECT_FALSE(compare_err_stream_substring("Unused EMS Actuator detected", false, false));
}
