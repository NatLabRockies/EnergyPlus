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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

#include <nlohmann/json.hpp>

using namespace EnergyPlus;
using namespace EnergyPlus::DataZoneEquipment;
using namespace nlohmann::literals;

TEST_F(EnergyPlusFixture, DataZoneEquipment_TestGetSystemNodeNumberForZone)
{
    state->init_state(*state);
    state->dataGlobal->NumOfZones = 2;
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);

    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Zone1";
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;

    state->dataZoneEquip->ZoneEquipConfig(2).ZoneName = "Zone2";
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneNode = 2;
    state->dataZoneEquip->ZoneEquipConfig(2).IsControlled = true;

    state->dataZoneEquip->ZoneEquipInputsFilled = true;

    EXPECT_EQ(0, GetSystemNodeNumberForZone(*state, 0));
    EXPECT_EQ(1, GetSystemNodeNumberForZone(*state, 1));

    state->dataZoneEquip->ZoneEquipConfig.deallocate();
}

TEST_F(EnergyPlusFixture, DataZoneEquipment_TestCalcDesignSpecificationOutdoorAir)
{
    // #6225
    state->init_state(*state);
    state->dataHeatBal->Zone.allocate(1);
    state->dataSize->OARequirements.allocate(1);
    state->dataHeatBal->ZoneIntGain.allocate(1);
    state->dataHeatBal->People.allocate(1);
    state->dataContaminantBalance->ZoneCO2GainFromPeople.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2.allocate(1);
    state->dataContaminantBalance->ZoneSysContDemand.allocate(1);

    state->dataEnvrn->StdRhoAir = 1.20;

    state->dataHeatBal->Zone(1).FloorArea = 10.0;
    state->dataHeatBal->Zone(1).TotOccupants = 5.0;
    state->dataHeatBal->Zone(1).zoneContamControllerSched = Sched::AddScheduleConstant(*state, "ZONE CONTAM CONTROLLER");
    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->TotPeople = 1;
    state->dataHeatBal->People(1).activityLevelSched = Sched::AddScheduleConstant(*state, "ACTIVITY LEVEL SCHED");
    state->dataHeatBal->People(1).CO2RateFactor = 3.82e-8;
    state->dataHeatBal->People(1).NumberOfPeople = state->dataHeatBal->Zone(1).TotOccupants;

    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataContaminantBalance->OutdoorCO2 = 400.0;
    state->dataContaminantBalance->ZoneCO2GainFromPeople(1) = 3.82E-8 * 5.0;

    state->dataSize->NumOARequirements = 1;
    state->dataSize->OARequirements(1).Name = "ZONE OA";
    state->dataSize->OARequirements(1).OAFlowMethod = DataSizing::OAFlowCalcMethod::PCOccSch;
    state->dataSize->OARequirements(1).OAFlowPerPerson = 0.002;
    state->dataSize->OARequirements(1).OAFlowPerArea = 0.003;
    state->dataHeatBal->ZoneIntGain(1).NOFOCC = 0.5;
    state->dataHeatBal->Zone(1).zoneContamControllerSched->currentVal = 1.0;
    state->dataHeatBal->People(1).activityLevelSched->currentVal = 131.881995;

    Real64 OAVolumeFlowRate;
    // Test ZOAM_ProportionalControlSchOcc
    state->dataContaminantBalance->ZoneAirCO2(1) = 500.0;
    OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(*state, 1, 1, false, false);
    EXPECT_NEAR(0.031, OAVolumeFlowRate, 0.00001);

    state->dataContaminantBalance->ZoneAirCO2(1) = 405.0;
    OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(*state, 1, 1, false, false);
    EXPECT_NEAR(0.0308115, OAVolumeFlowRate, 0.00001);

    // Test ZOAM_ProportionalControlDesOcc
    state->dataContaminantBalance->ZoneAirCO2(1) = 500.0;
    state->dataSize->OARequirements(1).OAFlowMethod = DataSizing::OAFlowCalcMethod::PCDesOcc;
    OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(*state, 1, 1, false, false);
    EXPECT_NEAR(0.0315879, OAVolumeFlowRate, 0.00001);

    // Test ZOAM_IAQP
    state->dataSize->OARequirements(1).OAFlowMethod = DataSizing::OAFlowCalcMethod::IAQProcedure;
    state->dataContaminantBalance->ZoneSysContDemand(1).OutputRequiredToCO2SP = 0.2 * state->dataEnvrn->StdRhoAir;
    OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(*state, 1, 1, false, false);
    EXPECT_NEAR(0.2, OAVolumeFlowRate, 0.00001);

    // Cleanup
    state->dataHeatBal->Zone.deallocate();
    state->dataSize->OARequirements.deallocate();
    state->dataHeatBal->ZoneIntGain.deallocate();
    state->dataHeatBal->People.deallocate();
    state->dataContaminantBalance->ZoneCO2GainFromPeople.deallocate();
    state->dataContaminantBalance->ZoneAirCO2.deallocate();
    state->dataContaminantBalance->ZoneSysContDemand.deallocate();
}

TEST_F(EnergyPlusFixture, GetZoneEquipmentData_epJSON)
{

    state->dataInputProcessing->inputProcessor->epJSON = R"(
{
  "ZoneHVAC:EquipmentList": {
    "Zone1 Equipment List": {
      "equipment": [
        {
          "zone_equipment_cooling_sequence": 1,
          "zone_equipment_heating_or_no_load_sequence": 1,
          "zone_equipment_name": "Fan Zone Exhaust 1",
          "zone_equipment_object_type": "Fan:ZoneExhaust"
        },
        {
          "zone_equipment_cooling_sequence": 3,
          "zone_equipment_heating_or_no_load_sequence": 2,
          "zone_equipment_name": "ADU Air Terminal Single Duct Constant Volume No Reheat 1",
          "zone_equipment_object_type": "ZoneHVAC:AirDistributionUnit"
        },
        {
          "zone_equipment_cooling_sequence": 2,
          "zone_equipment_heating_or_no_load_sequence": 3,
          "zone_equipment_name": "ADU Air Terminal Single Duct VAV Reheat 1",
          "zone_equipment_object_type": "ZoneHVAC:AirDistributionUnit"
        }
      ],
      "load_distribution_scheme": "SequentialLoad"
    }
  },
  "Zone": {
    "Zone1": {
      "direction_of_relative_north": 0.0,
      "multiplier": 1,
      "part_of_total_floor_area": "Yes",
      "x_origin": 0.0,
      "y_origin": 0.0,
      "z_origin": 0.0
    }
  },
  "NodeList": {
    "Packaged Rooftop Air Conditioner Demand Inlet Nodes": {
      "nodes": [
        {
          "node_name": "Node 6"
        }
      ]
    },
    "Packaged Rooftop Air Conditioner Supply Outlet Nodes": {
      "nodes": [
        {
          "node_name": "Node 5"
        }
      ]
    },
    "VAV with Reheat Demand Inlet Nodes": {
      "nodes": [
        {
          "node_name": "Node 18"
        }
      ]
    },
    "VAV with Reheat Supply Outlet Nodes": {
      "nodes": [
        {
          "node_name": "Node 17"
        }
      ]
    },
    "Zone1 Exhaust Node List": {
      "nodes": [
        {
          "node_name": "Node 2"
        }
      ]
    },
    "Zone1 Inlet Node List": {
      "nodes": [
        {
          "node_name": "Node 8"
        },
        {
          "node_name": "Node 20"
        }
      ]
    },
    "Zone1 Return Node List": {
      "nodes": [
        {
          "node_name": "Node 15"
        },
        {
          "node_name": "Node 77"
        }
      ]
    }
  },

  "ZoneHVAC:EquipmentConnections": {
    "ZoneHVAC:EquipmentConnections 1": {
      "zone_air_exhaust_node_or_nodelist_name": "Zone1 Exhaust Node List",
      "zone_air_inlet_node_or_nodelist_name": "Zone1 Inlet Node List",
      "zone_air_node_name": "Node 1",
      "zone_conditioning_equipment_list_name": "Zone1 Equipment List",
      "zone_name": "Zone1",
      "zone_return_air_node_or_nodelist_name": "Zone1 Return Node List"
    }
  },
  "AirLoopHVAC:SupplyPath": {
    "Packaged Rooftop Air Conditioner Node 6 Supply Path": {
      "components": [
        {
          "component_name": "Air Loop HVAC Zone Splitter 1",
          "component_object_type": "AirLoopHVAC:ZoneSplitter"
        }
      ],
      "supply_air_path_inlet_node_name": "Node 6"
    },
    "VAV with Reheat Node 18 Supply Path": {
      "components": [
        {
          "component_name": "Air Loop HVAC Zone Splitter 2",
          "component_object_type": "AirLoopHVAC:ZoneSplitter"
        }
      ],
      "supply_air_path_inlet_node_name": "Node 18"
    }
  },
  "AirLoopHVAC:ReturnPath": {
    "Packaged Rooftop Air Conditioner Return Path": {
      "components": [
        {
          "component_name": "Air Loop HVAC Zone Mixer 1",
          "component_object_type": "AirLoopHVAC:ZoneMixer"
        }
      ],
      "return_air_path_outlet_node_name": "Node 7"
    },
    "VAV with Reheat Return Path": {
      "components": [
        {
          "component_name": "Air Loop HVAC Zone Mixer 2",
          "component_object_type": "AirLoopHVAC:ZoneMixer"
        }
      ],
      "return_air_path_outlet_node_name": "Node 19"
    }
  },
  "Fan:ZoneExhaust": {
    "Fan Zone Exhaust 1": {
      "air_inlet_node_name": "Node 2",
      "air_outlet_node_name": "Node 3",
      "end_use_subcategory": "General",
      "fan_total_efficiency": 0.6,
      "pressure_rise": 0.0,
      "system_availability_manager_coupling_mode": "Decoupled"
    }
  },
  "ZoneHVAC:AirDistributionUnit": {
    "ADU Air Terminal Single Duct Constant Volume No Reheat 1": {
      "air_distribution_unit_outlet_node_name": "Node 8",
      "air_terminal_name": "Air Terminal Single Duct Constant Volume No Reheat 1",
      "air_terminal_object_type": "AirTerminal:SingleDuct:ConstantVolume:NoReheat"
    },
    "ADU Air Terminal Single Duct VAV Reheat 1": {
      "air_distribution_unit_outlet_node_name": "Node 20",
      "air_terminal_name": "Air Terminal Single Duct VAV Reheat 1",
      "air_terminal_object_type": "AirTerminal:SingleDuct:VAV:Reheat"
    }
  },
  "AirLoopHVAC:ZoneMixer": {
    "Air Loop HVAC Zone Mixer 1": {
      "nodes": [
        {
          "inlet_node_name": "Node 15"
        }
      ],
      "outlet_node_name": "Node 7"
    },
    "Air Loop HVAC Zone Mixer 2": {
      "nodes": [
        {
          "inlet_node_name": "Node 77"
        }
      ],
      "outlet_node_name": "Node 19"
    }
  },
  "AirLoopHVAC:ZoneSplitter": {
    "Air Loop HVAC Zone Splitter 1": {
      "inlet_node_name": "Node 6",
      "nodes": [
        {
          "outlet_node_name": "Node 14"
        }
      ]
    },
    "Air Loop HVAC Zone Splitter 2": {
      "inlet_node_name": "Node 18",
      "nodes": [
        {
          "outlet_node_name": "Node 54"
        }
      ]
    }
  }
}
    )"_json;

    state->dataGlobal->isEpJSON = true;
    state->dataInputProcessing->inputProcessor->initializeMaps();

    // GetZoneData and others use GetObjectItem with ipshortcuts so these need to be allocated
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

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    DataZoneEquipment::GetZoneEquipmentData(*state);

    ASSERT_EQ(1, state->dataZoneEquip->ZoneEquipList.size());
    auto &thisZoneEquipList = state->dataZoneEquip->ZoneEquipList(1);
    EXPECT_EQ(3, thisZoneEquipList.NumOfEquipTypes);

    EXPECT_ENUM_EQ(DataZoneEquipment::LoadDist::Sequential, thisZoneEquipList.LoadDistScheme);

    EXPECT_EQ("FAN ZONE EXHAUST 1", thisZoneEquipList.EquipName(1));
    EXPECT_EQ("FAN:ZONEEXHAUST", thisZoneEquipList.EquipTypeName(1));
    EXPECT_ENUM_EQ(DataZoneEquipment::ZoneEquipType::ExhaustFan, thisZoneEquipList.EquipType(1));
    EXPECT_EQ(1, thisZoneEquipList.CoolingPriority(1));
    EXPECT_EQ(1, thisZoneEquipList.HeatingPriority(1));
    EXPECT_EQ(Sched::GetScheduleAlwaysOn(*state), thisZoneEquipList.sequentialCoolingFractionScheds(1));
    EXPECT_EQ(Sched::GetScheduleAlwaysOn(*state), thisZoneEquipList.sequentialHeatingFractionScheds(1));

    EXPECT_EQ("ADU AIR TERMINAL SINGLE DUCT CONSTANT VOLUME NO REHEAT 1", thisZoneEquipList.EquipName(2));
    EXPECT_EQ("ZONEHVAC:AIRDISTRIBUTIONUNIT", thisZoneEquipList.EquipTypeName(2));
    EXPECT_ENUM_EQ(DataZoneEquipment::ZoneEquipType::AirDistributionUnit, thisZoneEquipList.EquipType(2));
    EXPECT_EQ(3, thisZoneEquipList.CoolingPriority(2));
    EXPECT_EQ(2, thisZoneEquipList.HeatingPriority(2));
    EXPECT_EQ(Sched::GetScheduleAlwaysOn(*state), thisZoneEquipList.sequentialCoolingFractionScheds(2));
    EXPECT_EQ(Sched::GetScheduleAlwaysOn(*state), thisZoneEquipList.sequentialHeatingFractionScheds(2));

    EXPECT_EQ("ADU AIR TERMINAL SINGLE DUCT VAV REHEAT 1", thisZoneEquipList.EquipName(3));
    EXPECT_EQ("ZONEHVAC:AIRDISTRIBUTIONUNIT", thisZoneEquipList.EquipTypeName(3));
    EXPECT_ENUM_EQ(DataZoneEquipment::ZoneEquipType::AirDistributionUnit, thisZoneEquipList.EquipType(3));
    EXPECT_EQ(2, thisZoneEquipList.CoolingPriority(3));
    EXPECT_EQ(3, thisZoneEquipList.HeatingPriority(3));
    EXPECT_EQ(Sched::GetScheduleAlwaysOn(*state), thisZoneEquipList.sequentialCoolingFractionScheds(3));
    EXPECT_EQ(Sched::GetScheduleAlwaysOn(*state), thisZoneEquipList.sequentialHeatingFractionScheds(3));
}
