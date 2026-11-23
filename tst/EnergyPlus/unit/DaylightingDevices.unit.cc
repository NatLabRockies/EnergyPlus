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

// Google Test Headers
#include <gtest/gtest.h>

// C++ Headers
#include <array>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/General.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::Dayltg;
using namespace EnergyPlus::DataSurfaces;

TEST_F(EnergyPlusFixture, DaylightingDevices_adjustViewFactorsWithShelfTest)
{
    // Defect #8154: Light shelf on a window that uses autosize for view factor to ground results in
    // the sum of all view factors being greater than 1.
    Real64 vfShelfSet;
    Real64 vfSkySet;
    Real64 vfGroundSet;
    Real64 vfShelfResult;
    Real64 vfSkyResult;
    Real64 vfGroundResult;
    Real64 acceptableTolerance = 0.00001;
    int WinSurf = 1;
    int ShelfNum = 1;

    // Allocate and set up base data for surfaces and shelf
    state->dataSurface->Surface.allocate(2);
    state->dataDaylightingDevicesData->Shelf.allocate(1);
    state->dataSurface->Surface(1).Vertex.allocate(4);
    state->dataSurface->Surface(2).Vertex.allocate(4);
    state->dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf = 2;
    state->dataDaylightingDevicesData->Shelf(1).Name = "Skywalker";
    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(2).Sides = 4;

    // Test 1A: Sky and Ground are both negative (shouldn't happen but gotta test it), Shelf <= 1.0
    vfShelfSet = 0.67;
    vfSkySet = -0.1;
    vfGroundSet = -0.1;
    vfShelfResult = 0.67;
    vfSkyResult = 0.0;
    vfGroundResult = 0.0;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 1B: Sky and Ground are both negative (shouldn't happen but gotta test it), Shelf > 1.0
    vfShelfSet = 1.1;
    vfSkySet = -0.1;
    vfGroundSet = -0.1;
    vfShelfResult = 1.0;
    vfSkyResult = 0.0;
    vfGroundResult = 0.0;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 2A: Sky and Ground are positive (less than 1 combined), Shelf < 0.0
    vfShelfSet = -0.1;
    vfSkySet = 0.4;
    vfGroundSet = 0.4;
    vfShelfResult = 0.0;
    vfSkyResult = 0.4;
    vfGroundResult = 0.4;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 2B: Sky and Ground are positive (greater than 1 combined), Shelf < 0.0
    vfShelfSet = -0.1;
    vfSkySet = 0.8;
    vfGroundSet = 0.4;
    vfShelfResult = 0.0;
    vfSkyResult = 2.0 / 3.0;
    vfGroundResult = 1.0 / 3.0;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 3: Sky, Ground, and Shelf are all positive (less than 1 combined)
    vfShelfSet = 0.2;
    vfSkySet = 0.3;
    vfGroundSet = 0.4;
    vfShelfResult = 0.2;
    vfSkyResult = 0.3;
    vfGroundResult = 0.4;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 4A: Sky, Ground, and Shelf are all positive (greater than 1 combined), shelf is below window
    vfShelfSet = 0.4;
    vfSkySet = 0.5;
    vfGroundSet = 0.5;
    vfShelfResult = 0.4;
    vfSkyResult = 0.5;
    vfGroundResult = 0.1;
    state->dataSurface->Surface(1).Vertex(1).z = 1.0;
    state->dataSurface->Surface(1).Vertex(2).z = 1.0;
    state->dataSurface->Surface(1).Vertex(3).z = 2.5;
    state->dataSurface->Surface(1).Vertex(4).z = 2.5;
    state->dataSurface->Surface(2).Vertex(1).z = 0.5;
    state->dataSurface->Surface(2).Vertex(2).z = 0.5;
    state->dataSurface->Surface(2).Vertex(3).z = 0.5;
    state->dataSurface->Surface(2).Vertex(4).z = 0.5;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 4B: Sky, Ground, and Shelf are all positive (greater than 1 combined), shelf is above window
    vfShelfSet = 0.4;
    vfSkySet = 0.5;
    vfGroundSet = 0.5;
    vfShelfResult = 0.4;
    vfSkyResult = 0.1;
    vfGroundResult = 0.5;
    state->dataSurface->Surface(1).Vertex(1).z = 1.0;
    state->dataSurface->Surface(1).Vertex(2).z = 1.0;
    state->dataSurface->Surface(1).Vertex(3).z = 2.5;
    state->dataSurface->Surface(1).Vertex(4).z = 2.5;
    state->dataSurface->Surface(2).Vertex(1).z = 3.0;
    state->dataSurface->Surface(2).Vertex(2).z = 3.0;
    state->dataSurface->Surface(2).Vertex(3).z = 3.0;
    state->dataSurface->Surface(2).Vertex(4).z = 3.0;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 4C: Sky, Ground, and Shelf are all positive (greater than 1 combined), shelf is in the middle of the window somewhere
    vfShelfSet = 0.4;
    vfSkySet = 0.25;
    vfGroundSet = 0.5;
    vfShelfResult = 0.4;
    vfSkyResult = 0.18;
    vfGroundResult = 0.42;
    state->dataSurface->Surface(1).Vertex(1).z = 1.0;
    state->dataSurface->Surface(1).Vertex(2).z = 1.0;
    state->dataSurface->Surface(1).Vertex(3).z = 2.5;
    state->dataSurface->Surface(1).Vertex(4).z = 2.5;
    state->dataSurface->Surface(2).Vertex(1).z = 2.2;
    state->dataSurface->Surface(2).Vertex(2).z = 2.2;
    state->dataSurface->Surface(2).Vertex(3).z = 2.2;
    state->dataSurface->Surface(2).Vertex(4).z = 2.2;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);

    // Test 4D: Sky, Ground, and Shelf are all positive (greater than 1 combined), shelf is in the middle of the window somewhere
    vfShelfSet = 0.4;
    vfSkySet = 0.5;
    vfGroundSet = 0.25;
    vfShelfResult = 0.4;
    vfSkyResult = 0.40;
    vfGroundResult = 0.20;
    state->dataSurface->Surface(1).Vertex(1).z = 1.0;
    state->dataSurface->Surface(1).Vertex(2).z = 1.0;
    state->dataSurface->Surface(1).Vertex(3).z = 2.5;
    state->dataSurface->Surface(1).Vertex(4).z = 2.5;
    state->dataSurface->Surface(2).Vertex(1).z = 2.2;
    state->dataSurface->Surface(2).Vertex(2).z = 2.2;
    state->dataSurface->Surface(2).Vertex(3).z = 2.2;
    state->dataSurface->Surface(2).Vertex(4).z = 2.2;
    adjustViewFactorsWithShelf(*state, vfShelfSet, vfSkySet, vfGroundSet, WinSurf, ShelfNum);
    EXPECT_NEAR(vfShelfSet, vfShelfResult, acceptableTolerance);
    EXPECT_NEAR(vfSkySet, vfSkyResult, acceptableTolerance);
    EXPECT_NEAR(vfGroundSet, vfGroundResult, acceptableTolerance);
}
