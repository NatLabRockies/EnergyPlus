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

// EnergyPlus::Pumps Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UFADManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, UFADManagerTest_sumUFADConvGainPerPlume)
{
    auto &dataHB = state->dataHeatBal;
    dataHB->ZoneElectric.allocate(5);
    dataHB->ZoneGas.allocate(5);
    dataHB->ZoneOtherEq.allocate(5);
    dataHB->ZoneHWEq.allocate(5);
    dataHB->ZoneSteamEq.allocate(5);

    // Set-up data for various tests
    dataHB->ZoneElectric(1).DesignLevel = 11.0;
    dataHB->ZoneElectric(2).DesignLevel = 12.0;
    dataHB->ZoneElectric(3).DesignLevel = 13.0;
    dataHB->ZoneElectric(4).DesignLevel = 14.0;
    dataHB->ZoneElectric(5).DesignLevel = 15.0;
    dataHB->ZoneElectric(1).FractionConvected = 0.11;
    dataHB->ZoneElectric(2).FractionConvected = 0.21;
    dataHB->ZoneElectric(3).FractionConvected = 0.31;
    dataHB->ZoneElectric(4).FractionConvected = 0.41;
    dataHB->ZoneElectric(5).FractionConvected = 0.51;
    dataHB->ZoneElectric(1).ZonePtr = 1;
    dataHB->ZoneElectric(2).ZonePtr = 1;
    dataHB->ZoneElectric(3).ZonePtr = 2;
    dataHB->ZoneElectric(4).ZonePtr = 2;
    dataHB->ZoneElectric(5).ZonePtr = 3;
    dataHB->ZoneGas(1).DesignLevel = 21.0;
    dataHB->ZoneGas(2).DesignLevel = 22.0;
    dataHB->ZoneGas(3).DesignLevel = 23.0;
    dataHB->ZoneGas(4).DesignLevel = 24.0;
    dataHB->ZoneGas(5).DesignLevel = 25.0;
    dataHB->ZoneGas(1).FractionConvected = 0.12;
    dataHB->ZoneGas(2).FractionConvected = 0.22;
    dataHB->ZoneGas(3).FractionConvected = 0.32;
    dataHB->ZoneGas(4).FractionConvected = 0.42;
    dataHB->ZoneGas(5).FractionConvected = 0.52;
    dataHB->ZoneGas(1).ZonePtr = 1;
    dataHB->ZoneGas(2).ZonePtr = 2;
    dataHB->ZoneGas(3).ZonePtr = 3;
    dataHB->ZoneGas(4).ZonePtr = 1;
    dataHB->ZoneGas(5).ZonePtr = 2;
    dataHB->ZoneOtherEq(1).DesignLevel = 31.0;
    dataHB->ZoneOtherEq(2).DesignLevel = 32.0;
    dataHB->ZoneOtherEq(3).DesignLevel = 33.0;
    dataHB->ZoneOtherEq(4).DesignLevel = 34.0;
    dataHB->ZoneOtherEq(5).DesignLevel = 35.0;
    dataHB->ZoneOtherEq(1).FractionConvected = 0.13;
    dataHB->ZoneOtherEq(2).FractionConvected = 0.23;
    dataHB->ZoneOtherEq(3).FractionConvected = 0.33;
    dataHB->ZoneOtherEq(4).FractionConvected = 0.43;
    dataHB->ZoneOtherEq(5).FractionConvected = 0.53;
    dataHB->ZoneOtherEq(1).ZonePtr = 3;
    dataHB->ZoneOtherEq(2).ZonePtr = 3;
    dataHB->ZoneOtherEq(3).ZonePtr = 4;
    dataHB->ZoneOtherEq(4).ZonePtr = 4;
    dataHB->ZoneOtherEq(5).ZonePtr = 5;
    dataHB->ZoneHWEq(1).DesignLevel = 41.0;
    dataHB->ZoneHWEq(2).DesignLevel = 42.0;
    dataHB->ZoneHWEq(3).DesignLevel = 43.0;
    dataHB->ZoneHWEq(4).DesignLevel = 44.0;
    dataHB->ZoneHWEq(5).DesignLevel = 45.0;
    dataHB->ZoneHWEq(1).FractionConvected = 0.14;
    dataHB->ZoneHWEq(2).FractionConvected = 0.24;
    dataHB->ZoneHWEq(3).FractionConvected = 0.34;
    dataHB->ZoneHWEq(4).FractionConvected = 0.44;
    dataHB->ZoneHWEq(5).FractionConvected = 0.54;
    dataHB->ZoneHWEq(1).ZonePtr = 5;
    dataHB->ZoneHWEq(2).ZonePtr = 5;
    dataHB->ZoneHWEq(3).ZonePtr = 6;
    dataHB->ZoneHWEq(4).ZonePtr = 6;
    dataHB->ZoneHWEq(5).ZonePtr = 7;
    dataHB->ZoneSteamEq(1).DesignLevel = 51.0;
    dataHB->ZoneSteamEq(2).DesignLevel = 52.0;
    dataHB->ZoneSteamEq(3).DesignLevel = 53.0;
    dataHB->ZoneSteamEq(4).DesignLevel = 54.0;
    dataHB->ZoneSteamEq(5).DesignLevel = 55.0;
    dataHB->ZoneSteamEq(1).FractionConvected = 0.15;
    dataHB->ZoneSteamEq(2).FractionConvected = 0.25;
    dataHB->ZoneSteamEq(3).FractionConvected = 0.35;
    dataHB->ZoneSteamEq(4).FractionConvected = 0.45;
    dataHB->ZoneSteamEq(5).FractionConvected = 0.55;
    dataHB->ZoneSteamEq(1).ZonePtr = 7;
    dataHB->ZoneSteamEq(2).ZonePtr = 7;
    dataHB->ZoneSteamEq(3).ZonePtr = 8;
    dataHB->ZoneSteamEq(4).ZonePtr = 8;
    dataHB->ZoneSteamEq(5).ZonePtr = 8;
    Real64 constexpr numOccupants = 10.0;
    std::vector<Real64> expectedAnswer = {74.633, 75.761, 75.64, 75.551, 76.437, 76.398, 77.495, 80.31};
    Real64 constexpr allowedTolerance = 0.00001;

    // Tests 1 - 8: testNum is used for the zone number which grabs different data and each has a different answer
    for (int testNum = 1; testNum <= 8; ++testNum) {
        Real64 actualAnswer = RoomAir::sumUFADConvGainPerPlume(*state, testNum, numOccupants);
        EXPECT_NEAR(actualAnswer, expectedAnswer[testNum - 1], allowedTolerance);
    }
}

} // namespace EnergyPlus
