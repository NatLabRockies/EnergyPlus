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

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/ExtendedHeatIndex.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>
#include <EnergyPlus/IceThermalStorage.hh>

using namespace EnergyPlus;
using namespace ExtendedHI;

TEST_F(EnergyPlusFixture, extendedHI_pvstar)
{
    Real64 tol = 1e-8;
    std::vector<Real64> T_values = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370};
    std::vector<Real64> result = {0.16315953,
                                  0.70457376,
                                  2.66392126,
                                  8.97272134,
                                  27.31539419,
                                  76.07472151,
                                  195.83100376,
                                  470.03352248,
                                  991.92542226,
                                  1920.68015554,
                                  3538.94082369,
                                  6235.88791594,
                                  10554.04916628,
                                  17222.31477378,
                                  27187.71571487,
                                  41643.76611223,
                                  62053.26405691,
                                  90163.72448627};

    for (size_t i = 0; i < T_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::pvstar(T_values[i]), result[i], tol);
    }
}

TEST_F(EnergyPlusFixture, extendedHI_Le)
{
    Real64 tol = 1e-2;
    std::vector<Real64> T_values = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370};
    std::vector<Real64> result = {2663805.16,
                                  2641405.16,
                                  2619005.16,
                                  2596605.16,
                                  2574205.16,
                                  2551805.16,
                                  2529405.16,
                                  2507005.16,
                                  2484605.16,
                                  2462205.16,
                                  2439805.16,
                                  2417405.16,
                                  2395005.16,
                                  2372605.16,
                                  2350205.16,
                                  2327805.16,
                                  2305405.16,
                                  2283005.16};
    for (size_t i = 0; i < T_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::Le(T_values[i]), result[i], tol);
    }
}

TEST_F(EnergyPlusFixture, extendedHI_Qv)
{
    Real64 tol = 1e-8;
    std::vector<Real64> T_values = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370};
    std::vector<Real64> P_values = {1, 10, 100, 1000, 10000};
    std::vector<std::vector<Real64>> result = {{49.94618971, 49.91176799, 49.56755081, 46.12537897, 11.70366056},
                                               {47.35664275, 47.32222103, 46.97800385, 43.53583201, 9.1141136},
                                               {44.76709579, 44.73267407, 44.38845689, 40.94628505, 6.52456664},
                                               {42.17754883, 42.14312711, 41.79890993, 38.35673809, 3.93501968},
                                               {39.58800187, 39.55358015, 39.20936297, 35.76719113, 1.34547272},
                                               {36.99845491, 36.96403319, 36.61981601, 33.17764417, -1.24407424},
                                               {34.40890795, 34.37448623, 34.03026905, 30.58809721, -3.8336212},
                                               {31.81936099, 31.78493927, 31.44072209, 27.99855025, -6.42316816},
                                               {29.22981403, 29.19539231, 28.85117513, 25.40900329, -9.01271512},
                                               {26.64026707, 26.60584535, 26.26162817, 22.81945633, -11.60226208},
                                               {24.05072011, 24.01629839, 23.67208121, 20.22990937, -14.19180904},
                                               {21.46117315, 21.42675143, 21.08253425, 17.64036241, -16.781356},
                                               {18.87162619, 18.83720447, 18.49298729, 15.05081545, -19.37090296},
                                               {16.28207923, 16.24765751, 15.90344033, 12.46126849, -21.96044992},
                                               {13.69253227, 13.65811055, 13.31389337, 9.87172153, -24.54999688},
                                               {11.10298531, 11.06856359, 10.72434641, 7.28217457, -27.13954384},
                                               {8.51343835, 8.47901663, 8.13479945, 4.69262761, -29.7290908},
                                               {5.92389139, 5.88946967, 5.54525249, 2.10308065, -32.31863776}};
    for (size_t i = 0; i < T_values.size(); ++i) {
        for (size_t j = 0; j < P_values.size(); ++j) {
            EXPECT_NEAR(ExtendedHI::Qv(T_values[i], P_values[j]), result[i][j], tol);
        }
    }
}

TEST_F(EnergyPlusFixture, extendedHI_Zs)
{
    Real64 tol = 1e-8;
    std::vector<Real64> Rs_values = {0.0387, 0.5, 1, 1.2};
    std::vector<Real64> result = {52.1, 18750000.0, 600000000.0, 1492991999.9999998};
    for (size_t i = 0; i < Rs_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::Zs(Rs_values[i]), result[i], tol);
    }
}

TEST_F(EnergyPlusFixture, extendedHI_Ra)
{
    Real64 tol = 1e-8;
    std::vector<Real64> Ts_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<Real64> Ta_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<std::vector<Real64>> result = {{0.05003743, 0.04919687, 0.04829495, 0.04733483, 0.04632048, 0.04525658, 0.04414843},
                                               {0.04919687, 0.04834035, 0.0474255, 0.04645568, 0.045435, 0.0443682, 0.04326057},
                                               {0.04829495, 0.0474255, 0.04650092, 0.04552473, 0.04450111, 0.04343485, 0.04233118},
                                               {0.04733483, 0.04645568, 0.04552473, 0.0445456, 0.04352252, 0.04246025, 0.04136396},
                                               {0.04632048, 0.045435, 0.04450111, 0.04352252, 0.04250345, 0.0414486, 0.04036303},
                                               {0.04525658, 0.0443682, 0.04343485, 0.04246025, 0.0414486, 0.04040451, 0.03933288},
                                               {0.04414843, 0.04326057, 0.04233118, 0.04136396, 0.04036303, 0.03933288, 0.03827822}};
    for (size_t i = 0; i < Ts_values.size(); ++i) {
        for (size_t j = 0; j < Ta_values.size(); ++j) {
            EXPECT_NEAR(ExtendedHI::Ra(Ts_values[i], Ta_values[j]), result[i][j], tol);
        }
    }
}

TEST_F(EnergyPlusFixture, extendedHI_Ra_bar)
{
    Real64 tol = 1e-8;
    std::vector<Real64> Tf_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<Real64> Ta_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<std::vector<Real64>> result = {{0.07141547, 0.06983279, 0.06815365, 0.0663875, 0.06454507, 0.06263806, 0.06067882},
                                               {0.06983279, 0.06823771, 0.06655337, 0.06478927, 0.06295607, 0.06106521, 0.05912865},
                                               {0.06815365, 0.06655337, 0.06487109, 0.06311629, 0.06129941, 0.05943158, 0.0575243},
                                               {0.0663875, 0.06478927, 0.06311629, 0.06137788, 0.05958421, 0.05774599, 0.05587418},
                                               {0.06454507, 0.06295607, 0.06129941, 0.05958421, 0.05782026, 0.05601779, 0.05418719},
                                               {0.06263806, 0.06106521, 0.05943158, 0.05774599, 0.05601779, 0.05425668, 0.05247244},
                                               {0.06067882, 0.05912865, 0.0575243, 0.05587418, 0.05418719, 0.05247244, 0.0507391}};

    for (size_t i = 0; i < Tf_values.size(); ++i) {
        for (size_t j = 0; j < Ta_values.size(); ++j) {
            EXPECT_NEAR(ExtendedHI::Ra_bar(Tf_values[i], Ta_values[j]), result[i][j], tol);
        }
    }
}

TEST_F(EnergyPlusFixture, extendedHI_Ra_un)
{
    Real64 tol = 1e-8;
    std::vector<Real64> Tf_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<Real64> Ta_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<std::vector<Real64>> result = {{0.06787493, 0.06642598, 0.06488609, 0.06326346, 0.06156753, 0.05980867, 0.05799795},
                                               {0.06642598, 0.06496324, 0.06341598, 0.0617925, 0.06010222, 0.05835534, 0.05656263},
                                               {0.06488609, 0.06341598, 0.06186786, 0.06025007, 0.0585719, 0.0568433, 0.05507465},
                                               {0.06326346, 0.0617925, 0.06025007, 0.05864444, 0.05698468, 0.05528041, 0.05354159},
                                               {0.06156753, 0.06010222, 0.0585719, 0.05698468, 0.05534934, 0.05367512, 0.0519715},
                                               {0.05980867, 0.05835534, 0.0568433, 0.05528041, 0.05367512, 0.05203623, 0.0503727},
                                               {0.05799795, 0.05656263, 0.05507465, 0.05354159, 0.0519715, 0.0503727, 0.0487536}};
    for (size_t i = 0; i < Tf_values.size(); ++i) {
        for (size_t j = 0; j < Ta_values.size(); ++j) {
            EXPECT_NEAR(ExtendedHI::Ra_un(Tf_values[i], Ta_values[j]), result[i][j], tol);
        }
    }
}

TEST_F(EnergyPlusFixture, extendedHI_find_eqvar)
{
    Real64 tol = 0.001;
    std::vector<Real64> Ta_values = {240, 260, 280, 300, 320, 340, 360};
    std::vector<Real64> RH_values = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
    std::vector<std::vector<EqvarName>> result_0 = {
        {EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf},
        {EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf},
        {EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf, EqvarName::Rf},
        {EqvarName::Rf, EqvarName::Rs, EqvarName::Rs, EqvarName::Rs, EqvarName::Rs, EqvarName::Rs},
        {EqvarName::Rs, EqvarName::Rs, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt},
        {EqvarName::Rs, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt},
        {EqvarName::Rs, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt, EqvarName::DTcdt}};

    std::vector<std::vector<Real64>> result_1 = {{33.043, 32.696, 32.355, 32.022, 31.695, 31.374},
                                                 {1.56, 1.546, 1.532, 1.519, 1.505, 1.491},
                                                 {0.442, 0.423, 0.404, 0.386, 0.369, 0.352},
                                                 {0.011, 0.038, 0.037, 0.035, 0.034, 0.031},
                                                 {0.027, 0.023, 0.0, 0.003, 0.006, 0.009},
                                                 {0.021, 0.005, 0.012, 0.02, 0.027, 0.035},
                                                 {0.001, 0.017, 0.034, 0.051, 0.068, 0.085}};

    EqvarName eqvar_name = EqvarName::Invalid;
    Real64 eqvar_value = 0.0;
    for (size_t i = 0; i < Ta_values.size(); ++i) {
        for (size_t j = 0; j < RH_values.size(); ++j) {
            eqvar_value = find_eqvar_name_and_value(*state, Ta_values[i], RH_values[j], eqvar_name);
            EXPECT_EQ(eqvar_name, result_0[i][j]);
            EXPECT_NEAR(eqvar_value, result_1[i][j], tol);
        }
    }
}

TEST_F(EnergyPlusFixture, extendedHI_find_T)
{
    Real64 tol = 0.05;
    state->dataRootFinder->rootAlgo = RootAlgo::Bisection;
    std::vector<Real64> Rf_values = {30, 32, 34, 36, 38};
    std::vector<Real64> result_0_rf = {240.08, 239.97, 239.89, 239.81, 239.74};
    for (size_t i = 0; i < Rf_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::find_T(*state, EnergyPlus::ExtendedHI::EqvarName::Rf, Rf_values[i]), result_0_rf[i], tol);
    }
    std::vector<Real64> Rs_values = {0.01, 0.02, 0.03};
    std::vector<Real64> result_0_rs = {337.87, 329.76, 307.48};
    for (size_t i = 0; i < Rs_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::find_T(*state, EnergyPlus::ExtendedHI::EqvarName::Rs, Rs_values[i]), result_0_rs[i], tol);
    }
    std::vector<Real64> phi_values = {0.86, 0.88, 0.90, 0.92, 0.94, 0.96};
    std::vector<Real64> result_0_phi = {228.69, 216.00, 199.00, 175.19, 139.71, 82.05};
    for (size_t i = 0; i < phi_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::find_T(*state, EnergyPlus::ExtendedHI::EqvarName::Phi, phi_values[i]), result_0_phi[i], tol);
    }
    std::vector<Real64> dTcdt_values = {0.01, 0.03, 0.05, 0.07, 0.09};
    std::vector<Real64> result_0_dTcdt = {412.53, 512.36, 584.55, 641.20, 688.04};
    for (size_t i = 0; i < dTcdt_values.size(); ++i) {
        EXPECT_NEAR(ExtendedHI::find_T(*state, EnergyPlus::ExtendedHI::EqvarName::DTcdt, dTcdt_values[i]), result_0_dTcdt[i], tol);
    }
}

TEST_F(EnergyPlusFixture, extendedHI_heatindex)
{

    state->dataRootFinder->rootAlgo = RootAlgo::Bisection;
    std::vector<std::vector<Real64>> HI_values = {{199.9994, 199.9997, 200.0},
                                                  {209.9976, 209.9988, 210.0},
                                                  {219.9916, 219.9958, 220.0},
                                                  {229.974, 229.987, 230.0},
                                                  {239.9254, 239.9627, 240.0},
                                                  {249.7677, 249.8837, 250.0},
                                                  {259.3736, 259.6864, 260.0},
                                                  {268.5454, 269.2746, 270.0},
                                                  {277.2234, 278.6369, 280.0},
                                                  {285.7511, 288.2814, 290.7861},
                                                  {297.5738, 300.2923, 305.3947},
                                                  {305.555, 318.6226, 359.9063},
                                                  {313.0299, 359.0539, 407.5345},
                                                  {320.5089, 398.576, 464.9949},
                                                  {328.0358, 445.8599, 530.5525},
                                                  {333.2806, 500.0422, 601.9518},
                                                  {343.6313, 559.664, 677.2462},
                                                  {354.1826, 623.196, 755.0833}};

    std::vector<Real64> T_values = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370};
    std::vector<Real64> RH_values = {0, 0.5, 1};

    Real64 tol = 0.1;

    for (size_t i = 0; i < T_values.size(); ++i) {
        for (size_t j = 0; j < RH_values.size(); ++j) {
            Real64 HI = HI_values[i][j];
            EXPECT_NEAR(ExtendedHI::heatindex(*state, T_values[i], RH_values[j]), HI, tol);
        }
    }
}

// helper: calculating HI the old way
Real64 calcHI(Real64 ZoneTF, Real64 ZoneRH)
{
    Real64 HI;
    Real64 constexpr c1 = -42.379;
    Real64 constexpr c2 = 2.04901523;
    Real64 constexpr c3 = 10.14333127;
    Real64 constexpr c4 = -.22475541;
    Real64 constexpr c5 = -.00683783;
    Real64 constexpr c6 = -.05481717;
    Real64 constexpr c7 = .00122874;
    Real64 constexpr c8 = .00085282;
    Real64 constexpr c9 = -.00000199;
    if (ZoneTF < 80) {
        HI = 0.5 * (ZoneTF + 61.0 + (ZoneTF - 68.0) * 1.2 + (ZoneRH * 0.094));
    } else {
        HI = c1 + c2 * ZoneTF + c3 * ZoneRH + c4 * ZoneTF * ZoneRH + c5 * ZoneTF * ZoneTF + c6 * ZoneRH * ZoneRH + c7 * ZoneTF * ZoneTF * ZoneRH +
             c8 * ZoneTF * ZoneRH * ZoneRH + c9 * ZoneTF * ZoneTF * ZoneRH * ZoneRH;
        if (ZoneRH < 13 && ZoneTF < 112) {
            HI -= (13 - ZoneRH) / 4 * std::sqrt((17 - abs(ZoneTF - 95)) / 17);
        } else if (ZoneRH > 85 && ZoneTF < 87) {
            HI += (ZoneRH - 85) / 10 * (87 - ZoneTF) / 5;
        }
    }
    HI = (HI - 32.0) * (5.0 / 9.0);
    return HI;
}

// compare different the old and new HI method
TEST_F(EnergyPlusFixture, extendedHI_heatindex_compare)
{
    //    uncomment the following to write out outputs comparing the old HI and the extended HI
    //    for (int T = 80; T < 112; T += 2) {
    //        for (int RH_percent = 40; RH_percent < 105; RH_percent += 5) {
    //            fmt::print("{},{},{},{}\n", T, Fahrenheit2Celsius(T) + 273.15, RH_percent, calcHI(T * 1.0, RH_percent) + 273.15);
    //        }
    //    }
    //
    //    fmt::print("--------------------------extended HI------------------------------\n");
    //
    //    for (int T = 80; T < 112; T += 2) {
    //        for (int RH_percent = 40; RH_percent < 105; RH_percent += 5) {
    //            fmt::print("{},{},{},{}\n",
    //                       T,
    //                       Fahrenheit2Celsius(T) + 273.15,
    //                       RH_percent,
    //                       ExtendedHI::heatindex(*state, Fahrenheit2Celsius(T) + 273.15, RH_percent / 100.0, false));
    //        }
    //    }

    std::vector<std::vector<Real64>> HI_values = {{299.50263955106493,
                                                   299.776740099187,
                                                   300.0697403535014,
                                                   300.3840374486754,
                                                   300.7224627089454,
                                                   301.0883855406428,
                                                   301.48585129820276,
                                                   301.91976206202526,
                                                   302.39612058678176,
                                                   302.9223619942786,
                                                   303.50781721121166,
                                                   304.164366737823,
                                                   304.90738965163473},
                                                  {300.7371586287627,
                                                   301.08082855993416,
                                                   301.45165138470475,
                                                   301.85347046062816,
                                                   302.29090810229536,
                                                   302.7695771184517,
                                                   303.2963640097296,
                                                   303.8798186200438,
                                                   304.5306942932075,
                                                   305.2627171872882,
                                                   306.0937017010292,
                                                   307.0472131000133,
                                                   308.15511379914824},
                                                  {302.02736771141645,
                                                   302.4561810697196,
                                                   302.92349559196737,
                                                   303.4353941195877,
                                                   303.9993409969611,
                                                   304.62460199545603,
                                                   305.322829571669,
                                                   306.10889266303275,
                                                   307.00207884714473,
                                                   308.027883155155,
                                                   309.2207373859128,
                                                   313.0628398671979,
                                                   321.5848247316899},
                                                  {303.3841495186789,
                                                   303.9170689758612,
                                                   304.50409744807985,
                                                   305.1547874516109,
                                                   305.88112467259634,
                                                   306.69836198852863,
                                                   307.62622559734154,
                                                   308.69069155014586,
                                                   309.9266883142991,
                                                   311.38231908960734,
                                                   318.54246110946406,
                                                   328.0043066147482,
                                                   333.3393972698832},
                                                  {304.8199354641838,
                                                   305.48032203514595,
                                                   306.21634121227544,
                                                   307.0429394330131,
                                                   307.97934497182723,
                                                   309.05072723689955,
                                                   310.29069143289234,
                                                   311.7451353772776,
                                                   313.95530016103294,
                                                   323.83440227771644,
                                                   332.4701096833451,
                                                   334.90678639907856,
                                                   337.31894975469913},
                                                  {306.3492841011612,
                                                   307.16624582593795,
                                                   308.0886627669679,
                                                   309.13992447021883,
                                                   310.35097800020594,
                                                   311.7636495939223,
                                                   313.43586634553503,
                                                   317.6494802389061,
                                                   328.3628307696199,
                                                   333.74059265770484,
                                                   336.3222867687,
                                                   338.8761290133698,
                                                   341.4020377426641},
                                                  {307.98957814869937,
                                                   308.99980285495985,
                                                   310.15708861581516,
                                                   311.49815879471134,
                                                   313.0731694836868,
                                                   314.95241500844713,
                                                   320.7787010347238,
                                                   331.8317274673609,
                                                   334.83416476810817,
                                                   337.5699159357464,
                                                   340.2740648871986,
                                                   342.94652084296104,
                                                   345.6054543473874},
                                                  {309.7619108116487,
                                                   311.0122043680167,
                                                   312.46811247605365,
                                                   314.187658362207,
                                                   316.2529625179013,
                                                   323.0801825976232,
                                                   332.79726637585554,
                                                   335.7326857879525,
                                                   338.63242015999276,
                                                   341.4963439834537,
                                                   344.32435625989456,
                                                   347.16610411327565,
                                                   349.9882094984059},
                                                  {311.6922567837173,
                                                   313.2431093422929,
                                                   315.0828601612011,
                                                   317.30399334745016,
                                                   324.3992667720886,
                                                   333.3013983833371,
                                                   336.4164381631417,
                                                   339.4910959439585,
                                                   342.5252268166514,
                                                   345.53482289920794,
                                                   348.55059671303025,
                                                   351.53749776567565,
                                                   354.49586372385966},
                                                  {313.81305196962785,
                                                   315.74373047973495,
                                                   318.0832217884017,
                                                   324.6363173221471,
                                                   333.556578377611,
                                                   336.86389599752147,
                                                   340.12557708716486,
                                                   343.3414515358163,
                                                   346.5464459863142,
                                                   349.74051965837134,
                                                   352.90212786378106,
                                                   356.0316775160027,
                                                   359.1295787363197},
                                                  {316.1653665843187,
                                                   318.58128181367647,
                                                   323.6855917231878,
                                                   333.5378166526789,
                                                   337.0514521509176,
                                                   340.51358119759243,
                                                   343.9240011043148,
                                                   347.3324049610528,
                                                   350.715991469624,
                                                   354.063080122869,
                                                   357.3741601619986,
                                                   360.6497245162609,
                                                   363.89026874647243},
                                                  {318.80192411888856,
                                                   321.8453275115462,
                                                   333.2175307610305,
                                                   336.9530971109634,
                                                   340.63061076856684,
                                                   344.2498254548991,
                                                   347.8701384467422,
                                                   351.4554844770464,
                                                   354.99980266642524,
                                                   358.5036799122463,
                                                   361.96770739537897,
                                                   365.39247917797184,
                                                   368.7785908780643},
                                                  {321.79129250056576,
                                                   332.5651462195674,
                                                   336.5400431206217,
                                                   340.4496004333487,
                                                   344.2935131903505,
                                                   348.13508995372104,
                                                   351.9356167348451,
                                                   355.6900213044719,
                                                   359.3990061376826,
                                                   363.0632788382354,
                                                   366.6835501792957,
                                                   370.2605324515025,
                                                   373.7949378686608},
                                                  {329.9497329717269,
                                                   335.7802855706541,
                                                   339.9405041738646,
                                                   344.0269053383963,
                                                   348.1004405542626,
                                                   352.130907458195,
                                                   356.1095161383855,
                                                   360.03710570017574,
                                                   363.9145213572192,
                                                   367.74261204962386,
                                                   371.5222280813032,
                                                   375.2542191799148,
                                                   378.9394325375906},
                                                  {334.63809705979656,
                                                   339.06981446722057,
                                                   343.41864144138526,
                                                   347.7367934017093,
                                                   352.013488568773,
                                                   356.2318584616878,
                                                   360.3929031532607,
                                                   364.49763041891856,
                                                   368.54705243284116,
                                                   372.5421826945967,
                                                   376.48403349361615,
                                                   380.3736132776248,
                                                   384.2119247702067},
                                                  {337.80000485421624,
                                                   342.43363185960334,
                                                   347.0118012771127,
                                                   351.55276412755484,
                                                   356.0280998123926,
                                                   360.438997685269,
                                                   364.78665706905304,
                                                   369.07228278025286,
                                                   373.2970810952247,
                                                   377.4622562734294,
                                                   381.56900731119094,
                                                   385.6185252717114,
                                                   389.61199074954493}};

    std::vector<Real64> T_values;
    for (int i = 0; i < 16; ++i) {
        T_values.push_back(80 + 2 * i);
    }
    std::vector<Real64> RH_values;
    for (int i = 0; i < 13; ++i) {
        RH_values.push_back(40 + 5 * i);
    }

    Real64 extended;
    for (size_t i = 0; i < T_values.size(); ++i) {
        for (size_t j = 0; j < RH_values.size(); ++j) {
            extended = ExtendedHI::heatindex(*state, IceThermalStorage::TempIPtoSI(T_values[i]) + 273.15, RH_values[j] / 100.0);
            EXPECT_NEAR(HI_values[i][j], extended, 0.1);
        }
    }
}
