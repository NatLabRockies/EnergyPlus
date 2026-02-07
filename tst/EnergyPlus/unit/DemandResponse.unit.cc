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

// EnergyPlus::DemandManager ventilation test

// Google test headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DemandManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/ScheduleManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::MixedAir;
using namespace EnergyPlus::DemandManager;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, DemandManagerGetInput)
{
    // Test input processing for DemandManager:Ventilation
    std::string const idf_objects = delimited_string({"DemandManager:Ventilation,",
                                                      " Ventilation Manager,",
                                                      " ,",
                                                      " FIXEDRATE,",
                                                      " 60,",
                                                      " 0.2,",
                                                      " ,", // N3 left blank because Numbers was only assigned up to 2
                                                      " ,", // N4 left blank because Numbers was only assigned up to 2
                                                      " ALL,",
                                                      " ,",
                                                      " OA CONTROLLER 1;"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataMixedAir->NumOAControllers = 1;
    state->dataMixedAir->OAController.allocate(state->dataMixedAir->NumOAControllers);
    state->dataMixedAir->OAController(1).Name = "OA CONTROLLER 1";

    GetDemandManagerInput(*state);
    auto &DemandMgr(state->dataDemandManager->DemandMgr);
    EXPECT_EQ(Sched::SchedNum_AlwaysOn, DemandMgr(1).availSched->Num);
    EXPECT_ENUM_EQ(ManagerLimit::Fixed, DemandMgr(1).LimitControl);
    EXPECT_DOUBLE_EQ(60.0, DemandMgr(1).LimitDuration);
    EXPECT_DOUBLE_EQ(0.2, DemandMgr(1).FixedRate);
    EXPECT_ENUM_EQ(ManagerSelection::All, DemandMgr(1).SelectionControl);
    EXPECT_EQ(1, DemandMgr(1).NumOfLoads);
}

TEST_F(EnergyPlusFixture, DemandManagerAssignmentListGetInputTest)
{
    std::string const idf_objects = delimited_string({
        "  DemandManagerAssignmentList,",
        "    Demand Manager,          !- Name",
        "    Electricity:Facility,    !- Meter Name",
        "    Limit Schedule,          !- Demand Limit Schedule Name",
        "    1.0,                     !- Demand Limit Safety Fraction",
        "    ,                        !- Billing Period Schedule Name",
        "    ,                        !- Peak Period Schedule Name",
        "    15,                      !- Demand Window Length {minutes}",
        "    SEQUENTIAL,              !- Demand Manager Priority",
        "    DemandManager:ExteriorLights,  !- DemandManager 1 Object Type",
        "    Ext Lights Manager 1;    !- DemandManager 1 Name",

        "  Schedule:Compact,",
        "    Limit Schedule,          !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    FOR: AllDays,            !- Field 2",
        "    Until: 8:00,9999999,     !- Field 3",
        "    Until: 20:00,10000,      !- Field 5",
        "    Until: 24:00,9999999;    !- Field 7",

        "  DemandManager:ExteriorLights,",
        "    Ext Lights Manager,      !- Name",
        "    ,                        !- Availability Schedule Name",
        "    FIXED,                   !- Limit Control",
        "    60,                      !- Minimum Limit Duration {minutes}",
        "    0.0,                     !- Maximum Limit Fraction",
        "    ,                        !- Limit Step Change",
        "    ALL,                     !- Selection Control",
        "    ,                        !- Rotation Duration {minutes}",
        "    Exterior Lights;         !- Exterior Lights 1 Name",

        "  Exterior:Lights,",
        "    Exterior Lights,         !- Name",
        "    ON,                      !- Schedule Name",
        "    1000,                    !- Design Level {W}",
        "    ScheduleNameOnly;        !- Control Option",

        "  Schedule:Constant,",
        "    ON,                      !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    1;                       !- TimeStep Value",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    ExteriorEnergyUse::GetExteriorEnergyUseInput(*state);
    GetDemandManagerInput(*state);

    int dMgrIndex = 0;
    auto &DemandMgr(state->dataDemandManager->DemandMgr);
    // check lights demand manager name
    dMgrIndex = Util::FindItemInList("EXT LIGHTS MANAGER", DemandMgr);
    auto &lightsDmndMgr = state->dataDemandManager->DemandMgr(dMgrIndex);
    EXPECT_EQ("EXT LIGHTS MANAGER", lightsDmndMgr.Name);
    // test expected fatal error due to wrong demand manager object name
    // object name in the list and in the object are different
    std::string expected_error = delimited_string({
        "   ** Severe  ** DemandManagerAssignmentList = \"DEMAND MANAGER\" invalid DemandManager Name = \"EXT LIGHTS MANAGER 1\" not found.",
        "   **  Fatal  ** Errors found in processing input for DemandManagerAssignmentList.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=DemandManagerAssignmentList = \"DEMAND MANAGER\" invalid DemandManager Name = \"EXT LIGHTS MANAGER 1\" not "
        "found.",
    });

    EXPECT_ANY_THROW(GetDemandManagerListInput(*state));
    EXPECT_TRUE(compare_err_stream(expected_error, true));
}

} // namespace EnergyPlus
