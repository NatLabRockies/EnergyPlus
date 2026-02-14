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

#include "AutosizingFixture.hh"
#include <gtest/gtest.h>

#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DesiccantDehumidifiers.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, AutoCalculateSizingGauntlet)
{
    static constexpr std::string_view routineName("AutoCalculateSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    AutoCalculateSizer sizer;
    Real64 inputValue = 37.5;
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

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001); // returns 0 since Data* variables are not set
    sizer.autoSizedValue = 0.0;           // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    // set autocalculate data constants and sizing string
    state->dataSize->DataFractionUsedForSizing = 1.0;
    state->dataSize->DataConstantUsedForSizing = 30.0;
    std::string sizingString = "Any sizing that requires AutoCalculate []";
    sizer.overrideSizingString(sizingString);

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(37.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    std::string eiooutput = std::string(
        "! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Any sizing that requires AutoCalculate [], 30.00000\n"
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Any sizing that requires AutoCalculate [], 37.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // must reset Data globals after a sizer run is executed
    state->dataSize->DataFractionUsedForSizing = 1.0;
    state->dataSize->DataConstantUsedForSizing = 30.0;

    // Test 2 - start with autosized value
    sizer.wasAutoSized = false;
    inputValue = DataSizing::AutoSize;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(30.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 3 - start with autosized value, Data globals were not set again
    sizer.wasAutoSized = false;
    inputValue = DataSizing::AutoSize;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType); // Data globals weren't set
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001); // result is 0 since Data globals are 0
    sizer.autoSizedValue = 0.0;           // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 4 - EMS override on
    state->dataSize->DataEMSOverrideON = true;
    state->dataSize->DataEMSOverride = 33.4;
    sizer.wasAutoSized = false;
    inputValue = 28.8;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(33.4, sizedValue, 0.0001); // EMS overrides input value
    sizer.autoSizedValue = 0.0;            // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Any sizing that requires AutoCalculate [], 33.40000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    // Test 5 - EMS override on, autosized input
    sizer.wasAutoSized = false;
    inputValue = DataSizing::AutoSize;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(33.4, sizedValue, 0.0001); // EMS overrides input value
    sizer.autoSizedValue = 0.0;            // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Any sizing that requires AutoCalculate [], 33.40000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, MaxHeaterOutletTempSizingGauntlet)
{
    static constexpr std::string_view routineName("MaxHeaterOutletTempSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    MaxHeaterOutletTempSizer sizer;
    Real64 inputValue = 37.5;
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

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(37.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(37.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Supply Air Temperature [C], 37.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 32.6;
    state->dataSize->ZoneSizingRunDone = true;

    // Test 2 - Zone Equipment, Single Duct TU (doesn't matter)
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(32.6, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - no reporting
    // Test 3 - Airloop Equipment
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
    inputValue = 27.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(27.8, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Airloop Equipment - CurDuctType not set
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatSupTemp = 25.8;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(25.8, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Supply Air Temperature [C], 25.80000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    sizer.wasAutoSized = false;
    inputValue = 28.8;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(28.8, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Supply Air Temperature [C], 25.80000\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Supply Air Temperature [C], 28.80000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, ZoneCoolingLoadSizingGauntlet)
{
    static constexpr std::string_view routineName("ZoneCoolingLoadSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    ZoneCoolingLoadSizer sizer;
    Real64 inputValue = 3007.5;
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

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(3007.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;              // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(3007.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;             // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                    " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Zone Cooling Sensible Load [W], 3007.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolLoad = 2500.0;
    state->dataSize->ZoneSizingRunDone = true;

    // Test 2 - Zone Equipment, Single Duct TU (doesn't matter)
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2500.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - no reporting
    // Test 3 - Airloop Equipment
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
    inputValue = 2007.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2007.8, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;              // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatSupTemp = 25.8;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Zone Cooling Sensible Load [W], 0.00000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    sizer.wasAutoSized = false;
    inputValue = 2880.0;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2880.0, sizedValue, 0.0001);
    EXPECT_EQ(sizer.autoSizedValue, 2880.0);
    EXPECT_EQ(sizer.originalValue, 2880.0);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Zone Cooling Sensible Load [W], 2880.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, ZoneHeatingLoadSizingGauntlet)
{
    static constexpr std::string_view routineName("ZoneHeatingLoadSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    ZoneHeatingLoadSizer sizer;
    Real64 inputValue = 3007.5;
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

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(3007.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;              // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(3007.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;             // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Zone Heating Sensible Load [W], 3007.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatLoad = 2500.0;
    state->dataSize->ZoneSizingRunDone = true;

    // Test 2 - Zone Equipment, Single Duct TU (doesn't matter)
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2500.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - CurDuctType not set, no reporting
    // Test 3 - Airloop Equipment
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
    inputValue = 2007.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2007.8, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;              // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Zone Heating Sensible Load [W], 0.00000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    sizer.wasAutoSized = false;
    inputValue = 2880.0;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2880.0, sizedValue, 0.0001);
    EXPECT_EQ(sizer.autoSizedValue, 2880.0);
    EXPECT_EQ(sizer.originalValue, 2880.0);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Zone Heating Sensible Load [W], 2880.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, ASHRAEMinSATCoolingSizingGauntlet)
{
    static constexpr std::string_view routineName("ASHRAEMinSATCoolingSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    ASHRAEMinSATCoolingSizer sizer;
    Real64 inputValue = 16.5;
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

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(16.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(16.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    std::string eiooutput = std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                                        " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Minimum Supply Air "
                                        "Temperature in Cooling Mode [C], 16.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).ZoneTempAtCoolPeak = 23.9;
    state->dataSize->FinalZoneSizing(1).ZoneHumRatAtCoolPeak = 0.009;
    state->dataSize->DataCapacityUsedForSizing = 2500.0;
    state->dataSize->DataFlowUsedForSizing = 0.125; // = 0.00005 m3/s/W
    state->dataSize->ZoneSizingRunDone = true;
    state->dataEnvrn->StdRhoAir = 1.2;

    // Test 2 - Zone Equipment
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(7.585, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 3 - Zone Equipment, DataFlowUsedForSizing = 0.0
    state->dataSize->DataFlowUsedForSizing = 0.0; // not allowed
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - no reporting
    // Test 4 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 14.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(14.8, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    state->dataSize->DataZoneUsedForSizing = 1;

    // do sizing, DataAirFlowUsedForSizing = 0
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(errorsFound);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test
    errorsFound = false;

    // reset eio stream
    has_eio_output(true);

    // Test 6 - Airloop Equipment
    // start with an auto-sized value as the user input
    state->dataSize->DataFlowUsedForSizing = 0.125; // = 0.00005 m3/s/W
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(7.585, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Minimum Supply Air Temperature in Cooling Mode [C], 7.58525\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 7 - Airloop Equipment
    sizer.wasAutoSized = false;
    inputValue = 9.0;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(9.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Minimum Supply Air Temperature in Cooling Mode [C], 7.58525\n"
        " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Minimum Supply Air Temperature in Cooling Mode [C], "
        "9.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, ASHRAEMaxSATHeatingSizingGauntlet)
{
    static constexpr std::string_view routineName("ASHRAEMaxSATHeatingSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    ASHRAEMaxSATHeatingSizer sizer;
    Real64 inputValue = 26.5;
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

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(26.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(26.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    std::string eiooutput = std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                                        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Supply Air "
                                        "Temperature in Heating Mode [C], 26.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).ZoneTempAtHeatPeak = 21.9;
    state->dataSize->FinalZoneSizing(1).ZoneHumRatAtHeatPeak = 0.007;
    state->dataSize->DataCapacityUsedForSizing = 2500.0;
    state->dataSize->DataFlowUsedForSizing = 0.125; // = 0.00005 m3/s/W
    state->dataSize->ZoneSizingRunDone = true;
    state->dataEnvrn->StdRhoAir = 1.2;

    // Test 2 - Zone Equipment
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(38.274, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 3 - Zone Equipment, DataFlowUsedForSizing = 0
    state->dataSize->DataFlowUsedForSizing = 0.0; // not allowed
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING
    state->dataSize->DataFlowUsedForSizing = 0.125; // = 0.00005 m3/s/W
    // Test 3 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 32.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(32.8, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    state->dataSize->DataZoneUsedForSizing = 1;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(38.274, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Supply Air Temperature in Heating Mode [C], 38.27434\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    sizer.wasAutoSized = false;
    inputValue = 32.3;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(32.3, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Maximum Supply Air Temperature in Heating Mode [C], 38.27434\n"
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Maximum Supply Air Temperature in Heating Mode [C], "
        "32.30000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // test sizer method failure with DataCapacityUsedForSizing = 0
    // Test 6 - Airloop Equipment
    sizer.wasAutoSized = false;
    state->dataSize->DataCapacityUsedForSizing = 0.0;
    inputValue = DataSizing::AutoSize;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test
}

TEST_F(AutoSizingFixture, DesiccantDehumidifierBFPerfDataFaceVelocitySizingGauntlet)
{
    static constexpr std::string_view routineName("DesiccantDehumidifierBFPerfDataFaceVelocitySizingGauntlet");
    static constexpr std::string_view compType("HeatExchanger:Desiccant:BalancedFlow:PerformanceDataType1");

    // create the sizer and set up the flags to specify the sizing configuration
    DesiccantDehumidifierBFPerfDataFaceVelocitySizer sizer;
    Real64 inputValue = 4.5;
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

    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(4.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(4.5, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;          // reset for next test

    std::string eiooutput = std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                                        " Component Sizing Information, " +
                                        std::string{compType} + ", MyDesiccantHX, User-Specified Nominal Air Face Velocity [m/s], 4.50000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->DataAirFlowUsedForSizing = 0.125; // = 0.00005 m3/s/W
    state->dataSize->ZoneSizingRunDone = true;

    // Test 2 - Zone Equipment
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4.307, sizedValue, 0.001);
    Real64 expectedValue = 4.30551 + 0.01969 * state->dataSize->DataAirFlowUsedForSizing;
    EXPECT_NEAR(expectedValue, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 3 - Zone Equipment, EMS override ON
    state->dataSize->DataEMSOverrideON = true;
    state->dataSize->DataEMSOverride = 2.887;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2.887, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING
    state->dataSize->DataEMSOverrideON = false;
    // Test 4 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 3.2;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(3.2, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;          // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    state->dataSize->DataZoneUsedForSizing = 1;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4.307, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only HX sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, " + std::string{compType} +
                            ", MyDesiccantHX, Design Size Nominal Air Face Velocity [m/s], 4.30797\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 6 - Airloop Equipment
    sizer.wasAutoSized = false;
    inputValue = 3.2;
    sizer.initializeWithinEP(*this->state, compType, "MyDesiccantHX", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(3.2, sizedValue, 0.001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only HX sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, " + std::string{compType} +
                            ", MyDesiccantHX, Design Size Nominal Air Face Velocity [m/s], 4.30797\n"
                            " Component Sizing Information, " +
                            std::string{compType} + ", MyDesiccantHX, User-Specified Nominal Air Face Velocity [m/s], 3.20000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, HeatingCoilDesAirInletTempSizingGauntlet)
{
    static constexpr std::string_view routineName("HeatingCoilDesAirInletTempSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    HeatingCoilDesAirInletTempSizer sizer;
    Real64 inputValue = 17.5;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.0001); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING - Zone equipment not supported
    state->dataSize->CurZoneEqNum = 1;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(17.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // AIRLOOP EQUIPMENT TESTING - no reporting
    // Test 2 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 17.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(17.8, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 3 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatRetTemp = 15.8;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatOutTemp = 13.8;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // DataDesicRegCoil not set
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 4 - Airloop Equipment w/ desiccant regen coil
    // do sizing
    state->dataSize->DataDesicRegCoil = true;
    state->dataSize->DataDesicDehumNum = 1;
    state->dataDesiccantDehumidifiers->DesicDehum.allocate(1);
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(15.8, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n Component Sizing "
                    "Information, Coil:Heating:Water, MyWaterCoil, Design Size Rated Inlet Air Temperature, 15.80000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment w/ desiccant regen coil in OA system
    state->dataDesiccantDehumidifiers->DesicDehum(1).RegenInletIsOutsideAirNode = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(13.8, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0;

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // Test 6 - Airloop Equipment - hard-sized desiccant regen coil in OA system
    sizer.wasAutoSized = false;
    inputValue = 19.8;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(19.8, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Rated Inlet Air Temperature, 13.80000\n"
                            " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Rated Inlet Air Temperature, 19.80000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, HeatingCoilDesAirOutletTempSizingGauntlet)
{
    static constexpr std::string_view routineName("HeatingCoilDesAirOutletTempSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    HeatingCoilDesAirOutletTempSizer sizer;
    Real64 inputValue = 37.5;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.0001); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING - Zone equipment not supported
    state->dataSize->CurZoneEqNum = 1;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(37.5, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // AIRLOOP EQUIPMENT TESTING - no reporting
    // Test 2 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 27.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(27.8, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 3 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatRetTemp = 15.8;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatOutTemp = 13.8;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // DataDesicRegCoil not set
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 4 - Airloop Equipment w/ desiccant regen coil
    // do sizing
    state->dataSize->DataDesicRegCoil = true;
    state->dataSize->DataDesicDehumNum = 1;
    state->dataDesiccantDehumidifiers->DesicDehum.allocate(1);
    state->dataDesiccantDehumidifiers->DesicDehum(1).RegenSetPointTemp = 26.4;
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(26.4, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n Component Sizing "
                    "Information, Coil:Heating:Water, MyWaterCoil, Design Size Rated Outlet Air Temperature, 26.40000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment - hard-sized desiccant regen coil in OA system
    sizer.wasAutoSized = false;
    inputValue = 32.8;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(32.8, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Rated Outlet Air Temperature, 26.40000\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Rated Outlet Air Temperature, 32.80000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

TEST_F(AutoSizingFixture, HeatingCoilDesAirInletHumRatSizingGauntlet)
{
    static constexpr std::string_view routineName("HeatingCoilDesAirInletHumRatSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    HeatingCoilDesAirInletHumRatSizer sizer;
    Real64 inputValue = 0.005;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.0001); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING - Zone equipment not supported
    state->dataSize->CurZoneEqNum = 1;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.005, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;             // reset for next test

    // AIRLOOP EQUIPMENT TESTING - no reporting
    // Test 2 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 0.008;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.008, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;             // reset for next test

    // Test 3 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatRetHumRat = 0.008;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatOutHumRat = 0.004;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // DataDesicRegCoil not set
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 4 - Airloop Equipment w/ desiccant regen coil
    // do sizing
    state->dataSize->DataDesicRegCoil = true;
    state->dataSize->DataDesicDehumNum = 1;
    state->dataDesiccantDehumidifiers->DesicDehum.allocate(1);
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.008, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n Component Sizing "
                    "Information, Coil:Heating:Water, MyWaterCoil, Design Size Rated Inlet Air Humidity Ratio, 8.00000E-003\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment w/ desiccant regen coil in OA system
    state->dataDesiccantDehumidifiers->DesicDehum(1).RegenInletIsOutsideAirNode = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.004, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0;

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // Test 6 - Airloop Equipment - hard-sized desiccant regen coil in OA system
    sizer.wasAutoSized = false;
    inputValue = 0.009;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.009, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Rated Inlet Air Humidity Ratio, 4.00000E-003\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Rated Inlet Air Humidity Ratio, 9.00000E-003\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

} // namespace EnergyPlus
