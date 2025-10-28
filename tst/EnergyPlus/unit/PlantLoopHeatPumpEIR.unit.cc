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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma ide diagnostic ignored "cert-err58-cpp"
#pragma ide diagnostic ignored "modernize-use-equals-delete"

#include <stdexcept>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantLoopHeatPumpEIR.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/WeatherManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::EIRPlantLoopHeatPumps;

TEST_F(EnergyPlusFixture, ConstructionFullObjectsHeatingAndCooling_WaterSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  2,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  2,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the heating side
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRHeating, thisHeatingPLHP->EIRHPType);
    EXPECT_EQ(thisCoolingPLHP, thisHeatingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisHeatingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisHeatingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisHeatingPLHP->powerRatioFuncPLRCurveIndex);

    // validate the cooling side
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRCooling, thisCoolingPLHP->EIRHPType);
    EXPECT_EQ(thisHeatingPLHP, thisCoolingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisCoolingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncPLRCurveIndex);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP HEATING SIDE"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP COOLING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, PairingCompanionCoils)
{
    state->dataEIRPlantLoopHeatPump->heatPumps.resize(2);
    EIRPlantLoopHeatPump *coil1 = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *coil2 = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    {
        // a successful try
        coil1->name = "name1";
        coil1->companionCoilName = "name2";
        coil1->EIRHPType = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
        coil1->companionHeatPumpCoil = nullptr;
        coil2->name = "name2";
        coil2->companionCoilName = "name1";
        coil2->EIRHPType = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
        coil2->companionHeatPumpCoil = nullptr;
        EIRPlantLoopHeatPumps::EIRPlantLoopHeatPump::pairUpCompanionCoils(*state);
        EXPECT_EQ(coil2, coil1->companionHeatPumpCoil);
        EXPECT_EQ(coil1, coil2->companionHeatPumpCoil);
    }

    {
        // but what if we can't find a companion!
        coil1->name = "name1";
        coil1->companionCoilName = "name6";
        coil1->EIRHPType = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
        coil1->companionHeatPumpCoil = nullptr;
        coil2->name = "name2";
        coil2->companionCoilName = "name1";
        coil2->EIRHPType = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
        coil2->companionHeatPumpCoil = nullptr;
        EXPECT_THROW(EIRPlantLoopHeatPumps::EIRPlantLoopHeatPump::pairUpCompanionCoils(*state), std::runtime_error);
    }

    {
        // or what if we find a companion but it's the same coil type
        coil1->name = "name1";
        coil1->companionCoilName = "name2";
        coil1->EIRHPType = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
        coil1->companionHeatPumpCoil = nullptr;
        coil2->name = "name2";
        coil2->companionCoilName = "name1";
        coil2->EIRHPType = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
        coil2->companionHeatPumpCoil = nullptr;
        EXPECT_THROW(EIRPlantLoopHeatPumps::EIRPlantLoopHeatPump::pairUpCompanionCoils(*state), std::runtime_error);
    }
}

TEST_F(EnergyPlusFixture, HeatingConstructionFullObjectsNoCompanion)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the heating side
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRHeating, thisHeatingPLHP->EIRHPType);
    EXPECT_EQ(nullptr, thisHeatingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisHeatingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisHeatingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisHeatingPLHP->powerRatioFuncPLRCurveIndex);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP HEATING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, CoolingConstructionFullObjectsNoCompanion)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the cooling side
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRCooling, thisCoolingPLHP->EIRHPType);
    EXPECT_EQ(nullptr, thisCoolingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisCoolingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncPLRCurveIndex);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP COOLING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, CoolingConstructionFullObjectWithDefaults)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  ,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the cooling side
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRCooling, thisCoolingPLHP->EIRHPType);
    EXPECT_NEAR(1, thisCoolingPLHP->sizingFactor, 0.001);
}

TEST_F(EnergyPlusFixture, CoolingConstructionFullyAutoSized_WaterSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the cooling side
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRCooling, thisCoolingPLHP->EIRHPType);
    EXPECT_EQ(nullptr, thisCoolingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisCoolingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncPLRCurveIndex);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP COOLING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, CatchErrorsOnBadCurves)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  1,",
                                                      "  dummyCurveA,",
                                                      "  dummyCurveB,",
                                                      "  dummyCurveC;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs, it should throw for the bad curves
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, HeatingSimulate_AirSource_AWHP)
{
    std::string const idf_objects = delimited_string({"HeatPump:AirToWater,",
                                                      "test_AWHP, !- Name",
                                                      ", !- Availability Schedule Name Heating",
                                                      ", !- Availability Schedule Name Cooling",
                                                      "Load , !- Operating Mode Control Method",
                                                      "SingleMode, !- Operating Mode Control Option for Multiple Unit",
                                                      ", !- Operating Mode Control Schedule Name",
                                                      "20 , !-  Rated Inlet Air Temperature in Heating Mode",
                                                      "1.0, !-  Rated Air Flow Rate in Heating Mode",
                                                      "50 , !-  Rated Leaving Water Temperature in Heating Mode",
                                                      "0.0001, !-  Rated Water Flow Rate in Heating Mode",
                                                      ", !-  Minimum Outdoor Air Temperature in Heating Mode",
                                                      ", !-  Maximum Outdoor Air Temperature in Heating Mode",
                                                      ",    !- Minimum Leaving Water Temperature Curve Name in Heating Mode",
                                                      ",    !- Maximum Leaving Water Temperature Curve Name in Heating Mode",
                                                      ",    !- Minimum Part Load Ratio",
                                                      "1.0, !-  Sizing Factor for Heating",
                                                      "25, !-  Rated Inlet Air Temperature in Cooling Mode",
                                                      "0.002, !-  Rated Air Flow Rate in Cooling Mode",
                                                      "22 , !-  Rated Leaving Water Temperature in Cooling Mode",
                                                      "0.005, !-  Rated Water Flow Rate in Cooling Mode",
                                                      ", !-  Minimum Outdoor Air Temperature in Cooling Mode",
                                                      ", !-  Maximum Outdoor Air Temperature in Cooling Mode",
                                                      ",    !- Minimum Leaving Water Temperature Curve Name in Cooling Mode",
                                                      ",    !- Maximum Leaving Water Temperature Curve Name in Cooling Mode",
                                                      "0.9, !-  Sizing Factor for Cooling",
                                                      "Outdoor Air Inlet Node , !-  Air Inlet Node Name",
                                                      "Outdoor Air Outlet Node, !-  Air Outlet Node Name",
                                                      "Heating Coil Load Loop Intermediate Node, !-  Hot Water Inlet Node Name",
                                                      "Heating Coil Load Loop Supply Outlet Node, !-  Hot Water Outlet Node Name",
                                                      "Cooling Coil Load Loop Intermediate Node , !-  Chilled Water Inlet Node Name",
                                                      "Cooling Coil Load Loop Supply Outlet Node , !-  Chilled Water Outlet Node Name",
                                                      ", !- Maximum Outdoor Dry Bulb Temperature For Defrost Operation",
                                                      ", !-  Heat Pump Defrost Control",
                                                      ", !-  Defrost Time Period Fraction",
                                                      ", !-  Resistive Defrost Heater Capacity",
                                                      ", !-  Defrost Energy Input Ratio Function of Temperature Curve Name",
                                                      "1 , !- Compressor Multiplier",
                                                      "FixedSpeed , !- Control Type",
                                                      "100 , !- Crankcase Heater Capacity",
                                                      "EIRCurveFuncPLR, !- Crankcase Heater Capacity Function of Temperature Curve Name",
                                                      "20 , !- Maximum Ambient Temperature for Crankcase Heater Operation",
                                                      "1 , !- Number of Speeds for Heating",
                                                      "1000, !-  Rated Heating Capacity at Speed 1",
                                                      "3.14, !-  Rated COP for Heating at Speed 1",
                                                      "dummyCurve, !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 1",
                                                      "dummyCurve, !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1",
                                                      "dummyCurve, !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 1",
                                                      ", !-  Rated Heating Capacity at Speed 2",
                                                      ", !-  Rated COP for Heating at Speed 2",
                                                      ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 2",
                                                      ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2",
                                                      ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 2",
                                                      ", !-  Rated Heating Capacity at Speed 3",
                                                      ", !-  Rated COP for Heating at Speed 3",
                                                      ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 3",
                                                      ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3",
                                                      ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 3",
                                                      ", !-  Rated Heating Capacity at Speed 4",
                                                      ", !-  Rated COP for Heating at Speed 4",
                                                      ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 4",
                                                      ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4",
                                                      ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 4",
                                                      ", !-  Rated Heating Capacity at Speed 5",
                                                      ", !-  Rated COP for Heating at Speed 5",
                                                      ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 5",
                                                      ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5",
                                                      ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 5",
                                                      ", !-  Booster Mode On Heating",
                                                      ", !-  Rated Heating Capacity in Booster Mode",
                                                      ", !-  Rated Heating COP in Booster Mode",
                                                      ", !-  Normalized Heating Capacity Function of Temperature Curve Name in Booster Mode",
                                                      ", !-  Heating Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
                                                      ", !-  Heating Energy Input Ratio Function of PLR Curve Name in Booster Mode",
                                                      "1, !-  Number of Speeds for Cooling",
                                                      "20000, !-  Rated Cooling Capacity at Speed 1",
                                                      "3, !-  Rated COP for Cooling at Speed 1",
                                                      "dummyCurve, !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1",
                                                      "dummyCurve, !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1",
                                                      "dummyCurve, !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1",
                                                      ", !-  Rated Cooling Capacity at Speed 2",
                                                      ", !-  Rated COP for Cooling at Speed 2",
                                                      ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2",
                                                      ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2",
                                                      ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2",
                                                      ", !-  Rated Cooling Capacity at Speed 3",
                                                      ", !-  Rated COP for Cooling at Speed 3",
                                                      ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3",
                                                      ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3",
                                                      ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3",
                                                      ", !-  Rated Cooling Capacity at Speed 4",
                                                      ", !-  Rated COP for Cooling at Speed 4",
                                                      ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4",
                                                      ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4",
                                                      ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4",
                                                      ", !-  Rated Cooling Capacity at Speed 5",
                                                      ", !-  Rated COP for Cooling at Speed 5",
                                                      ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5",
                                                      ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5",
                                                      ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5",
                                                      ", !-  Booster Mode On Cooling",
                                                      ", !-  Rated Cooling Capacity in Booster Mode",
                                                      ", !-  Rated Cooling COP in Booster Mode",
                                                      ", !-  Normalized Cooling Capacity Function of Temperature Curve Name in Booster Mode",
                                                      ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
                                                      "; !-  Cooling Energy Input Ratio Function of PLR Curve Name in Booster Mode",

                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideCompHeating = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideCompHeating.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating;
    PLHPPlantLoadSideCompHeating.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideCompCooling = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideCompCooling.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    PLHPPlantLoadSideCompCooling.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    HeatPumpAirToWater::factory(*state, DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating, "TEST_AWHP");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataHeatPumpAirToWater->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    HeatPumpAirToWater *thisAWHPHeating = &state->dataHeatPumpAirToWater->heatPumps[1];
    HeatPumpAirToWater *thisAWHPCooling = &state->dataHeatPumpAirToWater->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideCompHeating.Name = thisAWHPHeating->name;
    PLHPPlantLoadSideCompHeating.NodeNumIn = thisAWHPHeating->loadSideNodes.inlet;
    PLHPPlantLoadSideCompCooling.Name = thisAWHPCooling->name;
    PLHPPlantLoadSideCompCooling.NodeNumIn = thisAWHPCooling->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisAWHPHeating->onInitLoopEquip(*state, myLoadLocation);
    thisAWHPHeating->loadSidePlantLoc.loopNum = 1;
    thisAWHPHeating->loadSidePlantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    thisAWHPHeating->loadSidePlantLoc.branchNum = 1;
    thisAWHPHeating->loadSidePlantLoc.compNum = 1;
    thisAWHPCooling->loadSidePlantLoc.loopNum = 2;
    thisAWHPCooling->loadSidePlantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    thisAWHPCooling->loadSidePlantLoc.branchNum = 1;
    thisAWHPCooling->loadSidePlantLoc.compNum = 1;

    // call it from the load side, but this time there is a negative (cooling) load - shouldn't try to run
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = true; // plant actually shouldn't do this but the component can be smart enough to handle it
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisAWHPHeating->sourceSideNodes.inlet).Temp = 30;
        thisAWHPHeating->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(loadInletTemp, thisAWHPHeating->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.0, thisAWHPHeating->loadSideHeatTransfer, 0.001);
    }

    // call it from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisAWHPHeating->sourceSideNodes.inlet).Temp = 30;
        // if need to test defrost, uncomment this
        // state->dataEnvrn->OutBaroPress = 98934;
        thisAWHPHeating->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(specifiedLoadSetpoint, thisAWHPHeating->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisAWHPHeating->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 1200;
        Real64 availableCapacity = 950.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisAWHPHeating->sourceSideNodes.inlet).Temp = 30;
        thisAWHPHeating->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(44.402, thisAWHPHeating->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(availableCapacity, thisAWHPHeating->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is no load (still firsthvac)
    {
        bool firstHVAC = true;
        Real64 curLoad = 0.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisAWHPHeating->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisAWHPHeating->sourceSideNodes.inlet).Temp = 30;
        thisAWHPHeating->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(45.0, thisAWHPHeating->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(30.0, thisAWHPHeating->sourceSideOutletTemp, 0.001);
    }
}

TEST_F(EnergyPlusFixture, processInputForEIRPLHP_AWHP)
{
    std::string const idf_objects = delimited_string({
        "HeatPump:AirToWater,",
        "test_AWHP, !- Name",
        ", !- Availability Schedule Name Heating",
        ", !- Availability Schedule Name Cooling",
        "Load , !- Operating Mode Control Method",
        "SingleMode, !- Operating Mode Control Option for Multiple Unit",
        ", !- Operating Mode Control Schedule Name",
        "20 , !-  Rated Inlet Air Temperature in Heating Mode",
        "0.1 , !-  Rated Air Flow Rate in Heating Mode",
        "50 , !-  Rated Leaving Water Temperature in Heating Mode",
        "0.02 , !-  Rated Water Flow Rate in Heating Mode",
        "-20, !-  Minimum Outdoor Air Temperature in Heating Mode",
        "25, !-  Maximum Outdoor Air Temperature in Heating Mode",
        "MinLWTvsOAT,    !- Minimum Leaving Water Temperature Curve Name in Heating Mode",
        "MaxLWTvsOAT,    !- Maximum Leaving Water Temperature Curve Name in Heating Mode",
        ",    !- Minimum Part Load Ratio",
        "1.0, !-  Sizing Factor for Heating",
        "25, !-  Rated Inlet Air Temperature in Cooling Mode",
        "0.1 , !-  Rated Air Flow Rate in Cooling Mode",
        "22 , !-  Rated Leaving Water Temperature in Cooling Mode",
        "0.05 , !-  Rated Water Flow Rate in Cooling Mode",
        "18 , !-  Minimum Outdoor Air Temperature in Cooling Mode",
        "40, !-  Maximum Outdoor Air Temperature in Cooling Mode",
        "MinLWTvsOAT,    !- Minimum Leaving Water Temperature Curve Name in Cooling Mode",
        "MaxLWTvsOAT,    !- Maximum Leaving Water Temperature Curve Name in Cooling Mode",
        "0.9, !-  Sizing Factor for Cooling",
        "Outdoor Air Inlet Node , !-  Air Inlet Node Name",
        "Outdoor Air Outlet Node, !-  Air Outlet Node Name",
        "Heating Coil Load Loop Intermediate Node, !-  Hot Water Inlet Node Name",
        "Heating Coil Load Loop Supply Outlet Node, !-  Hot Water Outlet Node Name",
        "Cooling Coil Load Loop Intermediate Node , !-  Chilled Water Inlet Node Name",
        "Cooling Coil Load Loop Supply Outlet Node , !-  Chilled Water Outlet Node Name",
        "10, !- Maximum Outdoor Dry Bulb Temperature For Defrost Operation",
        "Timed, !-  Heat Pump Defrost Control",
        "0.2, !-  Defrost Time Period Fraction",
        "150, !-  Resistive Defrost Heater Capacity",
        ", !-  Defrost Energy Input Ratio Function of Temperature Curve Name",
        "1 , !- Compressor Multiplier",
        "FixedSpeed , !- Control Type",
        "100 , !- Crankcase Heater Capacity",
        "EIRCurveFuncPLR, !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "20 , !- Maximum Ambient Temperature for Crankcase Heater Operation",
        "2 , !- Number of Speeds for Heating",
        "100 , !-  Rated Heating Capacity at Speed 1",
        "3, !-  Rated COP for Heating at Speed 1",
        "CapCurveFuncTemp, !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncTemp, !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncPLR, !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 1",
        "200, !-  Rated Heating Capacity at Speed 2",
        "3.5, !-  Rated COP for Heating at Speed 2",
        "CapCurveFuncTemp, !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 2",
        "EIRCurveFuncTemp, !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2",
        "EIRCurveFuncPLR, !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 2",
        ", !-  Rated Heating Capacity at Speed 3",
        ", !-  Rated COP for Heating at Speed 3",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 3",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 3",
        ", !-  Rated Heating Capacity at Speed 4",
        ", !-  Rated COP for Heating at Speed 4",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 4",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 4",
        ", !-  Rated Heating Capacity at Speed 5",
        ", !-  Rated COP for Heating at Speed 5",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 5",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 5",
        "Yes, !-  Booster Mode On Heating",
        "50000, !-  Rated Heating Capacity in Booster Mode",
        "2.5, !-  Rated Heating COP in Booster Mode",
        "CapCurveFuncTemp, !-  Normalized Heating Capacity Function of Temperature Curve Name in Booster Mode",
        "EIRCurveFuncTemp, !-  Heating Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
        "EIRCurveFuncPLR,  !-  Heating Energy Input Ratio Function of PLR Curve Name in Booster Mode",
        "2, !-  Number of Speeds for Cooling",
        "120 , !-  Rated Cooling Capacity at Speed 1",
        "4, !-  Rated COP for Cooling at Speed 1",
        "CapCurveFuncTemp, !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncTemp, !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncPLR, !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1",
        "240, !-  Rated Cooling Capacity at Speed 2",
        "3.5, !-  Rated COP for Cooling at Speed 2",
        "CapCurveFuncTemp, !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2",
        "EIRCurveFuncTemp, !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2",
        "EIRCurveFuncPLR, !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2",
        ", !-  Rated Cooling Capacity at Speed 3",
        ", !-  Rated COP for Cooling at Speed 3",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3",
        ", !-  Rated Cooling Capacity at Speed 4",
        ", !-  Rated COP for Cooling at Speed 4",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4",
        ", !-  Rated Cooling Capacity at Speed 5",
        ", !-  Rated COP for Cooling at Speed 5",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5",
        "Yes, !-  Booster Mode On Cooling",
        "500, !-  Rated Cooling Capacity in Booster Mode",
        "2.0, !-  Rated Cooling COP in Booster Mode",
        "CapCurveFuncTemp, !-  Normalized Cooling Capacity Function of Temperature Curve Name in Booster Mode",
        "EIRCurveFuncTemp, !-  Cooling Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
        "EIRCurveFuncPLR; !-  Cooling Energy Input Ratio Function of PLR Curve Name in Booster Mode",

        "HeatPump:AirToWater,",
        "test_AWHP_defaults, !- Name",
        ", !- Availability Schedule Name Heating",
        ", !- Availability Schedule Name Cooling",
        ", !- Operating Mode Control Method",
        ", !- Operating Mode Control Option for Multiple Unit",
        ", !- Operating Mode Control Schedule Name",
        ", !-  Rated Inlet Air Temperature in Heating Mode",
        ", !-  Rated Air Flow Rate in Heating Mode",
        ", !-  Rated Leaving Water Temperature in Heating Mode",
        ", !-  Rated Water Flow Rate in Heating Mode",
        ", !-  Minimum Outdoor Air Temperature in Heating Mode",
        ", !-  Maximum Outdoor Air Temperature in Heating Mode",
        ",    !- Minimum Leaving Water Temperature Curve Name in Heating Mode",
        ",    !- Maximum Leaving Water Temperature Curve Name in Heating Mode",
        ",    !- Minimum Part Load Ratio",
        ", !-  Sizing Factor for Heating",
        ", !-  Rated Inlet Air Temperature in Cooling Mode",
        ", !-  Rated Air Flow Rate in Cooling Mode",
        ", !-  Rated Leaving Water Temperature in Cooling Mode",
        ", !-  Rated Water Flow Rate in Cooling Mode",
        ", !-  Minimum Outdoor Air Temperature in Cooling Mode",
        ", !-  Maximum Outdoor Air Temperature in Cooling Mode",
        ",    !- Minimum Leaving Water Temperature Curve Name in Cooling Mode",
        ",    !- Maximum Leaving Water Temperature Curve Name in Cooling Mode",
        ", !-  Sizing Factor for Cooling",
        "Outdoor Air Inlet Node , !-  Air Inlet Node Name",
        "Outdoor Air Outlet Node, !-  Air Outlet Node Name",
        "Heating Coil Load Loop Intermediate Node, !-  Hot Water Inlet Node Name",
        "Heating Coil Load Loop Supply Outlet Node, !-  Hot Water Outlet Node Name",
        "Cooling Coil Load Loop Intermediate Node , !-  Chilled Water Inlet Node Name",
        "Cooling Coil Load Loop Supply Outlet Node , !-  Chilled Water Outlet Node Name",
        ", !- Maximum Outdoor Dry Bulb Temperature For Defrost Operation",
        ", !-  Heat Pump Defrost Control",
        ", !-  Defrost Time Period Fraction",
        ", !-  Resistive Defrost Heater Capacity",
        ", !-  Defrost Energy Input Ratio Function of Temperature Curve Name",
        ", !- Compressor Multiplier",
        ", !- Control Type",
        ", !- Crankcase Heater Capacity",
        ", !- Crankcase Heater Capacity Function of Temperature Curve Name",
        ", !- Maximum Ambient Temperature for Crankcase Heater Operation",
        ", !- Number of Speeds for Heating",
        ", !-  Rated Heating Capacity at Speed 1",
        ", !-  Rated COP for Heating at Speed 1",
        "CapCurveFuncTemp, !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncTemp, !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncPLR, !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 1",
        ", !-  Rated Heating Capacity at Speed 2",
        ", !-  Rated COP for Heating at Speed 2",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 2",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 2",
        ", !-  Rated Heating Capacity at Speed 3",
        ", !-  Rated COP for Heating at Speed 3",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 3",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 3",
        ", !-  Rated Heating Capacity at Speed 4",
        ", !-  Rated COP for Heating at Speed 4",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 4",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 4",
        ", !-  Rated Heating Capacity at Speed 5",
        ", !-  Rated COP for Heating at Speed 5",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 5",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 5",
        ", !-  Booster Mode On Heating",
        ", !-  Rated Heating Capacity in Booster Mode",
        ", !-  Rated Heating COP in Booster Mode",
        ", !-  Normalized Heating Capacity Function of Temperature Curve Name in Booster Mode",
        ", !-  Heating Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
        ", !-  Heating Energy Input Ratio Function of PLR Curve Name in Booster Mode",
        ", !-  Number of Speeds for Cooling",
        ", !-  Rated Cooling Capacity at Speed 1",
        ", !-  Rated COP for Cooling at Speed 1",
        "CapCurveFuncTemp, !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncTemp, !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1",
        "EIRCurveFuncPLR,  !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1",
        ", !-  Rated Cooling Capacity at Speed 2",
        ", !-  Rated COP for Cooling at Speed 2",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2",
        ", !-  Rated Cooling Capacity at Speed 3",
        ", !-  Rated COP for Cooling at Speed 3",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3",
        ", !-  Rated Cooling Capacity at Speed 4",
        ", !-  Rated COP for Cooling at Speed 4",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4",
        ", !-  Rated Cooling Capacity at Speed 5",
        ", !-  Rated COP for Cooling at Speed 5",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5",
        ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5",
        ", !-  Booster Mode On Cooling",
        ", !-  Rated Cooling Capacity in Booster Mode",
        ", !-  Rated Cooling COP in Booster Mode",
        ", !-  Normalized Cooling Capacity Function of Temperature Curve Name in Booster Mode",
        ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
        "; !-  Cooling Energy Input Ratio Function of PLR Curve Name in Booster Mode",

        "Curve:Quadratic,",
        "  MinLWTvsOAT,             !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  -17.77778,               !- Minimum Value of x",
        "  35.0,                    !- Maximum Value of x",
        "  20.0,                    !- Minimum Curve Output",
        "  35.0,                    !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature;             !- Output Unit Type",

        "Curve:Quadratic,",
        "  MaxLWTvsOAT,             !- Name",
        "  53.1666666666667,        !- Coefficient1 Constant",
        "  0.85,                    !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  -17.777778,              !- Minimum Value of x",
        "  35.0,                    !- Maximum Value of x",
        "  20.0,                    !- Minimum Curve Output",
        "  60.0,                    !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature;             !- Output Unit Type",

        "Curve:Biquadratic,",
        "CapCurveFuncTemp,        !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Coefficient4 y",
        "0.0,                     !- Coefficient5 y**2",
        "0.0,                     !- Coefficient6 x*y",
        "5.0,                     !- Minimum Value of x",
        "10.0,                    !- Maximum Value of x",
        "24.0,                    !- Minimum Value of y",
        "35.0,                    !- Maximum Value of y",
        ",                        !- Minimum Curve Output",
        ",                        !- Maximum Curve Output",
        "Temperature,             !- Input Unit Type for X",
        "Temperature,             !- Input Unit Type for Y",
        "Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "EIRCurveFuncTemp,        !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Coefficient4 y",
        "0.0,                     !- Coefficient5 y**2",
        "0.0,                     !- Coefficient6 x*y",
        "5.0,                     !- Minimum Value of x",
        "10.0,                    !- Maximum Value of x",
        "24.0,                    !- Minimum Value of y",
        "35.0,                    !- Maximum Value of y",
        ",                        !- Minimum Curve Output",
        ",                        !- Maximum Curve Output",
        "Temperature,             !- Input Unit Type for X",
        "Temperature,             !- Input Unit Type for Y",
        "Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "EIRCurveFuncPLR,         !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Minimum Value of x",
        "1.0;                     !- Maximum Value of x",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    HeatPumpAirToWater::factory(*state, DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating, "TEST_AWHP");
    HeatPumpAirToWater::factory(*state, DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling, "TEST_AWHP");
    // cooling component in the AWHP
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[0].EIRHPType, DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].availSchedName, "");
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].availSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].operationModeControlSche, nullptr);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].sourceSideDesignInletTemp, 25);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].sourceSideDesignVolFlowRate, 0.1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedLeavingWaterTemperature, 22);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].loadSideDesignVolFlowRate, 0.05);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].minSourceTempLimit, 18);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].maxSourceTempLimit, 40);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].minSupplyWaterTempCurveIndex, 3);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].maxSupplyWaterTempCurveIndex, 4);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].sizingFactor, 0.9);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].loadSideNodes.inlet, 1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].loadSideNodes.outlet, 2);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].sourceSideNodes.inlet, 3);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].sourceSideNodes.outlet, 4);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].heatPumpMultiplier, 1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].controlType, HeatPumpAirToWater::CompressorControlType::FixedSpeed);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].boosterOn, true);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].numSpeeds, 3);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].referenceCapacity, 500);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedCapacity[0], 120);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedCOP[0], 4);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].capFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].powerRatioFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].powerRatioFuncPLRCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedCapacity[1], 240);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedCOP[1], 3.5);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].capFuncTempCurveIndex[1], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].powerRatioFuncTempCurveIndex[1], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].powerRatioFuncPLRCurveIndex[1], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedCapacity[2], 500);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].ratedCOP[2], 2.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].capFuncTempCurveIndex[2], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].powerRatioFuncTempCurveIndex[2], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[0].powerRatioFuncPLRCurveIndex[2], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[0].operatingModeControlMethod, HeatPumpAirToWater::OperatingModeControlMethod::Load);
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[0].operatingModeControlOptionMultipleUnit,
                   HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::SingleMode);

    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[1].EIRHPType, DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].name, "TEST_AWHP");
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[1].operatingModeControlMethod, HeatPumpAirToWater::OperatingModeControlMethod::Load);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].availSchedName, "");
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].availSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].operationModeControlSche, nullptr);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].sourceSideDesignInletTemp, 20);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].sourceSideDesignVolFlowRate, 0.1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedLeavingWaterTemperature, 50);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].loadSideDesignVolFlowRate, 0.02);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].minSourceTempLimit, -20);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].maxSourceTempLimit, 25);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].minSupplyWaterTempCurveIndex, 3);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].maxSupplyWaterTempCurveIndex, 4);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].sizingFactor, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].loadSideNodes.inlet, 5);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].loadSideNodes.outlet, 6);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].sourceSideNodes.inlet, 3);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].sourceSideNodes.outlet, 4);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].maxOutdoorTemperatureDefrost, 10);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].defrostStrategy, DefrostControl::Timed);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].defrostTime, 0.2);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].defrostResistiveHeaterCap, 150);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].defrostEIRFTIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].heatPumpMultiplier, 1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].controlType, HeatPumpAirToWater::CompressorControlType::FixedSpeed);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].CrankcaseHeaterCapacity, 100);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].CrankcaseHeaterCapacityCurveIndex, Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].MaxOATCrankcaseHeater, 20);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].boosterOn, true);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].numSpeeds, 3); // with booster mode, one more speed level
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].referenceCapacity, 50000);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedCapacity[0], 100);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedCOP[0], 3);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].capFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].powerRatioFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].powerRatioFuncPLRCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedCapacity[1], 200);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedCOP[1], 3.5);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].capFuncTempCurveIndex[1], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].powerRatioFuncTempCurveIndex[1], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].powerRatioFuncPLRCurveIndex[1], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedCapacity[2], 50000);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].ratedCOP[2], 2.5);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].capFuncTempCurveIndex[2], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].powerRatioFuncTempCurveIndex[2], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[1].powerRatioFuncPLRCurveIndex[2], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[1].operatingModeControlMethod, HeatPumpAirToWater::OperatingModeControlMethod::Load);
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[1].operatingModeControlOptionMultipleUnit,
                   HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::SingleMode);

    HeatPumpAirToWater::factory(*state, DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating, "test_AWHP_defaults");
    HeatPumpAirToWater::factory(*state, DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling, "test_AWHP_defaults");

    // cooling component in the AWHP
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[2].EIRHPType, DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].name, "TEST_AWHP_DEFAULTS");
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].availSchedName, "");
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].availSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].operationModeControlSche, nullptr);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].sourceSideDesignInletTemp, 8);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].sourceSideDesignVolFlowRate, -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].ratedLeavingWaterTemperature, 40);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].loadSideDesignVolFlowRate, -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].minSourceTempLimit, -30);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].maxSourceTempLimit, 100);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].minSupplyWaterTempCurveIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].maxSupplyWaterTempCurveIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].sizingFactor, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].heatPumpMultiplier, 1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].controlType, HeatPumpAirToWater::CompressorControlType::VariableSpeed);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].boosterOn, false);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].boosterMultCap, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].boosterMultCOP, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].numSpeeds, 1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].referenceCapacity, -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].ratedCapacity[0], -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].ratedCOP[0], 3.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].capFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].powerRatioFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].powerRatioFuncPLRCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[2].operatingModeControlMethod, HeatPumpAirToWater::OperatingModeControlMethod::Load);
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[2].operatingModeControlOptionMultipleUnit,
                   HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::SingleMode);

    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[3].EIRHPType, DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].name, "TEST_AWHP_DEFAULTS");
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[3].operatingModeControlMethod, HeatPumpAirToWater::OperatingModeControlMethod::Load);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].availSchedName, "");
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].availSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].operationModeControlSche, nullptr);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].sourceSideDesignInletTemp, 8);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].sourceSideDesignVolFlowRate, -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].ratedLeavingWaterTemperature, 40);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].loadSideDesignVolFlowRate, -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].minSourceTempLimit, -30);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].maxSourceTempLimit, 100);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].minSupplyWaterTempCurveIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[2].maxSupplyWaterTempCurveIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].sizingFactor, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].maxOutdoorTemperatureDefrost, 10);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].defrostStrategy, DefrostControl::None);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].defrostTime, 0.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].defrostResistiveHeaterCap, 0.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].defrostEIRFTIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].heatPumpMultiplier, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].controlType, HeatPumpAirToWater::CompressorControlType::VariableSpeed);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].CrankcaseHeaterCapacity, 0.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].CrankcaseHeaterCapacityCurveIndex, 0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].MaxOATCrankcaseHeater, 10.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].boosterOn, false);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].boosterMultCap, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].boosterMultCOP, 1.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].numSpeeds, 1);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].referenceCapacity, -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].ratedCapacity[0], -99999);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].ratedCOP[0], 3.0);
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].capFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].powerRatioFuncTempCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP"));
    EXPECT_EQ(state->dataHeatPumpAirToWater->heatPumps[3].powerRatioFuncPLRCurveIndex[0], Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR"));
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[3].operatingModeControlMethod, HeatPumpAirToWater::OperatingModeControlMethod::Load);
    EXPECT_ENUM_EQ(state->dataHeatPumpAirToWater->heatPumps[3].operatingModeControlOptionMultipleUnit,
                   HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::SingleMode);
}

TEST_F(EnergyPlusFixture, calcLoadSideHeatTransfer_AWHP)
{
    auto thisAWHP = HeatPumpAirToWater();

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    state->dataLoopNodes->Node.allocate(2);
    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    thisAWHP.loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisAWHP.loadSidePlantLoc.branchNum = 1;
    thisAWHP.loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisAWHP.loadSidePlantLoc);
    thisAWHP.loadSideNodes.outlet = 2;
    thisAWHP.loadSideNodes.inlet = 1;
    thisAWHP.loadSideMassFlowRate = 2;
    thisAWHP.loadSideInletTemp = 20;
    thisAWHP.loadSidePlantLoc.loop = &state->dataPlnt->PlantLoop(1);
    state->dataLoopNodes->Node(thisAWHP.loadSideNodes.inlet).Temp = thisAWHP.loadSideInletTemp;
    Real64 CpLoad = thisAWHP.loadSidePlantLoc.loop->glycol->getSpecificHeat(
        *state, state->dataLoopNodes->Node(thisAWHP.loadSideNodes.inlet).Temp, "HeatPumpAirToWater::calcLoadSideHeatTransfer()");
    // if capacity is less than load, heat transfer is just capacity
    Real64 capacity = 100;
    Real64 load = 120;
    thisAWHP.calcLoadOutletTemp = EIRPlantLoopHeatPumps::HeatPumpAirToWater::subtract;
    thisAWHP.calcLoadSideHeatTransfer(*state, capacity, load);
    EXPECT_EQ(thisAWHP.loadSideHeatTransfer, capacity);
    EXPECT_NEAR(
        CpLoad * (thisAWHP.loadSideOutletTemp - thisAWHP.loadSideInletTemp) * thisAWHP.loadSideMassFlowRate, -thisAWHP.loadSideHeatTransfer, 1e-6);

    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating;
    thisAWHP.calcLoadOutletTemp = EIRPlantLoopHeatPumps::HeatPumpAirToWater::add;
    thisAWHP.calcLoadSideHeatTransfer(*state, capacity, load);
    EXPECT_EQ(thisAWHP.loadSideHeatTransfer, capacity);
    EXPECT_NEAR(
        CpLoad * (thisAWHP.loadSideOutletTemp - thisAWHP.loadSideInletTemp) * thisAWHP.loadSideMassFlowRate, thisAWHP.loadSideHeatTransfer, 1e-6);

    // if capacity is less than load, heat transfer is load
    capacity = 120;
    load = 100;
    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    thisAWHP.calcLoadOutletTemp = EIRPlantLoopHeatPumps::HeatPumpAirToWater::subtract;
    thisAWHP.calcLoadSideHeatTransfer(*state, capacity, load);
    EXPECT_EQ(thisAWHP.loadSideHeatTransfer, load);
    EXPECT_NEAR(
        CpLoad * (thisAWHP.loadSideOutletTemp - thisAWHP.loadSideInletTemp) * thisAWHP.loadSideMassFlowRate, -thisAWHP.loadSideHeatTransfer, 1e-6);

    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating;
    thisAWHP.calcLoadOutletTemp = EIRPlantLoopHeatPumps::HeatPumpAirToWater::add;
    thisAWHP.calcLoadSideHeatTransfer(*state, capacity, load);
    EXPECT_EQ(thisAWHP.loadSideHeatTransfer, load);
    EXPECT_NEAR(
        CpLoad * (thisAWHP.loadSideOutletTemp - thisAWHP.loadSideInletTemp) * thisAWHP.loadSideMassFlowRate, thisAWHP.loadSideHeatTransfer, 1e-6);

    // if load is smaller than minimum load (minimumPLR * availableCapacity), heat transfer is minimum load
    capacity = 120;
    load = 10;
    thisAWHP.minimumPLR = 0.1;
    thisAWHP.referenceCapacity = capacity;
    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    thisAWHP.calcLoadOutletTemp = EIRPlantLoopHeatPumps::HeatPumpAirToWater::subtract;
    thisAWHP.calcLoadSideHeatTransfer(*state, capacity, load);
    EXPECT_EQ(thisAWHP.loadSideHeatTransfer, capacity * thisAWHP.minimumPLR);
}

TEST_F(EnergyPlusFixture, calcPowerUsage_AWHP)
{

    std::string const idf_objects = delimited_string({
        "Curve:Biquadratic,",
        "CapCurveFuncTemp,        !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Coefficient4 y",
        "0.0,                     !- Coefficient5 y**2",
        "0.0,                     !- Coefficient6 x*y",
        "5.0,                     !- Minimum Value of x",
        "10.0,                    !- Maximum Value of x",
        "24.0,                    !- Minimum Value of y",
        "35.0,                    !- Maximum Value of y",
        ",                        !- Minimum Curve Output",
        ",                        !- Maximum Curve Output",
        "Temperature,             !- Input Unit Type for X",
        "Temperature,             !- Input Unit Type for Y",
        "Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "EIRCurveFuncTemp,        !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Coefficient4 y",
        "0.0,                     !- Coefficient5 y**2",
        "0.0,                     !- Coefficient6 x*y",
        "5.0,                     !- Minimum Value of x",
        "10.0,                    !- Maximum Value of x",
        "24.0,                    !- Minimum Value of y",
        "35.0,                    !- Maximum Value of y",
        ",                        !- Minimum Curve Output",
        ",                        !- Maximum Curve Output",
        "Temperature,             !- Input Unit Type for X",
        "Temperature,             !- Input Unit Type for Y",
        "Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "EIRCurveFuncPLR,         !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Minimum Value of x",
        "1.0;                     !- Maximum Value of x",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    auto thisAWHP = HeatPumpAirToWater();
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    thisAWHP.loadSidePlantLoc.loop = &state->dataPlnt->PlantLoop(1);
    thisAWHP.loadSidePlantLoc.comp = &PLHPPlantLoadSideComp;
    thisAWHP.loadSidePlantLoc.comp->CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;

    thisAWHP.loadSideNodes.inlet = 1;
    thisAWHP.loadSideNodes.outlet = 2;
    state->dataLoopNodes->Node.allocate(2);
    state->dataLoopNodes->Node(2).TempSetPoint = 20;
    thisAWHP.sourceSideHeatTransfer = 100;
    thisAWHP.loadSideHeatTransfer = 90;
    thisAWHP.numSpeeds = 2;
    thisAWHP.ratedCapacity[0] = 600;
    thisAWHP.ratedCOP[0] = 1;
    thisAWHP.capFuncTempCurveIndex[0] = Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP");
    thisAWHP.powerRatioFuncTempCurveIndex[0] = Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP");
    thisAWHP.powerRatioFuncPLRCurveIndex[0] = Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR");
    thisAWHP.ratedCapacity[1] = 1200;
    thisAWHP.ratedCOP[1] = 1;
    thisAWHP.capFuncTempCurveIndex[1] = Curve::GetCurveIndex(*state, "CAPCURVEFUNCTEMP");
    thisAWHP.powerRatioFuncTempCurveIndex[1] = Curve::GetCurveIndex(*state, "EIRCURVEFUNCTEMP");
    thisAWHP.powerRatioFuncPLRCurveIndex[1] = Curve::GetCurveIndex(*state, "EIRCURVEFUNCPLR");
    thisAWHP.heatPumpMultiplier = 2;
    thisAWHP.cyclingRatio = 1.0;
    Real64 availableCapacityBeforeMultiplier = thisAWHP.ratedCapacity[1];
    // when COP = 1, power usage should equal heat transfer
    thisAWHP.loadSideHeatTransfer = 500;
    thisAWHP.calcPowerUsage(*state, availableCapacityBeforeMultiplier);
    EXPECT_EQ(thisAWHP.speedLevel, 1);
    EXPECT_EQ(thisAWHP.powerUsage, 500);
    EXPECT_EQ(thisAWHP.cyclingRatio, 500.0 / 600.0);
    EXPECT_EQ(thisAWHP.numUnitUsed, 1);
    thisAWHP.loadSideHeatTransfer = 1000;
    thisAWHP.cyclingRatio = 1.0; // reset cycling ratio back to 1
    thisAWHP.calcPowerUsage(*state, availableCapacityBeforeMultiplier);
    EXPECT_EQ(thisAWHP.speedLevel, 2);
    EXPECT_EQ(thisAWHP.powerUsage, 1000);
    EXPECT_EQ(thisAWHP.cyclingRatio, 1.0);
    EXPECT_EQ(thisAWHP.numUnitUsed, 1);
    thisAWHP.loadSideHeatTransfer = 1500;
    thisAWHP.cyclingRatio = 1.0; // reset cycling ratio back to 1
    thisAWHP.calcPowerUsage(*state, availableCapacityBeforeMultiplier);
    EXPECT_EQ(thisAWHP.speedLevel, 1);
    EXPECT_EQ(thisAWHP.powerUsage, 1500);
    EXPECT_EQ(thisAWHP.numUnitUsed, 2);
    EXPECT_EQ(thisAWHP.cyclingRatio, 300.0 / 600.0);
    thisAWHP.loadSideHeatTransfer = 2000;
    thisAWHP.cyclingRatio = 1.0; // reset cycling ratio back to 1
    thisAWHP.calcPowerUsage(*state, availableCapacityBeforeMultiplier);
    EXPECT_EQ(thisAWHP.speedLevel, 2);
    EXPECT_EQ(thisAWHP.powerUsage, 2000);
    EXPECT_EQ(thisAWHP.numUnitUsed, 2);
    EXPECT_EQ(thisAWHP.cyclingRatio, 1.0);
}

TEST_F(EnergyPlusFixture, crankcaseHeater_AWHP)
{
    std::string const idf_objects = delimited_string({
        "Curve:Linear,",
        "heaterCapCurve,          !- Name",
        "10.0,                    !- Coefficient1 Constant",
        "2.,                      !- Coefficient2 x",
        "-10.0,                    !- Minimum Value of x",
        "70;                      !- Maximum Value of x",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    auto thisAWHP = HeatPumpAirToWater();
    thisAWHP.CrankcaseHeaterCapacity = 100;
    thisAWHP.CrankcaseHeaterCapacityCurveIndex = Curve::GetCurveIndex(*state, "HEATERCAPCURVE");
    // when outdoor temperature is higher than temperature threshold, crankcase heater is off
    state->dataEnvrn->OutDryBulbTemp = 15;
    thisAWHP.MaxOATCrankcaseHeater = 10;
    thisAWHP.CrankcaseHeaterPower = thisAWHP.calcCrankcaseHeaterPower(*state);
    ASSERT_EQ(thisAWHP.CrankcaseHeaterPower, 0.0);
    state->dataEnvrn->OutDryBulbTemp = 9;
    thisAWHP.CrankcaseHeaterPower = thisAWHP.calcCrankcaseHeaterPower(*state);
    ASSERT_EQ(thisAWHP.CrankcaseHeaterPower, 100 * (10 + 2 * 9));
}

TEST_F(EnergyPlusFixture, calcOpMode_AWHP)
{
    std::string const idf_objects = delimited_string({
        "Curve:Biquadratic,"
        "CapCurveFuncTemp,        !- Name",
        "1.0,                     !- Coefficient1 Constant",
        "0.0,                     !- Coefficient2 x",
        "0.0,                     !- Coefficient3 x**2",
        "0.0,                     !- Coefficient4 y",
        "0.0,                     !- Coefficient5 y**2",
        "0.0,                     !- Coefficient6 x*y",
        "5.0,                     !- Minimum Value of x",
        "10.0,                    !- Maximum Value of x",
        "24.0,                    !- Minimum Value of y",
        "35.0,                    !- Maximum Value of y",
        ",                        !- Minimum Curve Output",
        ",                        !- Maximum Curve Output",
        "Temperature,             !- Input Unit Type for X",
        "Temperature,             !- Input Unit Type for Y",
        "Dimensionless;           !- Output Unit Type",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    auto thisAWHP = HeatPumpAirToWater();
    auto companionAWHP = HeatPumpAirToWater();
    thisAWHP.companionHeatPumpCoil = &companionAWHP;
    companionAWHP.companionHeatPumpCoil = &thisAWHP;
    thisAWHP.heatPumpMultiplier = 6;
    companionAWHP.heatPumpMultiplier = 6;
    thisAWHP.referenceCapacityOneUnit = 100;
    companionAWHP.referenceCapacityOneUnit = 150;
    thisAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    companionAWHP.EIRHPType = EnergyPlus::DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating;

    state->dataPlnt->PlantLoop.allocate(2);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    thisAWHP.loadSidePlantLoc.loop = &state->dataPlnt->PlantLoop(1);
    thisAWHP.loadSidePlantLoc.comp = &PLHPPlantLoadSideComp;
    thisAWHP.loadSidePlantLoc.loopNum = 1;
    thisAWHP.loadSidePlantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    thisAWHP.loadSidePlantLoc.branchNum = 1;
    thisAWHP.loadSidePlantLoc.compNum = 1;
    thisAWHP.OperationModeEMSOverrideOn = false;

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &companionPLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    companionPLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating;
    companionAWHP.loadSidePlantLoc.loop = &state->dataPlnt->PlantLoop(2);
    companionAWHP.loadSidePlantLoc.comp = &companionPLHPPlantLoadSideComp;
    companionAWHP.loadSidePlantLoc.loopNum = 2;
    companionAWHP.loadSidePlantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    companionAWHP.loadSidePlantLoc.branchNum = 1;
    companionAWHP.loadSidePlantLoc.compNum = 1;
    companionAWHP.OperationModeEMSOverrideOn = false;

    thisAWHP.capFuncTempCurveIndex[0] = 1;
    thisAWHP.loadSideOutletTemp = 65;
    thisAWHP.sourceSideInletTemp = 20;
    companionAWHP.capFuncTempCurveIndex[0] = 1;
    companionAWHP.loadSideOutletTemp = 65;
    companionAWHP.sourceSideInletTemp = 20;

    Real64 currentLoad = -300;
    companionPLHPPlantLoadSideComp.MyLoad = 400;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::SingleMode);
    EXPECT_EQ(thisAWHP.operatingMode, 0);
    EXPECT_EQ(companionAWHP.operatingMode, 3);

    currentLoad = -500;
    companionPLHPPlantLoadSideComp.MyLoad = 300;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::SingleMode);
    EXPECT_EQ(thisAWHP.operatingMode, 5);
    EXPECT_EQ(companionAWHP.operatingMode, 0);

    currentLoad = -300;
    companionPLHPPlantLoadSideComp.MyLoad = 400;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::CoolingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 3);
    EXPECT_EQ(companionAWHP.operatingMode, 3);

    currentLoad = -500;
    companionPLHPPlantLoadSideComp.MyLoad = 300;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::CoolingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 5);
    EXPECT_EQ(companionAWHP.operatingMode, 1);

    currentLoad = -250;
    companionPLHPPlantLoadSideComp.MyLoad = 250;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::CoolingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 3);
    EXPECT_EQ(companionAWHP.operatingMode, 2);

    currentLoad = -300;
    companionPLHPPlantLoadSideComp.MyLoad = 400;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::HeatingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 3);
    EXPECT_EQ(companionAWHP.operatingMode, 3);

    currentLoad = -500;
    companionPLHPPlantLoadSideComp.MyLoad = 300;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::HeatingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 4);
    EXPECT_EQ(companionAWHP.operatingMode, 2);

    currentLoad = -250;
    companionPLHPPlantLoadSideComp.MyLoad = 250;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::HeatingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 3);
    EXPECT_EQ(companionAWHP.operatingMode, 2);

    currentLoad = -200;
    companionPLHPPlantLoadSideComp.MyLoad = 1000;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::HeatingPriority);
    EXPECT_EQ(thisAWHP.operatingMode, 0);
    EXPECT_EQ(companionAWHP.operatingMode, 6);

    currentLoad = 0;
    companionPLHPPlantLoadSideComp.MyLoad = 400;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 0);
    EXPECT_EQ(companionAWHP.operatingMode, 3);

    currentLoad = 0;
    companionPLHPPlantLoadSideComp.MyLoad = 4000;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 0);
    EXPECT_EQ(companionAWHP.operatingMode, 6);

    currentLoad = -400;
    companionPLHPPlantLoadSideComp.MyLoad = 0;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 4);
    EXPECT_EQ(companionAWHP.operatingMode, 0);

    currentLoad = -4000;
    companionPLHPPlantLoadSideComp.MyLoad = 0;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 6);
    EXPECT_EQ(companionAWHP.operatingMode, 0);

    currentLoad = -300;
    companionPLHPPlantLoadSideComp.MyLoad = 400;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 3);
    EXPECT_EQ(companionAWHP.operatingMode, 3);

    currentLoad = -500;
    companionPLHPPlantLoadSideComp.MyLoad = 300;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 4);
    EXPECT_EQ(companionAWHP.operatingMode, 2);

    currentLoad = -1000;
    companionPLHPPlantLoadSideComp.MyLoad = 1500;
    thisAWHP.calcOpMode(*state, currentLoad, HeatPumpAirToWater::OperatingModeControlOptionMultipleUnit::Balanced);
    EXPECT_EQ(thisAWHP.operatingMode, 3);
    EXPECT_EQ(companionAWHP.operatingMode, 3);
}

TEST_F(EnergyPlusFixture, processInputForEIRPLHP_TestAirSourceDuplicateNodes)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 3,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    // expects fatal error due to same node names
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");, std::runtime_error);
    // expect error related to same node names
    std::string error_msg = delimited_string({
        "   ** Severe  ** PlantLoopHeatPump hp cooling side has the same inlet and outlet node.",
        "   **   ~~~   ** Node Name: NODE 3",
        "   **  Fatal  ** Previous EIR PLHP errors cause program termination",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=PlantLoopHeatPump hp cooling side has the same inlet and outlet node.",
    });
    EXPECT_TRUE(compare_err_stream(error_msg));
}

TEST_F(EnergyPlusFixture, processInputForEIRPLHP_TestAirSourceOANode)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;",
                                                      "OutdoorAir:NodeList,",
                                                      "  node 3;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    // setup the outdoor air nodes
    OutAirNodeManager::SetOutAirNodes(*state);
    // call the factory with a valid name to trigger reading inputs
    // expects fatal error due to same node names
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");
    EXPECT_TRUE(compare_err_stream(""));
}

TEST_F(EnergyPlusFixture, processInputForEIRPLHP_TestAirSourceNoOANode)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    // expects fatal error due to same node names
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");
    bool ErrFound = false;
    BranchNodeConnections::CheckNodeConnections(*state, ErrFound);
    // expect error related to OA node not being an OutdoorAirNode
    std::string error_msg = delimited_string({
        "   ** Severe  ** Node Connection Error, Node=\"NODE 1\", Inlet node did not find an appropriate matching \"outlet\" node.",
        "   **   ~~~   ** If this is an outdoor air inlet node, it must be listed in an OutdoorAir:Node or OutdoorAir:NodeList object.",
        "   **   ~~~   ** Reference Object=HeatPump:PlantLoop:EIR:Cooling, Name=HP COOLING SIDE",
        "   ** Severe  ** Node Connection Error, Node=\"NODE 3\", Inlet node did not find an appropriate matching \"outlet\" node.",
        "   **   ~~~   ** If this is an outdoor air inlet node, it must be listed in an OutdoorAir:Node or OutdoorAir:NodeList object.",
        "   **   ~~~   ** Reference Object=HeatPump:PlantLoop:EIR:Cooling, Name=HP COOLING SIDE",

    });
    EXPECT_TRUE(compare_err_stream(error_msg));
}

TEST_F(EnergyPlusFixture, Initialization)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // call for initialization, oneTimeInit only first
    state->dataGlobal->BeginEnvrnFlag = false;
    thisCoolingPLHP->onInitLoopEquip(*state, myLocation);

    // validate that location work got done correctly
    EXPECT_EQ(1, thisCoolingPLHP->loadSidePlantLoc.loopNum);
    EXPECT_ENUM_EQ(DataPlant::LoopSideLocation::Supply, thisCoolingPLHP->loadSidePlantLoc.loopSideNum);
    EXPECT_EQ(1, thisCoolingPLHP->loadSidePlantLoc.branchNum);
    EXPECT_EQ(1, thisCoolingPLHP->loadSidePlantLoc.compNum);
    EXPECT_EQ(2, thisCoolingPLHP->sourceSidePlantLoc.loopNum);
    EXPECT_ENUM_EQ(DataPlant::LoopSideLocation::Demand, thisCoolingPLHP->sourceSidePlantLoc.loopSideNum);
    EXPECT_EQ(1, thisCoolingPLHP->sourceSidePlantLoc.branchNum);
    EXPECT_EQ(1, thisCoolingPLHP->sourceSidePlantLoc.compNum);

    // now call for initialization again, for begin environment
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLocation);

    // validate that plant sizing went ok
    Real64 constexpr flowTol = 0.001;
    Real64 constexpr rho = 999.89; // easy to edit here if the expected density gets adjusted in E+
    Real64 const expectedLoadSideMassFlow = rho * thisCoolingPLHP->loadSideDesignVolFlowRate;
    Real64 const expectedSourceSideMassFlow = rho * thisCoolingPLHP->sourceSideDesignVolFlowRate;
    EXPECT_NEAR(expectedLoadSideMassFlow, thisCoolingPLHP->loadSideDesignMassFlowRate, flowTol);
    EXPECT_NEAR(expectedSourceSideMassFlow, thisCoolingPLHP->sourceSideDesignMassFlowRate, flowTol);
    EXPECT_NEAR(0.0, state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRateMin, flowTol);
    EXPECT_NEAR(0.0, state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRateMinAvail, flowTol);
    EXPECT_NEAR(expectedLoadSideMassFlow, state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRateMax, flowTol);
    EXPECT_NEAR(expectedLoadSideMassFlow, state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRateMaxAvail, flowTol);
    EXPECT_NEAR(0.0, state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRateMin, flowTol);
    EXPECT_NEAR(0.0, state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRateMinAvail, flowTol);
    EXPECT_NEAR(expectedSourceSideMassFlow, state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRateMax, flowTol);
    EXPECT_NEAR(expectedSourceSideMassFlow, state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRateMaxAvail, flowTol);
}

TEST_F(EnergyPlusFixture, TestSizing_FullyAutosizedCoolingWithCompanion_WaterSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      ",",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      ",",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // validate that we have the right ones
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(2);

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    Real64 constexpr plantSizingLoadVolFlow = 0.01;
    Real64 constexpr plantSizingLoadDeltaT = 1.0;

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.010;
    state->dataSize->PlantSizData(1).DeltaT = 1.0;
    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.030;
    state->dataSize->PlantSizData(2).DeltaT = 1.0;

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    auto &loop1supplyComponent2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(2);
    auto &loop2demandComponent2 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1supplyComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop2demandComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop2demandComponent1.Name = thisHeatingPLHP->name;
    loop1supplyComponent2.Name = thisCoolingPLHP->name;
    loop2demandComponent2.Name = thisCoolingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop2demandComponent1.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;
    loop1supplyComponent2.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    loop2demandComponent2.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myCoolingSourceLocation = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 2);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // assign the plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;

    // The load side should be what is imposed by the plant sizing data for the design flow rate.
    // The source side should be calculated based on the COP, which was set at 1.0 for convenience.
    //   This works out to a multiplier of 2 on the source flow rate.  With the same deltaT on the design of the source
    //   loop, the flow rate must be twice as high.
    // The source side has a slightly different set of thermal properties so the expected flow is scaled by those.
    Real64 constexpr expectedLoadCp = 4197.93;
    Real64 constexpr expectedLoadRho = 999.898;
    Real64 constexpr expectedSourceCp = 4185.0;
    Real64 constexpr expectedSourceRho = 983.2;
    Real64 const expectedLoadFlow = plantSizingLoadVolFlow;
    Real64 expectedCapacity = expectedLoadRho * expectedLoadFlow * expectedLoadCp * plantSizingLoadDeltaT;
    Real64 expectedSourceFlow = plantSizingLoadVolFlow * 2.0;
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(expectedLoadFlow, thisCoolingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedSourceFlow, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedCapacity, thisCoolingPLHP->referenceCapacity, 0.0001);

    // with a sizing run complete, we can also go ahead and get the design capacities...
    // they should be nonzero for the load side of things
    Real64 tmpMin = -1.0, tmpMax = -1.0, tmpOpt = -1.0;
    thisCoolingPLHP->getDesignCapacities(*state, myCoolingLoadLocation, tmpMax, tmpMin, tmpOpt);
    EXPECT_NEAR(0.0, tmpMin, 0.001);
    EXPECT_NEAR(expectedCapacity, tmpMax, 0.001);
    EXPECT_NEAR(expectedCapacity, tmpOpt, 0.001);
    // but always zero for the source side of things
    tmpMin = -1.0, tmpMax = -1.0, tmpOpt = -1.0;
    thisCoolingPLHP->getDesignCapacities(*state, myCoolingSourceLocation, tmpMax, tmpMin, tmpOpt);
    EXPECT_NEAR(0.0, tmpMin, 0.001);
    EXPECT_NEAR(0.0, tmpMax, 0.001);
    EXPECT_NEAR(0.0, tmpOpt, 0.001);

    // we can reset things and do a few more corner cases here

    // lets just try it again but with the plant sizing data set to zero flow, it should try to use the companion
    // but the companion isn't sized yet, so it should get zero conditions
    thisCoolingPLHP->loadSideDesignVolFlowRate = DataSizing::AutoSize;
    thisCoolingPLHP->sourceSideDesignVolFlowRate = DataSizing::AutoSize;
    thisCoolingPLHP->referenceCapacity = DataSizing::AutoSize;
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.0;
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->referenceCapacity, 0.0001);

    // but now let's try to size the heating coil, which will try to use the cooling coil's sized data
    thisCoolingPLHP->loadSideDesignVolFlowRate = expectedLoadFlow;
    thisCoolingPLHP->sourceSideDesignVolFlowRate = expectedSourceFlow;
    thisCoolingPLHP->referenceCapacity = expectedCapacity;
    Real64 const designSourceSideHeatTransfer = expectedCapacity * (1 - 1 / thisHeatingPLHP->referenceCOP);
    expectedSourceFlow = designSourceSideHeatTransfer / (state->dataSize->PlantSizData(2).DeltaT * expectedSourceCp * expectedSourceRho);
    expectedCapacity = expectedLoadRho * expectedLoadFlow * expectedLoadCp * plantSizingLoadDeltaT;
    thisHeatingPLHP->sizeLoadSide(*state);
    thisHeatingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(expectedLoadFlow, thisHeatingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedSourceFlow, thisHeatingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedCapacity, thisHeatingPLHP->referenceCapacity, 0.0001);
}

TEST_F(EnergyPlusFixture, TestSizing_FullyHardsizedHeatingWithCompanion)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  0.01,",
                                                      "  0.02,",
                                                      "  ,",
                                                      "  1200,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  0.01,",
                                                      "  0.02,",
                                                      "  ,",
                                                      "  1200,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // validate that we have the right ones
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.020;
    state->dataSize->PlantSizData(1).DeltaT = 1.0;
    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.030;
    state->dataSize->PlantSizData(2).DeltaT = 1.0;

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    auto &loop1supplyComponent2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(2);
    auto &loop2demandComponent2 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1supplyComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop2demandComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop2demandComponent1.Name = thisHeatingPLHP->name;
    loop1supplyComponent2.Name = thisCoolingPLHP->name;
    loop2demandComponent2.Name = thisCoolingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop2demandComponent1.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;
    loop1supplyComponent2.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    loop2demandComponent2.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->DisplayExtraWarnings = true;

    // initialize so the components can find themselves on the plant
    thisHeatingPLHP->onInitLoopEquip(*state, myLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // The values really should just come out all as the hard-sized values, this just makes sure that function didn't
    // botch something up.
    thisHeatingPLHP->sizeLoadSide(*state);
    thisHeatingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(0.01, thisHeatingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.02, thisHeatingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(1200, thisHeatingPLHP->referenceCapacity, 0.0001);

    // Call it again, but this time with PlantSizing on, it should come out the same again
    state->dataGlobal->DoPlantSizing = true;
    thisHeatingPLHP->sizeLoadSide(*state);
    thisHeatingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(0.01, thisHeatingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.02, thisHeatingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(1200, thisHeatingPLHP->referenceCapacity, 0.0001);
}

TEST_F(EnergyPlusFixture, TestSizing_WithCompanionNoPlantSizing)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // validate that we have the right ones
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(2);

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    auto &loop1supplyComponent2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(2);
    auto &loop2demandComponent2 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1supplyComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop2demandComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop2demandComponent1.Name = thisHeatingPLHP->name;
    loop1supplyComponent2.Name = thisCoolingPLHP->name;
    loop2demandComponent2.Name = thisCoolingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop2demandComponent1.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;
    loop1supplyComponent2.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    loop2demandComponent2.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 2);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // let's just fake that the companion coil already got autosized properly
    thisHeatingPLHP->loadSideDesignVolFlowRate = 0.1;
    thisHeatingPLHP->sourceSideDesignVolFlowRate = 0.2;
    thisHeatingPLHP->referenceCapacity = 1000.0;

    // With no plant sizing, it will try to use the autosized companion instead
    // the load flow should be the companion load flow
    // with no source plant sizing, the source flow will actually work out to be the same as the load flow (not the source flow)
    // the capacity will be the companion capacity
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(0.1, thisCoolingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.1, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(1000.0, thisCoolingPLHP->referenceCapacity, 0.0001);
}

TEST_F(EnergyPlusFixture, TestSizing_NoCompanionNoPlantSizingError)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate that we have the right ones
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop2demandComponent1.Name = thisHeatingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop2demandComponent1.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    // initialize so the components can find themselves on the plant
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // with no plant sizing available and no companion coil to size from, it should throw a fatal
    EXPECT_THROW(thisHeatingPLHP->sizeLoadSide(*state), std::runtime_error);
}

TEST_F(EnergyPlusFixture, TestSizing_NoCompanionNoPlantSizingHardSized)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.1,",
                                                      "  0.1,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate that we have the right ones
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop2demandComponent1.Name = thisHeatingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop2demandComponent1.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    // initialize so the components can find themselves on the plant
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // this should report out to the sizing output, but just the user defined stuff
    thisHeatingPLHP->sizeLoadSide(*state);
    thisHeatingPLHP->sizeSrcSideWSHP(*state);
    EXPECT_NEAR(0.1, thisHeatingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.1, thisHeatingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(1000, thisHeatingPLHP->referenceCapacity, 0.0001);
}

TEST_F(EnergyPlusFixture, CoolingOutletSetpointWorker)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    auto &PLHPPlantLoadSideLoop = state->dataPlnt->PlantLoop(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a little setup here
    thisCoolingPLHP->loadSidePlantLoc.loopNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisCoolingPLHP->loadSidePlantLoc.branchNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->loadSidePlantLoc);
    thisCoolingPLHP->loadSideNodes.outlet = 1;

    // the factory would've called GetOnlySingleNode for the in/out pairs on the PLHP, add another one for the loop
    // outlet setpoint node
    state->dataLoopNodes->Node.allocate(5);
    PLHPPlantLoadSideLoop.TempSetPointNodeNum = 5;

    // set up the plant setpoint conditions and test for single setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = 3.141;
    state->dataLoopNodes->Node(5).TempSetPoint = 2.718;
    EXPECT_NEAR(3.141, thisCoolingPLHP->getLoadSideOutletSetPointTemp(*state), 0.001);
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;
    EXPECT_NEAR(2.718, thisCoolingPLHP->getLoadSideOutletSetPointTemp(*state), 0.001);

    // test for dual setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPointHi = 6.282;
    state->dataLoopNodes->Node(5).TempSetPointHi = 5.436;
    EXPECT_NEAR(6.282, thisCoolingPLHP->getLoadSideOutletSetPointTemp(*state), 0.001);
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;
    EXPECT_NEAR(5.436, thisCoolingPLHP->getLoadSideOutletSetPointTemp(*state), 0.001);
}

TEST_F(EnergyPlusFixture, HeatingOutletSetpointWorker)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 3,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    auto &PLHPPlantLoadSideLoop = state->dataPlnt->PlantLoop(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a little setup here
    thisHeatingPLHP->loadSidePlantLoc.loopNum = 1;
    thisHeatingPLHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisHeatingPLHP->loadSidePlantLoc.branchNum = 1;
    thisHeatingPLHP->loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisHeatingPLHP->loadSidePlantLoc);

    thisHeatingPLHP->loadSideNodes.outlet = 1;

    // the factory would've called GetOnlySingleNode for the in/out pairs on the PLHP, add another one for the loop
    // outlet setpoint node
    state->dataLoopNodes->Node.allocate(5);
    PLHPPlantLoadSideLoop.TempSetPointNodeNum = 5;

    // test for dual setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPointHi = 30.0;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPointLo = 10.0;
    state->dataLoopNodes->Node(5).TempSetPointHi = 30.0;
    state->dataLoopNodes->Node(5).TempSetPointLo = 12.0;
    EXPECT_NEAR(10.0, thisHeatingPLHP->getLoadSideOutletSetPointTemp(*state), 0.001);
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::HeatingRB;
    EXPECT_NEAR(12.0, thisHeatingPLHP->getLoadSideOutletSetPointTemp(*state), 0.001);
}

TEST_F(EnergyPlusFixture, Initialization2_WaterSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool firstHVACIteration = true;
    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLocation);

    // call with run flag off, loose limits on node min/max
    thisCoolingPLHP->running = false;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag off, nonzero minimums
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRateMinAvail = 0.1;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRateMinAvail = 0.2;
    thisCoolingPLHP->running = false;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.1, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.2, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag off, load side flow locked
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.24;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRateMinAvail = 0.0;
    thisCoolingPLHP->running = false;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.24, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on load side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRate = 0.2;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.2, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on source side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.2;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRate = 0.0;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.2, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on both sides
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRate = 0.0;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at nonzero both
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.14;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRate = 0.13;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesWSHP(*state, firstHVACIteration);
    EXPECT_NEAR(0.14, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.13, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);
}

TEST_F(EnergyPlusFixture, OnInitLoopEquipTopologyErrorCases)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  0.0001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up a couple simple plant loops with one branch per loop-side and one component per branch
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantSupplySideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &PLHPPlantDemandSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    auto &extraPLHPPlantSupplySideComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &extraPLHPPlantDemandSideComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantSupplySideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantDemandSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    extraPLHPPlantSupplySideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    extraPLHPPlantDemandSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // init the plant component data with the name we have now from the factory call
    PLHPPlantSupplySideComp.Name = thisCoolingPLHP->name;
    PLHPPlantDemandSideComp.Name = thisCoolingPLHP->name;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // test the case where the heat pump is connected to both the supply and demand sides of the same loop
    PLHPPlantSupplySideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantDemandSideComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;
    extraPLHPPlantSupplySideComp.NodeNumIn = -1;
    extraPLHPPlantDemandSideComp.NodeNumIn = -1;
    // call for all initialization, it should abort because the coil load and supply sides were on the same loop
    EXPECT_THROW(thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation), std::runtime_error);

    // test the case where the heat pump source side cannot be found
    PLHPPlantSupplySideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantDemandSideComp.NodeNumIn = -1;
    extraPLHPPlantSupplySideComp.NodeNumIn = -1;
    extraPLHPPlantDemandSideComp.NodeNumIn = -1;
    // call for all initialization, it should abort because the coil source side inlet node is not found on plant
    EXPECT_THROW(thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation), std::runtime_error);

    // test the case where the heat pump load side cannot be found
    PLHPPlantSupplySideComp.NodeNumIn = -1;
    PLHPPlantDemandSideComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;
    extraPLHPPlantSupplySideComp.NodeNumIn = -1;
    extraPLHPPlantDemandSideComp.NodeNumIn = -1;
    // call for all initialization, it should abort because the coil load side inlet node is not found on plant
    EXPECT_THROW(thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation), std::runtime_error);

    // test the case where the heat pump source side is found, but it's on the supply side of a loop
    // still need to drop the load side onto a (extra) plant supply to trigger this condition
    PLHPPlantSupplySideComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;
    PLHPPlantDemandSideComp.NodeNumIn = -1;
    extraPLHPPlantSupplySideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    extraPLHPPlantDemandSideComp.NodeNumIn = -1;
    // call for all initialization, it should abort because the coil source was found on a supply side
    EXPECT_THROW(thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation), std::runtime_error);

    // test the case where the heat pump load side is found, but it's on the demand side of a loop
    // still need to drop the source side onto a (extra) plant demand to trigger this condition
    PLHPPlantSupplySideComp.NodeNumIn = -1;
    PLHPPlantDemandSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    extraPLHPPlantSupplySideComp.NodeNumIn = -1;
    extraPLHPPlantDemandSideComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;
    // call for all initialization, it should abort because the coil load was found on a demand side
    EXPECT_THROW(thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation), std::runtime_error);
}

TEST_F(EnergyPlusFixture, CoolingSimulate_WaterSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  0.0001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation mySourceLocation = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // call from load side location, firsthvac, no load, not running, verify the unit doesn't have any values lingering
    thisCoolingPLHP->loadSideHeatTransfer = 1000;
    thisCoolingPLHP->loadSideInletTemp = 23.0;
    thisCoolingPLHP->loadSideOutletTemp = 42.0;
    thisCoolingPLHP->powerUsage = 4.0;
    thisCoolingPLHP->sourceSideHeatTransfer = 60.0;
    thisCoolingPLHP->sourceSideInletTemp = 43.0;
    thisCoolingPLHP->sourceSideOutletTemp = 83.0;
    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    bool runFlag = false;
    thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->powerUsage, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->loadSideInletTemp, thisCoolingPLHP->loadSideOutletTemp, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->sourceSideInletTemp, thisCoolingPLHP->sourceSideOutletTemp, 0.001);

    // call from source side location, firsthvac, no load, not running, connected loop should be triggered to resimulate
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).SimLoopSideNeeded = false;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).SimLoopSideNeeded = false;
    thisCoolingPLHP->simulate(*state, mySourceLocation, firstHVAC, curLoad, runFlag);
    EXPECT_TRUE(state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).SimLoopSideNeeded);

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        firstHVAC = true;
        curLoad = -800;
        runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4183;
        Real64 constexpr specifiedLoadSetpoint = 15;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(specifiedLoadSetpoint, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(-curLoad, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        firstHVAC = true;
        curLoad = -1200;
        Real64 availableCapacity = 950.0;
        runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4183;
        Real64 constexpr specifiedLoadSetpoint = 15;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(15.597, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(availableCapacity, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    }
}

TEST_F(EnergyPlusFixture, HeatingSimulate_WaterSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  0.0001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisHeatingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // call it from the load side, but this time there is a negative (cooling) load - shouldn't try to run
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = true; // plant actually shouldn't do this but the component can be smart enough to handle it
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(loadInletTemp, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // call it from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(specifiedLoadSetpoint, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 1200;
        Real64 availableCapacity = 950.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(44.402, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(availableCapacity, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }
}

TEST_F(EnergyPlusFixture, TestConcurrentOperationChecking)
{
    state->dataEIRPlantLoopHeatPump->heatPumps.resize(4);
    EIRPlantLoopHeatPump *coil1 = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *coil2 = &state->dataEIRPlantLoopHeatPump->heatPumps[1];
    EIRPlantLoopHeatPump *coil3 = &state->dataEIRPlantLoopHeatPump->heatPumps[2];
    EIRPlantLoopHeatPump *coil4 = &state->dataEIRPlantLoopHeatPump->heatPumps[3];

    // pair up the last two
    coil3->companionHeatPumpCoil = coil4;
    coil4->companionHeatPumpCoil = coil3;

    // set all of them to running
    coil1->running = true;
    coil2->running = true;
    coil3->running = true;
    coil4->running = true;

    // check to warn about concurrent operation
    EIRPlantLoopHeatPump::checkConcurrentOperation(*state);

    // that will just add a recurring warning to the end, so to check whether
    //  a warning was actually made, I'll just check the warning index values
    ASSERT_EQ(0, coil1->recurringConcurrentOperationWarningIndex);
    ASSERT_EQ(0, coil2->recurringConcurrentOperationWarningIndex);
    ASSERT_EQ(1, coil3->recurringConcurrentOperationWarningIndex);
    ASSERT_EQ(1, coil4->recurringConcurrentOperationWarningIndex);
}

TEST_F(EnergyPlusFixture, ConstructionFullObjectsHeatingAndCooling_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  2,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  0.001,",
                                                      "  0.001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  2,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the heating side
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRHeating, thisHeatingPLHP->EIRHPType);
    EXPECT_EQ(thisCoolingPLHP, thisHeatingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisHeatingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisHeatingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisHeatingPLHP->powerRatioFuncPLRCurveIndex);

    // validate the cooling side
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRCooling, thisCoolingPLHP->EIRHPType);
    EXPECT_EQ(thisHeatingPLHP, thisCoolingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisCoolingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncPLRCurveIndex);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP HEATING SIDE"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP COOLING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, CoolingSimulate_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  1,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // call from load side location, firsthvac, no load, not running, verify the unit doesn't have any values lingering
    thisCoolingPLHP->loadSideHeatTransfer = 1000;
    thisCoolingPLHP->loadSideInletTemp = 23.0;
    thisCoolingPLHP->loadSideOutletTemp = 42.0;
    thisCoolingPLHP->powerUsage = 4.0;
    thisCoolingPLHP->sourceSideHeatTransfer = 60.0;
    thisCoolingPLHP->sourceSideInletTemp = 43.0;
    thisCoolingPLHP->sourceSideOutletTemp = 83.0;
    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    bool runFlag = false;
    thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->powerUsage, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->loadSideInletTemp, thisCoolingPLHP->loadSideOutletTemp, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->sourceSideInletTemp, thisCoolingPLHP->sourceSideOutletTemp, 0.001);

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        firstHVAC = true;
        curLoad = -800;
        runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4183;
        Real64 constexpr specifiedLoadSetpoint = 15;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(specifiedLoadSetpoint, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(-curLoad, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        firstHVAC = true;
        curLoad = -1200;
        Real64 availableCapacity = 950.0;
        runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4183;
        Real64 constexpr specifiedLoadSetpoint = 15;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(15.597, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(availableCapacity, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    }
}

TEST_F(EnergyPlusFixture, HeatingSimulate_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  1,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  1.0,",
                                                      "  HeatingCapacity,",
                                                      "  Load,",
                                                      "  ConstantFlow,",
                                                      "  0.5;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisHeatingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // call it from the load side, but this time there is a negative (cooling) load - shouldn't try to run
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = true; // plant actually shouldn't do this but the component can be smart enough to handle it
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(loadInletTemp, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // call it from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(specifiedLoadSetpoint, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 1200;
        Real64 availableCapacity = 950.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(44.402, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(availableCapacity, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is no load (still firsthvac)
    {
        bool firstHVAC = true;
        Real64 curLoad = 0.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(45.0, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(30.0, thisHeatingPLHP->sourceSideOutletTemp, 0.001);
    }

    // now we can call it again from the load side, but this time there is a very low load (still firsthvac)
    {
        bool firstHVAC = true;
        Real64 curLoad = 100.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR((curLoad / thisHeatingPLHP->referenceCOP) * 0.95 * 0.95, thisHeatingPLHP->powerUsage, 0.001);
    }
}

TEST_F(EnergyPlusFixture, CoolingConstructionFullyAutoSized_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // validate the cooling side
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpEIRCooling, thisCoolingPLHP->EIRHPType);
    EXPECT_EQ(nullptr, thisCoolingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisCoolingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(1, thisCoolingPLHP->powerRatioFuncPLRCurveIndex);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "fake"), std::runtime_error);
    EXPECT_THROW(EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP COOLING SIDE"), std::runtime_error);
}

TEST_F(EnergyPlusFixture, ClearState)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");
    EXPECT_EQ(state->dataEIRPlantLoopHeatPump->heatPumps.size(), 1u);

    // test that vector is cleared
    state->dataEIRPlantLoopHeatPump->clear_state();
    EXPECT_EQ(state->dataEIRPlantLoopHeatPump->heatPumps.size(), 0u);
}

TEST_F(EnergyPlusFixture, Initialization2_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.001,",
                                                      "  1,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool firstHVACIteration = true;
    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLocation);

    // call with run flag off, loose limits on node min/max
    thisCoolingPLHP->running = false;
    Real64 constexpr currentLoad = 0.0;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag off, nonzero minimums
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRateMinAvail = 0.1;
    thisCoolingPLHP->running = false;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.1, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag off, load side flow locked
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.24;
    thisCoolingPLHP->running = false;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.24, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on load side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.0;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on source side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.2;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.2, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(1.29, thisCoolingPLHP->sourceSideMassFlowRate, 0.1);

    // call with run flag ON, flow locked at zero on both sides
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.0;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at nonzero both
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 0.14;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.14, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(1.29, thisCoolingPLHP->sourceSideMassFlowRate, 0.1);
}

TEST_F(EnergyPlusFixture, TestSizing_FullyAutosizedCoolingWithCompanion_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  AirSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  2.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // validate that we have the right ones
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(2);

    Real64 constexpr plantSizingLoadVolFlow = 0.01;
    Real64 constexpr plantSizingLoadDeltaT = 1.0;

    Real64 constexpr plantSizingSrcDeltaT = 10.0;

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.01;
    state->dataSize->PlantSizData(1).DeltaT = 1.0;
    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.03;
    state->dataSize->PlantSizData(2).DeltaT = 1.0;

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop1supplyComponent2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(2);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1supplyComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop1supplyComponent2.Name = thisCoolingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop1supplyComponent2.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 2);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // assign the plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;

    Real64 constexpr sourceSideInitTemp = 20.0;
    Real64 constexpr sourceSideHumRat = 0.0;
    Real64 expectedLoadCp = 4197.93;
    Real64 expectedLoadRho = 999.898;
    Real64 expectedSourceCp = Psychrometrics::PsyCpAirFnW(sourceSideHumRat);
    Real64 expectedSourceRho = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, sourceSideInitTemp, sourceSideHumRat);
    Real64 expectedLoadFlow = plantSizingLoadVolFlow;
    Real64 expectedCapacity = expectedLoadRho * expectedLoadFlow * expectedLoadCp * plantSizingLoadDeltaT;
    Real64 expectedSourceLoad = 0.0;
    expectedSourceLoad = expectedCapacity * (1 + 1 / thisCoolingPLHP->referenceCOP);
    Real64 expectedSourceFlow = expectedSourceLoad / (expectedSourceCp * expectedSourceRho * plantSizingSrcDeltaT);
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideASHP(*state);
    EXPECT_NEAR(expectedLoadFlow, thisCoolingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedSourceFlow, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.1);
    EXPECT_NEAR(expectedCapacity, thisCoolingPLHP->referenceCapacity, 0.0001);

    // with a sizing run complete, we can also go ahead and get the design capacities...
    // they should be nonzero for the load side of things
    Real64 tmpMin = -1.0, tmpMax = -1.0, tmpOpt = -1.0;
    thisCoolingPLHP->getDesignCapacities(*state, myCoolingLoadLocation, tmpMax, tmpMin, tmpOpt);
    EXPECT_NEAR(0.0, tmpMin, 0.001);
    EXPECT_NEAR(expectedCapacity, tmpMax, 0.001);
    EXPECT_NEAR(expectedCapacity, tmpOpt, 0.001);

    // we can reset things and do a few more corner cases here

    // lets just try it again but with the plant sizing data set to zero flow, it should try to use the companion
    // but the companion isn't sized yet, so it should get zero conditions
    thisCoolingPLHP->loadSideDesignVolFlowRate = DataSizing::AutoSize;
    thisCoolingPLHP->sourceSideDesignVolFlowRate = DataSizing::AutoSize;
    thisCoolingPLHP->referenceCapacity = DataSizing::AutoSize;
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.0;
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideASHP(*state);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->referenceCapacity, 0.0001);

    // but now let's try to size the heating coil, which will try to use the cooling coil's sized data
    thisCoolingPLHP->loadSideDesignVolFlowRate = expectedLoadFlow;
    thisCoolingPLHP->sourceSideDesignVolFlowRate = expectedSourceFlow;
    thisCoolingPLHP->referenceCapacity = expectedCapacity;
    expectedCapacity = expectedLoadRho * expectedLoadFlow * expectedLoadCp * plantSizingLoadDeltaT;
    expectedSourceLoad = expectedCapacity * (1 - 1 / thisHeatingPLHP->referenceCOP);
    expectedSourceFlow = expectedSourceLoad / (expectedSourceCp * expectedSourceRho * plantSizingSrcDeltaT);
    thisHeatingPLHP->sizeLoadSide(*state);
    thisHeatingPLHP->sizeSrcSideASHP(*state);
    EXPECT_NEAR(expectedLoadFlow, thisHeatingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedSourceFlow, thisHeatingPLHP->sourceSideDesignVolFlowRate, 0.1);
    EXPECT_NEAR(expectedCapacity, thisHeatingPLHP->referenceCapacity, 0.0001);
}

TEST_F(EnergyPlusFixture, TestSizing_HardsizedFlowAutosizedCoolingWithCompanion_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  Autosize,",
                                                      "  2.0,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  AirSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  Autosize,",
                                                      "  2.0,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // validate that we have the right ones
    EXPECT_EQ("HP COOLING SIDE", thisCoolingPLHP->name);
    EXPECT_EQ("HP HEATING SIDE", thisHeatingPLHP->name);

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(2);

    Real64 constexpr plantSizingLoadVolFlow = 0.01;
    Real64 constexpr plantSizingLoadDeltaT = 1.0;

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.01;
    state->dataSize->PlantSizData(1).DeltaT = 1.0;
    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.03;
    state->dataSize->PlantSizData(2).DeltaT = 1.0;

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop1supplyComponent2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(2);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1supplyComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop1supplyComponent2.Name = thisCoolingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop1supplyComponent2.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 2);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // assign the plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;

    Real64 expectedLoadCp = 4197.93;
    Real64 expectedLoadRho = 999.898;
    Real64 expectedLoadFlow = plantSizingLoadVolFlow;
    Real64 expectedCapacity = expectedLoadRho * expectedLoadFlow * expectedLoadCp * plantSizingLoadDeltaT;
    Real64 expectedSourceFlow = 2.0;
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideASHP(*state);
    EXPECT_NEAR(expectedLoadFlow, thisCoolingPLHP->loadSideDesignVolFlowRate, 0.0001);
    EXPECT_NEAR(expectedSourceFlow, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.1);
    EXPECT_NEAR(expectedCapacity, thisCoolingPLHP->referenceCapacity, 0.0001);

    // with a sizing run complete, we can also go ahead and get the design capacities...
    // they should be nonzero for the load side of things
    Real64 tmpMin = -1.0, tmpMax = -1.0, tmpOpt = -1.0;
    thisCoolingPLHP->getDesignCapacities(*state, myCoolingLoadLocation, tmpMax, tmpMin, tmpOpt);
    EXPECT_NEAR(0.0, tmpMin, 0.001);
    EXPECT_NEAR(expectedCapacity, tmpMax, 0.001);
    EXPECT_NEAR(expectedCapacity, tmpOpt, 0.001);
}

TEST_F(EnergyPlusFixture, TestSizing_AutosizedFlowWithCompanion_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  AirSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  ,",
                                                      "  Autosize,",
                                                      "  2.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // We'll set up two plant loops: a load and a source loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(2);

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DeltaT = 25.0;
    state->dataSize->PlantSizData(2).DeltaT = 20.0;

    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop1supplyComponent2 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(2);

    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1supplyComponent2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    loop1supplyComponent1.Name = thisHeatingPLHP->name;
    loop1supplyComponent2.Name = thisCoolingPLHP->name;

    loop1supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    loop1supplyComponent2.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 2);

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // assign the plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;

    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    Real64 expectedClgSourceFlow = 86.71;
    Real64 expectedHtgSourceFlow = 21.68; // changed from 86.71 due to issue#10381;
    EXPECT_NEAR(expectedClgSourceFlow, thisCoolingPLHP->sourceSideDesignVolFlowRate, 0.1);
    EXPECT_NEAR(expectedHtgSourceFlow, thisHeatingPLHP->sourceSideDesignVolFlowRate, 0.1);
}

TEST_F(EnergyPlusFixture, Test_DoPhysics)
{

    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  0.002,",
                                                      "  ,",
                                                      "  20000,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  CapCurveFuncTemp,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  EIRCurveFuncPLR;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  0.002,",
                                                      "  ,",
                                                      "  20000,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  CapCurveFuncTemp,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  EIRCurveFuncPLR;",
                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideLoop = state->dataPlnt->PlantLoop(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);

    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a little setup here
    thisCoolingPLHP->loadSidePlantLoc.loopNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisCoolingPLHP->loadSidePlantLoc.branchNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->loadSidePlantLoc);
    thisCoolingPLHP->loadSideNodes.outlet = 1;
    thisCoolingPLHP->sourceSidePlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->sourceSidePlantLoc);

    // the factory would've called GetOnlySingleNode for the in/out pairs on the PLHP, add another one for the loop
    // outlet setpoint node
    state->dataLoopNodes->Node.allocate(5);
    PLHPPlantLoadSideLoop.TempSetPointNodeNum = 5;

    // set up the plant setpoint conditions and test for single setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = 3.141;
    state->dataLoopNodes->Node(5).TempSetPoint = 2.718;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    // test for dual setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPointHi = 6.282;
    state->dataLoopNodes->Node(5).TempSetPointHi = 5.436;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    state->dataHVACGlobal->TimeStepSys = 60;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    Real64 curLoad = -10000;

    thisCoolingPLHP->loadSideMassFlowRate = 0.3;
    thisCoolingPLHP->sourceSideMassFlowRate = 0.8;
    thisCoolingPLHP->loadSideInletTemp = 20;
    thisCoolingPLHP->sourceSideInletTemp = 20;
    thisCoolingPLHP->doPhysics(*state, curLoad);

    EXPECT_NEAR(thisCoolingPLHP->loadSideOutletTemp, 12.00, 0.1);
    EXPECT_NEAR(thisCoolingPLHP->sourceSideOutletTemp, 47.90, 0.1);
}

TEST_F(EnergyPlusFixture, Test_DoPhysics_AWHP)
{

    std::string const idf_objects =
        delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                          "  hp cooling side,",
                          "  node 1,",
                          "  node 2,",
                          "  AirSource,",
                          "  node 3,",
                          "  node 4,",
                          "  ,",
                          "  ,",
                          "  hp heating side,",
                          "  0.005,",
                          "  0.002,",
                          "  ,",
                          "  20000,",
                          "  3.0,",
                          "  1,",
                          "  CapCurveFuncTemp,",
                          "  EIRCurveFuncTemp,",
                          "  EIRCurveFuncPLR;",

                          "HeatPump:PlantLoop:EIR:Heating,",
                          "  hp heating side,",
                          "  node 5,",
                          "  node 6,",
                          "  AirSource,",
                          "  node 7,",
                          "  node 8,",
                          "  ,",
                          "  ,",
                          "  hp cooling side,",
                          "  0.005,",
                          "  0.002,",
                          "  ,",
                          "  20000,",
                          "  3.0,",
                          "  1,",
                          "  CapCurveFuncTemp,",
                          "  EIRCurveFuncTemp,",
                          "  EIRCurveFuncPLR;",

                          "HeatPump:AirToWater,",
                          "test_AWHP, !- Name",
                          ", !- Availability Schedule Name Heating",
                          ", !- Availability Schedule Name Cooling",
                          "Load , !- Operating Mode Control Method",
                          "SingleMode, !- Operating Mode Control Option for Multiple Unit",
                          ", !- Operating Mode Control Schedule Name",
                          "20 , !-  Rated Inlet Air Temperature in Heating Mode",
                          "0.002, !-  Rated Air Flow Rate in Heating Mode",
                          "50 , !-  Rated Leaving Water Temperature in Heating Mode",
                          "0.005, !-  Rated Water Flow Rate in Heating Mode",
                          "-20, !-  Minimum Outdoor Air Temperature in Heating Mode",
                          "25, !-  Maximum Outdoor Air Temperature in Heating Mode",
                          ",    !- Minimum Leaving Water Temperature Curve Name in Heating Mode",
                          ",    !- Maximum Leaving Water Temperature Curve Name in Heating Mode",
                          ",    !- Minimum Part Load Ratio",
                          "1.0, !-  Sizing Factor for Heating",
                          "25, !-  Rated Inlet Air Temperature in Cooling Mode",
                          "0.002, !-  Rated Air Flow Rate in Cooling Mode",
                          "22 , !-  Rated Leaving Water Temperature in Cooling Mode",
                          "0.005, !-  Rated Water Flow Rate in Cooling Mode",
                          "18 , !-  Minimum Outdoor Air Temperature in Cooling Mode",
                          "40, !-  Maximum Outdoor Air Temperature in Cooling Mode",
                          ",    !- Minimum Leaving Water Temperature Curve Name in Cooling Mode",
                          ",    !- Maximum Leaving Water Temperature Curve Name in Cooling Mode",
                          "0.9, !-  Sizing Factor for Cooling",
                          "Outdoor Air Inlet Node , !-  Air Inlet Node Name",
                          "Outdoor Air Outlet Node, !-  Air Outlet Node Name",
                          "Heating Coil Load Loop Intermediate Node, !-  Hot Water Inlet Node Name",
                          "Heating Coil Load Loop Supply Outlet Node, !-  Hot Water Outlet Node Name",
                          "Cooling Coil Load Loop Intermediate Node , !-  Chilled Water Inlet Node Name",
                          "Cooling Coil Load Loop Supply Outlet Node , !-  Chilled Water Outlet Node Name",
                          "10, !- Maximum Outdoor Dry Bulb Temperature For Defrost Operation",
                          "Timed, !-  Heat Pump Defrost Control",
                          "0.2, !-  Defrost Time Period Fraction",
                          "150, !-  Resistive Defrost Heater Capacity",
                          ", !-  Defrost Energy Input Ratio Function of Temperature Curve Name",
                          "1 , !- Compressor Multiplier",
                          "FixedSpeed , !- Control Type",
                          "100 , !- Crankcase Heater Capacity",
                          "EIRCurveFuncPLR, !- Crankcase Heater Capacity Function of Temperature Curve Name",
                          "20 , !- Maximum Ambient Temperature for Crankcase Heater Operation",
                          "1 , !- Number of Speeds for Heating",
                          "20000, !-  Rated Heating Capacity at Speed 1",
                          "3, !-  Rated COP for Heating at Speed 1",
                          "CapCurveFuncTemp, !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 1",
                          "EIRCurveFuncTemp, !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1",
                          "EIRCurveFuncPLR, !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 1",
                          ", !-  Rated Heating Capacity at Speed 2",
                          ", !-  Rated COP for Heating at Speed 2",
                          ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 2",
                          ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2",
                          ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 2",
                          ", !-  Rated Heating Capacity at Speed 3",
                          ", !-  Rated COP for Heating at Speed 3",
                          ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 3",
                          ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3",
                          ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 3",
                          ", !-  Rated Heating Capacity at Speed 4",
                          ", !-  Rated COP for Heating at Speed 4",
                          ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 4",
                          ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4",
                          ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 4",
                          ", !-  Rated Heating Capacity at Speed 5",
                          ", !-  Rated COP for Heating at Speed 5",
                          ", !-  Normalized Heating Capacity Function of Temperature Curve Name at Speed 5",
                          ", !-  Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5",
                          ", !-  Heating Energy Input Ratio Function of PLR Curve Name at Speed 5",
                          ", !-  Booster Mode On Heating",
                          ", !-  Rated Heating Capacity in Booster Mode",
                          ", !-  Rated Heating COP in Booster Mode",
                          ", !-  Normalized Heating Capacity Function of Temperature Curve Name in Booster Mode",
                          ", !-  Heating Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
                          ", !-  Heating Energy Input Ratio Function of PLR Curve Name in Booster Mode",
                          "1, !-  Number of Speeds for Cooling",
                          "20000, !-  Rated Cooling Capacity at Speed 1",
                          "3, !-  Rated COP for Cooling at Speed 1",
                          "CapCurveFuncTemp, !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1",
                          "EIRCurveFuncTemp, !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1",
                          "EIRCurveFuncPLR, !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1",
                          ", !-  Rated Cooling Capacity at Speed 2",
                          ", !-  Rated COP for Cooling at Speed 2",
                          ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2",
                          ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2",
                          ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2",
                          ", !-  Rated Cooling Capacity at Speed 3",
                          ", !-  Rated COP for Cooling at Speed 3",
                          ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3",
                          ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3",
                          ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3",
                          ", !-  Rated Cooling Capacity at Speed 4",
                          ", !-  Rated COP for Cooling at Speed 4",
                          ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4",
                          ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4",
                          ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4",
                          ", !-  Rated Cooling Capacity at Speed 5",
                          ", !-  Rated COP for Cooling at Speed 5",
                          ", !-  Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5",
                          ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5",
                          ", !-  Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5",
                          ", !-  Booster Mode On Cooling",
                          ", !-  Rated Cooling Capacity in Booster Mode",
                          ", !-  Rated Cooling COP in Booster Mode",
                          ", !-  Normalized Cooling Capacity Function of Temperature Curve Name in Booster Mode",
                          ", !-  Cooling Energy Input Ratio Function of Temperature Curve Name in Booster Mode",
                          "; !-  Cooling Energy Input Ratio Function of PLR Curve Name in Booster Mode",

                          "Curve:Biquadratic,",
                          "  CapCurveFuncTemp,",
                          "  1.0,",
                          "  0.0,",
                          "  0.0,",
                          "  0.0,",
                          "  0.0,",
                          "  0.0,",
                          "  5.0,",
                          "  10.0,",
                          "  24.0,",
                          "  35.0,",
                          "  ,",
                          "  ,",
                          "  Temperature,",
                          "  Temperature,",
                          "  Dimensionless;",
                          "Curve:Biquadratic,",
                          "  EIRCurveFuncTemp,",
                          "  1.0,",
                          "  0.0,",
                          "  0.0,",
                          "  1.0,",
                          "  0.0,",
                          "  0.0,",
                          "  5.0,",
                          "  10.0,",
                          "  24.0,",
                          "  35.0,",
                          "  ,",
                          "  ,",
                          "  Temperature,",
                          "  Temperature,",
                          "  Dimensionless;",
                          "Curve:Quadratic,",
                          "  EIRCurveFuncPLR,",
                          "  1.0,",
                          "  0.0,",
                          "  0.0,",
                          "  0.0,",
                          "  1.0;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 5;
    state->dataPlnt->PlantLoop.allocate(5);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &AWHPPlantLoadSideLoop = state->dataPlnt->PlantLoop(1);
    auto &AWHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    AWHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "Air";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);

    // call the factory with a valid name to trigger reading inputs
    HeatPumpAirToWater::factory(*state, DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling, "test_AWHP");

    state->dataPlnt->PlantLoop(3).FluidName = "WATER";
    state->dataPlnt->PlantLoop(3).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideLoop = state->dataPlnt->PlantLoop(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterCooling;
    // then the source side

    state->dataPlnt->PlantLoop(4).FluidName = "Air";
    state->dataPlnt->PlantLoop(4).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(4).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(4).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(4).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(4).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);

    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    state->dataPlnt->PlantLoop(5).FluidName = "WATER";
    state->dataPlnt->PlantLoop(5).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(5).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(5).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(5).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(5).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &AWHPHeatPlantLoadSideComp = state->dataPlnt->PlantLoop(5).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    AWHPHeatPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpAirToWaterHeating;

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "hp cooling side");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataHeatPumpAirToWater->heatPumps.size());
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    HeatPumpAirToWater *thisCoolingAWHP = &state->dataHeatPumpAirToWater->heatPumps[0];
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    HeatPumpAirToWater *thisHeatingAWHP = &state->dataHeatPumpAirToWater->heatPumps[1];
    thisCoolingAWHP->companionHeatPumpCoil = thisHeatingAWHP;
    thisHeatingAWHP->companionHeatPumpCoil = thisCoolingAWHP;

    // do a little setup here
    thisCoolingAWHP->loadSidePlantLoc.loopNum = 1;
    thisCoolingAWHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisCoolingAWHP->loadSidePlantLoc.branchNum = 1;
    thisCoolingAWHP->loadSidePlantLoc.compNum = 1;
    thisHeatingAWHP->loadSidePlantLoc.loopNum = 5;
    thisHeatingAWHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisHeatingAWHP->loadSidePlantLoc.branchNum = 1;
    thisHeatingAWHP->loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingAWHP->loadSidePlantLoc);
    thisCoolingAWHP->loadSideNodes.outlet = 1;
    thisCoolingAWHP->sourceSidePlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingAWHP->sourceSidePlantLoc);

    thisCoolingPLHP->loadSidePlantLoc.loopNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisCoolingPLHP->loadSidePlantLoc.branchNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->loadSidePlantLoc);
    thisCoolingPLHP->loadSideNodes.outlet = 1;
    thisCoolingPLHP->sourceSidePlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->sourceSidePlantLoc);

    // the factory would've called GetOnlySingleNode for the in/out pairs on the PLHP, add another one for the loop
    // outlet setpoint node
    state->dataLoopNodes->Node.allocate(10);
    AWHPPlantLoadSideLoop.TempSetPointNodeNum = 5;
    PLHPPlantLoadSideLoop.TempSetPointNodeNum = 10;

    // set up the plant setpoint conditions and test for single setpoint operation
    AWHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    AWHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingAWHP->loadSideNodes.outlet).TempSetPoint = 3.141;
    state->dataLoopNodes->Node(5).TempSetPoint = 2.718;
    AWHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    // test for dual setpoint operation
    AWHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand;
    AWHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingAWHP->loadSideNodes.outlet).TempSetPointHi = 6.282;
    state->dataLoopNodes->Node(5).TempSetPointHi = 5.436;
    AWHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    // set up the plant setpoint conditions and test for single setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = 3.141;
    state->dataLoopNodes->Node(10).TempSetPoint = 2.718;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    // test for dual setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingAWHP->loadSideNodes.outlet).TempSetPointHi = 6.282;
    state->dataLoopNodes->Node(10).TempSetPointHi = 5.436;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    state->dataHVACGlobal->TimeStepSys = 60;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    Real64 curLoad = -10000;

    thisCoolingAWHP->loadSideMassFlowRate = 0.3;
    thisCoolingAWHP->sourceSideMassFlowRate = 0.8;
    thisCoolingAWHP->loadSideInletTemp = 20;
    thisCoolingAWHP->sourceSideInletTemp = 20;
    thisCoolingAWHP->doPhysics(*state, curLoad);

    thisCoolingPLHP->loadSideMassFlowRate = 0.3;
    thisCoolingPLHP->sourceSideMassFlowRate = 0.8;
    thisCoolingPLHP->loadSideInletTemp = 20;
    thisCoolingPLHP->sourceSideInletTemp = 20;
    thisCoolingPLHP->doPhysics(*state, curLoad);

    EXPECT_NEAR(thisCoolingAWHP->loadSideOutletTemp, thisCoolingPLHP->loadSideOutletTemp, 0.1);
    // this value is from the Test_DoPhysics with WaterSource changed to AirSource
    EXPECT_NEAR(thisCoolingAWHP->sourceSideOutletTemp, thisCoolingPLHP->sourceSideOutletTemp, 0.1);
    // fixme: add cases with 2 or more speed levels
}

TEST_F(EnergyPlusFixture, CoolingMetering)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  0.0001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation);

    int NumFound;
    std::string TypeOfComp = "HeatPump:PlantLoop:EIR:Cooling";
    std::string NameOfComp = thisCoolingPLHP->name;
    int NumVariables = GetNumMeteredVariables(*state, TypeOfComp, NameOfComp);
    Array1D<OutputProcessor::MeteredVar> meteredVars(NumVariables); // Variable Types (1=integer, 2=real, 3=meter)

    NumFound = GetMeteredVariables(*state, NameOfComp, meteredVars);

    EXPECT_EQ(2, NumFound);
    EXPECT_ENUM_EQ(meteredVars(1).resource, Constant::eResource::EnergyTransfer); // ENERGYTRANSFER
    EXPECT_ENUM_EQ(meteredVars(1).endUseCat, OutputProcessor::EndUseCat::Invalid);
    EXPECT_ENUM_EQ(meteredVars(1).group, OutputProcessor::Group::Plant);
    EXPECT_ENUM_EQ(meteredVars(2).resource, Constant::eResource::Electricity); // Electric
    EXPECT_ENUM_EQ(meteredVars(2).endUseCat, OutputProcessor::EndUseCat::Cooling);
    EXPECT_ENUM_EQ(meteredVars(2).group, OutputProcessor::Group::Plant);
}

TEST_F(EnergyPlusFixture, HeatingMetering)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0001,",
                                                      "  0.0001,",
                                                      "  ,",
                                                      "  1000,",
                                                      "  3.14,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRHeating, "HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisHeatingPLHP->sourceSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisHeatingPLHP->onInitLoopEquip(*state, myLoadLocation);

    int NumFound;

    std::string TypeOfComp = "HeatPump:PlantLoop:EIR:Heating";
    std::string NameOfComp = thisHeatingPLHP->name;
    int NumVariables = GetNumMeteredVariables(*state, TypeOfComp, NameOfComp);

    Array1D_int VarIndexes(NumVariables);                           // Variable Numbers
    Array1D<OutputProcessor::MeteredVar> meteredVars(NumVariables); // Variable Types (1=integer, 2=real, 3=meter)

    NumFound = GetMeteredVariables(*state, NameOfComp, meteredVars);

    EXPECT_EQ(2, NumFound);
    EXPECT_ENUM_EQ(meteredVars(1).resource, Constant::eResource::EnergyTransfer); // ENERGYTRANSFER
    EXPECT_ENUM_EQ(meteredVars(1).endUseCat, OutputProcessor::EndUseCat::Invalid);
    EXPECT_ENUM_EQ(meteredVars(1).group, OutputProcessor::Group::Plant);
    EXPECT_ENUM_EQ(meteredVars(2).resource, Constant::eResource::Electricity); // Electric
    EXPECT_ENUM_EQ(meteredVars(2).endUseCat, OutputProcessor::EndUseCat::Heating);
    EXPECT_ENUM_EQ(meteredVars(2).group, OutputProcessor::Group::Plant);
}

TEST_F(EnergyPlusFixture, TestOperatingFlowRates_FullyAutosized_AirSource)
{
    std::string const idf_objects =
        delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                          "  hp cooling side,",
                          "  node 1,",
                          "  node 2,",
                          "  AirSource,",
                          "  node 3,",
                          "  node 4,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  Autosize,",
                          "  Autosize,",
                          "  ,",
                          "  Autosize,",
                          "  3.14,",
                          "  ,",
                          "  dummyCurve,",
                          "  dummyCurve,",
                          "  dummyCurve,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ,",
                          "  ThermoCapFracCurve;",

                          "Curve:Linear, ThermoCapFracCurve, 0.0, 0.06, 0.0, 10.0, 0.0, 1.0, Dimensionless, Dimensionless;",
                          "Curve:Linear,",
                          "  dummyCurve,",
                          "  1,",
                          "  0,",
                          "  1,",
                          "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool firstHVACIteration = true;
    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;

    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.010;
    state->dataSize->PlantSizData(1).DeltaT = 1.0;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLocation);

    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // assign the plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;

    // call with run flag ON, flow locked at nonzero both
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = 1.0;
    thisCoolingPLHP->running = true;
    thisCoolingPLHP->sizeLoadSide(*state);
    thisCoolingPLHP->sizeSrcSideASHP(*state);
    Real64 constexpr currentLoad = 0.0;
    thisCoolingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(1.0, thisCoolingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_TRUE(thisCoolingPLHP->running);

    // test thermosiphon model
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = 10.0;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).Temp = 6.0;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = 6.0;
    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 12.0; // condenser inlet temp > evap outlet temp
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).TempSetPointNodeNum = thisCoolingPLHP->loadSideNodes.outlet;
    Real64 CurLoad = -20000.0;
    bool RunFlag = true;
    EnergyPlus::PlantLocation calledFromLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    thisCoolingPLHP->simulate(*state, calledFromLocation, firstHVACIteration, CurLoad, RunFlag);
    EXPECT_GT(thisCoolingPLHP->partLoadRatio, 0.4);    // load is large
    EXPECT_EQ(thisCoolingPLHP->thermosiphonStatus, 0); // thermosiphon is off
    EXPECT_GT(thisCoolingPLHP->powerUsage, 6300.0);    // power is non-zero

    state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 5.0; // condenser inlet temp < evap outlet temp

    thisCoolingPLHP->simulate(*state, calledFromLocation, firstHVACIteration, CurLoad, RunFlag);
    EXPECT_GT(thisCoolingPLHP->partLoadRatio, 0.4);    // load is large
    EXPECT_EQ(thisCoolingPLHP->thermosiphonStatus, 0); // thermosiphon is off
    EXPECT_GT(thisCoolingPLHP->powerUsage, 6300.0);    // power is non-zero

    CurLoad /= 20.0; // reduce load such that thermosiphon can meet load
    thisCoolingPLHP->simulate(*state, calledFromLocation, firstHVACIteration, CurLoad, RunFlag);
    EXPECT_GT(thisCoolingPLHP->partLoadRatio, 0.02);   // load is small
    EXPECT_EQ(thisCoolingPLHP->thermosiphonStatus, 1); // thermosiphon is on
    EXPECT_EQ(thisCoolingPLHP->powerUsage, 0.0);       // power is zero
}

TEST_F(EnergyPlusFixture, Test_Curve_Negative_Energy)
{

    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  0.002,",
                                                      "  ,",
                                                      "  20000,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  CapCurveFuncTemp,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  badEIRCurveFuncPLR;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  WaterSource,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  ,",
                                                      "  ,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  0.002,",
                                                      "  ,",
                                                      "  20000,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  CapCurveFuncTemp,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  EIRCurveFuncPLR;",
                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"
                                                      "Curve:Quadratic,",
                                                      "  badEIRCurveFuncPLR,",
                                                      "  -1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideLoop = state->dataPlnt->PlantLoop(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);

    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a little setup here
    thisCoolingPLHP->loadSidePlantLoc.loopNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisCoolingPLHP->loadSidePlantLoc.branchNum = 1;
    thisCoolingPLHP->loadSidePlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->loadSidePlantLoc);
    thisCoolingPLHP->loadSideNodes.outlet = 1;
    thisCoolingPLHP->sourceSidePlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, thisCoolingPLHP->sourceSidePlantLoc);

    // the factory would've called GetOnlySingleNode for the in/out pairs on the PLHP, add another one for the loop
    // outlet setpoint node
    state->dataLoopNodes->Node.allocate(5);
    PLHPPlantLoadSideLoop.TempSetPointNodeNum = 5;

    // set up the plant setpoint conditions and test for single setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = 3.141;
    state->dataLoopNodes->Node(5).TempSetPoint = 2.718;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    // test for dual setpoint operation
    PLHPPlantLoadSideLoop.LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPointHi = 6.282;
    state->dataLoopNodes->Node(5).TempSetPointHi = 5.436;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CoolingRB;

    state->dataHVACGlobal->TimeStepSys = 60;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    Real64 curLoad = -10000;

    thisCoolingPLHP->loadSideMassFlowRate = 0.3;
    thisCoolingPLHP->sourceSideMassFlowRate = 0.8;
    thisCoolingPLHP->loadSideInletTemp = 20;
    thisCoolingPLHP->sourceSideInletTemp = 20;
    thisCoolingPLHP->doPhysics(*state, curLoad);
    thisCoolingPLHP->report(*state);

    // Power and energy are now zero since the curve is reset with zero values
    EXPECT_NEAR(thisCoolingPLHP->powerUsage, 0.000, 1e-3);
    EXPECT_NEAR(thisCoolingPLHP->powerEnergy, 0.000, 1e-3);

    EXPECT_NEAR(thisCoolingPLHP->sourceSideHeatTransfer, 10000.000, 1e-3);
    EXPECT_NEAR(thisCoolingPLHP->sourceSideEnergy, 2160000000.000, 1e-3);

    EXPECT_NEAR(thisCoolingPLHP->loadSideOutletTemp, 12.095, 1e-3);

    EXPECT_NEAR(thisCoolingPLHP->sourceSideOutletTemp, 22.989, 1e-3);

    EXPECT_EQ(thisCoolingPLHP->eirModFPLRErrorIndex, 1);

    EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 1);
    EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
    EXPECT_EQ(state->dataErrTracking->LastSevereError, "HeatPump:PlantLoop:EIR:Cooling \"HP COOLING SIDE\":");

    EXPECT_EQ(state->dataErrTracking->RecurringErrors(1).Count, 1);
    EXPECT_EQ(state->dataErrTracking->RecurringErrors(1).Message,
              " ** Warning ** HeatPump:PlantLoop:EIR:Cooling \"HP COOLING SIDE\": EIR Modifier curve (function of PLR) output is negative warning "
              "continues...");
}

TEST_F(EnergyPlusFixture, GAHP_HeatingConstructionFullObjectsNoCompanion)
{
    std::string const idf_objects = delimited_string({"HeatPump:AirToWater:FuelFired:Heating,",
                                                      "  Fuel Fired hp heating side, ! A1",
                                                      "  node w1, ! A2",
                                                      "  node w2, ! A3",
                                                      "  node a3, ! A4",
                                                      "  , ! A5 Comanion coil",
                                                      "  NaturalGas, ! A6 fuel type",
                                                      "  GAHP_Custom, ! A7 end use cat",
                                                      "  3000, ! N1 capacity",
                                                      "  1.5, ! N2 nominal COP",
                                                      "  0.005, ! N3 design flow rate",
                                                      "  60, ! N4 Design Supply Temp",
                                                      "  11.1, ! N5 Design Lift",
                                                      "  1.0, ! N6 sizing factor",
                                                      " NotModulated, ! A8 flow mode",
                                                      " DryBulb, ! A9 oa temp var type",
                                                      " EnteringCondenser, ! A10 oa temp var type",
                                                      "  CapCurveFuncTemp, ! A11 CapFoT",
                                                      "  EIRCurveFuncTemp, ! A12 EIRFoT",
                                                      "  EIRCurveFuncPLR, ! A13 EIRFoPLR",
                                                      " 0.2, ! N7 minPLR",
                                                      " 1.0, ! N8 maxPLR",
                                                      " OnDemand, ! A14 defrost control type",
                                                      " , ! N9 defrost time frac",
                                                      " , ! A15 EIRdefrost curve",
                                                      " 3.0, ! N10 max oa DBT for defrost",
                                                      " , ! N11 resistive defrost heater capacity",
                                                      " uniCRFCurve5, ! A16 crf curve name",
                                                      " 500, ! N12 nominal aux elec power",
                                                      " EIRCurveFuncTemp, ! A17 EIRAuxFoT",
                                                      " uniAuxElecEIRFoPLRCurve6, ! A18 EIRAuxFoPLR",
                                                      " 20; ! N13 standby elec power",

                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"

                                                      "Curve:Linear,",
                                                      "  uniDefrostCurve4,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniCRFCurve5,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniAuxElecEIRFoPLRCurve6,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "FUEL FIRED HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRFuelFiredHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRFuelFiredHeatPump *thisHeatingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[0];

    // validate the heating side
    EXPECT_EQ("FUEL FIRED HP HEATING SIDE", thisHeatingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, thisHeatingPLHP->EIRHPType);
    EXPECT_EQ(nullptr, thisHeatingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisHeatingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(2, thisHeatingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(3, thisHeatingPLHP->powerRatioFuncPLRCurveIndex);
    EXPECT_EQ(0, thisHeatingPLHP->defrostEIRCurveIndex);
    EXPECT_EQ(5, thisHeatingPLHP->cycRatioCurveIndex);
    EXPECT_EQ(2, thisHeatingPLHP->auxElecEIRFoTempCurveIndex);
    EXPECT_EQ(6, thisHeatingPLHP->auxElecEIRFoPLRCurveIndex);

    EXPECT_EQ(500.0, thisHeatingPLHP->nominalAuxElecPower);
    EXPECT_EQ(20.0, thisHeatingPLHP->standbyElecPower);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "fake"), std::runtime_error);
    EXPECT_THROW(EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredCooling, "FUEL FIRED HP HEATING SIDE"),
                 std::runtime_error);
}

TEST_F(EnergyPlusFixture, GAHP_HeatingConstructionFullObjectsNoCompanion_with_Defrost)
{
    std::string const idf_objects = delimited_string({"HeatPump:AirToWater:FuelFired:Heating,",
                                                      "  Fuel Fired hp heating side, ! A1",
                                                      "  node w1, ! A2",
                                                      "  node w2, ! A3",
                                                      "  node a3, ! A4",
                                                      "  , ! A5 Comanion coil",
                                                      "  NaturalGas, ! A6 fuel type",
                                                      "  GAHP_Custom, ! A7 end use cat",
                                                      "  3000, ! N1 capacity",
                                                      "  1.5, ! N2 nominal COP",
                                                      "  0.005, ! N3 design flow rate",
                                                      "  60, ! N4 Design Supply Temp",
                                                      "  11.1, ! N5 Design Lift",
                                                      "  1.0, ! N6 sizing factor",
                                                      " NotModulated, ! A8 flow mode",
                                                      " DryBulb, ! A9 oa temp var type",
                                                      " EnteringCondenser, ! A10 oa temp var type",
                                                      "  CapCurveFuncTemp, ! A11 CapFoT",
                                                      "  EIRCurveFuncTemp, ! A12 EIRFoT",
                                                      "  EIRCurveFuncPLR, ! A13 EIRFoPLR",
                                                      " 0.2, ! N7 minPLR",
                                                      " 1.0, ! N8 maxPLR",
                                                      " OnDemand, ! A14 defrost control type",
                                                      " , ! N9 defrost time frac",
                                                      " uniDefrostCurve4, ! A15 EIRdefrost curve",
                                                      " 3.0, ! N10 max oa DBT for defrost",
                                                      " , ! N11 resistive defrost heater capacity",
                                                      " uniCRFCurve5, ! A16 crf curve name",
                                                      " 500, ! N12 nominal aux elec power",
                                                      " EIRCurveFuncTemp, ! A17 EIRAuxFoT",
                                                      " uniAuxElecEIRFoPLRCurve6, ! A18 EIRAuxFoPLR",
                                                      " 20; ! N13 standby elec power",

                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"

                                                      "Curve:Linear,",
                                                      "  uniDefrostCurve4,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniCRFCurve5,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniAuxElecEIRFoPLRCurve6,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "FUEL FIRED HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRFuelFiredHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRFuelFiredHeatPump *thisHeatingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[0];

    // validate the heating side
    EXPECT_EQ("FUEL FIRED HP HEATING SIDE", thisHeatingPLHP->name);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, thisHeatingPLHP->EIRHPType);
    EXPECT_EQ(nullptr, thisHeatingPLHP->companionHeatPumpCoil);
    EXPECT_EQ(1, thisHeatingPLHP->capFuncTempCurveIndex);
    EXPECT_EQ(2, thisHeatingPLHP->powerRatioFuncTempCurveIndex);
    EXPECT_EQ(3, thisHeatingPLHP->powerRatioFuncPLRCurveIndex);
    EXPECT_EQ(4, thisHeatingPLHP->defrostEIRCurveIndex);
    EXPECT_EQ(5, thisHeatingPLHP->cycRatioCurveIndex);
    EXPECT_EQ(2, thisHeatingPLHP->auxElecEIRFoTempCurveIndex);
    EXPECT_EQ(6, thisHeatingPLHP->auxElecEIRFoPLRCurveIndex);

    EXPECT_EQ(500.0, thisHeatingPLHP->nominalAuxElecPower);
    EXPECT_EQ(20.0, thisHeatingPLHP->standbyElecPower);

    // calling the factory with an invalid name or type will call ShowFatalError, which will trigger a runtime exception
    EXPECT_THROW(EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "fake"), std::runtime_error);
    EXPECT_THROW(EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredCooling, "FUEL FIRED HP HEATING SIDE"),
                 std::runtime_error);
}

TEST_F(EnergyPlusFixture, GAHP_Initialization_Test)
{
    std::string const idf_objects = delimited_string({"HeatPump:AirToWater:FuelFired:Heating,",
                                                      "  Fuel Fired hp heating side, ! A1",
                                                      "  node w1, ! A2",
                                                      "  node w2, ! A3",
                                                      "  node a3, ! A4",
                                                      "  , ! A5 Comanion coil",
                                                      "  NaturalGas, ! A6 fuel type",
                                                      "  GAHP_Custom, ! A7 end use cat",
                                                      "  3000, ! N1 capacity",
                                                      "  1.5, ! N2 nominal COP",
                                                      "  0.005, ! N3 design flow rate",
                                                      "  60, ! N4 Design Supply Temp",
                                                      "  11.1, ! N5 Design Lift",
                                                      "  1.0, ! N6 sizing factor",
                                                      " NotModulated, ! A8 flow mode",
                                                      " DryBulb, ! A9 oa temp var type",
                                                      " EnteringCondenser, ! A10 oa temp var type",
                                                      "  CapCurveFuncTemp, ! A11 CapFoT",
                                                      "  EIRCurveFuncTemp, ! A12 EIRFoT",
                                                      "  EIRCurveFuncPLR, ! A13 EIRFoPLR",
                                                      " 0.2, ! N7 minPLR",
                                                      " 1.0, ! N8 maxPLR",
                                                      " OnDemand, ! A14 defrost control type",
                                                      " , ! N9 defrost time frac",
                                                      " , ! A15 EIRdefrost curve",
                                                      " , ! N10 resistive defrost heater capacity",
                                                      " 3.0, ! N11 max oa DBT for defrost",
                                                      " uniCRFCurve5, ! A16 crf curve name",
                                                      " 500, ! N12 nominal aux elec power",
                                                      " EIRCurveFuncTemp, ! A17 EIRAuxFoT",
                                                      " uniAuxElecEIRFoPLRCurve6, ! A18 EIRAuxFoPLR",
                                                      " 20; ! N13 standby elec power",

                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"

                                                      "Curve:Linear,",
                                                      "  uniDefrostCurve4,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniCRFCurve5,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniAuxElecEIRFoPLRCurve6,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool firstHVACIteration = true;
    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating;

    // the init call expects a "from" calling point
    PlantLocation myLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "FUEL FIRED HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRFuelFiredHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRFuelFiredHeatPump *thisHeatingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisHeatingPLHP->onInitLoopEquip(*state, myLocation);

    // call with run flag off, loose limits on node min/max
    thisHeatingPLHP->running = false;
    Real64 constexpr currentLoad = 0.0;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisHeatingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag off, nonzero minimums
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).MassFlowRateMinAvail = 0.1;
    thisHeatingPLHP->running = false;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.1, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0, thisHeatingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag off, load side flow locked
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).MassFlowRate = 0.24;
    thisHeatingPLHP->running = false;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.24, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisHeatingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on load side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).MassFlowRate = 0.0;
    thisHeatingPLHP->running = true;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0, thisHeatingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at zero on source side
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).MassFlowRate = 0.2;
    thisHeatingPLHP->running = true;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.2, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(1.29, thisHeatingPLHP->sourceSideMassFlowRate, 0.1);

    // call with run flag ON, flow locked at zero on both sides
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).MassFlowRate = 0.0;
    thisHeatingPLHP->running = true;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(0.0, thisHeatingPLHP->sourceSideMassFlowRate, 0.001);

    // call with run flag ON, flow locked at nonzero both
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).MassFlowRate = 0.14;
    thisHeatingPLHP->running = true;
    thisHeatingPLHP->setOperatingFlowRatesASHP(*state, firstHVACIteration, currentLoad);
    EXPECT_NEAR(0.14, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    EXPECT_NEAR(1.29, thisHeatingPLHP->sourceSideMassFlowRate, 0.1);
}

TEST_F(EnergyPlusFixture, GAHP_HeatingSimulate_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:AirToWater:FuelFired:Heating,",
                                                      "  Fuel Fired hp heating side, ! A1",
                                                      "  node w1, ! A2",
                                                      "  node w2, ! A3",
                                                      "  node a3, ! A4",
                                                      "  , ! A5 Comanion coil",
                                                      "  NaturalGas, ! A6 fuel type",
                                                      "  GAHP_Custom, ! A7 end use cat",
                                                      "  3000, ! N1 capacity",
                                                      "  1.5, ! N2 nominal COP",
                                                      "  0.005, ! N3 design flow rate",
                                                      "  60, ! N4 Design Supply Temp",
                                                      "  11.1, ! N5 Design Lift",
                                                      "  1.0, ! N6 sizing factor",
                                                      " NotModulated, ! A8 flow mode",
                                                      " DryBulb, ! A9 oa temp var type",
                                                      " EnteringCondenser, ! A10 oa temp var type",
                                                      "  CapCurveFuncTemp, ! A11 CapFoT",
                                                      "  EIRCurveFuncTemp, ! A12 EIRFoT",
                                                      "  EIRCurveFuncPLR, ! A13 EIRFoPLR",
                                                      " 0.2, ! N7 minPLR",
                                                      " 1.0, ! N8 maxPLR",
                                                      " OnDemand, ! A14 defrost control type",
                                                      " , ! N9 defrost time frac",
                                                      " , ! A15 EIRdefrost curve",
                                                      " , ! N10 resistive defrost heater capacity",
                                                      " 3.0, ! N11 max oa DBT for defrost",
                                                      " uniCRFCurve5, ! A16 crf curve name",
                                                      " 500, ! N12 nominal aux elec power",
                                                      " EIRCurveFuncTemp, ! A17 EIRAuxFoT",
                                                      " uniAuxElecEIRFoPLRCurve6, ! A18 EIRAuxFoPLR",
                                                      " 20; ! N13 standby elec power",

                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"

                                                      "Curve:Linear,",
                                                      "  uniDefrostCurve4,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniCRFCurve5,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniAuxElecEIRFoPLRCurve6,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "FUEL FIRED HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRFuelFiredHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRFuelFiredHeatPump *thisHeatingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisHeatingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // do a runflag = false to test out execution order
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = false;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
    }

    // call it from the load side, but this time there is a negative (cooling) load - shouldn't try to run
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = true; // plant actually shouldn't do this but the component can be smart enough to handle it
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(loadInletTemp, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // call it from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // EXPECT_NEAR(specifiedLoadSetpoint, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 1200;
        // Real64 availableCapacity = 950.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        // EXPECT_NEAR(44.402, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        // EXPECT_NEAR(availableCapacity, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is no load (still firsthvac)
    {
        bool firstHVAC = true;
        Real64 curLoad = 0.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(45.0, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(30.0, thisHeatingPLHP->sourceSideOutletTemp, 0.001);
    }

    // Test cycling calcs
    {
        bool firstHVAC = true;
        Real64 curLoad = 500.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        // Use user specified curve
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        EXPECT_NEAR(1.0, thisHeatingPLHP->cyclingRatioFraction, 0.001);
        // Use default assumptions
        thisHeatingPLHP->cycRatioCurveIndex = 0;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        EXPECT_NEAR(0.861, thisHeatingPLHP->cyclingRatioFraction, 0.001);
    }

    // call it from the load side, very low load
    {
        bool firstHVAC = true;
        Real64 curLoad = 100;
        bool runFlag = true;
        Real64 constexpr specifiedLoadSetpoint = 45;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = 40;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        EXPECT_TRUE(thisHeatingPLHP->fuelRate > 0);
        EXPECT_NEAR(5.0, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    }

    {
        bool firstHVAC = false;
        Real64 curLoad = 2000;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.0478;
        Real64 constexpr specifiedLoadSetpoint = 45;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = 35;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->flowMode = DataPlant::FlowMode::LeavingSetpointModulated;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        EXPECT_NEAR(expectedLoadMassFlowRate, thisHeatingPLHP->loadSideMassFlowRate, 0.001);
    }
}

TEST_F(EnergyPlusFixture, GAHP_HeatingSimulate_AirSource_with_Defrost)
{
    std::string const idf_objects = delimited_string({"HeatPump:AirToWater:FuelFired:Heating,",
                                                      "  Fuel Fired hp heating side, ! A1",
                                                      "  node w1, ! A2",
                                                      "  node w2, ! A3",
                                                      "  node a3, ! A4",
                                                      "  , ! A5 Comanion coil",
                                                      "  NaturalGas, ! A6 fuel type",
                                                      "  GAHP_Custom, ! A7 end use cat",
                                                      "  3000, ! N1 capacity",
                                                      "  1.5, ! N2 nominal COP",
                                                      "  0.005, ! N3 design flow rate",
                                                      "  60, ! N4 Design Supply Temp",
                                                      "  11.1, ! N5 Design Lift",
                                                      "  1.0, ! N6 sizing factor",
                                                      " NotModulated, ! A8 flow mode",
                                                      " DryBulb, ! A9 oa temp var type",
                                                      " EnteringCondenser, ! A10 oa temp var type",
                                                      "  CapCurveFuncTemp, ! A11 CapFoT",
                                                      "  EIRCurveFuncTemp, ! A12 EIRFoT",
                                                      "  EIRCurveFuncPLR, ! A13 EIRFoPLR",
                                                      " 0.2, ! N7 minPLR",
                                                      " 1.0, ! N8 maxPLR",
                                                      " OnDemand, ! A14 defrost control type",
                                                      " , ! N9 defrost time frac",
                                                      " uniDefrostCurve4, ! A15 EIRdefrost curve",
                                                      " , ! N10 resistive defrost heater capacity",
                                                      " 3.0, ! N11 max oa DBT for defrost",
                                                      " uniCRFCurve5, ! A16 crf curve name",
                                                      " 500, ! N12 nominal aux elec power",
                                                      " EIRCurveFuncTemp, ! A17 EIRAuxFoT",
                                                      " uniAuxElecEIRFoPLRCurve6, ! A18 EIRAuxFoPLR",
                                                      " 20; ! N13 standby elec power",

                                                      "Curve:Biquadratic,",
                                                      "  CapCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Biquadratic,",
                                                      "  EIRCurveFuncTemp,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  5.0,",
                                                      "  10.0,",
                                                      "  24.0,",
                                                      "  35.0,",
                                                      "  ,",
                                                      "  ,",
                                                      "  Temperature,",
                                                      "  Temperature,",
                                                      "  Dimensionless;",
                                                      "Curve:Quadratic,",
                                                      "  EIRCurveFuncPLR,",
                                                      "  1.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  0.0,",
                                                      "  1.0;"

                                                      "Curve:Linear,",
                                                      "  uniDefrostCurve4,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniCRFCurve5,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"
                                                      "Curve:Linear,",
                                                      "  uniAuxElecEIRFoPLRCurve6,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "FUEL FIRED HP HEATING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRFuelFiredHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRFuelFiredHeatPump *thisHeatingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[0];
    auto thisEIRPlantLoopHP = &(*(EIRPlantLoopHeatPump *)thisHeatingPLHP);

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisHeatingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // do a runflag = false to test out execution order
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = false;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
    }

    // call it from the load side, but this time there is a negative (cooling) load - shouldn't try to run
    {
        bool firstHVAC = true;
        Real64 curLoad = -900;
        bool runFlag = true; // plant actually shouldn't do this but the component can be smart enough to handle it
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 constexpr loadInletTemp = 46;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = loadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(loadInletTemp, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // call it from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        EXPECT_NEAR(16533.333, thisHeatingPLHP->fuelRate, 0.001);
        EXPECT_NEAR(14880000.0, thisHeatingPLHP->fuelEnergy, 0.001);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // EXPECT_NEAR(specifiedLoadSetpoint, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
        EXPECT_NEAR(15520.0, thisEIRPlantLoopHP->powerUsage, 0.001);
    }

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit cannot meet load)
    {
        bool firstHVAC = true;
        Real64 curLoad = 1200;
        // Real64 availableCapacity = 950.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        EXPECT_NEAR(24800.0, thisHeatingPLHP->fuelRate, 0.001);
        EXPECT_NEAR(22320000.0, thisHeatingPLHP->fuelEnergy, 0.001);
        EXPECT_NEAR(15520.0, thisEIRPlantLoopHP->powerUsage, 0.001);
        // expect it to miss setpoint and be at max capacity
        // EXPECT_NEAR(44.402, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        // EXPECT_NEAR(availableCapacity, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    }

    // now we can call it again from the load side, but this time there is no load (still firsthvac)
    {
        bool firstHVAC = true;
        Real64 curLoad = 0.0;
        bool runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.09999;
        Real64 constexpr expectedCp = 4180;
        Real64 constexpr specifiedLoadSetpoint = 45;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 30;
        thisHeatingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to miss setpoint and be at max capacity
        EXPECT_NEAR(45.0, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(30.0, thisHeatingPLHP->sourceSideOutletTemp, 0.001);
        EXPECT_NEAR(0.0, thisEIRPlantLoopHP->powerUsage, 0.001);
    }
}

TEST_F(EnergyPlusFixture, Test_HeatRecoveryGetInputs_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  AirSource,",
                                                      "  node 9,",
                                                      "  node 10,",
                                                      "  node 11,",
                                                      "  node 12,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  2.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // check heat recovery input fields
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_EQ(thisCoolingPLHP->maxHeatRecoveryTempLimit, 60.0);
    EXPECT_EQ(thisHeatingPLHP->minHeatRecoveryTempLimit, 4.5);
}

TEST_F(EnergyPlusFixture, Test_HeatRecoveryFlowSizing_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  1.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  AirSource,",
                                                      "  node 9,",
                                                      "  node 10,",
                                                      "  node 11,",
                                                      "  node 12,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  2.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // check heat recovery input fields
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_EQ(thisHeatingPLHP->minHeatRecoveryTempLimit, 4.5);
    EXPECT_EQ(thisCoolingPLHP->maxHeatRecoveryTempLimit, 60.0);

    // We'll set up two plant loops: load heating loop and load side cooling loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(2);
    // chilled water plant loop
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop1demandComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop1supplyComponent1.Name = thisCoolingPLHP->name;
    loop1supplyComponent1.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    // heat recovery component on the demand side of loop2
    loop1demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1demandComponent1.Name = thisHeatingPLHP->name;
    loop1demandComponent1.NodeNumIn = thisHeatingPLHP->heatRecoveryNodes.inlet;
    // assign the CW plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataSize->PlantSizData(1).DeltaT = 6.67;

    // hot water plant loop
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &loop2supplyComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    loop2supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2supplyComponent1.Name = thisHeatingPLHP->name;
    loop2supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    // heat recovery component on the demand side of loop1
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop2demandComponent1.Name = thisCoolingPLHP->name;
    loop2demandComponent1.NodeNumIn = thisCoolingPLHP->heatRecoveryNodes.inlet;
    // assign the HW plant sizing data
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataSize->PlantSizData(2).DeltaT = 11.11;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHWHeatRecoveryLocation = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(2, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myCWHeatRecoveryLocation = PlantLocation(1, DataPlant::LoopSideLocation::Demand, 1, 1);
    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    // size the HW heat-recovery flow rate
    // set properties at design HW temp (60.0C)
    Real64 rhoHR = 983.20;
    Real64 CpHR = 4185.00;
    Real64 designHWHeatRecoveryHeatTransfer = thisCoolingPLHP->referenceCapacity * (1 + 1 / thisCoolingPLHP->referenceCOP);
    Real64 expectedHWHeatRecoveryFlow = designHWHeatRecoveryHeatTransfer / (state->dataSize->PlantSizData(2).DeltaT * CpHR * rhoHR);
    // size the CW heat-recovery flow rate
    // reset properties at design CW temp (5.5C)
    rhoHR = 999.90;
    CpHR = 4197.93;
    Real64 designCWHeatRecoveryHeatTransfer = thisHeatingPLHP->referenceCapacity * (1 - 1 / thisHeatingPLHP->referenceCOP);
    Real64 expectedCWHeatRecoveryFlow = designCWHeatRecoveryHeatTransfer / (state->dataSize->PlantSizData(1).DeltaT * CpHR * rhoHR);
    // check autosized heat recovery flow rates
    EXPECT_NEAR(expectedHWHeatRecoveryFlow, thisCoolingPLHP->heatRecoveryDesignVolFlowRate, 0.00001); // 0.00612
    EXPECT_NEAR(expectedCWHeatRecoveryFlow, thisHeatingPLHP->heatRecoveryDesignVolFlowRate, 0.00001); // 0.00250
}

TEST_F(EnergyPlusFixture, CoolingwithHeatRecoverySimulate_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  AirSource,",
                                                      "  node 9,",
                                                      "  node 10,",
                                                      "  node 11,",
                                                      "  node 12,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // check heat recovery input fields
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_EQ(thisHeatingPLHP->minHeatRecoveryTempLimit, 4.5);
    EXPECT_EQ(thisCoolingPLHP->maxHeatRecoveryTempLimit, 60.0);

    // We'll set up two plant loops: load heating loop and load side cooling loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(2);
    // chilled water plant loop
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop1demandComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop1supplyComponent1.Name = thisCoolingPLHP->name;
    loop1supplyComponent1.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    // heat recovery component on the demand side of loop2
    loop1demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1demandComponent1.Name = thisHeatingPLHP->name;
    loop1demandComponent1.NodeNumIn = thisHeatingPLHP->heatRecoveryNodes.inlet;
    // assign the CW plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataSize->PlantSizData(1).DeltaT = 6.67;

    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // hot water plant loop
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &loop2supplyComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    loop2supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2supplyComponent1.Name = thisHeatingPLHP->name;
    loop2supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    // heat recovery component on the demand side of loop1
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop2demandComponent1.Name = thisCoolingPLHP->name;
    loop2demandComponent1.NodeNumIn = thisCoolingPLHP->heatRecoveryNodes.inlet;
    // assign the HW plant sizing data
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataSize->PlantSizData(2).DeltaT = 11.11;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHWHeatRecoveryLocation = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(2, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myCWHeatRecoveryLocation = PlantLocation(1, DataPlant::LoopSideLocation::Demand, 1, 1);
    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    // call from load side location, firsthvac, no load, not running, verify the unit doesn't have any values lingering
    thisCoolingPLHP->loadSideHeatTransfer = 2000;
    thisCoolingPLHP->loadSideInletTemp = 23.0;
    thisCoolingPLHP->loadSideOutletTemp = 43.0;
    thisCoolingPLHP->powerUsage = 100.0;
    thisCoolingPLHP->sourceSideHeatTransfer = 60.0;
    thisCoolingPLHP->sourceSideInletTemp = 33.0;
    thisCoolingPLHP->sourceSideOutletTemp = 43.0;
    thisCoolingPLHP->heatRecoveryInletTemp = 45.0;
    thisCoolingPLHP->heatRecoveryOutletTemp = 55.0;
    thisCoolingPLHP->sysControlType = ControlType::Setpoint;
    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    bool runFlag = false;
    thisCoolingPLHP->simulate(*state, myCoolingLoadLocation, firstHVAC, curLoad, runFlag);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->powerUsage, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->heatRecoveryRate, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->loadSideInletTemp, thisCoolingPLHP->loadSideOutletTemp, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->sourceSideInletTemp, thisCoolingPLHP->sourceSideOutletTemp, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->heatRecoveryInletTemp, thisCoolingPLHP->heatRecoveryOutletTemp, 0.001);

    // now we can call it again from the load side, but this time there is load
    {
        firstHVAC = true;
        curLoad = -69993.3; // current cooling load
        runFlag = true;
        Real64 expectedLoadMassFlowRate = thisCoolingPLHP->loadSideDesignMassFlowRate;
        Real64 constexpr expectedCp = 4182.3220354805;
        Real64 constexpr specifiedLoadSetpoint = 15.0;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 30;
        state->dataLoopNodes->Node(thisCoolingPLHP->heatRecoveryNodes.inlet).Temp = 45.0;
        thisCoolingPLHP->simulate(*state, myCoolingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet the setpoint while operating at part load
        EXPECT_NEAR(15.0, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.5, thisCoolingPLHP->partLoadRatio, 0.001);
        EXPECT_NEAR(69993.3, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
        EXPECT_NEAR(23331.1, thisCoolingPLHP->powerUsage, 0.001);
        EXPECT_NEAR(93324.4, thisCoolingPLHP->heatRecoveryRate, 0.001);
        EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideHeatTransfer, 0.001);
        EXPECT_NEAR(45.0, thisCoolingPLHP->heatRecoveryInletTemp, 0.001);
        EXPECT_NEAR(50.469, thisCoolingPLHP->heatRecoveryOutletTemp, 0.001);
    }

    // now we can call it again from the load side, but this time there is load
    // higher heat recovery temperature
    {
        firstHVAC = true;
        curLoad = -69993.3; // current cooling load
        runFlag = true;
        Real64 expectedLoadMassFlowRate = thisCoolingPLHP->loadSideDesignMassFlowRate;
        Real64 constexpr expectedCp = 4182.3220354805;
        Real64 constexpr specifiedLoadSetpoint = 15.0;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 30;
        state->dataLoopNodes->Node(thisCoolingPLHP->heatRecoveryNodes.inlet).Temp = 58.0;
        thisCoolingPLHP->simulate(*state, myCoolingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet the setpoint while operating at part load
        EXPECT_NEAR(15.0, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.5, thisCoolingPLHP->partLoadRatio, 0.001);
        EXPECT_NEAR(69993.3, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
        EXPECT_NEAR(23331.1, thisCoolingPLHP->powerUsage, 0.001);
        // energy balance or energy conservation at the condenser
        Real64 energyBalanceCondenser = thisCoolingPLHP->loadSideHeatTransfer + thisCoolingPLHP->powerUsage;
        EXPECT_NEAR(93324.4, energyBalanceCondenser, 0.001);
        // heat rejected is split b/n heat recovery and source side heat transfer due to tem limit
        EXPECT_NEAR(34164.275, thisCoolingPLHP->heatRecoveryRate, 0.001);
        EXPECT_NEAR(59160.125, thisCoolingPLHP->sourceSideHeatTransfer, 0.001);
        Real64 totalHeatRejected = thisCoolingPLHP->heatRecoveryRate + thisCoolingPLHP->sourceSideHeatTransfer;
        EXPECT_NEAR(93324.4, totalHeatRejected, 0.001);
        // total heat rejected == energy balance at the condenser
        EXPECT_NEAR(energyBalanceCondenser, totalHeatRejected, 0.001);
        EXPECT_NEAR(58.0, thisCoolingPLHP->heatRecoveryInletTemp, 0.001);
        // heat recovery outlet temperature is capped @ 60C.
        EXPECT_NEAR(60.0, thisCoolingPLHP->heatRecoveryOutletTemp, 0.001);
    }
}

TEST_F(EnergyPlusFixture, HeatingwithHeatRecoverySimulate_AirSource)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  AirSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  node 5,",
                                                      "  node 6,",
                                                      "  hp heating side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "HeatPump:PlantLoop:EIR:Heating,",
                                                      "  hp heating side,",
                                                      "  node 7,",
                                                      "  node 8,",
                                                      "  AirSource,",
                                                      "  node 9,",
                                                      "  node 10,",
                                                      "  node 11,",
                                                      "  node 12,",
                                                      "  hp cooling side,",
                                                      "  0.005,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  Autosize,",
                                                      "  3.0,",
                                                      "  1,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  1,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];
    EIRPlantLoopHeatPump *thisHeatingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[1];

    // check heat recovery input fields
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryAvailable);
    EXPECT_TRUE(thisCoolingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_TRUE(thisHeatingPLHP->heatRecoveryDesignVolFlowRateWasAutoSized);
    EXPECT_EQ(thisHeatingPLHP->minHeatRecoveryTempLimit, 4.5);
    EXPECT_EQ(thisCoolingPLHP->maxHeatRecoveryTempLimit, 60.0);

    // We'll set up two plant loops: load heating loop and load side cooling loop
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(2);
    // chilled water plant loop
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &loop1supplyComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop1demandComponent1 = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    loop1supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop1supplyComponent1.Name = thisCoolingPLHP->name;
    loop1supplyComponent1.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    // heat recovery component on the demand side of loop1
    loop1demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop1demandComponent1.Name = thisHeatingPLHP->name;
    loop1demandComponent1.NodeNumIn = thisHeatingPLHP->heatRecoveryNodes.inlet;
    // assign the CW plant sizing data
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataSize->PlantSizData(1).DeltaT = 6.67;

    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // hot water plant loop
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &loop2supplyComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    auto &loop2demandComponent1 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    loop2supplyComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    loop2supplyComponent1.Name = thisHeatingPLHP->name;
    loop2supplyComponent1.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    // heat recovery component on the demand side of loop2
    loop2demandComponent1.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    loop2demandComponent1.Name = thisCoolingPLHP->name;
    loop2demandComponent1.NodeNumIn = thisCoolingPLHP->heatRecoveryNodes.inlet;
    // assign the HW plant sizing data
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataSize->PlantSizData(2).DeltaT = 11.11;

    // the init call expects a "from" calling point
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myHWHeatRecoveryLocation = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantLocation myHeatingLoadLocation = PlantLocation(2, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myCWHeatRecoveryLocation = PlantLocation(1, DataPlant::LoopSideLocation::Demand, 1, 1);

    state->dataPlnt->PlantLoop(2).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    auto &PLHPPlantLoadSideComp2 = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp2.Type = DataPlant::PlantEquipmentType::HeatPumpEIRHeating;
    PLHPPlantLoadSideComp2.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // set a couple global flags
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    // initialize so the components can find themselves on the plant
    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);
    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);

    // call from load side location, firsthvac, no load, not running, verify the unit doesn't have any values lingering
    thisHeatingPLHP->loadSideHeatTransfer = 5000;
    thisHeatingPLHP->loadSideInletTemp = 43.0;
    thisHeatingPLHP->loadSideOutletTemp = 53.0;
    thisHeatingPLHP->powerUsage = 200.0;
    thisHeatingPLHP->sourceSideHeatTransfer = 5200.0;
    thisHeatingPLHP->sourceSideInletTemp = 13.0;
    thisHeatingPLHP->sourceSideOutletTemp = 8.0;
    thisHeatingPLHP->heatRecoveryInletTemp = 15.0;
    thisHeatingPLHP->heatRecoveryOutletTemp = 10.0;
    thisHeatingPLHP->sysControlType = ControlType::Setpoint;
    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    bool runFlag = false;
    thisHeatingPLHP->simulate(*state, myHeatingLoadLocation, firstHVAC, curLoad, runFlag);
    EXPECT_NEAR(0.0, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisHeatingPLHP->sourceSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisHeatingPLHP->powerUsage, 0.001);
    EXPECT_NEAR(0.0, thisHeatingPLHP->heatRecoveryRate, 0.001);
    EXPECT_NEAR(thisHeatingPLHP->loadSideInletTemp, thisHeatingPLHP->loadSideOutletTemp, 0.001);
    EXPECT_NEAR(thisHeatingPLHP->sourceSideInletTemp, thisHeatingPLHP->sourceSideOutletTemp, 0.001);
    EXPECT_NEAR(thisHeatingPLHP->heatRecoveryInletTemp, thisHeatingPLHP->heatRecoveryOutletTemp, 0.001);

    // now we can call it again from the load side, but this time there is heating load
    {
        firstHVAC = true;
        curLoad = 69993.3; // current heating load
        runFlag = true;
        Real64 expectedLoadMassFlowRate = thisHeatingPLHP->loadSideDesignMassFlowRate;
        Real64 constexpr expectedCp = 4182.3220354805;
        Real64 constexpr specifiedLoadSetpoint = 55.0;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 13;
        state->dataLoopNodes->Node(thisHeatingPLHP->heatRecoveryNodes.inlet).Temp = 15.0;
        thisHeatingPLHP->simulate(*state, myHeatingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet the setpoint while operating at part load
        EXPECT_NEAR(55.0, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.5, thisHeatingPLHP->partLoadRatio, 0.001);
        EXPECT_NEAR(69982.238, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
        EXPECT_NEAR(23327.413, thisHeatingPLHP->powerUsage, 0.001);
        EXPECT_NEAR(46654.825, thisHeatingPLHP->heatRecoveryRate, 0.001);
        EXPECT_NEAR(0.0, thisHeatingPLHP->sourceSideHeatTransfer, 0.001);
        EXPECT_NEAR(15.0, thisHeatingPLHP->heatRecoveryInletTemp, 0.001);
        EXPECT_NEAR(11.655, thisHeatingPLHP->heatRecoveryOutletTemp, 0.001);
    }

    // now we can call it again from the load side, but this time there is heating load
    {
        firstHVAC = true;
        curLoad = 69993.3; // current heating load
        runFlag = true;
        Real64 expectedLoadMassFlowRate = thisHeatingPLHP->loadSideDesignMassFlowRate;
        Real64 constexpr expectedCp = 4182.3220354805;
        Real64 constexpr specifiedLoadSetpoint = 55.0;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = 13;
        state->dataLoopNodes->Node(thisHeatingPLHP->heatRecoveryNodes.inlet).Temp = 7.0;
        thisHeatingPLHP->simulate(*state, myHeatingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet the setpoint while operating at part load
        EXPECT_NEAR(55.0, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(0.5, thisHeatingPLHP->partLoadRatio, 0.001);
        EXPECT_NEAR(69982.238, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
        EXPECT_NEAR(23327.413, thisHeatingPLHP->powerUsage, 0.001);
        EXPECT_NEAR(34956.434, thisHeatingPLHP->heatRecoveryRate, 0.001);
        EXPECT_NEAR(11698.391, thisHeatingPLHP->sourceSideHeatTransfer, 0.001);
        // heat balance or energy conservation applied at the evaporator (source side)
        Real64 heatBalanceEvap = thisHeatingPLHP->loadSideHeatTransfer - thisHeatingPLHP->powerUsage;
        EXPECT_NEAR(46654.825, heatBalanceEvap, 0.001);
        // heat rejected is split b/n heat recovery and source side heat transfer due to temp limit
        EXPECT_NEAR(34956.434, thisHeatingPLHP->heatRecoveryRate, 0.001);
        EXPECT_NEAR(11698.391, thisHeatingPLHP->sourceSideHeatTransfer, 0.001);
        // check the heat recovered at the evaporator (source) side
        Real64 chilledWaterEnergyRecovered = thisHeatingPLHP->heatRecoveryRate + thisHeatingPLHP->sourceSideHeatTransfer;
        EXPECT_NEAR(46654.825, chilledWaterEnergyRecovered, 0.001);
        // check energy balance
        EXPECT_NEAR(heatBalanceEvap, chilledWaterEnergyRecovered, 0.001);
        EXPECT_NEAR(7.0, thisHeatingPLHP->heatRecoveryInletTemp, 0.001);
        // heat recovery outlet temperature is capped @ 4.5C.
        EXPECT_NEAR(4.5, thisHeatingPLHP->heatRecoveryOutletTemp, 0.001);
    }
}

TEST_F(EnergyPlusFixture, CoolingSimulate_WSHP_SourceSideOutletTemp)
{
    std::string const idf_objects = delimited_string({"HeatPump:PlantLoop:EIR:Cooling,",
                                                      "  hp cooling side,",
                                                      "  node 1,",
                                                      "  node 2,",
                                                      "  WaterSource,",
                                                      "  node 3,",
                                                      "  node 4,",
                                                      "  ,",
                                                      "  ,",
                                                      "  ,",
                                                      "  0.0002,",
                                                      "  0.0002,",
                                                      "  ,",
                                                      "  2000,",
                                                      "  3.00,",
                                                      "  ,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve,",
                                                      "  dummyCurve;",
                                                      "Curve:Linear,",
                                                      "  dummyCurve,",
                                                      "  0.95,",
                                                      "  0,",
                                                      "  1,",
                                                      "  1;"});
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(2);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideComp = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;
    PLHPPlantLoadSideComp.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;
    // then the source side

    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSourceComp = state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1);
    PLHPPlantLoadSourceComp.Type = DataPlant::PlantEquipmentType::HeatPumpEIRCooling;

    // the init call expects a "from" calling point
    PlantLocation myLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation mySourceLocation = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRPlantLoopHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpEIRCooling, "HP COOLING SIDE");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(1u, state->dataEIRPlantLoopHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    EIRPlantLoopHeatPump *thisCoolingPLHP = &state->dataEIRPlantLoopHeatPump->heatPumps[0];

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideComp.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSourceComp.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSourceComp.NodeNumIn = thisCoolingPLHP->sourceSideNodes.inlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    thisCoolingPLHP->onInitLoopEquip(*state, myLoadLocation);

    // call from load side location, firsthvac, no load, not running, verify the unit doesn't have any values lingering
    thisCoolingPLHP->loadSideHeatTransfer = 2000;
    thisCoolingPLHP->loadSideInletTemp = 23.0;
    thisCoolingPLHP->loadSideOutletTemp = 42.0;
    thisCoolingPLHP->powerUsage = 400.0;
    thisCoolingPLHP->sourceSideHeatTransfer = 2000.0;
    thisCoolingPLHP->sourceSideInletTemp = 45.0;
    thisCoolingPLHP->sourceSideOutletTemp = 83.0;
    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    bool runFlag = false;
    thisCoolingPLHP->heatRecoveryHeatPump = true;
    thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
    EXPECT_NEAR(0.0, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->sourceSideHeatTransfer, 0.001);
    EXPECT_NEAR(0.0, thisCoolingPLHP->powerUsage, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->loadSideInletTemp, thisCoolingPLHP->loadSideOutletTemp, 0.001);
    EXPECT_NEAR(thisCoolingPLHP->sourceSideInletTemp, thisCoolingPLHP->sourceSideOutletTemp, 0.001);

    // call from source side location, firsthvac, no load, not running, connected loop should be triggered to resimulate
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).SimLoopSideNeeded = false;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).SimLoopSideNeeded = false;
    thisCoolingPLHP->simulate(*state, mySourceLocation, firstHVAC, curLoad, runFlag);
    EXPECT_TRUE(state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).SimLoopSideNeeded);

    // now we can call it again from the load side, but this time there is load (still firsthvac, unit can meet load)
    {
        firstHVAC = true;
        curLoad = -1900;
        runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.200;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = expectedLoadMassFlowRate;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRate = expectedLoadMassFlowRate;
        Real64 constexpr expectedCp = 4183;
        Real64 constexpr specifiedLoadSetpoint = 15;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 45;
        thisCoolingPLHP->maxSourceTempLimit = 50.0;
        thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        EXPECT_NEAR(1.000, thisCoolingPLHP->partLoadRatio, 0.01);
        EXPECT_NEAR(specifiedLoadSetpoint, thisCoolingPLHP->loadSideOutletTemp, 0.01);
        EXPECT_NEAR(-curLoad, thisCoolingPLHP->loadSideHeatTransfer, 0.01);
        EXPECT_NEAR(2471.583, thisCoolingPLHP->sourceSideHeatTransfer, 0.01);
        EXPECT_NEAR(47.957, thisCoolingPLHP->sourceSideOutletTemp, 0.01);
    }

    // now we can call it again from the load side, but this time there is source side temperature limit exceeded
    {
        firstHVAC = true;
        curLoad = -1900;
        runFlag = true;
        Real64 constexpr expectedLoadMassFlowRate = 0.200;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).MassFlowRate = expectedLoadMassFlowRate;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).MassFlowRate = expectedLoadMassFlowRate;
        Real64 constexpr expectedCp = 4183;
        Real64 constexpr specifiedLoadSetpoint = 15;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = 48;
        thisCoolingPLHP->maxSourceTempLimit = 50.0;
        thisCoolingPLHP->simulate(*state, myLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // reduced PLR to meet the source side outlet temperature limit specified
        EXPECT_NEAR(0.68, thisCoolingPLHP->partLoadRatio, 0.01);
        EXPECT_NEAR(15.73, thisCoolingPLHP->loadSideOutletTemp, 0.01);
        EXPECT_NEAR(1285.38, thisCoolingPLHP->loadSideHeatTransfer, 0.01);
        EXPECT_NEAR(386.69, thisCoolingPLHP->powerUsage, 0.01);
        EXPECT_NEAR(1672.07, thisCoolingPLHP->loadSideHeatTransfer + thisCoolingPLHP->powerUsage, 0.01);
        EXPECT_NEAR(1672.07, thisCoolingPLHP->sourceSideHeatTransfer, 0.01);
        EXPECT_NEAR(50.0, thisCoolingPLHP->sourceSideOutletTemp, 0.01);
    }
}

TEST_F(EnergyPlusFixture, GAHP_AirSource_CurveEval)
{
    // Test for #10665
    std::string const idf_objects = delimited_string({

        "HeatPump:AirToWater:FuelFired:Heating,",
        "  FuelFired GAHP Heating,                 !- Name",
        "  Node 3,                                 !- Water Inlet Node Name",
        "  Node 7,                                 !- Water Outlet Node Name",
        "  FuelFired GAHP Heating OA Node,         !- Air Source Node Name",
        "  FuelFired GAHP Cooling,                 !- Companion Cooling Heat Pump Name",
        "  NaturalGas,                             !- Fuel Type",
        "  GAHP,                                   !- End-Use Subcategory",
        "  3000,                                   !- Nominal Heating Capacity {W}",
        "  1.5,                                    !- Nominal COP {W/W}",
        "  0.005,                                  !- Design Flow Rate {m3/s}",
        "  60,                                     !- Design Supply Temperature {C}",
        "  11.1,                                   !- Design Temperature Lift {deltaC}",
        "  1,                                      !- Sizing Factor",
        "  NotModulated,                           !- Flow Mode",
        "  DryBulb,                                !- Outdoor Air Temperature Curve Input Variable",
        "  EnteringCondenser,                      !- Water Temperature Curve Input Variable",
        "  CapCurveFuncTemp,                       !- Normalized Capacity Function of Temperature Curve Name",
        "  EIRCurveFuncTemp,                       !- Fuel Energy Input Ratio Function of Temperature Curve Name",
        "  EIRCurveFuncPLR,                        !- Fuel Energy Input Ratio Function of PLR Curve Name",
        "  0.1,                                    !- Minimum Part Load Ratio",
        "  1,                                      !- Maximum Part Load Ratio",
        "  OnDemand,                               !- Defrost Control Type",
        "  0,                                      !- Defrost Operation Time Fraction",
        "  EIRDefrostFoTCurve,                     !- Fuel Energy Input Ratio Defrost Adjustment Curve Name",
        "  0,                                      !- Resistive Defrost Heater Capacity {W}",
        "  5,                                      !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation {C}",
        "  CRFCurve,                               !- Cycling Ratio Factor Curve Name",
        "  500,                                    !- Nominal Auxiliary Electric Power {W}",
        "  auxElecEIRCurveFuncTempCurve,           !- Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name",
        "  auxElecEIRFoPLRCurve,                   !- Auxiliary Electric Energy Input Ratio Function of PLR Curve Name",
        "  20;                                     !- Standby Electric Power {W}",

        "OutdoorAir:Node,",
        "  FuelFired GAHP Heating OA Node;         !- Name",

        "HeatPump:AirToWater:FuelFired:Cooling,",
        "  FuelFired GAHP Cooling,                 !- Name",
        "  FuelFired GAHP Cooling Water Inlet Node, !- Water Inlet Node Name",
        "  FuelFired GAHP Cooling Water Outlet Node, !- Water Outlet Node Name",
        "  FuelFired GAHP Cooling OA Node,         !- Air Source Node Name",
        "  FuelFired GAHP Heating,                 !- Companion Heating Heat Pump Name",
        "  NaturalGas,                             !- Fuel Type",
        "  GAHP,                                   !- End-Use Subcategory",
        "  4000,                                   !- Nominal Cooling Capacity {W}",
        "  2,                                      !- Nominal COP {W/W}",
        "  0.006,                                  !- Design Flow Rate {m3/s}",
        "  7,                                      !- Design Supply Temperature {C}",
        "  11.1,                                   !- Design Temperature Lift {deltaC}",
        "  1,                                      !- Sizing Factor",
        "  NotModulated,                           !- Flow Mode",
        "  DryBulb,                                !- Outdoor Air Temperature Curve Input Variable",
        "  EnteringEvaporator,                     !- Water Temperature Curve Input Variable",
        "  CapCurveFuncTemp,                       !- Normalized Capacity Function of Temperature Curve Name",
        "  EIRCurveFuncTemp,                       !- Fuel Energy Input Ratio Function of Temperature Curve Name",
        "  EIRCurveFuncPLR,                        !- Fuel Energy Input Ratio Function of PLR Curve Name",
        "  0.1,                                    !- Minimum Part Load Ratio",
        "  1,                                      !- Maximum Part Load Ratio",
        "  CRFCurve,                               !- Cycling Ratio Factor Curve Name",
        "  500,                                    !- Nominal Auxiliary Electric Power {W}",
        "  auxElecEIRCurveFuncTempCurve,           !- Auxiliary Electric Energy Input Ratio Function of Temperature Curve Name",
        "  auxElecEIRFoPLRCurve,                   !- Auxiliary Electric Energy Input Ratio Function of PLR Curve Name",
        "  20;                                     !- Standby Electric Power {W}",

        "OutdoorAir:Node,",
        "  FuelFired GAHP Cooling OA Node;         !- Name",

        "Curve:Biquadratic,",
        "  CapCurveFuncTemp,                       !- Name",
        "  1,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 y",
        "  0,                                      !- Coefficient5 y**2",
        "  0,                                      !- Coefficient6 x*y",
        "  5,                                      !- Minimum Value of x {BasedOnField A2}",
        "  10,                                     !- Maximum Value of x {BasedOnField A2}",
        "  24,                                     !- Minimum Value of y {BasedOnField A3}",
        "  35,                                     !- Maximum Value of y {BasedOnField A3}",
        "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
        "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
        "  Temperature,                            !- Input Unit Type for X",
        "  Temperature;                            !- Input Unit Type for Y",

        "Curve:Biquadratic,",
        "  EIRCurveFuncTemp,                       !- Name",
        "  1,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 y",
        "  0,                                      !- Coefficient5 y**2",
        "  0,                                      !- Coefficient6 x*y",
        "  5,                                      !- Minimum Value of x {BasedOnField A2}",
        "  10,                                     !- Maximum Value of x {BasedOnField A2}",
        "  24,                                     !- Minimum Value of y {BasedOnField A3}",
        "  35,                                     !- Maximum Value of y {BasedOnField A3}",
        "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
        "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
        "  Temperature,                            !- Input Unit Type for X",
        "  Temperature;                            !- Input Unit Type for Y",

        "Curve:Quadratic,",
        "  EIRCurveFuncPLR,                        !- Name",
        "  1,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Minimum Value of x {BasedOnField A2}",
        "  1;                                      !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  CRFCurve,                               !- Name",
        "  1,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Minimum Value of x {BasedOnField A2}",
        "  100,                                    !- Maximum Value of x {BasedOnField A2}",
        "  0,                                      !- Minimum Curve Output {BasedOnField A3}",
        "  10;                                     !- Maximum Curve Output {BasedOnField A3}",

        "Curve:Biquadratic,",
        "  auxElecEIRCurveFuncTempCurve,           !- Name",
        "  1,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 y",
        "  0,                                      !- Coefficient5 y**2",
        "  0,                                      !- Coefficient6 x*y",
        "  -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "  100,                                    !- Maximum Value of x {BasedOnField A2}",
        "  -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "  100;                                    !- Maximum Value of y {BasedOnField A3}",

        "Curve:Cubic,",
        "  auxElecEIRFoPLRCurve,                   !- Name",
        "  1,                                      !- Coefficient1 Constant",
        "  0,                                      !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0,                                      !- Coefficient4 x**3",
        "  -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "  100;                                    !- Maximum Value of x {BasedOnField A2}",

        "Curve:Quadratic,",
        "  EIRDefrostFoTCurve,                     !- Name",
        "  1.0317,                                 !- Coefficient1 Constant",
        "  -0.006,                                 !- Coefficient2 x",
        "  -0.0011,                                !- Coefficient3 x**2",
        "  -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "  100,                                    !- Maximum Value of x {BasedOnField A2}",
        "  1,                                      !- Minimum Curve Output {BasedOnField A3}",
        "  10;                                     !- Maximum Curve Output {BasedOnField A3}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // set up the plant loops
    // first the load side
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(2);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    auto &PLHPPlantLoadSideCompHeating = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1);
    PLHPPlantLoadSideCompHeating.Type = DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating;
    PLHPPlantLoadSideCompHeating.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp.allocate(1);
    auto &PLHPPlantLoadSideCompCooling = state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(2).Comp(1);
    PLHPPlantLoadSideCompCooling.Type = DataPlant::PlantEquipmentType::HeatPumpFuelFiredCooling;
    PLHPPlantLoadSideCompCooling.CurOpSchemeType = DataPlant::OpScheme::CompSetPtBased;

    // the init call expects a "from" calling point
    PlantLocation myHeatingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantLocation myCoolingLoadLocation = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 2, 1);

    // call the factory with a valid name to trigger reading inputs
    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredHeating, "FUELFIRED GAHP HEATING");

    EIRFuelFiredHeatPump::factory(*state, DataPlant::PlantEquipmentType::HeatPumpFuelFiredCooling, "FUELFIRED GAHP COOLING");

    // verify the size of the vector and the processed condition
    EXPECT_EQ(2u, state->dataEIRFuelFiredHeatPump->heatPumps.size());

    // for now we know the order is maintained, so get each heat pump object
    const bool is_heating_first = state->dataEIRFuelFiredHeatPump->heatPumps[0].name == "FUELFIRED GAHP HEATING";
    EIRFuelFiredHeatPump *thisHeatingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[is_heating_first ? 0 : 1];
    EIRFuelFiredHeatPump *thisCoolingPLHP = &state->dataEIRFuelFiredHeatPump->heatPumps[is_heating_first ? 1 : 0];
    EXPECT_EQ("FUELFIRED GAHP HEATING", thisHeatingPLHP->name);
    EXPECT_EQ("FUELFIRED GAHP COOLING", thisCoolingPLHP->name);

    // do a bit of extra wiring up to the plant
    PLHPPlantLoadSideCompHeating.Name = thisHeatingPLHP->name;
    PLHPPlantLoadSideCompHeating.NodeNumIn = thisHeatingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSideCompHeating.NodeNumOut = thisHeatingPLHP->loadSideNodes.outlet;

    PLHPPlantLoadSideCompCooling.Name = thisCoolingPLHP->name;
    PLHPPlantLoadSideCompCooling.NodeNumIn = thisCoolingPLHP->loadSideNodes.inlet;
    PLHPPlantLoadSideCompCooling.NodeNumOut = thisCoolingPLHP->loadSideNodes.outlet;

    // call for all initialization
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // I am picking a temperature that is:
    // * Below the 'Maximum Outdoor Dry-bulb Temperature for Defrost Operation' I entered (5.0C)
    // * Between the harcoded min/max defrost temperatures of 16F/-8.88C | 38F/3.33C
    double constexpr oaTemp = 3.0;
    state->dataEnvrn->OutDryBulbTemp = oaTemp;

    double const oaWetbulb = Psychrometrics::PsyTwbFnTdbWPb(*state, oaTemp, 0.0, 101325.0);

    Real64 constexpr expectedLoadMassFlowRate = 0.09999;
    Real64 constexpr expectedCp = 4180;
    Real64 constexpr specifiedLoadSetpoint = 45;

    // This is not the case, even though the E+ I/O Documentation says it should
    constexpr bool isLoadSideHeatTransferNegativeForCooling = false;

    thisHeatingPLHP->onInitLoopEquip(*state, myHeatingLoadLocation);
    {
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::OATempCurveVar::DryBulb, thisHeatingPLHP->oaTempCurveInputVar);
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::WaterTempCurveVar::EnteringCondenser, thisHeatingPLHP->waterTempCurveInputVar);

        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);

        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = oaTemp;

        thisHeatingPLHP->simulate(*state, myHeatingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // EXPECT_NEAR(specifiedLoadSetpoint, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
        {
            ASSERT_GT(thisHeatingPLHP->capFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisHeatingPLHP->capFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(calculatedLoadInletTemp, waterTempforCurve);
            EXPECT_EQ(oaTemp, oaTempforCurve);
        }
        {
            ASSERT_GT(thisHeatingPLHP->powerRatioFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisHeatingPLHP->powerRatioFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(calculatedLoadInletTemp, waterTempforCurve);
            EXPECT_EQ(oaTemp, oaTempforCurve);
        }
        {
            ASSERT_GT(thisHeatingPLHP->defrostEIRCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisHeatingPLHP->defrostEIRCurveIndex);
            Real64 const oaTempforCurve = thisCurve->inputs[0];
            EXPECT_EQ(oaTemp, oaTempforCurve);
        }
    }

    thisCoolingPLHP->onInitLoopEquip(*state, myCoolingLoadLocation);

    {
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::OATempCurveVar::DryBulb, thisCoolingPLHP->oaTempCurveInputVar);
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::WaterTempCurveVar::EnteringEvaporator, thisCoolingPLHP->waterTempCurveInputVar);

        bool firstHVAC = true;
        Real64 curLoad = -800;
        bool runFlag = true;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint + curLoad / (expectedLoadMassFlowRate * expectedCp);

        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = oaTemp;

        thisCoolingPLHP->simulate(*state, myCoolingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // EXPECT_NEAR(specifiedLoadSetpoint, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(isLoadSideHeatTransferNegativeForCooling ? curLoad : -curLoad, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
        {
            ASSERT_GT(thisCoolingPLHP->powerRatioFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisCoolingPLHP->capFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(calculatedLoadInletTemp, waterTempforCurve);
            EXPECT_EQ(oaTemp, oaTempforCurve);
        }
        {
            ASSERT_GT(thisCoolingPLHP->powerRatioFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisCoolingPLHP->powerRatioFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(calculatedLoadInletTemp, waterTempforCurve);
            EXPECT_EQ(oaTemp, oaTempforCurve);
        }
        ASSERT_EQ(0, thisCoolingPLHP->defrostEIRCurveIndex);
    }

    // Now we switch the evaluation variables to Wetbulb and Leaving
    thisHeatingPLHP->oaTempCurveInputVar = EIRFuelFiredHeatPump::OATempCurveVar::WetBulb;
    thisHeatingPLHP->waterTempCurveInputVar = EIRFuelFiredHeatPump::WaterTempCurveVar::LeavingCondenser;
    thisCoolingPLHP->oaTempCurveInputVar = EIRFuelFiredHeatPump::OATempCurveVar::WetBulb;
    thisCoolingPLHP->waterTempCurveInputVar = EIRFuelFiredHeatPump::WaterTempCurveVar::LeavingEvaporator;

    {
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::OATempCurveVar::WetBulb, thisHeatingPLHP->oaTempCurveInputVar);
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::WaterTempCurveVar::LeavingCondenser, thisHeatingPLHP->waterTempCurveInputVar);

        bool firstHVAC = true;
        Real64 curLoad = 800;
        bool runFlag = true;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);

        Real64 const ori_loadSideOutletTemp = thisHeatingPLHP->loadSideOutletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisHeatingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisHeatingPLHP->sourceSideNodes.inlet).Temp = oaTemp;

        thisHeatingPLHP->simulate(*state, myHeatingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // EXPECT_NEAR(specifiedLoadSetpoint, thisHeatingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(curLoad, thisHeatingPLHP->loadSideHeatTransfer, 0.001);
        {
            ASSERT_GT(thisHeatingPLHP->capFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisHeatingPLHP->capFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(ori_loadSideOutletTemp, waterTempforCurve);
            EXPECT_EQ(oaWetbulb, oaTempforCurve);
        }
        {
            ASSERT_GT(thisHeatingPLHP->powerRatioFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisHeatingPLHP->powerRatioFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(ori_loadSideOutletTemp, waterTempforCurve);
            EXPECT_EQ(oaWetbulb, oaTempforCurve);
        }
        {
            ASSERT_GT(thisHeatingPLHP->defrostEIRCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisHeatingPLHP->defrostEIRCurveIndex);
            Real64 const oaTempforCurve = thisCurve->inputs[0];
            EXPECT_EQ(oaWetbulb, oaTempforCurve);
        }
    }

    {
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::OATempCurveVar::WetBulb, thisCoolingPLHP->oaTempCurveInputVar);
        EXPECT_ENUM_EQ(EIRFuelFiredHeatPump::WaterTempCurveVar::LeavingEvaporator, thisCoolingPLHP->waterTempCurveInputVar);

        bool firstHVAC = true;
        Real64 curLoad = -800;
        bool runFlag = true;
        Real64 const calculatedLoadInletTemp = specifiedLoadSetpoint - curLoad / (expectedLoadMassFlowRate * expectedCp);

        Real64 const ori_loadSideOutletTemp = thisCoolingPLHP->loadSideOutletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.inlet).Temp = calculatedLoadInletTemp;
        state->dataLoopNodes->Node(thisCoolingPLHP->loadSideNodes.outlet).TempSetPoint = specifiedLoadSetpoint;
        state->dataLoopNodes->Node(thisCoolingPLHP->sourceSideNodes.inlet).Temp = oaTemp;

        thisCoolingPLHP->simulate(*state, myCoolingLoadLocation, firstHVAC, curLoad, runFlag);
        // expect it to meet setpoint and have some pre-evaluated conditions
        // EXPECT_NEAR(specifiedLoadSetpoint, thisCoolingPLHP->loadSideOutletTemp, 0.001);
        EXPECT_NEAR(isLoadSideHeatTransferNegativeForCooling ? curLoad : -curLoad, thisCoolingPLHP->loadSideHeatTransfer, 0.001);
        {
            ASSERT_GT(thisCoolingPLHP->powerRatioFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisCoolingPLHP->capFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(ori_loadSideOutletTemp, waterTempforCurve);
            EXPECT_EQ(oaWetbulb, oaTempforCurve);
        }
        {
            ASSERT_GT(thisCoolingPLHP->powerRatioFuncTempCurveIndex, 0);
            auto const *thisCurve = state->dataCurveManager->curves(thisCoolingPLHP->powerRatioFuncTempCurveIndex);
            Real64 const waterTempforCurve = thisCurve->inputs[0];
            Real64 const oaTempforCurve = thisCurve->inputs[1];
            EXPECT_EQ(ori_loadSideOutletTemp, waterTempforCurve);
            EXPECT_EQ(oaWetbulb, oaTempforCurve);
        }
        ASSERT_EQ(0, thisCoolingPLHP->defrostEIRCurveIndex);
    }
}
#pragma clang diagnostic pop
#pragma clang diagnostic pop
