import copy
import unittest
from unittest.mock import MagicMock, PropertyMock

from src.hvac_template import HVACTemplate
from src.hvac_template import InvalidTemplateException
from src.expand_objects import ExpandObjects, ExpandSystem, ExpandZone, ExpandPlantLoop, ExpandPlantEquipment
from . import BaseTest

minimum_objects_d = {
    "Building": {
        "Test Building": {}
    },
    "GlobalGeometryRules": {
        "GlobalGeometryRules 1": {
            "coordinate_system": "Relative",
            "starting_vertex_position": "UpperLeftCorner",
            "vertex_entry_direction": "Counterclockwise"
        }
    }
}

mock_zone_epjson = {
    "AirTerminal:SingleDuct:VAV:Reheat": {
        "SPACE1-1 VAV Reheat": {
            "air_inlet_node_name": "SPACE1-1 Zone Equip Inlet",
            "air_outlet_node_name": "SPACE1-1 Supply Inlet"
        }
    },
    "ZoneHVAC:EquipmentConnections": {
        "ZoneHVAC:EquipmentConnections 1": {
            "zone_air_inlet_node_or_nodelist_name": "{} Supply Inlet",
            "zone_air_node_name": "{} Zone Air Node",
            "zone_conditioning_equipment_list_name": "{} Equipment",
            "zone_name": "{}",
            "zone_return_air_node_or_nodelist_name": "{} Return Outlet"
        }
    }
}


class TestHVACTemplateObjectConnections(BaseTest, unittest.TestCase):
    """
    Methods and functions that connect HVACTemplate outputs
    """
    def setUp(self):
        self.hvac_template = HVACTemplate()
        self.hvac_template.logger.setLevel('INFO')
        self.hvac_template._load_schema()
        return

    def tearDown(self):
        return

    def test_create_zonecontrol_thermostat_dualsetpoint(self):
        et = MagicMock()
        et_epjson = PropertyMock(return_value={
            "ThermostatSetpoint:DualSetpoint": {
                "All Zones": {
                    "cooling_setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                    "heating_setpoint_temperature_schedule_name": "Htg-SetP-Sch"

                }
            }
        })
        type(et).epjson = et_epjson
        self.hvac_template.expanded_thermostats = {'All Zones': et}
        ez = ExpandZone(template={
            "HVACTemplate:Zone:VAV": {
                "HVACTemplate:Zone:VAV 1": {
                    "zone_name": "TEST ZONE",
                    'template_thermostat_name': 'All Zones'
                }
            }
        })
        self.hvac_template._create_zonecontrol_thermostat(zone_class_object=ez)
        self.assertEqual(
            'ThermostatSetpoint:DualSetpoint',
            self.hvac_template.epjson['ZoneControl:Thermostat']['TEST ZONE Thermostat']['control_1_object_type'])
        self.assertEqual(
            'HVACTemplate-Always4',
            list(self.hvac_template.epjson['Schedule:Compact'].keys())[0])
        return

    def test_create_zonecontrol_thermostat_single_heating(self):
        et = MagicMock()
        et_epjson = PropertyMock(return_value={
            "ThermostatSetpoint:SingleHeating": {
                "All Zones": {
                    "setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                }
            }
        })
        type(et).epjson = et_epjson
        self.hvac_template.expanded_thermostats = {'All Zones': et}
        ez = ExpandZone(template={
            "HVACTemplate:Zone:VAV": {
                "HVACTemplate:Zone:VAV 1": {
                    "zone_name": "TEST ZONE",
                    'template_thermostat_name': 'All Zones'
                }
            }
        })
        self.hvac_template._create_zonecontrol_thermostat(zone_class_object=ez)
        self.assertEqual(
            'ThermostatSetpoint:SingleHeating',
            self.hvac_template.epjson['ZoneControl:Thermostat']['TEST ZONE Thermostat']['control_1_object_type'])
        self.assertEqual(
            'HVACTemplate-Always1',
            list(self.hvac_template.epjson['Schedule:Compact'].keys())[0])
        return

    def test_create_zonecontrol_thermostat_single_cooling(self):
        et = MagicMock()
        et_epjson = PropertyMock(return_value={
            "ThermostatSetpoint:SingleCooling": {
                "All Zones": {
                    "setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                }
            }
        })
        type(et).epjson = et_epjson
        self.hvac_template.expanded_thermostats = {'All Zones': et}
        ez = ExpandZone(template={
            "HVACTemplate:Zone:VAV": {
                "HVACTemplate:Zone:VAV 1": {
                    "zone_name": "TEST ZONE",
                    'template_thermostat_name': 'All Zones'
                }
            }
        })
        self.hvac_template._create_zonecontrol_thermostat(zone_class_object=ez)
        self.assertEqual(
            'ThermostatSetpoint:SingleCooling',
            self.hvac_template.epjson['ZoneControl:Thermostat']['TEST ZONE Thermostat']['control_1_object_type'])
        self.assertEqual(
            'HVACTemplate-Always2',
            list(self.hvac_template.epjson['Schedule:Compact'].keys())[0])
        return

    def test_exception_zonecontrol_thermostat_bad_thermostat(self):
        et = MagicMock()
        et_epjson = PropertyMock(return_value={
            "ThermostatSetpoint:BadThermostat": {
                "All Zones SP Control": {
                    "setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                }
            }
        })
        type(et).epjson = et_epjson
        self.hvac_template.expanded_thermostats = {'All Zones': et}
        ez = ExpandZone(template={
            "HVACTemplate:Zone:VAV": {
                "HVACTemplate:Zone:VAV 1": {
                    "zone_name": "TEST ZONE",
                    'template_thermostat_name': 'All Zones'
                }
            }
        })
        with self.assertRaises(InvalidTemplateException):
            self.hvac_template._create_zonecontrol_thermostat(zone_class_object=ez)
        return

    def test_exception_create_zonecontrol_thermostat_no_thermostat(self):
        ez = ExpandZone(template={
            "HVACTemplate:Zone:VAV": {
                "HVACTemplate:Zone:VAV 1": {
                    "zone_name": "TEST ZONE",
                    'template_thermostat_name': 'All Zones'
                }
            }
        })
        with self.assertRaisesRegex(InvalidTemplateException, 'Thermostat object does not exist'):
            self.hvac_template._create_zonecontrol_thermostat(zone_class_object=ez)
        return

    def test_zone_field_name_from_system_template_type(self):
        output = self.hvac_template._get_zone_template_field_from_system_type(
            template_type='HVACTemplate:System:ConstantVolume')
        self.assertEqual('template_constant_volume_system_name', output)
        output = self.hvac_template._get_zone_template_field_from_system_type(
            template_type='HVACTemplate:System:DedicatedOutdoorAir')
        self.assertEqual('dedicated_outdoor_air_system_name', output)
        output = self.hvac_template._get_zone_template_field_from_system_type(
            template_type='HVACTemplate:System:DualDuct')
        self.assertEqual('template_dual_duct_system_name', output)
        output = self.hvac_template._get_zone_template_field_from_system_type(
            template_type='HVACTemplate:System:Unitary')
        self.assertEqual('template_unitary_system_name', output)
        output = self.hvac_template._get_zone_template_field_from_system_type(
            template_type='HVACTemplate:System:VAV')
        self.assertEqual('template_vav_system_name', output)
        output = self.hvac_template._get_zone_template_field_from_system_type(
            template_type='HVACTemplate:System:VRF')
        self.assertEqual('template_vrf_system_name', output)
        return

    def test_reject_zone_field_name_from_system_template_type_bad_input(self):
        with self.assertRaisesRegex(InvalidTemplateException, 'Invalid system'):
            self.hvac_template._get_zone_template_field_from_system_type(
                template_type='HVACTemplate:System:BadSystem')
        return

    def test_system_path_objects_no_plenum(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            del ez.supply_plenum_name
            del ez.return_plenum_name
            expanded_zones[unique_name] = ez
        self.hvac_template._create_system_path_connection_objects(
            system_class_object=es,
            expanded_zones=expanded_zones)
        self.assertEqual(
            {'AirLoopHVAC:ZoneSplitter': 1, 'AirLoopHVAC:ZoneMixer': 1,
             'AirLoopHVAC:SupplyPath': 1, 'AirLoopHVAC:ReturnPath': 1},
            eo.summarize_epjson(self.hvac_template.epjson)
        )
        return

    def test_system_path_objects_zone_no_supply_plenum(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            type(ez).supply_plenum_name = 'PLENUM-1'
            del ez.return_plenum_name
            expanded_zones[unique_name] = ez
        with self.assertRaisesRegex(InvalidTemplateException, 'supply_plenum_name indicated for zone template'):
            self.hvac_template._create_system_path_connection_objects(
                system_class_object=es,
                expanded_zones=expanded_zones)
        return

    def test_system_path_objects_zone_no_return_plenum(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            type(ez).return_plenum_name = 'PLENUM-1'
            del ez.supply_plenum_name
            expanded_zones[unique_name] = ez
        with self.assertRaisesRegex(InvalidTemplateException, 'return_plenum_name indicated for zone template'):
            self.hvac_template._create_system_path_connection_objects(
                system_class_object=es,
                expanded_zones=expanded_zones)
        return

    def test_system_path_objects_supply_plenum(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                    'supply_plenum_name': 'test plenum'
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            del ez.supply_plenum_name
            del ez.return_plenum_name
            expanded_zones[unique_name] = ez
        self.hvac_template._create_system_path_connection_objects(
            system_class_object=es,
            expanded_zones=expanded_zones)
        self.assertEqual(
            {'AirLoopHVAC:SupplyPlenum': 1, 'AirLoopHVAC:ZoneMixer': 1,
             'AirLoopHVAC:SupplyPath': 1, 'AirLoopHVAC:ReturnPath': 1},
            eo.summarize_epjson(self.hvac_template.epjson)
        )
        return

    def test_system_path_objects_return_plenum(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                    'return_plenum_name': 'test plenum'
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            del ez.supply_plenum_name
            del ez.return_plenum_name
            expanded_zones[unique_name] = ez
        self.hvac_template._create_system_path_connection_objects(
            system_class_object=es,
            expanded_zones=expanded_zones)
        self.assertEqual(
            {'AirLoopHVAC:ZoneSplitter': 1, 'AirLoopHVAC:ReturnPlenum': 1,
             'AirLoopHVAC:SupplyPath': 1, 'AirLoopHVAC:ReturnPath': 1},
            eo.summarize_epjson(self.hvac_template.epjson)
        )
        return

    def test_system_path_objects_both_plenum(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                    'supply_plenum_name': 'test supply plenum',
                    'return_plenum_name': 'test return plenum'
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            del ez.supply_plenum_name
            del ez.return_plenum_name
            expanded_zones[unique_name] = ez
        self.hvac_template._create_system_path_connection_objects(
            system_class_object=es,
            expanded_zones=expanded_zones)
        self.assertEqual(
            {'AirLoopHVAC:SupplyPlenum': 1, 'AirLoopHVAC:ReturnPlenum': 1,
             'AirLoopHVAC:SupplyPath': 1, 'AirLoopHVAC:ReturnPath': 1},
            eo.summarize_epjson(self.hvac_template.epjson)
        )
        return

    def test_reject_system_path_objects_no_zone_equipment(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                    'supply_plenum_name': 'test supply plenum',
                    'return_plenum_name': 'test return plenum'
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            mock_epjson.pop('AirTerminal:SingleDuct:VAV:Reheat')
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            type(ez).template_vav_system_name = 'VAV Sys 1'
            type(ez).zone_name = unique_name
            type(ez).unique_name = unique_name
            del ez.supply_plenum_name
            expanded_zones[unique_name] = ez
        with self.assertRaisesRegex(InvalidTemplateException, 'Search for zone equipment'):
            self.hvac_template._create_system_path_connection_objects(
                system_class_object=es,
                expanded_zones=expanded_zones)
        return

    def test_reject_system_path_objects_no_zone_connection_equipment(self):
        eo = ExpandObjects()
        es = ExpandSystem(template={
            "HVACTemplate:System:VAV": {
                "VAV Sys 1": {
                    'supply_plenum_name': 'test supply plenum',
                    'return_plenum_name': 'test return plenum'
                }
            }
        })
        expanded_zones = {}
        for unique_name in ['SPACE1-1', 'SPACE2-1']:
            eo.unique_name = unique_name
            mock_epjson = eo.resolve_objects(epjson=copy.deepcopy(mock_zone_epjson))
            mock_epjson.pop('ZoneHVAC:EquipmentConnections')
            ez = MagicMock()
            ez_epjson = PropertyMock(return_value=mock_epjson)
            type(ez).epjson = ez_epjson
            ez.template_vav_system_name = 'VAV Sys 1'
            ez.zone_name = unique_name
            ez.unique_name = unique_name
            del ez.supply_plenum_name
            del ez.return_plenum_name
            expanded_zones[unique_name] = ez
        with self.assertRaisesRegex(InvalidTemplateException, 'Search for ZoneHVAC:EquipmentConnections'):
            self.hvac_template._create_system_path_connection_objects(
                system_class_object=es,
                expanded_zones=expanded_zones)
        return

    def test_plant_equipment_creates_loop_and_equipment_template(self):
        ep = MagicMock()
        ep.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        expanded_plant_loops = {'Test Chilled Water': ep}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.condenser_type = 'WaterCooled'
        output_plant, output_equipment = self.hvac_template._create_templates_from_plant_equipment(
            plant_equipment_class_object=epe,
            expanded_plant_loops=expanded_plant_loops)
        self.assertEqual('HVACTemplate:Plant:CondenserWaterLoop', list(output_plant.keys())[0])
        return

    def test_plant_equipment_doesnt_create_loop_template_if_existing(self):
        ep = MagicMock()
        ep.template_type = 'HVACTemplate:Plant:CondenserWaterLoop'
        expanded_plant_loops = {'Test Hot Water': ep}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.condenser_type = 'WaterCooled'
        output_plant, output_equipment = self.hvac_template._create_templates_from_plant_equipment(
            plant_equipment_class_object=epe,
            expanded_plant_loops=expanded_plant_loops)
        self.assertEqual(0, len(output_plant.keys()))
        self.assertEqual(0, len(output_equipment.keys()))
        return

    def test_condenser_water_plant_loop_object_created_from_chiller_wo_attributes_transitioned(self):
        eph = MagicMock()
        eph.template_type = 'HVACTemplate:Plant:HotWaterLoop'
        epc = MagicMock()
        epc.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        del epc.primary_chilled_water_pump_rated_head
        del epc.condenser_plant_operation_scheme_type
        del epc.condenser_water_pump_type
        expanded_plant_loops = {'Test Hot Water': eph, 'Test Chilled Water': epc}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.condenser_type = 'WaterCooled'
        expanded_plant_equipment = {'Test Equipment Name': epe}
        expanded_pe = copy.deepcopy(expanded_plant_equipment)
        for _, expanded_pe in expanded_pe.items():
            plant_loop_template, plant_equipment_template = self.hvac_template._create_templates_from_plant_equipment(
                plant_equipment_class_object=expanded_pe,
                expanded_plant_loops=expanded_plant_loops)
            if plant_loop_template:
                self.hvac_template.merge_epjson(
                    super_dictionary=self.hvac_template.epjson,
                    object_dictionary=plant_loop_template
                )
                additional_plant_loops = self.hvac_template._expand_templates(
                    templates=plant_loop_template,
                    expand_class=ExpandPlantLoop
                )
                for k, v in additional_plant_loops.items():
                    if k not in expanded_plant_loops.keys():
                        expanded_plant_loops[k] = v
            if plant_equipment_template:
                self.hvac_template.merge_epjson(
                    super_dictionary=self.hvac_template.epjson,
                    object_dictionary=plant_equipment_template
                )
                additional_plant_equipment = self.hvac_template._expand_templates(
                    templates=plant_equipment_template,
                    expand_class=ExpandPlantEquipment,
                    plant_loop_class_objects=expanded_plant_loops
                )
                for k, v in additional_plant_equipment.items():
                    if k not in expanded_plant_equipment.keys():
                        expanded_plant_equipment[k] = v
        eo = ExpandObjects()
        self.assertEqual(
            {
                'AvailabilityManagerAssignmentList': 1,
                'Branch': 4,
                'CondenserEquipmentOperationSchemes': 1,
                'CondenserLoop': 1,
                'Pipe:Adiabatic': 3,
                'PlantEquipmentOperation:CoolingLoad': 1,
                'Pump:VariableSpeed': 1,
                'Schedule:Compact': 1,
                'Sizing:Plant': 1
            },
            eo.summarize_epjson(expanded_plant_loops['Condenser Water Loop'].epjson))
        return

    def test_condenser_water_plant_loop_object_created_from_multiple_chiller_wo_attributes_transitioned(self):
        eph = MagicMock()
        eph.template_type = 'HVACTemplate:Plant:HotWaterLoop'
        epc = MagicMock()
        epc.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        del epc.primary_chilled_water_pump_rated_head
        del epc.condenser_plant_operation_scheme_type
        del epc.condenser_water_pump_type
        expanded_plant_loops = {'Test Hot Water': eph, 'Test Chilled Water': epc}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.condenser_type = 'WaterCooled'
        epe2 = MagicMock()
        epe2.template_type = 'HVACTemplate:Plant:Chiller'
        epe2.condenser_type = 'WaterCooled'
        expanded_plant_equipment = {'Test Equipment Name': epe, 'Test Equipment Name 2': epe2}
        expanded_pe = copy.deepcopy(expanded_plant_equipment)
        for _, expanded_pe in expanded_pe.items():
            plant_loop_template, plant_equipment_template = self.hvac_template._create_templates_from_plant_equipment(
                plant_equipment_class_object=expanded_pe,
                expanded_plant_loops=expanded_plant_loops)
            if plant_loop_template:
                self.hvac_template.merge_epjson(
                    super_dictionary=self.hvac_template.epjson,
                    object_dictionary=plant_loop_template
                )
                additional_plant_loops = self.hvac_template._expand_templates(
                    templates=plant_loop_template,
                    expand_class=ExpandPlantLoop
                )
                for k, v in additional_plant_loops.items():
                    if k not in expanded_plant_loops.keys():
                        expanded_plant_loops[k] = v
            if plant_equipment_template:
                self.hvac_template.merge_epjson(
                    super_dictionary=self.hvac_template.epjson,
                    object_dictionary=plant_equipment_template
                )
                additional_plant_equipment = self.hvac_template._expand_templates(
                    templates=plant_equipment_template,
                    expand_class=ExpandPlantEquipment,
                    plant_loop_class_objects=expanded_plant_loops
                )
                for k, v in additional_plant_equipment.items():
                    if k not in expanded_plant_equipment.keys():
                        expanded_plant_equipment[k] = v
        eo = ExpandObjects()
        self.assertEqual(
            {
                'AvailabilityManagerAssignmentList': 1,
                'Branch': 4,
                'CondenserEquipmentOperationSchemes': 1,
                'CondenserLoop': 1,
                'Pipe:Adiabatic': 3,
                'PlantEquipmentOperation:CoolingLoad': 1,
                'Pump:VariableSpeed': 1,
                'Schedule:Compact': 1,
                'Sizing:Plant': 1
            },
            eo.summarize_epjson(expanded_plant_loops['Condenser Water Loop'].epjson))
        return

    def test_condenser_water_plant_loop_object_created_from_chiller_with_attributes_transitioned(self):
        eph = MagicMock()
        eph.template_type = 'HVACTemplate:Plant:HotWaterLoop'
        epc = MagicMock()
        epc.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        epc.condenser_water_pump_rated_head = 2000
        del epc.condenser_plant_operation_scheme_type
        del epc.condenser_water_pump_type
        expanded_plant_loops = {'Test Hot Water': eph, 'Test Chilled Water': epc}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.condenser_type = 'WaterCooled'
        expanded_plant_equipment = {'Test Equipment Name': epe}
        expanded_pe = copy.deepcopy(expanded_plant_equipment)
        for _, epe in expanded_pe.items():
            plant_loop_template, plant_equipment_template = self.hvac_template._create_templates_from_plant_equipment(
                plant_equipment_class_object=epe,
                expanded_plant_loops=expanded_plant_loops)
            if plant_loop_template:
                self.hvac_template.merge_epjson(
                    super_dictionary=self.hvac_template.epjson,
                    object_dictionary=plant_loop_template
                )
                additional_plant_loops = self.hvac_template._expand_templates(
                    templates=plant_loop_template,
                    expand_class=ExpandPlantLoop
                )
                for k, v in additional_plant_loops.items():
                    if k not in expanded_plant_loops.keys():
                        expanded_plant_loops[k] = v
            if plant_equipment_template:
                self.hvac_template.merge_epjson(
                    super_dictionary=self.hvac_template.epjson,
                    object_dictionary=plant_equipment_template
                )
                additional_plant_equipment = self.hvac_template._expand_templates(
                    templates=plant_equipment_template,
                    expand_class=ExpandPlantEquipment,
                    plant_loop_class_objects=expanded_plant_loops
                )
                for k, v in additional_plant_equipment.items():
                    if k not in expanded_plant_equipment.keys():
                        expanded_plant_equipment[k] = v
        eo = ExpandObjects()
        self.assertEqual({
            'AvailabilityManagerAssignmentList': 1,
            'Branch': 4,
            'CondenserLoop': 1,
            'CondenserEquipmentOperationSchemes': 1,
            'Pipe:Adiabatic': 3,
            'PlantEquipmentOperation:CoolingLoad': 1,
            'Pump:VariableSpeed': 1,
            'Schedule:Compact': 1,
            'Sizing:Plant': 1},
            eo.summarize_epjson(expanded_plant_loops['Condenser Water Loop'].epjson))
        self.assertEqual(
            2000,
            expanded_plant_loops['Condenser Water Loop'].epjson
            ['Pump:VariableSpeed']['Condenser Water Loop Supply Pump']['design_pump_head'])
        return

    def test_retrieve_branches_by_loop_type(self):
        eph = MagicMock()
        eph.template_type = 'HVACTemplate:Plant:HotWaterLoop'
        epc = MagicMock()
        epc.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        expanded_plant_loops = {'Test Hot Water': eph, 'Test Chilled Water': epc}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:HotWaterLoop'
        epe.template_plant_loop_type = 'HotWater'
        epe.epjson = {
            "Branch": {
                "Main Boiler HW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Boiler HW Inlet",
                            "component_name": "Main Boiler",
                            "component_object_type": "Boiler:HotWater",
                            "component_outlet_node_name": "Main Boiler HW Outlet"
                        }
                    ]
                }
            }
        }
        plant_equipment_class_objects = {'Main Boiler': epe}
        output_list = []
        for plc in expanded_plant_loops.values():
            branch_dictionary = self.hvac_template._get_plant_equipment_waterloop_branches_by_loop_type(
                plant_loop_class_object=plc,
                expanded_plant_equipment=plant_equipment_class_objects
            )
            output_list.append(branch_dictionary)
        self.assertEqual(
            1,
            len([i for i in output_list[0]['Branch'].keys()])
        )
        self.assertIsNone(output_list[1])
        return

    def test_retrieve_water_cooled_chiller_branches_by_loop_type(self):
        epch = MagicMock()
        epch.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        epcn = MagicMock()
        epcn.template_type = 'HVACTemplate:Plant:CondenserWaterLoop'
        expanded_plant_loops = {'Test Chilled Water': epch, 'Test Condenser Water': epcn}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.template_plant_loop_type = 'ChilledWater'
        epe.condenser_type = 'WaterCooled'
        epe.epjson = {
            "Branch": {
                "Main Chiller ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller ChW Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller ChW Outlet"
                        }
                    ]
                },
                "Main Chiller CndW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller Cnd Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller Cnd Outlet"
                        }
                    ]
                }
            }
        }
        epe2 = MagicMock()
        epe2.template_type = 'HVACTemplate:Plant:Chiller'
        epe2.template_plant_loop_type = 'ChilledWater'
        epe2.condenser_type = 'WaterCooled'
        epe2.epjson = {
            "Branch": {
                "Second Chiller ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller ChW Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller ChW Outlet"
                        }
                    ]
                },
                "Second Chiller CndW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller Cnd Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller Cnd Outlet"
                        }
                    ]
                }
            }
        }
        plant_equipment_class_objects = {'Main Chiller': epe, 'Second Chiller': epe2}
        output_list = []
        for plc in expanded_plant_loops.values():
            branch_dictionary = self.hvac_template._get_plant_equipment_waterloop_branches_by_loop_type(
                plant_loop_class_object=plc,
                expanded_plant_equipment=plant_equipment_class_objects
            )
            output_list.append(branch_dictionary)
        self.assertEqual(
            2,
            len([i for i in output_list[0]['Branch'].keys() if 'chw' in i.lower()])
        )
        self.assertEqual(
            2,
            len([i for i in output_list[1]['Branch'].keys() if 'cnd' in i.lower()])
        )
        return

    def test_retrieve_demand_branch_loop_type(self):
        ez = MagicMock()
        ez.template_type = 'HVACTemplate:Zone:VAV'
        ez.epjson = {
            "Branch": {
                "VAV Sys 1 Cooling Coil ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "VAV Sys 1 Cooling Coil ChW Inlet",
                            "component_name": "VAV Sys 1 Cooling Coil",
                            "component_object_type": "Coil:Cooling:Water",
                            "component_outlet_node_name": "VAV Sys 1 Cooling Coil ChW Outlet"
                        }
                    ]
                }
            }
        }
        zone_class_objects = {'Zone Template 1': ez}
        output_list = []
        ep = MagicMock()
        ep.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        ep2 = MagicMock()
        ep2.template_type = 'HVACTemplate:Plant:HotWaterLoop'
        expanded_plant_loops = {'Test Loop 1': ep, 'Test Loop 2': ep2}
        for epl in expanded_plant_loops.values():
            branch_dictionary = self.hvac_template._get_zone_system_waterloop_branches_by_loop_type(
                plant_loop_class_object=epl,
                expanded_zones=zone_class_objects,
                expanded_systems={})
            output_list.append(branch_dictionary)
        self.assertEqual(
            1,
            len([i for i in output_list[0]['Branch'].keys()]))
        self.assertIsNone(output_list[1])
        return

    def test_condenser_water_loop_connectors(self):
        epl = MagicMock()
        epl.template_name = 'Condenser Water Loop'
        epl.template_type = 'HVACTemplate:Plant:CondenserWaterLoop'
        plant_loop_class_object = {'Test Plant Loop': epl}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.template_plant_loop_type = 'ChilledWater'
        epe.condenser_type = 'WaterCooled'
        epe.epjson = {
            'Branch': {
                "Main Chiller ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller ChW Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller ChW Outlet"
                        }
                    ]
                },
                'Main Chiller CndW Branch': {'components': [{'component_inlet_node_name': 'Main Chiller Cnd Inlet',
                                                             'component_name': 'Main Chiller',
                                                             'component_object_type': 'Chiller:Electric:EIR',
                                                             'component_outlet_node_name': 'Main Chiller Cnd Outlet'}]}
            }
        }
        epe2 = MagicMock()
        epe2.template_type = 'HVACTemplate:Plant:Tower'
        epe2.template_plant_loop_type = 'CondenserWater'
        epe2.epjson = {
            'Branch': {
                "Main Tower CndW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Tower CndW Inlet",
                            "component_name": "Main Tower",
                            "component_object_type": "CoolingTower:SingleSpeed",
                            "component_outlet_node_name": "Main Tower CndW Outlet"
                        }
                    ]
                }
            }
        }
        expanded_plant_equipment = {'Test Chiller': epe, 'Test Tower': epe2}
        for epl in plant_loop_class_object.values():
            self.hvac_template._create_water_loop_connectors_and_nodelist(
                plant_loop_class_object=epl,
                expanded_plant_equipment=expanded_plant_equipment)
        self.assertEqual(
            {'BranchList': 2, 'Connector:Mixer': 2, 'Connector:Splitter': 2, 'ConnectorList': 2, 'NodeList': 1},
            self.hvac_template.summarize_epjson(self.hvac_template.epjson))
        return

    def test_plantequipmentlist_objects(self):
        epl = MagicMock()
        epl.template_name = 'Chilled Water Loop'
        epl.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        plant_loop_class_object = {'Test Plant Loop': epl}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.template_plant_loop_type = 'ChilledWater'
        epe.prirority = 1
        epe.condenser_type = 'WaterCooled'
        epe.epjson = {
            "Chiller:Electric:EIR": {
                "Main Chiller": {}
            },
            'Branch': {
                "Main Chiller ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller ChW Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller ChW Outlet"
                        }
                    ]
                },
                'Main Chiller CndW Branch': {'components': [{'component_inlet_node_name': 'Main Chiller Cnd Inlet',
                                                             'component_name': 'Main Chiller',
                                                             'component_object_type': 'Chiller:Electric:EIR',
                                                             'component_outlet_node_name': 'Main Chiller Cnd Outlet'}]}
            }
        }
        expanded_plant_equipment = {'Main Chiller': epe}
        for epl in plant_loop_class_object.values():
            self.hvac_template._create_plant_equipment_lists(
                plant_loop_class_object=epl,
                expanded_plant_equipment=expanded_plant_equipment)
        self.assertEqual(
            'Main Chiller',
            self.hvac_template.epjson['PlantEquipmentList']['Chilled Water Loop All Equipment']
            ['equipment'][0]['equipment_name']
        )
        return

    def test_plantequipmentlist_objects_with_priority(self):
        epl = MagicMock()
        epl.template_name = 'Chilled Water Loop'
        epl.template_type = 'HVACTemplate:Plant:ChilledWaterLoop'
        plant_loop_class_object = {'Test Plant Loop': epl}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:Chiller'
        epe.template_plant_loop_type = 'ChilledWater'
        epe.condenser_type = 'WaterCooled'
        epe.priority = 2
        epe.epjson = {
            "Chiller:Electric:EIR": {
                "Main Chiller": {
                }
            },
            'Branch': {
                "Main Chiller ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Chiller ChW Inlet",
                            "component_name": "Main Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Main Chiller ChW Outlet"
                        }
                    ]
                },
                'Main Chiller CndW Branch': {'components': [{'component_inlet_node_name': 'Main Chiller Cnd Inlet',
                                                             'component_name': 'Main Chiller',
                                                             'component_object_type': 'Chiller:Electric:EIR',
                                                             'component_outlet_node_name': 'Main Chiller Cnd Outlet'}]},
            }
        }
        epe2 = MagicMock()
        epe2.template_type = 'HVACTemplate:Plant:Chiller'
        epe2.template_plant_loop_type = 'ChilledWater'
        epe2.condenser_type = 'WaterCooled'
        epe2.priority = 1
        epe2.epjson = {
            "Chiller:Electric:EIR": {
                "Second Chiller": {
                }
            },
            'Branch': {
                "Second Chiller ChW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Second Chiller ChW Inlet",
                            "component_name": "Second Chiller",
                            "component_object_type": "Chiller:Electric:EIR",
                            "component_outlet_node_name": "Second Chiller ChW Outlet"
                        }
                    ]
                },
                'Second Chiller CndW Branch': {'components': [{'component_inlet_node_name': 'Second Chiller Cnd Inlet',
                                                               'component_name': 'Second Chiller',
                                                               'component_object_type': 'Chiller:Electric:EIR',
                                                               'component_outlet_node_name': 'Second Chiller Cnd Outlet'}]}
            }
        }
        expanded_plant_equipment = {'Main Chiller': epe, 'Second Chiller': epe2}
        for epl in plant_loop_class_object.values():
            self.hvac_template._create_plant_equipment_lists(
                plant_loop_class_object=epl,
                expanded_plant_equipment=expanded_plant_equipment)
        self.assertEqual(
            'Second Chiller',
            self.hvac_template.epjson['PlantEquipmentList']['Chilled Water Loop All Equipment']
            ['equipment'][0]['equipment_name']
        )
        self.assertEqual(
            'Main Chiller',
            self.hvac_template.epjson['PlantEquipmentList']['Chilled Water Loop All Equipment']
            ['equipment'][1]['equipment_name']
        )
        return

    def test_condenserequipmentlist_objects(self):
        epl = MagicMock()
        epl.template_name = 'Condenser Water Loop'
        epl.template_type = 'HVACTemplate:Plant:CondenserWaterLoop'
        plant_loop_class_object = {'Test Plant Loop': epl}
        epe = MagicMock()
        epe.template_type = 'HVACTemplate:Plant:CoolingTower'
        epe.template_plant_loop_type = 'CondenserWater'
        epe.epjson = {
            "CoolingTower:SingleSpeed": {
                "Main Tower": {}
            },
            'Branch': {
                "Main Tower CndW Branch": {
                    "components": [
                        {
                            "component_inlet_node_name": "Main Tower CndW Inlet",
                            "component_name": "Main Tower",
                            "component_object_type": "CoolingTower:SingleSpeed",
                            "component_outlet_node_name": "Main Tower CndW Outlet"
                        }
                    ]
                }
            }
        }
        expanded_plant_equipment = {'Main Tower': epe}
        for epl in plant_loop_class_object.values():
            self.hvac_template._create_plant_equipment_lists(
                plant_loop_class_object=epl,
                expanded_plant_equipment=expanded_plant_equipment)
        self.assertEqual(
            'Main Tower',
            self.hvac_template.epjson['CondenserEquipmentList']['Condenser Water Loop All Equipment']
            ['equipment'][0]['equipment_name'])
        return
