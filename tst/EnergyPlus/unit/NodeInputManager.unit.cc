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

// EnergyPlus::NodeInputManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::NodeInputManager;
using namespace EnergyPlus::DataLoopNode;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, NodeMoreInfoEMSsensorCheck1)
{
    std::string const idf_objects = delimited_string({
        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Sensor,",
        "test_node_wb,",
        "Test Node, ",
        "System Node Wetbulb Temperature;",

        "EnergyManagementSystem:Sensor,",
        "test_node_rh,",
        "Test Node, ",
        "System Node Relative Humidity;",

        "EnergyManagementSystem:Sensor,",
        "test_node_dp,",
        "Test Node, ",
        "System Node Dewpoint Temperature;",

        "EnergyManagementSystem:Sensor,",
        "test_node_cp,",
        "Test Node, ",
        "System Node Specific Heat;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    OutAirNodeManager::SetOutAirNodes(*state);

    NodeInputManager::SetupNodeVarsForReporting(*state);

    EMSManager::CheckIfAnyEMS(*state);

    state->dataEMSMgr->FinishProcessingUserInput = true;

    bool anyEMSRan;
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyEMSRan, ObjexxFCL::Optional_int_const());

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(1).HumRat = 0.01;
    state->dataEnvrn->OutBaroPress = 100000;
    state->dataEnvrn->StdRhoAir = 1.2;

    NodeInputManager::CalcMoreNodeInfo(*state);

    EXPECT_NEAR(state->dataLoopNodes->MoreNodeInfo(1).RelHumidity, 67.65, 0.01);
    EXPECT_NEAR(state->dataLoopNodes->MoreNodeInfo(1).AirDewPointTemp, 13.84, 0.01);
    EXPECT_NEAR(state->dataLoopNodes->MoreNodeInfo(1).WetBulbTemp, 16.12, 0.01);
    EXPECT_NEAR(state->dataLoopNodes->MoreNodeInfo(1).SpecificHeat, 1023.43, 0.01);
}

TEST_F(EnergyPlusFixture, CheckUniqueNodesTest_Test1)
{
    bool UniqueNodeError(false);

    InitUniqueNodeCheck(*state, "Context");
    // set up initial list using names
    CheckUniqueNodeNames(*state, "NodeFieldName", UniqueNodeError, "TestInputNode1", "ObjectName");
    CheckUniqueNodeNames(*state, "NodeFieldName", UniqueNodeError, "TestOutputNode1", "ObjectName");
    CheckUniqueNodeNames(*state, "NodeFieldName", UniqueNodeError, "TestInputNode2", "ObjectName");
    CheckUniqueNodeNames(*state, "NodeFieldName", UniqueNodeError, "TestOutputNode2", "ObjectName");

    // now to test if a new node is in the list - should not be an error and should be false
    CheckUniqueNodeNames(*state, "NodeFieldName", UniqueNodeError, "NonUsedNode", "ObjectName");
    EXPECT_FALSE(UniqueNodeError);

    // try one that is already in the list - should be an error and show up as true
    CheckUniqueNodeNames(*state, "NodeFieldName", UniqueNodeError, "TestInputNode2", "ObjectName");
    EXPECT_TRUE(UniqueNodeError);

    EndUniqueNodeCheck(*state, "Context");
}

} // namespace EnergyPlus
