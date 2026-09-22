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

// EnergyPlus::Boilers Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/Plant/Branch.hh>
#include <EnergyPlus/Plant/Component.hh>

#include "../Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, Plant_Topology_Branch_MaxAbsLoad)
{
    EnergyPlus::DataPlant::BranchData b;
    b.Comp.allocate(3);

    b.Comp[0].MyLoad = 20000;
    b.Comp[1].MyLoad = 21000;
    b.Comp[2].MyLoad = 22000;
    Real64 maxLoad = b.max_abs_Comp_MyLoad();
    ASSERT_NEAR(22000, maxLoad, 0.001);

    b.Comp[0].MyLoad = 22000;
    b.Comp[1].MyLoad = 21000;
    b.Comp[2].MyLoad = 20000;
    maxLoad = b.max_abs_Comp_MyLoad();
    ASSERT_NEAR(22000, maxLoad, 0.001);

    b.Comp[0].MyLoad = 0;
    b.Comp[1].MyLoad = -21000;
    b.Comp[2].MyLoad = 22000;
    maxLoad = b.max_abs_Comp_MyLoad();
    ASSERT_NEAR(22000, maxLoad, 0.001);

    b.Comp[0].MyLoad = 0;
    b.Comp[1].MyLoad = 0;
    b.Comp[2].MyLoad = -22000; // still highest via absolute value
    maxLoad = b.max_abs_Comp_MyLoad();
    ASSERT_NEAR(22000, maxLoad, 0.001);

    b.Comp[0].MyLoad = 0;
    b.Comp[1].MyLoad = 21000;
    b.Comp[2].MyLoad = -22000;
    maxLoad = b.max_abs_Comp_MyLoad();
    ASSERT_NEAR(22000, maxLoad, 0.001);
}

TEST_F(EnergyPlusFixture, TestDetermineBranchFlowRequest)
{
    // create a branch to mess with
    DataPlant::BranchData b;

    // set up the nodes
    b.NodeNumIn = 1;
    b.NodeNumOut = 3;
    state->dataLoopNodes->Node.allocate(3);
    auto &nodeIn = state->dataLoopNodes->Node(1);
    auto &nodeMiddle = state->dataLoopNodes->Node(2);
    auto &nodeOut = state->dataLoopNodes->Node(3);

    // allocate some components
    b.TotalComponents = 2;
    b.Comp.allocate(2);
    b.Comp(1).NodeNumIn = 1;
    b.Comp(2).NodeNumIn = 2;

    nodeIn.MassFlowRateRequest = 1.0;
    nodeIn.MassFlowRateMaxAvail = 5.0;
    nodeMiddle.MassFlowRateRequest = 2.0;
    nodeMiddle.MassFlowRateMaxAvail = 5.0;
    nodeOut.MassFlowRateRequest = 3.0; // only the component inlet nodes are checked, so this shouldn't be used
    nodeOut.MassFlowRateMaxAvail = 5.0;

    // for non-series-active branch, it just takes the inlet node request
    b.controlType = DataBranchAirLoopPlant::ControlType::Active;
    Real64 flowRequest = b.DetermineBranchFlowRequest(*state);
    EXPECT_NEAR(1.0, flowRequest, 0.001);

    // for series-active branch, it takes the max of all of them
    b.controlType = DataBranchAirLoopPlant::ControlType::SeriesActive;
    flowRequest = b.DetermineBranchFlowRequest(*state);
    EXPECT_NEAR(2.0, flowRequest, 0.001);
}

TEST_F(EnergyPlusFixture, TestDetermineBranchFlowRequest_ActiveComponentNotFirstOnBranch)
{
    // Reproduces https://github.com/NatLabRockies/EnergyPlus/issues/10165
    // A branch with a passive component (e.g. Pipe:Outdoor) upstream of the
    // one Active component (e.g. WaterUse:Connections) on the branch. The
    // Active component registers its flow request on its own inlet node
    // (the "middle" node here), not on the branch's first-component inlet
    // node. DetermineBranchFlowRequest must find that Active component's
    // request rather than blindly reading the branch inlet node, which the
    // passive upstream component never writes to.

    DataPlant::BranchData b;

    // set up the nodes
    b.NodeNumIn = 1;
    b.NodeNumOut = 3;
    state->dataLoopNodes->Node.allocate(3);
    auto &nodeIn = state->dataLoopNodes->Node(1);
    auto &nodeMiddle = state->dataLoopNodes->Node(2);
    auto &nodeOut = state->dataLoopNodes->Node(3);

    // allocate two components: a passive pipe first, then the active component
    b.TotalComponents = 2;
    b.Comp.allocate(2);
    b.Comp(1).NodeNumIn = 1; // passive pipe, e.g. Pipe:Outdoor
    b.Comp(1).FlowCtrl = DataBranchAirLoopPlant::ControlType::Passive;
    b.Comp(2).NodeNumIn = 2; // e.g. WaterUse:Connections
    b.Comp(2).FlowCtrl = DataBranchAirLoopPlant::ControlType::Active;

    // the passive pipe never calls SetComponentFlowRate, so its inlet node request stays at 0
    nodeIn.MassFlowRateRequest = 0.0;
    nodeIn.MassFlowRateMaxAvail = 5.0;
    // the active component registers its request on its own inlet node
    nodeMiddle.MassFlowRateRequest = 2.0;
    nodeMiddle.MassFlowRateMaxAvail = 5.0;
    nodeOut.MassFlowRateRequest = 3.0; // only the component inlet nodes are checked, so this shouldn't be used
    nodeOut.MassFlowRateMaxAvail = 5.0;

    // the branch as a whole is Active (only one Active component on it), so the
    // request should come from that component's inlet node, not the branch inlet node
    b.controlType = DataBranchAirLoopPlant::ControlType::Active;
    Real64 flowRequest = b.DetermineBranchFlowRequest(*state);
    EXPECT_NEAR(2.0, flowRequest, 0.001);
}
