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
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EconomicTariff.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"
#include "Fixtures/SQLiteFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::EconomicTariff;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::OutputReportPredefined;

TEST_F(EnergyPlusFixture, EconomicTariff_GetInput_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleFmc,              !- Name                                        ",
        "    ElectricityPurchased:Facility,  !- Output Meter Name                    ",
        "    kWh,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    TimeOfDaySchedule-Fmc,   !- Time of Use Period Schedule Name            ",
        "    TwoSeasonSchedule-Fmc,   !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    37.75;                   !- Monthly Charge or Variable Name             ",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    SummerOnPeak,            !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    peakEnergy,              !- Source Variable                             ",
        "    summer,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    0.14009;                 !- Cost per Unit Value or Variable Name        ",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    SummerOffPeak,           !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    offPeakEnergy,           !- Source Variable                             ",
        "    summer,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    0.06312;                 !- Cost per Unit Value or Variable Name        ",
        "                                                                            ",
        "  UtilityCost:Charge:Block,                                                 ",
        "    WinterOnPeak,            !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    peakEnergy,              !- Source Variable                             ",
        "    winter,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    ,                        !- Remaining Into Variable                     ",
        "    ,                        !- Block Size Multiplier Value or Variable Name",
        "    650,                     !- Block Size 1 Value or Variable Name         ",
        "    0.04385,                 !- Block 1 Cost per Unit Value or Variable Name",
        "    350,                     !- Block Size 2 Value or Variable Name         ",
        "    0.03763,                 !- Block 2 Cost per Unit Value or Variable Name",
        "    remaining,               !- Block Size 3 Value or Variable Name         ",
        "    0.03704;                 !- Block 3 Cost per Unit Value or Variable Name",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    WinterOffPeak,           !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    offPeakEnergy,           !- Source Variable                             ",
        "    winter,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    0.02420;                 !- Cost per Unit Value or Variable Name        ",
        "                                                                            ",
        "  UtilityCost:Qualify,                                                      ",
        "    MinDemand,               !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    TotalDemand,             !- Variable Name                               ",
        "    Minimum,                 !- Qualify Type                                ",
        "    12,                      !- Threshold Value or Variable Name            ",
        "    Annual,                  !- Season                                      ",
        "    Count,                   !- Threshold Test                              ",
        "    2;                       !- Number of Months                            ",
        "                                                                            ",
        "  UtilityCost:Computation,                                                  ",
        "    ManualExample,           !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    SumEneCharges SUM SUMMERONPEAK SUMMEROFFPEAK,  !- Compute Step 1        ",
        "    WinEneCharges SUM WINTERONPEAK WINTEROFFPEAK,  !- Compute Step 2        ",
        "    EnergyCharges SUM SumEneCharges WinEneCharges,  !- Compute Step 3       ",
        "    Basis SUM EnergyCharges DemandCharges ServiceCharges,  !- Compute Step 4",
        "    Subtotal SUM Basis Adjustment Surcharge,  !- Compute Step 5             ",
        "    Total SUM Subtotal Taxes;!- Compute Step 6                              ",
        "                                                                            ",
        "  UtilityCost:Ratchet,                                                      ",
        "    BillingDemand1,          !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    TotalDemand,             !- Baseline Source Variable                    ",
        "    TotalDemand,             !- Adjustment Source Variable                  ",
        "    Summer,                  !- Season From                                 ",
        "    Annual,                  !- Season To                                   ",
        "    0.80,                    !- Multiplier Value or Variable Name           ",
        "    0;                       !- Offset Value or Variable Name               ",
        "                                                                            ",
        "  Schedule:Compact,                                                         ",
        "    TwoSeasonSchedule-Fmc,   !- Name                                        ",
        "    number,                  !- Schedule Type Limits Name                   ",
        "    Through: 5/31,           !- Field 1                                     ",
        "    For: AllDays,            !- Field 2                                     ",
        "    Until: 24:00,1,          !- Field 3                                     ",
        "    Through: 9/30,           !- Field 5                                     ",
        "    For: AllDays,            !- Field 6                                     ",
        "    Until: 24:00,3,          !- Field 7                                     ",
        "    Through: 12/31,          !- Field 9                                     ",
        "    For: AllDays,            !- Field 10                                    ",
        "    Until: 24:00,1;          !- Field 11                                    ",
        "                                                                            ",
        "  Schedule:Compact,                                                         ",
        "    TimeOfDaySchedule-Fmc,   !- Name                                        ",
        "    number,                  !- Schedule Type Limits Name                   ",
        "    Through: 5/31,           !- Field 1                                     ",
        "    For: AllDays,            !- Field 2                                     ",
        "    Until: 15:00,3,          !- Field 3                                     ",
        "    Until: 22:00,1,          !- Field 5                                     ",
        "    Until: 24:00,3,          !- Field 7                                     ",
        "    Through: 9/30,           !- Field 9                                     ",
        "    For: AllDays,            !- Field 10                                    ",
        "    Until: 10:00,3,          !- Field 11                                    ",
        "    Until: 19:00,1,          !- Field 13                                    ",
        "    Until: 24:00,3,          !- Field 15                                    ",
        "    Through: 12/31,          !- Field 17                                    ",
        "    For: AllDays,            !- Field 18                                    ",
        "    Until: 15:00,3,          !- Field 19                                    ",
        "    Until: 22:00,1,          !- Field 21                                    ",
        "    Until: 24:00,3;          !- Field 23                                    ",
        "                                                                            ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    UpdateUtilityBills(*state);

    // tariff
    EXPECT_EQ(1, state->dataEconTariff->numTariff);
    EXPECT_EQ("EXAMPLEFMC", state->dataEconTariff->tariff(1).tariffName);
    EXPECT_ENUM_EQ(EconConv::KWH, state->dataEconTariff->tariff(1).convChoice);
    EXPECT_EQ(37.75, state->dataEconTariff->tariff(1).monthChgVal);

    // qualify
    EXPECT_EQ(1, state->dataEconTariff->numQualify);
    EXPECT_FALSE(state->dataEconTariff->qualify(1).isMaximum);
    EXPECT_EQ(12, state->dataEconTariff->qualify(1).thresholdVal);
    EXPECT_ENUM_EQ(Season::Annual, state->dataEconTariff->qualify(1).season);
    EXPECT_FALSE(state->dataEconTariff->qualify(1).isConsecutive);
    EXPECT_EQ(2, state->dataEconTariff->qualify(1).numberOfMonths);

    // ChargeSimple
    EXPECT_EQ(3, state->dataEconTariff->numChargeSimple);
    EXPECT_ENUM_EQ(Season::Winter, state->dataEconTariff->chargeSimple(3).season);
    EXPECT_EQ(0.02420, state->dataEconTariff->chargeSimple(3).costPerVal);

    // ChargeBlock
    EXPECT_EQ(1, state->dataEconTariff->numChargeBlock);
    EXPECT_ENUM_EQ(Season::Winter, state->dataEconTariff->chargeBlock(1).season);
    EXPECT_EQ(3, state->dataEconTariff->chargeBlock(1).numBlk);
    EXPECT_EQ(350, state->dataEconTariff->chargeBlock(1).blkSzVal(2));
    EXPECT_EQ(0.03763, state->dataEconTariff->chargeBlock(1).blkCostVal(2));

    // Ratchet
    EXPECT_EQ(1, state->dataEconTariff->numRatchet);
    EXPECT_ENUM_EQ(Season::Summer, state->dataEconTariff->ratchet(1).seasonFrom);
    EXPECT_ENUM_EQ(Season::Annual, state->dataEconTariff->ratchet(1).seasonTo);
    EXPECT_EQ(0.80, state->dataEconTariff->ratchet(1).multiplierVal);
    EXPECT_EQ(0.0, state->dataEconTariff->ratchet(1).offsetVal);

    // Computation
    EXPECT_EQ(1, state->dataEconTariff->numComputation);
}

// /** Test that if a meter is a water meter, and no conversion choice is give, it defaults to m3 **/
TEST_F(EnergyPlusFixture, EconomicTariff_Water_DefaultConv_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleWaterTariff,      !- Name                                        ",
        "    Water:Facility,          !- Output Meter Name                           ",
        "    ,                        !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    FlatWaterChargePerm3,    !- Name                                        ",
        "    ExampleWaterTariff,      !- Tariff Name                                 ",
        "    totalEnergy,             !- Source Variable                             ",
        "    Annual,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    3.3076;                  !- Cost per Unit Value or Variable Name        ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Create a water meter, can't use AddMeter because we would need to create a variable for that
    OutputProcessor::Meter *meter = new OutputProcessor::Meter("WATER:FACILITY");
    meter->resource = Constant::eResource::Water;
    state->dataOutputProcessor->meters.push_back(meter);
    state->dataOutputProcessor->meterMap.insert_or_assign("WATER:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    UpdateUtilityBills(*state);

    // tariff
    EXPECT_EQ(1, state->dataEconTariff->numTariff);
    EXPECT_EQ("EXAMPLEWATERTARIFF", state->dataEconTariff->tariff(1).tariffName);

    // Check that it correctly assesses the meter type
    EXPECT_ENUM_EQ(EconomicTariff::MeterType::Water, state->dataEconTariff->tariff(1).kindMtr);

    // Check that if defaults the conversion choice correctly
    EXPECT_ENUM_EQ(EconConv::M3, state->dataEconTariff->tariff(1).convChoice);
    EXPECT_EQ(1, state->dataEconTariff->tariff(1).energyConv);
    EXPECT_EQ(3600, state->dataEconTariff->tariff(1).demandConv);
    EXPECT_EQ(10, state->dataEconTariff->tariff(1).monthChgVal);
}

// /** Test that if a meter is a water meter, and CCF is used, it uses the right conversion (not the gas one) **/
TEST_F(EnergyPlusFixture, EconomicTariff_Water_CCF_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleWaterTariff,      !- Name                                        ",
        "    Water:Facility,          !- Output Meter Name                           ",
        "    CCF,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Create a water meter
    OutputProcessor::Meter *meter = new Meter("WATER:FACILITY");
    state->dataOutputProcessor->meters.push_back(meter);
    meter->resource = Constant::eResource::Water;
    state->dataOutputProcessor->meterMap.insert_or_assign("WATER:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    UpdateUtilityBills(*state);

    // tariff
    EXPECT_EQ(1, state->dataEconTariff->numTariff);

    // Check that it correctly assesses the meter type (water)
    EXPECT_ENUM_EQ(EconomicTariff::MeterType::Water, state->dataEconTariff->tariff(1).kindMtr);

    // Check conversion choice
    EXPECT_ENUM_EQ(EconConv::CCF, state->dataEconTariff->tariff(1).convChoice);
    ASSERT_DOUBLE_EQ(0.35314666721488586, state->dataEconTariff->tariff(1).energyConv);
}

// /** Test that if a meter is a gas meter, and CCF is used, it uses the right conversion (not the water one) **/
TEST_F(EnergyPlusFixture, EconomicTariff_Gas_CCF_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleTariff,           !- Name                                        ",
        "    NaturalGas:Facility,     !- Output Meter Name                           ",
        "    CCF,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Create a water meter
    OutputProcessor::Meter *meter = new Meter("NATURALGAS:FACILITY");
    state->dataOutputProcessor->meters.push_back(meter);
    meter->resource = Constant::eResource::NaturalGas;
    state->dataOutputProcessor->meterMap.insert_or_assign("NATURALGAS:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    UpdateUtilityBills(*state);
    ;

    // tariff
    EXPECT_EQ(1, state->dataEconTariff->numTariff);

    // Check that it correctly assesses the meter type (gas)
    EXPECT_ENUM_EQ(EconomicTariff::MeterType::Gas, state->dataEconTariff->tariff(1).kindMtr);

    // Check conversion choice

    EXPECT_ENUM_EQ(EconConv::CCF, state->dataEconTariff->tariff(1).convChoice);
    ASSERT_DOUBLE_EQ(9.4781712e-9, state->dataEconTariff->tariff(1).energyConv);
}

// /** Test that if a meter is an Electric meter, and CCF is used, it still defaults to kWh (not allowed) **/
TEST_F(EnergyPlusFixture, EconomicTariff_Electric_CCF_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleTariff,           !- Name                                        ",
        "    Electricity:Facility,    !- Output Meter Name                           ",
        "    CCF,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Create a water meter
    OutputProcessor::Meter *meter = new Meter("ELECTRICITY:FACILITY");
    state->dataOutputProcessor->meters.push_back(meter);
    meter->resource = Constant::eResource::Electricity;
    state->dataOutputProcessor->meterMap.insert_or_assign("ELECTRICITY:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    UpdateUtilityBills(*state);
    ;

    // tariff
    EXPECT_EQ(1, state->dataEconTariff->numTariff);

    // Check that it correctly assesses the meter type (electricity, and electric simple in particular)
    EXPECT_EQ(EconomicTariff::MeterType::ElecSimple, state->dataEconTariff->tariff(1).kindMtr);

    // Check conversion choice, should force back to kWh
    EXPECT_ENUM_EQ(EconConv::KWH, state->dataEconTariff->tariff(1).convChoice);
    ASSERT_DOUBLE_EQ(0.0000002778, state->dataEconTariff->tariff(1).energyConv);
    ASSERT_DOUBLE_EQ(0.001, state->dataEconTariff->tariff(1).demandConv);
}

TEST_F(EnergyPlusFixture, EconomicTariff_LEEDtariffReporting_Test)
{
    state->init_state(*state);
    state->dataOutputProcessor->meters.push_back(new Meter("ELECTRICITY:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("ELECTRICITY:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("NATURALGAS:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("NATURALGAS:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("DISTRICTCOOLING:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("DISTRICTCOOLING:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("DISTRICTHEATINGWATER:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("DISTRICTHEATINGWATER:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    state->dataEconTariff->numTariff = 4;
    state->dataEconTariff->tariff.allocate(state->dataEconTariff->numTariff);
    state->dataEconTariff->tariff(1).tariffName = "SecondaryGeneralUnit";
    state->dataEconTariff->tariff(1).isSelected = true;
    state->dataEconTariff->tariff(1).totalAnnualCost = 4151.45;
    state->dataEconTariff->tariff(1).totalAnnualEnergy = 4855.21;
    state->dataEconTariff->tariff(1).kindMtr = EconomicTariff::MeterType::ElecPurchased;
    state->dataEconTariff->tariff(1).reportMeterIndx = GetMeterIndex(*state, "ELECTRICITY:FACILITY");
    state->dataEconTariff->tariff(1).demandWindow = DemandWindow::Day;

    state->dataEconTariff->tariff(2).tariffName = "SmallCGUnit";
    state->dataEconTariff->tariff(2).isSelected = true;
    state->dataEconTariff->tariff(2).totalAnnualCost = 415.56;
    state->dataEconTariff->tariff(2).totalAnnualEnergy = 0.00;
    state->dataEconTariff->tariff(2).kindMtr = EconomicTariff::MeterType::Gas;
    state->dataEconTariff->tariff(2).reportMeterIndx = GetMeterIndex(*state, "NATURALGAS:FACILITY");
    state->dataEconTariff->tariff(2).demandWindow = DemandWindow::Day;

    state->dataEconTariff->tariff(3).tariffName = "DistrictCoolingUnit";
    state->dataEconTariff->tariff(3).isSelected = true;
    state->dataEconTariff->tariff(3).totalAnnualCost = 55.22;
    state->dataEconTariff->tariff(3).totalAnnualEnergy = 8.64;
    state->dataEconTariff->tariff(3).kindMtr = EconomicTariff::MeterType::Other;
    state->dataEconTariff->tariff(3).reportMeterIndx = GetMeterIndex(*state, "DISTRICTCOOLING:FACILITY");
    state->dataEconTariff->tariff(3).demandWindow = DemandWindow::Day;

    state->dataEconTariff->tariff(4).tariffName = "DistrictHeatingUnit";
    state->dataEconTariff->tariff(4).isSelected = true;
    state->dataEconTariff->tariff(4).totalAnnualCost = 15.98;
    state->dataEconTariff->tariff(4).totalAnnualEnergy = 1.47;
    state->dataEconTariff->tariff(4).kindMtr = EconomicTariff::MeterType::Other;
    state->dataEconTariff->tariff(4).reportMeterIndx = GetMeterIndex(*state, "DISTRICTHEATINGWATER:FACILITY");
    state->dataEconTariff->tariff(4).demandWindow = DemandWindow::Day;

    for (auto &tariff : state->dataEconTariff->tariff) {
        tariff.demandWindow = EconomicTariff::DemandWindow::Hour;
    }

    SetPredefinedTables(*state); // need to setup the predefined table entry numbers

    LEEDtariffReporting(*state);

    EXPECT_EQ("SecondaryGeneralUnit", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "Electricity"));
    EXPECT_EQ("SmallCGUnit", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "Natural Gas"));
    EXPECT_EQ("DistrictCoolingUnit", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "District Cooling"));
    EXPECT_EQ("DistrictHeatingUnit", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "District Heating Water"));

    EXPECT_EQ("0.855", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "Electricity"));
    EXPECT_EQ("6.391", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "District Cooling"));
    EXPECT_EQ("10.871", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "District Heating Water"));
}

TEST_F(EnergyPlusFixture, EconomicTariff_GatherForEconomics)
{
    // Test for #7814 to ensure that Tariff seasons are calculated properly

    std::string const idf_objects = delimited_string({

        "RunPeriodControl:DaylightSavingTime,",
        "  2nd Sunday in March,     !- Start Date",
        "  1st Sunday in November;  !- End Date",

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  YES;                     !- Run Simulation for Weather File Run Periods",

        "Building,",
        "  Mid-Rise Apartment,      !- Name",
        "  0,                       !- North Axis {deg}",
        "  City,                    !- Terrain",
        "  0.04,                    !- Loads Convergence Tolerance Value",
        "  0.4,                     !- Temperature Convergence Tolerance Value {deltaC}",
        "  FullExterior,            !- Solar Distribution",
        "  25,                      !- Maximum Number of Warmup Days",
        "  6;                       !- Minimum Number of Warmup Days",

        "Timestep,",
        "  4;                       !- Number of Timesteps per Hour",

        "RunPeriod,",
        "  Annual,                  !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  12,                      !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Sunday,                  !- Day of Week for Start Day",
        "  No,                      !- Use Weather File Holidays and Special Days",
        "  No,                      !- Use Weather File Daylight Saving Period",
        "  Yes,                     !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "GlobalGeometryRules,",
        "  LowerLeftCorner,         !- Starting Vertex Position",
        "  Clockwise,               !- Vertex Entry Direction",
        "  Relative;                !- Coordinate System",

        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",

        "Schedule:Constant,",
        "  Always On Discrete,      !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  1;                       !- Hourly Value",

        "Exterior:Lights,",
        "  Exterior Facade Lighting,!- Name",
        "  Always On Discrete,      !- Schedule Name",
        "  1000.00,                 !- Design Level {W}",
        "  ScheduleNameOnly,        !- Control Option",
        "  Exterior Facade Lighting;!- End-Use Subcategory",

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  Seasonal_Tariff,         !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Charge:Simple,",
        "  Seasonal_Tariff_Winter_Charge, !- Utility Cost Charge Simple Name",
        "  Seasonal_Tariff,         !- Tariff Name",
        "  totalEnergy,             !- Source Variable",
        "  Winter,                  !- Season",
        "  EnergyCharges,           !- Category Variable Name",
        "  0.02;                    !- Cost per Unit Value or Variable Name",

        "UtilityCost:Charge:Simple,",
        "  Seasonal_Tariff_Summer_Charge, !- Utility Cost Charge Simple Name",
        "  Seasonal_Tariff,         !- Tariff Name",
        "  totalEnergy,             !- Source Variable",
        "  Summer,                  !- Season",
        "  EnergyCharges,           !- Category Variable Name",
        "  0.04;                    !- Cost per Unit Value or Variable Name",

        "Output:Table:SummaryReports,",
        "  TariffReport;            !- Report 1 Name",

        "OutputControl:Table:Style,",
        "  HTML;                                   !- Column Separator",

        "Output:SQLite,",
        "  SimpleAndTabular;                       !- Option Type",

        "Output:Meter,Electricity:Facility,timestep;"

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;

    state->init_state(*state);
    ExteriorEnergyUse::ManageExteriorEnergyUse(*state);
    EXPECT_EQ(1, state->dataExteriorEnergyUse->NumExteriorLights);
    EXPECT_EQ(1000, state->dataExteriorEnergyUse->ExteriorLights(1).DesignLevel);

    // This will only do the get input routines
    EconomicTariff::UpdateUtilityBills(*state);
    ;

    // tariff
    EXPECT_EQ(1, state->dataEconTariff->numTariff);
    EXPECT_EQ("SEASONAL_TARIFF", state->dataEconTariff->tariff(1).tariffName);
    EXPECT_ENUM_EQ(EconConv::KWH, state->dataEconTariff->tariff(1).convChoice);
    EXPECT_EQ(0, state->dataEconTariff->tariff(1).monthChgVal);
    EXPECT_EQ("ELECTRICITY SEASON SCHEDULE", state->dataEconTariff->tariff(1).seasonSched->Name);

    // Two Simple Charges
    EXPECT_EQ(2, state->dataEconTariff->numChargeSimple);

    EXPECT_ENUM_EQ(Season::Winter, state->dataEconTariff->chargeSimple(1).season);
    EXPECT_EQ(0.02, state->dataEconTariff->chargeSimple(1).costPerVal);

    EXPECT_ENUM_EQ(Season::Summer, state->dataEconTariff->chargeSimple(2).season);
    EXPECT_EQ(0.04, state->dataEconTariff->chargeSimple(2).costPerVal);

    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // fake a weather run

    // Unitialized: default initialized to 0
    EXPECT_ENUM_EQ(Season::Invalid, state->dataEconTariff->tariff(1).seasonForMonth(5));
    EXPECT_ENUM_EQ(Season::Invalid, state->dataEconTariff->tariff(1).seasonForMonth(6));

    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 23;
    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    state->dataEnvrn->MonthTomorrow = 6;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 4;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(1.0, Sched::GetSchedule(*state, "ALWAYS ON DISCRETE")->getHrTsVal(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep));
    EXPECT_EQ(1.0, state->dataEconTariff->tariff(1).seasonSched->getCurrentVal());

    ExteriorEnergyUse::ManageExteriorEnergyUse(*state);

    EXPECT_EQ(1000.0, state->dataExteriorEnergyUse->ExteriorLights(1).Power);
    EXPECT_EQ(state->dataExteriorEnergyUse->ExteriorLights(1).Power * state->dataGlobal->TimeStepZoneSec,
              state->dataExteriorEnergyUse->ExteriorLights(1).CurrentUse);

    EXPECT_EQ(0, state->dataEconTariff->tariff(1).gatherEnergy(state->dataEnvrn->Month)[(int)Period::Peak]);

    // This Should now call GatherForEconomics
    state->dataGlobal->DoOutputReporting = true;
    EconomicTariff::UpdateUtilityBills(*state);
    ;
    EXPECT_ENUM_EQ(Season::Winter, state->dataEconTariff->tariff(1).seasonForMonth(5));
    EXPECT_ENUM_EQ(Season::Invalid, state->dataEconTariff->tariff(1).seasonForMonth(6));

    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 24;
    state->dataEnvrn->DSTIndicator = 1; // DST IS ON
    state->dataEnvrn->MonthTomorrow = 6;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    Sched::UpdateScheduleVals(*state);
    EXPECT_EQ(3.0, state->dataEconTariff->tariff(1).seasonSched->getCurrentVal());

    ExteriorEnergyUse::ManageExteriorEnergyUse(*state);

    EXPECT_EQ(1000.0, state->dataExteriorEnergyUse->ExteriorLights(1).Power);
    EXPECT_EQ(state->dataExteriorEnergyUse->ExteriorLights(1).Power * state->dataGlobal->TimeStepZoneSec,
              state->dataExteriorEnergyUse->ExteriorLights(1).CurrentUse);

    // This Should now call GatherForEconomics
    EconomicTariff::UpdateUtilityBills(*state);
    ;
    EXPECT_ENUM_EQ(Season::Winter, state->dataEconTariff->tariff(1).seasonForMonth(5));
    EXPECT_ENUM_EQ(Season::Summer, state->dataEconTariff->tariff(1).seasonForMonth(6));
}

TEST_F(EnergyPlusFixture, EconomicTariff_GatherForEconomics_ZeroMeterIndex)
{
    state->init_state(*state);
    // Test for PR #10521 and Issue #10519
    state->dataEconTariff->numTariff = 1;
    state->dataEconTariff->tariff.allocate(state->dataEconTariff->numTariff);
    state->dataEconTariff->tariff(1).reportMeterIndx = 0;
    state->dataEconTariff->tariff(1).demWinTime = 1.;
    state->dataEconTariff->tariff(1).energyConv = 1.;

    state->dataGlobal->TimeStepZoneSec = 3600;
    state->dataEnvrn->Month = 1;

    Meter *meter = nullptr;
    meter = new Meter("FacElec");
    meter->CurTSValue = 100;
    state->dataOutputProcessor->meters.push_back(meter);

    GatherForEconomics(*state);
    EXPECT_EQ(100, state->dataEconTariff->tariff(1).gatherEnergy(1)[1]);
}

TEST_F(EnergyPlusFixture, EconomicTariff_PushPopStack)
{
    state->init_state(*state);
    state->dataEconTariff->numTariff = 1;
    state->dataEconTariff->tariff.allocate(state->dataEconTariff->numTariff);

    Array1D<Real64> aMonths(NumMonths);
    int aVarPt;

    Array1D<Real64> bMonths(NumMonths);
    int bVarPt;

    Array1D<Real64> cMonths(NumMonths);
    int cVarPt;

    aMonths = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
    aVarPt = 0;

    pushStack(*state, aMonths, aVarPt);

    EXPECT_EQ(1, state->dataEconTariff->topOfStack);
    EXPECT_EQ(0, state->dataEconTariff->stack(1).varPt);
    EXPECT_EQ(aMonths(1), state->dataEconTariff->stack(1).values(1));
    EXPECT_EQ(aMonths(2), state->dataEconTariff->stack(1).values(2));
    EXPECT_EQ(aMonths(3), state->dataEconTariff->stack(1).values(3));
    EXPECT_EQ(aMonths(4), state->dataEconTariff->stack(1).values(4));
    EXPECT_EQ(aMonths(5), state->dataEconTariff->stack(1).values(5));
    EXPECT_EQ(aMonths(6), state->dataEconTariff->stack(1).values(6));

    bMonths = {2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6};
    bVarPt = 0;

    pushStack(*state, bMonths, bVarPt);

    EXPECT_EQ(2, state->dataEconTariff->topOfStack);
    EXPECT_EQ(0, state->dataEconTariff->stack(1).varPt);
    EXPECT_EQ(aMonths(1), state->dataEconTariff->stack(1).values(1));
    EXPECT_EQ(aMonths(2), state->dataEconTariff->stack(1).values(2));
    EXPECT_EQ(aMonths(3), state->dataEconTariff->stack(1).values(3));
    EXPECT_EQ(aMonths(4), state->dataEconTariff->stack(1).values(4));
    EXPECT_EQ(aMonths(5), state->dataEconTariff->stack(1).values(5));
    EXPECT_EQ(aMonths(6), state->dataEconTariff->stack(1).values(6));
    EXPECT_EQ(0, state->dataEconTariff->stack(2).varPt);
    EXPECT_EQ(bMonths(1), state->dataEconTariff->stack(2).values(1));
    EXPECT_EQ(bMonths(2), state->dataEconTariff->stack(2).values(2));
    EXPECT_EQ(bMonths(3), state->dataEconTariff->stack(2).values(3));
    EXPECT_EQ(bMonths(4), state->dataEconTariff->stack(2).values(4));
    EXPECT_EQ(bMonths(5), state->dataEconTariff->stack(2).values(5));
    EXPECT_EQ(bMonths(6), state->dataEconTariff->stack(2).values(6));

    popStack(*state, cMonths, cVarPt);
    EXPECT_EQ(1, state->dataEconTariff->topOfStack);
    EXPECT_EQ(0, state->dataEconTariff->stack(1).varPt);
    EXPECT_EQ(aMonths(1), state->dataEconTariff->stack(1).values(1));
    EXPECT_EQ(aMonths(2), state->dataEconTariff->stack(1).values(2));
    EXPECT_EQ(aMonths(3), state->dataEconTariff->stack(1).values(3));
    EXPECT_EQ(aMonths(4), state->dataEconTariff->stack(1).values(4));
    EXPECT_EQ(aMonths(5), state->dataEconTariff->stack(1).values(5));
    EXPECT_EQ(aMonths(6), state->dataEconTariff->stack(1).values(6));
    EXPECT_EQ(bMonths(1), cMonths(1));
    EXPECT_EQ(bMonths(2), cMonths(2));
    EXPECT_EQ(bMonths(3), cMonths(3));
    EXPECT_EQ(bMonths(4), cMonths(4));
    EXPECT_EQ(bMonths(5), cMonths(5));
    EXPECT_EQ(bMonths(6), cMonths(6));
    EXPECT_EQ(bVarPt, cVarPt);

    popStack(*state, cMonths, cVarPt);
    EXPECT_EQ(0, state->dataEconTariff->topOfStack);
    EXPECT_EQ(aMonths(1), cMonths(1));
    EXPECT_EQ(aMonths(2), cMonths(2));
    EXPECT_EQ(aMonths(3), cMonths(3));
    EXPECT_EQ(aMonths(4), cMonths(4));
    EXPECT_EQ(aMonths(5), cMonths(5));
    EXPECT_EQ(aMonths(6), cMonths(6));
    EXPECT_EQ(aVarPt, cVarPt);

    state->dataEconTariff->econVar.allocate(10);
    state->dataEconTariff->econVar(1).isEvaluated = false;
    state->dataEconTariff->econVar(1).kindOfObj = ObjType::Variable;

    // test getting econ variable
    Array1D<Real64> dMonths(NumMonths);
    int dVarPt;

    dMonths = {3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6};
    dVarPt = 1;

    state->dataEconTariff->econVar(dVarPt).values = dMonths;

    pushStack(*state, aMonths, dVarPt); // note the aMonths should not be used
    popStack(*state, cMonths, cVarPt);
    EXPECT_EQ(0, state->dataEconTariff->topOfStack);
    EXPECT_EQ(dMonths(1), cMonths(1));
    EXPECT_EQ(dMonths(2), cMonths(2));
    EXPECT_EQ(dMonths(3), cMonths(3));
    EXPECT_EQ(dMonths(4), cMonths(4));
    EXPECT_EQ(dMonths(5), cMonths(5));
    EXPECT_EQ(dMonths(6), cMonths(6));
    EXPECT_EQ(dVarPt, cVarPt);
}

TEST_F(EnergyPlusFixture, EconomicTariff_evaluateChargeSimple)
{
    state->init_state(*state);
    int curTariff = 6;
    state->dataEconTariff->tariff.allocate(curTariff);

    int curEconVar = 7;
    state->dataEconTariff->econVar.allocate(curEconVar);
    state->dataEconTariff->econVar(curEconVar).tariffIndx = curTariff;
    state->dataEconTariff->econVar(curEconVar).kindOfObj = ObjType::Variable;
    Array1D<Real64> results(NumMonths);

    int sourceEconVar = 4;
    Array1D<Real64> sourceMonths(NumMonths);
    sourceMonths = {310, 320, 330, 340, 350, 360, 360, 350, 340, 330, 320, 310};
    state->dataEconTariff->econVar(sourceEconVar).values = sourceMonths;

    int costPerEconVar = 6;
    Array1D<Real64> costPerMonths(NumMonths);
    costPerMonths = {0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.1, 0.1, 0.1, 0.1};
    state->dataEconTariff->econVar(costPerEconVar).values = costPerMonths;

    int curSimpChg = 3;
    state->dataEconTariff->econVar(curEconVar).index = curSimpChg;
    state->dataEconTariff->chargeSimple.allocate(curSimpChg);
    state->dataEconTariff->chargeSimple(curSimpChg).namePt = curEconVar;
    state->dataEconTariff->chargeSimple(curSimpChg).tariffIndx = curTariff;
    state->dataEconTariff->chargeSimple(curSimpChg).sourcePt = sourceEconVar;
    state->dataEconTariff->chargeSimple(curSimpChg).costPerPt = costPerEconVar;
    state->dataEconTariff->chargeSimple(curSimpChg).season = Season::Annual;

    evaluateChargeSimple(*state, curEconVar);
    results = state->dataEconTariff->econVar(curEconVar).values;

    EXPECT_NEAR(results(1), 310 * 0.1, 0.01);
    EXPECT_NEAR(results(2), 320 * 0.1, 0.01);
    EXPECT_NEAR(results(3), 330 * 0.1, 0.01);
    EXPECT_NEAR(results(4), 340 * 0.2, 0.01);
    EXPECT_NEAR(results(5), 350 * 0.2, 0.01);
    EXPECT_NEAR(results(6), 360 * 0.2, 0.01);
    EXPECT_NEAR(results(7), 360 * 0.2, 0.01);
    EXPECT_NEAR(results(8), 350 * 0.2, 0.01);
    EXPECT_NEAR(results(9), 340 * 0.1, 0.01);
    EXPECT_NEAR(results(10), 330 * 0.1, 0.01);
    EXPECT_NEAR(results(11), 320 * 0.1, 0.01);
    EXPECT_NEAR(results(12), 310 * 0.1, 0.01);

    state->dataEconTariff->chargeSimple(curSimpChg).costPerPt = 0;
    state->dataEconTariff->chargeSimple(curSimpChg).costPerVal = 0.15;

    evaluateChargeSimple(*state, curEconVar);
    results = state->dataEconTariff->econVar(curEconVar).values;

    EXPECT_NEAR(results(1), 310 * 0.15, 0.01);
    EXPECT_NEAR(results(2), 320 * 0.15, 0.01);
    EXPECT_NEAR(results(3), 330 * 0.15, 0.01);
    EXPECT_NEAR(results(4), 340 * 0.15, 0.01);
    EXPECT_NEAR(results(5), 350 * 0.15, 0.01);
    EXPECT_NEAR(results(6), 360 * 0.15, 0.01);
    EXPECT_NEAR(results(7), 360 * 0.15, 0.01);
    EXPECT_NEAR(results(8), 350 * 0.15, 0.01);
    EXPECT_NEAR(results(9), 340 * 0.15, 0.01);
    EXPECT_NEAR(results(10), 330 * 0.15, 0.01);
    EXPECT_NEAR(results(11), 320 * 0.15, 0.01);
    EXPECT_NEAR(results(12), 310 * 0.15, 0.01);

    state->dataEconTariff->chargeSimple(curSimpChg).season = Season::Summer;
    int summerEconVar = 2;
    Array1D<Real64> summerMonths(NumMonths);
    costPerMonths = {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0};
    state->dataEconTariff->econVar(summerEconVar).values = costPerMonths;
    state->dataEconTariff->tariff(curTariff).natives[(int)Native::IsSummer] = summerEconVar;

    evaluateChargeSimple(*state, curEconVar);
    results = state->dataEconTariff->econVar(curEconVar).values;

    EXPECT_NEAR(results(1), 0., 0.01);
    EXPECT_NEAR(results(2), 0., 0.01);
    EXPECT_NEAR(results(3), 0., 0.01);
    EXPECT_NEAR(results(4), 0., 0.01);
    EXPECT_NEAR(results(5), 350 * 0.15, 0.01);
    EXPECT_NEAR(results(6), 360 * 0.15, 0.01);
    EXPECT_NEAR(results(7), 360 * 0.15, 0.01);
    EXPECT_NEAR(results(8), 350 * 0.15, 0.01);
    EXPECT_NEAR(results(9), 340 * 0.15, 0.01);
    EXPECT_NEAR(results(10), 0., 0.01);
    EXPECT_NEAR(results(11), 0., 0.01);
    EXPECT_NEAR(results(12), 0., 0.01);
}

TEST_F(EnergyPlusFixture, EconomicTariff_evaluateChargeBlock)
{
    state->init_state(*state);
    int curTariff = 8;
    state->dataEconTariff->tariff.allocate(curTariff);

    int curEconVar = 11;
    state->dataEconTariff->econVar.allocate(curEconVar);
    state->dataEconTariff->econVar(curEconVar).tariffIndx = curTariff;
    state->dataEconTariff->econVar(curEconVar).kindOfObj = ObjType::Variable;
    Array1D<Real64> results(NumMonths);

    int sourceEconVar = 4;
    Array1D<Real64> sourceMonths(NumMonths);
    sourceMonths = {450, 650, 950, 1350, 1850, 2850, 500, 1000, 1500, 501, 1001, 1501};
    state->dataEconTariff->econVar(sourceEconVar).values = sourceMonths;

    int curChgBlk = 3;
    state->dataEconTariff->econVar(curEconVar).index = curChgBlk;
    state->dataEconTariff->chargeBlock.allocate(curChgBlk);
    state->dataEconTariff->chargeBlock(curChgBlk).namePt = curEconVar;
    state->dataEconTariff->chargeBlock(curChgBlk).tariffIndx = curTariff;
    state->dataEconTariff->chargeBlock(curChgBlk).sourcePt = sourceEconVar;
    state->dataEconTariff->chargeBlock(curChgBlk).season = Season::Annual;
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzMultPt = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzMultVal = 1;

    int numBlocks = 3;
    state->dataEconTariff->chargeBlock(curChgBlk).numBlk = numBlocks;
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzPt.allocate(numBlocks);
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzVal.allocate(numBlocks);
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostPt.allocate(numBlocks);
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostVal.allocate(numBlocks);

    state->dataEconTariff->chargeBlock(curChgBlk).blkSzPt(1) = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzVal(1) = 500;
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostPt(1) = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostVal(1) = 0.15;

    state->dataEconTariff->chargeBlock(curChgBlk).blkSzPt(2) = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzVal(2) = 1000;
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostPt(2) = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostVal(2) = 0.12;

    state->dataEconTariff->chargeBlock(curChgBlk).blkSzPt(3) = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkSzVal(3) = 999999999; // a big number
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostPt(3) = 0;
    state->dataEconTariff->chargeBlock(curChgBlk).blkCostVal(3) = 0.10;

    evaluateChargeBlock(*state, curEconVar);
    results = state->dataEconTariff->econVar(curEconVar).values;

    EXPECT_NEAR(results(1), 450 * 0.15, 0.01);
    EXPECT_NEAR(results(2), 500 * 0.15 + 150 * 0.12, 0.01);
    EXPECT_NEAR(results(3), 500 * 0.15 + 450 * 0.12, 0.01);
    EXPECT_NEAR(results(4), 500 * 0.15 + 850 * 0.12, 0.01);
    EXPECT_NEAR(results(5), 500 * 0.15 + 1000 * 0.12 + 350 * 0.10, 0.01);
    EXPECT_NEAR(results(6), 500 * 0.15 + 1000 * 0.12 + 1350 * 0.10, 0.01);
    EXPECT_NEAR(results(7), 500 * 0.15, 0.01);
    EXPECT_NEAR(results(8), 500 * 0.15 + 500 * 0.12, 0.01);
    EXPECT_NEAR(results(9), 500 * 0.15 + 1000 * 0.12, 0.01);
    EXPECT_NEAR(results(10), 500 * 0.15 + 1 * 0.12, 0.01);
    EXPECT_NEAR(results(11), 500 * 0.15 + 501 * 0.12, 0.01);
    EXPECT_NEAR(results(12), 500 * 0.15 + 1000 * 0.12 + 1 * 0.10, 0.01);
}

TEST_F(EnergyPlusFixture, InputEconomics_UtilityCost_Variable_Test0)
{
    // Tests for PR #8456 and Issue #8455 ... Case 0 of Cases 0-3
    // to ensure UtilityCost:Variable inputs being procesed properly

    std::string const idf_objects = delimited_string({

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  ExampleAWithVariableMonthlyCharge,     !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Variable,",
        "VariableFixedCharge, !-Name",
        "ExampleAWithVariableMonthlyCharge, !-Tariff Name",
        "Demand, !-Variable Type",
        "1.00, !-January Value",
        "2.00, !-February Value",
        "3.00, !-March Value",
        "4.00, !-April Value",
        "5.00, !-May Value",
        "6.00, !-June Value",
        "7.00, !-July Value",
        "8.00, !-August Value",
        "9.00, !-September Value",
        "10.00, !-October Value",
        "11.00, !-November Value",
        "12.00; !-December Value"});

    bool ErrorsFound = false;

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetInputEconomicsVariable(*state, ErrorsFound);

    // Make sure the "Demand" UtilityCost:Variable is now processed correctly
    EXPECT_ENUM_EQ(state->dataEconTariff->econVar(1).varUnitType, VarUnitType::Demand);

    // Make sure the numerical inputs after A3 is still being processed correctly
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(1), 1.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(2), 2.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(3), 3.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(4), 4.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(5), 5.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(6), 6.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(7), 7.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(8), 8.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(9), 9.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(10), 10.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(11), 11.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(12), 12.00);
}

TEST_F(EnergyPlusFixture, InputEconomics_UtilityCost_Variable_Test1)
{
    // Tests for PR #8456 and Issue #8455 ... Case 1 of Cases 0-3
    // to ensure UtilityCost:Variable inputs being procesed properly

    std::string const idf_objects1 = delimited_string({

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  ExampleAWithVariableMonthlyCharge,     !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Variable,",
        "VariableFixedCharge, !-Name",
        "ExampleAWithVariableMonthlyCharge, !-Tariff Name",
        "Energy, !-Variable Type",
        "1.00, !-January Value",
        "2.00, !-February Value",
        "3.00, !-March Value",
        "4.00, !-April Value",
        "5.00, !-May Value",
        "6.00, !-June Value",
        "7.00, !-July Value",
        "8.00, !-August Value",
        "9.00, !-September Value",
        "10.00, !-October Value",
        "11.00, !-November Value",
        "12.00; !-December Value"});

    bool ErrorsFound = false;

    ASSERT_TRUE(process_idf(idf_objects1));
    state->init_state(*state);
    GetInputEconomicsVariable(*state, ErrorsFound);

    // Make sure variable input type "Energy" is processed as expected
    EXPECT_ENUM_EQ(state->dataEconTariff->econVar(1).varUnitType, VarUnitType::Energy);
    // Make sure the numerical inputs after A3 is still being processed correctly
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(1), 1.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(2), 2.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(3), 3.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(4), 4.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(5), 5.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(6), 6.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(7), 7.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(8), 8.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(9), 9.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(10), 10.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(11), 11.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(12), 12.00);
}

TEST_F(EnergyPlusFixture, InputEconomics_UtilityCost_Variable_Test2)
{
    // Tests for PR #8456 and Issue #8455 ... Case 2 of Cases 0-3
    // to ensure UtilityCost:Variable inputs being procesed properly

    std::string const idf_objects2 = delimited_string({

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  ExampleAWithVariableMonthlyCharge,     !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Variable,",
        "VariableFixedCharge, !-Name",
        "ExampleAWithVariableMonthlyCharge, !-Tariff Name",
        "Dimensionless, !-Variable Type",
        "1.00, !-January Value",
        "2.00, !-February Value",
        "3.00, !-March Value",
        "4.00, !-April Value",
        "5.00, !-May Value",
        "6.00, !-June Value",
        "7.00, !-July Value",
        "8.00, !-August Value",
        "9.00, !-September Value",
        "10.00, !-October Value",
        "11.00, !-November Value",
        "12.00; !-December Value"});

    bool ErrorsFound = false;
    ASSERT_TRUE(process_idf(idf_objects2));
    state->init_state(*state);
    GetInputEconomicsVariable(*state, ErrorsFound);

    // Make sure variable input type "Dimensionless" is processed as expected
    EXPECT_ENUM_EQ(state->dataEconTariff->econVar(1).varUnitType, VarUnitType::Dimensionless);
    // Make sure the numerical inputs after A3 is still being processed correctly
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(1), 1.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(2), 2.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(3), 3.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(4), 4.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(5), 5.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(6), 6.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(7), 7.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(8), 8.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(9), 9.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(10), 10.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(11), 11.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(12), 12.00);
}

TEST_F(EnergyPlusFixture, InputEconomics_UtilityCost_Variable_Test3)
{
    // Tests for PR #8456 and Issue #8455 ... Case 3 of Cases 0-3
    // to ensure UtilityCost:Variable inputs being procesed properly

    std::string const idf_objects3 = delimited_string({

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  ExampleAWithVariableMonthlyCharge,     !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Variable,",
        "VariableFixedCharge, !-Name",
        "ExampleAWithVariableMonthlyCharge, !-Tariff Name",
        "Currency, !-Variable Type",
        "1.00, !-January Value",
        "2.00, !-February Value",
        "3.00, !-March Value",
        "4.00, !-April Value",
        "5.00, !-May Value",
        "6.00, !-June Value",
        "7.00, !-July Value",
        "8.00, !-August Value",
        "9.00, !-September Value",
        "10.00, !-October Value",
        "11.00, !-November Value",
        "12.00; !-December Value"});

    bool ErrorsFound = false;
    ASSERT_TRUE(process_idf(idf_objects3));
    state->init_state(*state);
    GetInputEconomicsVariable(*state, ErrorsFound);

    // Make sure variable input type "Currency" is processed as expected
    EXPECT_ENUM_EQ(state->dataEconTariff->econVar(1).varUnitType, VarUnitType::Currency);
    // Make sure the numerical inputs after A3 is still being processed correctly
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(1), 1.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(2), 2.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(3), 3.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(4), 4.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(5), 5.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(6), 6.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(7), 7.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(8), 8.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(9), 9.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(10), 10.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(11), 11.00);
    EXPECT_EQ(state->dataEconTariff->econVar(1).values(12), 12.00);
}

TEST_F(SQLiteFixture, WriteEconomicTariffTable_DualUnits)
{
    std::string const idf_objects = delimited_string({

        "RunPeriodControl:DaylightSavingTime,",
        "  2nd Sunday in March,     !- Start Date",
        "  1st Sunday in November;  !- End Date",

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  YES;                     !- Run Simulation for Weather File Run Periods",

        "Building,",
        "  Mid-Rise Apartment,      !- Name",
        "  0,                       !- North Axis {deg}",
        "  City,                    !- Terrain",
        "  0.04,                    !- Loads Convergence Tolerance Value",
        "  0.4,                     !- Temperature Convergence Tolerance Value {deltaC}",
        "  FullExterior,            !- Solar Distribution",
        "  25,                      !- Maximum Number of Warmup Days",
        "  6;                       !- Minimum Number of Warmup Days",

        "Timestep,",
        "  4;                       !- Number of Timesteps per Hour",

        "RunPeriod,",
        "  Annual,                  !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  12,                      !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Sunday,                  !- Day of Week for Start Day",
        "  No,                      !- Use Weather File Holidays and Special Days",
        "  No,                      !- Use Weather File Daylight Saving Period",
        "  Yes,                     !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "GlobalGeometryRules,",
        "  LowerLeftCorner,         !- Starting Vertex Position",
        "  Clockwise,               !- Vertex Entry Direction",
        "  Relative;                !- Coordinate System",

        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",

        "Schedule:Constant,",
        "  Always On Discrete,      !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  1;                       !- Hourly Value",

        "Exterior:Lights,",
        "  Exterior Facade Lighting,!- Name",
        "  Always On Discrete,      !- Schedule Name",
        "  1000.00,                 !- Design Level {W}",
        "  ScheduleNameOnly,        !- Control Option",
        "  Exterior Facade Lighting;!- End-Use Subcategory",

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  Seasonal_Tariff,         !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Charge:Simple,",
        "  Seasonal_Tariff_Winter_Charge, !- Utility Cost Charge Simple Name",
        "  Seasonal_Tariff,         !- Tariff Name",
        "  totalEnergy,             !- Source Variable",
        "  Winter,                  !- Season",
        "  EnergyCharges,           !- Category Variable Name",
        "  0.02;                    !- Cost per Unit Value or Variable Name",

        "UtilityCost:Charge:Simple,",
        "  Seasonal_Tariff_Summer_Charge, !- Utility Cost Charge Simple Name",
        "  Seasonal_Tariff,         !- Tariff Name",
        "  totalEnergy,             !- Source Variable",
        "  Summer,                  !- Season",
        "  EnergyCharges,           !- Category Variable Name",
        "  0.04;                    !- Cost per Unit Value or Variable Name",

        "Output:Table:SummaryReports,",
        "  TariffReport;            !- Report 1 Name",

        "OutputControl:Table:Style,",
        "  HTML;                                   !- Column Separator",

        "Output:SQLite,",
        "  SimpleAndTabular;                       !- Option Type",

        "Output:Meter,Electricity:Facility,timestep;"

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->MinutesInTimeStep = 15;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;

    state->init_state(*state);
    ExteriorEnergyUse::ManageExteriorEnergyUse(*state);

    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularBEPS = true;
    state->dataOutRptTab->displayDemandEndUse = true;
    state->dataOutRptTab->displayLEEDSummary = true;

    state->dataOutRptTab->WriteTabularFiles = true;

    OutputReportTabular::SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoKWH;
    Real64 enerConv = OutputReportTabular::getSpecificUnitDivider(*state, "m2", "ft2");
    EXPECT_NEAR(enerConv, 0.092903, 0.001); // 0.092893973326981863

    ExteriorEnergyUse::ManageExteriorEnergyUse(*state);
    EconomicTariff::UpdateUtilityBills(*state);
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;

    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 23;
    state->dataEnvrn->DSTIndicator = 1;
    state->dataEnvrn->MonthTomorrow = 6;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 4;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    Sched::UpdateScheduleVals(*state);
    ExteriorEnergyUse::ManageExteriorEnergyUse(*state);

    state->dataGlobal->DoOutputReporting = true;
    EconomicTariff::UpdateUtilityBills(*state);

    state->dataOutRptTab->buildingGrossFloorArea = 200.0;
    state->dataOutRptTab->buildingConditionedFloorArea = 100.0;
    state->dataOutRptTab->displayEconomicResultSummary = true;

    state->dataEconTariff->chargeBlock.allocate(1);
    state->dataEconTariff->tariff(1).totalAnnualCost = 2000.0;
    state->dataEconTariff->tariff(1).isSelected = true;
    EconomicTariff::WriteTabularTariffReports(*state);

    EXPECT_EQ(state->dataEconTariff->tariff(1).totalAnnualCost, 2000.0);

    // Now test the dual-unit reporting:
    // First case single unit:
    const std::string reportName = "Economics Results Summary Report";
    const std::string tableName = "Annual Cost";

    std::vector<std::tuple<std::string, std::string, std::string, Real64>> results0(
        {{"Total", "Cost", "~~$~~", 2000.0},
         {"Total", "Cost per Total Building Area", "~~$~~/m2", 10.0},
         {"Total", "Cost per Net Conditioned Building Area", "~~$~~/m2", 20.0}});

    for (auto &v : results0) {

        std::string columnName = std::get<0>(v);
        std::string rowName = std::get<1>(v);
        std::string unitName = std::get<2>(v);
        Real64 expectedValue = std::get<3>(v);

        // for (auto &rowe : rowName) {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = '" +
                          reportName +
                          "'"
                          "  AND TableName = '" +
                          tableName +
                          "'"
                          "  AND RowName = '" +
                          rowName +
                          "'"
                          "  AND ColumnName = '" +
                          columnName +
                          "'"
                          "  AND Units = '" +
                          unitName + "'");

        Real64 return_val = execAndReturnFirstDouble(query);

        // Add informative message if failed
        EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
    }

    // Second case dual-unit:
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    EconomicTariff::WriteTabularTariffReports(*state);

    EXPECT_EQ(state->dataEconTariff->tariff(1).totalAnnualCost, 2000.0);

    std::vector<std::tuple<std::string, std::string, std::string, Real64>> results1(
        {{"Total", "Cost", "~~$~~", 2000.0},
         {"Total", "Cost per Total Building Area", "~~$~~/ft2", 10.0 * enerConv},
         {"Total", "Cost per Net Conditioned Building Area", "~~$~~/ft2", 20.0 * enerConv}});

    for (auto &v : results1) {

        std::string columnName = std::get<0>(v);
        std::string rowName = std::get<1>(v);
        std::string unitName = std::get<2>(v);
        Real64 expectedValue = std::get<3>(v);

        // for (auto &rowe : rowName) {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = '" +
                          reportName +
                          "'"
                          "  AND TableName = '" +
                          tableName +
                          "'"
                          "  AND RowName = '" +
                          rowName +
                          "'"
                          "  AND ColumnName = '" +
                          columnName +
                          "'"
                          "  AND Units = '" +
                          unitName + "'");

        Real64 return_val = execAndReturnFirstDouble(query);

        // Add informative message if failed
        EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
    }
}

TEST_F(EnergyPlusFixture, EconomicTariff_LEEDtariff_with_Custom_Meter)
{
    std::string const idf_objects = delimited_string({

        "Meter:Custom,",
        "    Building Natural Gas,    !- Name",
        "    NaturalGas,              !- Resource Type",
        "    ,                        !- Key Name 1",
        "    NaturalGas:Facility;     !- Output Variable or Meter Name 1",

        "UtilityCost:Tariff,",
        "    Sample with All Utilities_NGas,  !- Name",
        "    Building Natural Gas,    !- Output Meter Name",
        "    Therm,                   !- Conversion Factor Choice",
        "    ,                        !- Energy Conversion Factor",
        "    ,                        !- Demand Conversion Factor",
        "    ,                        !- Time of Use Period Schedule Name",
        "    ,                        !- Season Schedule Name",
        "    ,                        !- Month Schedule Name",
        "    ,                        !- Demand Window Length",
        "    ,                        !- Monthly Charge or Variable Name",
        "    ,                        !- Minimum Monthly Charge or Variable Name",
        "    ,                        !- Real Time Pricing Charge Schedule Name",
        "    ,                        !- Customer Baseline Load Schedule Name",
        "    ,                        !- Group Name",
        "    BuyFromUtility;          !- Buy Or Sell",

        "UtilityCost:Charge:Simple,",
        "    FlatEnergyCharge-Gas_Custom,  !- Utility Cost Charge Simple Name",
        "    Sample with All Utilities_NGas,  !- Tariff Name",
        "    totalEnergy,             !- Source Variable",
        "    Annual,                  !- Season",
        "    EnergyCharges,           !- Category Variable Name",
        "    0.50;                    !- Cost per Unit Value or Variable Name",

        "UtilityCost:Tariff,                                                 ",
        "    ExampleA,                !- Name                                ",
        "    ElectricityPurchased:Facility,  !- Output Meter Name            ",
        "    kWh,                     !- Conversion Factor Choice            ",
        "    ,                        !- Energy Conversion Factor            ",
        "    ,                        !- Demand Conversion Factor            ",
        "    ,                        !- Time of Use Period Schedule Name    ",
        "    ,                        !- Season Schedule Name                ",
        "    ,                        !- Month Schedule Name                 ",
        "    ,                        !- Demand Window Length                ",
        "    2.51;                    !- Monthly Charge or Variable Name     ",

        "UtilityCost:Charge:Simple,                                          ",
        "    FlatEnergyCharge,        !- Utility Cost Charge Simple Name     ",
        "    ExampleA,                !- Tariff Name                         ",
        "    totalEnergy,             !- Source Variable                     ",
        "    Annual,                  !- Season                              ",
        "    EnergyCharges,           !- Category Variable Name              ",
        "    0.055342;                !- Cost per Unit Value or Variable Name",

        "UtilityCost:Tariff,                                                      ",
        "    ExampleI-Sell,           !- Name                                     ",
        "    ElectricitySurplusSold:Facility,  !- Output Meter Name               ",
        "    kWh,                     !- Conversion Factor Choice                 ",
        "    ,                        !- Energy Conversion Factor                 ",
        "    ,                        !- Demand Conversion Factor                 ",
        "    ,                        !- Time of Use Period Schedule Name         ",
        "    ,                        !- Season Schedule Name                     ",
        "    ,                        !- Month Schedule Name                      ",
        "    ,                        !- Demand Window Length                     ",
        "    ,                        !- Monthly Charge or Variable Name          ",
        "    ,                        !- Minimum Monthly Charge or Variable Name  ",
        "    ,                        !- Real Time Pricing Charge Schedule Name   ",
        "    ,                        !- Customer Baseline Load Schedule Name     ",
        "    ,                        !- Group Name                               ",
        "    sellToUtility;           !- Buy Or Sell                              ",

        "UtilityCost:Charge:Simple,                                               ",
        "    GeneratedElectricSold,   !- Utility Cost Charge Simple Name          ",
        "    ExampleI-Sell,           !- Tariff Name                              ",
        "    totalEnergy,             !- Source Variable                          ",
        "    Annual,                  !- Season                                   ",
        "    EnergyCharges,           !- Category Variable Name                   ",
        "    -0.02;                   !- Cost per Unit Value or Variable Name     ",

        "UtilityCost:Tariff,                                             ",
        "    ExampleA-Gas,            !- Name                            ",
        "    NaturalGas:Facility,     !- Output Meter Name               ",
        "    Therm,                   !- Conversion Factor Choice        ",
        "    ,                        !- Energy Conversion Factor        ",
        "    ,                        !- Demand Conversion Factor        ",
        "    ,                        !- Time of Use Period Schedule Name",
        "    ,                        !- Season Schedule Name            ",
        "    ,                        !- Month Schedule Name             ",
        "    ,                        !- Demand Window Length            ",
        "    15.;                     !- Monthly Charge or Variable Name ",

        "UtilityCost:Charge:Simple,                                          ",
        "    FlatEnergyCharge-Gas,    !- Utility Cost Charge Simple Name     ",
        "    ExampleA-Gas,            !- Tariff Name                         ",
        "    totalEnergy,             !- Source Variable                     ",
        "    Annual,                  !- Season                              ",
        "    EnergyCharges,           !- Category Variable Name              ",
        "    0.50;                    !- Cost per Unit Value or Variable Name",

        "UtilityCost:Tariff,                                                 ",
        "    Water Tariff,            !- Name                                ",
        "    Water:Facility,          !- Output Meter Name                   ",
        "    UserDefined,             !- Conversion Factor Choice            ",
        "    1,                       !- Energy Conversion Factor            ",
        "    ,                        !- Demand Conversion Factor            ",
        "    ,                        !- Time of Use Period Schedule Name    ",
        "    ,                        !- Season Schedule Name                ",
        "    ,                        !- Month Schedule Name                 ",
        "    ,                        !- Demand Window Length                ",
        "    0.0;                     !- Monthly Charge or Variable Name     ",

        "UtilityCost:Charge:Simple,                                          ",
        "    WaterTariffEnergyCharge, !- Utility Cost Charge Simple Name     ",
        "    Water Tariff,            !- Tariff Name                         ",
        "    totalEnergy,             !- Source Variable                     ",
        "    Annual,                  !- Season                              ",
        "    EnergyCharges,           !- Category Variable Name              ",
        "    7.0;                     !- Cost per Unit Value or Variable Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool errors_found = false;

    GetCustomMeterInput(*state, errors_found);

    EXPECT_FALSE(errors_found);

    state->dataOutputProcessor->meters.push_back(new Meter("ELECTRICITY:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("ELECTRICITY:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("ElectricitySurplusSold:Facility"));
    state->dataOutputProcessor->meterMap.insert_or_assign("ELECTRICITYSURPLUSSOLD:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    state->dataOutputProcessor->meters.push_back(new Meter("NATURALGAS:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("NATURALGAS:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("DISTRICTCOOLING:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("DISTRICTCOOLING:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("DISTRICTHEATINGWATER:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("DISTRICTHEATINGWATER:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("WATER:FACILITY"));
    state->dataOutputProcessor->meterMap.insert_or_assign("WATER:FACILITY", state->dataOutputProcessor->meters.size() - 1);
    state->dataOutputProcessor->meters.push_back(new Meter("Building Natural Gas"));
    state->dataOutputProcessor->meterMap.insert_or_assign("BUILDING NATURAL GAS", state->dataOutputProcessor->meters.size() - 1);

    // int elecFacilMeter = GetMeterIndex(*state, "ELECTRICITY:FACILITY");
    // int gasFacilMeter = GetMeterIndex(*state, "NATURALGAS:FACILITY");
    // int distCoolFacilMeter = GetMeterIndex(*state, "DISTRICTCOOLING:FACILITY");
    // int distHeatFacilMeter = GetMeterIndex(*state, "DISTRICTHEATING:FACILITY");

    state->dataEconTariff->numTariff = 7;
    state->dataEconTariff->tariff.allocate(state->dataEconTariff->numTariff);
    state->dataEconTariff->tariff(1).tariffName = "ExampleA";
    state->dataEconTariff->tariff(1).isSelected = true;
    state->dataEconTariff->tariff(1).totalAnnualCost = 1000.0;
    state->dataEconTariff->tariff(1).totalAnnualEnergy = 10000.0;
    state->dataEconTariff->tariff(1).kindMtr = EconomicTariff::MeterType::ElecPurchased;
    state->dataEconTariff->tariff(1).reportMeterIndx = GetMeterIndex(*state, "ELECTRICITY:FACILITY");

    state->dataEconTariff->tariff(2).tariffName = "ExampleI-Sell";
    state->dataEconTariff->tariff(2).isSelected = true;
    state->dataEconTariff->tariff(2).totalAnnualCost = -40.0;
    state->dataEconTariff->tariff(2).totalAnnualEnergy = 2000.0;
    state->dataEconTariff->tariff(2).kindMtr = EconomicTariff::MeterType::ElecSurplusSold;
    state->dataEconTariff->tariff(2).reportMeterIndx = GetMeterIndex(*state, "ELECTRICITYSURPLUSSOLD:FACILITY");

    state->dataEconTariff->tariff(3).tariffName = "ExampleA-Gas";
    state->dataEconTariff->tariff(3).isSelected = true;
    state->dataEconTariff->tariff(3).totalAnnualCost = 1500.0;
    state->dataEconTariff->tariff(3).totalAnnualEnergy = 3000.0;
    state->dataEconTariff->tariff(3).kindMtr = EconomicTariff::MeterType::Gas;
    state->dataEconTariff->tariff(3).reportMeterIndx = GetMeterIndex(*state, "NATURALGAS:FACILITY");

    state->dataEconTariff->tariff(4).tariffName = "DistrictCoolingUnit";
    state->dataEconTariff->tariff(4).isSelected = true;
    state->dataEconTariff->tariff(4).totalAnnualCost = 50.0;
    state->dataEconTariff->tariff(4).totalAnnualEnergy = 10.0;
    state->dataEconTariff->tariff(4).reportMeterIndx = GetMeterIndex(*state, "DISTRICTCOOLING:FACILITY");

    state->dataEconTariff->tariff(5).tariffName = "DistrictHeatingUnit";
    state->dataEconTariff->tariff(5).isSelected = true;
    state->dataEconTariff->tariff(5).totalAnnualCost = 168;
    state->dataEconTariff->tariff(5).totalAnnualEnergy = 15;
    state->dataEconTariff->tariff(5).reportMeterIndx = GetMeterIndex(*state, "DISTRICTHEATINGWATER:FACILITY");

    state->dataEconTariff->tariff(6).tariffName = "Water Tariff";
    state->dataEconTariff->tariff(6).isSelected = true;
    state->dataEconTariff->tariff(6).totalAnnualCost = 1050;
    state->dataEconTariff->tariff(6).totalAnnualEnergy = 150;
    state->dataEconTariff->tariff(6).kindMtr = EconomicTariff::MeterType::Water;
    state->dataEconTariff->tariff(6).reportMeterIndx = GetMeterIndex(*state, "WATER:FACILITY");

    state->dataEconTariff->tariff(7).tariffName = "Sample with All Utilities_NGas";
    state->dataEconTariff->tariff(7).isSelected = true;
    state->dataEconTariff->tariff(7).totalAnnualCost = 1500.0;
    state->dataEconTariff->tariff(7).totalAnnualEnergy = 3000.0;
    state->dataEconTariff->tariff(7).kindMtr = EconomicTariff::MeterType::Gas;
    state->dataEconTariff->tariff(7).reportMeterIndx = GetMeterIndex(*state, "BUILDING NATURAL GAS");

    SetPredefinedTables(*state); // setup the predefined table entry numbers first

    LEEDtariffReporting(*state);

    EXPECT_EQ("ExampleA ExampleI-Sell", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "Electricity"));

    EXPECT_EQ("ExampleA-Gas Sample with All Utilities_NGas",
              RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "Natural Gas"));

    EXPECT_EQ("DistrictCoolingUnit", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "District Cooling"));

    EXPECT_EQ("DistrictHeatingUnit", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "District Heating Water"));

    EXPECT_EQ("", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsRtNm, "Other"));

    EXPECT_EQ("0.096", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "Electricity"));

    EXPECT_EQ("1.000", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "Natural Gas"));

    EXPECT_EQ("5.000", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "District Cooling"));

    EXPECT_EQ("11.200", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "District Heating Water"));

    EXPECT_EQ("NOT FOUND", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedEtsVirt, "Other"));
}
