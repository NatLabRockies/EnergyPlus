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

// EnergyPlus::WindowManager unit tests

// C++ Headers
#include <iostream>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/EconomicLifeCycleCost.hh>
#include <EnergyPlus/EconomicTariff.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>

#include "Fixtures/EnergyPlusFixture.hh"

#include <nlohmann/json.hpp>

using namespace EnergyPlus;
using namespace EnergyPlus::EconomicLifeCycleCost;
using namespace EnergyPlus::EconomicTariff;

TEST_F(EnergyPlusFixture, EconomicLifeCycleCost_GetInput)
{

    std::string const idf_objects = delimited_string({
        "  LifeCycleCost:Parameters,                                           ",
        "    TypicalLCC,              !- Name                                  ",
        "    EndOfYear,               !- Discounting Convention                ",
        "    ConstantDollar,          !- Inflation Approach                    ",
        "    0.03,                    !- Real Discount Rate                    ",
        "    ,                        !- Nominal Discount Rate                 ",
        "    ,                        !- Inflation                             ",
        "    January,                 !- Base Date Month                       ",
        "    2012,                    !- Base Date Year                        ",
        "    January,                 !- Service Date Month                    ",
        "    2014,                    !- Service Date Year                     ",
        "    22,                      !- Length of Study Period in Years       ",
        "    0,                       !- Tax rate                              ",
        "    ;                        !- Depreciation Method                   ",
        "                                                                      ",
        "  LifeCycleCost:NonrecurringCost,                                     ",
        "    Phase One Investment,    !- Name                                  ",
        "    Construction,            !- Category                              ",
        "    51500,                   !- Cost                                  ",
        "    BasePeriod,              !- Start of Costs                        ",
        "    0,                       !- Years from Start                      ",
        "    0;                       !- Months from Start                     ",
        "                                                                      ",
        "  LifeCycleCost:NonrecurringCost,                                     ",
        "    Phase Two Investment,    !- Name                                  ",
        "    Construction,            !- Category                              ",
        "    51500,                   !- Cost                                  ",
        "    BasePeriod,              !- Start of Costs                        ",
        "    1,                       !- Years from Start                      ",
        "    0;                       !- Months from Start                     ",
        "                                                                      ",
        "  LifeCycleCost:NonrecurringCost,                                     ",
        "    FanReplacement,          !- Name                                  ",
        "    OtherCapital,            !- Category                              ",
        "    12000,                   !- Cost                                  ",
        "    BasePeriod,              !- Start of Costs                        ",
        "    13,                      !- Years from Start                      ",
        "    0;                       !- Months from Start                     ",
        "                                                                      ",
        "  LifeCycleCost:NonrecurringCost,                                     ",
        "    PlantReplacement,        !- Name                                  ",
        "    OtherCapital,            !- Category                              ",
        "    60000,                   !- Cost                                  ",
        "    BasePeriod,              !- Start of Costs                        ",
        "    16,                      !- Years from Start                      ",
        "    0;                       !- Months from Start                     ",
        "                                                                      ",
        "  LifeCycleCost:NonrecurringCost,                                     ",
        "    ResidualValue,           !- Name                                  ",
        "    Salvage,                 !- Category                              ",
        "    -20000,                  !- Cost                                  ",
        "    BasePeriod,              !- Start of Costs                        ",
        "    21,                      !- Years from Start                      ",
        "    0;                       !- Months from Start                     ",
        "                                                                      ",
        "  LifeCycleCost:RecurringCosts,                                       ",
        "    AnnualMaint,             !- Name                                  ",
        "    Maintenance,             !- Category                              ",
        "    7000,                    !- Cost                                  ",
        "    ServicePeriod,           !- Start of Costs                        ",
        "    0,                       !- Years from Start                      ",
        "    0,                       !- Months from Start                     ",
        "    1,                       !- Repeat Period Years                   ",
        "    0,                       !- Repeat Period Months                  ",
        "    0;                       !- Annual escalation rate                ",
        "                                                                      ",
        "  LifeCycleCost:UsePriceEscalation,                                   ",
        "    MidWest  Commercial-Electricity,  !- Name                         ",
        "    ElectricityPurchased,    !- Resource                              ",
        "    2012,                    !- Escalation Start Year                 ",
        "    January,                 !- Escalation Start Month                ",
        "    1.0072,                  !- Year 1 Escalation                     ",
        "    1.0148,                  !- Year 2 Escalation                     ",
        "    1.0315,                  !- Year 3 Escalation                     ",
        "    1.0493,                  !- Year 4 Escalation                     ",
        "    1.0505,                  !- Year 5 Escalation                     ",
        "    1.0451,                  !- Year 6 Escalation                     ",
        "    1.0429,                  !- Year 7 Escalation                     ",
        "    1.0410,                  !- Year 8 Escalation                     ",
        "    1.0406,                  !- Year 9 Escalation                     ",
        "    1.0444,                  !- Year 10 Escalation                    ",
        "    1.0505,                  !- Year 11 Escalation                    ",
        "    1.0535,                  !- Year 12 Escalation                    ",
        "    1.0524,                  !- Year 13 Escalation                    ",
        "    1.0478,                  !- Year 14 Escalation                    ",
        "    1.0429,                  !- Year 15 Escalation                    ",
        "    1.0391,                  !- Year 16 Escalation                    ",
        "    1.0372,                  !- Year 17 Escalation                    ",
        "    1.0360,                  !- Year 18 Escalation                    ",
        "    1.0341,                  !- Year 19 Escalation                    ",
        "    1.0319,                  !- Year 20 Escalation                    ",
        "    1.0288,                  !- Year 21 Escalation                    ",
        "    1.0341,                  !- Year 22 Escalation                    ",
        "    1.0425,                  !- Year 23 Escalation                    ",
        "    1.0508,                  !- Year 24 Escalation                    ",
        "    1.0569,                  !- Year 25 Escalation                    ",
        "    1.0626,                  !- Year 26 Escalation                    ",
        "    1.0694,                  !- Year 27 Escalation                    ",
        "    1.0721,                  !- Year 28 Escalation                    ",
        "    1.0744,                  !- Year 29 Escalation                    ",
        "    1.0774;                  !- Year 30 Escalation                    ",
        "                                                                      ",
        "  LifeCycleCost:UsePriceEscalation,                                   ",
        "    MidWest  Commercial-Electricity,  !- Name                         ",
        "    ElectricitySurplusSold,  !- Resource                              ",
        "    2012,                    !- Escalation Start Year                 ",
        "    January,                 !- Escalation Start Month                ",
        "    1.0072,                  !- Year 1 Escalation                     ",
        "    1.0148,                  !- Year 2 Escalation                     ",
        "    1.0315,                  !- Year 3 Escalation                     ",
        "    1.0493,                  !- Year 4 Escalation                     ",
        "    1.0505,                  !- Year 5 Escalation                     ",
        "    1.0451,                  !- Year 6 Escalation                     ",
        "    1.0429,                  !- Year 7 Escalation                     ",
        "    1.0410,                  !- Year 8 Escalation                     ",
        "    1.0406,                  !- Year 9 Escalation                     ",
        "    1.0444,                  !- Year 10 Escalation                    ",
        "    1.0505,                  !- Year 11 Escalation                    ",
        "    1.0535,                  !- Year 12 Escalation                    ",
        "    1.0524,                  !- Year 13 Escalation                    ",
        "    1.0478,                  !- Year 14 Escalation                    ",
        "    1.0429,                  !- Year 15 Escalation                    ",
        "    1.0391,                  !- Year 16 Escalation                    ",
        "    1.0372,                  !- Year 17 Escalation                    ",
        "    1.0360,                  !- Year 18 Escalation                    ",
        "    1.0341,                  !- Year 19 Escalation                    ",
        "    1.0319,                  !- Year 20 Escalation                    ",
        "    1.0288,                  !- Year 21 Escalation                    ",
        "    1.0341,                  !- Year 22 Escalation                    ",
        "    1.0425,                  !- Year 23 Escalation                    ",
        "    1.0508,                  !- Year 24 Escalation                    ",
        "    1.0569,                  !- Year 25 Escalation                    ",
        "    1.0626,                  !- Year 26 Escalation                    ",
        "    1.0694,                  !- Year 27 Escalation                    ",
        "    1.0721,                  !- Year 28 Escalation                    ",
        "    1.0744,                  !- Year 29 Escalation                    ",
        "    1.0774;                  !- Year 30 Escalation                    ",
        "                                                                      ",
        "  LifeCycleCost:UsePriceEscalation,                                   ",
        "    MidWest  Commercial-Natural gas,  !- Name                         ",
        "    NaturalGas,              !- Resource                              ",
        "    2012,                    !- Escalation Start Year                 ",
        "    January,                 !- Escalation Start Month                ",
        "    0.9950,                  !- Year 1 Escalation                     ",
        "    0.9711,                  !- Year 2 Escalation                     ",
        "    0.9774,                  !- Year 3 Escalation                     ",
        "    0.9849,                  !- Year 4 Escalation                     ",
        "    0.9899,                  !- Year 5 Escalation                     ",
        "    1.0000,                  !- Year 6 Escalation                     ",
        "    1.0113,                  !- Year 7 Escalation                     ",
        "    1.0289,                  !- Year 8 Escalation                     ",
        "    1.0616,                  !- Year 9 Escalation                     ",
        "    1.1031,                  !- Year 10 Escalation                    ",
        "    1.1321,                  !- Year 11 Escalation                    ",
        "    1.1484,                  !- Year 12 Escalation                    ",
        "    1.1623,                  !- Year 13 Escalation                    ",
        "    1.1786,                  !- Year 14 Escalation                    ",
        "    1.1925,                  !- Year 15 Escalation                    ",
        "    1.2025,                  !- Year 16 Escalation                    ",
        "    1.2138,                  !- Year 17 Escalation                    ",
        "    1.2289,                  !- Year 18 Escalation                    ",
        "    1.2453,                  !- Year 19 Escalation                    ",
        "    1.2642,                  !- Year 20 Escalation                    ",
        "    1.2818,                  !- Year 21 Escalation                    ",
        "    1.3182,                  !- Year 22 Escalation                    ",
        "    1.3560,                  !- Year 23 Escalation                    ",
        "    1.3899,                  !- Year 24 Escalation                    ",
        "    1.4151,                  !- Year 25 Escalation                    ",
        "    1.4491,                  !- Year 26 Escalation                    ",
        "    1.4881,                  !- Year 27 Escalation                    ",
        "    1.4818,                  !- Year 28 Escalation                    ",
        "    1.4931,                  !- Year 29 Escalation                    ",
        "    1.5145;                  !- Year 30 Escalation                    ",
        "                                                                      ",
        "  LifeCycleCost:UseAdjustment,                                        ",
        "    NoElectricUseAdjustment, !- Name                                  ",
        "    ElectricityPurchased,    !- Resource                              ",
        "    1.0;                     !- Year 1 Multiplier                     ",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetInputForLifeCycleCost(*state);

    EXPECT_ENUM_EQ(DiscConv::EndOfYear, state->dataEconLifeCycleCost->discountConvention);
    EXPECT_ENUM_EQ(InflAppr::ConstantDollar, state->dataEconLifeCycleCost->inflationApproach);
    EXPECT_EQ(0.03, state->dataEconLifeCycleCost->realDiscountRate);
    EXPECT_EQ(0, state->dataEconLifeCycleCost->baseDateMonth);
    EXPECT_EQ(2012, state->dataEconLifeCycleCost->baseDateYear);
    EXPECT_EQ(22 * 12, state->dataEconLifeCycleCost->lengthStudyTotalMonths);

    EXPECT_EQ(5, state->dataEconLifeCycleCost->numNonrecurringCost);
    EXPECT_EQ("RESIDUALVALUE", state->dataEconLifeCycleCost->NonrecurringCost[4].name);
    EXPECT_EQ(CostCategory::Salvage, state->dataEconLifeCycleCost->NonrecurringCost[4].category);
    EXPECT_ENUM_EQ(StartCosts::BasePeriod, state->dataEconLifeCycleCost->NonrecurringCost[4].startOfCosts);
    EXPECT_EQ(-20000., state->dataEconLifeCycleCost->NonrecurringCost[4].cost);

    EXPECT_EQ(1, state->dataEconLifeCycleCost->numRecurringCosts);
    EXPECT_EQ("ANNUALMAINT", state->dataEconLifeCycleCost->RecurringCosts[0].name);
    EXPECT_EQ(CostCategory::Maintenance, state->dataEconLifeCycleCost->RecurringCosts[0].category);
    EXPECT_EQ(7000., state->dataEconLifeCycleCost->RecurringCosts[0].cost);
    EXPECT_ENUM_EQ(StartCosts::ServicePeriod, state->dataEconLifeCycleCost->RecurringCosts[0].startOfCosts);
    EXPECT_EQ(1, state->dataEconLifeCycleCost->RecurringCosts[0].repeatPeriodYears);

    EXPECT_EQ(3, state->dataEconLifeCycleCost->numUsePriceEscalation);
    EXPECT_EQ("MIDWEST  COMMERCIAL-NATURAL GAS", state->dataEconLifeCycleCost->UsePriceEscalation(3).name);
    EXPECT_EQ(2012, state->dataEconLifeCycleCost->UsePriceEscalation(3).escalationStartYear);
    EXPECT_EQ(1.1321, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(11));
    EXPECT_EQ(1.2818, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(21));

    EXPECT_EQ(1, state->dataEconLifeCycleCost->numUseAdjustment);
    EXPECT_EQ("NOELECTRICUSEADJUSTMENT", state->dataEconLifeCycleCost->UseAdjustment(1).name);
    EXPECT_EQ(1.0, state->dataEconLifeCycleCost->UseAdjustment(1).Adjustment(1));
}

TEST_F(EnergyPlusFixture, EconomicLifeCycleCost_ProcessMaxInput)
{

    std::string const idf_objects = delimited_string({
        "  LifeCycleCost:Parameters,                                           ",
        "    TypicalLCC,              !- Name                                  ",
        "    EndOfYear,               !- Discounting Convention                ",
        "    ConstantDollar,          !- Inflation Approach                    ",
        "    0.03,                    !- Real Discount Rate                    ",
        "    ,                        !- Nominal Discount Rate                 ",
        "    ,                        !- Inflation                             ",
        "    January,                 !- Base Date Month                       ",
        "    2012,                    !- Base Date Year                        ",
        "    January,                 !- Service Date Month                    ",
        "    2014,                    !- Service Date Year                     ",
        "    100,                     !- Length of Study Period in Years       ",
        "    0,                       !- Tax rate                              ",
        "    ;                        !- Depreciation Method                   ",
        "                                                                      ",
        "  LifeCycleCost:NonrecurringCost,                                     ",
        "    FanReplacement,          !- Name                                  ",
        "    OtherCapital,            !- Category                              ",
        "    12000,                   !- Cost                                  ",
        "    BasePeriod,              !- Start of Costs                        ",
        "    13,                      !- Years from Start                      ",
        "    0;                       !- Months from Start                     ",
        "                                                                      ",
        "  LifeCycleCost:RecurringCosts,                                       ",
        "    AnnualMaint,             !- Name                                  ",
        "    Maintenance,             !- Category                              ",
        "    7000,                    !- Cost                                  ",
        "    ServicePeriod,           !- Start of Costs                        ",
        "    0,                       !- Years from Start                      ",
        "    0,                       !- Months from Start                     ",
        "    1,                       !- Repeat Period Years                   ",
        "    0,                       !- Repeat Period Months                  ",
        "    0;                       !- Annual escalation rate                ",
        "                                                                      ",
        "  LifeCycleCost:UsePriceEscalation,                                   ",
        "    MidWest  Commercial-Electricity,  !- Name                         ",
        "    ElectricityPurchased,    !- Resource                              ",
        "    2012,                    !- Escalation Start Year                 ",
        "    January,                 !- Escalation Start Month                ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 1-10 Escalation ",
        "    1.008, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 11-20 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 21-30 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 31-40 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 41-50 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 51-60 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 61-70 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 71-80 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 81-90 Escalation ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.099, 1.100; !- Year 91-100 Escalation ",
        "                                                                      ",
        "  LifeCycleCost:UsePriceEscalation,                                   ",
        "    MidWest  Commercial-Electricity,  !- Name                         ",
        "    ElectricitySurplusSold,  !- Resource                              ",
        "    2012,                    !- Escalation Start Year                 ",
        "    January,                 !- Escalation Start Month                ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 1-10 Escalation ",
        "    1.008, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 11-20 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 21-30 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 31-40 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 41-50 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 51-60 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 61-70 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 71-80 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 81-90 Escalation ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.099, 1.100; !- Year 91-100 Escalation ",
        "                                                                      ",
        "  LifeCycleCost:UsePriceEscalation,                                   ",
        "    MidWest  Commercial-Natural gas,  !- Name                         ",
        "    NaturalGas,              !- Resource                              ",
        "    2012,                    !- Escalation Start Year                 ",
        "    January,                 !- Escalation Start Month                ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 1-10 Escalation ",
        "    1.008, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 11-20 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 21-30 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 31-40 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 41-50 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 51-60 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 61-70 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 71-80 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 81-90 Escalation ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.099, 1.100; !- Year 91-100 Escalation ",
        "                                                                      ",
        "  LifeCycleCost:UseAdjustment,                                        ",
        "    NoElectricUseAdjustment, !- Name                                  ",
        "    ElectricityPurchased,    !- Resource                              ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 1-10 Escalation ",
        "    1.008, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 11-20 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 21-30 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 31-40 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 41-50 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 51-60 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 61-70 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 71-80 Escalation ",
        "    1.009, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.040, 1.044, !- Year 81-90 Escalation ",
        "    1.007, 1.014, 1.031, 1.049, 1.050, 1.045, 1.042, 1.041, 1.099, 1.100; !- Year 91-100 Escalation ",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetInputForLifeCycleCost(*state);

    EXPECT_ENUM_EQ(DiscConv::EndOfYear, state->dataEconLifeCycleCost->discountConvention);
    EXPECT_ENUM_EQ(InflAppr::ConstantDollar, state->dataEconLifeCycleCost->inflationApproach);
    EXPECT_EQ(0.03, state->dataEconLifeCycleCost->realDiscountRate);
    EXPECT_EQ(0, state->dataEconLifeCycleCost->baseDateMonth);
    EXPECT_EQ(2012, state->dataEconLifeCycleCost->baseDateYear);
    EXPECT_EQ(100 * 12, state->dataEconLifeCycleCost->lengthStudyTotalMonths);

    EXPECT_EQ(3, state->dataEconLifeCycleCost->numUsePriceEscalation);
    EXPECT_EQ("MIDWEST  COMMERCIAL-NATURAL GAS", state->dataEconLifeCycleCost->UsePriceEscalation(3).name);
    EXPECT_EQ(2012, state->dataEconLifeCycleCost->UsePriceEscalation(3).escalationStartYear);
    EXPECT_EQ(1.007, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(1));
    EXPECT_EQ(1.008, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(11));
    EXPECT_EQ(1.009, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(21));
    EXPECT_EQ(1.099, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(99));
    EXPECT_EQ(1.100, state->dataEconLifeCycleCost->UsePriceEscalation(3).Escalation(100));

    EXPECT_EQ(1, state->dataEconLifeCycleCost->numUseAdjustment);
    EXPECT_EQ("NOELECTRICUSEADJUSTMENT", state->dataEconLifeCycleCost->UseAdjustment(1).name);
    EXPECT_EQ(1.007, state->dataEconLifeCycleCost->UseAdjustment(1).Adjustment(1));
    EXPECT_EQ(1.008, state->dataEconLifeCycleCost->UseAdjustment(1).Adjustment(11));
    EXPECT_EQ(1.009, state->dataEconLifeCycleCost->UseAdjustment(1).Adjustment(21));
    EXPECT_EQ(1.099, state->dataEconLifeCycleCost->UseAdjustment(1).Adjustment(99));
    EXPECT_EQ(1.100, state->dataEconLifeCycleCost->UseAdjustment(1).Adjustment(100));
}

TEST_F(EnergyPlusFixture, EconomicLifeCycleCost_ComputeEscalatedEnergyCosts)
{
    state->dataEconLifeCycleCost->lengthStudyYears = 5;

    state->dataEconLifeCycleCost->numCashFlow = 1;
    state->dataEconLifeCycleCost->CashFlow.resize(state->dataEconLifeCycleCost->numCashFlow);
    state->dataEconLifeCycleCost->CashFlow[0].pvKind = PrValKind::Energy;
    state->dataEconLifeCycleCost->CashFlow[0].Resource = Constant::eResource::Electricity;
    state->dataEconLifeCycleCost->CashFlow[0].yrAmount.allocate(state->dataEconLifeCycleCost->lengthStudyYears);
    state->dataEconLifeCycleCost->CashFlow[0].yrAmount(1) = 100;
    state->dataEconLifeCycleCost->CashFlow[0].yrAmount(2) = 110;
    state->dataEconLifeCycleCost->CashFlow[0].yrAmount(3) = 120;
    state->dataEconLifeCycleCost->CashFlow[0].yrAmount(4) = 130;
    state->dataEconLifeCycleCost->CashFlow[0].yrAmount(5) = 140;

    state->dataEconLifeCycleCost->numResourcesUsed = 1;

    for (int year = 1; year <= state->dataEconLifeCycleCost->lengthStudyYears; ++year) {
        std::array<Real64, static_cast<int>(Constant::eResource::Num)> yearMap;
        std::fill(yearMap.begin(), yearMap.end(), 0.0);
        state->dataEconLifeCycleCost->EscalatedEnergy[year] = yearMap;
    }

    state->dataEconLifeCycleCost->EscalatedTotEnergy.allocate(state->dataEconLifeCycleCost->lengthStudyYears);
    state->dataEconLifeCycleCost->EscalatedTotEnergy = 0.0;

    ComputeEscalatedEnergyCosts(*state);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(1)[static_cast<int>(Constant::eResource::Electricity)], 100., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(2)[static_cast<int>(Constant::eResource::Electricity)], 110., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(3)[static_cast<int>(Constant::eResource::Electricity)], 120., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(4)[static_cast<int>(Constant::eResource::Electricity)], 130., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(5)[static_cast<int>(Constant::eResource::Electricity)], 140., 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(1), 100., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(2), 110., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(3), 120., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(4), 130., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(5), 140., 0.001);

    state->dataEconLifeCycleCost->numUsePriceEscalation = 1;
    state->dataEconLifeCycleCost->UsePriceEscalation.allocate(state->dataEconLifeCycleCost->numUsePriceEscalation);
    state->dataEconLifeCycleCost->UsePriceEscalation(1).resource = Constant::eResource::Electricity;
    state->dataEconLifeCycleCost->UsePriceEscalation(1).Escalation.allocate(state->dataEconLifeCycleCost->lengthStudyYears);
    state->dataEconLifeCycleCost->UsePriceEscalation(1).Escalation(1) = 1.03;
    state->dataEconLifeCycleCost->UsePriceEscalation(1).Escalation(2) = 1.05;
    state->dataEconLifeCycleCost->UsePriceEscalation(1).Escalation(3) = 1.07;
    state->dataEconLifeCycleCost->UsePriceEscalation(1).Escalation(4) = 1.11;
    state->dataEconLifeCycleCost->UsePriceEscalation(1).Escalation(5) = 1.15;

    // reset this variable to zero
    state->dataEconLifeCycleCost->EscalatedTotEnergy = 0.0;

    ComputeEscalatedEnergyCosts(*state);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(1)[static_cast<int>(Constant::eResource::Electricity)], 103.0, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(2)[static_cast<int>(Constant::eResource::Electricity)], 115.5, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(3)[static_cast<int>(Constant::eResource::Electricity)], 128.4, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(4)[static_cast<int>(Constant::eResource::Electricity)], 144.3, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedEnergy.at(5)[static_cast<int>(Constant::eResource::Electricity)], 161.0, 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(1), 103., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(2), 115.5, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(3), 128.4, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(4), 144.3, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->EscalatedTotEnergy(5), 161.0, 0.001);
}

TEST_F(EnergyPlusFixture, EconomicLifeCycleCost_GetMonthNumber)
{
    EXPECT_EQ(0, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("January")));
    EXPECT_EQ(1, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("February")));
    EXPECT_EQ(2, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("March")));
    EXPECT_EQ(3, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("April")));
    EXPECT_EQ(4, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("May")));
    EXPECT_EQ(5, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("June")));
    EXPECT_EQ(6, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("July")));
    EXPECT_EQ(7, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("August")));
    EXPECT_EQ(8, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("September")));
    EXPECT_EQ(9, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("October")));
    EXPECT_EQ(10, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("November")));
    EXPECT_EQ(11, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("December")));
    EXPECT_EQ(-1, getEnumValue(Util::MonthNamesUC, Util::makeUPPER("Hexember")));
}

TEST_F(EnergyPlusFixture, EconomicLifeCycleCost_ExpressAsCashFlows)
{
    state->dataEconLifeCycleCost->baseDateYear = 2020;
    state->dataEconLifeCycleCost->baseDateMonth = 0;

    state->dataEconLifeCycleCost->serviceDateYear = 2023;
    state->dataEconLifeCycleCost->serviceDateMonth = 0;

    state->dataEconLifeCycleCost->lengthStudyYears = 5;
    state->dataEconLifeCycleCost->lengthStudyTotalMonths = state->dataEconLifeCycleCost->lengthStudyYears * 12;

    state->dataEconTariff->numTariff = 1;
    state->dataEconTariff->tariff.allocate(1);
    state->dataEconTariff->tariff(1).isSelected = true;
    state->dataEconTariff->tariff(1).resource = Constant::eResource::Electricity;
    state->dataEconTariff->tariff(1).cats[(int)Cat::Total] = 1;
    state->dataEconTariff->econVar.allocate(1);
    state->dataEconTariff->econVar(1).values.allocate(12);
    state->dataEconTariff->econVar(1).values(1) = 101.;
    state->dataEconTariff->econVar(1).values(2) = 102.;
    state->dataEconTariff->econVar(1).values(3) = 103.;
    state->dataEconTariff->econVar(1).values(4) = 104.;
    state->dataEconTariff->econVar(1).values(5) = 105.;
    state->dataEconTariff->econVar(1).values(6) = 106.;
    state->dataEconTariff->econVar(1).values(7) = 107.;
    state->dataEconTariff->econVar(1).values(8) = 108.;
    state->dataEconTariff->econVar(1).values(9) = 109.;
    state->dataEconTariff->econVar(1).values(10) = 110.;
    state->dataEconTariff->econVar(1).values(11) = 111.;
    state->dataEconTariff->econVar(1).values(12) = 112.;

    state->dataEconLifeCycleCost->numNonrecurringCost = 1;
    state->dataEconLifeCycleCost->NonrecurringCost.resize(1);
    state->dataEconLifeCycleCost->NonrecurringCost[0].name = "MiscConstruction";
    state->dataEconLifeCycleCost->NonrecurringCost[0].name = "MiscConstruction";
    state->dataEconLifeCycleCost->NonrecurringCost[0].category = CostCategory::Construction;
    state->dataEconLifeCycleCost->NonrecurringCost[0].cost = 123456.;
    state->dataEconLifeCycleCost->NonrecurringCost[0].startOfCosts = StartCosts::ServicePeriod;
    state->dataEconLifeCycleCost->NonrecurringCost[0].totalMonthsFromStart = 10;

    ExpressAsCashFlows(*state);

    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[16].mnAmount(47), 123456., 0.001); // 36 months plus 10 months plus one month

    // first year
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(37), 101., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(38), 102., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(39), 103., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(40), 104., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(41), 105., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(42), 106., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(43), 107., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(44), 108., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(45), 109., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(46), 110., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(47), 111., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(48), 112., 0.001);
    // second  year
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(49), 101., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(50), 102., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(51), 103., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(52), 104., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(53), 105., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(54), 106., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(55), 107., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(56), 108., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(57), 109., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(58), 110., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(59), 111., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].mnAmount(60), 112., 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].yrAmount(4), 1278., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[17].yrAmount(5), 1278., 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::Energy].yrAmount(4), 1278., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::Energy].yrAmount(5), 1278., 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::TotEnergy].yrAmount(4), 1278., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::TotEnergy].yrAmount(5), 1278., 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::Construction].yrAmount(4), 123456, 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::TotCaptl].yrAmount(4), 123456, 0.001);

    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::TotGrand].yrAmount(4), 1278. + 123456., 0.001);
    EXPECT_NEAR(state->dataEconLifeCycleCost->CashFlow[CostCategory::TotGrand].yrAmount(5), 1278., 0.001);
}

TEST_F(EnergyPlusFixture, EconomicLifeCycleCost_GetInput_EnsureFuelTypesAllRecognized)
{
    using json = nlohmann::json;
    const json &lcc_useprice_props = state->dataInputProcessing->inputProcessor->getObjectSchemaProps(*state, "LifeCycleCost:UsePriceEscalation");
    const json &resource_field = lcc_useprice_props.at("resource");
    const json &enum_values = resource_field.at("enum");

    // Should support all fuels + ElectricityXXX (Purchased, Produced, SurplusSold, Net)
    // THIS IS BRITTLE AS HECK.  DON'T RELY ON SUBSETS OF ENUMERATIONS BEING IN SOME ORDER.
    constexpr size_t numResources = static_cast<size_t>(Constant::eFuel::Num) + 3;
    // Constant::eFuel::Num has 15 fuel types including "None" (which is a fuel type for "OtherEquipment")
    // "LifeCycleCost:UsePriceEscalation" has 18 fuel types including  ElectricityXXX (Purchased, Produced, SurplusSold, Net)
    EXPECT_EQ(numResources, enum_values.size());
    std::string idf_objects = delimited_string({
        "LifeCycleCost:Parameters,",
        "  TypicalLCC,              !- Name",
        "  EndOfYear,               !- Discounting Convention",
        "  ConstantDollar,          !- Inflation Approach",
        "  0.03,                    !- Real Discount Rate",
        "  ,                        !- Nominal Discount Rate",
        "  ,                        !- Inflation",
        "  January,                 !- Base Date Month",
        "  2012,                    !- Base Date Year",
        "  January,                 !- Service Date Month",
        "  2014,                    !- Service Date Year",
        "  100,                     !- Length of Study Period in Years",
        "  0,                       !- Tax rate",
        "  ;                        !- Depreciation Method",
    });
    // All should be valid resources
    for (const auto &enum_value : enum_values) {
        const std::string enum_string = Util::makeUPPER(enum_value.get<std::string>());

        const auto resource = static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, enum_string));
        // WHY IS COMPARE ENUMS THIS WAY?
        EXPECT_ENUM_NE(Constant::eResource::Invalid, resource);

        idf_objects += fmt::format(R"idf(
LifeCycleCost:UsePriceEscalation,
  LCCUsePriceEscalation {0},             !- Name
  {0},                                   !- Resource
  2009,                                   !- Escalation Start Year
  January,                                !- Escalation Start Month
  1,                                      !- Year Escalation 1
  1.01,                                   !- Year Escalation 2
  1.02;                                   !- Year Escalation 3

LifeCycleCost:UseAdjustment,
  LCCUseAdjustment {0},              !- Name
  {0},                               !- Resource
  1,                                      !- Year Multiplier 1
  1.005,                                  !- Year Multiplier 2
  1.01;                                   !- Year Multiplier 3
  )idf",
                                   enum_string);
    }
    ASSERT_TRUE(process_idf(idf_objects));

    GetInputForLifeCycleCost(*state);

    EXPECT_EQ(numResources, state->dataEconLifeCycleCost->numUsePriceEscalation);
    for (const auto &lcc : state->dataEconLifeCycleCost->UsePriceEscalation) {
        EXPECT_ENUM_NE(lcc.resource, Constant::eResource::Invalid);
    }
    EXPECT_EQ(numResources, state->dataEconLifeCycleCost->numUseAdjustment);
    for (const auto &lcc : state->dataEconLifeCycleCost->UseAdjustment) {
        EXPECT_ENUM_NE(lcc.resource, Constant::eResource::Invalid);
    }
}
