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

// EnergyPlus::OutputReportTabular Unit Tests

#include <tuple>

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include "Fixtures/SQLiteFixture.hh"

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/CondenserLoopTowers.hh>
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/PollutionModule.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/WindowModel.hh>

// C++ Headers
#include <algorithm>

using namespace EnergyPlus;
using namespace DataEnvironment;
using namespace DataHeatBalance;
using namespace DataSizing;
using namespace DataSurfaces;
using namespace OutputProcessor;
using namespace OutputReportPredefined;
using namespace OutputReportTabular;
using namespace OutputProcessor;
using namespace SimulationManager;

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ConfirmSetUnitsStyleFromString)
{

    EXPECT_ENUM_EQ(OutputReportTabular::UnitsStyle::None, SetUnitsStyleFromString("None"));
    EXPECT_ENUM_EQ(OutputReportTabular::UnitsStyle::JtoKWH, SetUnitsStyleFromString("JTOKWH"));
    EXPECT_ENUM_EQ(OutputReportTabular::UnitsStyle::JtoMJ, SetUnitsStyleFromString("JTOMJ"));
    EXPECT_ENUM_EQ(OutputReportTabular::UnitsStyle::JtoGJ, SetUnitsStyleFromString("JTOGJ"));
    EXPECT_ENUM_EQ(OutputReportTabular::UnitsStyle::InchPound, SetUnitsStyleFromString("INCHPOUND"));
    EXPECT_ENUM_EQ(OutputReportTabular::UnitsStyle::NotFound, SetUnitsStyleFromString("qqq"));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_Basic)
{
    state->dataOutRptTab->OutputTableBinned.allocate(10);
    EXPECT_TRUE(warningAboutKeyNotFound(*state, 0, 1, "moduleName"));
    EXPECT_FALSE(warningAboutKeyNotFound(*state, 100, 1, "moduleName"));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_RealToStr)
{
    EXPECT_EQ("       0.001", RealToStr(0.0011, 3));
    EXPECT_NE("       0.001", RealToStr(0.0019, 3));

    EXPECT_EQ("          1.", RealToStr(1.23456789, 0));
    EXPECT_EQ("         1.2", RealToStr(1.23456789, 1));
    EXPECT_EQ("        1.23", RealToStr(1.23456789, 2));
    EXPECT_EQ("       1.235", RealToStr(1.23456789, 3));
    EXPECT_EQ("      1.2346", RealToStr(1.23456789, 4));
    EXPECT_EQ("     1.23457", RealToStr(1.23456789, 5));
    EXPECT_EQ("    1.234568", RealToStr(1.23456789, 6));
    EXPECT_EQ("   1.2345679", RealToStr(1.23456789, 7));
    EXPECT_EQ("  1.23456789", RealToStr(1.23456789, 8));

    EXPECT_EQ("    1.234000", RealToStr(1.234, 6));
    EXPECT_EQ("   1.2340000", RealToStr(1.234, 7));
    EXPECT_EQ("  1.23400000", RealToStr(1.234, 8));

    EXPECT_EQ("     123457.", RealToStr(123456.789, 0));
    EXPECT_EQ("    123456.8", RealToStr(123456.789, 1));
    EXPECT_EQ("   123456.79", RealToStr(123456.789, 2));
    EXPECT_EQ("  123456.789", RealToStr(123456.789, 3));
    EXPECT_EQ(" 123456.7890", RealToStr(123456.789, 4));

    EXPECT_EQ("1.234568E+05", RealToStr(123456.789, 5));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_isNumber)
{
    EXPECT_TRUE(isNumber("0"));
    EXPECT_TRUE(isNumber("0.12"));
    EXPECT_TRUE(isNumber("0.12E01"));
    EXPECT_TRUE(isNumber("-6"));
    EXPECT_TRUE(isNumber("-6.12"));
    EXPECT_TRUE(isNumber("-6.12E-09"));
    EXPECT_TRUE(isNumber(" 0"));
    EXPECT_TRUE(isNumber(" 0.12"));
    EXPECT_TRUE(isNumber(" 0.12E01"));
    EXPECT_TRUE(isNumber("0 "));
    EXPECT_TRUE(isNumber("0.12 "));
    EXPECT_TRUE(isNumber("0.12E01 "));
    EXPECT_TRUE(isNumber(" 0 "));
    EXPECT_TRUE(isNumber(" 0.12 "));
    EXPECT_TRUE(isNumber(" 0.12E01 "));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_digitsAferDecimal)
{
    EXPECT_EQ(0, digitsAferDecimal("0"));
    EXPECT_EQ(0, digitsAferDecimal("1."));
    EXPECT_EQ(2, digitsAferDecimal("0.12"));
    EXPECT_EQ(4, digitsAferDecimal("0.1234"));
    EXPECT_EQ(2, digitsAferDecimal("3.12E01"));
    EXPECT_EQ(0, digitsAferDecimal("-6"));
    EXPECT_EQ(0, digitsAferDecimal("-6."));
    EXPECT_EQ(2, digitsAferDecimal("-6.12"));
    EXPECT_EQ(5, digitsAferDecimal("-6.12765"));
    EXPECT_EQ(2, digitsAferDecimal("-6.12E-09"));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_splitCommaString)
{
    std::vector<std::string> actual;
    actual.push_back("part1");
    EXPECT_EQ(actual, splitCommaString("part1"));
    actual.push_back("part2");
    EXPECT_EQ(actual, splitCommaString("part1,part2"));
    EXPECT_EQ(actual, splitCommaString(" part1,part2 "));
    EXPECT_EQ(actual, splitCommaString(" part1 , part2 "));
    actual.push_back("part3");
    EXPECT_EQ(actual, splitCommaString("part1,part2,part3"));
    EXPECT_EQ(actual, splitCommaString(" part1 , part2 , part3 "));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_stringJoinDelimiter)
{
    std::vector<std::string> original;
    EXPECT_EQ("", stringJoinDelimiter(original, ";"));
    original.push_back("part1");
    EXPECT_EQ("part1", stringJoinDelimiter(original, ";"));
    original.push_back("part2");
    EXPECT_EQ("part1;part2", stringJoinDelimiter(original, ";"));
    EXPECT_EQ("part1 ; part2", stringJoinDelimiter(original, " ; "));
    original.push_back("part3");
    EXPECT_EQ("part1;part2;part3", stringJoinDelimiter(original, ";"));
    EXPECT_EQ("part1 ; part2 ; part3", stringJoinDelimiter(original, " ; "));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_unitsFromHeading)
{
    std::string unitString;
    int indexUnitConv;
    std::string curUnits;
    Real64 curConversionFactor;
    Real64 curConversionOffset;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;

    unitString = "";
    EXPECT_EQ(97, unitsFromHeading(*state, unitString));
    EXPECT_EQ("", unitString);
    unitString = "Zone Floor Area {m2}";
    EXPECT_EQ(46, unitsFromHeading(*state, unitString));
    EXPECT_EQ("Zone Floor Area {ft2}", unitString);
    unitString = "Fictional field {nonsense}";
    EXPECT_EQ(0, unitsFromHeading(*state, unitString));
    EXPECT_EQ("Fictional field {nonsense}", unitString);

    // Check a few report column headings too
    unitString = "Standard Rated Net Cooling Capacity [W]";
    indexUnitConv = unitsFromHeading(*state, unitString);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    // We expect W to convert to tons because it's cooling
    EXPECT_EQ(70, indexUnitConv);
    EXPECT_EQ("ton", curUnits);
    EXPECT_EQ(0.0002843333, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    unitString = "Rated Net Cooling Capacity Test A [W]";
    indexUnitConv = unitsFromHeading(*state, unitString);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    // We expect W to convert to tons because it's cooling
    EXPECT_EQ(70, indexUnitConv);
    EXPECT_EQ("ton", curUnits);
    EXPECT_EQ(0.0002843333, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ConfirmResourceWarning)
{
    EXPECT_EQ("In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: Electricity [kWh]",
              ResourceWarningMessage("Electricity [kWh]"));
    EXPECT_EQ("In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: Natural Gas [kWh]",
              ResourceWarningMessage("Natural Gas [kWh]"));
    EXPECT_EQ(
        "In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: Additional Fuel [kWh]",
        ResourceWarningMessage("Additional Fuel [kWh]"));
    EXPECT_EQ(
        "In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: District Cooling [kBtu]",
        ResourceWarningMessage("District Cooling [kBtu]"));
    EXPECT_EQ("In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: District Heating "
              "Water [kBtu]",
              ResourceWarningMessage("District Heating Water [kBtu]"));
    EXPECT_EQ("In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: Water [GJ]",
              ResourceWarningMessage("Water [GJ]"));
    EXPECT_EQ("In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: Electricity [GJ]",
              ResourceWarningMessage("Electricity [GJ]"));
    EXPECT_NE("In the Annual Building Utility Performance Summary Report the total row does not match the sum of the column for: Gas [kWh]",
              ResourceWarningMessage("Electricity [kWh]"));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ConfirmWaterConversion)
{
    EXPECT_EQ(15, WaterConversionFunct(75, 5));
    EXPECT_EQ(1, WaterConversionFunct(1, 1));
    EXPECT_EQ(13.756, WaterConversionFunct(481.46, 35));
    EXPECT_EQ(-2, WaterConversionFunct(-12, 6));
    EXPECT_NE(15, WaterConversionFunct(135, 5));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetUnitConversion)
{
    int indexUnitConv;
    std::string curUnits;
    Real64 curConversionFactor;
    Real64 curConversionOffset;
    std::string varNameWithUnits;

    SetupUnitConversions(*state);

    varNameWithUnits = "ZONE AIR SYSTEM SENSIBLE COOLING RATE[W]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(70, indexUnitConv);
    EXPECT_EQ("ton", curUnits);
    EXPECT_EQ(0.0002843333, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    varNameWithUnits = "SITE OUTDOOR AIR DRYBULB TEMPERATURE[C]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(11, indexUnitConv);
    EXPECT_EQ("F", curUnits);
    EXPECT_EQ(1.8, curConversionFactor);
    EXPECT_EQ(32., curConversionOffset);

    varNameWithUnits = "SET > 30°C DEGREE-HOURS [°C·hr]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(118, indexUnitConv);
    EXPECT_EQ("°F·hr", curUnits);
    EXPECT_EQ(1.8, curConversionFactor);

    varNameWithUnits = "ZONE ELECTRIC EQUIPMENT ELECTRICITY ENERGY[J]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(20, indexUnitConv);
    EXPECT_EQ("kWh", curUnits);
    EXPECT_EQ(0.000000277778, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    varNameWithUnits = "ZONE COOLING SETPOINT NOT MET TIME[hr]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(17, indexUnitConv);
    EXPECT_EQ("hr", curUnits);
    EXPECT_EQ(1.0, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    varNameWithUnits = "ZONE LIGHTS TOTAL HEATING ENERGY[Invalid/Undefined]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(96, indexUnitConv);
    EXPECT_EQ("Invalid/Undefined", curUnits);
    EXPECT_EQ(1.0, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    varNameWithUnits = "FICTIONAL VARIABLE[qqq]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(0, indexUnitConv);
    EXPECT_EQ("", curUnits);
    EXPECT_EQ(1.0, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    varNameWithUnits = "ZONE PEOPLE OCCUPANT COUNT[]";
    LookupSItoIP(*state, varNameWithUnits, indexUnitConv, curUnits);
    GetUnitConversion(*state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    EXPECT_EQ(97, indexUnitConv);
    EXPECT_EQ("", curUnits);
    EXPECT_EQ(1.0, curConversionFactor);
    EXPECT_EQ(0.0, curConversionOffset);

    std::vector<std::string> units = {
        "[ ]",
        "[%]",
        "[]",
        "[A]",
        "[ach]",
        "[Ah]",
        "[C]",
        "[cd/m2]",
        "[clo]",
        "[deg]",
        "[deltaC]",
        "[hr]",
        "[J/kg]",
        "[J/kg-K]",
        "[J/kgWater]",
        "[J/m2]",
        "[J]",
        "[K/m]",
        "[kg/kg]",
        "[kg/m3]",
        "[kg/s]",
        "[kg]",
        "[kgWater/kgDryAir]",
        "[kgWater/s]",
        "[kmol/s]",
        "[L]",
        "[lum/W]",
        "[lux]",
        "[m/s]",
        "[m]",
        "[m2]",
        "[m3/s]",
        "[m3]",
        "[min]",
        "[Pa]",
        "[ppm]",
        "[rad]",
        "[rev/min]",
        "[s]",
        "[V]",
        "[W/K]",
        "[W/m2]",
        "[W/m2-C]",
        "[W/m2-K]",
        "[W/W]",
        "[W]",
        "[person/m2]",
    };

    for (auto u : units) {
        LookupSItoIP(*state, u, indexUnitConv, curUnits);
        EXPECT_NE(indexUnitConv, 0);
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_LookupJtokWH)
{
    int indexUnitConv;
    std::string curUnits;
    std::string varNameWithUnits;

    SetupUnitConversions(*state);

    varNameWithUnits = "ZONE AIR SYSTEM SENSIBLE COOLING RATE[W]";
    LookupJtokWH(*state, varNameWithUnits, indexUnitConv, curUnits);
    EXPECT_EQ(0, indexUnitConv);
    EXPECT_EQ("ZONE AIR SYSTEM SENSIBLE COOLING RATE[W]", curUnits);

    varNameWithUnits = "Electricity Energy Use [GJ]";
    LookupJtokWH(*state, varNameWithUnits, indexUnitConv, curUnits);
    EXPECT_EQ(86, indexUnitConv);
    EXPECT_EQ("Electricity Energy Use [kWh]", curUnits);

    varNameWithUnits = "Electricity [MJ/m2]";
    LookupJtokWH(*state, varNameWithUnits, indexUnitConv, curUnits);
    EXPECT_EQ(95, indexUnitConv);
    EXPECT_EQ("Electricity [kWh/m2]", curUnits);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetColumnUsingTabs)
{
    {
        std::string inString = " Col1 \t Col2 \t Col3 ";
        EXPECT_EQ(" Col1 ", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ(" Col2 ", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ(" Col3 ", GetColumnUsingTabs(inString, 3));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 4));
    }

    {
        std::string inString = "Col1\tCol2\tCol3";
        EXPECT_EQ("Col1", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("Col2", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ("Col3", GetColumnUsingTabs(inString, 3));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 4));
    }

    {
        std::string inString = "Col1\tCol2\tCol3\t";
        EXPECT_EQ("Col1", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("Col2", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ("Col3", GetColumnUsingTabs(inString, 3));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 4));
    }

    {
        std::string inString = "";
        EXPECT_EQ("", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 2));
    }

    {
        std::string inString = " ";
        EXPECT_EQ(" ", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 2));
    }

    {
        std::string inString = "\t";
        EXPECT_EQ("", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 3));
    }

    {
        std::string inString = " \t ";
        EXPECT_EQ(" ", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ(" ", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 3));
    }

    {
        std::string inString = "\tCol1\tCol2\tCol3\t";
        EXPECT_EQ("", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("Col1", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ("Col2", GetColumnUsingTabs(inString, 3));
        EXPECT_EQ("Col3", GetColumnUsingTabs(inString, 4));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 5));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 6));
    }

    {
        std::string inString = "Col1\t\tCol2\tCol3\t";
        EXPECT_EQ("Col1", GetColumnUsingTabs(inString, 1));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 2));
        EXPECT_EQ("Col2", GetColumnUsingTabs(inString, 3));
        EXPECT_EQ("Col3", GetColumnUsingTabs(inString, 4));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 5));
        EXPECT_EQ("", GetColumnUsingTabs(inString, 6));
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_AllocateLoadComponentArraysTest)
{
    state->dataEnvrn->TotDesDays = 2;
    state->dataEnvrn->TotRunDesPersDays = 3;
    state->dataGlobal->NumOfZones = 4;
    state->dataViewFactor->NumOfRadiantEnclosures = 4;
    state->dataSurface->TotSurfaces = 7;
    state->dataGlobal->TimeStepsInHour = 4;

    AllocateLoadComponentArrays(*state);

    // radiantPulseTimestep.allocate( { 0, TotDesDays + TotRunDesPersDays }, NumOfZones );
    EXPECT_EQ(state->dataOutRptTab->radiantPulseTimestep.size(), 24u);

    // radiantPulseReceived.allocate( { 0, TotDesDays + TotRunDesPersDays }, TotSurfaces );
    EXPECT_EQ(state->dataOutRptTab->radiantPulseReceived.size(), 42u);

    // decayCurveCool.allocate( TimeStepsInHour * 24, TotSurfaces );
    EXPECT_EQ(state->dataOutRptTab->decayCurveCool.size(), 672u);

    // decayCurveHeat.allocate( TimeStepsInHour * 24, TotSurfaces );
    EXPECT_EQ(state->dataOutRptTab->decayCurveHeat.size(), 672u);

    EXPECT_EQ(state->dataOutRptTab->surfCompLoads.size(), 5u);
    EXPECT_EQ(state->dataOutRptTab->surfCompLoads[0].ts.size(), 96u);
    EXPECT_EQ(state->dataOutRptTab->surfCompLoads[0].ts[0].surf.size(), 7u);
    EXPECT_EQ(state->dataOutRptTab->surfCompLoads[4].ts.size(), 96u);
    EXPECT_EQ(state->dataOutRptTab->surfCompLoads[4].ts[95].surf.size(), 7u);

    EXPECT_EQ(state->dataOutRptTab->znCompLoads.size(), 5u);
    EXPECT_EQ(state->dataOutRptTab->znCompLoads[0].ts.size(), 96u);
    EXPECT_EQ(state->dataOutRptTab->znCompLoads[0].ts[0].spacezone.size(), 4u);
    EXPECT_EQ(state->dataOutRptTab->znCompLoads[4].ts.size(), 96u);
    EXPECT_EQ(state->dataOutRptTab->znCompLoads[4].ts[95].spacezone.size(), 4u);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ConfirmConvertToEscaped)
{
    // check xml
    EXPECT_EQ("", ConvertToEscaped("", true));
    EXPECT_EQ(" ", ConvertToEscaped(" ", true));
    EXPECT_EQ("Xml string with &gt; in it", ConvertToEscaped("Xml string with > in it", true));
    EXPECT_EQ("Xml string with &lt; in it", ConvertToEscaped("Xml string with < in it", true));
    EXPECT_EQ("Xml string with &amp; in it", ConvertToEscaped("Xml string with & in it", true));
    EXPECT_EQ("Xml string with &quot; in it", ConvertToEscaped("Xml string with \" in it", true));
    EXPECT_EQ("Xml string with &apos; in it", ConvertToEscaped("Xml string with \' in it", true));
    EXPECT_EQ("Xml string with &quot; in it", ConvertToEscaped(R"(Xml string with \" in it)", true));
    EXPECT_EQ("Xml string with &apos; in it", ConvertToEscaped(R"(Xml string with \' in it)", true));
    EXPECT_EQ("気", ConvertToEscaped("気", true)); // Don't mangle the Japanese character for air in unicode.
    EXPECT_EQ(std::string("Xml string with ") + char(176) + std::string(" in it"),
              ConvertToEscaped(std::string("Xml string with ") + char(176) + std::string(" in it"), true));
    // Don't escape degree symbol with &deg; in xml since it is not a valid escape.

    EXPECT_EQ("Xml string with \xC2 in it", ConvertToEscaped("Xml string with \xC2 in it", true));
    EXPECT_EQ("Xml string with \xC2\xB1 in it", ConvertToEscaped("Xml string with \xC2\xB1 in it", true));
    EXPECT_EQ("Xml string with ≤ in it", ConvertToEscaped("Xml string with ≤ in it", true));
    // Don't escape less than or equal symbol with &le; in xml since it is not a valid escape.

    EXPECT_EQ("Xml string with ≥; in it", ConvertToEscaped("Xml string with ≥; in it", true));
    EXPECT_ANY_THROW(ConvertToEscaped(R"(Xml string with \u in it)", true));
    EXPECT_ANY_THROW(ConvertToEscaped(R"(Xml string with \x in it)", true));

    // check html
    EXPECT_EQ("", ConvertToEscaped("", false));
    EXPECT_EQ(" ", ConvertToEscaped(" ", false));
    EXPECT_EQ("Html string with &gt; in it", ConvertToEscaped("Html string with > in it", false));
    EXPECT_EQ("Html string with &lt; in it", ConvertToEscaped("Html string with < in it", false));
    EXPECT_EQ("Html string with &amp; in it", ConvertToEscaped("Html string with & in it", false));
    EXPECT_EQ("Html string with &deg; in it", ConvertToEscaped(std::string("Html string with ") + char(176) + std::string(" in it"), false));
    EXPECT_EQ("Html string with &deg; in it", ConvertToEscaped("Html string with \u00B0 in it", false));
    EXPECT_EQ("Html string with &deg; in it", ConvertToEscaped("Html string with \xB0 in it", false));
    EXPECT_EQ("Html string with &deg; in it", ConvertToEscaped("Html string with \xC2\xB0 in it", false));
    EXPECT_EQ("Html string with \xC2 in it", ConvertToEscaped("Html string with \xC2 in it", false));
    EXPECT_EQ("Html string with \xC2\xB1 in it", ConvertToEscaped("Html string with \xC2\xB1 in it", false));
    EXPECT_EQ("Html string with &deg; in it", ConvertToEscaped(R"(Html string with \u00B0 in it)", false));
    EXPECT_EQ("Html string with &deg; in it", ConvertToEscaped(R"(Html string with \xB0 in it)", false));
    EXPECT_EQ("Html string with &le; in it", ConvertToEscaped("Html string with ≤ in it", false));
    EXPECT_EQ("Html string with &ge; in it", ConvertToEscaped("Html string with ≥ in it", false));
    EXPECT_ANY_THROW(ConvertToEscaped(R"(Html string with \u in it)", false));
    EXPECT_ANY_THROW(ConvertToEscaped(R"(Html string with \x in it)", false));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ConvertUnicodeToUTF8)
{
    {
        std::string test;
        test += static_cast<char>(0);
        EXPECT_EQ(test, ConvertUnicodeToUTF8(std::stoul("0x0000", nullptr, 16)));
    }
    EXPECT_EQ("\x7F", ConvertUnicodeToUTF8(std::stoul("0x7F", nullptr, 16)));
    EXPECT_EQ("\xC2\xB0", ConvertUnicodeToUTF8(std::stoul("0xB0", nullptr, 16)));
    EXPECT_EQ("\xC2\xB0", ConvertUnicodeToUTF8(std::stoul("0x00B0", nullptr, 16)));
    EXPECT_EQ("\xEF\xBF\xBF", ConvertUnicodeToUTF8(std::stoul("0xFFFF", nullptr, 16)));
    EXPECT_EQ("\xF4\x8F\xBF\xBF", ConvertUnicodeToUTF8(std::stoul("0x10FFFF", nullptr, 16)));
    EXPECT_EQ("", ConvertUnicodeToUTF8(std::stoul("0x110000", nullptr, 16)));
    EXPECT_EQ("", ConvertUnicodeToUTF8(std::stoul("0x1FFFFF", nullptr, 16)));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetUnitSubStringTest)
{
    EXPECT_EQ("", GetUnitSubString(""));
    EXPECT_EQ("", GetUnitSubString(" "));
    EXPECT_EQ("J/KG", GetUnitSubString("[J/KG]"));
    EXPECT_EQ("M3/S-PERSON", GetUnitSubString(" [M3/S-PERSON]")); // leading space
    EXPECT_EQ("W/M2-K", GetUnitSubString("[W/M2-K] "));           // trailing space
    EXPECT_EQ("MJ/m2", GetUnitSubString(" [MJ/m2] "));            // leading and trailing space
    EXPECT_EQ("PA", GetUnitSubString("This is a column header with units [PA] "));
    EXPECT_EQ("W", GetUnitSubString("This is a column header with units [W] "));
    EXPECT_EQ("K/M", GetUnitSubString("This is a column header with units [K/M] and trailing text."));
}

TEST_F(EnergyPlusFixture, OutputReportTabular_ZoneMultiplierTest)
{
    // AUTHOR: R. Raustad, FSEC
    // DATE WRITTEN: Sep 2015

    std::string const idf_objects = delimited_string({

        " Output:Diagnostics, DisplayExtraWarnings;",
        "  Timestep, 4;",

        "BUILDING, OutputReportTabular_ZoneMultiplierTest, 0.0, Suburbs, .04, .4, FullExterior, 25, 6;",

        "SimulationControl, YES, NO, NO, YES, NO;",

        "  Site:Location,",
        "    Miami Intl Ap FL USA TMY3 WMO=722020E,    !- Name",
        "    25.82,                   !- Latitude {deg}",
        "    -80.30,                  !- Longitude {deg}",
        "    -5.00,                   !- Time Zone {hr}",
        "    11;                      !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        " Miami Intl Ap Ann Clg 1% Condns DB/MCWB, !- Name",
        " 7,                        !- Month",
        " 21,                       !- Day of Month",
        " SummerDesignDay,          !- Day Type",
        " 31.7,                     !- Maximum Dry - Bulb Temperature{ C }",
        " 10.0,                      !- Daily Dry - Bulb Temperature Range{ deltaC }",
        " ,                         !- Dry - Bulb Temperature Range Modifier Type",
        " ,                         !- Dry - Bulb Temperature Range Modifier Day Schedule Name",
        " Wetbulb,                  !- Humidity Condition Type",
        " 22.7,                     !- Wetbulb or DewPoint at Maximum Dry - Bulb{ C }",
        " ,                         !- Humidity Condition Day Schedule Name",
        " ,                         !- Humidity Ratio at Maximum Dry - Bulb{ kgWater / kgDryAir }",
        " ,                         !- Enthalpy at Maximum Dry - Bulb{ J / kg }",
        " ,                         !- Daily Wet - Bulb Temperature Range{ deltaC }",
        " 101217.,                  !- Barometric Pressure{ Pa }",
        " 3.8,                      !- Wind Speed{ m / s }",
        " 340,                      !- Wind Direction{ deg }",
        " No,                       !- Rain Indicator",
        " No,                       !- Snow Indicator",
        " No,                       !- Daylight Saving Time Indicator",
        " ASHRAEClearSky,           !- Solar Model Indicator",
        " ,                         !- Beam Solar Day Schedule Name",
        " ,                         !- Diffuse Solar Day Schedule Name",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Beam Irradiance( taub ) { dimensionless }",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance( taud ) { dimensionless }",
        " 1.00;                     !- Sky Clearness",

        "SizingPeriod:DesignDay,",
        " Miami Intl Ap Ann Htg 99.6% Condns DB, !- Name",
        " 1,                        !- Month",
        " 21,                       !- Day of Month",
        " WinterDesignDay,          !- Day Type",
        " 8.7,                      !- Maximum Dry - Bulb Temperature{ C }",
        " 0.0,                      !- Daily Dry - Bulb Temperature Range{ deltaC }",
        " ,                         !- Dry - Bulb Temperature Range Modifier Type",
        " ,                         !- Dry - Bulb Temperature Range Modifier Day Schedule Name",
        " Wetbulb,                  !- Humidity Condition Type",
        " 8.7,                      !- Wetbulb or DewPoint at Maximum Dry - Bulb{ C }",
        " ,                         !- Humidity Condition Day Schedule Name",
        " ,                         !- Humidity Ratio at Maximum Dry - Bulb{ kgWater / kgDryAir }",
        " ,                         !- Enthalpy at Maximum Dry - Bulb{ J / kg }",
        " ,                         !- Daily Wet - Bulb Temperature Range{ deltaC }",
        " 101217.,                  !- Barometric Pressure{ Pa }",
        " 3.8,                      !- Wind Speed{ m / s }",
        " 340,                      !- Wind Direction{ deg }",
        " No,                       !- Rain Indicator",
        " No,                       !- Snow Indicator",
        " No,                       !- Daylight Saving Time Indicator",
        " ASHRAEClearSky,           !- Solar Model Indicator",
        " ,                         !- Beam Solar Day Schedule Name",
        " ,                         !- Diffuse Solar Day Schedule Name",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Beam Irradiance( taub ) { dimensionless }",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance( taud ) { dimensionless }",
        " 0.00;                     !- Sky Clearness",

        "OutputControl:Table:Style,",
        "  HTML;                    !- Column Separator",
        " ",
        "Output:Table:SummaryReports,",
        "  AllSummary; !- Report 1 Name",
        " ",
        "Zone,",
        "  Space,                   !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  2.4,                     !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",
        " ",
        "ZoneGroup,",
        " Zone Group,               !- Name",
        " Zone List,                !- Zone List Name",
        " 10;                       !- Zone List Multiplier",
        " ",
        "ZoneList,",
        " Zone List,                !- Name",
        " Spacex10;                 !- Zone 1 Name",
        " ",
        "Zone,",
        "  Spacex10,                !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  2.4,                     !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",
        " ",
        "Sizing:Zone,",
        " Space,                    !- Zone or ZoneList Name",
        " SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
        " 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
        " ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
        " SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
        " 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
        " ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
        " 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " SZ DSOA,                  !- Design Specification Outdoor Air Object Name",
        " 0.0,                      !- Zone Heating Sizing Factor",
        " 0.0,                      !- Zone Cooling Sizing Factor",
        " DesignDay,                !- Cooling Design Air Flow Method",
        " 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Cooling Minimum Air Flow{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow Fraction",
        " DesignDay,                !- Heating Design Air Flow Method",
        " 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
        " ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Heating Maximum Air Flow{ m3 / s }",
        " ;                         !- Heating Maximum Air Flow Fraction",
        " ",
        "Sizing:Zone,",
        " Spacex10,                 !- Zone or ZoneList Name",
        " SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
        " 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
        " ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
        " SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
        " 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
        " ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
        " 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " SZ DSOA,                  !- Design Specification Outdoor Air Object Name",
        " 0.0,                      !- Zone Heating Sizing Factor",
        " 0.0,                      !- Zone Cooling Sizing Factor",
        " DesignDay,                !- Cooling Design Air Flow Method",
        " 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Cooling Minimum Air Flow{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow Fraction",
        " DesignDay,                !- Heating Design Air Flow Method",
        " 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
        " ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Heating Maximum Air Flow{ m3 / s }",
        " ;                         !- Heating Maximum Air Flow Fraction",
        " ",
        "DesignSpecification:OutdoorAir,",
        " SZ DSOA,                  !- Name",
        " flow/person,              !- Outdoor Air Method",
        " 0.00944,                  !- Outdoor Air Flow per Person{ m3 / s - person }",
        " 0.0,                      !- Outdoor Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " 0.0;                      !- Outdoor Air Flow per Zone{ m3 / s }",
        " ",
        "ZoneHVAC:EquipmentConnections,",
        " Space,                    !- Zone Name",
        " Space Eq,                 !- Zone Conditioning Equipment List Name",
        " Space In Node,            !- Zone Air Inlet Node or NodeList Name",
        " Space Out Node,           !- Zone Air Exhaust Node or NodeList Name",
        " Space Node,               !- Zone Air Node Name",
        " Space Ret Node;           !- Zone Return Air Node Name",
        " ",
        "ZoneHVAC:EquipmentConnections,",
        " Spacex10,                 !- Zone Name",
        " Spacex10 Eq,              !- Zone Conditioning Equipment List Name",
        " Spacex10 In Node,         !- Zone Air Inlet Node or NodeList Name",
        " Spacex10 Out Node,        !- Zone Air Exhaust Node or NodeList Name",
        " Spacex10 Node,            !- Zone Air Node Name",
        " Spacex10 Ret Node;        !- Zone Return Air Node Name",
        " ",
        "ZoneHVAC:EquipmentList,",
        " Space Eq,                 !- Name",
        " SequentialLoad,           !- Load Distribution Scheme",
        " ZoneHVAC:WindowAirConditioner, !- Zone Equipment 1 Object Type",
        " WindAC,                   !- Zone Equipment 1 Name",
        " 1,                        !- Zone Equipment 1 Cooling Sequence",
        " 1;                        !- Zone Equipment 1 Heating or No - Load Sequence",
        " ",
        "ZoneHVAC:EquipmentList,",
        " Spacex10 Eq,              !- Name",
        " SequentialLoad,           !- Load Distribution Scheme",
        " ZoneHVAC:WindowAirConditioner, !- Zone Equipment 1 Object Type",
        " WindACx10,                !- Zone Equipment 1 Name",
        " 1,                        !- Zone Equipment 1 Cooling Sequence",
        " 1;                        !- Zone Equipment 1 Heating or No - Load Sequence",
        " ",
        "ZoneHVAC:WindowAirConditioner,",
        " WindAC,                   !- Name",
        " AvailSched,               !- Availability Schedule Name",
        " autosize,                 !- Maximum Supply Air Flow Rate{ m3 / s }",
        " autosize,                 !- Maximum Outdoor Air Flow Rate{ m3 / s }",
        " Space Out Node,           !- Air Inlet Node Name",
        " Space In Node,            !- Air Outlet Node Name",
        " OutdoorAir:Mixer,         !- Outdoor Air Mixer Object Type",
        " WindACOAMixer,            !- Outdoor Air Mixer Name",
        " Fan:OnOff,                !- Supply Air Fan Object Type",
        " WindACFan,                !- Supply Air Fan Name",
        " Coil:Cooling:DX:SingleSpeed, !- Cooling Coil Object Type",
        " WindACDXCoil,             !- DX Cooling Coil Name",
        " FanOpModeSchedule,        !- Supply Air Fan Operating Mode Schedule Name",
        " BlowThrough,              !- Fan Placement",
        " 0.001;                    !- Cooling Convergence Tolerance",
        " ",
        "ZoneHVAC:WindowAirConditioner,",
        " WindACx10,                !- Name",
        " AvailSched,               !- Availability Schedule Name",
        " autosize,                 !- Maximum Supply Air Flow Rate{ m3 / s }",
        " autosize,                 !- Maximum Outdoor Air Flow Rate{ m3 / s }",
        " Spacex10 Out Node,        !- Air Inlet Node Name",
        " Spacex10 In Node,         !- Air Outlet Node Name",
        " OutdoorAir:Mixer,         !- Outdoor Air Mixer Object Type",
        " WindACx10OAMixer,         !- Outdoor Air Mixer Name",
        " Fan:OnOff,                !- Supply Air Fan Object Type",
        " WindACx10Fan,             !- Supply Air Fan Name",
        " Coil:Cooling:DX:SingleSpeed, !- Cooling Coil Object Type",
        " WindACx10DXCoil,          !- DX Cooling Coil Name",
        " FanOpModeSchedule,        !- Supply Air Fan Operating Mode Schedule Name",
        " BlowThrough,              !- Fan Placement",
        " 0.001;                    !- Cooling Convergence Tolerance",
        " ",
        "OutdoorAir:Mixer,",
        " WindACOAMixer,            !- Name",
        " WindACOAMixerOutletNode,  !- Mixed Air Node Name",
        " WindACOAInNode,           !- Outdoor Air Stream Node Name",
        " WindACExhNode,            !- Relief Air Stream Node Name",
        " Space Out Node;           !- Return Air Stream Node Name",
        " ",
        "OutdoorAir:Mixer,",
        " WindACx10OAMixer,         !- Name",
        " WindACx10OAMixerOutletNode, !- Mixed Air Node Name",
        " WindACx10OAInNode,        !- Outdoor Air Stream Node Name",
        " WindACx10ExhNode,         !- Relief Air Stream Node Name",
        " Spacex10 Out Node;        !- Return Air Stream Node Name",
        " ",
        "Fan:OnOff,",
        " WindACFan,                !- Name",
        " AvailSched,               !- Availability Schedule Name",
        " 0.5,                      !- Fan Total Efficiency",
        " 75.0,                     !- Pressure Rise{ Pa }",
        " autosize,                 !- Maximum Flow Rate{ m3 / s }",
        " 0.9,                      !- Motor Efficiency",
        " 1.0,                      !- Motor In Airstream Fraction",
        " WindACOAMixerOutletNode,  !- Air Inlet Node Name",
        " WindACFanOutletNode;      !- Air Outlet Node Name",
        " ",
        "Fan:OnOff,",
        " WindACx10Fan,             !- Name",
        " AvailSched,               !- Availability Schedule Name",
        " 0.5,                      !- Fan Total Efficiency",
        " 75.0,                     !- Pressure Rise{ Pa }",
        " autosize,                 !- Maximum Flow Rate{ m3 / s }",
        " 0.9,                      !- Motor Efficiency",
        " 1.0,                      !- Motor In Airstream Fraction",
        " WindACx10OAMixerOutletNode, !- Air Inlet Node Name",
        " WindACx10FanOutletNode;   !- Air Outlet Node Name",
        " ",
        "Coil:Cooling:DX:SingleSpeed,",
        " WindACDXCoil,             !- Name",
        " AvailSched,               !- Availability Schedule Name",
        " autosize,                 !- Gross Rated Total Cooling Capacity{ W }",
        " autosize,                 !- Gross Rated Sensible Heat Ratio",
        " 3.0,                      !- Gross Rated Cooling COP{ W / W }",
        " autosize,                 !- Rated Air Flow Rate{ m3 / s }",
        " ,                         !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate{W/(m3/s)}",
        " ,                         !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate{W/(m3/s)}",
        " WindACFanOutletNode,      !- Air Inlet Node Name",
        " Space In Node,            !- Air Outlet Node Name",
        " Biquadratic,              !- Total Cooling Capacity Function of Temperature Curve Name",
        " Cubic,                    !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        " Biquadratic,              !- Energy Input Ratio Function of Temperature Curve Name",
        " Cubic,                    !- Energy Input Ratio Function of Flow Fraction Curve Name",
        " Cubic;                    !- Part Load Fraction Correlation Curve Name",
        " ",
        "Coil:Cooling:DX:SingleSpeed,",
        " WindACx10DXCoil,          !- Name",
        " AvailSched,               !- Availability Schedule Name",
        " autosize,                 !- Gross Rated Total Cooling Capacity{ W }",
        " autosize,                 !- Gross Rated Sensible Heat Ratio",
        " 3.0,                      !- Gross Rated Cooling COP{ W / W }",
        " autosize,                 !- Rated Air Flow Rate{ m3 / s }",
        " ,                         !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate{W/(m3/s)}",
        " ,                         !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate{W/(m3/s)}",
        " WindACx10FanOutletNode,   !- Air Inlet Node Name",
        " Spacex10 In Node,         !- Air Outlet Node Name",
        " Biquadratic,              !- Total Cooling Capacity Function of Temperature Curve Name",
        " Cubic,                    !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        " Biquadratic,              !- Energy Input Ratio Function of Temperature Curve Name",
        " Cubic,                    !- Energy Input Ratio Function of Flow Fraction Curve Name",
        " Cubic;                    !- Part Load Fraction Correlation Curve Name",
        " ",
        "People,",
        " Space People,             !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " OnSched,                  !- Number of People Schedule Name",
        " people,                   !- Number of People Calculation Method",
        " 11,                       !- Number of People",
        " ,                         !- People per Zone Floor Area{ person / m2 }",
        " ,                         !- Zone Floor Area per Person{ m2 / person }",
        " 0.3,                      !- Fraction Radiant",
        " AutoCalculate,            !- Sensible Heat Fraction",
        " OnSched;                  !- Activity Level Schedule Name",
        " ",
        "People,",
        " Spacex10 People,          !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " OnSched,                  !- Number of People Schedule Name",
        " people,                   !- Number of People Calculation Method",
        " 11,                       !- Number of People",
        " ,                         !- People per Zone Floor Area{ person / m2 }",
        " ,                         !- Zone Floor Area per Person{ m2 / person }",
        " 0.3,                      !- Fraction Radiant",
        " AutoCalculate,            !- Sensible Heat Fraction",
        " OnSched;                  !- Activity Level Schedule Name",
        " ",
        "Lights,",
        " Space Lights,             !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Lighting Level{ W }",
        " 10.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Return Air Fraction",
        " 0.59,                     !- Fraction Radiant",
        " 0.2,                      !- Fraction Visible",
        " 0,                        !- Fraction Replaceable",
        " GeneralLights;            !- End - Use Subcategory",
        " ",
        "Lights,",
        " Space Lights x10,         !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Lighting Level{ W }",
        " 10.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Return Air Fraction",
        " 0.59,                     !- Fraction Radiant",
        " 0.2,                      !- Fraction Visible",
        " 0,                        !- Fraction Replaceable",
        " GeneralLights;            !- End - Use Subcategory",
        " ",
        "ElectricEquipment,",
        " Space ElecEq,             !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Design Level{ W }",
        " 20.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Fraction Latent",
        " 0.3,                      !- Fraction Radiant",
        " 0.1;                      !- Fraction Lost",
        " ",
        "ElectricEquipment,",
        " Space ElecEq x10,         !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Design Level{ W }",
        " 20.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Fraction Latent",
        " 0.3,                      !- Fraction Radiant",
        " 0.1;                      !- Fraction Lost",
        " ",
        "Schedule:Compact,",
        " OnSched,                  !- Name",
        " Fraction,                 !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0;        !- Field 26",
        " ",
        "ScheduleTypeLimits,",
        " Fraction, !- Name",
        " 0.0, !- Lower Limit Value",
        " 1.0, !- Upper Limit Value",
        " CONTINUOUS;              !- Numeric Type",
        " ",
        "Construction,",
        " INT-WALL-1,               !- Name",
        " GP02,                     !- Outside Layer",
        " AL21,                     !- Layer 2",
        " GP02;                     !- Layer 3",
        " ",
        "Material,",
        " GP02,                     !- Name",
        " MediumSmooth,             !- Roughness",
        " 1.5900001E-02,            !- Thickness{ m }",
        " 0.1600000,                !- Conductivity{ W / m - K }",
        " 801.0000,                 !- Density{ kg / m3 }",
        " 837.0000,                 !- Specific Heat{ J / kg - K }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.7500000,                !- Solar Absorptance",
        " 0.7500000;                !- Visible Absorptance",
        " ",
        "Material:AirGap,",
        " AL21,                     !- Name",
        " 0.1570000;                !- Thermal Resistance{ m2 - K / W }",
        " ",
        "Construction,",
        "FLOOR-SLAB-1,              !- Name",
        "CC03,                      !- Outside Layer",
        "CP01;                      !- Layer 2",
        " ",
        "Material,",
        " CC03,                     !- Name",
        " MediumRough,              !- Roughness",
        " 0.1016000,                !- Thickness{ m }",
        " 1.310000,                 !- Conductivity{ W / m - K }",
        " 2243.000,                 !- Density{ kg / m3 }",
        " 837.0000,                 !- Specific Heat{ J / kg - K }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.6500000,                !- Solar Absorptance",
        " 0.6500000;                !- Visible Absorptance",
        " ",
        "Material:NoMass,",
        " CP01,                     !- Name",
        " Rough,                    !- Roughness",
        " 0.3670000,                !- Thermal Resistance{ m2 - K / W }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.7500000,                !- Solar Absorptance",
        " 0.7500000;                !- Visible Absorptance",
        " ",
        "Construction,",
        " CLNG-1,                   !- Name",
        " MAT-CLNG-1;               !- Outside Layer",
        " ",
        "Material:NoMass,",
        " MAT-CLNG-1,               !- Name",
        " Rough,                    !- Roughness",
        " 0.652259290,              !- Thermal Resistance{ m2 - K / W }",
        " 0.65,                     !- Thermal Absorptance",
        " 0.65,                     !- Solar Absorptance",
        " 0.65;                     !- Visible Absorptance",
        " ",
        "BuildingSurface:Detailed,",
        " FRONT-1,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " SunExposed,               !- Sun Exposure",
        " WindExposed,              !- Wind Exposure",
        " 0.50000,                  !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " C1-1,                     !- Name",
        " CEILING,                  !- Surface Type",
        " CLNG-1,                   !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " F1-1,                     !- Name",
        " FLOOR,                    !- Surface Type",
        " FLOOR-SLAB-1,             !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Ground,                   !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 0.0;            !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " SB12,                     !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " SB14,                     !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 0.0, 0.0, 2.4;            !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " SB15,                     !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 2.4;            !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "ZoneControl:Thermostat,",
        " Space Thermostat,         !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        " ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        " Space DualSPSched;        !- Control 1 Name",
        " ",
        "ZoneControl:Thermostat,",
        " Spacex10 Thermostat,      !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        " ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        " Space DualSPSched;        !- Control 1 Name",
        " ",
        "Schedule:Compact,",
        " Dual Zone Control Type Sched,  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00,4;           !- Field 3",
        " ",
        "ThermostatSetpoint:DualSetpoint,",
        " Space DualSPSched,        !- Name",
        " HTGSETP_SCH,              !- Heating Setpoint Temperature Schedule Name",
        " CLGSETP_SCH;              !- Cooling Setpoint Temperature Schedule Name",
        " ",
        "Schedule:Compact,",
        " CLGSETP_SCH,              !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 19",
        " Until: 24:00,24.0;        !- Field 20",
        " ",
        "Schedule:Compact,",
        " HTGSETP_SCH,              !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 22",
        " Until: 24:00, 20.0;       !- Field 23",
        " ",
        "BuildingSurface:Detailed,",
        " FRONT-1x10,               !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " SunExposed,               !- Sun Exposure",
        " WindExposed,              !- Wind Exposure",
        " 0.50000,                  !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " C1-1x10,                  !- Name",
        " CEILING,                  !- Surface Type",
        " CLNG-1,                   !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " F1-1x10,                  !- Name",
        " FLOOR,                    !- Surface Type",
        " FLOOR-SLAB-1,             !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Ground,                   !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 0.0;            !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " SB12x10,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " SB14x10,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 0.0, 0.0, 2.4;            !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        " SB15x10,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 2.4;            !- X, Y, Z == > Vertex 4 {m}",
        " ",
        "OutdoorAir:NodeList,",
        "  OutsideAirInletNodes;    !- Node or NodeList Name 1",
        " ",
        "NodeList,",
        "  OutsideAirInletNodes,    !- Name",
        "  WindACOAInNode,          !- Node 1 Name",
        "  WindACx10OAInNode;       !- Node 1 Name",
        " ",
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",
        " ",
        "Schedule:Compact,",
        "  AvailSched,              !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 3",
        "  For: AllDays,            !- Field 4",
        "  Until: 24:00,1.0;        !- Field 5",
        " ",
        "Schedule:Compact,",
        "  FanOpModeSchedule,       !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,1.0;        !- Field 7",
        " ",
        "Curve:Biquadratic,",
        "  Biquadratic,             !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  5,                       !- Minimum Value of x",
        "  40,                      !- Maximum Value of x",
        "  -5,                      !- Minimum Value of y",
        "  30,                      !- Maximum Value of y",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",
        " ",
        "Curve:Cubic,",
        "  Cubic,                   !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0,                       !- Coefficient4 x**3",
        "  11,                      !- Minimum Value of x",
        "  30,                      !- Maximum Value of x",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature;             !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    ManageSimulation(*state); // run the design day over the warmup period (24 hrs, 25 days)

    EXPECT_EQ(10.0,
              (state->dataHeatBal->Zone(2).Volume * state->dataHeatBal->Zone(2).Multiplier * state->dataHeatBal->Zone(2).ListMultiplier) /
                  (state->dataHeatBal->Zone(1).Volume * state->dataHeatBal->Zone(1).Multiplier * state->dataHeatBal->Zone(1).ListMultiplier));
    // leaving a little wiggle room on these
    EXPECT_NEAR(10.0, (state->dataDXCoils->DXCoil(2).RatedTotCap(1) / state->dataDXCoils->DXCoil(1).RatedTotCap(1)), 0.00001);
    EXPECT_NEAR(10.0, (state->dataDXCoils->DXCoil(2).RatedAirVolFlowRate(1) / state->dataDXCoils->DXCoil(1).RatedAirVolFlowRate(1)), 0.00001);
    EXPECT_NEAR(10.0,
                (state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).TotalOutputRequired /
                 state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).TotalOutputRequired),
                0.00001);

    state->dataGlobal->DoWeathSim = true;                                 // flag to trick tabular reports to scan meters
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // fake a weather run since a weather file can't be used (could it?)
    UpdateTabularReports(*state, OutputProcessor::TimeStepType::System);

    // zone equipment should report single zone magnitude, multipliers do not apply, should be > 0 or what's the point
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleRadGain, state->dataHeatBal->ZoneRpt(2).PeopleRadGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleConGain, state->dataHeatBal->ZoneRpt(2).PeopleConGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleSenGain, state->dataHeatBal->ZoneRpt(2).PeopleSenGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleNumOcc, state->dataHeatBal->ZoneRpt(2).PeopleNumOcc);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleLatGain, state->dataHeatBal->ZoneRpt(2).PeopleLatGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleTotGain, state->dataHeatBal->ZoneRpt(2).PeopleTotGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleRadGainRate, state->dataHeatBal->ZoneRpt(2).PeopleRadGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleConGainRate, state->dataHeatBal->ZoneRpt(2).PeopleConGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleSenGainRate, state->dataHeatBal->ZoneRpt(2).PeopleSenGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleLatGainRate, state->dataHeatBal->ZoneRpt(2).PeopleLatGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).PeopleTotGainRate, state->dataHeatBal->ZoneRpt(2).PeopleTotGainRate);

    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsPower, state->dataHeatBal->ZoneRpt(2).LtsPower);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsElecConsump, state->dataHeatBal->ZoneRpt(2).LtsElecConsump);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsRadGain, state->dataHeatBal->ZoneRpt(2).LtsRadGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsVisGain, state->dataHeatBal->ZoneRpt(2).LtsVisGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsConGain, state->dataHeatBal->ZoneRpt(2).LtsConGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsRetAirGain, state->dataHeatBal->ZoneRpt(2).LtsRetAirGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsTotGain, state->dataHeatBal->ZoneRpt(2).LtsTotGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsRadGainRate, state->dataHeatBal->ZoneRpt(2).LtsRadGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsVisGainRate, state->dataHeatBal->ZoneRpt(2).LtsVisGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsConGainRate, state->dataHeatBal->ZoneRpt(2).LtsConGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsRetAirGainRate, state->dataHeatBal->ZoneRpt(2).LtsRetAirGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).LtsTotGainRate, state->dataHeatBal->ZoneRpt(2).LtsTotGainRate);

    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecPower, state->dataHeatBal->ZoneRpt(2).ElecPower);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecConsump, state->dataHeatBal->ZoneRpt(2).ElecConsump);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecRadGain, state->dataHeatBal->ZoneRpt(2).ElecRadGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecConGain, state->dataHeatBal->ZoneRpt(2).ElecConGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecLatGain, state->dataHeatBal->ZoneRpt(2).ElecLatGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecLost, state->dataHeatBal->ZoneRpt(2).ElecLost);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecTotGain, state->dataHeatBal->ZoneRpt(2).ElecTotGain);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecRadGainRate, state->dataHeatBal->ZoneRpt(2).ElecRadGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecConGainRate, state->dataHeatBal->ZoneRpt(2).ElecConGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecLatGainRate, state->dataHeatBal->ZoneRpt(2).ElecLatGainRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecLostRate, state->dataHeatBal->ZoneRpt(2).ElecLostRate);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(1).ElecTotGainRate, state->dataHeatBal->ZoneRpt(2).ElecTotGainRate);

    // expect occupancy time data to be equal
    EXPECT_EQ(state->dataHeatBal->ZonePreDefRep(1).NumOccAccumTime, state->dataHeatBal->ZonePreDefRep(2).NumOccAccumTime);
    EXPECT_EQ(state->dataHeatBal->ZonePreDefRep(1).TotTimeOcc, state->dataHeatBal->ZonePreDefRep(2).TotTimeOcc);

    // occupancy is reported on a zone basis without multipliers (until this changes, expect results to be equal)
    EXPECT_EQ(state->dataHeatBal->ZonePreDefRep(1).NumOccAccum, state->dataHeatBal->ZonePreDefRep(2).NumOccAccum);

    // expect energy to report according to multipliers
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).MechVentVolTotalOcc / state->dataHeatBal->ZonePreDefRep(1).MechVentVolTotalOcc), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).MechVentVolMin / state->dataHeatBal->ZonePreDefRep(1).MechVentVolMin), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).SHGSAnZoneEqCl / state->dataHeatBal->ZonePreDefRep(1).SHGSAnZoneEqCl), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).SHGSAnPeoplAdd / state->dataHeatBal->ZonePreDefRep(1).SHGSAnPeoplAdd), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).SHGSAnLiteAdd / state->dataHeatBal->ZonePreDefRep(1).SHGSAnLiteAdd), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).SHGSAnEquipAdd / state->dataHeatBal->ZonePreDefRep(1).SHGSAnEquipAdd), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).SHGSAnOtherRem / state->dataHeatBal->ZonePreDefRep(1).SHGSAnOtherRem), 0.00001);
    EXPECT_NEAR(10.0, (state->dataHeatBal->ZonePreDefRep(2).clPeak / state->dataHeatBal->ZonePreDefRep(1).clPeak), 0.00001);
}

TEST_F(EnergyPlusFixture, AirloopHVAC_ZoneSumTest)
{
    // AUTHOR: R. Raustad, FSEC
    // DATE WRITTEN: Sep 2015

    std::string const idf_objects = delimited_string({

        " Output:Diagnostics, DisplayExtraWarnings;",
        " Timestep, 4;",
        " BUILDING, AirloopHVAC_ZoneSumTest, 0.0, Suburbs, .04, .4, FullExterior, 25, 6;",
        " SimulationControl, YES, YES, NO, YES, NO;",

        "  Site:Location,",
        "    Miami Intl Ap FL USA TMY3 WMO=722020E,    !- Name",
        "    25.82,                 !- Latitude {deg}",
        "    -80.30,                !- Longitude {deg}",
        "    -5.00,                 !- Time Zone {hr}",
        "    11;                    !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        " Miami Intl Ap Ann Clg .4% Condns DB/MCWB, !- Name",
        " 7,                        !- Month",
        " 21,                       !- Day of Month",
        " SummerDesignDay,          !- Day Type",
        " 31.7,                     !- Maximum Dry - Bulb Temperature{ C }",
        " 10.0,                      !- Daily Dry - Bulb Temperature Range{ deltaC }",
        " ,                         !- Dry - Bulb Temperature Range Modifier Type",
        " ,                         !- Dry - Bulb Temperature Range Modifier Day Schedule Name",
        " Wetbulb,                  !- Humidity Condition Type",
        " 22.7,                     !- Wetbulb or DewPoint at Maximum Dry - Bulb{ C }",
        " ,                         !- Humidity Condition Day Schedule Name",
        " ,                         !- Humidity Ratio at Maximum Dry - Bulb{ kgWater / kgDryAir }",
        " ,                         !- Enthalpy at Maximum Dry - Bulb{ J / kg }",
        " ,                         !- Daily Wet - Bulb Temperature Range{ deltaC }",
        " 101217.,                  !- Barometric Pressure{ Pa }",
        " 3.8,                      !- Wind Speed{ m / s }",
        " 340,                      !- Wind Direction{ deg }",
        " No,                       !- Rain Indicator",
        " No,                       !- Snow Indicator",
        " No,                       !- Daylight Saving Time Indicator",
        " ASHRAEClearSky,           !- Solar Model Indicator",
        " ,                         !- Beam Solar Day Schedule Name",
        " ,                         !- Diffuse Solar Day Schedule Name",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Beam Irradiance( taub ) { dimensionless }",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance( taud ) { dimensionless }",
        " 1.00;                     !- Sky Clearness",

        "SizingPeriod:DesignDay,",
        " Miami Intl Ap Ann Htg 99.6% Condns DB, !- Name",
        " 1,                        !- Month",
        " 21,                       !- Day of Month",
        " WinterDesignDay,          !- Day Type",
        " 8.7,                      !- Maximum Dry - Bulb Temperature{ C }",
        " 0.0,                      !- Daily Dry - Bulb Temperature Range{ deltaC }",
        " ,                         !- Dry - Bulb Temperature Range Modifier Type",
        " ,                         !- Dry - Bulb Temperature Range Modifier Day Schedule Name",
        " Wetbulb,                  !- Humidity Condition Type",
        " 8.7,                      !- Wetbulb or DewPoint at Maximum Dry - Bulb{ C }",
        " ,                         !- Humidity Condition Day Schedule Name",
        " ,                         !- Humidity Ratio at Maximum Dry - Bulb{ kgWater / kgDryAir }",
        " ,                         !- Enthalpy at Maximum Dry - Bulb{ J / kg }",
        " ,                         !- Daily Wet - Bulb Temperature Range{ deltaC }",
        " 101217.,                  !- Barometric Pressure{ Pa }",
        " 3.8,                      !- Wind Speed{ m / s }",
        " 340,                      !- Wind Direction{ deg }",
        " No,                       !- Rain Indicator",
        " No,                       !- Snow Indicator",
        " No,                       !- Daylight Saving Time Indicator",
        " ASHRAEClearSky,           !- Solar Model Indicator",
        " ,                         !- Beam Solar Day Schedule Name",
        " ,                         !- Diffuse Solar Day Schedule Name",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Beam Irradiance( taub ) { dimensionless }",
        " ,                         !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance( taud ) { dimensionless }",
        " 0.00;                     !- Sky Clearness",

        "OutputControl:Table:Style,",
        "  HTML;                    !- Column Separator",

        "Output:Table:SummaryReports,",
        "  AllSummaryAndSizingPeriod; !- Report 1 Name",

        "Zone,",
        "  Space,                   !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  2.4,                     !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",

        "ZoneGroup,",
        " Zone Group,               !- Name",
        " Zone List,                !- Zone List Name",
        " 10;                       !- Zone List Multiplier",

        "ZoneList,",
        " Zone List,                !- Name",
        " Spacex10;                 !- Zone 1 Name",

        "Zone,",
        "  Spacex10,                !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  2.4,                     !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",

        "Sizing:Zone,",
        " Space,                    !- Zone or ZoneList Name",
        " SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
        " 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
        " ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
        " SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
        " 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
        " ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
        " 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " Space DSOA Design OA Spec,  !- Design Specification Outdoor Air Object Name",
        " 0.0,                      !- Zone Heating Sizing Factor",
        " 0.0,                      !- Zone Cooling Sizing Factor",
        " DesignDay,                !- Cooling Design Air Flow Method",
        " 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Cooling Minimum Air Flow{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow Fraction",
        " DesignDay,                !- Heating Design Air Flow Method",
        " 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
        " ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Heating Maximum Air Flow{ m3 / s }",
        " ;                         !- Heating Maximum Air Flow Fraction",

        "Sizing:Zone,",
        " Spacex10,                 !- Zone or ZoneList Name",
        " SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
        " 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
        " ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
        " SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
        " 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
        " ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
        " 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " Spacex10 DSOA Design OA Spec, !- Design Specification Outdoor Air Object Name",
        " 0.0,                      !- Zone Heating Sizing Factor",
        " 0.0,                      !- Zone Cooling Sizing Factor",
        " DesignDay,                !- Cooling Design Air Flow Method",
        " 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Cooling Minimum Air Flow{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow Fraction",
        " DesignDay,                !- Heating Design Air Flow Method",
        " 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
        " ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Heating Maximum Air Flow{ m3 / s }",
        " ;                         !- Heating Maximum Air Flow Fraction",
        " ",

        "People,",
        " Space People,             !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " OnSched,                  !- Number of People Schedule Name",
        " people,                   !- Number of People Calculation Method",
        " 11,                       !- Number of People",
        " ,                         !- People per Zone Floor Area{ person / m2 }",
        " ,                         !- Zone Floor Area per Person{ m2 / person }",
        " 0.3,                      !- Fraction Radiant",
        " AutoCalculate,            !- Sensible Heat Fraction",
        " ActivityLevelSched;       !- Activity Level Schedule Name",

        "People,",
        " Spacex10 People,          !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " OnSched,                  !- Number of People Schedule Name",
        " people,                   !- Number of People Calculation Method",
        " 11,                       !- Number of People",
        " ,                         !- People per Zone Floor Area{ person / m2 }",
        " ,                         !- Zone Floor Area per Person{ m2 / person }",
        " 0.3,                      !- Fraction Radiant",
        " AutoCalculate,            !- Sensible Heat Fraction",
        " ActivityLevelSched;       !- Activity Level Schedule Name",

        "Lights,",
        " Space Lights,             !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Lighting Level{ W }",
        " 10.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Return Air Fraction",
        " 0.59,                     !- Fraction Radiant",
        " 0.2,                      !- Fraction Visible",
        " 0,                        !- Fraction Replaceable",
        " GeneralLights;            !- End - Use Subcategory",

        "Lights,",
        " Space Lights x10,         !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Lighting Level{ W }",
        " 10.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Return Air Fraction",
        " 0.59,                     !- Fraction Radiant",
        " 0.2,                      !- Fraction Visible",
        " 0,                        !- Fraction Replaceable",
        " GeneralLights;            !- End - Use Subcategory",

        "ElectricEquipment,",
        " Space ElecEq,             !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Design Level{ W }",
        " 20.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Fraction Latent",
        " 0.3,                      !- Fraction Radiant",
        " 0.1;                      !- Fraction Lost",

        "ElectricEquipment,",
        " Space ElecEq x10,         !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " OnSched,                  !- Schedule Name",
        " Watts/Area,               !- Design Level Calculation Method",
        " ,                         !- Design Level{ W }",
        " 20.0,                     !- Watts per Zone Floor Area{ W / m2 }",
        " ,                         !- Watts per Person{ W / person }",
        " 0.1,                      !- Fraction Latent",
        " 0.3,                      !- Fraction Radiant",
        " 0.1;                      !- Fraction Lost",

        "Schedule:Compact,",
        " OnSched,                  !- Name",
        " Fraction,                 !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0;        !- Field 26",

        "ScheduleTypeLimits,",
        " Fraction,                 !- Name",
        " 0.0,                      !- Lower Limit Value",
        " 1.0,                      !- Upper Limit Value",
        " CONTINUOUS;               !- Numeric Type",

        "Construction,",
        " INT-WALL-1,               !- Name",
        " GP02,                     !- Outside Layer",
        " AL21,                     !- Layer 2",
        " GP02;                     !- Layer 3",

        "Material,",
        " GP02,                     !- Name",
        " MediumSmooth,             !- Roughness",
        " 1.5900001E-02,            !- Thickness{ m }",
        " 0.1600000,                !- Conductivity{ W / m - K }",
        " 801.0000,                 !- Density{ kg / m3 }",
        " 837.0000,                 !- Specific Heat{ J / kg - K }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.7500000,                !- Solar Absorptance",
        " 0.7500000;                !- Visible Absorptance",

        "Material:AirGap,",
        " AL21,                     !- Name",
        " 0.1570000;                !- Thermal Resistance{ m2 - K / W }",

        "Construction,",
        "FLOOR-SLAB-1,              !- Name",
        "CC03,                      !- Outside Layer",
        "CP01;                      !- Layer 2",

        "Material,",
        " CC03,                     !- Name",
        " MediumRough,              !- Roughness",
        " 0.1016000,                !- Thickness{ m }",
        " 1.310000,                 !- Conductivity{ W / m - K }",
        " 2243.000,                 !- Density{ kg / m3 }",
        " 837.0000,                 !- Specific Heat{ J / kg - K }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.6500000,                !- Solar Absorptance",
        " 0.6500000;                !- Visible Absorptance",

        "Material:NoMass,",
        " CP01,                     !- Name",
        " Rough,                    !- Roughness",
        " 0.3670000,                !- Thermal Resistance{ m2 - K / W }",
        " 0.9000000,                !- Thermal Absorptance",
        " 0.7500000,                !- Solar Absorptance",
        " 0.7500000;                !- Visible Absorptance",

        "Construction,",
        " CLNG-1,                   !- Name",
        " MAT-CLNG-1;               !- Outside Layer",

        "Material:NoMass,",
        " MAT-CLNG-1,               !- Name",
        " Rough,                    !- Roughness",
        " 0.652259290,              !- Thermal Resistance{ m2 - K / W }",
        " 0.65,                     !- Thermal Absorptance",
        " 0.65,                     !- Solar Absorptance",
        " 0.65;                     !- Visible Absorptance",

        "BuildingSurface:Detailed,",
        " FRONT-1,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " SunExposed,               !- Sun Exposure",
        " WindExposed,              !- Wind Exposure",
        " 0.50000,                  !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " C1-1,                     !- Name",
        " CEILING,                  !- Surface Type",
        " CLNG-1,                   !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " F1-1,                     !- Name",
        " FLOOR,                    !- Surface Type",
        " FLOOR-SLAB-1,             !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Ground,                   !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 0.0;            !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " SB12,                     !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " SB14,                     !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 0.0, 0.0, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " SB15,                     !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Space,                    !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

        "ZoneControl:Thermostat,",
        " Space Thermostat,         !- Name",
        " Space,                    !- Zone or ZoneList Name",
        " Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        " ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        " Space DualSPSched;        !- Control 1 Name",

        "ZoneControl:Thermostat,",
        " Spacex10 Thermostat,      !- Name",
        " Spacex10,                 !- Zone or ZoneList Name",
        " Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        " ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        " Space DualSPSched;        !- Control 1 Name",

        "Schedule:Compact,",
        " Dual Zone Control Type Sched,  !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00,4;           !- Field 3",

        "ThermostatSetpoint:DualSetpoint,",
        " Space DualSPSched,        !- Name",
        " HTGSETP_SCH,              !- Heating Setpoint Temperature Schedule Name",
        " CLGSETP_SCH;              !- Cooling Setpoint Temperature Schedule Name",

        "Schedule:Compact,",
        " CLGSETP_SCH,              !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 19",
        " Until: 24:00,22.1;        !- Field 20",

        "Schedule:Compact,",
        " HTGSETP_SCH,              !- Name",
        " Any Number,               !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 22",
        " Until: 24:00, 21.9;       !- Field 23",

        "BuildingSurface:Detailed,",
        " FRONT-1x10,               !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " SunExposed,               !- Sun Exposure",
        " WindExposed,              !- Wind Exposure",
        " 0.50000,                  !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " C1-1x10,                  !- Name",
        " CEILING,                  !- Surface Type",
        " CLNG-1,                   !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Outdoors,                 !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 2 {m}",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " F1-1x10,                  !- Name",
        " FLOOR,                    !- Surface Type",
        " FLOOR-SLAB-1,             !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Ground,                   !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 0.0;            !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " SB12x10,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
        " 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " SB14x10,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
        " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 0.0, 0.0, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        " SB15x10,                  !- Name",
        " WALL,                     !- Surface Type",
        " INT-WALL-1,               !- Construction Name",
        " Spacex10,                 !- Zone Name",
        "    ,                        !- Space Name",
        " Adiabatic,                !- Outside Boundary Condition",
        " ,                         !- Outside Boundary Condition Object",
        " NoSun,                    !- Sun Exposure",
        " NoWind,                   !- Wind Exposure",
        " 0.0,                      !- View Factor to Ground",
        " 4,                        !- Number of Vertices",
        " 26.8, 3.7, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
        " 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
        " 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
        " 3.7, 3.7, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

        "Sizing:System,",
        " DOAS,                     !- AirLoop Name",
        " VentilationRequirement,   !- Type of Load to Size On",
        " autosize,                 !- Design Outdoor Air Flow Rate {m3/s}",
        " 1.0,                      !- Central Heating Maximum System Air Flow Ratio",
        " 2,                        !- Preheat Design Temperature {C}",
        " 0.008,                    !- Preheat Design Humidity Ratio {kgWater/kgDryAir}",
        " 11,                       !- Precool Design Temperature {C}",
        " 0.008,                    !- Precool Design Humidity Ratio {kgWater/kgDryAir}",
        " 16,                       !- Central Cooling Design Supply Air Temperature {C}",
        " 12.2,                     !- Central Heating Design Supply Air Temperature {C}",
        " NonCoincident,            !- Type of Zone Sum to Use",
        " Yes,                      !- 100% Outdoor Air in Cooling",
        " Yes,                      !- 100% Outdoor Air in Heating",
        " 0.0103,                   !- Central Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        " 0.003,                    !- Central Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
        " DesignDay,                !- Cooling Supply Air Flow Rate Method",
        " 0,                        !- Cooling Supply Air Flow Rate {m3/s}",
        " ,                         !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        " ,                         !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        " ,                         !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}",
        " DesignDay,                !- Heating Supply Air Flow Rate Method",
        " 0,                        !- Heating Supply Air Flow Rate {m3/s}",
        " ,                         !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        " ,                         !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        " ,                         !- Heating Fraction of Autosized Cooling Supply Air Flow Rate",
        " ,                         !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}",
        " ZoneSum,                  !- System Outdoor Air Method",
        " 1.0,                      !- Zone Maximum Outdoor Air Fraction {dimensionless}",
        " CoolingDesignCapacity,    !- Cooling Design Capacity Method",
        " autosize,                 !- Cooling Design Capacity {W}",
        " ,                         !- Cooling Design Capacity Per Floor Area {W/m2}",
        " ,                         !- Fraction of Autosized Cooling Design Capacity",
        " HeatingDesignCapacity,    !- Heating Design Capacity Method",
        " autosize,                 !- Heating Design Capacity {W}",
        " ,                         !- Heating Design Capacity Per Floor Area {W/m2}",
        " ,                         !- Fraction of Autosized Heating Design Capacity",
        " OnOff;                    !- Central Cooling Capacity Control Method",

        "AirLoopHVAC,",
        "  DOAS,                    !- Name",
        "  ,                        !- Controller List Name",
        "  DOAS Availability Managers,  !- Availability Manager List Name",
        "  autosize,                !- Design Supply Air Flow Rate {m3/s}",
        "  DOAS Branches,           !- Branch List Name",
        "  ,                        !- Connector List Name",
        "  DOAS Air Loop Inlet,     !- Supply Side Inlet Node Name",
        "  DOAS Return Air Outlet,  !- Demand Side Outlet Node Name",
        "  DOAS Supply Path Inlet,  !- Demand Side Inlet Node Names",
        "  DOAS Supply Fan Outlet;  !- Supply Side Outlet Node Names",

        "BranchList,",
        "  DOAS Branches,           !- Name",
        "  DOAS Main Branch;        !- Branch 1 Name",

        "Branch,",
        "  DOAS Main Branch,        !- Name",
        "  ,                        !- Pressure Drop Curve Name",
        "  AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "  DOAS OA System,          !- Component 1 Name",
        "  DOAS Air Loop Inlet,     !- Component 1 Inlet Node Name",
        "  DOAS Mixed Air Outlet,   !- Component 1 Outlet Node Name",
        "  CoilSystem:Cooling:DX,   !- Component 2 Object Type",
        "  DOAS Cooling Coil,       !- Component 2 Name",
        "  DOAS Mixed Air Outlet,   !- Component 2 Inlet Node Name",
        "  DOAS Cooling Coil Outlet,!- Component 2 Outlet Node Name",
        "  Coil:Heating:Fuel,        !- Component 2 Object Type",
        "  DOAS Heating Coil,       !- Component 2 Name",
        "  DOAS Cooling Coil Outlet,  !- Component 2 Inlet Node Name",
        "  DOAS Heating Coil Outlet,!- Component 2 Outlet Node Name",
        "  Fan:VariableVolume,      !- Component 3 Object Type",
        "  DOAS Supply Fan,         !- Component 3 Name",
        "  DOAS Heating Coil Outlet,!- Component 3 Inlet Node Name",
        "  DOAS Supply Fan Outlet;  !- Component 3 Outlet Node Name",

        "AirLoopHVAC:SupplyPath,",
        "  DOAS Supply Path,        !- Name",
        "  DOAS Supply Path Inlet,  !- Supply Air Path Inlet Node Name",
        "  AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "  DOAS Zone Splitter;      !- Component 1 Name",

        "AirLoopHVAC:ZoneSplitter,",
        "  DOAS Zone Splitter,      !- Name",
        "  DOAS Supply Path Inlet,  !- Inlet Node Name",
        "  Space ATU In Node,  !- Outlet 1 Node Name",
        "  Spacex10 ATU In Node;  !- Outlet 27 Node Name",

        "AirLoopHVAC:ReturnPath,",
        "  DOAS Return Path,        !- Name",
        "  DOAS Return Air Outlet,  !- Return Air Path Outlet Node Name",
        "  AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "  DOAS Zone Mixer;         !- Component 1 Name",

        "AirLoopHVAC:ZoneMixer,",
        "  DOAS Zone Mixer,         !- Name",
        "  DOAS Return Air Outlet,  !- Outlet Node Name",
        "  Space Ret Node,          !- Inlet 1 Node Name",
        "  Spacex10 Ret Node;       !- Inlet 27 Node Name",

        "AvailabilityManagerAssignmentList,",
        "  DOAS Availability Managers,  !- Name",
        "  AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "  DOAS Availability;       !- Availability Manager 1 Name",

        "AvailabilityManager:Scheduled,",
        "  DOAS Availability,       !- Name",
        "  AvailSched;              !- Schedule Name",

        "NodeList,",
        "  DOAS Cooling Setpoint Nodes,  !- Name",
        "  DOAS Cooling Coil Outlet, !- Node 1 Name",
        "  DOAS Heating Coil Outlet; !- Node 1 Name",

        "Schedule:Compact,",
        "  Always 22,               !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,22;         !- Field 3",

        "SetpointManager:Scheduled,",
        "  DOAS Cooling Supply Air Temp Manager,  !- Name",
        "  Temperature,             !- Control Variable",
        "  Always 22,               !- Schedule Name",
        "  DOAS Supply Fan Outlet;  !- Setpoint Node or NodeList Name",

        "SetpointManager:MixedAir,",
        "  DOAS Cooling Coil Air Temp Manager,  !- Name",
        "  Temperature,             !- Control Variable",
        "  DOAS Supply Fan Outlet,  !- Reference Setpoint Node Name",
        "  DOAS Cooling Coil Outlet,!- Fan Inlet Node Name",
        "  DOAS Supply Fan Outlet,  !- Fan Outlet Node Name",
        "  DOAS Cooling Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "CoilSystem:Cooling:DX,",
        "  DOAS Cooling Coil,       !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  DOAS Mixed Air Outlet,   !- DX Cooling Coil System Inlet Node Name",
        "  DOAS Cooling Coil Outlet,  !- DX Cooling Coil System Outlet Node Name",
        "  DOAS Cooling Coil Outlet,  !- DX Cooling Coil System Sensor Node Name",
        "  Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "  DOAS DX Cooling Coil;    !- Cooling Coil Name",

        "Coil:Cooling:DX:SingleSpeed,",
        "   DOAS DX Cooling Coil,   !- Name",
        "   AvailSched,            !- Availability Schedule Name",
        "   autosize,              !- Gross Rated Total Cooling Capacity { W }",
        "   autosize,              !- Gross Rated Sensible Heat Ratio",
        "   4.40,                  !- Gross Rated Cooling COP { W / W }",
        "   autosize,              !- Rated Air Flow Rate { m3 / s }",
        "   ,                      !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   ,                      !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   DOAS Mixed Air Outlet, !- Air Inlet Node Name",
        "   DOAS Cooling Coil Outlet,    !- Air Outlet Node Name",
        "   Biquadratic,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "   Cubic,                 !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "   Biquadratic,           !- Energy Input Ratio Function of Temperature Curve Name",
        "   Cubic,                 !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "   Cubic,                 !- Part Load Fraction Correlation Curve Name",
        "   ,                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "   0.0,                   !- Nominal Time for Condensate Removal to Begin",
        "   0.0,                   !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity",
        "   0.0,                   !- Maximum Cycling Rate",
        "   0.0,                   !- Latent Capacity Time Constant",
        "   Cooling Coil Condenser Inlet, !- Condenser Air Inlet Node Name",
        "   EvaporativelyCooled,   !- Condenser Type",
        "   0.0,                   !- Evaporative Condenser Effectiveness",
        "   ,                      !- Evaporative Condenser Air Flow Rate",
        "   autosize,              !- Evaporative Condenser Pump Rated Power Consumption",
        "   0.0,                   !- Crankcase Heater Capacity",
        "   ,                      !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "   10.0;                  !- Maximum Outdoor DryBulb Temperature for Crankcase Heater Operation",

        "Coil:Heating:Fuel,",
        "  DOAS Heating Coil,       !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  NaturalGas,              !- Fuel Type",
        "  0.8,                     !- Gas Burner Efficiency",
        "  autosize,                !- Nominal Capacity {W}",
        "  DOAS Cooling Coil Outlet,  !- Air Inlet Node Name",
        "  DOAS Heating Coil Outlet,  !- Air Outlet Node Name",
        "  DOAS Heating Coil Outlet;  !- Temperature Setpoint Node Name",

        "Fan:VariableVolume,",
        "  DOAS Supply Fan,         !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  0.7,                     !- Fan Total Efficiency",
        "  1000,                    !- Pressure Rise {Pa}",
        "  autosize,                !- Maximum Flow Rate {m3/s}",
        "  Fraction,                !- Fan Power Minimum Flow Rate Input Method",
        "  0.0,                     !- Fan Power Minimum Flow Fraction",
        "  ,                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.9,                     !- Motor Efficiency",
        "  1,                       !- Motor In Airstream Fraction",
        "  0.0015302446,            !- Fan Power Coefficient 1",
        "  0.0052080574,            !- Fan Power Coefficient 2",
        "  1.1086242,               !- Fan Power Coefficient 3",
        "  -0.11635563,             !- Fan Power Coefficient 4",
        "  0,                       !- Fan Power Coefficient 5",
        "  DOAS Heating Coil Outlet,!- Air Inlet Node Name",
        "  DOAS Supply Fan Outlet;  !- Air Outlet Node Name",

        "OutdoorAir:NodeList,",
        "  DOAS Outdoor Air Inlet,  !- Node or NodeList Name 1",
        "  Cooling Coil Condenser Inlet;  !- Node or NodeList Name 2",

        "AirLoopHVAC:OutdoorAirSystem,",
        "  DOAS OA System,           !- Name",
        "  DOAS OA System Controllers,  !- Controller List Name",
        "  DOAS OA System Equipment; !- Outdoor Air Equipment List Name",

        "AirLoopHVAC:ControllerList,",
        "  DOAS OA System Controllers,  !- Name",
        "  Controller:OutdoorAir,   !- Controller 1 Object Type",
        "  DOAS OA Controller;      !- Controller 1 Name",

        "AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "  DOAS OA System Equipment,!- Name",
        "  OutdoorAir:Mixer,        !- Component 1 Object Type",
        "  DOAS OA Mixing Box;      !- Component 1 Name",

        "OutdoorAir:Mixer,",
        "  DOAS OA Mixing Box,      !- Name",
        "  DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
        "  DOAS Outdoor Air Inlet,  !- Outdoor Air Stream Node Name",
        "  DOAS Relief Air Outlet,  !- Relief Air Stream Node Name",
        "  DOAS Air Loop Inlet;     !- Return Air Stream Node Name",

        "Controller:OutdoorAir,",
        "  DOAS OA Controller,      !- Name",
        "  DOAS Relief Air Outlet,  !- Relief Air Outlet Node Name",
        "  DOAS Air Loop Inlet,     !- Return Air Node Name",
        "  DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
        "  DOAS Outdoor Air Inlet,  !- Actuator Node Name",
        "  0,                       !- Minimum Outdoor Air Flow Rate {m3/s}",
        "  autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "  NoEconomizer,            !- Economizer Control Type",
        "  MinimumFlowWithBypass,   !- Economizer Control Action Type",
        "  ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "  ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
        "  ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "  ,                        !- Electronic Enthalpy Limit Curve Name",
        "  12.2,                    !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "  NoLockout,               !- Lockout Type",
        "  FixedMinimum,            !- Minimum Limit Type",
        "  ,                        !- Minimum Outdoor Air Schedule Name",
        "  ,                        !- Minimum Fraction of Outdoor Air Schedule Name",
        "  ,                        !- Maximum Fraction of Outdoor Air Schedule Name",
        "  DCVObject;               !- Mechanical Ventilation Controller Name",

        "Controller:MechanicalVentilation,",
        "  DCVObject,               !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  Yes,                     !- Demand Controlled Ventilation",
        "  Standard62.1VentilationRateProcedure,!- System Outdoor Air Method",
        "  ,                        !- Zone Maximum Outdoor Air Fraction {dimensionless}",
        "  Space,  !- Zone 1 Name",
        "  Space DSOA Design OA Spec,  !- Design Specification Outdoor Air Object Name 1",
        "  Space DSOA Design ADE Spec, !- Design Specification Zone Air Distribution Object Name 1",
        "  Spacex10,      !- Zone 2 Name",
        "  Spacex10 DSOA Design OA Spec,  !- Design Specification Outdoor Air Object Name 2",
        "  Space DSOA Design ADE Spec;  !- Design Specification Zone Air Distribution Object Name 2",

        "DesignSpecification:OutdoorAir,",
        "  Space DSOA Design OA Spec,  !- Name",
        "  sum,                     !- Outdoor Air Method",
        "  0.0,                     !- Outdoor Air Flow per Person {m3/s-person}",
        "  0.0009,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "  0;                       !- Outdoor Air Flow per Zone {m3/s}",

        "DesignSpecification:OutdoorAir,",
        "  Spacex10 DSOA Design OA Spec,  !- Name",
        "  sum,                     !- Outdoor Air Method",
        "  0.008,                   !- Outdoor Air Flow per Person {m3/s-person}",
        "  0.0009,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "  0;                       !- Outdoor Air Flow per Zone {m3/s}",

        "DesignSpecification:ZoneAirDistribution,",
        "  Space DSOA Design ADE Spec,  !- Name",
        "  1,                       !- Zone Air Distribution Effectiveness in Cooling Mode {dimensionless}",
        "  1;                       !- Zone Air Distribution Effectiveness in Heating Mode {dimensionless}",

        "ZoneHVAC:EquipmentConnections,",
        " Space,                    !- Zone Name",
        " Space Eq,                 !- Zone Conditioning Equipment List Name",
        " Space In Node,            !- Zone Air Inlet Node or NodeList Name",
        " ,                         !- Zone Air Exhaust Node or NodeList Name",
        " Space Node,               !- Zone Air Node Name",
        " Space Ret Node;           !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        " Space Eq,                 !- Name",
        " SequentialLoad,           !- Load Distribution Scheme",
        " ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        " Space ATU,                   !- Zone Equipment 1 Name",
        " 1,                        !- Zone Equipment 1 Cooling Sequence",
        " 1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  Space ATU,  !- Name",
        "  Space In Node,  !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "  Space Air Terminal;  !- Air Terminal Name",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  Space Air Terminal,      !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  Space In Node,           !- Air Outlet Node Name",
        "  Space ATU In Node,       !- Air Inlet Node Name",
        "  autosize,                !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                !- Zone Minimum Air Flow Input Method",
        "  0.0,                     !- Constant Minimum Air Flow Fraction",
        "  ,                        !- Fixed Minimum Air Flow Rate {m3/s}",
        "  ,                        !- Minimum Air Flow Fraction Schedule Name",
        "  ;                        !- Design Specification Outdoor Air Object Name",

        "ZoneHVAC:EquipmentConnections,",
        " Spacex10,                 !- Zone Name",
        " Spacex10 Eq,              !- Zone Conditioning Equipment List Name",
        " Spacex10 In Node,         !- Zone Air Inlet Node or NodeList Name",
        " ,                         !- Zone Air Exhaust Node or NodeList Name",
        " Spacex10 Node,            !- Zone Air Node Name",
        " Spacex10 Ret Node;        !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        " Spacex10 Eq,              !- Name",
        " SequentialLoad,           !- Load Distribution Scheme",
        " ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        " Spacex10 ATU,                !- Zone Equipment 1 Name",
        " 1,                        !- Zone Equipment 1 Cooling Sequence",
        " 1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

        "ZoneHVAC:AirDistributionUnit,",
        "  Spacex10 ATU,  !- Name",
        "  Spacex10 In Node,  !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "  Spacex10 Air Terminal;  !- Air Terminal Name",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  Spacex10 Air Terminal,   !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  Spacex10 In Node,        !- Air Outlet Node Name",
        "  Spacex10 ATU In Node,    !- Air Inlet Node Name",
        "  autosize,                !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                !- Zone Minimum Air Flow Input Method",
        "  0.0,                     !- Constant Minimum Air Flow Fraction",
        "  ,                        !- Fixed Minimum Air Flow Rate {m3/s}",
        "  ,                        !- Minimum Air Flow Fraction Schedule Name",
        "  ;                        !- Design Specification Outdoor Air Object Name",

        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",

        "Schedule:Compact,",
        "  AvailSched,              !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 3",
        "  For: AllDays,            !- Field 4",
        "  Until: 24:00,1.0;        !- Field 5",

        "Schedule:Compact,",
        "  EquipSched,              !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 3",
        "  For: WinterDesignDay,    !- Field 4",
        "  Until: 24:00,0.0,        !- Field 5",
        "  For: AllOtherDays,       !- Field 4",
        "  Until: 24:00,1.0;        !- Field 5",

        "Schedule:Compact,",
        "  ActivityLevelSched,      !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 3",
        "  For: AllDays,            !- Field 4",
        "  Until: 24:00,120.0;      !- Field 5",

        "Schedule:Compact,",
        "  FanOpModeSchedule,       !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,1.0;        !- Field 7",

        "Curve:Biquadratic,",
        "  Biquadratic,             !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  5,                       !- Minimum Value of x",
        "  40,                      !- Maximum Value of x",
        "  5,                       !- Minimum Value of y",
        "  40,                      !- Maximum Value of y",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "  Cubic,                   !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0,                       !- Coefficient4 x**3",
        "  5,                       !- Minimum Value of x",
        "  40,                      !- Maximum Value of x",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature;             !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    ManageSimulation(*state); // run the design day over the warmup period (24 hrs, 25 days)

    EXPECT_EQ(10.0,
              (state->dataHeatBal->Zone(2).Volume * state->dataHeatBal->Zone(2).Multiplier * state->dataHeatBal->Zone(2).ListMultiplier) /
                  (state->dataHeatBal->Zone(1).Volume * state->dataHeatBal->Zone(1).Multiplier * state->dataHeatBal->Zone(1).ListMultiplier));

    state->dataGlobal->DoWeathSim = true;                                 // flag to trick tabular reports to scan meters
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // fake a weather run since a weather file can't be used (could it?)
    UpdateTabularReports(*state, OutputProcessor::TimeStepType::System);

    EXPECT_NEAR(1.86168, state->dataSize->FinalSysSizing(1).DesOutAirVolFlow, 0.0001);
}

// TEST_F( EnergyPlusFixture, AirloopHVAC_VentilationRateProcedure )
//{
//// AUTHOR: R. Raustad, FSEC
//// DATE WRITTEN: Sep 2015

// std::string const idf_objects = delimited_string( {
//" Output:Diagnostics, DisplayExtraWarnings;",
//" Timestep, 4;",
//" BUILDING, AirloopHVAC_VentilationRateProcedure, 0.0, Suburbs, .04, .4, FullExterior, 25, 6;",
//" SimulationControl, YES, YES, NO, YES, NO;",

//"  Site:Location,",
//"    Miami Intl Ap FL USA TMY3 WMO=722020E,    !- Name",
//"    25.82,                 !- Latitude {deg}",
//"    -80.30,                !- Longitude {deg}",
//"    -5.00,                 !- Time Zone {hr}",
//"    11;                    !- Elevation {m}",

//"SizingPeriod:DesignDay,",
//" Miami Intl Ap Ann Clg .4% Condns DB/MCWB, !- Name",
//" 7,                        !- Month",
//" 21,                       !- Day of Month",
//" SummerDesignDay,          !- Day Type",
//" 31.7,                     !- Maximum Dry - Bulb Temperature{ C }",
//" 10.0,                      !- Daily Dry - Bulb Temperature Range{ deltaC }",
//" ,                         !- Dry - Bulb Temperature Range Modifier Type",
//" ,                         !- Dry - Bulb Temperature Range Modifier Day Schedule Name",
//" Wetbulb,                  !- Humidity Condition Type",
//" 22.7,                     !- Wetbulb or DewPoint at Maximum Dry - Bulb{ C }",
//" ,                         !- Humidity Condition Day Schedule Name",
//" ,                         !- Humidity Ratio at Maximum Dry - Bulb{ kgWater / kgDryAir }",
//" ,                         !- Enthalpy at Maximum Dry - Bulb{ J / kg }",
//" ,                         !- Daily Wet - Bulb Temperature Range{ deltaC }",
//" 101217.,                  !- Barometric Pressure{ Pa }",
//" 3.8,                      !- Wind Speed{ m / s }",
//" 340,                      !- Wind Direction{ deg }",
//" No,                       !- Rain Indicator",
//" No,                       !- Snow Indicator",
//" No,                       !- Daylight Saving Time Indicator",
//" ASHRAEClearSky,           !- Solar Model Indicator",
//" ,                         !- Beam Solar Day Schedule Name",
//" ,                         !- Diffuse Solar Day Schedule Name",
//" ,                         !- ASHRAE Clear Sky Optical Depth for Beam Irradiance( taub ) { dimensionless }",
//" ,                         !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance( taud ) { dimensionless }",
//" 1.00;                     !- Sky Clearness",

//"SizingPeriod:DesignDay,",
//" Miami Intl Ap Ann Htg 99.6% Condns DB, !- Name",
//" 1,                        !- Month",
//" 21,                       !- Day of Month",
//" WinterDesignDay,          !- Day Type",
//" 8.7,                      !- Maximum Dry - Bulb Temperature{ C }",
//" 0.0,                      !- Daily Dry - Bulb Temperature Range{ deltaC }",
//" ,                         !- Dry - Bulb Temperature Range Modifier Type",
//" ,                         !- Dry - Bulb Temperature Range Modifier Day Schedule Name",
//" Wetbulb,                  !- Humidity Condition Type",
//" 8.7,                      !- Wetbulb or DewPoint at Maximum Dry - Bulb{ C }",
//" ,                         !- Humidity Condition Day Schedule Name",
//" ,                         !- Humidity Ratio at Maximum Dry - Bulb{ kgWater / kgDryAir }",
//" ,                         !- Enthalpy at Maximum Dry - Bulb{ J / kg }",
//" ,                         !- Daily Wet - Bulb Temperature Range{ deltaC }",
//" 101217.,                  !- Barometric Pressure{ Pa }",
//" 3.8,                      !- Wind Speed{ m / s }",
//" 340,                      !- Wind Direction{ deg }",
//" No,                       !- Rain Indicator",
//" No,                       !- Snow Indicator",
//" No,                       !- Daylight Saving Time Indicator",
//" ASHRAEClearSky,           !- Solar Model Indicator",
//" ,                         !- Beam Solar Day Schedule Name",
//" ,                         !- Diffuse Solar Day Schedule Name",
//" ,                         !- ASHRAE Clear Sky Optical Depth for Beam Irradiance( taub ) { dimensionless }",
//" ,                         !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance( taud ) { dimensionless }",
//" 0.00;                     !- Sky Clearness",

//"OutputControl:Table:Style,",
//"  HTML;                    !- Column Separator",

//"Output:Table:SummaryReports,",
//"  AllSummaryAndSizingPeriod; !- Report 1 Name",

//"Zone,",
//"  Space,                   !- Name",
//"  0.0000,                  !- Direction of Relative North {deg}",
//"  0.0000,                  !- X Origin {m}",
//"  0.0000,                  !- Y Origin {m}",
//"  0.0000,                  !- Z Origin {m}",
//"  1,                       !- Type",
//"  1,                       !- Multiplier",
//"  2.4,                     !- Ceiling Height {m}",
//"  ,                        !- Volume {m3}",
//"  autocalculate,           !- Floor Area {m2}",
//"  ,                        !- Zone Inside Convection Algorithm",
//"  ,                        !- Zone Outside Convection Algorithm",
//"  Yes;                     !- Part of Total Floor Area",

//"ZoneGroup,",
//" Zone Group,               !- Name",
//" Zone List,                !- Zone List Name",
//" 10;                       !- Zone List Multiplier",

//"ZoneList,",
//" Zone List,                !- Name",
//" Spacex10;                 !- Zone 1 Name",

//"Zone,",
//"  Spacex10,                !- Name",
//"  0.0000,                  !- Direction of Relative North {deg}",
//"  0.0000,                  !- X Origin {m}",
//"  0.0000,                  !- Y Origin {m}",
//"  0.0000,                  !- Z Origin {m}",
//"  1,                       !- Type",
//"  1,                       !- Multiplier",
//"  2.4,                     !- Ceiling Height {m}",
//"  ,                        !- Volume {m3}",
//"  autocalculate,           !- Floor Area {m2}",
//"  ,                        !- Zone Inside Convection Algorithm",
//"  ,                        !- Zone Outside Convection Algorithm",
//"  Yes;                     !- Part of Total Floor Area",

//"Sizing:Zone,",
//" Space,                    !- Zone or ZoneList Name",
//" SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
//" 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
//" ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
//" SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
//" 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
//" ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
//" 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
//" 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
//" Space DSOA Design OA Spec,  !- Design Specification Outdoor Air Object Name",
//" 0.0,                      !- Zone Heating Sizing Factor",
//" 0.0,                      !- Zone Cooling Sizing Factor",
//" DesignDay,                !- Cooling Design Air Flow Method",
//" 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
//" ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
//" ,                         !- Cooling Minimum Air Flow{ m3 / s }",
//" ,                         !- Cooling Minimum Air Flow Fraction",
//" DesignDay,                !- Heating Design Air Flow Method",
//" 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
//" ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
//" ,                         !- Heating Maximum Air Flow{ m3 / s }",
//" ;                         !- Heating Maximum Air Flow Fraction",

//"Sizing:Zone,",
//" Spacex10,                 !- Zone or ZoneList Name",
//" SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
//" 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
//" ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
//" SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
//" 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
//" ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
//" 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
//" 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
//" Spacex10 DSOA Design OA Spec, !- Design Specification Outdoor Air Object Name",
//" 0.0,                      !- Zone Heating Sizing Factor",
//" 0.0,                      !- Zone Cooling Sizing Factor",
//" DesignDay,                !- Cooling Design Air Flow Method",
//" 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
//" ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
//" ,                         !- Cooling Minimum Air Flow{ m3 / s }",
//" ,                         !- Cooling Minimum Air Flow Fraction",
//" DesignDay,                !- Heating Design Air Flow Method",
//" 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
//" ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
//" ,                         !- Heating Maximum Air Flow{ m3 / s }",
//" ;                         !- Heating Maximum Air Flow Fraction",
//" ",

//"People,",
//" Space People,             !- Name",
//" Space,                    !- Zone or ZoneList Name",
//" OnSched,                  !- Number of People Schedule Name",
//" people,                   !- Number of People Calculation Method",
//" 11,                       !- Number of People",
//" ,                         !- People per Zone Floor Area{ person / m2 }",
//" ,                         !- Zone Floor Area per Person{ m2 / person }",
//" 0.3,                      !- Fraction Radiant",
//" AutoCalculate,            !- Sensible Heat Fraction",
//" ActivityLevelSched;       !- Activity Level Schedule Name",

//"People,",
//" Spacex10 People,          !- Name",
//" Spacex10,                 !- Zone or ZoneList Name",
//" OnSched,                  !- Number of People Schedule Name",
//" people,                   !- Number of People Calculation Method",
//" 11,                       !- Number of People",
//" ,                         !- People per Zone Floor Area{ person / m2 }",
//" ,                         !- Zone Floor Area per Person{ m2 / person }",
//" 0.3,                      !- Fraction Radiant",
//" AutoCalculate,            !- Sensible Heat Fraction",
//" ActivityLevelSched;       !- Activity Level Schedule Name",

//"Lights,",
//" Space Lights,             !- Name",
//" Space,                    !- Zone or ZoneList Name",
//" OnSched,                  !- Schedule Name",
//" Watts/Area,               !- Design Level Calculation Method",
//" ,                         !- Lighting Level{ W }",
//" 10.0,                     !- Watts per Zone Floor Area{ W / m2 }",
//" ,                         !- Watts per Person{ W / person }",
//" 0.1,                      !- Return Air Fraction",
//" 0.59,                     !- Fraction Radiant",
//" 0.2,                      !- Fraction Visible",
//" 0,                        !- Fraction Replaceable",
//" GeneralLights;            !- End - Use Subcategory",

//"Lights,",
//" Space Lights x10,         !- Name",
//" Spacex10,                 !- Zone or ZoneList Name",
//" OnSched,                  !- Schedule Name",
//" Watts/Area,               !- Design Level Calculation Method",
//" ,                         !- Lighting Level{ W }",
//" 10.0,                     !- Watts per Zone Floor Area{ W / m2 }",
//" ,                         !- Watts per Person{ W / person }",
//" 0.1,                      !- Return Air Fraction",
//" 0.59,                     !- Fraction Radiant",
//" 0.2,                      !- Fraction Visible",
//" 0,                        !- Fraction Replaceable",
//" GeneralLights;            !- End - Use Subcategory",

//"ElectricEquipment,",
//" Space ElecEq,             !- Name",
//" Space,                    !- Zone or ZoneList Name",
//" OnSched,                  !- Schedule Name",
//" Watts/Area,               !- Design Level Calculation Method",
//" ,                         !- Design Level{ W }",
//" 20.0,                     !- Watts per Zone Floor Area{ W / m2 }",
//" ,                         !- Watts per Person{ W / person }",
//" 0.1,                      !- Fraction Latent",
//" 0.3,                      !- Fraction Radiant",
//" 0.1;                      !- Fraction Lost",

//"ElectricEquipment,",
//" Space ElecEq x10,         !- Name",
//" Spacex10,                 !- Zone or ZoneList Name",
//" OnSched,                  !- Schedule Name",
//" Watts/Area,               !- Design Level Calculation Method",
//" ,                         !- Design Level{ W }",
//" 20.0,                     !- Watts per Zone Floor Area{ W / m2 }",
//" ,                         !- Watts per Person{ W / person }",
//" 0.1,                      !- Fraction Latent",
//" 0.3,                      !- Fraction Radiant",
//" 0.1;                      !- Fraction Lost",

//"Schedule:Compact,",
//" OnSched,                  !- Name",
//" Fraction,                 !- Schedule Type Limits Name",
//" Through: 12/31,           !- Field 1",
//" For: AllDays,             !- Field 2",
//" Until: 24:00, 1.0;        !- Field 26",

//"ScheduleTypeLimits,",
//" Fraction,                 !- Name",
//" 0.0,                      !- Lower Limit Value",
//" 1.0,                      !- Upper Limit Value",
//" CONTINUOUS;               !- Numeric Type",

//"Construction,",
//" INT-WALL-1,               !- Name",
//" GP02,                     !- Outside Layer",
//" AL21,                     !- Layer 2",
//" GP02;                     !- Layer 3",

//"Material,",
//" GP02,                     !- Name",
//" MediumSmooth,             !- Roughness",
//" 1.5900001E-02,            !- Thickness{ m }",
//" 0.1600000,                !- Conductivity{ W / m - K }",
//" 801.0000,                 !- Density{ kg / m3 }",
//" 837.0000,                 !- Specific Heat{ J / kg - K }",
//" 0.9000000,                !- Thermal Absorptance",
//" 0.7500000,                !- Solar Absorptance",
//" 0.7500000;                !- Visible Absorptance",

//"Material:AirGap,",
//" AL21,                     !- Name",
//" 0.1570000;                !- Thermal Resistance{ m2 - K / W }",

//"Construction,",
//"FLOOR-SLAB-1,              !- Name",
//"CC03,                      !- Outside Layer",
//"CP01;                      !- Layer 2",

//"Material,",
//" CC03,                     !- Name",
//" MediumRough,              !- Roughness",
//" 0.1016000,                !- Thickness{ m }",
//" 1.310000,                 !- Conductivity{ W / m - K }",
//" 2243.000,                 !- Density{ kg / m3 }",
//" 837.0000,                 !- Specific Heat{ J / kg - K }",
//" 0.9000000,                !- Thermal Absorptance",
//" 0.6500000,                !- Solar Absorptance",
//" 0.6500000;                !- Visible Absorptance",

//"Material:NoMass,",
//" CP01,                     !- Name",
//" Rough,                    !- Roughness",
//" 0.3670000,                !- Thermal Resistance{ m2 - K / W }",
//" 0.9000000,                !- Thermal Absorptance",
//" 0.7500000,                !- Solar Absorptance",
//" 0.7500000;                !- Visible Absorptance",

//"Construction,",
//" CLNG-1,                   !- Name",
//" MAT-CLNG-1;               !- Outside Layer",

//"Material:NoMass,",
//" MAT-CLNG-1,               !- Name",
//" Rough,                    !- Roughness",
//" 0.652259290,              !- Thermal Resistance{ m2 - K / W }",
//" 0.65,                     !- Thermal Absorptance",
//" 0.65,                     !- Solar Absorptance",
//" 0.65;                     !- Visible Absorptance",

//"BuildingSurface:Detailed,",
//" FRONT-1,                  !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Space,                    !- Zone Name",
//" Outdoors,                 !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" SunExposed,               !- Sun Exposure",
//" WindExposed,              !- Wind Exposure",
//" 0.50000,                  !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
//" 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
//" 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
//" 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" C1-1,                     !- Name",
//" CEILING,                  !- Surface Type",
//" CLNG-1,                   !- Construction Name",
//" Space,                    !- Zone Name",
//" Outdoors,                 !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
//" 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 2 {m}",
//" 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 3 {m}",
//" 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" F1-1,                     !- Name",
//" FLOOR,                    !- Surface Type",
//" FLOOR-SLAB-1,             !- Construction Name",
//" Space,                    !- Zone Name",
//" Ground,                   !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 1 {m}",
//" 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
//" 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
//" 3.7, 3.7, 0.0;            !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" SB12,                     !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Space,                    !- Zone Name",
//" Adiabatic,                !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
//" 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
//" 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
//" 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" SB14,                     !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Space,                    !- Zone Name",
//" Adiabatic,                !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
//" 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
//" 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
//" 0.0, 0.0, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" SB15,                     !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Space,                    !- Zone Name",
//" Adiabatic,                !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 26.8, 3.7, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
//" 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
//" 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
//" 3.7, 3.7, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

//"ZoneControl:Thermostat,",
//" Space Thermostat,         !- Name",
//" Space,                    !- Zone or ZoneList Name",
//" Dual Zone Control Type Sched,  !- Control Type Schedule Name",
//" ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
//" Space DualSPSched;        !- Control 1 Name",

//"ZoneControl:Thermostat,",
//" Spacex10 Thermostat,      !- Name",
//" Spacex10,                 !- Zone or ZoneList Name",
//" Dual Zone Control Type Sched,  !- Control Type Schedule Name",
//" ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
//" Space DualSPSched;        !- Control 1 Name",

//"Schedule:Compact,",
//" Dual Zone Control Type Sched,  !- Name",
//" Any Number,               !- Schedule Type Limits Name",
//" Through: 12/31,           !- Field 1",
//" For: AllDays,             !- Field 2",
//" Until: 24:00,4;           !- Field 3",

//"ThermostatSetpoint:DualSetpoint,",
//" Space DualSPSched,        !- Name",
//" HTGSETP_SCH,              !- Heating Setpoint Temperature Schedule Name",
//" CLGSETP_SCH;              !- Cooling Setpoint Temperature Schedule Name",

//"Schedule:Compact,",
//" CLGSETP_SCH,              !- Name",
//" Any Number,               !- Schedule Type Limits Name",
//" Through: 12/31,           !- Field 1",
//" For: AllDays,             !- Field 19",
//" Until: 24:00,22.1;        !- Field 20",

//"Schedule:Compact,",
//" HTGSETP_SCH,              !- Name",
//" Any Number,               !- Schedule Type Limits Name",
//" Through: 12/31,           !- Field 1",
//" For: AllDays,             !- Field 22",
//" Until: 24:00, 21.9;       !- Field 23",

//"BuildingSurface:Detailed,",
//" FRONT-1x10,               !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Spacex10,                 !- Zone Name",
//" Outdoors,                 !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" SunExposed,               !- Sun Exposure",
//" WindExposed,              !- Wind Exposure",
//" 0.50000,                  !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
//" 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
//" 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
//" 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" C1-1x10,                  !- Name",
//" CEILING,                  !- Surface Type",
//" CLNG-1,                   !- Construction Name",
//" Spacex10,                 !- Zone Name",
//" Outdoors,                 !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
//" 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 2 {m}",
//" 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 3 {m}",
//" 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" F1-1x10,                  !- Name",
//" FLOOR,                    !- Surface Type",
//" FLOOR-SLAB-1,             !- Construction Name",
//" Spacex10,                 !- Zone Name",
//" Ground,                   !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 1 {m}",
//" 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
//" 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
//" 3.7, 3.7, 0.0;            !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" SB12x10,                  !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Spacex10,                 !- Zone Name",
//" Adiabatic,                !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 30.5, 0.0, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
//" 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
//" 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
//" 26.8, 3.7, 2.4;           !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" SB14x10,                  !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Spacex10,                 !- Zone Name",
//" Adiabatic,                !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 3.7, 3.7, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
//" 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
//" 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
//" 0.0, 0.0, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

//"BuildingSurface:Detailed,",
//" SB15x10,                  !- Name",
//" WALL,                     !- Surface Type",
//" INT-WALL-1,               !- Construction Name",
//" Spacex10,                 !- Zone Name",
//" Adiabatic,                !- Outside Boundary Condition",
//" ,                         !- Outside Boundary Condition Object",
//" NoSun,                    !- Sun Exposure",
//" NoWind,                   !- Wind Exposure",
//" 0.0,                      !- View Factor to Ground",
//" 4,                        !- Number of Vertices",
//" 26.8, 3.7, 2.4,           !- X, Y, Z == > Vertex 1 {m}",
//" 26.8, 3.7, 0.0,           !- X, Y, Z == > Vertex 2 {m}",
//" 3.7, 3.7, 0.0,            !- X, Y, Z == > Vertex 3 {m}",
//" 3.7, 3.7, 2.4;            !- X, Y, Z == > Vertex 4 {m}",

//"Sizing:System,",
//" DOAS,                     !- AirLoop Name",
//" VentilationRequirement,   !- Type of Load to Size On",
//" autosize,                 !- Design Outdoor Air Flow Rate {m3/s}",
//" 1.0,                      !- Central Heating Maximum System Air Flow Ratio",
//" 2,                        !- Preheat Design Temperature {C}",
//" 0.008,                    !- Preheat Design Humidity Ratio {kgWater/kgDryAir}",
//" 11,                       !- Precool Design Temperature {C}",
//" 0.008,                    !- Precool Design Humidity Ratio {kgWater/kgDryAir}",
//" 16,                       !- Central Cooling Design Supply Air Temperature {C}",
//" 12.2,                     !- Central Heating Design Supply Air Temperature {C}",
//" NonCoincident,            !- Type of Zone Sum to Use",
//" Yes,                      !- 100% Outdoor Air in Cooling",
//" Yes,                      !- 100% Outdoor Air in Heating",
//" 0.0103,                   !- Central Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
//" 0.003,                    !- Central Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}",
//" DesignDay,                !- Cooling Supply Air Flow Rate Method",
//" 0,                        !- Cooling Supply Air Flow Rate {m3/s}",
//" ,                         !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
//" ,                         !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
//" ,                         !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}",
//" DesignDay,                !- Heating Supply Air Flow Rate Method",
//" 0,                        !- Heating Supply Air Flow Rate {m3/s}",
//" ,                         !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
//" ,                         !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
//" ,                         !- Heating Fraction of Autosized Cooling Supply Air Flow Rate",
//" ,                         !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}",
//" Standard62.1VentilationRateProcedure, !- System Outdoor Air Method",
//" 1.0,                      !- Zone Maximum Outdoor Air Fraction {dimensionless}",
//" CoolingDesignCapacity,    !- Cooling Design Capacity Method",
//" autosize,                 !- Cooling Design Capacity {W}",
//" ,                         !- Cooling Design Capacity Per Floor Area {W/m2}",
//" ,                         !- Fraction of Autosized Cooling Design Capacity",
//" HeatingDesignCapacity,    !- Heating Design Capacity Method",
//" autosize,                 !- Heating Design Capacity {W}",
//" ,                         !- Heating Design Capacity Per Floor Area {W/m2}",
//" ,                         !- Fraction of Autosized Heating Design Capacity",
//" OnOff;                    !- Central Cooling Capacity Control Method",

//"AirLoopHVAC,",
//"  DOAS,                    !- Name",
//"  ,                        !- Controller List Name",
//"  DOAS Availability Managers,  !- Availability Manager List Name",
//"  autosize,                !- Design Supply Air Flow Rate {m3/s}",
//"  DOAS Branches,           !- Branch List Name",
//"  ,                        !- Connector List Name",
//"  DOAS Air Loop Inlet,     !- Supply Side Inlet Node Name",
//"  DOAS Return Air Outlet,  !- Demand Side Outlet Node Name",
//"  DOAS Supply Path Inlet,  !- Demand Side Inlet Node Names",
//"  DOAS Supply Fan Outlet;  !- Supply Side Outlet Node Names",

//"BranchList,",
//"  DOAS Branches,           !- Name",
//"  DOAS Main Branch;        !- Branch 1 Name",

//"Branch,",
//"  DOAS Main Branch,        !- Name",
//"  ,                        !- Pressure Drop Curve Name",
//"  AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
//"  DOAS OA System,          !- Component 1 Name",
//"  DOAS Air Loop Inlet,     !- Component 1 Inlet Node Name",
//"  DOAS Mixed Air Outlet,   !- Component 1 Outlet Node Name",
//"  CoilSystem:Cooling:DX,   !- Component 2 Object Type",
//"  DOAS Cooling Coil,       !- Component 2 Name",
//"  DOAS Mixed Air Outlet,   !- Component 2 Inlet Node Name",
//"  DOAS Cooling Coil Outlet,!- Component 2 Outlet Node Name",
//"  Coil:Heating:Fuel,        !- Component 2 Object Type",
//"  DOAS Heating Coil,       !- Component 2 Name",
//"  DOAS Cooling Coil Outlet,  !- Component 2 Inlet Node Name",
//"  DOAS Heating Coil Outlet,!- Component 2 Outlet Node Name",
//"  Fan:VariableVolume,      !- Component 3 Object Type",
//"  DOAS Supply Fan,         !- Component 3 Name",
//"  DOAS Heating Coil Outlet,!- Component 3 Inlet Node Name",
//"  DOAS Supply Fan Outlet;  !- Component 3 Outlet Node Name",

//"AirLoopHVAC:SupplyPath,",
//"  DOAS Supply Path,        !- Name",
//"  DOAS Supply Path Inlet,  !- Supply Air Path Inlet Node Name",
//"  AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
//"  DOAS Zone Splitter;      !- Component 1 Name",

//"AirLoopHVAC:ZoneSplitter,",
//"  DOAS Zone Splitter,      !- Name",
//"  DOAS Supply Path Inlet,  !- Inlet Node Name",
//"  Space ATU In Node,  !- Outlet 1 Node Name",
//"  Spacex10 ATU In Node;  !- Outlet 27 Node Name",

//"AirLoopHVAC:ReturnPath,",
//"  DOAS Return Path,        !- Name",
//"  DOAS Return Air Outlet,  !- Return Air Path Outlet Node Name",
//"  AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
//"  DOAS Zone Mixer;         !- Component 1 Name",

//"AirLoopHVAC:ZoneMixer,",
//"  DOAS Zone Mixer,         !- Name",
//"  DOAS Return Air Outlet,  !- Outlet Node Name",
//"  Space Ret Node,          !- Inlet 1 Node Name",
//"  Spacex10 Ret Node;       !- Inlet 27 Node Name",

//"AvailabilityManagerAssignmentList,",
//"  DOAS Availability Managers,  !- Name",
//"  AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
//"  DOAS Availability;       !- Availability Manager 1 Name",

//"AvailabilityManager:Scheduled,",
//"  DOAS Availability,       !- Name",
//"  AvailSched;              !- Schedule Name",

//"NodeList,",
//"  DOAS Cooling Setpoint Nodes,  !- Name",
//"  DOAS Cooling Coil Outlet, !- Node 1 Name",
//"  DOAS Heating Coil Outlet; !- Node 1 Name",

//"Schedule:Compact,",
//"  Always 22,               !- Name",
//"  Any Number,              !- Schedule Type Limits Name",
//"  Through: 12/31,          !- Field 1",
//"  For: AllDays,            !- Field 2",
//"  Until: 24:00,22;         !- Field 3",

//"SetpointManager:Scheduled,",
//"  DOAS Cooling Supply Air Temp Manager,  !- Name",
//"  Temperature,             !- Control Variable",
//"  Always 22,               !- Schedule Name",
//"  DOAS Supply Fan Outlet;  !- Setpoint Node or NodeList Name",

//"SetpointManager:MixedAir,",
//"  DOAS Cooling Coil Air Temp Manager,  !- Name",
//"  Temperature,             !- Control Variable",
//"  DOAS Supply Fan Outlet,  !- Reference Setpoint Node Name",
//"  DOAS Cooling Coil Outlet,!- Fan Inlet Node Name",
//"  DOAS Supply Fan Outlet,  !- Fan Outlet Node Name",
//"  DOAS Cooling Setpoint Nodes;  !- Setpoint Node or NodeList Name",

//"CoilSystem:Cooling:DX,",
//"  DOAS Cooling Coil,       !- Name",
//"  AvailSched,              !- Availability Schedule Name",
//"  DOAS Mixed Air Outlet,   !- DX Cooling Coil System Inlet Node Name",
//"  DOAS Cooling Coil Outlet,  !- DX Cooling Coil System Outlet Node Name",
//"  DOAS Cooling Coil Outlet,  !- DX Cooling Coil System Sensor Node Name",
//"  Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
//"  DOAS DX Cooling Coil;    !- Cooling Coil Name",

//"Coil:Cooling:DX:SingleSpeed,",
//" DOAS DX Cooling Coil,   !- Name",
//"     AvailSched,            !- Availability Schedule Name",
//" autosize,              !- Gross Rated Total Cooling Capacity { W }",
//" autosize,              !- Gross Rated Sensible Heat Ratio",
//" 4.40,                  !- Gross Rated Cooling COP { W / W }",
//" autosize,              !- Rated Air Flow Rate { m3 / s }",
//" ,                      !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
//" ,                      !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
//" DOAS Mixed Air Outlet, !- Air Inlet Node Name",
//" DOAS Cooling Coil Outlet,    !- Air Outlet Node Name",
//" Biquadratic,           !- Total Cooling Capacity Function of Temperature Curve Name",
//" Cubic,                 !- Total Cooling Capacity Function of Flow Fraction Curve Name",
//" Biquadratic,           !- Energy Input Ratio Function of Temperature Curve Name",
//" Cubic,                 !- Energy Input Ratio Function of Flow Fraction Curve Name",
//" Cubic,                 !- Part Load Fraction Correlation Curve Name",
//" ,                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
//" 0.0,                   !- Nominal Time for Condensate Removal to Begin",
//" 0.0,                   !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity",
//" 0.0,                   !- Maximum Cycling Rate",
//" 0.0,                   !- Latent Capacity Time Constant",
//" Cooling Coil Condenser Inlet, !- Condenser Air Inlet Node Name",
//" EvaporativelyCooled,   !- Condenser Type",
//" 0.0,                   !- Evaporative Condenser Effectiveness",
//" ,                      !- Evaporative Condenser Air Flow Rate",
//" autosize,              !- Evaporative Condenser Pump Rated Power Consumption",
//" 0.0,                   !- Crankcase Heater Capacity",
//" 10.0;                  !- Maximum Outdoor DryBulb Temperature for Crankcase Heater Operation",

//"Coil:Heating:Fuel,",
//"  DOAS Heating Coil,       !- Name",
//"  AvailSched,              !- Availability Schedule Name",
//"  Gas,                     !- Fuel Type",
//"  0.8,                     !- Gas Burner Efficiency",
//"  autosize,                !- Nominal Capacity {W}",
//"  DOAS Cooling Coil Outlet,  !- Air Inlet Node Name",
//"  DOAS Heating Coil Outlet,  !- Air Outlet Node Name",
//"  DOAS Heating Coil Outlet;  !- Temperature Setpoint Node Name",

//"Fan:VariableVolume,",
//"  DOAS Supply Fan,         !- Name",
//"  AvailSched,              !- Availability Schedule Name",
//"  0.7,                     !- Fan Total Efficiency",
//"  1000,                    !- Pressure Rise {Pa}",
//"  autosize,                !- Maximum Flow Rate {m3/s}",
//"  Fraction,                !- Fan Power Minimum Flow Rate Input Method",
//"  0.0,                     !- Fan Power Minimum Flow Fraction",
//"  ,                        !- Fan Power Minimum Air Flow Rate {m3/s}",
//"  0.9,                     !- Motor Efficiency",
//"  1,                       !- Motor In Airstream Fraction",
//"  0.0015302446,            !- Fan Power Coefficient 1",
//"  0.0052080574,            !- Fan Power Coefficient 2",
//"  1.1086242,               !- Fan Power Coefficient 3",
//"  -0.11635563,             !- Fan Power Coefficient 4",
//"  0,                       !- Fan Power Coefficient 5",
//"  DOAS Heating Coil Outlet,!- Air Inlet Node Name",
//"  DOAS Supply Fan Outlet;  !- Air Outlet Node Name",

//"OutdoorAir:NodeList,",
//"  DOAS Outdoor Air Inlet,  !- Node or NodeList Name 1",
//"  Cooling Coil Condenser Inlet;  !- Node or NodeList Name 2",

//"AirLoopHVAC:OutdoorAirSystem,",
//"  DOAS OA System,          !- Name",
//"  DOAS OA System Controllers,  !- Controller List Name",
//"  DOAS OA System Equipment;!- Outdoor Air Equipment List Name",

//"AirLoopHVAC:ControllerList,",
//"  DOAS OA System Controllers,  !- Name",
//"  Controller:OutdoorAir,   !- Controller 1 Object Type",
//"  DOAS OA Controller;      !- Controller 1 Name",

//"AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
//"  DOAS OA System Equipment,!- Name",
//"  OutdoorAir:Mixer,        !- Component 1 Object Type",
//"  DOAS OA Mixing Box;      !- Component 1 Name",

//"OutdoorAir:Mixer,",
//"  DOAS OA Mixing Box,      !- Name",
//"  DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
//"  DOAS Outdoor Air Inlet,  !- Outdoor Air Stream Node Name",
//"  DOAS Relief Air Outlet,  !- Relief Air Stream Node Name",
//"  DOAS Air Loop Inlet;     !- Return Air Stream Node Name",

//"Controller:OutdoorAir,",
//"  DOAS OA Controller,      !- Name",
//"  DOAS Relief Air Outlet,  !- Relief Air Outlet Node Name",
//"  DOAS Air Loop Inlet,     !- Return Air Node Name",
//"  DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
//"  DOAS Outdoor Air Inlet,  !- Actuator Node Name",
//"  0,                       !- Minimum Outdoor Air Flow Rate {m3/s}",
//"  autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
//"  NoEconomizer,            !- Economizer Control Type",
//"  MinimumFlowWithBypass,   !- Economizer Control Action Type",
//"  ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
//"  ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
//"  ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
//"  ,                        !- Electronic Enthalpy Limit Curve Name",
//"  12.2,                    !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
//"  NoLockout,               !- Lockout Type",
//"  FixedMinimum,            !- Minimum Limit Type",
//"  ,                        !- Minimum Outdoor Air Schedule Name",
//"  ,                        !- Minimum Fraction of Outdoor Air Schedule Name",
//"  ,                        !- Maximum Fraction of Outdoor Air Schedule Name",
//"  DCVObject;               !- Mechanical Ventilation Controller Name",

//"Controller:MechanicalVentilation,",
//"  DCVObject,               !- Name",
//"  AvailSched,              !- Availability Schedule Name",
//"  Yes,                     !- Demand Controlled Ventilation",
//"  Standard62.1VentilationRateProcedure,!- System Outdoor Air Method",
//"  ,                        !- Zone Maximum Outdoor Air Fraction {dimensionless}",
//"  Space,  !- Zone 1 Name",
//"  Space DSOA Design OA Spec,  !- Design Specification Outdoor Air Object Name 1",
//"  Space DSOA Design ADE Spec, !- Design Specification Zone Air Distribution Object Name 1",
//"  Spacex10,      !- Zone 2 Name",
//"  Spacex10 DSOA Design OA Spec,  !- Design Specification Outdoor Air Object Name 2",
//"  Space DSOA Design ADE Spec;  !- Design Specification Zone Air Distribution Object Name 2",

//"DesignSpecification:OutdoorAir,",
//"  Space DSOA Design OA Spec,  !- Name",
//"  sum,                     !- Outdoor Air Method",
//"  0.0,                     !- Outdoor Air Flow per Person {m3/s-person}",
//"  0.0009,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
//"  0;                       !- Outdoor Air Flow per Zone {m3/s}",

//"DesignSpecification:OutdoorAir,",
//"  Spacex10 DSOA Design OA Spec,  !- Name",
//"  sum,                     !- Outdoor Air Method",
//"  0.008,                   !- Outdoor Air Flow per Person {m3/s-person}",
//"  0.0009,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
//"  0;                       !- Outdoor Air Flow per Zone {m3/s}",

//"DesignSpecification:ZoneAirDistribution,",
//"  Space DSOA Design ADE Spec,  !- Name",
//"  1,                       !- Zone Air Distribution Effectiveness in Cooling Mode {dimensionless}",
//"  1;                       !- Zone Air Distribution Effectiveness in Heating Mode {dimensionless}",

//"ZoneHVAC:EquipmentConnections,",
//" Space,                    !- Zone Name",
//" Space Eq,                 !- Zone Conditioning Equipment List Name",
//" Space In Node,            !- Zone Air Inlet Node or NodeList Name",
//" ,                         !- Zone Air Exhaust Node or NodeList Name",
//" Space Node,               !- Zone Air Node Name",
//" Space Ret Node;           !- Zone Return Air Node Name",

//"ZoneHVAC:EquipmentList,",
//" Space Eq,                 !- Name",
//" ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
//" Space ATU,                   !- Zone Equipment 1 Name",
//" 1,                        !- Zone Equipment 1 Cooling Sequence",
//" 1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

//"ZoneHVAC:AirDistributionUnit,",
//"  Space ATU,  !- Name",
//"  Space In Node,  !- Air Distribution Unit Outlet Node Name",
//"  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
//"  Space Air Terminal;  !- Air Terminal Name",

//"AirTerminal:SingleDuct:VAV:NoReheat,",
//"  Space Air Terminal,      !- Name",
//"  AvailSched,              !- Availability Schedule Name",
//"  Space In Node,           !- Air Outlet Node Name",
//"  Space ATU In Node,       !- Air Inlet Node Name",
//"  autosize,                !- Maximum Air Flow Rate {m3/s}",
//"  Constant,                !- Zone Minimum Air Flow Input Method",
//"  0.0,                     !- Constant Minimum Air Flow Fraction",
//"  ,                        !- Fixed Minimum Air Flow Rate {m3/s}",
//"  ,                        !- Minimum Air Flow Fraction Schedule Name",
//"  ;                        !- Design Specification Outdoor Air Object Name",

//"ZoneHVAC:EquipmentConnections,",
//" Spacex10,                 !- Zone Name",
//" Spacex10 Eq,              !- Zone Conditioning Equipment List Name",
//" Spacex10 In Node,         !- Zone Air Inlet Node or NodeList Name",
//" ,                         !- Zone Air Exhaust Node or NodeList Name",
//" Spacex10 Node,            !- Zone Air Node Name",
//" Spacex10 Ret Node;        !- Zone Return Air Node Name",

//"ZoneHVAC:EquipmentList,",
//" Spacex10 Eq,              !- Name",
//" ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
//" Spacex10 ATU,                !- Zone Equipment 1 Name",
//" 1,                        !- Zone Equipment 1 Cooling Sequence",
//" 1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

//"ZoneHVAC:AirDistributionUnit,",
//"  Spacex10 ATU,  !- Name",
//"  Spacex10 In Node,  !- Air Distribution Unit Outlet Node Name",
//"  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
//"  Spacex10 Air Terminal;  !- Air Terminal Name",

//"AirTerminal:SingleDuct:VAV:NoReheat,",
//"  Spacex10 Air Terminal,   !- Name",
//"  AvailSched,              !- Availability Schedule Name",
//"  Spacex10 In Node,        !- Air Outlet Node Name",
//"  Spacex10 ATU In Node,    !- Air Inlet Node Name",
//"  autosize,                !- Maximum Air Flow Rate {m3/s}",
//"  Constant,                !- Zone Minimum Air Flow Input Method",
//"  0.0,                     !- Constant Minimum Air Flow Fraction",
//"  ,                        !- Fixed Minimum Air Flow Rate {m3/s}",
//"  ,                        !- Minimum Air Flow Fraction Schedule Name",
//"  ;                        !- Design Specification Outdoor Air Object Name",

//"ScheduleTypeLimits,",
//"  Any Number;              !- Name",

//"Schedule:Compact,",
//"  AvailSched,              !- Name",
//"  Any Number,              !- Schedule Type Limits Name",
//"  Through: 12/31,          !- Field 3",
//"  For: AllDays,            !- Field 4",
//"  Until: 24:00,1.0;        !- Field 5",

//"Schedule:Compact,",
//"  EquipSched,              !- Name",
//"  Any Number,              !- Schedule Type Limits Name",
//"  Through: 12/31,          !- Field 3",
//"  For: WinterDesignDay,    !- Field 4",
//"  Until: 24:00,0.0,        !- Field 5",
//"  For: AllOtherDays,       !- Field 4",
//"  Until: 24:00,1.0;        !- Field 5",

//"Schedule:Compact,",
//"  ActivityLevelSched,      !- Name",
//"  Any Number,              !- Schedule Type Limits Name",
//"  Through: 12/31,          !- Field 3",
//"  For: AllDays,            !- Field 4",
//"  Until: 24:00,120.0;      !- Field 5",

//"Schedule:Compact,",
//"  FanOpModeSchedule,       !- Name",
//"  Any Number,              !- Schedule Type Limits Name",
//"  Through: 12/31,          !- Field 1",
//"  For: AllDays,            !- Field 2",
//"  Until: 24:00,1.0;        !- Field 7",

//"Curve:Biquadratic,",
//"  Biquadratic,             !- Name",
//"  1.0,                     !- Coefficient1 Constant",
//"  0.0,                     !- Coefficient2 x",
//"  0.0,                     !- Coefficient3 x**2",
//"  0.0,                     !- Coefficient4 y",
//"  0.0,                     !- Coefficient5 y**2",
//"  0.0,                     !- Coefficient6 x*y",
//"  5,                       !- Minimum Value of x",
//"  40,                      !- Maximum Value of x",
//"  5,                       !- Minimum Value of y",
//"  40,                      !- Maximum Value of y",
//"  ,                        !- Minimum Curve Output",
//"  ,                        !- Maximum Curve Output",
//"  Temperature,             !- Input Unit Type for X",
//"  Temperature,             !- Input Unit Type for Y",
//"  Dimensionless;           !- Output Unit Type",

//"Curve:Cubic,",
//"  Cubic,                   !- Name",
//"  1.0,                     !- Coefficient1 Constant",
//"  0.0,                     !- Coefficient2 x",
//"  0.0,                     !- Coefficient3 x**2",
//"  0,                       !- Coefficient4 x**3",
//"  5,                       !- Minimum Value of x",
//"  40,                      !- Maximum Value of x",
//"  ,                        !- Minimum Curve Output",
//"  ,                        !- Maximum Curve Output",
//"  Temperature,             !- Input Unit Type for X",
//"  Temperature;             !- Output Unit Type",
//} );

// ASSERT_TRUE( process_idf( idf_objects ) );

// OutputProcessor::TimeValue.allocate(2);
////state->dataGlobal->DDOnlySimulation = true;

// ManageSimulation(); // run the design day over the warmup period (24 hrs, 25 days)

// EXPECT_EQ( 10.0, ( Zone( 2 ).Volume * Zone( 2 ).Multiplier * Zone( 2 ).ListMultiplier ) / ( Zone( 1 ).Volume * Zone( 1 ).Multiplier * Zone( 1
// ).ListMultiplier ) );

// state->dataGlobal->DoWeathSim = true; // flag to trick tabular reports to scan meters
// DataGlobals::KindOfSim = Constant::KindOfSim::RunPeriodWeather; // fake a weather run since a weather file can't be used (could it?)
// UpdateTabularReports( OutputProcessor::TimeStepType::TimeStepSystem );

// EXPECT_NEAR( 1.86168, DataSizing::FinalSysSizing( 1 ).DesOutAirVolFlow, 0.0001 );

//}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_ResetMonthlyGathering)
{
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "Space Gains Annual Report, !- Name",
        "2, !-  Digits After Decimal",
        "Exterior Lights Electricity Energy, !- Variable or Meter 1 Name",
        "SumOrAverage, !- Aggregation Type for Variable or Meter 1",
        "Exterior Lights Electricity Rate, !- Variable or Meter 2 Name",
        "Maximum, !- Aggregation Type for Variable or Meter 2",
        "Exterior Lights Electricity Rate, !- Variable or Meter 2 Name",
        "Minimum; !- Aggregation Type for Variable or Meter 2",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite3",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    InitializeTabularMonthly(*state);

    extLitUse = 1.01;

    state->dataEnvrn->Month = 12;

    GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 1, state->dataOutRptTab->MonthlyColumns(1).reslt(12));

    GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 2, state->dataOutRptTab->MonthlyColumns(1).reslt(12));

    GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 3, state->dataOutRptTab->MonthlyColumns(1).reslt(12));

    ResetMonthlyGathering(*state);

    EXPECT_EQ(0., state->dataOutRptTab->MonthlyColumns(1).reslt(12));

    GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 1, state->dataOutRptTab->MonthlyColumns(1).reslt(12));
}

TEST_F(EnergyPlusFixture, OutputReportTabular_ConfirmResetBEPSGathering)
{

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite3",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 3600.0;
    state->dataGlobal->MinutesInTimeStep = state->dataGlobal->TimeStepZone * 60.0;
    state->dataOutRptTab->displayTabularBEPS = true;
    // OutputProcessor::TimeValue.allocate(2);

    auto timeStep = 1.0;

    SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, timeStep);
    SetupTimePointers(*state, OutputProcessor::TimeStepType::System, timeStep);

    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep = 60;
    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep = 60;

    GetInputOutputTableSummaryReports(*state);

    extLitUse = 1.01;

    state->dataEnvrn->Month = 12;

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 3, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1));

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 6, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1));

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 9, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1));

    ResetBEPSGathering(*state);

    EXPECT_EQ(0., state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1));

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_EQ(extLitUse * 3, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1));
}

TEST_F(EnergyPlusFixture, OutputReportTabular_GatherPeakDemandForTimestep)
{
    // Glazer - Sep 2017
    auto &op = state->dataOutputProcessor;

    state->dataOutRptTab->displayDemandEndUse = true;
    state->dataOutRptTab->displayLEEDSummary = true;
    state->dataGlobal->TimeStepZoneSec = 900.0;

    int resourceNum = 1;

    Meter *meterTotal = new Meter("Total");
    Meter *meterEndUse = new Meter("EndUse");
    Meter *meterSubEndUse = new Meter("SubEndUse");

    op->meters.push_back(meterTotal);
    int totalMeterNum = (int)op->meters.size() - 1;
    state->dataOutputProcessor->meters.push_back(meterEndUse);
    int endUseMeterNum = (int)op->meters.size() - 1;
    state->dataOutputProcessor->meters.push_back(meterSubEndUse);
    int subEndUseMeterNum = (int)op->meters.size() - 1;

    int endUseNum = 1;
    int subEndUseNum = 1;

    state->dataOutRptTab->meterNumEndUseSubBEPS.allocate(10, 10, 10);
    state->dataOutRptTab->gatherDemandEndUseSub.allocate(10, 10, 10);
    state->dataOutRptTab->gatherDemandIndEndUseSub.allocate(10, 10, 10);

    state->dataOutputProcessor->EndUseCategory.allocate(endUseNum);
    state->dataOutputProcessor->EndUseCategory(endUseNum).NumSubcategories = 1;

    state->dataOutRptTab->meterNumTotalsBEPS(resourceNum) = totalMeterNum; // create a test meter number
    state->dataOutRptTab->gatherDemandTotal(resourceNum) = 0.;

    state->dataOutRptTab->meterNumEndUseBEPS(resourceNum, endUseNum) = endUseMeterNum;
    state->dataOutRptTab->gatherDemandEndUse(resourceNum, endUseNum) = 0.;
    state->dataOutRptTab->gatherDemandIndEndUse(resourceNum, endUseNum) = 0.;

    state->dataOutRptTab->meterNumEndUseSubBEPS(subEndUseNum, endUseNum, resourceNum) = subEndUseMeterNum;
    state->dataOutRptTab->gatherDemandEndUseSub(subEndUseNum, endUseNum, resourceNum) = 0.;
    state->dataOutRptTab->gatherDemandIndEndUseSub(subEndUseNum, endUseNum, resourceNum) = 0.;

    // first "timestep"

    op->meters[totalMeterNum]->CurTSValue = 123.0 * state->dataGlobal->TimeStepZoneSec;    // create the current value for the total meter
    op->meters[endUseMeterNum]->CurTSValue = 47.0 * state->dataGlobal->TimeStepZoneSec;    // create the current value for the end use meter
    op->meters[subEndUseMeterNum]->CurTSValue = 28.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the sub end use meter

    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);

    EXPECT_EQ(123., state->dataOutRptTab->gatherDemandTotal(resourceNum));

    EXPECT_EQ(47., state->dataOutRptTab->gatherDemandEndUse(resourceNum, endUseNum));
    EXPECT_EQ(47., state->dataOutRptTab->gatherDemandIndEndUse(resourceNum, endUseNum));

    EXPECT_EQ(28., state->dataOutRptTab->gatherDemandEndUseSub(subEndUseNum, endUseNum, resourceNum));
    EXPECT_EQ(28., state->dataOutRptTab->gatherDemandIndEndUseSub(subEndUseNum, endUseNum, resourceNum));

    // next "timestep" total higher

    state->dataOutputProcessor->meters[totalMeterNum]->CurTSValue =
        133.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the total meter
    state->dataOutputProcessor->meters[endUseMeterNum]->CurTSValue =
        57.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the end use meter
    state->dataOutputProcessor->meters[subEndUseMeterNum]->CurTSValue =
        38.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the sub end use meter

    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);

    EXPECT_EQ(133., state->dataOutRptTab->gatherDemandTotal(resourceNum));

    EXPECT_EQ(57., state->dataOutRptTab->gatherDemandEndUse(resourceNum, endUseNum));
    EXPECT_EQ(57., state->dataOutRptTab->gatherDemandIndEndUse(resourceNum, endUseNum));

    EXPECT_EQ(38., state->dataOutRptTab->gatherDemandEndUseSub(subEndUseNum, endUseNum, resourceNum));
    EXPECT_EQ(38., state->dataOutRptTab->gatherDemandIndEndUseSub(subEndUseNum, endUseNum, resourceNum));

    // next "timestep" total lower but end use higher and sub end use higher

    state->dataOutputProcessor->meters[totalMeterNum]->CurTSValue =
        103.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the total meter
    state->dataOutputProcessor->meters[endUseMeterNum]->CurTSValue =
        61.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the end use meter
    state->dataOutputProcessor->meters[subEndUseMeterNum]->CurTSValue =
        42.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the sub end use meter

    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);

    EXPECT_EQ(133., state->dataOutRptTab->gatherDemandTotal(resourceNum));

    EXPECT_EQ(57., state->dataOutRptTab->gatherDemandEndUse(resourceNum, endUseNum));
    EXPECT_EQ(61., state->dataOutRptTab->gatherDemandIndEndUse(resourceNum, endUseNum));

    EXPECT_EQ(38., state->dataOutRptTab->gatherDemandEndUseSub(subEndUseNum, endUseNum, resourceNum));
    EXPECT_EQ(42., state->dataOutRptTab->gatherDemandIndEndUseSub(subEndUseNum, endUseNum, resourceNum));

    // next "timestep" total higher but end use lower and sub end use lower

    state->dataOutputProcessor->meters[totalMeterNum]->CurTSValue =
        143.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the total meter
    state->dataOutputProcessor->meters[endUseMeterNum]->CurTSValue =
        59.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the end use meter
    state->dataOutputProcessor->meters[subEndUseMeterNum]->CurTSValue =
        39.0 * state->dataGlobal->TimeStepZoneSec; // create the current value for the sub end use meter

    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);

    EXPECT_EQ(143., state->dataOutRptTab->gatherDemandTotal(resourceNum));

    EXPECT_EQ(59., state->dataOutRptTab->gatherDemandEndUse(resourceNum, endUseNum));
    EXPECT_EQ(61., state->dataOutRptTab->gatherDemandIndEndUse(resourceNum, endUseNum));

    EXPECT_EQ(39., state->dataOutRptTab->gatherDemandEndUseSub(subEndUseNum, endUseNum, resourceNum));
    EXPECT_EQ(42., state->dataOutRptTab->gatherDemandIndEndUseSub(subEndUseNum, endUseNum, resourceNum));

    state->dataOutRptTab->meterNumEndUseSubBEPS.deallocate();
    state->dataOutRptTab->gatherDemandEndUseSub.deallocate();
    state->dataOutRptTab->gatherDemandIndEndUseSub.deallocate();
}

TEST_F(EnergyPlusFixture, OutputReportTabular_GatherHeatEmissionReport)
{

    state->dataOutRptTab->displayHeatEmissionsSummary = true;
    state->dataGlobal->DoWeathSim = true;
    state->dataHVACGlobal->TimeStepSys = 10.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    state->dataEnvrn->OutHumRat = 0.005;
    state->dataEnvrn->OutDryBulbTemp = 25.0;

    state->dataMixedAir->NumOAControllers = 2;
    state->dataMixedAir->OAController.allocate(2);
    state->dataMixedAir->OAController(1).RelTotalLossRate = 1.0;
    state->dataMixedAir->OAController(2).RelTotalLossRate = 1.0;
    state->dataCondenserLoopTowers->towers.allocate(1);
    state->dataCondenserLoopTowers->towers(1).Qactual = 1.0;
    state->dataCondenserLoopTowers->towers(1).FanEnergy = 50.0;

    Real64 reliefEnergy = 2.0 * state->dataHVACGlobal->TimeStepSysSec;
    Real64 condenserReject = 1.0 * state->dataHVACGlobal->TimeStepSysSec + 50.0;

    GatherHeatEmissionReport(*state, OutputProcessor::TimeStepType::System);

    EXPECT_EQ(reliefEnergy, state->dataHeatBal->SysTotalHVACReliefHeatLoss);
    EXPECT_EQ(reliefEnergy * Constant::convertJtoGJ, state->dataHeatBal->BuildingPreDefRep.emiHVACRelief);
    EXPECT_EQ(condenserReject, state->dataHeatBal->SysTotalHVACRejectHeatLoss);
    EXPECT_EQ(condenserReject * Constant::convertJtoGJ, state->dataHeatBal->BuildingPreDefRep.emiHVACReject);

    state->dataDXCoils->NumDXCoils = 2;
    state->dataDXCoils->DXCoil.allocate(2);
    state->dataDXCoils->DXCoil(1).DXCoilType_Num = HVAC::CoilDX_MultiSpeedCooling;
    state->dataDXCoils->DXCoil(1).CondenserType(1) = DataHeatBalance::RefrigCondenserType::Air;
    state->dataDXCoils->DXCoil(1).FuelType = Constant::eFuel::NaturalGas;
    state->dataDXCoils->DXCoil(1).ElecCoolingConsumption = 100.0;
    state->dataDXCoils->DXCoil(1).TotalCoolingEnergy = 100.0;
    state->dataDXCoils->DXCoil(1).MSFuelWasteHeat = 1.0;
    state->dataDXCoils->DXCoil(1).DefrostConsumption = 0.0;
    state->dataDXCoils->DXCoil(1).CrankcaseHeaterConsumption = 0.0;
    state->dataDXCoils->DXCoil(2).DXCoilType_Num = HVAC::CoilDX_HeatingEmpirical;
    state->dataDXCoils->DXCoil(2).ElecHeatingConsumption = 50.0;
    state->dataDXCoils->DXCoil(2).TotalHeatingEnergy = 40.0;
    state->dataDXCoils->DXCoil(2).DefrostConsumption = 0.0;
    // state->dataDXCoils->DXCoil(2).FuelConsumed = 0.0; should be initialized to zero
    state->dataDXCoils->DXCoil(2).CrankcaseHeaterConsumption = 0.0;

    Real64 coilReject = 1.0 * state->dataHVACGlobal->TimeStepSysSec + (100.0 + 100.0) + (50.0 - 40.0);
    GatherHeatEmissionReport(*state, OutputProcessor::TimeStepType::System);
    EXPECT_EQ(reliefEnergy, state->dataHeatBal->SysTotalHVACReliefHeatLoss);
    EXPECT_EQ(2 * reliefEnergy * Constant::convertJtoGJ, state->dataHeatBal->BuildingPreDefRep.emiHVACRelief);
    EXPECT_EQ(condenserReject + coilReject, state->dataHeatBal->SysTotalHVACRejectHeatLoss);
    EXPECT_EQ(2 * condenserReject * Constant::convertJtoGJ + coilReject * Constant::convertJtoGJ,
              state->dataHeatBal->BuildingPreDefRep.emiHVACReject);

    state->dataDXCoils->DXCoil(1).DXCoilType_Num = HVAC::CoilDX_MultiSpeedCooling;
    state->dataDXCoils->DXCoil(1).CondenserType(1) = DataHeatBalance::RefrigCondenserType::Air;
    state->dataDXCoils->DXCoil(1).FuelType = Constant::eFuel::NaturalGas;
    state->dataDXCoils->DXCoil(1).ElecCoolingConsumption = 100.0;
    state->dataDXCoils->DXCoil(1).TotalCoolingEnergy = 100.0;
    state->dataDXCoils->DXCoil(1).MSFuelWasteHeat = 1.0;
    state->dataDXCoils->DXCoil(1).DefrostConsumption = 0.0;
    state->dataDXCoils->DXCoil(1).CrankcaseHeaterConsumption = 20.0;
    state->dataDXCoils->DXCoil(1).FuelConsumed = 50.0; // not included for cooling
    state->dataDXCoils->DXCoil(2).DXCoilType_Num = HVAC::CoilDX_MultiSpeedHeating;
    state->dataDXCoils->DXCoil(2).ElecHeatingConsumption = 15.0;
    state->dataDXCoils->DXCoil(2).TotalHeatingEnergy = 100.0;
    state->dataDXCoils->DXCoil(2).DefrostConsumption = 10.0;
    state->dataDXCoils->DXCoil(2).FuelConsumed = 30.0;
    state->dataDXCoils->DXCoil(2).CrankcaseHeaterConsumption = 5.0;

    Real64 coilReject2 = 1.0 * state->dataHVACGlobal->TimeStepSysSec + (100.0 + 100.0 + 20.0) + (15.0 + 10.0 + 30.0 + 5.0 - 100.0);
    GatherHeatEmissionReport(*state, OutputProcessor::TimeStepType::System);
    EXPECT_EQ(reliefEnergy, state->dataHeatBal->SysTotalHVACReliefHeatLoss);
    EXPECT_NEAR(3 * reliefEnergy * Constant::convertJtoGJ, state->dataHeatBal->BuildingPreDefRep.emiHVACRelief, 0.0000001);
    EXPECT_EQ(condenserReject + coilReject2, state->dataHeatBal->SysTotalHVACRejectHeatLoss);
    EXPECT_NEAR(3 * condenserReject * Constant::convertJtoGJ + (coilReject + coilReject2) * Constant::convertJtoGJ,
                state->dataHeatBal->BuildingPreDefRep.emiHVACReject,
                0.0000001);
}

TEST_F(EnergyPlusFixture, OutputTableTimeBins_GetInput)
{
    std::string const idf_objects = delimited_string({"Output:Table:TimeBins,",
                                                      "System1, !- Key Value",
                                                      "Some Temperature Variable, !- Variable Name",
                                                      "0.00, !- Interval Start",
                                                      "0.20, !- Interval Size",
                                                      "5,                       !- Interval Count",
                                                      "Constant-1.0; !- Schedule Name"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->DoWeathSim = true;

    GetInputTabularTimeBins(*state);

    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned.size(), 1u);
    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned(1).keyValue, "SYSTEM1");
    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned(1).varOrMeter, "SOME TEMPERATURE VARIABLE");
    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned(1).intervalStart, 0.0);
    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned(1).intervalSize, 0.20);
    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned(1).intervalCount, 5);
    EXPECT_EQ(state->dataOutRptTab->OutputTableBinned(1).sched->Name, "Constant-1.0");
}

// TEST_F( EnergyPlusFixture, FinAndOverhangCount )
//{
//// based on 4ZoneWithShading_Simple_2.idf
// std::string const idf_objects = delimited_string( {

//"                                                                                                   ",
//" Output:Diagnostics, DisplayExtraWarnings;",
//"  Timestep,4;                                                                                      ",
//"                                                                                                   ",
//"  Building,                                                                                        ",
//"    BUILDING #1,             !- Name                                                               ",
//"    0.0000000E+00,           !- North Axis {deg}                                                   ",
//"    Suburbs,                 !- Terrain                                                            ",
//"    3.9999999E-02,           !- Loads Convergence Tolerance Value                                  ",
//"    0.4000000,               !- Temperature Convergence Tolerance Value {deltaC}                   ",
//"    FullExterior,            !- Solar Distribution                                                 ",
//"    25,                      !- Maximum Number of Warmup Days                                      ",
//"    6;                       !- Minimum Number of Warmup Days                                      ",
//"                                                                                                   ",
//"  SurfaceConvectionAlgorithm:Inside,Simple;                                                        ",
//"                                                                                                   ",
//"  SurfaceConvectionAlgorithm:Outside,SimpleCombined;                                               ",
//"                                                                                                   ",
//"  HeatBalanceAlgorithm,ConductionTransferFunction;                                                 ",
//"                                                                                                   ",
//"  SimulationControl,                                                                               ",
//"    No,                     !- Do Zone Sizing Calculation                                         ",
//"    No,                      !- Do System Sizing Calculation                                       ",
//"    No,                      !- Do Plant Sizing Calculation                                        ",
//"    Yes,                     !- Run Simulation for Sizing Periods                                  ",
//"    No;                      !- Run Simulation for Weather File Run Periods                        ",
//"                                                                                                   ",
//"  Site:Location,                                                                                   ",
//"    Chicago Ohare Intl Ap IL USA WMO=725300,  !- Name                                              ",
//"    41.99,                   !- Latitude {deg}                                                     ",
//"    -87.91,                  !- Longitude {deg}                                                    ",
//"    -6.00,                   !- Time Zone {hr}                                                     ",
//"    205.00;                  !- Elevation {m}                                                      ",
//"                                                                                                   ",
//"  SizingPeriod:DesignDay,                                                                          ",
//"    Chicago Ohare Intl Ap Ann Htg 99% Condns DB,  !- Name                                          ",
//"    1,                       !- Month                                                              ",
//"    21,                      !- Day of Month                                                       ",
//"    WinterDesignDay,         !- Day Type                                                           ",
//"    -16.6,                   !- Maximum Dry-Bulb Temperature {C}                                   ",
//"    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}                          ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Type                           ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name              ",
//"    Wetbulb,                 !- Humidity Condition Type                                            ",
//"    -16.6,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}                        ",
//"    ,                        !- Humidity Condition Day Schedule Name                               ",
//"    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}              ",
//"    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}                                ",
//"    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}                          ",
//"    98886.,                  !- Barometric Pressure {Pa}                                           ",
//"    4.9,                     !- Wind Speed {m/s}                                                   ",
//"    270,                     !- Wind Direction {deg}                                               ",
//"    No,                      !- Rain Indicator                                                     ",
//"    No,                      !- Snow Indicator                                                     ",
//"    No,                      !- Daylight Saving Time Indicator                                     ",
//"    ASHRAEClearSky,          !- Solar Model Indicator                                              ",
//"    ,                        !- Beam Solar Day Schedule Name                                       ",
//"    ,                        !- Diffuse Solar Day Schedule Name                                    ",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensio",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimen",
//"    0.00;                    !- Sky Clearness                                                      ",
//"                                                                                                   ",
//"  SizingPeriod:DesignDay,                                                                          ",
//"    Chicago Ohare Intl Ap Ann Clg 1% Condns DB=>MWB,  !- Name                                      ",
//"    7,                       !- Month                                                              ",
//"    21,                      !- Day of Month                                                       ",
//"    SummerDesignDay,         !- Day Type                                                           ",
//"    31.6,                    !- Maximum Dry-Bulb Temperature {C}                                   ",
//"    10.5,                    !- Daily Dry-Bulb Temperature Range {deltaC}                          ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Type                           ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name              ",
//"    Wetbulb,                 !- Humidity Condition Type                                            ",
//"    23,                      !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}                        ",
//"    ,                        !- Humidity Condition Day Schedule Name                               ",
//"    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}              ",
//"    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}                                ",
//"    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}                          ",
//"    98886.,                  !- Barometric Pressure {Pa}                                           ",
//"    5.2,                     !- Wind Speed {m/s}                                                   ",
//"    230,                     !- Wind Direction {deg}                                               ",
//"    No,                      !- Rain Indicator                                                     ",
//"    No,                      !- Snow Indicator                                                     ",
//"    No,                      !- Daylight Saving Time Indicator                                     ",
//"    ASHRAEClearSky,          !- Solar Model Indicator                                              ",
//"    ,                        !- Beam Solar Day Schedule Name                                       ",
//"    ,                        !- Diffuse Solar Day Schedule Name                                    ",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensio",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimen",
//"    1.00;                    !- Sky Clearness                                                      ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    A2 - 4 IN DENSE FACE BRICK,  !- Name                                                           ",
//"    Rough,                   !- Roughness                                                          ",
//"    0.1014984,               !- Thickness {m}                                                      ",
//"    1.245296,                !- Conductivity {W/m-K}                                               ",
//"    2082.400,                !- Density {kg/m3}                                                    ",
//"    920.4800,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.9300000,               !- Solar Absorptance                                                  ",
//"    0.9300000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    B3 - 2 IN INSULATION,    !- Name                                                               ",
//"    VeryRough,               !- Roughness                                                          ",
//"    5.0901599E-02,           !- Thickness {m}                                                      ",
//"    4.3239430E-02,           !- Conductivity {W/m-K}                                               ",
//"    32.03693,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.5000000,               !- Solar Absorptance                                                  ",
//"    0.5000000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    C2 - 4 IN LW CONCRETE BLOCK,  !- Name                                                          ",
//"    MediumRough,             !- Roughness                                                          ",
//"    0.1014984,               !- Thickness {m}                                                      ",
//"    0.3805070,               !- Conductivity {W/m-K}                                               ",
//"    608.7016,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.6500000,               !- Solar Absorptance                                                  ",
//"    0.6500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name                                                   ",
//"    Smooth,                  !- Roughness                                                          ",
//"    1.9050000E-02,           !- Thickness {m}                                                      ",
//"    0.7264224,               !- Conductivity {W/m-K}                                               ",
//"    1601.846,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.9200000,               !- Solar Absorptance                                                  ",
//"    0.9200000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"   ! (R=.0472,TRANS=.80,VERY SMOOTH,GLASS), from 6mm clear                                         ",
//"                                                                                                   ",
//"  WindowMaterial:Glazing,                                                                          ",
//"    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name                                                         ",
//"    SpectralAverage,         !- Optical Data Type                                                  ",
//"    ,                        !- Window Glass Spectral Data Set Name                                ",
//"    6.0000001E-03,           !- Thickness {m}                                                      ",
//"    0.7750000,               !- Solar Transmittance at Normal Incidence                            ",
//"    7.1000002E-02,           !- Front Side Solar Reflectance at Normal Incidence                   ",
//"    7.1000002E-02,           !- Back Side Solar Reflectance at Normal Incidence                    ",
//"    0.8810000,               !- Visible Transmittance at Normal Incidence                          ",
//"    7.9999998E-02,           !- Front Side Visible Reflectance at Normal Incidence                 ",
//"    7.9999998E-02,           !- Back Side Visible Reflectance at Normal Incidence                  ",
//"    0.0000000E+00,           !- Infrared Transmittance at Normal Incidence                         ",
//"    0.8400000,               !- Front Side Infrared Hemispherical Emissivity                       ",
//"    0.8400000,               !- Back Side Infrared Hemispherical Emissivity                        ",
//"    0.9000000;               !- Conductivity {W/m-K}                                               ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    WOOD - HARDWOOD 1 / 8 IN,!- Name                                                               ",
//"    MediumSmooth,            !- Roughness                                                          ",
//"    3.1699201E-03,           !- Thickness {m}                                                      ",
//"    0.1591211,               !- Conductivity {W/m-K}                                               ",
//"    720.8308,                !- Density {kg/m3}                                                    ",
//"    1255.200,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.7800000,               !- Solar Absorptance                                                  ",
//"    0.7800000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material:AirGap,                                                                                 ",
//"    B1 - AIRSPACE RESISTANCE,!- Name                                                               ",
//"    0.1603675;               !- Thermal Resistance {m2-K/W}                                        ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    C7 - 8 IN LW CONCRETE BLOCK,  !- Name                                                          ",
//"    Rough,                   !- Roughness                                                          ",
//"    0.2033016,               !- Thickness {m}                                                      ",
//"    0.5707605,               !- Conductivity {W/m-K}                                               ",
//"    608.7016,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.6500000,               !- Solar Absorptance                                                  ",
//"    0.6500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    E5 - ACOUSTIC TILE,      !- Name                                                               ",
//"    MediumSmooth,            !- Roughness                                                          ",
//"    1.9050000E-02,           !- Thickness {m}                                                      ",
//"    6.0535200E-02,           !- Conductivity {W/m-K}                                               ",
//"    480.5539,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.3200000,               !- Solar Absorptance                                                  ",
//"    0.3200000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material:AirGap,                                                                                 ",
//"    E4 - CEILING AIRSPACE,   !- Name                                                               ",
//"    0.1762280;               !- Thermal Resistance {m2-K/W}                                        ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    C5 - 4 IN HW CONCRETE,   !- Name                                                               ",
//"    MediumRough,             !- Roughness                                                          ",
//"    0.1014984,               !- Thickness {m}                                                      ",
//"    1.729577,                !- Conductivity {W/m-K}                                               ",
//"    2242.585,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.6500000,               !- Solar Absorptance                                                  ",
//"    0.6500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    C10 - 8 IN HW CONCRETE,  !- Name                                                               ",
//"    MediumRough,             !- Roughness                                                          ",
//"    0.2033016,               !- Thickness {m}                                                      ",
//"    1.729577,                !- Conductivity {W/m-K}                                               ",
//"    2242.585,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.6500000,               !- Solar Absorptance                                                  ",
//"    0.6500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    E2 - 1 / 2 IN SLAG OR STONE,  !- Name                                                          ",
//"    Rough,                   !- Roughness                                                          ",
//"    1.2710161E-02,           !- Thickness {m}                                                      ",
//"    1.435549,                !- Conductivity {W/m-K}                                               ",
//"    881.0155,                !- Density {kg/m3}                                                    ",
//"    1673.600,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.5500000,               !- Solar Absorptance                                                  ",
//"    0.5500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Name                                                      ",
//"    Rough,                   !- Roughness                                                          ",
//"    9.5402403E-03,           !- Thickness {m}                                                      ",
//"    0.1902535,               !- Conductivity {W/m-K}                                               ",
//"    1121.292,                !- Density {kg/m3}                                                    ",
//"    1673.600,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.7500000,               !- Solar Absorptance                                                  ",
//"    0.7500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    B6 - 2 IN DENSE INSULATION,  !- Name                                                           ",
//"    VeryRough,               !- Roughness                                                          ",
//"    5.0901599E-02,           !- Thickness {m}                                                      ",
//"    4.3239430E-02,           !- Conductivity {W/m-K}                                               ",
//"    91.30524,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.5000000,               !- Solar Absorptance                                                  ",
//"    0.5000000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Material,                                                                                        ",
//"    C12 - 2 IN HW CONCRETE,  !- Name                                                               ",
//"    MediumRough,             !- Roughness                                                          ",
//"    5.0901599E-02,           !- Thickness {m}                                                      ",
//"    1.729577,                !- Conductivity {W/m-K}                                               ",
//"    2242.585,                !- Density {kg/m3}                                                    ",
//"    836.8000,                !- Specific Heat {J/kg-K}                                             ",
//"    0.9000000,               !- Thermal Absorptance                                                ",
//"    0.6500000,               !- Solar Absorptance                                                  ",
//"    0.6500000;               !- Visible Absorptance                                                ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    EXTERIOR,                !- Name                                                               ",
//"    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer                                                  ",
//"    B3 - 2 IN INSULATION,    !- Layer 2                                                            ",
//"    C2 - 4 IN LW CONCRETE BLOCK,  !- Layer 3                                                       ",
//"    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4                                                ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    SINGLE PANE HW WINDOW,   !- Name                                                               ",
//"    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer                                                ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    HOLLOW WOOD DOOR,        !- Name                                                               ",
//"    WOOD - HARDWOOD 1 / 8 IN,!- Outside Layer                                                      ",
//"    B1 - AIRSPACE RESISTANCE,!- Layer 2                                                            ",
//"    WOOD - HARDWOOD 1 / 8 IN;!- Layer 3                                                            ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    INTERIOR,                !- Name                                                               ",
//"    C7 - 8 IN LW CONCRETE BLOCK;  !- Outside Layer                                                 ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    FLOOR,                   !- Name                                                               ",
//"    E5 - ACOUSTIC TILE,      !- Outside Layer                                                      ",
//"    E4 - CEILING AIRSPACE,   !- Layer 2                                                            ",
//"    C5 - 4 IN HW CONCRETE;   !- Layer 3                                                            ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    CEILING34,               !- Name                                                               ",
//"    C10 - 8 IN HW CONCRETE;  !- Outside Layer                                                      ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    FLOOR34,                 !- Name                                                               ",
//"    C10 - 8 IN HW CONCRETE;  !- Outside Layer                                                      ",
//"                                                                                                   ",
//"  Construction,                                                                                    ",
//"    ROOF31,                  !- Name                                                               ",
//"    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer                                                 ",
//"    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Layer 2                                                   ",
//"    B6 - 2 IN DENSE INSULATION,  !- Layer 3                                                        ",
//"    C12 - 2 IN HW CONCRETE;  !- Layer 4                                                            ",
//"                                                                                                   ",
//"  ScheduleTypeLimits,                                                                              ",
//"    Any Number;              !- Name                                                               ",
//"                                                                                                   ",
//"  ScheduleTypeLimits,                                                                              ",
//"    Fraction,                !- Name                                                               ",
//"    0.0,                     !- Lower Limit Value                                                  ",
//"    1.0,                     !- Upper Limit Value                                                  ",
//"    CONTINUOUS;              !- Numeric Type                                                       ",
//"                                                                                                   ",
//"  Site:GroundTemperature:BuildingSurface,20,20,20,20,20,20,20,20,20,20,20,20;                      ",
//"                                                                                                   ",
//"  Zone,                                                                                            ",
//"    ZONE 1,                  !- Name                                                               ",
//"    0,                       !- Direction of Relative North {deg}                                  ",
//"    0,                       !- X Origin {m}                                                       ",
//"    0,                       !- Y Origin {m}                                                       ",
//"    -10,                     !- Z Origin {m}                                                       ",
//"    1,                       !- Type                                                               ",
//"    1,                       !- Multiplier                                                         ",
//"    AutoCalculate,           !- Ceiling Height {m}                                                 ",
//"    AutoCalculate;           !- Volume {m3}                                                        ",
//"                                                                                                   ",
//"  Zone,                                                                                            ",
//"    ZONE 2,                  !- Name                                                               ",
//"    0,                       !- Direction of Relative North {deg}                                  ",
//"    20,                      !- X Origin {m}                                                       ",
//"    0,                       !- Y Origin {m}                                                       ",
//"    -10,                     !- Z Origin {m}                                                       ",
//"    1,                       !- Type                                                               ",
//"    1,                       !- Multiplier                                                         ",
//"    AutoCalculate,           !- Ceiling Height {m}                                                 ",
//"    AutoCalculate;           !- Volume {m3}                                                        ",
//"                                                                                                   ",
//"  Zone,                                                                                            ",
//"    ZONE 3,                  !- Name                                                               ",
//"    0,                       !- Direction of Relative North {deg}                                  ",
//"    0,                       !- X Origin {m}                                                       ",
//"    0,                       !- Y Origin {m}                                                       ",
//"    0,                       !- Z Origin {m}                                                       ",
//"    1,                       !- Type                                                               ",
//"    1,                       !- Multiplier                                                         ",
//"    AutoCalculate,           !- Ceiling Height {m}                                                 ",
//"    AutoCalculate;           !- Volume {m3}                                                        ",
//"                                                                                                   ",
//"  Zone,                                                                                            ",
//"    ZONE 4,                  !- Name                                                               ",
//"    0,                       !- Direction of Relative North {deg}                                  ",
//"    20,                      !- X Origin {m}                                                       ",
//"    0,                       !- Y Origin {m}                                                       ",
//"    0,                       !- Z Origin {m}                                                       ",
//"    1,                       !- Type                                                               ",
//"    1,                       !- Multiplier                                                         ",
//"    AutoCalculate,           !- Ceiling Height {m}                                                 ",
//"    AutoCalculate;           !- Volume {m3}                                                        ",
//"                                                                                                   ",
//"  GlobalGeometryRules,                                                                             ",
//"    UpperLeftCorner,         !- Starting Vertex Position                                           ",
//"    CounterClockWise,        !- Vertex Entry Direction                                             ",
//"    Relative,                !- Coordinate System                                                  ",
//"    ,                        !- Daylighting Reference Point Coordinate System                      ",
//"    Relative;                !- Rectangular Surface Coordinate System                              ",
//"                                                                                                   ",
//"  Shading:Building,                                                                                ",
//"    Bushes-East,             !- Name                                                               ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    45,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    50,                      !- Length {m}                                                         ",
//"    1;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Shading:Site,                                                                                    ",
//"    Bushes-North,            !- Name                                                               ",
//"    0,                       !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    45,                      !- Starting X Coordinate {m}                                          ",
//"    50,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    50,                      !- Length {m}                                                         ",
//"    1;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Shading:Site,                                                                                    ",
//"    Bushes-West,             !- Name                                                               ",
//"    270,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    -5,                      !- Starting X Coordinate {m}                                          ",
//"    50,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    50,                      !- Length {m}                                                         ",
//"    1;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn001:Wall001,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Window,                                                                                          ",
//"    Zn001:Wall001:Win001,    !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn001:Wall001,           !- Building Surface Name                                              ",
//"    ,                        !- Frame and Divider Name                                             ",
//"    1,                       !- Multiplier                                                         ",
//"    4,                       !- Starting X Coordinate {m}                                          ",
//"    3,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  GlazedDoor,                                                                                      ",
//"    Zn001:Wall001:Door001,   !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn001:Wall001,           !- Building Surface Name                                              ",
//"    ,                        !- Frame and Divider Name                                             ",
//"    1,                       !- Multiplier                                                         ",
//"    14,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Shading:Overhang:Projection,                                                                     ",
//"    Zn001:Wall001:Win001:Shade001,  !- Name                                                        ",
//"    Zn001:Wall001:Win001,    !- Window or Door Name                                                ",
//"    .7,                      !- Height above Window or Door {m}                                    ",
//"    90,                      !- Tilt Angle from Window/Door {deg}                                  ",
//"    .2,                      !- Left extension from Window/Door Width {m}                          ",
//"    .2,                      !- Right extension from Window/Door Width {m}                         ",
//"    .6;                      !- Depth as Fraction of Window/Door Height {dimensionless}            ",
//"                                                                                                   ",
//"  Shading:Overhang,                                                                                ",
//"    Zn001:Wall001:Door001:Shade001,  !- Name                                                       ",
//"    Zn001:Wall001:Door001,   !- Window or Door Name                                                ",
//"    .6,                      !- Height above Window or Door {m}                                    ",
//"    90,                      !- Tilt Angle from Window/Door {deg}                                  ",
//"    0,                       !- Left extension from Window/Door Width {m}                          ",
//"    0,                       !- Right extension from Window/Door Width {m}                         ",
//"    3;                       !- Depth {m}                                                          ",
//"                                                                                                   ",
//"  Shading:Fin:Projection,                                                                          ",
//"    Zn001:Wall001:Shade003,  !- Name                                                               ",
//"    Zn001:Wall001:Win001,    !- Window or Door Name                                                ",
//"    .1,                      !- Left Extension from Window/Door {m}                                ",
//"    .1,                      !- Left Distance Above Top of Window {m}                              ",
//"    .1,                      !- Left Distance Below Bottom of Window {m}                           ",
//"    90,                      !- Left Tilt Angle from Window/Door {deg}                             ",
//"    .6,                      !- Left Depth as Fraction of Window/Door Width {dimensionless}        ",
//"    .1,                      !- Right Extension from Window/Door {m}                               ",
//"    .1,                      !- Right Distance Above Top of Window {m}                             ",
//"    .1,                      !- Right Distance Below Bottom of Window {m}                          ",
//"    90,                      !- Right Tilt Angle from Window/Door {deg}                            ",
//"    .6;                      !- Right Depth as Fraction of Window/Door Width {dimensionless}       ",
//"                                                                                                   ",
//"  Wall:Underground,                                                                                ",
//"    Zn001:Wall002,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    0,                       !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Underground,                                                                                ",
//"    Zn001:Wall003,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    270,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Door,                                                                                            ",
//"    Zn001:Wall001:Door002,   !- Name                                                               ",
//"    HOLLOW WOOD DOOR,        !- Construction Name                                                  ",
//"    Zn001:Wall003,           !- Building Surface Name                                              ",
//"    1,                       !- Multiplier                                                         ",
//"    14,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Adiabatic,                                                                                  ",
//"    Zn001:Wall004,           !- Name                                                               ",
//"    INTERIOR,                !- Construction Name                                                  ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Floor:GroundContact,                                                                             ",
//"    Zn001:Flr001,            !- Name                                                               ",
//"    FLOOR,                   !- Construction Name                                                  ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    180,                     !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Ceiling:Adiabatic,                                                                               ",
//"    Zn001:Roof001,           !- Name                                                               ",
//"    CEILING34,               !- Construction Name                                                  ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    0,                       !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    10,                      !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn002:Wall001,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Window,                                                                                          ",
//"    Zn002:Wall001:Win001,    !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn002:Wall001,           !- Building Surface Name                                              ",
//"    ,                        !- Frame and Divider Name                                             ",
//"    1,                       !- Multiplier                                                         ",
//"    4,                       !- Starting X Coordinate {m}                                          ",
//"    3,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Shading:Overhang,                                                                                ",
//"    Zn002:Wall001:Win001:Shade001,  !- Name                                                        ",
//"    Zn002:Wall001:Win001,    !- Window or Door Name                                                ",
//"    .7,                      !- Height above Window or Door {m}                                    ",
//"    90,                      !- Tilt Angle from Window/Door {deg}                                  ",
//"    0,                       !- Left extension from Window/Door Width {m}                          ",
//"    0,                       !- Right extension from Window/Door Width {m}                         ",
//"    3;                       !- Depth {m}                                                          ",
//"                                                                                                   ",
//"  Window,                                                                                          ",
//"    Zn002:Wall001:Win002,    !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn002:Wall001,           !- Building Surface Name                                              ",
//"    ,                        !- Frame and Divider Name                                             ",
//"    1,                       !- Multiplier                                                         ",
//"    10,                      !- Starting X Coordinate {m}                                          ",
//"    3,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Underground,                                                                                ",
//"    Zn002:Wall002,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Underground,                                                                                ",
//"    Zn002:Wall003,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    0,                       !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Adiabatic,                                                                                  ",
//"    Zn002:Wall004,           !- Name                                                               ",
//"    INTERIOR,                !- Construction Name                                                  ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    270,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Floor:GroundContact,                                                                             ",
//"    Zn002:Flr001,            !- Name                                                               ",
//"    FLOOR,                   !- Construction Name                                                  ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    180,                     !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Ceiling:Adiabatic,                                                                               ",
//"    Zn002:Roof001,           !- Name                                                               ",
//"    CEILING34,               !- Construction Name                                                  ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    0,                       !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    10,                      !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn003:Wall001,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Window,                                                                                          ",
//"    Zn003:Wall001:Win001,    !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn003:Wall001,           !- Building Surface Name                                              ",
//"    ,                        !- Frame and Divider Name                                             ",
//"    1,                       !- Multiplier                                                         ",
//"    8,                       !- Starting X Coordinate {m}                                          ",
//"    3,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn003:Wall002,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    0,                       !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn003:Wall003,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    270,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Interzone,                                                                                  ",
//"    Zn003:Wall004,           !- Name                                                               ",
//"    INTERIOR,                !- Construction Name                                                  ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    ZONE 4,                  !- Outside Boundary Condition Object                                  ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  GlazedDoor:Interzone,                                                                            ",
//"    Zn003:Wall004:Door002,   !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn003:Wall004,           !- Building Surface Name                                              ",
//"    ZONE 4,                  !- Outside Boundary Condition Object                                  ",
//"    1,                       !- Multiplier                                                         ",
//"    10,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Window:Interzone,                                                                                ",
//"    Zn003:Wall004:Win001,    !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn003:Wall004,           !- Building Surface Name                                              ",
//"    ZONE 4,                  !- Outside Boundary Condition Object                                  ",
//"    1,                       !- Multiplier                                                         ",
//"    4,                       !- Starting X Coordinate {m}                                          ",
//"    3,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Door:Interzone,                                                                                  ",
//"    Zn003:Wall004:Door001,   !- Name                                                               ",
//"    HOLLOW WOOD DOOR,        !- Construction Name                                                  ",
//"    Zn003:Wall004,           !- Building Surface Name                                              ",
//"    ZONE 4,                  !- Outside Boundary Condition Object                                  ",
//"    1,                       !- Multiplier                                                         ",
//"    10,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Floor:Interzone,                                                                                 ",
//"    Zn003:Flr001,            !- Name                                                               ",
//"    FLOOR34,                 !- Construction Name                                                  ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    Zn001:Roof001,           !- Outside Boundary Condition Object                                  ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    180,                     !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Roof,                                                                                            ",
//"    Zn003:Roof001,           !- Name                                                               ",
//"    ROOF31,                  !- Construction Name                                                  ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    0,                       !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    10,                      !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn004:Wall001,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 4,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Window,                                                                                          ",
//"    Zn004:Wall001:Win001,    !- Name                                                               ",
//"    SINGLE PANE HW WINDOW,   !- Construction Name                                                  ",
//"    Zn004:Wall001,           !- Building Surface Name                                              ",
//"    ,                        !- Frame and Divider Name                                             ",
//"    1,                       !- Multiplier                                                         ",
//"    8,                       !- Starting X Coordinate {m}                                          ",
//"    3,                       !- Starting Z Coordinate {m}                                          ",
//"    3,                       !- Length {m}                                                         ",
//"    5;                       !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn004:Wall002,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 4,                  !- Zone Name                                                          ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Wall:Exterior,                                                                                   ",
//"    Zn004:Wall003,           !- Name                                                               ",
//"    EXTERIOR,                !- Construction Name                                                  ",
//"    ZONE 4,                  !- Zone Name                                                          ",
//"    0,                       !- Azimuth Angle {deg}                                                ",
//"    90,                      !- Tilt Angle {deg}                                                   ",
//"    20,                      !- Starting X Coordinate {m}                                          ",
//"    20,                      !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    10;                      !- Height {m}                                                         ",
//"                                                                                                   ",
//"  Floor:Interzone,                                                                                 ",
//"    Zn004:Flr001,            !- Name                                                               ",
//"    FLOOR34,                 !- Construction Name                                                  ",
//"    ZONE 4,                  !- Zone Name                                                          ",
//"    Zn002:Roof001,           !- Outside Boundary Condition Object                                  ",
//"    90,                      !- Azimuth Angle {deg}                                                ",
//"    180,                     !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    0,                       !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  Roof,                                                                                            ",
//"    Zn004:Roof001,           !- Name                                                               ",
//"    ROOF31,                  !- Construction Name                                                  ",
//"    ZONE 4,                  !- Zone Name                                                          ",
//"    180,                     !- Azimuth Angle {deg}                                                ",
//"    0,                       !- Tilt Angle {deg}                                                   ",
//"    0,                       !- Starting X Coordinate {m}                                          ",
//"    0,                       !- Starting Y Coordinate {m}                                          ",
//"    10,                      !- Starting Z Coordinate {m}                                          ",
//"    20,                      !- Length {m}                                                         ",
//"    20;                      !- Width {m}                                                          ",
//"                                                                                                   ",
//"  ScheduleTypeLimits,                                                                              ",
//"    Temperature,             !- Name                                                               ",
//"    -100,                    !- Lower Limit Value                                                  ",
//"    200,                     !- Upper Limit Value                                                  ",
//"    CONTINUOUS;              !- Numeric Type                                                       ",
//"                                                                                                   ",
//"  ScheduleTypeLimits,                                                                              ",
//"    ControlType,             !- Name                                                               ",
//"    0,                       !- Lower Limit Value                                                  ",
//"    4,                       !- Upper Limit Value                                                  ",
//"    DISCRETE;                !- Numeric Type                                                       ",
//"                                                                                                   ",
//"  SCHEDULE:COMPACT,                                                                                ",
//"    ZONE CONTROL TYPE SCHEDULE,  !- Name                                                           ",
//"    CONTROLTYPE,             !- Schedule Type Limits Name                                          ",
//"    Through: 12/31,          !- Field 1                                                            ",
//"    For: AllDays,            !- Field 2                                                            ",
//"    Until: 24:00,4;          !- Field 3                                                            ",
//"                                                                                                   ",
//"  SCHEDULE:COMPACT,                                                                                ",
//"    ZONE HEATING SETPOINTS,  !- Name                                                               ",
//"    TEMPERATURE,             !- Schedule Type Limits Name                                          ",
//"    Through: 12/31,          !- Field 1                                                            ",
//"    For: Weekdays WinterDesignDay, !- Field 2                                                      ",
//"    Until: 7:00,15.60,       !- Field 3                                                            ",
//"    Until: 17:00,19.40,      !- Field 5                                                            ",
//"    Until: 24:00,15.60,      !- Field 7                                                            ",
//"    For: AllOtherDays,       !- Field 9                                                            ",
//"    Until: 24:00,15.60;      !- Field 10                                                           ",
//"                                                                                                   ",
//"  SCHEDULE:COMPACT,                                                                                ",
//"    ZONE COOLING SETPOINTS,  !- Name                                                               ",
//"    TEMPERATURE,             !- Schedule Type Limits Name                                          ",
//"    Through: 12/31,          !- Field 1                                                            ",
//"    For: Weekdays SummerDesignDay, !- Field 2                                                      ",
//"    Until: 7:00,100.00,      !- Field 3                                                            ",
//"    Until: 17:00,26.10,      !- Field 5                                                            ",
//"    Until: 24:00,100.00,     !- Field 7                                                            ",
//"    For: AllOtherDays,       !- Field 9                                                            ",
//"    Until: 24:00,100.00;     !- Field 10                                                           ",
//"                                                                                                   ",
//"  ZoneControl:Thermostat,                                                                          ",
//"    ZONE 1 CONTROLS,         !- Name                                                               ",
//"    ZONE 1,                  !- Zone or ZoneList Name                                              ",
//"    Zone Control Type Schedule,  !- Control Type Schedule Name                                     ",
//"    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type                                     ",
//"    ZONE 1 SETPOINTS;        !- Control 1 Name                                                     ",
//"                                                                                                   ",
//"  ThermostatSetpoint:DualSetpoint,                                                                 ",
//"    ZONE 1 SETPOINTS,        !- Name                                                               ",
//"    ZONE Heating Setpoints,  !- Heating Setpoint Temperature Schedule Name                         ",
//"    ZONE Cooling Setpoints;  !- Cooling Setpoint Temperature Schedule Name                         ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentConnections,                                                                   ",
//"    ZONE 1,                  !- Zone Name                                                          ",
//"    ZONE 1 EQUIPMENT,        !- Zone Conditioning Equipment List Name                              ",
//"    ZONE 1 INLETS,           !- Zone Air Inlet Node or NodeList Name                               ",
//"    ,                        !- Zone Air Exhaust Node or NodeList Name                             ",
//"    ZONE 1 NODE,             !- Zone Air Node Name                                                 ",
//"    ZONE 1 OUTLET;           !- Zone Return Air Node Name                                          ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentList,                                                                          ",
//"    ZONE 1 EQUIPMENT,        !- Name                                                               ",
//"    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type                                 ",
//"    ZONE 1 Ideal Loads,      !- Zone Equipment 1 Name                                              ",
//"    1,                       !- Zone Equipment 1 Cooling Sequence                                  ",
//"    1;                       !- Zone Equipment 1 Heating or No-Load Sequence                       ",
//"                                                                                                   ",
//"  ZoneHVAC:IdealLoadsAirSystem,                                                                    ",
//"    ZONE 1 Ideal Loads,      !- Name                                                               ",
//"    ,                        !- Availability Schedule Name                                         ",
//"    ZONE 1 INLETS,           !- Zone Supply Air Node Name                                          ",
//"    ,                        !- Zone Exhaust Air Node Name                                         ",
//"    ,                        !- System Inlet Air Node Name                                         ",
//"    50,                      !- Maximum Heating Supply Air Temperature {C}                         ",
//"    13,                      !- Minimum Cooling Supply Air Temperature {C}                         ",
//"    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    NoLimit,                 !- Heating Limit                                                      ",
//"    autosize,                !- Maximum Heating Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Sensible Heating Capacity {W}                              ",
//"    NoLimit,                 !- Cooling Limit                                                      ",
//"    autosize,                !- Maximum Cooling Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Total Cooling Capacity {W}                                 ",
//"    ,                        !- Heating Availability Schedule Name                                 ",
//"    ,                        !- Cooling Availability Schedule Name                                 ",
//"    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type                                 ",
//"    ,                        !- Cooling Sensible Heat Ratio {dimensionless}                        ",
//"    ConstantSupplyHumidityRatio,  !- Humidification Control Type                                   ",
//"    ,                        !- Design Specification Outdoor Air Object Name                       ",
//"    ,                        !- Outdoor Air Inlet Node Name                                        ",
//"    ,                        !- Demand Controlled Ventilation Type                                 ",
//"    ,                        !- Outdoor Air Economizer Type                                        ",
//"    ,                        !- Heat Recovery Type                                                 ",
//"    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}               ",
//"    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}                 ",
//"                                                                                                   ",
//"  NodeList,                                                                                        ",
//"    ZONE 1 INLETS,           !- Name                                                               ",
//"    ZONE 1 INLET;            !- Node 1 Name                                                        ",
//"                                                                                                   ",
//"  ZoneControl:Thermostat,                                                                          ",
//"    ZONE 2 CONTROLS,         !- Name                                                               ",
//"    ZONE 2,                  !- Zone or ZoneList Name                                              ",
//"    Zone Control Type Schedule,  !- Control Type Schedule Name                                     ",
//"    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type                                     ",
//"    ZONE 2 SETPOINTS;        !- Control 1 Name                                                     ",
//"                                                                                                   ",
//"  ThermostatSetpoint:DualSetpoint,                                                                 ",
//"    ZONE 2 SETPOINTS,        !- Name                                                               ",
//"    ZONE Heating Setpoints,  !- Heating Setpoint Temperature Schedule Name                         ",
//"    ZONE Cooling Setpoints;  !- Cooling Setpoint Temperature Schedule Name                         ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentConnections,                                                                   ",
//"    ZONE 2,                  !- Zone Name                                                          ",
//"    ZONE 2 EQUIPMENT,        !- Zone Conditioning Equipment List Name                              ",
//"    ZONE 2 INLETS,           !- Zone Air Inlet Node or NodeList Name                               ",
//"    ,                        !- Zone Air Exhaust Node or NodeList Name                             ",
//"    ZONE 2 NODE,             !- Zone Air Node Name                                                 ",
//"    ZONE 2 OUTLET;           !- Zone Return Air Node Name                                          ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentList,                                                                          ",
//"    ZONE 2 EQUIPMENT,        !- Name                                                               ",
//"    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type                                 ",
//"    ZONE 2 Ideal Loads,      !- Zone Equipment 1 Name                                              ",
//"    1,                       !- Zone Equipment 1 Cooling Sequence                                  ",
//"    1;                       !- Zone Equipment 1 Heating or No-Load Sequence                       ",
//"                                                                                                   ",
//"  ZoneHVAC:IdealLoadsAirSystem,                                                                    ",
//"    ZONE 2 Ideal Loads,      !- Name                                                               ",
//"    ,                        !- Availability Schedule Name                                         ",
//"    ZONE 2 INLETS,           !- Zone Supply Air Node Name                                          ",
//"    ,                        !- Zone Exhaust Air Node Name                                         ",
//"    ,                        !- System Inlet Air Node Name                                         ",
//"    50,                      !- Maximum Heating Supply Air Temperature {C}                         ",
//"    13,                      !- Minimum Cooling Supply Air Temperature {C}                         ",
//"    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    NoLimit,                 !- Heating Limit                                                      ",
//"    autosize,                !- Maximum Heating Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Sensible Heating Capacity {W}                              ",
//"    NoLimit,                 !- Cooling Limit                                                      ",
//"    autosize,                !- Maximum Cooling Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Total Cooling Capacity {W}                                 ",
//"    ,                        !- Heating Availability Schedule Name                                 ",
//"    ,                        !- Cooling Availability Schedule Name                                 ",
//"    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type                                 ",
//"    ,                        !- Cooling Sensible Heat Ratio {dimensionless}                        ",
//"    ConstantSupplyHumidityRatio,  !- Humidification Control Type                                   ",
//"    ,                        !- Design Specification Outdoor Air Object Name                       ",
//"    ,                        !- Outdoor Air Inlet Node Name                                        ",
//"    ,                        !- Demand Controlled Ventilation Type                                 ",
//"    ,                        !- Outdoor Air Economizer Type                                        ",
//"    ,                        !- Heat Recovery Type                                                 ",
//"    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}               ",
//"    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}                 ",
//"                                                                                                   ",
//"  NodeList,                                                                                        ",
//"    ZONE 2 INLETS,           !- Name                                                               ",
//"    ZONE 2 INLET;            !- Node 1 Name                                                        ",
//"                                                                                                   ",
//"  ZoneControl:Thermostat,                                                                          ",
//"    ZONE 3 CONTROLS,         !- Name                                                               ",
//"    ZONE 3,                  !- Zone or ZoneList Name                                              ",
//"    Zone Control Type Schedule,  !- Control Type Schedule Name                                     ",
//"    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type                                     ",
//"    ZONE 3 SETPOINTS;        !- Control 1 Name                                                     ",
//"                                                                                                   ",
//"  ThermostatSetpoint:DualSetpoint,                                                                 ",
//"    ZONE 3 SETPOINTS,        !- Name                                                               ",
//"    ZONE Heating Setpoints,  !- Heating Setpoint Temperature Schedule Name                         ",
//"    ZONE Cooling Setpoints;  !- Cooling Setpoint Temperature Schedule Name                         ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentConnections,                                                                   ",
//"    ZONE 3,                  !- Zone Name                                                          ",
//"    ZONE 3 EQUIPMENT,        !- Zone Conditioning Equipment List Name                              ",
//"    ZONE 3 INLETS,           !- Zone Air Inlet Node or NodeList Name                               ",
//"    ,                        !- Zone Air Exhaust Node or NodeList Name                             ",
//"    ZONE 3 NODE,             !- Zone Air Node Name                                                 ",
//"    ZONE 3 OUTLET;           !- Zone Return Air Node Name                                          ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentList,                                                                          ",
//"    ZONE 3 EQUIPMENT,        !- Name                                                               ",
//"    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type                                 ",
//"    ZONE 3 Ideal Loads,      !- Zone Equipment 1 Name                                              ",
//"    1,                       !- Zone Equipment 1 Cooling Sequence                                  ",
//"    1;                       !- Zone Equipment 1 Heating or No-Load Sequence                       ",
//"                                                                                                   ",
//"  ZoneHVAC:IdealLoadsAirSystem,                                                                    ",
//"    ZONE 3 Ideal Loads,      !- Name                                                               ",
//"    ,                        !- Availability Schedule Name                                         ",
//"    ZONE 3 INLETS,           !- Zone Supply Air Node Name                                          ",
//"    ,                        !- Zone Exhaust Air Node Name                                         ",
//"    ,                        !- System Inlet Air Node Name                                         ",
//"    50,                      !- Maximum Heating Supply Air Temperature {C}                         ",
//"    13,                      !- Minimum Cooling Supply Air Temperature {C}                         ",
//"    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    NoLimit,                 !- Heating Limit                                                      ",
//"    autosize,                !- Maximum Heating Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Sensible Heating Capacity {W}                              ",
//"    NoLimit,                 !- Cooling Limit                                                      ",
//"    autosize,                !- Maximum Cooling Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Total Cooling Capacity {W}                                 ",
//"    ,                        !- Heating Availability Schedule Name                                 ",
//"    ,                        !- Cooling Availability Schedule Name                                 ",
//"    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type                                 ",
//"    ,                        !- Cooling Sensible Heat Ratio {dimensionless}                        ",
//"    ConstantSupplyHumidityRatio,  !- Humidification Control Type                                   ",
//"    ,                        !- Design Specification Outdoor Air Object Name                       ",
//"    ,                        !- Outdoor Air Inlet Node Name                                        ",
//"    ,                        !- Demand Controlled Ventilation Type                                 ",
//"    ,                        !- Outdoor Air Economizer Type                                        ",
//"    ,                        !- Heat Recovery Type                                                 ",
//"    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}               ",
//"    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}                 ",
//"                                                                                                   ",
//"  NodeList,                                                                                        ",
//"    ZONE 3 INLETS,           !- Name                                                               ",
//"    ZONE 3 INLET;            !- Node 1 Name                                                        ",
//"                                                                                                   ",
//"  ZoneControl:Thermostat,                                                                          ",
//"    ZONE 4 CONTROLS,         !- Name                                                               ",
//"    ZONE 4,                  !- Zone or ZoneList Name                                              ",
//"    Zone Control Type Schedule,  !- Control Type Schedule Name                                     ",
//"    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type                                     ",
//"    ZONE 4 SETPOINTS;        !- Control 1 Name                                                     ",
//"                                                                                                   ",
//"  ThermostatSetpoint:DualSetpoint,                                                                 ",
//"    ZONE 4 SETPOINTS,        !- Name                                                               ",
//"    ZONE Heating Setpoints,  !- Heating Setpoint Temperature Schedule Name                         ",
//"    ZONE Cooling Setpoints;  !- Cooling Setpoint Temperature Schedule Name                         ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentConnections,                                                                   ",
//"    ZONE 4,                  !- Zone Name                                                          ",
//"    ZONE 4 EQUIPMENT,        !- Zone Conditioning Equipment List Name                              ",
//"    ZONE 4 INLETS,           !- Zone Air Inlet Node or NodeList Name                               ",
//"    ,                        !- Zone Air Exhaust Node or NodeList Name                             ",
//"    ZONE 4 NODE,             !- Zone Air Node Name                                                 ",
//"    ZONE 4 OUTLET;           !- Zone Return Air Node Name                                          ",
//"                                                                                                   ",
//"  ZoneHVAC:EquipmentList,                                                                          ",
//"    ZONE 4 EQUIPMENT,        !- Name                                                               ",
//"    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type                                 ",
//"    ZONE 4 Ideal Loads,      !- Zone Equipment 1 Name                                              ",
//"    1,                       !- Zone Equipment 1 Cooling Sequence                                  ",
//"    1;                       !- Zone Equipment 1 Heating or No-Load Sequence                       ",
//"                                                                                                   ",
//"  ZoneHVAC:IdealLoadsAirSystem,                                                                    ",
//"    ZONE 4 Ideal Loads,      !- Name                                                               ",
//"    ,                        !- Availability Schedule Name                                         ",
//"    ZONE 4 INLETS,           !- Zone Supply Air Node Name                                          ",
//"    ,                        !- Zone Exhaust Air Node Name                                         ",
//"    ,                        !- System Inlet Air Node Name                                         ",
//"    50,                      !- Maximum Heating Supply Air Temperature {C}                         ",
//"    13,                      !- Minimum Cooling Supply Air Temperature {C}                         ",
//"    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}       ",
//"    NoLimit,                 !- Heating Limit                                                      ",
//"    autosize,                !- Maximum Heating Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Sensible Heating Capacity {W}                              ",
//"    NoLimit,                 !- Cooling Limit                                                      ",
//"    autosize,                !- Maximum Cooling Air Flow Rate {m3/s}                               ",
//"    ,                        !- Maximum Total Cooling Capacity {W}                                 ",
//"    ,                        !- Heating Availability Schedule Name                                 ",
//"    ,                        !- Cooling Availability Schedule Name                                 ",
//"    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type                                 ",
//"    ,                        !- Cooling Sensible Heat Ratio {dimensionless}                        ",
//"    ConstantSupplyHumidityRatio,  !- Humidification Control Type                                   ",
//"    ,                        !- Design Specification Outdoor Air Object Name                       ",
//"    ,                        !- Outdoor Air Inlet Node Name                                        ",
//"    ,                        !- Demand Controlled Ventilation Type                                 ",
//"    ,                        !- Outdoor Air Economizer Type                                        ",
//"    ,                        !- Heat Recovery Type                                                 ",
//"    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}               ",
//"    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}                 ",
//"                                                                                                   ",
//"  NodeList,                                                                                        ",
//"    ZONE 4 INLETS,           !- Name                                                               ",
//"    ZONE 4 INLET;            !- Node 1 Name                                                        ",
//"                                                                                                   ",
//"OutputControl:Table:Style,                                                                         ",
//"    HTML,                    !- Column Separator                                                   ",
//"    JtoKWH;                  !- Unit Conversion                                                    ",
//"                                                                                                   ",
//"Output:Table:SummaryReports,                                                                       ",
//"    AllSummary;              !- Report 1 Name                                                      ",
//"                                                                                                   "
//} );

// ASSERT_TRUE( process_idf( idf_objects ) );

// OutputProcessor::TimeValue.allocate(2);
////state->dataGlobal->DDOnlySimulation = true;

// ManageSimulation();
////    compare_err_stream( "" );

// EXPECT_EQ( "3", RetrievePreDefTableEntry(*state,  pdchSurfCntTot, "Overhang" ) );
// EXPECT_EQ( "3", RetrievePreDefTableEntry(*state,  pdchSurfCntExt, "Overhang" ) );

// EXPECT_EQ( "1", RetrievePreDefTableEntry(*state,  pdchSurfCntTot, "Fin" ) );
// EXPECT_EQ( "1", RetrievePreDefTableEntry(*state,  pdchSurfCntExt, "Fin" ) );

//}

// TEST_F( EnergyPlusFixture, TubularDaylightDiffuserCount )
//{
//// based on DaylightingDeviceTubular.idf
// std::string const idf_objects = delimited_string( {
//"                                                                                          ",
//"  Building,                                                                               ",
//"    Tubular Daylighting Device Example,  !- Name                                          ",
//"    0.0,                     !- North Axis {deg}                                          ",
//"    Suburbs,                 !- Terrain                                                   ",
//"    0.04,                    !- Loads Convergence Tolerance Value                         ",
//"    0.04,                    !- Temperature Convergence Tolerance Value {deltaC}          ",
//"    FullInteriorAndExterior, !- Solar Distribution                                        ",
//"    25,                      !- Maximum Number of Warmup Days                             ",
//"    6;                       !- Minimum Number of Warmup Days                             ",
//"                                                                                          ",
//"  Timestep,6;                                                                             ",
//"                                                                                          ",
//" ShadowCalculation,",
//"    PolygonClipping,         !- Shading Calculation Method",
//"    Periodic,                !- Shading Calculation Update Frequency Method",
//"    20,                      !- Shading Calculation Update Frequency",
//"    15000;                   !- Maximum Figures in Shadow Overlap Calculations",
//"                                                                                          ",
//"  HeatBalanceAlgorithm,ConductionTransferFunction;                                        ",
//"                                                                                          ",
//"  SurfaceConvectionAlgorithm:Inside,TARP;                                                 ",
//"                                                                                          ",
//"  SurfaceConvectionAlgorithm:Outside,DOE-2;                                               ",
//"                                                                                          ",
//"  GlobalGeometryRules,                                                                    ",
//"    UpperLeftCorner,         !- Starting Vertex Position                                  ",
//"    CounterClockWise,        !- Vertex Entry Direction                                    ",
//"    Relative;                !- Coordinate System                                         ",
//"                                                                                          ",
//"  Site:Location,                                                                          ",
//"    CHICAGO_IL_USA_WMO_725300,  !- Name                                                   ",
//"    42.00,                   !- Latitude {deg}                                            ",
//"    -87.88,                  !- Longitude {deg}                                           ",
//"    -6.00,                   !- Time Zone {hr}                                            ",
//"    190.00;                  !- Elevation {m}                                             ",
//"                                                                                          ",
//"  SizingPeriod:DesignDay,                                                                 ",
//"    CHICAGO Ann Htg 99% Condns DB,  !- Name                                               ",
//"    1,                       !- Month                                                     ",
//"    21,                      !- Day of Month                                              ",
//"    WinterDesignDay,         !- Day Type                                                  ",
//"    -17.3,                   !- Maximum Dry-Bulb Temperature {C}                          ",
//"    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}                 ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Type                  ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name     ",
//"    Wetbulb,                 !- Humidity Condition Type                                   ",
//"    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}               ",
//"    ,                        !- Humidity Condition Day Schedule Name                      ",
//"    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}     ",
//"    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}                       ",
//"    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}                 ",
//"    99063.,                  !- Barometric Pressure {Pa}                                  ",
//"    4.9,                     !- Wind Speed {m/s}                                          ",
//"    270,                     !- Wind Direction {deg}                                      ",
//"    No,                      !- Rain Indicator                                            ",
//"    No,                      !- Snow Indicator                                            ",
//"    No,                      !- Daylight Saving Time Indicator                            ",
//"    ASHRAEClearSky,          !- Solar Model Indicator                                     ",
//"    ,                        !- Beam Solar Day Schedule Name                              ",
//"    ,                        !- Diffuse Solar Day Schedule Name                           ",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) ",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (tau",
//"    0.00;                    !- Sky Clearness                                             ",
//"                                                                                          ",
//"  SizingPeriod:DesignDay,                                                                 ",
//"    CHICAGO Ann Clg 1% Condns DB=>MWB,  !- Name                                           ",
//"    7,                       !- Month                                                     ",
//"    21,                      !- Day of Month                                              ",
//"    SummerDesignDay,         !- Day Type                                                  ",
//"    31.5,                    !- Maximum Dry-Bulb Temperature {C}                          ",
//"    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}                 ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Type                  ",
//"    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name     ",
//"    Wetbulb,                 !- Humidity Condition Type                                   ",
//"    23,                      !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}               ",
//"    ,                        !- Humidity Condition Day Schedule Name                      ",
//"    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}     ",
//"    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}                       ",
//"    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}                 ",
//"    99063.,                  !- Barometric Pressure {Pa}                                  ",
//"    5.3,                     !- Wind Speed {m/s}                                          ",
//"    230,                     !- Wind Direction {deg}                                      ",
//"    No,                      !- Rain Indicator                                            ",
//"    No,                      !- Snow Indicator                                            ",
//"    No,                      !- Daylight Saving Time Indicator                            ",
//"    ASHRAEClearSky,          !- Solar Model Indicator                                     ",
//"    ,                        !- Beam Solar Day Schedule Name                              ",
//"    ,                        !- Diffuse Solar Day Schedule Name                           ",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) ",
//"    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (tau",
//"    1.00;                    !- Sky Clearness                                             ",
//"                                                                                          ",
//"  SimulationControl,                                                                      ",
//"    NO,                      !- Do Zone Sizing Calculation                                ",
//"    NO,                      !- Do System Sizing Calculation                              ",
//"    NO,                      !- Do Plant Sizing Calculation                               ",
//"    YES,                     !- Run Simulation for Sizing Periods                         ",
//"    NO;                      !- Run Simulation for Weather File Run Periods               ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    A1 - 1 IN STUCCO,        !- Name                                                      ",
//"    Smooth,                  !- Roughness                                                 ",
//"    2.5389841E-02,           !- Thickness {m}                                             ",
//"    0.6918309,               !- Conductivity {W/m-K}                                      ",
//"    1858.142,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.92,                    !- Solar Absorptance                                         ",
//"    0.92;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    C4 - 4 IN COMMON BRICK,  !- Name                                                      ",
//"    Rough,                   !- Roughness                                                 ",
//"    0.1014984,               !- Thickness {m}                                             ",
//"    0.7264224,               !- Conductivity {W/m-K}                                      ",
//"    1922.216,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.76,                    !- Solar Absorptance                                         ",
//"    0.76;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name                                          ",
//"    Smooth,                  !- Roughness                                                 ",
//"    1.9050000E-02,           !- Thickness {m}                                             ",
//"    0.7264224,               !- Conductivity {W/m-K}                                      ",
//"    1601.846,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.92,                    !- Solar Absorptance                                         ",
//"    0.92;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    C6 - 8 IN CLAY TILE,     !- Name                                                      ",
//"    Smooth,                  !- Roughness                                                 ",
//"    0.2033016,               !- Thickness {m}                                             ",
//"    0.5707605,               !- Conductivity {W/m-K}                                      ",
//"    1121.292,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.82,                    !- Solar Absorptance                                         ",
//"    0.82;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    C10 - 8 IN HW CONCRETE,  !- Name                                                      ",
//"    MediumRough,             !- Roughness                                                 ",
//"    0.2033016,               !- Thickness {m}                                             ",
//"    1.729577,                !- Conductivity {W/m-K}                                      ",
//"    2242.585,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.65,                    !- Solar Absorptance                                         ",
//"    0.65;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    E2 - 1 / 2 IN SLAG OR STONE,  !- Name                                                 ",
//"    Rough,                   !- Roughness                                                 ",
//"    1.2710161E-02,           !- Thickness {m}                                             ",
//"    1.435549,                !- Conductivity {W/m-K}                                      ",
//"    881.0155,                !- Density {kg/m3}                                           ",
//"    1673.6,                  !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.55,                    !- Solar Absorptance                                         ",
//"    0.55;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Name                                             ",
//"    Rough,                   !- Roughness                                                 ",
//"    9.5402403E-03,           !- Thickness {m}                                             ",
//"    0.1902535,               !- Conductivity {W/m-K}                                      ",
//"    1121.292,                !- Density {kg/m3}                                           ",
//"    1673.6,                  !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.75,                    !- Solar Absorptance                                         ",
//"    0.75;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    B5 - 1 IN DENSE INSULATION,  !- Name                                                  ",
//"    VeryRough,               !- Roughness                                                 ",
//"    2.5389841E-02,           !- Thickness {m}                                             ",
//"    4.3239430E-02,           !- Conductivity {W/m-K}                                      ",
//"    91.30524,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.50,                    !- Solar Absorptance                                         ",
//"    0.50;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    C12 - 2 IN HW CONCRETE,  !- Name                                                      ",
//"    MediumRough,             !- Roughness                                                 ",
//"    5.0901599E-02,           !- Thickness {m}                                             ",
//"    1.729577,                !- Conductivity {W/m-K}                                      ",
//"    2242.585,                !- Density {kg/m3}                                           ",
//"    836.8,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.65,                    !- Solar Absorptance                                         ",
//"    0.65;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    ROOFING - ASPHALT SHINGLES,  !- Name                                                  ",
//"    VeryRough,               !- Roughness                                                 ",
//"    3.1999999E-03,           !- Thickness {m}                                             ",
//"    2.9999999E-02,           !- Conductivity {W/m-K}                                      ",
//"    1121.29,                 !- Density {kg/m3}                                           ",
//"    830.0,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.70,                    !- Solar Absorptance                                         ",
//"    0.70;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    BB46 - 5 / 8 IN PLYWOOD, !- Name                                                      ",
//"    Smooth,                  !- Roughness                                                 ",
//"    9.9999998E-03,           !- Thickness {m}                                             ",
//"    0.110,                   !- Conductivity {W/m-K}                                      ",
//"    544.62,                  !- Density {kg/m3}                                           ",
//"    1210.0,                  !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.70,                    !- Solar Absorptance                                         ",
//"    0.70;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    INS - GLASS FIBER BONDED 3 IN,  !- Name                                               ",
//"    VeryRough,               !- Roughness                                                 ",
//"    7.000E-02,               !- Thickness {m}                                             ",
//"    2.9999999E-02,           !- Conductivity {W/m-K}                                      ",
//"    96.11,                   !- Density {kg/m3}                                           ",
//"    790.0,                   !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.50,                    !- Solar Absorptance                                         ",
//"    0.50;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  WindowMaterial:Glazing,                                                                 ",
//"    Clear Acrylic Plastic,   !- Name                                                      ",
//"    SpectralAverage,         !- Optical Data Type                                         ",
//"    ,                        !- Window Glass Spectral Data Set Name                       ",
//"    0.003,                   !- Thickness {m}                                             ",
//"    0.92,                    !- Solar Transmittance at Normal Incidence                   ",
//"    0.05,                    !- Front Side Solar Reflectance at Normal Incidence          ",
//"    0.05,                    !- Back Side Solar Reflectance at Normal Incidence           ",
//"    0.92,                    !- Visible Transmittance at Normal Incidence                 ",
//"    0.05,                    !- Front Side Visible Reflectance at Normal Incidence        ",
//"    0.05,                    !- Back Side Visible Reflectance at Normal Incidence         ",
//"    0.00,                    !- Infrared Transmittance at Normal Incidence                ",
//"    0.90,                    !- Front Side Infrared Hemispherical Emissivity              ",
//"    0.90,                    !- Back Side Infrared Hemispherical Emissivity               ",
//"    0.90;                    !- Conductivity {W/m-K}                                      ",
//"                                                                                          ",
//"  WindowMaterial:Glazing,                                                                 ",
//"    Diffusing Acrylic Plastic,  !- Name                                                   ",
//"    SpectralAverage,         !- Optical Data Type                                         ",
//"    ,                        !- Window Glass Spectral Data Set Name                       ",
//"    0.0022,                  !- Thickness {m}                                             ",
//"    0.90,                    !- Solar Transmittance at Normal Incidence                   ",
//"    0.08,                    !- Front Side Solar Reflectance at Normal Incidence          ",
//"    0.08,                    !- Back Side Solar Reflectance at Normal Incidence           ",
//"    0.90,                    !- Visible Transmittance at Normal Incidence                 ",
//"    0.08,                    !- Front Side Visible Reflectance at Normal Incidence        ",
//"    0.08,                    !- Back Side Visible Reflectance at Normal Incidence         ",
//"    0.00,                    !- Infrared Transmittance at Normal Incidence                ",
//"    0.90,                    !- Front Side Infrared Hemispherical Emissivity              ",
//"    0.90,                    !- Back Side Infrared Hemispherical Emissivity               ",
//"    0.90;                    !- Conductivity {W/m-K}                                      ",
//"                                                                                          ",
//"  Material,                                                                               ",
//"    Very High Reflectivity Surface,  !- Name                                              ",
//"    Smooth,                  !- Roughness                                                 ",
//"    0.0005,                  !- Thickness {m}                                             ",
//"    237,                     !- Conductivity {W/m-K}                                      ",
//"    2702,                    !- Density {kg/m3}                                           ",
//"    903,                     !- Specific Heat {J/kg-K}                                    ",
//"    0.90,                    !- Thermal Absorptance                                       ",
//"    0.05,                    !- Solar Absorptance                                         ",
//"    0.05;                    !- Visible Absorptance                                       ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    EXTWALL80,               !- Name                                                      ",
//"    A1 - 1 IN STUCCO,        !- Outside Layer                                             ",
//"    C4 - 4 IN COMMON BRICK,  !- Layer 2                                                   ",
//"    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3                                       ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    FLOOR SLAB 8 IN,         !- Name                                                      ",
//"    C10 - 8 IN HW CONCRETE;  !- Outside Layer                                             ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    ROOF,                    !- Name                                                      ",
//"    ROOFING - ASPHALT SHINGLES,  !- Outside Layer                                         ",
//"    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Layer 2                                          ",
//"    BB46 - 5 / 8 IN PLYWOOD; !- Layer 3                                                   ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    CEILING IN ZONE,         !- Name                                                      ",
//"    INS - GLASS FIBER BONDED 3 IN,  !- Outside Layer                                      ",
//"    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 2                                       ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    CEILING IN ATTIC,        !- Name                                                      ",
//"    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer                                 ",
//"    INS - GLASS FIBER BONDED 3 IN;  !- Layer 2                                            ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    TDD Pipe,                !- Name                                                      ",
//"    Very High Reflectivity Surface;  !- Outside Layer                                     ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    TDD Dome,                !- Name                                                      ",
//"    Clear Acrylic Plastic;   !- Outside Layer                                             ",
//"                                                                                          ",
//"  Construction,                                                                           ",
//"    TDD Diffuser,            !- Name                                                      ",
//"    Diffusing Acrylic Plastic;  !- Outside Layer                                          ",
//"                                                                                          ",
//"  Zone,                                                                                   ",
//"    Daylit Zone,             !- Name                                                      ",
//"    0.0,                     !- Direction of Relative North {deg}                         ",
//"    0.0,                     !- X Origin {m}                                              ",
//"    0.0,                     !- Y Origin {m}                                              ",
//"    0.0,                     !- Z Origin {m}                                              ",
//"    1,                       !- Type                                                      ",
//"    1,                       !- Multiplier                                                ",
//"    autocalculate,           !- Ceiling Height {m}                                        ",
//"    autocalculate;           !- Volume {m3}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit South Wall,       !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit West Wall,        !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    0.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit North Wall,       !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    0.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit East Wall,        !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    5.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Floor,            !- Name                                                      ",
//"    Floor,                   !- Surface Type                                              ",
//"    FLOOR SLAB 8 IN,         !- Construction Name                                         ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Surface,                 !- Outside Boundary Condition                                ",
//"    Daylit Floor,            !- Outside Boundary Condition Object                         ",
//"    NoSun,                   !- Sun Exposure                                              ",
//"    NoWind,                  !- Wind Exposure                                             ",
//"    1.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,0.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Ceiling,          !- Name                                                      ",
//"    Roof,                    !- Surface Type                                              ",
//"    CEILING IN ZONE,         !- Construction Name                                         ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Surface,                 !- Outside Boundary Condition                                ",
//"    Daylit Attic Floor,      !- Outside Boundary Condition Object                         ",
//"    NoSun,                   !- Sun Exposure                                              ",
//"    NoWind,                  !- Wind Exposure                                             ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  Zone,                                                                                   ",
//"    Daylit Attic Zone,       !- Name                                                      ",
//"    0.0,                     !- Direction of Relative North {deg}                         ",
//"    0.0,                     !- X Origin {m}                                              ",
//"    0.0,                     !- Y Origin {m}                                              ",
//"    0.0,                     !- Z Origin {m}                                              ",
//"    1,                       !- Type                                                      ",
//"    1,                       !- Multiplier                                                ",
//"    autocalculate,           !- Ceiling Height {m}                                        ",
//"    autocalculate;           !- Volume {m3}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Attic South Wall, !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Attic Zone,       !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Attic West Wall,  !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Attic Zone,       !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Attic North Wall, !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Attic Zone,       !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    0.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Attic East Wall,  !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Daylit Attic Zone,       !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Attic Floor,      !- Name                                                      ",
//"    Floor,                   !- Surface Type                                              ",
//"    CEILING IN ATTIC,        !- Construction Name                                         ",
//"    Daylit Attic Zone,       !- Zone Name                                                 ",
//"    Surface,                 !- Outside Boundary Condition                                ",
//"    Daylit Ceiling,          !- Outside Boundary Condition Object                         ",
//"    NoSun,                   !- Sun Exposure                                              ",
//"    NoWind,                  !- Wind Exposure                                             ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Daylit Attic Roof,       !- Name                                                      ",
//"    Roof,                    !- Surface Type                                              ",
//"    ROOF,                    !- Construction Name                                         ",
//"    Daylit Attic Zone,       !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  Zone,                                                                                   ",
//"    Standard Zone,           !- Name                                                      ",
//"    0.0,                     !- Direction of Relative North {deg}                         ",
//"    50.0,                    !- X Origin {m}                                              ",
//"    50.0,                    !- Y Origin {m}                                              ",
//"    0.0,                     !- Z Origin {m}                                              ",
//"    1,                       !- Type                                                      ",
//"    1,                       !- Multiplier                                                ",
//"    autocalculate,           !- Ceiling Height {m}                                        ",
//"    autocalculate;           !- Volume {m3}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard South Wall,     !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard West Wall,      !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    0.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard North Wall,     !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    0.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard East Wall,      !- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    5.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Floor,          !- Name                                                      ",
//"    Floor,                   !- Surface Type                                              ",
//"    FLOOR SLAB 8 IN,         !- Construction Name                                         ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Surface,                 !- Outside Boundary Condition                                ",
//"    Standard Floor,          !- Outside Boundary Condition Object                         ",
//"    NoSun,                   !- Sun Exposure                                              ",
//"    NoWind,                  !- Wind Exposure                                             ",
//"    1.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,0.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Ceiling,        !- Name                                                      ",
//"    Roof,                    !- Surface Type                                              ",
//"    CEILING IN ZONE,         !- Construction Name                                         ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Surface,                 !- Outside Boundary Condition                                ",
//"    Standard Attic Floor,    !- Outside Boundary Condition Object                         ",
//"    NoSun,                   !- Sun Exposure                                              ",
//"    NoWind,                  !- Wind Exposure                                             ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  Zone,                                                                                   ",
//"    Standard Attic Zone,     !- Name                                                      ",
//"    0.0,                     !- Direction of Relative North {deg}                         ",
//"    50.0,                    !- X Origin {m}                                              ",
//"    50.0,                    !- Y Origin {m}                                              ",
//"    0.0,                     !- Z Origin {m}                                              ",
//"    1,                       !- Type                                                      ",
//"    1,                       !- Multiplier                                                ",
//"    autocalculate,           !- Ceiling Height {m}                                        ",
//"    autocalculate;           !- Volume {m3}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Attic South Wall,  !- Name                                                   ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Attic Zone,     !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Attic West Wall,!- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Attic Zone,     !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Attic North Wall,  !- Name                                                   ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Attic Zone,     !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    0.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Attic East Wall,!- Name                                                      ",
//"    Wall,                    !- Surface Type                                              ",
//"    EXTWALL80,               !- Construction Name                                         ",
//"    Standard Attic Zone,     !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.5,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    5.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Attic Floor,    !- Name                                                      ",
//"    Floor,                   !- Surface Type                                              ",
//"    CEILING IN ATTIC,        !- Construction Name                                         ",
//"    Standard Attic Zone,     !- Zone Name                                                 ",
//"    Surface,                 !- Outside Boundary Condition                                ",
//"    Standard Ceiling,        !- Outside Boundary Condition Object                         ",
//"    NoSun,                   !- Sun Exposure                                              ",
//"    NoWind,                  !- Wind Exposure                                             ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                               ",
//"    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                              ",
//"    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                              ",
//"    5.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                               ",
//"                                                                                          ",
//"  BuildingSurface:Detailed,                                                               ",
//"    Standard Attic Roof,     !- Name                                                      ",
//"    Roof,                    !- Surface Type                                              ",
//"    ROOF,                    !- Construction Name                                         ",
//"    Standard Attic Zone,     !- Zone Name                                                 ",
//"    Outdoors,                !- Outside Boundary Condition                                ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    SunExposed,              !- Sun Exposure                                              ",
//"    WindExposed,             !- Wind Exposure                                             ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    4,                       !- Number of Vertices                                        ",
//"    0.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}                                              ",
//"    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}                                               ",
//"    5.0,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}                                               ",
//"    5.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}                                              ",
//"                                                                                          ",
//"  Lights,                                                                                 ",
//"    Daylit Zone Lights 1,    !- Name                                                      ",
//"    Daylit Zone,             !- Zone or ZoneList Name                                     ",
//"    AlwaysOnSchedule,        !- Schedule Name                                             ",
//"    LightingLevel,           !- Design Level Calculation Method                           ",
//"    800,                     !- Lighting Level {W}                                        ",
//"    ,                        !- Watts per Zone Floor Area {W/m2}                          ",
//"    ,                        !- Watts per Person {W/person}                               ",
//"    0,                       !- Return Air Fraction                                       ",
//"    0.40,                    !- Fraction Radiant                                          ",
//"    0.20,                    !- Fraction Visible                                          ",
//"    1.0,                     !- Fraction Replaceable                                      ",
//"    GeneralLights;           !- End-Use Subcategory                                       ",
//"                                                                                          ",
//"  Lights,                                                                                 ",
//"    Standard Zone Lights 1,  !- Name                                                      ",
//"    Standard Zone,           !- Zone or ZoneList Name                                     ",
//"    AlwaysOnSchedule,        !- Schedule Name                                             ",
//"    LightingLevel,           !- Design Level Calculation Method                           ",
//"    800,                     !- Lighting Level {W}                                        ",
//"    ,                        !- Watts per Zone Floor Area {W/m2}                          ",
//"    ,                        !- Watts per Person {W/person}                               ",
//"    0,                       !- Return Air Fraction                                       ",
//"    0.40,                    !- Fraction Radiant                                          ",
//"    0.20,                    !- Fraction Visible                                          ",
//"    1.0,                     !- Fraction Replaceable                                      ",
//"    GeneralLights;           !- End-Use Subcategory                                       ",
//"                                                                                          ",
//"  DaylightingDevice:Tubular,                                                              ",
//"    Pipe1,                   !- Name                                                      ",
//"    Dome1,                   !- Dome Name                                                 ",
//"    Diffuser1,               !- Diffuser Name                                             ",
//"    TDD Pipe,                !- Construction Name                                         ",
//"    0.3556,                  !- Diameter {m}                                              ",
//"    1.4,                     !- Total Length {m}                                          ",
//"    0.28,                    !- Effective Thermal Resistance {m2-K/W}                     ",
//"    Daylit Attic Zone,       !- Transition Zone 1 Name                                    ",
//"    1.1;                     !- Transition Zone 1 Length {m}                              ",
//"                                                                                          ",
//"  FenestrationSurface:Detailed,                                                           ",
//"    Dome1,                   !- Name                                                      ",
//"    TubularDaylightDome,     !- Surface Type                                              ",
//"    TDD Dome,                !- Construction Name                                         ",
//"    Daylit Attic Roof,       !- Building Surface Name                                     ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    ,                        !- Frame and Divider Name                                    ",
//"    1.0,                     !- Multiplier                                                ",
//"    4,                       !- Number of Vertices                                        ",
//"    2.3425,3.209,3.64,  !- X,Y,Z ==> Vertex 1 {m}                                         ",
//"    2.3425,2.906,3.58,  !- X,Y,Z ==> Vertex 2 {m}                                         ",
//"    2.6575,2.906,3.58,  !- X,Y,Z ==> Vertex 3 {m}                                         ",
//"    2.6575,3.209,3.64;  !- X,Y,Z ==> Vertex 4 {m}                                         ",
//"                                                                                          ",
//"  FenestrationSurface:Detailed,                                                           ",
//"    Diffuser1,               !- Name                                                      ",
//"    TubularDaylightDiffuser, !- Surface Type                                              ",
//"    TDD Diffuser,            !- Construction Name                                         ",
//"    Daylit Ceiling,          !- Building Surface Name                                     ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    ,                        !- Frame and Divider Name                                    ",
//"    1.0,                     !- Multiplier                                                ",
//"    4,                       !- Number of Vertices                                        ",
//"    2.3425,3.1575,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                         ",
//"    2.3425,2.8425,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                         ",
//"    2.6575,2.8425,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                         ",
//"    2.6575,3.1575,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                         ",
//"                                                                                          ",
//"  DaylightingDevice:Tubular,                                                              ",
//"    Pipe2,                   !- Name                                                      ",
//"    Dome2,                   !- Dome Name                                                 ",
//"    Diffuser2,               !- Diffuser Name                                             ",
//"    TDD Pipe,                !- Construction Name                                         ",
//"    0.3556,                  !- Diameter {m}                                              ",
//"    2.2,                     !- Total Length {m}                                          ",
//"    0.28,                    !- Effective Thermal Resistance {m2-K/W}                     ",
//"    Daylit Attic Zone,       !- Transition Zone 1 Name                                    ",
//"    1.9;                     !- Transition Zone 1 Length {m}                              ",
//"                                                                                          ",
//"  FenestrationSurface:Detailed,                                                           ",
//"    Dome2,                   !- Name                                                      ",
//"    TubularDaylightDome,     !- Surface Type                                              ",
//"    TDD Dome,                !- Construction Name                                         ",
//"    Daylit Attic Roof,       !- Building Surface Name                                     ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    ,                        !- Frame and Divider Name                                    ",
//"    1.0,                     !- Multiplier                                                ",
//"    4,                       !- Number of Vertices                                        ",
//"    2.3425,7.209134615385,4.441826923077,  !- X,Y,Z ==> Vertex 1 {m}                      ",
//"    2.3425,6.90625,4.38125,  !- X,Y,Z ==> Vertex 2 {m}                                    ",
//"    2.6575,6.90625,4.38125,  !- X,Y,Z ==> Vertex 3 {m}                                    ",
//"    2.6575,7.209134615385,4.441826923077;  !- X,Y,Z ==> Vertex 4 {m}                      ",
//"                                                                                          ",
//"  FenestrationSurface:Detailed,                                                           ",
//"    Diffuser2,               !- Name                                                      ",
//"    TubularDaylightDiffuser, !- Surface Type                                              ",
//"    TDD Diffuser,            !- Construction Name                                         ",
//"    Daylit Ceiling,          !- Building Surface Name                                     ",
//"    ,                        !- Outside Boundary Condition Object                         ",
//"    0.0,                     !- View Factor to Ground                                     ",
//"    ,                        !- Frame and Divider Name                                    ",
//"    1.0,                     !- Multiplier                                                ",
//"    4,                       !- Number of Vertices                                        ",
//"    2.3425,7.1575,2.5,  !- X,Y,Z ==> Vertex 1 {m}                                         ",
//"    2.3425,6.8425,2.5,  !- X,Y,Z ==> Vertex 2 {m}                                         ",
//"    2.6575,6.8425,2.5,  !- X,Y,Z ==> Vertex 3 {m}                                         ",
//"    2.6575,7.1575,2.5;  !- X,Y,Z ==> Vertex 4 {m}                                         ",
//"                                                                                          ",
//"  Daylighting:Controls,                                                                                                  ",
//"    Daylit Zone_DaylCtrl,    !- Name                                                                                     ",
//"    Daylit Zone,             !- Zone Name                                                                                ",
//"    SplitFlux,               !- Daylighting Method                                                                       ",
//"    ,                        !- Availability Schedule Name                                                               ",
//"    Continuous,              !- Lighting Control Type                                                                    ",
//"    0.3,                     !- Minimum Input Power Fraction for Continuous or ContinuousOff Dimming Control             ",
//"    0.2,                     !- Minimum Light Output Fraction for Continuous or ContinuousOff Dimming Control            ",
//"    ,                        !- Number of Stepped Control Steps                                                          ",
//"    1.0,                     !- Probability Lighting will be Reset When Needed in Manual Stepped Control                 ",
//"    Daylit Zone_DaylRefPt1,  !- Glare Calculation Daylighting Reference Point Name                                       ",
//"    0.0,                     !- Glare Calculation Azimuth Angle of View Direction Clockwise from Zone y-Axis {deg}       ",
//"    20.0,                    !- Maximum Allowable Discomfort Glare Index                                                 ",
//"    ,                        !- DElight Gridding Resolution {m2}                                                         ",
//"    Daylit Zone_DaylRefPt1,  !- Daylighting Reference Point 1 Name                                                       ",
//"    1.0,                     !- Fraction of Zone Controlled by Reference Point 1                                         ",
//"    550,                     !- Illuminance Setpoint at Reference Point 1 {lux}                                          ",
//"    Daylit Zone_DaylRefPt2,  !- Daylighting Reference Point 2 Name                                                       ",
//"    0.0,                     !- Fraction of Zone Controlled by Reference Point 2                                         ",
//"    0;                       !- Illuminance Setpoint at Reference Point 2 {lux}                                          ",
//"                                                                                                                         ",
//"  Daylighting:ReferencePoint,                                                                                            ",
//"    Daylit Zone_DaylRefPt1,  !- Name                                                                                     ",
//"    Daylit Zone,             !- Zone Name                                                                                ",
//"    2.5,                     !- X-Coordinate of Reference Point {m}                                                      ",
//"    5.0,                     !- Y-Coordinate of Reference Point {m}                                                      ",
//"    0.8;                     !- Z-Coordinate of Reference Point {m}                                                      ",
//"                                                                                                                         ",
//"  Daylighting:ReferencePoint,                                                                                            ",
//"    Daylit Zone_DaylRefPt2,  !- Name                                                                                     ",
//"    Daylit Zone,             !- Zone Name                                                                                ",
//"    2.5,                     !- X-Coordinate of Reference Point {m}                                                      ",
//"    3.0,                     !- Y-Coordinate of Reference Point {m}                                                      ",
//"    0.8;                     !- Z-Coordinate of Reference Point {m}                                                      ",
//"                                                                                          ",
//"  OutputControl:IlluminanceMap:Style,                                                     ",
//"    Comma;                   !- Column Separator                                          ",
//"                                                                                          ",
//"  Output:IlluminanceMap,                                                                  ",
//"    Daylit Map,              !- Name                                                      ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    0.8,                     !- Z height {m}                                              ",
//"    0.1,                     !- X Minimum Coordinate {m}                                  ",
//"    4.9,                     !- X Maximum Coordinate {m}                                  ",
//"    10,                      !- Number of X Grid Points                                   ",
//"    0.1,                     !- Y Minimum Coordinate {m}                                  ",
//"    9.9,                     !- Y Maximum Coordinate {m}                                  ",
//"    10;                      !- Number of Y Grid Points                                   ",
//"                                                                                          ",
//"  ScheduleTypeLimits,                                                                     ",
//"    On/Off,                  !- Name                                                      ",
//"    0,                       !- Lower Limit Value                                         ",
//"    1,                       !- Upper Limit Value                                         ",
//"    DISCRETE;                !- Numeric Type                                              ",
//"                                                                                          ",
//"  Schedule:Compact,                                                                       ",
//"    AlwaysOnSchedule,        !- Name                                                      ",
//"    On/Off,                  !- Schedule Type Limits Name                                 ",
//"    THROUGH: 12/31,          !- Field 1                                                   ",
//"    FOR: AllDays,            !- Field 2                                                   ",
//"    UNTIL: 24:00,1;          !- Field 3                                                   ",
//"                                                                                          ",
//"  ScheduleTypeLimits,                                                                     ",
//"    Control Type,            !- Name                                                      ",
//"    0,                       !- Lower Limit Value                                         ",
//"    4,                       !- Upper Limit Value                                         ",
//"    DISCRETE;                !- Numeric Type                                              ",
//"                                                                                          ",
//"  Schedule:Compact,                                                                       ",
//"    Zone Control Type Sch,   !- Name                                                      ",
//"    Control Type,            !- Schedule Type Limits Name                                 ",
//"    THROUGH: 12/31,          !- Field 1                                                   ",
//"    FOR: AllDays,            !- Field 2                                                   ",
//"    UNTIL: 24:00,3;          !- Field 3                                                   ",
//"                                                                                          ",
//"  ScheduleTypeLimits,                                                                     ",
//"    Temperature,             !- Name                                                      ",
//"    -60,                     !- Lower Limit Value                                         ",
//"    200,                     !- Upper Limit Value                                         ",
//"    CONTINUOUS,              !- Numeric Type                                              ",
//"    Temperature;             !- Unit Type                                                 ",
//"                                                                                          ",
//"  Schedule:Compact,                                                                       ",
//"    Zone Temp Sch,           !- Name                                                      ",
//"    Temperature,             !- Schedule Type Limits Name                                 ",
//"    THROUGH: 12/31,          !- Field 1                                                   ",
//"    FOR: AllDays,            !- Field 2                                                   ",
//"    UNTIL: 24:00,21;         !- Field 3                                                   ",
//"                                                                                          ",
//"  NodeList,                                                                               ",
//"    Daylit Zone Inlets,      !- Name                                                      ",
//"    Daylit Zone Supply Node; !- Node 1 Name                                               ",
//"                                                                                          ",
//"  ZoneHVAC:EquipmentConnections,                                                          ",
//"    Daylit Zone,             !- Zone Name                                                 ",
//"    Daylit Zone Equipment,   !- Zone Conditioning Equipment List Name                     ",
//"    Daylit Zone Inlets,      !- Zone Air Inlet Node or NodeList Name                      ",
//"    ,                        !- Zone Air Exhaust Node or NodeList Name                    ",
//"    Daylit Zone Air Node,    !- Zone Air Node Name                                        ",
//"    Daylit Zone Return Node; !- Zone Return Air Node Name                                 ",
//"                                                                                          ",
//"  ThermostatSetpoint:SingleHeatingOrCooling,                                              ",
//"    Heating Cooling Setpoint,!- Name                                                      ",
//"    Zone Temp Sch;           !- Setpoint Temperature Schedule Name                        ",
//"                                                                                          ",
//"  ZoneControl:Thermostat,                                                                 ",
//"    Daylit Zone Thermostat,  !- Name                                                      ",
//"    Daylit Zone,             !- Zone or ZoneList Name                                     ",
//"    Zone Control Type Sch,   !- Control Type Schedule Name                                ",
//"    ThermostatSetpoint:SingleHeatingOrCooling,  !- Control 1 Object Type                  ",
//"    Heating Cooling Setpoint;!- Control 1 Name                                            ",
//"                                                                                          ",
//"  ZoneHVAC:EquipmentList,                                                                 ",
//"    Daylit Zone Equipment,   !- Name                                                      ",
//"    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type                        ",
//"    Daylit Zone Air,         !- Zone Equipment 1 Name                                     ",
//"    1,                       !- Zone Equipment 1 Cooling Sequence                         ",
//"    1;                       !- Zone Equipment 1 Heating or No-Load Sequence              ",
//"                                                                                          ",
//"  ZoneHVAC:IdealLoadsAirSystem,                                                           ",
//"    Daylit Zone Air,         !- Name                                                      ",
//"    ,                        !- Availability Schedule Name                                ",
//"    Daylit Zone Supply Node, !- Zone Supply Air Node Name                                 ",
//"    ,                        !- Zone Exhaust Air Node Name                                ",
//"    ,                        !- System Inlet Air Node Name                                         ",
//"    50,                      !- Maximum Heating Supply Air Temperature {C}                ",
//"    13,                      !- Minimum Cooling Supply Air Temperature {C}                ",
//"    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAi",
//"    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAi",
//"    NoLimit,                 !- Heating Limit                                             ",
//"    AUTOSIZE,                !- Maximum Heating Air Flow Rate {m3/s}                      ",
//"    ,                        !- Maximum Sensible Heating Capacity {W}                     ",
//"    NoLimit,                 !- Cooling Limit                                             ",
//"    AUTOSIZE,                !- Maximum Cooling Air Flow Rate {m3/s}                      ",
//"    ,                        !- Maximum Total Cooling Capacity {W}                        ",
//"    ,                        !- Heating Availability Schedule Name                        ",
//"    ,                        !- Cooling Availability Schedule Name                        ",
//"    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type                        ",
//"    ,                        !- Cooling Sensible Heat Ratio {dimensionless}               ",
//"    ConstantSupplyHumidityRatio,  !- Humidification Control Type                          ",
//"    ,                        !- Design Specification Outdoor Air Object Name              ",
//"    ,                        !- Outdoor Air Inlet Node Name                               ",
//"    ,                        !- Demand Controlled Ventilation Type                        ",
//"    ,                        !- Outdoor Air Economizer Type                               ",
//"    ,                        !- Heat Recovery Type                                        ",
//"    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}      ",
//"    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}        ",
//"                                                                                          ",
//"  NodeList,                                                                               ",
//"    Standard Zone Inlets,    !- Name                                                      ",
//"    Standard Zone Supply Node;  !- Node 1 Name                                            ",
//"                                                                                          ",
//"  ZoneHVAC:EquipmentConnections,                                                          ",
//"    Standard Zone,           !- Zone Name                                                 ",
//"    Standard Zone Equipment, !- Zone Conditioning Equipment List Name                     ",
//"    Standard Zone Inlets,    !- Zone Air Inlet Node or NodeList Name                      ",
//"    ,                        !- Zone Air Exhaust Node or NodeList Name                    ",
//"    Standard Zone Air Node,  !- Zone Air Node Name                                        ",
//"    Standard Zone Return Node;  !- Zone Return Air Node Name                              ",
//"                                                                                          ",
//"  ThermostatSetpoint:SingleHeatingOrCooling,                                              ",
//"    Heating Cooling Setpoint,!- Name                                                      ",
//"    Zone Temp Sch;           !- Setpoint Temperature Schedule Name                        ",
//"                                                                                          ",
//"  ZoneControl:Thermostat,                                                                 ",
//"    Standard Zone Thermostat,!- Name                                                      ",
//"    Standard Zone,           !- Zone or ZoneList Name                                     ",
//"    Zone Control Type Sch,   !- Control Type Schedule Name                                ",
//"    ThermostatSetpoint:SingleHeatingOrCooling,  !- Control 1 Object Type                  ",
//"    Heating Cooling Setpoint;!- Control 1 Name                                            ",
//"                                                                                          ",
//"  ZoneHVAC:EquipmentList,                                                                 ",
//"    Standard Zone Equipment, !- Name                                                      ",
//"    ZoneHVAC:IdealLoadsAirSystem,  !- Zone Equipment 1 Object Type                        ",
//"    Standard Zone Air,       !- Zone Equipment 1 Name                                     ",
//"    1,                       !- Zone Equipment 1 Cooling Sequence                         ",
//"    1;                       !- Zone Equipment 1 Heating or No-Load Sequence              ",
//"                                                                                          ",
//"  ZoneHVAC:IdealLoadsAirSystem,                                                           ",
//"    Standard Zone Air,       !- Name                                                      ",
//"    ,                        !- Availability Schedule Name                                ",
//"    Standard Zone Supply Node,  !- Zone Supply Air Node Name                              ",
//"    ,                        !- Zone Exhaust Air Node Name                                ",
//"    ,                        !- System Inlet Air Node Name                                         ",
//"    50,                      !- Maximum Heating Supply Air Temperature {C}                ",
//"    13,                      !- Minimum Cooling Supply Air Temperature {C}                ",
//"    0.015,                   !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAi",
//"    0.009,                   !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAi",
//"    NoLimit,                 !- Heating Limit                                             ",
//"    AUTOSIZE,                !- Maximum Heating Air Flow Rate {m3/s}                      ",
//"    ,                        !- Maximum Sensible Heating Capacity {W}                     ",
//"    NoLimit,                 !- Cooling Limit                                             ",
//"    AUTOSIZE,                !- Maximum Cooling Air Flow Rate {m3/s}                      ",
//"    ,                        !- Maximum Total Cooling Capacity {W}                        ",
//"    ,                        !- Heating Availability Schedule Name                        ",
//"    ,                        !- Cooling Availability Schedule Name                        ",
//"    ConstantSupplyHumidityRatio,  !- Dehumidification Control Type                        ",
//"    ,                        !- Cooling Sensible Heat Ratio {dimensionless}               ",
//"    ConstantSupplyHumidityRatio,  !- Humidification Control Type                          ",
//"    ,                        !- Design Specification Outdoor Air Object Name              ",
//"    ,                        !- Outdoor Air Inlet Node Name                               ",
//"    ,                        !- Demand Controlled Ventilation Type                        ",
//"    ,                        !- Outdoor Air Economizer Type                               ",
//"    ,                        !- Heat Recovery Type                                        ",
//"    ,                        !- Sensible Heat Recovery Effectiveness {dimensionless}      ",
//"    ;                        !- Latent Heat Recovery Effectiveness {dimensionless}        ",
//"                                                                                          ",
//"  Output:VariableDictionary,Regular;                                                      ",
//"                                                                                          ",
//"  Output:Surfaces:Drawing,DXF;                                                            ",
//"                                                                                          ",
//"  Output:Constructions,Constructions;                                                     ",
//"                                                                                          ",
//"  Output:Surfaces:List,Detailswithvertices;                                               ",
//"                                                                                          ",
//"  OutputControl:Table:Style,                                                              ",
//"    HTML;                    !- Column Separator                                          ",
//"                                                                                          ",
//"  Output:Table:SummaryReports,                                                            ",
//"    AllSummary;              !- Report 1 Name                                             ",
//"                                                                                          ",
//"  Output:SQLite,                                                                          ",
//"    SimpleAndTabular;        !- Option Type                                               ",
//} );

// ASSERT_TRUE( process_idf( idf_objects ) );

// OutputProcessor::TimeValue.allocate(2);
////state->dataGlobal->DDOnlySimulation = true;

// ManageSimulation();
////compare_err_stream( "" );

// EXPECT_EQ( "2", RetrievePreDefTableEntry(*state,  pdchSurfCntTot, "Tubular Daylighting Device Diffuser" ) );
// EXPECT_EQ( "2", RetrievePreDefTableEntry(*state,  pdchSurfCntExt, "Tubular Daylighting Device Diffuser" ) );

//}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_PredefinedTableRowMatchingTest)
{

    SetPredefinedTables(*state);

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedPerfElEneUse, "Exterior Lighting", 1000., 2);
    EXPECT_EQ("1000.00", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedPerfElEneUse, "Exterior Lighting"));
    EXPECT_EQ("NOT FOUND", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedPerfElEneUse, "EXTERIOR LIGHTING"));

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedPerfElEneUse, "EXTERIOR LIGHTING", 2000., 2);
    EXPECT_EQ("1000.00", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedPerfElEneUse, "Exterior Lighting"));
    EXPECT_EQ("2000.00", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchLeedPerfElEneUse, "EXTERIOR LIGHTING"));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetUnitSubstring_Test)
{
    EXPECT_EQ("", GetUnitSubString(""));
    EXPECT_EQ("", GetUnitSubString(" "));
    EXPECT_EQ("", GetUnitSubString("String with no units"));
    EXPECT_EQ("feet", GetUnitSubString("[feet]"));
    EXPECT_EQ("meters", GetUnitSubString("String with  unit string at end [meters]"));
    EXPECT_EQ("newtons", GetUnitSubString("[newtons] String with unit string at beginning"));
    EXPECT_EQ("m", GetUnitSubString("String with unit string at end [m]"));
    EXPECT_EQ("N", GetUnitSubString("[N] String with unit string at beginning"));
    EXPECT_EQ("", GetUnitSubString("[]"));
    EXPECT_EQ("", GetUnitSubString("String with empty unit string at end []"));
    EXPECT_EQ("", GetUnitSubString("[] String with empty unit string at beginning"));
}

TEST_F(SQLiteFixture, WriteVeriSumTableAreasTest)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularVeriSum = true;
    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).FrameDivider = 0;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).FrameDivider = 0;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 1;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).FrameDivider = 1;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).FrameDivider = 2;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 1;

    // frames
    state->dataHeatBal->TotFrameDivider = 2;
    state->dataSurface->FrameDivider.allocate(state->dataHeatBal->TotFrameDivider);
    state->dataSurface->FrameDivider(1).FrameWidth = 0.3;
    state->dataSurface->FrameDivider(2).FrameWidth = 0.2;

    // zone
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->Zone(1).FloorArea = 600.; // 20 x 30
    state->dataHeatBal->Zone(1).Volume = 6000.;   // 20 x 30 x 10
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 500.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    WriteVeriSumTable(*state);

    auto tabularData = queryResult("SELECT * FROM TabularData;", "TabularData");
    auto strings = queryResult("SELECT * FROM Strings;", "Strings");
    auto stringTypes = queryResult("SELECT * FROM StringTypes;", "StringTypes");

    EXPECT_EQ(174ul, tabularData.size());
    // tabularDataIndex, reportNameIndex, reportForStringIndex, tableNameIndex, rowLabelIndex, columnLabelIndex, unitsIndex, simulationIndex, rowId,
    // columnId, value
    EXPECT_EQ("       12.30", tabularData[3][10]);
    EXPECT_EQ("       45.60", tabularData[4][10]);
    // envelope - window-wall ratio subtable
    // north
    EXPECT_EQ("      200.00", tabularData[15][10]);
    EXPECT_EQ("      200.00", tabularData[16][10]);
    EXPECT_EQ("       48.16", tabularData[17][10]);
    EXPECT_EQ("       24.08", tabularData[18][10]);
    EXPECT_EQ("       24.08", tabularData[19][10]);
    // east
    EXPECT_EQ("      300.00", tabularData[20][10]);
    EXPECT_EQ("      300.00", tabularData[21][10]);
    EXPECT_EQ("       66.56", tabularData[22][10]);
    EXPECT_EQ("       22.19", tabularData[23][10]);
    EXPECT_EQ("       22.19", tabularData[24][10]);
    // Performance - zone summary table
    EXPECT_EQ("      600.00", tabularData[63][10]);  // area
    EXPECT_EQ("Yes", tabularData[68][10]);           // conditioned
    EXPECT_EQ("Yes", tabularData[73][10]);           // part of total floor area
    EXPECT_EQ("     6000.00", tabularData[78][10]);  // volume
    EXPECT_EQ("        1.00", tabularData[83][10]);  // multiplier
    EXPECT_EQ("      500.00", tabularData[88][10]);  // above ground gross floor area
    EXPECT_EQ("      100.00", tabularData[98][10]);  // window glass area
    EXPECT_EQ("      114.72", tabularData[103][10]); // window opening area
}

// Test for #6350 and #6469
TEST_F(SQLiteFixture, WriteVeriSumTable_TestNotPartOfTotal)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularVeriSum = true;
    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 2;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 2;

    // Loads
    state->dataHeatBal->TotLights = 3;
    state->dataHeatBal->Lights.allocate(state->dataHeatBal->TotLights);

    state->dataHeatBal->TotPeople = 3;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);

    state->dataHeatBal->TotElecEquip = 3;
    state->dataHeatBal->ZoneElectric.allocate(state->dataHeatBal->TotElecEquip);

    state->dataHeatBal->TotITEquip = 3;
    state->dataHeatBal->ZoneITEq.allocate(state->dataHeatBal->TotITEquip);

    state->dataHeatBal->Lights(1).ZonePtr = 1;
    state->dataHeatBal->Lights(1).spaceIndex = 1;
    state->dataHeatBal->Lights(1).DesignLevel = 1000.0;
    state->dataHeatBal->Lights(2).ZonePtr = 2;
    state->dataHeatBal->Lights(2).spaceIndex = 2;
    state->dataHeatBal->Lights(2).DesignLevel = 100.0;
    state->dataHeatBal->Lights(3).ZonePtr = 3;
    state->dataHeatBal->Lights(3).spaceIndex = 3;
    state->dataHeatBal->Lights(3).DesignLevel = 10.0;

    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).spaceIndex = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 10.0;
    state->dataHeatBal->People(2).ZonePtr = 2;
    state->dataHeatBal->People(2).spaceIndex = 2;
    state->dataHeatBal->People(2).NumberOfPeople = 5.0;
    state->dataHeatBal->People(3).ZonePtr = 3;
    state->dataHeatBal->People(3).spaceIndex = 3;
    state->dataHeatBal->People(3).NumberOfPeople = 1.0;

    state->dataHeatBal->ZoneElectric(1).ZonePtr = 1;
    state->dataHeatBal->ZoneElectric(1).spaceIndex = 1;
    state->dataHeatBal->ZoneElectric(1).DesignLevel = 500.0;
    state->dataHeatBal->ZoneElectric(2).ZonePtr = 2;
    state->dataHeatBal->ZoneElectric(2).spaceIndex = 2;
    state->dataHeatBal->ZoneElectric(2).DesignLevel = 50.0;
    state->dataHeatBal->ZoneElectric(3).ZonePtr = 3;
    state->dataHeatBal->ZoneElectric(3).spaceIndex = 3;
    state->dataHeatBal->ZoneElectric(3).DesignLevel = 5.0;

    state->dataHeatBal->ZoneITEq(1).ZonePtr = 1;
    state->dataHeatBal->ZoneITEq(1).spaceIndex = 1;
    state->dataHeatBal->ZoneITEq(1).DesignTotalPower = 750.0;
    state->dataHeatBal->ZoneITEq(2).ZonePtr = 2;
    state->dataHeatBal->ZoneITEq(2).spaceIndex = 2;
    state->dataHeatBal->ZoneITEq(2).DesignTotalPower = 300.0;
    state->dataHeatBal->ZoneITEq(3).ZonePtr = 3;
    state->dataHeatBal->ZoneITEq(3).spaceIndex = 3;
    state->dataHeatBal->ZoneITEq(3).DesignTotalPower = 150.0;

    // zone
    state->dataGlobal->NumOfZones = 3;
    state->dataGlobal->numSpaces = 3;
    state->dataViewFactor->NumOfSolarEnclosures = 3;
    state->dataViewFactor->EnclSolInfo.allocate(state->dataViewFactor->NumOfSolarEnclosures);
    state->dataGlobal->numSpaceTypes = 1;
    state->dataHeatBal->spaceTypes.allocate(state->dataGlobal->numSpaceTypes);
    state->dataHeatBal->spaceTypes(1) = "General";
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->space.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->space(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->space(1).spaceTypeNum = 1;
    state->dataHeatBal->space(1).solarEnclosureNum = 1;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->Zone(1).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes(1) = 1;
    // 10x10x2
    state->dataHeatBal->Zone(1).FloorArea = 1000.;
    state->dataHeatBal->Zone(1).Volume = 2000.;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 800.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    state->dataHeatBal->Zone(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->space(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->space(2).spaceTypeNum = 1;
    state->dataHeatBal->space(2).solarEnclosureNum = 2;
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 0; // Unconditioned
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).Multiplier = 1.;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.;
    state->dataHeatBal->Zone(2).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(2).spaceIndexes(1) = 2;
    // 10x10x2
    state->dataHeatBal->Zone(2).FloorArea = 100.;
    state->dataHeatBal->Zone(2).Volume = 200.;
    state->dataHeatBal->Zone(2).ExtGrossWallArea = 80.;
    state->dataHeatBal->Zone(2).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(2).ExtWindowArea = 0.0;

    state->dataHeatBal->Zone(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->space(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->space(3).spaceTypeNum = 1;
    state->dataHeatBal->space(3).solarEnclosureNum = 3;
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(3).isPartOfTotalArea = false;
    state->dataHeatBal->Zone(3).Multiplier = 1.;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.;
    state->dataHeatBal->Zone(3).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(3).spaceIndexes(1) = 3;
    // 10x10x2
    state->dataHeatBal->Zone(3).FloorArea = 10.;
    state->dataHeatBal->Zone(3).Volume = 20.;
    state->dataHeatBal->Zone(3).ExtGrossWallArea = 8.;
    state->dataHeatBal->Zone(3).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(3).ExtWindowArea = 0.0;

    WriteVeriSumTable(*state);

    // Check Yes/No flag

    // RowName, ColumnName, value
    std::vector<std::tuple<std::string, std::string, std::string>> results_strings({
        {state->dataHeatBal->Zone(1).Name, "Conditioned (Y/N)", "Yes"},
        {state->dataHeatBal->Zone(1).Name, "Part of Total Floor Area (Y/N)", "Yes"},

        {state->dataHeatBal->Zone(2).Name, "Conditioned (Y/N)", "No"},
        {state->dataHeatBal->Zone(2).Name, "Part of Total Floor Area (Y/N)", "Yes"},

        {state->dataHeatBal->Zone(3).Name, "Conditioned (Y/N)", "Yes"},
        {state->dataHeatBal->Zone(3).Name, "Part of Total Floor Area (Y/N)", "No"},
    });

    // Would have used bind_text in sqlite3 with a single prepared statement, but m_db is protected in SQLiteProcedures
    std::string rowName;
    std::string columnName;

    for (auto v : results_strings) {

        rowName = std::get<0>(v);
        columnName = std::get<1>(v);

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = 'InputVerificationandResultsSummary'"
                          "  AND TableName = 'Zone Summary'"
                          "  AND RowName = '" +
                          rowName + "'" + "  AND ColumnName = '" + columnName + "'");

        std::string flag = queryResult(query, "TabularDataWithStrings")[0][0];

        // Add informative message if failed
        EXPECT_EQ(std::get<2>(v), flag) << "Failed for RowName=" << rowName << "; ColumnName=" << columnName;
    }

    // Check each zone and total rows

    // RowName, ColumnName, value
    std::vector<std::tuple<std::string, std::string, Real64>> results({
        {state->dataHeatBal->Zone(1).Name, "Area", state->dataHeatBal->Zone(1).FloorArea},
        {state->dataHeatBal->Zone(2).Name, "Area", state->dataHeatBal->Zone(2).FloorArea},
        {state->dataHeatBal->Zone(3).Name, "Area", state->dataHeatBal->Zone(3).FloorArea},
        {"Total", "Area", state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea},
        {"Conditioned Total", "Area", state->dataHeatBal->Zone(1).FloorArea},
        {"Unconditioned Total", "Area", state->dataHeatBal->Zone(2).FloorArea},
        {"Not Part of Total", "Area", state->dataHeatBal->Zone(3).FloorArea},

        {state->dataHeatBal->Zone(1).Name, "Volume", state->dataHeatBal->Zone(1).Volume},
        {state->dataHeatBal->Zone(2).Name, "Volume", state->dataHeatBal->Zone(2).Volume},
        {state->dataHeatBal->Zone(3).Name, "Volume", state->dataHeatBal->Zone(3).Volume},
        {"Total", "Volume", state->dataHeatBal->Zone(1).Volume + state->dataHeatBal->Zone(2).Volume},
        {"Conditioned Total", "Volume", state->dataHeatBal->Zone(1).Volume},
        {"Unconditioned Total", "Volume", state->dataHeatBal->Zone(2).Volume},
        {"Not Part of Total", "Volume", state->dataHeatBal->Zone(3).Volume},

        {state->dataHeatBal->Zone(1).Name, "Lighting", state->dataHeatBal->Lights(1).DesignLevel / state->dataHeatBal->Zone(1).FloorArea},
        {state->dataHeatBal->Zone(2).Name, "Lighting", state->dataHeatBal->Lights(2).DesignLevel / state->dataHeatBal->Zone(2).FloorArea},
        {state->dataHeatBal->Zone(3).Name, "Lighting", state->dataHeatBal->Lights(3).DesignLevel / state->dataHeatBal->Zone(3).FloorArea},
        {"Total",
         "Lighting",
         (state->dataHeatBal->Lights(1).DesignLevel + state->dataHeatBal->Lights(2).DesignLevel) /
             (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea)},
        {"Conditioned Total", "Lighting", state->dataHeatBal->Lights(1).DesignLevel / state->dataHeatBal->Zone(1).FloorArea},
        {"Unconditioned Total", "Lighting", state->dataHeatBal->Lights(2).DesignLevel / state->dataHeatBal->Zone(2).FloorArea},
        {"Not Part of Total", "Lighting", state->dataHeatBal->Lights(3).DesignLevel / state->dataHeatBal->Zone(3).FloorArea},

        // People/m^2
        {state->dataHeatBal->Zone(1).Name, "People", state->dataHeatBal->Zone(1).FloorArea / state->dataHeatBal->People(1).NumberOfPeople},
        {state->dataHeatBal->Zone(2).Name, "People", state->dataHeatBal->Zone(2).FloorArea / state->dataHeatBal->People(2).NumberOfPeople},
        {state->dataHeatBal->Zone(3).Name, "People", state->dataHeatBal->Zone(3).FloorArea / state->dataHeatBal->People(3).NumberOfPeople},
        {"Total",
         "People",
         (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea) /
             (state->dataHeatBal->People(1).NumberOfPeople + state->dataHeatBal->People(2).NumberOfPeople)},
        {"Conditioned Total", "People", state->dataHeatBal->Zone(1).FloorArea / state->dataHeatBal->People(1).NumberOfPeople},
        {"Unconditioned Total", "People", state->dataHeatBal->Zone(2).FloorArea / state->dataHeatBal->People(2).NumberOfPeople},
        {"Not Part of Total", "People", state->dataHeatBal->Zone(3).FloorArea / state->dataHeatBal->People(3).NumberOfPeople},

        {state->dataHeatBal->Zone(1).Name,
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(1).DesignLevel + state->dataHeatBal->ZoneITEq(1).DesignTotalPower) /
             state->dataHeatBal->Zone(1).FloorArea},
        {state->dataHeatBal->Zone(2).Name,
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(2).DesignLevel + state->dataHeatBal->ZoneITEq(2).DesignTotalPower) /
             state->dataHeatBal->Zone(2).FloorArea},
        {state->dataHeatBal->Zone(3).Name,
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(3).DesignLevel + state->dataHeatBal->ZoneITEq(3).DesignTotalPower) /
             state->dataHeatBal->Zone(3).FloorArea},
        {"Total",
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(1).DesignLevel + state->dataHeatBal->ZoneElectric(2).DesignLevel +
          +state->dataHeatBal->ZoneITEq(1).DesignTotalPower + state->dataHeatBal->ZoneITEq(2).DesignTotalPower) /
             (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea)},
        {"Conditioned Total",
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(1).DesignLevel + state->dataHeatBal->ZoneITEq(1).DesignTotalPower) /
             state->dataHeatBal->Zone(1).FloorArea},
        {"Unconditioned Total",
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(2).DesignLevel + state->dataHeatBal->ZoneITEq(2).DesignTotalPower) /
             state->dataHeatBal->Zone(2).FloorArea},
        {"Not Part of Total",
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(3).DesignLevel + state->dataHeatBal->ZoneITEq(3).DesignTotalPower) /
             state->dataHeatBal->Zone(3).FloorArea},
    });

    // Would have used bind_text in sqlite3 with a single prepared statement, but m_db is protected in SQLiteProcedures
    for (auto v : results) {

        rowName = std::get<0>(v);
        columnName = std::get<1>(v);

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = 'InputVerificationandResultsSummary'"
                          "  AND TableName = 'Zone Summary'"
                          "  AND RowName = '" +
                          rowName + "'" + "  AND ColumnName = '" + columnName + "'");

        Real64 return_val = execAndReturnFirstDouble(query);

        // Add informative message if failed
        EXPECT_NEAR(std::get<2>(v), return_val, 0.01) << "Failed for RowName=" << rowName << "; ColumnName=" << columnName;
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_invalidAggregationOrder)
{
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "Space Gains Annual Report, !- Name",
        "2, !-  Digits After Decimal",
        "Exterior Lights Electricity Energy, !- Variable or Meter 1 Name",
        "SumOrAverageDuringHoursShown, !- Aggregation Type for Variable or Meter 1",
        "Exterior Lights Electricity Rate, !- Variable or Meter 2 Name",
        "Maximum, !- Aggregation Type for Variable or Meter 2",
        "Exterior Lights Electricity Rate, !- Variable or Meter 2 Name",
        "Minimum; !- Aggregation Type for Variable or Meter 2",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite3",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    InitializeTabularMonthly(*state);

    EXPECT_TRUE(isInvalidAggregationOrder(*state));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_CollectPeakZoneConditions_test)
{
    state->init_state(*state);
    createCoilSelectionReportObj(*state);

    CompLoadTablesType compLoad;
    int timeOfMax = 63;
    int zoneIndex = 1;
    bool isCooling = true;

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Multiplier = 1;
    state->dataHeatBal->Zone(1).ListMultiplier = 1;
    state->dataHeatBal->Zone(1).FloorArea = 12.;

    state->dataWeather->DesDayInput.allocate(1);
    state->dataWeather->DesDayInput(1).Month = 5;
    state->dataWeather->DesDayInput(1).DayOfMonth = 21;

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->MinutesInTimeStep = 15;

    state->dataSize->CalcFinalZoneSizing.allocate(1);
    state->dataSize->CalcFinalZoneSizing(1).CoolOutTempSeq.allocate(96);
    state->dataSize->CalcFinalZoneSizing(1).CoolOutTempSeq(63) = 38.;
    state->dataSize->CalcFinalZoneSizing(1).CoolOutHumRatSeq.allocate(96);
    state->dataSize->CalcFinalZoneSizing(1).CoolOutHumRatSeq(63) = 0.01459;
    state->dataSize->CalcFinalZoneSizing(1).CoolZoneTempSeq.allocate(96);
    state->dataSize->CalcFinalZoneSizing(1).CoolZoneTempSeq(63) = 24.;
    state->dataSize->CalcFinalZoneSizing(1).CoolZoneHumRatSeq.allocate(96);
    state->dataSize->CalcFinalZoneSizing(1).CoolZoneHumRatSeq(63) = 0.00979;
    state->dataSize->CalcFinalZoneSizing(1).DesCoolLoad = 500.;
    state->dataSize->CalcFinalZoneSizing(1).ZnCoolDgnSAMethod = SupplyAirTemperature;
    state->dataSize->CalcFinalZoneSizing(1).CoolDesTemp = 13.;
    state->dataSize->CalcFinalZoneSizing(1).DesCoolVolFlow = 3.3;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).DesCoolLoad = 600.;

    state->dataHeatBal->People.allocate(2);
    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(2).ZonePtr = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 3;
    state->dataHeatBal->People(2).NumberOfPeople = 5;

    CollectPeakZoneConditions(*state, compLoad, 1, timeOfMax, zoneIndex, isCooling);

    EXPECT_EQ(compLoad.peakDateHrMin, "5/21 15:45:00");
    EXPECT_EQ(compLoad.outsideDryBulb, 38.);
    EXPECT_EQ(compLoad.outsideHumRatio, 0.01459);
    EXPECT_NEAR(compLoad.outsideWetBulb, 25.003, 0.001);
    EXPECT_EQ(compLoad.zoneDryBulb, 24.);
    EXPECT_EQ(compLoad.zoneHumRatio, 0.00979);
    EXPECT_NEAR(compLoad.zoneRelHum, 0.526, 0.001);
    EXPECT_EQ(compLoad.peakDesSensLoad, 500.);
    EXPECT_EQ(compLoad.designPeakLoad, 600.);
    EXPECT_EQ(compLoad.supAirTemp, 13.);
    EXPECT_EQ(compLoad.mainFanAirFlow, 3.3);
    EXPECT_EQ(compLoad.numPeople, 8.0);
    EXPECT_NEAR(compLoad.airflowPerFlrArea, 3.3 / 12., 0.0001);
    EXPECT_NEAR(compLoad.totCapPerArea, 600. / 12., 0.0001);
    EXPECT_NEAR(compLoad.airflowPerTotCap, 3.3 / 600., 0.0001);
    EXPECT_NEAR(compLoad.areaPerTotCap, 12. / 600., 0.0001);
    EXPECT_NEAR(state->dataEnvrn->StdBaroPress, 101325.0, 0.001);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ComputeEngineeringChecks_test)
{
    CompLoadTablesType compLoad;

    compLoad.outsideAirFlow = 0.12;
    compLoad.mainFanAirFlow = 0.50;
    compLoad.floorArea = 13.;
    compLoad.designPeakLoad = 800;

    ComputeEngineeringChecks(compLoad);

    EXPECT_EQ(compLoad.outsideAirRatio, 0.12 / 0.50);
    EXPECT_EQ(compLoad.airflowPerFlrArea, 0.50 / 13.);
    EXPECT_EQ(compLoad.totCapPerArea, 800. / 13.);
    EXPECT_EQ(compLoad.airflowPerTotCap, 0.50 / 800.);
    EXPECT_EQ(compLoad.areaPerTotCap, 13. / 800.);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetZoneComponentAreas_test)
{
    Array1D<ZompComponentAreasType> areas;
    Array1D<ZompComponentAreasType> spaceAreas;
    areas.allocate(1);

    state->dataHeatBal->Zone.allocate(1);
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone(1).FloorArea = 12.;

    state->dataSurface->Surface.allocate(13);

    state->dataSurface->Surface(1).GrossArea = 5.; // extWall
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).HeatTransSurf = true;

    state->dataSurface->Surface(2).GrossArea = 6.; // grdCntWall
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).ExtBoundCond = GroundFCfactorMethod;
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(2).HeatTransSurf = true;

    state->dataSurface->Surface(3).GrossArea = 7.; // intZoneWall
    state->dataSurface->Surface(3).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).ExtBoundCond = 2;
    state->dataSurface->Surface(3).Zone = 1;
    state->dataSurface->Surface(3).HeatTransSurf = true;

    state->dataSurface->Surface(4).GrossArea = 8.; // roof
    state->dataSurface->Surface(4).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Zone = 1;
    state->dataSurface->Surface(4).HeatTransSurf = true;

    state->dataSurface->Surface(5).GrossArea = 9.; // ceiling
    state->dataSurface->Surface(5).Class = SurfaceClass::Roof;
    state->dataSurface->Surface(5).ExtBoundCond = 5;
    state->dataSurface->Surface(5).Zone = 1;
    state->dataSurface->Surface(5).HeatTransSurf = true;

    state->dataSurface->Surface(6).GrossArea = 10.; // extFloor
    state->dataSurface->Surface(6).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(6).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(6).Zone = 1;
    state->dataSurface->Surface(6).HeatTransSurf = true;

    state->dataSurface->Surface(7).GrossArea = 11.; // grndCntFloor
    state->dataSurface->Surface(7).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(7).ExtBoundCond = Ground;
    state->dataSurface->Surface(7).Zone = 1;
    state->dataSurface->Surface(7).HeatTransSurf = true;

    state->dataSurface->Surface(8).GrossArea = 12.; // intZoneFloor
    state->dataSurface->Surface(8).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(8).ExtBoundCond = 3;
    state->dataSurface->Surface(8).Zone = 1;
    state->dataSurface->Surface(8).HeatTransSurf = true;

    state->dataSurface->Surface(9).GrossArea = 13.; // fenestration
    state->dataSurface->Surface(9).Class = SurfaceClass::Window;
    state->dataSurface->Surface(9).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(9).Zone = 1;
    state->dataSurface->Surface(9).HeatTransSurf = true;

    state->dataSurface->Surface(10).GrossArea = 14.; // door
    state->dataSurface->Surface(10).Class = SurfaceClass::Door;
    state->dataSurface->Surface(10).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(10).Zone = 1;
    state->dataSurface->Surface(10).HeatTransSurf = true;

    state->dataSurface->Surface(11).GrossArea = 15.; // door (again)
    state->dataSurface->Surface(11).Class = SurfaceClass::GlassDoor;
    state->dataSurface->Surface(11).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(11).Zone = 1;
    state->dataSurface->Surface(11).HeatTransSurf = true;

    state->dataSurface->Surface(12).GrossArea = 16.; // fenestration (again)
    state->dataSurface->Surface(12).Class = SurfaceClass::TDD_Dome;
    state->dataSurface->Surface(12).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(12).Zone = 1;
    state->dataSurface->Surface(12).HeatTransSurf = true;

    state->dataSurface->Surface(13).GrossArea = 17.; // grndCntFloor (again)
    state->dataSurface->Surface(13).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(13).ExtBoundCond = KivaFoundation;
    state->dataSurface->Surface(13).Zone = 1;
    state->dataSurface->Surface(13).HeatTransSurf = true;

    GetZoneComponentAreas(*state, areas, spaceAreas);

    EXPECT_EQ(12., areas(1).floor);
    EXPECT_EQ(8., areas(1).roof);
    EXPECT_EQ(9., areas(1).ceiling);
    EXPECT_EQ(5., areas(1).extWall);
    EXPECT_EQ(7., areas(1).intZoneWall);
    EXPECT_EQ(6., areas(1).grndCntWall);
    EXPECT_EQ(10., areas(1).extFloor);
    EXPECT_EQ(12., areas(1).intZoneFloor);
    EXPECT_EQ(28., areas(1).grndCntFloor);
    EXPECT_EQ(29., areas(1).fenestration);
    EXPECT_EQ(29., areas(1).door);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_CombineLoadCompResults_test)
{
    CompLoadTablesType compLoadTotal;
    // printf("3");
    compLoadTotal.cells.allocate(10, 30);
    compLoadTotal.cells = 0.;
    compLoadTotal.cellUsed.allocate(10, 30);
    compLoadTotal.cellUsed = false;

    // printf("4");
    CompLoadTablesType compLoadPartial;
    compLoadPartial.cells.allocate(10, 30);
    compLoadPartial.cells = 0.;
    compLoadPartial.cellUsed.allocate(10, 30);
    compLoadPartial.cellUsed = false;

    Real64 multiplier = 3.;

    compLoadPartial.cells(1, 1) = 1.1;
    compLoadPartial.cells(4, 25) = 1.2;
    compLoadPartial.cellUsed(3, 17) = true;
    compLoadPartial.outsideWetBulb = 17.;
    compLoadPartial.diffDesignPeak = 11.;

    // printf("5");
    CombineLoadCompResults(compLoadTotal, compLoadPartial, multiplier);
    // printf("6");

    EXPECT_EQ(1.1 * 3., compLoadTotal.cells(1, 1));
    EXPECT_EQ(1.2 * 3., compLoadTotal.cells(4, 25));
    EXPECT_EQ(true, compLoadTotal.cellUsed(3, 17));
    EXPECT_EQ(17., compLoadTotal.outsideWetBulb);
    EXPECT_EQ(33., compLoadTotal.diffDesignPeak);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_AddTotalRowsForLoadSummary_test)
{
    CompLoadTablesType compLoad;
    compLoad.cells.allocate(LoadCompCol::PerArea, LoadCompRow::GrdTot);
    compLoad.cells = 0.;
    compLoad.cellUsed.allocate(LoadCompCol::PerArea, LoadCompRow::GrdTot);
    compLoad.cellUsed = true;

    compLoad.cells(LoadCompCol::SensInst, LoadCompRow::Lights) = 3.;
    compLoad.cells(LoadCompCol::SensInst, LoadCompRow::Refrig) = 4.;
    compLoad.cells(LoadCompCol::Latent, LoadCompRow::Lights) = 10.;
    compLoad.cells(LoadCompCol::Latent, LoadCompRow::Refrig) = 20.;

    compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights) = 5.;
    compLoad.cells(LoadCompCol::Area, LoadCompRow::Refrig) = 5.;

    AddTotalRowsForLoadSummary(compLoad);

    EXPECT_EQ(3. + 4., compLoad.cells(LoadCompCol::SensInst, LoadCompRow::GrdTot));
    EXPECT_EQ(10 + 20., compLoad.cells(LoadCompCol::Latent, LoadCompRow::GrdTot));
    EXPECT_EQ(3. + 10., compLoad.cells(LoadCompCol::Total, LoadCompRow::Lights));
    EXPECT_EQ(4 + 20., compLoad.cells(LoadCompCol::Total, LoadCompRow::Refrig));

    EXPECT_EQ(37., compLoad.cells(LoadCompCol::Total, LoadCompRow::GrdTot));

    EXPECT_EQ(100. * 13. / 37., compLoad.cells(LoadCompCol::Perc, LoadCompRow::Lights));
    EXPECT_EQ(100. * 24. / 37., compLoad.cells(LoadCompCol::Perc, LoadCompRow::Refrig));

    EXPECT_EQ(13. / 5., compLoad.cells(LoadCompCol::PerArea, LoadCompRow::Lights));
    EXPECT_EQ(24. / 5., compLoad.cells(LoadCompCol::PerArea, LoadCompRow::Refrig));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_LoadSummaryUnitConversion_test)
{
    CompLoadTablesType compLoad;
    compLoad.cells.allocate(LoadCompCol::PerArea, LoadCompRow::GrdTot);
    compLoad.cells = 0.;
    compLoad.cellUsed.allocate(LoadCompCol::PerArea, LoadCompRow::GrdTot);
    compLoad.cellUsed = true;

    compLoad.cells(LoadCompCol::SensInst, LoadCompRow::Lights) = 3.;
    compLoad.cells(LoadCompCol::Latent, LoadCompRow::Lights) = 10.;

    compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights) = 5.;

    compLoad.outsideDryBulb = 20.;
    compLoad.mainFanAirFlow = 0.7;
    compLoad.airflowPerTotCap = 0.2;
    compLoad.totCapPerArea = 0.15;

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;
    SetupUnitConversions(*state);
    Real64 powerConversion = getSpecificUnitMultiplier(*state, "W", "Btu/h");
    Real64 areaConversion = getSpecificUnitMultiplier(*state, "m2", "ft2");
    Real64 airFlowConversion = getSpecificUnitMultiplier(*state, "m3/s", "ft3/min");
    int tempConvIndx = getSpecificUnitIndex(*state, "C", "F");

    LoadSummaryUnitConversion(*state, compLoad);

    EXPECT_EQ(3. * powerConversion, compLoad.cells(LoadCompCol::SensInst, LoadCompRow::Lights));
    EXPECT_EQ(10. * powerConversion, compLoad.cells(LoadCompCol::Latent, LoadCompRow::Lights));
    EXPECT_EQ(5. * areaConversion, compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights));
    EXPECT_EQ(5. * areaConversion, compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights));

    EXPECT_EQ(ConvertIP(*state, tempConvIndx, 20.), compLoad.outsideDryBulb);
    EXPECT_EQ(0.7 * airFlowConversion, compLoad.mainFanAirFlow);
    EXPECT_EQ(0.2 * airFlowConversion / powerConversion, compLoad.airflowPerTotCap);
    EXPECT_EQ(0.15 * powerConversion / areaConversion, compLoad.totCapPerArea);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_CreateListOfZonesForAirLoop_test)
{
    CompLoadTablesType compLoad;
    Array1D_int zoneToAirLoop;

    state->dataGlobal->NumOfZones = 15;
    compLoad.zoneIndices.allocate(state->dataGlobal->NumOfZones);
    compLoad.zoneIndices = 0;

    zoneToAirLoop.allocate(state->dataGlobal->NumOfZones);
    zoneToAirLoop(1) = 3;
    zoneToAirLoop(2) = 2;
    zoneToAirLoop(3) = 1;
    zoneToAirLoop(4) = 1;
    zoneToAirLoop(5) = 2;
    zoneToAirLoop(6) = 3;
    zoneToAirLoop(7) = 1;
    zoneToAirLoop(8) = 1;
    zoneToAirLoop(9) = 2;
    zoneToAirLoop(10) = 2;
    zoneToAirLoop(11) = 1;
    zoneToAirLoop(12) = 1;
    zoneToAirLoop(13) = 3;
    zoneToAirLoop(14) = 3;
    zoneToAirLoop(15) = 1;

    CreateListOfZonesForAirLoop(*state, compLoad, zoneToAirLoop, 1);
    EXPECT_EQ(3, compLoad.zoneIndices(1));
    EXPECT_EQ(4, compLoad.zoneIndices(2));
    EXPECT_EQ(7, compLoad.zoneIndices(3));
    EXPECT_EQ(8, compLoad.zoneIndices(4));
    EXPECT_EQ(11, compLoad.zoneIndices(5));
    EXPECT_EQ(12, compLoad.zoneIndices(6));
    EXPECT_EQ(15, compLoad.zoneIndices(7));
    EXPECT_EQ(0, compLoad.zoneIndices(8));

    compLoad.zoneIndices = 0;
    CreateListOfZonesForAirLoop(*state, compLoad, zoneToAirLoop, 2);
    EXPECT_EQ(2, compLoad.zoneIndices(1));
    EXPECT_EQ(5, compLoad.zoneIndices(2));
    EXPECT_EQ(9, compLoad.zoneIndices(3));
    EXPECT_EQ(10, compLoad.zoneIndices(4));
    EXPECT_EQ(0, compLoad.zoneIndices(5));

    compLoad.zoneIndices = 0;
    CreateListOfZonesForAirLoop(*state, compLoad, zoneToAirLoop, 3);
    EXPECT_EQ(1, compLoad.zoneIndices(1));
    EXPECT_EQ(6, compLoad.zoneIndices(2));
    EXPECT_EQ(13, compLoad.zoneIndices(3));
    EXPECT_EQ(14, compLoad.zoneIndices(4));
    EXPECT_EQ(0, compLoad.zoneIndices(5));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetDelaySequencesTwice_test)
{

    int coolDesSelected = 1;
    int iZone = 1;
    state->dataEnvrn->TotDesDays = 2;
    state->dataEnvrn->TotRunDesPersDays = 3;
    state->dataGlobal->TimeStepsInHour = 4;

    state->dataGlobal->NumOfZones = 4;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataGlobal->numSpaces = 4;
    state->dataHeatBal->space.allocate(state->dataGlobal->numSpaces);
    state->dataViewFactor->NumOfRadiantEnclosures = 4;

    state->dataHeatBal->Zone(iZone).spaceIndexes.emplace_back(iZone);
    state->dataHeatBal->space(iZone).HTSurfaceFirst = 1;
    state->dataHeatBal->space(iZone).HTSurfaceLast = 1;
    state->dataHeatBal->space(iZone).radiantEnclosureNum = 1;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).Class = SurfaceClass::Window;
    state->dataSurface->Surface(1).RadEnclIndex = 1;

    Array1D<Real64> peopleDelaySeq;
    peopleDelaySeq.allocate(state->dataGlobal->TimeStepsInHour * 24);
    peopleDelaySeq = 0.;

    Array1D<Real64> peopleDelaySeqCool;
    peopleDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    peopleDelaySeqCool = 0.;

    Array1D<Real64> equipDelaySeqCool;
    equipDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    equipDelaySeqCool = 0.;

    Array1D<Real64> hvacLossDelaySeqCool;
    hvacLossDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    hvacLossDelaySeqCool = 0.;

    Array1D<Real64> powerGenDelaySeqCool;
    powerGenDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    powerGenDelaySeqCool = 0.;

    Array1D<Real64> lightDelaySeqCool;
    lightDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    lightDelaySeqCool = 0.;

    Array1D<Real64> feneSolarDelaySeqCool;
    feneSolarDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    feneSolarDelaySeqCool = 0.;

    Array2D<Real64> surfDelaySeqCool;
    surfDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24, state->dataSurface->TotSurfaces);
    surfDelaySeqCool = 0.0;

    AllocateLoadComponentArrays(*state);

    auto &znCL = state->dataOutRptTab->znCompLoads;
    znCL[coolDesSelected - 1].ts[0].spacezone[iZone - 1].feneCondInstantSeq = 0.88;

    state->dataOutRptTab->surfCompLoads[coolDesSelected - 1].ts[0].surf[0].netSurfRadSeq = 0.05;

    GetDelaySequences(*state,
                      coolDesSelected,
                      true,
                      iZone,
                      peopleDelaySeqCool,
                      equipDelaySeqCool,
                      hvacLossDelaySeqCool,
                      powerGenDelaySeqCool,
                      lightDelaySeqCool,
                      feneSolarDelaySeqCool,
                      znCL,
                      surfDelaySeqCool);

    EXPECT_EQ(
        0.83,
        znCL[coolDesSelected - 1].ts[0].spacezone[iZone - 1].feneCondInstantSeq); // the first time the subtraction operation should have occurred

    GetDelaySequences(*state,
                      coolDesSelected,
                      true,
                      iZone,
                      peopleDelaySeqCool,
                      equipDelaySeqCool,
                      hvacLossDelaySeqCool,
                      powerGenDelaySeqCool,
                      lightDelaySeqCool,
                      feneSolarDelaySeqCool,
                      znCL,
                      surfDelaySeqCool);

    EXPECT_EQ(0.83,
              znCL[coolDesSelected - 1].ts[0].spacezone[iZone - 1].feneCondInstantSeq); // the second time the subtraction should not have happened
                                                                                        // since it is
    // only adjusted once so the value should be the same.
}

TEST_F(SQLiteFixture, OutputReportTabular_WriteLoadComponentSummaryTables_AirLoop_ZeroDesignDay)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->SysSizPeakDDNum.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataSize->FinalSysSizing.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataSize->CalcSysSizing.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    int numDesDays = 2;
    state->dataAirLoop->AirToZoneNodeInfo.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataGlobal->NumOfZones = 0;
    state->dataOutRptTab->displayAirLoopComponentLoadSummary = true;
    state->dataGlobal->CompLoadReportIsReq = true;
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TimeStepAtTotCoolPk.allocate(numDesDays);

    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TotCoolPeakDD = 0; // set to zero to indicate no design day chosen
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).HeatPeakDD = 0;    // set to zero to indicate no design day chosen

    WriteLoadComponentSummaryTables(*state);

    auto tabularData = queryResult("SELECT * FROM TabularData;", "TabularData");
    auto strings = queryResult("SELECT * FROM Strings;", "Strings");
    auto stringTypes = queryResult("SELECT * FROM StringTypes;", "StringTypes");

    EXPECT_EQ(460ul, tabularData.size());
    EXPECT_EQ(77ul, strings.size());
    EXPECT_EQ("AirLoop Component Load Summary", strings[0][2]); // just make sure that the output table was generated and did not crash
}

// Test for https://github.com/NREL/EnergyPlus/issues/7346
// We ensure that if the Airloop peak matches the zone peak, we don't do the IP conversion twice
TEST_F(SQLiteFixture, OutputReportTabular_WriteLoadComponentSummaryTables_AirLoop_IPConversion)
{
    state->init_state(*state);

    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    OutputReportTabular::SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    // We ask for the air loop component load summary since that's the one we test
    // We also ask for the zone component load summary because that's necessary to "copy" the load and trigger a potential double conversion
    state->dataOutRptTab->displayAirLoopComponentLoadSummary = true;
    state->dataOutRptTab->displayZoneComponentLoadSummary = true;
    state->dataOutRptTab->displayFacilityComponentLoadSummary = true;
    state->dataGlobal->CompLoadReportIsReq = true;

    createCoilSelectionReportObj(*state);

    // Two design days
    int numDesDays = 2;
    state->dataEnvrn->TotDesDays = numDesDays;
    state->dataEnvrn->TotRunDesPersDays = 0;
    state->dataWeather->DesDayInput.allocate(2);
    state->dataWeather->DesDayInput(1).Month = 7;
    state->dataWeather->DesDayInput(1).DayOfMonth = 21;
    state->dataWeather->DesDayInput(2).Month = 1;
    state->dataWeather->DesDayInput(2).DayOfMonth = 21;

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->MinutesInTimeStep = 15;
    int numTimeStepInDay = 96;

    // One Zone
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Multiplier = 1;
    state->dataHeatBal->Zone(1).ListMultiplier = 1;
    state->dataHeatBal->Zone(1).FloorArea = 100.;
    // Trick E+ into not iterating on Surfaces
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 0;

    // Cool Peak on 1st DD at 16:00 and Heat Peak on 2nd DD at 1:00
    state->dataSize->CalcFinalZoneSizing.allocate(state->dataGlobal->NumOfZones);

    int coolDDNum = 1;
    int coolTimeOfMax = 64;
    auto &finalZoneSizing = state->dataSize->CalcFinalZoneSizing(1);
    finalZoneSizing.CoolDDNum = coolDDNum;
    finalZoneSizing.TimeStepNumAtCoolMax = coolTimeOfMax;

    finalZoneSizing.CoolOutTempSeq.allocate(numTimeStepInDay);
    finalZoneSizing.CoolOutTempSeq(coolTimeOfMax) = 38.;
    finalZoneSizing.CoolOutHumRatSeq.allocate(numTimeStepInDay);
    finalZoneSizing.CoolOutHumRatSeq(coolTimeOfMax) = 0.01459;
    finalZoneSizing.CoolZoneTempSeq.allocate(numTimeStepInDay);
    finalZoneSizing.CoolZoneTempSeq(coolTimeOfMax) = 24.;
    finalZoneSizing.CoolZoneHumRatSeq.allocate(numTimeStepInDay);
    finalZoneSizing.CoolZoneHumRatSeq(coolTimeOfMax) = 0.00979;
    finalZoneSizing.DesCoolLoad = 500.;
    finalZoneSizing.ZnCoolDgnSAMethod = SupplyAirTemperature;
    finalZoneSizing.CoolDesTemp = 13.;
    finalZoneSizing.DesCoolVolFlow = 3.3;

    int heatDDNum = 2;
    int heatTimeOfMax = 4;
    finalZoneSizing.HeatDDNum = heatDDNum;
    finalZoneSizing.TimeStepNumAtHeatMax = heatTimeOfMax;

    finalZoneSizing.HeatOutTempSeq.allocate(numTimeStepInDay);
    finalZoneSizing.HeatOutTempSeq(heatTimeOfMax) = -17.4;
    finalZoneSizing.HeatOutHumRatSeq.allocate(numTimeStepInDay);
    finalZoneSizing.HeatOutHumRatSeq(heatTimeOfMax) = 0.01459;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(1).DesCoolLoad = 600.;

    // One airloop, that serves this zone.
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataSize->SysSizPeakDDNum.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataSize->FinalSysSizing.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataSize->CalcSysSizing.allocate(state->dataHVACGlobal->NumPrimaryAirSys);

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;

    finalZoneSizing.HeatZoneTempSeq.allocate(numTimeStepInDay);
    finalZoneSizing.HeatZoneTempSeq(heatTimeOfMax) = 20.;
    finalZoneSizing.HeatZoneHumRatSeq.allocate(numTimeStepInDay);
    finalZoneSizing.HeatZoneHumRatSeq(heatTimeOfMax) = 0.00979;
    finalZoneSizing.DesHeatLoad = 750.;
    finalZoneSizing.ZnHeatDgnSAMethod = SupplyAirTemperature;
    finalZoneSizing.HeatDesTemp = 35.;
    finalZoneSizing.DesHeatVolFlow = 3.3;

    state->dataSize->CalcZoneSizing.allocate(numDesDays, state->dataGlobal->NumOfZones);
    state->dataSize->CalcZoneSizing(1, 1).DOASHeatAddSeq.allocate(numTimeStepInDay);
    state->dataSize->CalcZoneSizing(1, 1).DOASHeatAddSeq = 0.0;
    state->dataSize->CalcZoneSizing(1, 1).DOASLatAddSeq.allocate(numTimeStepInDay);
    state->dataSize->CalcZoneSizing(1, 1).DOASLatAddSeq = 0.0;
    state->dataSize->CalcZoneSizing(2, 1).DOASHeatAddSeq.allocate(numTimeStepInDay);
    state->dataSize->CalcZoneSizing(2, 1).DOASHeatAddSeq = 0.0;
    state->dataSize->CalcZoneSizing(2, 1).DOASLatAddSeq.allocate(numTimeStepInDay);
    state->dataSize->CalcZoneSizing(2, 1).DOASLatAddSeq = 0.0;

    state->dataAirLoop->AirToZoneNodeInfo.allocate(state->dataHVACGlobal->NumPrimaryAirSys);
    state->dataAirLoop->AirToZoneNodeInfo(1).NumZonesCooled = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).CoolCtrlZoneNums(1) = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).NumZonesHeated = 1;
    state->dataAirLoop->AirToZoneNodeInfo(1).HeatCtrlZoneNums.allocate(1);
    state->dataAirLoop->AirToZoneNodeInfo(1).HeatCtrlZoneNums(1) = 1;

    // same Design Days peak and timestep peak as the zone it serves. This is the critical part of the test
    state->dataSize->FinalSysSizing(1).loadSizingType = DataSizing::LoadSizing::Total;

    // For #9772
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(state->dataSize->NumSysSizInput);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataSize->SysSizInput(1).SizingOption = DataSizing::SizingConcurrence::NonCoincident;
    auto degC_to_F = [](Real64 celsius) constexpr { return celsius * (9.0 / 5.0) + 32.0; };
    constexpr Real64 coolMixTempSys = 26.2;
    constexpr Real64 coolMixTempSysIP = degC_to_F(coolMixTempSys);
    constexpr Real64 heatMixTempSys = -1.7;
    constexpr Real64 heatMixTempSysIP = degC_to_F(heatMixTempSys);
    state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak = coolMixTempSys;
    state->dataSize->FinalSysSizing(1).HeatMixTemp = heatMixTempSys;

    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TotCoolPeakDD = coolDDNum;
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).HeatPeakDD = heatDDNum;
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TimeStepAtTotCoolPk.allocate(numDesDays);
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TimeStepAtTotCoolPk(1) = coolTimeOfMax;
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TimeStepAtHeatPk.allocate(numDesDays);
    state->dataSize->SysSizPeakDDNum(state->dataHVACGlobal->NumPrimaryAirSys).TimeStepAtHeatPk(2) = heatTimeOfMax;

    // Set up Facility to peak like the zone too
    state->dataSize->CalcFinalFacilitySizing.CoolDDNum = coolDDNum;
    state->dataSize->CalcFinalFacilitySizing.TimeStepNumAtCoolMax = coolTimeOfMax;
    state->dataSize->CalcFinalFacilitySizing.HeatDDNum = heatDDNum;
    state->dataSize->CalcFinalFacilitySizing.TimeStepNumAtHeatMax = heatTimeOfMax;

    AllocateLoadComponentArrays(*state);
    WriteLoadComponentSummaryTables(*state);

    // TableName, ReportName, value
    std::vector<std::tuple<std::string, std::string, std::string>> results_strings({
        // -17.4C gives 0.68F
        {"Heating Peak Conditions", "Zone Component Load Summary", "        0.68"},
        {"Heating Peak Conditions", "AirLoop Component Load Summary", "        0.68"},
        {"Heating Peak Conditions", "Facility Component Load Summary", "        0.68"},

        // 38C gives 100.4 F
        {"Cooling Peak Conditions", "Zone Component Load Summary", "      100.40"},
        {"Cooling Peak Conditions", "AirLoop Component Load Summary", "      100.40"},
        {"Cooling Peak Conditions", "Facility Component Load Summary", "      100.40"},

    });

    // Would have used bind_text in sqlite3 with a single prepared statement, but m_db is protected in SQLiteProcedures
    std::string tableName;
    std::string reportName;

    for (auto v : results_strings) {

        tableName = std::get<0>(v);
        reportName = std::get<1>(v);

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = '" +
                          reportName +
                          "'"
                          "  AND TableName = '" +
                          tableName +
                          "'"
                          "  AND RowName = 'Outside Dry Bulb Temperature';");

        std::string oa_db = queryResult(query, "TabularDataWithStrings")[0][0];

        // Add informative message if failed
        EXPECT_EQ(std::get<2>(v), oa_db) << "Failed for TableName=" << tableName << "; ReportName=" << reportName;
    }

    // https://github.com/NREL/EnergyPlus/pull/7741
    std::string query_2("SELECT Value From TabularDataWithStrings"
                        "  WHERE TableName = 'Engineering Checks for Cooling'"
                        "  AND RowName = 'Outside Air Fraction';");

    auto result = queryResult(query_2, "TabularDataWithStrings")[0][0];
    EXPECT_EQ(result, "0.0000");

    // Test for #9772
    {
        {
            const std::string queryCool = R"sql(
            SELECT Value From TabularDataWithStrings
             WHERE TableName = 'Cooling Peak Conditions'
                AND ReportName = 'AirLoop Component Load Summary'
                AND RowName = 'Mixed Air Temperature')sql";

            EXPECT_EQ(coolMixTempSysIP, execAndReturnFirstDouble(queryCool));
        }
        {
            const std::string queryHeat = R"sql(
            SELECT Value From TabularDataWithStrings
             WHERE TableName = 'Heating Peak Conditions'
                AND ReportName = 'AirLoop Component Load Summary'
                AND RowName = 'Mixed Air Temperature')sql";

            EXPECT_EQ(heatMixTempSysIP, execAndReturnFirstDouble(queryHeat));
        }
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_hasSizingPeriodsDays_SizingPeriodDesignDay)
{
    std::string const idf_objects = delimited_string({
        "SizingPeriod:DesignDay,",
        "  CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "  1,                       !- Month",
        "  21,                      !- Day of Month",
        "  WinterDesignDay,         !- Day Type",
        "  -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "  0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                 !- Humidity Condition Type",
        "  -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                        !- Humidity Condition Day Schedule Name",
        "  ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  99063.,                  !- Barometric Pressure {Pa}",
        "  4.9,                     !- Wind Speed {m/s}",
        "  270,                     !- Wind Direction {deg}",
        "  No,                      !- Rain Indicator",
        "  No,                      !- Snow Indicator",
        "  No,                      !- Daylight Saving Time Indicator",
        "  ASHRAEClearSky,          !- Solar Model Indicator",
        "  ,                        !- Beam Solar Day Schedule Name",
        "  ,                        !- Diffuse Solar Day Schedule Name",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "  0.0;                     !- Sky Clearness",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_TRUE(hasSizingPeriodsDays(*state));
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_hasSizingPeriodsDays_SizingPeriodWeatherFileDays)
{
    std::string const idf_objects = delimited_string({
        "SizingPeriod:WeatherFileDays,",
        "  Summer including Extreme Summer days,  !- Name",
        "  7,                       !- Begin Month",
        "  18,                      !- Begin Day of Month",
        "  7,                       !- End Month",
        "  25,                      !- End Day of Month",
        "  SummerDesignDay,         !- Day of Week for Start Day",
        "  No,                      !- Use Weather File Daylight Saving Period",
        "  No;                      !- Use Weather File Rain and Snow Indicators",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_TRUE(hasSizingPeriodsDays(*state));
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_hasSizingPeriodsDays_SizingPeriodWeatherFileConditionType)
{
    std::string const idf_objects = delimited_string({
        "SizingPeriod:WeatherFileConditionType,",
        "  Hot,                     !- Name",
        "  SummerExtreme,           !- Period Selection",
        "  Monday,                  !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  Yes;                     !- Use Weather File Rain and Snow Indicators",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // this test should show a false since this type of sizing period is not compatible with the component loads calculations
    EXPECT_FALSE(hasSizingPeriodsDays(*state));
}

// This tests aims to ensure that the needed Output:Variables for the Predefined Monthly table
// are indeeed set up, and that as a result the numTables is good.
// https://github.com/NREL/EnergyPlus/issues/7019
TEST_F(EnergyPlusFixture, OutputReportTabularMonthlyPredefined_FindNeededOutputVars)
{

    std::string const idf_objects = delimited_string({
        "Output:Table:SummaryReports,",
        " SetpointsNotMetWithTemperaturesMonthly; !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EXPECT_EQ(1, state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "Output:Table:SummaryReports"));
    EXPECT_EQ(0, state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "Output:Variable"));

    // InputProcessor::addVariablesForMonthlyReport should have requested 5 variables
    // for the SetpointsNotMetWithTemperatureMonthly report
    EXPECT_EQ(state->dataOutput->OutputVariablesForSimulation.size(), 5u);

    // The Variables needed for the report are initialized (=SetupOutputVariable)
    // Inside ThermalComfort::InitThermalComfort();
    // Except Zone Mean Air Temperature, which is in HeatBalanceAirManager::GetSimpleAirModelInputs()
    // Instead of calling these
    // Fake the setup of the OutputVariables needed for two different zones
    Real64 zoneTemp;
    Real64 timeNotMet;
    std::vector<std::string> ZoneNames({"Zone1", "Zone2"});

    for (int i = 0; i < 2; ++i) {
        SetupOutputVariable(*state,
                            "Zone Mean Air Temperature",
                            Constant::Units::C,
                            zoneTemp,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            ZoneNames[i]);

        SetupOutputVariable(*state,
                            "Zone Heating Setpoint Not Met Time",
                            Constant::Units::hr,
                            timeNotMet,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            ZoneNames[i]);
        SetupOutputVariable(*state,
                            "Zone Heating Setpoint Not Met While Occupied Time",
                            Constant::Units::hr,
                            timeNotMet,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            ZoneNames[i]);
        SetupOutputVariable(*state,
                            "Zone Cooling Setpoint Not Met Time",
                            Constant::Units::hr,
                            timeNotMet,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            ZoneNames[i]);
        SetupOutputVariable(*state,
                            "Zone Cooling Setpoint Not Met While Occupied Time",
                            Constant::Units::hr,
                            timeNotMet,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            ZoneNames[i]);
    }

    // We do need to trick it into thinking it's a weather simulation, otherwise the monthly reports aren't reported
    state->dataGlobal->DoWeathSim = true; // flag to trick tabular reports to scan meters

    OutputProcessor::GetReportVariableInput(*state);
    OutputReportTabular::GetInputOutputTableSummaryReports(*state);
    OutputReportTabular::InitializeTabularMonthly(*state);

    // We check that the Predefined Table is actually set to show
    EXPECT_EQ("SetpointsNotMetWithTemperaturesMonthly", state->dataOutRptTab->namedMonthly(31).title);
    EXPECT_TRUE(state->dataOutRptTab->namedMonthly(31).show);

    // Check that it's the only one that's shown
    for (int i = 1; i <= OutputReportTabular::numNamedMonthly; ++i) {
        if (i != 31) {
            EXPECT_FALSE(state->dataOutRptTab->namedMonthly(i).show);
        }
    }

    // Variables aren't going to be output to SQL/ESO anyways
    EXPECT_EQ(state->dataOutputProcessor->reqVars.size(), 0);

    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    // If everything worked, we should have 2 tables, one for each zone.
    // Previously, KeyCount was 0  because it couldn't find the variable in the OutputVariablesForSimulation
    // and so the numTables was zero
    EXPECT_EQ(state->dataOutRptTab->MonthlyInput(1).numTables, 2);
}
// https://github.com/NREL/EnergyPlus/issues/6442
TEST_F(SQLiteFixture, OutputReportTabularTest_PredefinedTableDXConversion)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    SetPredefinedTables(*state);
    std::string CompName = "My DX Coil with 10000W cooling";

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilType2, CompName, "Coil:Cooling:DX:SingleSpeed");
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilNetCapSIA, CompName, 10000., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilNetCapSIB, CompName, 12000., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilNetCapSIC, CompName, 14000., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilNetCapSID, CompName, 16000., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilElecPowerA, CompName, 3300., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilElecPowerB, CompName, 4300., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilElecPowerC, CompName, 5300., 1);
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilElecPowerD, CompName, 6300., 1);

    EXPECT_EQ("10000.0", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilNetCapSIA, CompName));

    // We enable the report we care about, making sure it's the right one
    EXPECT_EQ("EquipmentSummary", state->dataOutRptPredefined->reportName(5).name);
    state->dataOutRptPredefined->reportName(5).show = true;

    WritePredefinedTables(*state);
    state->dataSQLiteProcedures->sqlite->initializeIndexes();

    auto units = queryResult("Select Units From TabularDataWithStrings "
                             "WHERE ReportName = \"EquipmentSummary\" "
                             "  AND ColumnName = \"Rated Net Cooling Capacity Test A\"",
                             "TabularDataWithStrings");
    auto values = queryResult("Select Value From TabularDataWithStrings "
                              "WHERE ReportName = \"EquipmentSummary\" "
                              "  AND ColumnName = \"Rated Net Cooling Capacity Test A\"",
                              "TabularDataWithStrings");

    EXPECT_EQ(1u, units.size());
    // Because the table has 8 cols
    EXPECT_EQ(8u, units[0].size());

    EXPECT_EQ("ton", units[0][0]);

    EXPECT_EQ(1u, values.size());
    // 10000 W equavals 2.843 tons, rounded to 1 decimal gives 2.8
    std::string s = values[0][0];
    // Trim the string, it has leading spaces
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

    EXPECT_EQ("2.8", s);
}
// https://github.com/NREL/EnergyPlus/issues/7565
TEST_F(SQLiteFixture, OutputReportTabularTest_PredefinedTableCoilHumRat)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    SetPredefinedTables(*state);
    std::string CompName = "My DX Coil";

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilType, CompName, "Coil:Cooling:DX:SingleSpeed");
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdch2CoilLvgHumRatIdealPeak, CompName, 0.006, 8);
    // CoilSizingDetails
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchCoilLvgHumRatIdealPeak, CompName, 0.006, 8);

    // We enable the reports we care about, making sure we have the right ones
    EXPECT_EQ("HVACSizingSummary", state->dataOutRptPredefined->reportName(7).name);
    state->dataOutRptPredefined->reportName(7).show = true;
    EXPECT_EQ("CoilSizingDetails", state->dataOutRptPredefined->reportName(8).name);
    state->dataOutRptPredefined->reportName(8).show = true;

    WritePredefinedTables(*state);
    state->dataSQLiteProcedures->sqlite->initializeIndexes();

    for (const std::string reportName : {"HVACSizingSummary", "CoilSizingDetails"}) {

        auto result = queryResult("SELECT Value, Units From TabularDataWithStrings "
                                  "WHERE ReportName = \"" +
                                      reportName +
                                      "\""
                                      "  AND ColumnName = \"Coil Leaving Air Humidity Ratio at Ideal Loads Peak\"",
                                  "TabularDataWithStrings");

        EXPECT_EQ(1u, result.size());
        // Because the table has 8 cols
        EXPECT_EQ(8u, result[0].size());

        // 0.006 is a ratio, so unitconv = 1
        std::string s = result[0][0];
        // Trim the string, it has leading spaces
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

        EXPECT_EQ("0.00600000", s);

        EXPECT_EQ("lbWater/lbDryAir", result[0][1]);
    }
}

// Test for #7046
// Ensures that we get consistency between the displayed Azimuth and its cardinal classification
TEST_F(EnergyPlusFixture, AzimuthToCardinal)
{
    // >= 45 and < 135 => E
    // >= 135 and < 225 => S
    // >= 225 and < 315 => W
    // >= 315 and < 45 => N
    std::vector<std::pair<double, std::string>> expectedAzimuthToCards{
        {45.0, "E"},
        {45.01, "E"},
        {134.991, "E"},   // Rounds to 134.99, so classified as E
        {134.99978, "S"}, // Gets rounded to 135.00, so classified as S
        {135.0, "S"},
        {135.001, "S"},
        {136, "S"},
        {314.991, "W"},
        {314.9999, "N"},
        {315.0, "N"},
        {315.00001, "N"},
    };

    int nTests = expectedAzimuthToCards.size();

    // Allocate some needed arrays
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).ListMultiplier = 1;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).Name = "A Construction";
    // Avoid triggering CalcNominalWindowCond
    state->dataConstruction->Construct(1).SummerSHGC = 0.70;
    state->dataConstruction->Construct(1).VisTransNorm = 0.80;

    state->dataHeatBal->NominalU.allocate(1);
    state->dataHeatBal->NominalU(1) = 0.2;
    // Create one wall and one window with each azimuth from expectedAzimuthToCards
    // Azimuth & Cardinal entries happen in two separate blocks,
    // so test both to increase coverage and make sure both are correct

    state->dataSurface->TotSurfaces = 2 * nTests;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    SurfaceGeometry::AllocateSurfaceWindows(*state, state->dataSurface->TotSurfaces);

    for (int i = 1; i <= nTests * 2; ++i) {

        state->dataSurface->Surface(i).HeatTransSurf = true;
        state->dataSurface->Surface(i).ExtBoundCond = ExternalEnvironment;
        state->dataSurface->Surface(i).GrossArea = 200.;
        state->dataSurface->Surface(i).Tilt = 90.;
        state->dataSurface->Surface(i).Zone = 1;
        state->dataSurface->Surface(i).spaceNum = 1;
        state->dataSurface->Surface(i).Construction = 1;
        state->dataSurface->AllSurfaceListReportOrder.push_back(i);

        // Actual interesting stuff
        int entryIndex = (i - 1) / 2;
        double azimuth = expectedAzimuthToCards[entryIndex].first;
        state->dataSurface->Surface(i).Azimuth = azimuth;

        if (i % 2 == 1) {
            // It's a wall
            state->dataSurface->Surface(i).Class = DataSurfaces::SurfaceClass::Wall;
            state->dataSurface->Surface(i).Name = format("ExtWall_{}_{}", i, entryIndex);
        } else {
            // It's a window
            state->dataSurface->Surface(i).Class = DataSurfaces::SurfaceClass::Window;
            state->dataSurface->Surface(i).Name = format("ExtWindow_{}_{}", i, entryIndex);
            // Window references the previous wall
            state->dataSurface->Surface(i).BaseSurf = i - 1;
        }
    }

    // Setup pre def tables
    OutputReportPredefined::SetPredefinedTables(*state);

    // Call the routine that fills up the table we care about
    HeatBalanceSurfaceManager::GatherForPredefinedReport(*state);

    // Looking for Report 'EnvelopeSummary' (pdrEnvelope)
    // SubTable 'Opaque Exterior' (pdstOpaque)
    // Entry 'Azimuth [deg]' (pdchOpAzimuth)
    // Entry 'Cardinal Direction' (pdchOpDir)

    // Note: Unused because we don't need SQL
    //// We enable the report we care about, making sure it's the right one
    // EXPECT_EQ("EnvelopeSummary", OutputReportPredefined::reportName(2).name);
    // OutputReportPredefined::reportName(2).show = true;
    //// Write the Predef Tables
    // OutputReportTabular::WritePredefinedTables();

    // Integer to find surfaces
    int i = 1;
    for (const auto &expectedAzimuthToCard : expectedAzimuthToCards) {
        double oriAzimuth = expectedAzimuthToCard.first;
        std::string cardinalDir = expectedAzimuthToCard.second;

        // Internal: Just to ensure that we gets the same one with round
        EXPECT_EQ(format("{:.2R}", round(oriAzimuth * 100.0) / 100.0), format("{:.2R}", oriAzimuth));

        // Wall (odd entries)

        // Internal: Dumb check to ensure we didn't mess up in the indexation
        EXPECT_EQ(oriAzimuth, state->dataSurface->Surface(i).Azimuth) << "Surface Name = " << state->dataSurface->Surface(i).Name;

        // Check that the azimuth entry is the rounded version indeed
        EXPECT_EQ(
            OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchOpAzimuth, state->dataSurface->Surface(i).Name),
            format("{:.2R}", expectedAzimuthToCard.first))
            << "Surface Name = " << state->dataSurface->Surface(i).Name;
        // Check that we do get the expected cardinal direction
        EXPECT_EQ(
            OutputReportPredefined::RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchOpDir, state->dataSurface->Surface(i).Name),
            cardinalDir)
            << "Azimuth was " << expectedAzimuthToCard.first << "for Surface '" << state->dataSurface->Surface(i).Name << "'.";

        // Window (even entries)

        EXPECT_EQ(oriAzimuth, state->dataSurface->Surface(i + 1).Azimuth) << "Surface Name = " << state->dataSurface->Surface(i + 1).Name;

        // Check that the azimuth entry is the rounded version indeed
        EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                      *state, state->dataOutRptPredefined->pdchFenAzimuth, state->dataSurface->Surface(i + 1).Name),
                  format("{:.2R}", expectedAzimuthToCard.first))
            << "Surface Name = " << state->dataSurface->Surface(i + 1).Name;
        // Check that we do get the expected cardinal direction
        EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                      *state, state->dataOutRptPredefined->pdchFenDir, state->dataSurface->Surface(i + 1).Name),
                  cardinalDir)
            << "Azimuth was " << expectedAzimuthToCard.first << "for Surface '" << state->dataSurface->Surface(i + 1).Name << "'.";

        // Increment twice
        i = i + 2;
    }
}

// Test for interior surface report
TEST_F(EnergyPlusFixture, InteriorSurfaceEnvelopeSummaryReport)
{
    // Allocate some needed arrays
    state->dataHeatBal->space.allocate(2);
    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).ListMultiplier = 1;
    state->dataHeatBal->Zone(2).ListMultiplier = 1;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).Name = "A Construction";
    state->dataConstruction->Construct(1).OutsideAbsorpSolar = 0.4;

    state->dataHeatBal->NominalU.allocate(1);
    state->dataHeatBal->NominalU(1) = 0.2;
    state->dataHeatBal->NominalUBeforeAdjusted.allocate(1);
    state->dataHeatBal->NominalUBeforeAdjusted(1) = 0.2;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // first surface, interzonal walls and doors
    for (int i = 1; i <= state->dataSurface->TotSurfaces; i++) {
        state->dataSurface->Surface(i).HeatTransSurf = true;
        state->dataSurface->Surface(i).Azimuth = 180.;
        state->dataSurface->Surface(i).Tilt = 90.;
        state->dataSurface->Surface(i).Construction = 1;
        // odd number - wall, even number - door
        if (i % 2 == 1) {
            state->dataSurface->Surface(i).Name = "Interzonal_Wall_" + fmt::to_string((i + 1) / 2);
            state->dataSurface->Surface(i).GrossArea = 200.;
            state->dataSurface->Surface(i).Class = DataSurfaces::SurfaceClass::Wall;
            state->dataSurface->AllSurfaceListReportOrder.push_back(i);
        } else {
            state->dataSurface->Surface(i).Name = "Interzonal_Door_" + fmt::to_string((i + 1) / 2);
            state->dataSurface->Surface(i).BaseSurfName = state->dataSurface->Surface(i - 1).Name;
            state->dataSurface->Surface(i).BaseSurf = i - 1;
            state->dataSurface->Surface(i).GrossArea = 50.;
            state->dataSurface->Surface(i).Class = DataSurfaces::SurfaceClass::Door;
            state->dataSurface->AllSurfaceListReportOrder.push_back(i);
        }
        if ((i + 1) / 2 == 1) {
            // first pair of wall and door
            state->dataSurface->Surface(i).spaceNum = 1;
            state->dataSurface->Surface(i).Zone = 1;
            state->dataSurface->Surface(i).ExtBoundCond = i + 2;
        } else {
            // second pair of wall and door
            state->dataSurface->Surface(i).spaceNum = 2;
            state->dataSurface->Surface(i).Zone = 2;
            state->dataSurface->Surface(i).ExtBoundCond = i - 2;
        }
    }
    state->dataSurface->Surface(1).ExtBoundCondName = "Interzonal_Wall_2";
    state->dataSurface->Surface(2).ExtBoundCondName = "Interzonal_Door_2";
    state->dataSurface->Surface(3).ExtBoundCondName = "Interzonal_Wall_1";
    state->dataSurface->Surface(4).ExtBoundCondName = "Interzonal_Door_1";

    // Setup pre def tables
    OutputReportPredefined::SetPredefinedTables(*state);

    // Call the routine that fills up the table we care about
    HeatBalanceSurfaceManager::GatherForPredefinedReport(*state);

    // Looking for Report 'EnvelopeSummary' (pdrEnvelope)
    // SubTable 'Opaque Interior' (pdstIntOpaque)

    for (int i = 1; i <= state->dataSurface->TotSurfaces; i++) {
        if (i % 2 == 1) {

            // Wall (odd entries)

            // Check the wall gross area
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpGrArea, state->dataSurface->Surface(i).Name),
                      "200.00");
            // Check the wall net area
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpNetArea, state->dataSurface->Surface(i).Name),
                      "150.00");
            // Check the wall u value
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpUfactNoFilm, state->dataSurface->Surface(i).Name),
                      "0.200");
            // Check the wall construction
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpCons, state->dataSurface->Surface(i).Name),
                      "A Construction");
            // Check the wall reflectance
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpRefl, state->dataSurface->Surface(i).Name),
                      "0.60");
            // Check the tilt
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpTilt, state->dataSurface->Surface(i).Name),
                      "90.00");
            // Check the azimuth
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpAzimuth, state->dataSurface->Surface(i).Name),
                      "180.00");
            // Check cardinal direction
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntOpDir, state->dataSurface->Surface(i).Name),
                      "S");
        } else {

            // Door (even entries)

            // Check the door gross area
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntDrGrArea, state->dataSurface->Surface(i).Name),
                      "50.00");
            // Check the wall u value
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntDrUfactNoFilm, state->dataSurface->Surface(i).Name),
                      "0.200");
            // Check the door construction
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntDrCons, state->dataSurface->Surface(i).Name),
                      "A Construction");
            // Check the door parant surface name
            EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(
                          *state, state->dataOutRptPredefined->pdchIntDrParent, state->dataSurface->Surface(i).Name),
                      state->dataSurface->Surface(i - 1).Name);
        }
    }
}

TEST_F(SQLiteFixture, WriteSourceEnergyEndUseSummary_TestPerArea)
{

    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displaySourceEnergyEndUseSummary = true;

    // DetermineBuildingFloorArea

    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 2;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 2;

    // Loads
    state->dataHeatBal->TotLights = 3;
    state->dataHeatBal->Lights.allocate(state->dataHeatBal->TotLights);

    state->dataHeatBal->TotPeople = 3;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);

    state->dataHeatBal->TotElecEquip = 3;
    state->dataHeatBal->ZoneElectric.allocate(state->dataHeatBal->TotElecEquip);

    state->dataHeatBal->Lights(1).ZonePtr = 1;
    state->dataHeatBal->Lights(1).DesignLevel = 1000.0;
    state->dataHeatBal->Lights(2).ZonePtr = 2;
    state->dataHeatBal->Lights(2).DesignLevel = 100.0;
    state->dataHeatBal->Lights(3).ZonePtr = 3;
    state->dataHeatBal->Lights(3).DesignLevel = 10.0;

    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 10.0;
    state->dataHeatBal->People(2).ZonePtr = 2;
    state->dataHeatBal->People(2).NumberOfPeople = 5.0;
    state->dataHeatBal->People(3).ZonePtr = 3;
    state->dataHeatBal->People(3).NumberOfPeople = 1.0;

    state->dataHeatBal->ZoneElectric(1).ZonePtr = 1;
    state->dataHeatBal->ZoneElectric(1).DesignLevel = 500.0;
    state->dataHeatBal->ZoneElectric(2).ZonePtr = 2;
    state->dataHeatBal->ZoneElectric(2).DesignLevel = 50.0;
    state->dataHeatBal->ZoneElectric(3).ZonePtr = 3;
    state->dataHeatBal->ZoneElectric(3).DesignLevel = 5.0;

    // zone
    state->dataGlobal->NumOfZones = 3;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->Zone(1).FloorArea = 1000.;
    state->dataHeatBal->Zone(1).Volume = 2000.;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 800.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    state->dataHeatBal->Zone(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 0; // Unconditioned
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).Multiplier = 1.;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.;
    state->dataHeatBal->Zone(2).FloorArea = 100.;
    state->dataHeatBal->Zone(2).Volume = 200.;
    state->dataHeatBal->Zone(2).ExtGrossWallArea = 80.;
    state->dataHeatBal->Zone(2).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(2).ExtWindowArea = 0.0;

    state->dataHeatBal->Zone(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(3).isPartOfTotalArea = false;
    state->dataHeatBal->Zone(3).Multiplier = 1.;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.;
    state->dataHeatBal->Zone(3).FloorArea = 10.;
    state->dataHeatBal->Zone(3).Volume = 20.;
    state->dataHeatBal->Zone(3).ExtGrossWallArea = 8.;
    state->dataHeatBal->Zone(3).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(3).ExtWindowArea = 0.0;

    // Gross takes all that are PartOfTot
    Real64 expectedBuildingGrossFloorArea = state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea;
    // Conditioned takes only PartOfTot AND COnditioned
    Real64 expectedBuildingConditionedFloorArea = state->dataHeatBal->Zone(1).FloorArea;

    // Assume that we only have electricity with a value of 3.6e6 * 1e4 J =10.000 kWh.
    // And that this only comes for a single end use state->dataGlobalConst->iEndUse.at(Constant::EndUse::Heating)=1
    state->dataOutRptTab->gatherEndUseBySourceBEPS(1, static_cast<int>(Constant::EndUse::Heating) + 1) = 3.6e10;
    state->dataOutRptTab->gatherTotalsBySourceBEPS(1) = 3.6e10;
    Real64 eleckWh = 1e4;

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoKWH;

    // Now we're ready to call the actual function of interest
    OutputReportTabular::WriteSourceEnergyEndUseSummary(*state);

    // Before we test the reporting itself, we check that DetermineBuildingFloorArea (called from WriteSourceEnergyEndUseSummary)
    // actually did what we expected
    EXPECT_EQ(expectedBuildingGrossFloorArea, state->dataOutRptTab->buildingGrossFloorArea);
    EXPECT_EQ(expectedBuildingConditionedFloorArea, state->dataOutRptTab->buildingConditionedFloorArea);

    // Now we test the reporting itself:
    // We consistently test in the same report (three different tables) and at the same column for fuel = Elec
    const std::string reportName = "SourceEnergyEndUseComponentsSummary";
    const std::string columnName = "Source Electricity";

    // We test for Heating and Total, since they should be the same
    std::vector<std::string> testRowNames = {"Heating", "Total Source Energy End Use Components"};

    // TableName, value
    std::vector<std::tuple<std::string, Real64>> results({
        {"Source Energy End Use Components Summary", eleckWh},
        {"Source Energy End Use Components Per Conditioned Floor Area", 10000.0 / expectedBuildingConditionedFloorArea},
        {"Source Energy End Use Components Per Total Floor Area", 10000.0 / expectedBuildingGrossFloorArea},
    });

    for (auto &v : results) {

        std::string tableName = std::get<0>(v);
        Real64 expectedValue = std::get<1>(v);

        for (auto &rowName : testRowNames) {
            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE ReportName = '" +
                              reportName +
                              "'"
                              "  AND TableName = '" +
                              tableName +
                              "'"
                              "  AND RowName = '" +
                              rowName + "'" + "  AND ColumnName = '" + columnName + "'");

            Real64 return_val = execAndReturnFirstDouble(query);

            // Add informative message if failed
            EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
        }
    }
}

TEST_F(SQLiteFixture, OutputReportTabular_EndUseBySubcategorySQL)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularBEPS = true;
    state->dataOutRptTab->displayDemandEndUse = true;
    state->dataOutRptTab->displayLEEDSummary = true;

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoKWH;

    // Needed to avoid crash (from ElectricPowerServiceManager.hh)
    createFacilityElectricPowerServiceObject(*state);

    SetPredefinedTables(*state);

    Real64 extLitUse = 1e8;
    Real64 CoalHeating = 2e8;
    Real64 GasolineHeating = 3e8;
    Real64 PropaneHeating = 4e8;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "AnotherEndUseSubCat");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite3",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Coal Energy",
                        Constant::Units::J,
                        CoalHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite4",
                        Constant::eResource::Coal,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Gasoline Energy",
                        Constant::Units::J,
                        GasolineHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite5",
                        Constant::eResource::Gasoline,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Propane Energy",
                        Constant::Units::J,
                        PropaneHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite6",
                        Constant::eResource::Propane,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->MinutesInTimeStep = state->dataGlobal->TimeStepZone * 60;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 3600.0;
    state->dataOutRptTab->displayTabularBEPS = true;
    // OutputProcessor::TimeValue.allocate(2);

    auto timeStep = 1.0;

    SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, timeStep);
    SetupTimePointers(*state, OutputProcessor::TimeStepType::System, timeStep);

    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep = 60;
    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep = 60;

    GetInputOutputTableSummaryReports(*state);

    state->dataEnvrn->Month = 12;

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 3, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 2, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 1, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 6, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 4, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 2, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 9, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 6, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 3, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    OutputReportTabular::WriteBEPSTable(*state);
    OutputReportTabular::WriteDemandEndUseSummary(*state);

    // We test for Heating and Total, since they should be the same
    std::vector<std::string> testReportNames = {"AnnualBuildingUtilityPerformanceSummary", "DemandEndUseComponentsSummary"};
    std::vector<std::string> endUseSubCategoryNames = {"General", "AnotherEndUseSubCat"};

    std::string endUseName = "Exterior Lighting";
    std::string endUseSubCategoryName = "AnotherEndUseSubCat";
    std::string rowName = endUseName + ":" + endUseSubCategoryName;
    std::string columnName = "Electricity";

    for (auto &endUseSubCategoryName : endUseSubCategoryNames) {
        for (auto &reportName : testReportNames) {

            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE TableName = 'End Uses By Subcategory'"
                              "  AND ColumnName = 'Electricity'"
                              "  AND ReportName = '" +
                              reportName +
                              "'"
                              "  AND RowName = '" +
                              endUseName + ":" + endUseSubCategoryName + "'"); // Now Like 'Exterior Lighting:General'

            auto result = queryResult(query, "TabularDataWithStrings");

            ASSERT_EQ(1ul, result.size()) << "Query crashed for reportName=" << reportName;
        }
    }

    for (auto &reportName : testReportNames) {

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ColumnName = 'Electricity'"
                          "  AND ReportName = '" +
                          reportName +
                          "'"
                          "  AND RowName = '" +
                          endUseName + "'");

        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(1ul, result.size()) << "Query crashed for reportName=" << reportName;
    }

    // Specifically get the electricity usage for End Use = Exterior Lighting, and End Use Subcat = AnotherEndUseSubCat,
    // and make sure it's the right number that's returned
    std::string query("SELECT Value From TabularDataWithStrings"
                      "  WHERE TableName = 'End Uses By Subcategory'"
                      "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                      "  AND ColumnName = 'Electricity'"
                      "  AND RowName = 'Exterior Lighting:AnotherEndUseSubCat'");
    Real64 return_val = execAndReturnFirstDouble(query);

    EXPECT_NEAR(extLitUse * 3 / 3.6e6, return_val, 0.01) << "Failed for query: " << query;

    // Get all Interior Lighting End Uses (all subcats) for Electricity
    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Electricity'"
                          "  AND RowName LIKE 'Exterior Lighting:%'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(2u, result.size()) << "Failed for query: " << query;
    }

    // Get all subcat usage for all fuels (13)
    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND RowName = 'Exterior Lighting:AnotherEndUseSubCat'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }

    // Specifically get the each fuel (Coal, Gasoline, and Propane) usage for End Use = Heating,
    // and make sure it's the right number that's returned

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Coal'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val1 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        EXPECT_NEAR(CoalHeating * 3 / 3.6e6, return_val1, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Gasoline'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val2 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        EXPECT_NEAR(GasolineHeating * 3 / 3.6e6, return_val2, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Propane'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val3 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        EXPECT_NEAR(PropaneHeating * 3 / 3.6e6, return_val3, 0.01) << "Failed for query: " << query;
    }

    // Check the heating category has the result size of 13 (including all disaggregated additional fuels) in both reports)

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'DemandEndUseComponentsSummary'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }
}

TEST_F(EnergyPlusFixture, StatFileCharacterMatching)
{

    auto getLineType = [](const std::string statLine) -> StatLineType {
        StatLineType lineTypeReturn = StatLineType::Initialized;
        bool desCondLinePassed = false;
        bool htgDesignLinePassed = false;
        bool clgDesignLinePassed = false;
        bool isKoppen = false;
        bool insideLiquidPrecipitation = false;
        parseStatLine(statLine, lineTypeReturn, desCondLinePassed, htgDesignLinePassed, clgDesignLinePassed, isKoppen, insideLiquidPrecipitation);
        return lineTypeReturn;
    };

    std::string coolingLineGoodDegrees = "    - 2874 annual (standard) cooling degree-days (10°C baseline)";
    EXPECT_ENUM_EQ(StatLineType::StdCDDLine, getLineType(coolingLineGoodDegrees));

    std::string coolingLineBadDegrees = "    - 2874 annual (standard) cooling degree-days (10_BADDEGREESYMBOL_C baseline)";
    EXPECT_ENUM_EQ(StatLineType::StdCDDLine, getLineType(coolingLineGoodDegrees));

    std::string koppenLineWithDots = " - Climate type \"Cfa\" (Köppen classification)**";
    EXPECT_ENUM_EQ(StatLineType::KoppenLine, getLineType(koppenLineWithDots));

    std::string koppenLineNoDots = " - Climate type \"Cfa\" (Koppen classification)**";
    EXPECT_ENUM_EQ(StatLineType::KoppenLine, getLineType(koppenLineNoDots));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_GetDelaySequencesSurfaceOrder_test)
{

    int coolDesSelected = 1;
    int iZone = 1;
    state->dataEnvrn->TotDesDays = 2;
    state->dataEnvrn->TotRunDesPersDays = 3;
    state->dataGlobal->TimeStepsInHour = 4;

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataGlobal->numSpaces = 1;
    state->dataHeatBal->space.allocate(state->dataGlobal->numSpaces);
    state->dataViewFactor->NumOfRadiantEnclosures = 1;

    state->dataHeatBal->Zone(iZone).spaceIndexes.emplace_back(iZone);
    state->dataHeatBal->space(iZone).HTSurfaceFirst = 1;
    state->dataHeatBal->space(iZone).HTSurfaceLast = 4;
    state->dataHeatBal->space(iZone).radiantEnclosureNum = 1;
    int radEnclosureNum = 1;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    Array1D<Real64> peopleDelaySeq;
    peopleDelaySeq.allocate(state->dataGlobal->TimeStepsInHour * 24);
    peopleDelaySeq = 0.;

    Array1D<Real64> peopleDelaySeqCool;
    peopleDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    peopleDelaySeqCool = 0.;

    Array1D<Real64> equipDelaySeqCool;
    equipDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    equipDelaySeqCool = 0.;

    Array1D<Real64> hvacLossDelaySeqCool;
    hvacLossDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    hvacLossDelaySeqCool = 0.;

    Array1D<Real64> powerGenDelaySeqCool;
    powerGenDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    powerGenDelaySeqCool = 0.;

    Array1D<Real64> lightDelaySeqCool;
    lightDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    lightDelaySeqCool = 0.;

    Array1D<Real64> feneSolarDelaySeqCool;
    feneSolarDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24);
    feneSolarDelaySeqCool = 0.;

    Array2D<Real64> surfDelaySeqCool;
    surfDelaySeqCool.allocate(state->dataGlobal->TimeStepsInHour * 24, state->dataSurface->TotSurfaces);
    surfDelaySeqCool = 0.0;

    AllocateLoadComponentArrays(*state);

    // Set surface values
    std::vector<Real64> surfBaseValue{100.0, 200.0, 300.0, 400.0};
    state->dataSurface->Surface(1).Area = 10;
    state->dataSurface->Surface(2).Area = 20;
    state->dataSurface->Surface(3).Area = 30;
    state->dataSurface->Surface(4).Area = 40;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(4).HeatTransSurf = false;
    state->dataSurface->Surface(1).Class = SurfaceClass::Window;
    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(3).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(4).Class = SurfaceClass::Shading;
    state->dataSurface->Surface(1).RadEnclIndex = radEnclosureNum;
    state->dataSurface->Surface(2).RadEnclIndex = radEnclosureNum;
    state->dataSurface->Surface(3).RadEnclIndex = radEnclosureNum;
    state->dataSurface->Surface(4).RadEnclIndex = radEnclosureNum;

    auto &znCL = state->dataOutRptTab->znCompLoads;
    // auto &znCLDay = znCL[coolDesSelected - 1];
    for (int jSurf = 1; jSurf <= 4; ++jSurf) {
        for (int step = 1; step <= 10; ++step) {
            // auto &znCLDayTS = znCLDay.ts[step - 1].spacezone[iZone - 1];
            auto &surfCLDayTS = state->dataOutRptTab->surfCompLoads[coolDesSelected - 1].ts[step - 1].surf[jSurf - 1];
            surfCLDayTS.TMULTseq = 0.1 * step;
            surfCLDayTS.ITABSFseq = 0.2 * step * surfBaseValue[jSurf - 1];
            state->dataOutRptTab->decayCurveCool(step, jSurf) = 0.3 * step * surfBaseValue[jSurf - 1];
        }
    }

    for (int step = 1; step <= 10; ++step) {
        auto &enclCLDayTS = state->dataOutRptTab->enclCompLoads[coolDesSelected - 1].ts[step - 1].encl[radEnclosureNum - 1];
        enclCLDayTS.peopleRadSeq = 0.4 * step;
        enclCLDayTS.equipRadSeq = 0.5 * step;
        enclCLDayTS.hvacLossRadSeq = 0.6 * step;
        enclCLDayTS.powerGenRadSeq = 0.7 * step;
        enclCLDayTS.lightLWRadSeq = 0.8 * step;
    }

    GetDelaySequences(*state,
                      coolDesSelected,
                      true,
                      iZone,
                      peopleDelaySeqCool,
                      equipDelaySeqCool,
                      hvacLossDelaySeqCool,
                      powerGenDelaySeqCool,
                      lightDelaySeqCool,
                      feneSolarDelaySeqCool,
                      znCL,
                      surfDelaySeqCool);

    // Save some results from first pass
    Real64 peopleDelaySeqCool1 = peopleDelaySeqCool(1);
    Real64 equipDelaySeqCool1 = equipDelaySeqCool(2);
    Real64 hvacLossDelaySeqCool1 = hvacLossDelaySeqCool(3);
    Real64 powerGenDelaySeqCool1 = powerGenDelaySeqCool(4);
    Real64 lightDelaySeqCool1 = lightDelaySeqCool(5);
    Real64 feneSolarDelaySeqCool1 = feneSolarDelaySeqCool(6);

    // Rearrange surface values
    surfBaseValue = {300.0, 100.0, 400.0, 200.0};
    state->dataSurface->Surface(2).Area = 10;
    state->dataSurface->Surface(4).Area = 20;
    state->dataSurface->Surface(1).Area = 30;
    state->dataSurface->Surface(3).Area = 40;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(3).HeatTransSurf = false;
    state->dataSurface->Surface(2).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).Class = SurfaceClass::Floor;
    state->dataSurface->Surface(3).Class = SurfaceClass::Shading;
    state->dataSurface->Surface(2).RadEnclIndex = 1;
    state->dataSurface->Surface(4).RadEnclIndex = 1;
    state->dataSurface->Surface(1).RadEnclIndex = 1;
    state->dataSurface->Surface(3).RadEnclIndex = 1;

    for (int jSurf = 1; jSurf <= 4; ++jSurf) {
        for (int step = 1; step <= 10; ++step) {
            // auto &znCLDayTS = znCLDay.ts[step - 1].spacezone[iZone - 1];
            auto &surfCLDayTS = state->dataOutRptTab->surfCompLoads[coolDesSelected - 1].ts[step - 1].surf[jSurf - 1];
            surfCLDayTS.TMULTseq = 0.1 * step;
            surfCLDayTS.ITABSFseq = 0.2 * step * surfBaseValue[jSurf - 1];
            state->dataOutRptTab->decayCurveCool(step, jSurf) = 0.3 * step * surfBaseValue[jSurf - 1];
        }
    }

    for (int step = 1; step <= 10; ++step) {
        auto &enclCLDayTS = state->dataOutRptTab->enclCompLoads[coolDesSelected - 1].ts[step - 1].encl[radEnclosureNum - 1];
        enclCLDayTS.peopleRadSeq = 0.4 * step;
        enclCLDayTS.equipRadSeq = 0.5 * step;
        enclCLDayTS.hvacLossRadSeq = 0.6 * step;
        enclCLDayTS.powerGenRadSeq = 0.7 * step;
        enclCLDayTS.lightLWRadSeq = 0.8 * step;
    }

    GetDelaySequences(*state,
                      coolDesSelected,
                      true,
                      iZone,
                      peopleDelaySeqCool,
                      equipDelaySeqCool,
                      hvacLossDelaySeqCool,
                      powerGenDelaySeqCool,
                      lightDelaySeqCool,
                      feneSolarDelaySeqCool,
                      znCL,
                      surfDelaySeqCool);

    // Save some results from second pass
    Real64 peopleDelaySeqCool2 = peopleDelaySeqCool(1);
    Real64 equipDelaySeqCool2 = equipDelaySeqCool(2);
    Real64 hvacLossDelaySeqCool2 = hvacLossDelaySeqCool(3);
    Real64 powerGenDelaySeqCool2 = powerGenDelaySeqCool(4);
    Real64 lightDelaySeqCool2 = lightDelaySeqCool(5);
    Real64 feneSolarDelaySeqCool2 = feneSolarDelaySeqCool(6);

    EXPECT_EQ(peopleDelaySeqCool1, peopleDelaySeqCool2);
    EXPECT_EQ(equipDelaySeqCool1, equipDelaySeqCool2);
    EXPECT_EQ(hvacLossDelaySeqCool1, hvacLossDelaySeqCool2);
    EXPECT_EQ(powerGenDelaySeqCool1, powerGenDelaySeqCool2);
    EXPECT_EQ(lightDelaySeqCool1, lightDelaySeqCool2);
    EXPECT_EQ(feneSolarDelaySeqCool1, feneSolarDelaySeqCool2);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_ConfirmConversionFactors)
{

    Real64 curSourceFactor;
    bool fuelFactorUsed;
    bool fFScheduleUsed;
    Sched::Schedule *ffSched;

    Pollution::GetFuelFactorInfo(*state, Constant::eFuel::DistrictHeatingSteam, fuelFactorUsed, curSourceFactor, fFScheduleUsed, &ffSched);
    EXPECT_EQ(curSourceFactor, 1.2);
}

TEST_F(EnergyPlusFixture, OutputReportTabular_GatherHeatGainReport)
{
    state->dataGlobal->DoWeathSim = true;

    state->dataOutRptPredefined->pdrSensibleGain = 1;
    state->dataOutRptPredefined->reportName.allocate(1);
    state->dataOutRptPredefined->reportName(state->dataOutRptPredefined->pdrSensibleGain).show = true;

    state->dataHVACGlobal->TimeStepSys = 10.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    state->dataGlobal->TimeStepZone = 20.0;

    state->dataHeatBal->ZonePreDefRep.allocate(1);
    state->dataDefineEquipment->AirDistUnit.allocate(1);
    state->dataDefineEquipment->AirDistUnit(1).ZoneNum = 1;
    state->dataDefineEquipment->AirDistUnit(1).HeatGain = 1000.0;
    state->dataDefineEquipment->AirDistUnit(1).CoolGain = 2000.0;
    state->dataDefineEquipment->AirDistUnit(1).HeatRate = 3.0;
    state->dataDefineEquipment->AirDistUnit(1).CoolRate = 4.0;

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Multiplier = 1;
    state->dataHeatBal->Zone(1).ListMultiplier = 1;

    state->dataHeatBal->ZoneRpt.allocate(1);
    state->dataHeatBal->ZnAirRpt.allocate(1);

    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy = 0.0;
    state->dataHeatBal->ZoneWinHeatLossRepEnergy.allocate(1);
    state->dataHeatBal->ZoneWinHeatLossRepEnergy = 0.0;

    GatherHeatGainReport(*state, OutputProcessor::TimeStepType::System);

    EXPECT_EQ(1.0 * state->dataHVACGlobal->TimeStepSysSec, state->dataHeatBal->ZonePreDefRep(1).SHGSAnZoneEqHt);
    EXPECT_EQ(0.0 * state->dataHVACGlobal->TimeStepSysSec, state->dataHeatBal->ZonePreDefRep(1).SHGSAnZoneEqCl);
    EXPECT_EQ(1000.0, state->dataHeatBal->ZonePreDefRep(1).SHGSAnHvacATUHt);
    EXPECT_EQ(-2000.0, state->dataHeatBal->ZonePreDefRep(1).SHGSAnHvacATUCl);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_8317_ValidateOutputTableMonthly)
{
    // Test for #8317 - Output:Table:Monthly does not warn about invalid variable or meter name
    std::string const idf_objects = delimited_string({

        "Schedule:Compact,",
        " OnSched,                  !- Name",
        " Fraction,                 !- Schedule Type Limits Name",
        " Through: 12/31,           !- Field 1",
        " For: AllDays,             !- Field 2",
        " Until: 24:00, 1.0;        !- Field 3",

        "ScheduleTypeLimits,",
        " Fraction,                 !- Name",
        " 0.0,                      !- Lower Limit Value",
        " 1.0,                      !- Upper Limit Value",
        " CONTINUOUS;               !- Numeric Type",

        "Output:Table:Monthly,",
        "  My Report,                        !- Name",
        "  2,                                !- Digits After Decimal",
        "  Heating:Gas,                      !- Variable or Meter 1 Name", // A bad meter name
        "  SumOrAverage,                     !- Aggregation Type for Variable or Meter 1",
        "  Exterior Lights Electric Power,   !- Variable or Meter 2 Name", // A bad variable name
        "  Maximum,                          !- Aggregation Type for Variable or Meter 2",
        "  AlwaysOn,                         !- Variable or Meter 3 Name", // A bad name (eg: Schedule)
        "  Maximum,                          !- Aggregation Type for Variable or Meter 3",
        "  Exterior Lights Electricity Rate, !- Variable or Meter 4 Name", // A good variable name
        "  Minimum,                          !- Aggregation Type for Variable or Meter 4",
        "  OnSched,                          !- Variable or Meter 5 Name", // A good schedule name
        "  Minimum,                          !- Aggregation Type for Variable or Meter 5",
        "  Heating:NaturalGas,               !- Variable or Meter 6 Name", // A good meter name
        "  SumOrAverage;                     !- Aggregation Type for Variable or Meter 6",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Rate",
                        Constant::Units::W,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = false;

    InitializeOutput(*state);

    std::string endUseSub("");
    std::string const zoneName("");
    std::string const spaceType("");

    // AttachMeters(*state, Constant::Units::J, resource, sovEndUseCat, endUseSub, sovGroup, zoneName, spaceType, -1);
    Meter *meter1 = new Meter("NATURALGAS:FACILITY");
    meter1->resource = Constant::eResource::NaturalGas;
    meter1->endUseCat = OutputProcessor::EndUseCat::Invalid;
    state->dataOutputProcessor->meters.push_back(meter1);
    state->dataOutputProcessor->meterMap.insert_or_assign("NATURALGAS:FACILITY", state->dataOutputProcessor->meters.size() - 1);

    Meter *meter2 = new Meter("HEATING:NATURALGAS");
    meter2->resource = Constant::eResource::NaturalGas;
    meter2->endUseCat = OutputProcessor::EndUseCat::Heating;
    state->dataOutputProcessor->meters.push_back(meter2);
    state->dataOutputProcessor->meterMap.insert_or_assign("HEATING:NATURALGAS", state->dataOutputProcessor->meters.size() - 1);

    OutputReportTabular::GetInputTabularMonthly(*state);
    OutputReportTabular::InitializeTabularMonthly(*state);

    std::string const expected_error = delimited_string({
        "   ** Warning ** Processing Monthly Tabular Reports: Variable names not valid for this simulation",
        "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual variables.",
    });
    compare_err_stream(expected_error);
}

TEST_F(SQLiteFixture, ORT_DualUnits_Process_Regular_Case_1)
{
    // Test units to ensure proper Output:SQLite unit conversion handling

    // Test the regular scenario (No missing or default values) Case 1: UseoutputControlTableStyle
    std::string const idf_objects =
        delimited_string({"Output:SQLite,", "SimpleAndTabular, !-Option Type", "UseOutputControlTableStyle; !-Tabular Unit Conversion"});

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_ENUM_EQ(state->dataOutRptTab->unitsStyle_SQLite, UnitsStyle::NotFound);
    ASSERT_NE(state->dataSQLiteProcedures->sqlite.get(), nullptr);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite(), true);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite(), true);
}

TEST_F(SQLiteFixture, ORT_DualUnits_Process_Regular_Case_2)
{
    // Test the regular scenario (No missing or default values) Case 2: InchPound
    std::string const idf_objects = delimited_string({"Output:SQLite,", "SimpleAndTabular, !-Option Type", "InchPound; !-Tabular Unit Conversion"});

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_ENUM_EQ(state->dataOutRptTab->unitsStyle_SQLite, UnitsStyle::InchPound);
    ASSERT_NE(state->dataSQLiteProcedures->sqlite.get(), nullptr);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite(), true);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite(), true);
}

TEST_F(SQLiteFixture, ORT_DualUnits_Process_Regular_Case_3)
{
    // Test the regular scenario (No missing or default values) Case 3: None
    std::string const idf_objects = delimited_string({"Output:SQLite,", "SimpleAndTabular, !-Option Type", "None; !-Tabular Unit Conversion"});

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_ENUM_EQ(state->dataOutRptTab->unitsStyle_SQLite, UnitsStyle::None);
    ASSERT_NE(state->dataSQLiteProcedures->sqlite.get(), nullptr);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite(), true);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite(), true);
}

TEST_F(SQLiteFixture, ORT_DualUnits_Process_Missing_Case_1)
{
    // Test the missing scenario (has missing or default fields) Case 1: Default empty input
    std::string const idf_objects = delimited_string({"Output:SQLite,", "SimpleAndTabular, !-Option Type", "; !-Tabular Unit Conversion"});

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_ENUM_EQ(state->dataOutRptTab->unitsStyle_SQLite, UnitsStyle::NotFound);
    ASSERT_NE(state->dataSQLiteProcedures->sqlite.get(), nullptr);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite(), true);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite(), true);
}

TEST_F(SQLiteFixture, ORT_DualUnits_Process_Missing_Case_2)
{
    // Test the missing scenario (has missing or default fields) Case 2: Missing A2 field at all
    // This will allow a backward compatiability: even an earlier version format can be correctly handeled.
    std::string const idf_objects = delimited_string({"Output:SQLite,", "SimpleAndTabular; !-Option Type"});

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_ENUM_EQ(state->dataOutRptTab->unitsStyle_SQLite, UnitsStyle::NotFound);
    ASSERT_NE(state->dataSQLiteProcedures->sqlite.get(), nullptr);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite(), true);
    EXPECT_EQ(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite(), true);
}

TEST_F(SQLiteFixture, ORT_DualUnits_Heat_Emission)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayHeatEmissionsSummary = true;

    // DetermineBuildingFloorArea
    state->dataHeatBal->BuildingPreDefRep.emiEnvelopConv = 1000.00;    // * energyconversion, 2);
    state->dataHeatBal->BuildingPreDefRep.emiZoneExfiltration = 50.00; // * energyconversion, 2);
    state->dataHeatBal->BuildingPreDefRep.emiZoneExhaust = 200.00;     // * energyconversion, 2);
    state->dataHeatBal->BuildingPreDefRep.emiHVACRelief = 500.00;      // * energyconversion, 2);
    state->dataHeatBal->BuildingPreDefRep.emiHVACReject = 1750.00;     // * energyconversion, 2);
    state->dataHeatBal->BuildingPreDefRep.emiTotHeat = 4000.00;        // * energyconversion, 2);

    // Test Combination 0: GJ
    state->dataOutRptTab->unitsStyle = UnitsStyle::JtoGJ;
    state->dataOutRptTab->unitsStyle_SQLite = UnitsStyle::JtoGJ;
    Real64 energyconversion = 1.0;

    WriteHeatEmissionTable(*state);

    // Now test the reporting:
    const std::string reportName = "AnnualHeatEmissionsReport";
    const std::string tableName = "Annual Heat Emissions Summary";
    std::string unitsName = "GJ";

    // Test the row of heat emissions
    std::vector<std::string> testRowNames = {"Heat Emissions"};

    // TableName, value
    std::vector<std::tuple<std::string, Real64>> results0({
        {"Envelope Convection", state->dataHeatBal->BuildingPreDefRep.emiEnvelopConv * energyconversion},
        {"Zone Exfiltration", state->dataHeatBal->BuildingPreDefRep.emiZoneExfiltration * energyconversion},
        {"Zone Exhaust Air", state->dataHeatBal->BuildingPreDefRep.emiZoneExhaust * energyconversion},
        {"HVAC Relief Air", state->dataHeatBal->BuildingPreDefRep.emiHVACRelief * energyconversion},
        {"HVAC Reject Heat", state->dataHeatBal->BuildingPreDefRep.emiHVACReject * energyconversion},
        {"Total", state->dataHeatBal->BuildingPreDefRep.emiTotHeat * energyconversion},

    });

    for (auto &v : results0) {

        std::string columnName = std::get<0>(v);
        Real64 expectedValue = std::get<1>(v);

        for (auto &rowName : testRowNames) {
            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE ReportName = '" +
                              reportName +
                              "'"
                              "  AND TableName = '" +
                              tableName +
                              "'"
                              "  AND RowName = '" +
                              rowName +
                              "'"
                              "  AND ColumnName = '" +
                              columnName +
                              "'"
                              "  AND Units = '" +
                              unitsName + "'");

            Real64 return_val = execAndReturnFirstDouble(query);

            // Add informative message if failed
            EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
        }
    }

    // Test Combination 1: None
    state->dataOutRptTab->unitsStyle = UnitsStyle::None;
    state->dataOutRptTab->unitsStyle_SQLite = UnitsStyle::None;
    energyconversion = 1.0;

    WriteHeatEmissionTable(*state);

    // Now test the reporting:
    // const std::string reportName = "AnnualHeatEmissionsReport";
    // const std::string tableName = "Annual Heat Emissions Summary";
    unitsName = "GJ";

    // Test the row of heat emissions
    // std::vector<std::string> testRowNames = {"Heat Emissions"};

    // TableName, value
    std::vector<std::tuple<std::string, Real64>> results1({
        {"Envelope Convection", state->dataHeatBal->BuildingPreDefRep.emiEnvelopConv * energyconversion},
        {"Zone Exfiltration", state->dataHeatBal->BuildingPreDefRep.emiZoneExfiltration * energyconversion},
        {"Zone Exhaust Air", state->dataHeatBal->BuildingPreDefRep.emiZoneExhaust * energyconversion},
        {"HVAC Relief Air", state->dataHeatBal->BuildingPreDefRep.emiHVACRelief * energyconversion},
        {"HVAC Reject Heat", state->dataHeatBal->BuildingPreDefRep.emiHVACReject * energyconversion},
        {"Total", state->dataHeatBal->BuildingPreDefRep.emiTotHeat * energyconversion},

    });

    for (auto &v : results1) {

        std::string columnName = std::get<0>(v);
        Real64 expectedValue = std::get<1>(v);

        for (auto &rowName : testRowNames) {
            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE ReportName = '" +
                              reportName +
                              "'"
                              "  AND TableName = '" +
                              tableName +
                              "'"
                              "  AND RowName = '" +
                              rowName +
                              "'"
                              "  AND ColumnName = '" +
                              columnName +
                              "'"
                              "  AND Units = '" +
                              unitsName + "'");

            Real64 return_val = execAndReturnFirstDouble(query);

            // Add informative message if failed
            EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
        }
    }

    // Test Combination 2:
    state->dataOutRptTab->unitsStyle = UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = UnitsStyle::InchPound;

    // Test 2.5:
    // Actually here is an additonal test unit for the getSpecificUnitDivider:
    SetupUnitConversions(*state);
    Real64 rconv = getSpecificUnitDivider(*state, "GJ", "kBtu");
    energyconversion = 1.0 / rconv; // 948.45
    EXPECT_NEAR(energyconversion, 948.0, 0.5);

    WriteHeatEmissionTable(*state);

    // Now test the reporting:
    // const std::string reportName = "AnnualHeatEmissionsReport";
    // const std::string tableName = "Annual Heat Emissions Summary";
    unitsName = "kBtu";

    // Test the row of heat emissions
    // std::vector<std::string> testRowNames = {"Heat Emissions"};

    // TableName, value
    std::vector<std::tuple<std::string, Real64>> results2({
        {"Envelope Convection", state->dataHeatBal->BuildingPreDefRep.emiEnvelopConv * energyconversion},
        {"Zone Exfiltration", state->dataHeatBal->BuildingPreDefRep.emiZoneExfiltration * energyconversion},
        {"Zone Exhaust Air", state->dataHeatBal->BuildingPreDefRep.emiZoneExhaust * energyconversion},
        {"HVAC Relief Air", state->dataHeatBal->BuildingPreDefRep.emiHVACRelief * energyconversion},
        {"HVAC Reject Heat", state->dataHeatBal->BuildingPreDefRep.emiHVACReject * energyconversion},
        {"Total", state->dataHeatBal->BuildingPreDefRep.emiTotHeat * energyconversion},

    });

    for (auto &v : results2) {

        std::string columnName = std::get<0>(v);
        Real64 expectedValue = std::get<1>(v);

        for (auto &rowName : testRowNames) {
            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE ReportName = '" +
                              reportName +
                              "'"
                              "  AND TableName = '" +
                              tableName +
                              "'"
                              "  AND RowName = '" +
                              rowName +
                              "'"
                              "  AND ColumnName = '" +
                              columnName +
                              "'"
                              "  AND Units = '" +
                              unitsName + "'");

            Real64 return_val = execAndReturnFirstDouble(query);

            // Add informative message if failed
            EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
        }
    }
}

TEST_F(SQLiteFixture, WriteSourceEnergyEndUseSummary_DualUnits)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displaySourceEnergyEndUseSummary = true;

    // DetermineBuildingFloorArea
    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 2;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 2;

    // Loads
    state->dataHeatBal->TotLights = 3;
    state->dataHeatBal->Lights.allocate(state->dataHeatBal->TotLights);

    state->dataHeatBal->TotPeople = 3;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);

    state->dataHeatBal->TotElecEquip = 3;
    state->dataHeatBal->ZoneElectric.allocate(state->dataHeatBal->TotElecEquip);

    state->dataHeatBal->Lights(1).ZonePtr = 1;
    state->dataHeatBal->Lights(1).DesignLevel = 1000.0;
    state->dataHeatBal->Lights(2).ZonePtr = 2;
    state->dataHeatBal->Lights(2).DesignLevel = 100.0;
    state->dataHeatBal->Lights(3).ZonePtr = 3;
    state->dataHeatBal->Lights(3).DesignLevel = 10.0;

    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 10.0;
    state->dataHeatBal->People(2).ZonePtr = 2;
    state->dataHeatBal->People(2).NumberOfPeople = 5.0;
    state->dataHeatBal->People(3).ZonePtr = 3;
    state->dataHeatBal->People(3).NumberOfPeople = 1.0;

    state->dataHeatBal->ZoneElectric(1).ZonePtr = 1;
    state->dataHeatBal->ZoneElectric(1).DesignLevel = 500.0;
    state->dataHeatBal->ZoneElectric(2).ZonePtr = 2;
    state->dataHeatBal->ZoneElectric(2).DesignLevel = 50.0;
    state->dataHeatBal->ZoneElectric(3).ZonePtr = 3;
    state->dataHeatBal->ZoneElectric(3).DesignLevel = 5.0;

    // zone
    state->dataGlobal->NumOfZones = 3;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->Zone(1).FloorArea = 1000.;
    state->dataHeatBal->Zone(1).Volume = 2000.;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 800.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    state->dataHeatBal->Zone(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 0; // Unconditioned
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).Multiplier = 1.;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.;
    state->dataHeatBal->Zone(2).FloorArea = 100.;
    state->dataHeatBal->Zone(2).Volume = 200.;
    state->dataHeatBal->Zone(2).ExtGrossWallArea = 80.;
    state->dataHeatBal->Zone(2).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(2).ExtWindowArea = 0.0;

    state->dataHeatBal->Zone(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(3).isPartOfTotalArea = false;
    state->dataHeatBal->Zone(3).Multiplier = 1.;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.;
    state->dataHeatBal->Zone(3).FloorArea = 10.;
    state->dataHeatBal->Zone(3).Volume = 20.;
    state->dataHeatBal->Zone(3).ExtGrossWallArea = 8.;
    state->dataHeatBal->Zone(3).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(3).ExtWindowArea = 0.0;

    // Gross takes all that are PartOfTot
    Real64 expectedBuildingGrossFloorArea = state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea;
    // Conditioned takes only PartOfTot AND COnditioned
    Real64 expectedBuildingConditionedFloorArea = state->dataHeatBal->Zone(1).FloorArea;

    // Assume that we only have electricity with a value of 3.6e6 * 1e4 J =10.000 kWh.
    // And that this only comes for a single end use static_cast<int>(Constant::EndUse::Heating)=1
    state->dataOutRptTab->gatherEndUseBySourceBEPS(1, static_cast<int>(Constant::EndUse::Heating) + 1) = 3.6e10;
    state->dataOutRptTab->gatherTotalsBySourceBEPS(1) = 3.6e10;
    Real64 eleckWh = 1e4;

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    SetupUnitConversions(*state);
    Real64 largeConv = getSpecificUnitDivider(*state, "J", "kBtu");
    largeConv /= (3.6e6);
    Real64 areaConv = getSpecificUnitDivider(*state, "m2", "ft2");

    // Now we're ready to call the actual function of interest
    OutputReportTabular::WriteSourceEnergyEndUseSummary(*state);

    // Before we test the reporting itself, we check that DetermineBuildingFloorArea (called from WriteSourceEnergyEndUseSummary)
    // actually did what we expected
    EXPECT_EQ(expectedBuildingGrossFloorArea, state->dataOutRptTab->buildingGrossFloorArea);
    EXPECT_EQ(expectedBuildingConditionedFloorArea, state->dataOutRptTab->buildingConditionedFloorArea);

    // expectedBuildingGrossFloorArea /= areaConv;
    // expectedBuildingConditionedFloorArea /= areaConv;

    // eleckWh /= largeConv;

    // Now we test the reporting itself:
    // We consistently test in the same report (three different tables) and at the same column for fuel = Elec
    const std::string reportName = "SourceEnergyEndUseComponentsSummary";
    const std::string columnName = "Source Electricity";

    // We test for Heating and Total, since they should be the same
    std::vector<std::string> testRowNames = {"Heating", "Total Source Energy End Use Components"};

    // TableName, value
    std::vector<std::tuple<std::string, Real64>> results({
        {"Source Energy End Use Components Summary", eleckWh / largeConv},
        {"Source Energy End Use Components Per Conditioned Floor Area", 10000.0 / largeConv / (expectedBuildingConditionedFloorArea / areaConv)},
        {"Source Energy End Use Components Per Total Floor Area", 10000.0 / largeConv / (expectedBuildingGrossFloorArea / areaConv)},
    });

    for (auto &v : results) {

        std::string tableName = std::get<0>(v);
        Real64 expectedValue = std::get<1>(v);

        for (auto &rowName : testRowNames) {
            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE ReportName = '" +
                              reportName +
                              "'"
                              "  AND TableName = '" +
                              tableName +
                              "'"
                              "  AND RowName = '" +
                              rowName + "'" + "  AND ColumnName = '" + columnName + "'");

            Real64 return_val = execAndReturnFirstDouble(query);

            // Add informative message if failed
            EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
        }
    }
}

TEST_F(EnergyPlusFixture, ORT_LoadSummaryUnitConversion_OverLoad_DualUnits)
{
    CompLoadTablesType compLoad;
    compLoad.cells.allocate(LoadCompCol::PerArea, LoadCompRow::GrdTot);
    compLoad.cells = 0.;
    compLoad.cellUsed.allocate(LoadCompCol::PerArea, LoadCompRow::GrdTot);
    compLoad.cellUsed = true;

    compLoad.cells(LoadCompCol::SensInst, LoadCompRow::Lights) = 3.;
    compLoad.cells(LoadCompCol::Latent, LoadCompRow::Lights) = 10.;

    compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights) = 5.;

    compLoad.outsideDryBulb = 20.;
    compLoad.mainFanAirFlow = 0.7;
    compLoad.airflowPerTotCap = 0.2;
    compLoad.totCapPerArea = 0.15;

    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;
    SetupUnitConversions(*state);
    Real64 powerConversion = getSpecificUnitMultiplier(*state, "W", "Btu/h");
    Real64 areaConversion = getSpecificUnitMultiplier(*state, "m2", "ft2");
    Real64 airFlowConversion = getSpecificUnitMultiplier(*state, "m3/s", "ft3/min");
    int tempConvIndx = getSpecificUnitIndex(*state, "C", "F");

    // LoadSummaryUnitConversion(*state, compLoad);
    LoadSummaryUnitConversion(*state, compLoad, state->dataOutRptTab->unitsStyle_SQLite);

    EXPECT_EQ(3. * powerConversion, compLoad.cells(LoadCompCol::SensInst, LoadCompRow::Lights));
    EXPECT_EQ(10. * powerConversion, compLoad.cells(LoadCompCol::Latent, LoadCompRow::Lights));
    EXPECT_EQ(5. * areaConversion, compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights));
    EXPECT_EQ(5. * areaConversion, compLoad.cells(LoadCompCol::Area, LoadCompRow::Lights));

    EXPECT_EQ(ConvertIP(*state, tempConvIndx, 20.), compLoad.outsideDryBulb);
    EXPECT_EQ(0.7 * airFlowConversion, compLoad.mainFanAirFlow);
    EXPECT_EQ(0.2 * airFlowConversion / powerConversion, compLoad.airflowPerTotCap);
    EXPECT_EQ(0.15 * powerConversion / areaConversion, compLoad.totCapPerArea);
}

TEST_F(SQLiteFixture, WriteVeriSumTableAreasTest_DualUnits)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularVeriSum = true;
    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).FrameDivider = 0;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).FrameDivider = 0;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 1;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).FrameDivider = 1;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).FrameDivider = 2;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 1;

    // frames
    state->dataHeatBal->TotFrameDivider = 2;
    state->dataSurface->FrameDivider.allocate(state->dataHeatBal->TotFrameDivider);
    state->dataSurface->FrameDivider(1).FrameWidth = 0.3;
    state->dataSurface->FrameDivider(2).FrameWidth = 0.2;

    // zone
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->Zone(1).FloorArea = 600.; // 20 x 30
    state->dataHeatBal->Zone(1).Volume = 6000.;   // 20 x 30 x 10
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 500.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::None;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    SetupUnitConversions(*state);
    Real64 areaConv = getSpecificUnitDivider(*state, "m2", "ft2");
    Real64 volConv = getSpecificUnitDivider(*state, "m3", "ft3");

    WriteVeriSumTable(*state);

    auto tabularData = queryResult("SELECT * FROM TabularData;", "TabularData");
    auto strings = queryResult("SELECT * FROM Strings;", "Strings");
    auto stringTypes = queryResult("SELECT * FROM StringTypes;", "StringTypes");

    EXPECT_EQ(174ul, tabularData.size());
    // tabularDataIndex, reportNameIndex, reportForStringIndex, tableNameIndex, rowLabelIndex, columnLabelIndex, unitsIndex, simulationIndex, rowId,
    // columnId, value
    EXPECT_EQ("       12.30", tabularData[3][10]);
    EXPECT_EQ("       45.60", tabularData[4][10]);
    // envelope - window-wall ratio subtable
    // north
    EXPECT_NEAR(200.00 / areaConv, std::stod(tabularData[15][10]), 0.01);
    EXPECT_NEAR(200.00 / areaConv, std::stod(tabularData[16][10]), 0.01);
    EXPECT_NEAR(48.16 / areaConv, std::stod(tabularData[17][10]), 0.01);
    EXPECT_EQ("       24.08", tabularData[18][10]);
    EXPECT_EQ("       24.08", tabularData[19][10]);
    // east
    EXPECT_NEAR(300.00 / areaConv, std::stod(tabularData[20][10]), 0.01);
    EXPECT_NEAR(300.00 / areaConv, std::stod(tabularData[21][10]), 0.01);
    EXPECT_NEAR(66.56 / areaConv, std::stod(tabularData[22][10]), 0.01);
    EXPECT_EQ("       22.19", tabularData[23][10]);
    EXPECT_EQ("       22.19", tabularData[24][10]);
    // Performance - zone summary table
    EXPECT_NEAR(600.00 / areaConv, std::stod(tabularData[63][10]), 0.01);
    EXPECT_EQ("Yes", tabularData[68][10]); // conditioned
    EXPECT_EQ("Yes", tabularData[73][10]); // part of total floor area
    EXPECT_NEAR(6000.00 / volConv, std::stod(tabularData[78][10]), 0.01);
    EXPECT_EQ("        1.00", tabularData[83][10]); // multiplier
    EXPECT_NEAR(500.00 / areaConv, std::stod(tabularData[88][10]), 0.01);
    EXPECT_NEAR(100.00 / areaConv, std::stod(tabularData[98][10]), 0.01);
    EXPECT_NEAR(114.72 / areaConv, std::stod(tabularData[103][10]), 0.01);
}

// Dual Unit test: Borrowed from Test for #6350 and #6469
TEST_F(SQLiteFixture, WriteVeriSumTable_TestNotPartOfTotal_DualUnits)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularVeriSum = true;
    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 2;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 2;

    // Loads
    state->dataHeatBal->TotLights = 3;
    state->dataHeatBal->Lights.allocate(state->dataHeatBal->TotLights);

    state->dataHeatBal->TotPeople = 3;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);

    state->dataHeatBal->TotElecEquip = 3;
    state->dataHeatBal->ZoneElectric.allocate(state->dataHeatBal->TotElecEquip);

    state->dataHeatBal->Lights(1).ZonePtr = 1;
    state->dataHeatBal->Lights(1).spaceIndex = 1;
    state->dataHeatBal->Lights(1).DesignLevel = 1000.0;
    state->dataHeatBal->Lights(2).ZonePtr = 2;
    state->dataHeatBal->Lights(2).spaceIndex = 2;
    state->dataHeatBal->Lights(2).DesignLevel = 100.0;
    state->dataHeatBal->Lights(3).ZonePtr = 3;
    state->dataHeatBal->Lights(3).spaceIndex = 3;
    state->dataHeatBal->Lights(3).DesignLevel = 10.0;

    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).spaceIndex = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 10.0;
    state->dataHeatBal->People(2).ZonePtr = 2;
    state->dataHeatBal->People(2).spaceIndex = 2;
    state->dataHeatBal->People(2).NumberOfPeople = 5.0;
    state->dataHeatBal->People(3).ZonePtr = 3;
    state->dataHeatBal->People(3).spaceIndex = 3;
    state->dataHeatBal->People(3).NumberOfPeople = 1.0;

    state->dataHeatBal->ZoneElectric(1).ZonePtr = 1;
    state->dataHeatBal->ZoneElectric(1).spaceIndex = 1;
    state->dataHeatBal->ZoneElectric(1).DesignLevel = 500.0;
    state->dataHeatBal->ZoneElectric(2).ZonePtr = 2;
    state->dataHeatBal->ZoneElectric(2).spaceIndex = 2;
    state->dataHeatBal->ZoneElectric(2).DesignLevel = 50.0;
    state->dataHeatBal->ZoneElectric(3).ZonePtr = 3;
    state->dataHeatBal->ZoneElectric(3).spaceIndex = 3;
    state->dataHeatBal->ZoneElectric(3).DesignLevel = 5.0;

    // zone
    state->dataGlobal->NumOfZones = 3;
    state->dataGlobal->numSpaces = 3;
    state->dataViewFactor->NumOfSolarEnclosures = 3;
    state->dataViewFactor->EnclSolInfo.allocate(state->dataViewFactor->NumOfSolarEnclosures);
    state->dataGlobal->numSpaceTypes = 1;
    state->dataHeatBal->spaceTypes.allocate(state->dataGlobal->numSpaceTypes);
    state->dataHeatBal->spaceTypes(1) = "General";

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->space.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->space(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->space(1).spaceTypeNum = 1;
    state->dataHeatBal->space(1).solarEnclosureNum = 1;
    state->dataHeatBal->Zone(1).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes(1) = 1;
    // 10x10x2
    state->dataHeatBal->Zone(1).FloorArea = 1000.;
    state->dataHeatBal->Zone(1).Volume = 2000.;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 800.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    state->dataHeatBal->Zone(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 0; // Unconditioned
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).Multiplier = 1.;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.;
    state->dataHeatBal->space(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->space(2).spaceTypeNum = 1;
    state->dataHeatBal->space(2).solarEnclosureNum = 2;
    state->dataHeatBal->Zone(2).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(2).spaceIndexes(1) = 2;
    // 10x10x2
    state->dataHeatBal->Zone(2).FloorArea = 100.;
    state->dataHeatBal->Zone(2).Volume = 200.;
    state->dataHeatBal->Zone(2).ExtGrossWallArea = 80.;
    state->dataHeatBal->Zone(2).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(2).ExtWindowArea = 0.0;

    state->dataHeatBal->Zone(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(3).isPartOfTotalArea = false;
    state->dataHeatBal->Zone(3).Multiplier = 1.;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.;
    state->dataHeatBal->space(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->space(3).spaceTypeNum = 1;
    state->dataHeatBal->space(3).solarEnclosureNum = 3;
    state->dataHeatBal->Zone(3).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(3).spaceIndexes(1) = 3;
    // 10x10x2
    state->dataHeatBal->Zone(3).FloorArea = 10.;
    state->dataHeatBal->Zone(3).Volume = 20.;
    state->dataHeatBal->Zone(3).ExtGrossWallArea = 8.;
    state->dataHeatBal->Zone(3).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(3).ExtWindowArea = 0.0;

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::None;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    SetupUnitConversions(*state);
    Real64 areaConv = getSpecificUnitDivider(*state, "m2", "ft2");
    Real64 volConv = getSpecificUnitDivider(*state, "m3", "ft3");

    std::string SIunit = "[W/m2]";
    int unitConvIndex;
    std::string Wm2_unitName;
    Real64 Wm2_unitConv;
    LookupSItoIP(*state, SIunit, unitConvIndex, Wm2_unitName);
    Wm2_unitConv = ConvertIP(*state, unitConvIndex, 1.0);

    WriteVeriSumTable(*state);

    // Check yes/no flag

    // RowName, ColumnName, value
    std::vector<std::tuple<std::string, std::string, std::string>> results_strings({
        {state->dataHeatBal->Zone(1).Name, "Conditioned (Y/N)", "Yes"},
        {state->dataHeatBal->Zone(1).Name, "Part of Total Floor Area (Y/N)", "Yes"},

        {state->dataHeatBal->Zone(2).Name, "Conditioned (Y/N)", "No"},
        {state->dataHeatBal->Zone(2).Name, "Part of Total Floor Area (Y/N)", "Yes"},

        {state->dataHeatBal->Zone(3).Name, "Conditioned (Y/N)", "Yes"},
        {state->dataHeatBal->Zone(3).Name, "Part of Total Floor Area (Y/N)", "No"},
    });

    // Would have used bind_text in sqlite3 with a single prepared statement, but m_db is protected in SQLiteProcedures
    std::string rowName;
    std::string columnName;

    for (auto v : results_strings) {

        rowName = std::get<0>(v);
        columnName = std::get<1>(v);

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = 'InputVerificationandResultsSummary'"
                          "  AND TableName = 'Zone Summary'"
                          "  AND RowName = '" +
                          rowName + "'" + "  AND ColumnName = '" + columnName + "'");

        std::string flag = queryResult(query, "TabularDataWithStrings")[0][0];

        // Add informative message if failed
        EXPECT_EQ(std::get<2>(v), flag) << "Failed for RowName=" << rowName << "; ColumnName=" << columnName;
    }

    // Check each zone and total rows

    // RowName, ColumnName, value
    std::vector<std::tuple<std::string, std::string, Real64>> results({
        {state->dataHeatBal->Zone(1).Name, "Area", state->dataHeatBal->Zone(1).FloorArea / areaConv},
        {state->dataHeatBal->Zone(2).Name, "Area", state->dataHeatBal->Zone(2).FloorArea / areaConv},
        {state->dataHeatBal->Zone(3).Name, "Area", state->dataHeatBal->Zone(3).FloorArea / areaConv},
        {"Total", "Area", (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea) / areaConv},
        {"Conditioned Total", "Area", state->dataHeatBal->Zone(1).FloorArea / areaConv},
        {"Unconditioned Total", "Area", state->dataHeatBal->Zone(2).FloorArea / areaConv},
        {"Not Part of Total", "Area", state->dataHeatBal->Zone(3).FloorArea / areaConv},

        {state->dataHeatBal->Zone(1).Name, "Volume", state->dataHeatBal->Zone(1).Volume / volConv},
        {state->dataHeatBal->Zone(2).Name, "Volume", state->dataHeatBal->Zone(2).Volume / volConv},
        {state->dataHeatBal->Zone(3).Name, "Volume", state->dataHeatBal->Zone(3).Volume / volConv},
        {"Total", "Volume", (state->dataHeatBal->Zone(1).Volume + state->dataHeatBal->Zone(2).Volume) / volConv},
        {"Conditioned Total", "Volume", state->dataHeatBal->Zone(1).Volume / volConv},
        {"Unconditioned Total", "Volume", state->dataHeatBal->Zone(2).Volume / volConv},
        {"Not Part of Total", "Volume", state->dataHeatBal->Zone(3).Volume / volConv},

        {state->dataHeatBal->Zone(1).Name,
         "Lighting",
         state->dataHeatBal->Lights(1).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(1).FloorArea},
        {state->dataHeatBal->Zone(2).Name,
         "Lighting",
         state->dataHeatBal->Lights(2).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(2).FloorArea},
        {state->dataHeatBal->Zone(3).Name,
         "Lighting",
         state->dataHeatBal->Lights(3).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(3).FloorArea},
        {"Total",
         "Lighting",
         (state->dataHeatBal->Lights(1).DesignLevel + state->dataHeatBal->Lights(2).DesignLevel) * Wm2_unitConv /
             (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea)},
        {"Conditioned Total", "Lighting", state->dataHeatBal->Lights(1).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(1).FloorArea},
        {"Unconditioned Total", "Lighting", state->dataHeatBal->Lights(2).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(2).FloorArea},
        {"Not Part of Total", "Lighting", state->dataHeatBal->Lights(3).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(3).FloorArea},

        // People/m^2
        {state->dataHeatBal->Zone(1).Name, "People", state->dataHeatBal->Zone(1).FloorArea / areaConv / state->dataHeatBal->People(1).NumberOfPeople},
        {state->dataHeatBal->Zone(2).Name, "People", state->dataHeatBal->Zone(2).FloorArea / areaConv / state->dataHeatBal->People(2).NumberOfPeople},
        {state->dataHeatBal->Zone(3).Name, "People", state->dataHeatBal->Zone(3).FloorArea / areaConv / state->dataHeatBal->People(3).NumberOfPeople},
        {"Total",
         "People",
         (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea) / areaConv /
             (state->dataHeatBal->People(1).NumberOfPeople + state->dataHeatBal->People(2).NumberOfPeople)},
        {"Conditioned Total", "People", state->dataHeatBal->Zone(1).FloorArea / areaConv / state->dataHeatBal->People(1).NumberOfPeople},
        {"Unconditioned Total", "People", state->dataHeatBal->Zone(2).FloorArea / areaConv / state->dataHeatBal->People(2).NumberOfPeople},
        {"Not Part of Total", "People", state->dataHeatBal->Zone(3).FloorArea / areaConv / state->dataHeatBal->People(3).NumberOfPeople},

        {state->dataHeatBal->Zone(1).Name,
         "Plug and Process",
         state->dataHeatBal->ZoneElectric(1).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(1).FloorArea},
        {state->dataHeatBal->Zone(2).Name,
         "Plug and Process",
         state->dataHeatBal->ZoneElectric(2).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(2).FloorArea},
        {state->dataHeatBal->Zone(3).Name,
         "Plug and Process",
         state->dataHeatBal->ZoneElectric(3).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(3).FloorArea},
        {"Total",
         "Plug and Process",
         (state->dataHeatBal->ZoneElectric(1).DesignLevel + state->dataHeatBal->ZoneElectric(2).DesignLevel) * Wm2_unitConv /
             (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea)},
        {"Conditioned Total",
         "Plug and Process",
         state->dataHeatBal->ZoneElectric(1).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(1).FloorArea},
        {"Unconditioned Total",
         "Plug and Process",
         state->dataHeatBal->ZoneElectric(2).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(2).FloorArea},
        {"Not Part of Total",
         "Plug and Process",
         state->dataHeatBal->ZoneElectric(3).DesignLevel * Wm2_unitConv / state->dataHeatBal->Zone(3).FloorArea},
    });

    // Would have used bind_text in sqlite3 with a single prepared statement, but m_db is protected in SQLiteProcedures
    for (auto v : results) {

        rowName = std::get<0>(v);
        columnName = std::get<1>(v);

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE ReportName = 'InputVerificationandResultsSummary'"
                          "  AND TableName = 'Zone Summary'"
                          "  AND RowName = '" +
                          rowName + "'" + "  AND ColumnName = '" + columnName + "'");

        Real64 return_val = execAndReturnFirstDouble(query);

        // Add informative message if failed
        EXPECT_NEAR(std::get<2>(v), return_val, 0.01) << "Failed for RowName=" << rowName << "; ColumnName=" << columnName;
    }
}

TEST_F(SQLiteFixture, ORT_EndUseBySubcategorySQL_DualUnits)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularBEPS = true;
    state->dataOutRptTab->displayDemandEndUse = true;
    state->dataOutRptTab->displayLEEDSummary = true;

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;
    Real64 enerConv = getSpecificUnitDivider(*state, "GJ", "kBtu"); // 948.45
    EXPECT_NEAR(1.0 / enerConv, 948.0, 0.5);

    // Needed to avoid crash (from ElectricPowerServiceManager.hh)
    createFacilityElectricPowerServiceObject(*state);

    SetPredefinedTables(*state);

    Real64 extLitUse = 1e8;
    Real64 CoalHeating = 2e8;
    Real64 GasolineHeating = 3e8;
    Real64 PropaneHeating = 4e8;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "AnotherEndUseSubCat");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite3",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Coal Energy",
                        Constant::Units::J,
                        CoalHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite4",
                        Constant::eResource::Coal,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Gasoline Energy",
                        Constant::Units::J,
                        GasolineHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite5",
                        Constant::eResource::Gasoline,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Propane Energy",
                        Constant::Units::J,
                        PropaneHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite6",
                        Constant::eResource::Propane,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 3600.0;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataOutRptTab->displayTabularBEPS = true;
    // OutputProcessor::TimeValue.allocate(2);

    auto timeStep = 1.0;

    SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, timeStep);
    SetupTimePointers(*state, OutputProcessor::TimeStepType::System, timeStep);

    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep = 60;
    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep = 60;

    GetInputOutputTableSummaryReports(*state);

    state->dataEnvrn->Month = 12;

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 3, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 2, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 1, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 6, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 4, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 2, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 9, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 6, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 3, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    OutputReportTabular::WriteBEPSTable(*state);
    OutputReportTabular::WriteDemandEndUseSummary(*state);

    // We test for Heating and Total, since they should be the same
    std::vector<std::string> testReportNames = {"AnnualBuildingUtilityPerformanceSummary", "DemandEndUseComponentsSummary"};
    std::vector<std::string> endUseSubCategoryNames = {"General", "AnotherEndUseSubCat"};

    std::string endUseName = "Exterior Lighting";
    std::string endUseSubCategoryName = "AnotherEndUseSubCat";
    std::string rowName = endUseName + ":" + endUseSubCategoryName;
    std::string columnName = "Electricity";

    for (auto &endUseSubCategoryName : endUseSubCategoryNames) {
        for (auto &reportName : testReportNames) {

            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE TableName = 'End Uses By Subcategory'"
                              "  AND ColumnName = 'Electricity'"
                              "  AND ReportName = '" +
                              reportName +
                              "'"
                              "  AND RowName = '" +
                              endUseName + ":" + endUseSubCategoryName + "'"); // Now Like 'Exterior Lighting:General'

            auto result = queryResult(query, "TabularDataWithStrings");

            ASSERT_EQ(1ul, result.size()) << "Query crashed for reportName=" << reportName;
        }
    }

    for (auto &reportName : testReportNames) {

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ColumnName = 'Electricity'"
                          "  AND ReportName = '" +
                          reportName +
                          "'"
                          "  AND RowName = '" +
                          endUseName + "'");

        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(1ul, result.size()) << "Query crashed for reportName=" << reportName;
    }

    // Specifically get the electricity usage for End Use = Exterior Lighting, and End Use Subcat = AnotherEndUseSubCat,
    // and make sure it's the right number that's returned
    std::string query("SELECT Value From TabularDataWithStrings"
                      "  WHERE TableName = 'End Uses By Subcategory'"
                      "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                      "  AND ColumnName = 'Electricity'"
                      "  AND RowName = 'Exterior Lighting:AnotherEndUseSubCat'");
    Real64 return_val = execAndReturnFirstDouble(query);

    // EXPECT_NEAR(extLitUse * 3 / 3.6e6, return_val, 0.01) << "Failed for query: " << query;
    Real64 expected_value = extLitUse * 3.0 / 1.0e9 / enerConv;
    EXPECT_NEAR(expected_value, return_val, 0.01) << "Failed for query: " << query;

    // Get all Interior Lighting End Uses (all subcats) for Electricity
    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Electricity'"
                          "  AND RowName LIKE 'Exterior Lighting:%'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(2u, result.size()) << "Failed for query: " << query;
    }

    // Get all subcat usage for all fuels (13)
    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND RowName = 'Exterior Lighting:AnotherEndUseSubCat'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }

    // Specifically get the each fuel (Coal, Gasoline, and Propane) usage for End Use = Heating,
    // and make sure it's the right number that's returned

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Coal'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val1 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        // EXPECT_NEAR(CoalHeating * 3 / 3.6e6, return_val1, 0.01) << "Failed for query: " << query;
        Real64 expected_coalHt = CoalHeating * 3 / 1.0e9 / enerConv;
        EXPECT_NEAR(expected_coalHt, return_val1, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Gasoline'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val2 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        // EXPECT_NEAR(GasolineHeating * 3 / 3.6e6, return_val2, 0.01) << "Failed for query: " << query;
        EXPECT_NEAR(GasolineHeating * 3 / 1.0e9 / enerConv, return_val2, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Propane'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val3 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        // EXPECT_NEAR(PropaneHeating * 3 / 3.6e6, return_val3, 0.01) << "Failed for query: " << query;
        EXPECT_NEAR(PropaneHeating * 3 / 1.0e9 / enerConv, return_val3, 0.01) << "Failed for query: " << query;
    }

    // Check the heating category has the result size of 13 (including all disaggregated additional fuels) in both reports)

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'DemandEndUseComponentsSummary'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }
}

TEST_F(SQLiteFixture, OutputReportTabularTest_EscapeHTML)
{
    // Test for #8542 - Ensures strings are escaped before going to HTML
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    auto &ort(state->dataOutRptTab);
    ort->numStyles = 1;
    ort->TableStyle(1) = OutputReportTabular::TableStyle::HTML;
    ort->del(1) = DataStringGlobals::CharSpace; // space - this is not used much for HTML output

    ort->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    ort->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;

    SetPredefinedTables(*state);
    std::string CompName = "My Coil <coil is DX>";

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchDXCoolCoilType, CompName, "Coil:Cooling:DX:SingleSpeed");
    // This would normally be called with numerics such as CompName, 0.006, 8, but I don't really care
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdch2CoilLvgHumRatIdealPeak, CompName, "My Design Day where it's >= 8\u00B0");
    PreDefTableEntry(*state,
                     state->dataOutRptPredefined->pdst2CoilSummaryCoilSelection,
                     CompName,
                     "My Design Day where it's >= 8\u00B0"); // this is >= 8 degree sign

    // We enable the reports we care about, making sure we have the right ones
    EXPECT_EQ("HVACSizingSummary", state->dataOutRptPredefined->reportName(7).name);
    state->dataOutRptPredefined->reportName(7).show = true;

    OutputReportTabular::OpenOutputTabularFile(*state);

    WritePredefinedTables(*state);

    OutputReportTabular::CloseOutputTabularFile(*state);

    std::vector<std::string> lines = read_lines_in_file(state->dataStrGlobals->outputTblHtmFilePath);

    // Lambda helper to locate a line in the html file, and compare that line with the expected html after trimming
    auto compare_html_output = [this, &lines](const std::string &lookup, const std::string &expectedHTMLString) {
        std::string found_cell;
        for (const auto &line : lines) {
            if (line.find(lookup) != std::string::npos) {
                found_cell = line;
                break;
            }
        }
        EXPECT_FALSE(found_cell.empty()) << "Did not find the lookup string '" << lookup << "' string in the html output at '"
                                         << state->dataStrGlobals->outputTblHtmFilePath << "'..." << '\n'
                                         << delimited_string(lines);

        // Trim leading and trailing spaces
        found_cell.erase(0, found_cell.find_first_not_of(' ')); // ltrim
        found_cell.erase(found_cell.find_last_not_of(' ') + 1); // rtrim

        EXPECT_EQ(expectedHTMLString, found_cell) << found_cell;
    };

    compare_html_output("My Coil", "<td align=\"right\">My Coil &lt;coil is DX&gt;</td>");

    // Note that I DO NOT expect `'` to be escaped by `&apos;` like it would in xml. Technically HTML4 doesn't support that, though most browsers
    // would anyways. Also, escaping single and double quotes is only needed inside attributes
    compare_html_output("My Design Day", "<td align=\"right\">My Design Day where it's &gt;= 8&deg;</td>");

    // We ensure that SQL doesn't have the same escape
    for (const std::string reportName : {"HVACSizingSummary"}) {

        auto result = queryResult("SELECT RowName, Value From TabularDataWithStrings "
                                  "WHERE ReportName = \"" +
                                      reportName +
                                      "\""
                                      "  AND ColumnName = \"Coil Leaving Air Humidity Ratio at Ideal Loads Peak\"",
                                  "TabularDataWithStrings");

        EXPECT_EQ(1u, result.size());
        // Because the table has 8 cols
        EXPECT_EQ(8u, result[0].size());

        // 0.006 is a ratio, so unitconv = 1
        std::string s = result[0][0];
        // Trim the string, it has leading spaces
        // s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

        EXPECT_EQ("My Coil <coil is DX>", s);

        EXPECT_EQ("My Design Day where it's >= 8\u00B0", result[0][1]);
    }

    // Clean up
    FileSystem::removeFile(state->dataStrGlobals->outputTblHtmFilePath);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_PredefinedTable_SigDigits_Force_NonZero)
{

    SetPredefinedTables(*state);

    // < 1e8, not using scientific notation
    Real64 value = 123.456;
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPkTimeMin, "MyPlant Sizing Pass 1", value, 2);
    EXPECT_EQ("123.46", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPkTimeMin, "MyPlant Sizing Pass 1"));

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPkTimeDayOfSim, "MyPlant Sizing Pass 1", value, 1);
    EXPECT_EQ("123.5", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPkTimeDayOfSim, "MyPlant Sizing Pass 1"));

    // Force reset to numSigDigits = 2
    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPkTimeHour, "MyPlant Sizing Pass 1", value, 0);
    EXPECT_EQ("123.", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPkTimeHour, "MyPlant Sizing Pass 1"));

    // > 1e8, switch to scientific notation
    value = 123456789.1;

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizDesDay, "MyPlant Sizing Pass 1", value, 3);
    EXPECT_EQ("1.235E+08", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizDesDay, "MyPlant Sizing Pass 1"));

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPrevVdot, "MyPlant Sizing Pass 1", value, 2);
    EXPECT_EQ("1.23E+08", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizPrevVdot, "MyPlant Sizing Pass 1"));

    // Force reset to numSigDigits = 2 since we switch to scientific notation

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizMeasVdot, "MyPlant Sizing Pass 1", value, 1);
    EXPECT_EQ("1.23E+08", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizMeasVdot, "MyPlant Sizing Pass 1"));

    PreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizCalcVdot, "MyPlant Sizing Pass 1", value, 0);
    EXPECT_EQ("1.23E+08", RetrievePreDefTableEntry(*state, state->dataOutRptPredefined->pdchPlantSizCalcVdot, "MyPlant Sizing Pass 1"));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_PredefinedTable_Standard62_1_NoSizing)
{
    // Test for #8822 - new warning to explain why Standard62.1 report is not enabled
    std::string const idf_objects = delimited_string({
        "Output:Table:SummaryReports,",
        " Standard62.1Summary; !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // These are already set to these values, but be explicit.
    // Because DoZoneSizing and DoSystemSizing are both false Standard62.1Summary is **not** enabled
    state->files.outputControl.tabular = true;
    state->dataGlobal->DoZoneSizing = false;
    state->dataGlobal->DoSystemSizing = false;

    EXPECT_EQ(1, state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "Output:Table:SummaryReports"));

    EXPECT_EQ(0, state->dataOutRptPredefined->numReportName);
    SetPredefinedTables(*state);
    EXPECT_GT(state->dataOutRptPredefined->numReportName, 0);
    auto &reportNameArray = state->dataOutRptPredefined->reportName;
    auto it =
        std::find_if(reportNameArray.begin(), reportNameArray.end(), [](const auto &rN) { return Util::SameString("Standard62.1Summary", rN.name); });
    EXPECT_FALSE(it != reportNameArray.end()); // Not found

    GetInputOutputTableSummaryReports(*state);

    std::string expected_error =
        delimited_string({"   ** Warning ** Output:Table:SummaryReports Field[1]=\"Standard62.1Summary\", Report is not enabled.",
                          "   **   ~~~   ** Do Zone Sizing or Do System Sizing must be enabled in SimulationControl."});

    compare_err_stream(expected_error, true);
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_PredefinedTable_Standard62_1_WithSizing)
{
    // Test for #8822 - ensures that when Zone/System sizing is requested, the report is there and the warning isn't.
    std::string const idf_objects = delimited_string({
        "Output:Table:SummaryReports,",
        " Standard62.1Summary; !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Because DoZoneSizing is true Standard62.1Summary **is** enabled
    state->files.outputControl.tabular = true;
    state->dataGlobal->DoZoneSizing = true;
    state->dataGlobal->DoSystemSizing = false;

    EXPECT_EQ(1, state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "Output:Table:SummaryReports"));

    EXPECT_EQ(0, state->dataOutRptPredefined->numReportName);
    SetPredefinedTables(*state);
    EXPECT_GT(state->dataOutRptPredefined->numReportName, 0);
    auto &reportNameArray = state->dataOutRptPredefined->reportName;
    auto it =
        std::find_if(reportNameArray.begin(), reportNameArray.end(), [](const auto &rN) { return Util::SameString("Standard62.1Summary", rN.name); });
    EXPECT_TRUE(it != reportNameArray.end());
    // EXPECT_TRUE(Util::FindItem("Standard62.1Summary", state->dataOutRptPredefined->reportName));

    GetInputOutputTableSummaryReports(*state);

    EXPECT_FALSE(has_err_output(true));
}

TEST_F(SQLiteFixture, OutputReportTabularMonthly_CurlyBraces)
{
    // Test for #8921

    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    std::string const idf_objects = delimited_string({

        "Output:Table:Monthly,",
        "  MONTHLY EXAMPLE,                        !- Name",
        "  ,                                       !- Digits After Decimal",
        "  Electricity:Facility,                   !- Variable or Meter Name 1",
        "  SumOrAverage,                           !- Aggregation Type for Variable or Meter 1",
        "  Electricity:Facility,                   !- Variable or Meter Name 2",
        "  Maximum,                                !- Aggregation Type for Variable or Meter 2",
        "  Electricity:Facility,                   !- Variable or Meter Name 3",
        "  Minimum,                                !- Aggregation Type for Variable or Meter 3",
        "  Electricity:Facility,                   !- Variable or Meter Name 4",
        "  ValueWhenMaximumOrMinimum,              !- Aggregation Type for Variable or Meter 4",
        "  Electricity:Facility,                   !- Variable or Meter Name 5",
        "  HoursNonZero,                           !- Aggregation Type for Variable or Meter 5",
        "  Electricity:Facility,                   !- Variable or Meter Name 6",
        "  HoursZero,                              !- Aggregation Type for Variable or Meter 6",
        "  Electricity:Facility,                   !- Variable or Meter Name 7",
        "  HoursPositive,                          !- Aggregation Type for Variable or Meter 7",
        "  Electricity:Facility,                   !- Variable or Meter Name 8",
        "  HoursNonPositive,                       !- Aggregation Type for Variable or Meter 8",
        "  Electricity:Facility,                   !- Variable or Meter Name 9",
        "  HoursNegative,                          !- Aggregation Type for Variable or Meter 9",
        "  Electricity:Facility,                   !- Variable or Meter Name 10",
        "  HoursNonNegative,                       !- Aggregation Type for Variable or Meter 10",
        "  Electricity:Facility,                   !- Variable or Meter Name 11",
        "  SumOrAverageDuringHoursShown,           !- Aggregation Type for Variable or Meter 11",
        "  Electricity:Facility,                   !- Variable or Meter Name 12",
        "  MaximumDuringHoursShown,                !- Aggregation Type for Variable or Meter 12",
        "  Electricity:Facility,                   !- Variable or Meter Name 13",
        "  MinimumDuringHoursShown;                !- Aggregation Type for Variable or Meter 13",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    Meter *meter = new Meter("Electricity:Facility");
    meter->units = Constant::Units::J;
    state->dataOutputProcessor->meters.push_back(meter);
    state->dataOutputProcessor->meterMap.insert_or_assign("ELECTRICITY:FACILITY", (int)state->dataOutputProcessor->meters.size() - 1);
    // We do need to trick it into thinking it's a weather simulation, otherwise the monthly reports aren't reported
    state->dataGlobal->DoWeathSim = true; // flag to trick tabular reports to scan meters
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;

    OutputReportTabular::GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    OutputReportTabular::InitializeTabularMonthly(*state);

    OutputReportTabular::WriteMonthlyTables(*state);

    auto columnHeaders = queryResult(
        R"(SELECT DISTINCT(ColumnName) FROM TabularDataWithStrings
             WHERE ReportName LIKE "MONTHLY EXAMPLE%")",
        "TabularDataWithStrings");

    // 13 agg types for the same variable requested above + the {TIMESTAMP} ones (but distinct, so counts as 1)
    EXPECT_EQ(14, columnHeaders.size());

    auto missingBracesHeaders = queryResult(
        R"(SELECT DISTINCT(ColumnName) FROM TabularDataWithStrings
             WHERE ReportName LIKE "MONTHLY EXAMPLE%"
             AND ColumnName LIKE "%{%" AND ColumnName NOT LIKE "%}%")",
        "TabularDataWithStrings");

    // Should be none!
    for (auto &col : missingBracesHeaders) {
        std::string colHeader = col[0];
        EXPECT_TRUE(false) << "Missing braces in monthly table for : " << colHeader;
    }

    // Test for #9436
    auto extraSpaceAfterBracesHeaders = queryResult(
        R"(SELECT DISTINCT(ColumnName) FROM TabularDataWithStrings
             WHERE ReportName LIKE "MONTHLY EXAMPLE%"
             AND ColumnName LIKE "%} %")",
        "TabularDataWithStrings");

    // Should be none!
    for (auto &col : extraSpaceAfterBracesHeaders) {
        std::string colHeader = col[0];
        ADD_FAILURE() << "Extra space after brace in monthly table for : '" << colHeader << "'";
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_WarningMultiplePeopleObj)
{
    // when multiple people objects with different threshold settings are defined for one zone,
    // a warning is thrown, as people-dependent resilience metrics are not meaningful in these
    // settings
    std::string const idf_objects = delimited_string({

        "Zone,",
        "ZONE ONE,                !- Name",
        "0,                       !- Direction of Relative North {deg}",
        "0,                       !- X Origin {m}",
        "0,                       !- Y Origin {m}",
        "0,                       !- Z Origin {m}",
        "1,                       !- Type",
        "1,                       !- Multiplier",
        "autocalculate,           !- Ceiling Height {m}",
        "autocalculate;           !- Volume {m3}",

        "People,",
        "ZONE ONE People 1,       !- Name",
        "ZONE ONE,                !- Zone or ZoneList Name",
        "BLDG_OCC_SCH,            !- Number of People Schedule Name",
        "People,                  !- Number of People Calculation Method",
        "10,                      !- Number of People",
        ",                        !- People per Zone Floor Area {person/m2}",
        ",                        !- Zone Floor Area per Person {m2/person}",
        "0.3000,                  !- Fraction Radiant",
        "AUTOCALCULATE,           !- Sensible Heat Fraction",
        "ACTIVITY_SCH,            !- Activity Level Schedule Name",
        ",                        !- Carbon Dioxide Generation Rate {m3/s-W}",
        "No,                      !- Enable ASHRAE 55 Comfort Warnings",
        "EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        ",                        !- Surface Name/Angle Factor List Name",
        ",                        !- Work Efficiency Schedule Name",
        ",                        !- Clothing Insulation Calculation Method",
        ",                        !- Clothing Insulation Calculation Method Schedule Name",
        ",                        !- Clothing Insulation Schedule Name",
        ",                        !- Air Velocity Schedule Name",
        ",                        !- Thermal Comfort Model 1 Type",
        ",                        !- Thermal Comfort Model 2 Type",
        ",                        !- Thermal Comfort Model 3 Type",
        ",                        !- Thermal Comfort Model 4 Type",
        ",                        !- Thermal Comfort Model 5 Type",
        ",                        !- Thermal Comfort Model 6 Type",
        ",                        !- Thermal Comfort Model 7 Type",
        ",                        !- Ankle Level Air Velocity Schedule Name",
        "10.5,                    !- Cold Stress Temperature Threshold [C]",
        "32.5;                    !- Heat Stress Temperature Threshold [C]",

        "People,",
        "ZONE ONE People 2,       !- Name",
        "ZONE ONE,                !- Zone or ZoneList Name",
        "BLDG_OCC_SCH,            !- Number of People Schedule Name",
        "People,                  !- Number of People Calculation Method",
        "10,                      !- Number of People",
        ",                        !- People per Zone Floor Area {person/m2}",
        ",                        !- Zone Floor Area per Person {m2/person}",
        "0.3000,                  !- Fraction Radiant",
        "AUTOCALCULATE,           !- Sensible Heat Fraction",
        "ACTIVITY_SCH,            !- Activity Level Schedule Name",
        ",                        !- Carbon Dioxide Generation Rate {m3/s-W}",
        "No,                      !- Enable ASHRAE 55 Comfort Warnings",
        "EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        ",                        !- Surface Name/Angle Factor List Name",
        ",                        !- Work Efficiency Schedule Name",
        ",                        !- Clothing Insulation Calculation Method",
        ",                        !- Clothing Insulation Calculation Method Schedule Name",
        ",                        !- Clothing Insulation Schedule Name",
        ",                        !- Air Velocity Schedule Name",
        ",                        !- Thermal Comfort Model 1 Type",
        ",                        !- Thermal Comfort Model 2 Type",
        ",                        !- Thermal Comfort Model 3 Type",
        ",                        !- Thermal Comfort Model 4 Type",
        ",                        !- Thermal Comfort Model 5 Type",
        ",                        !- Thermal Comfort Model 6 Type",
        ",                        !- Thermal Comfort Model 7 Type",
        ",                        !- Ankle Level Air Velocity Schedule Name",
        "11.5,                    !- Cold Stress Temperature Threshold [C]",
        "30.5;                    !- Heat Stress Temperature Threshold [C]",

        "ScheduleTypeLimits,",
        "Any Number;              !- Name",
        "ScheduleTypeLimits,",
        "Fraction,                !- Name",
        "0.0,                     !- Lower Limit Value",
        "1.0,                     !- Upper Limit Value",
        "CONTINUOUS;              !- Numeric Type",

        "Schedule:Compact,",
        "ACTIVITY_SCH,            !- Name",
        "Any Number,              !- Schedule Type Limits Name",
        "Through: 12/31,          !- Field 1",
        "For: AllDays,            !- Field 2",
        "Until: 24:00,120.;       !- Field 3",

        "Schedule:Compact,",
        "BLDG_OCC_SCH,            !- Name",
        "Fraction,                !- Schedule Type Limits Name",
        "Through: 12/31,          !- Field 1",
        "For: AllDays,            !- Field 2",
        "Until: 24:00,1.0;        !- Field 3",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Resilience.allocate(state->dataGlobal->NumOfZones);
    InternalHeatGains::GetInternalHeatGainsInput(*state);

    state->dataOutRptTab->displayThermalResilienceSummary = true;
    UpdateTabularReports(*state, OutputProcessor::TimeStepType::System);

    std::string error_string =
        delimited_string({"   ** Warning ** Zone 1 has multiple people objects with different Cold Stress Temperature Threshold.",
                          "   ** Warning ** Zone 1 has multiple people objects with different Heat Stress Temperature Threshold."});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_WriteSETHoursTableReportingPeriod)
{

    // test unit conversion in SET Degree-Hour report table for reporting periods
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Resilience.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "Test";

    int columnNum = 5;

    Array1D_int columnWidth;
    columnWidth.allocate(columnNum);
    columnWidth = 10;
    Array1D_string columnHead(columnNum);
    columnHead(1) = "SET ≤ 12.2°C Degree-Hours [°C·hr]";
    columnHead(2) = "SET ≤ 12.2°C Occupant-Weighted Degree-Hours [°C·hr]";
    columnHead(3) = "SET ≤ 12.2°C Occupied Degree-Hours [°C·hr]";
    columnHead(4) = "Longest SET ≤ 12.2°C Duration for Occupied Period [hr]";
    columnHead(5) = "Start Time of the Longest SET ≤ 12.2°C Duration for Occupied Period ";

    Real64 degreeHourConversion = 1.8;
    state->dataWeather->TotReportPers = 2;

    state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotReportPers);
    for (int i = 1; i <= state->dataWeather->TotReportPers; i++) {
        state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, i).assign(5, 0.0);
    }

    int encodedMonDayHrMin;
    for (int k = 1; k <= state->dataWeather->TotReportPers; k++) {
        for (int i = 0; i < 4; i++) {
            state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, k)[i] = float(k) * std::pow(-1.0, float(i)) * std::pow(float(i), 2.0);
        }
        General::EncodeMonDayHrMin(encodedMonDayHrMin, 1, 1, 5 * k, 30);
        state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, k)[4] = encodedMonDayHrMin;
    }

    std::string tableName = "Heating SET Degree-Hours";

    Array1D_string rowHead;
    Array2D_string tableBody;
    rowHead.allocate(state->dataGlobal->NumOfZones + 3);
    tableBody.allocate(columnNum, state->dataGlobal->NumOfZones + 3);

    WriteSETHoursTableReportingPeriod(*state,
                                      columnNum,
                                      1,
                                      "Test Period 1",
                                      tableName,
                                      columnHead,
                                      columnWidth,
                                      state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod,
                                      rowHead,
                                      tableBody,
                                      degreeHourConversion);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 1, 1));
    EXPECT_EQ("-1.8", RetrieveEntryFromTableBody(tableBody, 1, 2));
    EXPECT_EQ("7.20", RetrieveEntryFromTableBody(tableBody, 1, 3));
    //    duration hour don't change, 1, 1
    EXPECT_EQ("-9.0", RetrieveEntryFromTableBody(tableBody, 1, 4));
    EXPECT_EQ("01-JAN-04:30", RetrieveEntryFromTableBody(tableBody, 1, 5));

    WriteSETHoursTableReportingPeriod(*state,
                                      columnNum,
                                      2,
                                      "Test Period 2",
                                      tableName,
                                      columnHead,
                                      columnWidth,
                                      state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod,
                                      rowHead,
                                      tableBody,
                                      degreeHourConversion);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 1, 1));
    EXPECT_EQ("-3.6", RetrieveEntryFromTableBody(tableBody, 1, 2));
    EXPECT_EQ("14.40", RetrieveEntryFromTableBody(tableBody, 1, 3));
    //    duration hour don't change
    EXPECT_EQ("-18.0", RetrieveEntryFromTableBody(tableBody, 1, 4));
    EXPECT_EQ("01-JAN-09:30", RetrieveEntryFromTableBody(tableBody, 1, 5));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_UnmetDegreeHourRepPeriodUnitConv)
{
    // test unit conversion in unmet degree hour table for reporting periods
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "Test";

    int columnNumUnmetDegHr = 6;
    Array1D_string columnHeadUnmetDegHr(6);
    // must initialize this otherwise it will only output 5 columns
    Array1D_int columnWidthUnmetDegHr;
    columnWidthUnmetDegHr.allocate(columnNumUnmetDegHr);
    columnWidthUnmetDegHr = 10;
    columnHeadUnmetDegHr(1) = "Cooling Setpoint Unmet Degree-Hours [°C·hr]";
    columnHeadUnmetDegHr(2) = "Cooling Setpoint Unmet Occupant-Weighted Degree-Hours [°C·hr]";
    columnHeadUnmetDegHr(3) = "Cooling Setpoint Unmet Occupied Degree-Hours [°C·hr]";
    columnHeadUnmetDegHr(4) = "Heating Setpoint Unmet Degree-Hours [°C·hr]";
    columnHeadUnmetDegHr(5) = "Heating Setpoint Unmet Occupant-Weighted Degree-Hours [°C·hr]";
    columnHeadUnmetDegHr(6) = "Heating Setpoint Unmet Occupied Degree-Hours [°C·hr]";
    std::string tableName = "Unmet Degree-Hours";

    Real64 degreeHourConversion = 1.8;
    state->dataWeather->TotReportPers = 2;

    state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotReportPers);
    for (int i = 1; i <= state->dataWeather->TotReportPers; i++) {
        state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, i).assign(columnNumUnmetDegHr, 0.0);
    }
    // state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins: [0, -1, 4, -9, 16, -25]
    for (int k = 1; k <= state->dataWeather->TotReportPers; k++) {
        for (int i = 0; i < 6; i++) {
            state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, k)[i] = float(k) * std::pow(-1.0, float(i)) * std::pow(float(i), 2.0);
        }
    }

    Array1D_string rowHead;
    Array2D_string tableBody;
    rowHead.allocate(state->dataGlobal->NumOfZones + 4);
    tableBody.allocate(columnNumUnmetDegHr, state->dataGlobal->NumOfZones + 4);

    WriteResilienceBinsTableReportingPeriod(*state,
                                            "Thermal",
                                            columnNumUnmetDegHr,
                                            1,
                                            "Test Period 1",
                                            tableName,
                                            columnHeadUnmetDegHr,
                                            columnWidthUnmetDegHr,
                                            state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod,
                                            rowHead,
                                            tableBody,
                                            degreeHourConversion);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 1, 1));
    EXPECT_EQ("-1.8", RetrieveEntryFromTableBody(tableBody, 1, 2));
    EXPECT_EQ("7.20", RetrieveEntryFromTableBody(tableBody, 1, 3));
    EXPECT_EQ("-16.2", RetrieveEntryFromTableBody(tableBody, 1, 4));
    EXPECT_EQ("28.80", RetrieveEntryFromTableBody(tableBody, 1, 5));
    EXPECT_EQ("-45.0", RetrieveEntryFromTableBody(tableBody, 1, 6));

    WriteResilienceBinsTableReportingPeriod(*state,
                                            "Thermal",
                                            columnNumUnmetDegHr,
                                            2,
                                            "Test Period 1",
                                            tableName,
                                            columnHeadUnmetDegHr,
                                            columnWidthUnmetDegHr,
                                            state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod,
                                            rowHead,
                                            tableBody,
                                            degreeHourConversion);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 2, 1));
    EXPECT_EQ("-3.6", RetrieveEntryFromTableBody(tableBody, 2, 2));
    EXPECT_EQ("14.40", RetrieveEntryFromTableBody(tableBody, 2, 3));
    EXPECT_EQ("-32.4", RetrieveEntryFromTableBody(tableBody, 2, 4));
    EXPECT_EQ("57.60", RetrieveEntryFromTableBody(tableBody, 2, 5));
    EXPECT_EQ("-90.0", RetrieveEntryFromTableBody(tableBody, 2, 6));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_RetrieveEntryFromTableBody)
{

    Array2D_string tableBody;
    int columnCount = 4;
    int rowCount = 3;
    tableBody.allocate(columnCount, rowCount);
    for (int col_i = 1; col_i <= columnCount; col_i++) {
        for (int row_i = 1; row_i <= rowCount; row_i++) {
            tableBody(col_i, row_i) = fmt::format("{}-{}", col_i, row_i);
        }
    }
    EXPECT_EQ(RetrieveEntryFromTableBody(tableBody, 1, 1), "1-1");
    EXPECT_EQ(RetrieveEntryFromTableBody(tableBody, 2, 1), "1-2");
    EXPECT_EQ(RetrieveEntryFromTableBody(tableBody, 3, 4), "4-3");
}

// fixme: change the testcase
TEST_F(EnergyPlusFixture, OutputReportTabularTest_WriteResilienceBinsTableNonPreDef)
{
    std::string tableName = "test table";
    Array1D_string columnHead;
    columnHead.allocate(numColumnThermalTbl);
    columnHead(1) = "col 1";
    columnHead(2) = "col 2";
    columnHead(3) = "col 3";
    columnHead(4) = "col 4";
    columnHead(5) = "col 5";
    Array1D_int columnWidth;
    columnWidth.allocate(numColumnThermalTbl);
    columnWidth = 10;
    Array1D_string rowHead;
    Array2D_string tableBody;
    state->dataGlobal->NumOfZones = 2;
    int numZone = state->dataGlobal->NumOfZones;
    state->dataHeatBal->Zone.allocate(numZone);
    state->dataHeatBal->Resilience.allocate(numZone);
    state->dataHeatBal->Zone(1).Name = "Zone 1";
    state->dataHeatBal->Zone(2).Name = "Zone 2";
    int rowNum = numZone + 4;
    rowHead.allocate(rowNum);
    tableBody.allocate(numColumnThermalTbl, rowNum);
    Real64 unitConvMultiplier = 1.0;

    for (int zone_i = 1; zone_i <= numZone; zone_i++) {
        for (int j = 0; j < numColumnThermalTbl; j++) {
            (state->dataHeatBal->Resilience(zone_i).ZoneHeatIndexHourBins).at(j) = std::pow(j, 2) * zone_i;
        }
    }

    std::array<Real64, numColumnThermalTbl> DataHeatBalance::ZoneResilience::*ptrHeatIndex = &DataHeatBalance::ZoneResilience::ZoneHeatIndexHourBins;
    WriteResilienceBinsTableNonPreDefUseZoneData<numColumnThermalTbl>(
        *state, tableName, columnHead, columnWidth, ptrHeatIndex, rowHead, tableBody, unitConvMultiplier);
    for (int zone_i = 1; zone_i <= numZone; zone_i++) {
        for (int j = 0; j < numColumnThermalTbl; j++) {
            EXPECT_EQ(tableBody(j + 1, zone_i), RealToStr(std::pow(j, 2) * zone_i * 1.0, 2));
        }
    }

    for (int j = 0; j < numColumnThermalTbl; j++) {
        EXPECT_EQ(tableBody(j + 1, numZone + 1), RealToStr(std::pow(j, 2) * 1 * 1.0, 2));                                    // min
        EXPECT_EQ(tableBody(j + 1, numZone + 2), RealToStr(std::pow(j, 2) * 2 * 1.0, 2));                                    // max
        EXPECT_EQ(tableBody(j + 1, numZone + 3), RealToStr((std::pow(j, 2) * 1 * 1.0 + std::pow(j, 2) * 2 * 1.0) / 2.0, 2)); // mean
        EXPECT_EQ(tableBody(j + 1, numZone + 4), RealToStr(std::pow(j, 2) * 1 * 1.0 + std::pow(j, 2) * 2 * 1.0, 2));         // sum
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_WriteSETHoursTableNonPreDef)
{

    Array1D_string columnHead;
    columnHead.allocate(numColumnThermalTbl);
    columnHead(1) = "col 1";
    columnHead(2) = "col 2";
    columnHead(3) = "col 3";
    columnHead(4) = "col 4";
    columnHead(5) = "col 5";
    Array1D_int columnWidth;
    columnWidth.allocate(numColumnThermalTbl);
    columnWidth = 10;
    Array1D_string rowHead;
    Array2D_string tableBody;
    state->dataGlobal->NumOfZones = 2;
    int numZone = state->dataGlobal->NumOfZones;
    state->dataHeatBal->Zone.allocate(numZone);
    state->dataHeatBal->Resilience.allocate(numZone);
    state->dataHeatBal->Zone(1).Name = "Zone 1";
    state->dataHeatBal->Zone(2).Name = "Zone 2";

    state->dataHeatBal->Zone.allocate(numZone);

    int encodedMonDayHrMin;
    for (int zone_i = 1; zone_i <= numZone; zone_i++) {
        for (int i = 0; i < 5; i++) {
            (state->dataHeatBal->Resilience(zone_i).ZoneLowSETHours).at(i) = float(zone_i) * std::pow(-1.0, float(i)) * std::pow(float(i), 2.0);
        }
        General::EncodeMonDayHrMin(encodedMonDayHrMin, 1, 1, 5 * zone_i, 30);
        (state->dataHeatBal->Resilience(zone_i).ZoneLowSETHours).at(4) = encodedMonDayHrMin;
    }

    std::string tableName = "Heating SET Degree-Hours";

    rowHead.allocate(state->dataGlobal->NumOfZones + 3);
    tableBody.allocate(numColumnThermalTbl, state->dataGlobal->NumOfZones + 3);
    Real64 unitConvMultiplier = 1.0;

    std::array<Real64, numColumnThermalTbl> DataHeatBalance::ZoneResilience::*ptrZoneLowSETHours = &DataHeatBalance::ZoneResilience::ZoneLowSETHours;
    WriteSETHoursTableNonPreDefUseZoneData(
        *state, numColumnThermalTbl, tableName, columnHead, columnWidth, ptrZoneLowSETHours, rowHead, tableBody, unitConvMultiplier);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 1, 1));
    EXPECT_EQ("-1.0", RetrieveEntryFromTableBody(tableBody, 1, 2));
    EXPECT_EQ("4.00", RetrieveEntryFromTableBody(tableBody, 1, 3));
    EXPECT_EQ("-9.0", RetrieveEntryFromTableBody(tableBody, 1, 4));
    EXPECT_EQ("01-JAN-04:30", RetrieveEntryFromTableBody(tableBody, 1, 5));
    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 2, 1));
    EXPECT_EQ("-2.0", RetrieveEntryFromTableBody(tableBody, 2, 2));
    EXPECT_EQ("8.00", RetrieveEntryFromTableBody(tableBody, 2, 3));
    EXPECT_EQ("-18.0", RetrieveEntryFromTableBody(tableBody, 2, 4));
    EXPECT_EQ("01-JAN-09:30", RetrieveEntryFromTableBody(tableBody, 2, 5));

    unitConvMultiplier = 1.8;
    WriteSETHoursTableNonPreDefUseZoneData(
        *state, numColumnThermalTbl, tableName, columnHead, columnWidth, ptrZoneLowSETHours, rowHead, tableBody, unitConvMultiplier);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 1, 1));
    EXPECT_EQ("-1.8", RetrieveEntryFromTableBody(tableBody, 1, 2));
    EXPECT_EQ("7.20", RetrieveEntryFromTableBody(tableBody, 1, 3));
    //    duration hour don't change, 1, 1
    EXPECT_EQ("-9.0", RetrieveEntryFromTableBody(tableBody, 1, 4));
    EXPECT_EQ("01-JAN-04:30", RetrieveEntryFromTableBody(tableBody, 1, 5));
    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 2, 1));
    EXPECT_EQ("-3.6", RetrieveEntryFromTableBody(tableBody, 2, 2));
    EXPECT_EQ("14.40", RetrieveEntryFromTableBody(tableBody, 2, 3));
    //    duration hour don't change, 1, 1
    EXPECT_EQ("-18.0", RetrieveEntryFromTableBody(tableBody, 2, 4));
    EXPECT_EQ("01-JAN-09:30", RetrieveEntryFromTableBody(tableBody, 2, 5));
}

TEST_F(EnergyPlusFixture, OutputReportTabularTest_WriteHourOfSafetyTableNonPreDef)
{
    std::string tableName = "Hours of Safety for Cold Events";
    Array1D_string columnHead;
    columnHead.allocate(numColumnThermalTbl);
    Array1D_string rowHead;
    Array2D_string tableBody;
    Array1D_int columnWidth;
    columnWidth.allocate(numColumnThermalTbl);
    columnWidth = 10;
    state->dataGlobal->NumOfZones = 2;
    int numZone = state->dataGlobal->NumOfZones;
    state->dataHeatBal->Zone.allocate(numZone);
    state->dataHeatBal->Resilience.allocate(numZone);
    state->dataHeatBal->Zone(1).Name = "Zone 1";
    state->dataHeatBal->Zone(2).Name = "Zone 2";
    rowHead.allocate(numZone + 4);
    tableBody.allocate(numColumnThermalTbl, numZone + 4);
    columnHead(1) = "Hours of Safety [hr]";
    columnHead(2) = "End Time of the Safety Duration";
    columnHead(3) = "Safe Temperature Exceedance Hours [hr]";
    columnHead(4) = "Safe Temperature Exceedance OccupantHours [hr]";
    columnHead(5) = "Safe Temperature Exceedance OccupiedHours [hr]";

    state->dataHeatBal->Zone.allocate(numZone);
    int timeColumnIdx = 2;
    int encodedMonDayHrMin;
    for (int zone_i = 1; zone_i <= numZone; zone_i++) {
        for (int j = 0; j < numColumnThermalTbl; j++) {
            (state->dataHeatBal->Resilience(zone_i).ZoneColdHourOfSafetyBins).at(j) = std::pow(j, 2) * zone_i;
        }
        General::EncodeMonDayHrMin(encodedMonDayHrMin, 1, 1, 5 * zone_i, 30);
        (state->dataHeatBal->Resilience(zone_i).ZoneColdHourOfSafetyBins).at(timeColumnIdx - 1) = encodedMonDayHrMin;
    }

    std::array<Real64, numColumnThermalTbl> DataHeatBalance::ZoneResilience::*ptrColdHourOfSafetyBins =
        &DataHeatBalance::ZoneResilience::ZoneColdHourOfSafetyBins;
    WriteHourOfSafetyTableNonPreDefUseZoneData(
        *state, numColumnThermalTbl, tableName, columnHead, columnWidth, ptrColdHourOfSafetyBins, rowHead, tableBody, timeColumnIdx);

    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 1, 1));
    EXPECT_EQ("01-JAN-04:30", RetrieveEntryFromTableBody(tableBody, 1, 2));
    EXPECT_EQ("4.00", RetrieveEntryFromTableBody(tableBody, 1, 3));
    EXPECT_EQ("9.00", RetrieveEntryFromTableBody(tableBody, 1, 4));
    EXPECT_EQ("16.00", RetrieveEntryFromTableBody(tableBody, 1, 5));
    EXPECT_EQ("0.00", RetrieveEntryFromTableBody(tableBody, 2, 1));
    EXPECT_EQ("01-JAN-09:30", RetrieveEntryFromTableBody(tableBody, 2, 2));
    EXPECT_EQ("8.00", RetrieveEntryFromTableBody(tableBody, 2, 3));
    EXPECT_EQ("18.00", RetrieveEntryFromTableBody(tableBody, 2, 4));
    EXPECT_EQ("32.00", RetrieveEntryFromTableBody(tableBody, 2, 5));
}

TEST_F(SQLiteFixture, StatFile_TMYx)
{
    // Test for #9400 and #9420
    state->files.inStatFilePath.filePath =
        configured_source_directory() / "tst/EnergyPlus/unit/Resources/USA_IL_Chicago.OHare.Intl.AP.725300_TMYx.stat";

    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;

    SetPredefinedTables(*state);

    FillWeatherPredefinedEntries(*state);

    // Enable the ClimaticDataSummary report, in a future-proof way
    auto &predefReports = state->dataOutRptPredefined->reportName;
    auto it = std::find_if(predefReports.begin(), predefReports.end(), [](const auto &s) { return s.name == "ClimaticDataSummary"; });
    ASSERT_NE(it, predefReports.end());
    it->show = true;
    // EXPECT_EQ("ClimaticDataSummary", state->dataOutRptPredefined->reportName(1).name);
    // EXPECT_TRUE(state->dataOutRptPredefined->reportName(1).show);

    WritePredefinedTables(*state);

    // - Monthly Statistics for Liquid Precipitation [mm]
    //              Jan     Feb     Mar     Apr     May     Jun     Jul     Aug     Sep     Oct     Nov     Dec
    //    Total     41      18      65      48      110     106     75      53      24      143     63      61
    //  Max Hourly  5       5       7       6       7       7       5       6       5       7       6       6
    //  => Total: 807
    //     Max hourly: 7, occurs in Mar

    constexpr static auto queryStr = R"sqlite(SELECT Value FROM TabularDataWithStrings
        WHERE ReportName = "ClimaticDataSummary"
          AND TableName = "Weather Statistics File"
          AND RowName = "{}";
    )sqlite";
    EXPECT_EQ(807.0, execAndReturnFirstDouble(fmt::format(queryStr, "Annual Total Precipitation")));
    EXPECT_EQ(7.0, execAndReturnFirstDouble(fmt::format(queryStr, "Max Hourly Precipitation")));
    auto result = queryResult(fmt::format(queryStr, "Max Hourly Precipitation Occurs in"), "TabularDataWithStrings");
    ASSERT_EQ(1, result.size());
    ASSERT_FALSE(result[0].empty());
    EXPECT_EQ("Mar", result[0][0]);
}

TEST_F(SQLiteFixture, WriteVeriSumSpaceTables_Test)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularVeriSum = true;
    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).FrameDivider = 0;
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).FrameDivider = 0;
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 1;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).FrameDivider = 1;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).FrameDivider = 2;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 1;

    // frames
    state->dataHeatBal->TotFrameDivider = 2;
    state->dataSurface->FrameDivider.allocate(state->dataHeatBal->TotFrameDivider);
    state->dataSurface->FrameDivider(1).FrameWidth = 0.3;
    state->dataSurface->FrameDivider(2).FrameWidth = 0.2;

    // zone
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(1).FloorArea = 100.0; // 10 x 10
    state->dataHeatBal->Zone(1).Volume = 500.0;    // 10 x 10 x 5
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 200;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = 0.0; // state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;
    state->dataHeatBal->Zone(1).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes(1) = 1;

    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 2;
    state->dataHeatBal->Zone(2).Multiplier = 1.0;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(2).FloorArea = 100.0; // 10 x 10
    state->dataHeatBal->Zone(2).Volume = 500.0;    // 10 x 10 x 5
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).ExtGrossWallArea = 200.0;
    state->dataHeatBal->Zone(2).ExteriorTotalGroundSurfArea = 100.0;
    state->dataHeatBal->Zone(2).ExtWindowArea = 9.0; // state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;
    state->dataHeatBal->Zone(2).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(2).spaceIndexes(1) = 2;

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::None;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::None;

    SetupUnitConversions(*state);
    // Real64 areaConv = getSpecificUnitDivider(*state, "m2", "ft2");
    // Real64 volConv = getSpecificUnitDivider(*state, "m3", "ft3");

    // WriteVeriSumTable(*state);
    bool produceTabular = true;
    bool produceSQLite = true;
    state->dataGlobal->numSpaces = 2;

    state->dataOutRptTab->m_unitName = "[m]";
    state->dataOutRptTab->m_unitConv = 1.0;
    state->dataOutRptTab->m2_unitName = "[m2]";
    state->dataOutRptTab->m2_unitConvWVST = 1.0;
    state->dataOutRptTab->m3_unitName = "[m3]";
    state->dataOutRptTab->m3_unitConv = 1.0;
    state->dataOutRptTab->Wm2_unitName = "[W/m2]";
    state->dataOutRptTab->Wm2_unitConv = 1.0;

    state->dataGlobal->numSpaceTypes = 1;
    state->dataHeatBal->spaceTypes.allocate(state->dataGlobal->numSpaceTypes);

    state->dataHeatBal->TotPeople = 1;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);
    state->dataHeatBal->People(1).spaceIndex = 2;
    state->dataHeatBal->People(1).NumberOfPeople = 10;

    state->dataHeatBal->TotLights = 1;
    state->dataHeatBal->Lights.allocate(state->dataHeatBal->TotLights);
    state->dataHeatBal->Lights(1).spaceIndex = 2;
    state->dataHeatBal->Lights(1).DesignLevel = 10;

    state->dataHeatBal->space.allocate(state->dataGlobal->numSpaces);
    state->dataHeatBal->space(1).spaceTypeNum = 1;
    state->dataHeatBal->space(2).spaceTypeNum = 1;

    state->dataHeatBal->TotITEquip = 1;
    state->dataHeatBal->ZoneITEq.allocate(state->dataHeatBal->TotITEquip);
    state->dataHeatBal->ZoneITEq(1).spaceIndex = 2;
    state->dataHeatBal->ZoneITEq(1).DesignTotalPower = 5;

    state->dataHeatBal->Zone(1).Name = "Zone_1";
    state->dataHeatBal->Zone(2).Name = "Zone_2";
    state->dataHeatBal->space(1).Name = "Zone_Space_1";
    state->dataHeatBal->space(2).Name = "Zone_Space_2";
    // state->dataHeatBal->space(1).spaceType = "GENERAL";
    // state->dataHeatBal->space(2).spaceType = "GENERAL";

    state->dataHeatBal->space(1).solarEnclosureNum = 1;
    state->dataHeatBal->space(2).solarEnclosureNum = 2;

    state->dataViewFactor->EnclSolInfo.allocate(2);
    state->dataViewFactor->EnclSolInfo(1).Name = "Enclosure_1";
    state->dataViewFactor->EnclSolInfo(2).Name = "Enclosure_2";

    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;

    state->dataHeatBal->space(1).FloorArea = 100.0;
    state->dataHeatBal->space(2).FloorArea = 100.0;

    OutputReportTabular::writeVeriSumSpaceTables(*state, produceTabular, produceSQLite);

    auto tabularData = queryResult("SELECT * FROM TabularData;", "TabularData");
    auto strings = queryResult("SELECT * FROM Strings;", "Strings");
    auto stringTypes = queryResult("SELECT * FROM StringTypes;", "StringTypes");

    EXPECT_EQ(80ul, tabularData.size());
    // tabularDataIndex, reportNameIndex, reportForStringIndex, tableNameIndex, rowLabelIndex, columnLabelIndex, unitsIndex, simulationIndex, rowId,
    // columnId, value
    // Zone Areas
    EXPECT_EQ("      100.00", tabularData[0][10]);
    EXPECT_EQ("      100.00", tabularData[1][10]);
    // Space Areas
    EXPECT_EQ("      200.00", tabularData[2][10]);
    EXPECT_EQ("      200.00", tabularData[3][10]);

    EXPECT_EQ("Yes", tabularData[6][10]);
    EXPECT_EQ("Yes", tabularData[7][10]);

    // values
    EXPECT_NEAR(100.00, std::stod(tabularData[0][10]), 0.01);
    EXPECT_NEAR(100.00, std::stod(tabularData[1][10]), 0.01);
    EXPECT_NEAR(200.00, std::stod(tabularData[2][10]), 0.01);
    EXPECT_NEAR(200.00, std::stod(tabularData[3][10]), 0.01);
}

TEST_F(SQLiteFixture, DOASDirectToZone_ZoneMultiplierRemoved)
{
    std::string const idf_objects_1 = R"IDF(
  Version,23.1;

  Timestep,4;

  Building,
    NONE,                    !- Name
    0,                       !- North Axis {deg}
    Suburbs,                 !- Terrain
    0.039999999,             !- Loads Convergence Tolerance Value {W}
    0.4,                     !- Temperature Convergence Tolerance Value {deltaC}
    FullInteriorAndExterior, !- Solar Distribution
    25,                      !- Maximum Number of Warmup Days
    6;                       !- Minimum Number of Warmup Days

  Zone,
    Test Zone,               !- Name
    0,                       !- Direction of Relative North {deg}
    0,                       !- X Origin {m}
    0,                       !- Y Origin {m}
    0,                       !- Z Origin {m}
    ,                        !- Type
    10;                      !- Multiplier

  BuildingSurface:Detailed,
    Zn001:Flr001,            !- Name
    Floor,                   !- Surface Type
    FLOOR SLAB 8 IN,         !- Construction Name
    Test Zone,               !- Zone Name
    ,                        !- Space Name
    Adiabatic,               !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    NoSun,                   !- Sun Exposure
    NoWind,                  !- Wind Exposure
    1,                       !- View Factor to Ground
    ,                        !- Number of Vertices
    6,6,0,                   !- X,Y,Z ==> Vertex 1 {m}
    6,0,0,                   !- X,Y,Z ==> Vertex 2 {m}
    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}
    0,6,0;                   !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Roof001,           !- Name
    Roof,                    !- Surface Type
    ROOF34,                  !- Construction Name
    Test Zone,               !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0,                       !- View Factor to Ground
    ,                        !- Number of Vertices
    6,0,3,                   !- X,Y,Z ==> Vertex 1 {m}
    6,6,3,                   !- X,Y,Z ==> Vertex 2 {m}
    0,6,3,                   !- X,Y,Z ==> Vertex 3 {m}
    0,0,3;                   !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall001,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,               !- Construction Name
    Test Zone,               !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    0,0,3,                   !- X,Y,Z ==> Vertex 1 {m}
    0,0,0,                   !- X,Y,Z ==> Vertex 2 {m}
    6,0,0,                   !- X,Y,Z ==> Vertex 3 {m}
    6,0,3;                   !- X,Y,Z ==> Vertex 4 {m}

  FenestrationSurface:Detailed,
    Zn001:Wall001:Win001,    !- Name
    Window,                  !- Surface Type
    SimpleGlazing,           !- Construction Name
    Zn001:Wall001,           !- Building Surface Name
    ,                        !- Outside Boundary Condition Object
    0.5,                     !- View Factor to Ground
    ,                        !- Frame and Divider Name
    1,                       !- Multiplier
    ,                        !- Number of Vertices
    0.548,0,2,               !- X,Y,Z ==> Vertex 1 {m}
    0.548,0,1,               !- X,Y,Z ==> Vertex 2 {m}
    5.548,0,1,               !- X,Y,Z ==> Vertex 3 {m}
    5.548,0,2;               !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall002,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,            !- Construction Name
    Test Zone,               !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    0,6,3,                   !- X,Y,Z ==> Vertex 1 {m}
    0,6,0,                   !- X,Y,Z ==> Vertex 2 {m}
    0,0,0,                   !- X,Y,Z ==> Vertex 3 {m}
    0,0,3;                   !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall003,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,            !- Construction Name
    Test Zone,               !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    6,6,3,                   !- X,Y,Z ==> Vertex 1 {m}
    6,6,0,                   !- X,Y,Z ==> Vertex 2 {m}
    0,6,0,                   !- X,Y,Z ==> Vertex 3 {m}
    0,6,3;                   !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall004,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,            !- Construction Name
    Test Zone,               !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    6,0,3,  !- X,Y,Z ==> Vertex 1 {m}
    6,0,0,  !- X,Y,Z ==> Vertex 2 {m}
    6,6,0,  !- X,Y,Z ==> Vertex 3 {m}
    6,6,3;  !- X,Y,Z ==> Vertex 4 {m}

  Lights,
    TestZone Lights,       !- Name
    Test Zone,               !- Zone or ZoneList or Space or SpaceList Name
    Always On,               !- Schedule Name
    LightingLevel,           !- Design Level Calculation Method
    1878.6252,               !- Lighting Level {W}
    ,                        !- Watts per Zone Floor Area {W/m2}
    ,                        !- Watts per Person {W/person}
    0,                       !- Return Air Fraction
    0.2,                     !- Fraction Radiant
    0.2,                     !- Fraction Visible
    0,                       !- Fraction Replaceable
    GeneralLights;           !- End-Use Subcategory

  ElectricEquipment,
    TestZone ElecEq,         !- Name
    Test Zone,               !- Zone or ZoneList or Space or SpaceList Name
    Always On,               !- Schedule Name
    EquipmentLevel,          !- Design Level Calculation Method
    1928.751,                !- Design Level {W}
    ,                        !- Watts per Zone Floor Area {W/m2}
    ,                        !- Watts per Person {W/person}
    0,                       !- Fraction Latent
    0.3,                     !- Fraction Radiant
    0;                       !- Fraction Lost

  ZoneInfiltration:DesignFlowRate,
    TestZone Infiltration,   !- Name
    Test Zone,               !- Zone or ZoneList or Space or SpaceList Name
    Always On,               !- Schedule Name
    Flow/Zone,               !- Design Flow Rate Calculation Method
    0,                       !- Design Flow Rate {m3/s}
    ,                        !- Flow Rate per Floor Area {m3/s-m2}
    ,                        !- Flow Rate per Exterior Surface Area {m3/s-m2}
    ,                        !- Air Changes per Hour {1/hr}
    0,                       !- Constant Term Coefficient
    0,                       !- Temperature Term Coefficient
    0.2237,                  !- Velocity Term Coefficient
    0;                       !- Velocity Squared Term Coefficient

  ZoneControl:Thermostat,
    Test Zone Thermostat,  !- Name
    Test Zone,        !- Zone or ZoneList Name
    Test Zone Thermostat Schedule,  !- Control Type Schedule Name
    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type
    DualThermostat,          !- Control 1 Name
    ,                        !- Control 2 Object Type
    ,                        !- Control 2 Name
    ,                        !- Control 3 Object Type
    ,                        !- Control 3 Name
    ,                        !- Control 4 Object Type
    ,                        !- Control 4 Name
    0;                       !- Temperature Difference Between Cutout And Setpoint {deltaC}

  Schedule:Compact,
    Test Zone Thermostat Schedule,  !- Name
    Test Zone Thermostat Schedule Type Limits,  !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,4;          !- Field 3

  ScheduleTypeLimits,
    Test Zone Thermostat Schedule Type Limits,  !- Name
    0,                       !- Lower Limit Value
    4,                       !- Upper Limit Value
    DISCRETE;                !- Numeric Type

  ThermostatSetpoint:DualSetpoint,
    DualThermostat,          !- Name
    Heating Setpoints,       !- Heating Setpoint Temperature Schedule Name
    Cooling Setpoints;       !- Cooling Setpoint Temperature Schedule Name

  Schedule:Compact,
    Heating Setpoints,       !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,22.5;       !- Field 3

  ScheduleTypeLimits,
    Temperature,             !- Name
    -60,                     !- Lower Limit Value
    200,                     !- Upper Limit Value
    CONTINUOUS,              !- Numeric Type
    temperature;             !- Unit Type

  Schedule:Compact,
    Cooling Setpoints,       !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,23.5;       !- Field 3

  Schedule:Constant,Always On,On/Off,1;

  ScheduleTypeLimits,
    On/Off,                  !- Name
    0,                       !- Lower Limit Value
    1,                       !- Upper Limit Value
    Discrete,                !- Numeric Type
    availability;            !- Unit Type

  ZoneHVAC:EquipmentConnections,
    Test Zone,        !- Zone Name
    Test Zone Equipment List,  !- Zone Conditioning Equipment List Name
    Test Zone Inlet Node List,  !- Zone Air Inlet Node or NodeList Name
    ,                        !- Zone Air Exhaust Node or NodeList Name
    ZoneAirNode,             !- Zone Air Node Name
    Test Zone Return Node List;  !- Zone Return Air Node or NodeList Name

  NodeList,
    Test Zone Inlet Node List,  !- Name
    Node 5;                  !- Node 1 Name

  NodeList,
    Test Zone Return Node List,  !- Name
    Node 29;                 !- Node 1 Name

  ZoneHVAC:AirDistributionUnit,
    ADU VAV No Rht,          !- Name
    Node 5,                  !- Air Distribution Unit Outlet Node Name
    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type
    VAV No Rht;              !- Air Terminal Name

  AirTerminal:SingleDuct:VAV:NoReheat,
    VAV No Rht,              !- Name
    Always On,               !- Availability Schedule Name
    Node 5,                  !- Air Outlet Node Name
    Node 10,                 !- Air Inlet Node Name
    Autosize,                !- Maximum Air Flow Rate {m3/s}
    ,                        !- Zone Minimum Air Flow Input Method
    ,                        !- Constant Minimum Air Flow Fraction
    Autosize;                !- Fixed Minimum Air Flow Rate {m3/s}

  ZoneHVAC:EquipmentList,
    Test Zone Equipment List,  !- Name
    SequentialLoad,          !- Load Distribution Scheme
    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type
    ADU VAV No Rht,          !- Zone Equipment 1 Name
    1,                       !- Zone Equipment 1 Cooling Sequence
    1,                       !- Zone Equipment 1 Heating or No-Load Sequence
    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name
    ;                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name

  Sizing:Zone,
    Test Zone,               !- Zone or ZoneList Name
    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method
    12,                      !- Zone Cooling Design Supply Air Temperature {C}
    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}
    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method
    50,                      !- Zone Heating Design Supply Air Temperature {C}
    11.11,                   !- Zone Heating Design Supply Air Temperature Difference {deltaC}
    0.0085,                  !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    SZ DSOA TestZone,        !- Design Specification Outdoor Air Object Name
    0,                       !- Zone Heating Sizing Factor
    0,                       !- Zone Cooling Sizing Factor
    DesignDay,               !- Cooling Design Air Flow Method
    0,                       !- Cooling Design Air Flow Rate {m3/s}
    0.000762,                !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}
    0,                       !- Cooling Minimum Air Flow {m3/s}
    0,                       !- Cooling Minimum Air Flow Fraction
    DesignDay,               !- Heating Design Air Flow Method
    0,                       !- Heating Design Air Flow Rate {m3/s}
    0.002032,                !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}
    0.1415762,               !- Heating Maximum Air Flow {m3/s}
    0.3,                     !- Heating Maximum Air Flow Fraction
    ,                        !- Design Specification Zone Air Distribution Object Name
    Yes,                     !- Account for Dedicated Outdoor Air System
    ,                        !- Dedicated Outdoor Air System Control Strategy
    ,                        !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}
    ,                        !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}
    Sensible Load Only No Latent Load,  !- Zone Load Sizing Method
    HumidityRatioDifference, !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method
    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.005,                   !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}
    HumidityRatioDifference, !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method
    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.005;                   !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}

)IDF";

    std::string const idf_objects_2 = R"IDF(

  DesignSpecification:OutdoorAir,
    SZ DSOA TestZone,        !- Name
    Sum,                     !- Outdoor Air Method
    0.0009,                  !- Outdoor Air Flow per Person {m3/s-person}
    0.01020,                 !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}
    0,                       !- Outdoor Air Flow per Zone {m3/s}
    0;                       !- Outdoor Air Flow Air Changes per Hour {1/hr}

  SimulationControl,
    Yes,                     !- Do Zone Sizing Calculation
    Yes,                     !- Do System Sizing Calculation
    No,                      !- Do Plant Sizing Calculation
    Yes,                     !- Run Simulation for Sizing Periods
    No,                      !- Run Simulation for Weather File Run Periods
    No,                      !- Do HVAC Sizing Simulation for Sizing Periods
    ;                        !- Maximum Number of HVAC Sizing Simulation Passes

  GlobalGeometryRules,
    UpperLeftCorner,         !- Starting Vertex Position
    Counterclockwise,        !- Vertex Entry Direction
    Relative,                !- Coordinate System
    Relative,                !- Daylighting Reference Point Coordinate System
    Relative;                !- Rectangular Surface Coordinate System

  Material,
    B6 - 4 IN DENSE INSULATION,  !- Name
    VeryRough,               !- Roughness
    0.101559364,             !- Thickness {m}
    0.04323943,              !- Conductivity {W/m-K}
    91.30524,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.5,                     !- Solar Absorptance
    0.5;                     !- Visible Absorptance

  Material,
    C10 - 8 IN HW CONCRETE,  !- Name
    MediumRough,             !- Roughness
    0.2033016,               !- Thickness {m}
    1.729577,                !- Conductivity {W/m-K}
    2242.585,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.65,                    !- Solar Absorptance
    0.65;                    !- Visible Absorptance

  Material,
    C4 - 4 IN COMMON BRICK,  !- Name
    Rough,                   !- Roughness
    0.1014984,               !- Thickness {m}
    0.7264224,               !- Conductivity {W/m-K}
    1922.216,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.76,                    !- Solar Absorptance
    0.76;                    !- Visible Absorptance

  Material,
    E2 - 1 / 2 IN SLAG OR STONE,  !- Name
    Rough,                   !- Roughness
    0.012710161,             !- Thickness {m}
    1.435549,                !- Conductivity {W/m-K}
    881.0155,                !- Density {kg/m3}
    1673.6,                  !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.55,                    !- Solar Absorptance
    0.55;                    !- Visible Absorptance

  Material,
    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Name
    Rough,                   !- Roughness
    0.0095402403,            !- Thickness {m}
    0.1902535,               !- Conductivity {W/m-K}
    1121.292,                !- Density {kg/m3}
    1673.6,                  !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.75,                    !- Solar Absorptance
    0.75;                    !- Visible Absorptance

  WindowMaterial:SimpleGlazingSystem,
    Theoretical Glazing,     !- Name
    3.0,                     !- U-Factor {W/m2-K}
    0.5,                     !- Solar Heat Gain Coefficient
    0.4;                     !- Visible Transmittance

  Construction,
    ExteriorWall,            !- Name
    C4 - 4 IN COMMON BRICK;  !- Layer 2

  Construction,
    FLOOR SLAB 8 IN,         !- Name
    C10 - 8 IN HW CONCRETE;  !- Outside Layer

  Construction,
    ROOF34,                  !- Name
    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer
    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Layer 2
    B6 - 4 IN DENSE INSULATION;  !- Layer 3

  Construction,
    SimpleGlazing,           !- Name
    Theoretical Glazing;     !- Outside Layer

  OutdoorAir:Node,
    Model Outdoor Air Node;  !- Name

  AirLoopHVAC,
    DOAS System,             !- Name
    ,                        !- Controller List Name
    DOAS SystemAvailability Manager List,  !- Availability Manager List Name
    Autosize,                !- Design Supply Air Flow Rate {m3/s}
    DOAS System Supply Branches,  !- Branch List Name
    ,                        !- Connector List Name
    Node 1,                  !- Supply Side Inlet Node Name
    Node 4,                  !- Demand Side Outlet Node Name
    DOAS System Demand Inlet Nodes,  !- Demand Side Inlet Node Names
    DOAS System Supply Outlet Nodes,  !- Supply Side Outlet Node Names
    1;                       !- Design Return Air Flow Fraction of Supply Air Flow

  NodeList,
    DOAS System Supply Outlet Nodes,  !- Name
    Node 2;                  !- Node 1 Name

  NodeList,
    DOAS System Demand Inlet Nodes,  !- Name
    Node 3;                  !- Node 1 Name

  Sizing:System,
    DOAS System,             !- AirLoop Name
    Sensible,                !- Type of Load to Size On
    Autosize,                !- Design Outdoor Air Flow Rate {m3/s}
    0.3,                     !- Central Heating Maximum System Air Flow Ratio
    7,                       !- Preheat Design Temperature {C}
    0.008,                   !- Preheat Design Humidity Ratio {kgWater/kgDryAir}
    12.8,                    !- Precool Design Temperature {C}
    0.008,                   !- Precool Design Humidity Ratio {kgWater/kgDryAir}
    12.8,                    !- Central Cooling Design Supply Air Temperature {C}
    40,                      !- Central Heating Design Supply Air Temperature {C}
    NonCoincident,           !- Type of Zone Sum to Use
    Yes,                     !- 100% Outdoor Air in Cooling
    Yes,                     !- 100% Outdoor Air in Heating
    0.0085,                  !- Central Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.008,                   !- Central Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    DesignDay,               !- Cooling Supply Air Flow Rate Method
    0,                       !- Cooling Supply Air Flow Rate {m3/s}
    0.0099676501,            !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}
    1,                       !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate
    3.9475456e-05,           !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}
    DesignDay,               !- Heating Supply Air Flow Rate Method
    0,                       !- Heating Supply Air Flow Rate {m3/s}
    0.0099676501,            !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}
    1,                       !- Heating Fraction of Autosized Heating Supply Air Flow Rate
    1,                       !- Heating Fraction of Autosized Cooling Supply Air Flow Rate
    3.1588213e-05,           !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}
    ZoneSum,                 !- System Outdoor Air Method
    1,                       !- Zone Maximum Outdoor Air Fraction {dimensionless}
    CoolingDesignCapacity,   !- Cooling Design Capacity Method
    Autosize,                !- Cooling Design Capacity {W}
    234.7,                   !- Cooling Design Capacity Per Floor Area {W/m2}
    1,                       !- Fraction of Autosized Cooling Design Capacity
    HeatingDesignCapacity,   !- Heating Design Capacity Method
    Autosize,                !- Heating Design Capacity {W}
    157,                     !- Heating Design Capacity Per Floor Area {W/m2}
    1,                       !- Fraction of Autosized Heating Design Capacity
    OnOff,                   !- Central Cooling Capacity Control Method
    Autosize;                !- Occupant Diversity

  AvailabilityManagerAssignmentList,
    DOAS SystemAvailability Manager List,  !- Name
    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type
    DOAS System Availability Manager;  !- Availability Manager 1 Name

  AvailabilityManager:Scheduled,
    DOAS System Availability Manager,  !- Name
    Always On;      !- Schedule Name

  BranchList,
    DOAS System Supply Branches,  !- Name
    DOAS System Main Branch;  !- Branch 1 Name

  Branch,
    DOAS System Main Branch, !- Name
    ,                        !- Pressure Drop Curve Name
    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type
    DOAS System OA System,   !- Component 1 Name
    Node 1,                  !- Component 1 Inlet Node Name
    Node 8,                  !- Component 1 Outlet Node Name
    CoilSystem:Cooling:DX,   !- Component 2 Object Type
    Main DX Clg Coil System, !- Component 2 Name
    Node 8,                  !- Component 2 Inlet Node Name
    Node 11,                 !- Component 2 Outlet Node Name
    Coil:Heating:Electric,   !- Component 3 Object Type
    Elec Htg Coil,           !- Component 3 Name
    Node 11,                 !- Component 3 Inlet Node Name
    Node 9,                  !- Component 3 Outlet Node Name
    Fan:SystemModel,         !- Component 4 Object Type
    DOAS System Supply Fan,  !- Component 4 Name
    Node 9,                  !- Component 4 Inlet Node Name
    Node 2;                  !- Component 4 Outlet Node Name

)IDF";

    std::string const idf_objects_3 = R"IDF(

  AirLoopHVAC:OutdoorAirSystem,
    DOAS System OA System,  !- Name
    DOAS System OA System Controller List,  !- Controller List Name
    DOAS System OA System Equipment List;  !- Outdoor Air Equipment List Name

  AirLoopHVAC:ControllerList,
    DOAS System OA System Controller List,  !- Name
    Controller:OutdoorAir,   !- Controller 1 Object Type
    DOAS System OA Controller;  !- Controller 1 Name

  Controller:OutdoorAir,
    DOAS System OA Controller,  !- Name
    Node 7,                  !- Relief Air Outlet Node Name
    Node 1,                  !- Return Air Node Name
    Node 8,                  !- Mixed Air Node Name
    Node 6,                  !- Actuator Node Name
    0,                       !- Minimum Outdoor Air Flow Rate {m3/s}
    Autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}
    NoEconomizer,            !- Economizer Control Type
    ModulateFlow,            !- Economizer Control Action Type
    28,                      !- Economizer Maximum Limit Dry-Bulb Temperature {C}
    64000,                   !- Economizer Maximum Limit Enthalpy {J/kg}
    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}
    ,                        !- Electronic Enthalpy Limit Curve Name
    ,                        !- Economizer Minimum Limit Dry-Bulb Temperature {C}
    NoLockout,               !- Lockout Type
    FixedMinimum,            !- Minimum Limit Type
    ,                        !- Minimum Outdoor Air Schedule Name
    ,                        !- Minimum Fraction of Outdoor Air Schedule Name
    ,                        !- Maximum Fraction of Outdoor Air Schedule Name
    ,                        !- Mechanical Ventilation Controller Name
    ,                        !- Time of Day Economizer Control Schedule Name
    No,                      !- High Humidity Control
    ,                        !- Humidistat Control Zone Name
    ,                        !- High Humidity Outdoor Air Flow Ratio
    Yes,                     !- Control High Indoor Humidity Based on Outdoor Humidity Ratio
    BypassWhenWithinEconomizerLimits;  !- Heat Recovery Bypass Control Type

  OutdoorAir:NodeList,
    Node 6;                  !- Node or NodeList Name 1

  AirLoopHVAC:OutdoorAirSystem:EquipmentList,
    DOAS System OA System Equipment List,  !- Name
    OutdoorAir:Mixer,        !- Component 1 Object Type
    OA Mixer;  !- Component 1 Name

  OutdoorAir:Mixer,
    OA Mixer,                !- Name
    Node 8,                  !- Mixed Air Node Name
    Node 6,                  !- Outdoor Air Stream Node Name
    Node 7,                  !- Relief Air Stream Node Name
    Node 1;                  !- Return Air Stream Node Name

  SetpointManager:MixedAir,
    Node 8 OS Default SPM,   !- Name
    Temperature,             !- Control Variable
    Node 2,                  !- Reference Setpoint Node Name
    Node 9,                  !- Fan Inlet Node Name
    Node 2,                  !- Fan Outlet Node Name
    Node 8;                  !- Setpoint Node or NodeList Name

  CoilSystem:Cooling:DX,
    Main DX Clg Coil System, !- Name
    Always On,               !- Availability Schedule Name
    Node 8,                  !- DX Cooling Coil System Inlet Node Name
    Node 11,                 !- DX Cooling Coil System Outlet Node Name
    Node 11,                 !- DX Cooling Coil System Sensor Node Name
    Coil:Cooling:DX:SingleSpeed,!- Cooling Coil Object Type
    Main DX Clg Coil;        !- Cooling Coil Name

  Coil:Cooling:DX:SingleSpeed,
    Main DX Clg Coil,        !- Name
    Always On,               !- Availability Schedule Name
    Autosize,                !- Gross Rated Total Cooling Capacity {W}
    0.75,                    !- Gross Rated Sensible Heat Ratio
    3.0,                     !- Gross Rated Cooling COP {W/W}
    Autosize,                !- Rated Air Flow Rate {m3/s}
    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
    Node 8,                  !- Air Inlet Node Name
    Node 11,                 !- Air Outlet Node Name
    Curve Biquadratic,       !- Total Cooling Capacity Function of Temperature Curve Name
    Curve Quadratic,         !- Total Cooling Capacity Function of Flow Fraction Curve Name
    Curve Biquadratic,       !- Energy Input Ratio Function of Temperature Curve Name
    Curve Quadratic,         !- Energy Input Ratio Function of Flow Fraction Curve Name
    Curve Quadratic,         !- Part Load Fraction Correlation Curve Name
    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}
    1000,                    !- Nominal Time for Condensate Removal to Begin {s}
    0.4,                     !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}
    4,                       !- Maximum Cycling Rate {cycles/hr}
    45;                      !- Latent Capacity Time Constant {s}

  SetpointManager:MixedAir,
    Node 11 OS Default SPM,  !- Name
    Temperature,             !- Control Variable
    Node 2,                  !- Reference Setpoint Node Name
    Node 9,                  !- Fan Inlet Node Name
    Node 2,                  !- Fan Outlet Node Name
    Node 11;                 !- Setpoint Node or NodeList Name

  Curve:Biquadratic,
    Curve Biquadratic,       !- Name
    1.0,                     !- Coefficient1 Constant
    0.0,                     !- Coefficient2 x
    0.0,                     !- Coefficient3 x**2
    0.0,                     !- Coefficient4 y
    0.0,                     !- Coefficient5 y**2
    0.0,                     !- Coefficient6 x*y
    0,                       !- Minimum Value of x
    50,                      !- Maximum Value of x
    0,                       !- Minimum Value of y
    50;                      !- Maximum Value of y

  Curve:Quadratic,
    Curve Quadratic,         !- Name
    1.0,                     !- Coefficient1 Constant
    0.0,                     !- Coefficient2 x
    0.0,                     !- Coefficient3 x**2
    0.9,                     !- Minimum Value of x
    1.0;                     !- Maximum Value of x

  Coil:Heating:Electric,
    Elec Htg Coil,           !- Name
    Always On,               !- Availability Schedule Name
    1,                       !- Efficiency
    Autosize,                !- Nominal Capacity {W}
    Node 11,                 !- Air Inlet Node Name
    Node 9,                  !- Air Outlet Node Name
    Node 9;                  !- Temperature Setpoint Node Name

  SetpointManager:MixedAir,
    Node 9 OS Default SPM,   !- Name
    Temperature,             !- Control Variable
    Node 2,                  !- Reference Setpoint Node Name
    Node 9,                  !- Fan Inlet Node Name
    Node 2,                  !- Fan Outlet Node Name
    Node 9;                  !- Setpoint Node or NodeList Name

  Fan:SystemModel,
    DOAS System Supply Fan,  !- Name
    Always On,               !- Availability Schedule Name
    Node 9,                  !- Air Inlet Node Name
    Node 2,                  !- Air Outlet Node Name
    Autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Continuous,              !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500,                     !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1,                       !- Motor In Air Stream Fraction
    Autosize,                !- Design Electric Power Consumption {W}
    PowerPerFlowPerPressure, !- Design Power Sizing Method
    840,                     !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    1.66667,                 !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.524386048,             !- Fan Total Efficiency
    VAV Fan Curve,           !- Electric Power Function of Flow Fraction Curve Name
    ,                        !- Night Ventilation Mode Pressure Rise {Pa}
    ,                        !- Night Ventilation Mode Flow Fraction
    ,                        !- Motor Loss Zone Name
    0,                       !- Motor Loss Radiative Fraction
    General,                 !- End-Use Subcategory
    1;                       !- Number of Speeds

  Curve:Quartic,
    VAV Fan Curve,           !- Name
    0.040759894,             !- Coefficient1 Constant
    0.08804497,              !- Coefficient2 x
    -0.07292612,             !- Coefficient3 x**2
    0.943739823,             !- Coefficient4 x**3
    0,                       !- Coefficient5 x**4
    0,                       !- Minimum Value of x
    1,                       !- Maximum Value of x
    0,                       !- Minimum Curve Output
    1,                       !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

  SetpointManager:Warmest,
    DOAS System SetPoint Manager Warmest,  !- Name
    Temperature,             !- Control Variable
    DOAS System,         !- HVAC Air Loop Name
    12.2,                    !- Minimum Setpoint Temperature {C}
    15.6,                    !- Maximum Setpoint Temperature {C}
    MaximumTemperature,      !- Strategy
    Node 2;                  !- Setpoint Node or NodeList Name

  AirLoopHVAC:SupplyPath,
    DOAS System Node 3 Supply Path,  !- Name
    Node 3,                  !- Supply Air Path Inlet Node Name
    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type
    Air Loop HVAC Zone Splitter 1;  !- Component 1 Name

  AirLoopHVAC:ZoneSplitter,
    Air Loop HVAC Zone Splitter 1,  !- Name
    Node 3,                  !- Inlet Node Name
    Node 10;                 !- Outlet 1 Node Name

  AirLoopHVAC:ReturnPath,
    DOAS System Return Path,  !- Name
    Node 4,                  !- Return Air Path Outlet Node Name
    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type
    Air Loop HVAC Zone Mixer 1;  !- Component 1 Name

  AirLoopHVAC:ZoneMixer,
    Air Loop HVAC Zone Mixer 1,  !- Name
    Node 4,                  !- Outlet Node Name
    Node 29;                 !- Inlet 1 Node Name

  Site:Location,
    Atlanta Hartsfield Intl Ap,  !- Name
    33.63,                   !- Latitude {deg}
    -84.43,                  !- Longitude {deg}
    -5,                      !- Time Zone {hr}
    308;                     !- Elevation {m}

  SizingPeriod:DesignDay,
    Atlanta Hartsfield Intl Ap Ann Clg .4% Condns DB=>MWB,  !- Name
    7,                       !- Month
    21,                      !- Day of Month
    SummerDesignDay,         !- Day Type
    34.4,                    !- Maximum Dry-Bulb Temperature {C}
    9.5,                     !- Daily Dry-Bulb Temperature Range {deltaC}
    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type
    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name
    Wetbulb,                 !- Humidity Condition Type
    23.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}
    ,                        !- Humidity Condition Day Schedule Name
    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}
    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}
    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}
    97679,                   !- Barometric Pressure {Pa}
    4,                       !- Wind Speed {m/s}
    300,                     !- Wind Direction {deg}
    No,                      !- Rain Indicator
    No,                      !- Snow Indicator
    No,                      !- Daylight Saving Time Indicator
    ASHRAETau,               !- Solar Model Indicator
    ,                        !- Beam Solar Day Schedule Name
    ,                        !- Diffuse Solar Day Schedule Name
    0.556,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}
    1.779,                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}
    ;                       !- Sky Clearness

  SizingPeriod:DesignDay,
    Atlanta Hartsfield Intl Ap Ann Htg 99.6% Condns DB,  !- Name
    1,                       !- Month
    21,                      !- Day of Month
    WinterDesignDay,         !- Day Type
    -6.3,                    !- Maximum Dry-Bulb Temperature {C}
    0,                       !- Daily Dry-Bulb Temperature Range {deltaC}
    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type
    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name
    Wetbulb,                 !- Humidity Condition Type
    -6.3,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}
    ,                        !- Humidity Condition Day Schedule Name
    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}
    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}
    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}
    97679,                   !- Barometric Pressure {Pa}
    5.3,                     !- Wind Speed {m/s}
    320,                     !- Wind Direction {deg}
    No,                      !- Rain Indicator
    No,                      !- Snow Indicator
    No,                      !- Daylight Saving Time Indicator
    ASHRAEClearSky,          !- Solar Model Indicator
    ,                        !- Beam Solar Day Schedule Name
    ,                        !- Diffuse Solar Day Schedule Name
    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}
    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}
    0;                       !- Sky Clearness

  OutputControl:Table:Style,
    TabAndHTML,              !- Column Separator
    InchPound;               !- Unit Conversion

  Output:VariableDictionary,IDF,Unsorted;

  Output:SQLite,
    SimpleAndTabular;        !- Option Type

  Output:Table:SummaryReports,
    AllSummary,              !- Report 1 Name
    AllSummaryAndSizingPeriod;  !- Report 2 Name
)IDF";

    std::string const idf_objects = idf_objects_1 + idf_objects_2 + idf_objects_3;
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    ManageSimulation(*state); // run the design days
    auto &finalSysSizing = state->dataSize->FinalSysSizing(1);
    EXPECT_ENUM_EQ(finalSysSizing.coolingPeakLoad, DataSizing::PeakLoad::SensibleCooling);

    // get the total 'DOAS Direct to Zone' cooling peak load component
    std::string query_total("SELECT Value From TabularDataWithStrings"
                            "  WHERE TableName = 'Estimated Cooling Peak Load Components'"
                            "  AND ReportName = 'Zone Component Load Summary'"
                            "  AND ColumnName = 'Total'"
                            "  AND RowName = 'DOAS Direct to Zone'");
    // check the value from result records
    Real64 return_val_total = execAndReturnFirstDouble(query_total);
    EXPECT_EQ(return_val_total, 598.2);

    // get the sensible instant 'DOAS Direct to Zone' cooling peak load component
    std::string query_sensible_instant("SELECT Value From TabularDataWithStrings"
                                       "  WHERE TableName = 'Estimated Cooling Peak Load Components'"
                                       "  AND ReportName = 'Zone Component Load Summary'"
                                       "  AND ColumnName = 'Sensible - Instant'"
                                       "  AND RowName = 'DOAS Direct to Zone'");
    // check the value from result records
    Real64 return_val_sensible_instant = execAndReturnFirstDouble(query_sensible_instant);
    EXPECT_EQ(return_val_sensible_instant, 600.28);
}

TEST_F(SQLiteFixture, UpdateSizing_EndSysSizingCalc)
{
    std::string const idf_objects_1 = R"IDF(
  Version,23.1;

  Timestep,4;

  Building,
    NONE,                    !- Name
    0,                       !- North Axis {deg}
    Suburbs,                 !- Terrain
    0.039999999,             !- Loads Convergence Tolerance Value {W}
    0.4,                     !- Temperature Convergence Tolerance Value {deltaC}
    FullInteriorAndExterior, !- Solar Distribution
    25,                      !- Maximum Number of Warmup Days
    6;                       !- Minimum Number of Warmup Days

  Zone,
    Thermal Zone one,        !- Name
    0,                       !- Direction of Relative North {deg}
    0,                       !- X Origin {m}
    0,                       !- Y Origin {m}
    0;                       !- Z Origin {m}

  BuildingSurface:Detailed,
    Zn001:Flr001,            !- Name
    Floor,                   !- Surface Type
    FLOOR SLAB 8 IN,         !- Construction Name
    Thermal Zone one,        !- Zone Name
    ,                        !- Space Name
    Adiabatic,               !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    NoSun,                   !- Sun Exposure
    NoWind,                  !- Wind Exposure
    1,                       !- View Factor to Ground
    ,                        !- Number of Vertices
    6,6,0,  !- X,Y,Z ==> Vertex 1 {m}
    6,0,0,  !- X,Y,Z ==> Vertex 2 {m}
    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}
    0,6,0;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Roof001,           !- Name
    Roof,                    !- Surface Type
    ROOF34,                  !- Construction Name
    Thermal Zone one,        !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0,                       !- View Factor to Ground
    ,                        !- Number of Vertices
    6,0,3,  !- X,Y,Z ==> Vertex 1 {m}
    6,6,3,  !- X,Y,Z ==> Vertex 2 {m}
    0,6,3,  !- X,Y,Z ==> Vertex 3 {m}
    0,0,3;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall001,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,               !- Construction Name
    Thermal Zone one,        !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    0,0,3,  !- X,Y,Z ==> Vertex 1 {m}
    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}
    6,0,0,  !- X,Y,Z ==> Vertex 3 {m}
    6,0,3;  !- X,Y,Z ==> Vertex 4 {m}

  FenestrationSurface:Detailed,
    Zn001:Wall001:Win001,    !- Name
    Window,                  !- Surface Type
    SimpleGlazing,           !- Construction Name
    Zn001:Wall001,           !- Building Surface Name
    ,                        !- Outside Boundary Condition Object
    0.5,                     !- View Factor to Ground
    ,                        !- Frame and Divider Name
    1,                       !- Multiplier
    ,                        !- Number of Vertices
    0.548,0,2,  !- X,Y,Z ==> Vertex 1 {m}
    0.548,0,1,  !- X,Y,Z ==> Vertex 2 {m}
    5.548,0,1,  !- X,Y,Z ==> Vertex 3 {m}
    5.548,0,2;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall002,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,            !- Construction Name
    Thermal Zone one,        !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    0,6,3,  !- X,Y,Z ==> Vertex 1 {m}
    0,6,0,  !- X,Y,Z ==> Vertex 2 {m}
    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}
    0,0,3;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall003,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,            !- Construction Name
    Thermal Zone one,        !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    6,6,3,  !- X,Y,Z ==> Vertex 1 {m}
    6,6,0,  !- X,Y,Z ==> Vertex 2 {m}
    0,6,0,  !- X,Y,Z ==> Vertex 3 {m}
    0,6,3;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall004,           !- Name
    Wall,                    !- Surface Type
    ExteriorWall,            !- Construction Name
    Thermal Zone one,        !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5,                     !- View Factor to Ground
    ,                        !- Number of Vertices
    6,0,3,  !- X,Y,Z ==> Vertex 1 {m}
    6,0,0,  !- X,Y,Z ==> Vertex 2 {m}
    6,6,0,  !- X,Y,Z ==> Vertex 3 {m}
    6,6,3;  !- X,Y,Z ==> Vertex 4 {m}

  Lights,
    West Zone Lights,        !- Name
    Thermal Zone one,        !- Zone or ZoneList or Space or SpaceList Name
    Always On,               !- Schedule Name
    LightingLevel,           !- Design Level Calculation Method
    1878.6252,               !- Lighting Level {W}
    ,                        !- Watts per Zone Floor Area {W/m2}
    ,                        !- Watts per Person {W/person}
    0,                       !- Return Air Fraction
    0.2,                     !- Fraction Radiant
    0.2,                     !- Fraction Visible
    0,                       !- Fraction Replaceable
    GeneralLights;           !- End-Use Subcategory

  ElectricEquipment,
    West Zone ElecEq,        !- Name
    Thermal Zone one,        !- Zone or ZoneList or Space or SpaceList Name
    Always On,               !- Schedule Name
    EquipmentLevel,          !- Design Level Calculation Method
    1928.751,                !- Design Level {W}
    ,                        !- Watts per Zone Floor Area {W/m2}
    ,                        !- Watts per Person {W/person}
    0,                       !- Fraction Latent
    0.3,                     !- Fraction Radiant
    0;                       !- Fraction Lost

  ZoneInfiltration:DesignFlowRate,
    West Zone Infiltration,  !- Name
    Thermal Zone one,        !- Zone or ZoneList or Space or SpaceList Name
    Always On,               !- Schedule Name
    Flow/Zone,               !- Design Flow Rate Calculation Method
    0,                       !- Design Flow Rate {m3/s}
    ,                        !- Flow Rate per Floor Area {m3/s-m2}
    ,                        !- Flow Rate per Exterior Surface Area {m3/s-m2}
    ,                        !- Air Changes per Hour {1/hr}
    0,                       !- Constant Term Coefficient
    0,                       !- Temperature Term Coefficient
    0.2237,                  !- Velocity Term Coefficient
    0;                       !- Velocity Squared Term Coefficient

  ZoneControl:Thermostat,
    Thermal Zone one Thermostat,  !- Name
    Thermal Zone one,        !- Zone or ZoneList Name
    Thermal Zone one Thermostat Schedule,  !- Control Type Schedule Name
    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type
    DualThermostat,          !- Control 1 Name
    ,                        !- Control 2 Object Type
    ,                        !- Control 2 Name
    ,                        !- Control 3 Object Type
    ,                        !- Control 3 Name
    ,                        !- Control 4 Object Type
    ,                        !- Control 4 Name
    0;                       !- Temperature Difference Between Cutout And Setpoint {deltaC}

  Schedule:Compact,
    Thermal Zone one Thermostat Schedule,  !- Name
    Thermal Zone one Thermostat Schedule Type Limits,  !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,4;          !- Field 3

  ScheduleTypeLimits,
    Thermal Zone one Thermostat Schedule Type Limits,  !- Name
    0,                       !- Lower Limit Value
    4,                       !- Upper Limit Value
    DISCRETE;                !- Numeric Type

  ThermostatSetpoint:DualSetpoint,
    DualThermostat,          !- Name
    Heating Setpoints,       !- Heating Setpoint Temperature Schedule Name
    Cooling Setpoints;       !- Cooling Setpoint Temperature Schedule Name

  Schedule:Compact,
    Heating Setpoints,       !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,22.5;       !- Field 3

  ScheduleTypeLimits,
    Temperature,             !- Name
    -60,                     !- Lower Limit Value
    200,                     !- Upper Limit Value
    CONTINUOUS,              !- Numeric Type
    temperature;             !- Unit Type

  Schedule:Compact,
    Cooling Setpoints,       !- Name
    Temperature,             !- Schedule Type Limits Name
    Through: 12/31,          !- Field 1
    For: AllDays,            !- Field 2
    Until: 24:00,23.5;       !- Field 3

  Schedule:Constant,Always On,On/Off,1;

  ScheduleTypeLimits,
    On/Off,                  !- Name
    0,                       !- Lower Limit Value
    1,                       !- Upper Limit Value
    Discrete,                !- Numeric Type
    availability;            !- Unit Type

  ZoneHVAC:EquipmentConnections,
    Thermal Zone one,        !- Zone Name
    Thermal Zone one Equipment List,  !- Zone Conditioning Equipment List Name
    Thermal Zone one Inlet Node List,  !- Zone Air Inlet Node or NodeList Name
    ,                        !- Zone Air Exhaust Node or NodeList Name
    ZoneAirNode,             !- Zone Air Node Name
    Thermal Zone one Return Node List;  !- Zone Return Air Node or NodeList Name

  NodeList,
    Thermal Zone one Inlet Node List,  !- Name
    Node 5;                  !- Node 1 Name

  NodeList,
    Thermal Zone one Return Node List,  !- Name
    Node 29;                 !- Node 1 Name

  ZoneHVAC:AirDistributionUnit,
    ADU VAV No Rht,          !- Name
    Node 5,                  !- Air Distribution Unit Outlet Node Name
    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type
    VAV No Rht;              !- Air Terminal Name

  AirTerminal:SingleDuct:VAV:NoReheat,
    VAV No Rht,              !- Name
    Always On,               !- Availability Schedule Name
    Node 5,                  !- Air Outlet Node Name
    Node 10,                 !- Air Inlet Node Name
    Autosize,                !- Maximum Air Flow Rate {m3/s}
    ,                        !- Zone Minimum Air Flow Input Method
    ,                        !- Constant Minimum Air Flow Fraction
    Autosize;                !- Fixed Minimum Air Flow Rate {m3/s}

  ZoneHVAC:EquipmentList,
    Thermal Zone one Equipment List,  !- Name
    SequentialLoad,          !- Load Distribution Scheme
    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type
    ADU VAV No Rht,          !- Zone Equipment 1 Name
    1,                       !- Zone Equipment 1 Cooling Sequence
    1,                       !- Zone Equipment 1 Heating or No-Load Sequence
    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name
    ;                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name

  Sizing:Zone,
    Thermal Zone one,        !- Zone or ZoneList Name
    SupplyAirTemperature,    !- Zone Cooling Design Supply Air Temperature Input Method
    12,                      !- Zone Cooling Design Supply Air Temperature {C}
    11.11,                   !- Zone Cooling Design Supply Air Temperature Difference {deltaC}
    SupplyAirTemperature,    !- Zone Heating Design Supply Air Temperature Input Method
    50,                      !- Zone Heating Design Supply Air Temperature {C}
    11.11,                   !- Zone Heating Design Supply Air Temperature Difference {deltaC}
    0.0085,                  !- Zone Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.008,                   !- Zone Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    SZ DSOA West Zone,       !- Design Specification Outdoor Air Object Name
    0,                       !- Zone Heating Sizing Factor
    0,                       !- Zone Cooling Sizing Factor
    DesignDay,               !- Cooling Design Air Flow Method
    0,                       !- Cooling Design Air Flow Rate {m3/s}
    0.000762,                !- Cooling Minimum Air Flow per Zone Floor Area {m3/s-m2}
    0,                       !- Cooling Minimum Air Flow {m3/s}
    0,                       !- Cooling Minimum Air Flow Fraction
    DesignDay,               !- Heating Design Air Flow Method
    0,                       !- Heating Design Air Flow Rate {m3/s}
    0.002032,                !- Heating Maximum Air Flow per Zone Floor Area {m3/s-m2}
    0.1415762,               !- Heating Maximum Air Flow {m3/s}
    0.3,                     !- Heating Maximum Air Flow Fraction
    ,                        !- Design Specification Zone Air Distribution Object Name
    No,                      !- Account for Dedicated Outdoor Air System
    ,                        !- Dedicated Outdoor Air System Control Strategy
    ,                        !- Dedicated Outdoor Air Low Setpoint Temperature for Design {C}
    ,                        !- Dedicated Outdoor Air High Setpoint Temperature for Design {C}
    Sensible Load Only No Latent Load,  !- Zone Load Sizing Method
    HumidityRatioDifference, !- Zone Latent Cooling Design Supply Air Humidity Ratio Input Method
    ,                        !- Zone Dehumidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.005,                   !- Zone Cooling Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}
    HumidityRatioDifference, !- Zone Latent Heating Design Supply Air Humidity Ratio Input Method
    ,                        !- Zone Humidification Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.005;                   !- Zone Humidification Design Supply Air Humidity Ratio Difference {kgWater/kgDryAir}

)IDF";

    std::string const idf_objects_2 = R"IDF(

  DesignSpecification:OutdoorAir,
    SZ DSOA West Zone,       !- Name
    Sum,                     !- Outdoor Air Method
    0.00944,                 !- Outdoor Air Flow per Person {m3/s-person}
    0,                       !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}
    0,                       !- Outdoor Air Flow per Zone {m3/s}
    0;                       !- Outdoor Air Flow Air Changes per Hour {1/hr}

  SimulationControl,
    Yes,                     !- Do Zone Sizing Calculation
    Yes,                     !- Do System Sizing Calculation
    No,                      !- Do Plant Sizing Calculation
    Yes,                     !- Run Simulation for Sizing Periods
    No,                      !- Run Simulation for Weather File Run Periods
    No,                      !- Do HVAC Sizing Simulation for Sizing Periods
    ;                        !- Maximum Number of HVAC Sizing Simulation Passes

  ShadowCalculation,
    PolygonClipping,         !- Shading Calculation Method
    Periodic,                !- Shading Calculation Update Frequency Method
    20,                      !- Shading Calculation Update Frequency
    15000,                   !- Maximum Figures in Shadow Overlap Calculations
    SutherlandHodgman,       !- Polygon Clipping Algorithm
    512,                     !- Pixel Counting Resolution
    SimpleSkyDiffuseModeling,!- Sky Diffuse Modeling Algorithm
    No,                      !- Output External Shading Calculation Results
    No,                      !- Disable Self-Shading Within Shading Zone Groups
    No;                      !- Disable Self-Shading From Shading Zone Groups to Other Zones

  GlobalGeometryRules,
    UpperLeftCorner,         !- Starting Vertex Position
    Counterclockwise,        !- Vertex Entry Direction
    Relative,                !- Coordinate System
    Relative,                !- Daylighting Reference Point Coordinate System
    Relative;                !- Rectangular Surface Coordinate System

  Material,
    A1 - 1 IN STUCCO,        !- Name
    Smooth,                  !- Roughness
    0.025389841,             !- Thickness {m}
    0.6918309,               !- Conductivity {W/m-K}
    1858.142,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.92,                    !- Solar Absorptance
    0.92;                    !- Visible Absorptance

  Material,
    B5 - 1 IN DENSE INSULATION,  !- Name
    VeryRough,               !- Roughness
    0.025389841,             !- Thickness {m}
    0.04323943,              !- Conductivity {W/m-K}
    91.30524,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.5,                     !- Solar Absorptance
    0.5;                     !- Visible Absorptance

  Material,
    B6 - 4 IN DENSE INSULATION,  !- Name
    VeryRough,               !- Roughness
    0.101559364,             !- Thickness {m}
    0.04323943,              !- Conductivity {W/m-K}
    91.30524,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.5,                     !- Solar Absorptance
    0.5;                     !- Visible Absorptance

  Material,
    C10 - 8 IN HW CONCRETE,  !- Name
    MediumRough,             !- Roughness
    0.2033016,               !- Thickness {m}
    1.729577,                !- Conductivity {W/m-K}
    2242.585,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.65,                    !- Solar Absorptance
    0.65;                    !- Visible Absorptance

  Material,
    C12 - 2 IN HW CONCRETE,  !- Name
    MediumRough,             !- Roughness
    0.050901599,             !- Thickness {m}
    1.729577,                !- Conductivity {W/m-K}
    2242.585,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.65,                    !- Solar Absorptance
    0.65;                    !- Visible Absorptance

  Material,
    C4 - 4 IN COMMON BRICK,  !- Name
    Rough,                   !- Roughness
    0.1014984,               !- Thickness {m}
    0.7264224,               !- Conductivity {W/m-K}
    1922.216,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.76,                    !- Solar Absorptance
    0.76;                    !- Visible Absorptance

  Material,
    C6 - 8 IN CLAY TILE,     !- Name
    Smooth,                  !- Roughness
    0.2033016,               !- Thickness {m}
    0.5707605,               !- Conductivity {W/m-K}
    1121.292,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.82,                    !- Solar Absorptance
    0.82;                    !- Visible Absorptance

  Material,
    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name
    Smooth,                  !- Roughness
    0.01905,                 !- Thickness {m}
    0.7264224,               !- Conductivity {W/m-K}
    1601.846,                !- Density {kg/m3}
    836.8,                   !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.92,                    !- Solar Absorptance
    0.92;                    !- Visible Absorptance

  Material,
    E2 - 1 / 2 IN SLAG OR STONE,  !- Name
    Rough,                   !- Roughness
    0.012710161,             !- Thickness {m}
    1.435549,                !- Conductivity {W/m-K}
    881.0155,                !- Density {kg/m3}
    1673.6,                  !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.55,                    !- Solar Absorptance
    0.55;                    !- Visible Absorptance

  Material,
    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Name
    Rough,                   !- Roughness
    0.0095402403,            !- Thickness {m}
    0.1902535,               !- Conductivity {W/m-K}
    1121.292,                !- Density {kg/m3}
    1673.6,                  !- Specific Heat {J/kg-K}
    0.9,                     !- Thermal Absorptance
    0.75,                    !- Solar Absorptance
    0.75;                    !- Visible Absorptance

  Material:NoMass,
    CP02 CARPET PAD,         !- Name
    VeryRough,               !- Roughness
    0.21648,                 !- Thermal Resistance {m2-K/W}
    0.9,                     !- Thermal Absorptance
    0.7,                     !- Solar Absorptance
    0.7;                     !- Visible Absorptance

  WindowMaterial:SimpleGlazingSystem,
    Theoretical Glazing,     !- Name
    3.0,                     !- U-Factor {W/m2-K}
    0.5,                     !- Solar Heat Gain Coefficient
    0.4;                     !- Visible Transmittance

  Construction,
    ExteriorWall,            !- Name
    A1 - 1 IN STUCCO,        !- Outside Layer
    C4 - 4 IN COMMON BRICK,  !- Layer 2
    B5 - 1 IN DENSE INSULATION,  !- Layer 3
    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4

  Construction,
    FLOOR SLAB 8 IN,         !- Name
    C10 - 8 IN HW CONCRETE,  !- Outside Layer
    CP02 CARPET PAD;         !- Layer 2

  Construction,
    ROOF34,                  !- Name
    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer
    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Layer 2
    B6 - 4 IN DENSE INSULATION,  !- Layer 3
    C12 - 2 IN HW CONCRETE;  !- Layer 4

  Construction,
    SimpleGlazing,           !- Name
    Theoretical Glazing;     !- Outside Layer

  OutdoorAir:Node,
    Model Outdoor Air Node;  !- Name

  AirLoopHVAC,
    VAV with Reheat,         !- Name
    ,                        !- Controller List Name
    VAV with ReheatAvailability Manager List,  !- Availability Manager List Name
    Autosize,                !- Design Supply Air Flow Rate {m3/s}
    VAV with Reheat Supply Branches,  !- Branch List Name
    ,                        !- Connector List Name
    Node 1,                  !- Supply Side Inlet Node Name
    Node 4,                  !- Demand Side Outlet Node Name
    VAV with Reheat Demand Inlet Nodes,  !- Demand Side Inlet Node Names
    VAV with Reheat Supply Outlet Nodes,  !- Supply Side Outlet Node Names
    1;                       !- Design Return Air Flow Fraction of Supply Air Flow

  NodeList,
    VAV with Reheat Supply Outlet Nodes,  !- Name
    Node 2;                  !- Node 1 Name

  NodeList,
    VAV with Reheat Demand Inlet Nodes,  !- Name
    Node 3;                  !- Node 1 Name

  Sizing:System,
    VAV with Reheat,         !- AirLoop Name
    Total,                   !- Type of Load to Size On
    Autosize,                !- Design Outdoor Air Flow Rate {m3/s}
    0.3,                     !- Central Heating Maximum System Air Flow Ratio
    7,                       !- Preheat Design Temperature {C}
    0.008,                   !- Preheat Design Humidity Ratio {kgWater/kgDryAir}
    12.8,                    !- Precool Design Temperature {C}
    0.008,                   !- Precool Design Humidity Ratio {kgWater/kgDryAir}
    12.8,                    !- Central Cooling Design Supply Air Temperature {C}
    40,                      !- Central Heating Design Supply Air Temperature {C}
    NonCoincident,           !- Type of Zone Sum to Use
    No,                      !- 100% Outdoor Air in Cooling
    No,                      !- 100% Outdoor Air in Heating
    0.0085,                  !- Central Cooling Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    0.008,                   !- Central Heating Design Supply Air Humidity Ratio {kgWater/kgDryAir}
    DesignDay,               !- Cooling Supply Air Flow Rate Method
    0,                       !- Cooling Supply Air Flow Rate {m3/s}
    0.0099676501,            !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}
    1,                       !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate
    3.9475456e-05,           !- Cooling Supply Air Flow Rate Per Unit Cooling Capacity {m3/s-W}
    DesignDay,               !- Heating Supply Air Flow Rate Method
    0,                       !- Heating Supply Air Flow Rate {m3/s}
    0.0099676501,            !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}
    1,                       !- Heating Fraction of Autosized Heating Supply Air Flow Rate
    1,                       !- Heating Fraction of Autosized Cooling Supply Air Flow Rate
    3.1588213e-05,           !- Heating Supply Air Flow Rate Per Unit Heating Capacity {m3/s-W}
    ZoneSum,                 !- System Outdoor Air Method
    1,                       !- Zone Maximum Outdoor Air Fraction {dimensionless}
    CoolingDesignCapacity,   !- Cooling Design Capacity Method
    Autosize,                !- Cooling Design Capacity {W}
    234.7,                   !- Cooling Design Capacity Per Floor Area {W/m2}
    1,                       !- Fraction of Autosized Cooling Design Capacity
    HeatingDesignCapacity,   !- Heating Design Capacity Method
    Autosize,                !- Heating Design Capacity {W}
    157,                     !- Heating Design Capacity Per Floor Area {W/m2}
    1,                       !- Fraction of Autosized Heating Design Capacity
    OnOff,                   !- Central Cooling Capacity Control Method
    Autosize;                !- Occupant Diversity

  AvailabilityManagerAssignmentList,
    VAV with ReheatAvailability Manager List,  !- Name
    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type
    VAV with Reheat Availability Manager;  !- Availability Manager 1 Name

  AvailabilityManager:Scheduled,
    VAV with Reheat Availability Manager,  !- Name
    Always On;      !- Schedule Name

  BranchList,
    VAV with Reheat Supply Branches,  !- Name
    VAV with Reheat Main Branch;  !- Branch 1 Name

  Branch,
    VAV with Reheat Main Branch,  !- Name
    ,                        !- Pressure Drop Curve Name
    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type
    VAV with Reheat OA System,  !- Component 1 Name
    Node 1,                  !- Component 1 Inlet Node Name
    Node 8,                  !- Component 1 Outlet Node Name
    CoilSystem:Cooling:DX,   !- Component 2 Object Type
    VAV Main DX Clg Coil CoilSystem,  !- Component 2 Name
    Node 8,                  !- Component 2 Inlet Node Name
    Node 11,                 !- Component 2 Outlet Node Name
    Coil:Heating:Electric,   !- Component 3 Object Type
    Elec Htg Coil,           !- Component 3 Name
    Node 11,                 !- Component 3 Inlet Node Name
    Node 9,                  !- Component 3 Outlet Node Name
    Fan:SystemModel,         !- Component 4 Object Type
    VAV with Reheat Supply Fan,  !- Component 4 Name
    Node 9,                  !- Component 4 Inlet Node Name
    Node 2;                  !- Component 4 Outlet Node Name

)IDF";

    std::string const idf_objects_3 = R"IDF(

  AirLoopHVAC:OutdoorAirSystem,
    VAV with Reheat OA System,  !- Name
    VAV with Reheat OA System Controller List,  !- Controller List Name
    VAV with Reheat OA System Equipment List;  !- Outdoor Air Equipment List Name

  AirLoopHVAC:ControllerList,
    VAV with Reheat OA System Controller List,  !- Name
    Controller:OutdoorAir,   !- Controller 1 Object Type
    VAV with Reheat OA Controller;  !- Controller 1 Name

  Controller:OutdoorAir,
    VAV with Reheat OA Controller,  !- Name
    Node 7,                  !- Relief Air Outlet Node Name
    Node 1,                  !- Return Air Node Name
    Node 8,                  !- Mixed Air Node Name
    Node 6,                  !- Actuator Node Name
    0,                       !- Minimum Outdoor Air Flow Rate {m3/s}
    Autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}
    NoEconomizer,            !- Economizer Control Type
    ModulateFlow,            !- Economizer Control Action Type
    28,                      !- Economizer Maximum Limit Dry-Bulb Temperature {C}
    64000,                   !- Economizer Maximum Limit Enthalpy {J/kg}
    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}
    ,                        !- Electronic Enthalpy Limit Curve Name
    ,                        !- Economizer Minimum Limit Dry-Bulb Temperature {C}
    NoLockout,               !- Lockout Type
    FixedMinimum,            !- Minimum Limit Type
    ,                        !- Minimum Outdoor Air Schedule Name
    ,                        !- Minimum Fraction of Outdoor Air Schedule Name
    ,                        !- Maximum Fraction of Outdoor Air Schedule Name
    ,                        !- Mechanical Ventilation Controller Name
    ,                        !- Time of Day Economizer Control Schedule Name
    No,                      !- High Humidity Control
    ,                        !- Humidistat Control Zone Name
    ,                        !- High Humidity Outdoor Air Flow Ratio
    Yes,                     !- Control High Indoor Humidity Based on Outdoor Humidity Ratio
    BypassWhenWithinEconomizerLimits;  !- Heat Recovery Bypass Control Type

  AvailabilityManagerAssignmentList,
    VAV with Reheat OA System Availability Manager List,  !- Name
    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type
    VAV with Reheat OA System Availability Manager;  !- Availability Manager 1 Name

  AvailabilityManager:Scheduled,
    VAV with Reheat OA System Availability Manager,  !- Name
    Always On;      !- Schedule Name

  OutdoorAir:NodeList,
    Node 6;                  !- Node or NodeList Name 1

  AirLoopHVAC:OutdoorAirSystem:EquipmentList,
    VAV with Reheat OA System Equipment List,  !- Name
    OutdoorAir:Mixer,        !- Component 1 Object Type
    OA Mixer;  !- Component 1 Name

  OutdoorAir:Mixer,
    OA Mixer,                !- Name
    Node 8,                  !- Mixed Air Node Name
    Node 6,                  !- Outdoor Air Stream Node Name
    Node 7,                  !- Relief Air Stream Node Name
    Node 1;                  !- Return Air Stream Node Name

  SetpointManager:MixedAir,
    Node 8 OS Default SPM,   !- Name
    Temperature,             !- Control Variable
    Node 2,                  !- Reference Setpoint Node Name
    Node 9,                  !- Fan Inlet Node Name
    Node 2,                  !- Fan Outlet Node Name
    Node 8;                  !- Setpoint Node or NodeList Name

  CoilSystem:Cooling:DX,
    VAV Main DX Clg Coil CoilSystem,  !- Name
    Always On,               !- Availability Schedule Name
    Node 8,                  !- DX Cooling Coil System Inlet Node Name
    Node 11,                 !- DX Cooling Coil System Outlet Node Name
    Node 11,                 !- DX Cooling Coil System Sensor Node Name
    Coil:Cooling:DX:TwoSpeed,!- Cooling Coil Object Type
    VAV Main DX Clg Coil;    !- Cooling Coil Name

  Coil:Cooling:DX:TwoSpeed,
    VAV Main DX Clg Coil,    !- Name
    Always On,               !- Availability Schedule Name
    Autosize,                !- High Speed Gross Rated Total Cooling Capacity {W}
    0.75,                    !- High Speed Rated Sensible Heat Ratio
    3,                       !- High Speed Gross Rated Cooling COP {W/W}
    Autosize,                !- High Speed Rated Air Flow Rate {m3/s}
    ,                        !- High Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
    ,                        !- High Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
    773.3,                   !- Unit Internal Static Air Pressure {Pa}
    Node 8,                  !- Air Inlet Node Name
    Node 11,                 !- Air Outlet Node Name
    Curve Biquadratic,       !- Total Cooling Capacity Function of Temperature Curve Name
    Curve Quadratic,         !- Total Cooling Capacity Function of Flow Fraction Curve Name
    Curve Biquadratic,       !- Energy Input Ratio Function of Temperature Curve Name
    Curve Quadratic,         !- Energy Input Ratio Function of Flow Fraction Curve Name
    Curve Quadratic,         !- Part Load Fraction Correlation Curve Name
    Autosize,                !- Low Speed Gross Rated Total Cooling Capacity {W}
    0.75,                    !- Low Speed Gross Rated Sensible Heat Ratio
    3,                       !- Low Speed Gross Rated Cooling COP {W/W}
    Autosize,                !- Low Speed Rated Air Flow Rate {m3/s}
    ,                        !- Low Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
    ,                        !- Low Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
    Curve Biquadratic,       !- Low Speed Total Cooling Capacity Function of Temperature Curve Name
    Curve Biquadratic,       !- Low Speed Energy Input Ratio Function of Temperature Curve Name
    ,                        !- Condenser Air Inlet Node Name
    AirCooled,               !- Condenser Type
    -25,                     !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}
    0,                       !- High Speed Evaporative Condenser Effectiveness {dimensionless}
    Autosize,                !- High Speed Evaporative Condenser Air Flow Rate {m3/s}
    Autosize,                !- High Speed Evaporative Condenser Pump Rated Power Consumption {W}
    0,                       !- Low Speed Evaporative Condenser Effectiveness {dimensionless}
    Autosize,                !- Low Speed Evaporative Condenser Air Flow Rate {m3/s}
    Autosize,                !- Low Speed Evaporative Condenser Pump Rated Power Consumption {W}
    ,                        !- Supply Water Storage Tank Name
    ,                        !- Condensate Collection Water Storage Tank Name
    10,                      !- Basin Heater Capacity {W/K}
    2;                       !- Basin Heater Setpoint Temperature {C}

  SetpointManager:MixedAir,
    Node 11 OS Default SPM,  !- Name
    Temperature,             !- Control Variable
    Node 2,                  !- Reference Setpoint Node Name
    Node 9,                  !- Fan Inlet Node Name
    Node 2,                  !- Fan Outlet Node Name
    Node 11;                 !- Setpoint Node or NodeList Name

  Curve:Biquadratic,
    Curve Biquadratic,       !- Name
    1.0,                     !- Coefficient1 Constant
    0.0,                     !- Coefficient2 x
    0.0,                     !- Coefficient3 x**2
    0.0,                     !- Coefficient4 y
    0.0,                     !- Coefficient5 y**2
    0.0,                     !- Coefficient6 x*y
    0,                       !- Minimum Value of x
    50,                      !- Maximum Value of x
    0,                       !- Minimum Value of y
    50;                      !- Maximum Value of y

  Curve:Quadratic,
    Curve Quadratic,         !- Name
    1.0,                     !- Coefficient1 Constant
    0.0,                     !- Coefficient2 x
    0.0,                     !- Coefficient3 x**2
    0.9,                     !- Minimum Value of x
    1.0;                     !- Maximum Value of x

  Coil:Heating:Electric,
    Elec Htg Coil,           !- Name
    Always On,               !- Availability Schedule Name
    1,                       !- Efficiency
    Autosize,                !- Nominal Capacity {W}
    Node 11,                 !- Air Inlet Node Name
    Node 9,                  !- Air Outlet Node Name
    Node 9;                  !- Temperature Setpoint Node Name

  SetpointManager:MixedAir,
    Node 9 OS Default SPM,   !- Name
    Temperature,             !- Control Variable
    Node 2,                  !- Reference Setpoint Node Name
    Node 9,                  !- Fan Inlet Node Name
    Node 2,                  !- Fan Outlet Node Name
    Node 9;                  !- Setpoint Node or NodeList Name

  Fan:SystemModel,
    VAV with Reheat Supply Fan,  !- Name
    Always On,               !- Availability Schedule Name
    Node 9,                  !- Air Inlet Node Name
    Node 2,                  !- Air Outlet Node Name
    Autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Continuous,              !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500,                     !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1,                       !- Motor In Air Stream Fraction
    Autosize,                !- Design Electric Power Consumption {W}
    PowerPerFlowPerPressure, !- Design Power Sizing Method
    840,                     !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    1.66667,                 !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.524386048,             !- Fan Total Efficiency
    VAV Fan Curve,           !- Electric Power Function of Flow Fraction Curve Name
    ,                        !- Night Ventilation Mode Pressure Rise {Pa}
    ,                        !- Night Ventilation Mode Flow Fraction
    ,                        !- Motor Loss Zone Name
    0,                       !- Motor Loss Radiative Fraction
    General,                 !- End-Use Subcategory
    1;                       !- Number of Speeds

  Curve:Quartic,
    VAV Fan Curve,           !- Name
    0.040759894,             !- Coefficient1 Constant
    0.08804497,              !- Coefficient2 x
    -0.07292612,             !- Coefficient3 x**2
    0.943739823,             !- Coefficient4 x**3
    0,                       !- Coefficient5 x**4
    0,                       !- Minimum Value of x
    1,                       !- Maximum Value of x
    0,                       !- Minimum Curve Output
    1,                       !- Maximum Curve Output
    Dimensionless,           !- Input Unit Type for X
    Dimensionless;           !- Output Unit Type

  SetpointManager:Warmest,
    VAV with Reheat SetPoint Manager Warmest,  !- Name
    Temperature,             !- Control Variable
    VAV with Reheat,         !- HVAC Air Loop Name
    12.2,                    !- Minimum Setpoint Temperature {C}
    15.6,                    !- Maximum Setpoint Temperature {C}
    MaximumTemperature,      !- Strategy
    Node 2;                  !- Setpoint Node or NodeList Name

  AirLoopHVAC:SupplyPath,
    VAV with Reheat Node 3 Supply Path,  !- Name
    Node 3,                  !- Supply Air Path Inlet Node Name
    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type
    Air Loop HVAC Zone Splitter 1;  !- Component 1 Name

  AirLoopHVAC:ZoneSplitter,
    Air Loop HVAC Zone Splitter 1,  !- Name
    Node 3,                  !- Inlet Node Name
    Node 10;                 !- Outlet 1 Node Name

  AirLoopHVAC:ReturnPath,
    VAV with Reheat Return Path,  !- Name
    Node 4,                  !- Return Air Path Outlet Node Name
    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type
    Air Loop HVAC Zone Mixer 1;  !- Component 1 Name

  AirLoopHVAC:ZoneMixer,
    Air Loop HVAC Zone Mixer 1,  !- Name
    Node 4,                  !- Outlet Node Name
    Node 29;                 !- Inlet 1 Node Name

  Site:Location,
    Atlanta Hartsfield Intl Ap,  !- Name
    33.63,                   !- Latitude {deg}
    -84.43,                  !- Longitude {deg}
    -5,                      !- Time Zone {hr}
    308;                     !- Elevation {m}

  SizingPeriod:DesignDay,
    Atlanta Hartsfield Intl Ap Ann Clg .4% Condns DB=>MWB,  !- Name
    7,                       !- Month
    21,                      !- Day of Month
    SummerDesignDay,         !- Day Type
    34.4,                    !- Maximum Dry-Bulb Temperature {C}
    9.5,                     !- Daily Dry-Bulb Temperature Range {deltaC}
    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type
    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name
    Wetbulb,                 !- Humidity Condition Type
    23.5,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}
    ,                        !- Humidity Condition Day Schedule Name
    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}
    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}
    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}
    97679,                   !- Barometric Pressure {Pa}
    4,                       !- Wind Speed {m/s}
    300,                     !- Wind Direction {deg}
    No,                      !- Rain Indicator
    No,                      !- Snow Indicator
    No,                      !- Daylight Saving Time Indicator
    ASHRAETau,               !- Solar Model Indicator
    ,                        !- Beam Solar Day Schedule Name
    ,                        !- Diffuse Solar Day Schedule Name
    0.556,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}
    1.779,                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}
    ;                       !- Sky Clearness

  SizingPeriod:DesignDay,
    Atlanta Hartsfield Intl Ap Ann Htg 99.6% Condns DB,  !- Name
    1,                       !- Month
    21,                      !- Day of Month
    WinterDesignDay,         !- Day Type
    -6.3,                    !- Maximum Dry-Bulb Temperature {C}
    0,                       !- Daily Dry-Bulb Temperature Range {deltaC}
    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type
    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name
    Wetbulb,                 !- Humidity Condition Type
    -6.3,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}
    ,                        !- Humidity Condition Day Schedule Name
    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}
    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}
    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}
    97679,                   !- Barometric Pressure {Pa}
    5.3,                     !- Wind Speed {m/s}
    320,                     !- Wind Direction {deg}
    No,                      !- Rain Indicator
    No,                      !- Snow Indicator
    No,                      !- Daylight Saving Time Indicator
    ASHRAEClearSky,          !- Solar Model Indicator
    ,                        !- Beam Solar Day Schedule Name
    ,                        !- Diffuse Solar Day Schedule Name
    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}
    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}
    0;                       !- Sky Clearness

  OutputControl:Table:Style,
    HTML;                    !- Column Separator

  Output:VariableDictionary,IDF,Unsorted;

  Output:SQLite,
    SimpleAndTabular;        !- Option Type

  Output:Table:SummaryReports,
    AllSummary,              !- Report 1 Name
    AllSummaryAndSizingPeriod;  !- Report 2 Name
)IDF";

    std::string const idf_objects = idf_objects_1 + idf_objects_2 + idf_objects_3;
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    ManageSimulation(*state); // run the design days
    auto &finalSysSizing = state->dataSize->FinalSysSizing(1);
    EXPECT_ENUM_EQ(finalSysSizing.coolingPeakLoad, DataSizing::PeakLoad::TotalCooling);

    // get the 'Peak Sensible Load with Sizing Factor'
    std::string query("SELECT Value From TabularDataWithStrings"
                      "  WHERE TableName = 'Cooling Peak Conditions'"
                      "  AND ReportName = 'AirLoop Component Load Summary'"
                      "  AND ColumnName = 'Value'"
                      "  AND RowName = 'Peak Sensible Load with Sizing Factor'");
    // check the value from result records
    Real64 return_val = execAndReturnFirstDouble(query);
    EXPECT_NEAR(5080.22, return_val, 0.011);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_WarnMonthly)
{
    // #9621 - Only warn if a bad variable is defined in a Monthly table user requested, not on the AllSummaryAndMonthly ones
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "  Space Gains Annual Report, !- Name",
        "  2, !-  Digits After Decimal",
        "  Exterior Lights Electricity Energy, !- Variable or Meter 1 Name",
        "  SumOrAverage, !- Aggregation Type for Variable or Meter 1",
        "  NON EXISTANT VARIABLE, !- Variable or Meter 2 Name", // That's not how you spell EXISTENT
        "  Maximum; !- Aggregation Type for Variable or Meter 2",

        "Output:Table:SummaryReports,",
        "  AllSummaryAndMonthly;              !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // Trigger the extra warning
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = false;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    GetInputOutputTableSummaryReports(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, numNamedMonthly + 1);

    InitializeTabularMonthly(*state);

    std::string const expected_error = delimited_string({
        "   ** Warning ** Processing Monthly Tabular Reports: Variable names not valid for this simulation",
        "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual variables.",
    });
    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_WarnMonthly_AlwaysIfWeatherRunRequested)
{
    // #9621 - Only warn if a bad variable is defined in a Monthly table user requested, not on the AllSummaryAndMonthly ones
    // If I request a Weater File run Period I expect a warning
    // Regardless of the fact that the sizing run happens first
    std::string const idf_objects = delimited_string({
        "SimulationControl,",
        "  No,                                 !- Do Zone Sizing Calculation",
        "  No,                                 !- Do System Sizing Calculation",
        "  No,                                 !- Do Plant Sizing Calculation",
        "  Yes,                                !- Run Simulation for Sizing Periods",
        "  Yes;                                !- Run Simulation for Weather File Run Periods",

        "Output:Table:Monthly,",
        "  Space Gains Annual Report,          !- Name",
        "  2,                                  !-  Digits After Decimal",
        "  Exterior Lights Electricity Energy, !- Variable or Meter 1 Name",
        "  SumOrAverage,                       !- Aggregation Type for Variable or Meter 1",
        "  NON EXISTANT VARIABLE,              !- Variable or Meter 2 Name",
        "  Maximum;                            !- Aggregation Type for Variable or Meter 2",

        "Output:Table:SummaryReports,",
        "  AllSummaryAndMonthly;               !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::GetProjectData(*state);
    EXPECT_TRUE(state->dataGlobal->DoDesDaySim);
    EXPECT_TRUE(state->dataGlobal->DoWeathSim);

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    // In a regular simulation with the above SimulationControl, when InitializeTabularMonthly is called it's DesignDay
    state->dataGlobal->KindOfSim = Constant::KindOfSim::DesignDay;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = false;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    GetInputOutputTableSummaryReports(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, numNamedMonthly + 1);

    InitializeTabularMonthly(*state);

    std::string const expected_error = delimited_string({
        "   ** Warning ** Processing Monthly Tabular Reports: Variable names not valid for this simulation",
        "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual variables.",
    });
    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_WarnMonthlyDisplayExtraWarnings)
{
    // #9621 - Only warn if a bad variable is defined in a Monthly table user requested, not on the AllSummaryAndMonthly ones
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "  Space Gains Annual Report, !- Name",
        "  2, !-  Digits After Decimal",
        "  NON EXISTANT VARIABLE, !- Variable or Meter 1 Name",
        "  SumOrAverage, !- Aggregation Type for Variable or Meter 1",
        "  NON EXISTANT VARIABLE BIS, !- Variable or Meter 2 Name",
        "  Maximum; !- Aggregation Type for Variable or Meter 2",

        "Output:Table:SummaryReports,",
        "  AllSummaryAndMonthly;              !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // Trigger the extra warning
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = true;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    GetInputOutputTableSummaryReports(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, numNamedMonthly + 1);

    InitializeTabularMonthly(*state);

    std::string const expected_error = delimited_string({
        "   ** Warning ** Processing Monthly Tabular Reports: Variable names not valid for this simulation",
        "   **   ~~~   ** ..Variables not valid for this simulation will have \"[Invalid/Undefined]\" in the Units Column of the Table Report.",
        "   ** Warning ** In Output:Table:Monthly 'SPACE GAINS ANNUAL REPORT' invalid Variable or Meter Name 'NON EXISTANT VARIABLE'",
        "   ** Warning ** In Output:Table:Monthly 'SPACE GAINS ANNUAL REPORT' invalid Variable or Meter Name 'NON EXISTANT VARIABLE BIS'",
    });
    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_WarnMonthlyBlankVariable)
{
    // #6919 - Warn instead of fatal on blank monthly table
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "  Space Gains Annual Report, !- Name",
        "  2, !-  Digits After Decimal",
        "  , !- Variable or Meter 1 Name",
        "  SumOrAverage; !- Aggregation Type for Variable or Meter 1",

        "Output:Table:SummaryReports,",
        "  AllSummaryAndMonthly;              !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // Trigger the extra warning
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = true;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    GetInputOutputTableSummaryReports(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, numNamedMonthly + 1);

    InitializeTabularMonthly(*state);

    std::string const expected_error = delimited_string(
        {"   ** Warning ** Output:Table:Monthly: Blank column specified in 'SPACE GAINS ANNUAL REPORT', need to provide a variable or meter name "});
    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_NoWarnMonthlIfNoWeatherFileRun)
{
    // #9621 - Only warn if a bad variable is defined in a Monthly table user requested, not on the AllSummaryAndMonthly ones
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "  Space Gains Annual Report, !- Name",
        "  2, !-  Digits After Decimal",
        "  NON EXISTANT VARIABLE, !- Variable or Meter 1 Name",
        "  SumOrAverage, !- Aggregation Type for Variable or Meter 1",
        "  NON EXISTANT VARIABLE BIS, !- Variable or Meter 2 Name",
        "  Maximum; !- Aggregation Type for Variable or Meter 2",

        "Output:Table:SummaryReports,",
        "  AllSummaryAndMonthly;              !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = false; // <- here
    state->dataGlobal->KindOfSim = Constant::KindOfSim::DesignDay;
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = true;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 1);
    GetInputOutputTableSummaryReports(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, numNamedMonthly);

    InitializeTabularMonthly(*state);

    std::string const expected_error = delimited_string({
        "   ** Warning ** Output:Table:Monthly requested with SimulationControl Run Simulation for Weather File Run Periods set to No so "
        "Output:Table:Monthly will not be generated",
    });
    compare_err_stream(expected_error);
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_DontWarnMonthlyIfOnlyNamedReports)
{
    // #9621 - Only warn if a bad variable is defined in a Monthly table user requested, not on the AllSummaryAndMonthly ones
    std::string const idf_objects = delimited_string({
        "Output:Table:SummaryReports,",
        "  AllSummaryAndMonthly;              !- Report 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather; // Trigger the extra warning
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataGlobal->DisplayExtraWarnings = true;

    GetInputTabularMonthly(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, 0);
    GetInputOutputTableSummaryReports(*state);
    EXPECT_EQ(state->dataOutRptTab->MonthlyInputCount, numNamedMonthly);

    InitializeTabularMonthly(*state);

    compare_err_stream("");
}

TEST_F(SQLiteFixture, OutputReportTabular_DistrictHeating)
{
    // Test for #10190 - District Heating Steam is empty
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularBEPS = true;
    state->dataOutRptTab->displayDemandEndUse = true;
    state->dataOutRptTab->displayLEEDSummary = true;

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoKWH;

    // Needed to avoid crash (from ElectricPowerServiceManager.hh)
    createFacilityElectricPowerServiceObject(*state);

    SetPredefinedTables(*state);

    Real64 DistrictHeatingWater = 4e8;
    SetupOutputVariable(*state,
                        "Exterior Equipment DistrictHeatingWater Energy",
                        Constant::Units::J,
                        DistrictHeatingWater,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "DHWaterExtEq",
                        Constant::eResource::DistrictHeatingWater,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorEquipment,
                        "General");

    Real64 DistrictHeatingSteam = 5e8;
    SetupOutputVariable(*state,
                        "Exterior Equipment DistrictHeatingSteam Energy",
                        Constant::Units::J,
                        DistrictHeatingSteam,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "DHSteamExtEq",
                        Constant::eResource::DistrictHeatingSteam,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorEquipment,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->MinutesInTimeStep = state->dataGlobal->TimeStepZone * 60;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 3600.0;
    state->dataOutRptTab->displayTabularBEPS = true;
    // OutputProcessor::TimeValue.allocate(2);

    auto timeStep = 1.0;

    SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, timeStep);
    SetupTimePointers(*state, OutputProcessor::TimeStepType::System, timeStep);

    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep = 60;
    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep = 60;

    GetInputOutputTableSummaryReports(*state);

    state->dataEnvrn->Month = 12;

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);

    auto &ort = state->dataOutRptTab;
    constexpr int dhWaterIndex = 4;
    constexpr int dhSteamIndex = 5;
    EXPECT_EQ("DistrictHeatingWater", ort->resourceTypeNames(dhWaterIndex));
    EXPECT_EQ("DistrictHeatingSteam", ort->resourceTypeNames(dhSteamIndex));

    EXPECT_NEAR(DistrictHeatingWater, state->dataOutRptTab->gatherTotalsBEPS(dhWaterIndex), 1.);
    EXPECT_NEAR(
        DistrictHeatingWater, state->dataOutRptTab->gatherEndUseBEPS(dhWaterIndex, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1), 1.);
    // General
    EXPECT_NEAR(DistrictHeatingWater,
                state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1, dhWaterIndex),
                1.);

    EXPECT_NEAR(DistrictHeatingSteam, state->dataOutRptTab->gatherTotalsBEPS(dhSteamIndex), 1.);
    EXPECT_NEAR(
        DistrictHeatingSteam, state->dataOutRptTab->gatherEndUseBEPS(dhSteamIndex, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1), 1.);
    // General
    EXPECT_NEAR(DistrictHeatingSteam,
                state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1, dhSteamIndex),
                1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(DistrictHeatingWater * 2, state->dataOutRptTab->gatherTotalsBEPS(dhWaterIndex), 1.);
    EXPECT_NEAR(DistrictHeatingWater * 2,
                state->dataOutRptTab->gatherEndUseBEPS(dhWaterIndex, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1),
                1.);
    // General
    EXPECT_NEAR(DistrictHeatingWater * 2,
                state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1, dhWaterIndex),
                1.);

    EXPECT_NEAR(DistrictHeatingSteam * 2, state->dataOutRptTab->gatherTotalsBEPS(dhSteamIndex), 1.);
    EXPECT_NEAR(DistrictHeatingSteam * 2,
                state->dataOutRptTab->gatherEndUseBEPS(dhSteamIndex, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1),
                1.);
    // General
    EXPECT_NEAR(DistrictHeatingSteam * 2,
                state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorEquipment) + 1, dhSteamIndex),
                1.);

    OutputReportTabular::WriteBEPSTable(*state);
    OutputReportTabular::WriteDemandEndUseSummary(*state);

    // We test for Heating and Total, since they should be the same
    std::vector<std::string> testReportNames = {"AnnualBuildingUtilityPerformanceSummary", "DemandEndUseComponentsSummary"};
    std::vector<std::string> endUseSubCategoryNames = {"General"};

    // Query End Use
    {
        std::string query(R"sql(
        SELECT Value From TabularDataWithStrings
           WHERE TableName = 'End Uses'
             AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'
             AND ColumnName = 'District Heating Water'
             AND RowName = 'Exterior Equipment')sql");
        auto const result = queryResult(query, "TabularDataWithStrings");
        Real64 const return_val1 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        EXPECT_NEAR(DistrictHeatingWater * 2 / 3.6e6, return_val1, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'District Heating Steam'"
                          "  AND RowName = 'Exterior Equipment'");
        auto const result = queryResult(query, "TabularDataWithStrings");
        Real64 const return_val = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        EXPECT_NEAR(DistrictHeatingSteam * 2 / 3.6e6, return_val, 0.01) << "Failed for query: " << query;
    }

    // Query End Use with Subcategory
    {
        std::string const query("SELECT Value From TabularDataWithStrings"
                                "  WHERE TableName = 'End Uses By Subcategory'"
                                "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                                "  AND ColumnName = 'District Heating Water'"
                                "  AND RowName = 'Exterior Equipment:General'");
        Real64 const return_val = execAndReturnFirstDouble(query);
        EXPECT_NEAR(DistrictHeatingWater * 2 / 3.6e6, return_val, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'District Heating Steam'"
                          "  AND RowName = 'Exterior Equipment:General'");
        Real64 return_val = execAndReturnFirstDouble(query);
        EXPECT_NEAR(DistrictHeatingSteam * 2 / 3.6e6, return_val, 0.01) << "Failed for query: " << query;
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabular_Test_SetupUnitConversion_Fix)
{
    // Unit test for PR 10261 that fixes Issue 10260
    std::string unitString;
    std::string curUnits;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPound;

    // Mimic the unit conversion for LEED Table EAp2-17b
    // Energy Use Intensity - Natural Gas
    unitString = "Natural Gas [MJ/m2]";
    int unitConvNum = unitsFromHeading(*state, unitString);
    EXPECT_EQ("Natural Gas [kBtu/ft2]", unitString);
    EXPECT_EQ(93, unitConvNum);
    Real64 unitConvFactor = state->dataOutRptTab->UnitConv(unitConvNum).mult;
    EXPECT_NEAR(unitConvFactor, 0.94708628903179 / 10.764961, 1e-5);

    // Mimic the unit conversion for LEED Table EAp2-17b
    // Energy Use Intensity - Natural Gas
    // Another form
    unitString = "Natural Gas {MJ/m2}";
    unitConvNum = unitsFromHeading(*state, unitString);
    EXPECT_EQ(93, unitConvNum);
    EXPECT_EQ("Natural Gas {kBtu/ft2}", unitString);
    unitConvFactor = state->dataOutRptTab->UnitConv(unitConvNum).mult;
    EXPECT_NEAR(unitConvFactor, 0.94708628903179 / 10.764961, 1e-5);

    // Test affected Unit Conversion Entry #94 as well
    unitString = "Additional [MJ/m2]";
    unitConvNum = unitsFromHeading(*state, unitString);
    EXPECT_EQ(94, unitConvNum);
    EXPECT_EQ("Additional [kBtu/ft2]", unitString);
    unitConvFactor = state->dataOutRptTab->UnitConv(unitConvNum).mult;
    EXPECT_NEAR(unitConvFactor, 0.94708628903179 / 10.764961, 1e-5);

    // Should not affect entries after #93-94
    unitString = "";
    unitConvNum = unitsFromHeading(*state, unitString);
    EXPECT_EQ(97, unitConvNum);
    EXPECT_EQ("", unitString);
    unitConvFactor = state->dataOutRptTab->UnitConv(unitConvNum).mult;
    EXPECT_NEAR(unitConvFactor, 1.0, 1e-5);
}

TEST_F(SQLiteFixture, WriteSourceEnergyEndUseSummaryperArea_IPExceptElec)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displaySourceEnergyEndUseSummary = true;

    // DetermineBuildingFloorArea

    state->dataEnvrn->Latitude = 12.3;
    state->dataEnvrn->Longitude = 45.6;

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    // walls
    state->dataSurface->Surface(1).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(1).Azimuth = 0.;
    state->dataSurface->Surface(1).GrossArea = 200.; // 20 x 10
    state->dataSurface->Surface(1).Tilt = 90.;
    state->dataSurface->Surface(1).Zone = 1;

    state->dataSurface->Surface(2).Class = SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(2).Azimuth = 90.;
    state->dataSurface->Surface(2).GrossArea = 300.; // 30 x 10
    state->dataSurface->Surface(2).Tilt = 90.;
    state->dataSurface->Surface(2).Zone = 2;

    // windows
    state->dataSurface->Surface(3).Class = SurfaceClass::Window;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(3).Azimuth = 0.;
    state->dataSurface->Surface(3).GrossArea = 40.;
    state->dataSurface->Surface(3).Height = 5;
    state->dataSurface->Surface(3).Width = 8;
    state->dataSurface->Surface(3).Tilt = 90.;
    state->dataSurface->Surface(3).Zone = 1;

    state->dataSurface->Surface(4).Class = SurfaceClass::Window;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).ExtBoundCond = ExternalEnvironment;
    state->dataSurface->Surface(4).Azimuth = 90.;
    state->dataSurface->Surface(4).GrossArea = 60.;
    state->dataSurface->Surface(4).Height = 6;
    state->dataSurface->Surface(4).Width = 10;
    state->dataSurface->Surface(4).Tilt = 90.;
    state->dataSurface->Surface(4).Zone = 2;

    // Loads
    state->dataHeatBal->TotLights = 3;
    state->dataHeatBal->Lights.allocate(state->dataHeatBal->TotLights);

    state->dataHeatBal->TotPeople = 3;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);

    state->dataHeatBal->TotElecEquip = 3;
    state->dataHeatBal->ZoneElectric.allocate(state->dataHeatBal->TotElecEquip);

    state->dataHeatBal->Lights(1).ZonePtr = 1;
    state->dataHeatBal->Lights(1).DesignLevel = 1000.0;
    state->dataHeatBal->Lights(2).ZonePtr = 2;
    state->dataHeatBal->Lights(2).DesignLevel = 100.0;
    state->dataHeatBal->Lights(3).ZonePtr = 3;
    state->dataHeatBal->Lights(3).DesignLevel = 10.0;

    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 10.0;
    state->dataHeatBal->People(2).ZonePtr = 2;
    state->dataHeatBal->People(2).NumberOfPeople = 5.0;
    state->dataHeatBal->People(3).ZonePtr = 3;
    state->dataHeatBal->People(3).NumberOfPeople = 1.0;

    state->dataHeatBal->ZoneElectric(1).ZonePtr = 1;
    state->dataHeatBal->ZoneElectric(1).DesignLevel = 500.0;
    state->dataHeatBal->ZoneElectric(2).ZonePtr = 2;
    state->dataHeatBal->ZoneElectric(2).DesignLevel = 50.0;
    state->dataHeatBal->ZoneElectric(3).ZonePtr = 3;
    state->dataHeatBal->ZoneElectric(3).DesignLevel = 5.0;

    // zone
    state->dataGlobal->NumOfZones = 3;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Zone(1).Name = "PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(1).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(1).Multiplier = 1.;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.;
    state->dataHeatBal->Zone(1).FloorArea = 1000.;
    state->dataHeatBal->Zone(1).Volume = 2000.;
    state->dataHeatBal->Zone(1).ExtGrossWallArea = 800.;
    state->dataHeatBal->Zone(1).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(1).ExtWindowArea = state->dataSurface->Surface(3).GrossArea + state->dataSurface->Surface(4).GrossArea;

    state->dataHeatBal->Zone(2).Name = "PartofTot Unconditioned Zone";
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 0; // Unconditioned
    state->dataHeatBal->Zone(2).isPartOfTotalArea = true;
    state->dataHeatBal->Zone(2).Multiplier = 1.;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.;
    state->dataHeatBal->Zone(2).FloorArea = 100.;
    state->dataHeatBal->Zone(2).Volume = 200.;
    state->dataHeatBal->Zone(2).ExtGrossWallArea = 80.;
    state->dataHeatBal->Zone(2).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(2).ExtWindowArea = 0.0;

    state->dataHeatBal->Zone(3).Name = "NOT PartofTot Conditioned Zone";
    state->dataHeatBal->Zone(3).SystemZoneNodeNumber = 1; // Conditioned
    state->dataHeatBal->Zone(3).isPartOfTotalArea = false;
    state->dataHeatBal->Zone(3).Multiplier = 1.;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.;
    state->dataHeatBal->Zone(3).FloorArea = 10.;
    state->dataHeatBal->Zone(3).Volume = 20.;
    state->dataHeatBal->Zone(3).ExtGrossWallArea = 8.;
    state->dataHeatBal->Zone(3).ExteriorTotalGroundSurfArea = 0;
    state->dataHeatBal->Zone(3).ExtWindowArea = 0.0;

    // 2023-10-13: Need a set up unit conversion first:
    OutputReportTabular::SetupUnitConversions(*state);

    // Gross areas
    Real64 expectedBuildingGrossFloorArea = (state->dataHeatBal->Zone(1).FloorArea + state->dataHeatBal->Zone(2).FloorArea);
    // Conditioned areas
    Real64 expectedBuildingConditionedFloorArea = state->dataHeatBal->Zone(1).FloorArea;

    // Assume heating electricity usage with a value of 3.6e6 * 1e4 J =10000 kWh that comes for a single end use
    // state->dataGlobalConst->iEndUse.at(Constant::EndUse::Heating)=1
    state->dataOutRptTab->gatherEndUseBySourceBEPS(1, static_cast<int>(Constant::EndUse::Heating) + 1) = 3.6e10; // J
    state->dataOutRptTab->gatherTotalsBySourceBEPS(1) = 3.6e10;                                                  // J
    Real64 eleckWh = 1e4;                                                                                        // kWh

    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::InchPoundExceptElectricity;

    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPoundExceptElectricity;

    // Call Source Energy End Use Summary writing function
    OutputReportTabular::WriteSourceEnergyEndUseSummary(*state);

    // Check that DetermineBuildingFloorArea is converted correctly (called from WriteSourceEnergyEndUseSummary)
    EXPECT_EQ(expectedBuildingGrossFloorArea, state->dataOutRptTab->buildingGrossFloorArea);
    EXPECT_EQ(expectedBuildingConditionedFloorArea, state->dataOutRptTab->buildingConditionedFloorArea);

    // Now test the reporting text:
    // We consistently test in the same report (three different tables) and at the same column for fuel = Elec
    const std::string reportName = "SourceEnergyEndUseComponentsSummary";
    const std::string columnName = "Source Electricity";

    // Test for Heating and Total, and they should be the same for this case
    std::vector<std::string> testRowNames = {"Heating", "Total Source Energy End Use Components"};

    // TableName, value
    std::vector<std::tuple<std::string, Real64>> results({
        {"Source Energy End Use Components Summary", eleckWh},
        {"Source Energy End Use Components Per Conditioned Floor Area", 10000.0 / (expectedBuildingConditionedFloorArea / (0.3048 * 0.3048))},
        {"Source Energy End Use Components Per Total Floor Area", 10000.0 / (expectedBuildingGrossFloorArea / (0.3048 * 0.3048))},
    });

    // Test that the reported values are successfully matching kWh/ft2 values (kWh Electricity with InchPound others)
    for (auto &v : results) {

        std::string tableName = std::get<0>(v);
        Real64 expectedValue = std::get<1>(v);

        for (auto &rowName : testRowNames) {
            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE ReportName = '" +
                              reportName +
                              "'"
                              "  AND TableName = '" +
                              tableName +
                              "'"
                              "  AND RowName = '" +
                              rowName + "'" + "  AND ColumnName = '" + columnName + "'");

            Real64 return_val = execAndReturnFirstDouble(query);

            // Add informative message if failed
            EXPECT_NEAR(expectedValue, return_val, 0.01) << "Failed for TableName=" << tableName << "; RowName=" << rowName;
        }
    }
}

TEST_F(SQLiteFixture, ORT_EndUseBySubcategorySQL_IPUnitExceptElec)
{
    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    state->dataOutRptTab->displayTabularBEPS = true;
    state->dataOutRptTab->displayDemandEndUse = true;
    state->dataOutRptTab->displayLEEDSummary = true;

    state->dataOutRptTab->WriteTabularFiles = true;

    SetupUnitConversions(*state);
    state->dataOutRptTab->unitsStyle = OutputReportTabular::UnitsStyle::JtoKWH;
    // state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;
    state->dataOutRptTab->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPoundExceptElectricity;
    Real64 enerConv = getSpecificUnitDivider(*state, "GJ", "kBtu"); // 948.45
    EXPECT_NEAR(1.0 / enerConv, 948.0, 0.5);

    // Needed to avoid crash (from ElectricPowerServiceManager.hh)
    createFacilityElectricPowerServiceObject(*state);

    SetPredefinedTables(*state);

    Real64 extLitUse = 1e8;
    Real64 CoalHeating = 2e8;
    Real64 GasolineHeating = 3e8;
    Real64 PropaneHeating = 4e8;

    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite1",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite2",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "AnotherEndUseSubCat");
    SetupOutputVariable(*state,
                        "Exterior Lights Electricity Energy",
                        Constant::Units::J,
                        extLitUse,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite3",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Coal Energy",
                        Constant::Units::J,
                        CoalHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite4",
                        Constant::eResource::Coal,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Gasoline Energy",
                        Constant::Units::J,
                        GasolineHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite5",
                        Constant::eResource::Gasoline,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    SetupOutputVariable(*state,
                        "Heating Propane Energy",
                        Constant::Units::J,
                        PropaneHeating,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Lite6",
                        Constant::eResource::Propane,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::Heating,
                        "General");
    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 3600.0;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataOutRptTab->displayTabularBEPS = true;
    // OutputProcessor::TimeValue.allocate(2);

    auto timeStep = 1.0;

    SetupTimePointers(*state, OutputProcessor::TimeStepType::Zone, timeStep);
    SetupTimePointers(*state, OutputProcessor::TimeStepType::System, timeStep);

    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::Zone].TimeStep = 60;
    *state->dataOutputProcessor->TimeValue[(int)OutputProcessor::TimeStepType::System].TimeStep = 60;

    GetInputOutputTableSummaryReports(*state);

    state->dataEnvrn->Month = 12;

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 3, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 2, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 1, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 6, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 4, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 2, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    UpdateMeterReporting(*state);
    UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    GatherBEPSResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    GatherPeakDemandForTimestep(*state, OutputProcessor::TimeStepType::Zone);
    EXPECT_NEAR(extLitUse * 9, state->dataOutRptTab->gatherEndUseBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1), 1.);
    // General
    EXPECT_NEAR(extLitUse * 6, state->dataOutRptTab->gatherEndUseSubBEPS(1, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);
    // AnotherEndUseSubCat
    EXPECT_NEAR(extLitUse * 3, state->dataOutRptTab->gatherEndUseSubBEPS(2, static_cast<int>(Constant::EndUse::ExteriorLights) + 1, 1), 1.);

    OutputReportTabular::WriteBEPSTable(*state);
    OutputReportTabular::WriteDemandEndUseSummary(*state);

    // We test for Heating and Total, since they should be the same
    std::vector<std::string> testReportNames = {"AnnualBuildingUtilityPerformanceSummary", "DemandEndUseComponentsSummary"};
    std::vector<std::string> endUseSubCategoryNames = {"General", "AnotherEndUseSubCat"};

    std::string endUseName = "Exterior Lighting";
    std::string endUseSubCategoryName = "AnotherEndUseSubCat";
    std::string rowName = endUseName + ":" + endUseSubCategoryName;
    std::string columnName = "Electricity";

    for (auto &endUseSubCategoryName : endUseSubCategoryNames) {
        for (auto &reportName : testReportNames) {

            std::string query("SELECT Value From TabularDataWithStrings"
                              "  WHERE TableName = 'End Uses By Subcategory'"
                              "  AND ColumnName = 'Electricity'"
                              "  AND ReportName = '" +
                              reportName +
                              "'"
                              "  AND RowName = '" +
                              endUseName + ":" + endUseSubCategoryName + "'"); // Now Like 'Exterior Lighting:General'

            auto result = queryResult(query, "TabularDataWithStrings");

            ASSERT_EQ(1ul, result.size()) << "Query crashed for reportName=" << reportName;
        }
    }

    for (auto &reportName : testReportNames) {

        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ColumnName = 'Electricity'"
                          "  AND ReportName = '" +
                          reportName +
                          "'"
                          "  AND RowName = '" +
                          endUseName + "'");

        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(1ul, result.size()) << "Query crashed for reportName=" << reportName;
    }

    // Specifically get the electricity usage for End Use = Exterior Lighting, and End Use Subcat = AnotherEndUseSubCat,
    // and make sure it's the right number that's returned
    std::string query("SELECT Value From TabularDataWithStrings"
                      "  WHERE TableName = 'End Uses By Subcategory'"
                      "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                      "  AND ColumnName = 'Electricity'"
                      "  AND RowName = 'Exterior Lighting:AnotherEndUseSubCat'");
    Real64 return_val = execAndReturnFirstDouble(query);

    // EXPECT_NEAR(extLitUse * 3 / 3.6e6, return_val, 0.01) << "Failed for query: " << query;
    Real64 expected_value = extLitUse * 3.0 / 3.6e6; // 1.0e9 / enerConv;
    EXPECT_NEAR(expected_value, return_val, 0.01) << "Failed for query: " << query;

    // Get all Interior Lighting End Uses (all subcats) for Electricity
    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Electricity'"
                          "  AND RowName LIKE 'Exterior Lighting:%'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(2u, result.size()) << "Failed for query: " << query;
    }

    // Get all subcat usage for all fuels (13)
    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses By Subcategory'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND RowName = 'Exterior Lighting:AnotherEndUseSubCat'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }

    // Specifically get the each fuel (Coal, Gasoline, and Propane) usage for End Use = Heating,
    // and make sure it's the right number that's returned

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Coal'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val1 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        // EXPECT_NEAR(CoalHeating * 3 / 3.6e6, return_val1, 0.01) << "Failed for query: " << query;
        Real64 expected_coalHt = CoalHeating * 3 / 1.0e9 / enerConv;
        EXPECT_NEAR(expected_coalHt, return_val1, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Gasoline'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val2 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        // EXPECT_NEAR(GasolineHeating * 3 / 3.6e6, return_val2, 0.01) << "Failed for query: " << query;
        EXPECT_NEAR(GasolineHeating * 3 / 1.0e9 / enerConv, return_val2, 0.01) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND ColumnName = 'Propane'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");
        Real64 return_val3 = execAndReturnFirstDouble(query);

        ASSERT_EQ(1u, result.size()) << "Failed for query: " << query;
        // EXPECT_NEAR(PropaneHeating * 3 / 3.6e6, return_val3, 0.01) << "Failed for query: " << query;
        EXPECT_NEAR(PropaneHeating * 3 / 1.0e9 / enerConv, return_val3, 0.01) << "Failed for query: " << query;
    }

    // Check the heating category has the result size of 13 (including all disaggregated additional fuels) in both reports)

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }

    {
        std::string query("SELECT Value From TabularDataWithStrings"
                          "  WHERE TableName = 'End Uses'"
                          "  AND ReportName = 'DemandEndUseComponentsSummary'"
                          "  AND RowName = 'Heating'");
        auto result = queryResult(query, "TabularDataWithStrings");

        ASSERT_EQ(14u, result.size()) << "Failed for query: " << query;
    }
}

TEST_F(EnergyPlusFixture, OutputReportTabularMonthly_HandleMultipleDuringHoursShown)
{
    // Test for #10899
    std::string const idf_objects = delimited_string({
        "Output:Table:Monthly,",
        "  Test,                               !- Name",
        "  3,                                  !- Digits After Decimal",
        "  ConditionA,                         !- Variable or Meter 1 Name",
        "  HoursNonZero,                       !- Aggregation Type for Variable or Meter 1",
        "  VariableToBeSummedDuringHoursShown, !- Variable or Meter 2 Name",
        "  SumOrAverageDuringHoursShown,       !- Aggregation Type for Variable or Meter 2",
        "  ConditionA_Inverse,                 !- Variable or Meter 3 Name",
        "  HoursNonZero,                       !- Aggregation Type for Variable or Meter 3",
        "  VariableToBeSummedDuringHoursShown, !- Variable or Meter 4 Name",
        "  SumOrAverageDuringHoursShown,       !- Aggregation Type for Variable or Meter 4",
        "  VariableToBeSummedDuringHoursShown, !- Variable or Meter 5 Name",
        "  SumOrAverage;                       !- Aggregation Type for Variable or Meter 5",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Can't be const since it's taken as a non-const reference in SetupOutputVariable, but I will NOT modify it
    Real64 VariableToBeSummedDuringHourShown = 1.0;

    Real64 ConditionA = 0.0;
    Real64 ConditionNotA = 0.0;

    Real64 expectedTotalConditionA = 0.0;
    Real64 expectedTotalConditionNotA = 0.0;

    auto setConditionAB = [&ConditionA, &ConditionNotA, &expectedTotalConditionA, &expectedTotalConditionNotA](bool isConditionA) {
        if (isConditionA) {
            ConditionA = 1.0;
            ConditionNotA = 0.0;
        } else {
            ConditionA = 0.0;
            ConditionNotA = 1.0;
        }
        expectedTotalConditionA += ConditionA;
        expectedTotalConditionNotA += ConditionNotA;
    };

    SetupOutputVariable(*state,
                        "ConditionA",
                        Constant::Units::J,
                        ConditionA,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "SomethingNamed",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "ConditionA_Inverse",
                        Constant::Units::J,
                        ConditionNotA,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "SomethingNamed",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");
    SetupOutputVariable(*state,
                        "VariableToBeSummedDuringHoursShown",
                        Constant::Units::J,
                        VariableToBeSummedDuringHourShown,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "SomethingNamed",
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::ExteriorLights,
                        "General");

    state->dataGlobal->DoWeathSim = true;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * 60.0;
    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    auto &ort = state->dataOutRptTab;
    OutputReportTabular::GetInputTabularMonthly(*state);
    EXPECT_EQ(ort->MonthlyInputCount, 1);
    OutputReportTabular::InitializeTabularMonthly(*state);

    compare_err_stream("");

    // I made all 3 variables use the same ObjectName for simplicity (if I use 3 object names, there are actually 3 * 5 = 15 MonthlyColumns...)
    EXPECT_EQ(5, ort->MonthlyColumns.size());

    constexpr int colConditionA = 1;
    constexpr int colValueWhenConditionA = 2;
    constexpr int colConditionNotA = 3;
    constexpr int colValueWhenConditionNotA = 4;
    constexpr int colValue = 5;
    EXPECT_EQ("CONDITIONA", ort->MonthlyColumns(colConditionA).varName);
    EXPECT_EQ("VARIABLETOBESUMMEDDURINGHOURSSHOWN", ort->MonthlyColumns(colValueWhenConditionA).varName);
    EXPECT_EQ("CONDITIONA_INVERSE", ort->MonthlyColumns(colConditionNotA).varName);
    EXPECT_EQ("VARIABLETOBESUMMEDDURINGHOURSSHOWN", ort->MonthlyColumns(colValueWhenConditionNotA).varName);
    EXPECT_EQ("VARIABLETOBESUMMEDDURINGHOURSSHOWN", ort->MonthlyColumns(colValue).varName);

    state->dataEnvrn->Month = 12;

    {
        setConditionAB(true);
        GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
        compare_err_stream("");
        EXPECT_EQ(1.0, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(1.0, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(0.0, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(0.0, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(1.0, ort->MonthlyColumns(colValue).reslt(12));

        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA + expectedTotalConditionNotA, ort->MonthlyColumns(colValue).reslt(12));
    }

    {
        setConditionAB(true);
        GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
        EXPECT_EQ(2.0, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(2.0, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(0.0, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(0.0, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(2.0, ort->MonthlyColumns(colValue).reslt(12));

        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA + expectedTotalConditionNotA, ort->MonthlyColumns(colValue).reslt(12));
    }

    {
        setConditionAB(false);
        GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA + expectedTotalConditionNotA, ort->MonthlyColumns(colValue).reslt(12));
    }

    {
        setConditionAB(false);
        GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA + expectedTotalConditionNotA, ort->MonthlyColumns(colValue).reslt(12));
    }

    {
        setConditionAB(true);
        GatherMonthlyResultsForTimestep(*state, OutputProcessor::TimeStepType::Zone);
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA, ort->MonthlyColumns(colValueWhenConditionA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionNotA, ort->MonthlyColumns(colValueWhenConditionNotA).reslt(12));
        EXPECT_EQ(expectedTotalConditionA + expectedTotalConditionNotA, ort->MonthlyColumns(colValue).reslt(12));
    }
}

TEST_F(EnergyPlusFixture, LEEDSummary_RenewableEnergySourceSummary)
{
    // Test for #11224
    std::string const idf_objects_1 = R"IDF(
  Building,
    PVWatts Test Case,       !- Name
    0.0,                     !- North Axis {deg}
    Suburbs,                 !- Terrain
    0.04,                    !- Loads Convergence Tolerance Value {W}
    0.4,                     !- Temperature Convergence Tolerance Value {deltaC}
    FullInteriorAndExterior, !- Solar Distribution
    25,                      !- Maximum Number of Warmup Days
    6;                       !- Minimum Number of Warmup Days

  Timestep,1;

  SimulationControl,
    No,                      !- Do Zone Sizing Calculation
    No,                      !- Do System Sizing Calculation
    No,                      !- Do Plant Sizing Calculation
    Yes,                     !- Run Simulation for Sizing Periods
    No,                      !- Run Simulation for Weather File Run Periods
    No,                      !- Do HVAC Sizing Simulation for Sizing Periods
    1;                       !- Maximum Number of HVAC Sizing Simulation Passes

  RunPeriod,
    All Year,                !- Name
    1,                       !- Begin Month
    1,                       !- Begin Day of Month
    ,                        !- Begin Year
    12,                      !- End Month
    31,                      !- End Day of Month
    ,                        !- End Year
    ,                        !- Day of Week for Start Day
    Yes,                     !- Use Weather File Holidays and Special Days
    Yes,                     !- Use Weather File Daylight Saving Period
    No,                      !- Apply Weekend Holiday Rule
    Yes,                     !- Use Weather File Rain Indicators
    Yes;                     !- Use Weather File Snow Indicators

  GlobalGeometryRules,
    UpperLeftCorner,         !- Starting Vertex Position
    CounterClockWise,        !- Vertex Entry Direction
    Relative;                !- Coordinate System

  Site:Location,
    Phoenix Sky Harbor Intl Ap_AZ_USA Design_Conditions,  !- Name
    33.45,                   !- Latitude {deg}
    -111.98,                 !- Longitude {deg}
    -7.00,                   !- Time Zone {hr}
    337.00;                  !- Elevation {m}

! Phoenix Sky Harbor Intl Ap_AZ_USA Annual Heating 99%, MaxDB=5.2°C

  SizingPeriod:DesignDay,
    Phoenix Sky Harbor Intl Ap Ann Htg 99% Condns DB,  !- Name
    12,                      !- Month
    21,                      !- Day of Month
    WinterDesignDay,         !- Day Type
    5.2,                     !- Maximum Dry-Bulb Temperature {C}
    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}
    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type
    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name
    Wetbulb,                 !- Humidity Condition Type
    5.2,                     !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}
    ,                        !- Humidity Condition Day Schedule Name
    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}
    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}
    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}
    97342.,                  !- Barometric Pressure {Pa}
    1.7,                     !- Wind Speed {m/s}
    100,                     !- Wind Direction {deg}
    No,                      !- Rain Indicator
    No,                      !- Snow Indicator
    No,                      !- Daylight Saving Time Indicator
    ASHRAEClearSky,          !- Solar Model Indicator
    ,                        !- Beam Solar Day Schedule Name
    ,                        !- Diffuse Solar Day Schedule Name
    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}
    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}
    0.00;                    !- Sky Clearness

! Phoenix Sky Harbor Intl Ap_AZ_USA Annual Cooling (DB=>MWB) 1%, MaxDB=42.3°C MWB=21°C

  SizingPeriod:DesignDay,
    Phoenix Sky Harbor Intl Ap Ann Clg 1% Condns DB=>MWB,  !- Name
    7,                       !- Month
    21,                      !- Day of Month
    SummerDesignDay,         !- Day Type
    42.3,                    !- Maximum Dry-Bulb Temperature {C}
    12,                      !- Daily Dry-Bulb Temperature Range {deltaC}
    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type
    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name
    Wetbulb,                 !- Humidity Condition Type
    21,                      !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}
    ,                        !- Humidity Condition Day Schedule Name
    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}
    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}
    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}
    97342.,                  !- Barometric Pressure {Pa}
    4.1,                     !- Wind Speed {m/s}
    260,                     !- Wind Direction {deg}
    No,                      !- Rain Indicator
    No,                      !- Snow Indicator
    No,                      !- Daylight Saving Time Indicator
    ASHRAETau,               !- Solar Model Indicator
    ,                        !- Beam Solar Day Schedule Name
    ,                        !- Diffuse Solar Day Schedule Name
    0.588,                   !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}
    1.653;                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}
)IDF";

    std::string const idf_objects_2 = R"IDF(

  Material:NoMass,
    R13LAYER,                !- Name
    Rough,                   !- Roughness
    2.290965,                !- Thermal Resistance {m2-K/W}
    0.9000000,               !- Thermal Absorptance
    0.7500000,               !- Solar Absorptance
    0.7500000;               !- Visible Absorptance

  Material:NoMass,
    R31LAYER,                !- Name
    Rough,                   !- Roughness
    5.456,                   !- Thermal Resistance {m2-K/W}
    0.9000000,               !- Thermal Absorptance
    0.7500000,               !- Solar Absorptance
    0.7500000;               !- Visible Absorptance

  Material,
    C5 - 4 IN HW CONCRETE,   !- Name
    MediumRough,             !- Roughness
    0.1014984,               !- Thickness {m}
    1.729577,                !- Conductivity {W/m-K}
    2242.585,                !- Density {kg/m3}
    836.8000,                !- Specific Heat {J/kg-K}
    0.9000000,               !- Thermal Absorptance
    0.6500000,               !- Solar Absorptance
    0.6500000;               !- Visible Absorptance

  Construction,
    R13WALL,                 !- Name
    R13LAYER;                !- Outside Layer

  Construction,
    FLOOR,                   !- Name
    C5 - 4 IN HW CONCRETE;   !- Outside Layer

  Construction,
    ROOF31,                  !- Name
    R31LAYER;                !- Outside Layer

  Zone,
    ZONE ONE,                !- Name
    0,                       !- Direction of Relative North {deg}
    0,                       !- X Origin {m}
    0,                       !- Y Origin {m}
    0,                       !- Z Origin {m}
    1,                       !- Type
    1,                       !- Multiplier
    autocalculate,           !- Ceiling Height {m}
    autocalculate;           !- Volume {m3}

  ScheduleTypeLimits,
    OnOff,                   !- Name
    0,                       !- Lower Limit Value
    1,                       !- Upper Limit Value
    Discrete;                !- Numeric Type

  BuildingSurface:Detailed,
    Zn001:Wall001,           !- Name
    Wall,                    !- Surface Type
    R13WALL,                 !- Construction Name
    ZONE ONE,                !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5000000,               !- View Factor to Ground
    4,                       !- Number of Vertices
    0,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}
    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}
    15.24000,0,0,  !- X,Y,Z ==> Vertex 3 {m}
    15.24000,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall002,           !- Name
    Wall,                    !- Surface Type
    R13WALL,                 !- Construction Name
    ZONE ONE,                !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5000000,               !- View Factor to Ground
    4,                       !- Number of Vertices
    15.24000,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}
    15.24000,0,0,  !- X,Y,Z ==> Vertex 2 {m}
    15.24000,15.24000,0,  !- X,Y,Z ==> Vertex 3 {m}
    15.24000,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall003,           !- Name
    Wall,                    !- Surface Type
    R13WALL,                 !- Construction Name
    ZONE ONE,                !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5000000,               !- View Factor to Ground
    4,                       !- Number of Vertices
    15.24000,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}
    15.24000,15.24000,0,  !- X,Y,Z ==> Vertex 2 {m}
    0,15.24000,0,  !- X,Y,Z ==> Vertex 3 {m}
    0,15.24000,4.572000;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Wall004,           !- Name
    Wall,                    !- Surface Type
    R13WALL,                 !- Construction Name
    ZONE ONE,                !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0.5000000,               !- View Factor to Ground
    4,                       !- Number of Vertices
    0,15.24000,4.572000,  !- X,Y,Z ==> Vertex 1 {m}
    0,15.24000,0,  !- X,Y,Z ==> Vertex 2 {m}
    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}
    0,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Flr001,            !- Name
    Floor,                   !- Surface Type
    FLOOR,                   !- Construction Name
    ZONE ONE,                !- Zone Name
    ,                        !- Space Name
    Adiabatic,               !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    NoSun,                   !- Sun Exposure
    NoWind,                  !- Wind Exposure
    1.000000,                !- View Factor to Ground
    4,                       !- Number of Vertices
    15.24000,0.000000,0.0,  !- X,Y,Z ==> Vertex 1 {m}
    0.000000,0.000000,0.0,  !- X,Y,Z ==> Vertex 2 {m}
    0.000000,15.24000,0.0,  !- X,Y,Z ==> Vertex 3 {m}
    15.24000,15.24000,0.0;  !- X,Y,Z ==> Vertex 4 {m}

  BuildingSurface:Detailed,
    Zn001:Roof001,           !- Name
    Roof,                    !- Surface Type
    ROOF31,                  !- Construction Name
    ZONE ONE,                !- Zone Name
    ,                        !- Space Name
    Outdoors,                !- Outside Boundary Condition
    ,                        !- Outside Boundary Condition Object
    SunExposed,              !- Sun Exposure
    WindExposed,             !- Wind Exposure
    0,                       !- View Factor to Ground
    4,                       !- Number of Vertices
    0.000000,15.24000,4.572,  !- X,Y,Z ==> Vertex 1 {m}
    0.000000,0.000000,4.572,  !- X,Y,Z ==> Vertex 2 {m}
    15.24000,0.000000,4.572,  !- X,Y,Z ==> Vertex 3 {m}
    15.24000,15.24000,4.572;  !- X,Y,Z ==> Vertex 4 {m}
)IDF";

    std::string const idf_objects_3 = R"IDF(

  ElectricLoadCenter:Distribution,
    ELC1,                    !- Name
    PVList1,                 !- Generator List Name
    Baseload,                !- Generator Operation Scheme Type
    0,                       !- Generator Demand Limit Scheme Purchased Electric Demand Limit {W}
    ,                        !- Generator Track Schedule Name Scheme Schedule Name
    ,                        !- Generator Track Meter Scheme Meter Name
    DirectCurrentWithInverter,  !- Electrical Buss Type
    Inverter1;               !- Inverter Name

  ElectricLoadCenter:Inverter:PVWatts,
    Inverter1,               !- Name
    1.10,                    !- DC to AC Size Ratio
    0.96;                    !- Inverter Efficiency

  ElectricLoadCenter:Generators,
    PVList1,                 !- Name
    PVWatts1,                !- Generator 1 Name
    Generator:PVWatts,       !- Generator 1 Object Type
    4000,                    !- Generator 1 Rated Electric Power Output {W}
    ,                        !- Generator 1 Availability Schedule Name
    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio
    PVWatts2,                !- Generator 2 Name
    Generator:PVWatts,       !- Generator 2 Object Type
    3000,                    !- Generator 2 Rated Electric Power Output {W}
    ,                        !- Generator 2 Availability Schedule Name
    ,                        !- Generator 2 Rated Thermal to Electrical Power Ratio
    PVWatts3,                !- Generator 3 Name
    Generator:PVWatts,       !- Generator 3 Object Type
    3000,                    !- Generator 3 Rated Electric Power Output {W}
    ,                        !- Generator 3 Availability Schedule Name
    ;                        !- Generator 3 Rated Thermal to Electrical Power Ratio

  Generator:PVWatts,
    PVWatts1,                !- Name
    5,                       !- PVWatts Version
    4000,                    !- DC System Capacity {W}
    Standard,                !- Module Type
    FixedOpenRack,           !- Array Type
    0.14,                    !- System Losses
    TiltAzimuth,             !- Array Geometry Type
    20,                      !- Tilt Angle {deg}
    180;                     !- Azimuth Angle {deg}

  Shading:Site:Detailed,
    FlatSurface,             !- Name
    ,                        !- Transmittance Schedule Name
    4,                       !- Number of Vertices
    40.0,2.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}
    40.0,0.00,0.0,  !- X,Y,Z ==> Vertex 2 {m}
    45.0,0.00,0.0,  !- X,Y,Z ==> Vertex 3 {m}
    45.0,2.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}

  Generator:PVWatts,
    PVWatts2,                !- Name
    5,                       !- PVWatts Version
    3000,                    !- DC System Capacity {W}
    Premium,                 !- Module Type
    FixedOpenRack,           !- Array Type
    0.14,                    !- System Losses
    Surface,                 !- Array Geometry Type
    ,                        !- Tilt Angle {deg}
    ,                        !- Azimuth Angle {deg}
    FlatSurface;             !- Surface Name

  Shading:Site:Detailed,
    FlatSurfaceShadetoEast,  !- Name
    ,                        !- Transmittance Schedule Name
    4,                       !- Number of Vertices
    0.0,25.0,12.0,  !- X,Y,Z ==> Vertex 1 {m}
    0.0,20.00,12.0,  !- X,Y,Z ==> Vertex 2 {m}
    5.0,20.00,12.0,  !- X,Y,Z ==> Vertex 3 {m}
    5.0,25.0,12.0;  !- X,Y,Z ==> Vertex 4 {m}

  Shading:Site:Detailed,
    ShadetoEast,             !- Name
    ,                        !- Transmittance Schedule Name
    4,                       !- Number of Vertices
    5.0,25.0,22.0,  !- X,Y,Z ==> Vertex 1 {m}
    5.0,25.00,12.0,  !- X,Y,Z ==> Vertex 2 {m}
    5.0,20.00,12.0,  !- X,Y,Z ==> Vertex 3 {m}
    5.0,20.0,22.0;  !- X,Y,Z ==> Vertex 4 {m}

  Generator:PVWatts,
    PVWatts3,                !- Name
    5,                       !- PVWatts Version
    3000,                    !- DC System Capacity {W}
    Premium,                 !- Module Type
    FixedOpenRack,           !- Array Type
    0.14,                    !- System Losses
    Surface,                 !- Array Geometry Type
    ,                        !- Tilt Angle {deg}
    ,                        !- Azimuth Angle {deg}
    FlatSurfaceShadetoEast;  !- Surface Name

  Output:VariableDictionary,regular;

  OutputControl:Table:Style,
    TabAndHTML;              !- Column Separator

  Output:SQLite,
    SimpleAndTabular;        !- Option Type

  Output:Table:SummaryReports,
    AllSummary,                 !- Report 1 Name
    AllSummaryAndSizingPeriod;  !- Report 2 Name
    )IDF";

    std::string const idf_objects = idf_objects_1 + idf_objects_2 + idf_objects_3;
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    ManageSimulation(*state); // run the design days
    // get user specified generators capacity
    auto &elecGenObjs = state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj;
    double gens_cap_kW = 0.0;
    for (const auto &g : elecGenObjs) {
        if (g->generatorType == GeneratorType::PVWatts) {
            gens_cap_kW += g->pvwattsGenerator->getDCSystemCapacity() / 1000.0;
        }
    }
    // check user specified generators capacity, kW
    EXPECT_EQ(10.0, gens_cap_kW);
    // check tabular data for a design day run
    auto &orp = *state->dataOutRptPredefined;
    EXPECT_EQ("10.00", RetrievePreDefTableEntry(*state, orp.pdchLeedRenRatCap, "Photovoltaic"));
    EXPECT_EQ("0.00", RetrievePreDefTableEntry(*state, orp.pdchLeedRenAnGen, "Photovoltaic"));
}

TEST_F(EnergyPlusFixture, ExteriorFenestrationShadedStateTest)
{
    // Test for Fix of Defect #10919
    // Allocations and setting of test data
    auto &dHB = state->dataHeatBal;
    auto &dCon = state->dataConstruction;
    auto &dSurf = state->dataSurface;
    dHB->space.allocate(1);
    dHB->Zone.allocate(1);
    dHB->Zone(1).ListMultiplier = 1;
    dCon->Construct.allocate(2);
    dCon->Construct(1).Name = "LowTalker";
    dCon->Construct(1).OutsideAbsorpSolar = 0.1;
    dCon->Construct(1).VisTransNorm = 0.3;
    dCon->Construct(1).SummerSHGC = 0.1;
    dCon->Construct(1).TotLayers = 1;
    dCon->Construct(1).TotGlassLayers = 1;
    dCon->Construct(1).LayerPoint.allocate(dCon->Construct(1).TotLayers);
    dCon->Construct(1).LayerPoint(1) = 1;
    dCon->Construct(1).AbsDiff.allocate(dCon->Construct(1).TotLayers);
    dCon->Construct(1).AbsDiff(1) = 0.75;
    dCon->Construct(2).Name = "CloseTalker";
    dCon->Construct(2).OutsideAbsorpSolar = 0.9;
    dCon->Construct(2).VisTransNorm = 0.7;
    dCon->Construct(2).SummerSHGC = 0.9;
    dCon->Construct(2).TotLayers = 1;
    dCon->Construct(2).TotGlassLayers = 1;
    dCon->Construct(2).LayerPoint.allocate(dCon->Construct(2).TotLayers);
    dCon->Construct(2).LayerPoint(1) = 2;
    dCon->Construct(2).AbsDiff.allocate(dCon->Construct(1).TotLayers);
    dCon->Construct(2).AbsDiff(1) = 0.75;

    state->init_state(*state);
    auto &s_mat = state->dataMaterial;

    auto *mat1 = new Material::MaterialGlass;
    s_mat->materials.push_back(mat1);
    mat1->Thickness = 0.1;
    mat1->Conductivity = 0.125;
    mat1->Resistance = 1.25;
    mat1->Roughness = Material::SurfaceRoughness::VerySmooth;
    mat1->group = Material::Group::Glass;
    mat1->AbsorpSolar = 0.75;
    mat1->AbsorpThermal = 0.75;
    mat1->Trans = 0.25;
    mat1->ReflectSolBeamFront = 0.20;

    auto *mat2 = new Material::MaterialGlass;
    s_mat->materials.push_back(mat2);
    mat2->Thickness = 0.1;
    mat2->Conductivity = 0.25;
    mat2->Resistance = 2.5;
    mat2->Roughness = Material::SurfaceRoughness::VerySmooth;
    mat2->group = Material::Group::Glass;
    mat2->AbsorpSolar = 0.25;
    mat2->AbsorpThermal = 0.25;
    mat2->Trans = 0.5;
    mat2->ReflectSolBeamFront = 0.20;

    EnergyPlus::Window::initWindowModel(*state);

    // auto aModel = std::make_unique<WindowModel>(); // (AUTO_OK)
    // aModel->m_Model = static_cast<WindowModel>(getEnumValue(EnergyPlus::Window::windowsModelNamesUC, "BUILTINWINDOWSMODEL"));

    dHB->NominalU.allocate(2);
    dHB->NominalU(1) = 1.1;
    dHB->NominalU(2) = 2.2;
    dHB->NominalUBeforeAdjusted.allocate(2);
    dHB->NominalUBeforeAdjusted(1) = 1.1;
    dHB->NominalUBeforeAdjusted(2) = 2.2;

    dSurf->TotSurfaces = 4;
    dSurf->Surface.allocate(dSurf->TotSurfaces);
    dSurf->SurfaceWindow.allocate(dSurf->TotSurfaces);
    dSurf->Surface(2).windowShadingControlList.resize(1);
    dSurf->Surface(2).windowShadingControlList[0] = 1;
    dSurf->Surface(4).windowShadingControlList.resize(1);
    dSurf->Surface(4).windowShadingControlList[0] = 2;
    dSurf->Surface(2).shadedConstructionList.resize(1);
    dSurf->Surface(2).shadedConstructionList[0] = 1;
    dSurf->Surface(4).shadedConstructionList.resize(1);
    dSurf->Surface(4).shadedConstructionList[0] = 2;

    // first surface, exterior walls and doors
    for (int i = 1; i <= dSurf->TotSurfaces; i++) {
        dSurf->Surface(i).HeatTransSurf = true;
        dSurf->Surface(i).Azimuth = 180.;
        dSurf->Surface(i).Tilt = 90.;
        dSurf->Surface(i).ExtBoundCond = 0;
        dSurf->Surface(i).FrameDivider = 0;
        dSurf->Surface(i).spaceNum = 1;
        dSurf->Surface(i).Zone = 1;
        if ((i == 1) || (i == 2)) {
            dSurf->Surface(i).Construction = 1;
        } else {
            dSurf->Surface(i).Construction = 2;
        }
        // odd number - wall, even number - window
        if (i % 2 == 1) {
            dSurf->Surface(i).Name = "Exterior_Wall_" + fmt::to_string((i + 1) / 2);
            dSurf->Surface(i).GrossArea = 200.;
            dSurf->Surface(i).Class = DataSurfaces::SurfaceClass::Wall;
            dSurf->AllSurfaceListReportOrder.push_back(i);
        } else {
            dSurf->Surface(i).Name = "Window_" + fmt::to_string((i + 1) / 2);
            dSurf->Surface(i).BaseSurfName = dSurf->Surface(i - 1).Name;
            dSurf->Surface(i).BaseSurf = i - 1;
            dSurf->Surface(i).GrossArea = 50.;
            dSurf->Surface(i).Class = DataSurfaces::SurfaceClass::Window;
            dSurf->AllSurfaceListReportOrder.push_back(i);
        }
    }

    // Setup pre def tables
    OutputReportPredefined::SetPredefinedTables(*state);

    // Call the routine that fills up the table we care about
    HeatBalanceSurfaceManager::GatherForPredefinedReport(*state);

    // Check output to see that it matches expectations
    auto &dORP = state->dataOutRptPredefined;
    EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(*state, dORP->pdchFenShdUfact, dCon->Construct(1).Name), "0.805");
    EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(*state, dORP->pdchFenShdSHGC, dCon->Construct(1).Name), "0.309");
    EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(*state, dORP->pdchFenShdVisTr, dCon->Construct(1).Name), "0.300");
    EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(*state, dORP->pdchFenShdUfact, dCon->Construct(2).Name), "1.237");
    EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(*state, dORP->pdchFenShdSHGC, dCon->Construct(2).Name), "0.272");
    EXPECT_EQ(OutputReportPredefined::RetrievePreDefTableEntry(*state, dORP->pdchFenShdVisTr, dCon->Construct(2).Name), "0.700");
}
