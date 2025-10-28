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

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include "EnergyPlus/InputProcessing/InputProcessor.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/IOFiles.hh>

namespace EnergyPlus {
TEST_F(EnergyPlusFixture, OutputFiles_Expected_Formatting_Tests)
{
    // EXPECT_EQ(format("{:#12.{}F}", 123.456, 0), "        123."); // valid for fmt <=7.0.0 but not >=7.1.3, changed sometime in between
    EXPECT_EQ(format("{:#11.{}F}", 123.456, 0), "        123.");
    EXPECT_EQ(format("{:#12.{}F}", 0.85505055394102414, 3), "       0.855");
    EXPECT_EQ(format("{:#12.{}F}", 18229.761511696095, 2), "    18229.76");
    EXPECT_EQ(format("{:12}", 4), "           4");

    // R syntax, which replicates the "RoundSigDigits" function
    // by rounding and displaying 3 digits in the exponent and chooses between
    // fixed and E notation depending on magnitude
    EXPECT_EQ(format("{:.4R}", 8.4138E-02), "8.4138E-002");
    EXPECT_EQ(format("{:.4R}", 8.41385E-02), "8.4139E-002");

    EXPECT_EQ(format("{:.4R}", 0.1518), "0.1518");
    EXPECT_EQ(format("{:.4R}", 0.15185), "0.1519");
    EXPECT_EQ(format("{:.2R}", 42.7350), "42.74");

    // This edge case is rounding up from the old formatting routines and
    // wants to round down with 'R' format
    //
    // These are specific cases that have come up while testing E+ with regression diffs
    EXPECT_EQ(format("{:.2R}", 42.73499999999999232614), "42.74");
    EXPECT_EQ(format("{:.2R}", 42.734), "42.73");

    EXPECT_EQ(format("{:.10R}", 0.14227301774935188772), "0.1422730177");
    EXPECT_EQ(format("{:.10R}", 9.90178849143378697617E-02), "9.9017884914E-002");

    EXPECT_EQ(format("{:.10R}", 0.15991370194912388203), "0.1599137019");

    EXPECT_EQ(format("{:.3R}", 4.71499999999999974687E-02), "4.715E-002");
    EXPECT_EQ(format("{:.3R}", 2.58370321661460875667E-04), "2.584E-004");
    EXPECT_EQ(format("{:.3R}", 2.35749999999999978670E-03), "2.358E-003");
    EXPECT_EQ(format("{:.3R}", 0.37915258851937216900), "0.379");

    EXPECT_EQ(format("{:.3R}", 0.10589999999999999414), "0.106");
    EXPECT_EQ(format("{:.3R}", 1.09763681592039800961E-03), "1.098E-003");
    EXPECT_EQ(format("{:.3R}", 9.62727272727272737063E-03), "9.627E-003");
    EXPECT_EQ(format("{:.3R}", 1.59349720571666519930), "1.593");

    EXPECT_EQ(format("{:.10R}", 0.11686688704901793123), "0.1168668870");
    EXPECT_EQ(format("{:.10R}", 0.14602401770121714586), "0.1460240177");

    EXPECT_EQ(format("{:.10R}", 9.12850042469573067808E-016), "9.1285004247E-016");
    EXPECT_EQ(format("{:.10R}", 1.60782525664980535959E-015), "1.6078252566E-015");

    EXPECT_EQ(format("{:.10R}", 0.10797418337603230387), "0.1079741834");
    EXPECT_EQ(format("{:.10R}", 0.14820485805540076218), "0.1482048581");
    EXPECT_EQ(format("{:.10R}", 3.08684514533120978041E-002), "3.0868451453E-002");

    EXPECT_EQ(format("{:.3R}", 7.63142731775999418747E-003), "7.631E-003");
    EXPECT_EQ(format("{:.3R}", 1.28349999999999948505E-004), "1.283E-004");
    EXPECT_EQ(format("{:.3R}", 2.56700000000000005430E-004), "2.567E-004");
    EXPECT_EQ(format("{:.3R}", 0.15159450340364988286), "0.152");

    EXPECT_EQ(format("{:.3R}", 2.14633893312000043063E-002), "2.146E-002");
    EXPECT_EQ(format("{:.3R}", 8.55666666666666278192E-005), "8.557E-005");
    EXPECT_EQ(format("{:.3R}", 6.41749999999999878051E-005), "6.418E-005");
    EXPECT_EQ(format("{:.3R}", 0.10106298657208269420), "0.101");

    EXPECT_EQ(format("{:.3R}", 5.72357048832000323002E-003), "5.724E-003");
    EXPECT_EQ(format("{:.3R}", 8.55666666666666142667E-005), "8.557E-005");
    EXPECT_EQ(format("{:.3R}", 6.41749999999999742525E-005), "6.417E-005");
    EXPECT_EQ(format("{:.3R}", 0.10106298987671752387), "0.101");

    EXPECT_EQ(format("{:.3R}", 7.86990942144000400760E-003), "7.870E-003");
    EXPECT_EQ(format("{:.3R}", 8.55666666666666007142E-005), "8.557E-005");
    EXPECT_EQ(format("{:.3R}", 6.41749999999999607000E-005), "6.417E-005");
    EXPECT_EQ(format("{:.3R}", 0.10106298537039738739), "0.101");

    EXPECT_EQ(format("{:.3R}", 2.14633893312000077758E-002), "2.146E-002");
    EXPECT_EQ(format("{:.3R}", 8.55666666666666413717E-005), "8.557E-005");
    EXPECT_EQ(format("{:.3R}", 0.10106298657208269420), "0.101");

    EXPECT_EQ(format("{:.8R}", 3299120.2346041048876941), "3299120.23460410");

    EXPECT_EQ(format("{:.5R}", 8678.2915949994276161), "8678.29159");
    EXPECT_EQ(format("{:.5R}", 1000000000000000.00000), "1000000000000000.");
    EXPECT_EQ(format("{:.5R}", 2070.8390649997299988), "2070.83906");

    EXPECT_EQ(format("{:.2R}", 166.60499927514288743), "166.60");
    EXPECT_EQ(format("{:.5R}", 245.90393499959708379), "245.90393");

    EXPECT_EQ(format("{:.3R}", 0.16149998966664602662), "0.161");
    EXPECT_EQ(format("{:.3R}", 23.989999896666461154), "23.990");

    EXPECT_EQ(format("{:.2R}", 42.734999999999985221), "42.74");

    EXPECT_EQ(format("{:.3R}", 14391.882499999999709), "14391.883");

    EXPECT_EQ(format("{:.2R}", -3.04999999999999760192), "-3.05");
    EXPECT_EQ(format("{:.2R}", -2.28500000000000058620), "-2.29");
    EXPECT_EQ(format("{:.2R}", -6.09999999999999609201), "-6.10");
    EXPECT_EQ(format("{:.2R}", -4.57000017199999675199), "-4.57");

    EXPECT_EQ(format("{:.2R}", -0.0), "0.00");

    // kept in the code for the sake of documentation
    // code is expected to not round up with the old version, but clearly it should
    // EXPECT_EQ(format("{:.3R}", 6.41750000000000013576E-005), "6.417E-005");
    // this code should round up as well to match behavior of others
    // EXPECT_EQ(format("{:.5R}", 0.059576949999999996577), "5.95769E-002");

    // T formatting is like R, but it trims instead of rounding
    EXPECT_EQ(format("{:.3T}", 7.63142731775999418747E-003), "7.631E-003");
    EXPECT_EQ(format("{:.3T}", 1.28349999999999948505E-004), "1.283E-004");
    EXPECT_EQ(format("{:.3T}", 2.56700000000000005430E-004), "2.567E-004");
    EXPECT_EQ(format("{:.3T}", 0.15159450340364988286), "0.151");
    EXPECT_EQ(format("{:.3T}", 0.0), "0.000");

    EXPECT_EQ(format("{:.3T}", 2.14633893312000043063E-002), "2.146E-002");
    EXPECT_EQ(format("{:.3T}", 8.55666666666666278192E-005), "8.556E-005");
    EXPECT_EQ(format("{:.3T}", 6.41749999999999878051E-005), "6.417E-005");
    EXPECT_EQ(format("{:.3T}", 0.10106298657208269420), "0.101");

    EXPECT_EQ(format("{:.4T}", 0.14999999999999999445), "0.1500");
    EXPECT_EQ(format("{:.3T}", 4500.0), "4500.000");
    EXPECT_EQ(format("{:.4T}", 7.1846416734478406596), "7.1846");
    EXPECT_EQ(format("{:.4T}", 1.1846416734478406596), "1.1846");
    EXPECT_EQ(format("{:.4T}", 6.2565195738294026029), "6.2565");
    EXPECT_EQ(format("{:.4T}", 0.25651957382940215879), "0.2565");

    // kept in the code for the sake of documentation
    // code is expected to round down with the old version, but clearly it should not
    // for the case of "Trim"
    // EXPECT_EQ(format("{:.4T}", 0.096970000000000000639), "9.6969E-002");
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
    std::string const idf_objects_fmt = R"(
OutputControl:Files,
  {csv},              !- Output CSV
  {mtr},              !- Output MTR
  {eso},              !- Output ESO
  {eio},              !- Output EIO
  {tabular},          !- Output Tabular
  {sqlite},           !- Output SQLite
  {json},             !- Output JSON
  {audit},            !- Output AUDIT
  {spsz},             !- Output Space Sizing
  {zsz},              !- Output Zone Sizing
  {ssz},              !- Output System Sizing
  {dxf},              !- Output DXF
  {bnd},              !- Output BND
  {rdd},              !- Output RDD
  {mdd},              !- Output MDD
  {mtd},              !- Output MTD
  {end},              !- Output END
  {shd},              !- Output SHD
  {dfs},              !- Output DFS
  {glhe},             !- Output GLHE
  {delightin},        !- Output DelightIn
  {delighteldmp},     !- Output DelightELdmp
  {delightdfdmp},     !- Output DelightDFdmp
  {edd},              !- Output EDD
  {dbg},              !- Output DBG
  {perflog},          !- Output PerfLog
  {sln},              !- Output SLN
  {sci},              !- Output SCI
  {wrl},              !- Output WRL
  {screen},           !- Output Screen
  {extshd},           !- Output ExtShd
  {tarcog};           !- Output Tarcog
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

        std::string const idf_objects = fmt::format(idf_objects_fmt,
                                                    fmt::arg("csv", boolToString(csv)),
                                                    fmt::arg("mtr", boolToString(mtr)),
                                                    fmt::arg("eso", boolToString(eso)),
                                                    fmt::arg("eio", boolToString(eio)),
                                                    fmt::arg("tabular", boolToString(tabular)),
                                                    fmt::arg("sqlite", boolToString(sqlite)),
                                                    fmt::arg("json", boolToString(json)),
                                                    fmt::arg("audit", boolToString(audit)),
                                                    fmt::arg("spsz", boolToString(spsz)),
                                                    fmt::arg("zsz", boolToString(zsz)),
                                                    fmt::arg("ssz", boolToString(ssz)),
                                                    fmt::arg("dxf", boolToString(dxf)),
                                                    fmt::arg("bnd", boolToString(bnd)),
                                                    fmt::arg("rdd", boolToString(rdd)),
                                                    fmt::arg("mdd", boolToString(mdd)),
                                                    fmt::arg("mtd", boolToString(mtd)),
                                                    fmt::arg("end", boolToString(end)),
                                                    fmt::arg("shd", boolToString(shd)),
                                                    fmt::arg("dfs", boolToString(dfs)),
                                                    fmt::arg("glhe", boolToString(glhe)),
                                                    fmt::arg("delightin", boolToString(delightin)),
                                                    fmt::arg("delighteldmp", boolToString(delighteldmp)),
                                                    fmt::arg("delightdfdmp", boolToString(delightdfdmp)),
                                                    fmt::arg("edd", boolToString(edd)),
                                                    fmt::arg("dbg", boolToString(dbg)),
                                                    fmt::arg("perflog", boolToString(perflog)),
                                                    fmt::arg("sln", boolToString(sln)),
                                                    fmt::arg("sci", boolToString(sci)),
                                                    fmt::arg("wrl", boolToString(wrl)),
                                                    fmt::arg("screen", boolToString(screen)),
                                                    fmt::arg("extshd", boolToString(extshd)),
                                                    fmt::arg("tarcog", boolToString(tarcog)));

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
