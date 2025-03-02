import unittest
import copy

from src.expand_objects import ExpandSystem
from src.expand_objects import PyExpandObjectsException, PyExpandObjectsYamlStructureException, \
    PyExpandObjectsTypeError
from . import BaseTest

mock_template = {
    "HVACTemplate:System:VAV": {
        "VAV Sys 1": {
            "cooling_coil_design_setpoint": 12.8,
            "cooling_coil_setpoint_reset_type": "None",
            "cooling_coil_type": "ChilledWater",
            "dehumidification_control_type": "None",
            "dehumidification_setpoint": 60.0,
            "economizer_lockout": "NoLockout",
            "economizer_lower_temperature_limit": 4,
            "economizer_type": "DifferentialDryBulb",
            "economizer_upper_temperature_limit": 19,
            "gas_heating_coil_efficiency": 0.8,
            "gas_heating_coil_parasitic_electric_load": 0.0,
            "gas_preheat_coil_efficiency": 0.8,
            "gas_preheat_coil_parasitic_electric_load": 0.0,
            "heat_recovery_type": "None",
            "heating_coil_design_setpoint": 10.0,
            "heating_coil_setpoint_reset_type": "None",
            "heating_coil_type": "HotWater",
            "humidifier_rated_capacity": 1e-06,
            "humidifier_rated_electric_power": 2690.0,
            "humidifier_setpoint": 30.0,
            "humidifier_type": "None",
            "latent_heat_recovery_effectiveness": 0.65,
            "maximum_outdoor_air_flow_rate": "Autosize",
            "minimum_outdoor_air_control_type": "FixedMinimum",
            "minimum_outdoor_air_flow_rate": "Autosize",
            "minimum_outdoor_air_schedule_name": "Min OA Sched",
            "night_cycle_control": "CycleOnAny",
            "preheat_coil_type": "None",
            "return_plenum_name": "PLENUM-1",
            "sensible_heat_recovery_effectiveness": 0.7,
            "sizing_option": "NonCoincident",
            "supply_fan_delta_pressure": 600,
            "supply_fan_maximum_flow_rate": "Autosize",
            "supply_fan_minimum_flow_rate": "Autosize",
            "supply_fan_motor_efficiency": 0.9,
            "supply_fan_motor_in_air_stream_fraction": 1,
            "supply_fan_part_load_power_coefficients": "InletVaneDampers",
            "supply_fan_placement": "DrawThrough",
            "supply_fan_total_efficiency": 0.7,
            "system_availability_schedule_name": "FanAvailSched"
        }
    }
}

mock_build_path = [
    {
        'OutdoorAir:Mixer': {
            'Fields': {
                'name': '{} OA Mixing Box',
                'mixed_air_node_name': '{} Mixed Air Outlet',
                'outdoor_air_stream_node_name': '{} Outside Air Inlet',
                'relief_air_stream_node_name': '{} Relief Air Outlet',
                'return_air_stream_node_name': '{} Air Loop Inlet'
            },
            'Connectors': {
                'AirLoop': {
                    'Inlet': 'return_air_stream_node_name',
                    'Outlet': 'mixed_air_node_name'
                }
            }
        }
    },
    {
        'Fan:VariableVolume': {
            'Fields': {
                'name': '{} Supply Fan',
                'air_inlet_node_name': '{} Supply Fan Inlet',
                'air_outlet_node_name': '{} Supply Fan Outlet'
            },
            'Connectors': {
                'AirLoop': {
                    'Inlet': 'air_inlet_node_name',
                    'Outlet': 'air_outlet_node_name'
                }
            }
        }
    }
]


class TestExpandSystem(BaseTest, unittest.TestCase):
    def setUp(self):
        return

    def teardown(self):
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:System:Input Template Required")
    def test_check_templates_are_required(self):
        with self.assertRaises(TypeError):
            ExpandSystem()
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:System:Verify valid template object")
    def test_verify_good_template(self):
        output = ExpandSystem(template=mock_template)
        self.assertEqual('VAV Sys 1', output.template_name)
        return

    def test_create_water_controller_list_from_epjson(self):
        es = ExpandSystem(template=mock_template)
        es.build_path = [
            {
                'Coil:Cooling:Water': {
                    'Fields': {
                        'name': 'Cooling Coil',
                        'water_inlet_node_name': 'Test Chw Inlet'
                    }
                }
            },
            {
                'Coil:Heating:Water': {
                    'Fields': {
                        'name': 'Heating Coil',
                        'water_inlet_node_name': 'Test HW Inlet'
                    }
                }
            }
        ]
        es.epjson = {
            'Controller:WaterCoil': {
                'test cooling water coil': {
                    'sensor_node_name': 'Test sensor 1',
                    'actuator_node_name': 'Test HW Inlet'
                },
                'test heating water coil': {
                    'sensor_node_name': 'Test sensonr 2',
                    'actuator_node_name': 'Test Chw Inlet'
                }
            }
        }
        controllerlist = es._create_controller_list_from_epjson()
        self.assertEqual('AirLoopHVAC:ControllerList', list(controllerlist.keys())[0])
        self.assertEqual(
            'test cooling water coil',
            controllerlist['AirLoopHVAC:ControllerList']['VAV Sys 1 Controllers']['controller_2_name'])
        self.assertEqual(
            'test heating water coil',
            controllerlist['AirLoopHVAC:ControllerList']['VAV Sys 1 Controllers']['controller_1_name'])
        return

    def test_create_water_controller_list_from_epjson_bad_reference(self):
        es = ExpandSystem(template=mock_template)
        es.build_path = [
            {
                'Coil:Cooling:Water': {
                    'Fields': {
                        'name': 'Cooling Coil',
                        'water_inlet_node_name': 'Test Chw Inlet'
                    }
                }
            },
            {
                'Coil:Heating:Water': {
                    'Fields': {
                        'name': 'Heating Coil',
                        'water_inlet_node_name': 'Test HW Inlet'
                    }
                }
            }
        ]
        es.epjson = {
            'Controller:WaterCoil': {
                'test cooling water coil': {
                    'sensor_node_name': 'Test sensor 1',
                    'actuator_node_name': 'Bad Inlet'
                },
                'test heating water coil': {
                    'sensor_node_name': 'Test sensonr 2',
                    'actuator_node_name': 'Test Chw Inlet'
                }
            }
        }
        with self.assertRaisesRegex(PyExpandObjectsTypeError, 'Actuator node for water coil object does not'):
            es._create_controller_list_from_epjson()
        return

    def test_create_water_controller_list_from_epjson_bad_controller(self):
        es = ExpandSystem(template=mock_template)
        es.build_path = [
            {
                'Coil:Cooling:Water': {
                    'Fields': {
                        'name': 'Cooling Coil',
                        'water_inlet_node_name': 'Test Chw Inlet'
                    }
                }
            },
            {
                'Coil:Heating:Water': {
                    'Fields': {
                        'name': 'Heating Coil',
                        'water_inlet_node_name': 'Test HW Inlet'
                    }
                }
            }
        ]
        es.epjson = {
            'Controller:WaterCoil': {
                'test cooling water coil': {},
                'test heating water coil': {
                    'sensor_node_name': 'Test sensonr 2',
                    'actuator_node_name': 'Test Chw Inlet'
                }
            }
        }
        with self.assertRaisesRegex(PyExpandObjectsTypeError, 'Controller object not properly formatted'):
            es._create_controller_list_from_epjson()
        return

    def test_create_outdoor_air_equipment_list_from_epjson(self):
        es = ExpandSystem(template=mock_template)
        temp_mock_build_path = copy.deepcopy(mock_build_path)
        temp_mock_build_path.insert(0, {
            'TestObject': {
                'Fields': {
                    'name': 'Test Name',
                },
                'Connectors': {}
            }
        })
        es.build_path = temp_mock_build_path
        oa_equipment_list_object = es._create_outdoor_air_equipment_list_from_build_path()
        self.assertEqual('AirLoopHVAC:OutdoorAirSystem:EquipmentList', list(oa_equipment_list_object.keys())[0])
        self.assertEqual(
            'Test Name',
            oa_equipment_list_object['AirLoopHVAC:OutdoorAirSystem:EquipmentList']
            ['VAV Sys 1 OA System Equipment']['component_1_name'])
        self.assertEqual(
            'VAV Sys 1 OA Mixing Box',
            oa_equipment_list_object['AirLoopHVAC:OutdoorAirSystem:EquipmentList']
            ['VAV Sys 1 OA System Equipment']['component_2_name'])
        return

    def test_create_outdoor_air_equipment_list_from_epjson_with_return_fan(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.build_path = [
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Return Fan',
                        'air_inlet_node_name': '{} Return Fan Inlet',
                        'air_outlet_node_name': '{} Return Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}},
            {
                'OutdoorAir:Mixer': {
                    'Fields': {
                        'name': '{} OA Mixing Box',
                        'mixed_air_node_name': '{} Mixed Air Outlet',
                        'outdoor_air_stream_node_name': '{} Outside Air Inlet',
                        'relief_air_stream_node_name': '{} Relief Air Outlet',
                        'return_air_stream_node_name': '{} Air Loop Inlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'return_air_stream_node_name',
                            'Outlet': 'mixed_air_node_name'}}}},
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Supply Fan',
                        'air_inlet_node_name': '{} Supply Fan Inlet',
                        'air_outlet_node_name': '{} Supply Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}}
        ]
        es.unique_name = 'TEST SYSTEM'
        es.return_fan = 'Yes'
        oa_equipment_list_object = es._create_outdoor_air_equipment_list_from_build_path()
        self.assertEqual('AirLoopHVAC:OutdoorAirSystem:EquipmentList', list(oa_equipment_list_object.keys())[0])
        self.assertEqual(
            'TEST SYSTEM OA Mixing Box',
            oa_equipment_list_object['AirLoopHVAC:OutdoorAirSystem:EquipmentList']
            ['TEST SYSTEM OA System Equipment']['component_1_name'])
        return

    def test_create_outdoor_air_equipment_list_from_epjson_with_return_fan_option_but_no_equipment(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.build_path = [
            {
                'OutdoorAir:Mixer': {
                    'Fields': {
                        'name': '{} OA Mixing Box',
                        'mixed_air_node_name': '{} Mixed Air Outlet',
                        'outdoor_air_stream_node_name': '{} Outside Air Inlet',
                        'relief_air_stream_node_name': '{} Relief Air Outlet',
                        'return_air_stream_node_name': '{} Air Loop Inlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'return_air_stream_node_name',
                            'Outlet': 'mixed_air_node_name'}}}},
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Supply Fan',
                        'air_inlet_node_name': '{} Supply Fan Inlet',
                        'air_outlet_node_name': '{} Supply Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}}
        ]
        es.unique_name = 'TEST SYSTEM'
        es.return_fan = 'Yes'
        with self.assertRaisesRegex(PyExpandObjectsException, 'Return fan specified'):
            es._create_outdoor_air_equipment_list_from_build_path()
        return

    def test_reject_create_outdoor_air_equipment_list_from_epjson_without_build_path(self):
        es = ExpandSystem(template=mock_template)
        with self.assertRaisesRegex(PyExpandObjectsException, 'Build path was not provided nor was it available'):
            es._create_outdoor_air_equipment_list_from_build_path()
        return

    def test_create_availability_manager_assignment_list(self):
        es = ExpandSystem(template=mock_template)
        es.epjson = {
            "AvailabilityManager:NightCycle": {
                "VAV Sys 1 Availability": {
                    "applicability_schedule_name": "HVACTemplate-Always 1",
                    "control_type": "CycleOnAny",
                    "cycling_run_time": 3600,
                    "cycling_run_time_control_type": "FixedRunTime",
                    "fan_schedule_name": "FanAvailSched",
                    "thermostat_tolerance": 0.2
                }
            }
        }
        availability_manager_assignment_list_object = es._create_availability_manager_assignment_list()
        self.assertEqual(
            'AvailabilityManagerAssignmentList',
            list(availability_manager_assignment_list_object.keys())[0])
        self.assertEqual(
            'VAV Sys 1 Availability',
            availability_manager_assignment_list_object['AvailabilityManagerAssignmentList']
            ['VAV Sys 1 Availability Managers']['managers'][0]['availability_manager_name'])
        return

    def test_reject_create_availability_manager_assignment_list_bad_object(self):
        es = ExpandSystem(template=mock_template)
        es.epjson = {
            "AvailabilityManager:NightCycle": ['bad', 'format']
        }
        with self.assertRaisesRegex(PyExpandObjectsTypeError, 'AvailabilityManager object not properly'):
            es._create_availability_manager_assignment_list()
        return

    def test_airloophvac_outdoor_air_system(self):
        es = ExpandSystem(template=mock_template)
        es.epjson = {
            "AirLoopHVAC:ControllerList": {
                "VAV Sys 1 Controllers": {
                    "controller_1_name": "VAV Sys 1 Cooling Coil Controller",
                    "controller_1_object_type": "Controller:WaterCoil",
                    "controller_2_name": "VAV Sys 1 Heating Coil Controller",
                    "controller_2_object_type": "Controller:WaterCoil"
                },
                "VAV Sys 1 OA System Controllers": {
                    "controller_1_name": "VAV Sys 1 OA Controller",
                    "controller_1_object_type": "Controller:OutdoorAir"
                }
            },
            "AirLoopHVAC:OutdoorAirSystem:EquipmentList": {
                "VAV Sys 1 OA System Equipment": {
                    "component_1_name": "VAV Sys 1 OA Mixing Box",
                    "component_1_object_type": "OutdoorAir:Mixer"
                }
            },
            "AvailabilityManagerAssignmentList": {
                "VAV Sys 1 Availability Managers": {
                    "managers": [
                        {
                            "availability_manager_name": "VAV Sys 1 Availability",
                            "availability_manager_object_type": "AvailabilityManager:NightCycle"
                        }
                    ]
                }
            }
        }
        oa_system_object = es._create_outdoor_air_system()
        self.assertEqual('AirLoopHVAC:OutdoorAirSystem', list(oa_system_object.keys())[0])
        self.assertEqual(
            'VAV Sys 1 OA System Controllers',
            oa_system_object['AirLoopHVAC:OutdoorAirSystem']['VAV Sys 1 OA System']['controller_list_name'])
        return

    def test_reject_outdoor_air_system_no_oa_controllerlist(self):
        es = ExpandSystem(template=mock_template)
        test_epjson = {
            "AirLoopHVAC:ControllerList": {
                "VAV Sys 1 Controllers": {
                    "controller_1_name": "VAV Sys 1 Cooling Coil Controller",
                    "controller_1_object_type": "Controller:WaterCoil",
                    "controller_2_name": "VAV Sys 1 Heating Coil Controller",
                    "controller_2_object_type": "Controller:WaterCoil"
                },
                "VAV Sys 1 OA System Controllers": {
                    "controller_1_name": "VAV Sys 1 OA Controller",
                    "controller_1_object_type": "Controller:OutdoorAir"
                }
            },
            "AirLoopHVAC:OutdoorAirSystem:EquipmentList": {
                "VAV Sys 1 OA System Equipment": {
                    "component_1_name": "VAV Sys 1 OA Mixing Box",
                    "component_1_object_type": "OutdoorAir:Mixer"
                }
            },
            "AvailabilityManagerAssignmentList": {
                "VAV Sys 1 Availability Managers": {
                    "managers": [
                        {
                            "availability_manager_name": "VAV Sys 1 Availability",
                            "availability_manager_object_type": "AvailabilityManager:NightCycle"
                        }
                    ]
                }
            }
        }
        no_oa_controller_epjson = copy.deepcopy(test_epjson)
        no_oa_controller_epjson["AirLoopHVAC:ControllerList"].pop("VAV Sys 1 OA System Controllers")
        es.epjson = no_oa_controller_epjson
        no_oa_system_epjson = copy.deepcopy(test_epjson)
        no_oa_system_epjson.pop('AirLoopHVAC:OutdoorAirSystem:EquipmentList')
        es.epjson = no_oa_system_epjson
        with self.assertRaisesRegex(
                PyExpandObjectsException,
                'Only one AirLoopHVAC:OutdoorAirSystem:EquipmentList'):
            es._create_outdoor_air_system()
        no_avail_epjson = copy.deepcopy(test_epjson)
        no_avail_epjson.pop('AvailabilityManagerAssignmentList')
        es.epjson = no_avail_epjson
        with self.assertRaisesRegex(
                PyExpandObjectsException,
                'Only one AvailabilityManagerAssignmentList'):
            es._create_outdoor_air_system()
        return

    def test_modify_build_path_for_outside_air_system(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.build_path = mock_build_path
        es.unique_name = 'TEST SYSTEM'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        modified_build_path = es._modify_build_path_for_outside_air_system()
        self.assertEqual('AirLoopHVAC:OutdoorAirSystem', list(modified_build_path[0].keys())[0])
        self.assertEqual(
            mock_build_path[0]['OutdoorAir:Mixer']['Fields']['return_air_stream_node_name'],
            modified_build_path[0]['AirLoopHVAC:OutdoorAirSystem']['Fields']['inlet_node_name']
        )
        return

    def test_modify_build_path_for_outside_air_system_with_return_fan(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.build_path = [
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Return Fan',
                        'air_inlet_node_name': '{} Return Fan Inlet',
                        'air_outlet_node_name': '{} Return Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}},
            {
                'OutdoorAir:Mixer': {
                    'Fields': {
                        'name': '{} OA Mixing Box',
                        'mixed_air_node_name': '{} Mixed Air Outlet',
                        'outdoor_air_stream_node_name': '{} Outside Air Inlet',
                        'relief_air_stream_node_name': '{} Relief Air Outlet',
                        'return_air_stream_node_name': '{} Air Loop Inlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'return_air_stream_node_name',
                            'Outlet': 'mixed_air_node_name'}}}},
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Supply Fan',
                        'air_inlet_node_name': '{} Supply Fan Inlet',
                        'air_outlet_node_name': '{} Supply Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}}
        ]
        es.unique_name = 'TEST SYSTEM'
        es.return_fan = 'Yes'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        modified_build_path = es._modify_build_path_for_outside_air_system()
        self.assertEqual('AirLoopHVAC:OutdoorAirSystem', list(modified_build_path[1].keys())[0])
        self.assertEqual(
            'Fan:VariableVolume',
            list(modified_build_path[0].keys())[0])
        return

    def test_reject_modify_build_path_for_outside_air_system_with_return_fan_specified_but_not_present(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.build_path = mock_build_path
        es.unique_name = 'TEST SYSTEM'
        es.return_fan = 'Yes'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        with self.assertRaisesRegex(PyExpandObjectsException, 'Return fan was specified'):
            es._modify_build_path_for_outside_air_system()
        return

    def test_modify_build_path_for_unitary_equipment(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.unique_name = 'TEST SYSTEM'
        es.template_type = 'HVACTemplate:System:Unitary'
        es.cooling_coil_type = 'ChilledWater'
        es.heating_coil_type = 'HotWater'
        es.build_path = [
            {
                'AirLoopHVAC:OutdoorAirSystem': {}
            },
            {
                'Coil:Cooling:Water': {}
            },
            {
                'Coil:Heating:Fuel': {}
            },
            {
                'Fan:VariableVolume': {}
            },
            {
                "AirLoopHVAC:Unitary:Furnace:HeatCool": {'Fields': {}, 'Connectors': {}}
            }
        ]
        output = es._modify_build_path_for_equipment()
        self.assertEqual(
            'AirLoopHVAC:Unitary:Furnace:HeatCool',
            list(output[1].keys())[0]
        )
        return

    def test_branch_from_build_path(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.unique_name = 'TEST SYSTEM'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        es.build_path = mock_build_path
        es._connect_and_convert_build_path_to_object_list()
        output = es._create_branch_and_branchlist_from_build_path()
        self.assertEqual(
            mock_build_path[0]['OutdoorAir:Mixer']['Fields']['return_air_stream_node_name'].format(es.unique_name),
            output['Branch']['{} Main Branch'.format(es.unique_name)]['components'][0]['component_inlet_node_name'])
        return

    def test_branch_from_build_path_with_return_fan(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.unique_name = 'TEST SYSTEM'
        es.return_fan = 'Yes'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        es.build_path = [
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Return Fan',
                        'air_inlet_node_name': '{} Return Fan Inlet',
                        'air_outlet_node_name': '{} Return Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}},
            {
                'OutdoorAir:Mixer': {
                    'Fields': {
                        'name': '{} OA Mixing Box',
                        'mixed_air_node_name': '{} Mixed Air Outlet',
                        'outdoor_air_stream_node_name': '{} Outside Air Inlet',
                        'relief_air_stream_node_name': '{} Relief Air Outlet',
                        'return_air_stream_node_name': '{} Air Loop Inlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'return_air_stream_node_name',
                            'Outlet': 'mixed_air_node_name'}}}},
            {
                'Fan:VariableVolume': {
                    'Fields': {
                        'name': '{} Supply Fan',
                        'air_inlet_node_name': '{} Supply Fan Inlet',
                        'air_outlet_node_name': '{} Supply Fan Outlet'},
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'air_inlet_node_name',
                            'Outlet': 'air_outlet_node_name'}}}}
        ]
        es._connect_and_convert_build_path_to_object_list()
        output = es._create_branch_and_branchlist_from_build_path()
        self.assertEqual(
            'TEST SYSTEM Return Fan Outlet',
            output['Branch']['{} Main Branch'.format(es.unique_name)]['components'][1]['component_inlet_node_name'])
        return

    def test_branchlist_from_build_path(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        es.unique_name = 'TEST SYSTEM'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        es.build_path = mock_build_path
        es._connect_and_convert_build_path_to_object_list()
        output = es._create_branch_and_branchlist_from_build_path()
        self.assertEqual(
            '{} Main Branch'.format(es.unique_name),
            output['BranchList']['{} Branches'.format(es.unique_name)]['branches'][0]['branch_name'])
        return

    def test_reject_create_branch_and_branchlist_from_build_path_no_build_path(self):
        es = ExpandSystem(template={'template_type': {'template_name': {}}})
        with self.assertRaisesRegex(PyExpandObjectsException, 'Build path was not provided'):
            es._create_branch_and_branchlist_from_build_path(build_path=[])
        return

    def test_reject_create_branch_and_branchlist_from_build_path_mismatch_connectors(self):
        temp_mock_build_path = copy.deepcopy(mock_build_path)
        temp_mock_build_path[1]['Fan:VariableVolume']['Connectors']['AirLoop']['Inlet'] = 'bad_connector_name'
        es = ExpandSystem(template=mock_template)
        es.unique_name = 'TEST SYSTEM'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, "Field/Connector mismatch"):
            es._create_branch_and_branchlist_from_build_path(build_path=temp_mock_build_path)
        return

    def test_reject_create_branch_and_branchlist_from_build_path_no_connectors(self):
        temp_mock_build_path = copy.deepcopy(mock_build_path)
        temp_mock_build_path[1]['Fan:VariableVolume'].pop('Connectors')
        es = ExpandSystem(template=mock_template)
        es.unique_name = 'TEST SYSTEM'
        es.epjson = {
            "AirLoopHVAC:OutdoorAirSystem": {
                "TEST SYSTEM OA System": {
                    "availability_manager_list_name": "TEST SYSTEM Availability Managers",
                    "controller_list_name": "TEST SYSTEM OA System Controllers",
                    "outdoor_air_equipment_list_name": "TEST SYSTEM OA System Equipment"
                }
            }
        }
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, "Super object is missing Connectors"):
            es._create_branch_and_branchlist_from_build_path(build_path=temp_mock_build_path)
        return
