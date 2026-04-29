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

#include <EnergyPlus/Autosizing/HeatingWaterDesCoilLoadUsedForUASizing.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, HeatingWaterDesCoilLoadUsedForUASizingGauntlet)
{
    state->dataFluid->init_state(*state);
    // this global state is what would be set up by E+ currently
    state->dataEnvrn->StdRhoAir = 1.2;
    static constexpr std::string_view routineName("HeatingWaterDesCoilLoadUsedForUASizingGauntlet");

    state->dataSize->ZoneEqSizing.allocate(1);

    // create the sizer and set up the flags to specify the sizing configuration
    HeatingWaterDesCoilLoadUsedForUASizer sizer;
    Real64 inputValue = 5125.3;
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
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitSingDuct = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing (this sizer is not typically hard-sized)
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5125.3, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5125.3, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    std::string eiooutput = std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                                        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Water Heating Design Coil "
                                        "Load for UA Sizing [W], 5125.30000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // now allocate sizing arrays for testing autosized field
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing(1).DesHeatCoilInTempTU = 15.0;
    state->dataSize->TermUnitFinalZoneSizing(1).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->TermUnitSizing(1).AirVolFlow = 0.0005;
    state->dataSize->TermUnitSizing(1).ReheatAirFlowMult = 1.0;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->FinalZoneSizing(1).ZoneRetTempAtHeatPeak = 24.0;
    state->dataSize->FinalZoneSizing(1).ZoneHumRatAtHeatPeak = 0.007;
    state->dataSize->FinalZoneSizing(1).ZoneHumRatAtHeatPeak = 0.006;
    state->dataSize->FinalZoneSizing(1).DesHeatMassFlow = 0.2;
    state->dataSize->FinalZoneSizing(1).HeatDesTemp = 30.0;
    state->dataSize->FinalZoneSizing(1).HeatDesHumRat = 0.004;
    state->dataSize->FinalZoneSizing(1).OutTempAtHeatPeak = 5.0;
    state->dataSize->FinalZoneSizing(1).OutHumRatAtHeatPeak = 0.002;
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(1).MaxHWVolFlow = 0.0002;
    state->dataSize->ZoneEqSizing(1).ATMixerHeatPriDryBulb = 28.0;
    state->dataSize->ZoneEqSizing(1).ATMixerHeatPriHumRat = 0.0045;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataSize->DataWaterLoopNum = 1;
    state->dataSize->DataWaterCoilSizHeatDeltaT = 5.0;
    state->dataSize->DataWaterFlowUsedForSizing = 0.0002;
    state->dataSize->ZoneSizingRunDone = true;

    // Test 2 - Zone Equipment, Single Duct TU
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.zoneSizingInput.allocate(1);
    sizer.zoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4114.69, sizedValue, 0.01);
    EXPECT_NEAR(1.2, state->dataEnvrn->StdRhoAir, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 4 - Zone Equipment, Zone Eq Fan Coil
    state->dataSize->TermUnitSingDuct = false;
    state->dataSize->ZoneEqFanCoil = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4114.69, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 5 - Zone Equipment, Zone Eq Fan Coil
    state->dataSize->ZoneEqFanCoil = false;
    state->dataSize->TermUnitIU = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4114.69, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    state->dataSize->ZoneEqSizing(1).OAVolFlow = 0.05;
    // Test 6 - Zone Equipment, Other Equipment
    state->dataSize->TermUnitIU = false;
    // start with an auto-sized value as the user input, OA flow > 0
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2935.6, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 7 - Zone Equipment, Other Equipment
    state->dataSize->ZoneEqSizing(1).ATMixerVolFlow = 0.03;
    // start with an auto-sized value as the user input, AT Mixer present
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1068.96, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test
    state->dataSize->ZoneEqSizing(1).OAVolFlow = 0.0;
    state->dataSize->ZoneEqSizing(1).ATMixerVolFlow = 0.0;

    // Test 8 - Zone Equipment, Other Equipment
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SystemAirFlow = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow = 0.3;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3644.19, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 9 - Zone Equipment, Other Equipment
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SystemAirFlow = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow = 0.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow = 0.4;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4858.92, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 10 - Zone Equipment, Other Equipment
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirFlow = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow = 0.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2024.55, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 11 - Zone Equipment, Other Equipment, heating sizing ratio = 0.5
    // start with a hard-sized value as the user input
    inputValue = 1500.0;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(1500.0, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Water Heating Design Coil Load for UA Sizing [W], 2024.55160\n"
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Water Heating Design Coil Load for UA Sizing [W], "
        "1500.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING
    // Test 12 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;
    state->dataSize->CurTermUnitSizingNum = 0;
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 5000.0;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5000.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 13 - Airloop Equipment - sizing arrays set
    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 10.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 24.0;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 30.0;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataSize->DataAirFlowUsedForSizing = 0.6;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(14469.96, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Water Heating Design Coil Load for UA "
                            "Sizing [W], 14469.96369\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 14 - Airloop Equipment, heating sizing ratio = 1.25
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatingCapMethod = DataSizing::FractionOfAutosizedHeatingCapacity;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity = 1.25;

    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    Real64 expectedValue = 14469.96 * state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity;
    EXPECT_NEAR(expectedValue, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity = 1.0;

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Water Heating Design Coil Load for UA "
                            "Sizing [W], 18087.45461\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // Test 15 - Airloop Equipment, 50% OA, MinOA option
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatOAOption = DataSizing::OAControl::MinOA;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = state->dataSize->DataAirFlowUsedForSizing / 2.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).PreheatTemp = 19.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(9405.48, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 16 - Airloop Equipment - desiccant regen heating coil
    state->dataSize->DataDesicRegCoil = true;
    state->dataSize->DataDesOutletAirTemp = 20.0;
    state->dataSize->DataDesInletAirTemp = 10.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(7234.98, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 17 - Air Loop Equipment, DOAS air loop w/ heating coil
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumOAHeatCoils = 1;
    state->dataSize->DataDesicRegCoil = false;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(6149.73, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // OUTDOOR AIR SYSTEM EQUIPMENT TESTING
    // Test 18 - Outdoor Air System Equipment, DOAS air loop
    state->dataSize->OASysEqSizing.allocate(1);
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataSize->CurOASysNum = 1;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(6511.48, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 19 - Outdoor Air System Equipment with DOAS system
    state->dataAirLoop->OutsideAirSys(state->dataSize->CurOASysNum).AirLoopDOASNum = 0;
    state->dataAirLoopHVACDOAS->airloopDOAS.emplace_back();
    state->dataAirLoopHVACDOAS->airloopDOAS[0].HeatOutTemp = 8.0;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].PreheatTemp = 15.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5064.49, sizedValue, 0.01); // uses a mass flow rate for sizing
    sizer.autoSizedValue = 0.0;             // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 20 - Outdoor Air System Equipment with DOAS system, hard-sized temperature
    inputValue = 7000.0;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // cumulative of previous calls
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(7000.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size Water Heating Design Coil Load for UA Sizing [W], 5064.48729\n"
        " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified Water Heating Design Coil Load for UA Sizing [W], "
        "7000.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}

} // namespace EnergyPlus
