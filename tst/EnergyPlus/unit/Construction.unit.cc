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

// EnergyPlus::Construction Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/OutputReportPredefined.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::Construction;

TEST_F(EnergyPlusFixture, Construction_reportLayers)
{
    using namespace EnergyPlus::OutputReportPredefined;

    auto &c = state->dataConstruction;
    auto &m = state->dataMaterial;
    auto &orp = *state->dataOutRptPredefined;

    SetPredefinedTables(*state);

    auto *mata = new Material::MaterialBase;
    mata->Name = "mat a";
    m->materials.push_back(mata);
    auto *matb = new Material::MaterialBase;
    matb->Name = "mat b";
    m->materials.push_back(matb);
    auto *matc = new Material::MaterialBase;
    matc->Name = "mat c";
    m->materials.push_back(matc);
    auto *matd = new Material::MaterialBase;
    matd->Name = "mat d";
    m->materials.push_back(matd);
    auto *mate = new Material::MaterialBase;
    mate->Name = "mat e";
    m->materials.push_back(mate);
    auto *matf = new Material::MaterialBase;
    matf->Name = "mat f";
    m->materials.push_back(matf);
    auto *matg = new Material::MaterialBase;
    matg->Name = "mat g";
    m->materials.push_back(matg);
    auto *math = new Material::MaterialBase;
    math->Name = "mat h";
    m->materials.push_back(math);

    c->Construct.allocate(3);

    c->Construct(1).Name = "ConsB";
    c->Construct(1).TotLayers = 1;
    c->Construct(1).LayerPoint(1) = 2;
    c->Construct(1).reportLayers(*state);

    c->Construct(2).Name = "ConsCEGAH";
    c->Construct(2).TotLayers = 5;
    c->Construct(2).LayerPoint(1) = 3;
    c->Construct(2).LayerPoint(2) = 5;
    c->Construct(2).LayerPoint(3) = 7;
    c->Construct(2).LayerPoint(4) = 1;
    c->Construct(2).LayerPoint(5) = 8;
    c->Construct(2).reportLayers(*state);

    c->Construct(3).Name = "ConsDA";
    c->Construct(3).TotLayers = 2;
    c->Construct(3).LayerPoint(1) = 4;
    c->Construct(3).LayerPoint(2) = 1;
    c->Construct(3).reportLayers(*state);

    EXPECT_EQ("mat b", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[0], "ConsB"));
    EXPECT_EQ("NOT FOUND", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[1], "ConsB"));

    EXPECT_EQ("mat c", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[0], "ConsCEGAH"));
    EXPECT_EQ("mat e", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[1], "ConsCEGAH"));
    EXPECT_EQ("mat g", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[2], "ConsCEGAH"));
    EXPECT_EQ("mat a", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[3], "ConsCEGAH"));
    EXPECT_EQ("mat h", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[4], "ConsCEGAH"));
    EXPECT_EQ("NOT FOUND", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[5], "ConsCEGAH"));

    EXPECT_EQ("mat d", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[0], "ConsDA"));
    EXPECT_EQ("mat a", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[1], "ConsDA"));
    EXPECT_EQ("NOT FOUND", RetrievePreDefTableEntry(*state, orp.pdchOpqConsLayCol[2], "ConsDA"));
}
