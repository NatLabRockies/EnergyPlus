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

#include "AutosizingFixture.hh"
#include <gtest/gtest.h>

#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, SystemAirFlowSizingGauntlet)
{
    // this global state is what would be set up by E+ currently
    state->dataSize->ZoneEqSizing.allocate(1);
    static constexpr std::string_view routineName("SystemAirFlowSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    SystemAirFlowSizer sizer;
    Real64 inputValue = 5;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.0001); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING
    state->dataSize->CurZoneEqNum = 1;

    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Flow Rate [m3/s], 5.00000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    // now allocate sizing arrays for testing autosized field
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(1).SizingMethod.allocate(35);

    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing(1).DesCoolVolFlow = 1.6;
    state->dataSize->FinalZoneSizing(1).DesHeatVolFlow = 1.2;
    state->dataSize->FinalZoneSizing(1).CoolDDNum = 1;
    state->dataSize->FinalZoneSizing(1).HeatDDNum = 2;
    state->dataSize->FinalZoneSizing(1).TimeStepNumAtCoolMax = 12;
    state->dataSize->FinalZoneSizing(1).TimeStepNumAtHeatMax = 6;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataEnvrn->TotDesDays = 2;
    state->dataWeather->DesDayInput.allocate(2);
    state->dataWeather->DesDayInput(1).Month = 7;
    state->dataWeather->DesDayInput(1).DayOfMonth = 7;
    state->dataWeather->DesDayInput(2).Month = 1;
    state->dataWeather->DesDayInput(2).DayOfMonth = 1;
    state->dataWeather->DesDayInput(1).Title = "CoolingDD";
    state->dataWeather->DesDayInput(2).Title = "HeatingDD";

    // Test 2 - Zone Equipment, sizes to greater of zone cooling and heating flow
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.6, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Flow Rate [m3/s], 1.60000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 3 - Zone Equipment, set heating only fan
    state->dataSize->ZoneHeatingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 4 - Zone Equipment, set cooling only fan
    state->dataSize->ZoneHeatingOnlyFan = false;
    state->dataSize->ZoneCoolingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.6, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 5 - Zone Equipment, cooling only fan, set fraction used for sizing
    state->dataSize->DataFractionUsedForSizing = 0.5;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.8, sizedValue, 0.0001);             // 1/2 of zone cooling air flow rate
    sizer.autoSizedValue = 0.0;                       // reset for next test
    state->dataSize->DataFractionUsedForSizing = 0.0; // reset for next test

    // Test 6 - Zone Equipment, set ZoneEqSizing data
    state->dataSize->ZoneEqSizing(1).SystemAirFlow = true;
    state->dataSize->ZoneEqSizing(1).AirVolFlow = 1.8;
    state->dataSize->ZoneEqSizing(1).SizingMethod(int(sizer.sizingType)) = DataSizing::SupplyAirFlowRate;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.8, sizedValue, 0.0001); // max of zone cooling/heating/ZoneEqSizing
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 7 - Zone Equipment, set cooling only fan
    state->dataSize->ZoneEqSizing(1).SystemAirFlow = false;
    state->dataSize->ZoneCoolingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.6, sizedValue, 0.0001); // zone cooling flow
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 8 - Zone Equipment, set heating only fan
    state->dataSize->ZoneCoolingOnlyFan = false;
    state->dataSize->ZoneHeatingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // zone heating flow
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 9 - Zone Equipment, set ZoneEqSizing cooling air flow
    state->dataSize->ZoneHeatingOnlyFan = false;
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    state->dataSize->ZoneEqSizing(1).CoolingAirVolFlow = 2.2;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2.2, sizedValue, 0.0001); // ZoneEqSizing cooling flow
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 10 - Zone Equipment, set ZoneEqSizing heating air flow
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(1).HeatingAirVolFlow = 3.2;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3.2, sizedValue, 0.0001); // ZoneEqSizing heating flow
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 11 - Zone Equipment, set ZoneEqSizing cooling and heating air flow
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3.2, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow
    sizer.autoSizedValue = 0.0;           // reset for next test
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = false;

    state->dataSize->ZoneEqSizing(1).SizingMethod(int(sizer.sizingType)) = DataSizing::FractionOfAutosizedCoolingAirflow;

    // Test 12 - Zone Equipment, set fraction of autosized cooling flow for cooling only fan
    state->dataSize->DataFracOfAutosizedCoolingAirflow = 0.4;
    state->dataSize->ZoneCoolingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.64, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 13 - Zone Equipment, set fraction of autosized heating flow for heating only fan
    state->dataSize->DataFracOfAutosizedHeatingAirflow = 0.4;
    state->dataSize->ZoneCoolingOnlyFan = false;
    state->dataSize->ZoneHeatingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.48, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 14 - Zone Equipment, cooling or heating fan not set
    state->dataSize->ZoneHeatingOnlyFan = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.64, sizedValue, 0.0001); // max of FinalZoneSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 15 - Zone Equipment, ZoneEqSizing cooling
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.88, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 16 - Zone Equipment, ZoneEqSizing heating
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.28, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 17 - Zone Equipment, ZoneEqSizing cooling and heating fan
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.28, sizedValue, 0.0001); // max of FinalZoneSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    state->dataSize->ZoneEqSizing(1).SizingMethod(int(sizer.sizingType)) = DataSizing::FractionOfAutosizedHeatingAirflow;

    // Test 18 - Zone Equipment, set fraction of autosized cooling flow for cooling only fan
    state->dataSize->DataFracOfAutosizedCoolingAirflow = 0.4;
    state->dataSize->ZoneCoolingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.64, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 19 - Zone Equipment, set fraction of autosized heating flow for heating only fan
    state->dataSize->ZoneCoolingOnlyFan = false;
    state->dataSize->ZoneHeatingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.48, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 20 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing CoolingAirFlow is set
    state->dataSize->ZoneHeatingOnlyFan = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.88, sizedValue, 0.0001); // max of ZoneEqSizing cooling flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 21 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing HeatingAirFlow is set
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.28, sizedValue, 0.0001); // max of ZoneEqSizing heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 22 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing Cooling/HeatingAirFlow are set
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.28, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = false;

    // Test 23 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing Cooling/HeatingAirFlow not set
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.64, sizedValue, 0.0001); // max of FinalZoneSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;            // reset for next test

    state->dataSize->ZoneEqSizing(1).SizingMethod(int(sizer.sizingType)) = DataSizing::FlowPerCoolingCapacity;

    // Test 24 - Zone Equipment, set fraction of autosized cooling capacity for cooling only fan
    state->dataSize->DataFlowPerCoolingCapacity = 0.00005;
    state->dataSize->DataAutosizedCoolingCapacity = 10000.0;
    state->dataSize->DataFlowPerHeatingCapacity = 0.00006;
    state->dataSize->DataAutosizedHeatingCapacity = 20000.0;
    state->dataSize->ZoneCoolingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.5, sizedValue, 0.0001); // flow per cooling capacity
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 25 - Zone Equipment, set fraction of autosized heating flow for heating only fan
    state->dataSize->ZoneCoolingOnlyFan = false;
    state->dataSize->ZoneHeatingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // flow per heating capacity
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 26 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing CoolingAirFlow is set
    state->dataSize->ZoneHeatingOnlyFan = false;
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.5, sizedValue, 0.0001); // max of ZoneEqSizing cooling capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 27 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing HeatingAirFlow is set
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // ZoneEqSizing heating capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 28 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing Cooling/HeatingAirFlow are set
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // max of ZoneEqSizing cooling/heating flow * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = false;

    // Test 29 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing Cooling/HeatingAirFlow not set
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // max of autosized cooling/heating capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test

    state->dataSize->ZoneEqSizing(1).SizingMethod(int(sizer.sizingType)) = DataSizing::FlowPerHeatingCapacity;

    // Test 30 - Zone Equipment, set fraction of autosized cooling capacity for cooling only fan
    state->dataSize->ZoneCoolingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.5, sizedValue, 0.0001); // flow per cooling capacity
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 31 - Zone Equipment, set fraction of autosized heating flow for heating only fan
    state->dataSize->ZoneCoolingOnlyFan = false;
    state->dataSize->ZoneHeatingOnlyFan = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // flow per heating capacity
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 32 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing CoolingAirFlow is set
    state->dataSize->ZoneHeatingOnlyFan = false;
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.5, sizedValue, 0.0001); // max of autosized cooling capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 33 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing HeatingAirFlow is set
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // autosized heating capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 34 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing Cooling/HeatingAirFlow are set
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // max of autosized cooling/heating capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test
    state->dataSize->ZoneEqSizing(1).CoolingAirFlow = false;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = false;

    // Test 35 - Zone Equipment, cooling or heating fan not set, ZoneEqSizing Cooling/HeatingAirFlow not set
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.2, sizedValue, 0.0001); // max of autosized cooling/heating capacity * fraction
    sizer.autoSizedValue = 0.0;           // reset for next test

    // Test 36 - Zone Equipment, set design size from parent
    state->dataSize->ZoneEqSizing(1).DesignSizeFromParent = true;
    state->dataSize->ZoneEqSizing(1).AirVolFlow = 1.75;
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.75, sizedValue, 0.0001); // parent passed size
    sizer.autoSizedValue = 0.0;            // reset for next test
    state->dataSize->ZoneEqSizing(1).DesignSizeFromParent = false;

    // Test 37 - Zone Equipment, hard size with zone sizing run
    inputValue = 1.44;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(1.44, sizedValue, 0.0001); // hard sized result
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 38 - Zone Equipment, hard size
    inputValue = 1.44;
    state->dataSize->ZoneSizingRunDone = false;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(1.44, sizedValue, 0.0001); // hard sized result
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 39 - EMS override
    state->dataSize->DataEMSOverrideON = true;
    state->dataSize->DataEMSOverride = 1.33;
    inputValue = 1.44;
    state->dataSize->ZoneSizingRunDone = false;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(1.33, sizedValue, 0.0001);          // override result
    EXPECT_NEAR(1.44, sizer.originalValue, 0.0001); // original input
    sizer.autoSizedValue = 0.0;                     // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - no reporting, CurDuctType not set
    // Test 40 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;
    // baseFlags.otherEqType = false; set in initialize function based on other flags
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 5.0;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 41 - Airloop Equipment - ems override is on
    state->dataSize->SysSizingRunDone = true;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataEnvrn->TotDesDays = 2;
    state->dataSize->SysSizPeakDDNum.allocate(2);
    state->dataSize->SysSizPeakDDNum(1).CoolFlowPeakDD = 1;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk.allocate(2);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk(1) = 12;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk(2) = 6;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(1).HeatDDNum = 2;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 5.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.33, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test
    state->dataSize->DataEMSOverrideON = false;

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Flow Rate [m3/s], 1.33000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 42 - Airloop Equipment - CurDuctType = HVAC::AirDuctType::Main
    state->dataSize->CurDuctType = HVAC::AirDuctType::Main;
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 5.0;
    state->dataSize->FinalSysSizing(1).DesCoolVolFlow = 5.0;
    state->dataSize->FinalSysSizing(1).SysAirMinFlowRat = 0.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 43 - Airloop Equipment - AirLoopSysFlag set
    state->dataSize->UnitarySysEqSizing.allocate(1);
    state->dataSize->UnitarySysEqSizing(1).CoolingCapacity = true;
    state->dataSize->UnitarySysEqSizing(1).CoolingAirFlow = true;
    state->dataSize->UnitarySysEqSizing(1).CoolingAirVolFlow = 6.0;
    state->dataSize->UnitarySysEqSizing(1).HeatingAirFlow = true;
    state->dataSize->UnitarySysEqSizing(1).HeatingAirVolFlow = 7.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(7.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 44 - Airloop Equipment - AirLoopSysFlag set, heating only
    state->dataSize->UnitarySysEqSizing(1).CoolingAirFlow = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(7.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 45 - Airloop Equipment - AirLoopSysFlag set, cooling only
    state->dataSize->UnitarySysEqSizing(1).CoolingAirFlow = true;
    state->dataSize->UnitarySysEqSizing(1).HeatingAirFlow = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(6.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // OUTDOOR AIR SYSTEM EQUIPMENT TESTING
    // Test 46 - Outdoor Air System HX Equipment
    state->dataSize->HRFlowSizingFlag = true;
    state->dataSize->FinalSysSizing(1).DesHeatVolFlow = 0.0;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 3.0;
    state->dataSize->OASysEqSizing.allocate(1);
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataSize->CurOASysNum = 1;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 47 - Air Loop HX Equipment, CurDuctType = Main
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses main flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 48 - Air Loop HX Equipment, CurDuctType = Cooling
    state->dataSize->CurDuctType = HVAC::AirDuctType::Cooling;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses cooling flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 49 - Air Loop HX Equipment, CurDuctType = HVAC::AirDuctType::Heating
    state->dataSize->CurDuctType = HVAC::AirDuctType::Heating;
    state->dataSize->FinalSysSizing(1).DesHeatVolFlow = 8.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(8.0, sizedValue, 0.01); // uses heating flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 50 - Air Loop HX Equipment, CurDuctType = HVAC::AirDuctType::Other
    state->dataSize->CurDuctType = HVAC::AirDuctType::Other;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses a main flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 51 - Air Loop HX Equipment, CurDuctType = HVAC::AirDuctType::Main
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurDuctType = HVAC::AirDuctType::Main;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses main flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 52 - Air Loop HX Equipment, CurDuctType = Cooling
    state->dataSize->CurDuctType = HVAC::AirDuctType::Cooling;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses cooling flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 53 - Air Loop HX Equipment, CurDuctType = HVAC::AirDuctType::Heating
    state->dataSize->CurDuctType = HVAC::AirDuctType::Heating;
    state->dataSize->FinalSysSizing(1).DesHeatVolFlow = 8.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(8.0, sizedValue, 0.01); // uses heating flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 54 - Air Loop HX Equipment, CurDuctType = HVAC::AirDuctType::Other
    state->dataSize->CurDuctType = HVAC::AirDuctType::Other;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses a main flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test 55 - Air Loop HX Equipment, CurDuctType = HVAC::AirDuctType::RAB
    state->dataSize->CurDuctType = HVAC::AirDuctType::RAB;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // uses a main flow rate
    sizer.autoSizedValue = 0.0;         // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 56 - Air Loop HX Equipment, CurDuctType = RAB, hard-sized air flow rate
    // start with an hard-sized value as the user input
    inputValue = 2.0;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // cumulative of previous calls
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Flow Rate [m3/s], 5.00000\n"
                            " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Flow Rate [m3/s], 2.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 57 - Air Loop HX Equipment, CurDuctType = RAB, hard-sized air flow rate
    // start with an hard-sized value as the user input
    inputValue = 2.2;
    state->dataSize->DataConstantUsedForSizing = 3.5;
    state->dataSize->DataFractionUsedForSizing = 1.0;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::Coil_HeatingWater), "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // cumulative of previous calls
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2.2, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test
    state->dataSize->DataConstantUsedForSizing = 0.0;
    state->dataSize->DataFractionUsedForSizing = 0.0;

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Flow Rate [m3/s], 3.50000\n"
                            " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Flow Rate [m3/s], 2.20000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 58 - DOAS fan air flow rate autosizing
    state->dataSize->HRFlowSizingFlag = false;
    state->dataSize->CurOASysNum = 1;
    state->dataAirLoop->OutsideAirSys(1).AirLoopDOASNum = 0;
    AirLoopHVACDOAS::AirLoopDOAS thisDOAS;
    state->dataAirLoopHVACDOAS->airloopDOAS.push_back(thisDOAS);
    // use 0.53 m3/s multiplied by StdRhoAir = 1.2
    state->dataAirLoopHVACDOAS->airloopDOAS[state->dataAirLoop->OutsideAirSys(1).AirLoopDOASNum].SizingMassFlow = 0.53 * 1.2;
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, "Fan:SystemModel", "MyDOASFan", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // cumulative of previous calls
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.53, sizedValue, 0.01); // auto-sized value
}

} // namespace EnergyPlus
