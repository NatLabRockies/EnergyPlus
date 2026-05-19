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

#include <EnergyPlus/Autosizing/WaterHeatingCoilUASizing.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

TEST_F(AutoSizingFixture, WaterHeatingCoilUASizingGauntlet)
{
    state->dataFluid->init_state(*state);
    // this global state is what would be set up by E+ currently
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataSize->ZoneEqSizing.allocate(1);
    static constexpr std::string_view routineName("WaterHeatingCoilUASizingGauntlet");

    // create the sizer and set up the flags to specify the sizing configuration
    WaterHeatingCoilUASizer sizer;
    Real64 inputValue = 35;
    bool errorsFound = false;
    bool printFlag = false;

    // uninitialized sizing type
    Real64 sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType2, sizer.errorType);
    EXPECT_NEAR(0.0, sizedValue, 0.01); // uninitialized sizing types always return 0
    errorsFound = false;

    // ZONE EQUIPMENT TESTING
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitSingDuct = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);

    // Test #1 - Zone Equipment, no autosizing
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(35.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;          // reset for next test

    // reset eio stream
    has_eio_output(true);
    printFlag = true;

    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(35.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;          // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified U-Factor Times Area Value [W/K], 35.0000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // reset eio stream
    has_eio_output(true);

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidIndex = 1;
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataWaterCoils->WaterCoil.allocate(1);
    state->dataWaterCoils->WaterCoil(1).InletAirTemp = 21.0;
    state->dataWaterCoils->WaterCoil(1).InletAirHumRat = 0.006;
    state->dataWaterCoils->WaterCoil(1).Control = state->dataWaterCoils->DesignCalc;
    state->dataWaterCoils->WaterCoil(1).InletWaterTemp = 60.0;
    state->dataWaterCoils->WaterCoil(1).InletAirMassFlowRate = 0.2;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = 0.8;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loop = &state->dataPlnt->PlantLoop(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->WaterCoil(1).availSched = Sched::GetScheduleAlwaysOn(*state);

    // now allocate sizing arrays for testing autosized field
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->TermUnitSizing(1).AirVolFlow = 0.0008;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).HeatDesTemp = 30.0;
    state->dataSize->FinalZoneSizing(1).HeatDesHumRat = 0.007;
    state->dataSize->FinalZoneSizing(1).ZoneName = "MyZone";
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).ExitTemp = 60.0;

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loop = &state->dataPlnt->PlantLoop(1);

    state->dataSize->ZoneSizingRunDone = true;

    // Test 2 - Zone Equipment
    state->dataSize->DataCapacityUsedForSizing = 3000.0;
    state->dataSize->DataWaterFlowUsedForSizing = 0.0002;
    state->dataSize->DataFlowUsedForSizing = 0.2;
    state->dataSize->DataCoilNum = 1;
    state->dataSize->DataFanOp = HVAC::FanOp::Continuous;
    state->dataSize->DataPltSizHeatNum = 1;
    state->dataSize->DataWaterCoilSizHeatDeltaT = 5.0;
    state->dataSize->DataDesInletAirTemp = 21.0;
    state->dataSize->DataDesInletAirHumRat = 0.009;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(98.35, sizedValue, 0.01);

    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size U-Factor Times Area Value [W/K], 98.3510\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 2 - Zone Equipment, UA sizing fails
    state->dataWaterCoils->WaterCoil(1).InletAirTemp = 61.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;
    // do sizing
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_TRUE(errorsFound);
    EXPECT_TRUE(state->dataSize->DataErrorsFound);
    EXPECT_TRUE(sizer.dataErrorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(3.0, sizedValue, 0.01); // 0.1% of 3000 W capacity
    state->dataWaterCoils->WaterCoil(1).InletAirTemp = 21.0;
    state->dataSize->DataErrorsFound = false;
    sizer.dataErrorsFound = false;
    errorsFound = false;

    // reset eio stream
    has_eio_output(true);
    eiooutput = "";

    // AIRLOOP EQUIPMENT TESTING - CurDuctType not set, no reporting
    // Test 3 - Airloop Equipment
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->NumZoneSizingInput = 0;
    state->dataSize->CurTermUnitSizingNum = 0;
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
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 4 - Airloop Equipment
    state->dataSize->CurSysNum = 1;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->NumSysSizInput = 1;
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
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(98.35, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput = std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size U-Factor Times Area Value [W/K], 98.3510\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

    // Test 5 - Airloop Equipment
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_NEAR(98.35, sizedValue, 0.01);
    sizer.autoSizedValue = 0.0; // reset for next test

    // Test 5 - Airloop Equipment, failed UA sizing
    state->dataWaterCoils->WaterCoil(1).InletAirTemp = 61.0;
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::ErrorType1, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    EXPECT_TRUE(errorsFound);
    EXPECT_TRUE(state->dataSize->DataErrorsFound);
    EXPECT_TRUE(sizer.dataErrorsFound);
    EXPECT_NEAR(3.0, sizedValue, 0.01); // 0.1% of 3000 W capacity
    state->dataWaterCoils->WaterCoil(1).InletAirTemp = 21.0;
    state->dataSize->DataErrorsFound = false;
    sizer.dataErrorsFound = false;
    errorsFound = false;
    sizer.autoSizedValue = 0.0; // reset for next test

    // OUTDOOR AIR SYSTEM EQUIPMENT TESTING
    // Test 6 - Outdoor Air System Equipment, no DOAS air loop
    state->dataSize->CurOASysNum = 1;
    state->dataSize->OASysEqSizing.allocate(1);
    // start with an auto-sized value as the user input
    inputValue = DataSizing::AutoSize;

    // do sizing
    sizer.wasAutoSized = false;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType);
    EXPECT_TRUE(sizer.wasAutoSized);
    // tolerance was 0.01, this is a root finding algo sensitivity (RegulaFalsi finds solution at -0.00008
    // RegulaFalsiThenBisection finds it at 0.00008).
    EXPECT_NEAR(98.35, sizedValue, 0.03);
    sizer.autoSizedValue = 0.0; // reset for next test

    // reset eio stream
    has_eio_output(true);

    // Test 7 - Outdoor Air System
    // start with an auto-sized value as the user input
    inputValue = 5.0;

    // do sizing
    sizer.wasAutoSized = false;
    printFlag = true;
    sizer.initializeWithinEP(*this->state, HVAC::coilTypeNames[(int)HVAC::CoilType::HeatingWater], "MyWaterCoil", printFlag, routineName);
    sizedValue = sizer.size(*state, inputValue, errorsFound);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, sizer.errorType); // cumulative of previous calls
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;         // reset for next test

    EXPECT_FALSE(errorsFound);

#ifdef GET_OUT
    // <Component Sizing Information> header already reported above (and flag set false). Only coil sizing information reported here.
    eiooutput =
        std::string(" Component Sizing Information, Coil:Heating:Water, MyWaterCoil, Design Size U-Factor Times Area Value [W/K], 98.3510\n"
                    " Component Sizing Information, Coil:Heating:Water, MyWaterCoil, User-Specified U-Factor Times Area Value [W/K], 5.00000\n");
    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
    // How to compare these sizing strings if they have some tolerance on them?
    // Maybe not use the optimizing SolveRoot on sizing routines?  Stick to RegulaFalsi?
#endif // GET_OUT
}

} // namespace EnergyPlus
