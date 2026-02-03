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

// EnergyPlus::HVACControllers Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataRootFinder.hh>
#include <EnergyPlus/RootFinder.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, RootFinder_CheckConvergence)
{
    DataRootFinder::RootFinderDataType rootFinderData;
    DataRootFinder::Slope slopeType = DataRootFinder::Slope::Decreasing;
    DataRootFinder::RootFinderMethod methodType = DataRootFinder::RootFinderMethod::Brent;
    Real64 relativeXTolerance = 0.0;
    Real64 absoluteXTolerance = 1.0e-6;
    Real64 absoluteYTolerance = 1.0e-5;

    RootFinder::SetupRootFinder(*state, rootFinderData, slopeType, methodType, relativeXTolerance, absoluteXTolerance, absoluteYTolerance);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);

    Real64 xMin = 0.0;
    Real64 xMax = 100.0;
    RootFinder::InitializeRootFinder(*state, rootFinderData, xMin, xMax);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);

    bool isDone = false;

    // Set min point
    Real64 xValue = xMin;
    Real64 yValue = 100.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // Set max point
    xValue = xMax;
    yValue = -100.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // Set 3rd point
    xValue = 20.0;
    yValue = -1.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // 4th point should converge
    xValue = rootFinderData.XCandidate;
    yValue = absoluteYTolerance / 2.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::OK);
    EXPECT_TRUE(isDone);
}

TEST_F(EnergyPlusFixture, RootFinder_CheckBracketRoundOff)
{
    DataRootFinder::RootFinderDataType rootFinderData;
    DataRootFinder::Slope slopeType = DataRootFinder::Slope::Decreasing;
    DataRootFinder::RootFinderMethod methodType = DataRootFinder::RootFinderMethod::Brent;
    Real64 relativeXTolerance = 0.0;
    Real64 absoluteXTolerance = 1.0e-6;
    Real64 absoluteYTolerance = 1.0e-5;

    RootFinder::SetupRootFinder(*state, rootFinderData, slopeType, methodType, relativeXTolerance, absoluteXTolerance, absoluteYTolerance);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);

    Real64 xMin = 0.0;
    Real64 xMax = 100.0;
    RootFinder::InitializeRootFinder(*state, rootFinderData, xMin, xMax);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);

    bool isDone = false;

    // Set min point
    Real64 xValue = xMin;
    Real64 yValue = 100.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // Set max point
    xValue = xMax;
    yValue = -100.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // Set 3rd point (this will be the upper point)
    xValue = 20.0;
    yValue = -10.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // Set 4th point slightly below 3rd point but with opposite sign for y (this will be the lower point)
    xValue = xValue - absoluteXTolerance / 3.0;
    yValue = 10.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::None);
    EXPECT_FALSE(isDone);

    // Set 5th point in between upper and lower, but non-convergent so it gets to the Increment check)
    xValue = rootFinderData.XCandidate;
    yValue = 5.0;
    RootFinder::IterateRootFinder(*state, rootFinderData, xValue, yValue, isDone);
    EXPECT_ENUM_EQ(rootFinderData.StatusFlag, DataRootFinder::RootFinderStatus::OKRoundOff);
    EXPECT_TRUE(isDone);
}
