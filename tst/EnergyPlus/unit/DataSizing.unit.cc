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

// EnergyPlus::DataSizing Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"
#include <nlohmann/json.hpp>

using namespace EnergyPlus;
using namespace nlohmann::literals;

TEST_F(EnergyPlusFixture, DataSizingTest_resetHVACSizingGlobals)
{
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->CurZoneEqNum = 1;
    bool FirstPass = true;

    // set up Data globals to non-initial state to test that they are reset appropriately
    state->dataSize->DataTotCapCurveIndex = 1;
    state->dataSize->DataPltSizCoolNum = 1;
    state->dataSize->DataPltSizHeatNum = 1;
    state->dataSize->DataWaterLoopNum = 1;
    state->dataSize->DataCoilNum = 1;
    state->dataSize->DataFanOp = HVAC::FanOp::Cycling;
    state->dataSize->DataCoilIsSuppHeater = true;
    state->dataSize->DataIsDXCoil = true;
    state->dataSize->DataAutosizable = false;
    state->dataSize->DataEMSOverrideON = true;
    state->dataSize->DataScalableSizingON = true;
    state->dataSize->DataScalableCapSizingON = true;
    state->dataSize->DataSysScalableFlowSizingON = true;
    state->dataSize->DataSysScalableCapSizingON = true;
    state->dataSize->DataDesInletWaterTemp = 1.0;
    state->dataSize->DataDesInletAirHumRat = 1.0;
    state->dataSize->DataDesInletAirTemp = 1.0;
    state->dataSize->DataDesOutletAirTemp = 1.0;
    state->dataSize->DataDesOutletAirHumRat = 1.0;
    state->dataSize->DataCoolCoilCap = 1.0;
    state->dataSize->DataFlowUsedForSizing = 1.0;
    state->dataSize->DataAirFlowUsedForSizing = 1.0;
    state->dataSize->DataWaterFlowUsedForSizing = 1.0;
    state->dataSize->DataCapacityUsedForSizing = 1.0;
    state->dataSize->DataDesignCoilCapacity = 1.0;
    state->dataSize->DataHeatSizeRatio = 2.0;
    state->dataSize->DataEMSOverride = 1.0;
    state->dataSize->DataBypassFrac = 1.0;
    state->dataSize->DataFracOfAutosizedCoolingAirflow = 2.0;
    state->dataSize->DataFracOfAutosizedHeatingAirflow = 2.0;
    state->dataSize->DataFlowPerCoolingCapacity = 1.0;
    state->dataSize->DataFlowPerHeatingCapacity = 1.0;
    state->dataSize->DataFracOfAutosizedCoolingCapacity = 2.0;
    state->dataSize->DataFracOfAutosizedHeatingCapacity = 2.0;
    state->dataSize->DataAutosizedCoolingCapacity = 1.0;
    state->dataSize->DataAutosizedHeatingCapacity = 1.0;
    state->dataSize->DataConstantUsedForSizing = 1.0;
    state->dataSize->DataFractionUsedForSizing = 1.0;
    state->dataSize->DataNonZoneNonAirloopValue = 1.0;
    state->dataSize->DataZoneNumber = 1;
    state->dataSize->DataFanType = HVAC::FanType::Constant;
    state->dataSize->DataFanIndex = 0;
    state->dataSize->DataWaterCoilSizCoolDeltaT = 1.0;
    state->dataSize->DataWaterCoilSizHeatDeltaT = 1.0;
    state->dataSize->DataNomCapInpMeth = true;

    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirFlow = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingAirFlow = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SystemAirFlow = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).Capacity = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingCapacity = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingCapacity = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).MaxHWVolFlow = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).MaxCWVolFlow = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesCoolingLoad = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesHeatingLoad = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingAirVolFlow = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow = 1.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SystemAirVolFlow = 1.0;

    // test a few to ensure not equal to initial state
    EXPECT_NE(state->dataSize->DataTotCapCurveIndex, 0);
    EXPECT_NE(state->dataSize->DataDesInletWaterTemp, 0.0);
    EXPECT_NE(state->dataSize->DataHeatSizeRatio, 1.0);
    EXPECT_NE((int)state->dataSize->DataFanType, (int)HVAC::FanType::Invalid);
    EXPECT_TRUE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirFlow);
    EXPECT_FALSE(state->dataSize->DataAutosizable);

    // function argument initialized to true at beginning of simulation
    EXPECT_TRUE(FirstPass);

    // call reset function
    DataSizing::resetHVACSizingGlobals(*state, state->dataSize->CurZoneEqNum, 0, FirstPass);

    // expect function argument to be false after calling resetHVACSizingGlobals
    EXPECT_FALSE(FirstPass);

    // expect these to be reset to the initial state as defined in DataSizing.cc and DataSizing.hh
    EXPECT_EQ(state->dataSize->DataTotCapCurveIndex, 0);
    EXPECT_EQ(state->dataSize->DataPltSizCoolNum, 0);
    EXPECT_EQ(state->dataSize->DataPltSizHeatNum, 0);
    EXPECT_EQ(state->dataSize->DataWaterLoopNum, 0);
    EXPECT_EQ(state->dataSize->DataCoilNum, 0);
    EXPECT_EQ((int)state->dataSize->DataFanOp, (int)HVAC::FanOp::Invalid);
    EXPECT_FALSE(state->dataSize->DataCoilIsSuppHeater);
    EXPECT_FALSE(state->dataSize->DataIsDXCoil);
    EXPECT_TRUE(state->dataSize->DataAutosizable);
    EXPECT_FALSE(state->dataSize->DataEMSOverrideON);
    EXPECT_FALSE(state->dataSize->DataScalableSizingON);
    EXPECT_FALSE(state->dataSize->DataScalableCapSizingON);
    EXPECT_FALSE(state->dataSize->DataSysScalableFlowSizingON);
    EXPECT_FALSE(state->dataSize->DataSysScalableCapSizingON);
    EXPECT_EQ(state->dataSize->DataDesInletWaterTemp, 0.0);
    EXPECT_EQ(state->dataSize->DataDesInletAirHumRat, 0.0);
    EXPECT_EQ(state->dataSize->DataDesInletAirTemp, 0.0);
    EXPECT_EQ(state->dataSize->DataDesOutletAirTemp, 0.0);
    EXPECT_EQ(state->dataSize->DataDesOutletAirHumRat, 0.0);
    EXPECT_EQ(state->dataSize->DataCoolCoilCap, 0.0);
    EXPECT_EQ(state->dataSize->DataFlowUsedForSizing, 0.0);
    EXPECT_EQ(state->dataSize->DataAirFlowUsedForSizing, 0.0);
    EXPECT_EQ(state->dataSize->DataWaterFlowUsedForSizing, 0.0);
    EXPECT_EQ(state->dataSize->DataCapacityUsedForSizing, 0.0);
    EXPECT_EQ(state->dataSize->DataDesignCoilCapacity, 0.0);
    EXPECT_EQ(state->dataSize->DataHeatSizeRatio, 1.0);
    EXPECT_EQ(state->dataSize->DataEMSOverride, 0.0);
    EXPECT_EQ(state->dataSize->DataBypassFrac, 0.0);
    EXPECT_EQ(state->dataSize->DataFracOfAutosizedCoolingAirflow, 1.0);
    EXPECT_EQ(state->dataSize->DataFracOfAutosizedHeatingAirflow, 1.0);
    EXPECT_EQ(state->dataSize->DataFlowPerCoolingCapacity, 0.0);
    EXPECT_EQ(state->dataSize->DataFlowPerHeatingCapacity, 0.0);
    EXPECT_EQ(state->dataSize->DataFracOfAutosizedCoolingCapacity, 1.0);
    EXPECT_EQ(state->dataSize->DataFracOfAutosizedHeatingCapacity, 1.0);
    EXPECT_EQ(state->dataSize->DataAutosizedCoolingCapacity, 0.0);
    EXPECT_EQ(state->dataSize->DataAutosizedHeatingCapacity, 0.0);
    EXPECT_EQ(state->dataSize->DataConstantUsedForSizing, 0.0);
    EXPECT_EQ(state->dataSize->DataFractionUsedForSizing, 0.0);
    EXPECT_EQ(state->dataSize->DataNonZoneNonAirloopValue, 0.0);
    EXPECT_EQ(state->dataSize->DataZoneNumber, 0);
    EXPECT_EQ((int)state->dataSize->DataFanType, (int)HVAC::FanType::Invalid);
    EXPECT_EQ(state->dataSize->DataFanIndex, 0);
    EXPECT_EQ(state->dataSize->DataWaterCoilSizCoolDeltaT, 0.0);
    EXPECT_EQ(state->dataSize->DataWaterCoilSizHeatDeltaT, 0.0);
    EXPECT_FALSE(state->dataSize->DataNomCapInpMeth);

    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirFlow);
    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingAirFlow);
    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirFlow);
    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SystemAirFlow);
    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).Capacity);
    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingCapacity);
    EXPECT_FALSE(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingCapacity);

    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).MaxHWVolFlow, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).MaxCWVolFlow, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesCoolingLoad, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesHeatingLoad, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingAirVolFlow, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow, 0.0);
    EXPECT_EQ(state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SystemAirVolFlow, 0.0);

    // Test clean return if CurZoneEqNum = 0
    FirstPass = true;
    state->dataSize->CurZoneEqNum = 0;
    DataSizing::resetHVACSizingGlobals(*state, state->dataSize->CurZoneEqNum, 0, FirstPass);
    EXPECT_FALSE(FirstPass);

    // Test clean return if ZoneEqSizing is not allocated
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataSize->CurZoneEqNum = 1;
    FirstPass = true;
    // call reset function
    DataSizing::resetHVACSizingGlobals(*state, state->dataSize->CurZoneEqNum, 0, FirstPass);
    EXPECT_FALSE(FirstPass);
}

TEST_F(EnergyPlusFixture, OARequirements_calcDesignSpecificationOutdoorAir)
{
    // Test OARequirements.calcDesignSpecificationOutdoorAir()

    // GetZoneData uses GetObjectItem with ipshortcuts so these need to be allocated
    int NumAlphas = 4;
    int NumNumbers = 9;
    state->dataIPShortCut->lNumericFieldBlanks.allocate(NumNumbers);
    state->dataIPShortCut->lAlphaFieldBlanks.allocate(NumAlphas);
    state->dataIPShortCut->cAlphaFieldNames.allocate(NumAlphas);
    state->dataIPShortCut->cNumericFieldNames.allocate(NumNumbers);
    state->dataIPShortCut->cAlphaArgs.allocate(NumAlphas);
    state->dataIPShortCut->rNumericArgs.allocate(NumNumbers);
    state->dataIPShortCut->lNumericFieldBlanks = false;
    state->dataIPShortCut->lAlphaFieldBlanks = false;
    state->dataIPShortCut->cAlphaFieldNames = " ";
    state->dataIPShortCut->cNumericFieldNames = " ";
    state->dataIPShortCut->cAlphaArgs = " ";
    state->dataIPShortCut->rNumericArgs = 0.0;

    // Original method of initializing epJSON input objects, but couldn't get this to work for extensible arrays
    // state->dataInputProcessing->inputProcessor->epJSON["Zone"]["Zone 1"] = {};
    // state->dataInputProcessing->inputProcessor->epJSON["Zone"]["Zone 2"] = {};
    // state->dataInputProcessing->inputProcessor->epJSON["Space"]["Space 1a"] = {{"zone_name", "Zone 1"}};

    // Using R_json raw string parsing to get the arrays processed correctly
    // Reference https://github.com/nlohmann/json#json-as-first-class-data-type
    state->dataInputProcessing->inputProcessor->epJSON = R"(
    {
        "Zone": {
            "Zone 1" : {
                 "volume": 1200.0,
                 "floor_area": 400.0
            },
            "Zone 2" : {
                 "volume": 1200.0,
                 "floor_area": 400.0
            }
        },
        "Space": {
            "Space 1a" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0
            },
            "Space 1b" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0
            },
            "Space 1c" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0
            },
            "Space 1d" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0,
                 "volume": 300.0
            }
        },
        "DesignSpecification:OutdoorAir": {
            "DSOA Per Person": {
                "outdoor_air_flow_per_person": 0.0125,
                "outdoor_air_method": "Flow/Person"
            },
            "DSOA Per Area": {
                "outdoor_air_flow_per_zone_floor_area": 0.001,
                "outdoor_air_method": "Flow/Area"
            },
            "DSOA Per Zone": {
                "outdoor_air_flow_per_zone": 1.0,
                "outdoor_air_method": "Flow/Zone"
            },
            "DSOA ACH": {
                "outdoor_air_flow_air_changes_per_hour": 0.1,
                "outdoor_air_method": "AirChanges/Hour"
            },
            "DSOA Sum": {
                "outdoor_air_method": "Sum",
                "outdoor_air_flow_per_person": 0.0125,
                "outdoor_air_flow_per_zone_floor_area": 0.00025,
                "outdoor_air_flow_per_zone": 1.0,
                "outdoor_air_flow_air_changes_per_hour": 0.025
            }
        },
        "DesignSpecification:OutdoorAir:SpaceList": {
            "DSOA Zone 1 Spaces" : {
                 "space_specs": [
                    {
                        "space_name": "Space 1a",
                        "space_design_specification_outdoor_air_object_name": "DSOA Per Person"
                    },
                    {
                        "space_name": "Space 1b",
                        "space_design_specification_outdoor_air_object_name": "DSOA Per Area"
                    },
                    {
                        "space_name": "Space 1c",
                        "space_design_specification_outdoor_air_object_name": "DSOA Per Zone"
                    },
                    {
                        "space_name": "Space 1d",
                        "space_design_specification_outdoor_air_object_name": "DSOA ACH"
                    }
                ]
            }
        }
    }
    )"_json;

    state->dataGlobal->isEpJSON = true;
    state->dataInputProcessing->inputProcessor->initializeMaps();

    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);

    std::string const error_string = delimited_string(
        {EnergyPlus::format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
         "   ** Warning ** No Timestep object found.  Number of TimeSteps in Hour defaulted to 4."});

    EXPECT_TRUE(compare_err_stream(error_string, true));
    EXPECT_FALSE(ErrorsFound);

    int zoneNum = 1;
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(4, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[1]).Name);
    EXPECT_EQ("SPACE 1C", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[2]).Name);
    EXPECT_EQ("SPACE 1D", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[3]).Name);
    zoneNum = 2;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(1, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);

    SizingManager::GetOARequirements(*state);
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->numSpaces);
    ZoneEquipmentManager::SetUpZoneSizingArrays(*state);
    compare_err_stream("");

    std::string thisOAReqName = "DSOA ZONE 1 SPACES";
    int oaNum = Util::FindItemInList(thisOAReqName, state->dataSize->OARequirements);
    EXPECT_TRUE(oaNum > 0);
    EXPECT_EQ(4, state->dataSize->OARequirements(oaNum).dsoaIndexes.size());

    // Set zone and space occupants
    state->dataHeatBal->spaceIntGain.allocate(state->dataGlobal->numSpaces);
    state->dataHeatBal->ZoneIntGain.allocate(state->dataGlobal->NumOfZones);
    std::string thisSpaceName = "SPACE 1A";
    int spaceNum = Util::FindItemInList(thisSpaceName, state->dataHeatBal->space);
    state->dataHeatBal->space(spaceNum).FloorArea = 100.0;
    state->dataHeatBal->space(spaceNum).TotOccupants = 10;
    state->dataHeatBal->spaceIntGain(spaceNum).NOFOCC = 1;
    state->dataHeatBal->space(spaceNum).maxOccupants = 12;

    thisSpaceName = "SPACE 1B";
    spaceNum = Util::FindItemInList(thisSpaceName, state->dataHeatBal->space);
    state->dataHeatBal->space(spaceNum).FloorArea = 100.0;

    thisSpaceName = "SPACE 1C";
    spaceNum = Util::FindItemInList(thisSpaceName, state->dataHeatBal->space);
    state->dataHeatBal->space(spaceNum).FloorArea = 100.0;

    thisSpaceName = "SPACE 1D";
    spaceNum = Util::FindItemInList(thisSpaceName, state->dataHeatBal->space);
    state->dataHeatBal->space(spaceNum).FloorArea = 100.0;
    state->dataHeatBal->space(spaceNum).Volume = 300.0;

    std::string thisZoneName = "ZONE 2";
    zoneNum = Util::FindItemInList(thisZoneName, state->dataHeatBal->Zone);
    state->dataHeatBal->Zone(zoneNum).FloorArea = 400.0;
    state->dataHeatBal->Zone(zoneNum).TotOccupants = 10;
    state->dataHeatBal->ZoneIntGain(zoneNum).NOFOCC = 1;
    state->dataHeatBal->Zone(zoneNum).maxOccupants = 12;

    bool UseOccSchFlag = false;
    bool UseMinOASchFlag = false;

    thisZoneName = "ZONE 1";
    zoneNum = Util::FindItemInList(thisZoneName, state->dataHeatBal->Zone);
    state->dataHeatBal->Zone(zoneNum).FloorArea = 400.0;
    thisOAReqName = "DSOA ZONE 1 SPACES";
    oaNum = Util::FindItemInList(thisOAReqName, state->dataSize->OARequirements);
    Real64 zone1OA = DataSizing::calcDesignSpecificationOutdoorAir(*state, oaNum, zoneNum, UseOccSchFlag, UseMinOASchFlag);
    // 0.0125/person * 10.0 + 0.001/area * 100.0 + 1.0/zone + 0.1ACH * 300.0 / 3600.0;
    Real64 expectedOA = 0.0125 * 10.0 + 0.001 * 100.0 + 1.0 + 0.1 * 300.0 / 3600.0;
    EXPECT_EQ(expectedOA, zone1OA);

    thisZoneName = "ZONE 2";
    zoneNum = Util::FindItemInList(thisZoneName, state->dataHeatBal->Zone);
    state->dataHeatBal->Zone(zoneNum).FloorArea = 400.0;
    thisOAReqName = "DSOA SUM";
    oaNum = Util::FindItemInList(thisOAReqName, state->dataSize->OARequirements);
    Real64 zone2OA = DataSizing::calcDesignSpecificationOutdoorAir(*state, oaNum, zoneNum, UseOccSchFlag, UseMinOASchFlag);
    EXPECT_EQ(expectedOA, zone2OA);
}

TEST_F(EnergyPlusFixture, GetCoilDesFlowT_Test)
{
    state->init_state(*state);
    state->dataEnvrn->StdRhoAir = 1.1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->SysSizPeakDDNum.allocate(1);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtTotCoolPk.allocate(2);
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtSensCoolPk.allocate(2);
    state->dataSize->CalcSysSizing.allocate(1);
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq.allocate(2);
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq.allocate(2);
    state->dataSize->SysSizInput(1).AirPriLoopName = "MyAirloop";
    state->dataSize->FinalSysSizing(1).AirPriLoopName = "MyAirloop";
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 12.3;
    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.008;
    state->dataSize->FinalSysSizing(1).DesCoolVolFlow = 1.0;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesCoolVolFlow * state->dataEnvrn->StdRhoAir;
    state->dataSize->SysSizPeakDDNum(1).TotCoolPeakDD = 1;
    state->dataSize->SysSizPeakDDNum(1).SensCoolPeakDD = 1;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtTotCoolPk(1) = 1;
    state->dataSize->SysSizPeakDDNum(1).TimeStepAtSensCoolPk(1) = 2;
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq(1) = 1000.0;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(1) = 24.0;
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq(2) = 500.0;
    state->dataSize->CalcSysSizing(1).CoolZoneAvgTempSeq(2) = 22.0;
    state->dataSize->CalcSysSizing(1).MixTempAtCoolPeak = 30.0;
    state->dataSize->DataAirFlowUsedForSizing = 0.5;

    int curSysNum = 1;
    Real64 CpAirStd = 1.1;
    Real64 DesCoilAirFlow = 0.0;
    Real64 DesCoilExitTemp = 0.0;
    Real64 DesCoilExitHumRat = 0.0;

    // design data are used
    state->dataSize->SysSizInput(1).coolingPeakLoad = DataSizing::PeakLoad::TotalCooling;
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak / state->dataEnvrn->StdRhoAir, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, state->dataSize->FinalSysSizing(1).CoolSupHumRat, 0.00001);

    // design data and DataAirFlowUsedForSizing are used
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::OnOff;
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, state->dataSize->DataAirFlowUsedForSizing, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, state->dataSize->FinalSysSizing(1).CoolSupHumRat, 0.00001);

    // at high loads DesCoilExitTemp equals design
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VT;
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, state->dataSize->FinalSysSizing(1).DesCoolVolFlow, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, 0.008005, 0.00001);

    // at low loads DesCoilExitTemp is higher than design
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq(1) = 10.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, state->dataSize->FinalSysSizing(1).DesCoolVolFlow, 0.00001);
    EXPECT_GT(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp);
    EXPECT_NEAR(DesCoilExitTemp, 15.735537, 0.00001);
    EXPECT_GT(DesCoilExitHumRat, state->dataSize->FinalSysSizing(1).CoolSupHumRat);
    EXPECT_NEAR(DesCoilExitHumRat, 0.01003, 0.00001);

    // coil air flow and humrat are calculated
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::Bypass;
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, 0.805901, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, 0.008005, 0.00001);

    // design data used, humrat is calculated.
    // VT and Bypass used sequenced cooling load and zone cooling temp using DD timestep
    state->dataSize->SysSizInput(1).coolingPeakLoad = DataSizing::PeakLoad::SensibleCooling;
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VT;
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, state->dataSize->FinalSysSizing(1).DesCoolVolFlow, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, 0.008005, 0.00001);

    // design data used, humrat is calculated
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::Bypass;
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, state->dataSize->FinalSysSizing(1).DesCoolVolFlow, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, 0.008005, 0.00001);

    // DesCoilAirFlow will be calculated
    state->dataSize->CalcSysSizing(1).SumZoneCoolLoadSeq(2) = 5.0;
    DesCoilAirFlow = 0.0;
    DesCoilExitTemp = 0.0;
    DesCoilExitHumRat = 0.0;
    DataSizing::GetCoilDesFlowT(*state, curSysNum, CpAirStd, DesCoilAirFlow, DesCoilExitTemp, DesCoilExitHumRat);
    EXPECT_NEAR(DesCoilAirFlow, 0.685436, 0.00001);
    EXPECT_NEAR(DesCoilExitTemp, state->dataSize->FinalSysSizing(1).CoolSupTemp, 0.00001);
    EXPECT_NEAR(DesCoilExitHumRat, 0.008005, 0.00001);
}
