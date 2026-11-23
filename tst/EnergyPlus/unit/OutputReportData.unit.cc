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

// EnergyPlus::OutputReportTabular Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportData.hh>
#include <EnergyPlus/UtilityRoutines.hh>

using namespace EnergyPlus;
using namespace OutputProcessor;
using namespace DataOutputs;
TEST_F(EnergyPlusFixture, OutputReportData_AnnualFieldSetConstructor)
{
    std::string varNameTest = "TestReport";
    AnnualFieldSet::AggregationKind kindOfAggregationTest = AnnualFieldSet::AggregationKind::sumOrAvg;
    int numDigitsShownTest = 3;
    AnnualFieldSet fldStTest = AnnualFieldSet(varNameTest, kindOfAggregationTest, numDigitsShownTest);
    EXPECT_EQ(fldStTest.m_variMeter, varNameTest);
    EXPECT_EQ(fldStTest.m_aggregate, kindOfAggregationTest);
    EXPECT_EQ(fldStTest.m_showDigits, numDigitsShownTest);
}

TEST_F(EnergyPlusFixture, OutputReportData_getVariableKeys)
{
    std::string varNameTest = "TestReport";
    AnnualFieldSet::AggregationKind kindOfAggregationTest = AnnualFieldSet::AggregationKind::sumOrAvg;
    int numDigitsShownTest = 3;
    AnnualFieldSet fldStTest = AnnualFieldSet(varNameTest, kindOfAggregationTest, numDigitsShownTest);

    Real64 extLitPow;
    Real64 extLitUse;

    SetupOutputVariable(*state,
                        "Exterior Lights Electric Energy",
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
                        "Exterior Lights Electric Energy",
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
                        "Exterior Lights Electric Energy",
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
                        "Exterior Lights Electric Power",
                        Constant::Units::W,
                        extLitPow,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        "Lite1");
    SetupOutputVariable(*state,
                        "Exterior Lights Electric Power",
                        Constant::Units::W,
                        extLitPow,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        "Lite2");
    SetupOutputVariable(*state,
                        "Exterior Lights Electric Power",
                        Constant::Units::W,
                        extLitPow,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        "Lite3");

    int keyCount = 0;
    OutputProcessor::VariableType typeVar = OutputProcessor::VariableType::Invalid;
    OutputProcessor::StoreType avgSumVar;
    OutputProcessor::TimeStepType stepTypeVar;
    Constant::Units unitsVar = Constant::Units::None;

    fldStTest.m_variMeter = "EXTERIOR LIGHTS ELECTRIC ENERGY";
    keyCount = fldStTest.getVariableKeyCountandTypeFromFldSt(*state, typeVar, avgSumVar, stepTypeVar, unitsVar);
    EXPECT_EQ(keyCount, 3);

    fldStTest.getVariableKeysFromFldSt(*state, typeVar, keyCount, fldStTest.m_namesOfKeys, fldStTest.m_indexesForKeyVar);

    EXPECT_EQ(fldStTest.m_namesOfKeys[0], "LITE1");
    EXPECT_EQ(fldStTest.m_namesOfKeys[1], "LITE2");
    EXPECT_EQ(fldStTest.m_namesOfKeys[2], "LITE3");
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "Outside Air Inlet Node,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "Relief Air Outlet Node,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "(Relief|Outside) Air (Outlet|Inlet) Node,",
        "System Node Temperature,",
        "timestep;",
        " Output:Variable,",
        "Mixed Air Node,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "(Mixed|Single) Air Node,",
        "System Node Temperature,",
        "timestep;",
        " Output:Variable,",
        "*,",
        "Unitary System Compressor Part Load Ratio,",
        "timestep;",
        " Output:Variable,",
        ".*,",
        "Zone Air System Sensible Heating Rate,",
        "timestep;",
        " Output:Variable,",
        "SALESFLOOR OUTLET NODE,",
        "System Node Temperature,",
        "timestep;",
        " Output:Variable,",
        "BackRoom(.*),",
        "System Node Temperature,",
        "timestep;",
        " Output:Variable,",
        "(.*)N(ode|ODE),",
        "System Node Humidity Ratio,",
        "timestep;",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 10);
    // FindItemInVariableList is case-insensitive, so we also test that
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "OUTSIDE AIR INLET NODE", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "OutsIDE AiR InLEt NoDE", "System NoDE MaSS FLOw Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "OUTSIDE AIR INLET NODE", "System NODE Mass Flow RATE"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Mixed Air Node", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Inlet Node", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Inlet Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Any Node Here", "Zone Air System Sensible Heating Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Salesfloor Outlet Node", "System Node Temperature"));
    EXPECT_FALSE(FindItemInVariableList(*state, "AnySalesfloor Outlet Node", "System Node Temperature"));
    EXPECT_FALSE(FindItemInVariableList(*state, "AnyOutside Air Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom OUTLET NODE", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom Inlet NODE", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom Node", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom OUTLET NODE", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom Inlet NODE", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom Any Node", "System Node Humidity Ratio"));
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_Plus)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "(.+)Inlet(.+),",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "(.+)Inlet,",
        "System Node Humidity Ratio,",
        "timestep;",
        " Output:Variable,",
        "(.+)Node,",
        "Zone Air System Sensible Heating Rate,",
        "timestep;",
        " Output:Variable,",
        "Outside Air (.+) Node,",
        "Unitary System Compressor Part Load Ratio,",
        "timestep;",
        " Output:Variable,",
        "Outside Air .+ Node,",
        "Unitary System Load Ratio,",
        "timestep;",
        " Output:Variable,",
        ".+,",
        "System Node Temperature,",
        "timestep;",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 6);
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor INLET Node", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Inlet", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "BackRoom Inlet Node", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom Any Node", "Zone Air System Sensible Heating Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Inlet Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Outlet Node", "Unitary System Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Any Node", "System Node Temperature"));
    EXPECT_FALSE(FindItemInVariableList(*state, "", "System Node Temperature"));
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_Star)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "(.*)Inlet(.*),",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "(.*)Inlet,",
        "System Node Humidity Ratio,",
        "timestep;",
        " Output:Variable,",
        "(.*)Node,",
        "Zone Air System Sensible Heating Rate,",
        "timestep;",
        " Output:Variable,",
        "Outside Air(.*) Node,",
        "Unitary System Compressor Part Load Ratio,",
        "timestep;",
        " Output:Variable,",
        "Outside Air.* Node,",
        "Unitary System Load Ratio,",
        "timestep;",
        " Output:Variable,",
        ".*,",
        "System Node Temperature,",
        "timestep;",
        " Output:Variable,",
        "*,",
        "Refrigeration Compressor Rack Electric Power,",
        "timestep;",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 7);
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor INLET Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Inlet", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "BackRoom Inlet Node", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Inlet", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Any Inlet", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom Any Node", "Zone Air System Sensible Heating Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Node", "Zone Air System Sensible Heating Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "NODE", "Zone Air System Sensible Heating Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Inlet Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Outlet Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Outlet Node", "Unitary System Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outside Air Node", "Unitary System Load Ratio"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Outside AirNode", "Unitary System Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Any Node", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "", "System Node Temperature"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Any Node", "Refrigeration Compressor Rack Electric Power"));
    EXPECT_TRUE(FindItemInVariableList(*state, "", "Refrigeration Compressor Rack Electric Power"));
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_Pipe)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "SalesFloor I(nlet|NLET) Node,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "SalesFloor O(utlet|UTLET) Node,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "System (Inlet|Outlet) Node,",
        "Unitary System Compressor Part Load Ratio,",
        "timestep;",
        " Output:Variable,",
        "(BackRoom|BACKROOM|SALESFLOOR|SalesFloor) (Outlet|OUTLET) (NODE|Node),",
        "System Node Humidity Ratio,",
        "timestep;",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 4);
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor INLET Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Outlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor OUTLET Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "System Inlet Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "System Outlet Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_FALSE(FindItemInVariableList(*state, "System Another Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom OUTLET NODE", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SALESFLOOR OUTLET NODE", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Outlet Node", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BACKROOM Outlet Node", "System Node Humidity Ratio"));
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_Brackets)
{
    std::string const idf_objects = delimited_string({
        "Output:Variable,",
        "([A-Za-z] ?)+,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "[A-Za-z0-9_]+,",
        "System Node Humidity Ratio,",
        "timestep;",
        " Output:Variable,",
        "[A-Z]{4},",
        "Unitary System Compressor Part Load Ratio,",
        "timestep;",
        " Output:Variable,",
        "[A-Za-z]{5,6},",
        "Zone Air System Sensible Heating Rate,",
        "timestep;",
        " Output:Variable,",
        "[A-Za-z ]{5,},",
        "Refrigeration Compressor Rack Electric Power,",
        "timestep;",
        " Output:Variable,",
        "([A-Za-z] ?)+,",
        "System Node Mass Flow Rate,",
        "timestep;",
    });
    EXPECT_FALSE(process_idf(idf_objects, false));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 6);
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Node", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "BackRoom OUTLET NODE", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "BackRoom_NODE1", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "NODE", "Unitary System Compressor Part Load Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Node", "Unitary System Compressor Part Load Ratio"));
    EXPECT_FALSE(FindItemInVariableList(*state, "NOD", "Unitary System Compressor Part Load Ratio"));
    // next 7 test cases are meant for "{,}" type of regexes
    EXPECT_FALSE(FindItemInVariableList(*state, "Inlet", "Zone Air System Sensible Heating Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Outlet", "Zone Air System Sensible Heating Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Any Node", "Zone Air System Sensible Heating Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Inlet", "Refrigeration Compressor Rack Electric Power"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Outlet", "Refrigeration Compressor Rack Electric Power"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Outlet Node", "Refrigeration Compressor Rack Electric Power"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Node", "Refrigeration Compressor Rack Electric Power"));
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_SpecChars)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "\\w,",
        "System Node Mass Flow Rate,",
        "timestep;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 1);

    // TODO: FIXME: This is failing. The IdfParser probably needs to be double checked and tested.
    // EXPECT_TRUE( FindItemInVariableList( "SalesFloor Inlet Node", "System Node Mass Flow Rate" ));

    compare_err_stream("");
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_Carrot)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "^Inlet(.*)Node,",
        "System Node Mass Flow Rate,",
        "timestep;",
        " Output:Variable,",
        "[^0-9]+,",
        "System Node Humidity Ratio,",
        "timestep;",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 2);
    EXPECT_FALSE(FindItemInVariableList(*state, "SalesFloor Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Inlet System Node", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "SalesFloor1", "System Node Humidity Ratio"));
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor", "System Node Humidity Ratio"));
}

TEST_F(EnergyPlusFixture, OutputReportData_Regex_Dollar)
{
    std::string const idf_objects = delimited_string({
        " Output:Variable,",
        "(.*)Node$,",
        "System Node Mass Flow Rate,",
        "timestep;",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    EXPECT_EQ(state->dataOutput->NumConsideredOutputVariables, 1);
    EXPECT_TRUE(FindItemInVariableList(*state, "SalesFloor Inlet Node", "System Node Mass Flow Rate"));
    EXPECT_TRUE(FindItemInVariableList(*state, "Outlet Node", "System Node Mass Flow Rate"));
    EXPECT_FALSE(FindItemInVariableList(*state, "Inlet Node1 ", "System Node Mass Flow Rate"));
}
