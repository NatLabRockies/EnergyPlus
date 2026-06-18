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

#include <EnergyPlus/Autosizing/CoolingWaterDesAirOutletHumRatSizing.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, CoolingWaterDesAirOutletHumRatSizingGauntlet)
{
    // this global state is what would be set up by E+ currently
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataEnvrn->StdRhoAir = 1.2;
    static constexpr std::string_view routineName("CoolingWaterDesAirOutletHumRatSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    CoolingWaterDesAirOutletHumRatSizer sizer;
    Real64 inputValue = 0.006;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.01); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING
    state->dataSize->CurZoneEqNum = 1;

    // reset eio stream
    has_eio_output(true);

    // Test #1 - Zone Equipment, no autosizing, TU type not set
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.006, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    std::string eiooutput = std::string("");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test #2 - Zone Equipment, no autosizing, TU type not set
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.006, sizedValue, 0.001); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    eiooutput = std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                            " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Design Outlet Air Humidity Ratio "
                            "[kgWater/kgDryAir], 6.000E-03\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    // now allocate sizing arrays for testing autosized field
    state->dataSize->FinalZoneSizing.allocate(1);

    // Sizing Type Prerequisites:
    // TermUnitIU
    state->dataSize->DataDesInletAirHumRat = 0.009;
    state->dataSize->DataDesOutletAirTemp = 12.0;
    state->dataSize->DataDesInletWaterTemp = 7.0;
    // All other TU types
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.008;

    state->dataSize->ZoneSizingRunDone = true;

    // Test 3 - Zone Equipment, Single Duct TU
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.008, sizedValue, 0.0001);

    eiooutput = std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Outlet Air Humidity Ratio "
                            "[kgWater/kgDryAir], 8.000E-03\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Zone Equipment, Powered Induction TU
    state->dataSize->TermUnitSingDuct = false;
    state->dataSize->TermUnitPIU = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.008, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 5 - Zone Equipment, Zone Eq Fan Coil
    state->dataSize->TermUnitPIU = false;
    state->dataSize->ZoneEqFanCoil = true;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.008, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 6 - Zone Equipment, Induction Unit
    state->dataSize->TermUnitPIU = false;
    state->dataSize->TermUnitIU = true;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    // humidity ratio at 12 C and 90% RH
    EXPECT_NEAR(0.0078, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test
    state->dataSize->TermUnitIU = false;

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - CurDuctType not set, no reporting
    // Test 9 - Airloop Equipment
    // delete zone sizing info
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 0.012;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.012, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;             // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 10 - Airloop Equipment - no OA coils
    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.0105; // set SA hum rat > inlet air hum rat
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    // outlet air hum rat > inlet air hum rat and capped at inlet air hum rat
    EXPECT_GT(state->dataSize->FinalSysSizing(1).CoolSupHumRat, state->dataSize->DataDesInletAirHumRat);
    EXPECT_NEAR(state->dataSize->DataDesInletAirHumRat, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Outlet Air Humidity Ratio "
                            "[kgWater/kgDryAir], 9.000E-03\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 11 - Airloop Equipment - 1 OA coil, use PrecoolHumRat
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumOACoolCoils = 1;
    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.0075;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0075, sizedValue, 0.00001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 12 - Airloop Equipment - 1 OA coil use previously set hum rat since DataDesOutletAirHumRat > 0
    state->dataSize->DataDesOutletAirHumRat = 0.0077; // system coil outlet air hum rat set
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0077, sizedValue, 0.00001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // OUTDOOR AIR SYSTEM EQUIPMENT TESTING
    // Test 13 - Outdoor Air System Equipment, no DOAS air loop
    state->dataSize->OASysEqSizing.allocate(1);
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataSize->CurOASysNum = 1;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).OutHumRatAtCoolPeak = 0.005;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).PrecoolHumRat = 0.004;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    Real64 precoolHumRat = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).PrecoolHumRat;
    EXPECT_NEAR(precoolHumRat, sizedValue, 0.00001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 14 - Outdoor Air System Equipment with DOAS system
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.0;
    state->dataAirLoop->OutsideAirSys(1).AirLoopDOASNum = 0;
    state->dataAirLoopHVACDOAS->airloopDOAS.emplace_back();
    state->dataAirLoopHVACDOAS->airloopDOAS[0].PrecoolHumRat = 0.0036;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0036, sizedValue, 0.00001); // DOAS system hum rat
    sizer.autoSizedValue = 0.0;               // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 15 - Outdoor Air System Equipment with DOAS system, hard-sized humidity ratio
    inputValue = 0.00665; // value not previously used

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // cumulative of previous calls
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(inputValue, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;                // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Outlet Air Humidity Ratio "
                            "[kgWater/kgDryAir], 3.600E-03\n"
                            " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Design Outlet Air Humidity Ratio "
                            "[kgWater/kgDryAir], 6.650E-03\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    // Test 16 - Repeat w/ dry coil, Outdoor Air System Equipment with DOAS system, autosized humidity ratio
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    state->dataSize->DataDesInletAirHumRat = 0.003; // set up prior to sizing outlet hum rat, dryer than autosized outlet

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(state->dataSize->DataDesInletAirHumRat, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;                                              // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Outlet Air Humidity Ratio "
                            "[kgWater/kgDryAir], 3.000E-03\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 17 - Repeat w/ dry coil and high water temp, Outdoor Air System Equipment with DOAS system, autosized humidity ratio
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    state->dataSize->DataDesInletAirHumRat = 0.010;
    state->dataSize->DataDesInletWaterTemp = 12.0;
    state->dataSize->DataDesInletAirHumRat = 0.008;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(state->dataSize->DataDesInletAirHumRat, sizedValue, 0.0001); // hard-sized value
    EXPECT_NEAR(0.008, sizedValue, 0.0001);                                  // hum rat set to inlet hum rat
    sizer.autoSizedValue = 0.0;                                              // reset for next test
}

} // namespace EnergyPlus
