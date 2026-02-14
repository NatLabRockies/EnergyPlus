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

#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/SimAirServingZones.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, CoolingCapacitySizingGauntlet)
{
    // fan heat is needed for DOAS systems
    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    MyFan,                       !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    0.2,                         !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    100.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                    !- Design Electric Power Consumption",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                            !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    0.50;                        !- Fan Total Efficiency",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->StdRhoAir = 1.2;
    // call simulate to trigger sizing call
    Fans::GetFanInput(*state);
    state->dataLoopNodes->Node(1).Press = 101325.0;
    state->dataLoopNodes->Node(1).Temp = 24.0;
    state->dataFans->fans(1)->simulate(*state, false, _, _);

    // this global state is what would be set up by E+ currently
    static constexpr std::string_view routineName("CoolingCapacitySizingGauntlet");

    state->dataSize->ZoneEqSizing.allocate(1);

    // create the sizer and set up the flags to specify the sizing configuration
    CoolingCapacitySizer sizer;
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

    // Test #1 - Zone Equipment, no autosizing
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5125.3, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;

    state->dataSize->ZoneEqSizing(1).DesignSizeFromParent = true;
    state->dataSize->ZoneEqSizing(1).DesCoolingLoad = sizedValue;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5125.3, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;            // reset for next test
    state->dataSize->ZoneEqSizing(1).DesignSizeFromParent = false;

    // eio header reported in fan sizing
    std::string eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Cooling Design Capacity [W], 5125.30000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // now allocate sizing arrays for testing autosized field
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).DesCoolMassFlow = 0.2;
    state->dataSize->FinalZoneSizing(1).DesCoolCoilInTemp = 24.0;
    state->dataSize->FinalZoneSizing(1).DesCoolCoilInHumRat = 0.009;
    state->dataSize->FinalZoneSizing(1).CoolDesTemp = 7.0;
    state->dataSize->FinalZoneSizing(1).CoolDesHumRat = 0.006;
    state->dataSize->FinalZoneSizing(1).ZoneRetTempAtCoolPeak = 22.0;
    state->dataSize->FinalZoneSizing(1).ZoneTempAtCoolPeak = 23.0;
    state->dataSize->FinalZoneSizing(1).ZoneHumRatAtCoolPeak = 0.008;

    state->dataSize->ZoneEqSizing(1).ATMixerCoolPriDryBulb = 20.0;
    state->dataSize->ZoneEqSizing(1).ATMixerCoolPriHumRat = 0.007;

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataSize->DataWaterLoopNum = 1;
    state->dataSize->DataWaterCoilSizHeatDeltaT = 5.0;

    // Test 2 - Zone Equipment, Single Duct TU
    state->dataSize->TermUnitSingDuct = true;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.zoneSizingInput.allocate(1);
    sizer.zoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4981.71, sizedValue, 0.01);
    EXPECT_NEAR(1.2, state->dataEnvrn->StdRhoAir, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 3 - Zone Equipment, Zone Eq Fan Coil
    state->dataSize->TermUnitSingDuct = false;
    state->dataSize->ZoneEqFanCoil = true;
    state->dataSize->ZoneEqSizing(1).DesCoolingLoad = 4000.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4000.0, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 4 - Zone Equipment, Zone Eq Fan Coil, HX assisted water coil
    state->dataSize->ZoneEqSizing(1).DesCoolingLoad = 0.0;
    state->dataSize->DataFlowUsedForSizing = state->dataSize->FinalZoneSizing(1).DesCoolMassFlow / state->dataEnvrn->StdRhoAir;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWaterHXAssisted], "MyHXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4268.66, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 5 - Zone Equipment, Terminal Induction Unit
    state->dataSize->TermUnitIU = true;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).DesCoolingLoad = 3500.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3500.0, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 6 - Zone Equipment, OA flow > 0
    state->dataSize->ZoneEqFanCoil = false;
    state->dataSize->ZoneEqSizing(1).OAVolFlow = 0.05;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingDXSingleSpeed], "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4981.71, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 7 - Zone DX Equipment, inlet side AT Mixer
    state->dataSize->ZoneEqSizing(1).ATMixerVolFlow = 0.03;
    state->dataSize->ZoneEqDXCoil = true;
    // start with an auto-sized value as the user input, AT Mixer present
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingDXSingleSpeed], "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3899.81, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test
    state->dataSize->ZoneEqSizing(1).ATMixerVolFlow = 0.0;

    // Test 8 - Zone DX Equipment, no AT Mixer, local OA only
    state->dataSize->ZoneEqSizing(1).ATMixerVolFlow = 0.0;
    // start with an auto-sized value as the user input, AT Mixer present
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingDXSingleSpeed], "MyDXCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4981.71, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test
    state->dataSize->ZoneEqSizing(1).OAVolFlow = 0.0;

    // reset eio stream
    has_eio_output(true);

    // Test 9 - Zone Equipment, hard size
    // start with an auto-sized value as the user input
    inputValue = 5500.0;
    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5500.0, sizedValue, 0.01); // hard size value
    sizer.autoSizedValue = 0.0;            // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Cooling Design Capacity [W], 3500.00000\n"
                    " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Cooling Design Capacity [W], 5500.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    eiooutput = "";

    // AIRLOOP Equipment
    // Test 10 - Airloop Equipment, hard size, no sizing run
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
    inputValue = 2700.8;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(2700.8, sizedValue, 0.0001); // hard-sized value
    sizer.autoSizedValue = 0.0;              // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    state->dataSize->SysSizingRunDone = true;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 7.0;
    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.006;
    state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak = 24.0;
    state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak = 0.009;
    state->dataSize->FinalSysSizing(1).RetTempAtCoolPeak = 25.0;
    state->dataSize->FinalSysSizing(1).RetHumRatAtCoolPeak = 0.0085;
    state->dataSize->FinalSysSizing(1).OutTempAtCoolPeak = 35.0;

    // Test 11 - Airloop Equipment, no OA
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4981.71, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 12 - Airloop Equipment, with OA but no precooling of OA stream
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.02;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4981.71, sizedValue, 0.01); // no change in capacity because coil is in air loop
    sizer.autoSizedValue = 0.0;             // reset for next test

    // Test 13 - Airloop Equipment, with OA and precooling of OA stream
    state->dataAirSystemsData->PrimaryAirSystems(1).NumOACoolCoils = 1;
    state->dataSize->FinalSysSizing(1).PrecoolTemp = 12.0;
    state->dataSize->FinalSysSizing(1).PrecoolHumRat = 0.008;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4582.31, sizedValue, 0.01); // change in capacity because precool conditions mixed with return
    sizer.autoSizedValue = 0.0;             // reset for next test

    // add fan heat
    auto *fan1 = new Fans::FanComponent;

    fan1->Name = "CONSTANT FAN 1";
    fan1->deltaPress = 600.0;
    fan1->motorEff = 0.9;
    fan1->totalEff = 0.6;
    fan1->motorInAirFrac = 0.5;
    fan1->type = HVAC::FanType::Constant;

    state->dataFans->fans.push_back(fan1);
    state->dataFans->fanMap.insert_or_assign(fan1->Name, state->dataFans->fans.size());

    state->dataAirSystemsData->PrimaryAirSystems(1).supFanNum = Fans::GetFanIndex(*state, "CONSTANT FAN 1");
    state->dataAirSystemsData->PrimaryAirSystems(1).supFanType = HVAC::FanType::Constant;
    state->dataSize->DataFanPlacement = HVAC::FanPlace::BlowThru;

    // Test 14 - Airloop Equipment, with OA and precooling of OA stream, add fan heat
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4740.64, sizedValue, 0.01);                              // change in capacity because precool conditions mixed with return
    EXPECT_NEAR(158.33, sizer.primaryAirSystem(1).FanDesCoolLoad, 0.01); // air loop fan heat is saved in sizer class
    EXPECT_NEAR(state->dataSize->DataCoilSizingAirInTemp, 23.44, 0.01);  // does not include fan heat because PrimaryAirSys fan place not set
    EXPECT_EQ(1.0, state->dataSize->DataFracOfAutosizedCoolingCapacity);
    sizer.autoSizedValue = 0.0; // reset for next test
    Real64 unScaledCapacity = sizedValue;

    // Test 15 - Airloop Equipment, with OA and precooling of OA stream, add fan heat, add scalable capacity sizing
    state->dataSize->FinalSysSizing(1).CoolingCapMethod = DataSizing::FractionOfAutosizedCoolingCapacity;
    state->dataSize->FinalSysSizing(1).FractionOfAutosizedCoolingCapacity = 0.5;
    state->dataAirSystemsData->PrimaryAirSystems(1).supFanPlace = HVAC::FanPlace::BlowThru;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(unScaledCapacity * 0.5, sizedValue, 0.01);               // change in capacity because precool conditions mixed with return
    EXPECT_NEAR(158.33, sizer.primaryAirSystem(1).FanDesCoolLoad, 0.01); // air loop fan heat is saved in sizer class
    EXPECT_EQ(1.0, state->dataSize->DataFracOfAutosizedCoolingCapacity); // Data global is not affected
    EXPECT_NEAR(state->dataSize->DataCoilSizingAirInTemp, 24.22, 0.01);  // does include fan heat because PrimaryAirSys fan place is set
    EXPECT_EQ(0.5, sizer.dataFracOfAutosizedCoolingCapacity);            // sizer class holds fractional value
    sizer.autoSizedValue = 0.0;                                          // reset for next test

    // Test 16 - Airloop Equipment, with OA and precooling of OA stream, add scalable capacity per floor area sizing
    state->dataSize->FinalSysSizing(1).CoolingCapMethod = DataSizing::CapacityPerFloorArea;
    state->dataSize->FinalSysSizing(1).CoolingTotalCapacity = 4500.0;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(4500.0, sizedValue, 0.01); // capacity precalculated and saved in FinalSysSizing(1).CoolingTotalCapacity
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 17 - Airloop Equipment, with OA and precooling of OA stream, add scalable capacity sizing
    state->dataSize->FinalSysSizing(1).CoolingCapMethod = DataSizing::CoolingDesignCapacity;
    state->dataSize->FinalSysSizing(1).CoolingTotalCapacity = 3500.0;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3500.0, sizedValue, 0.01); // capacity precalculated and saved in FinalSysSizing(1).CoolingTotalCapacity
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 18 - Airloop Equipment, with OA and precooling of OA stream, add scalable capacity sizing
    state->dataSize->UnitarySysEqSizing.allocate(1);
    state->dataSize->UnitarySysEqSizing(1).CoolingCapacity = true;
    state->dataSize->UnitarySysEqSizing(1).DesCoolingLoad = 2500.0;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(2500.0, sizedValue, 0.01); // capacity precalculated and saved in UnitarySysEqSizing(1).DesCoolingLoad
    sizer.autoSizedValue = 0.0;            // reset for next test
    state->dataSize->UnitarySysEqSizing(1).CoolingCapacity = false;

    // Test 19 - OA Equipment, OA Sys capacity sizing
    state->dataSize->CurOASysNum = 1;
    state->dataSize->OASysEqSizing.allocate(1);
    state->dataSize->OASysEqSizing(1).CoolingCapacity = true;
    state->dataSize->OASysEqSizing(1).DesCoolingLoad = 1500.0;
    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(1500.0, sizedValue, 0.01); // capacity precalculated and saved in OASysEqSizing(1).DesCoolingLoad
    sizer.autoSizedValue = 0.0;            // reset for next test

    // Test 20 - OA Equipment, DOAS Air loop
    state->dataSize->OASysEqSizing(1).CoolingCapacity = false;
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataAirLoop->OutsideAirSys(1).AirLoopDOASNum = 0;
    state->dataAirLoopHVACDOAS->airloopDOAS.emplace_back();
    state->dataAirLoopHVACDOAS->airloopDOAS[0].SizingMassFlow = 0.2;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].SizingCoolOATemp = 32.0;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanIndex = Fans::GetFanIndex(*state, "MYFAN");
    state->dataAirLoopHVACDOAS->airloopDOAS[0].FanBeforeCoolingCoilFlag = true;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanTypeNum = SimAirServingZones::CompType::Fan_System_Object;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].SizingCoolOAHumRat = 0.009;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].PrecoolTemp = 12.0;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].PrecoolHumRat = 0.006;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanInletNodeNum = 1;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanOutletNodeNum = 2;

    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5634.12, sizedValue, 0.01); // capacity includes system fan heat
    sizer.autoSizedValue = 0.0;             // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 21 - OA Equipment, DOAS Air loop
    // start with an autosized value
    inputValue = 4200.0;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(4200.0, sizedValue, 0.01); // hard sized capacity
    sizer.autoSizedValue = 0.0;            // reset for next test

    EXPECT_FALSE(errorsFound);

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, Design Size Cooling Design Capacity [W], 5634.11835\n"
                    " Component Sizing Information, Coil:Cooling:Water, MyWaterCoil, User-Specified Cooling Design Capacity [W], 4200.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 22 - OA Equipment, DOAS Air loop with no fan heat for cooling capacity
    state->dataSize->OASysEqSizing(1).CoolingCapacity = false;
    state->dataAirLoop->OutsideAirSys.allocate(1);
    state->dataAirLoop->OutsideAirSys(1).AirLoopDOASNum = 0;
    state->dataAirLoopHVACDOAS->airloopDOAS.emplace_back();
    state->dataAirLoopHVACDOAS->airloopDOAS[0].SizingMassFlow = 0.2;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].SizingCoolOATemp = 32.0;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanIndex = Fans::GetFanIndex(*state, "MYFAN");
    state->dataAirLoopHVACDOAS->airloopDOAS[0].FanBeforeCoolingCoilFlag = false;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanTypeNum = SimAirServingZones::CompType::Fan_System_Object;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].SizingCoolOAHumRat = 0.009;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].PrecoolTemp = 12.0;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].PrecoolHumRat = 0.006;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanInletNodeNum = 1;
    state->dataAirLoopHVACDOAS->airloopDOAS[0].m_FanOutletNodeNum = 2;

    // start with an autosized value
    inputValue = DataSizing::AutoSize;
    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::CoolingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*this->state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(5633.933, sizedValue, 0.01); // capacity includes system fan heat
    sizer.autoSizedValue = 0.0;              // reset for next test
}

} // namespace EnergyPlus
