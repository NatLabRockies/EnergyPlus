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
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

#include <nlohmann/json.hpp>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace nlohmann::literals;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalanceAirManager_RoomAirModelType_Test)
{
    // Issue User file with RoomAirSettings:AirflowNetwork is failing with fatal error #6086

    std::string const idf_objects = delimited_string({
        "  RoomAirModelType,",
        "  Skinny_Model,            !- Name",
        "  South Skin,              !- Zone Name",
        "  AirflowNetwork,          !- Room - Air Modeling Type",
        "  Direct;                  !- Air Temperature Coupling Strategy",

        "  RoomAirModelType,",
        "  Phat_Model,              !- Name",
        "  Thermal Zone,            !- Zone Name",
        "  AirflowNetwork,          !- Room - Air Modeling Type",
        "  Direct;                  !- Air Temperature Coupling Strategy",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->NumOfZones = 2;

    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "SOUTH SKIN";
    state->dataHeatBal->Zone(2).Name = "THERMAL ZONE";

    bool ErrorsFound(false);

    HeatBalanceAirManager::GetRoomAirModelParameters(*state, ErrorsFound);

    EXPECT_TRUE(ErrorsFound);

    std::string const error_string = delimited_string({
        "   ** Severe  ** In RoomAirModelType = SKINNY_MODEL: Room-Air Modeling Type = AIRFLOWNETWORK.",
        "   **   ~~~   ** This model requires AirflowNetwork:* objects to form a complete network, including AirflowNetwork:Intrazone:Node and "
        "AirflowNetwork:Intrazone:Linkage.",
        "   **   ~~~   ** AirflowNetwork:SimulationControl not found.",
        "   ** Severe  ** In RoomAirModelType = PHAT_MODEL: Room-Air Modeling Type = AIRFLOWNETWORK.",
        "   **   ~~~   ** This model requires AirflowNetwork:* objects to form a complete network, including AirflowNetwork:Intrazone:Node and "
        "AirflowNetwork:Intrazone:Linkage.",
        "   **   ~~~   ** AirflowNetwork:SimulationControl not found.",
        "   ** Severe  ** Errors found in processing input for RoomAirModelType",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}
TEST_F(EnergyPlusFixture, HeatBalanceAirManager_GetInfiltrationAndVentilation)
{
    // Test input processing of Infiltration objects with spaces

    state->dataInputProcessing->inputProcessor->epJSON = R"(
    {
        "Zone": {
            "Zone 1" : {
                "volume": 100.0
            },
            "Zone 2" : {
                "volume": 2000.0,
                 "floor_area": 1000.0
            }
        },
        "Space": {
            "Space 1a" : {
                 "zone_name": "Zone 1",
                 "floor_area": 10.0
            },
            "Space 1b" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0
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
        },
        "ZoneList": {
            "AllZones" : {
                 "zones": [
                    {
                        "zone_name": "Zone 1"
                    },
                    {
                        "zone_name": "Zone 2"
                    }
                ]
            }
        },
        "Building": {
            "Some building somewhere": {
            }
        },
        "GlobalGeometryRules": {
            "GlobalGeometryRules 1": {
                "coordinate_system": "Relative",
                "starting_vertex_position": "UpperLeftCorner",
                "vertex_entry_direction": "Counterclockwise"
            }
        },
        "Construction": {
            "ext-slab": {
                "outside_layer": "HW CONCRETE"
            }
        },
        "Material": {
            "HW CONCRETE": {
                "conductivity": 1.311,
                "density": 2240.0,
                "roughness": "Rough",
                "solar_absorptance": 0.7,
                "specific_heat": 836.8,
                "thermal_absorptance": 0.9,
                "thickness": 0.1016,
                "visible_absorptance": 0.7
            }
        },
        "BuildingSurface:Detailed": {
            "Dummy Space 1a Floor": {
                "zone_name": "Zone 1",
                "space_name": "Space 1a",
                "surface_type": "Floor",
                "construction_name": "ext-slab",
                "number_of_vertices": 4,
                "outside_boundary_condition": "adiabatic",
                "sun_exposure": "nosun",
                "vertices": [
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    }
                ]
            },
            "Dummy Space 1b Floor": {
                "zone_name": "Zone 1",
                "space_name": "Space 1b",
                "surface_type": "Floor",
                "construction_name": "ext-slab",
                "number_of_vertices": 4,
                "outside_boundary_condition": "adiabatic",
                "sun_exposure": "nosun",
                "vertices": [
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    }
                ]
            },
            "Dummy Zone 2 Floor": {
                "zone_name": "Zone 2",
                "surface_type": "Floor",
                "construction_name": "ext-slab",
                "number_of_vertices": 4,
                "outside_boundary_condition": "adiabatic",
                "sun_exposure": "nosun",
                "vertices": [
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    }
                ]
            }
        },
        "ZoneInfiltration:DesignFlowRate": {
            "Zone1Infiltration": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 1.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Zone 1",
                "density_basis": "Standard"
            },
            "Zone2Infiltration": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 2.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Zone 2",
                "density_basis": "Indoor"
            },
            "Space1aInfiltration": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 3.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Space 1a",
                "density_basis": "Outdoor"
            },
            "Space1bInfiltration": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 4.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Space 1b"
            },
            "SomeSpacesInfiltration": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 5.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "SomeSpaces"
            },
            "AllZonesInfiltration": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 6.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "AllZones"
            }
        },
        "ZoneVentilation:DesignFlowRate": {
            "Zone1Ventilation": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 1.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Zone 1",
                "density_basis": "Standard"
            },
            "Zone2Ventilation": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 2.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Zone 2",
                "density_basis": "Indoor"
            },
            "Space1aVentilation": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 3.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Space 1a",
                "density_basis": "Outdoor"
            },
            "Space1bVentilation": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 4.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "Space 1b"
            },
            "SomeSpacesVentilation": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 5.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "SomeSpaces"
            },
            "AllZonesVentilation": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 6.0,
                "zone_or_zonelist_or_space_or_spacelist_name": "AllZones"
            }
        }
    }
    )"_json;

    state->dataGlobal->isEpJSON = true;
    state->dataInputProcessing->inputProcessor->initializeMaps();

    // GetZoneData and others use GetObjectItem with ipshortcuts so these need to be allocated
    // copied from InputProcessor::processInput (which expects an input file to be present)
    int MaxArgs = 0;
    int MaxAlpha = 0;
    int MaxNumeric = 0;
    state->dataInputProcessing->inputProcessor->getMaxSchemaArgs(MaxArgs, MaxAlpha, MaxNumeric);

    state->dataIPShortCut->cAlphaFieldNames.allocate(MaxAlpha);
    state->dataIPShortCut->cAlphaArgs.allocate(MaxAlpha);
    state->dataIPShortCut->lAlphaFieldBlanks.dimension(MaxAlpha, false);
    state->dataIPShortCut->cNumericFieldNames.allocate(MaxNumeric);
    state->dataIPShortCut->rNumericArgs.dimension(MaxNumeric, 0.0);
    state->dataIPShortCut->lNumericFieldBlanks.dimension(MaxNumeric, false);

    bool ErrorsFound = false;

    state->init_state(*state);

    HeatBalanceManager::GetHeatBalanceInput(*state);
    std::string const error_string = delimited_string(
        {format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
         "   ** Warning ** No Timestep object found.  Number of TimeSteps in Hour defaulted to 4.",
         "   ** Warning ** GetSurfaceData: Entered Space Floor Area(s) differ more than 5% from calculated Space Floor Area(s).",
         "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual Spaces.",
         "   ** Warning ** CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; ",
         "   ** Warning ** CalcApproximateViewFactors: Zero area for all other zone surfaces.",
         "   **   ~~~   ** Happens for Surface=\"DUMMY SPACE 1A FLOOR\" in Zone=ZONE 1",
         "   ** Warning ** CalcApproximateViewFactors: Zero area for all other zone surfaces.",
         "   **   ~~~   ** Happens for Surface=\"DUMMY SPACE 1B FLOOR\" in Zone=ZONE 1",
         "   ** Warning ** Surfaces in Zone/Enclosure=\"ZONE 1\" do not define an enclosure.",
         "   **   ~~~   ** Number of surfaces <= 3, view factors are set to force reciprocity but may not fulfill completeness.",
         "   **   ~~~   ** Reciprocity means that radiant exchange between two surfaces will match and not lead to an energy loss.",
         "   **   ~~~   ** Completeness means that all of the view factors between a surface and the other surfaces in a zone add up to unity.",
         "   **   ~~~   ** So, when there are three or less surfaces in a zone, EnergyPlus will make sure there are no losses of energy but",
         "   **   ~~~   ** it will not exchange the full amount of radiation with the rest of the zone as it would if there was a completed "
         "enclosure."});

    compare_err_stream(error_string, true);
    EXPECT_FALSE(ErrorsFound);

    state->dataHeatBalFanSys->ZoneReOrder.allocate(state->dataGlobal->NumOfZones);
    ErrorsFound = false;
    HeatBalanceAirManager::GetSimpleAirModelInputs(*state, ErrorsFound);
    compare_err_stream("", true);
    EXPECT_FALSE(ErrorsFound);

    // Expected floor areas
    Real64 constexpr Space1aFloorArea = 10.0;
    Real64 constexpr Space1bFloorArea = 100.0;
    Real64 constexpr Zone2FloorArea = 1000.0;

    // Zone and space nums
    int const zone1 = Util::FindItemInList("ZONE 1", state->dataHeatBal->Zone);
    int const zone2 = Util::FindItemInList("ZONE 2", state->dataHeatBal->Zone);
    int const space1a = Util::FindItemInList("SPACE 1A", state->dataHeatBal->space);
    int const space1b = Util::FindItemInList("SPACE 1B", state->dataHeatBal->space);
    int const spaceZone2 = Util::FindItemInList("ZONE 2", state->dataHeatBal->space);

    int zoneNum = zone1;
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(2, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[1]).Name);
    zoneNum = zone2;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(1, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);

    int spaceNum = space1a;
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    EXPECT_EQ(Space1aFloorArea, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ(Space1aFloorArea, state->dataHeatBal->space(spaceNum).FloorArea);

    spaceNum = space1b;
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    EXPECT_EQ(Space1bFloorArea, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ(Space1bFloorArea, state->dataHeatBal->space(spaceNum).FloorArea);

    EXPECT_EQ("SOMESPACES", state->dataHeatBal->spaceList(1).Name);
    EXPECT_EQ(2, state->dataHeatBal->spaceList(1).spaces.size());
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->spaceList(1).spaces[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->spaceList(1).spaces[1]).Name);

    // This space is auto-generated
    spaceNum = spaceZone2;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    // Defaults
    EXPECT_EQ(-99999, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ(Zone2FloorArea, state->dataHeatBal->space(spaceNum).FloorArea);

    // Now get to the point and check the infiltration and ventilation setup
    // Expected number of infiltration and ventilation instances:
    // AllZonesInfiltration - 3, 3 spaces total in the zones in the zonelist, flow/area =
    // SomeSpacesInfiltration - 2, 2 spaces in the spacelist
    // Space1aInfiltration - 1
    // Space1bInfiltration - 1
    // Zone1Infiltration - 2, 2 spaces in the zone
    // Zone2Infiltration - 1, 1 space in the zone
    constexpr int numInstances = 10;

    constexpr Real64 AllZonesFlowPerArea = 6.0;
    constexpr Real64 SomeSpacesFlowPerArea = 5.0;
    constexpr Real64 Space1aFlowPerArea = 3.0;
    constexpr Real64 Space1bFlowPerArea = 4.0;
    constexpr Real64 Zone1FlowPerArea = 1.0;
    constexpr Real64 Zone2FlowPerArea = 2.0;

    // Note that the epJSON input will sort the objects alphabetically, so AllZoneInfiltration comes first
    const std::array<int, numInstances> spaceNums = {space1a, space1b, spaceZone2, space1a, space1b, space1a, space1b, space1a, space1b, spaceZone2};

    const std::array<int, numInstances> zoneNums = {zone1, zone1, zone2, zone1, zone1, zone1, zone1, zone1, zone1, zone2};

    constexpr std::array<std::string_view, numInstances> infilNames = {"Space 1a AllZonesInfiltration",
                                                                       "Space 1b AllZonesInfiltration",
                                                                       "Zone 2 AllZonesInfiltration",
                                                                       "Space 1a SomeSpacesInfiltration",
                                                                       "Space 1b SomeSpacesInfiltration",
                                                                       "Space1aInfiltration",
                                                                       "Space1bInfiltration",
                                                                       "Space 1a Zone1Infiltration",
                                                                       "Space 1b Zone1Infiltration",
                                                                       "Zone2Infiltration"};

    constexpr std::array<std::string_view, numInstances> ventNames = {"Space 1a AllZonesVentilation",
                                                                      "Space 1b AllZonesVentilation",
                                                                      "Zone 2 AllZonesVentilation",
                                                                      "Space 1a SomeSpacesVentilation",
                                                                      "Space 1b SomeSpacesVentilation",
                                                                      "Space1aVentilation",
                                                                      "Space1bVentilation",
                                                                      "Space 1a Zone1Ventilation",
                                                                      "Space 1b Zone1Ventilation",
                                                                      "Zone2Ventilation"};

    // Same flow rates for both infiltration and ventilation
    constexpr std::array<Real64, numInstances> flows = {Space1aFloorArea * AllZonesFlowPerArea,
                                                        Space1bFloorArea * AllZonesFlowPerArea,
                                                        Zone2FloorArea * AllZonesFlowPerArea,
                                                        Space1aFloorArea * SomeSpacesFlowPerArea,
                                                        Space1bFloorArea * SomeSpacesFlowPerArea,
                                                        Space1aFloorArea * Space1aFlowPerArea,
                                                        Space1bFloorArea * Space1bFlowPerArea,
                                                        Space1aFloorArea * Zone1FlowPerArea,
                                                        Space1bFloorArea * Zone1FlowPerArea,
                                                        Zone2FloorArea * Zone2FlowPerArea};

    // Same density basis for both infiltration and ventilation
    constexpr std::array<DataHeatBalance::InfVentDensityBasis, numInstances> density = {DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Outdoor,
                                                                                        DataHeatBalance::InfVentDensityBasis::Standard,
                                                                                        DataHeatBalance::InfVentDensityBasis::Standard,
                                                                                        DataHeatBalance::InfVentDensityBasis::Indoor};

    for (int itemNum = 0; itemNum <= numInstances - 1; ++itemNum) {
        auto &thisInfiltration = state->dataHeatBal->Infiltration[itemNum];
        auto &thisVentilation = state->dataHeatBal->Ventilation[itemNum];
        EXPECT_TRUE(Util::SameString(infilNames[itemNum], thisInfiltration.Name));
        EXPECT_EQ(thisInfiltration.DesignLevel, flows[itemNum]);
        EXPECT_TRUE(Util::SameString(ventNames[itemNum], thisVentilation.Name));
        EXPECT_EQ(thisVentilation.DesignLevel, flows[itemNum]);
        EXPECT_EQ(thisInfiltration.ZonePtr, zoneNums[itemNum]);
        EXPECT_EQ(thisVentilation.ZonePtr, zoneNums[itemNum]);
        EXPECT_EQ(thisInfiltration.spaceIndex, spaceNums[itemNum]);
        EXPECT_EQ(thisVentilation.spaceIndex, spaceNums[itemNum]);
        EXPECT_EQ(thisInfiltration.densityBasis, density[itemNum]);
        EXPECT_EQ(thisVentilation.densityBasis, density[itemNum]);
    }
}
TEST_F(EnergyPlusFixture, HeatBalanceAirManager_GetMixingAndCrossMixing)
{

    // Test input processing of Infiltration objects with spaces

    state->dataInputProcessing->inputProcessor->epJSON = R"(
    {
    "SimulationControl": {
        "SimulationControl 1": {
            "do_plant_sizing_calculation": "No",
            "do_system_sizing_calculation": "No",
            "do_zone_sizing_calculation": "No",
            "run_simulation_for_sizing_periods": "Yes",
            "run_simulation_for_weather_file_run_periods": "No"
        }
    },
    "ZoneAirHeatBalanceAlgorithm": {
        "ZoneAirHeatBalanceAlgorithm 1": {
            "algorithm": "AnalyticalSolution",
            "do_space_heat_balance_for_simulation": "Yes"
        }
    },
    "Site:Location": {
        "USA IL-CHICAGO-OHARE": {
            "elevation": 190,
            "latitude": 41.77,
            "longitude": -87.75,
            "time_zone": -6.0
        }
    },
        "SizingPeriod:DesignDay": {
        "CHICAGO Ann Clg .4% Condns WB=>MDB": {
            "barometric_pressure": 99063.0,
            "daily_dry_bulb_temperature_range": 10.7,
            "day_of_month": 21,
            "day_type": "SummerDesignDay",
            "daylight_saving_time_indicator": "No",
            "humidity_condition_type": "WetBulb",
            "maximum_dry_bulb_temperature": 31.2,
            "month": 7,
            "rain_indicator": "No",
            "sky_clearness": 1.0,
            "snow_indicator": "No",
            "solar_model_indicator": "ASHRAEClearSky",
            "wetbulb_or_dewpoint_at_maximum_dry_bulb": 25.5,
            "wind_direction": 230,
            "wind_speed": 5.3
          }
        },
        "Zone": {
            "Zone 1" : {
                "volume": 100.0
            },
            "Zone 2" : {
                "volume": 2000.0,
                 "floor_area": 1000.0
            }
        },
        "Space": {
            "Space 1a" : {
                 "zone_name": "Zone 1",
                 "floor_area": 10.0
            },
            "Space 1b" : {
                 "zone_name": "Zone 1",
                 "floor_area": 100.0
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
        },
        "ZoneList": {
            "AllZones" : {
                 "zones": [
                    {
                        "zone_name": "Zone 1"
                    },
                    {
                        "zone_name": "Zone 2"
                    }
                ]
            }
        },
        "Building": {
            "Some building somewhere": {
            }
        },
        "GlobalGeometryRules": {
            "GlobalGeometryRules 1": {
                "coordinate_system": "Relative",
                "starting_vertex_position": "UpperLeftCorner",
                "vertex_entry_direction": "Counterclockwise"
            }
        },
        "Construction": {
            "ext-slab": {
                "outside_layer": "HW CONCRETE"
            }
        },
        "Material": {
            "HW CONCRETE": {
                "conductivity": 1.311,
                "density": 2240.0,
                "roughness": "Rough",
                "solar_absorptance": 0.7,
                "specific_heat": 836.8,
                "thermal_absorptance": 0.9,
                "thickness": 0.1016,
                "visible_absorptance": 0.7
            }
        },
        "BuildingSurface:Detailed": {
            "Dummy Space 1a Floor": {
                "zone_name": "Zone 1",
                "space_name": "Space 1a",
                "surface_type": "Floor",
                "construction_name": "ext-slab",
                "number_of_vertices": 4,
                "outside_boundary_condition": "adiabatic",
                "sun_exposure": "nosun",
                "vertices": [
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    }
                ]
            },
            "Dummy Space 1b Floor": {
                "zone_name": "Zone 1",
                "space_name": "Space 1b",
                "surface_type": "Floor",
                "construction_name": "ext-slab",
                "number_of_vertices": 4,
                "outside_boundary_condition": "adiabatic",
                "sun_exposure": "nosun",
                "vertices": [
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    }
                ]
            },
            "Dummy Zone 2 Floor": {
                "zone_name": "Zone 2",
                "surface_type": "Floor",
                "construction_name": "ext-slab",
                "number_of_vertices": 4,
                "outside_boundary_condition": "adiabatic",
                "sun_exposure": "nosun",
                "vertices": [
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 45.3375,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 4.5732,
                        "vertex_z_coordinate": 0.0
                    },
                    {
                        "vertex_x_coordinate": 4.5732,
                        "vertex_y_coordinate": 28.7006,
                        "vertex_z_coordinate": 0.0
                    }
                ]
            }
        },
        "ZoneMixing": {
            "Zone1Mixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 1.0,
                "zone_or_space_name": "Zone 1",
                "source_zone_or_space_name": "Zone 2"
            },
            "Zone2Mixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 2.0,
                "zone_or_space_name": "Zone 2",
                "source_zone_or_space_name": "Zone 1"
            },
            "Space1aMixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 3.0,
                "zone_or_space_name": "Space 1a",
                "source_zone_or_space_name": "Space 1b"
            },
            "Space1bMixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 4.0,
                "zone_or_space_name": "Space 1b",
                "source_zone_or_space_name": "Zone 2"
            }
        },
        "ZoneCrossMixing": {
            "Zone1CrossMixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 1.0,
                "zone_or_space_name": "Zone 1",
                "source_zone_or_space_name": "Zone 2"
            },
            "Zone2CrossMixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 2.0,
                "zone_or_space_name": "Zone 2",
                "source_zone_or_space_name": "Zone 1"
            },
            "Space1aCrossMixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 3.0,
                "zone_or_space_name": "Space 1a",
                "source_zone_or_space_name": "Space 1b"
            },
            "Space1bCrossMixing": {
                "design_flow_rate_calculation_method": "Flow/Area",
                "flow_rate_per_floor_area": 4.0,
                "zone_or_space_name": "Space 1b",
                "source_zone_or_space_name": "Zone 2"
            }
        }
    }
    )"_json;

    state->dataGlobal->isEpJSON = true;
    state->dataInputProcessing->inputProcessor->initializeMaps();

    // GetZoneData and others use GetObjectItem with ipshortcuts so these need to be allocated
    // copied from InputProcessor::processInput (which expects an input file to be present)
    int MaxArgs = 0;
    int MaxAlpha = 0;
    int MaxNumeric = 0;
    state->dataInputProcessing->inputProcessor->getMaxSchemaArgs(MaxArgs, MaxAlpha, MaxNumeric);

    state->dataIPShortCut->cAlphaFieldNames.allocate(MaxAlpha);
    state->dataIPShortCut->cAlphaArgs.allocate(MaxAlpha);
    state->dataIPShortCut->lAlphaFieldBlanks.dimension(MaxAlpha, false);
    state->dataIPShortCut->cNumericFieldNames.allocate(MaxNumeric);
    state->dataIPShortCut->rNumericArgs.dimension(MaxNumeric, 0.0);
    state->dataIPShortCut->lNumericFieldBlanks.dimension(MaxNumeric, false);

    state->init_state(*state);

    // HeatBalanceManager::GetHeatBalanceInput(*state);
    std::string const error_string = delimited_string(
        {format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
         "   ** Warning ** No Timestep object found.  Number of TimeSteps in Hour defaulted to 4.",
         "   ** Warning ** No reporting elements have been requested. No simulation results produced.",
         "   **   ~~~   ** ...Review requirements such as \"Output:Table:SummaryReports\", \"Output:Table:Monthly\", \"Output:Variable\", "
         "\"Output:Meter\" and others.",
         "   ** Warning ** GetSurfaceData: Entered Space Floor Area(s) differ more than 5% from calculated Space Floor Area(s).",
         "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual Spaces.",
         "   ** Warning ** CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; ",
         "   ** Warning ** CalcApproximateViewFactors: Zero area for all other zone surfaces.",
         "   **   ~~~   ** Happens for Surface=\"DUMMY SPACE 1A FLOOR\" in Zone=ZONE 1",
         "   ** Warning ** CalcApproximateViewFactors: Zero area for all other zone surfaces.",
         "   **   ~~~   ** Happens for Surface=\"DUMMY SPACE 1B FLOOR\" in Zone=ZONE 1",
         "   ** Warning ** Surfaces in Zone/Enclosure=\"ZONE 1\" do not define an enclosure.",
         "   **   ~~~   ** Number of surfaces <= 3, view factors are set to force reciprocity but may not fulfill completeness.",
         "   **   ~~~   ** Reciprocity means that radiant exchange between two surfaces will match and not lead to an energy loss.",
         "   **   ~~~   ** Completeness means that all of the view factors between a surface and the other surfaces in a zone add up to unity.",
         "   **   ~~~   ** So, when there are three or less surfaces in a zone, EnergyPlus will make sure there are no losses of energy but",
         "   **   ~~~   ** it will not exchange the full amount of radiation with the rest of the zone as it would if there was a completed "
         "enclosure.",
         "   ** Warning ** CalcApproximateViewFactors: Zero area for all other zone surfaces.", // Why are these errors being reported twice, because
                                                                                                // something is being calculated twice?
         "   **   ~~~   ** Happens for Surface=\"DUMMY SPACE 1A FLOOR\" in Zone=ZONE 1",
         "   ** Warning ** CalcApproximateViewFactors: Zero area for all other zone surfaces.",
         "   **   ~~~   ** Happens for Surface=\"DUMMY SPACE 1B FLOOR\" in Zone=ZONE 1",
         "   ** Warning ** Surfaces in Zone/Enclosure=\"ZONE 1\" do not define an enclosure.",
         "   **   ~~~   ** Number of surfaces <= 3, view factors are set to force reciprocity but may not fulfill completeness.",
         "   **   ~~~   ** Reciprocity means that radiant exchange between two surfaces will match and not lead to an energy loss.",
         "   **   ~~~   ** Completeness means that all of the view factors between a surface and the other surfaces in a zone add up to unity.",
         "   **   ~~~   ** So, when there are three or less surfaces in a zone, EnergyPlus will make sure there are no losses of energy but",
         "   **   ~~~   ** it will not exchange the full amount of radiation with the rest of the zone as it would if there was a completed "
         "enclosure.",
         "   ************* Testing Individual Branch Integrity",
         "   ************* All Branches passed integrity testing",
         "   ************* Testing Individual Supply Air Path Integrity",
         "   ************* All Supply Air Paths passed integrity testing",
         "   ************* Testing Individual Return Air Path Integrity",
         "   ************* All Return Air Paths passed integrity testing",
         "   ************* No node connection errors were found.",
         "   ************* Beginning Simulation"});

    // HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    // EXPECT_FALSE(ErrorsFound);

    SimulationManager::ManageSimulation(*state);

    compare_err_stream(error_string, true);

    // Expected floor areas
    Real64 constexpr Space1aFloorArea = 10.0;
    Real64 constexpr Space1bFloorArea = 100.0;
    Real64 constexpr Zone2FloorArea = 1000.0;

    // Zone  and space nums
    int const zone1 = Util::FindItemInList("ZONE 1", state->dataHeatBal->Zone);
    int const zone2 = Util::FindItemInList("ZONE 2", state->dataHeatBal->Zone);
    int const space1a = Util::FindItemInList("SPACE 1A", state->dataHeatBal->space);
    int const space1b = Util::FindItemInList("SPACE 1B", state->dataHeatBal->space);
    int const spaceZone2 = Util::FindItemInList("ZONE 2", state->dataHeatBal->space);

    int zoneNum = zone1;
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(2, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[1]).Name);
    zoneNum = zone2;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(zoneNum).Name);
    EXPECT_EQ(1, state->dataHeatBal->Zone(zoneNum).numSpaces);
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(state->dataHeatBal->Zone(zoneNum).spaceIndexes[0]).Name);

    int spaceNum = space1a;
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    EXPECT_EQ(Space1aFloorArea, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ(Space1aFloorArea, state->dataHeatBal->space(spaceNum).FloorArea);

    spaceNum = space1b;
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("ZONE 1", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    EXPECT_EQ(Space1bFloorArea, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ(Space1bFloorArea, state->dataHeatBal->space(spaceNum).FloorArea);

    EXPECT_EQ("SOMESPACES", state->dataHeatBal->spaceList(1).Name);
    EXPECT_EQ(2, state->dataHeatBal->spaceList(1).spaces.size());
    EXPECT_EQ("SPACE 1A", state->dataHeatBal->space(state->dataHeatBal->spaceList(1).spaces[0]).Name);
    EXPECT_EQ("SPACE 1B", state->dataHeatBal->space(state->dataHeatBal->spaceList(1).spaces[1]).Name);

    // This space is auto-generated
    spaceNum = spaceZone2;
    EXPECT_EQ("ZONE 2", state->dataHeatBal->space(spaceNum).Name);
    EXPECT_EQ("ZONE 2", state->dataHeatBal->Zone(state->dataHeatBal->space(spaceNum).zoneNum).Name);
    // Defaults
    EXPECT_EQ(-99999, state->dataHeatBal->space(spaceNum).userEnteredFloorArea);
    EXPECT_EQ(Zone2FloorArea, state->dataHeatBal->space(spaceNum).FloorArea);

    // Now get to the point and check the mixing setup
    // Expected number of mixing and crossmixing instances:
    // Space1aMixing - 1
    // Space1bMixing - 1
    // Zone1Mixing - 2, 2 spaces in the zone
    // Zone2Mixing - 1, 1 space in the zone
    constexpr int numInstances = 5;

    constexpr Real64 Space1aFlowPerArea = 3.0;
    constexpr Real64 Space1bFlowPerArea = 4.0;
    constexpr Real64 Zone1FlowPerArea = 1.0;
    constexpr Real64 Zone2FlowPerArea = 2.0;

    // Note that the epJSON input will sort the objects alphabetically, so Space1aMixing comes first
    const std::array<int, numInstances> spaceNums = {space1a, space1b, space1a, space1b, spaceZone2};

    const std::array<int, numInstances> zoneNums = {zone1, zone1, zone1, zone1, zone2};

    const std::array<int, numInstances> fromSpaceNums = {space1b, spaceZone2, spaceZone2, spaceZone2, 0};

    const std::array<int, numInstances> fromZoneNums = {zone1, zone2, zone2, zone2, zone1};

    constexpr std::array<std::string_view, numInstances> mixNames = {
        "Space1aMixing", "Space1bMixing", "Space 1a Zone1Mixing", "Space 1b Zone1Mixing", "Zone2Mixing"};

    constexpr std::array<std::string_view, numInstances> crossMixNames = {
        "Space1aCrossMixing", "Space1bCrossMixing", "Space 1a Zone1CrossMixing", "Space 1b Zone1CrossMixing", "Zone2CrossMixing"};

    // Same flow rates for both infiltration and ventilation
    constexpr std::array<Real64, numInstances> flows = {Space1aFloorArea * Space1aFlowPerArea,
                                                        Space1bFloorArea * Space1bFlowPerArea,
                                                        Space1aFloorArea * Zone1FlowPerArea,
                                                        Space1bFloorArea * Zone1FlowPerArea,
                                                        Zone2FloorArea * Zone2FlowPerArea};

    for (int itemNum = 0; itemNum <= numInstances - 1; ++itemNum) {
        auto &thisMixing = state->dataHeatBal->Mixing[itemNum];
        auto &thisCrossMixing = state->dataHeatBal->CrossMixing[itemNum];
        EXPECT_TRUE(Util::SameString(mixNames[itemNum], thisMixing.Name));
        EXPECT_EQ(thisMixing.DesignLevel, flows[itemNum]);
        EXPECT_TRUE(Util::SameString(crossMixNames[itemNum], thisCrossMixing.Name));
        EXPECT_EQ(thisCrossMixing.DesignLevel, flows[itemNum]);
        EXPECT_EQ(thisMixing.ZonePtr, zoneNums[itemNum]);
        EXPECT_EQ(thisCrossMixing.ZonePtr, zoneNums[itemNum]);
        EXPECT_EQ(thisMixing.spaceIndex, spaceNums[itemNum]);
        EXPECT_EQ(thisCrossMixing.spaceIndex, spaceNums[itemNum]);
        EXPECT_EQ(thisMixing.fromSpaceIndex, fromSpaceNums[itemNum]);
        EXPECT_EQ(thisCrossMixing.fromSpaceIndex, fromSpaceNums[itemNum]);
        EXPECT_EQ(thisMixing.FromZone, fromZoneNums[itemNum]);
        EXPECT_EQ(thisCrossMixing.FromZone, fromZoneNums[itemNum]);
    }
}

TEST_F(EnergyPlusFixture, HeatBalanceAirManager_InitSimpleMixingConvectiveHeatGains_Test)
{
    state->init_state(*state);
    Real64 expectedResult1;
    Real64 expectedResult2;
    Real64 constexpr allowedTolerance = 0.00001;

    // Base line data that do not change between tests
    state->dataHeatBal->TotRefDoorMixing = 0;
    state->dataHeatBal->TotCrossMixing = 0;
    state->dataHeatBal->TotMixing = 3;
    state->dataHeatBal->Mixing.allocate(state->dataHeatBal->TotMixing);
    state->dataHeatBal->Mixing(1).sched = Sched::GetScheduleAlwaysOn(*state);
    state->dataHeatBal->Mixing(2).sched = Sched::GetScheduleAlwaysOn(*state);
    state->dataHeatBal->Mixing(3).sched = Sched::GetScheduleAlwaysOn(*state);
    state->dataHeatBal->Mixing(1).EMSSimpleMixingOn = false;
    state->dataHeatBal->Mixing(2).EMSSimpleMixingOn = false;
    state->dataHeatBal->Mixing(3).EMSSimpleMixingOn = false;
    state->dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = true;
    state->dataHeatBal->MassConservation.allocate(2);
    state->dataHeatBal->MassConservation(1).NumReceivingZonesMixingObject = 2;
    state->dataHeatBal->MassConservation(2).NumReceivingZonesMixingObject = 0;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr.allocate(2);

    // Test 1: not air flow flag--don't do anything
    state->dataHeatBal->AirFlowFlag = false;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(1) = -9999.9;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(2) = -9999.9;
    expectedResult1 = -9999.9;
    expectedResult2 = -9999.9;
    HeatBalanceAirManager::InitSimpleMixingConvectiveHeatGains(*state);
    EXPECT_NEAR(expectedResult1, state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(1), allowedTolerance);
    EXPECT_NEAR(expectedResult2, state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(2), allowedTolerance);

    // Test 2: yes to air flow flag, but no mixing flow sum, set ZoneMixingReceivingFr to zero
    state->dataHeatBal->AirFlowFlag = true;
    state->dataHeatBal->Mixing(1).DesignLevel = 0.0;
    state->dataHeatBal->Mixing(2).DesignLevel = 0.0;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(1) = -9999.9;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(2) = -9999.9;
    expectedResult1 = 0.0;
    expectedResult2 = 0.0;
    HeatBalanceAirManager::InitSimpleMixingConvectiveHeatGains(*state);
    EXPECT_NEAR(expectedResult1, state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(1), allowedTolerance);
    EXPECT_NEAR(expectedResult2, state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(2), allowedTolerance);

    // Test 3: yes to air flow flag, with mixing, ZoneMixingReceivingFr set to appropriate value
    state->dataHeatBal->AirFlowFlag = true;
    state->dataHeatBal->Mixing(1).DesignLevel = 100.0;
    state->dataHeatBal->Mixing(2).DesignLevel = 300.0;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(1) = -9999.9;
    state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(2) = -9999.9;
    expectedResult1 = 0.25;
    expectedResult2 = 0.75;
    HeatBalanceAirManager::InitSimpleMixingConvectiveHeatGains(*state);
    EXPECT_NEAR(expectedResult1, state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(1), allowedTolerance);
    EXPECT_NEAR(expectedResult2, state->dataHeatBal->MassConservation(1).ZoneMixingReceivingFr(2), allowedTolerance);
}

} // namespace EnergyPlus
