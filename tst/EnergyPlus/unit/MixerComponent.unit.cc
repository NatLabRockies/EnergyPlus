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
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>

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

TEST_F(EnergyPlusFixture, MixerParallelPIULeakageMatchesReturnNode)
{
    // Return path mixer with inlet nodes 11 and 12. Zone 1 returns to node 12 on air loop 3; zone 2 returns to node 20,
    // which is not connected to the mixer, on air loop 1. Leakage must only come from the PIU serving zone 1.
    int mixerNum = 1;
    constexpr int numInletNodes = 2;
    constexpr int outletNodeNum = 13;

    state->dataLoopNodes->Node.allocate(20);
    state->dataMixerComponent->MixerCond.allocate(1);
    auto &mixer = state->dataMixerComponent->MixerCond(mixerNum);
    mixer.MixerName = "RETURN MIXER";
    mixer.NumInletNodes = numInletNodes;
    mixer.OutletNode = outletNodeNum;
    mixer.InletNode.allocate(numInletNodes);
    mixer.InletNode(1) = 11;
    mixer.InletNode(2) = 12;
    mixer.InletMassFlowRate.dimension(numInletNodes, 0.5);
    mixer.InletMassFlowRateMaxAvail.dimension(numInletNodes, 0.5);
    mixer.InletMassFlowRateMinAvail.dimension(numInletNodes, 0.0);
    mixer.InletTemp.dimension(numInletNodes, 22.0);
    mixer.InletHumRat.dimension(numInletNodes, 0.008);
    mixer.InletEnthalpy.dimension(numInletNodes, 42500.0);
    mixer.InletPressure.dimension(numInletNodes, 101325.0);

    state->dataZoneEquip->ReturnAirPath.allocate(1);
    auto &returnAirPath = state->dataZoneEquip->ReturnAirPath(1);
    returnAirPath.NumOfComponents = 1;
    returnAirPath.ComponentName.allocate(1);
    returnAirPath.ComponentName(1) = mixer.MixerName;
    returnAirPath.ComponentTypeEnum.allocate(1);
    returnAirPath.ComponentTypeEnum(1) = DataZoneEquipment::AirLoopHVACZone::Mixer;

    state->dataZoneEquip->ZoneEquipConfig.allocate(2);
    auto setupZone = [&](int zoneNum, int returnNode, int airLoopNum) {
        auto &zoneEquipConfig = state->dataZoneEquip->ZoneEquipConfig(zoneNum);
        zoneEquipConfig.NumReturnNodes = 1;
        zoneEquipConfig.ReturnNode.allocate(1);
        zoneEquipConfig.ReturnNode(1) = returnNode;
        zoneEquipConfig.ReturnNodeAirLoopNum.allocate(1);
        zoneEquipConfig.ReturnNodeAirLoopNum(1) = airLoopNum;
    };
    setupZone(1, 12, 3);
    setupZone(2, 20, 1);

    // leak air conditions match the return air so only the flow rate is affected
    constexpr int leakZoneNum = 2;
    auto &leakNode = state->dataLoopNodes->Node(leakZoneNum);
    leakNode.HumRat = 0.008;
    leakNode.Press = 101325.0;
    leakNode.Enthalpy = 42500.0;

    state->dataPowerInductionUnits->NumParallelPIUs = 2;
    state->dataDefineEquipment->AirDistUnit.allocate(2);
    for (int aduNum = 1; aduNum <= 2; ++aduNum) {
        auto &adu = state->dataDefineEquipment->AirDistUnit(aduNum);
        adu.ZoneNum = aduNum;
        adu.piuLkZoneNum = leakZoneNum;
    }
    state->dataDefineEquipment->AirDistUnit(1).massFlowRateParallelPIULk = 0.1;
    state->dataDefineEquipment->AirDistUnit(2).massFlowRateParallelPIULk = 0.2;

    CalcAirMixer(*state, mixerNum);

    EXPECT_DOUBLE_EQ(1.1, mixer.OutletMassFlowRate);
    EXPECT_DOUBLE_EQ(0.008, mixer.OutletHumRat);
    EXPECT_DOUBLE_EQ(42500.0, mixer.OutletEnthalpy);
}
