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

// EnergyPlus::DataPlant Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataPlant;

TEST_F(EnergyPlusFixture, DataPlant_AnyPlantLoopSidesNeedSim)
{
    state->dataPlnt->TotNumLoops = 3;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    EXPECT_TRUE(PlantUtilities::AnyPlantLoopSidesNeedSim(*state)); // SimLoopSideNeeded is set to true in default ctor
    PlantUtilities::SetAllPlantSimFlagsToValue(*state, false);     // Set all SimLoopSideNeeded to false
    EXPECT_FALSE(PlantUtilities::AnyPlantLoopSidesNeedSim(*state));
}

TEST_F(EnergyPlusFixture, DataPlant_verifyTwoNodeNumsOnSamePlantLoop)
{

    // not using the DataPlantTest base class because of how specific this one is and that one is very general
    if (state->dataPlnt->PlantLoop.allocated()) {
        state->dataPlnt->PlantLoop.deallocate();
    }
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    // initialize all node numbers to zero
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = 0;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 0;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = 0;

    // specify the node numbers of interest
    int constexpr nodeNumA = 1;
    int constexpr nodeNumB = 2;

    // first test, expected pass
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = 2;
    EXPECT_TRUE(PlantUtilities::verifyTwoNodeNumsOnSamePlantLoop(*state, nodeNumA, nodeNumB));

    // reset node numbers
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = 0;

    // second test, expected false
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 2;
    EXPECT_FALSE(PlantUtilities::verifyTwoNodeNumsOnSamePlantLoop(*state, nodeNumA, nodeNumB));

    state->dataPlnt->TotNumLoops = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.deallocate();
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.deallocate();
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.deallocate();
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.deallocate();
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.deallocate();
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.deallocate();
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.deallocate();
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
}

// Three unit tests for the condenser loop constant speed branch pump cold start deadlock
// (chiller runs and rejects heat, but the cooling tower/condenser pump stay off).
// See HalfLoopData::DisableAnyBranchPumpsConnectedToUnloadedEquipment in Plant/LoopSide.cc
TEST_F(EnergyPlusFixture, PlantLoopSide_CondenserTowerBranchPumpNotDisabledWhenDemandCallsForFlow)
{
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    auto &loop = state->dataPlnt->PlantLoop(1);
    loop.TypeOfLoop = DataPlant::LoopType::Condenser;

    auto &supply = loop.LoopSide(DataPlant::LoopSideLocation::Supply);
    supply.TotalBranches = 3; // inlet(1), parallel tower branch(2), outlet(3)
    supply.Branch.allocate(3);
    supply.plantLoc.loopNum = 1;
    supply.plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;

    auto &branch = supply.Branch(2);
    branch.TotalComponents = 2;
    branch.Comp.allocate(2);
    branch.Comp(1).Type = DataPlant::PlantEquipmentType::PumpConstantSpeed; // pumps are skipped in the load sum
    branch.Comp(1).MyLoad = 0.0;
    branch.Comp(2).Type = DataPlant::PlantEquipmentType::CoolingTower_TwoSpd; // tower, unloaded because no flow yet
    branch.Comp(2).MyLoad = 0.0;
    branch.disableOverrideForCSBranchPumping = false;

    // the chiller condenser on the demand side is asking for flow
    loop.LoopSide(DataPlant::LoopSideLocation::Demand).flowRequestNeedAndTurnOn = 1.0;

    supply.DisableAnyBranchPumpsConnectedToUnloadedEquipment(*state);

    // the branch pump must stay available, otherwise the loop can never start circulating
    EXPECT_FALSE(supply.Branch(2).disableOverrideForCSBranchPumping);
}

TEST_F(EnergyPlusFixture, PlantLoopSide_CondenserTowerBranchPumpDisabledWhenNoDemand)
{
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    auto &loop = state->dataPlnt->PlantLoop(1);
    loop.TypeOfLoop = DataPlant::LoopType::Condenser;

    auto &supply = loop.LoopSide(DataPlant::LoopSideLocation::Supply);
    supply.TotalBranches = 3;
    supply.Branch.allocate(3);
    supply.plantLoc.loopNum = 1;
    supply.plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;

    auto &branch = supply.Branch(2);
    branch.TotalComponents = 2;
    branch.Comp.allocate(2);
    branch.Comp(1).Type = DataPlant::PlantEquipmentType::PumpConstantSpeed;
    branch.Comp(1).MyLoad = 0.0;
    branch.Comp(2).Type = DataPlant::PlantEquipmentType::CoolingTower_TwoSpd;
    branch.Comp(2).MyLoad = 0.0;
    branch.disableOverrideForCSBranchPumping = false;

    // nothing on the demand side wants flow
    loop.LoopSide(DataPlant::LoopSideLocation::Demand).flowRequestNeedAndTurnOn = 0.0;

    supply.DisableAnyBranchPumpsConnectedToUnloadedEquipment(*state);

    // existing behavior is preserved: an unloaded branch pump is still shut off
    EXPECT_TRUE(supply.Branch(2).disableOverrideForCSBranchPumping);
}

TEST_F(EnergyPlusFixture, PlantLoopSide_PlantLoopBranchPumpStillDisabledEvenWithDemand)
{
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    auto &loop = state->dataPlnt->PlantLoop(1);
    loop.TypeOfLoop = DataPlant::LoopType::Plant; // not a condenser loop

    auto &supply = loop.LoopSide(DataPlant::LoopSideLocation::Supply);
    supply.TotalBranches = 3;
    supply.Branch.allocate(3);
    supply.plantLoc.loopNum = 1;
    supply.plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;

    auto &branch = supply.Branch(2);
    branch.TotalComponents = 2;
    branch.Comp.allocate(2);
    branch.Comp(1).Type = DataPlant::PlantEquipmentType::PumpConstantSpeed;
    branch.Comp(1).MyLoad = 0.0;
    branch.Comp(2).Type = DataPlant::PlantEquipmentType::CoolingTower_TwoSpd;
    branch.Comp(2).MyLoad = 0.0;
    branch.disableOverrideForCSBranchPumping = false;

    loop.LoopSide(DataPlant::LoopSideLocation::Demand).flowRequestNeedAndTurnOn = 1.0;

    supply.DisableAnyBranchPumpsConnectedToUnloadedEquipment(*state);

    // the new exception is scoped to condenser loops only
    EXPECT_TRUE(supply.Branch(2).disableOverrideForCSBranchPumping);
}
