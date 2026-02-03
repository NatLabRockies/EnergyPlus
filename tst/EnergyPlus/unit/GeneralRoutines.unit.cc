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
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/Psychrometrics.hh>

#include "Fixtures/EnergyPlusFixture.hh"

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, BBConvergeCheckTest)
{

    int SimCompNum;
    Real64 MaxFlow;
    Real64 MinFlow;
    bool FunctionResult;

    // Test 1: Not a radiant/convective baseboard unit
    SimCompNum = 1;
    MaxFlow = 1.0;
    MinFlow = 0.99999999;
    FunctionResult = BBConvergeCheck(SimCompNum, MaxFlow, MinFlow);
    EXPECT_FALSE(FunctionResult);

    // Test 2: Also not a radiant/convective baseboard unit
    SimCompNum = 10;
    MaxFlow = 1.0;
    MinFlow = 0.99999999;
    FunctionResult = BBConvergeCheck(SimCompNum, MaxFlow, MinFlow);
    EXPECT_FALSE(FunctionResult);

    // Test 3: One of the radiant/convective baseboard units but Max and Min too different
    SimCompNum = 5;
    MaxFlow = 1.0;
    MinFlow = 0.9;
    FunctionResult = BBConvergeCheck(SimCompNum, MaxFlow, MinFlow);
    EXPECT_FALSE(FunctionResult);

    // Test 4: One of the radiant/convective baseboard units and Max and Min almost the same
    SimCompNum = 5;
    MaxFlow = 1.0;
    MinFlow = 0.9999999;
    FunctionResult = BBConvergeCheck(SimCompNum, MaxFlow, MinFlow);
    EXPECT_TRUE(FunctionResult);

    // Test 5: The other radiant/convective baseboard units but Max and Min too different
    SimCompNum = 6;
    MaxFlow = 1.0;
    MinFlow = 0.9;
    FunctionResult = BBConvergeCheck(SimCompNum, MaxFlow, MinFlow);
    EXPECT_FALSE(FunctionResult);

    // Test 6: The other radiant/convective baseboard units and Max and Min almost the same
    SimCompNum = 6;
    MaxFlow = 1.0;
    MinFlow = 0.9999999;
    FunctionResult = BBConvergeCheck(SimCompNum, MaxFlow, MinFlow);
    EXPECT_TRUE(FunctionResult);
}

TEST_F(EnergyPlusFixture, CalcComponentSensibleLatentOutputTest)
{

    Real64 MassFlowRate(0.0);
    Real64 CoilInletTemp(0.0);
    Real64 CoilOutletTemp(0.0);
    Real64 CoilInletHumRat(0.0);
    Real64 CoilOutletHumRat(0.0);
    Real64 totaloutput(0.0);
    Real64 sensibleoutput(0.0);
    Real64 latentoutput(0.0);
    Real64 results_totaloutput(0.0);
    Real64 results_sensibleoutput(0.0);
    Real64 results_latentoutput(0.0);

    // test 1: zero flow
    MassFlowRate = 0.0;
    CalcComponentSensibleLatentOutput(
        MassFlowRate, CoilOutletTemp, CoilOutletHumRat, CoilInletTemp, CoilInletHumRat, sensibleoutput, latentoutput, totaloutput);
    EXPECT_DOUBLE_EQ(results_totaloutput, totaloutput);
    EXPECT_DOUBLE_EQ(results_sensibleoutput, sensibleoutput);
    EXPECT_DOUBLE_EQ(results_latentoutput, latentoutput);
    // test 2: cooling
    MassFlowRate = 1.0;
    CoilInletTemp = 24.0;
    CoilOutletTemp = 12.0;
    CoilInletHumRat = 0.00850;
    CoilOutletHumRat = 0.00750;
    results_totaloutput =
        MassFlowRate * (Psychrometrics::PsyHFnTdbW(CoilOutletTemp, CoilOutletHumRat) - Psychrometrics::PsyHFnTdbW(CoilInletTemp, CoilInletHumRat));
    results_sensibleoutput = MassFlowRate * (1.00484e3 + min(CoilInletHumRat, CoilOutletHumRat) * 1.85895e3) * (CoilOutletTemp - CoilInletTemp);
    results_latentoutput = results_totaloutput - results_sensibleoutput;
    CalcComponentSensibleLatentOutput(
        MassFlowRate, CoilOutletTemp, CoilOutletHumRat, CoilInletTemp, CoilInletHumRat, sensibleoutput, latentoutput, totaloutput);
    EXPECT_DOUBLE_EQ(results_totaloutput, totaloutput);
    EXPECT_DOUBLE_EQ(results_sensibleoutput, sensibleoutput);
    EXPECT_DOUBLE_EQ(results_latentoutput, latentoutput);
    // test 3: heating
    MassFlowRate = 1.0;
    CoilInletTemp = 20.0;
    CoilOutletTemp = 32.0;
    CoilInletHumRat = 0.00750;
    CoilOutletHumRat = 0.00750;
    results_totaloutput =
        MassFlowRate * (Psychrometrics::PsyHFnTdbW(CoilOutletTemp, CoilOutletHumRat) - Psychrometrics::PsyHFnTdbW(CoilInletTemp, CoilInletHumRat));
    results_sensibleoutput = MassFlowRate * (1.00484e3 + min(CoilInletHumRat, CoilOutletHumRat) * 1.85895e3) * (CoilOutletTemp - CoilInletTemp);
    results_latentoutput = results_totaloutput - results_sensibleoutput;
    CalcComponentSensibleLatentOutput(
        MassFlowRate, CoilOutletTemp, CoilOutletHumRat, CoilInletTemp, CoilInletHumRat, sensibleoutput, latentoutput, totaloutput);
    EXPECT_DOUBLE_EQ(results_totaloutput, totaloutput);
    EXPECT_DOUBLE_EQ(results_sensibleoutput, sensibleoutput);
    EXPECT_NEAR(results_latentoutput, latentoutput, 1.0E-10);
}

} // namespace EnergyPlus
