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

// EnergyPlus::DXCoils unit tests
// Secondary DX cooling and heating coil heat rejection and removal rate
// from a secondary zone and SHR calculation

// Google test headers
#include <gtest/gtest.h>

// C++ Headers
#include <string>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace DXCoils;
using Psychrometrics::PsyHFnTdbW;
using Psychrometrics::PsyRhoAirFnPbTdbW;
using Psychrometrics::PsyTwbFnTdbWPb;

TEST_F(EnergyPlusFixture, SecondaryDXCoolingCoilSingleSpeed_Test1)
{
    // tests secondary DX coil calculation of single speed DX system or heat pump
    int DXCoilNum;

    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoil(DXCoilNum).IsSecondaryDXCoilInZone = true;
    state->dataDXCoils->DXCoil(DXCoilNum).coilType = HVAC::CoilType::CoolingDXSingleSpeed;
    state->dataDXCoils->DXCoil(DXCoilNum).TotalCoolingEnergyRate = 5000.0;
    state->dataDXCoils->DXCoil(DXCoilNum).ElecCoolingPower = 500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatGainRate = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataDXCoils->DXCoil(DXCoilNum).SecZonePtr = 1;

    CalcSecondaryDXCoils(*state, DXCoilNum);
    EXPECT_DOUBLE_EQ(5500.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatGainRate);

    // cleanup
    state->dataDXCoils->DXCoil.deallocate();
}
TEST_F(EnergyPlusFixture, SecondaryDXCoolingCoilTwoSpeed_Test2)
{

    // tests secondary DX coil calculation of two speed DX cooling system
    int DXCoilNum;

    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoil(DXCoilNum).IsSecondaryDXCoilInZone = true;
    state->dataDXCoils->DXCoil(DXCoilNum).coilType = HVAC::CoilType::CoolingDXTwoSpeed;
    state->dataDXCoils->DXCoil(DXCoilNum).TotalCoolingEnergyRate = 5000.0;
    state->dataDXCoils->DXCoil(DXCoilNum).ElecCoolingPower = 500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatGainRate = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataDXCoils->DXCoil(DXCoilNum).SecZonePtr = 1;

    CalcSecondaryDXCoils(*state, DXCoilNum);
    EXPECT_DOUBLE_EQ(5500.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatGainRate);

    // cleanup
    state->dataDXCoils->DXCoil.deallocate();
}
TEST_F(EnergyPlusFixture, SecondaryDXCoolingCoilMultiSpeed_Test3)
{

    // tests secondary DX coil calculation of multi speed heat pump
    int DXCoilNum;

    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoil(DXCoilNum).IsSecondaryDXCoilInZone = true;
    state->dataDXCoils->DXCoil(DXCoilNum).coilType = HVAC::CoilType::CoolingDXMultiSpeed;
    state->dataDXCoils->DXCoil(DXCoilNum).TotalCoolingEnergyRate = 5000.0;
    state->dataDXCoils->DXCoil(DXCoilNum).ElecCoolingPower = 500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatGainRate = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataDXCoils->DXCoil(DXCoilNum).SecZonePtr = 1;

    CalcSecondaryDXCoils(*state, DXCoilNum);
    EXPECT_DOUBLE_EQ(5500.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatGainRate);

    // cleanup
    state->dataDXCoils->DXCoil.deallocate();
}
TEST_F(EnergyPlusFixture, SecondaryDXHeatingCoilSingleSpeed_Test4)
{
    // tests secondary DX coil calculation of single speed heat pump
    state->init_state(*state);
    int DXCoilNum;

    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoil(DXCoilNum).IsSecondaryDXCoilInZone = true;
    state->dataDXCoils->DXCoil(DXCoilNum).coilType = HVAC::CoilType::HeatingDXSingleSpeed;
    state->dataDXCoils->DXCoil(DXCoilNum).MinOATCompressor = -5.0;
    state->dataDXCoils->DXCoil(DXCoilNum).TotalHeatingEnergyRate = 5500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).ElecHeatingPower = 500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilTotalHeatRemovalRate = 0.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatRemovalRate = 0.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilLatentHeatRemovalRate = 0.0;

    state->dataDXCoils->DXCoil(DXCoilNum).SecZonePtr = 1;
    state->dataLoopNodes->Node.allocate(2);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 10.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.003;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilAirFlow = 1.0;
    state->dataDXCoils->DXCoil(DXCoilNum).CompressorPartLoadRatio = 1.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilRatedSHR = 1.0;

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataDXCoils->DXCoil(DXCoilNum).AirInNode = 2;
    state->dataLoopNodes->Node(state->dataDXCoils->DXCoil(DXCoilNum).AirInNode).Temp = 20.0;

    CalcSecondaryDXCoils(*state, DXCoilNum);
    EXPECT_DOUBLE_EQ(-5000.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilTotalHeatRemovalRate);
    EXPECT_DOUBLE_EQ(1.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSHR);

    //// set up arguments
    Real64 constexpr EvapAirMassFlow = 1.2;
    Real64 constexpr TotalHeatRemovalRate = 5500.0;
    Real64 constexpr PartLoadRatio = 1.0;
    Real64 constexpr SecCoilRatedSHR = 1.0;
    Real64 constexpr EvapInletDryBulb = 10.0;
    Real64 constexpr EvapInletHumRat = 0.003;
    Real64 constexpr EvapInletWetBulb = 4.5;
    Real64 constexpr EvapInletEnthalpy = 17607.0;
    Real64 constexpr CondInletDryBulb = 20.0;
    Real64 constexpr SecCoilFlowFraction = 1.0;
    int constexpr SecCoilSHRFT = 0;
    int constexpr SecCoilSHRFF = 0;

    // output variable
    Real64 SHRTest;

    // make the call
    SHRTest = CalcSecondaryDXCoilsSHR(*state,
                                      DXCoilNum,
                                      EvapAirMassFlow,
                                      TotalHeatRemovalRate,
                                      PartLoadRatio,
                                      SecCoilRatedSHR,
                                      EvapInletDryBulb,
                                      EvapInletHumRat,
                                      EvapInletWetBulb,
                                      EvapInletEnthalpy,
                                      CondInletDryBulb,
                                      SecCoilFlowFraction,
                                      SecCoilSHRFT,
                                      SecCoilSHRFF);

    EXPECT_DOUBLE_EQ(1.0, SHRTest);

    // cleanup
    state->dataDXCoils->DXCoil.deallocate();
    state->dataLoopNodes->Node.deallocate();
}

TEST_F(EnergyPlusFixture, SecondaryDXHeatingCoilMultiSpeed_Test5)
{
    // tests secondary DX coil calculation of multi speed heat pump
    state->init_state(*state);

    int DXCoilNum;

    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoil(DXCoilNum).NumOfSpeeds = 2;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilAirFlow.allocate(state->dataDXCoils->DXCoil(DXCoilNum).NumOfSpeeds);
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilRatedSHR.allocate(state->dataDXCoils->DXCoil(DXCoilNum).NumOfSpeeds);
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFT.allocate(state->dataDXCoils->DXCoil(DXCoilNum).NumOfSpeeds);
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFF.allocate(state->dataDXCoils->DXCoil(DXCoilNum).NumOfSpeeds);

    state->dataDXCoils->DXCoil(DXCoilNum).IsSecondaryDXCoilInZone = true;
    state->dataDXCoils->DXCoil(DXCoilNum).coilType = HVAC::CoilType::HeatingDXMultiSpeed;
    state->dataDXCoils->DXCoil(DXCoilNum).MinOATCompressor = -5.0;
    state->dataDXCoils->DXCoil(DXCoilNum).TotalHeatingEnergyRate = 5500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).ElecHeatingPower = 500.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilTotalHeatRemovalRate = 0.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSensibleHeatRemovalRate = 0.0;
    state->dataDXCoils->DXCoil(DXCoilNum).SecCoilLatentHeatRemovalRate = 0.0;

    state->dataDXCoils->DXCoil(DXCoilNum).SecZonePtr = 1;
    state->dataLoopNodes->Node.allocate(2);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 10.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.003;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilAirFlow(1) = 1.0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilAirFlow(2) = 1.0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFT(1) = 0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFF(1) = 0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFT(2) = 0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFF(2) = 0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilRatedSHR(1) = 1.0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilRatedSHR(2) = 1.0;

    state->dataDXCoils->DXCoil(DXCoilNum).MSSpeedRatio = 0;
    state->dataDXCoils->DXCoil(DXCoilNum).MSCycRatio = 1;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSpeedNumHS = 1;
    state->dataDXCoils->DXCoil(DXCoilNum).MSSpeedNumLS = 1;

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataDXCoils->DXCoil(DXCoilNum).AirInNode = 2;
    state->dataLoopNodes->Node(state->dataDXCoils->DXCoil(DXCoilNum).AirInNode).Temp = 20.0;

    CalcSecondaryDXCoils(*state, DXCoilNum);
    EXPECT_DOUBLE_EQ(-5000.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilTotalHeatRemovalRate);
    EXPECT_DOUBLE_EQ(1.0, state->dataDXCoils->DXCoil(DXCoilNum).SecCoilSHR);

    // cleanup
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilAirFlow.deallocate();
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilRatedSHR.deallocate();
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFT.deallocate();
    state->dataDXCoils->DXCoil(DXCoilNum).MSSecCoilSHRFF.deallocate();
    state->dataDXCoils->DXCoil.deallocate();
    state->dataLoopNodes->Node.deallocate();
}
