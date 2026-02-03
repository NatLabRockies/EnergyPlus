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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace Curve;

TEST_F(EnergyPlusFixture, AirflowNetwork_AdvancedTest_Test1)
{

    int AirflowNetworkNumOfOccuVentCtrls;
    Real64 TimeOpenElapsed;
    Real64 TimeCloseElapsed;
    int OpenStatus;
    int OpenProbStatus;
    int CloseProbStatus;

    AirflowNetworkNumOfOccuVentCtrls = 1;
    state->afn->OccupantVentilationControl.allocate(AirflowNetworkNumOfOccuVentCtrls);
    state->afn->OccupantVentilationControl(1).MinOpeningTime = 4;
    state->afn->OccupantVentilationControl(1).MinClosingTime = 4;
    state->afn->OccupantVentilationControl(1).MinTimeControlOnly = true;

    TimeOpenElapsed = 3.0;
    TimeCloseElapsed = 0.0;
    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(1, OpenStatus);

    TimeOpenElapsed = 5.0;
    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(0, OpenStatus);

    TimeOpenElapsed = 0.0;
    TimeCloseElapsed = 3.0;
    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(2, OpenStatus);

    TimeOpenElapsed = 0.0;
    TimeCloseElapsed = 5.0;
    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(0, OpenStatus);

    state->dataEnvrn->OutDryBulbTemp = 15.0;
    state->dataHeatBal->Zone.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 22.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MRT = 22.0;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::Uncontrolled;
    state->dataHeatBalFanSys->zoneTstatSetpts.allocate(1);

    TimeOpenElapsed = 5.0;
    TimeCloseElapsed = 0.0;
    state->afn->OccupantVentilationControl(1).MinTimeControlOnly = false;
    state->afn->OccupantVentilationControl(1).ComfortBouPoint = 10.0;
    state->afn->OccupantVentilationControl(1).ComfortLowTempCurveNum = 1;
    state->afn->OccupantVentilationControl(1).ComfortHighTempCurveNum = 2;

    auto *curve1 = AddCurve(*state, "Curve1");
    curve1->curveType = CurveType::Quadratic;
    curve1->coeff[0] = 21.2;
    curve1->coeff[1] = 0.09;
    curve1->coeff[2] = 0.0;
    curve1->coeff[3] = 0.0;
    curve1->coeff[4] = 0.0;
    curve1->coeff[5] = 0.0;
    curve1->inputLimits[0].min = -50.0;
    curve1->inputLimits[0].max = 10.0;
    curve1->inputLimits[1].min = 0.0;
    curve1->inputLimits[1].max = 2.0;

    auto *curve2 = AddCurve(*state, "Curve2");
    curve2->curveType = CurveType::Quadratic;
    curve2->coeff[0] = 18.8;
    curve2->coeff[1] = 0.33;
    curve2->coeff[2] = 0.0;
    curve2->coeff[3] = 0.0;
    curve2->coeff[4] = 0.0;
    curve2->coeff[5] = 0.0;
    curve2->inputLimits[0].min = 10.0;
    curve2->inputLimits[0].max = 50.0;
    curve2->inputLimits[1].min = 0.0;
    curve2->inputLimits[1].max = 2.0;

    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(0, OpenProbStatus);
    EXPECT_EQ(1, CloseProbStatus);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 26.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MRT = 26.0;
    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(2, OpenProbStatus);
    EXPECT_EQ(0, CloseProbStatus);

    TimeOpenElapsed = 0.0;
    TimeCloseElapsed = 5.0;
    state->dataHeatBal->ZoneIntGain.allocate(1);
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 0.5;

    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(1, OpenProbStatus);
    EXPECT_EQ(0, CloseProbStatus);

    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptLo = 22.0;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 28.0;
    state->afn->OccupantVentilationControl(1).calc(*state, 1, TimeOpenElapsed, TimeCloseElapsed, OpenStatus, OpenProbStatus, CloseProbStatus);
    EXPECT_EQ(1, OpenProbStatus);
    EXPECT_EQ(0, CloseProbStatus);
}
