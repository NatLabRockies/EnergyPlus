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

// EnergyPlus::Construction Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/HVACCooledBeam.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/ScheduleManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::HVACCooledBeam;
using namespace EnergyPlus::DataPlant;

TEST_F(EnergyPlusFixture, HVACCooledBeam_reportTerminalUnit)
{
    using namespace EnergyPlus::OutputReportPredefined;
    auto &orp = *state->dataOutRptPredefined;

    SetPredefinedTables(*state);

    [[maybe_unused]] auto *schedA = Sched::AddScheduleConstant(*state, "schA");
    [[maybe_unused]] auto *schedB = Sched::AddScheduleConstant(*state, "schB");

    auto &adu = state->dataDefineEquipment->AirDistUnit;
    adu.allocate(2);
    adu(1).Name = "ADU a";
    adu(1).TermUnitSizingNum = 1;

    auto &siz = state->dataSize->TermUnitFinalZoneSizing;
    siz.allocate(2);
    siz(1).DesCoolVolFlowMin = 0.15;
    siz(1).MinOA = 0.05;
    siz(1).CoolDesTemp = 12.5;
    siz(1).HeatDesTemp = 40.0;
    siz(1).DesHeatLoad = 2000.0;
    siz(1).DesCoolLoad = 3000.0;

    auto &cb = state->dataHVACCooledBeam->CoolBeam;
    cb.allocate(2);
    cb(1).ADUNum = 1;
    cb(1).UnitType = "AirTerminal:SingleDuct:ConstantVolume:CooledBeam";
    cb(1).MaxAirVolFlow = 0.30;
    cb(1).CBTypeString = "active";

    cb(1).reportTerminalUnit(*state);

    EXPECT_EQ("0.15", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlow, "ADU a"));
    EXPECT_EQ("0.05", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOutdoorFlow, "ADU a"));
    EXPECT_EQ("12.50", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupCoolingSP, "ADU a"));
    EXPECT_EQ("40.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermSupHeatingSP, "ADU a"));
    EXPECT_EQ("2000.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatingCap, "ADU a"));
    EXPECT_EQ("3000.00", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolingCap, "ADU a"));
    EXPECT_EQ("AirTerminal:SingleDuct:ConstantVolume:CooledBeam", RetrievePreDefTableEntry(*state, orp.pdchAirTermTypeInp, "ADU a"));
    EXPECT_EQ("0.30", RetrievePreDefTableEntry(*state, orp.pdchAirTermPrimFlow, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermSecdFlow, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinFlowSch, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMaxFlowReh, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermMinOAflowSch, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermHeatCoilType, "ADU a"));
    EXPECT_EQ("active", RetrievePreDefTableEntry(*state, orp.pdchAirTermCoolCoilType, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanType, "ADU a"));
    EXPECT_EQ("n/a", RetrievePreDefTableEntry(*state, orp.pdchAirTermFanName, "ADU a"));
}
