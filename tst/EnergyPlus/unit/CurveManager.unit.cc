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

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/FileSystem.hh>
#include <embedded/EmbeddedEpJSONSchema.hh>
#include <nlohmann/json.hpp>

#include <stdexcept>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, CurveExponentialSkewNormal_MaximumCurveOutputTest)
{
    std::string const idf_objects = delimited_string({
        "Curve:ExponentialSkewNormal,",
        "  FanEff120CPLANormal,     !- Name",
        "  0.072613,                !- Coefficient1 C1",
        "  0.833213,                !- Coefficient2 C2",
        "  0.,                      !- Coefficient3 C3",
        "  0.013911,                !- Coefficient4 C4",
        "  -4.,                     !- Minimum Value of x",
        "  5.,                      !- Maximum Value of x",
        "  0.1,                     !- Minimum Curve Output",
        "  1.;                      !- Maximum Curve Output",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_EQ(1.0, state->dataCurveManager->curves(1)->outputLimits.max);
    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);

    EXPECT_EQ(0.1, state->dataCurveManager->curves(1)->outputLimits.min);
    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);
}

TEST_F(EnergyPlusFixture, QuadraticCurve)
{
    std::string const idf_objects = delimited_string({
        "Curve:QuadLinear,",
        "  MinDsnWBCurveName, ! Curve Name",
        "  -3.3333,           ! CoefficientC1",
        "  0.1,               ! CoefficientC2",
        "  38.9,              ! CoefficientC3",
        "  0.1,                ! CoefficientC4",
        "  0.5,                ! CoefficientC5",
        "  -30.,              ! Minimum Value of w",
        "  40.,               ! Maximum Value of w",
        "  0.,                ! Minimum Value of x",
        "  1.,                ! Maximum Value of x",
        "  5.,                ! Minimum Value of y",
        "  38.,               ! Maximum Value of y",
        "  0,                 ! Minimum Value of z",
        "  20,                ! Maximum Value of z",
        "  0.,                ! Minimum Curve Output",
        "  38.;               ! Maximum Curve Output",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_EQ(38.0, state->dataCurveManager->curves(1)->outputLimits.max);
    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);

    EXPECT_EQ(0., state->dataCurveManager->curves(1)->outputLimits.min);
    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);

    double var1 = 1, var2 = 0.1, var3 = 20, var4 = 10;
    double expected_value = -3.3333 + (0.1 * 1) + (38.9 * 0.1) + (0.1 * 20) + (0.5 * 10);
    EXPECT_EQ(expected_value, Curve::CurveValue(*state, 1, var1, var2, var3, var4));
}

TEST_F(EnergyPlusFixture, QuintLinearCurve)
{
    std::string const idf_objects = delimited_string({
        "Curve:QuintLinear,",
        "  MinDsnWBCurveName, ! Curve Name",
        "  -3.3333,           ! CoefficientC1",
        "  0.1,               ! CoefficientC2",
        "  38.9,              ! CoefficientC3",
        "  0.1,                ! CoefficientC4",
        "  0.5,                ! CoefficientC5",
        "  1.5,                ! CoefficientC6",
        "  0.,                ! Minimum Value of v",
        "  10.,               ! Maximum Value of v",
        "  -30.,              ! Minimum Value of w",
        "  40.,               ! Maximum Value of w",
        "  0.,                ! Minimum Value of x",
        "  1.,                ! Maximum Value of x",
        "  5.,                ! Minimum Value of y",
        "  38.,               ! Maximum Value of y",
        "  0,                 ! Minimum Value of z",
        "  20,                ! Maximum Value of z",
        "  0.,                ! Minimum Curve Output",
        "  38.;               ! Maximum Curve Output",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_EQ(38.0, state->dataCurveManager->curves(1)->outputLimits.max);
    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);

    EXPECT_EQ(0., state->dataCurveManager->curves(1)->outputLimits.min);
    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);

    double var1 = 1, var2 = 0.1, var3 = 0.5, var4 = 10, var5 = 15;
    double expected_value = -3.3333 + (0.1 * 1) + (38.9 * 0.1) + (0.1 * 0.5) + (0.5 * 10) + (1.5 * 15);
    EXPECT_EQ(expected_value, Curve::CurveValue(*state, 1, var1, var2, var3, var4, var5));
}

TEST_F(EnergyPlusFixture, TableLookup)
{
    std::string const idf_objects = delimited_string({"Table:IndependentVariable,",
                                                      "  SAFlow,                    !- Name",
                                                      "  Cubic,                     !- Interpolation Method",
                                                      "  Constant,                  !- Extrapolation Method",
                                                      "  0.714,                     !- Minimum Value",
                                                      "  1.2857,                    !- Maximum Value",
                                                      "  ,                          !- Normalization Reference Value",
                                                      "  Dimensionless,             !- Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.714286,                  !- Value 1",
                                                      "  1.0,",
                                                      "  1.2857;",
                                                      "Table:IndependentVariableList,",
                                                      "  SAFlow_Variables,          !- Name",
                                                      "  SAFlow;                    !- Independent Variable 1 Name",
                                                      "Table:Lookup,",
                                                      "  CoolCapModFuncOfSAFlow,    !- Name",
                                                      "  SAFlow_Variables,          !- Independent Variable List Name",
                                                      "  ,                          !- Normalization Method",
                                                      "  ,                          !- Normalization Divisor",
                                                      "  0.8234,                    !- Minimum Output",
                                                      "  1.1256,                    !- Maximum Output",
                                                      "  Dimensionless,             !- Output Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.823403,                  !- Output Value 1",
                                                      "  1.0,",
                                                      "  1.1256;",
                                                      "Table:Lookup,",
                                                      "  HeatCapModFuncOfSAFlow,    !- Name",
                                                      "  SAFlow_Variables,          !- Independent Variable List Name",
                                                      "  ,                          !- Normalization Method",
                                                      "  ,                          !- Normalization Divisor",
                                                      "  0.8554,                    !- Minimum Output",
                                                      "  1.0778,                    !- Maximum Output",
                                                      "  Dimensionless,             !- Output Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.8554,                    !- Output Value 1",
                                                      "  1.0,",
                                                      "  1.0778;",
                                                      "Table:IndependentVariable,",
                                                      "  WaterFlow,                 !- Name",
                                                      "  Cubic,                     !- Interpolation Method",
                                                      "  Constant,                  !- Extrapolation Method",
                                                      "  0.0,                       !- Minimum Value",
                                                      "  1.333333,                  !- Maximum Value",
                                                      "  ,                          !- Normalization Reference Value",
                                                      "  Dimensionless,             !- Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.0,                       !- Value 1,",
                                                      "  0.05,",
                                                      "  0.33333,",
                                                      "  0.5,",
                                                      "  0.666667,",
                                                      "  0.833333,",
                                                      "  1.0,",
                                                      "  1.333333;",
                                                      "Table:IndependentVariableList,",
                                                      "  WaterFlow_Variables,       !- Name",
                                                      "  WaterFlow;                 !- Independent Variable 1 Name",
                                                      "Table:Lookup,",
                                                      "  CapModFuncOfWaterFlow,     !- Name",
                                                      "  WaterFlow_Variables,       !- Independent Variable List Name",
                                                      "  ,                          !- Normalization Method",
                                                      "  ,                          !- Normalization Divisor",
                                                      "  0.0,                       !- Minimum Output",
                                                      "  1.04,                      !- Maximum Output",
                                                      "  Dimensionless,             !- Output Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.0,                       !- Output Value 1",
                                                      "  0.001,",
                                                      "  0.71,",
                                                      "  0.85,",
                                                      "  0.92,",
                                                      "  0.97,",
                                                      "  1.0,",
                                                      "  1.04;"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(3, state->dataCurveManager->curves.size());

    EXPECT_ENUM_EQ(Curve::CurveType::BtwxtTableLookup, state->dataCurveManager->curves(1)->curveType);
    EXPECT_EQ("CAPMODFUNCOFWATERFLOW", state->dataCurveManager->curves(1)->Name);

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);
    EXPECT_EQ(0.0, state->dataCurveManager->curves(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);
    EXPECT_EQ(1.04, state->dataCurveManager->curves(1)->outputLimits.max);
}

TEST_F(EnergyPlusFixture, DivisorNormalizationNone)
{
    //    /*
    //     *  Test: Normalization Method = None
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_curve_min{2.0};
    double expected_curve_max{21.0};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        ",                                      !- Normalization Method",
        ",                                      !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        "Linear,                                !- Interpolation Method",
        "Linear,                                !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->curves(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->curves(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second, Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, DivisorNormalizationDivisorOnly)
{
    //    /*
    //     *  Test: Normalization Method = DivisorOnly
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_divisor{3.0};
    double expected_curve_min{2.0 / expected_divisor};
    double expected_curve_max{21.0 / expected_divisor};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        "DivisorOnly,                           !- Normalization Method",
        "3.0,                                   !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        "Linear,                                !- Interpolation Method",
        "Linear,                                !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->curves(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->curves(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second / expected_divisor, Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, DivisorNormalizationDivisorOnlyButItIsZero)
{
    std::string const idf_objects = delimited_string({"Table:Lookup,",
                                                      "y_values,                              !- Name",
                                                      "y_values_list,                         !- Independent Variable List Name",
                                                      "DivisorOnly,                           !- Normalization Method",
                                                      "0.0,                                   !- Normalization Divisor",
                                                      "2.0,                                   !- Minimum Output",
                                                      "21.0,                                  !- Maximum Output",
                                                      "Dimensionless,                         !- Output Unit Type",
                                                      ",                                      !- External File Name",
                                                      ",                                      !- External File Column Number",
                                                      ",                                      !- External File Starting Row Number",
                                                      "2.0,                                   !- Value 1",
                                                      "4.0;                                   !- Value 2",

                                                      "Table:IndependentVariableList,",
                                                      "y_values_list,                         !- Name",
                                                      "x_values_1;                            !- Independent Variable Name 1",

                                                      "Table:IndependentVariable,",
                                                      "x_values_1,                            !- Name",
                                                      ",                                      !- Interpolation Method",
                                                      ",                                      !- Extrapolation Method",
                                                      ",                                      !- Minimum value",
                                                      ",                                      !- Maximum value",
                                                      ",                                      !- Normalization Reference Value",
                                                      "Dimensionless                          !- Output Unit Type",
                                                      ",                                      !- External File Name",
                                                      ",                                      !- External File Column Number",
                                                      ",                                      !- External File Starting Row Number",
                                                      ",",
                                                      "2.0,                                   !- Value 1",
                                                      "3.0;                                   !- Value 3"});

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_THROW(Curve::GetCurveInput(*state), std::runtime_error);
    EXPECT_TRUE(this->compare_err_stream_substring("Normalization divisor entered as zero"));
}

TEST_F(EnergyPlusFixture, DivisorNormalizationAutomaticWithDivisor)
{
    //    /*
    //     *  Test: Normalization Method = AutomaticWithDivisor
    //     *      Case: Default 'Normalization Divisor' Field
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_auto_divisor{6.0};
    double expected_curve_max{21.0 / expected_auto_divisor};
    double expected_curve_min{2.0 / expected_auto_divisor};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        "AutomaticWithDivisor,                  !- Normalization Method",
        ",                                      !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "3.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "2.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->curves(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->curves(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second / expected_auto_divisor,
                         Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, NormalizationAutomaticWithDivisorAndSpecifiedDivisor)
{
    //    /*
    //     *  Test: Normalization Method = AutomaticWithDivisor
    //     *      Case: Additionally Specified Divisor in 'Normalization Divisor' Field
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_auto_divisor{6.0};
    double normalization_divisor{4.0};
    double expected_curve_max{21.0 / expected_auto_divisor / normalization_divisor};
    double expected_curve_min{2.0 / expected_auto_divisor / normalization_divisor};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        "AutomaticWithDivisor,                  !- Normalization Method",
        "4.0,                                   !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "3.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "2.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->curves(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->curves(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->curves(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second / expected_auto_divisor / normalization_divisor,
                         Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, CSV_CarriageReturns_Handling)
{
    Curve::TableFile testTableFile = Curve::TableFile();
    fs::path testCSV = configured_source_directory() / "tst/EnergyPlus/unit/Resources/TestCarriageReturn.csv";
    testTableFile.filePath = testCSV;
    testTableFile.load(*state, testCSV);
    std::vector<double> TestArray;
    std::size_t col = 2;
    std::size_t row = 1;
    std::size_t expected_length = 168;
    TestArray = testTableFile.getArray(*state, std::make_pair(col, row));
    EXPECT_EQ(TestArray.size(), expected_length);

    for (std::size_t i = 0; i < TestArray.size(); i++) {
        EXPECT_FALSE(std::isnan(TestArray[i]));
    }
}

nlohmann::json const &getPatternProperties(nlohmann::json const &schema_obj)
{
    auto const &pattern_properties = schema_obj["patternProperties"];
    int dot_star_present = pattern_properties.count(".*");
    std::string pattern_property;
    if (dot_star_present > 0) {
        pattern_property = ".*";
    } else {
        int no_whitespace_present = pattern_properties.count(R"(^.*\S.*$)");
        if (no_whitespace_present > 0) {
            pattern_property = R"(^.*\S.*$)";
        } else {
            throw std::runtime_error(R"(The patternProperties value is not a valid choice (".*", "^.*\S.*$"))");
        }
    }
    auto const &schema_obj_props = pattern_properties[pattern_property]["properties"];
    return schema_obj_props;
}

std::vector<std::string> getPossibleChoicesFromSchema(const std::string &objectType, const std::string &fieldName)
{
    // Should consider making this public, at least to the EnergyPlusFixture, but maybe in the InputProcessor directly
    // At which point, should handle the "anyOf" case, here I don't need it, so not bothering
    static const auto json_schema = nlohmann::json::from_cbor(EmbeddedEpJSONSchema::embeddedEpJSONSchema());
    auto const &schema_properties = json_schema.at("properties");
    const auto &object_schema = schema_properties.at(objectType);
    auto const &schema_obj_props = getPatternProperties(object_schema);
    auto const &schema_field_obj = schema_obj_props.at(fieldName);
    std::vector<std::string> choices;
    for (const auto &e : schema_field_obj.at("enum")) {
        choices.push_back(e);
    }

    return choices;
}

TEST_F(EnergyPlusFixture, TableIndependentVariableUnitType_IsValid)
{
    std::vector<std::string> unit_type_choices = getPossibleChoicesFromSchema("Table:IndependentVariable", "unit_type");
    for (const auto &input_unit_type : unit_type_choices) {
        EXPECT_TRUE(Curve::IsCurveInputTypeValid(input_unit_type)) << input_unit_type << " is rejected by IsCurveInputTypeValid";
    }
    EXPECT_EQ(8, unit_type_choices.size());
}

TEST_F(EnergyPlusFixture, TableLookupUnitType_IsValid)
{
    std::vector<std::string> unit_type_choices = getPossibleChoicesFromSchema("Table:Lookup", "output_unit_type");
    for (const auto &output_unit_type : unit_type_choices) {
        if (output_unit_type.empty()) {
            continue;
        }
        EXPECT_TRUE(Curve::IsCurveOutputTypeValid(output_unit_type)) << output_unit_type << " is rejected by IsCurveOutputTypeValid";
    }
    EXPECT_EQ(6, unit_type_choices.size());
}

class InputUnitTypeIsValid : public EnergyPlusFixture, public ::testing::WithParamInterface<std::string_view>
{
};
TEST_P(InputUnitTypeIsValid, IndepentVariable)
{
    const auto &unit_type = GetParam();

    std::string const idf_objects = delimited_string({
        "Table:IndependentVariable,",
        "  SAFlow,                    !- Name",
        "  Cubic,                     !- Interpolation Method",
        "  Constant,                  !- Extrapolation Method",
        "  0.714,                     !- Minimum Value",
        "  1.2857,                    !- Maximum Value",
        "  ,                          !- Normalization Reference Value",
        fmt::format("  {},             !-  Unit Type", unit_type),
        "  ,                          !- External File Name",
        "  ,                          !- External File Column Number",
        "  ,                          !- External File Starting Row Number",
        "  0.714286,                  !- Value 1",
        "  1.0,",
        "  1.2857;",

        "Table:IndependentVariableList,",
        "  SAFlow_Variables,          !- Name",
        "  SAFlow;                    !- Independent Variable 1 Name",

        "Table:Lookup,",
        "  CoolCapModFuncOfSAFlow,    !- Name",
        "  SAFlow_Variables,          !- Independent Variable List Name",
        "  ,                          !- Normalization Method",
        "  ,                          !- Normalization Divisor",
        "  0.8234,                    !- Minimum Output",
        "  1.1256,                    !- Maximum Output",
        "  Dimensionless,             !- Output Unit Type",
        "  ,                          !- External File Name",
        "  ,                          !- External File Column Number",
        "  ,                          !- External File Starting Row Number",
        "  0.823403,                  !- Output Value 1",
        "  1.0,",
        "  1.1256;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    Curve::GetCurveInput(*state);
    EXPECT_TRUE(compare_err_stream("", true));
}

INSTANTIATE_TEST_SUITE_P(CurveManager,
                         InputUnitTypeIsValid,
                         testing::Values("", "Angle", "Dimensionless", "Distance", "MassFlow", "Power", "Temperature", "VolumetricFlow"),
                         [](const testing::TestParamInfo<InputUnitTypeIsValid::ParamType> &info) -> std::string {
                             if (info.param.empty()) {
                                 return "Blank";
                             }
                             return std::string{info.param};
                         });

class OutputUnitTypeIsValid : public EnergyPlusFixture, public ::testing::WithParamInterface<std::string_view>
{
};
TEST_P(OutputUnitTypeIsValid, TableLookup)
{
    const auto &unit_type = GetParam();

    std::string const idf_objects = delimited_string({
        "Table:IndependentVariable,",
        "  SAFlow,                    !- Name",
        "  Cubic,                     !- Interpolation Method",
        "  Constant,                  !- Extrapolation Method",
        "  0.714,                     !- Minimum Value",
        "  1.2857,                    !- Maximum Value",
        "  ,                          !- Normalization Reference Value",
        "  Dimensionless,             !- Unit Type",
        "  ,                          !- External File Name",
        "  ,                          !- External File Column Number",
        "  ,                          !- External File Starting Row Number",
        "  0.714286,                  !- Value 1",
        "  1.0,",
        "  1.2857;",

        "Table:IndependentVariableList,",
        "  SAFlow_Variables,          !- Name",
        "  SAFlow;                    !- Independent Variable 1 Name",

        "Table:Lookup,",
        "  CoolCapModFuncOfSAFlow,    !- Name",
        "  SAFlow_Variables,          !- Independent Variable List Name",
        "  ,                          !- Normalization Method",
        "  ,                          !- Normalization Divisor",
        "  0.8234,                    !- Minimum Output",
        "  1.1256,                    !- Maximum Output",
        fmt::format("  {},             !- Output Unit Type", unit_type),
        "  ,                          !- External File Name",
        "  ,                          !- External File Column Number",
        "  ,                          !- External File Starting Row Number",
        "  0.823403,                  !- Output Value 1",
        "  1.0,",
        "  1.1256;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    Curve::GetCurveInput(*state);
    EXPECT_TRUE(compare_err_stream("", true));
}

INSTANTIATE_TEST_SUITE_P(CurveManager,
                         OutputUnitTypeIsValid,
                         testing::Values("", "Capacity", "Dimensionless", "Power", "Pressure", "Temperature"),
                         [](const testing::TestParamInfo<InputUnitTypeIsValid::ParamType> &info) -> std::string {
                             if (info.param.empty()) {
                                 return "Blank";
                             }
                             return std::string{info.param};
                         });

std::pair<std::set<std::string>, std::set<std::string>> getAllPossibleInputOutputTypesForCurves()
{
    const auto json_schema = nlohmann::json::from_cbor(EmbeddedEpJSONSchema::embeddedEpJSONSchema());
    auto const &schema_properties = json_schema.at("properties");
    std::set<std::string> all_input_choices;
    std::set<std::string> all_output_choices;

    for (const auto &[objectType, object_schema] : schema_properties.items()) {
        const bool is_curve = (objectType.rfind("Curve:", 0) == 0) || (objectType == "Table:Lookup") || (objectType == "Table:IndependentVariable");
        if (!is_curve) {
            continue;
        }
        auto const &schema_obj_props = getPatternProperties(object_schema);
        for (const auto &[fieldName, schema_field_obj] : schema_obj_props.items()) {
            if (std::string(fieldName) == "output_unit_type") {
                for (const auto &e : schema_field_obj.at("enum")) {
                    all_output_choices.insert(std::string{e});
                }
            } else if (fieldName.find("unit_type") != std::string::npos) {
                for (const auto &e : schema_field_obj.at("enum")) {
                    all_input_choices.insert(std::string{e});
                }
            }
        }
    }

    return {all_input_choices, all_output_choices};
}

TEST_F(EnergyPlusFixture, AllPossibleUnitTypeValid)
{
    auto const [all_input_choices, all_output_choices] = getAllPossibleInputOutputTypesForCurves();

    // As of 2024-12-18
    // in = ["", "Angle", "Dimensionless", "Distance", "MassFlow", "Power", "Pressure", "Temperature", "VolumetricFlow","VolumetricFlowPerPower"]
    // out = ["", "Capacity", "Dimensionless", "Power", "Pressure", "Temperature"]
    EXPECT_FALSE(all_input_choices.empty()) << fmt::format("{}", all_input_choices);
    EXPECT_FALSE(all_output_choices.empty()) << fmt::format("{}", all_output_choices);

    for (const auto &input_unit_type : all_input_choices) {
        EXPECT_TRUE(Curve::IsCurveInputTypeValid(input_unit_type)) << input_unit_type << " is rejected by IsCurveOutputTypeValid";
    }

    for (const auto &output_unit_type : all_output_choices) {
        if (output_unit_type.empty()) {
            continue;
        }
        EXPECT_TRUE(Curve::IsCurveOutputTypeValid(output_unit_type)) << output_unit_type << " is rejected by IsCurveOutputTypeValid";
    }
}

TEST_F(EnergyPlusFixture, QuadraticCurve_CheckCurveMinMaxValues)
{
    std::string const idf_objects = delimited_string({
        "Curve:Quadratic,",
        "  DummyEIRfPLR,                       !- Name",
        "  1,                                  !- Coefficient1 Constant",
        "  1,                                  !- Coefficient2 x",
        "  0,                                  !- Coefficient3 x**2",
        "  0.8,                                !- Minimum Value of x {BasedOnField A2}",
        "  0.5,                                !- Maximum Value of x {BasedOnField A2}",
        "  ,                                   !- Minimum Curve Output {BasedOnField A3}",
        "  ,                                   !- Maximum Curve Output {BasedOnField A3}",
        "  ,                                   !- Input Unit Type for X",
        "  ;                                   !- Output Unit Type",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    ASSERT_THROW(Curve::GetCurveInput(*state), std::runtime_error);
    ASSERT_EQ(1, state->dataCurveManager->curves.size());

    auto const expected_error = delimited_string({
        "   ** Severe  ** GetCurveInput: For Curve:Quadratic: ",
        "   **   ~~~   ** Minimum Value of x [0.80] > Maximum Value of x [0.50]",
        "   **  Fatal  ** GetCurveInput: Errors found in getting Curve Objects.  Preceding condition(s) cause termination.",
    });
    EXPECT_TRUE(compare_err_stream_substring(expected_error));
}

struct CurveTestParam
{
    std::string object_name;
    std::string tested_dim;
    std::string idf_objects;
    std::string expected_error;

    // This is what I want to override, so why I'm using a struct and not a std::tuple<std::string, std::string, std::string, std::string>
    friend std::ostream &operator<<(std::ostream &os, const CurveTestParam &p)
    {
        return os << p.object_name << "_" << p.tested_dim;
    }
};

class CurveManagerValidationFixture : public EnergyPlusFixture, public ::testing::WithParamInterface<CurveTestParam>
{
public:
    static std::string delimited_string(std::vector<std::string> const &strings, std::string const &delimiter = "\n")
    {
        return EnergyPlusFixture::delimited_string(strings, delimiter);
    }
};

TEST_P(CurveManagerValidationFixture, CurveMinMaxValues)
{
    const auto &[object_name, tested_dim, idf_objects, expected_error] = GetParam(); // What is this? Are you bored?
    EXPECT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->curves.size());
    try {
        Curve::GetCurveInput(*state);
    } catch (const EnergyPlus::FatalError &err) {
        std::string w = err.what();
        EXPECT_TRUE(w.find("Error with format") == std::string::npos) << w; // What is this?
        // Otherwise, this is expected
    } catch (std::runtime_error const &err) {
        FAIL() << err.what();
    } catch (...) {
        FAIL() << "Got another exception!";
    }

    ASSERT_EQ(1, state->dataCurveManager->curves.size());
    EXPECT_TRUE(compare_err_stream_substring(expected_error));
}

INSTANTIATE_TEST_SUITE_P(CurveManager,
                         CurveManagerValidationFixture,
                         ::testing::Values(
                             // Curve:Functional:PressureDrop: 0 dimensions

                             // Curve:Linear: 1 dimensions
                             CurveTestParam{"Curve:Linear",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Linear,",
                                                "  Linear,                                 !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:Quadratic: 1 dimensions
                             CurveTestParam{"Curve:Quadratic",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Quadratic,",
                                                "  Quadratic,                              !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:Cubic: 1 dimensions
                             CurveTestParam{"Curve:Cubic",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Cubic,",
                                                "  Cubic,                                  !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 x**3",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:Quartic: 1 dimensions
                             CurveTestParam{"Curve:Quartic",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Quartic,",
                                                "  Quartic,                                !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 x**3",
                                                "  1,                                      !- Coefficient5 x**4",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:Exponent: 1 dimensions
                             CurveTestParam{"Curve:Exponent",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Exponent,",
                                                "  Exponent,                               !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 Constant",
                                                "  1,                                      !- Coefficient3 Constant",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:ExponentialSkewNormal: 1 dimensions
                             CurveTestParam{"Curve:ExponentialSkewNormal",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:ExponentialSkewNormal,",
                                                "  ExponentialSkewNormal,                  !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:Sigmoid: 1 dimensions
                             CurveTestParam{"Curve:Sigmoid",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Sigmoid,",
                                                "  Sigmoid,                                !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  1,                                      !- Coefficient5 C5",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:RectangularHyperbola1: 1 dimensions
                             CurveTestParam{"Curve:RectangularHyperbola1",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:RectangularHyperbola1,",
                                                "  RectangularHyperbola1,                  !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:RectangularHyperbola2: 1 dimensions
                             CurveTestParam{"Curve:RectangularHyperbola2",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:RectangularHyperbola2,",
                                                "  RectangularHyperbola2,                  !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:ExponentialDecay: 1 dimensions
                             CurveTestParam{"Curve:ExponentialDecay",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:ExponentialDecay,",
                                                "  ExponentialDecay,                       !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:DoubleExponentialDecay: 1 dimensions
                             CurveTestParam{"Curve:DoubleExponentialDecay",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:DoubleExponentialDecay,",
                                                "  DoubleExponentialDecay,                 !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  1,                                      !- Coefficient5 C5",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},

                             // Curve:Bicubic: 2 dimensions
                             CurveTestParam{"Curve:Bicubic",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Bicubic,",
                                                "  Bicubic,                                !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 y**2",
                                                "  1,                                      !- Coefficient6 x*y",
                                                "  1,                                      !- Coefficient7 x**3",
                                                "  1,                                      !- Coefficient8 y**3",
                                                "  1,                                      !- Coefficient9 x**2*y",
                                                "  1,                                      !- Coefficient10 x*y**2",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:Bicubic",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Bicubic,",
                                                "  Bicubic,                                !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 y**2",
                                                "  1,                                      !- Coefficient6 x*y",
                                                "  1,                                      !- Coefficient7 x**3",
                                                "  1,                                      !- Coefficient8 y**3",
                                                "  1,                                      !- Coefficient9 x**2*y",
                                                "  1,                                      !- Coefficient10 x*y**2",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},

                             // Curve:Biquadratic: 2 dimensions
                             CurveTestParam{"Curve:Biquadratic",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Biquadratic,",
                                                "  Biquadratic,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 y**2",
                                                "  1,                                      !- Coefficient6 x*y",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:Biquadratic",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Biquadratic,",
                                                "  Biquadratic,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 y**2",
                                                "  1,                                      !- Coefficient6 x*y",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},

                             // Curve:QuadraticLinear: 2 dimensions
                             CurveTestParam{"Curve:QuadraticLinear",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuadraticLinear,",
                                                "  QuadraticLinear,                        !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 x*y",
                                                "  1,                                      !- Coefficient6 x**2*y",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:QuadraticLinear",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuadraticLinear,",
                                                "  QuadraticLinear,                        !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 x*y",
                                                "  1,                                      !- Coefficient6 x**2*y",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},

                             // Curve:CubicLinear: 2 dimensions
                             CurveTestParam{"Curve:CubicLinear",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:CubicLinear,",
                                                "  CubicLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 x**3",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 x*y",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:CubicLinear",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:CubicLinear,",
                                                "  CubicLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x",
                                                "  1,                                      !- Coefficient3 x**2",
                                                "  1,                                      !- Coefficient4 x**3",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 x*y",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},

                             // Curve:FanPressureRise: 2 dimensions
                             CurveTestParam{"Curve:FanPressureRise",
                                            "Qfan",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:FanPressureRise,",
                                                "  FanPressureRise,                        !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  0.8,                                    !- Minimum Value of Qfan {m3/s}",
                                                "  0.5,                                    !- Maximum Value of Qfan {m3/s}",
                                                "  0,                                      !- Minimum Value of Psm {Pa}",
                                                "  1,                                      !- Maximum Value of Psm {Pa}",
                                                "  ,                                       !- Minimum Curve Output {Pa}",
                                                "  ;                                       !- Maximum Curve Output {Pa}",
                                            }),
                                            "Minimum Value of Qfan [0.80] > Maximum Value of Qfan [0.50]"},
                             CurveTestParam{"Curve:FanPressureRise",
                                            "Psm",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:FanPressureRise,",
                                                "  FanPressureRise,                        !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  0,                                      !- Minimum Value of Qfan {m3/s}",
                                                "  1,                                      !- Maximum Value of Qfan {m3/s}",
                                                "  0.8,                                    !- Minimum Value of Psm {Pa}",
                                                "  0.5,                                    !- Maximum Value of Psm {Pa}",
                                                "  ,                                       !- Minimum Curve Output {Pa}",
                                                "  ;                                       !- Maximum Curve Output {Pa}",
                                            }),
                                            "Minimum Value of Psm [0.80] > Maximum Value of Psm [0.50]"},

                             // Curve:Triquadratic: 3 dimensions
                             CurveTestParam{"Curve:Triquadratic",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Triquadratic,",
                                                "  Triquadratic,                           !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x**2",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y**2",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z**2",
                                                "  1,                                      !- Coefficient7 z",
                                                "  1,                                      !- Coefficient8 x**2*y**2",
                                                "  1,                                      !- Coefficient9 x*y",
                                                "  1,                                      !- Coefficient10 x*y**2",
                                                "  1,                                      !- Coefficient11 x**2*y",
                                                "  1,                                      !- Coefficient12 x**2*z**2",
                                                "  1,                                      !- Coefficient13 x*z",
                                                "  1,                                      !- Coefficient14 x*z**2",
                                                "  1,                                      !- Coefficient15 x**2*z",
                                                "  1,                                      !- Coefficient16 y**2*z**2",
                                                "  1,                                      !- Coefficient17 y*z",
                                                "  1,                                      !- Coefficient18 y*z**2",
                                                "  1,                                      !- Coefficient19 y**2*z",
                                                "  1,                                      !- Coefficient20 x**2*y**2*z**2",
                                                "  1,                                      !- Coefficient21 x**2*y**2*z",
                                                "  1,                                      !- Coefficient22 x**2*y*z**2",
                                                "  1,                                      !- Coefficient23 x*y**2*z**2",
                                                "  1,                                      !- Coefficient24 x**2*y*z",
                                                "  1,                                      !- Coefficient25 x*y**2*z",
                                                "  1,                                      !- Coefficient26 x*y*z**2",
                                                "  1,                                      !- Coefficient27 x*y*z",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless,                          !- Input Unit Type for Z",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:Triquadratic",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Triquadratic,",
                                                "  Triquadratic,                           !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x**2",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y**2",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z**2",
                                                "  1,                                      !- Coefficient7 z",
                                                "  1,                                      !- Coefficient8 x**2*y**2",
                                                "  1,                                      !- Coefficient9 x*y",
                                                "  1,                                      !- Coefficient10 x*y**2",
                                                "  1,                                      !- Coefficient11 x**2*y",
                                                "  1,                                      !- Coefficient12 x**2*z**2",
                                                "  1,                                      !- Coefficient13 x*z",
                                                "  1,                                      !- Coefficient14 x*z**2",
                                                "  1,                                      !- Coefficient15 x**2*z",
                                                "  1,                                      !- Coefficient16 y**2*z**2",
                                                "  1,                                      !- Coefficient17 y*z",
                                                "  1,                                      !- Coefficient18 y*z**2",
                                                "  1,                                      !- Coefficient19 y**2*z",
                                                "  1,                                      !- Coefficient20 x**2*y**2*z**2",
                                                "  1,                                      !- Coefficient21 x**2*y**2*z",
                                                "  1,                                      !- Coefficient22 x**2*y*z**2",
                                                "  1,                                      !- Coefficient23 x*y**2*z**2",
                                                "  1,                                      !- Coefficient24 x**2*y*z",
                                                "  1,                                      !- Coefficient25 x*y**2*z",
                                                "  1,                                      !- Coefficient26 x*y*z**2",
                                                "  1,                                      !- Coefficient27 x*y*z",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless,                          !- Input Unit Type for Z",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},
                             CurveTestParam{"Curve:Triquadratic",
                                            "z",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:Triquadratic,",
                                                "  Triquadratic,                           !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 x**2",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y**2",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z**2",
                                                "  1,                                      !- Coefficient7 z",
                                                "  1,                                      !- Coefficient8 x**2*y**2",
                                                "  1,                                      !- Coefficient9 x*y",
                                                "  1,                                      !- Coefficient10 x*y**2",
                                                "  1,                                      !- Coefficient11 x**2*y",
                                                "  1,                                      !- Coefficient12 x**2*z**2",
                                                "  1,                                      !- Coefficient13 x*z",
                                                "  1,                                      !- Coefficient14 x*z**2",
                                                "  1,                                      !- Coefficient15 x**2*z",
                                                "  1,                                      !- Coefficient16 y**2*z**2",
                                                "  1,                                      !- Coefficient17 y*z",
                                                "  1,                                      !- Coefficient18 y*z**2",
                                                "  1,                                      !- Coefficient19 y**2*z",
                                                "  1,                                      !- Coefficient20 x**2*y**2*z**2",
                                                "  1,                                      !- Coefficient21 x**2*y**2*z",
                                                "  1,                                      !- Coefficient22 x**2*y*z**2",
                                                "  1,                                      !- Coefficient23 x*y**2*z**2",
                                                "  1,                                      !- Coefficient24 x**2*y*z",
                                                "  1,                                      !- Coefficient25 x*y**2*z",
                                                "  1,                                      !- Coefficient26 x*y*z**2",
                                                "  1,                                      !- Coefficient27 x*y*z",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  0.8,                                    !- Minimum Value of z {BasedOnField A4}",
                                                "  0.5,                                    !- Maximum Value of z {BasedOnField A4}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                "  Dimensionless,                          !- Input Unit Type for X",
                                                "  Dimensionless,                          !- Input Unit Type for Y",
                                                "  Dimensionless,                          !- Input Unit Type for Z",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of z [0.80] > Maximum Value of z [0.50]"},

                             // Curve:ChillerPartLoadWithLift: 3 dimensions
                             CurveTestParam{"Curve:ChillerPartLoadWithLift",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:ChillerPartLoadWithLift,",
                                                "  ChillerPartLoadWithLift,                !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  1,                                      !- Coefficient5 C5",
                                                "  1,                                      !- Coefficient6 C6",
                                                "  1,                                      !- Coefficient7 C7",
                                                "  1,                                      !- Coefficient8 C8",
                                                "  1,                                      !- Coefficient9 C9",
                                                "  1,                                      !- Coefficient10 C10",
                                                "  1,                                      !- Coefficient11 C11",
                                                "  1,                                      !- Coefficient12 C12",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless,                          !- Input Unit Type for z",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:ChillerPartLoadWithLift",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:ChillerPartLoadWithLift,",
                                                "  ChillerPartLoadWithLift,                !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  1,                                      !- Coefficient5 C5",
                                                "  1,                                      !- Coefficient6 C6",
                                                "  1,                                      !- Coefficient7 C7",
                                                "  1,                                      !- Coefficient8 C8",
                                                "  1,                                      !- Coefficient9 C9",
                                                "  1,                                      !- Coefficient10 C10",
                                                "  1,                                      !- Coefficient11 C11",
                                                "  1,                                      !- Coefficient12 C12",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless,                          !- Input Unit Type for z",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},
                             CurveTestParam{"Curve:ChillerPartLoadWithLift",
                                            "z",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:ChillerPartLoadWithLift,",
                                                "  ChillerPartLoadWithLift,                !- Name",
                                                "  1,                                      !- Coefficient1 C1",
                                                "  1,                                      !- Coefficient2 C2",
                                                "  1,                                      !- Coefficient3 C3",
                                                "  1,                                      !- Coefficient4 C4",
                                                "  1,                                      !- Coefficient5 C5",
                                                "  1,                                      !- Coefficient6 C6",
                                                "  1,                                      !- Coefficient7 C7",
                                                "  1,                                      !- Coefficient8 C8",
                                                "  1,                                      !- Coefficient9 C9",
                                                "  1,                                      !- Coefficient10 C10",
                                                "  1,                                      !- Coefficient11 C11",
                                                "  1,                                      !- Coefficient12 C12",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                "  0.8,                                    !- Minimum Value of z {BasedOnField A4}",
                                                "  0.5,                                    !- Maximum Value of z {BasedOnField A4}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless,                          !- Input Unit Type for z",
                                                "  Dimensionless;                          !- Output Unit Type",
                                            }),
                                            "Minimum Value of z [0.80] > Maximum Value of z [0.50]"},

                             // Curve:QuadLinear: 4 dimensions
                             CurveTestParam{"Curve:QuadLinear",
                                            "w",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuadLinear,",
                                                "  QuadLinear,                             !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 w",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 z",
                                                "  0.8,                                    !- Minimum Value of w {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of w [0.80] > Maximum Value of w [0.50]"},
                             CurveTestParam{"Curve:QuadLinear",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuadLinear,",
                                                "  QuadLinear,                             !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 w",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 z",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:QuadLinear",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuadLinear,",
                                                "  QuadLinear,                             !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 w",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 z",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A4}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},
                             CurveTestParam{"Curve:QuadLinear",
                                            "z",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuadLinear,",
                                                "  QuadLinear,                             !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 w",
                                                "  1,                                      !- Coefficient3 x",
                                                "  1,                                      !- Coefficient4 y",
                                                "  1,                                      !- Coefficient5 z",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0.8,                                    !- Minimum Value of z {BasedOnField A5}",
                                                "  0.5,                                    !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of z [0.80] > Maximum Value of z [0.50]"},

                             // Curve:QuintLinear: 5 dimensions
                             CurveTestParam{"Curve:QuintLinear",
                                            "v",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuintLinear,",
                                                "  QuintLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 v",
                                                "  1,                                      !- Coefficient3 w",
                                                "  1,                                      !- Coefficient4 x",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z",
                                                "  0.8,                                    !- Minimum Value of v {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of v {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for v",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of v [0.80] > Maximum Value of v [0.50]"},
                             CurveTestParam{"Curve:QuintLinear",
                                            "w",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuintLinear,",
                                                "  QuintLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 v",
                                                "  1,                                      !- Coefficient3 w",
                                                "  1,                                      !- Coefficient4 x",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z",
                                                "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of w {BasedOnField A2}",
                                                "  0.5,                                    !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for v",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of w [0.80] > Maximum Value of w [0.50]"},
                             CurveTestParam{"Curve:QuintLinear",
                                            "x",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuintLinear,",
                                                "  QuintLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 v",
                                                "  1,                                      !- Coefficient3 w",
                                                "  1,                                      !- Coefficient4 x",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z",
                                                "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0.8,                                    !- Minimum Value of x {BasedOnField A3}",
                                                "  0.5,                                    !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for v",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of x [0.80] > Maximum Value of x [0.50]"},
                             CurveTestParam{"Curve:QuintLinear",
                                            "y",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuintLinear,",
                                                "  QuintLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 v",
                                                "  1,                                      !- Coefficient3 w",
                                                "  1,                                      !- Coefficient4 x",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z",
                                                "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0.8,                                    !- Minimum Value of y {BasedOnField A4}",
                                                "  0.5,                                    !- Maximum Value of y {BasedOnField A4}",
                                                "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for v",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of y [0.80] > Maximum Value of y [0.50]"},
                             CurveTestParam{"Curve:QuintLinear",
                                            "z",
                                            CurveManagerValidationFixture::delimited_string({
                                                "Curve:QuintLinear,",
                                                "  QuintLinear,                            !- Name",
                                                "  1,                                      !- Coefficient1 Constant",
                                                "  1,                                      !- Coefficient2 v",
                                                "  1,                                      !- Coefficient3 w",
                                                "  1,                                      !- Coefficient4 x",
                                                "  1,                                      !- Coefficient5 y",
                                                "  1,                                      !- Coefficient6 z",
                                                "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                "  0.8,                                    !- Minimum Value of z {BasedOnField A5}",
                                                "  0.5,                                    !- Maximum Value of z {BasedOnField A5}",
                                                "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                "  Dimensionless,                          !- Input Unit Type for v",
                                                "  Dimensionless,                          !- Input Unit Type for w",
                                                "  Dimensionless,                          !- Input Unit Type for x",
                                                "  Dimensionless,                          !- Input Unit Type for y",
                                                "  Dimensionless;                          !- Input Unit Type for z",
                                            }),
                                            "Minimum Value of z [0.80] > Maximum Value of z [0.50]"}),
                         [](const testing::TestParamInfo<CurveManagerValidationFixture::ParamType> &info) -> std::string {
                             auto object_name = info.param.object_name;
                             std::replace_if(object_name.begin(), object_name.end(), [](char c) { return !std::isalnum(c); }, '_');
                             return object_name + "_" + info.param.tested_dim;
                         });
