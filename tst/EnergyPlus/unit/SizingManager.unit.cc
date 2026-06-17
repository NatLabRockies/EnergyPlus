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

// EnergyPlus::ExteriorEnergyUse Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::SizingManager;
using namespace EnergyPlus::DataSizing;

TEST_F(EnergyPlusFixture, GetOARequirementsTest_DSOA1)
{
    state->init_state(*state);
    bool ErrorsFound(false); // If errors detected in input
    int OAIndex(0);          // Zone number
    int NumAlphas(2);
    int NumNumbers(4);

    std::string CurrentModuleObject = "DesignSpecification:OutdoorAir";
    int NumOARequirements = 6;
    state->dataSize->OARequirements.allocate(NumOARequirements);

    Array1D_string Alphas;         // Alpha input items for object
    Array1D_string cAlphaFields;   // Alpha field names
    Array1D_string cNumericFields; // Numeric field names
    Array1D<Real64> Numbers;       // Numeric input items for object
    Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.

    Alphas.allocate(NumAlphas);
    cAlphaFields.allocate(NumAlphas);
    cNumericFields.allocate(NumNumbers);
    Numbers.dimension(NumNumbers, 0.0);
    lAlphaBlanks.dimension(NumAlphas, true);
    lNumericBlanks.dimension(NumNumbers, true);

    // Flow/Area
    OAIndex = 1;
    Alphas(1) = "Test DSOA 1"; // Name
    Alphas(2) = "Flow/Area";   // Outdoor Air Method
    Numbers(1) = 0.1;          // Outdoor Air Flow per Person{ m3 / s - person }
    Numbers(2) = 0.2;          // Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }
    Numbers(3) = 0.3;          // Outdoor Air Flow per Zone{ m3 / s }
    Numbers(4) = 0.4;          // Outdoor Air Flow Air Changes per Hour

    ErrorsFound = false;
    ProcessInputOARequirements(*state, CurrentModuleObject, OAIndex, Alphas, NumAlphas, Numbers, NumNumbers, lAlphaBlanks, cAlphaFields, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    EXPECT_ENUM_EQ(OAFlowCalcMethod::PerArea, state->dataSize->OARequirements(OAIndex).OAFlowMethod);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerPerson);
    EXPECT_EQ(0.2, state->dataSize->OARequirements(OAIndex).OAFlowPerArea);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerZone);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowACH);

    // Flow/Person
    OAIndex = 2;
    Alphas(1) = "Test DSOA 2"; // Name
    Alphas(2) = "Flow/Person"; // Outdoor Air Method
    Numbers(1) = 0.1;          // Outdoor Air Flow per Person{ m3 / s - person }
    Numbers(2) = 0.2;          // Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }
    Numbers(3) = 0.3;          // Outdoor Air Flow per Zone{ m3 / s }
    Numbers(4) = 0.4;          // Outdoor Air Flow Air Changes per Hour

    ErrorsFound = false;
    ProcessInputOARequirements(*state, CurrentModuleObject, OAIndex, Alphas, NumAlphas, Numbers, NumNumbers, lAlphaBlanks, cAlphaFields, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    EXPECT_ENUM_EQ(OAFlowCalcMethod::PerPerson, state->dataSize->OARequirements(OAIndex).OAFlowMethod);
    EXPECT_EQ(0.1, state->dataSize->OARequirements(OAIndex).OAFlowPerPerson);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerArea);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerZone);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowACH);

    // Flow/Zone
    OAIndex = 3;
    Alphas(1) = "Test DSOA 3"; // Name
    Alphas(2) = "Flow/Zone";   // Outdoor Air Method
    Numbers(1) = 0.1;          // Outdoor Air Flow per Person{ m3 / s - person }
    Numbers(2) = 0.2;          // Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }
    Numbers(3) = 0.3;          // Outdoor Air Flow per Zone{ m3 / s }
    Numbers(4) = 0.4;          // Outdoor Air Flow Air Changes per Hour

    ErrorsFound = false;
    ProcessInputOARequirements(*state, CurrentModuleObject, OAIndex, Alphas, NumAlphas, Numbers, NumNumbers, lAlphaBlanks, cAlphaFields, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    EXPECT_ENUM_EQ(OAFlowCalcMethod::PerZone, state->dataSize->OARequirements(OAIndex).OAFlowMethod);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerPerson);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerArea);
    EXPECT_EQ(0.3, state->dataSize->OARequirements(OAIndex).OAFlowPerZone);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowACH);

    // Flow/Zone
    OAIndex = 4;
    Alphas(1) = "Test DSOA 4";     // Name
    Alphas(2) = "AirChanges/Hour"; // Outdoor Air Method
    Numbers(1) = 0.1;              // Outdoor Air Flow per Person{ m3 / s - person }
    Numbers(2) = 0.2;              // Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }
    Numbers(3) = 0.3;              // Outdoor Air Flow per Zone{ m3 / s }
    Numbers(4) = 0.4;              // Outdoor Air Flow Air Changes per Hour

    ErrorsFound = false;
    ProcessInputOARequirements(*state, CurrentModuleObject, OAIndex, Alphas, NumAlphas, Numbers, NumNumbers, lAlphaBlanks, cAlphaFields, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    EXPECT_ENUM_EQ(OAFlowCalcMethod::ACH, state->dataSize->OARequirements(OAIndex).OAFlowMethod);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerPerson);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerArea);
    EXPECT_EQ(0.0, state->dataSize->OARequirements(OAIndex).OAFlowPerZone);
    EXPECT_EQ(0.4, state->dataSize->OARequirements(OAIndex).OAFlowACH);

    // Sum
    OAIndex = 5;
    Alphas(1) = "Test DSOA 5"; // Name
    Alphas(2) = "Sum";         // Outdoor Air Method
    Numbers(1) = 0.1;          // Outdoor Air Flow per Person{ m3 / s - person }
    Numbers(2) = 0.2;          // Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }
    Numbers(3) = 0.3;          // Outdoor Air Flow per Zone{ m3 / s }
    Numbers(4) = 0.4;          // Outdoor Air Flow Air Changes per Hour

    ErrorsFound = false;
    ProcessInputOARequirements(*state, CurrentModuleObject, OAIndex, Alphas, NumAlphas, Numbers, NumNumbers, lAlphaBlanks, cAlphaFields, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    EXPECT_ENUM_EQ(OAFlowCalcMethod::Sum, state->dataSize->OARequirements(OAIndex).OAFlowMethod);
    EXPECT_EQ(0.1, state->dataSize->OARequirements(OAIndex).OAFlowPerPerson);
    EXPECT_EQ(0.2, state->dataSize->OARequirements(OAIndex).OAFlowPerArea);
    EXPECT_EQ(0.3, state->dataSize->OARequirements(OAIndex).OAFlowPerZone);
    EXPECT_EQ(0.4, state->dataSize->OARequirements(OAIndex).OAFlowACH);

    // Maximum
    OAIndex = 6;
    Alphas(1) = "Test DSOA 6"; // Name
    Alphas(2) = "Maximum";     // Outdoor Air Method
    Numbers(1) = 0.1;          // Outdoor Air Flow per Person{ m3 / s - person }
    Numbers(2) = 0.2;          // Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }
    Numbers(3) = 0.3;          // Outdoor Air Flow per Zone{ m3 / s }
    Numbers(4) = 0.4;          // Outdoor Air Flow Air Changes per Hour

    ErrorsFound = false;
    ProcessInputOARequirements(*state, CurrentModuleObject, OAIndex, Alphas, NumAlphas, Numbers, NumNumbers, lAlphaBlanks, cAlphaFields, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);

    EXPECT_ENUM_EQ(OAFlowCalcMethod::Max, state->dataSize->OARequirements(OAIndex).OAFlowMethod);
    EXPECT_EQ(0.1, state->dataSize->OARequirements(OAIndex).OAFlowPerPerson);
    EXPECT_EQ(0.2, state->dataSize->OARequirements(OAIndex).OAFlowPerArea);
    EXPECT_EQ(0.3, state->dataSize->OARequirements(OAIndex).OAFlowPerZone);
    EXPECT_EQ(0.4, state->dataSize->OARequirements(OAIndex).OAFlowACH);

    // Clean up
    state->dataSize->OARequirements.deallocate();
    Alphas.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();
}

TEST_F(EnergyPlusFixture, SizingManagerTest_TimeIndexToHrMinString_test)
{
    state->dataGlobal->MinutesInTimeStep = 15;

    EXPECT_EQ("00:00:00", TimeIndexToHrMinString(*state, 0));
    EXPECT_EQ("00:15:00", TimeIndexToHrMinString(*state, 1));
    EXPECT_EQ("01:45:00", TimeIndexToHrMinString(*state, 7));
    EXPECT_EQ("07:45:00", TimeIndexToHrMinString(*state, 31));
    EXPECT_EQ("19:45:00", TimeIndexToHrMinString(*state, 79));
    EXPECT_EQ("24:00:00", TimeIndexToHrMinString(*state, 96));

    state->dataGlobal->MinutesInTimeStep = 3;

    EXPECT_EQ("00:00:00", TimeIndexToHrMinString(*state, 0));
    EXPECT_EQ("00:03:00", TimeIndexToHrMinString(*state, 1));
    EXPECT_EQ("00:21:00", TimeIndexToHrMinString(*state, 7));
    EXPECT_EQ("01:33:00", TimeIndexToHrMinString(*state, 31));
    EXPECT_EQ("03:57:00", TimeIndexToHrMinString(*state, 79));
    EXPECT_EQ("04:48:00", TimeIndexToHrMinString(*state, 96));
    EXPECT_EQ("16:39:00", TimeIndexToHrMinString(*state, 333));
    EXPECT_EQ("24:00:00", TimeIndexToHrMinString(*state, 480));
}

TEST_F(EnergyPlusFixture, SizingManager_DOASControlStrategyDefaultSpecificationTest)
{
    // checks DOAS Control Strategy default setpoint values test
    std::string const idf_objects = delimited_string({

        " Zone,",
        "	SPACE1-1,      !- Name",
        "	0,             !- Direction of Relative North { deg }",
        "	0,             !- X Origin { m }",
        "	0,             !- Y Origin { m }",
        "	0,             !- Z Origin { m }",
        "	1,             !- Type",
        "	1,             !- Multiplier",
        "	3.0,           !- Ceiling Height {m}",
        "	240.0;         !- Volume {m3}",

        " Sizing:Zone,",
        "	SPACE1-1,             !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14.,                  !- Zone Cooling Design Supply Air Temperature { C }",
        "	,                     !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50.,                  !- Zone Heating Design Supply Air Temperature { C }",
        "	,                     !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009,                !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004,                !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1,     !- Design Specification Outdoor Air Object Name",
        "	0.0,                  !- Zone Heating Sizing Factor",
        "	0.0,                  !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit,   !- Cooling Design Air Flow Method",
        "	,                     !- Cooling Design Air Flow Rate { m3/s }",
        "	,                     !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	,                     !- Cooling Minimum Air Flow { m3/s }",
        "	,                     !- Cooling Minimum Air Flow Fraction",
        "	DesignDay,            !- Heating Design Air Flow Method",
        "	,                     !- Heating Design Air Flow Rate { m3/s }",
        "	,                     !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	,                     !- Heating Maximum Air Flow { m3/s }",
        "	,                     !- Heating Maximum Air Flow Fraction",
        "	,                     !- Design Specification Zone Air Distribution Object Name",
        "   Yes,                  !- Account for Dedicated Outside Air System",
        "   NeutralSupplyAir,     !- Dedicated Outside Air System Control Strategy",
        "   ,                     !- Dedicated Outside Air Low Setpoint for Design",
        "   ;                     !- Dedicated Outside Air High Setpoint for Design",

        " DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1,     !- Name",
        "	sum,                  !- Outdoor Air Method",
        "	0.00236,              !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305,             !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0;                  !- Outdoor Air Flow per Zone { m3/s }",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    SizingManager::GetOARequirements(*state);
    SizingManager::GetZoneSizingInput(*state);
    ASSERT_EQ(1, state->dataSize->NumZoneSizingInput);
    ASSERT_ENUM_EQ(DOASControl::NeutralSup, state->dataSize->ZoneSizingInput(1).DOASControlStrategy);
    ASSERT_EQ(DataSizing::AutoSize, state->dataSize->ZoneSizingInput(1).DOASLowSetpoint);
    ASSERT_EQ(DataSizing::AutoSize, state->dataSize->ZoneSizingInput(1).DOASHighSetpoint);
    // set default DOAS control strategy setpoint values
    ZoneEquipmentManager::AutoCalcDOASControlStrategy(*state);
    // check default low and high set point values
    ASSERT_EQ(21.1, state->dataSize->ZoneSizingInput(1).DOASLowSetpoint);
    ASSERT_EQ(23.9, state->dataSize->ZoneSizingInput(1).DOASHighSetpoint);
}

TEST_F(EnergyPlusFixture, SizingManager_DOASControlStrategyDefaultSpecificationTest2)
{
    // checks DOAS Control Strategy default setpoint values test
    std::string const idf_objects = delimited_string({

        " Zone,",
        "	SPACE1-1,      !- Name",
        "	0,             !- Direction of Relative North { deg }",
        "	0,             !- X Origin { m }",
        "	0,             !- Y Origin { m }",
        "	0,             !- Z Origin { m }",
        "	1,             !- Type",
        "	1,             !- Multiplier",
        "	3.0,           !- Ceiling Height {m}",
        "	240.0;         !- Volume {m3}",

        " Sizing:Zone,",
        "	SPACE1-1,             !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14.,                  !- Zone Cooling Design Supply Air Temperature { C }",
        "	,                     !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50.,                  !- Zone Heating Design Supply Air Temperature { C }",
        "	,                     !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009,                !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004,                !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1,     !- Design Specification Outdoor Air Object Name",
        "	0.0,                  !- Zone Heating Sizing Factor",
        "	0.0,                  !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit,   !- Cooling Design Air Flow Method",
        "	,                     !- Cooling Design Air Flow Rate { m3/s }",
        "	,                     !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	,                     !- Cooling Minimum Air Flow { m3/s }",
        "	,                     !- Cooling Minimum Air Flow Fraction",
        "	DesignDay,            !- Heating Design Air Flow Method",
        "	,                     !- Heating Design Air Flow Rate { m3/s }",
        "	,                     !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	,                     !- Heating Maximum Air Flow { m3/s }",
        "	,                     !- Heating Maximum Air Flow Fraction",
        "	,                     !- Design Specification Zone Air Distribution Object Name",
        "   Yes,                  !- Account for Dedicated Outside Air System",
        "   NeutralSupplyAir;     !- Dedicated Outside Air System Control Strategy",

        " DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1,     !- Name",
        "	sum,                  !- Outdoor Air Method",
        "	0.00236,              !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305,             !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0;                  !- Outdoor Air Flow per Zone { m3/s }",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    SizingManager::GetOARequirements(*state);
    SizingManager::GetZoneSizingInput(*state);
    ASSERT_EQ(1, state->dataSize->NumZoneSizingInput);
    ASSERT_ENUM_EQ(DOASControl::NeutralSup, state->dataSize->ZoneSizingInput(1).DOASControlStrategy);
    ASSERT_EQ(DataSizing::AutoSize, state->dataSize->ZoneSizingInput(1).DOASLowSetpoint);
    ASSERT_EQ(DataSizing::AutoSize, state->dataSize->ZoneSizingInput(1).DOASHighSetpoint);
    // set default DOAS control strategy setpoint values
    ZoneEquipmentManager::AutoCalcDOASControlStrategy(*state);
    // check default low and high set point values
    ASSERT_EQ(21.1, state->dataSize->ZoneSizingInput(1).DOASLowSetpoint);
    ASSERT_EQ(23.9, state->dataSize->ZoneSizingInput(1).DOASHighSetpoint);
}

TEST_F(EnergyPlusFixture, SizingManager_CalcdoLoadComponentPulseNowTest)
{

    bool Answer;
    bool WarmupFlag;
    bool PulseSizing;
    int HourNum;
    int TimeStepNum;

    // Tests for when to do a pulse test for the Load Component Output Report

    // Test 1a: Everything as it should be to set this to true-->result should be true
    PulseSizing = true;
    WarmupFlag = false;
    HourNum = 10;
    TimeStepNum = 1;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::RunPeriodDesign;
    state->dataGlobal->DayOfSim = 2;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_TRUE(Answer);

    // Test 16: Everything as it should be to set this to true-->result should be true
    PulseSizing = true;
    WarmupFlag = false;
    HourNum = 10;
    TimeStepNum = 1;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::DesignDay;
    state->dataGlobal->DayOfSim = 1;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_TRUE(Answer);

    // Test 2: PulseSizing is false-->result should be false
    PulseSizing = false;
    WarmupFlag = false;
    HourNum = 10;
    TimeStepNum = 1;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::RunPeriodDesign;
    state->dataGlobal->DayOfSim = 1;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_FALSE(Answer);

    // Test 3: Warmup is true-->result should be false
    PulseSizing = false;
    WarmupFlag = true;
    HourNum = 10;
    TimeStepNum = 1;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::RunPeriodDesign;
    state->dataGlobal->DayOfSim = 1;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_FALSE(Answer);

    // Test 4: HourNum not 10-->result should be false
    PulseSizing = true;
    WarmupFlag = false;
    HourNum = 7;
    TimeStepNum = 1;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::RunPeriodDesign;
    state->dataGlobal->DayOfSim = 1;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_FALSE(Answer);

    // Test 5: TimeStepNum not 1-->result should be false
    PulseSizing = true;
    WarmupFlag = false;
    HourNum = 10;
    TimeStepNum = 2;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::RunPeriodDesign;
    state->dataGlobal->DayOfSim = 1;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_FALSE(Answer);

    // Test 6: DayOfSim not 1 and KindSim not weather file period --> result should be false
    PulseSizing = true;
    WarmupFlag = false;
    HourNum = 10;
    TimeStepNum = 1;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::DesignDay;
    state->dataGlobal->DayOfSim = 2;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_FALSE(Answer);

    // Test 7: everything set to make the answer false
    PulseSizing = false;
    WarmupFlag = true;
    HourNum = 2;
    TimeStepNum = 7;
    state->dataGlobal->KindOfSim = EnergyPlus::Constant::KindOfSim::DesignDay;
    state->dataGlobal->DayOfSim = 2;
    Answer = CalcdoLoadComponentPulseNow(*state, PulseSizing, WarmupFlag, HourNum, TimeStepNum, state->dataGlobal->KindOfSim);
    ASSERT_FALSE(Answer);
}

TEST_F(EnergyPlusFixture, SizingManager_ReportTemperatureInputError)
{

    std::string objectName = "Sizing:Zone";
    int paramNum = 1;
    state->dataIPShortCut->cAlphaArgs.allocate(3);
    state->dataIPShortCut->cNumericFieldNames.allocate(3);
    state->dataIPShortCut->rNumericArgs.allocate(3);
    state->dataIPShortCut->cAlphaArgs(paramNum) = "SPACE1-1";
    state->dataIPShortCut->cNumericFieldNames(paramNum) = "Zone Cooling Design Supply Air Temperature";
    state->dataIPShortCut->rNumericArgs(paramNum) = 14.0;
    Real64 lowTempLimit = 0.0;
    bool errorsFound = false;

    // Test 1: No problems, no errors, both parameters
    ReportTemperatureInputError(*state, objectName, paramNum, lowTempLimit, false, errorsFound);
    ASSERT_FALSE(errorsFound);
    paramNum = 3;
    state->dataIPShortCut->cNumericFieldNames(paramNum) = "Zone Heating Design Supply Air Temperature";
    state->dataIPShortCut->rNumericArgs(paramNum) = 50.0;
    ReportTemperatureInputError(*state, objectName, paramNum, lowTempLimit, false, errorsFound);
    ASSERT_FALSE(errorsFound);

    // Test 2: Temperature is less than the lower limit allowed, but it's not a severe (check for both parameters)
    paramNum = 1;
    state->dataIPShortCut->rNumericArgs(paramNum) = -1.0;
    ReportTemperatureInputError(*state, objectName, paramNum, lowTempLimit, false, errorsFound);
    ASSERT_FALSE(errorsFound);
    EXPECT_TRUE(
        compare_err_stream(delimited_string({"   ** Warning ** Sizing:Zone=\"SPACE1-1\" has invalid data.",
                                             "   **   ~~~   ** ... incorrect Zone Cooling Design Supply Air Temperature=[-1.00] is less than [0.00]",
                                             "   **   ~~~   ** Please check your input to make sure this is correct."})));
    paramNum = 3;
    state->dataIPShortCut->rNumericArgs(paramNum) = -1.0;
    ReportTemperatureInputError(*state, objectName, paramNum, lowTempLimit, false, errorsFound);
    ASSERT_FALSE(errorsFound);
    EXPECT_TRUE(
        compare_err_stream(delimited_string({"   ** Warning ** Sizing:Zone=\"SPACE1-1\" has invalid data.",
                                             "   **   ~~~   ** ... incorrect Zone Heating Design Supply Air Temperature=[-1.00] is less than [0.00]",
                                             "   **   ~~~   ** Please check your input to make sure this is correct."})));

    // Test 3: Heating Temperature is lower Cooling Temperature--this should become a severe error
    state->dataIPShortCut->rNumericArgs(paramNum) = -2.0;
    ReportTemperatureInputError(*state, objectName, paramNum, lowTempLimit, true, errorsFound);
    ASSERT_TRUE(errorsFound);
    EXPECT_TRUE(compare_err_stream(delimited_string({"   ** Severe  ** Sizing:Zone=\"SPACE1-1\" has invalid data.",
                                                     "   **   ~~~   ** ... incorrect Zone Heating Design Supply Air Temperature=[-2.00] is less than "
                                                     "Zone Cooling Design Supply Air Temperature=[-1.00]",
                                                     "   **   ~~~   ** This is not allowed.  Please check and revise your input."})));
}

TEST_F(EnergyPlusFixture, SizingManager_OverrideAvgWindowInSizing)
{

    std::string const idf_objects = delimited_string({
        "SimulationControl,",
        "  No,                      !- Do Zone Sizing Calculation",
        "  No,                      !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  Yes;                     !- Run Simulation for Weather File Run Periods",
        "PerformancePrecisionTradeoffs,",
        ",                          !- Coil Direct Solutions",
        ",                          !- Zone Radiant Exchange Algorithm",
        "Mode01,                    !- Override Mode",
        ",                          !- MaxZoneTempDiff",
        ",                          !- MaxAllowedDelTemp",
        ";                          !- Use Representative Surfaces for Calculations",
        "Sizing:Parameters,",
        ",                          !- Heating Sizing Factor",
        ",                          !- Cooling Sizing Factor",
        "6;                         !- Timesteps in Averaging Window",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    SimulationManager::GetProjectData(*state);
    EXPECT_TRUE(state->dataGlobal->OverrideTimestep);
    SizingManager::GetSizingParams(*state);
    EXPECT_EQ(state->dataGlobal->TimeStepsInHour, 1);
    EXPECT_EQ(state->dataSize->NumTimeStepsInAvg, 1);
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_Coincident_1x)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    ,                        !- Zone Load Sizing Method",
        "    ,                        !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    Coincident;              !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ZONE 1 INLETS,           !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ZONE 1 OUTLET;           !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Ideal Loads,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        "    ZoneHVAC:IdealLoadsAirSystem,",
        "      ZONE 1 Ideal Loads,      !- Name",
        "      ,                        !- Availability Schedule Name",
        "      ZONE 1 INLETS,           !- Zone Supply Air Node Name",
        "      ,                        !- Zone Exhaust Air Node Name",
        "      ,                        !- System Inlet Air Node Name",
        "      50,                      !- Maximum Heating Supply Air Temperature {C}",
        "      13,                      !- Minimum Cooling Supply Air Temperature {C}",
        "      0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      NoLimit,                 !- Heating Limit",
        "      autosize,                !- Maximum Heating Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Sensible Heating Capacity {W}",
        "      NoLimit,                 !- Cooling Limit",
        "      autosize,                !- Maximum Cooling Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Total Cooling Capacity {W}",
        "      ,                        !- Heating Availability Schedule Name",
        "      ,                        !- Cooling Availability Schedule Name",
        "      ConstantSupplyHumidityRatio,  !- Dehumidification Control Type",
        "      ,                        !- Cooling Sensible Heat Ratio {dimensionless}",
        "      ConstantSupplyHumidityRatio,  !- Humidification Control Type",
        "      ,                        !- Design Specification Outdoor Air Object Name",
        "      ,                        !- Outdoor Air Inlet Node Name",
        "      ,                        !- Demand Controlled Ventilation Type",
        "      ,                        !- Outdoor Air Economizer Type",
        "      ,                        !- Heat Recovery Type",
        "      ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}",
        "      ;                        !- Latent Heat Recovery Effectiveness {dimensionless}",

        "    NodeList,",
        "      ZONE 1 INLETS,           !- Name",
        "      ZONE 1 INLET;            !- Node 1 Name",
        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    EXPECT_EQ("1898.77", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.053", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.053", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("832.44", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.058", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.058", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 17:30:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("631.20", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("940.20", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.065", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.065", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("631.20", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("810.65", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.056", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.056", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("7/21 19:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For coincident, expect zone Des Cooling Load to be less than sum of space loads which is 832.44 + 940.2 + 810.65 = 2583.29
    EXPECT_EQ("3155.31", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.088", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.088", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("1942.36", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.135", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.135", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_Coincident_10x)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    ,                        !- Zone Load Sizing Method",
        "    ,                        !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    Coincident;              !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    10,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ZONE 1 INLETS,           !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ZONE 1 OUTLET;           !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Ideal Loads,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        "    ZoneHVAC:IdealLoadsAirSystem,",
        "      ZONE 1 Ideal Loads,      !- Name",
        "      ,                        !- Availability Schedule Name",
        "      ZONE 1 INLETS,           !- Zone Supply Air Node Name",
        "      ,                        !- Zone Exhaust Air Node Name",
        "      ,                        !- System Inlet Air Node Name",
        "      50,                      !- Maximum Heating Supply Air Temperature {C}",
        "      13,                      !- Minimum Cooling Supply Air Temperature {C}",
        "      0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      NoLimit,                 !- Heating Limit",
        "      autosize,                !- Maximum Heating Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Sensible Heating Capacity {W}",
        "      NoLimit,                 !- Cooling Limit",
        "      autosize,                !- Maximum Cooling Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Total Cooling Capacity {W}",
        "      ,                        !- Heating Availability Schedule Name",
        "      ,                        !- Cooling Availability Schedule Name",
        "      ConstantSupplyHumidityRatio,  !- Dehumidification Control Type",
        "      ,                        !- Cooling Sensible Heat Ratio {dimensionless}",
        "      ConstantSupplyHumidityRatio,  !- Humidification Control Type",
        "      ,                        !- Design Specification Outdoor Air Object Name",
        "      ,                        !- Outdoor Air Inlet Node Name",
        "      ,                        !- Demand Controlled Ventilation Type",
        "      ,                        !- Outdoor Air Economizer Type",
        "      ,                        !- Heat Recovery Type",
        "      ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}",
        "      ;                        !- Latent Heat Recovery Effectiveness {dimensionless}",

        "    NodeList,",
        "      ZONE 1 INLETS,           !- Name",
        "      ZONE 1 INLET;            !- Node 1 Name",
        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    EXPECT_EQ("18987.69", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.527", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.527", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("8324.40", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.578", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.578", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 17:30:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("6311.95", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("9402.01", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("6311.95", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("8106.47", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("7/21 19:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For coincident, expect zone Des Cooling Load to be less than sum of space loads which is 832.44 + 940.2 + 810.65 = 2583.29
    EXPECT_EQ("31553.14", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("19423.64", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("1.348", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("1.348", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_NonCoincident1)
{
    // Spaces peak on same day
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    ,                        !- Zone Load Sizing Method",
        "    ,                        !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    NonCoincident;           !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    10,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ZONE 1 INLETS,           !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ZONE 1 OUTLET;           !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Ideal Loads,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        "    ZoneHVAC:IdealLoadsAirSystem,",
        "      ZONE 1 Ideal Loads,      !- Name",
        "      ,                        !- Availability Schedule Name",
        "      ZONE 1 INLETS,           !- Zone Supply Air Node Name",
        "      ,                        !- Zone Exhaust Air Node Name",
        "      ,                        !- System Inlet Air Node Name",
        "      50,                      !- Maximum Heating Supply Air Temperature {C}",
        "      13,                      !- Minimum Cooling Supply Air Temperature {C}",
        "      0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      NoLimit,                 !- Heating Limit",
        "      autosize,                !- Maximum Heating Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Sensible Heating Capacity {W}",
        "      NoLimit,                 !- Cooling Limit",
        "      autosize,                !- Maximum Cooling Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Total Cooling Capacity {W}",
        "      ,                        !- Heating Availability Schedule Name",
        "      ,                        !- Cooling Availability Schedule Name",
        "      ConstantSupplyHumidityRatio,  !- Dehumidification Control Type",
        "      ,                        !- Cooling Sensible Heat Ratio {dimensionless}",
        "      ConstantSupplyHumidityRatio,  !- Humidification Control Type",
        "      ,                        !- Design Specification Outdoor Air Object Name",
        "      ,                        !- Outdoor Air Inlet Node Name",
        "      ,                        !- Demand Controlled Ventilation Type",
        "      ,                        !- Outdoor Air Economizer Type",
        "      ,                        !- Heat Recovery Type",
        "      ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}",
        "      ;                        !- Latent Heat Recovery Effectiveness {dimensionless}",

        "    NodeList,",
        "      ZONE 1 INLETS,           !- Name",
        "      ZONE 1 INLET;            !- Node 1 Name",
        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("8324.40", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.578", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.578", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 17:30:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("9402.01", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("8106.47", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("7/21 19:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For noncoincident, expect zone Des Cooling Load to be sum of space loads = 832.44 + 940.2 + 810.65 = 2583.29
    // For noncoincident, expect zone Des Cooling Air Flow  to be sum of space flows = 0.058 + 0.065 + 0.056 = 0.179
    // Spaces peak on same day, so expect zone peak time to be the same as coincident sizing
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("25832.88", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("1.793", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("1.793", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_NonCoincident2)
// Spaces peak on different days
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA July Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA August Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    8,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA September Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    9,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    ,                        !- Zone Load Sizing Method",
        "    ,                        !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    NonCoincident;           !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    10,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ZONE 1 INLETS,           !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ZONE 1 OUTLET;           !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Ideal Loads,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        "    ZoneHVAC:IdealLoadsAirSystem,",
        "      ZONE 1 Ideal Loads,      !- Name",
        "      ,                        !- Availability Schedule Name",
        "      ZONE 1 INLETS,           !- Zone Supply Air Node Name",
        "      ,                        !- Zone Exhaust Air Node Name",
        "      ,                        !- System Inlet Air Node Name",
        "      50,                      !- Maximum Heating Supply Air Temperature {C}",
        "      13,                      !- Minimum Cooling Supply Air Temperature {C}",
        "      0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      NoLimit,                 !- Heating Limit",
        "      autosize,                !- Maximum Heating Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Sensible Heating Capacity {W}",
        "      NoLimit,                 !- Cooling Limit",
        "      autosize,                !- Maximum Cooling Air Flow Rate {m3/s}",
        "      ,                        !- Maximum Total Cooling Capacity {W}",
        "      ,                        !- Heating Availability Schedule Name",
        "      ,                        !- Cooling Availability Schedule Name",
        "      ConstantSupplyHumidityRatio,  !- Dehumidification Control Type",
        "      ,                        !- Cooling Sensible Heat Ratio {dimensionless}",
        "      ConstantSupplyHumidityRatio,  !- Humidification Control Type",
        "      ,                        !- Design Specification Outdoor Air Object Name",
        "      ,                        !- Outdoor Air Inlet Node Name",
        "      ,                        !- Demand Controlled Ventilation Type",
        "      ,                        !- Outdoor Air Economizer Type",
        "      ,                        !- Heat Recovery Type",
        "      ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}",
        "      ;                        !- Latent Heat Recovery Effectiveness {dimensionless}",

        "    NodeList,",
        "      ZONE 1 INLETS,           !- Name",
        "      ZONE 1 INLET;            !- Node 1 Name",
        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    July Morning,            !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    5000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    July Morning,            !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 6/30,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 7/31,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    August Afternoon,        !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    10000.0,                 !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    August Afternoon,        !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 7/31,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 8/31,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    September Evening,       !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    7500.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    September Evening,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 8/31,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 9/30,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,        !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("41650.19", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("2.891", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("2.891", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA JULY COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("77258.02", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("5.362", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("5.362", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA AUGUST COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("8/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("58268.65", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("4.044", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("4.044", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA SEPTEMBER COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("9/21 20:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For noncoincident, expect zone Des Cooling Load to be sum of space loads = 4165.02 + 7725.80 + 5826.87 = 17717.69
    // For noncoincident, expect zone Des Cooling Air Flow  to be sum of space flows = 0.289 + 0.536 + 0.404 = 1.229
    // Spaces peak on same day, so expect zone peak time to be the different coincident sizing, and day to be "N/A"
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.0", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("177176.86", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("12.297", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("12.297", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("N/A", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_Coincident_NonAir_1x_NoLatent)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    Sensible Load, !- Zone Load Sizing Method",
        "    HumidityRatioDifference, !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    HumidityRatioDifference, !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    Coincident;              !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ,                        !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ;                        !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Baseboard,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        " ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    Zone 1 Baseboard,        !- Name",
        "    ,                        !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    autosize,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    Zn001:Wall001,           !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    Space 1 Latent ElecEq,   !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    1.0,                     !- Fraction Latent",
        "    0.0,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    EXPECT_EQ("1898.77", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.053", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.053", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("832.44", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.058", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.058", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 17:30:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("631.20", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("940.20", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.065", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.065", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("631.20", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.018", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("810.65", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.056", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.056", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("7/21 19:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For coincident, expect zone Des Cooling Load to be less than sum of space loads which is 832.44 + 940.2 + 810.65 = 2583.29
    EXPECT_EQ("3155.31", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.088", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.088", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("1942.36", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.135", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.135", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_Coincident_NonAir_10x_NoLatent)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    Sensible Load, !- Zone Load Sizing Method",
        "    HumidityRatioDifference, !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    HumidityRatioDifference, !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    Coincident;              !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    10,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ,                        !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ;                        !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Baseboard,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        " ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    Zone 1 Baseboard,        !- Name",
        "    ,                        !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    autosize,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    Zn001:Wall001,           !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    Space 1 Latent ElecEq,   !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    1.0,                     !- Fraction Latent",
        "    0.0,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    EXPECT_EQ("18987.69", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.527", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.527", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("8324.40", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.578", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.578", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 17:30:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("6311.95", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("9402.01", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("6311.95", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("8106.47", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("7/21 19:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For coincident, expect zone Des Cooling Load to be less than sum of space loads which is 832.44 + 940.2 + 810.65 = 2583.29
    EXPECT_EQ("31553.14", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("19423.64", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("1.348", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("1.348", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_Coincident_NonAir_10x_NoLatent_NoSpaceHB)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    Sensible Load, !- Zone Load Sizing Method",
        "    HumidityRatioDifference, !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    HumidityRatioDifference, !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    Coincident;              !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    10,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ,                        !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ;                        !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Baseboard,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        " ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    Zone 1 Baseboard,        !- Name",
        "    ,                        !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    autosize,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    Zn001:Wall001,           !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    Space 1 Latent ElecEq,   !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    1.0,                     !- Fraction Latent",
        "    0.0,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "No,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    // For coincident, expect zone Des Cooling Load to be less than sum of space loads which is 832.44 + 940.2 + 810.65 = 2583.29
    EXPECT_EQ("31553.14", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("19423.64", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("1.348", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("1.348", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
TEST_F(EnergyPlusFixture, SizingManager_ZoneSizing_Coincident_NonAir_10x_Latent_SpaceHB)
{
    std::string const idf_objects = delimited_string({
        "  Timestep,6;",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud){dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  ZoneControl:Thermostat,",
        "    Zone 1 Thermostat,       !- Name",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Dual Setpoint with SB;   !- Control 1 Name",

        "  Schedule:Compact,",
        "    ZONE CONTROL TYPE SCHED, !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,4;          !- Field 11",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Dual Setpoint with SB,!- Name",
        "    Heating Setpoints,       !- Setpoint Temperature Schedule Name",
        "    Cooling Setpoints;       !- Setpoint Temperature Schedule Name",

        "  Schedule:Compact,",
        "    HEATING SETPOINTS,       !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,15.00,       !- Field 3",
        "    Until: 17:00,20.00,      !- Field 5",
        "    Until: 24:00,15.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,15.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,20.00;      !- Field 13",

        "  Schedule:Compact,",
        "    COOLING SETPOINTS,       !- Name",
        "    ,                      !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Weekdays Weekends Holidays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,30.00,       !- Field 3",
        "    Until: 17:00,24.00,      !- Field 5",
        "    Until: 24:00,30.00,      !- Field 7",
        "    For: SummerDesignDay,    !- Field 9",
        "    Until: 24:00,24.00,      !- Field 10",
        "    For: WinterDesignDay,    !- Field 12",
        "    Until: 24:00,50.00;      !- Field 13",

        "  Sizing:Zone,",
        "    Zone 1,                  !- Zone or ZoneList Name",
        "    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "    12.,                     !- Zone Cooling Design Supply Air Temperature {C}",
        "    ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "    50.,                     !- Zone Heating Design Supply Air Temperature {C}",
        "    ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "    0.008,                   !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name",
        "    1.0,                     !- Zone Heating Sizing Factor",
        "    1.0,                     !- Zone Cooling Sizing Factor",
        "    DesignDay,               !- Cooling Design Air Flow Method",
        "    0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Minimum Air Flow {m3/s}",
        "    ,                        !- Cooling Minimum Air Flow Fraction",
        "    DesignDay,               !- Heating Design Air Flow Method",
        "    0,                       !- Heating Design Air Flow Rate {m3/s}",
        "    ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Heating Maximum Air Flow {m3/s}",
        "    ,                        !- Heating Maximum Air Flow Fraction",
        "    ,                        !- Design Specification Zone Air Distribution Object Name",
        "    No,                      !- Account for Dedicated Outdoor Air System",
        "    NeutralSupplyAir,        !- Dedicated Outdoor Air System Control Strategy",
        "    autosize,                !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}",
        "    autosize,                !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}",
        "    Sensible And Latent Load, !- Zone Load Sizing Method",
        "    HumidityRatioDifference, !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    HumidityRatioDifference, !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method",
        "    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "    0.005,                   !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}",
        "    ,                        !- Zone Humidistat Dehumidification Set Point Schedule Name {percent}",
        "    ,                        !- Zone Humidistat Humidification Set Point Schedule Name {percent}",
        "    Coincident;              !- Type of Space Sum to Use",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA West Zone,       !- Name",
        "    flow/person,             !- Outdoor Air Method",
        "    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  Zone,",
        "    Zone 1,                  !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    10,                      !- Multiplier",
        "    3.048,                   !- Ceiling Height {m}",
        "    40.;                     !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,3.048000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000,0,3.048000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor001,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 1,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor002,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 2,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Floor003,           !- Name",
        "    Floor,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Zone 1,                  !- Zone Name",
        "    Space 3,                 !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,5,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7600000,               !- Solar Absorptance",
        "    0.7600000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "    ZoneHVAC:EquipmentConnections,",
        "      Zone 1,                  !- Zone Name",
        "      ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name",
        "      ,                        !- Zone Air Inlet Node or NodeList Name",
        "      ,                        !- Zone Air Exhaust Node or NodeList Name",
        "      ZONE 1 NODE,             !- Zone Air Node Name",
        "      ;                        !- Zone Return Air Node or NodeList Name",

        "    ZoneHVAC:EquipmentList,",
        "      ZONE 1 EQUIPMENT,        !- Name",
        "      SequentialLoad,          !- Load Distribution Scheme",
        "      ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "      ZONE 1 Baseboard,      !- Zone Equipment 1 Name",
        "      1,                       !- Zone Equipment 1 Cooling Sequence",
        "      1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "      ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "      ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        " ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    Zone 1 Baseboard,        !- Name",
        "    ,                        !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    autosize,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    Zn001:Wall001,           !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

        "Space,",
        "Space 1,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 2,            !- Name",
        "Zone 1;             !- Zone Name",
        "Space,",
        "Space 3,            !- Name",
        "Zone 1;             !- Zone Name",

        "  ElectricEquipment,",
        "    Space 1 ElecEq,          !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    Morning,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    Space 1 Latent ElecEq,   !- Name",
        "    Space 1,                 !- Zone or ZoneList Name",
        "    SummerMorning2,          !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    1.0,                     !- Fraction Latent",
        "    0.0,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Morning,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 9:00,0.5,         !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  Schedule:Compact,",
        "    SummerMorning2,          !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 5/31,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 9/30,           !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 8:00,0.0,         !- Field 11",
        "    Until: 9:00,0.5,         !- Field 11",
        "    Until: 11:00,0.8,        !- Field 11",
        "    Until: 12:00,1.0,        !- Field 11",
        "    Until: 13:00,0.9,        !- Field 11",
        "    Until: 24:00,0.0,        !- Field 11",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 2 ElecEq,          !- Name",
        "    Space 2,                 !- Zone or ZoneList Name",
        "    Afternoon,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1000.0,                  !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Afternoon,               !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 12:00,0.0,        !- Field 11",
        "    Until: 16:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "  ElectricEquipment,",
        "    Space 3 ElecEq,          !- Name",
        "    Space 3,                 !- Zone or ZoneList Name",
        "    Evening,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    750.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Schedule:Compact,",
        "    Evening,                 !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 16:00,0.0,         !- Field 11",
        "    Until: 20:00,1.0,        !- Field 11",
        "    Until: 24:00,0.0;        !- Field 11",

        "ZoneAirHeatBalanceAlgorithm,",
        "ThirdOrderBackwardDifference,  !- Algorithm",
        "Yes,                      !- Do Space Heat Balance for Sizing",
        "No;                       !- Do Space Heat Balance for Simulation",
        "  Output:Table:SummaryReports,",
        "      AllSummary;                             !- Report Name 1",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    SimulationManager::ManageSimulation(*state);

    int space1Num = 1;
    auto &calFinalSpSiz = state->dataSize->CalcFinalSpaceSizing(space1Num);
    EXPECT_EQ(calFinalSpSiz.TimeStepNumAtLatentCoolMax, 72);

    EXPECT_EQ("18989.14", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.527", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.527", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 1"));
    EXPECT_EQ("10000.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 1"));
    EXPECT_EQ("0.667", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 1"));
    EXPECT_EQ("0.667", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 1"));
    EXPECT_EQ("7/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 1"));

    EXPECT_EQ("6311.95", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 2"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 2"));
    EXPECT_EQ("9402.01", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 2"));
    EXPECT_EQ("0.653", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 2"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 2"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 2"));

    EXPECT_EQ("6311.95", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.175", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtDesDay, "SPACE 3"));
    EXPECT_EQ("1/21 12:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpHtPkTime, "SPACE 3"));
    EXPECT_EQ("8106.47", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesLd, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClCalcDesAirFlow, "SPACE 3"));
    EXPECT_EQ("0.563", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClUserDesAirFlow, "SPACE 3"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClDesDay, "SPACE 3"));
    EXPECT_EQ("7/21 19:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchSpClPkTime, "SPACE 3"));

    // For coincident, expect zone Des Cooling Load to be less than sum of space loads which is 832.44 + 940.2 + 810.65 = 2583.29
    EXPECT_EQ("31554.59", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesLd, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("0.876", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL HEATING 99% DESIGN CONDITIONS DB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtDesDay, "ZONE 1"));
    EXPECT_EQ("1/21 08:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnHtPkTime, "ZONE 1"));
    EXPECT_EQ("19406.72", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesLd, "ZONE 1"));
    EXPECT_EQ("1.347", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClCalcDesAirFlow, "ZONE 1"));
    EXPECT_EQ("1.347", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClUserDesAirFlow, "ZONE 1"));
    EXPECT_EQ("CHICAGO_IL_USA ANNUAL COOLING 1% DESIGN CONDITIONS DB/MCWB",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClDesDay, "ZONE 1"));
    EXPECT_EQ("7/21 16:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchZnClPkTime, "ZONE 1"));
}
