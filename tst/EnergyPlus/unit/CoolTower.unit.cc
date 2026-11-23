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

// EnergyPlus::BoilerSteam Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CoolTower.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, ExerciseCoolTower)
{
    std::string const idf_objects =
        delimited_string({"ScheduleTypeLimits, Any Number;",
                          "Schedule:Compact, Cooltower Operation, Any Number, Through: 12/31, For: AllDays, Until: 24:00, 1.0;",
                          "ZoneCoolTower:Shower,",
                          "    CoolTower 1,             !- Name",
                          "    ,     !- Availability Schedule Name",
                          "    Zone 1,                  !- Zone Name",
                          "    ,                        !- Water Supply Storage Tank Name",
                          "    WindDrivenFlow,          !- Flow Control Type",
                          "    Cooltower Operation,     !- Pump Flow Rate Schedule Name",
                          "    0.0005,                  !- Maximum Water Flow Rate {m3/s}",
                          "    5.0,                     !- Effective Tower Height {m}",
                          "    0.5,                     !- Airflow Outlet Area {m2}",
                          "    10.0,                    !- Maximum Air Flow Rate {m3/s}",
                          "    18.0,                    !- Minimum Indoor Temperature {C}",
                          "    0.05,                    !- Fraction of Water Loss",
                          "    0.05,                    !- Fraction of Flow Schedule",
                          "    200.0;                   !- Rated Power Consumption {W}"});

    ASSERT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 20.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZT = 1.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPC = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MCPTC = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).CTMFL = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 1;

    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutWetBulbTemp = 26.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutHumRat =
        Psychrometrics::PsyWFnTdbTwbPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutWetBulbTemp, state->dataEnvrn->OutBaroPress);
    state->dataEnvrn->StdRhoAir =
        Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->WindSpeed = 20.0;
    CoolTower::ManageCoolTower(*state);
    // auto &thisTower = state->dataCoolTower->CoolTowerSys(1);
}
