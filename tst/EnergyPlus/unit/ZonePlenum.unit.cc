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

// EnergyPlus::ZonePlenum Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/ZonePlenum.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::ZonePlenum;
using namespace EnergyPlus::DataLoopNode;

TEST_F(EnergyPlusFixture, ZonePlenum_InitAirZoneReturnPlenumTest)
{
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataContaminantBalance->Contaminant.GenericContamSimulation = true;

    state->dataZonePlenum->NumZoneReturnPlenums = 1;
    state->dataZonePlenum->ZoneRetPlenCond.allocate(state->dataZonePlenum->NumZoneReturnPlenums);
    int ZonePlenumNum = 1;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInletNodes = 0; // To avoid initializing extra zone equip config and ADU data
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes = 2;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InletNode.allocate(1); // Needed for the Update routine
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedNode.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedMassFlowRate.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedMassFlowRateMaxAvail.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedMassFlowRateMinAvail.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedTemp.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedHumRat.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedEnthalpy.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedPressure.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedCO2.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum)
        .InducedGenContam.allocate(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes);
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InletNode(1) = 1;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRate = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRateMaxAvail = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRateMinAvail = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedTemp = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedHumRat = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedEnthalpy = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedPressure = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedCO2 = 0.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedGenContam = 0.0;

    state->dataLoopNodes->Node.allocate(4); // One node per plenum plus total of NumInducedNodes for all plenums)
    int ZoneNodeNum = 1;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).ZoneNodeNum = ZoneNodeNum;
    state->dataLoopNodes->Node(ZoneNodeNum).Temp = 24.2;
    state->dataLoopNodes->Node(ZoneNodeNum).HumRat = 0.0003;
    state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy = 40000.0;
    state->dataLoopNodes->Node(ZoneNodeNum).Press = 99000.0;
    state->dataLoopNodes->Node(ZoneNodeNum).CO2 = 950.0;
    state->dataLoopNodes->Node(ZoneNodeNum).GenContam = 100.0;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).OutletPressure = 99000.0;

    int InducedNodeIndex = 1;
    int InducedNodeNum = 2;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedNode(InducedNodeIndex) = InducedNodeNum;
    state->dataLoopNodes->Node(InducedNodeNum).MassFlowRate = 0.20;
    state->dataLoopNodes->Node(InducedNodeNum).MassFlowRateMaxAvail = 0.25;
    state->dataLoopNodes->Node(InducedNodeNum).MassFlowRateMinAvail = 0.10;

    InducedNodeIndex = 2;
    InducedNodeNum = 3;
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedNode(InducedNodeIndex) = InducedNodeNum;
    state->dataLoopNodes->Node(InducedNodeNum).MassFlowRate = 0.40;
    state->dataLoopNodes->Node(InducedNodeNum).MassFlowRateMaxAvail = 0.50;
    state->dataLoopNodes->Node(InducedNodeNum).MassFlowRateMinAvail = 0.22;

    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).OutletNode = 4;

    InitAirZoneReturnPlenum(*state, ZonePlenumNum);
    UpdateAirZoneReturnPlenum(*state, ZonePlenumNum);

    EXPECT_EQ(state->dataLoopNodes->Node(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).ZoneNodeNum).CO2,
              state->dataLoopNodes->Node(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).OutletNode).CO2);
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).ZoneNodeNum).CO2,
              state->dataLoopNodes->Node(state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).OutletNode).CO2);

    for (InducedNodeIndex = 1; InducedNodeIndex <= state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).NumInducedNodes; ++InducedNodeIndex) {
        InducedNodeNum = state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedNode(InducedNodeIndex);
        EXPECT_EQ(state->dataLoopNodes->Node(InducedNodeNum).MassFlowRate,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRate(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(InducedNodeNum).MassFlowRateMaxAvail,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRateMaxAvail(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(InducedNodeNum).MassFlowRateMinAvail,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRateMinAvail(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedTemp(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).HumRat,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedHumRat(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedEnthalpy(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).Press,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedPressure(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).CO2, state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedCO2(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).GenContam,
                  state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedGenContam(InducedNodeIndex));
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).Temp, state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).ZoneTemp);
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).HumRat, state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).ZoneHumRat);
        EXPECT_EQ(state->dataLoopNodes->Node(ZoneNodeNum).Enthalpy, state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).ZoneEnthalpy);
    }

    // Deallocate everything
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InletNode.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedNode.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRate.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRateMaxAvail.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedMassFlowRateMinAvail.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedTemp.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedHumRat.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedEnthalpy.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedPressure.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedCO2.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond(ZonePlenumNum).InducedGenContam.deallocate();
    state->dataZonePlenum->ZoneRetPlenCond.deallocate();
    state->dataLoopNodes->Node.deallocate();
}
