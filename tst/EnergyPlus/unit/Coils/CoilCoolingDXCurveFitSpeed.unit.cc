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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitOperatingMode.hh>
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitSpeed.hh>
#include <EnergyPlus/DataLoopNode.hh>

#include "../Coils/CoilCoolingDXFixture.hh"

using namespace EnergyPlus;

TEST_F(CoilCoolingDXTest, CoilCoolingDXCurveFitSpeedInput)
{
    std::string idf_objects = this->getSpeedObjectString("speed1");
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    CoilCoolingDXCurveFitSpeed thisSpeed(*state, "speed1");
    EXPECT_EQ("SPEED1", thisSpeed.name);
}

TEST_F(CoilCoolingDXTest, CoilCoolingDXCurveFitSpeedTest)
{
    std::string idf_objects = this->getSpeedObjectString("speed1");
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    CoilCoolingDXCurveFitSpeed thisSpeed(*state, "speed1");
    EXPECT_EQ("SPEED1", thisSpeed.name);

    CoilCoolingDXCurveFitOperatingMode thisMode;
    thisMode.ratedGrossTotalCap = 12000;
    thisMode.ratedEvapAirFlowRate = 100;
    thisMode.ratedCondAirFlowRate = 200;

    DataLoopNode::NodeData inletNode;
    inletNode.Temp = 20.0;
    inletNode.HumRat = 0.008;
    inletNode.Enthalpy = 40000.0;
    DataLoopNode::NodeData outletNode;

    thisSpeed.PLR = 1.0;
    thisSpeed.ambPressure = 101325.0;
    inletNode.Press = thisSpeed.ambPressure;
    thisSpeed.AirFF = 1.0;
    thisSpeed.rated_total_capacity = 3000.0;
    thisSpeed.RatedAirMassFlowRate = 1.0;
    thisSpeed.grossRatedSHR = 0.75;
    thisSpeed.RatedCBF = 0.09;
    thisSpeed.RatedEIR = 0.30;
    thisSpeed.AirMassFlow = 1.0;
    HVAC::FanOp fanOp = HVAC::FanOp::Invalid;
    Real64 condInletTemp = 24;
    thisSpeed.CalcSpeedOutput(*state, inletNode, outletNode, thisSpeed.PLR, fanOp, condInletTemp);

    EXPECT_NEAR(outletNode.Temp, 17.791, 0.001);
    EXPECT_NEAR(outletNode.HumRat, 0.00754, 0.0001);
    EXPECT_NEAR(outletNode.Enthalpy, 37000.0, 0.1);
    EXPECT_NEAR(thisSpeed.fullLoadPower, 900.0, 0.1);
    EXPECT_NEAR(thisSpeed.RTF, 1.0, 0.01);
}
