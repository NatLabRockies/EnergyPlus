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

// EnergyPlus::AirflowNetworkSolver unit tests

// Google test headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/HeatBalanceManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace DataHeatBalance;
using namespace HeatBalanceManager;

TEST_F(EnergyPlusFixture, ConstructionInternalSource)
{

    std::string const idf_objects = delimited_string({
        "	ConstructionProperty:InternalHeatSource,	",
        "	Radiant Source,          !- Name",
        "	Slab Floor with Radiant, !- Construction Name",
        "	4,                       !- Source Present After Layer Number",
        "	4,                       !- Temperature Calculation Requested After Layer Number",
        "	2,                       !- Dimensions for the CTF Calculation",
        "	0.3048,                  !- Tube Spacing {m}",
        "	0.0;                     !- Two-Dimensional Position of Interior Temperature Calculation Request",
        "	Construction,	",
        "	Slab Floor with Radiant, !- Name",
        "	CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Outside Layer",
        "	INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Layer 2",
        "	GYP1,                    !- Layer 3",
        "	GYP2,                    !- Layer 4",
        "	FINISH FLOORING - TILE 1 / 16 IN;  !- Layer 5",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool errorsFound(false);

    GetConstructData(*state, errorsFound);

    EXPECT_NEAR(0.1524, state->dataConstruction->Construct(1).ThicknessPerpend, 0.0001);
}

TEST_F(EnergyPlusFixture, ConstructionInternalSourceEmptyField)
{

    // test whether empty "Two-Dimensional Temperature Calculation Position" causes reading errors
    std::string const idf_objects = delimited_string({
        "	ConstructionProperty:InternalHeatSource,	",
        "	Radiant Source,          !- Name",
        "	Slab Floor with Radiant, !- Construction Name",
        "	4,                       !- Source Present After Layer Number",
        "	4,                       !- Temperature Calculation Requested After Layer Number",
        "	2,                       !- Dimensions for the CTF Calculation",
        "	0.3048,                  !- Tube Spacing {m}",
        "	;                        !- Two-Dimensional Temperature Calculation Position",
        "	Construction,	",
        "	Slab Floor with Radiant, !- Name",
        "	CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Outside Layer",
        "	INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Layer 2",
        "	GYP1,                    !- Layer 3",
        "	GYP2,                    !- Layer 4",
        "	FINISH FLOORING - TILE 1 / 16 IN;  !- Layer 5",
        "	ConstructionProperty:InternalHeatSource,	",
        "	Radiant Source 2,          !- Name",
        "	Slab Floor with Radiant 2, !- Construction Name",
        "	4,                       !- Source Present After Layer Number",
        "	4,                       !- Temperature Calculation Requested After Layer Number",
        "	2,                       !- Dimensions for the CTF Calculation",
        "	0.3048,                  !- Tube Spacing {m}",
        "	0.2;                     !- Two-Dimensional Temperature Calculation Position",
        "	Construction,	",
        "	Slab Floor with Radiant 2, !- Name",
        "	CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Outside Layer",
        "	INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Layer 2",
        "	GYP1,                    !- Layer 3",
        "	GYP2,                    !- Layer 4",
        "	FINISH FLOORING - TILE 1 / 16 IN;  !- Layer 5",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool errorsFound(false);

    GetConstructData(*state, errorsFound);
    // errorsFound is true, because the materials are included in the idf snippet - that's ok

    EXPECT_EQ(state->dataConstruction->Construct(1).userTemperatureLocationPerpendicular, 0.0);
    EXPECT_EQ(state->dataConstruction->Construct(2).userTemperatureLocationPerpendicular, 0.2);
}
