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

// EnergyPlus::PurchasedAirManager (Ideal Loads) Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::PlantUtilities;
using namespace DataSizing;

TEST_F(EnergyPlusFixture, PlantUtilities_RegisterPlantCompDesignFlowTest1)
{
    // first call just puts first value in array
    int TestNodeNum1 = 123;
    Real64 TestFlowRate1 = 45.6;
    state->dataSize->SaveNumPlantComps = 0;
    RegisterPlantCompDesignFlow(*state, TestNodeNum1, TestFlowRate1);
    EXPECT_EQ(TestNodeNum1, state->dataSize->CompDesWaterFlow(1).SupNode);
    EXPECT_EQ(TestFlowRate1, state->dataSize->CompDesWaterFlow(1).DesVolFlowRate);

    // second call searches array and since node not found adds an entry to array
    int TestNodeNum2 = 234;
    Real64 TestFlowRate2 = 56.7;
    RegisterPlantCompDesignFlow(*state, TestNodeNum2, TestFlowRate2);
    EXPECT_EQ(TestNodeNum2, state->dataSize->CompDesWaterFlow(2).SupNode);
    EXPECT_EQ(TestFlowRate2, state->dataSize->CompDesWaterFlow(2).DesVolFlowRate);

    // third call searches array and since node was found adds an entry to array
    Real64 TestFlowRate3 = 67.8;
    RegisterPlantCompDesignFlow(*state, TestNodeNum1, TestFlowRate3);
    EXPECT_EQ(TestFlowRate3, state->dataSize->CompDesWaterFlow(1).DesVolFlowRate);
}

TEST_F(EnergyPlusFixture, TestRegulateCondenserCompFlowReqOp)
{
    // test consecutive call to fluid properties getInput
    Fluid::GetFluidPropertiesData(*state);
    EXPECT_EQ(1, state->dataFluid->refrigs.isize());
    EXPECT_EQ(1, state->dataFluid->glycols.isize());

    // should never happen but if it does it's safe

    // This is the second unit test that does this, when really we
    // should just ensure that it never happens.
    Fluid::GetFluidPropertiesData(*state);
    EXPECT_EQ(1, state->dataFluid->refrigs.isize());
    EXPECT_EQ(1, state->dataFluid->glycols.isize());

    // This test captures all code paths through the RegulateCondenserCompFlowReqOp function
    // We only need a single component to check here
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &thisComponent = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    Real64 flowRequest = 3.14;
    Real64 returnedFlow;

    // if the component's ON flag is false, then it should return zero flow request no matter the other values
    thisComponent.ON = false;

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::HeatingRB; // meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::CoolingRB; // meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased; // meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::Uncontrolled; // NOT meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);

    // if the component's ON flag is true, then it needs to make decisions
    thisComponent.ON = true;

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::HeatingRB; // meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::CoolingRB; // meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased; // meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(0.0, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);

    thisComponent.CurOpSchemeType = DataPlant::OpScheme::Uncontrolled; // NOT meaningful load

    thisComponent.MyLoad = 0.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);
    thisComponent.MyLoad = 1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);
    thisComponent.MyLoad = -1000.0;
    returnedFlow = PlantUtilities::RegulateCondenserCompFlowReqOp(*state, {1, DataPlant::LoopSideLocation::Demand, 1, 1}, flowRequest);
    EXPECT_NEAR(flowRequest, returnedFlow, 0.00001);
}

TEST_F(EnergyPlusFixture, TestAnyPlantSplitterMixerLacksContinuity)
{
    // This test captures all code paths through the AnyPlantSplitterMixerLacksContinuity function

    // We need to set up a two sided plant loop, we'll have one side not have a splitter for convenience
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Splitter.Exists = false;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).NodeNumOut = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).NodeNumOut = 3;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Splitter.Exists = true;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Splitter.NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Splitter.TotalOutletNodes = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Splitter.BranchNumOut.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Splitter.BranchNumOut(1) = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Splitter.BranchNumOut(2) = 2;

    state->dataLoopNodes->Node.allocate(3);

    // case 1: inlet flow and outlet flow match up
    state->dataLoopNodes->Node(1).MassFlowRate = 3.0;
    state->dataLoopNodes->Node(2).MassFlowRate = 1.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 2.0;
    EXPECT_FALSE(PlantUtilities::AnyPlantSplitterMixerLacksContinuity(*state));

    // case 2: inlet flow > outlet flow
    state->dataLoopNodes->Node(1).MassFlowRate = 4.0;
    state->dataLoopNodes->Node(2).MassFlowRate = 1.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 2.0;
    EXPECT_TRUE(PlantUtilities::AnyPlantSplitterMixerLacksContinuity(*state));

    // case 3: inlet flow < outlet flow
    state->dataLoopNodes->Node(1).MassFlowRate = 1.0;
    state->dataLoopNodes->Node(2).MassFlowRate = 2.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 3.0;
    EXPECT_TRUE(PlantUtilities::AnyPlantSplitterMixerLacksContinuity(*state));

    // case 4: all zero flow
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.0;
    EXPECT_FALSE(PlantUtilities::AnyPlantSplitterMixerLacksContinuity(*state));
}

TEST_F(EnergyPlusFixture, TestPullCompInterconnectTrigger)
{
    // This test captures all code paths through the PullCompInterconnectTrigger function

    state->dataPlnt->PlantLoop.allocate(2);

    // We'll set up two plant loops, the one to test and the connected one
    // each one will need a single loop side, but no branches are checked or anything like that
    PlantLocation plantLoc{1, DataPlant::LoopSideLocation::Demand, 0, 0};
    PlantUtilities::SetPlantLocationLinks(*state, plantLoc);
    // plantLoc.branch = &plantLoc.side->Branch(plantLoc.branchNum);
    // plantLoc.comp = &plantLoc.branch->Comp(plantLoc.compNum);

    PlantLocation connectedPlantLoc{2, DataPlant::LoopSideLocation::Demand, 0, 0};
    PlantUtilities::SetPlantLocationLinks(*state, connectedPlantLoc);

    int criteriaCheckIndex1 = 0, criteriaCheckIndex2 = 0, criteriaCheckIndex3 = 0;
    Real64 criteriaValue1 = 0.0, criteriaValue2 = 0.0, criteriaValue3 = 0.0;

    auto &connectedLoopSide = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand);

    // the first time we call each criteria check, we should just get an index back and it should trigger the connected loop
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex1, connectedPlantLoc, DataPlant::CriteriaType::MassFlowRate, criteriaValue1);
    EXPECT_EQ(1, criteriaCheckIndex1);
    EXPECT_TRUE(connectedLoopSide.SimLoopSideNeeded);

    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex2, connectedPlantLoc, DataPlant::CriteriaType::Temperature, criteriaValue2);
    EXPECT_EQ(2, criteriaCheckIndex2);
    EXPECT_TRUE(connectedLoopSide.SimLoopSideNeeded);

    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex3, connectedPlantLoc, DataPlant::CriteriaType::HeatTransferRate, criteriaValue3);
    EXPECT_EQ(3, criteriaCheckIndex3);
    EXPECT_TRUE(connectedLoopSide.SimLoopSideNeeded);

    // the second and afterward times we call each check, we need to actually expect it to check criteria

    // call it with a nonzero value here, and it should trigger the sim flag
    criteriaValue1 = 2.718;
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex1, connectedPlantLoc, DataPlant::CriteriaType::MassFlowRate, criteriaValue1);
    EXPECT_TRUE(connectedLoopSide.SimLoopSideNeeded);

    criteriaValue2 = 2.718;
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex2, connectedPlantLoc, DataPlant::CriteriaType::Temperature, criteriaValue2);
    EXPECT_TRUE(connectedLoopSide.SimLoopSideNeeded);

    criteriaValue3 = 2.718;
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex3, connectedPlantLoc, DataPlant::CriteriaType::HeatTransferRate, criteriaValue3);
    EXPECT_TRUE(connectedLoopSide.SimLoopSideNeeded);

    // call it with the same nonzero value here, and it should *not* trigger the sim flag
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex1, connectedPlantLoc, DataPlant::CriteriaType::MassFlowRate, criteriaValue1);
    EXPECT_FALSE(connectedLoopSide.SimLoopSideNeeded);

    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex2, connectedPlantLoc, DataPlant::CriteriaType::Temperature, criteriaValue2);
    EXPECT_FALSE(connectedLoopSide.SimLoopSideNeeded);

    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex3, connectedPlantLoc, DataPlant::CriteriaType::HeatTransferRate, criteriaValue3);
    EXPECT_FALSE(connectedLoopSide.SimLoopSideNeeded);

    // call it with a tiny (within tolerance) change and it should still not trigger it
    criteriaValue1 += DataPlant::CriteriaDelta_MassFlowRate / 2.0;
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex1, connectedPlantLoc, DataPlant::CriteriaType::MassFlowRate, criteriaValue1);
    EXPECT_FALSE(connectedLoopSide.SimLoopSideNeeded);

    criteriaValue2 += DataPlant::CriteriaDelta_Temperature / 2.0;
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex2, connectedPlantLoc, DataPlant::CriteriaType::Temperature, criteriaValue2);
    EXPECT_FALSE(connectedLoopSide.SimLoopSideNeeded);

    criteriaValue3 += DataPlant::CriteriaDelta_HeatTransferRate / 2.0;
    connectedLoopSide.SimLoopSideNeeded = false;
    PlantUtilities::PullCompInterconnectTrigger(
        *state, plantLoc, criteriaCheckIndex3, connectedPlantLoc, DataPlant::CriteriaType::HeatTransferRate, criteriaValue3);
    EXPECT_FALSE(connectedLoopSide.SimLoopSideNeeded);
}

TEST_F(EnergyPlusFixture, TestIntegerIsWithinTwoValues)
{

    // valid expected true cases
    EXPECT_TRUE(PlantUtilities::IntegerIsWithinTwoValues(1, 0, 2));
    EXPECT_TRUE(PlantUtilities::IntegerIsWithinTwoValues(0, -1, 1));

    // valid expected false cases
    EXPECT_FALSE(PlantUtilities::IntegerIsWithinTwoValues(0, 1, 2));
    EXPECT_FALSE(PlantUtilities::IntegerIsWithinTwoValues(-1, 0, 1));

    // odd expected false cases
    EXPECT_FALSE(PlantUtilities::IntegerIsWithinTwoValues(1, 2, 0));
}

TEST_F(EnergyPlusFixture, TestCheckPlantConvergence)
{

    // For this case, we need a single plant loop and loop side
    // We will leverage the LogPlantConvergencePoints function to manage the history terms
    // That function is nice because it is very tightly contained, so we don't have to set up a lot of global state
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataLoopNodes->Node.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut = 1;
    auto &inNode = state->dataLoopNodes->Node(1);
    auto &outNode = state->dataLoopNodes->Node(2);
    Real64 constexpr roomTemp = 25.0;
    Real64 constexpr nonZeroFlow = 3.14;

    // History terms should be allocated to 5 zeros
    EXPECT_EQ(5u, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).InletNode.TemperatureHistory.size());
    EXPECT_EQ(5u, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).OutletNode.TemperatureHistory.size());
    EXPECT_EQ(5u, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).InletNode.MassFlowRateHistory.size());
    EXPECT_EQ(5u, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).OutletNode.MassFlowRateHistory.size());
    EXPECT_NEAR(0.0, sum(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).InletNode.TemperatureHistory), 0.001);
    EXPECT_NEAR(0.0, sum(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).OutletNode.TemperatureHistory), 0.001);
    EXPECT_NEAR(0.0, sum(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).InletNode.MassFlowRateHistory), 0.001);
    EXPECT_NEAR(0.0, sum(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).OutletNode.MassFlowRateHistory), 0.001);

    // If we check the plant convergence right now with first hvac true, it should require a resimulation
    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(true));

    // But if we check it with first hvac false, everything should be stable and pass
    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));

    // Now let's introduce a disturbance by changing the inlet node temperature and logging it
    inNode.Temp = roomTemp;
    PlantUtilities::LogPlantConvergencePoints(*state, false);
    // We expect it to be false here since the temperature changed
    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));
    // But if we run it 4 more times and let the value propagate, we expect it to be stable and pass
    // Need to call it 5 times total to fully initialize the history
    for (int i = 1; i < 5; ++i) {
        PlantUtilities::LogPlantConvergencePoints(*state, false);
    }
    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));

    // Repeat this for the outlet node temperature
    outNode.Temp = roomTemp;
    PlantUtilities::LogPlantConvergencePoints(*state, false);
    // We expect it to be false here since the temperature changed
    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));
    // But if we run it 4 more times and let the value propagate, we expect it to be stable and pass
    // Need to call it 5 times total to fully initialize the history
    for (int i = 1; i < 5; ++i) {
        PlantUtilities::LogPlantConvergencePoints(*state, false);
    }
    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));

    // Repeat this for the inlet node mass flow rate
    inNode.MassFlowRate = nonZeroFlow;
    PlantUtilities::LogPlantConvergencePoints(*state, false);
    // We expect it to be false here since the temperature changed
    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));
    // But if we run it 4 more times and let the value propagate, we expect it to be stable and pass
    // Need to call it 5 times total to fully initialize the history
    for (int i = 1; i < 5; ++i) {
        PlantUtilities::LogPlantConvergencePoints(*state, false);
    }
    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));

    // And finally the outlet node mass flow rate
    outNode.MassFlowRate = nonZeroFlow;
    PlantUtilities::LogPlantConvergencePoints(*state, false);
    // We expect it to be false here since the temperature changed
    EXPECT_FALSE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));
    // But if we run it 4 more times and let the value propagate, we expect it to be stable and pass
    // Need to call it 5 times total to fully initialize the history
    for (int i = 1; i < 5; ++i) {
        PlantUtilities::LogPlantConvergencePoints(*state, false);
    }
    EXPECT_TRUE(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).CheckPlantConvergence(false));
}

TEST_F(EnergyPlusFixture, TestScanPlantLoopsErrorFlagReturnType)
{

    // test out some stuff on the scan plant loops function, for now just verifying errFlag is passed by reference
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataLoopNodes->Node.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "comp_name";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::Boiler_Simple;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 0; // just skip the supply side search

    PlantLocation plantLoc{0, DataPlant::LoopSideLocation::Invalid, 0, 0};
    bool errorFlag = false;

    // test simple searching first
    PlantUtilities::ScanPlantLoopsForObject(*state, "comp_name", DataPlant::PlantEquipmentType::Boiler_Simple, plantLoc, errorFlag);
    EXPECT_EQ(1, plantLoc.loopNum);
    EXPECT_ENUM_EQ(DataPlant::LoopSideLocation::Demand, plantLoc.loopSideNum);
    EXPECT_EQ(1, plantLoc.branchNum);
    EXPECT_EQ(1, plantLoc.compNum);
    EXPECT_EQ(&state->dataPlnt->PlantLoop(plantLoc.loopNum), plantLoc.loop);
    EXPECT_EQ(&plantLoc.loop->LoopSide(plantLoc.loopSideNum), plantLoc.side);
    EXPECT_EQ(&plantLoc.side->Branch(plantLoc.branchNum), plantLoc.branch);
    EXPECT_EQ(&plantLoc.branch->Comp(plantLoc.compNum), plantLoc.comp);
    EXPECT_FALSE(errorFlag);

    // then test to make sure errorFlag is passed by reference
    PlantUtilities::ScanPlantLoopsForObject(*state, "comp_name_not_here", DataPlant::PlantEquipmentType::Boiler_Simple, plantLoc, errorFlag);
    EXPECT_TRUE(errorFlag);
}
