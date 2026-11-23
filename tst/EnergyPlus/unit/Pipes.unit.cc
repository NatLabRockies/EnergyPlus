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

// EnergyPlus::Pumps Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/PipeHeatTransfer.hh>
#include <EnergyPlus/Pipes.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, TestPipesInput)
{

    std::string const idf_objects = delimited_string({
        "Pipe:Adiabatic,",
        " Pipe Name,           !- Name",
        " Pipe Inlet Node,     !- Inlet Node Name",
        " Pipe Outlet Node;    !- Outlet Node Name",
        "Pipe:Adiabatic:Steam,",
        " Pipe Name 2,           !- Name",
        " Pipe Inlet Node 2,     !- Inlet Node Name",
        " Pipe Outlet Node 2;    !- Outlet Node Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    Pipes::GetPipeInput(*state);
    EXPECT_EQ(2u, state->dataPipes->LocalPipe.size());
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::Pipe, state->dataPipes->LocalPipe(1).Type);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::PipeSteam, state->dataPipes->LocalPipe(2).Type);
}

TEST_F(EnergyPlusFixture, CalcPipeHeatTransCoef)
{
    state->init_state(*state);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataLoopNodes->Node.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "Indoor Pipe";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::PipeInterior;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 0; // just skip the supply side search
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataPipeHT->nsvNumOfPipeHT = 1;
    state->dataPipeHT->PipeHT.allocate(state->dataPipeHT->nsvNumOfPipeHT);
    state->dataPipeHT->PipeHTUniqueNames.reserve(static_cast<unsigned>(state->dataPipeHT->nsvNumOfPipeHT));
    state->dataPipeHT->GetPipeInputFlag = false;
    auto &pipe = state->dataPipeHT->PipeHT(1);
    pipe.Name = "Indoor Pipe";
    pipe.Type = DataPlant::PlantEquipmentType::PipeInterior;
    pipe.Construction = "Pipe construction";
    pipe.ConstructionNum = 1;
    pipe.InletNodeNum = 1;
    pipe.OutletNodeNum = 2;
    int constexpr NumPipeSections(20);
    pipe.FluidTemp.allocate({0, NumPipeSections});
    pipe.FluidTemp = 7.0;

    bool errFlag = false;
    // test simple searching first
    PlantUtilities::ScanPlantLoopsForObject(*state, pipe.Name, pipe.Type, pipe.plantLoc, errFlag);
    ASSERT_FALSE(errFlag);
    EXPECT_EQ(1, pipe.plantLoc.loopNum);
    EXPECT_ENUM_EQ(DataPlant::LoopSideLocation::Demand, pipe.plantLoc.loopSideNum);
    EXPECT_EQ(1, pipe.plantLoc.branchNum);
    EXPECT_EQ(1, pipe.plantLoc.compNum);

    constexpr Real64 massFlowRate = 0.5;
    constexpr Real64 diameter = 0.05;
    // Try a temperrature below the min
    EXPECT_NO_THROW(pipe.CalcPipeHeatTransCoef(*state, 1.0, massFlowRate, diameter));
    // Try a temperature above the max
    EXPECT_NO_THROW(pipe.CalcPipeHeatTransCoef(*state, 65.0, massFlowRate, diameter));
}

} // namespace EnergyPlus
