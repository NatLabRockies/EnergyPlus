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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantCondLoopOperation.hh>
#include <EnergyPlus/SetPointManager.hh>

using namespace EnergyPlus;

class DistributePlantLoadTest : public EnergyPlusFixture
{

public:
    //    static void SetUpTestCase()
    //    {
    //        EnergyPlusFixture::SetUpTestCase(); // Sets up the base fixture
    //    }
    static void TearDownTestCase()
    {
    }

    virtual void SetUp()
    {
        EnergyPlusFixture::SetUp(); // Sets up individual test cases.

        // unit test for plant equipment list load distribution
        // set up one plantloop side with 1 branches, 12 components
        // using 12 components here to test going beyond the old idd limit of 10 pieces of equipment
        state->dataPlnt->PlantLoop.allocate(1);
        state->dataPlnt->PlantLoop(1).OpScheme.allocate(1);
        state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList.allocate(1);
        auto &thisEquipList(state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1));
        thisEquipList.NumComps = 12;
        thisEquipList.Comp.allocate(thisEquipList.NumComps);

        state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
        state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(thisEquipList.NumComps);
        auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

        for (int compNum = 1; compNum <= state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps; ++compNum) {
            // set up equipment list data
            thisEquipList.Comp(compNum).CompNumPtr = compNum;

            thisEquipList.Comp(compNum).BranchNumPtr = 1;

            // set up individual component data - start with 12 equal size, all available
            thisBranch.Comp(compNum).Available = true;
            thisBranch.Comp(compNum).OptLoad = 90.0;
            thisBranch.Comp(compNum).MaxLoad = 100.0;
            thisBranch.Comp(compNum).MinLoad = 0.0;
            thisBranch.Comp(compNum).MyLoad = 0.0;
            thisBranch.Comp(compNum).CurCompLevelOpNum = 1;
            thisBranch.Comp(compNum).OpScheme.allocate(1);
            thisBranch.Comp(compNum).OpScheme(1).NumEquipLists = 1;
            thisBranch.Comp(compNum).OpScheme(1).OpSchemePtr = 1;
            thisBranch.Comp(compNum).OpScheme(1).EquipList.allocate(1);
            thisBranch.Comp(compNum).OpScheme(1).EquipList(1).ListPtr = 1;
        }
    }

    virtual void ResetLoads()
    {
        // reset loads
        auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        for (int compNum = 1; compNum <= state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps; ++compNum) {
            thisBranch.Comp(compNum).MyLoad = 0.0;
        }
    }
    virtual void TearDown()
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(DistributePlantLoadTest, DistributePlantLoad_Sequential)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::Sequential;

    // Loop demand 550W
    DistributePlantLoadTest::ResetLoads();
    Real64 loopDemand = 550.0;
    Real64 remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);
    DistributePlantLoadTest::ResetLoads();

    // Loop demand 50W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 50.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Loop demand 5000W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 5000.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 3800.0);

    // Loop demand 550W, even numbered components unavailable
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 550.0;
    remainingLoopDemand = 0.0;
    thisBranch.Comp(2).Available = false;
    thisBranch.Comp(4).Available = false;
    thisBranch.Comp(6).Available = false;
    thisBranch.Comp(8).Available = false;
    thisBranch.Comp(10).Available = false;
    thisBranch.Comp(12).Available = false;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Duplicate tests from engineering reference examples for Sequential
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 2;
    thisBranch.Comp(1).MaxLoad = 40.0;
    thisBranch.Comp(1).MinLoad = 0.2 * 40.0;
    thisBranch.Comp(1).OptLoad = 0.6 * 40.0;
    thisBranch.Comp(1).Available = true;
    thisBranch.Comp(2).MaxLoad = 100.0;
    thisBranch.Comp(2).MinLoad = 0.15 * 100.0;
    thisBranch.Comp(2).OptLoad = 0.4 * 100.0;
    thisBranch.Comp(2).Available = true;

    // 5W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 5.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 5.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 25W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 25.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 25.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 50W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 50.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 10.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 100W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 100.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 60.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 150W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 150.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 10.0);

    // 200W
    DistributePlantLoadTest::ResetLoads();
    loopDemand = 200.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 60.0);
}

TEST_F(DistributePlantLoadTest, DistributePlantLoad_Uniform)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::Uniform;

    // Start with 5 components
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 5;

    // Loop demand 550W
    DistributePlantLoadTest::ResetLoads();
    Real64 remainingLoopDemand = 0.0;
    Real64 loopDemand = 550.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 50.0);

    // Loop demand 50W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 50.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 10.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Loop demand 320W, one smaller equipment
    // "extra" load should be distributed sequentially amongst the other equipment
    // component 3 unavailable
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 320.0;
    thisBranch.Comp(4).MaxLoad = 50.0;
    thisBranch.Comp(3).Available = false;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 90.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 80.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Duplicate tests from engineering reference examples
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 2;
    thisBranch.Comp(1).MaxLoad = 40.0;
    thisBranch.Comp(1).MinLoad = 0.2 * 40.0;
    thisBranch.Comp(1).OptLoad = 0.6 * 40.0;
    thisBranch.Comp(2).MaxLoad = 100.0;
    thisBranch.Comp(2).MinLoad = 0.15 * 100.0;
    thisBranch.Comp(2).OptLoad = 0.4 * 100.0;

    // 10W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 10.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 5.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 5.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 25W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 25.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 12.5);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 12.5);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 50W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 50.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 25.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 25.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 100W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 100.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 60.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 150W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 150.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 10.0);

    // 200W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 200.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 60.0);
}

TEST_F(DistributePlantLoadTest, DistributePlantLoad_Optimal)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::Optimal;

    // Start with 5 components and smaller component 4
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 5;
    thisBranch.Comp(4).Available = true;
    thisBranch.Comp(4).OptLoad = 45.0;
    thisBranch.Comp(4).MaxLoad = 50.0;
    thisBranch.Comp(4).MinLoad = 0.0;
    thisBranch.Comp(4).MyLoad = 0.0;

    // Loop demand 550W
    DistributePlantLoadTest::ResetLoads();
    Real64 remainingLoopDemand = 0.0;
    Real64 loopDemand = 550.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 100.0);

    // Loop demand 50W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 440.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 99.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 97.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 97.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 97.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Loop demand 320W
    // component 3 unavailable
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 340.0;
    thisBranch.Comp(3).Available = false;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 97.5);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 96.25);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 96.25);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Duplicate tests from engineering reference examples
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 2;
    thisBranch.Comp(1).MaxLoad = 40.0;
    thisBranch.Comp(1).MinLoad = 0.2 * 40.0;
    thisBranch.Comp(1).OptLoad = 0.6 * 40.0;
    thisBranch.Comp(2).MaxLoad = 100.0;
    thisBranch.Comp(2).MinLoad = 0.15 * 100.0;
    thisBranch.Comp(2).OptLoad = 0.4 * 100.0;

    // 5W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 5.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 5.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 25W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 25.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 24.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 1.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 50W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 50.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 24.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 26.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 100W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 100.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 60.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 150W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 150.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 10.0);

    // 200W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 200.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 60.0);

    // 200W - no equipment available
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 200.0;
    thisBranch.Comp(1).Available = false;
    thisBranch.Comp(2).Available = false;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 200.0);
}

TEST_F(DistributePlantLoadTest, DistributePlantLoad_UniformPLR)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::UniformPLR;

    // Start with 5 components and smaller component 4
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 5;
    thisBranch.Comp(4).Available = true;
    thisBranch.Comp(4).OptLoad = 45.0;
    thisBranch.Comp(4).MaxLoad = 50.0;
    thisBranch.Comp(4).MinLoad = 0.0;
    thisBranch.Comp(4).MyLoad = 0.0;

    // Loop demand 550W
    DistributePlantLoadTest::ResetLoads();
    Real64 loopDemand = 550.0;
    Real64 remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 100.0);

    // Loop demand 45W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 45.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 5.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 10.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Loop demand 280W
    // "extra" load should be distributed sequentially amongst the other equipment
    // component 3 unavailable
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 280;
    remainingLoopDemand = 0.0;
    thisBranch.Comp(3).Available = false;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 80.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 80.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 80.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Duplicate tests from engineering reference examples
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 2;
    thisBranch.Comp(1).MaxLoad = 40.0;
    thisBranch.Comp(1).MinLoad = 0.2 * 40.0;
    thisBranch.Comp(1).OptLoad = 0.6 * 40.0;
    thisBranch.Comp(2).MaxLoad = 100.0;
    thisBranch.Comp(2).MinLoad = 0.15 * 100.0;
    thisBranch.Comp(2).OptLoad = 0.4 * 100.0;

    // 5W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 5.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 5.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 10W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 10.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 25W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 25.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_NEAR(thisBranch.Comp(1).MyLoad, 25.0, 0.1);
    EXPECT_NEAR(thisBranch.Comp(2).MyLoad, 0.0, 0.1);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 50W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 50.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_NEAR(thisBranch.Comp(1).MyLoad, 14.29, 0.1);
    EXPECT_NEAR(thisBranch.Comp(2).MyLoad, 35.71, 0.1);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 100W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 100.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_NEAR(thisBranch.Comp(1).MyLoad, 28.57, 0.1);
    EXPECT_NEAR(thisBranch.Comp(2).MyLoad, 71.43, 0.1);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 150W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 150.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 10.0);
}

TEST_F(DistributePlantLoadTest, DistributePlantLoad_SequentialUniformPLR)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::SequentialUniformPLR;

    // Start with 5 components and smaller component 4
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 5;
    thisBranch.Comp(4).Available = true;
    thisBranch.Comp(4).OptLoad = 45.0;
    thisBranch.Comp(4).MaxLoad = 50.0;
    thisBranch.Comp(4).MinLoad = 0.0;
    thisBranch.Comp(4).MyLoad = 0.0;

    // Loop demand 550W
    DistributePlantLoadTest::ResetLoads();
    Real64 remainingLoopDemand = 0.0;
    Real64 loopDemand = 550.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 100.0);

    // Loop demand 45W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 45.0;
    remainingLoopDemand = 0.0;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 45.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Loop demand 225W
    // "extra" load should be distributed sequentially amongst the other equipment
    // component 3 unavailable
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 225;
    remainingLoopDemand = 0.0;
    thisBranch.Comp(3).Available = false;

    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 90.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 90.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 45.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // Duplicate tests from engineering reference examples
    state->dataPlnt->PlantLoop(1).OpScheme(1).EquipList(1).NumComps = 2;
    thisBranch.Comp(1).MaxLoad = 40.0;
    thisBranch.Comp(1).MinLoad = 0.2 * 40.0;
    thisBranch.Comp(1).OptLoad = 0.6 * 40.0;
    thisBranch.Comp(2).MaxLoad = 100.0;
    thisBranch.Comp(2).MinLoad = 0.15 * 100.0;
    thisBranch.Comp(2).OptLoad = 0.4 * 100.0;

    // 5W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 5.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 5.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 10W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 10.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 10.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 25W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 25.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 25.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 50W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 50.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_NEAR(thisBranch.Comp(1).MyLoad, 14.3, 0.1);
    EXPECT_NEAR(thisBranch.Comp(2).MyLoad, 35.71, 0.1);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 100W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 100.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_NEAR(thisBranch.Comp(1).MyLoad, 28.6, 0.1);
    EXPECT_NEAR(thisBranch.Comp(2).MyLoad, 71.43, 0.1);
    EXPECT_EQ(remainingLoopDemand, 0.0);

    // 150W
    DistributePlantLoadTest::ResetLoads();
    remainingLoopDemand = 0.0;
    loopDemand = 150.0;
    PlantCondLoopOperation::DistributePlantLoad(*state, 1, DataPlant::LoopSideLocation::Demand, 1, 1, loopDemand, remainingLoopDemand);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 40.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(remainingLoopDemand, 10.0);
}

TEST_F(DistributePlantLoadTest, DistributePlantLoadSequentialDryBulbRB)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
    auto &thisOpScheme(state->dataPlnt->PlantLoop(1).OpScheme(1));
    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::Sequential;

    thisOpScheme.Type = DataPlant::OpScheme::DryBulbRB;
    thisOpScheme.EquipList(1).RangeUpperLimit = 12.0;
    thisOpScheme.EquipList(1).RangeLowerLimit = 0;
    thisOpScheme.Available = true;

    PlantLocation this_plantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    DistributePlantLoadTest::ResetLoads();
    Real64 loopDemand = 550.0;
    Real64 remainingLoopDemand = 0.0;
    bool LoopShutDownFlag = false;
    bool LoadDistributionWasPerformed = false;

    state->dataEnvrn->OutDryBulbTemp = 5.0;
    PlantCondLoopOperation::ManagePlantLoadDistribution(
        *state, this_plantLoc, loopDemand, remainingLoopDemand, false, LoopShutDownFlag, LoadDistributionWasPerformed);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);
    DistributePlantLoadTest::ResetLoads();

    state->dataEnvrn->OutDryBulbTemp = -5.0;
    PlantCondLoopOperation::ManagePlantLoadDistribution(
        *state, this_plantLoc, loopDemand, remainingLoopDemand, false, LoopShutDownFlag, LoadDistributionWasPerformed);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);
}

TEST_F(DistributePlantLoadTest, DistributePlantLoadSequentialDryBulbTDB)
{
    auto &thisBranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
    auto &thisOpScheme(state->dataPlnt->PlantLoop(1).OpScheme(1));
    state->dataPlnt->PlantLoop(1).LoadDistribution = DataPlant::LoadingScheme::Sequential;

    thisOpScheme.Type = DataPlant::OpScheme::DryBulbTDB;
    thisOpScheme.ReferenceNodeNumber = 1;
    thisOpScheme.EquipList(1).RangeUpperLimit = 5.0;
    thisOpScheme.EquipList(1).RangeLowerLimit = 0;
    thisOpScheme.Available = true;

    PlantLocation this_plantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    DistributePlantLoadTest::ResetLoads();
    Real64 loopDemand = 550.0;
    Real64 remainingLoopDemand = 0.0;
    bool LoopShutDownFlag = false;
    bool LoadDistributionWasPerformed = false;

    state->dataLoopNodes->Node.allocate(1);
    state->dataLoopNodes->Node(1).Temp = 8.0;
    state->dataEnvrn->OutDryBulbTemp = 5.0;
    PlantCondLoopOperation::ManagePlantLoadDistribution(
        *state, this_plantLoc, loopDemand, remainingLoopDemand, false, LoopShutDownFlag, LoadDistributionWasPerformed);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 100.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 50.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);
    DistributePlantLoadTest::ResetLoads();

    state->dataLoopNodes->Node(1).Temp = -8.0;
    state->dataEnvrn->OutDryBulbTemp = -5.0;
    PlantCondLoopOperation::ManagePlantLoadDistribution(
        *state, this_plantLoc, loopDemand, remainingLoopDemand, false, LoopShutDownFlag, LoadDistributionWasPerformed);
    EXPECT_EQ(thisBranch.Comp(1).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(2).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(3).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(4).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(5).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(6).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(7).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(8).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(9).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(10).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(11).MyLoad, 0.0);
    EXPECT_EQ(thisBranch.Comp(12).MyLoad, 0.0);
    EXPECT_EQ(remainingLoopDemand, 0.0);
}

TEST_F(EnergyPlusFixture, ThermalEnergyStorageWithIceForceDualOp)
{

    std::string const idf_objects = delimited_string({
        "PlantEquipmentOperation:ThermalEnergyStorage,",
        "  TEST PLANTOP SCHEME,          !- Name",
        "  Ice Thermal Storage On-peak,  !- On-Peak Schedule",
        "  Ice Thermal Storage Charging,  !- Charging Availability Schedule",
        "  7.00,                    !- Non-Charging Chilled Water Temperature {C}",
        "  -5.00,                   !- Charging Chilled Water Temperature {C}",
        "  Chiller:Electric:EIR,    !- Component 1 Object Type",
        "  Chiller,                 !- Component 1 Name",
        "  Primary CHW Loop Pump Water Outlet Node,  !- Component 1 Demand Calculation Node Name",
        "  Chiller CHW Outlet Node, !- Component 1 Setpoint Node Name",
        "  0.001351,                !- Component 1 Flow Rate {m3/s}",
        "  Cooling,                 !- Component 1 Operation Type",
        "  ThermalStorage:Ice:Detailed,  !- Component 2 Object Type",
        "  Ice Thermal Storage,     !- Component 2 Name",
        "  Chiller CHW Outlet Node, !- Component 2 Demand Calculation Node Name",
        "  Ice Thermal Storage Water Outlet Node,  !- Component 2 Setpoint Node Name",
        "  autosize,                !- Component 2 Flow Rate {m3/s}",
        "  Cooling;                 !- Component 2 Operation Type",
        "",

        "Schedule:Compact,",
        "  Ice Thermal Storage On-peak,  !- Name",
        "  Fraction,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 08:00,0,          !- Field 3",
        "  Until: 18:00,1,          !- Field 4",
        "  Until: 24:00,0;          !- Field 5",

        "Schedule:Compact,",
        "  Ice Thermal Storage Charging,  !- Name",
        "  Fraction,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 08:00,1,          !- Field 3",
        "  Until: 18:00,0,          !- Field 4",
        "  Until: 24:00,1;          !- Field 5",

        "ScheduleTypeLimits,",
        "  Fraction,                !- Name",
        "  0.0,                     !- Lower Limit Value",
        "  1.0,                     !- Upper Limit Value",
        "  CONTINUOUS,              !- Numeric Type",
        "  Dimensionless;           !- Unit Type",

    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    // Setup the plant itself manually
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).OpScheme.allocate(1);
    state->dataPlnt->PlantLoop(1).OpScheme(1).Name = "TEST PLANTOP SCHEME";

    bool ErrorsFound = false;
    int TESSPBO = 1;
    int LoopNum = 1;
    int SchemeNum = 1;
    std::string CurrentModuleObject = "PlantEquipmentOperation:ThermalEnergyStorage";
    PlantCondLoopOperation::FindCompSPInput(*state, CurrentModuleObject, TESSPBO, LoopNum, SchemeNum, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    std::string const error_string = delimited_string({
        "   ** Warning ** Equipment Operation Mode was reset to 'DUAL' for Component 'ICE THERMAL STORAGE' in "
        "PlantEquipmentOperation:ThermalEnergyStorage='TEST PLANTOP SCHEME'.",
        "   **   ~~~   ** Equipment Operation Mode can only be 'DUAL' for THERMALSTORAGE:ICE:DETAILED objects.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Might as well check that the Chiller is also Ok
    {
        int CompNum = 1;
        std::string compName = state->dataPlnt->PlantLoop(LoopNum).OpScheme(SchemeNum).EquipList(1).Comp(CompNum).Name;
        EXPECT_EQ(compName, "CHILLER");
        auto CtrlTypeNum = state->dataPlnt->PlantLoop(LoopNum).OpScheme(SchemeNum).EquipList(1).Comp(CompNum).CtrlType;
        EXPECT_ENUM_EQ(CtrlTypeNum, DataPlant::CtrlType::CoolingOp);
    }

    {
        int CompNum = 2;
        std::string compName = state->dataPlnt->PlantLoop(LoopNum).OpScheme(SchemeNum).EquipList(1).Comp(CompNum).Name;
        // Ensure we have the right component (the TES tank)
        EXPECT_EQ(compName, "ICE THERMAL STORAGE");

        auto CtrlTypeNum = state->dataPlnt->PlantLoop(LoopNum).OpScheme(SchemeNum).EquipList(1).Comp(CompNum).CtrlType;

        // Could just test this, but want to improve reporting
        // EXPECT_ENUM_EQ(CtrlType, PlantCondLoopOperation::DualOp);

        std::string ctrlType = "Unknown";
        if (CtrlTypeNum == DataPlant::CtrlType::CoolingOp) {
            ctrlType = "CoolingOp";
        } else if (CtrlTypeNum == DataPlant::CtrlType::HeatingOp) {
            ctrlType = "HeatingOp";
        } else if (CtrlTypeNum == DataPlant::CtrlType::DualOp) {
            ctrlType = "DualOp";
        }

        EXPECT_EQ(ctrlType, "DualOp") << compName << " has a wrong control type = '" << ctrlType << "'.";
    }

    // We should now alos have two TES SPMs created, and that's all of them
    EXPECT_EQ(state->dataSetPointManager->spms.size(), 2);
}

TEST_F(EnergyPlusFixture, FindRangeBasedOrUncontrolledInputTest)
{
    std::string currentModuleObject;
    int numSchemes;
    int loopNum;
    int schemeNum;
    bool errorFound;

    std::string const idf_objects = delimited_string({
        "PlantEquipmentOperationSchemes,",
        "  CW Loop Operation,       !- Name,",
        "  PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "  Central Chiller Only,    !- Control Scheme 1 Name",
        "  PlantOnSched;            !- Control Scheme 1 Schedule Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "  Central Chiller Only,    !- Name",
        "  0,                       !- Load Range 1 Lower Limit {W}",
        "  900000,                  !- Load Range 1 Upper Limit {W}",
        "  cooling plant;           !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "  HW Loop Operation,       !- Name",
        "  PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "  Central Boiler Only,     !- Control Scheme 1 Name",
        "  PlantOnSched;            !- Control Scheme 1 Schedule Name",

        "PlantEquipmentOperation:HeatingLoad,",
        "  Central Boiler Only,     !- Name",
        "  0,                       !- Load Range 1 Lower Limit {W}",
        "  1000000,                 !- Load Range 1 Upper Limit {W}",
        "  heating plant;           !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "  Alternate Operation,     !- Name",
        "  PlantEquipmentOperation:OutdoorDryBulb,  !- Control Scheme 1 Object Type",
        "  ODB Control,             !- Control Scheme 1 Name",
        "  PlantOnSched;            !- Control Scheme 1 Schedule Name",

        "PlantEquipmentOperation:OutdoorDryBulb,",
        "  ODB Control,     !- Name",
        "  -30.0,                   !- Load Range 1 Lower Temperature {C}",
        "  -10.0,                   !- Load Range 1 Upper Temperature {C}",
        "  heating plant,           !- Range 1 Equipment List Name",
        "  -10.0,                   !- Load Range 2 Lower Temperature {C}",
        "  -15.0,                   !- Load Range 2 Upper Temperature {C}",
        "  fighting against itself plant; !- Range 2 Equipment List Name",

        "PlantEquipmentList,",
        "  heating plant,           !- Name",
        "  Boiler:HotWater,         !- Equipment 1 Object Type",
        "  Central Boiler;          !- Equipment 1 Name",

        "PlantEquipmentList,",
        "  cooling plant,            !- Name",
        "  Chiller:Electric,         !- Equipment 1 Object Type",
        "  Central Chiller;          !- Equipment 1 Name",

        "PlantEquipmentList,",
        "  fighting against itself plant, !- Name",
        "  Chiller:Electric,         !- Equipment 1 Object Type",
        "  Central Chiller,          !- Equipment 1 Name",
        "  Boiler:HotWater,          !- Equipment 1 Object Type",
        "  Central Boiler;           !- Equipment 1 Name",

        "Schedule:Compact,",
        "  PlantOnSched,  !- Name",
        "  Fraction,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,1;          !- Field 3",

        "ScheduleTypeLimits,",
        "  Fraction,                !- Name",
        "  0.0,                     !- Lower Limit Value",
        "  1.0,                     !- Upper Limit Value",
        "  CONTINUOUS,              !- Numeric Type",
        "  Dimensionless;           !- Unit Type",

        "Chiller:Electric,",
        "  Central Chiller,         !- Name",
        "  AirCooled,               !- Condenser Type",
        "  autosize,                !- Nominal Capacity {W}",
        "  3.2,                     !- Nominal COP {W/W}",
        "  Central Chiller Inlet Node,  !- Chilled Water Inlet Node Name",
        "  Central Chiller Outlet Node,  !- Chilled Water Outlet Node Name",
        "  Central Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "  Central Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "  0.0,                     !- Minimum Part Load Ratio",
        "  1.0,                     !- Maximum Part Load Ratio",
        "  0.65,                    !- Optimum Part Load Ratio",
        "  35.0,                    !- Design Condenser Inlet Temperature {C}",
        "  2.778,                   !- Temperature Rise Coefficient",
        "  6.67,                    !- Design Chilled Water Outlet Temperature {C}",
        "  autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "  autosize,                !- Design Condenser Fluid Flow Rate {m3/s}",
        "  0.9949,                  !- Coefficient 1 of Capacity Ratio Curve",
        "  -0.045954,               !- Coefficient 2 of Capacity Ratio Curve",
        "  -0.0013543,              !- Coefficient 3 of Capacity Ratio Curve",
        "  2.333,                   !- Coefficient 1 of Power Ratio Curve",
        "  -1.975,                  !- Coefficient 2 of Power Ratio Curve",
        "  0.6121,                  !- Coefficient 3 of Power Ratio Curve",
        "  0.03303,                 !- Coefficient 1 of Full Load Ratio Curve",
        "  0.6852,                  !- Coefficient 2 of Full Load Ratio Curve",
        "  0.2818,                  !- Coefficient 3 of Full Load Ratio Curve",
        "  5,                       !- Chilled Water Outlet Temperature Lower Limit {C}",
        "  LeavingSetpointModulated;!- Chiller Flow Mode",

        "Boiler:HotWater,",
        "  Central Boiler,          !- Name",
        "  NaturalGas,              !- Fuel Type",
        "  autosize,                !- Nominal Capacity {W}",
        "  0.8,                     !- Nominal Thermal Efficiency",
        "  LeavingBoiler,           !- Efficiency Curve Temperature Evaluation Variable",
        "  BoilerEfficiency,        !- Normalized Boiler Efficiency Curve Name",
        "  autosize,                !- Design Water Flow Rate {m3/s}",
        "  0.0,                     !- Minimum Part Load Ratio",
        "  1.2,                     !- Maximum Part Load Ratio",
        "  1.0,                     !- Optimum Part Load Ratio",
        "  Central Boiler Inlet Node,  !- Boiler Water Inlet Node Name",
        "  Central Boiler Outlet Node,  !- Boiler Water Outlet Node Name",
        "  100.,                    !- Water Outlet Upper Temperature Limit {C}",
        "  LeavingSetpointModulated;!- Boiler Flow Mode",
    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    // Setup the plant itself manually
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).TypeOfLoop = EnergyPlus::DataPlant::LoopType::Plant;

    state->dataPlnt->PlantLoop(1).OpScheme.allocate(3);
    state->dataPlnt->PlantLoop(1).OpScheme(1).Name = "Central Chiller Only";
    state->dataPlnt->PlantLoop(1).OpScheme(2).Name = "Central Boiler Only";
    state->dataPlnt->PlantLoop(1).OpScheme(3).Name = "ODB Control";
    state->dataPlnt->PlantLoop(1).OpScheme(1).NumEquipLists = 1;
    state->dataPlnt->PlantLoop(1).OpScheme(2).NumEquipLists = 1;
    state->dataPlnt->PlantLoop(1).OpScheme(3).NumEquipLists = 2;

    // Test 1: Cooling scheme (all good, no errors)
    numSchemes = 3;
    loopNum = 1;
    schemeNum = 1;
    errorFound = false;
    currentModuleObject = "PlantEquipmentOperation:CoolingLoad";
    EnergyPlus::PlantCondLoopOperation::FindRangeBasedOrUncontrolledInput(*state, currentModuleObject, numSchemes, loopNum, schemeNum, errorFound);
    EXPECT_FALSE(errorFound);

    // Test 2: Heating scheme (all good, no errors)
    numSchemes = 3;
    loopNum = 1;
    schemeNum = 2;
    errorFound = false;
    currentModuleObject = "PlantEquipmentOperation:HeatingLoad";
    EnergyPlus::PlantCondLoopOperation::FindRangeBasedOrUncontrolledInput(*state, currentModuleObject, numSchemes, loopNum, schemeNum, errorFound);
    EXPECT_FALSE(errorFound);

    // Test 3: ODB scheme (load range 2 temperatures are incorrect, error found)
    numSchemes = 3;
    loopNum = 1;
    schemeNum = 3;
    errorFound = false;
    currentModuleObject = "PlantEquipmentOperation:OutdoorDryBulb";
    EnergyPlus::PlantCondLoopOperation::FindRangeBasedOrUncontrolledInput(*state, currentModuleObject, numSchemes, loopNum, schemeNum, errorFound);
    EXPECT_TRUE(errorFound);
    EXPECT_TRUE(
        compare_err_stream_substring("found a lower limit that is higher than an upper limit in PlantEquipmentOperation:OutdoorDryBulb", true));
}
