// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

// Unit tests for data transfer API methods
#include <utility>
#include <vector>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "../Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/api/datatransfer.h>

using namespace EnergyPlus;

class DataExchangeAPIUnitTestFixture : public EnergyPlusFixture
{

    struct DummyRealVariable
    {
        std::string varName;
        std::string varKey;
        Real64 value = 0.0;
        bool meterType = false;
        DummyRealVariable(std::string _varName, std::string _varKey, Real64 _value, bool _meterType)
            : varName(std::move(_varName)), varKey(std::move(_varKey)), value(_value), meterType(_meterType)
        {
        }
    };
    struct DummyIntVariable
    {
        std::string varName;
        std::string varKey;
        int value = 0;
        DummyIntVariable(std::string _varName, std::string _varKey, Real64 _value)
            : varName(std::move(_varName)), varKey(std::move(_varKey)), value(_value)
        {
        }
    };
    struct DummyBaseActuator
    {
        std::string objType;
        std::string controlType;
        std::string key;
        bool flag = false;
        DummyBaseActuator(std::string _objType, std::string _controlType, std::string _key)
            : objType(std::move(_objType)), controlType(std::move(_controlType)), key(std::move(_key))
        {
        }
    };
    struct DummyRealActuator : DummyBaseActuator
    {
        Real64 val = 0.0;
        DummyRealActuator(const std::string &_objType, const std::string &_controlType, const std::string &_key)
            : DummyBaseActuator(_objType, _controlType, _key)
        {
        }
    };
    struct DummyIntActuator : DummyBaseActuator
    {
        int val = 0;
        DummyIntActuator(const std::string &_objType, const std::string &_controlType, const std::string &_key)
            : DummyBaseActuator(_objType, _controlType, _key)
        {
        }
    };
    struct DummyBoolActuator : DummyBaseActuator
    {
        bool val = true;
        DummyBoolActuator(const std::string &_objType, const std::string &_controlType, const std::string &_key)
            : DummyBaseActuator(_objType, _controlType, _key)
        {
        }
    };
    struct DummyInternalVariable
    {
        std::string varName;
        std::string varKey;
        Real64 value = 0.0;
        DummyInternalVariable(std::string _varName, std::string _varKey, Real64 _value)
            : varName(std::move(_varName)), varKey(std::move(_varKey)), value(_value)
        {
        }
    };
    std::vector<DummyRealVariable> realVariablePlaceholders;
    std::vector<DummyIntVariable> intVariablePlaceholders;
    std::vector<DummyRealActuator> realActuatorPlaceholders;
    std::vector<DummyIntActuator> intActuatorPlaceholders;
    std::vector<DummyBoolActuator> boolActuatorPlaceholders;
    std::vector<DummyInternalVariable> internalVarPlaceholders;

    void SetUp() override
    {
        EnergyPlusFixture::SetUp();
        Real64 timeStep = 1.0;
        OutputProcessor::SetupTimePointers(*state, OutputProcessor::SOVTimeStepType::Zone, timeStep);
        OutputProcessor::SetupTimePointers(*state, OutputProcessor::SOVTimeStepType::HVAC, timeStep);
        *state->dataOutputProcessor->TimeValue.at(OutputProcessor::TimeStepType::Zone).TimeStep = 60;
        *state->dataOutputProcessor->TimeValue.at(OutputProcessor::TimeStepType::System).TimeStep = 60;
        state->dataPluginManager->pluginManager = std::make_unique<EnergyPlus::PluginManagement::PluginManager>(*state);
    }

    void TearDown() override
    {
        this->realVariablePlaceholders.clear();
        this->realActuatorPlaceholders.clear();
        EnergyPlusFixture::TearDown();
    }

public:
    void preRequestRealVariable(std::string const &varName, std::string const &key, Real64 initialValue = 0.0, bool meterType = false)
    {
        this->realVariablePlaceholders.emplace_back(varName, key, initialValue, meterType);
        requestVariable((void *)this->state, varName.c_str(), key.c_str());
    }

    void preRequestIntegerVariable(std::string const &varName, std::string const &key, int initialValue = 0)
    {
        this->intVariablePlaceholders.emplace_back(varName, key, initialValue);
        requestVariable((void *)this->state, varName.c_str(), key.c_str());
    }

    void setupVariablesOnceAllAreRequested()
    {
        state->dataInputProcessing->inputProcessor->preScanReportingVariables(*state);
        for (auto &val : this->realVariablePlaceholders) {
            if (val.meterType) {
                SetupOutputVariable(*state,
                                    val.varName,
                                    OutputProcessor::Unit::kg_s,
                                    val.value,
                                    OutputProcessor::SOVTimeStepType::Zone,
                                    OutputProcessor::SOVStoreType::Summed,
                                    val.varKey,
                                    _,
                                    "ELECTRICITY",
                                    "HEATING",
                                    _,
                                    "System");
            } else {
                SetupOutputVariable(*state,
                                    val.varName,
                                    OutputProcessor::Unit::kg_s,
                                    val.value,
                                    OutputProcessor::SOVTimeStepType::Zone,
                                    OutputProcessor::SOVStoreType::Average,
                                    val.varKey);
            }
        }
        for (auto &val : this->intVariablePlaceholders) {
            SetupOutputVariable(*state,
                                val.varName,
                                OutputProcessor::Unit::kg_s,
                                val.value,
                                OutputProcessor::SOVTimeStepType::Zone,
                                OutputProcessor::SOVStoreType::Average,
                                val.varKey);
        }
    }

    enum class ActuatorType
    {
        REAL,
        INTEGER,
        BOOL
    };
    void preRequestActuator(std::string const &objType, std::string const &controlType, std::string const &objKey, ActuatorType t)
    {
        switch (t) {
        case ActuatorType::REAL:
            this->realActuatorPlaceholders.emplace_back(objType, controlType, objKey);
            break;
        case ActuatorType::INTEGER:
            this->intActuatorPlaceholders.emplace_back(objType, controlType, objKey);
            break;
        case ActuatorType::BOOL:
            this->boolActuatorPlaceholders.emplace_back(objType, controlType, objKey);
            break;
        }
    }

    void setupActuatorsOnceAllAreRequested()
    {
        for (auto &act : this->realActuatorPlaceholders) {
            SetupEMSActuator(*state, act.objType, act.key, act.controlType, "kg/s", act.flag, act.val);
        }
        for (auto &act : this->intActuatorPlaceholders) {
            SetupEMSActuator(*state, act.objType, act.key, act.controlType, "kg/s", act.flag, act.val);
        }
        for (auto &act : this->boolActuatorPlaceholders) {
            SetupEMSActuator(*state, act.objType, act.key, act.controlType, "kg/s", act.flag, act.val);
        }
    }

    void preRequestInternalVariable(std::string const &varType, std::string const &varKey, Real64 const value)
    {
        this->internalVarPlaceholders.emplace_back(varType, varKey, value);
    }

    void setupInternalVariablesOnceAllAreRequested()
    {
        for (auto &iv : this->internalVarPlaceholders) {
            SetupEMSInternalVariable(*state, iv.varName, iv.varKey, "kg/s", iv.value);
        }
    }

    void addPluginGlobal(EnergyPlus::EnergyPlusData &state, std::string const &varName)
    {
        state.dataPluginManager->pluginManager->addGlobalVariable(state, varName);
    }

    void addTrendWithNewGlobal(std::string const &newGlobalVarName, std::string const &trendName, int numTrendValues)
    {
        state->dataPluginManager->pluginManager->addGlobalVariable(*state, newGlobalVarName);
        int i = EnergyPlus::PluginManagement::PluginManager::getGlobalVariableHandle(*state, newGlobalVarName, true);
        state->dataPluginManager->trends.emplace_back(*state, trendName, numTrendValues, i);
    }

    void simulateTimeStepAndReport()
    {
        UpdateMeterReporting(*state);
        UpdateDataandReport(*state, OutputProcessor::TimeStepType::Zone);
    }
};

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestListAllDataInCSV)
{

    std::string const idf_objects = delimited_string({"Version,", DataStringGlobals::MatchVersion + ";"});
    ASSERT_TRUE(process_idf(idf_objects, false)); // this had to be here or I was getting a strange segfault during a JSON string dtor

    // first off, the function should return, even if there isn't anything meaningful in it (it will have headers)
    char *charCsvDataEmpty = listAllAPIDataCSV((void *)this->state);
    std::string strCsvDataEmpty = std::string(charCsvDataEmpty);
    free(charCsvDataEmpty); // free the char*

    // then as we add stuff, and make sure it appears in the output
    this->preRequestRealVariable("Boiler Heat Transfer", "Boiler 1");                 // output variable
    this->preRequestRealVariable("Chiller Electric Energy", "Chiller 1", 3.14, true); // meter
    this->setupVariablesOnceAllAreRequested();
    this->preRequestActuator("Chiller:Electric", "Max Flow Rate", "Chiller 1", ActuatorType::REAL);
    this->setupActuatorsOnceAllAreRequested();
    this->preRequestInternalVariable("Floor Area", "Zone 1", 6.02e23);
    this->setupInternalVariablesOnceAllAreRequested();
    this->addPluginGlobal(*state, "Plugin_Global_Var_Name");
    this->addTrendWithNewGlobal("NewGlobalVarHere", "Trend 1", 3);
    char *charCsvDataFull = listAllAPIDataCSV((void *)this->state);
    std::string csvData = std::string(charCsvDataFull);
    free(charCsvDataFull);                                                        // free the char*
    std::size_t foundAddedBoiler = csvData.find("BOILER 1") != std::string::npos; // Note output variables only keep UC, so we should check UC here
    std::size_t foundAddedMeter = csvData.find("CHILLER 1") != std::string::npos; // Note output variables only keep UC, so we should check UC here
    std::size_t foundAddedActuator = csvData.find("Chiller:Electric") != std::string::npos;
    std::size_t foundAddedIV = csvData.find("Zone 1") != std::string::npos;
    std::size_t foundAddedGlobal =
        csvData.find("PLUGIN_GLOBAL_VAR_NAME") != std::string::npos; // Note globals are kept in upper case internally, check UC here
    std::size_t foundAddedTrend = csvData.find("Trend 1") != std::string::npos;
    EXPECT_TRUE(foundAddedBoiler);
    EXPECT_TRUE(foundAddedMeter);
    EXPECT_TRUE(foundAddedActuator);
    EXPECT_TRUE(foundAddedIV);
    EXPECT_TRUE(foundAddedGlobal);
    EXPECT_TRUE(foundAddedTrend);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestApiDataFullyReady)
{
    // basically, the data should not be ready at the beginning of a unit test -- ever, so just check that for now
    EXPECT_EQ(0, apiDataFullyReady((void *)this->state)); // 0 is false
    EXPECT_FALSE(apiDataFullyReady((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetVariableHandlesRealTypes)
{
    this->preRequestRealVariable("Chiller Heat Transfer", "Chiller 1");
    this->preRequestRealVariable("Zone Mean Temperature", "Zone 1");
    this->setupVariablesOnceAllAreRequested();
    int hChillerHT = getVariableHandle((void *)this->state, "Chiller Heat Transfer", "Chiller 1");
    int hZoneTemp = getVariableHandle((void *)this->state, "Zone Mean Temperature", "Zone 1");
    EXPECT_GT(hChillerHT, -1);
    EXPECT_GT(hZoneTemp, -1);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetVariableHandlesIntegerTypes)
{
    this->preRequestIntegerVariable("Chiller Operating Mode", "Chiller 1");
    this->preRequestIntegerVariable("Chiller Operating Mode", "Chiller 2");
    this->setupVariablesOnceAllAreRequested();
    int hChillerMode1 = getVariableHandle((void *)this->state, "Chiller Operating Mode", "Chiller 1");
    int hChillerMode2 = getVariableHandle((void *)this->state, "Chiller Operating Mode", "Chiller 2");
    EXPECT_GT(hChillerMode1, -1);
    EXPECT_GT(hChillerMode2, -1);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetVariableHandlesMixedTypes)
{
    // In order to access them, you must request them to make them available, then look them up...add a couple floating point ones and an integer one
    this->preRequestRealVariable("Chiller Heat Transfer", "Chiller 1");
    this->preRequestRealVariable("Zone Mean Temperature", "Zone 1");
    this->preRequestIntegerVariable("Chiller Operating Mode", "Chiller 1");
    this->setupVariablesOnceAllAreRequested();
    // Then try to get their handles
    int hChillerHT = getVariableHandle((void *)this->state, "Chiller Heat Transfer", "Chiller 1");
    int hZoneTemp = getVariableHandle((void *)this->state, "Zone Mean Temperature", "Zone 1");
    int hChillerMode = getVariableHandle((void *)this->state, "Chiller Operating Mode", "Chiller 1");
    EXPECT_GT(hChillerHT, -1);
    EXPECT_GT(hZoneTemp, -1);
    EXPECT_GT(hChillerMode, -1);
    // now try to get handles to variables that doesn't exist
    int hChiller2HT = getVariableHandle((void *)this->state, "Chiller Heat Transfer", "Chiller 2");
    int hZone2Temp = getVariableHandle((void *)this->state, "Zone Mean Radiant Temperature", "Zone 1");
    EXPECT_EQ(-1, hChiller2HT);
    EXPECT_EQ(-1, hZone2Temp);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetVariableValuesRealTypes)
{
    this->preRequestRealVariable("Chiller Heat Transfer", "Chiller 1", 3.14);
    this->preRequestRealVariable("Zone Mean Temperature", "Zone 1", 2.718);
    this->setupVariablesOnceAllAreRequested();
    int hChillerHT = getVariableHandle((void *)this->state, "Chiller Heat Transfer", "Chiller 1");
    int hZoneTemp = getVariableHandle((void *)this->state, "Zone Mean Temperature", "Zone 1");

    // pretend like E+ ran a time step
    this->simulateTimeStepAndReport();

    // get the values for valid handles
    Real64 curHeatTransfer = getVariableValue((void *)this->state, hChillerHT);
    Real64 curZoneTemp = getVariableValue((void *)this->state, hZoneTemp);
    EXPECT_NEAR(3.14, curHeatTransfer, 0.0001);
    EXPECT_NEAR(2.718, curZoneTemp, 0.0001);

    // now test invalid handles
    getVariableValue((void *)this->state, -1);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    getVariableValue((void *)this->state, 3);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetMeterHandles)
{
    this->preRequestRealVariable("Chiller Electric Energy", "Chiller 1", 3.14, true);
    this->setupVariablesOnceAllAreRequested();
    // Then try to get the meter handle
    int hFacilityElectricity = getMeterHandle((void *)this->state, "Electricity:Facility");
    EXPECT_GT(hFacilityElectricity, -1);
    // now try to get handles to meters that doesn't exist
    int hDummyMeter = getMeterHandle((void *)this->state, "EnergySomething");
    EXPECT_EQ(-1, hDummyMeter);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetMeterValues)
{
    this->preRequestRealVariable("Chiller Electric Energy", "Chiller 1", 3.14, true);
    this->setupVariablesOnceAllAreRequested();
    int hFacilityElectricity = getMeterHandle((void *)this->state, "Electricity:Facility");
    EXPECT_GT(hFacilityElectricity, -1);
    // pretend like E+ ran a time step
    this->simulateTimeStepAndReport();
    // get the value for a valid meter
    Real64 curFacilityElectricity = getMeterValue((void *)this->state, hFacilityElectricity);
    EXPECT_NEAR(3.14, curFacilityElectricity, 0.001);
    // TODO: Figure out how to get accrued meter value and test that here

    // test invalid handles
    getMeterValue((void *)this->state, -1);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    getMeterValue((void *)this->state, 5);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetRealActuatorHandles)
{
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 1", ActuatorType::REAL);
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 2", ActuatorType::REAL);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 1");
    EXPECT_GT(hActuator, -1);
    int hActuator2 = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 2");
    EXPECT_GT(hActuator2, -1);
    EXPECT_NE(hActuator, hActuator2);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetIntActuatorHandles)
{
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 1", ActuatorType::INTEGER);
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 2", ActuatorType::INTEGER);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 1");
    EXPECT_GT(hActuator, -1);
    int hActuator2 = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 2");
    EXPECT_GT(hActuator2, -1);
    EXPECT_NE(hActuator, hActuator2);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetBoolActuatorHandles)
{
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 1", ActuatorType::BOOL);
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 2", ActuatorType::BOOL);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 1");
    EXPECT_GT(hActuator, -1);
    int hActuator2 = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 2");
    EXPECT_GT(hActuator2, -1);
    EXPECT_NE(hActuator, hActuator2);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetMixedActuatorHandles)
{
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 1", ActuatorType::BOOL);
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 2", ActuatorType::INTEGER);
    this->preRequestActuator("Chiller", "Max Flow", "Chiller 3", ActuatorType::REAL);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 1");
    EXPECT_GT(hActuator, -1);
    int hActuator2 = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 2");
    EXPECT_GT(hActuator2, -1);
    int hActuator3 = getActuatorHandle((void *)this->state, "Chiller", "Max Flow", "Chiller 3");
    EXPECT_GT(hActuator2, -1);
    // cross check
    EXPECT_NE(hActuator, hActuator2);
    EXPECT_NE(hActuator, hActuator3);
    EXPECT_NE(hActuator2, hActuator3);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetBadActuatorHandles)
{
    this->preRequestActuator("Chiller:Electric", "Max Flow Rate", "Chiller 1", ActuatorType::REAL);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator = getActuatorHandle((void *)this->state, "Chiller:Electric", "Max Flow Rate", "Chiller 1");
    EXPECT_GT(hActuator, -1);
    // now try to get handles to invalid actuators
    {
        int hActuatorBad = getActuatorHandle((void *)this->state, "Chiller:Electric", "Max Flow Rate", "InvalidInstance");
        EXPECT_EQ(hActuatorBad, -1);
    }
    {
        int hActuatorBad = getActuatorHandle((void *)this->state, "Chiller:Electric", "InvalidVar", "Chiller 1");
        EXPECT_EQ(hActuatorBad, -1);
    }
    {
        int hActuatorBad = getActuatorHandle((void *)this->state, "InvalidType", "Max Flow Rate", "Chiller 1");
        EXPECT_EQ(hActuatorBad, -1);
    }
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetAndSetRealActuators)
{
    // add two actuators, just to differentiate
    this->preRequestActuator("a", "b", "c", ActuatorType::REAL);
    this->preRequestActuator("d", "e", "f", ActuatorType::REAL);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator1 = getActuatorHandle((void *)this->state, "a", "b", "c");
    int hActuator2 = getActuatorHandle((void *)this->state, "d", "e", "f");
    // just for good measure here
    EXPECT_GT(hActuator1, -1);
    EXPECT_GT(hActuator2, -1);
    // now let's set the values of the actuators
    setActuatorValue((void *)this->state, hActuator1, 3.14);
    setActuatorValue((void *)this->state, hActuator2, 6.28);
    // now make sure we don't get them mixed up
    Real64 val1 = getActuatorValue((void *)this->state, hActuator1);
    Real64 val2 = getActuatorValue((void *)this->state, hActuator2);
    EXPECT_DOUBLE_EQ(3.14, val1);
    EXPECT_DOUBLE_EQ(6.28, val2);

    // invalid handles
    getActuatorValue((void *)this->state, -1);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    getActuatorValue((void *)this->state, 3);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetAndSetIntActuators)
{
    // add two actuators, just to differentiate
    this->preRequestActuator("a", "b", "c", ActuatorType::INTEGER);
    this->preRequestActuator("d", "e", "f", ActuatorType::INTEGER);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator1 = getActuatorHandle((void *)this->state, "a", "b", "c");
    int hActuator2 = getActuatorHandle((void *)this->state, "d", "e", "f");
    // just for good measure here
    EXPECT_GT(hActuator1, -1);
    EXPECT_GT(hActuator2, -1);
    // now let's set the values of the actuators
    setActuatorValue((void *)this->state, hActuator1, 3);
    setActuatorValue((void *)this->state, hActuator2, -6.1); // should get rounded
    // now make sure we don't get them mixed up
    Real64 val1 = getActuatorValue((void *)this->state, hActuator1);
    Real64 val2 = getActuatorValue((void *)this->state, hActuator2);
    EXPECT_DOUBLE_EQ(3, val1);
    EXPECT_DOUBLE_EQ(-6, val2);

    // invalid handles
    getActuatorValue((void *)this->state, -1);
    // but the flag should be set
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    getActuatorValue((void *)this->state, 3);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestGetAndSetBoolActuators)
{
    // add two actuators, just to differentiate
    this->preRequestActuator("a", "b", "c", ActuatorType::BOOL);
    this->preRequestActuator("d", "e", "f", ActuatorType::BOOL);
    this->setupActuatorsOnceAllAreRequested();
    // Then try to get the actuator handle
    int hActuator1 = getActuatorHandle((void *)this->state, "a", "b", "c");
    int hActuator2 = getActuatorHandle((void *)this->state, "d", "e", "f");
    // just for good measure here
    EXPECT_GT(hActuator1, -1);
    EXPECT_GT(hActuator2, -1);
    // now let's set the values of the actuators
    setActuatorValue((void *)this->state, hActuator1, 0); // false
    setActuatorValue((void *)this->state, hActuator2, 1); // true
    // now make sure we don't get them mixed up
    Real64 val1 = getActuatorValue((void *)this->state, hActuator1);
    Real64 val2 = getActuatorValue((void *)this->state, hActuator2);
    EXPECT_DOUBLE_EQ(0, val1);
    EXPECT_DOUBLE_EQ(1, val2);

    // invalid handles
    getActuatorValue((void *)this->state, -1);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    getActuatorValue((void *)this->state, 3);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestResetActuators)
{
    // we can't really test that the actuator is being recalculated by E+ again, but we can make sure the call works anyway
    this->preRequestActuator("a", "b", "c", ActuatorType::REAL);
    this->preRequestActuator("d", "e", "f", ActuatorType::INTEGER);
    this->preRequestActuator("g", "h", "i", ActuatorType::BOOL);
    this->setupActuatorsOnceAllAreRequested();
    int hActuator1 = getActuatorHandle((void *)this->state, "a", "b", "c");
    int hActuator2 = getActuatorHandle((void *)this->state, "d", "e", "f");
    int hActuator3 = getActuatorHandle((void *)this->state, "g", "h", "i");
    resetActuator((void *)this->state, hActuator1);
    resetActuator((void *)this->state, hActuator2);
    resetActuator((void *)this->state, hActuator3);

    // invalid handles
    resetActuator((void *)this->state, -1);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    resetActuator((void *)this->state, 8);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestAccessingInternalVariables)
{
    // we can't really test that the actuator is being recalculated by E+ again, but we can make sure the call works anyway
    this->preRequestInternalVariable("a", "b", 1.0);
    this->preRequestInternalVariable("c", "d", 2.0);
    this->setupInternalVariablesOnceAllAreRequested();
    int hIntVar1 = getInternalVariableHandle((void *)this->state, "a", "b");
    int hIntVar2 = getInternalVariableHandle((void *)this->state, "c", "d");
    EXPECT_GT(hIntVar1, -1);
    EXPECT_GT(hIntVar2, -1);
    Real64 val1 = getInternalVariableValue((void *)this->state, hIntVar1);
    Real64 val2 = getInternalVariableValue((void *)this->state, hIntVar2);
    EXPECT_DOUBLE_EQ(1.0, val1);
    EXPECT_DOUBLE_EQ(2.0, val2);

    // invalid handles
    getInternalVariableValue((void *)this->state, -1);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
    resetErrorFlag((void *)this->state);
    getInternalVariableValue((void *)this->state, 3);
    EXPECT_EQ(1, apiErrorFlag((void *)this->state));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_TestMiscSimData)
{
    // there are a number of Data Exchange functions that return meaningful simulation parameters -- year, hour, etc.
    // we don't really need to set up the simulation to make sure they are returning the exact right values, but we
    // can certainly set up a unit test to call into them and make sure nothing goes wrong.  This will essentially be
    // just stabilizing the API itself.

    // so make calls into these functions, don't worry about testing the individual values, if something throws then this unit test will fail
    year((void *)this->state);
    month((void *)this->state);
    dayOfMonth((void *)this->state);
    dayOfWeek((void *)this->state);
    dayOfYear((void *)this->state);
    daylightSavingsTimeIndicator((void *)this->state);
    hour((void *)this->state);
    currentTime((void *)this->state);
    minutes((void *)this->state);
    systemTimeStep((void *)this->state);
    holidayIndex((void *)this->state);
    sunIsUp((void *)this->state);
    isRaining((void *)this->state);
    warmupFlag((void *)this->state);
    kindOfSim((void *)this->state);
    currentEnvironmentNum((void *)this->state);
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_Python_EMS_Override)
{
    // Test for #8084, should warn when getting a handle for an actuator that is actually already defined in the IDF

    std::string const idf_objects = delimited_string({

        "OutdoorAir:Node, Test node;",

        "EnergyManagementSystem:Actuator,",
        "TempSetpointLo,          !- Name",
        "Test node,  !- Actuated Component Unique Name",
        "System Node Setpoint,    !- Actuated Component Type",
        "Temperature Minimum Setpoint;    !- Actuated Component Control Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    OutAirNodeManager::SetOutAirNodes(*state);
    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;
    bool anyRan;
    // Calls SetupNodeSetpointsAsActuator (via InitEMS, which calls GetEMSInput too)
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan);
    EXPECT_GT(state->dataRuntimeLang->numEMSActuatorsAvailable, 0);
    EXPECT_EQ(1, state->dataRuntimeLang->numActuatorsUsed);

    // no error message until now
    EXPECT_TRUE(compare_err_stream("", true));

    // Then try to get the actuator handle
    int hActuator = getActuatorHandle(state, "System Node Setpoint", "Temperature Minimum Setpoint", "Test node");
    EXPECT_GT(hActuator, -1);

    // Both the EMS one and the Plugin one point to the same handle, which is the index into the DataRuntimeLanguage::EMSActuatorAvailable array
    EXPECT_EQ(state->dataRuntimeLang->EMSActuatorUsed(1).ActuatorVariableNum, hActuator);

    std::string const expectedError = delimited_string({
        "   ** Warning ** Data Exchange API: An EnergyManagementSystem:Actuator seems to be already defined in the EnergyPlus File and named "
        "'TEMPSETPOINTLO'.",
        "   **   ~~~   ** Occurred for componentType='SYSTEM NODE SETPOINT', controlType='TEMPERATURE MINIMUM SETPOINT', uniqueKey='TEST NODE'.",
        "   **   ~~~   ** The getActuatorHandle function will still return the handle (= 2) but caller should take note that there is a risk of "
        "overwriting.",
    });

    EXPECT_TRUE(compare_err_stream(expectedError, true));
}

TEST_F(DataExchangeAPIUnitTestFixture, DataTransfer_Python_Python_Override)
{
    // Test for #8084, should warn when getting a handle for an actuator via API Twice

    std::string const idf_objects = delimited_string({
        "OutdoorAir:Node, Test node;",

        "PythonPlugin:Instance,",
        "  Vav2Mixedairmanagers,",
        "  Yes,",
        "  PythonPluginDemandManager_LargeOffice,",
        "  Vav2Mixedairmanagers;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    OutAirNodeManager::SetOutAirNodes(*state);
    EMSManager::CheckIfAnyEMS(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;
    bool anyRan;
    // Calls SetupNodeSetpointsAsActuator (via InitEMS, which calls GetEMSInput too)
    EMSManager::ManageEMS(*state, EMSManager::EMSCallFrom::SetupSimulation, anyRan);
    EXPECT_GT(state->dataRuntimeLang->numEMSActuatorsAvailable, 0);
    EXPECT_EQ(0, state->dataRuntimeLang->numActuatorsUsed);

    // no error message until now
    EXPECT_TRUE(compare_err_stream("", true));

    // Then try to get the actuator handle
    int hActuator = getActuatorHandle(state, "System Node Setpoint", "Temperature Minimum Setpoint", "Test node");
    EXPECT_GT(hActuator, -1);

    // no error message until now
    EXPECT_TRUE(compare_err_stream("", true));

    // Then try to get the actuator handle a SECOND time
    int hActuator2 = getActuatorHandle(state, "System Node Setpoint", "Temperature Minimum Setpoint", "Test node");
    EXPECT_GT(hActuator2, -1);

    // Both Plugin ones point to the same handle, which is the index into the DataRuntimeLanguage::EMSActuatorAvailable array
    EXPECT_EQ(hActuator2, hActuator);

    std::string const expectedError = delimited_string({
        "   ** Warning ** Data Exchange API: You seem to already have tried to get an Actuator Handle on this one.",
        "   **   ~~~   ** Occurred for componentType='SYSTEM NODE SETPOINT', controlType='TEMPERATURE MINIMUM SETPOINT', uniqueKey='TEST NODE'.",
        "   **   ~~~   ** The getActuatorHandle function will still return the handle (= 2) but caller should take note that there is a risk of "
        "overwriting.",
    });

    EXPECT_TRUE(compare_err_stream(expectedError, true));
}
