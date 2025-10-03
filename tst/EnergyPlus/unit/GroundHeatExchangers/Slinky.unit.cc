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
#include <gtest/gtest.h>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Plant/DataPlant.hh>

#include <EnergyPlus/GroundHeatExchangers/ResponseFactors.hh>
#include <EnergyPlus/GroundHeatExchangers/Slinky.hh>

#include "../Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::GroundHeatExchangers;

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_Slinky_GetGFunc)
{

    // Initialization
    GLHESlinky thisGLHE;
    Real64 thisGFunc;
    Real64 time;

    int NPairs = 2;

    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisGLHE.myRespFactors = thisRF;

    thisGLHE.myRespFactors->GFNC = std::vector<Real64>(NPairs);

    thisGLHE.myRespFactors->LNTTS.push_back(0.0);
    thisGLHE.myRespFactors->LNTTS.push_back(5.0);
    thisGLHE.myRespFactors->GFNC[0] = 0.0;
    thisGLHE.myRespFactors->GFNC[1] = 5.0;

    time = std::pow(10.0, 2.5);

    thisGFunc = thisGLHE.getGFunc(time);

    EXPECT_EQ(2.5, thisGFunc);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_Interpolate)
{
    // Initialization
    GLHESlinky thisGLHE;
    Real64 thisLNTTS;
    Real64 thisGFunc;

    int NPairs = 2;

    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisGLHE.myRespFactors = thisRF;

    thisGLHE.myRespFactors->GFNC = std::vector<Real64>(NPairs);

    thisGLHE.myRespFactors->LNTTS.push_back(0.0);
    thisGLHE.myRespFactors->LNTTS.push_back(5.0);
    thisGLHE.myRespFactors->GFNC[0] = 0.0;
    thisGLHE.myRespFactors->GFNC[1] = 5.0;

    // Case when extrapolating beyond lower bound
    thisLNTTS = -1.0;
    thisGFunc = thisGLHE.interpGFunc(thisLNTTS);
    EXPECT_DOUBLE_EQ(-1.0, thisGFunc);

    // Case when extrapolating beyond upper bound
    thisLNTTS = 6.0;
    thisGFunc = thisGLHE.interpGFunc(thisLNTTS);
    EXPECT_DOUBLE_EQ(6.0, thisGFunc);

    // Case when we're actually interpolating
    thisLNTTS = 2.5;
    thisGFunc = thisGLHE.interpGFunc(thisLNTTS);
    EXPECT_DOUBLE_EQ(2.5, thisGFunc);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_Slinky_CalcHXResistance)
{
    state->init_state(*state);
    // Initializations
    GLHESlinky thisGLHE;

    state->dataPlnt->PlantLoop.allocate(1);
    thisGLHE.plantLoc.loopNum = 1;

    state->dataPlnt->PlantLoop(thisGLHE.plantLoc.loopNum).FluidName = "WATER";
    state->dataPlnt->PlantLoop(thisGLHE.plantLoc.loopNum).glycol = Fluid::GetWater(*state);

    thisGLHE.inletTemp = 5.0;
    thisGLHE.massFlowRate = 0.01;
    thisGLHE.numTrenches = 1;
    thisGLHE.pipe.outDia = 0.02667;
    thisGLHE.pipe.outRadius = thisGLHE.pipe.outDia / 2.0;
    thisGLHE.pipe.thickness = 0.004;
    thisGLHE.pipe.k = 0.4;

    // Re < 2300 mass flow rate
    EXPECT_NEAR(0.13487, thisGLHE.calcHXResistance(*state), 0.0001);

    // 4000 > Re > 2300 mass flow rate
    thisGLHE.massFlowRate = 0.07;
    EXPECT_NEAR(0.08582, thisGLHE.calcHXResistance(*state), 0.0001);

    // Re > 4000 mass flow rate
    thisGLHE.massFlowRate = 0.1;
    EXPECT_NEAR(0.077185, thisGLHE.calcHXResistance(*state), 0.0001);

    // Zero mass flow rate
    thisGLHE.massFlowRate = 0.0;
    EXPECT_NEAR(0.07094, thisGLHE.calcHXResistance(*state), 0.0001);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_Slinky_CalcGroundHeatExchanger)
{

    // Initializations
    GLHESlinky thisGLHE;

    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisGLHE.myRespFactors = thisRF;

    thisGLHE.numCoils = 100;
    thisGLHE.numTrenches = 2;
    thisGLHE.maxSimYears = 10;
    thisGLHE.coilPitch = 0.4;
    thisGLHE.coilDepth = 1.5;
    thisGLHE.coilDiameter = 0.8;
    thisGLHE.pipe.outDia = 0.034;
    thisGLHE.pipe.outRadius = thisGLHE.pipe.outDia / 2.0;
    thisGLHE.trenchSpacing = 3.0;
    thisGLHE.soil.diffusivity = 3.0e-007;
    thisGLHE.AGG = 192;
    thisGLHE.SubAGG = 15;

    // Horizontal G-Functions
    thisGLHE.calcGFunctions(*state);
    EXPECT_NEAR(19.08237, thisGLHE.myRespFactors->GFNC[27], 0.0001);

    // Vertical G-Functions
    thisGLHE.verticalConfig = true;
    thisGLHE.calcGFunctions(*state);
    EXPECT_NEAR(18.91819, thisGLHE.myRespFactors->GFNC[27], 0.0001);
}