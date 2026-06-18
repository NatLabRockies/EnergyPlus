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

// Google Test Headers
#include <gtest/gtest.h>

// Fixture Headers
#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>

namespace EnergyPlus {
TEST_F(EnergyPlusFixture, OutputFiles_Expected_Formatting_Tests)
{
    EXPECT_EQ(std::format("{:#12.{}F}", 123.456, 0), "        123.");
    EXPECT_EQ(std::format("{:#12.{}F}", 0.85505055394102414, 3), "       0.855");
    EXPECT_EQ(std::format("{:#12.{}F}", 18229.761511696095, 2), "    18229.76");
    EXPECT_EQ(std::format("{:12}", 4), "           4");
}

TEST_F(EnergyPlusFixture, OutputControlFiles)
{
    std::string const idf_objects = delimited_string({
        "OutputControl:Files,",
        "  No,                      !- Output CSV",
        "  No,                      !- Output MTR",
        "  No,                      !- Output ESO",
        "  No,                      !- Output EIO",
        "  No,                      !- Output Tabular",
        "  Yes,                     !- Output SQLite",
        "  Yes,                     !- Output JSON",
        "  No,                      !- Output AUDIT",
        "  Yes,                     !- Output Zone Sizing",
        "  Yes,                     !- Output System Sizing",
        "  Yes,                     !- Output DXF",
        "  No,                      !- Output BND",
        "  No,                      !- Output RDD",
        "  No,                      !- Output MDD",
        "  No,                      !- Output MTD",
        "  Yes,                     !- Output END",
        "  No,                      !- Output SHD",
        "  Yes,                     !- Output DFS",
        "  Yes,                     !- Output GLHE",
        "  Yes,                     !- Output DelightIn",
        "  Yes,                     !- Output DelightELdmp",
        "  Yes,                     !- Output DelightDFdmp",
        "  Yes,                     !- Output EDD",
        "  Yes,                     !- Output DBG",
        "  Yes,                     !- Output PerfLog",
        "  Yes,                     !- Output SLN",
        "  Yes,                     !- Output SCI",
        "  Yes,                     !- Output WRL",
        "  Yes,                     !- Output Screen",
        "  Yes,                     !- Output ExtShd",
        "  Yes;                     !- Output Tarcog",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->files.outputControl.getInput(*state);

    state->dataGlobal->DisplayUnusedObjects = true;

    state->dataInputProcessing->inputProcessor->reportOrphanRecordObjects(*state);

    // It does not include "   **   ~~~   ** Object=OutputControl:Files=OutputControl:Files 1"
    EXPECT_FALSE(match_err_stream("OutputControl:Files"));

    std::string expected_error = delimited_string({

        "   ** Warning ** The following lines are \"Unused Objects\".  These objects are in the input",
        "   **   ~~~   **  file but are never obtained by the simulation and therefore are NOT used.",
        "   **   ~~~   **  Only the first unused named object of an object class is shown.  Use Output:Diagnostics,DisplayAllWarnings; to see all.",
        "   **   ~~~   **  See InputOutputReference document for more details.",
        "   ************* Object=Building=Bldg",
        "   **   ~~~   ** Object=GlobalGeometryRules",
        "   **   ~~~   ** Object=Timestep",
        "   **   ~~~   ** Object=Version"});

    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, OutputControlFiles_GetInput)
{
    constexpr std::string_view idf_objects_fmt = R"(
OutputControl:Files,
  {},                 !- Output CSV
  {},                 !- Output MTR
  {},                 !- Output ESO
  {},                 !- Output EIO
  {},                 !- Output Tabular
  {},                 !- Output SQLite
  {},                 !- Output JSON
  {},                 !- Output AUDIT
  {},                 !- Output Space Sizing
  {},                 !- Output Zone Sizing
  {},                 !- Output System Sizing
  {},                 !- Output DXF
  {},                 !- Output BND
  {},                 !- Output RDD
  {},                 !- Output MDD
  {},                 !- Output MTD
  {},                 !- Output END
  {},                 !- Output SHD
  {},                 !- Output DFS
  {},                 !- Output GLHE
  {},                 !- Output DelightIn
  {},                 !- Output DelightELdmp
  {},                 !- Output DelightDFdmp
  {},                 !- Output EDD
  {},                 !- Output DBG
  {},                 !- Output PerfLog
  {},                 !- Output SLN
  {},                 !- Output SCI
  {},                 !- Output WRL
  {},                 !- Output Screen
  {},                 !- Output ExtShd
  {};                 !- Output Tarcog
  )";

    auto boolToString = [](bool b) { return b ? "Yes" : "No"; };

    for (int i = 0; i < 31; ++i) {
        bool csv = (i == 0);
        bool mtr = (i == 1);
        bool eso = (i == 2);
        bool eio = (i == 3);
        bool tabular = (i == 4);
        bool sqlite = (i == 5);
        bool json = (i == 6);
        bool audit = (i == 7);
        bool spsz = (i == 8);
        bool zsz = (i == 9);
        bool ssz = (i == 10);
        bool dxf = (i == 11);
        bool bnd = (i == 12);
        bool rdd = (i == 13);
        bool mdd = (i == 14);
        bool mtd = (i == 15);
        bool end = (i == 16);
        bool shd = (i == 17);
        bool dfs = (i == 18);
        bool glhe = (i == 19);
        bool delightin = (i == 20);
        bool delighteldmp = (i == 21);
        bool delightdfdmp = (i == 22);
        bool edd = (i == 23);
        bool dbg = (i == 24);
        bool perflog = (i == 25);
        bool sln = (i == 26);
        bool sci = (i == 27);
        bool wrl = (i == 28);
        bool screen = (i == 29);
        bool extshd = (i == 30);
        bool tarcog = (i == 31);

        std::string const idf_objects = std::format(idf_objects_fmt,
                                                    boolToString(csv),
                                                    boolToString(mtr),
                                                    boolToString(eso),
                                                    boolToString(eio),
                                                    boolToString(tabular),
                                                    boolToString(sqlite),
                                                    boolToString(json),
                                                    boolToString(audit),
                                                    boolToString(spsz),
                                                    boolToString(zsz),
                                                    boolToString(ssz),
                                                    boolToString(dxf),
                                                    boolToString(bnd),
                                                    boolToString(rdd),
                                                    boolToString(mdd),
                                                    boolToString(mtd),
                                                    boolToString(end),
                                                    boolToString(shd),
                                                    boolToString(dfs),
                                                    boolToString(glhe),
                                                    boolToString(delightin),
                                                    boolToString(delighteldmp),
                                                    boolToString(delightdfdmp),
                                                    boolToString(edd),
                                                    boolToString(dbg),
                                                    boolToString(perflog),
                                                    boolToString(sln),
                                                    boolToString(sci),
                                                    boolToString(wrl),
                                                    boolToString(screen),
                                                    boolToString(extshd),
                                                    boolToString(tarcog));

        EXPECT_TRUE(process_idf(idf_objects));

        state->files.outputControl.getInput(*state);

        EXPECT_EQ(csv, state->files.outputControl.csv);
        EXPECT_EQ(mtr, state->files.outputControl.mtr);
        EXPECT_EQ(eso, state->files.outputControl.eso);
        EXPECT_EQ(eio, state->files.outputControl.eio);
        EXPECT_EQ(tabular, state->files.outputControl.tabular);
        EXPECT_EQ(sqlite, state->files.outputControl.sqlite);
        EXPECT_EQ(json, state->files.outputControl.json);
        EXPECT_EQ(audit, state->files.outputControl.audit);
        EXPECT_EQ(spsz, state->files.outputControl.spsz);
        EXPECT_EQ(zsz, state->files.outputControl.zsz);
        EXPECT_EQ(ssz, state->files.outputControl.ssz);
        EXPECT_EQ(dxf, state->files.outputControl.dxf);
        EXPECT_EQ(bnd, state->files.outputControl.bnd);
        EXPECT_EQ(rdd, state->files.outputControl.rdd);
        EXPECT_EQ(mdd, state->files.outputControl.mdd);
        EXPECT_EQ(mtd, state->files.outputControl.mtd);
        EXPECT_EQ(end, state->files.outputControl.end);
        EXPECT_EQ(shd, state->files.outputControl.shd);
        EXPECT_EQ(dfs, state->files.outputControl.dfs);
        EXPECT_EQ(delightin, state->files.outputControl.delightin);
        EXPECT_EQ(delighteldmp, state->files.outputControl.delighteldmp);
        EXPECT_EQ(delightdfdmp, state->files.outputControl.delightdfdmp);
        EXPECT_EQ(edd, state->files.outputControl.edd);
        EXPECT_EQ(dbg, state->files.outputControl.dbg);
        EXPECT_EQ(perflog, state->files.outputControl.perflog);
        EXPECT_EQ(sln, state->files.outputControl.sln);
        EXPECT_EQ(sci, state->files.outputControl.sci);
        EXPECT_EQ(wrl, state->files.outputControl.wrl);
        EXPECT_EQ(screen, state->files.outputControl.screen);
        EXPECT_EQ(extshd, state->files.outputControl.extshd);
        EXPECT_EQ(tarcog, state->files.outputControl.tarcog);

        // state->clear_state();
        // Make explicit that we're resetting everything
        state->files.outputControl.csv = false;
        state->files.outputControl.mtr = false;
        state->files.outputControl.eso = false;
        state->files.outputControl.eio = false;
        state->files.outputControl.tabular = false;
        state->files.outputControl.sqlite = false;
        state->files.outputControl.json = false;
        state->files.outputControl.audit = false;
        state->files.outputControl.spsz = false;
        state->files.outputControl.zsz = false;
        state->files.outputControl.ssz = false;
        state->files.outputControl.dxf = false;
        state->files.outputControl.bnd = false;
        state->files.outputControl.rdd = false;
        state->files.outputControl.mdd = false;
        state->files.outputControl.mtd = false;
        state->files.outputControl.end = false;
        state->files.outputControl.shd = false;
        state->files.outputControl.dfs = false;
        state->files.outputControl.delightin = false;
        state->files.outputControl.delighteldmp = false;
        state->files.outputControl.delightdfdmp = false;
        state->files.outputControl.edd = false;
        state->files.outputControl.dbg = false;
        state->files.outputControl.perflog = false;
        state->files.outputControl.sln = false;
        state->files.outputControl.sci = false;
        state->files.outputControl.wrl = false;
        state->files.outputControl.screen = false;
        state->files.outputControl.extshd = false;
        state->files.outputControl.tarcog = false;
    }
}

} // namespace EnergyPlus
