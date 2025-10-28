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

// EnergyPlus::HVACInterfaceManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/HVACInterfaceManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {
TEST_F(EnergyPlusFixture, ExcessiveHeatStorage_Test)
{
    state->init_state(*state);
    using namespace DataPlant;
    using namespace HVACInterfaceManager;
    Real64 TankOutletTemp;
    state->dataHVACGlobal->TimeStepSys = 1;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    // Set Up PlantLoop Variables
    state->dataPlnt->PlantLoop(1).Mass = 50;
    state->dataPlnt->PlantLoop(1).FluidName = "Water";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn = 1;
    // Note LastTempInterfaceTankOutlet ends up getting reset to zero on the first pass
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LastTempInterfaceTankOutlet = 80;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalPumpHeat = 500;
    state->dataLoopNodes->Node.allocate(state->dataPlnt->TotNumLoops);
    state->dataLoopNodes->Node(1).Temp = 100;
    state->dataLoopNodes->Node(1).MassFlowRate = 10;
    state->dataPlnt->PlantLoop(1).OutletNodeFlowrate = 10;

    // LoopSideInlet_MdotCpDeltaT should be < LoopSideInlet_McpDTdt
    // Therefore CapExcessStorageTime AND TotalTime will increase by 1 timestep
    UpdateHalfLoopInletTemp(*state, 1, DataPlant::LoopSideLocation::Demand, TankOutletTemp);
    // Excess storage calcs moved here
    PlantManager::UpdateNodeThermalHistory(*state);
    EXPECT_NEAR((2928.82 - 500), state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_MdotCpDeltaT, 0.001);
    EXPECT_NEAR(2928.82, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_McpDTdt, 0.001);
    EXPECT_EQ(1, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_CapExcessStorageTime);
    EXPECT_EQ(1, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_TotalTime);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LastTempInterfaceTankOutlet = 120; // random

    // LoopSideInlet_MdotCpDeltaT should be > LoopSideInlet_McpDTdt
    // Therefore TotalTime will increase by 1 more timestep, but CapExcessStorageTime will NOT increase
    UpdateHalfLoopInletTemp(*state, 1, DataPlant::LoopSideLocation::Demand, TankOutletTemp);
    // Excess storage calcs moved here
    PlantManager::UpdateNodeThermalHistory(*state);
    EXPECT_NEAR((-588.264 - 500), state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_MdotCpDeltaT, 0.001);
    EXPECT_NEAR(-588.264, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_McpDTdt, .001);
    EXPECT_EQ(1, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_CapExcessStorageTime);
    EXPECT_EQ(2, state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_TotalTime);
}

TEST_F(EnergyPlusFixture, UpdateHVACInterface_Test)
{
    using namespace DataPlant;
    using namespace HVACInterfaceManager;

    int AirLoopNum = 1;
    int InletNode = 1;
    int OutletNode = 2;
    bool OutOfToleranceFlag = false;

    state->dataHVACInterfaceMgr->TmpRealARR.allocate(10);
    state->dataConvergeParams->AirLoopConvergence.allocate(AirLoopNum);
    state->dataLoopNodes->Node.allocate(2);

    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(OutletNode).MassFlowRate = 0.01;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.001;
    state->dataLoopNodes->Node(OutletNode).HumRat = 0.001;
    state->dataLoopNodes->Node(InletNode).Temp = 23.0;
    state->dataLoopNodes->Node(OutletNode).Temp = 23.0;
    state->dataLoopNodes->Node(InletNode).Enthalpy = Psychrometrics::PsyHFnTdbW(23.0, 0.001);
    state->dataLoopNodes->Node(OutletNode).Enthalpy = Psychrometrics::PsyHFnTdbW(23.0, 0.001);
    state->dataLoopNodes->Node(InletNode).Press = 101325.0;
    state->dataLoopNodes->Node(OutletNode).Press = 101325.0;
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataContaminantBalance->Contaminant.GenericContamSimulation = true;
    state->dataLoopNodes->Node(InletNode).CO2 = 400.0;
    state->dataLoopNodes->Node(OutletNode).CO2 = 400.0;
    state->dataLoopNodes->Node(InletNode).GenContam = 20.0;
    state->dataLoopNodes->Node(OutletNode).GenContam = 20.0;

    UpdateHVACInterface(*state, AirLoopNum, DataConvergParams::CalledFrom::AirSystemDemandSide, OutletNode, InletNode, OutOfToleranceFlag);

    EXPECT_FALSE(OutOfToleranceFlag);
    EXPECT_FALSE(state->dataConvergeParams->AirLoopConvergence(1).HVACCO2NotConverged[0]);
    EXPECT_FALSE(state->dataConvergeParams->AirLoopConvergence(1).HVACGenContamNotConverged[0]);

    UpdateHVACInterface(*state, AirLoopNum, DataConvergParams::CalledFrom::AirSystemSupplySideDeck1, OutletNode, InletNode, OutOfToleranceFlag);

    EXPECT_FALSE(OutOfToleranceFlag);
    EXPECT_FALSE(state->dataConvergeParams->AirLoopConvergence(1).HVACCO2NotConverged[1]);
    EXPECT_FALSE(state->dataConvergeParams->AirLoopConvergence(1).HVACGenContamNotConverged[1]);

    UpdateHVACInterface(*state, AirLoopNum, DataConvergParams::CalledFrom::AirSystemSupplySideDeck2, OutletNode, InletNode, OutOfToleranceFlag);

    EXPECT_FALSE(OutOfToleranceFlag);
    EXPECT_FALSE(state->dataConvergeParams->AirLoopConvergence(1).HVACCO2NotConverged[2]);
    EXPECT_FALSE(state->dataConvergeParams->AirLoopConvergence(1).HVACGenContamNotConverged[2]);

    state->dataLoopNodes->Node(InletNode).CO2 = 400.0;
    state->dataLoopNodes->Node(InletNode).GenContam = 20.0;
    state->dataLoopNodes->Node(OutletNode).CO2 = 401.0;
    state->dataLoopNodes->Node(OutletNode).GenContam = 20.5;

    UpdateHVACInterface(*state, AirLoopNum, DataConvergParams::CalledFrom::AirSystemDemandSide, OutletNode, InletNode, OutOfToleranceFlag);

    EXPECT_TRUE(OutOfToleranceFlag);
    EXPECT_TRUE(state->dataConvergeParams->AirLoopConvergence(1).HVACCO2NotConverged[0]);
    EXPECT_TRUE(state->dataConvergeParams->AirLoopConvergence(1).HVACGenContamNotConverged[0]);

    state->dataLoopNodes->Node(InletNode).CO2 = 400.0;
    state->dataLoopNodes->Node(InletNode).GenContam = 20.0;
    state->dataLoopNodes->Node(OutletNode).CO2 = 401.0;
    state->dataLoopNodes->Node(OutletNode).GenContam = 20.5;
    UpdateHVACInterface(*state, AirLoopNum, DataConvergParams::CalledFrom::AirSystemSupplySideDeck1, OutletNode, InletNode, OutOfToleranceFlag);

    EXPECT_TRUE(OutOfToleranceFlag);
    EXPECT_TRUE(state->dataConvergeParams->AirLoopConvergence(1).HVACCO2NotConverged[1]);
    EXPECT_TRUE(state->dataConvergeParams->AirLoopConvergence(1).HVACGenContamNotConverged[1]);

    state->dataLoopNodes->Node(InletNode).CO2 = 400.0;
    state->dataLoopNodes->Node(InletNode).GenContam = 20.0;
    state->dataLoopNodes->Node(OutletNode).CO2 = 401.0;
    state->dataLoopNodes->Node(OutletNode).GenContam = 20.5;
    UpdateHVACInterface(*state, AirLoopNum, DataConvergParams::CalledFrom::AirSystemSupplySideDeck2, OutletNode, InletNode, OutOfToleranceFlag);

    EXPECT_TRUE(OutOfToleranceFlag);
    EXPECT_TRUE(state->dataConvergeParams->AirLoopConvergence(1).HVACCO2NotConverged[2]);
    EXPECT_TRUE(state->dataConvergeParams->AirLoopConvergence(1).HVACGenContamNotConverged[2]);
}
} // namespace EnergyPlus
