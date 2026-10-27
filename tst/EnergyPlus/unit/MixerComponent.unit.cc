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

// EnergyPlus::Standalone MixerComponent Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/Psychrometrics.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::MixerComponent;

TEST_F(EnergyPlusFixture, GetZoneMixerIndex)
{
    // locals
    std::string CurrentModuleObject;
    std::string LINE;
    bool errFlag;
    int MixerIndex;
    // set some variables
    CurrentModuleObject = "AirLoopHVAC:ZoneMixer";
    state->dataMixerComponent->NumMixers = 3;
    errFlag = false;
    // allocate needed arrays
    state->dataMixerComponent->MixerCond.allocate(state->dataMixerComponent->NumMixers);
    state->dataMixerComponent->MixerCond(1).MixerName = "SPACE1-1 ATU Mixer";
    state->dataMixerComponent->MixerCond(2).MixerName = "SPACE2-1 ATU Mixer";
    state->dataMixerComponent->MixerCond(3).MixerName = "SPACE3-1 ATU Mixer";
    GetZoneMixerIndex(*state, state->dataMixerComponent->MixerCond(2).MixerName, MixerIndex, errFlag, CurrentModuleObject);
    EXPECT_EQ(2, MixerIndex);
    EXPECT_FALSE(errFlag);
    GetZoneMixerIndex(*state, "SPACE3-3 ATU Mixer", MixerIndex, errFlag, CurrentModuleObject);
    EXPECT_EQ(0, MixerIndex);
    EXPECT_TRUE(errFlag);
}

namespace {

// Two-inlet mixer whose inlet conditions are read from nodes 1 and 2 by InitAirMixer; node 3 is the outlet
void setupTwoInletMixer(EnergyPlusData &state)
{
    constexpr int numInletNodes = 2;
    state.dataMixerComponent->MixerCond.allocate(1);
    auto &mixer = state.dataMixerComponent->MixerCond(1);
    mixer.NumInletNodes = numInletNodes;
    mixer.OutletNode = numInletNodes + 1;
    mixer.InletNode.allocate(numInletNodes);
    mixer.InletMassFlowRate.allocate(numInletNodes);
    mixer.InletMassFlowRateMaxAvail.allocate(numInletNodes);
    mixer.InletMassFlowRateMinAvail.allocate(numInletNodes);
    mixer.InletTemp.allocate(numInletNodes);
    mixer.InletHumRat.allocate(numInletNodes);
    mixer.InletEnthalpy.allocate(numInletNodes);
    mixer.InletPressure.allocate(numInletNodes);
    mixer.InletNode(1) = 1;
    mixer.InletNode(2) = 2;

    state.dataLoopNodes->Node.allocate(numInletNodes + 1);
    state.dataContaminantBalance->Contaminant.CO2Simulation = true;
    state.dataContaminantBalance->Contaminant.GenericContamSimulation = true;
}

void setInletNode(EnergyPlusData &state, int nodeNum, Real64 massFlowRate, Real64 temp, Real64 humRat, Real64 press, Real64 co2, Real64 genContam)
{
    auto &node = state.dataLoopNodes->Node(nodeNum);
    node.MassFlowRate = massFlowRate;
    node.MassFlowRateMaxAvail = massFlowRate;
    node.MassFlowRateMinAvail = 0.0;
    node.Temp = temp;
    node.HumRat = humRat;
    node.Enthalpy = Psychrometrics::PsyHFnTdbW(temp, humRat);
    node.Press = press;
    node.CO2 = co2;
    node.GenContam = genContam;
}

void simMixer(EnergyPlusData &state)
{
    int mixerNum = 1;
    InitAirMixer(state, mixerNum);
    CalcAirMixer(state, mixerNum);
    UpdateAirMixer(state, mixerNum);
}

} // namespace

TEST_F(EnergyPlusFixture, MixerAveragesInletsAtZeroFlow)
{
    setupTwoInletMixer(*state);
    setInletNode(*state, 1, 0.0, 20.0, 0.005, 99000.0, 450.0, 2.0);
    setInletNode(*state, 2, 0.0, 30.0, 0.010, 101000.0, 800.0, 4.0);

    simMixer(*state);

    auto const &outletNode = state->dataLoopNodes->Node(3);
    Real64 const expectedEnthalpy = (state->dataLoopNodes->Node(1).Enthalpy + state->dataLoopNodes->Node(2).Enthalpy) / 2.0;
    EXPECT_DOUBLE_EQ(0.0, outletNode.MassFlowRate);
    EXPECT_DOUBLE_EQ(25.0, outletNode.Temp);
    EXPECT_DOUBLE_EQ(0.0075, outletNode.HumRat);
    EXPECT_DOUBLE_EQ(100000.0, outletNode.Press);
    EXPECT_DOUBLE_EQ(expectedEnthalpy, outletNode.Enthalpy);
    EXPECT_DOUBLE_EQ(625.0, outletNode.CO2);
    EXPECT_DOUBLE_EQ(3.0, outletNode.GenContam);

    // Reversing the inlet order gives the same outlet state
    auto &mixer = state->dataMixerComponent->MixerCond(1);
    mixer.InletNode(1) = 2;
    mixer.InletNode(2) = 1;
    simMixer(*state);

    EXPECT_DOUBLE_EQ(25.0, outletNode.Temp);
    EXPECT_DOUBLE_EQ(0.0075, outletNode.HumRat);
    EXPECT_DOUBLE_EQ(100000.0, outletNode.Press);
    EXPECT_DOUBLE_EQ(expectedEnthalpy, outletNode.Enthalpy);
    EXPECT_DOUBLE_EQ(625.0, outletNode.CO2);
    EXPECT_DOUBLE_EQ(3.0, outletNode.GenContam);
}

TEST_F(EnergyPlusFixture, MixerUsesCurrentInletsAtZeroFlowAfterFlow)
{
    setupTwoInletMixer(*state);
    setInletNode(*state, 1, 1.0, 20.0, 0.005, 99000.0, 450.0, 2.0);
    setInletNode(*state, 2, 2.0, 30.0, 0.010, 101000.0, 800.0, 4.0);
    simMixer(*state);

    // When flow stops, the outlet follows the current inlet conditions instead of holding the previous outlet state
    setInletNode(*state, 1, 0.0, 10.0, 0.002, 98000.0, 100.0, 0.5);
    setInletNode(*state, 2, 0.0, 14.0, 0.004, 100000.0, 200.0, 1.0);
    simMixer(*state);

    auto const &outletNode = state->dataLoopNodes->Node(3);
    Real64 const expectedEnthalpy = (state->dataLoopNodes->Node(1).Enthalpy + state->dataLoopNodes->Node(2).Enthalpy) / 2.0;
    EXPECT_DOUBLE_EQ(0.0, outletNode.MassFlowRate);
    EXPECT_DOUBLE_EQ(12.0, outletNode.Temp);
    EXPECT_DOUBLE_EQ(0.003, outletNode.HumRat);
    EXPECT_DOUBLE_EQ(99000.0, outletNode.Press);
    EXPECT_DOUBLE_EQ(expectedEnthalpy, outletNode.Enthalpy);
    EXPECT_DOUBLE_EQ(150.0, outletNode.CO2);
    EXPECT_DOUBLE_EQ(0.75, outletNode.GenContam);
}
