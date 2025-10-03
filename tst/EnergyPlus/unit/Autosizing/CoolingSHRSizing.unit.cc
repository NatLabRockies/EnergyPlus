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

#include "AutosizingFixture.hh"
#include <gtest/gtest.h>

#include <EnergyPlus/Autosizing/CoolingSHRSizing.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, CoolingSHRSizingGauntlet)
{
    // this global state is what would be set up by E+ currently
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    static constexpr std::string_view routineName("CoolingSHRSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    CoolingSHRSizer sizer;
    Real64 inputValue = 0.75;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.01); // unitialized sizing types always return 0
    EXPECT_FALSE(sizer.hardSizeNoDesignRun);
    EXPECT_FALSE(sizer.sizingDesRunThisZone);
    EXPECT_FALSE(sizer.sizingDesRunThisAirSys);
    EXPECT_FALSE(sizer.sizingDesValueFromParent);
    EXPECT_FALSE(sizer.airLoopSysFlag);
    EXPECT_FALSE(sizer.oaSysFlag);
    errorsFound = false;

    // DX Coil SHR Sizing
    inputValue = 0.85;
    state->dataSize->CurZoneEqNum = 1;

    // Test #1 - Zone Equipment, no autosizing, missing input data
    state->dataSize->DataFlowUsedForSizing = 0.5; // DataCapacityUsedForSizing not initialized
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // no sizing error since not autosized
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.85, sizedValue, 0.01); // default value on error
    EXPECT_FALSE(sizer.sizingDesRunThisZone);
    EXPECT_EQ(sizer.sizingString, "Gross Rated Sensible Heat Ratio");
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test #2 - Zone Equipment, no autosizing, has input data, print result
    printFlag = true;
    inputValue = 0.85;
    state->dataSize->DataCapacityUsedForSizing = 10000.0;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.85, sizedValue, 0.01); // hard-sized value
    Real64 RatedVolFlowPerRatedTotCap = state->dataSize->DataFlowUsedForSizing / state->dataSize->DataCapacityUsedForSizing;
    Real64 initialSHR = 0.431 + 6086.0 * RatedVolFlowPerRatedTotCap; // does not include impact of ValidateADP, which increases SHR
    EXPECT_LT(initialSHR, sizedValue);                               // hard-sized value > autosized value
    EXPECT_FALSE(sizer.sizingDesRunThisZone);
    sizer.autoSizedValue = 0.0; // reset for next test

    std::string eiooutput = std::string(
        "! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
        " Component Sizing Information, Coil:Cooling:DX:SingleSpeed, MyDXCoil, User-Specified Gross Rated Sensible Heat Ratio, 0.85000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    // Test #3 - Zone Equipment, autosized
    // start with an auto-sized value as the user input
    state->dataSize->ZoneSizingRunDone = true;
    inputValue = DataSizing::AutoSize;
    state->dataHVACGlobal->DXCT = HVAC::DXCoilType::Regular;
    // do sizing
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.776167, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test
    sizedValue = 0.0;

    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:DX:SingleSpeed, MyDXCoil, Design Size Gross Rated Sensible Heat Ratio, 0.77617\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test #4 - Zone Equipment, repeat with autosized and no initialized DataCapacityUsedForSizing
    inputValue = DataSizing::AutoSize;
    state->dataSize->DataCapacityUsedForSizing = 0.0;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(errorsFound);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.0, sizedValue, 0.01); // autosizing failed since Data* variables were not both > 0

    // Test #5 - Zone Equipment, flow to capacity ratio high
    state->dataSize->DataCapacityUsedForSizing = 10000.0;
    state->dataSize->DataFlowUsedForSizing = 1.0;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // missing Data* globals and Plant data
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.0, sizedValue, 0.000001); // includes impact of ValidateADP
    initialSHR = 0.431 + 6086.0 * HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state->dataHVACGlobal->DXCT];
    // does not include impact of ValidateADP, which increases SHR
    EXPECT_LT(initialSHR, sizedValue); // includes impact of ValidateADP
    sizer.autoSizedValue = 0.0;        // reset for next test

    // Test #6 - Zone Equipment, flow to capacity ratio low
    state->dataSize->DataFlowUsedForSizing = 0.1;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    initialSHR =
        0.431 +
        6086.0 * HVAC::MinRatedVolFlowPerRatedTotCap[(int)state->dataHVACGlobal->DXCT]; // does not include impact of ValidateADP, which increases SHR
    EXPECT_GT(initialSHR, sizedValue);                                                  // compares impact of ValidateADP
    EXPECT_NEAR(0.676083, initialSHR, 0.000001);                                        // does not include impact of ValidateADP
    EXPECT_NEAR(0.675083, sizedValue, 0.000001);                                        // includes impact of ValidateADP
    sizer.autoSizedValue = 0.0;                                                         // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test #7 - Zone Equipment, autosized
    state->dataSize->DataFlowUsedForSizing = 0.3;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    state->dataHVACGlobal->DXCT = HVAC::DXCoilType::DOAS;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    RatedVolFlowPerRatedTotCap = state->dataSize->DataFlowUsedForSizing / state->dataSize->DataCapacityUsedForSizing;
    initialSHR = 0.389 + 7684.0 * RatedVolFlowPerRatedTotCap;
    EXPECT_NEAR(0.61952, initialSHR, 0.000001);  // does not include impact of ValidateADP
    EXPECT_NEAR(0.631462, sizedValue, 0.000001); // includes impact of ValidateADP
    sizer.autoSizedValue = 0.0;                  // reset for next test
    sizedValue = 0.0;

    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:DX:SingleSpeed, MyDXCoil, Design Size Gross Rated Sensible Heat Ratio, 0.63146\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test #8 - Zone Equipment, flow to capacity ratio high
    state->dataSize->DataFlowUsedForSizing = 1.0;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // missing Data* globals and Plant data
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1.0, sizedValue, 0.000001); // includes impact of ValidateADP
    initialSHR =
        0.431 +
        6086.0 * HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state->dataHVACGlobal->DXCT]; // does not include impact of ValidateADP, which increases SHR
    EXPECT_LT(initialSHR, sizedValue);                                                  // includes impact of ValidateADP
    sizer.autoSizedValue = 0.0;                                                         // reset for next test

    // Test #9 - Zone Equipment, flow to capacity ratio low
    state->dataSize->DataFlowUsedForSizing = 0.1;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    initialSHR =
        0.431 +
        6086.0 * HVAC::MinRatedVolFlowPerRatedTotCap[(int)state->dataHVACGlobal->DXCT]; // does not include impact of ValidateADP, which increases SHR
    EXPECT_LT(initialSHR, sizedValue);                                                  // compares impact of ValidateADP
    EXPECT_NEAR(0.533062, initialSHR, 0.000001);                                        // does not include impact of ValidateADP
    EXPECT_NEAR(0.675925, sizedValue, 0.000001);                                        // includes impact of ValidateADP
    sizer.autoSizedValue = 0.0;                                                         // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING
    // Test #10 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;
    state->dataSize->ZoneEqSizing.deallocate();

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 0.67;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.67, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test #11 - Airloop Equipment
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->DataFlowUsedForSizing = 0.5; // flow to capacity ratio within limits
    state->dataHVACGlobal->DXCT = HVAC::DXCoilType::Regular;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.776167, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:DX:SingleSpeed, MyDXCoil, Design Size Gross Rated Sensible Heat Ratio, 0.77617\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test #12 - Airloop Equipment
    state->dataSize->DataFlowUsedForSizing = 0.6; // flow to capacity ratio high
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.856504, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test #13 - Airloop Equipment
    state->dataSize->DataFlowUsedForSizing = 0.1; // flow to capacity ratio low
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.675083, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // OUTDOOR AIR SYSTEM EQUIPMENT TESTING
    // Test #14 - Outdoor Air System Equipment, no DOAS air loop
    state->dataSize->CurOASysNum = 1;
    state->dataSize->OASysEqSizing.allocate(1);
    // start with an auto-sized value as the user input
    inputValue = 0.52;
    printFlag = true;
    errorsFound = false;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::cAllCoilTypes(HVAC::CoilDX_CoolingSingleSpeed), "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.52, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Cooling:DX:SingleSpeed, MyDXCoil, Design Size Gross Rated Sensible Heat Ratio, 0.67508\n"
        " Component Sizing Information, Coil:Cooling:DX:SingleSpeed, MyDXCoil, User-Specified Gross Rated Sensible Heat Ratio, 0.52000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

} // namespace EnergyPlus
