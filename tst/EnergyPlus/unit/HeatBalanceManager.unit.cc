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

// EnergyPlus::HeatBalanceManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include <nlohmann/json.hpp>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::ZoneEquipmentManager;
using namespace EnergyPlus::HeatBalanceAirManager;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataAirLoop;
using namespace EnergyPlus::DataAirSystems;
using namespace nlohmann::literals;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalanceManager_ZoneAirBalance_OutdoorAir)
{
    std::string const idf_objects = delimited_string({
        "ZoneAirBalance:OutdoorAir,\n",
        "    LIVING ZONE Balance 1,   !- Name\n",
        "    LIVING ZONE,             !- Zone Name\n",
        "    Quadrature,              !- Air Balance Method\n",
        "    0.01,                    !- Induced Outdoor Air Due to Unbalanced Duct Leakage {m3/s}\n",
        "    INF-SCHED;               !- Induced Outdoor Air Schedule Name",
        "ZoneAirBalance:OutdoorAir,\n",
        "    LIVING ZONE Balance 2,   !- Name\n",
        "    LIVING ZONE,             !- Zone Name\n",
        "    Quadrature,              !- Air Balance Method\n",
        "    0.01,                    !- Induced Outdoor Air Due to Unbalanced Duct Leakage {m3/s}\n",
        "    INF-SCHED2;              !- Induced Outdoor Air Schedule Name",
        "Zone,",
        "LIVING ZONE,             !- Name",
        "0,                       !- Direction of Relative North {deg}",
        "0,                       !- X Origin {m}",
        "0,                       !- Y Origin {m}",
        "0,                       !- Z Origin {m}",
        "1,                       !- Type",
        "1,                       !- Multiplier",
        "autocalculate,           !- Ceiling Height {m}",
        "autocalculate;           !- Volume {m3}",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    auto numZones = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, "Zone");
    state->dataHeatBalFanSys->ZoneReOrder.allocate(numZones);
    GetZoneData(*state, ErrorsFound);
    GetAirFlowFlag(*state, ErrorsFound);
    EXPECT_TRUE(ErrorsFound);
}

#ifdef GET_OUT
TEST_F(EnergyPlusFixture, HeatBalanceManager_WindowMaterial_Gap_Duplicate_Names)
{
    std::string const idf_objects = delimited_string({
        "  WindowMaterial:Gap,",
        "    Gap_1_Layer,             !- Name",
        "    0.0127,                  !- Thickness {m}",
        "    Gas_1_W_0_0127,          !- Gas (or Gas Mixture)",
        "    101325.0000;             !- Pressure {Pa}",
        "  WindowGap:DeflectionState,",
        "    DeflectionState_813_Measured_Gap_1,  !- Name",
        "    0.0120;                  !- Deflected Thickness {m}",
        "  WindowMaterial:Gap,",
        "    Gap_6_Layer,             !- Name",
        "    0.0060,                  !- Thickness {m}",
        "    Gap_6_W_0_0060,          !- Gas (or Gas Mixture)",
        "    101300.0000,             !- Pressure {Pa}",
        "    DeflectionState_813_Measured_Gap_1;  !- Deflection State",
        "  WindowMaterial:Gap,",
        "    Gap_1_Layer,             !- Name",
        "    0.0100,                  !- Thickness {m}",
        "    Gas_1_W_0_0100,          !- Gas (or Gas Mixture)",
        "    101325.0000;             !- Pressure {Pa}",
    });

    ASSERT_FALSE(process_idf(idf_objects, false)); // expect errors
    std::string const error_string = delimited_string({
        "   ** Severe  ** Duplicate name found for object of type \"WindowMaterial:Gap\" named \"Gap_1_Layer\". Overwriting existing object.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    bool ErrorsFound(false);

    Material::GetMaterialData(*state, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_WindowMaterial_Gap_Duplicate_Names_2)
{
    std::string const idf_objects = delimited_string({
        "  WindowGap:DeflectionState,",
        "    DeflectionState_813_Measured_Gap_1,  !- Name",
        "    0.0120;                  !- Deflected Thickness {m}",
        "  WindowMaterial:Gap,",
        "    Gap_6_Layer,             !- Name",
        "    0.0060,                  !- Thickness {m}",
        "    Gap_6_W_0_0060,          !- Gas (or Gas Mixture)",
        "    101300.0000,             !- Pressure {Pa}",
        "    DeflectionState_813_Measured_Gap_1;  !- Deflection State",
        "  WindowMaterial:Gap,",
        "    Gap_1_Layer,             !- Name",
        "    0.0127,                  !- Thickness {m}",
        "    Gas_1_W_0_0127,          !- Gas (or Gas Mixture)",
        "    101325.0000;             !- Pressure {Pa}",
        "  WindowMaterial:Gap,",
        "    Gap_1_Layer,             !- Name",
        "    0.0100,                  !- Thickness {m}",
        "    Gas_1_W_0_0100,          !- Gas (or Gas Mixture)",
        "    101325.0000;             !- Pressure {Pa}",
    });

    ASSERT_FALSE(process_idf(idf_objects, false)); // expect errors
    std::string const error_string = delimited_string({
        "   ** Severe  ** Duplicate name found for object of type \"WindowMaterial:Gap\" named \"Gap_1_Layer\". Overwriting existing object.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    bool ErrorsFound(false);

    Material::GetMaterialData(*state, ErrorsFound);

    EXPECT_FALSE(ErrorsFound);
}
#endif // GET_OUT

TEST_F(EnergyPlusFixture, HeatBalanceManager_ProcessZoneData)
{
    // Test input processing of Zone object
    //	Zone,
    //		ZONE ONE, !- Name
    //		0, !- Direction of Relative North{ deg }
    //		0, 0, 0, !- X, Y, Z{ m }
    //		1, !- Type
    //		1, !- Multiplier
    //		autocalculate, !- Ceiling Height{ m }
    //		autocalculate, !- Volume{ m3 }
    //		, !- Floor Area{ m2 }
    //		AdaptiveConvectionAlgorithm;  !- Zone Inside Convection Algorithm

    bool ErrorsFound(false); // If errors detected in input
    int ZoneNum(0);          // Zone number
    int NumAlphas(2);
    int NumNumbers(9);

    state->dataIPShortCut->cCurrentModuleObject = "Zone";
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);

    // Set up a Zone object
    NumAlphas = 2;
    NumNumbers = 9;
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

    ZoneNum = 1;
    state->dataIPShortCut->cAlphaArgs(1) = "Zone One";                    // Name
    state->dataIPShortCut->rNumericArgs(1) = 0.0;                         // Direction of Relative North[deg]
    state->dataIPShortCut->rNumericArgs(2) = 0.0;                         // X [m]
    state->dataIPShortCut->rNumericArgs(3) = 0.0;                         // Y [m]
    state->dataIPShortCut->rNumericArgs(4) = 0.0;                         // Z [m]
    state->dataIPShortCut->rNumericArgs(5) = 0.0;                         // Type
    state->dataIPShortCut->rNumericArgs(6) = 0.0;                         // Multiplier
    state->dataIPShortCut->lNumericFieldBlanks(7) = true;                 // Ceiling Height{ m }
    state->dataIPShortCut->lNumericFieldBlanks(8) = true;                 // Volume{ m3 }
    state->dataIPShortCut->lNumericFieldBlanks(9) = true;                 // Floor Area{ m2 }
    state->dataIPShortCut->cAlphaArgs(2) = "ADAPTIVECONVECTIONALGORITHM"; // Zone Inside Convection Algorithm - Must be UPPERCASE by this point

    ErrorsFound = false;
    ProcessZoneData(*state,
                    state->dataIPShortCut->cCurrentModuleObject,
                    ZoneNum,
                    state->dataIPShortCut->cAlphaArgs,
                    NumAlphas,
                    state->dataIPShortCut->rNumericArgs,
                    NumNumbers,
                    state->dataIPShortCut->lNumericFieldBlanks,
                    state->dataIPShortCut->lAlphaFieldBlanks,
                    state->dataIPShortCut->cAlphaFieldNames,
                    state->dataIPShortCut->cNumericFieldNames,
                    ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    ZoneNum = 2;
    state->dataIPShortCut->cAlphaArgs(1) = "Zone Two";      // Name
    state->dataIPShortCut->cAlphaArgs(2) = "InvalidChoice"; // Zone Inside Convection Algorithm - Must be UPPERCASE by this point
    ErrorsFound = false;
    ProcessZoneData(*state,
                    state->dataIPShortCut->cCurrentModuleObject,
                    ZoneNum,
                    state->dataIPShortCut->cAlphaArgs,
                    NumAlphas,
                    state->dataIPShortCut->rNumericArgs,
                    NumNumbers,
                    state->dataIPShortCut->lNumericFieldBlanks,
                    state->dataIPShortCut->lAlphaFieldBlanks,
                    state->dataIPShortCut->cAlphaFieldNames,
                    state->dataIPShortCut->cNumericFieldNames,
                    ErrorsFound);
    EXPECT_TRUE(ErrorsFound);

    ZoneNum = 2;
    state->dataIPShortCut->cAlphaArgs(1) = "Zone Two"; // Name
    state->dataIPShortCut->cAlphaArgs(2) = "TARP";     // Zone Inside Convection Algorithm - Must be UPPERCASE by this point
    ErrorsFound = false;
    ProcessZoneData(*state,
                    state->dataIPShortCut->cCurrentModuleObject,
                    ZoneNum,
                    state->dataIPShortCut->cAlphaArgs,
                    NumAlphas,
                    state->dataIPShortCut->rNumericArgs,
                    NumNumbers,
                    state->dataIPShortCut->lNumericFieldBlanks,
                    state->dataIPShortCut->lAlphaFieldBlanks,
                    state->dataIPShortCut->cAlphaFieldNames,
                    state->dataIPShortCut->cNumericFieldNames,
                    ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    EXPECT_EQ("Zone One", state->dataHeatBal->Zone(1).Name);
    EXPECT_EQ(Convect::HcInt::AdaptiveConvectionAlgorithm, state->dataHeatBal->Zone(1).IntConvAlgo);
    EXPECT_EQ("Zone Two", state->dataHeatBal->Zone(2).Name);
    EXPECT_EQ(Convect::HcInt::ASHRAETARP, state->dataHeatBal->Zone(2).IntConvAlgo);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_GetWindowConstructData)
{
    // Test get input for window construction object
    // Construction,
    //	 WINDOWWBLIND, !- Name
    //	 GLASS,        !- Outside Layer
    //	 AIRGAP,       !- Layer 2
    //	 GLASS;        !- Layer 3

    std::string const idf_objects = delimited_string({
        "Construction,",
        " WINDOWWBLIND, !- Name",
        " GLASS,        !- Outside Layer",
        " AIRGAP,       !- Layer 2",
        " GLASS;        !- Layer 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    auto &s_mat = state->dataMaterial;

    bool ErrorsFound(false); // If errors detected in input

    auto *mat1 = new Material::MaterialGlass;
    mat1->group = Material::Group::Glass;
    mat1->Name = "GLASS";
    s_mat->materials.push_back(mat1);
    mat1->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat1->Name, mat1->Num);

    auto *mat2 = new Material::MaterialGasMix;
    mat2->group = Material::Group::Gas;
    mat2->Name = "AIRGAP";
    s_mat->materials.push_back(mat2);
    mat2->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat2->Name, mat2->Num);

    state->dataHeatBal->NominalRforNominalUCalculation.allocate(1);
    state->dataHeatBal->NominalRforNominalUCalculation(1) = 0.0;
    mat1->NominalR = 0.4; // Set these explicity for each material layer to avoid random failures of check for
                          // NominalRforNominalUCalculation == 0.0 at end of GetConstructData
    mat2->NominalR = 0.4;

    // call to get valid window material types
    ErrorsFound = false;
    GetConstructData(*state, ErrorsFound); // returns ErrorsFound as false since all layers are valid
    EXPECT_FALSE(ErrorsFound);

    // Clear shared arrays that were allocated in GetConstructData
    state->dataConstruction->Construct.deallocate();

    // call to get invalid window material type
    //		Material( 2 ).Group = 16; // BlindEquivalentLayer, this layer is invalid in plain windows
    //		ErrorsFound = false;
    //		GetConstructData( ErrorsFound ); // returns ErrorsFound as true since layer 2 is invalid
    //		EXPECT_TRUE( ErrorsFound );
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_ZoneAirMassFlowConservationData1)
{
    // Test get input for ZoneAirMassFlowConservation object

    std::string const idf_objects = delimited_string({
        "Building,",
        "My Building, !- Name",
        "30., !- North Axis{ deg }",
        "City, !- Terrain",
        "0.04, !- Loads Convergence Tolerance Value",
        "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
        "FullExterior, !- Solar Distribution",
        "25, !- Maximum Number of Warmup Days",
        "6;                       !- Minimum Number of Warmup Days",
        "ZoneAirMassFlowConservation,",
        "AdjustMixingOnly, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
        "AddInfiltrationFlow, !- Infiltration Balancing Method",
        "MixingSourceZonesOnly; !- Infiltration Balancing Zones",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false); // If errors detected in input

    // call to process input
    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // returns ErrorsFound false, ZoneAirMassFlowConservation never sets it
    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(state->dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment, DataHeatBalance::AdjustmentType::AdjustMixingOnly);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment, DataHeatBalance::InfiltrationFlow::Add);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.InfiltrationForZones, DataHeatBalance::InfiltrationZoneType::MixingSourceZonesOnly);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_ZoneAirMassFlowConservationData2)
{
    // Test get input for ZoneAirMassFlowConservation object

    std::string const idf_objects = delimited_string({"Building,",
                                                      "My Building, !- Name",
                                                      "30., !- North Axis{ deg }",
                                                      "City, !- Terrain",
                                                      "0.04, !- Loads Convergence Tolerance Value",
                                                      "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
                                                      "FullExterior, !- Solar Distribution",
                                                      "25, !- Maximum Number of Warmup Days",
                                                      "6;                       !- Minimum Number of Warmup Days",
                                                      "ZoneAirMassFlowConservation,",
                                                      "None, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
                                                      "AdjustInfiltrationFlow, !- Infiltration Balancing Method",
                                                      "AllZones;                !- Infiltration Balancing Zones",
                                                      "Zone, Zone 1;",
                                                      "Zone, Zone 2;",
                                                      "ZoneMixing,",
                                                      "Zone 2 Zone Mixing, !- Name",
                                                      "Zone 2, !- Zone Name",
                                                      "Always1, !- Schedule Name",
                                                      "Flow/Zone, !- Design Flow Rate Calculation Method",
                                                      "0.07, !- Design Flow Rate{ m3 / s }",
                                                      ", !- Flow Rate per Zone Floor Area{ m3 / s - m2 }",
                                                      ", !- Flow Rate per Person{ m3 / s - person }",
                                                      ", !- Air Changes per Hour{ 1 / hr }",
                                                      "Zone 1, !- Source Zone Name",
                                                      "0.0;                     !- Delta Temperature{ deltaC }",
                                                      "ZoneInfiltration:DesignFlowRate,",
                                                      "Zone 1 Infil 1, !- Name",
                                                      "Zone 1, !- Zone or ZoneList Name",
                                                      "Always1, !- Schedule Name",
                                                      "flow/zone, !- Design Flow Rate Calculation Method",
                                                      "0.032, !- Design Flow Rate{ m3 / s }",
                                                      ", !- Flow per Zone Floor Area{ m3 / s - m2 }",
                                                      ", !- Flow per Exterior Surface Area{ m3 / s - m2 }",
                                                      ", !- Air Changes per Hour{ 1 / hr }",
                                                      "1, !- Constant Term Coefficient",
                                                      "0, !- Temperature Term Coefficient",
                                                      "0, !- Velocity Term Coefficient",
                                                      "0; !- Velocity Squared Term Coefficient",
                                                      "Schedule:Constant,Always1,,1.0;"

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound(false); // If errors detected in input

    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);

    EXPECT_TRUE(state->dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment, DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment, DataHeatBalance::InfiltrationFlow::Adjust);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.InfiltrationForZones, DataHeatBalance::InfiltrationZoneType::AllZones);

    // setup mixing and infiltration objects
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBalFanSys->ZoneReOrder.allocate(state->dataGlobal->NumOfZones);
    ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    AllocateHeatBalArrays(*state);
    ErrorsFound = false;
    GetSimpleAirModelInputs(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SetZoneMassConservationFlag(*state);
    // setup zone equipment configuration
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);

    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Zone 1";
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).returnFlowFracSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNodeADUNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).AirDistUnitCool.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).AirDistUnitHeat.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNodeADUNum(1) = 0;
    state->dataZoneEquip->ZoneEquipConfig(1).AirDistUnitCool(1).InNode = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNodeAirLoopNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNodeInletNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNodeAirLoopNum(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNodeInletNum(1) = 1;

    state->dataZoneEquip->ZoneEquipConfig(2).ZoneName = "Zone 2";
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneNode = 5;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(1) = 6;
    state->dataZoneEquip->ZoneEquipConfig(2).ExhaustNode(1) = 7;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode(1) = 8;
    state->dataZoneEquip->ZoneEquipConfig(2).FixedReturnFlow.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(2).returnFlowFracSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNodeADUNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).AirDistUnitCool.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).AirDistUnitHeat.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNodeADUNum(1) = 0;
    state->dataZoneEquip->ZoneEquipConfig(2).AirDistUnitCool(1).InNode = 6;
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNodeAirLoopNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNodeInletNum.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNodeAirLoopNum(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNodeInletNum(1) = 1;

    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirLoop->AirLoopFlow.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).OASysExists = true;
    state->dataLoopNodes->Node.allocate(8);

    // Avoid zero values in volume flow balance check
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataEnvrn->OutBaroPress = 100000.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 20.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.004;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(2).ZoneNode).Temp = 20.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(2).ZoneNode).HumRat = 0.004;

    state->dataLoopNodes->Node(1).MassFlowRate = 0.0; // Zone 1 zone node
    state->dataLoopNodes->Node(2).MassFlowRate = 1.0; // Zone 1 inlet node
    state->dataLoopNodes->Node(3).MassFlowRate = 2.0; // Zone 1 exhaust node
    state->dataLoopNodes->Node(4).MassFlowRate = 9.0; // Zone 1 return node
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneExh = 2.0;

    state->dataLoopNodes->Node(5).MassFlowRate = 0.0; // Zone 2 zone node
    state->dataLoopNodes->Node(6).MassFlowRate = 2.0; // Zone 2 inlet node
    state->dataLoopNodes->Node(7).MassFlowRate = 0.0; // Zone 2 exhaust node
    state->dataLoopNodes->Node(8).MassFlowRate = 8.0; // Zone 2 return node
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneExh = 0.0;
    state->dataAirLoop->AirLoopFlow(1).OAFlow = state->dataLoopNodes->Node(2).MassFlowRate + state->dataLoopNodes->Node(6).MassFlowRate;
    state->dataAirLoop->AirLoopFlow(1).MaxOutAir = state->dataAirLoop->AirLoopFlow(1).OAFlow;
    state->dataHeatBal->Infiltration(1).MassFlowRate = 0.5;
    state->dataHeatBal->Mixing(1).MixingMassFlowRate = 0.1;

    // call zone air mass balance
    CalcZoneMassBalance(*state, false);
    EXPECT_EQ(state->dataLoopNodes->Node(4).MassFlowRate, 0.0);       // Zone 1 return node (max(0.0, 1-2)
    EXPECT_EQ(state->dataHeatBal->Infiltration(1).MassFlowRate, 1.0); // Zone 1 infiltration flow rate (2 - 1)
    EXPECT_EQ(state->dataHeatBal->Mixing(1).MixingMassFlowRate, 0.1); // Zone 1 to Zone 2 mixing flow rate (unchanged)
    EXPECT_EQ(state->dataLoopNodes->Node(8).MassFlowRate,
              2.0); // Zone 2 return node (should be 2 now, because this has zone mass conservation active, so return should equal supply)

    state->dataHeatBalFanSys->ZoneReOrder.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataLoopNodes->Node.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataAirLoop->AirLoopFlow.deallocate();
    state->dataHVACGlobal->NumPrimaryAirSys = 0;
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_ZoneAirMassFlowConservationData3)
{
    // Test get input for ZoneAirMassFlowConservation object

    std::string const idf_objects = delimited_string({"Building,",
                                                      "My Building, !- Name",
                                                      "30., !- North Axis{ deg }",
                                                      "City, !- Terrain",
                                                      "0.04, !- Loads Convergence Tolerance Value",
                                                      "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
                                                      "FullExterior, !- Solar Distribution",
                                                      "25, !- Maximum Number of Warmup Days",
                                                      "6;                       !- Minimum Number of Warmup Days",
                                                      "ZoneAirMassFlowConservation,",
                                                      "None, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
                                                      "None, !- Infiltration Balancing Method",
                                                      ";                !- Infiltration Balancing Zones"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false); // If errors detected in input

    // call to process input
    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // returns ErrorsFound false, ZoneAirMassFlowConservation never sets it
    EXPECT_FALSE(ErrorsFound);
    EXPECT_FALSE(state->dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment, DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment, DataHeatBalance::InfiltrationFlow::No);
    EXPECT_ENUM_EQ(state->dataHeatBal->ZoneAirMassFlow.InfiltrationForZones, DataHeatBalance::InfiltrationZoneType::Invalid);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_ZoneAirMassFlowConservationReportVariableTest)
{
    // Test get output variables for ZoneAirMassFlowConservation object #5637

    std::string const idf_objects = delimited_string({
        "Building,",
        "My Building, !- Name",
        "30., !- North Axis{ deg }",
        "City, !- Terrain",
        "0.04, !- Loads Convergence Tolerance Value",
        "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
        "FullExterior, !- Solar Distribution",
        "25, !- Maximum Number of Warmup Days",
        "6;                       !- Minimum Number of Warmup Days",
        "ZoneAirMassFlowConservation,",
        "AdjustMixingOnly, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
        "AdjustInfiltrationFlow, !- Infiltration Balancing Method",
        "AllZones;                !- Infiltration Balancing Zones",

        "  Zone,",
        "    WEST ZONE,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Zone,",
        "    EAST ZONE,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",
        " Output:Variable,",
        "   *, !- Key Value",
        "   Zone Air Mass Balance Exhaust Mass Flow Rate, !- Variable Name",
        "   hourly;                  !- Reporting Frequency",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false); // If errors detected in input

    // call to process input
    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // returns ErrorsFound false, ZoneAirMassFlowConservation never sets it
    EXPECT_FALSE(ErrorsFound);
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBalFanSys->ZoneReOrder.allocate(state->dataGlobal->NumOfZones);
    ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    ErrorsFound = false;
    GetSimpleAirModelInputs(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // first 2 have indexes swapped now since they are in lexicigraphical order now according to the new input processor
    EXPECT_EQ("WEST ZONE:Zone Air Mass Balance Exhaust Mass Flow Rate", state->dataOutputProcessor->outVars[0]->keyColonName);
    EXPECT_EQ("EAST ZONE:Zone Air Mass Balance Exhaust Mass Flow Rate", state->dataOutputProcessor->outVars[1]->keyColonName);
    EXPECT_EQ(1, state->dataOutputProcessor->outVars[0]->ReportID);
    EXPECT_EQ(2, state->dataOutputProcessor->outVars[1]->ReportID);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_GetMaterialRoofVegetation)
{
    std::string const idf_objects = delimited_string({
        "  Material:RoofVegetation,",
        "    ThickSoil,               !- Name",
        "    0.5,                     !- Height of Plants {m}",
        "    5,                       !- Leaf Area Index {dimensionless}",
        "    0.2,                     !- Leaf Reflectivity {dimensionless}",
        "    0.95,                    !- Leaf Emissivity",
        "    180,                     !- Minimum Stomatal Resistance {s/m}",
        "    EcoRoofSoil,             !- Soil Layer Name",
        "    MediumSmooth,            !- Roughness",
        "    0.36,                    !- Thickness {m}",
        "    0.4,                     !- Conductivity of Dry Soil {W/m-K}",
        "    641,                     !- Density of Dry Soil {kg/m3}",
        "    1100,                    !- Specific Heat of Dry Soil {J/kg-K}",
        "    0.95,                    !- Thermal Absorptance",
        "    0.8,                     !- Solar Absorptance",
        "    0.7,                     !- Visible Absorptance",
        "    0.4,                     !- Saturation Volumetric Moisture Content of the Soil Layer",
        "    0.01,                    !- Residual Volumetric Moisture Content of the Soil Layer",
        "    0.45,                    !- Initial Volumetric Moisture Content of the Soil Layer",
        "    Advanced;                !- Moisture Diffusion Calculation Method",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // check the "Material:RoofVegetation" names
    auto const *mat1 = dynamic_cast<Material::MaterialEcoRoof const *>(state->dataMaterial->materials(1));
    EXPECT_EQ(mat1->Name, "THICKSOIL");
    // check maximum (saturated) moisture content
    EXPECT_EQ(0.4, mat1->Porosity);
    // check initial moisture Content was reset
    EXPECT_EQ(0.4, mat1->InitMoisture); // reset from 0.45 to 0.4 during get input
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_WarmUpConvergenceSmallLoadTest)
{

    state->dataGlobal->WarmupFlag = false;
    state->dataGlobal->DayOfSim = 7;
    state->dataHeatBal->MinNumberOfWarmupDays = 25;
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBalMgr->WarmupConvergenceValues.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->TempConvergTol = 0.01;
    state->dataHeatBal->LoadsConvergTol = 0.01;
    state->dataHeatBalMgr->MaxTempPrevDay.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->MaxTempPrevDay(1) = 23.0;
    state->dataHeatBalMgr->MaxTempZone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->MaxTempZone(1) = 23.0;
    state->dataHeatBalMgr->MinTempPrevDay.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->MinTempPrevDay(1) = 23.0;
    state->dataHeatBalMgr->MinTempZone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->MinTempZone(1) = 23.0;
    state->dataHeatBalMgr->MaxHeatLoadZone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->MaxHeatLoadPrevDay.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxHeatLoadValue = 0.0;
    state->dataHeatBalMgr->MaxCoolLoadZone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->MaxCoolLoadPrevDay.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxCoolLoadValue = 0.0;

    // Test 1: All Maxs both less than MinLoad (100.0)
    state->dataHeatBalMgr->MaxHeatLoadZone(1) = 50.0;
    state->dataHeatBalMgr->MaxHeatLoadPrevDay(1) = 90.0;
    state->dataHeatBalMgr->MaxCoolLoadZone(1) = 50.0;
    state->dataHeatBalMgr->MaxCoolLoadPrevDay(1) = 90.0;
    CheckWarmupConvergence(*state);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(3), 2);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(4), 2);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxHeatLoadValue, 0.0, 0.0001);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxCoolLoadValue, 0.0, 0.0001);

    // Test 2: Max Previous Day both less than MinLoad
    state->dataHeatBalMgr->MaxHeatLoadZone(1) = 100.5;
    state->dataHeatBalMgr->MaxHeatLoadPrevDay(1) = 90.0;
    state->dataHeatBalMgr->MaxCoolLoadZone(1) = 100.5;
    state->dataHeatBalMgr->MaxCoolLoadPrevDay(1) = 90.0;
    CheckWarmupConvergence(*state);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(3), 2);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(4), 2);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxHeatLoadValue, 0.005, 0.0001);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxCoolLoadValue, 0.005, 0.0001);

    // Test 3: Max Current Day both less than MinLoad
    state->dataHeatBalMgr->MaxHeatLoadZone(1) = 90.0;
    state->dataHeatBalMgr->MaxHeatLoadPrevDay(1) = 100.5;
    state->dataHeatBalMgr->MaxCoolLoadZone(1) = 90.0;
    state->dataHeatBalMgr->MaxCoolLoadPrevDay(1) = 100.5;
    CheckWarmupConvergence(*state);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(3), 2);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(4), 2);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxHeatLoadValue, 0.005, 0.0001);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxCoolLoadValue, 0.005, 0.0001);

    // Test 4: Everything greater than MinLoad (pass convergence test)
    state->dataHeatBalMgr->MaxHeatLoadZone(1) = 201.0;
    state->dataHeatBalMgr->MaxHeatLoadPrevDay(1) = 200.0;
    state->dataHeatBalMgr->MaxCoolLoadZone(1) = 201.0;
    state->dataHeatBalMgr->MaxCoolLoadPrevDay(1) = 200.0;
    CheckWarmupConvergence(*state);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(3), 2);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(4), 2);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxHeatLoadValue, 0.005, 0.0001);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxCoolLoadValue, 0.005, 0.0001);

    // Test 5: Everything greater than MinLoad (fail convergence test)
    state->dataHeatBalMgr->MaxHeatLoadZone(1) = 210.0;
    state->dataHeatBalMgr->MaxHeatLoadPrevDay(1) = 200.0;
    state->dataHeatBalMgr->MaxCoolLoadZone(1) = 210.0;
    state->dataHeatBalMgr->MaxCoolLoadPrevDay(1) = 200.0;
    CheckWarmupConvergence(*state);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(3), 1);
    EXPECT_EQ(state->dataHeatBalMgr->WarmupConvergenceValues(1).PassFlag(4), 1);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxHeatLoadValue, 0.05, 0.005);
    EXPECT_NEAR(state->dataHeatBalMgr->WarmupConvergenceValues(1).TestMaxCoolLoadValue, 0.05, 0.005);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_TestZonePropertyLocalEnv)
{

    std::string const idf_objects = delimited_string({

        "  Building,",
        "    House with Local Air Nodes,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

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
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

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
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material,",
        "    GP01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1600000,               !- Conductivity {W/m-K}",
        "    801.0000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                 !- Surface Type",
        "    ROOF,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ZoneProperty:LocalEnvironment,",
        "    LocEnv:LIVING ZONE,           !- Name",
        "    LIVING ZONE,                  !- Exterior Surface Name",
        "    OutdoorAirNode:0001;          !- Outdoor Air Node Name",

        "  OutdoorAir:Node,",
        "    OutdoorAirNode:0001,          !- Name",
        "    ,                             !- Height Above Ground",
        "    OutdoorAirNodeDryBulb:0001,   !- Drybulb Temperature Schedule Name",
        "    OutdoorAirNodeWetBulb:0001,   !- Wetbulb Schedule Name",
        "    OutdoorAirNodeWindSpeed:0001, !- Wind Speed Schedule Name",
        "    OutdoorAirNodeWindDir:0001;   !- Wind Direction Schedule Name",

        "  ScheduleTypeLimits,",
        "    Any Number;                   !- Name",

        "  Schedule:Compact,",
        "    OutdoorAirNodeDryBulb:0001,   !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    OutdoorAirNodeWetBulb:0001,   !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 12.0;           !- Field 3",

        "  Schedule:Compact,",
        "    OutdoorAirNodeWindSpeed:0001, !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 1.23;           !- Field 3",

        "  Schedule:Compact,",
        "    OutdoorAirNodeWindDir:0001,   !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 90;             !- Field 3"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "LIVING ZONE";
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataHeatBal->SurfTempEffBulkAir.allocate(6);

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;

    state->dataGlobal->KickOffSimulation = true;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    // Set up
    OutAirNodeManager::GetOutAirNodesInput(*state);
    state->dataEnvrn->OutBaroPress = 101325;

    Sched::GetSchedule(*state, "OUTDOORAIRNODEDRYBULB:0001")->currentVal = 25.0;
    Sched::GetSchedule(*state, "OUTDOORAIRNODEWETBULB:0001")->currentVal = 20.0;
    Sched::GetSchedule(*state, "OUTDOORAIRNODEWINDSPEED:0001")->currentVal = 1.5;
    Sched::GetSchedule(*state, "OUTDOORAIRNODEWINDDIR:0001")->currentVal = 90.0;

    OutAirNodeManager::InitOutAirNodes(*state);

    // Test if local nodes data correctly overwritten
    EXPECT_EQ(25.0, state->dataLoopNodes->Node(1).OutAirDryBulb);
    EXPECT_EQ(20.0, state->dataLoopNodes->Node(1).OutAirWetBulb);
    EXPECT_EQ(1.5, state->dataLoopNodes->Node(1).OutAirWindSpeed);
    EXPECT_EQ(90.0, state->dataLoopNodes->Node(1).OutAirWindDir);
    EXPECT_NEAR(0.012611481326656135, state->dataLoopNodes->Node(1).HumRat, 0.000000000000001);
    EXPECT_NEAR(57247.660939392081, state->dataLoopNodes->Node(1).Enthalpy, 0.000000001);

    InitHeatBalance(*state);

    // Test if local value correctly overwritten
    EXPECT_EQ(25.0, state->dataHeatBal->Zone(1).OutDryBulbTemp);
    EXPECT_EQ(20.0, state->dataHeatBal->Zone(1).OutWetBulbTemp);
    EXPECT_EQ(1.5, state->dataHeatBal->Zone(1).WindSpeed);
    EXPECT_EQ(90.0, state->dataHeatBal->Zone(1).WindDir);

    // Add a test for #7308 without inputs of schedule names
    state->dataLoopNodes->Node(1).outAirDryBulbSched = nullptr;
    state->dataLoopNodes->Node(1).outAirWetBulbSched = nullptr;
    state->dataLoopNodes->Node(1).outAirWindSpeedSched = nullptr;
    state->dataLoopNodes->Node(1).outAirWindDirSched = nullptr;
    state->dataEnvrn->OutDryBulbTemp = 25.0;
    state->dataEnvrn->OutWetBulbTemp = 20.0;
    state->dataEnvrn->WindSpeed = 1.5;
    state->dataEnvrn->WindDir = 90.0;

    InitHeatBalance(*state);

    // Test if local value correctly overwritten
    EXPECT_EQ(25.0, state->dataHeatBal->Zone(1).OutDryBulbTemp);
    EXPECT_EQ(20.0, state->dataHeatBal->Zone(1).OutWetBulbTemp);
    EXPECT_EQ(1.5, state->dataHeatBal->Zone(1).WindSpeed);
    EXPECT_EQ(90.0, state->dataHeatBal->Zone(1).WindDir);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HVACSystemRootFindingAlgorithmInputTest)
{
    // Test eio output for HVACSystemRootFindingAlgorithm

    std::string const idf_objects = delimited_string({
        "Building,",
        "My Building, !- Name",
        "30., !- North Axis{ deg }",
        "City, !- Terrain",
        "0.04, !- Loads Convergence Tolerance Value",
        "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
        "FullExterior, !- Solar Distribution",
        "25, !- Maximum Number of Warmup Days",
        "6;                       !- Minimum Number of Warmup Days",
        "ZoneAirMassFlowConservation,",
        "None, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
        "None, !- Infiltration Balancing Method",
        ";                !- Infiltration Balancing Zones",
        " HVACSystemRootFindingAlgorithm,",
        " RegulaFalsiThenBisection,!- Algorithm",
        " 5;                       !- Number of Iterations Before Algorithm Switch",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false); // If errors detected in input
    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // returns ErrorsFound false
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ(state->dataRootFinder->HVACSystemRootFinding.Algorithm, "REGULAFALSITHENBISECTION");
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HVACSystemRootFindingAlgorithmNoInputTest)
{
    // Test that root solver algorithm is RegulaFalsi when no HVACSystemRootFindingAlgorithm object exists

    std::string const idf_objects = delimited_string({
        "Building,",
        "My Building, !- Name",
        "30., !- North Axis{ deg }",
        "City, !- Terrain",
        "0.04, !- Loads Convergence Tolerance Value",
        "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
        "FullExterior, !- Solar Distribution",
        "25, !- Maximum Number of Warmup Days",
        "6;                       !- Minimum Number of Warmup Days",
        "ZoneAirMassFlowConservation,",
        "None, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
        "None, !- Infiltration Balancing Method",
        ";                !- Infiltration Balancing Zones",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false); // If errors detected in input
    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // returns ErrorsFound false
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ(state->dataRootFinder->HVACSystemRootFinding.Algorithm, "RegulaFalsi");
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_EMSConstructionTest)
{

    state->dataIPShortCut->lAlphaFieldBlanks = true;

    std::string const idf_objects = delimited_string({
        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    SunnyWinterDay,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    5.0,                    !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    4.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",
        "Material,",
        "  Concrete Block,          !- Name",
        "  MediumRough,             !- Roughness",
        "  0.1014984,               !- Thickness {m}",
        "  0.3805070,               !- Conductivity {W/m-K}",
        "  608.7016,                !- Density {kg/m3}",
        "  836.8000;                !- Specific Heat {J/kg-K}",
        "Construction,",
        "  WallConstruction,        !- Name",
        "  Concrete Block;          !- Outside Layer",
        "  WindowMaterial:Glazing,",
        "    ELECTRO GLASS DARK STATE,!- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.111,                   !- Solar Transmittance at Normal Incidence",
        "    0.179,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.179,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.128,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.0001,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.0001,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Glazing,",
        "    ELECTRO GLASS LIGHT STATE,!- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.9,                   !- Solar Transmittance at Normal Incidence",
        "    0.1,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.1,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.9,                   !- Visible Transmittance at Normal Incidence",
        "    0.1,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.1,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.0001,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.0001,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "Construction,",
        "  WindowConstruction1,      !- Name",
        "  ELECTRO GLASS LIGHT STATE;          !- Outside Layer",
        "Construction,",
        "  WindowConstruction2,      !- Name",
        "  ELECTRO GLASS DARK STATE;          !- Outside Layer",
        "FenestrationSurface:Detailed,",
        "  FenestrationSurface,     !- Name",
        "  Window,                  !- Surface Type",
        "  WindowConstruction1,      !- Construction Name",
        "  Wall,                    !- Building Surface Name",
        "  ,                        !- Outside Boundary Condition Object",
        "  0.5000000,               !- View Factor to Ground",
        "  ,                        !- Frame and Divider Name",
        "  1.0,                     !- Multiplier",
        "  4,                       !- Number of Vertices",
        "  0.200000,0.000000,9.900000,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.200000,0.000000,0.1000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  9.900000,0.000000,0.1000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  9.900000,0.000000,9.900000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,"
        "  Wall,                    !- Name",
        "  Wall,                    !- Surface Type",
        "  WallConstruction,        !- Construction Name",
        "  Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  SunExposed,              !- Sun Exposure",
        "  WindExposed,             !- Wind Exposure",
        "  0.5000000,               !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  0.000000,0.000000,10.00000,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  10.00000,0.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  10.00000,0.000000,10.00000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,"
        "  Floor,                   !- Name",
        "  Floor,                   !- Surface Type",
        "  WallConstruction,        !- Construction Name",
        "  Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  1.0,                     !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.000000,10.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  10.00000,10.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  10.00000,0.000000,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "Zone,"
        "  Zone,                    !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  6.000000,                !- X Origin {m}",
        "  6.000000,                !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",
        "  Daylighting:Controls,",
        "    Daylighting Control,!- Name",
        "    Zone,          !- Zone Name",
        "    SplitFlux,               !- Daylighting Method",
        "    ,                        !- Availability Schedule Name",
        "    Continuous,              !- Lighting Control Type",
        "    0.3,                     !- Minimum Input Power Fraction for Continuous or ContinuousOff Dimming Control",
        "    0.2,                     !- Minimum Light Output Fraction for Continuous or ContinuousOff Dimming Control",
        "    1,                       !- Number of Stepped Control Steps",
        "    1,                       !- Probability Lighting will be Reset When Needed in Manual Stepped Control",
        "    ,                        !- Glare Calculation Daylighting Reference Point Name",
        "    ,                        !- Glare Calculation Azimuth Angle of View Direction Clockwise from Zone y-Axis {deg}",
        "    22,                      !- Maximum Allowable Discomfort Glare Index",
        "    ,                        !- DElight Gridding Resolution {m2}",
        "    Reference Point 1,  !- Daylighting Reference Point 1 Name",
        "    1,                       !- Fraction of Zone Controlled by Reference Point 1",
        "    500;                     !- Illuminance Setpoint at Reference Point 1 {lux}",
        "",
        "  Daylighting:ReferencePoint,",
        "    Reference Point 1,  !- Name",
        "    Zone,          !- Zone Name",
        "    12,                      !- X-Coordinate of Reference Point {m}",
        "    2.5,                     !- Y-Coordinate of Reference Point {m}",
        "    0.8;                     !- Z-Coordinate of Reference Point {m}",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    30,                       !- Shading Calculation Update Frequency",
        "    15000;                   !- Maximum Figures in Shadow Overlap Calculations",
        "EnergyManagementSystem:ConstructionIndexVariable, Win_1, WINDOWCONSTRUCTION1;",
        "EnergyManagementSystem:ConstructionIndexVariable, Win_2, WINDOWCONSTRUCTION2;",
        "  EnergyManagementSystem:Actuator,",
        "    Win1_Construct,          !- Name",
        "    FenestrationSurface,  !- Actuated Component Unique Name",
        "    Surface,                 !- Actuated Component Type",
        "    Construction State;      !- Actuated Component Control Type",
        "",
        "  EnergyManagementSystem:ProgramCallingManager,",
        "    Window Switcher,  !- Name",
        "    BeginTimestepBeforePredictor,  !- EnergyPlus Model Calling Point",
        "    ZN_1_wall_south_Window_1_Control;  !- Program Name 1",
        "",
        "  EnergyManagementSystem:Program,",
        "    ZN_1_wall_south_Window_1_Control,  !- Name",
        "    IF Hour > 12,    !- Program Line 1",
        "    Set Win1_Construct = Win_2,  !- Program Line 2",
        "    ELSE,                    !- <none>",
        "    SET Win1_Construct = Win_1,  !- <none>",
        "    ENDIF;                   !- <none>",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    // OutputProcessor::TimeValue.allocate(2);
    SimulationManager::ManageSimulation(*state);
    state->dataGlobal->DayOfSim = 2; // avoid array bounds problem in RecKeepHeatBalance
    state->dataWeather->Envrn = 1;

    // Test 1 - Set time of day to morning - should use high transmittance window
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 11;
    state->dataGlobal->CurrentTime = 11.0;
    Weather::SetCurrentWeather(*state);
    HeatBalanceManager::ManageHeatBalance(*state);
    // For now, must call this twice in order to hit the BeginTimeStepBeforePredictor EMS calling point
    HeatBalanceManager::ManageHeatBalance(*state);
    // Find the fenestration surface
    int winSurfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    int win1ConstNum = Util::FindItemInList("WINDOWCONSTRUCTION1", state->dataConstruction->Construct);
    EXPECT_EQ(state->dataSurface->Surface(winSurfNum).Construction, win1ConstNum);
    Real64 transSol = state->dataSurface->SurfWinSysSolTransmittance(winSurfNum);
    EXPECT_GT(transSol, 0.8);
    Real64 refPtIllum = state->dataDayltg->daylightControl(1).refPts(1).lums[(int)DataSurfaces::Lum::Illum];
    EXPECT_GT(refPtIllum, 3000.0);

    // Test 2 - Set time of day to afternoon - should use low transmittance window
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 14;
    state->dataGlobal->CurrentTime = 14.0;
    Weather::SetCurrentWeather(*state);
    HeatBalanceManager::ManageHeatBalance(*state);
    // For now, must call this twice in order to hit the BeginTimeStepBeforePredictor EMS calling point
    HeatBalanceManager::ManageHeatBalance(*state);
    int win2ConstNum = Util::FindItemInList("WINDOWCONSTRUCTION2", state->dataConstruction->Construct);
    EXPECT_EQ(state->dataSurface->Surface(winSurfNum).Construction, win2ConstNum);
    transSol = state->dataSurface->SurfWinSysSolTransmittance(winSurfNum);
    EXPECT_LT(transSol, 0.2);
    refPtIllum = state->dataDayltg->daylightControl(1).refPts(1).lums[(int)DataSurfaces::Lum::Illum];
    EXPECT_LT(refPtIllum, 1000.0);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HeatBalanceAlgorithm_Default)
{
    // Test various inputs for HeatBalanceAlgorithm
    // Default is CTF if no HeatBalanceAlgorithm object is present

    EXPECT_FALSE(state->dataHeatBal->AnyCTF);
    bool errorsfound = false;

    std::string const idf_objects = delimited_string({
        "  Building, My Building;",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    HeatBalanceManager::GetProjectControlData(*state, errorsfound);
    EXPECT_FALSE(errorsfound);
    EXPECT_TRUE(state->dataHeatBal->AnyCTF);
    EXPECT_FALSE(state->dataHeatBal->AnyEMPD);
    EXPECT_FALSE(state->dataHeatBal->AnyCondFD);
    EXPECT_FALSE(state->dataHeatBal->AnyHAMT);
    EXPECT_ENUM_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::CTF);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HeatBalanceAlgorithm_CTF)
{
    // Test various inputs for HeatBalanceAlgorithm

    EXPECT_FALSE(state->dataHeatBal->AnyCTF);
    bool errorsfound = false;

    std::string const idf_objects = delimited_string({
        "  Building, My Building;",
        "  HeatBalanceAlgorithm,",
        "  ConductionTransferFunction, !- Algorithm",
        "  205.2,                      !- Surface Temperature Upper Limit",
        "  0.004,                      !- Minimum Surface Convection Heat Transfer Coefficient Value",
        "  200.6;                      !- Maximum Surface Convection Heat Transfer Coefficient Value",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    HeatBalanceManager::GetProjectControlData(*state, errorsfound);
    EXPECT_FALSE(errorsfound);
    EXPECT_TRUE(state->dataHeatBal->AnyCTF);
    EXPECT_FALSE(state->dataHeatBal->AnyEMPD);
    EXPECT_FALSE(state->dataHeatBal->AnyCondFD);
    EXPECT_FALSE(state->dataHeatBal->AnyHAMT);
    EXPECT_ENUM_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::CTF);
    EXPECT_EQ(state->dataHeatBalSurf->MaxSurfaceTempLimit, 205.2);
    EXPECT_EQ(state->dataHeatBal->LowHConvLimit, 0.004);
    EXPECT_EQ(state->dataHeatBal->HighHConvLimit, 200.6);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HeatBalanceAlgorithm_EMPD)
{
    // Test various inputs for HeatBalanceAlgorithm

    bool errorsfound = false;

    std::string const idf_objects = delimited_string({
        "  Building, My Building;",
        "  HeatBalanceAlgorithm,",
        "  MoisturePenetrationDepthConductionTransferFunction; !- Algorithm",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    HeatBalanceManager::GetProjectControlData(*state, errorsfound);
    EXPECT_FALSE(errorsfound);
    EXPECT_FALSE(state->dataHeatBal->AnyCTF);
    EXPECT_TRUE(state->dataHeatBal->AnyEMPD);
    EXPECT_FALSE(state->dataHeatBal->AnyCondFD);
    EXPECT_FALSE(state->dataHeatBal->AnyHAMT);
    EXPECT_ENUM_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::EMPD);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HeatBalanceAlgorithm_CondFD)
{
    // Test various inputs for HeatBalanceAlgorithm

    bool errorsfound = false;

    std::string const idf_objects = delimited_string({
        "  Building, My Building;",
        "  HeatBalanceAlgorithm,",
        "  ConductionFiniteDifference; !- Algorithm",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    HeatBalanceManager::GetProjectControlData(*state, errorsfound);
    EXPECT_FALSE(errorsfound);
    EXPECT_FALSE(state->dataHeatBal->AnyCTF);
    EXPECT_FALSE(state->dataHeatBal->AnyEMPD);
    EXPECT_TRUE(state->dataHeatBal->AnyCondFD);
    EXPECT_FALSE(state->dataHeatBal->AnyHAMT);
    EXPECT_ENUM_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::CondFD);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HeatBalanceAlgorithm_HAMT)
{
    // Test various inputs for HeatBalanceAlgorithm

    bool errorsfound = false;

    std::string const idf_objects = delimited_string({
        "  Building, My Building;",
        "  HeatBalanceAlgorithm,",
        "  CombinedHeatAndMoistureFiniteElement; !- Algorithm",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    HeatBalanceManager::GetProjectControlData(*state, errorsfound);
    EXPECT_FALSE(errorsfound);
    EXPECT_FALSE(state->dataHeatBal->AnyCTF);
    EXPECT_FALSE(state->dataHeatBal->AnyEMPD);
    EXPECT_FALSE(state->dataHeatBal->AnyCondFD);
    EXPECT_TRUE(state->dataHeatBal->AnyHAMT);
    EXPECT_ENUM_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::HAMT);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_GlazingEquivalentLayer_RValue)
{

    bool errorsfound = false;

    std::string const idf_objects = delimited_string({
        "  Building, My Building;",
        "WindowMaterial:Glazing:EquivalentLayer,",
        "GLZCLR,                  !- Name",
        "SpectralAverage,         !- Optical Data Type",
        ",                        !- Window Glass Spectral Data Set Name",
        "0.77,                    !- Front Side Beam-Beam Solar Transmittance {dimensionless}",
        "0.77,                    !- Back Side Beam-Beam Solar Transmittance {dimensionless}",
        "0.07,                    !- Front Side Beam-Beam Solar Reflectance {dimensionless}",
        "0.07,                    !- Back Side Beam-Beam Solar Reflectance {dimensionless}",
        "0.0,                     !- Front Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "0.0,                     !- Back Side Beam-Beam Visible Solar Transmittance {dimensionless}",
        "0.0,                     !- Front Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "0.0,                     !- Back Side Beam-Beam Visible Solar Reflectance {dimensionless}",
        "0.0,                     !- Front Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "0.0,                     !- Back Side Beam-Diffuse Solar Transmittance {dimensionless}",
        "0.0,                     !- Front Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "0.0,                     !- Back Side Beam-Diffuse Solar Reflectance {dimensionless}",
        "0.0,                     !- Front Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "0.0,                     !- Back Side Beam-Diffuse Visible Solar Transmittance {dimensionless}",
        "0.0,                     !- Front Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "0.0,                     !- Back Side Beam-Diffuse Visible Solar Reflectance {dimensionless}",
        "0.695,                   !- Diffuse-Diffuse Solar Transmittance {dimensionless}",
        "0.16,                    !- Front Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "0.16,                    !- Back Side Diffuse-Diffuse Solar Reflectance {dimensionless}",
        "0.0,                     !- Diffuse-Diffuse Visible Solar Transmittance {dimensionless}",
        "0.0,                     !- Front Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "0.0,                     !- Back Side Diffuse-Diffuse Visible Solar Reflectance {dimensionless}",
        "0.0,                     !- Infrared Transmittance (applies to front and back) {dimensionless}",
        "0.84,                    !- Front Side Infrared Emissivity {dimensionless}",
        "0.84;                    !- Back Side Infrared Emissivity {dimensionless}",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    Material::GetMaterialData(*state, errorsfound);

    EXPECT_FALSE(errorsfound);
    EXPECT_NEAR(state->dataMaterial->materials(1)->Resistance, 0.158, 0.0001);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_GetAirBoundaryConstructData)
{

    std::string const idf_objects = delimited_string({

        "Construction:AirBoundary,",
        "Grouped Air Boundary, !- Name",
        "None;                    !- Air Exchange Method",

        "Construction:AirBoundary,",
        "Air Boundary with Good Mixing Schedule, !- Name",
        "SimpleMixing,            !- Air Exchange Method",
        "0.4,                     !- Simple Mixing Air Changes per Hour {1 / hr}",
        "Always2;                 !- Simple Mixing Schedule Name",

        "Schedule:Constant,Always2,,2.0;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);

    // get constructions
    ErrorsFound = false;
    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    EXPECT_EQ(state->dataHeatBal->TotConstructs, 2);

    int constrNum = Util::FindItemInList(Util::makeUPPER("Grouped Air Boundary"), state->dataConstruction->Construct);
    EXPECT_TRUE(Util::SameString(state->dataConstruction->Construct(constrNum).Name, "Grouped Air Boundary"));
    EXPECT_TRUE(state->dataConstruction->Construct(constrNum).TypeIsAirBoundary);
    EXPECT_FALSE(state->dataConstruction->Construct(constrNum).IsUsedCTF);
    EXPECT_FALSE(state->dataConstruction->Construct(constrNum).TypeIsAirBoundaryMixing);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).TotLayers, 0);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).AirBoundaryACH, 0.0); // Not processed for GroupedZone mixing option
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).airBoundaryMixingSched, nullptr);
    EXPECT_EQ(state->dataHeatBal->NominalRforNominalUCalculation(constrNum), 0.0);

    constrNum = Util::FindItemInList(Util::makeUPPER("Air Boundary with Good Mixing Schedule"), state->dataConstruction->Construct);
    EXPECT_TRUE(Util::SameString(state->dataConstruction->Construct(constrNum).Name, "Air Boundary with Good Mixing Schedule"));
    EXPECT_TRUE(state->dataConstruction->Construct(constrNum).TypeIsAirBoundary);
    EXPECT_FALSE(state->dataConstruction->Construct(constrNum).IsUsedCTF);
    EXPECT_TRUE(state->dataConstruction->Construct(constrNum).TypeIsAirBoundaryMixing);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).TotLayers, 0);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).AirBoundaryACH, 0.4);
    EXPECT_NE(state->dataConstruction->Construct(constrNum).airBoundaryMixingSched, nullptr);
    EXPECT_EQ(state->dataHeatBal->NominalRforNominalUCalculation(constrNum), 0.0);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_GetAirBoundaryConstructData2)
{

    std::string const idf_objects = delimited_string({

        "Construction:AirBoundary,",
        "Air Boundary with Bad Mixing Schedule, !- Name",
        "SimpleMixing,            !- Air Exchange Method",
        "0.1,                     !- Simple Mixing Air Changes per Hour {1 / hr}",
        "xyz;                     !- Simple Mixing Schedule Name",

        "Schedule:Constant,Always2,,2.0;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound(false);

    // skip call to get material data since this doesn't use IRT
    ErrorsFound = false;
    EXPECT_EQ(state->dataMaterial->materials.size(), 0);

    // get constructions
    ErrorsFound = false;
    GetConstructData(*state, ErrorsFound);
    EXPECT_TRUE(ErrorsFound);

    std::string const error_string =
        delimited_string({"   ** Warning ** ProcessScheduleInput: Schedule:Constant = ALWAYS2",
                          "   **   ~~~   ** Schedule Type Limits Name is empty.",
                          "   **   ~~~   ** Schedule will not be validated.",
                          "   ** Severe  ** CreateAirBoundaryConstructions: Construction:AirBoundary = Air Boundary with Bad Mixing Schedule",
                          "   **   ~~~   ** Simple Mixing Schedule Name = xyz, item not found.",
                          "   ** Severe  ** Errors found in creating the constructions defined with Construction:AirBoundary.",
                          "   ** Warning ** This building has no thermal mass which can cause an unstable solution.",
                          "   **   ~~~   ** Use Material object for all opaque material definitions except very light insulation layers."});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    EXPECT_EQ(state->dataHeatBal->TotConstructs, 1);

    int constrNum = Util::FindItemInList(Util::makeUPPER("Air Boundary with Bad Mixing Schedule"), state->dataConstruction->Construct);
    EXPECT_TRUE(Util::SameString(state->dataConstruction->Construct(constrNum).Name, "Air Boundary with Bad Mixing Schedule"));
    EXPECT_TRUE(state->dataConstruction->Construct(constrNum).TypeIsAirBoundary);
    EXPECT_FALSE(state->dataConstruction->Construct(constrNum).IsUsedCTF);
    EXPECT_TRUE(state->dataConstruction->Construct(constrNum).TypeIsAirBoundaryMixing);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).TotLayers, 0);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).AirBoundaryACH, 0.1);
    EXPECT_EQ(state->dataConstruction->Construct(constrNum).airBoundaryMixingSched, nullptr);
    EXPECT_EQ(state->dataHeatBal->NominalRforNominalUCalculation(constrNum), 0.0);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_UpdateWindowFaceTempsNonBSDFWin)
{

    state->dataSurface->TotSurfaces = 3;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBal->TotConstructs = 3;
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);

    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(3).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Construction = 2;
    state->dataSurface->Surface(3).Construction = 3;
    state->dataSurface->AllHTWindowSurfaceList.push_back(2);
    state->dataSurface->AllHTWindowSurfaceList.push_back(3);
    state->dataConstruction->Construct(1).WindowTypeBSDF = false;
    state->dataConstruction->Construct(2).WindowTypeBSDF = false;
    state->dataConstruction->Construct(3).WindowTypeBSDF = true;

    int SurfsForRegWindow = 3;
    state->dataConstruction->Construct(1).TotLayers = 1;
    state->dataConstruction->Construct(2).TotLayers = SurfsForRegWindow;
    state->dataConstruction->Construct(3).TotLayers = 1;

    state->dataHeatBal->SurfWinFenLaySurfTempFront.dimension(state->dataSurface->TotSurfaces, 10, 0.0);
    state->dataHeatBal->SurfWinFenLaySurfTempBack.dimension(state->dataSurface->TotSurfaces, 10, 0.0);
    state->dataHeatBalSurf->SurfOutsideTempHist.allocate(1);
    state->dataHeatBalSurf->SurfInsideTempHist.allocate(1);
    state->dataHeatBalSurf->SurfOutsideTempHist(1).dimension(state->dataSurface->TotSurfaces, 0.0);
    state->dataHeatBalSurf->SurfInsideTempHist(1).dimension(state->dataSurface->TotSurfaces, 0.0);

    state->dataHeatBalSurf->SurfOutsideTempHist(1)(1) = 21.0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(2) = 22.0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(3) = 23.0;
    state->dataHeatBalSurf->SurfInsideTempHist(1)(1) = 34.0;
    state->dataHeatBalSurf->SurfInsideTempHist(1)(2) = 35.0;
    state->dataHeatBalSurf->SurfInsideTempHist(1)(3) = 36.0;

    Real64 ZeroResult = 0.0;

    HeatBalanceManager::UpdateWindowFaceTempsNonBSDFWin(*state);

    // First surface is NOT a window so these should NOT be set
    EXPECT_NEAR(state->dataHeatBal->SurfWinFenLaySurfTempFront(1, 1), ZeroResult, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->SurfWinFenLaySurfTempBack(1, 1), ZeroResult, 0.0001);

    // Second surface is a window so these should be set
    EXPECT_NEAR(state->dataHeatBal->SurfWinFenLaySurfTempFront(2, 1), state->dataHeatBalSurf->SurfOutsideTempHist(1)(2), 0.0001);
    EXPECT_NEAR(state->dataHeatBal->SurfWinFenLaySurfTempBack(2, SurfsForRegWindow), state->dataHeatBalSurf->SurfInsideTempHist(1)(2), 0.0001);

    // Third surface is a window but is also a BSDF (complex window) so these should NOT be set
    EXPECT_NEAR(state->dataHeatBal->SurfWinFenLaySurfTempFront(3, 1), ZeroResult, 0.0001);
    EXPECT_NEAR(state->dataHeatBal->SurfWinFenLaySurfTempBack(3, 1), ZeroResult, 0.0001);
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_HVACSystemRootFindingAlgorithmBisectionInputTest)
{
    // Test eio output for HVACSystemRootFindingAlgorithm

    std::string const idf_objects = delimited_string({
        "Building,",
        "My Building, !- Name",
        "30., !- North Axis{ deg }",
        "City, !- Terrain",
        "0.04, !- Loads Convergence Tolerance Value",
        "0.4, !- Temperature Convergence Tolerance Value{ deltaC }",
        "FullExterior, !- Solar Distribution",
        "25, !- Maximum Number of Warmup Days",
        "6;                       !- Minimum Number of Warmup Days",
        "ZoneAirMassFlowConservation,",
        "None, !- Adjust Zone Mixing and Return For Air Mass Flow Balance",
        "None, !- Infiltration Balancing Method",
        ";                !- Infiltration Balancing Zones",
        " HVACSystemRootFindingAlgorithm,",
        " Bisection;!- Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false); // If errors detected in input
    ErrorsFound = false;
    GetProjectControlData(*state, ErrorsFound); // returns ErrorsFound false
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ(state->dataRootFinder->HVACSystemRootFinding.Algorithm, "BISECTION");
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_EMSConstructionSwitchTest)
{

    state->dataIPShortCut->lAlphaFieldBlanks = true;

    std::string const idf_objects = delimited_string({
        "Version,9.3;",
        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  SizingPeriod:DesignDay,",
        "    SunnyWinterDay,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    5.0,                    !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    4.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    83411.,                  !- Barometric Pressure {Pa}",
        "    4,                       !- Wind Speed {m/s}",
        "    120,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  Site:Location,",
        "    Denver Stapleton Intl Arpt CO USA WMO=724690,  !- Name",
        "    39.77,                   !- Latitude {deg}",
        "    -104.87,                 !- Longitude {deg}",
        "    -7.00,                   !- Time Zone {hr}",
        "    1611.00;                 !- Elevation {m}",
        "Material,",
        "  Concrete Block,          !- Name",
        "  MediumRough,             !- Roughness",
        "  0.1014984,               !- Thickness {m}",
        "  0.3805070,               !- Conductivity {W/m-K}",
        "  608.7016,                !- Density {kg/m3}",
        "  836.8000;                !- Specific Heat {J/kg-K}",
        "Construction,",
        "  WallConstruction,        !- Name",
        "  Concrete Block;          !- Outside Layer",
        "  WindowMaterial:Glazing,",
        "    ELECTRO GLASS DARK STATE,!- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.111,                   !- Solar Transmittance at Normal Incidence",
        "    0.179,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.179,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.128,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.0001,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.0001,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Glazing,",
        "    ELECTRO GLASS LIGHT STATE,!- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.9,                   !- Solar Transmittance at Normal Incidence",
        "    0.1,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.1,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.9,                   !- Visible Transmittance at Normal Incidence",
        "    0.1,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.1,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.0001,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.0001,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "Construction,",
        "  WindowConstruction1,      !- Name",
        "  ELECTRO GLASS LIGHT STATE;          !- Outside Layer",
        "Construction,",
        "  WindowConstruction2,      !- Name",
        "  ELECTRO GLASS DARK STATE;          !- Outside Layer",
        "FenestrationSurface:Detailed,",
        "  FenestrationSurface,     !- Name",
        "  Window,                  !- Surface Type",
        "  WindowConstruction1,      !- Construction Name",
        "  Wall,                    !- Building Surface Name",
        "  ,                        !- Outside Boundary Condition Object",
        "  0.5000000,               !- View Factor to Ground",
        "  ,                        !- Frame and Divider Name",
        "  1.0,                     !- Multiplier",
        "  4,                       !- Number of Vertices",
        "  0.200000,0.000000,9.900000,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.200000,0.000000,0.1000000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  9.900000,0.000000,0.1000000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  9.900000,0.000000,9.900000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,"
        "  Wall,                    !- Name",
        "  Wall,                    !- Surface Type",
        "  WallConstruction,        !- Construction Name",
        "  Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  SunExposed,              !- Sun Exposure",
        "  WindExposed,             !- Wind Exposure",
        "  0.5000000,               !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  0.000000,0.000000,10.00000,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  10.00000,0.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  10.00000,0.000000,10.00000;  !- X,Y,Z ==> Vertex 4 {m}",
        "BuildingSurface:Detailed,"
        "  Floor,                   !- Name",
        "  Floor,                   !- Surface Type",
        "  WallConstruction,        !- Construction Name",
        "  Zone,                    !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  1.0,                     !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  0.000000,0.000000,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.000000,10.000000,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  10.00000,10.000000,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  10.00000,0.000000,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "Zone,"
        "  Zone,                    !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  6.000000,                !- X Origin {m}",
        "  6.000000,                !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",
        "  Daylighting:Controls,",
        "    Daylighting Control,!- Name",
        "    Zone,          !- Zone Name",
        "    SplitFlux,               !- Daylighting Method",
        "    ,                        !- Availability Schedule Name",
        "    Continuous,              !- Lighting Control Type",
        "    0.3,                     !- Minimum Input Power Fraction for Continuous or ContinuousOff Dimming Control",
        "    0.2,                     !- Minimum Light Output Fraction for Continuous or ContinuousOff Dimming Control",
        "    1,                       !- Number of Stepped Control Steps",
        "    1,                       !- Probability Lighting will be Reset When Needed in Manual Stepped Control",
        "    ,                        !- Glare Calculation Daylighting Reference Point Name",
        "    ,                        !- Glare Calculation Azimuth Angle of View Direction Clockwise from Zone y-Axis {deg}",
        "    22,                      !- Maximum Allowable Discomfort Glare Index",
        "    ,                        !- DElight Gridding Resolution {m2}",
        "    Reference Point 1,  !- Daylighting Reference Point 1 Name",
        "    1,                       !- Fraction of Zone Controlled by Reference Point 1",
        "    500;                     !- Illuminance Setpoint at Reference Point 1 {lux}",
        "",
        "  Daylighting:ReferencePoint,",
        "    Reference Point 1,  !- Name",
        "    Zone,          !- Zone Name",
        "    12,                      !- X-Coordinate of Reference Point {m}",
        "    2.5,                     !- Y-Coordinate of Reference Point {m}",
        "    0.8;                     !- Z-Coordinate of Reference Point {m}",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    30,                       !- Shading Calculation Update Frequency",
        "    15000;                   !- Maximum Figures in Shadow Overlap Calculations",
        "EnergyManagementSystem:ConstructionIndexVariable, Win_1, WINDOWCONSTRUCTION1;",
        "EnergyManagementSystem:ConstructionIndexVariable, Win_2, WINDOWCONSTRUCTION2;",
        "  EnergyManagementSystem:Actuator,",
        "    Win1_Construct,          !- Name",
        "    FenestrationSurface,  !- Actuated Component Unique Name",
        "    Surface,                 !- Actuated Component Type",
        "    Construction State;      !- Actuated Component Control Type",
        "",
        "  EnergyManagementSystem:ProgramCallingManager,",
        "    Window Switcher,  !- Name",
        "    BeginZoneTimestepBeforeInitHeatBalance,  !- EnergyPlus Model Calling Point",
        "    ZN_1_wall_south_Window_1_Control;  !- Program Name 1",
        "",
        "  EnergyManagementSystem:Program,",
        "    ZN_1_wall_south_Window_1_Control,  !- Name",
        "    Set Win1_Construct = Win_2;  !- Program Line 2",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    SimulationManager::ManageSimulation(*state);

    int surfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    EXPECT_EQ(state->dataSurface->Surface(surfNum).Construction, state->dataSurface->SurfEMSConstructionOverrideValue(surfNum));
    EXPECT_TRUE(state->dataSurface->SurfEMSConstructionOverrideON(surfNum));
}

TEST_F(EnergyPlusFixture, HeatBalanceManager_GetSpaceData)
{
    // Test input processing of Space object

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
            },
            "Zone 2" : {
            }
        },
        "Space": {
            "Space 1a" : {
                 "zone_name": "Zone 1"
            },
            "Space 1b" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0,
                 "space_type": "Office",
                 "tags": [
                    {
                        "tag": "Tag1"
                    },
                    {
                        "tag": "Tag2"
                    }
                ]
            }
        },
        "SpaceList": {
            "SomeSpaces" : {
                 "spaces": [
                    {
                        "space_name": "Space 1a"
                    },
                    {
                        "space_name": "Space 1b"
                    }
                ]
            }
        }
    }
    )"_json;

    state->dataGlobal->isEpJSON = true;
    state->dataInputProcessing->inputProcessor->initializeMaps();

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    compare_err_stream("");
    EXPECT_FALSE(ErrorsFound);

    int zoneNum = 1;
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(2, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[1]).Name);
    zoneNum = 2;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(1, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);

    int spaceNum = 1;
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("GENERAL", state->dataHeatBal->space(spaceNum).spaceType);
    EXPECT_EQ("GENERAL", state->dataHeatBal->spaceTypes(state->dataHeatBal->space(spaceNum).spaceTypeNum));
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    // Defaults
    EXPECT_EQ(-99999, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_TRUE(state->dataHeatBal->space(spaceNum).tags.empty());

    spaceNum = 2;
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("OFFICE", state->dataHeatBal->space(spaceNum).spaceType);
    EXPECT_EQ("OFFICE", state->dataHeatBal->spaceTypes(state->dataHeatBal->space(spaceNum).spaceTypeNum));
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    EXPECT_EQ(100, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ("TAG1", state->dataHeatBal->space(spaceNum).tags(1));
    EXPECT_EQ("TAG2", state->dataHeatBal->space(spaceNum).tags(2));

    EXPECT_EQ("SOMESPACES", state->dataHeatBal->spaceList(1).Name);
    EXPECT_EQ(2, state->dataHeatBal->spaceList(1).spaces.size());
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->spaceList(1).spaces[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->spaceList(1).spaces[1]).Name);

    // This space is auto-generated
    spaceNum = 3;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("GENERAL", state->dataHeatBal->space(spaceNum).spaceType);
    EXPECT_EQ("GENERAL", state->dataHeatBal->spaceTypes(state->dataHeatBal->space(spaceNum).spaceTypeNum));
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    // Defaults
    EXPECT_EQ(-99999, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_TRUE(state->dataHeatBal->space(spaceNum).tags.empty());
}

TEST_F(EnergyPlusFixture, Window5DataFileSpaceInName)
{

    fs::path window5DataFilePath;
    window5DataFilePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/Window5DataFile_NameWithSpace.dat";
    std::string ConstructName{"DOUBLE CLEAR"};
    bool ConstructionFound{false};
    bool EOFonW5File{false};
    bool ErrorsFound{false};
    state->dataHeatBal->MaxSolidWinLayers = 2;

    SearchWindow5DataFile(*state, window5DataFilePath, ConstructName, ConstructionFound, EOFonW5File, ErrorsFound);

    EXPECT_EQ(ConstructName, "DOUBLE CLEAR");
    EXPECT_TRUE(ConstructionFound);
}

TEST_F(EnergyPlusFixture, Window5DataExtraBlankLines)
{

    fs::path window5DataFilePath;
    window5DataFilePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/Window5DataFile_ExtraBlankLines.dat";
    std::string ConstructName{"EXAMPLE B"};
    bool ConstructionFound{false};
    bool EOFonW5File{false};
    bool ErrorsFound{false};
    state->dataHeatBal->MaxSolidWinLayers = 2;

    SearchWindow5DataFile(*state, window5DataFilePath, ConstructName, ConstructionFound, EOFonW5File, ErrorsFound);

    EXPECT_EQ(ConstructName, "EXAMPLE B");
    EXPECT_TRUE(ConstructionFound);
}

TEST_F(EnergyPlusFixture, Window5DataFileZeroSCSHGC)
{

    fs::path window5DataFilePath;
    window5DataFilePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/Window5DataFile_ZeroSCandSHGC.dat";
    std::string ConstructName{"DOUBLE CLEAR"};
    bool ConstructionFound{false};
    bool EOFonW5File{false};
    bool ErrorsFound{false};
    state->dataHeatBal->MaxSolidWinLayers = 2;

    SearchWindow5DataFile(*state, window5DataFilePath, ConstructName, ConstructionFound, EOFonW5File, ErrorsFound);

    EXPECT_EQ(ConstructName, "DOUBLE CLEAR");
    EXPECT_FALSE(ErrorsFound);

    std::string error_string = delimited_string({"   ** Warning ** HeatBalanceManager: SearchWindow5DataFile: Construction=DOUBLE CLEAR from the "
                                                 "Window5 data file has flawed data: it has a Shading Coefficient <= 0 in glazing system 1",
                                                 "   ** Warning ** HeatBalanceManager: SearchWindow5DataFile: Construction=DOUBLE CLEAR from the "
                                                 "Window5 data file has flawed data: it has a SHGC <= 0 in glazing system 1"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, ReadIncidentSolarMultiplierInput_invalidSched)
{
    std::string const idf_objects_invalid_sch = delimited_string({
        "Material:NoMass,",
        "R13LAYER,                !- Name",
        "Rough,                   !- Roughness",
        "2.290965,                !- Thermal Resistance {m2-K/W}",
        "0.9000000,               !- Thermal Absorptance",
        "0.7500000,               !- Solar Absorptance",
        "0.7500000;               !- Visible Absorptance",

        "Construction,",
        "R13WALL,                 !- Name",
        "R13LAYER;                !- Outside Layer",

        "CONSTRUCTION,",
        "GlzSys_1,                !- Name",
        "Glass_102_LayerAvg;      !- Outside Layer",

        "Construction,",
        "FLOOR,                   !- Name",
        "C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "Material,",
        "C5 - 4 IN HW CONCRETE,   !- Name",
        "MediumRough,             !- Roughness",
        "0.1014984,               !- Thickness {m}",
        "1.729577,                !- Conductivity {W/m-K}",
        "2242.585,                !- Density {kg/m3}",
        "836.8000,                !- Specific Heat {J/kg-K}",
        "0.9000000,               !- Thermal Absorptance",
        "0.6500000,               !- Solar Absorptance",
        "0.6500000;               !- Visible Absorptance",

        "WindowMaterial:Glazing,",
        "Glass_102_LayerAvg,      !- Name",
        "SpectralAverage,         !- Optical Data Type",
        ",                        !- Window Glass Spectral Data Set Name",
        "0.003048,                !- Thickness {m}",
        "0.833848,                !- Solar Transmittance at Normal Incidence",
        "7.476376e-002,           !- Front Side Solar Reflectance at Normal Incidence",
        "7.485449e-002,           !- Back Side Solar Reflectance at Normal Incidence",
        "0.899260,                !- Visible Transmittance at Normal Incidence",
        "0.082563,                !- Front Side Visible Reflectance at Normal Incidence",
        "0.082564,                !- Back Side Visible Reflectance at Normal Incidence",
        "0.000000,                !- Infrared Transmittance at Normal Incidence",
        "0.840000,                !- Front Side Infrared Hemispherical Emissivity",
        "0.840000,                !- Back Side Infrared Hemispherical Emissivity",
        "1.000000;                !- Conductivity {W/m-K}",

        "BuildingSurface:Detailed,",
        "Zn001:Wall001,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "Zn001:Wall001:Win001,    !- Name",
        "Window,                  !- Surface Type",
        "GlzSys_1,                !- Construction Name",
        "Zn001:Wall001,           !- Building Surface Name",
        ",                        !- Outside Boundary Condition Object",
        "0.5000000,               !- View Factor to Ground",
        ",                        !- Frame and Divider Name",
        "1.0,                     !- Multiplier",
        "4,                       !- Number of Vertices",
        "0.548000,0,2.5000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.548000,0,0.5000,  !- X,Y,Z ==> Vertex 2 {m}",
        "5.548000,0,0.5000,  !- X,Y,Z ==> Vertex 3 {m}",
        "5.548000,0,2.5000;  !- X,Y,Z ==> Vertex 4 {m}",

        "SurfaceProperty:IncidentSolarMultiplier,",
        "Zn001:Wall001:Win001,     !- Surface Name",
        "0.6,                      !- Shading Multiplier",
        "wrongSchedule;            !- Shading Multiplier Schedule Name",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "Schedule:Compact,",
        "  SolarMultCompact,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0.1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  0.3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  0.1;                       !- Field 12",
    });
    ASSERT_TRUE(process_idf(idf_objects_invalid_sch));
    bool ErrorsFound = false;
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    state->dataSurface->TotSurfaces = 2;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->Surface(1).Name = "ZN001:WALL001";
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Name = "ZN001:WALL001:WIN001";
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(2).Construction = 2;
    GetIncidentSolarMultiplier(*state, ErrorsFound);
    std::string error_string =
        delimited_string({"   ** Severe  ** GetIncidentSolarMultiplier: SurfaceProperty:IncidentSolarMultiplier = ZN001:WALL001:WIN001",
                          "   **   ~~~   ** Incident Solar Multiplier Schedule Name = WRONGSCHEDULE, item not found."});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, ReadIncidentSolarMultiplierInput)
{

    std::string const idf_objects = delimited_string({

        "Material:NoMass,",
        "R13LAYER,                !- Name",
        "Rough,                   !- Roughness",
        "2.290965,                !- Thermal Resistance {m2-K/W}",
        "0.9000000,               !- Thermal Absorptance",
        "0.7500000,               !- Solar Absorptance",
        "0.7500000;               !- Visible Absorptance",

        "Construction,",
        "R13WALL,                 !- Name",
        "R13LAYER;                !- Outside Layer",

        "CONSTRUCTION,",
        "GlzSys_1,                !- Name",
        "Glass_102_LayerAvg;      !- Outside Layer",

        "Construction,",
        "FLOOR,                   !- Name",
        "C5 - 4 IN HW CONCRETE;   !- Outside Layer",

        "Material,",
        "C5 - 4 IN HW CONCRETE,   !- Name",
        "MediumRough,             !- Roughness",
        "0.1014984,               !- Thickness {m}",
        "1.729577,                !- Conductivity {W/m-K}",
        "2242.585,                !- Density {kg/m3}",
        "836.8000,                !- Specific Heat {J/kg-K}",
        "0.9000000,               !- Thermal Absorptance",
        "0.6500000,               !- Solar Absorptance",
        "0.6500000;               !- Visible Absorptance",

        "WindowMaterial:Glazing,",
        "Glass_102_LayerAvg,      !- Name",
        "SpectralAverage,         !- Optical Data Type",
        ",                        !- Window Glass Spectral Data Set Name",
        "0.003048,                !- Thickness {m}",
        "0.833848,                !- Solar Transmittance at Normal Incidence",
        "7.476376e-002,           !- Front Side Solar Reflectance at Normal Incidence",
        "7.485449e-002,           !- Back Side Solar Reflectance at Normal Incidence",
        "0.899260,                !- Visible Transmittance at Normal Incidence",
        "0.082563,                !- Front Side Visible Reflectance at Normal Incidence",
        "0.082564,                !- Back Side Visible Reflectance at Normal Incidence",
        "0.000000,                !- Infrared Transmittance at Normal Incidence",
        "0.840000,                !- Front Side Infrared Hemispherical Emissivity",
        "0.840000,                !- Back Side Infrared Hemispherical Emissivity",
        "1.000000;                !- Conductivity {W/m-K}",

        "BuildingSurface:Detailed,",
        "Zn001:Wall001,           !- Name",
        "Wall,                    !- Surface Type",
        "R13WALL,                 !- Construction Name",
        "ZONE ONE,                !- Zone Name",
        ",                        !- Space Name",
        "Outdoors,                !- Outside Boundary Condition",
        ",                        !- Outside Boundary Condition Object",
        "SunExposed,              !- Sun Exposure",
        "WindExposed,             !- Wind Exposure",
        "0.5000000,               !- View Factor to Ground",
        "4,                       !- Number of Vertices",
        "0,0,4.572000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "15.24000,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "15.24000,0,4.572000;  !- X,Y,Z ==> Vertex 4 {m}",

        "FenestrationSurface:Detailed,",
        "Zn001:Wall001:Win001,    !- Name",
        "Window,                  !- Surface Type",
        "GlzSys_1,                !- Construction Name",
        "Zn001:Wall001,           !- Building Surface Name",
        ",                        !- Outside Boundary Condition Object",
        "0.5000000,               !- View Factor to Ground",
        ",                        !- Frame and Divider Name",
        "1.0,                     !- Multiplier",
        "4,                       !- Number of Vertices",
        "0.548000,0,2.5000,  !- X,Y,Z ==> Vertex 1 {m}",
        "0.548000,0,0.5000,  !- X,Y,Z ==> Vertex 2 {m}",
        "5.548000,0,0.5000,  !- X,Y,Z ==> Vertex 3 {m}",
        "5.548000,0,2.5000;  !- X,Y,Z ==> Vertex 4 {m}",

        "SurfaceProperty:IncidentSolarMultiplier,",
        "Zn001:Wall001:Win001,     !- Surface Name",
        "0.6,                      !- Shading Multiplier",
        "SolarMultCompact;           !- Shading Multiplier Schedule Name",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "Schedule:Compact,",
        "  SolarMultCompact,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0.1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  0.3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  0.1;                       !- Field 12",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;

    state->init_state(*state);

    bool ErrorsFound = false;

    state->dataEnvrn->Month = 5;
    state->dataEnvrn->DayOfMonth = 31;
    state->dataGlobal->HourOfDay = 24;
    state->dataEnvrn->DayOfWeek = 4;
    state->dataEnvrn->DayOfWeekTomorrow = 5;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 0);
    Sched::UpdateScheduleVals(*state);

    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurface->TotSurfaces = 2;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->Surface(1).Name = "ZN001:WALL001";
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Name = "ZN001:WALL001:WIN001";
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(2).Construction = 2;
    GetIncidentSolarMultiplier(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_EQ(state->dataSurface->SurfIncSolMultiplier(2).Scaler, 0.6);
    EXPECT_EQ(state->dataSurface->SurfIncSolMultiplier(2).sched->Name, "SOLARMULTCOMPACT");

    EXPECT_EQ(state->dataSurface->SurfIncSolMultiplier(2).sched->getCurrentVal(), 0.1);

    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Door;
    GetIncidentSolarMultiplier(*state, ErrorsFound);
    std::string error_string = delimited_string({"   ** Severe  ** IncidentSolarMultiplier defined for non-window surfaces"});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(2).ExtBoundCond = 1; // internal
    GetIncidentSolarMultiplier(*state, ErrorsFound);
    error_string = delimited_string({"   ** Severe  ** IncidentSolarMultiplier defined for interior surfaces"});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    state->dataSurface->Surface(2).ExtBoundCond = DataSurfaces::ExternalEnvironment;
    state->dataSurface->Surface(2).HasShadeControl = true;
    GetIncidentSolarMultiplier(*state, ErrorsFound);
    error_string =
        delimited_string({"   ** Severe  ** Non-compatible shades defined alongside SurfaceProperty:IncidentSolarMultiplier for the same window"});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    state->dataSurface->Surface(2).HasShadeControl = false;
    state->dataMaterial->materials(3)->group = Material::Group::Screen;
    GetIncidentSolarMultiplier(*state, ErrorsFound);
    error_string =
        delimited_string({"   ** Severe  ** Non-compatible shades defined alongside SurfaceProperty:IncidentSolarMultiplier for the same window"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}
} // namespace EnergyPlus
