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

TEST_F(EnergyPlusFixture, MixerRetainsOutletConditionsAtZeroFlow)
{
    int mixerNum = 1;
    constexpr int numInletNodes = 2;
    constexpr int outletNodeNum = numInletNodes + 1;

    state->dataMixerComponent->MixerCond.allocate(1);
    auto &mixer = state->dataMixerComponent->MixerCond(mixerNum);
    mixer.NumInletNodes = numInletNodes;
    mixer.OutletNode = outletNodeNum;
    mixer.InletNode.allocate(numInletNodes);
    mixer.InletMassFlowRate.allocate(numInletNodes);
    mixer.InletMassFlowRateMaxAvail.allocate(numInletNodes);
    mixer.InletMassFlowRateMinAvail.allocate(numInletNodes);
    mixer.InletTemp.allocate(numInletNodes);
    mixer.InletHumRat.allocate(numInletNodes);
    mixer.InletEnthalpy.allocate(numInletNodes);
    mixer.InletPressure.allocate(numInletNodes);

    state->dataLoopNodes->Node.allocate(outletNodeNum);
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataContaminantBalance->Contaminant.GenericContamSimulation = true;

    for (int inletNum = 1; inletNum <= numInletNodes; ++inletNum) {
        mixer.InletNode(inletNum) = inletNum;
        mixer.InletMassFlowRate(inletNum) = inletNum;
        mixer.InletMassFlowRateMaxAvail(inletNum) = inletNum;
        mixer.InletMassFlowRateMinAvail(inletNum) = 0.0;
    }

    mixer.InletTemp(1) = 20.0;
    mixer.InletTemp(2) = 30.0;
    mixer.InletHumRat(1) = 0.005;
    mixer.InletHumRat(2) = 0.010;
    mixer.InletPressure(1) = 99000.0;
    mixer.InletPressure(2) = 101000.0;
    mixer.InletEnthalpy(1) = 33000.0;
    mixer.InletEnthalpy(2) = 50000.0;
    state->dataLoopNodes->Node(1).CO2 = 450.0;
    state->dataLoopNodes->Node(2).CO2 = 800.0;
    state->dataLoopNodes->Node(1).GenContam = 2.0;
    state->dataLoopNodes->Node(2).GenContam = 4.0;

    CalcAirMixer(*state, mixerNum);
    UpdateAirMixer(*state, mixerNum);

    auto &outletNode = state->dataLoopNodes->Node(outletNodeNum);
    Real64 const outletTemp = outletNode.Temp;
    Real64 const outletHumRat = outletNode.HumRat;
    Real64 const outletPressure = outletNode.Press;
    Real64 const outletEnthalpy = outletNode.Enthalpy;
    Real64 const outletCO2 = outletNode.CO2;
    Real64 const outletGenericContaminant = outletNode.GenContam;

    mixer.InletMassFlowRate = 0.0;
    mixer.InletMassFlowRateMaxAvail = 0.0;
    mixer.InletTemp = 5.0;
    mixer.InletHumRat = 0.001;
    mixer.InletPressure = 95000.0;
    mixer.InletEnthalpy = 10000.0;
    state->dataLoopNodes->Node(1).CO2 = 100.0;
    state->dataLoopNodes->Node(2).CO2 = 200.0;
    state->dataLoopNodes->Node(1).GenContam = 0.5;
    state->dataLoopNodes->Node(2).GenContam = 1.0;

    CalcAirMixer(*state, mixerNum);
    UpdateAirMixer(*state, mixerNum);

    EXPECT_DOUBLE_EQ(0.0, outletNode.MassFlowRate);
    EXPECT_DOUBLE_EQ(outletTemp, outletNode.Temp);
    EXPECT_DOUBLE_EQ(outletHumRat, outletNode.HumRat);
    EXPECT_DOUBLE_EQ(outletPressure, outletNode.Press);
    EXPECT_DOUBLE_EQ(outletEnthalpy, outletNode.Enthalpy);
    EXPECT_DOUBLE_EQ(outletCO2, outletNode.CO2);
    EXPECT_DOUBLE_EQ(outletGenericContaminant, outletNode.GenContam);
}
