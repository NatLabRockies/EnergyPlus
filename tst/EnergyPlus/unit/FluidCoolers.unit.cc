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

// EnergyPlus::FluidCoolers Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidCoolers.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::FluidCoolers;
using namespace EnergyPlus::DataSizing;

TEST_F(EnergyPlusFixture, TwoSpeedFluidCoolerInput_Test1)
{
    state->init_state(*state);

    using DataSizing::AutoSize;
    int StringArraySize = 20;
    Array1D_string cNumericFieldNames;
    cNumericFieldNames.allocate(StringArraySize);
    Array1D_string cAlphaFieldNames;
    cAlphaFieldNames.allocate(StringArraySize);
    Array1D_string AlphArray;
    AlphArray.allocate(StringArraySize);
    for (int i = 1; i <= StringArraySize; ++i) {
        cAlphaFieldNames(i) = "AlphaField";
        cNumericFieldNames(i) = "NumerField";
        AlphArray(i) = "FieldValues";
    }
    std::string const cCurrentModuleObject("FluidCooler:TwoSpeed");
    int FluidCoolerNum(1);
    state->dataFluidCoolers->SimpleFluidCooler.allocate(FluidCoolerNum);

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).Name = "Test";
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerMassFlowRateMultiplier = 2.5;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).PerformanceInputMethod_Num = PerfInputMethod::NOMINAL_CAPACITY;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).WaterInletNodeNum = 1;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).WaterOutletNodeNum = 1;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerNominalCapacity = 50000;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringWaterTemp = 52;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringAirTemp = 35;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringAirWetBulbTemp = 25;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedAirFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedAirFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFanPower = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFanPowerWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedAirFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedAirFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFanPower = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFanPowerWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerLowSpeedNomCap = 30000;

    AlphArray(4) = "NominalCapacity";
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUA = 0;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFluidCoolerUA = 0;
    state->dataFluidCoolers->SimpleFluidCooler(1).DesignEnteringWaterTemp = 50;
    bool testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateTwoSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(1).DesignEnteringWaterTemp = -10;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateTwoSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_TRUE(testResult); // error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(1).DesignEnteringWaterTemp = 50;
    state->dataFluidCoolers->SimpleFluidCooler(1).FluidCoolerLowSpeedNomCap = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(1).FluidCoolerLowSpeedNomCapWasAutoSized = true;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateTwoSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(1).FluidCoolerLowSpeedNomCap = 0; // this should trigger the original error condition
    state->dataFluidCoolers->SimpleFluidCooler(1).FluidCoolerLowSpeedNomCapWasAutoSized = false;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateTwoSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_TRUE(testResult); // error message triggered

    state->dataFluidCoolers->SimpleFluidCooler.deallocate();
}

TEST_F(EnergyPlusFixture, TwoSpeedFluidCoolerInput_Test2)
{
    state->init_state(*state);

    using DataSizing::AutoSize;
    int StringArraySize = 20;
    Array1D_string cNumericFieldNames;
    cNumericFieldNames.allocate(StringArraySize);
    Array1D_string cAlphaFieldNames;
    cAlphaFieldNames.allocate(StringArraySize);
    Array1D_string AlphArray;
    AlphArray.allocate(StringArraySize);
    for (int i = 1; i <= StringArraySize; ++i) {
        cAlphaFieldNames(i) = "AlphaField";
        cNumericFieldNames(i) = "NumerField";
        AlphArray(i) = "FieldValues";
    }
    std::string const cCurrentModuleObject("FluidCooler:TwoSpeed");
    int FluidCoolerNum(1);
    state->dataFluidCoolers->SimpleFluidCooler.allocate(FluidCoolerNum);

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).Name = "Test";
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerMassFlowRateMultiplier = 1.0;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).PerformanceInputMethod_Num = PerfInputMethod::U_FACTOR;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringWaterTemp = 52;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringAirTemp = 35;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringAirWetBulbTemp = 25;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedAirFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedAirFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFanPower = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFanPowerWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedAirFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedAirFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFanPower = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFanPowerWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerLowSpeedNomCap = 30000;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFluidCoolerUA = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).LowSpeedFluidCoolerUAWasAutoSized = true;

    AlphArray(4) = "UFactorTimesAreaAndDesignWaterFlowRate";
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUA = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUAWasAutoSized = false;
    bool testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateTwoSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_TRUE(testResult); // error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUA = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUAWasAutoSized = true;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateTwoSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered

    state->dataFluidCoolers->SimpleFluidCooler.deallocate();
    cNumericFieldNames.deallocate();
    cAlphaFieldNames.deallocate();
    AlphArray.deallocate();
}

TEST_F(EnergyPlusFixture, SingleSpeedFluidCoolerInput_Test3)
{
    state->init_state(*state);

    using DataSizing::AutoSize;
    int StringArraySize = 20;
    Array1D_string cNumericFieldNames;
    cNumericFieldNames.allocate(StringArraySize);
    Array1D_string cAlphaFieldNames;
    cAlphaFieldNames.allocate(StringArraySize);
    Array1D_string AlphArray;
    AlphArray.allocate(StringArraySize);
    for (int i = 1; i <= StringArraySize; ++i) {
        cAlphaFieldNames(i) = "AlphaField";
        cNumericFieldNames(i) = "NumerField";
        AlphArray(i) = "FieldValues";
    }
    std::string const cCurrentModuleObject("FluidCooler:SingleSpeed");
    int FluidCoolerNum(1);
    state->dataFluidCoolers->SimpleFluidCooler.allocate(FluidCoolerNum);

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).Name = "Test";
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerMassFlowRateMultiplier = 2.5;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).PerformanceInputMethod_Num = PerfInputMethod::U_FACTOR;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).WaterInletNodeNum = 1;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).WaterOutletNodeNum = 1;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).FluidCoolerNominalCapacity = 50000;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringWaterTemp = 52;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringAirTemp = 35;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignEnteringAirWetBulbTemp = 25;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedAirFlowRate = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedAirFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFanPower = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFanPowerWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUA = AutoSize;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).HighSpeedFluidCoolerUAWasAutoSized = true;

    AlphArray(4) = "UFactorTimesAreaAndDesignWaterFlowRate";
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRate = 1;
    bool testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateSingleSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRateWasAutoSized = true;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRate = 0;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateSingleSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRateWasAutoSized = false;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRate = 1;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateSingleSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered

    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRateWasAutoSized = false;
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).DesignWaterFlowRate = 0;
    testResult = state->dataFluidCoolers->SimpleFluidCooler(1).validateSingleSpeedInputs(
        *state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_TRUE(testResult); // error message triggered
}

TEST_F(EnergyPlusFixture, SingleSpeedFluidCoolerInput_Test4)
{
    int FluidCoolerNum(1);

    std::string const idf_objects = delimited_string({
        "   FluidCooler:SingleSpeed,",
        "     FluidCooler_SingleSpeed, !- Name",
        "     FluidCooler_SingleSpeed Water Inlet,  !- Water Inlet Node Name",
        "     FluidCooler_SingleSpeed Water Outlet,  !- Water Outlet Node Name",
        "     UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "     autosize,                !- Design Air Flow Rate U-factor Times Area Value {W/K}",
        "     ,                        !- Nominal Capacity {W}",
        "     46.0,                    !- Design Entering Water Temperature {C}",
        "     35.0,                    !- Design Entering Air Temperature {C}",
        "     25.5,                    !- Design Entering Air Wetbulb Temperature {C}",
        "     5.05e-03,                 !- Design Water Flow Rate {m3/s}",
        "     autosize,                !- Design Air Flow Rate {m3/s}",
        "     autosize;                !- Design Air Flow Rate Fan Power {W}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetFluidCoolerInput(*state);
    auto &thisFluidCooler = state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum);
    EXPECT_TRUE(thisFluidCooler.HighSpeedFluidCoolerUAWasAutoSized);
    EXPECT_EQ(thisFluidCooler.HighSpeedFluidCoolerUA, DataSizing::AutoSize);
    EXPECT_EQ(thisFluidCooler.FluidCoolerNominalCapacity, 0.0);
}

TEST_F(EnergyPlusFixture, SingleSpeedFluidCoolerInput_Test5)
{
    state->init_state(*state);
    using DataSizing::AutoSize;
    int StringArraySize = 20;
    Array1D_string cNumericFieldNames;
    cNumericFieldNames.allocate(StringArraySize);
    Array1D_string cAlphaFieldNames;
    cAlphaFieldNames.allocate(StringArraySize);
    Array1D_string AlphArray;
    AlphArray.allocate(StringArraySize);
    for (int i = 1; i <= StringArraySize; ++i) {
        cAlphaFieldNames(i) = "AlphaField";
        cNumericFieldNames(i) = "NumerField";
        AlphArray(i) = "FieldValues";
    }
    std::string const cCurrentModuleObject("FluidCooler:SingleSpeed");
    int FluidCoolerNum(1);
    state->dataFluidCoolers->SimpleFluidCooler.allocate(FluidCoolerNum);
    auto &thisFluidCooler = state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum);

    thisFluidCooler.Name = "Test";
    thisFluidCooler.FluidCoolerMassFlowRateMultiplier = 2.5;
    thisFluidCooler.WaterInletNodeNum = 1;
    thisFluidCooler.WaterOutletNodeNum = 1;
    thisFluidCooler.DesignEnteringWaterTemp = 52;
    thisFluidCooler.DesignEnteringAirTemp = 35;
    thisFluidCooler.DesignEnteringAirWetBulbTemp = 25;
    thisFluidCooler.HighSpeedAirFlowRate = AutoSize;
    thisFluidCooler.HighSpeedAirFlowRateWasAutoSized = true;
    thisFluidCooler.HighSpeedFanPower = AutoSize;
    thisFluidCooler.HighSpeedFanPowerWasAutoSized = true;
    thisFluidCooler.DesignWaterFlowRateWasAutoSized = true;
    thisFluidCooler.DesignWaterFlowRate = 1;
    // test nominal capacity specified hard value
    AlphArray(4) = "NominalCapacity";
    thisFluidCooler.FluidCoolerNominalCapacity = 5000.0;
    thisFluidCooler.HighSpeedFluidCoolerUA = 500.0;
    thisFluidCooler.HighSpeedFluidCoolerUAWasAutoSized = false;
    // test input error check, if the nominal capacity specified and UA value is not zero, then it does not fatal out
    bool testResult = thisFluidCooler.validateSingleSpeedInputs(*state, cCurrentModuleObject, AlphArray, cNumericFieldNames, cAlphaFieldNames);
    EXPECT_FALSE(testResult); // no error message triggered
    EXPECT_ENUM_EQ(thisFluidCooler.PerformanceInputMethod_Num, PerfInputMethod::NOMINAL_CAPACITY);
    // UA value is reset to zero if nominal capacity is specified and input method is "NOMINAL_CAPACITY"
    EXPECT_EQ(thisFluidCooler.HighSpeedFluidCoolerUA, 0.0);
}

TEST_F(EnergyPlusFixture, FluidCooler_SizeWhenPlantSizingIndexIsZero)
{
    int FluidCoolerNum(1);

    std::string const idf_objects = delimited_string({
        "   FluidCooler:SingleSpeed,",
        "     Dry Cooler,              !- Name",
        "     Dry Cooler Inlet Node,   !- Water Inlet Node Name",
        "     Dry Cooler Outlet Node,  !- Water Outlet Node Name",
        "     NominalCapacity,         !- Performance Input Method",
        "     ,                        !- Design Air Flow Rate U-factor Times Area Value {W/K}",
        "     58601,                   !- Nominal Capacity {W}",
        "     50,                      !- Design Entering Water Temperature {C}",
        "     35,                      !- Design Entering Air Temperature {C}",
        "     25,                      !- Design Entering Air Wetbulb Temperature {C}",
        "     0.001262,                !- Design Water Flow Rate {m3/s}",
        "     2.124,                   !- Design Air Flow Rate {m3/s}",
        "     1491;                    !- Design Air Flow Rate Fan Power {W}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetFluidCoolerInput(*state);

    auto &thisFluidCooler = state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum);

    state->dataPlnt->PlantLoop.allocate(FluidCoolerNum);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    // state->dataFluidCoolers->SimpleFluidCooler.allocate(FluidCoolerNum);
    state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum).plantLoc.loopNum = 1;
    state->dataPlnt->PlantLoop(1).PlantSizNum = 0;

    EXPECT_EQ("DRY COOLER", thisFluidCooler.Name);
    EXPECT_FALSE(thisFluidCooler.HighSpeedFanPowerWasAutoSized);
    EXPECT_FALSE(thisFluidCooler.HighSpeedAirFlowRateWasAutoSized);
    EXPECT_FALSE(thisFluidCooler.HighSpeedFluidCoolerUAWasAutoSized);

    thisFluidCooler.size(*state);
}

TEST_F(EnergyPlusFixture, ExerciseSingleSpeedFluidCooler)
{
    std::string const idf_objects = delimited_string({
        "   FluidCooler:SingleSpeed,",
        "     Dry Cooler,              !- Name",
        "     Dry Cooler Inlet Node,   !- Water Inlet Node Name",
        "     Dry Cooler Outlet Node,  !- Water Outlet Node Name",
        "     NominalCapacity,         !- Performance Input Method",
        "     ,                        !- Design Air Flow Rate U-factor Times Area Value {W/K}",
        "     58601,                   !- Nominal Capacity {W}",
        "     50,                      !- Design Entering Water Temperature {C}",
        "     35,                      !- Design Entering Air Temperature {C}",
        "     25,                      !- Design Entering Air Wetbulb Temperature {C}",
        "     0.001262,                !- Design Water Flow Rate {m3/s}",
        "     2.124,                   !- Design Air Flow Rate {m3/s}",
        "     1491;                    !- Design Air Flow Rate Fan Power {W}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    FluidCoolerspecs *ptr = FluidCoolerspecs::factory(*state, DataPlant::PlantEquipmentType::FluidCooler_SingleSpd, "DRY COOLER");

    PlantLocation pl{1, EnergyPlus::DataPlant::LoopSideLocation::Supply, 1, 1};
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    Real64 max, opt, min = 0.0;
    ptr->getDesignCapacities(*state, pl, max, min, opt);
    EXPECT_NEAR(max, 58601.0, 1.0);
    EXPECT_NEAR(min, 0.0, 1.0);
    EXPECT_NEAR(opt, 58601.0, 1.0);

    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).TempSetPoint = 2.0;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).MyLoad = 1000;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).ON = true;
    state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 3;
    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 3;

    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMax = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).Temp = 20;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMax = 5;

    // We don't set ptr->OutdoorAirInletNodeNum to a node, so the fluid cooler uses the dataEnvrn values
    // We don't want them as 0 to avoid a divide by zero error, when PsyRhoAirFnPbTdbW will be called with these value in CalcFluidCoolerOutlet
    EXPECT_EQ(0, ptr->OutdoorAirInletNodeNum);
    state->dataEnvrn->OutBaroPress = 101325;
    state->dataEnvrn->OutHumRat = 0.0001;

    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    ptr->plantLoc.loopNum = 1;
    ptr->plantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    ptr->plantLoc.branchNum = 1;
    ptr->plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, ptr->plantLoc);

    ptr->DesWaterMassFlowRate = 3.141;
    ptr->WaterMassFlowRate = 3.141;
    ptr->onInitLoopEquip(*state, pl);
    ptr->HighSpeedFluidCoolerUA = 10;
    ptr->simulate(*state, pl, firstHVAC, curLoad, true);
}

TEST_F(EnergyPlusFixture, ExerciseTwoSpeedFluidCooler)
{
    std::string const idf_objects = delimited_string({"FluidCooler:TwoSpeed,",
                                                      "Big FluidCooler,         !- Name",
                                                      "Condenser FluidCooler Inlet Node,  !- Water Inlet Node Name",
                                                      "Condenser FluidCooler Outlet Node,  !- Water Outlet Node Name",
                                                      "NominalCapacity,         !- Performance Input Method",
                                                      ",                        !- High Fan Speed U-factor Times Area Value {W/K}",
                                                      ",                        !- Low Fan Speed U-factor Times Area Value {W/K}",
                                                      ",                        !- Low Fan Speed U-Factor Times Area Sizing Factor",
                                                      "58601.,                  !- High Speed Nominal Capacity {W}",
                                                      "28601.,                  !- Low Speed Nominal Capacity {W}",
                                                      ",                        !- Low Speed Nominal Capacity Sizing Factor",
                                                      "51.67,                   !- Design Entering Water Temperature {C}",
                                                      "35,                      !- Design Entering Air Temperature {C}",
                                                      "25.6,                    !- Design Entering Air Wet-bulb Temperature {C}",
                                                      "Autosize,                !- Design Water Flow Rate {m3/s}",
                                                      "Autosize,                !- High Fan Speed Air Flow Rate {m3/s}",
                                                      "Autosize,                !- High Fan Speed Fan Power {W}",
                                                      "autocalculate,           !- Low Fan Speed Air Flow Rate {m3/s}",
                                                      ",                        !- Low Fan Speed Air Flow Rate Sizing Factor",
                                                      "autocalculate,           !- Low Fan Speed Fan Power {W}",
                                                      ";                        !- Low Fan Speed Fan Power Sizing Factor"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    FluidCoolerspecs *ptr = FluidCoolerspecs::factory(*state, DataPlant::PlantEquipmentType::FluidCooler_TwoSpd, "BIG FLUIDCOOLER");

    PlantLocation pl{1, EnergyPlus::DataPlant::LoopSideLocation::Supply, 1, 1};
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    Real64 max, opt, min = 0.0;
    ptr->getDesignCapacities(*state, pl, max, min, opt);
    EXPECT_NEAR(max, 58601.0, 1.0);
    EXPECT_NEAR(min, 0.0, 1.0);
    EXPECT_NEAR(opt, 58601.0, 1.0);

    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).TempSetPoint = 2.0;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).MyLoad = 1000;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).ON = true;
    state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 3;
    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 3;

    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMax = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).Temp = 20;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMax = 5;

    // We don't set ptr->OutdoorAirInletNodeNum to a node, so the fluid cooler uses the dataEnvrn values
    // We don't want them as 0 to avoid a divide by zero error, when PsyRhoAirFnPbTdbW will be called with these value in CalcFluidCoolerOutlet
    EXPECT_EQ(0, ptr->OutdoorAirInletNodeNum);
    state->dataEnvrn->OutBaroPress = 101325;
    state->dataEnvrn->OutHumRat = 0.0001;

    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    ptr->plantLoc.loopNum = 1;
    ptr->plantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    ptr->plantLoc.branchNum = 1;
    ptr->plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, ptr->plantLoc);
    ptr->DesWaterMassFlowRate = 3.141;
    ptr->WaterMassFlowRate = 3.141;
    ptr->HighSpeedAirFlowRate = 2.124; // Autosizing didn't occur so...
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).ExitTemp = 25.0;
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    ptr->onInitLoopEquip(*state, pl);
    ptr->HighSpeedFluidCoolerUA = 10;
    ptr->simulate(*state, pl, firstHVAC, curLoad, true);
    state->dataPlnt->PlantLoop(pl.loopNum).LoopSide(pl.loopSideNum).FlowLock = DataPlant::FlowLock::Locked;
    ptr->simulate(*state, pl, firstHVAC, curLoad, true);
}

TEST_F(EnergyPlusFixture, FluidCooler_SizeWhenPlantSizingIndexIsZeroAndAutosized)
{
    // Test for #10817
    std::string const idf_objects = delimited_string({
        "   FluidCooler:SingleSpeed,",
        "     Dry Cooler,              !- Name",
        "     Dry Cooler Inlet Node,   !- Water Inlet Node Name",
        "     Dry Cooler Outlet Node,  !- Water Outlet Node Name",
        "     NominalCapacity,         !- Performance Input Method",
        "     Autosize,                !- Design Air Flow Rate U-factor Times Area Value {W/K}",
        "     58601,                   !- Nominal Capacity {W}",
        "     50,                      !- Design Entering Water Temperature {C}",
        "     35,                      !- Design Entering Air Temperature {C}",
        "     25,                      !- Design Entering Air Wetbulb Temperature {C}",
        "     Autosize,                !- Design Water Flow Rate {m3/s}",
        "     Autosize,                !- Design Air Flow Rate {m3/s}",
        "     Autosize;                !- Design Air Flow Rate Fan Power {W}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetFluidCoolerInput(*state);
    int FluidCoolerNum(1);

    state->dataPlnt->PlantLoop.allocate(FluidCoolerNum);
    state->dataPlnt->PlantLoop(FluidCoolerNum).PlantSizNum = 0;

    auto &thisFluidCooler = state->dataFluidCoolers->SimpleFluidCooler(FluidCoolerNum);
    thisFluidCooler.plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisFluidCooler.plantLoc);

    // Necessary to trigger the crash from #
    state->dataPlnt->PlantFirstSizesOkayToFinalize = false;

    EXPECT_TRUE(thisFluidCooler.DesignWaterFlowRateWasAutoSized);
    EXPECT_TRUE(thisFluidCooler.HighSpeedFanPowerWasAutoSized);
    EXPECT_TRUE(thisFluidCooler.HighSpeedAirFlowRateWasAutoSized);
    EXPECT_TRUE(thisFluidCooler.HighSpeedFluidCoolerUAWasAutoSized);

    thisFluidCooler.size(*state);
}
