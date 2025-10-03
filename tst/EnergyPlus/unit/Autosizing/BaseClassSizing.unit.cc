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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "../Fixtures/SQLiteFixture.hh"
#include "AutosizingFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/WeatherManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataAirSystems;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::Psychrometrics;

TEST_F(EnergyPlusFixture, BaseSizer_selectSizerOutput)
{
    // ficticious sizing to test the selectSizerOutput function
    // there are several if blocks to determine which type of reporting to output
    // report Design Size and User, Design Size only, or User size only
    // if these if block miss a possible input configuration the simulation may fail
    // with a Developer Error message
    // this unit test is a place holder to test for all the possible input configurations

    bool errorsFound = false;
    bool PrintFlag = true;
    std::string_view RoutineName = "BaseSizer_selectSizerOutput";
    std::string_view CompType = "testComp";
    std::string_view CompName = "testName";

    // pick a sizing method
    SystemAirFlowSizer thisSizer;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->ZoneSizingInput.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).ZoneNum = state->dataSize->CurZoneEqNum;

    // Test #1
    // Autosized value may be less than 0

    // negative numbers should never happen for air flow but can certainly happen for temperature
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow = -1.0;

    // set up inputs used for sizing
    Real64 TempSize = DataSizing::AutoSize;

    // call the selectSizerOutput function to make sure this input configuration is caught
    thisSizer.initializeWithinEP(*state, CompType, CompName, PrintFlag, RoutineName);
    Real64 autoSizedValue = thisSizer.size(*state, TempSize, errorsFound);

    EXPECT_EQ(-1.0, autoSizedValue);
    EXPECT_ENUM_EQ(AutoSizingResultType::NoError, thisSizer.errorType);
}

TEST_F(EnergyPlusFixture, BaseSizer_GetCoilDesFlowT)
{
    // setup global allocation
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizPeakDDNum.allocate(1);
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->CalcSysSizing.allocate(1);
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq.allocate(1);
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtSensCoolPk.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtTotCoolPk.allocate(1);

    // one-time global initialization
    int constexpr DesignDayForPeak = 1;
    state->dataSize->SysSizPeakDDNum(1).SensCoolPeakDD = DesignDayForPeak;
    state->dataSize->SysSizPeakDDNum(1).CoolFlowPeakDD = DesignDayForPeak;
    state->dataSize->SysSizPeakDDNum(1).TotCoolPeakDD = DesignDayForPeak;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtSensCoolPk(DesignDayForPeak) = 1;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk(DesignDayForPeak) = 1;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtTotCoolPk(DesignDayForPeak) = 1;
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 10;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = 2.0;
    state->dataSize->FinalSysSizing(1).DesCoolVolFlow = 0.15;
    state->dataSize->DataAirFlowUsedForSizing = 0.2;
    state->dataEnvrn->StdRhoAir = 1000;
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq(1) = 1250000;

    // one-time argument initialization
    int constexpr sysNum = 1;
    Real64 constexpr CpAir = 4179;

    // argument return values
    Real64 designFlowValue;
    Real64 designExitTemp;
    Real64 designExitHumRat;

    state->dataSize->SysSizInput(1).coolingPeakLoad = DataSizing::PeakLoad::TotalCooling;
    state->dataSize->FinalSysSizing(1).coolingPeakLoad = DataSizing::PeakLoad::TotalCooling;

    // Single path for VAV
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(0.002, designFlowValue);

    // Single path for OnOff
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::OnOff;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(0.2, designFlowValue);

    // Two paths for VT:
    // CoolSupTemp > calculated value
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VT;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(1) = 10;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).DesCoolVolFlow, designFlowValue);
    // CoolSupTemp < calculated value
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VT;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(1) = 15;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_NEAR(13.00590, designExitTemp, 0.0001);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).DesCoolVolFlow, designFlowValue);

    // Two paths for bypass:
    // MixTemp > DesExitTemp
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::Bypass;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(1) = 13;
    state->dataSize->CalcSysSizing(1).MixTempAtCoolPeak = 15;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(10, designExitTemp);
    EXPECT_NEAR(0.119823, designFlowValue, 0.0001);
    // MixTemp < DesExitTemp
    state->dataSize->CalcSysSizing(1).MixTempAtCoolPeak = 5;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(10, designExitTemp);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).DesCoolVolFlow, designFlowValue);

    // Oh and the sensible cases
    state->dataSize->SysSizInput(1).coolingPeakLoad = DataSizing::PeakLoad::SensibleCooling;
    state->dataSize->FinalSysSizing(1).coolingPeakLoad = DataSizing::PeakLoad::SensibleCooling;
    // Repeat a VT case
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VT;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(1) = 10;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).DesCoolVolFlow, designFlowValue);
    // And a bypass case
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::Bypass;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(1) = 13;
    state->dataSize->CalcSysSizing(1).MixTempAtCoolPeak = 15;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(10, designExitTemp);
    EXPECT_NEAR(0.119823, designFlowValue, 0.0001);
}
TEST_F(EnergyPlusFixture, BaseSizer_GetCoilDesFlowT_NoPeak)
{
    // setup global allocation
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizPeakDDNum.allocate(1);
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->CalcSysSizing.allocate(1);
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq.allocate(1);
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtSensCoolPk.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtTotCoolPk.allocate(1);

    // one-time global initialization
    int constexpr DesignDayForPeak = 0;
    state->dataSize->SysSizPeakDDNum(1).SensCoolPeakDD = DesignDayForPeak;
    state->dataSize->SysSizPeakDDNum(1).CoolFlowPeakDD = DesignDayForPeak;
    state->dataSize->SysSizPeakDDNum(1).TotCoolPeakDD = DesignDayForPeak;
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 10;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = 2.0;
    state->dataSize->FinalSysSizing(1).DesCoolVolFlow = 0.15;
    state->dataSize->DataAirFlowUsedForSizing = 0.2;
    state->dataEnvrn->StdRhoAir = 1000;
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq(1) = 1250000;

    // one-time argument initialization
    int constexpr sysNum = 1;
    Real64 constexpr CpAir = 4179;

    // argument return values
    Real64 designFlowValue;
    Real64 designExitTemp;
    Real64 designExitHumRat;

    state->dataSize->SysSizInput(1).coolingPeakLoad = DataSizing::PeakLoad::TotalCooling;
    state->dataSize->FinalSysSizing(1).coolingPeakLoad = DataSizing::PeakLoad::TotalCooling;

    // Single path for VAV
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(0.002, designFlowValue);

    // Single path for OnOff
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::OnOff;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(0.2, designFlowValue);

    // VT
    // CoolSupTemp > calculated value
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VT;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    // Expect warning and same result as VAV
    EXPECT_TRUE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(0.002, designFlowValue);

    // Bypass
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::Bypass;
    DataSizing::GetCoilDesFlowT(*state, sysNum, CpAir, designFlowValue, designExitTemp, designExitHumRat);
    // Expect warning and same result as VAV
    EXPECT_TRUE(has_err_output(true));
    EXPECT_DOUBLE_EQ(state->dataSize->FinalSysSizing(1).CoolSupTemp, designExitTemp);
    EXPECT_DOUBLE_EQ(0.002, designFlowValue);
}

TEST_F(EnergyPlusFixture, BaseSizer_RequestSizingSystem)
{
    state->init_state(*state);

    std::string CompName;       // component name
    std::string CompType;       // component type
    std::string SizingString;   // input field sizing description
    int SizingType;             // integer type of sizing requested
    Real64 SizingResult;        // autosized value of coil input field
    bool PrintWarning;          // true when sizing information is reported in the eio file
    std::string CallingRoutine; // calling routine

    state->dataSize->DataTotCapCurveIndex = 0;
    state->dataSize->DataDesOutletAirTemp = 0.0;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).CoolSupTemp = 12.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).CoolSupHumRat = 0.0085;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).MixTempAtCoolPeak = 28.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).MixHumRatAtCoolPeak = 0.0075;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesCoolVolFlow = 1.00;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.2;

    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumOACoolCoils = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanNum = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).retFanNum = 0;

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = state->dataSize->CurSysNum;
    state->dataSize->NumSysSizInput = 1;

    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.1583684;

    state->dataSize->DataFlowUsedForSizing = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesCoolVolFlow;
    // Need this to prevent crash in Sizers
    state->dataSize->UnitarySysEqSizing.allocate(1);
    state->dataSize->OASysEqSizing.allocate(1);

    CompType = "COIL:COOLING:DX:SINGLESPEED";
    CompName = "Single Speed DX Cooling Coil";
    SizingType = HVAC::CoolingCapacitySizing;
    SizingString = "Nominal Capacity";
    SizingResult = DataSizing::AutoSize;
    PrintWarning = true;
    CallingRoutine = "RequestSizing";
    state->dataSize->DataIsDXCoil = true;

    // dx cooling coil capacity sizing
    bool errorsFound = false;
    CoolingCapacitySizer sizerCoolingCapacity;
    sizerCoolingCapacity.overrideSizingString(SizingString);
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*state, SizingResult, errorsFound);
    EXPECT_NEAR(18882.0, SizingResult, 0.1);

    // confirm that sizing data is saved for use by parent object
    EXPECT_NEAR(state->dataSize->DataCoilSizingAirInTemp, 28.0, 0.0000001);
    EXPECT_NEAR(state->dataSize->DataCoilSizingAirInHumRat, 0.0075, 0.0000001);
    EXPECT_NEAR(state->dataSize->DataCoilSizingAirOutTemp, 12.0, 0.0000001);
    EXPECT_NEAR(state->dataSize->DataCoilSizingAirOutHumRat, 0.0075, 0.0000001);
    EXPECT_NEAR(state->dataSize->DataCoilSizingFanCoolLoad, 0.0, 0.0000001);
    EXPECT_NEAR(state->dataSize->DataCoilSizingCapFT, 1.0, 0.0000001);

    CompType = "COIL:COOLING:WATER";
    CompName = "Chilled Water Cooling Coil";
    SizingResult = DataSizing::AutoSize;
    state->dataEnvrn->StdRhoAir = 1.18;
    state->dataSize->DataIsDXCoil = false;

    // chilled water cooling coil capacity sizing
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*state, SizingResult, errorsFound);
    EXPECT_NEAR(19234.6, SizingResult, 0.1);
}

TEST_F(EnergyPlusFixture, BaseSizer_RequestSizingSystemWithFans)
{

    // Add three fans to this model - one Fan:ConstantVolume, and three Fan:SystemModel in order to make the SupFanIndex=2
    std::string const idf_objects = delimited_string({

        "  Fan:SystemModel,",
        "    Test Fan 1 ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFanAirInletNode,         !- Air Inlet Node Name",
        "    TestFanOutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    50.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                    !- Design Electric Power Consumption",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                            !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    0.50;                        !- Fan Total Efficiency",
        "  ",
        "  Fan:SystemModel,",
        "    Test Fan 2 ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFan2AirInletNode,         !- Air Inlet Node Name",
        "    TestFan2OutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
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
        "  Fan:SystemModel,",
        "    Test Fan 3 ,                   !- Name",
        "    ,                            !- Availability Schedule Name",
        "    TestFan3AirInletNode,         !- Air Inlet Node Name",
        "    TestFan3OutletNode,           !- Air Outlet Node Name",
        "    1.0 ,                        !- Design Maximum Air Flow Rate",
        "    Discrete ,                   !- Speed Control Method",
        "    0.0,                         !- Electric Power Minimum Flow Rate Fraction",
        "    200.0,                       !- Design Pressure Rise",
        "    0.9 ,                        !- Motor Efficiency",
        "    1.0 ,                        !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                    !- Design Electric Power Consumption",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                            !- Electric Power Per Unit Flow Rate",
        "    ,                            !- Electric Power Per Unit Flow Rate Per Unit Pressure",
        "    0.50;                        !- Fan Total Efficiency",
        "  Fan:ConstantVolume,",
        "    Test Fan 4,            !- Name",
        "    ,    !- Availability Schedule Name",
        "    0.5,                     !- Fan Total Efficiency",
        "    25.0,                   !- Pressure Rise {Pa}",
        "    1.0,                  !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    TestFan4AirInletNode,         !- Air Inlet Node Name",
        "    TestFan4OutletNode;           !- Air Outlet Node Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Fans::GetFanInput(*state);
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataEnvrn->StdRhoAir = 1.2;

    auto *fan1 = state->dataFans->fans(Fans::GetFanIndex(*state, "TEST FAN 1"));
    fan1->simulate(*state, false, _, _);          // triggers sizing call
    Real64 locFanSizeVdot = fan1->maxAirFlowRate; // get function
    Real64 locDesignHeatGain1 = fan1->getDesignHeatGain(*state, locFanSizeVdot);
    EXPECT_NEAR(locDesignHeatGain1, 100.0, 0.1);

    auto *fan2 = state->dataFans->fans(Fans::GetFanIndex(*state, "TEST FAN 2"));
    fan2->simulate(*state, false, _, _);   // triggers sizing call
    locFanSizeVdot = fan2->maxAirFlowRate; // get function
    Real64 locDesignHeatGain2 = fan2->getDesignHeatGain(*state, locFanSizeVdot);
    EXPECT_NEAR(locDesignHeatGain2, 200.0, 0.1);

    auto *fan3 = state->dataFans->fans(Fans::GetFanIndex(*state, "TEST FAN 3"));
    state->dataEnvrn->StdRhoAir = 1.2;
    fan3->simulate(*state, false, _, _);                       // triggers sizing call
    locFanSizeVdot = state->dataFans->fans(3)->maxAirFlowRate; // get function
    Real64 locDesignHeatGain3 = fan3->getDesignHeatGain(*state, locFanSizeVdot);
    EXPECT_NEAR(locDesignHeatGain3, 400.0, 0.1);

    auto *fan4 = state->dataFans->fans(Fans::GetFanIndex(*state, "TEST FAN 4"));
    Real64 locDesignHeatGain4 = fan4->getDesignHeatGain(*state, locFanSizeVdot);
    EXPECT_NEAR(locDesignHeatGain4, 50.0, 0.1);

    std::string CompName;       // component name
    std::string CompType;       // component type
    std::string SizingString;   // input field sizing description
    int SizingType;             // integer type of sizing requested
    Real64 SizingResult;        // autosized value of coil input field
    bool PrintWarning;          // true when sizing information is reported in the eio file
    std::string CallingRoutine; // calling routine

    state->dataSize->DataTotCapCurveIndex = 0;
    state->dataSize->DataDesOutletAirTemp = 0.0;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).CoolSupTemp = 12.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).CoolSupHumRat = 0.0085;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).MixTempAtCoolPeak = 28.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).MixHumRatAtCoolPeak = 0.0075;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesCoolVolFlow = 1.00;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.2;

    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumOACoolCoils = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanNum = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).retFanNum = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanType = HVAC::FanType::Invalid;

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = state->dataSize->CurSysNum;
    state->dataSize->NumSysSizInput = 1;

    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.1583684;

    state->dataSize->DataFlowUsedForSizing = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesCoolVolFlow;
    // Need this to prevent crash in Sizers
    state->dataSize->UnitarySysEqSizing.allocate(1);
    state->dataSize->OASysEqSizing.allocate(1);

    // Without fan heat
    CompType = "COIL:COOLING:DX:SINGLESPEED";
    CompName = "Single Speed DX Cooling Coil";
    SizingType = HVAC::CoolingCapacitySizing;
    SizingString = "Nominal Capacity";
    SizingResult = DataSizing::AutoSize;
    PrintWarning = true;
    CallingRoutine = "RequestSizing";
    state->dataSize->DataIsDXCoil = true;

    // dx cooling coil capacity sizing
    bool errorsFound = false;
    CoolingCapacitySizer sizerCoolingCapacity;
    sizerCoolingCapacity.overrideSizingString(SizingString);
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*this->state, SizingResult, errorsFound);
    EXPECT_NEAR(18882.0, SizingResult, 0.1);
    Real64 dxCoilSizeNoFan = SizingResult;

    // With Test Fan 4 fan heat
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanNum = Fans::GetFanIndex(*state, "TEST FAN 4");
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).retFanNum = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanType = HVAC::FanType::Constant;
    CompType = "COIL:COOLING:DX:SINGLESPEED";
    CompName = "Single Speed DX Cooling Coil";
    SizingType = HVAC::CoolingCapacitySizing;
    SizingString = "Nominal Capacity";
    SizingResult = DataSizing::AutoSize;
    PrintWarning = true;
    CallingRoutine = "RequestSizing";
    state->dataSize->DataIsDXCoil = true;
    // Expect coil size to increase by fan heat for fan 4
    Real64 expectedDXCoilSize = dxCoilSizeNoFan + locDesignHeatGain4;

    // dx cooling coil capacity sizing
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*this->state, SizingResult, errorsFound);
    EXPECT_NEAR(expectedDXCoilSize, SizingResult, 0.1);

    // With Test Fan 3 fan heat - this fails before the #6126 fix
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanNum = Fans::GetFanIndex(*state, "TEST FAN 3");
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).retFanNum = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanType = HVAC::FanType::SystemModel;
    CompType = "COIL:COOLING:DX:SINGLESPEED";
    CompName = "Single Speed DX Cooling Coil";
    SizingType = HVAC::CoolingCapacitySizing;
    SizingString = "Nominal Capacity";
    SizingResult = DataSizing::AutoSize;
    PrintWarning = true;
    CallingRoutine = "RequestSizing";
    state->dataSize->DataIsDXCoil = true;
    // Expect coil size to increase by fan heat for fan 3
    expectedDXCoilSize = dxCoilSizeNoFan + locDesignHeatGain3;

    // dx cooling coil capacity sizing
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*this->state, SizingResult, errorsFound);
    EXPECT_NEAR(expectedDXCoilSize, SizingResult, 0.1);
}

TEST_F(EnergyPlusFixture, BaseSizer_RequestSizingZone)
{
    state->init_state(*state);
    int constexpr ZoneNum = 1;
    std::string CompName;       // component name
    std::string CompType;       // component type
    std::string SizingString;   // input field sizing description
    int SizingType;             // integerized type of sizing requested
    Real64 SizingResult;        // autosized value of coil input field
    bool PrintWarning;          // true when sizing information is reported in the eio file
    std::string CallingRoutine; // calling routine

    state->dataSize->DataTotCapCurveIndex = 0;
    state->dataSize->DataDesOutletAirTemp = 0.0;

    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 12.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.0085;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 28.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.0075;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolOAFlowFrac = 0.2;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.30;

    state->dataSize->ZoneSizingRunDone = true;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.1583684;

    // Need this to prevent crash in Sizers
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = ZoneNum;
    state->dataSize->NumZoneSizingInput = 1;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->DataFlowUsedForSizing = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow;

    CompType = "COIL:COOLING:DX:SINGLESPEED";
    CompName = "Single Speed DX Cooling Coil";
    SizingType = HVAC::CoolingCapacitySizing;
    SizingString = "Nominal Capacity";
    SizingResult = DataSizing::AutoSize;
    PrintWarning = true;
    CallingRoutine = "RequestSizing";
    state->dataSize->DataIsDXCoil = true;

    // dx cooling coil capacity sizing
    bool errorsFound = false;
    CoolingCapacitySizer sizerCoolingCapacity;
    sizerCoolingCapacity.overrideSizingString(SizingString);
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*this->state, SizingResult, errorsFound);
    EXPECT_NEAR(5664.6, SizingResult, 0.1);

    CompType = "COIL:COOLING:WATER";
    CompName = "Chilled Water Cooling Coil";
    SizingResult = DataSizing::AutoSize;
    state->dataEnvrn->StdRhoAir = 1.18;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow * state->dataEnvrn->StdRhoAir;
    state->dataSize->DataIsDXCoil = false;

    // chilled water cooling coil capacity sizing
    sizerCoolingCapacity.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerCoolingCapacity.size(*this->state, SizingResult, errorsFound);
    EXPECT_NEAR(5770.4, SizingResult, 0.1);
}

TEST_F(SQLiteFixture, BaseSizer_SQLiteRecordReportSizerOutputTest)
{
    // issue #6112
    std::string CompName;
    std::string CompType;
    std::string VarDesc;
    std::string UsrDesc;
    Real64 VarValue;
    Real64 UsrValue;

    // input values
    CompType = "BOILER:HOTWATER";
    CompName = "RESIDENTIAL BOILER ELECTRIC";
    VarDesc = "Design Size Nominal Capacity [W]";
    VarValue = 105977.98934;
    UsrDesc = "User-Specified Nominal Capacity [W]";
    UsrValue = 26352.97405;
    // boiler hot water autosizing and userspecified nominal capacity reporting to SQLite output
    BaseSizer::reportSizerOutput(*state, CompType, CompName, VarDesc, VarValue, UsrDesc, UsrValue);
    // get the sqlite output
    // query the sqLite
    auto result = queryResult("SELECT * FROM ComponentSizes;", "ComponentSizes");
    // check that there are two sizing result records
    ASSERT_EQ(2ul, result.size());
    std::vector<std::string> testResult0{
        "1", "BOILER:HOTWATER", "RESIDENTIAL BOILER ELECTRIC", "Design Size Nominal Capacity", "105977.98934", "W", ""};
    std::vector<std::string> testResult1{
        "2", "BOILER:HOTWATER", "RESIDENTIAL BOILER ELECTRIC", "User-Specified Nominal Capacity", "26352.97405", "W", ""};
    EXPECT_EQ(testResult0, result[0]);
    EXPECT_EQ(testResult1, result[1]);
}

TEST_F(EnergyPlusFixture, BaseSizer_setOAFracForZoneEqSizing_Test)
{
    CoolingCapacitySizer sizer;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow = 0.34;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).ATMixerVolFlow = 0.0;
    state->dataEnvrn->StdRhoAir = 1.23;

    Real64 oaFrac = 0.0;
    Real64 DesMassFlow = 0.685;
    Real64 massFlowRate = state->dataEnvrn->StdRhoAir * state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow;
    Real64 oaFrac_Test = massFlowRate / DesMassFlow;

    ZoneEqSizingData &zoneEqSizing = state->dataSize->ZoneEqSizing(1);

    // ATMixer flow rate = 0 so oaFrac depends on ZoneEqSizing.OAVolFlow
    oaFrac = sizer.setOAFracForZoneEqSizing(*state, DesMassFlow, zoneEqSizing);
    EXPECT_EQ(oaFrac, oaFrac_Test);

    zoneEqSizing.ATMixerVolFlow = 0.11;

    oaFrac = 0.0;
    massFlowRate = state->dataEnvrn->StdRhoAir * zoneEqSizing.ATMixerVolFlow;
    oaFrac_Test = massFlowRate / DesMassFlow;

    // ATMixer flow rate > 0 so oaFrac depends on ZoneEqSizing.ATMixerVolFlow
    oaFrac = sizer.EnergyPlus::BaseSizer::setOAFracForZoneEqSizing(*state, DesMassFlow, zoneEqSizing);
    EXPECT_EQ(oaFrac, oaFrac_Test);

    DesMassFlow = 0.0;
    oaFrac = 1.0;
    oaFrac_Test = 0.0;
    zoneEqSizing.OAVolFlow = 1.0;
    zoneEqSizing.ATMixerVolFlow = 1.0;
    // DesMassFlow = 0 so oaFrac = 0 regardless of OAVolFlow or ATMixerVolFlow
    oaFrac = sizer.EnergyPlus::BaseSizer::setOAFracForZoneEqSizing(*state, DesMassFlow, zoneEqSizing);
    EXPECT_EQ(oaFrac, oaFrac_Test);
}

TEST_F(EnergyPlusFixture, BaseSizer_setZoneCoilInletConditions)
{
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    DataSizing::ZoneEqSizingData &zoneEqSizing = state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum);
    state->dataSize->FinalZoneSizing.allocate(1);
    DataSizing::ZoneSizingData &finalZoneSizing = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum);

    zoneEqSizing.OAVolFlow = 0.34;
    zoneEqSizing.ATMixerVolFlow = 0.0;
    state->dataEnvrn->StdRhoAir = 1.23;

    Real64 DesMassFlow = 0.685;
    Real64 massFlowRate = state->dataEnvrn->StdRhoAir * zoneEqSizing.OAVolFlow;
    Real64 oaFrac = massFlowRate / DesMassFlow;

    // Test heating mode coil inlet temperature
    zoneEqSizing.ATMixerHeatPriDryBulb = 22.0;
    finalZoneSizing.ZoneRetTempAtHeatPeak = 25.0;
    finalZoneSizing.ZoneTempAtHeatPeak = 20.0;
    finalZoneSizing.OutTempAtHeatPeak = 10.0;

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition = zone condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 0.0;
    Real64 zoneCond = finalZoneSizing.ZoneTempAtHeatPeak;
    Real64 calcCoilInletCond = zoneCond;
    CoolingCapacitySizer sizer;
    Real64 coilInletCond = sizer.setHeatCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate > 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition based on mixed return and ATMixer condition
    zoneEqSizing.ATMixerVolFlow = 1.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneRetTempAtHeatPeak;
    Real64 oaCond = zoneEqSizing.ATMixerHeatPriDryBulb;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setHeatCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow > 0 so coilInlet condition based on mixed return and OA condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 1.0;
    zoneCond = finalZoneSizing.ZoneTempAtHeatPeak;
    oaCond = finalZoneSizing.OutTempAtHeatPeak;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setHeatCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // Test heating mode coil inlet humidity ratio
    zoneEqSizing.ATMixerHeatPriDryBulb = 0.0;
    finalZoneSizing.ZoneRetTempAtHeatPeak = 0.0;
    finalZoneSizing.ZoneTempAtHeatPeak = 0.0;
    finalZoneSizing.OutTempAtHeatPeak = 0.0;

    zoneEqSizing.ATMixerHeatPriHumRat = 0.008;
    finalZoneSizing.ZoneHumRatAtHeatPeak = 0.01;
    finalZoneSizing.OutHumRatAtHeatPeak = 0.003;

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition = zone condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneHumRatAtHeatPeak;
    calcCoilInletCond = zoneCond;
    coilInletCond = sizer.setHeatCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate > 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition based on mixed return and ATMixer condition
    zoneEqSizing.ATMixerVolFlow = 1.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneHumRatAtHeatPeak;
    oaCond = zoneEqSizing.ATMixerHeatPriHumRat;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setHeatCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow > 0 so coilInlet condition based on mixed return and OA condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 1.0;
    zoneCond = finalZoneSizing.ZoneHumRatAtHeatPeak;
    oaCond = finalZoneSizing.OutHumRatAtHeatPeak;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setHeatCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, calcCoilInletCond);

    // Test cooling mode coil inlet temperature
    zoneEqSizing.ATMixerHeatPriHumRat = 0.0;
    finalZoneSizing.ZoneHumRatAtHeatPeak = 0.0;
    finalZoneSizing.OutHumRatAtHeatPeak = 0.0;

    zoneEqSizing.ATMixerCoolPriDryBulb = 22.0;
    finalZoneSizing.ZoneRetTempAtCoolPeak = 25.0;
    finalZoneSizing.ZoneTempAtCoolPeak = 20.0;
    finalZoneSizing.OutTempAtCoolPeak = 10.0;

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition = zone condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneTempAtCoolPeak;
    calcCoilInletCond = zoneCond;
    coilInletCond = sizer.setCoolCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate > 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition based on mixed return and ATMixer condition
    zoneEqSizing.ATMixerVolFlow = 1.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneRetTempAtCoolPeak;
    oaCond = zoneEqSizing.ATMixerCoolPriDryBulb;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setCoolCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow > 0 so coilInlet condition based on mixed return and OA condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 1.0;
    zoneCond = finalZoneSizing.ZoneTempAtCoolPeak;
    oaCond = finalZoneSizing.OutTempAtCoolPeak;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setCoolCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // Test cooling mode coil inlet humidity ratio
    zoneEqSizing.ATMixerCoolPriDryBulb = 0.0;
    finalZoneSizing.ZoneRetTempAtCoolPeak = 0.0;
    finalZoneSizing.ZoneTempAtCoolPeak = 0.0;
    finalZoneSizing.OutTempAtCoolPeak = 0.0;

    zoneEqSizing.ATMixerCoolPriHumRat = 0.008;
    finalZoneSizing.ZoneHumRatAtCoolPeak = 0.01;
    finalZoneSizing.OutHumRatAtCoolPeak = 0.003;

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition = zone condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneHumRatAtCoolPeak;
    calcCoilInletCond = zoneCond;
    coilInletCond = sizer.setCoolCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate > 0 and ZoneEqSizing.OAVolFlow = 0 so coilInlet condition based on mixed return and ATMixer condition
    zoneEqSizing.ATMixerVolFlow = 1.0;
    zoneEqSizing.OAVolFlow = 0.0;
    zoneCond = finalZoneSizing.ZoneHumRatAtCoolPeak;
    oaCond = zoneEqSizing.ATMixerCoolPriHumRat;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setCoolCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    // ATMixer flow rate = 0 and ZoneEqSizing.OAVolFlow > 0 so coilInlet condition based on mixed return and OA condition
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 1.0;
    zoneCond = finalZoneSizing.ZoneHumRatAtCoolPeak;
    oaCond = finalZoneSizing.OutHumRatAtCoolPeak;
    calcCoilInletCond = (oaFrac * oaCond) + ((1.0 - oaFrac) * zoneCond);
    coilInletCond = sizer.setCoolCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_DOUBLE_EQ(coilInletCond, calcCoilInletCond);

    zoneEqSizing.ATMixerCoolPriHumRat = 0.0;
    finalZoneSizing.ZoneHumRatAtCoolPeak = 0.0;
    finalZoneSizing.OutHumRatAtCoolPeak = 0.0;

    // all ZoneEqSizing and FinalZoneSizing temp and humrat variables have been zero'd out now, all results should = 0
    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 0.0;
    coilInletCond = sizer.setHeatCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setHeatCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setCoolCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setCoolCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);

    zoneEqSizing.ATMixerVolFlow = 1.0;
    zoneEqSizing.OAVolFlow = 0.0;
    coilInletCond = sizer.setHeatCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setHeatCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setCoolCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setCoolCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);

    zoneEqSizing.ATMixerVolFlow = 0.0;
    zoneEqSizing.OAVolFlow = 1.0;
    coilInletCond = sizer.setHeatCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setHeatCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setCoolCoilInletTempForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
    coilInletCond = sizer.setCoolCoilInletHumRatForZoneEqSizing(oaFrac, zoneEqSizing, finalZoneSizing);
    EXPECT_EQ(coilInletCond, 0.0);
}

// This tests checks that the Design Day + Peak Time is filled up for Fans
// https://github.com/NREL/EnergyPlus/issues/6899
TEST_F(EnergyPlusFixture, BaseSizer_FanPeak)
{

    // This is needed to compute time of Peak as a string
    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->MinutesInTimeStep = 15;

    // Setup the predefined tables, because that's where the info is written.
    EnergyPlus::OutputReportPredefined::SetPredefinedTables(*state);

    // If you wanted to check SQL, you also need this:
    // We enable the report we care about, making sure it's the right one
    // EXPECT_EQ("EquipmentSummary", OutputReportPredefined::reportName(5).name);
    // OutputReportPredefined::reportName(5).show = true;

    int constexpr ZoneNum = 1;
    std::string CompName;       // component name
    std::string CompType;       // component type
    std::string SizingString;   // input field sizing description
    int SizingType;             // integerized type of sizing requested
    Real64 SizingResult;        // autosized value of coil input field
    bool PrintWarning;          // true when sizing information is reported in the eio file
    std::string CallingRoutine; // calling routine

    CompType = "Fan:ConstantVolume";
    CompName = "My Fan";
    SizingType = HVAC::SystemAirflowSizing;
    SizingString = "Nominal Capacity";
    SizingResult = DataSizing::AutoSize;
    PrintWarning = true;
    CallingRoutine = "Size Fan: ";
    state->dataSize->DataIsDXCoil = false;

    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.30;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum = 1;
    // Time of peak, should equal to 18:00:00
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax = 72;

    // Fake a design day
    state->dataWeather->DesDayInput.allocate(1);
    std::string DDTitle = "CHICAGO ANN CLG 1% CONDNS DB=>MWB";
    state->dataWeather->DesDayInput(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum).Title = DDTitle;
    state->dataWeather->DesDayInput(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum).Month = 7;
    state->dataWeather->DesDayInput(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum).DayOfMonth = 15;
    // Also need to set this, it's used to check if DDNum <= TotDesDays
    state->dataEnvrn->TotDesDays = 1;

    state->dataSize->ZoneSizingRunDone = true;

    // Need this to prevent crash in Sizers
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = ZoneNum;
    state->dataSize->NumZoneSizingInput = 1;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(SizingType) = DataSizing::SupplyAirFlowRate;

    // Now, we're ready to call the function
    bool errorsFound = false;
    SystemAirFlowSizer sizerSystemAirFlow;
    sizerSystemAirFlow.initializeWithinEP(*state, CompType, CompName, PrintWarning, CallingRoutine);
    SizingResult = sizerSystemAirFlow.size(*state, SizingResult, errorsFound);

    // Check that the Design Day/Time is filled
    EXPECT_EQ(DDTitle, OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchFanDesDay, CompName));
    EXPECT_EQ("7/15 18:00:00", OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchFanPkTime, CompName));

    // Bonus test for #6949
    EXPECT_EQ("End Use Subcategory", state->dataOutRptPredefined->columnTag(state->dataOutRptPredefined->pdchFanEndUse).heading);
}
TEST_F(EnergyPlusFixture, BaseSizer_SupplyAirTempLessThanZoneTStatTest)
{
    // GitHub issue 7039
    std::string const idf_objects = delimited_string({

        "  Timestep,1;",

        "  Building,",
        "    Simple One Zone,         !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.004,                   !- Temperature Convergence Tolerance Value {deltaC}",
        "    MinimalShadowing,        !- Solar Distribution",
        "    30,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  SimulationControl,",
        "    Yes,                     !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "    Site:Location,",
        "      Phoenix Sky Harbor Intl Ap_AZ_USA,  !- Name",
        "      33.45,                   !- Latitude {deg}",
        "      -111.98,                 !- Longitude {deg}",
        "      -7,                      !- Time Zone {hr}",
        "      337;                     !- Elevation {m}",

        "    Site:GroundTemperature:BuildingSurface,20.83,20.81,20.88,20.96,21.03,23.32,23.68,23.74,23.75,21.42,21.09,20.9;",

        "    SizingPeriod:DesignDay,",
        "      Phoenix Sky Harbor Intl Ap Ann Clg .4% Condns DB=>MWB,  !- Name",
        "      7,                       !- Month",
        "      21,                      !- Day of Month",
        "      SummerDesignDay,         !- Day Type",
        "      43.4,                    !- Maximum Dry-Bulb Temperature {C}",
        "      12,                      !- Daily Dry-Bulb Temperature Range {deltaC}",
        "      DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type",
        "      ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "      Wetbulb,                 !- Humidity Condition Type",
        "      21.1,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "      ,                        !- Humidity Condition Day Schedule Name",
        "      ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "      ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "      ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "      97342,                   !- Barometric Pressure {Pa}",
        "      4.1,                     !- Wind Speed {m/s}",
        "      260,                     !- Wind Direction {deg}",
        "      No,                      !- Rain Indicator",
        "      No,                      !- Snow Indicator",
        "      No,                      !- Daylight Saving Time Indicator",
        "      ASHRAETau,               !- Solar Model Indicator",
        "      ,                        !- Beam Solar Day Schedule Name",
        "      ,                        !- Diffuse Solar Day Schedule Name",
        "      0.588,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "      1.653;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  ",
        "    SizingPeriod:DesignDay,",
        "      Phoenix Sky Harbor Intl Ap Ann Htg 99.6% Condns DB,  !- Name",
        "      12,                      !- Month",
        "      21,                      !- Day of Month",
        "      WinterDesignDay,         !- Day Type",
        "      3.7,                     !- Maximum Dry-Bulb Temperature {C}",
        "      0,                       !- Daily Dry-Bulb Temperature Range {deltaC}",
        "      DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type",
        "      ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "      Wetbulb,                 !- Humidity Condition Type",
        "      3.7,                     !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "      ,                        !- Humidity Condition Day Schedule Name",
        "      ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "      ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "      ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "      97342,                   !- Barometric Pressure {Pa}",
        "      1.7,                     !- Wind Speed {m/s}",
        "      100,                     !- Wind Direction {deg}",
        "      No,                      !- Rain Indicator",
        "      No,                      !- Snow Indicator",
        "      No,                      !- Daylight Saving Time Indicator",
        "      ASHRAEClearSky,          !- Solar Model Indicator",
        "      ,                        !- Beam Solar Day Schedule Name",
        "      ,                        !- Diffuse Solar Day Schedule Name",
        "      ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "      ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "      0;                       !- Sky Clearness",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R31LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    5.456,                   !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    C5 - 4 IN HW CONCRETE,   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    FLOOR,                   !- Name",
        "    C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    R31LAYER;                !- Outside Layer",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,           !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,4.572000,            !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,0,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,           !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,0,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,           !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "    15.24000,15.24000,0,     !- X,Y,Z ==> Vertex 2 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 3 {m}",
        "    0,15.24000,4.572000;     !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,           !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,15.24000,4.572000,     !- X,Y,Z ==> Vertex 1 {m}",
        "    0,15.24000,0,            !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,4.572000;            !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR,                   !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zn001:Flr001,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    15.24000,0.000000,0.0,   !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    0.000000,15.24000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,0.0;   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Roof001,           !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,15.24000,4.572, !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,4.572, !- X,Y,Z ==> Vertex 2 {m}",
        "    15.24000,0.000000,4.572, !- X,Y,Z ==> Vertex 3 {m}",
        "    15.24000,15.24000,4.572; !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall001:Win001,    !- Name",
        "    Window,                  !- Surface Type",
        "    SimpleWindowConstruct,   !- Construction Name",
        "    Zn001:Wall001,           !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.548000,0,2.5000,       !- X,Y,Z ==> Vertex 1 {m}",
        "    0.548000,0,0.5000,       !- X,Y,Z ==> Vertex 2 {m}",
        "    5.548000,0,0.5000,       !- X,Y,Z ==> Vertex 3 {m}",
        "    5.548000,0,2.5000;       !- X,Y,Z ==> Vertex 4 {m}",

        "    Construction,",
        "      SimpleWindowConstruct,   !- Name",
        "      SimpleWindowTest;        !- Outside Layer",

        "    WindowMaterial:SimpleGlazingSystem,",
        "      SimpleWindowTest,        !- Name",
        "      0.600,                   !- U-Factor {W/m2-K}",
        "      0.700,                   !- Solar Heat Gain Coefficient",
        "      0.700;                   !- Visible Transmittance",

        "    Sizing:Zone,",
        "      ZONE ONE,                !- Zone or ZoneList Name",
        "      SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method",
        "      12.0,                    !- Zone Cooling Design Supply Air Temperature {C}",
        "      ,                        !- Zone Cooling Design Supply Air Temperature Difference {deltaC}",
        "      SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method",
        "      12.0,                    !- Zone Heating Design Supply Air Temperature {C}",
        "      ,                        !- Zone Heating Design Supply Air Temperature Difference {deltaC}",
        "      0.0075,                  !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      0.004,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        "      SZ DSOA Zone One,        !- Design Specification Outdoor Air Object Name",
        "      0.0,                     !- Zone Heating Sizing Factor",
        "      0.0,                     !- Zone Cooling Sizing Factor",
        "      DesignDay,               !- Cooling Design Air Flow Method",
        "      0,                       !- Cooling Design Air Flow Rate {m3/s}",
        "      ,                        !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}",
        "      ,                        !- Cooling Minimum Air Flow {m3/s}",
        "      ,                        !- Cooling Minimum Air Flow Fraction",
        "      DesignDay,               !- Heating Design Air Flow Method",
        "      0,                       !- Heating Design Air Flow Rate {m3/s}",
        "      ,                        !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}",
        "      ,                        !- Heating Maximum Air Flow {m3/s}",
        "      ;                        !- Heating Maximum Air Flow Fraction",

        "    DesignSpecification:OutdoorAir,",
        "      SZ DSOA Zone One,        !- Name",
        "      Sum,                     !- Outdoor Air Method",
        "      0.0,                     !- Outdoor Air Flow per Person {m3/s-person}",
        "      0.0,                     !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "      0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "    ZoneControl:Thermostat,",
        "      Zone Thermostat,         !- Name",
        "      ZONE ONE,                !- Zone or ZoneList Name",
        "      Zone Control Type Sched, !- Control Type Schedule Name",
        "      ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "      Temperature Setpoints;   !- Control 1 Name",

        "    Schedule:Compact,",
        "      Zone Control Type Sched, !- Name",
        "      Control Type,            !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,4;          !- Field 3",

        "    ThermostatSetpoint:DualSetpoint,",
        "      Temperature Setpoints,   !- Name",
        "      Heating Setpoints,       !- Heating Setpoint Temperature Schedule Name",
        "      Cooling Setpoints;       !- Cooling Setpoint Temperature Schedule Name",

        "    Schedule:Compact,",
        "      Heating Setpoints,       !- Name",
        "      Temperature,             !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,21.0;       !- Field 3",

        "    Schedule:Compact,",
        "      Cooling Setpoints,       !- Name",
        "      Temperature,             !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,24.0;       !- Field 3",

        "    ScheduleTypeLimits,",
        "      Control Type,            !- Name",
        "      0,                       !- Lower Limit Value",
        "      4,                       !- Upper Limit Value",
        "      DISCRETE;                !- Numeric Type",

        "    ScheduleTypeLimits,",
        "      Temperature,             !- Name",
        "      -60,                     !- Lower Limit Value",
        "      200,                     !- Upper Limit Value",
        "      CONTINUOUS,              !- Numeric Type",
        "      Temperature;             !- Unit Type",

        "    ZoneHVAC:EquipmentConnections,",
        "      ZONE ONE,                  !- Zone Name",
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

        "  People,",
        "      OpenOffice People,       !- Name",
        "      ZONE ONE,                !- Zone or ZoneList Name",
        "      BLDG_OCC_SCH,            !- Number of People Schedule Name",
        "      People/Area,             !- Number of People Calculation Method",
        "      ,                        !- Number of People",
        "      0.010,                    !- People per Zone Floor Area {person/m2}",
        "      ,                        !- Zone Floor Area per Person {m2/person}",
        "      0.3,                     !- Fraction Radiant",
        "      ,                        !- Sensible Heat Fraction",
        "      ACTIVITY_SCH;            !- Activity Level Schedule Name",

        "  Lights,",
        "      OfficeLights,            !- Name",
        "      ZONE ONE,                !- Zone or ZoneList Name",
        "      BLDG_LIGHT_SCH,          !- Schedule Name",
        "      Watts/Area,              !- Design Level Calculation Method",
        "      ,                        !- Lighting Level {W}",
        "      8.0,                    !- Watts per Zone Floor Area {W/m2}",
        "      ,                        !- Watts per Person {W/person}",
        "      ,                        !- Return Air Fraction",
        "      ,                        !- Fraction Radiant",
        "      ,                        !- Fraction Visible",
        "      ;                        !- Fraction Replaceable",

        "  ElectricEquipment,",
        "      ElectricEquipment,       !- Name",
        "      ZONE ONE,                !- Zone or ZoneList Name",
        "      BLDG_EQUIP_SCH,          !- Schedule Name",
        "      Watts/Area,              !- Design Level Calculation Method",
        "      ,                        !- Design Level {W}",
        "      8.0,                    !- Watts per Zone Floor Area {W/m2}",
        "      ,                        !- Watts per Person {W/person}",
        "      ,                        !- Fraction Latent",
        "      ,                        !- Fraction Radiant",
        "      ;                        !- Fraction Lost",

        "  Schedule:Compact,",
        "      BLDG_EQUIP_SCH,          !- Name",
        "      Fraction,                !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,            !- Field 3",
        "      0.7;                     !- Field 4",

        "  Schedule:Compact,",
        "      BLDG_LIGHT_SCH,          !- Name",
        "      Fraction,                !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,            !- Field 3",
        "      0.5;                     !- Field 4",

        "  Schedule:Compact,",
        "      BLDG_OCC_SCH,            !- Name",
        "      Fraction,                !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,            !- Field 3",
        "      0.5;                     !- Field 4",

        "  Schedule:Compact,",
        "      ACTIVITY_SCH,            !- Name",
        "      Any Number,              !- Schedule Type Limits Name",
        "      Through: 12/31,          !- Field 1",
        "      For: AllDays,            !- Field 2",
        "      Until: 24:00,            !- Field 3",
        "      120.;                    !- Field 4",

        "    ScheduleTypeLimits,",
        "      Any Number;              !- Name",

        "  Output:Table:SummaryReports,",
        "      AllSummary,                             !- Report Name 1",
        "      AllSummaryAndSizingPeriod;              !- Report Name 2",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    SimulationManager::ManageSimulation(*state);

    int CtrlZoneNum(1);
    // design peak load conditons and design supply air temperature
    EXPECT_EQ(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).HeatTstatTemp, 21.0); // expects specified value
    EXPECT_EQ(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).HeatDesTemp, 12.0);   // less than zone air Temp
    EXPECT_EQ(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).HeatDesDay, "PHOENIX SKY HARBOR INTL AP ANN HTG 99.6% CONDNS DB");
    // actual zone air temperature at peak load
    EXPECT_NEAR(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).ZoneTempAtHeatPeak, 17.08, 0.01);
    EXPECT_NEAR(state->dataSize->FinalZoneSizing(CtrlZoneNum).ZoneTempAtHeatPeak, 17.08, 0.01);
    // Check heating design flow rates, expected to be zero due the above conditions
    EXPECT_EQ(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).DesHeatVolFlow, 0.0);  // expects zero
    EXPECT_EQ(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).DesHeatMassFlow, 0.0); // expects zero
    EXPECT_EQ(state->dataSize->FinalZoneSizing(CtrlZoneNum).DesHeatVolFlow, 0.0);      // expects zero
    EXPECT_EQ(state->dataSize->FinalZoneSizing(CtrlZoneNum).DesHeatMassFlow, 0.0);     // expects zero
    // expects non-zero peak heating load
    EXPECT_NEAR(state->dataSize->CalcFinalZoneSizing(CtrlZoneNum).DesHeatLoad, 6911.42, 0.5);
    EXPECT_NEAR(state->dataSize->FinalZoneSizing(CtrlZoneNum).DesHeatLoad, 6911.42, 0.5);
}
