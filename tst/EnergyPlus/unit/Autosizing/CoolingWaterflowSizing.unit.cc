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

#include <EnergyPlus/Autosizing/CoolingWaterflowSizing.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, CoolingWaterflowSizingGauntlet)
{
    state->dataFluid->init_state(*state);
    // this global state is what would be set up by E+ currently
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataSize->ZoneEqSizing.allocate(1);
    static constexpr std::string_view routineName("CoolingWaterflowSizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    CoolingWaterflowSizer sizer;
    Real64 inputValue = 5;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.01); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING
    inputValue = 5;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitSingDuct = true;
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->TermUnitSizing(1).MaxCWVolFlow = 0.005;

    // Test #1 - Zone Equipment, no autosizing, missing input data
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test

    // Test #2 - Zone Equipment, no autosizing, has input data
    state->dataSize->DataWaterLoopNum = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataSize->DataWaterCoilSizCoolDeltaT = 10.0;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                    " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Design Water Flow Rate [m3/s], 5.00000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    // now allocate sizing arrays for testing autosized field
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(1).MaxCWVolFlow = 0.3;
    state->dataSize->FinalZoneSizing(1).DesCoolMassFlow = 0.3;
    state->dataSize->FinalZoneSizing(1).CoolDesTemp = 12.0;
    state->dataSize->FinalZoneSizing(1).CoolDesHumRat = 0.006;
    state->dataSize->FinalZoneSizing(1).DesCoolCoilInTemp = 25.0;
    state->dataSize->FinalZoneSizing(1).DesCoolCoilInHumRat = 0.009;
    state->dataSize->ZoneSizingRunDone = true;

    // Test 3 - Zone Equipment, Single Duct TU, cooling coil water flow sizes > 0
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.000149, sizedValue, 0.000001);
    EXPECT_NEAR(1.2, state->dataEnvrn->StdRhoAir, 0.01);
    Real64 previousWaterFlow = sizedValue;
    sizer.autoSizedValue = 0.0; // reset for next test
    sizedValue = 0.0;

    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Water Flow Rate [m3/s], 0.000149018\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Zone Equipment, Single Duct TU, add fan heat
    auto *fan1 = new Fans::FanComponent;

    fan1->Name = "CONSTANT FAN 1";
    fan1->deltaPress = 600.0;
    fan1->motorEff = 0.9;
    fan1->totalEff = 0.6;
    fan1->motorInAirFrac = 0.5;
    fan1->type = HVAC::FanType::Constant;

    state->dataFans->fans.push_back(fan1);
    state->dataFans->fanMap.insert_or_assign(fan1->Name, state->dataFans->fans.size());

    state->dataSize->DataFanIndex = Fans::GetFanIndex(*state, "CONSTANT FAN 1");
    state->dataSize->DataFanType = HVAC::FanType::Constant;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // missing Data* globals and Plant data
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.000154, sizedValue, 0.000001);
    EXPECT_GT(sizedValue, previousWaterFlow); // water flow rate is greater with fan heat than without

    // test fan heat
    Real64 desVolFlow = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMassFlow / state->dataEnvrn->StdRhoAir;
    Real64 desFanHeat = sizer.calcFanDesHeatGain(desVolFlow);
    EXPECT_FALSE(sizer.fanCompModel); // fan is not a component model type
    EXPECT_NEAR(237.5, desFanHeat, 0.001);
    Real64 fanPowerTot = (desVolFlow * 600.0) / 0.6;
    Real64 designFanHeatGain = 0.9 * fanPowerTot + (fanPowerTot - 0.9 * fanPowerTot) * 0.5;
    EXPECT_NEAR(designFanHeatGain, desFanHeat, 0.0001); // compare with manual fan heat calculation

    // Test 5 - Zone Equipment, Powered Induction TU
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
    EXPECT_NEAR(0.000154, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 6 - Zone Equipment, Induction TU
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
    EXPECT_NEAR(0.005, sizedValue, 0.0001);
    EXPECT_NEAR(state->dataSize->TermUnitSizing(1).MaxCWVolFlow, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 7 - Zone Equipment, Zone Eq Fan Coil
    state->dataSize->TermUnitIU = false;
    state->dataSize->ZoneEqFanCoil = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.3, sizedValue, 0.0001);
    EXPECT_NEAR(state->dataSize->ZoneEqSizing(1).MaxCWVolFlow, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 8 - Zone Equipment, Unit Ventilator
    state->dataSize->ZoneEqFanCoil = false;
    state->dataSize->ZoneEqUnitVent = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.3, sizedValue, 0.0001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 9 - Zone Equipment, Other Equipment
    state->dataSize->ZoneEqUnitVent = false;
    state->dataSize->DataWaterCoilSizCoolDeltaT = 5.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.000309, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - CurDuctType not set, no reporting
    // Test 10 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;
    state->dataSize->CurTermUnitSizingNum = 0;
    // baseFlags.otherEqType = false; set in initialize function based on other flags
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();

    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizingRunDone = false;
    state->dataSize->DataCapacityUsedForSizing = 5000.0;
    // start with a hard-sized value as the user input, no system sizing arrays
    inputValue = 5.0;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 11 - Airloop Equipment - CurDuctType not set
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 5.0; // CurDuctType not set
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(0.000238, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Water Flow Rate [m3/s], 0.000238237\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // OUTDOOR AIR SYSTEM EQUIPMENT TESTING
    // Test 13 - Outdoor Air System Equipment, no DOAS air loop
    state->dataSize->CurOASysNum = 1;
    state->dataSize->OASysEqSizing.allocate(1);
    // start with an auto-sized value as the user input
    inputValue = 0.0002;
    printFlag = true;
    errorsFound = false;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(0.0002, sizedValue, 0.000001);
    sizer.autoSizedValue = 0.0; // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Design Water Flow Rate [m3/s], 0.000476474\n"
                    " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Design Water Flow Rate [m3/s], 0.000200000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // call the clearState
    sizer.clearState();
}

} // namespace EnergyPlus
