import unittest
from unittest.mock import MagicMock
import copy

from src.expand_objects import ExpandObjects, ExpandZone, ExpandSystem
from src.expand_objects import PyExpandObjectsTypeError, PyExpandObjectsYamlStructureException, \
    PyExpandObjectsYamlError, PyExpandObjectsException
from . import BaseTest

mock_zone_template = {
    'HVACTemplate:Zone:VAV': {
        'template_name': {
            'template_field': 'template_test_value',
            'template_field2': 'test_pre_mapped_value',
            'reheat_coil_type': 'HotWater',
            'zone_name': 'test zone'
        }
    }
}

mock_zone_option_tree = {
    'OptionTree': {
        'Zone': {
            'VAV': {
                'BaseObjects': {
                    "Objects": [
                        {
                            'ZoneHVAC:AirDistributionUnit': {
                                "name": "{} ATU",
                                "field_name": "field_value"
                            }
                        }
                    ],
                    'Transitions': [
                        {
                            "ZoneHVAC:AirDistributionUnit": {
                                "template_field": "object_test_field",
                                "template_field2": "object_test_field2"
                            }
                        }
                    ],
                    'Mappings': [
                        {
                            'ZoneHVAC:AirDistributionUnit': {
                                "template_field2": {
                                    "test_pre_mapped_value": {
                                        "test_map_field": "test_mapped_value"
                                    }
                                }
                            }
                        }
                    ]
                },
                'TemplateObjects': {
                    'reheat_coil_type': {
                        "HotWater": {
                            'Objects': [
                                [
                                    {
                                        'AirTerminal:SingleDuct:VAV:Reheat': {
                                            'name': '{} VAV Reheat',
                                            'maximum_air_flow_rate': 'Autosize',
                                        }
                                    }
                                ],
                                {
                                    "Branch": {
                                        "name": "{} HW Branch"
                                    }
                                }
                            ],
                            'Transitions': [
                                {
                                    "AirTerminal:.*": {
                                        "template_field": "object_test_field"
                                    }
                                }
                            ]
                        }
                    }
                }
            }
        }
    }
}

mock_system_template = {
    'HVACTemplate:System:VAV': {
        'template_name': {
            'template_field': 'template_test_value',
            'cooling_coil_type': 'ChilledWater'
        }
    }
}

mock_system_option_tree = {
    'OptionTree': {
        'HVACTemplate': {
            'System': {
                'VAV': {
                    'BuildPath': {
                        'BaseObjects': {
                            'Objects': [
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
                                                'Inlet': 'outdoor_air_stream_node_name',
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
                            ],
                            'Transitions': [
                                {
                                    "Fan:.*": {
                                        "template_field": "object_test_field"
                                    }
                                }
                            ]
                        },
                        'Actions': [
                            {
                                'cooling_coil_type': {
                                    'ChilledWater': {
                                        'ObjectReference': 'OutdoorAir:Mixer',
                                        'Location': 'After',
                                        'ActionType': 'Insert',
                                        'Objects': [
                                            {
                                                'Coil:Cooling:Water': {
                                                    'Fields': {
                                                        'name': '{} Cooling Coil',
                                                        'air_inlet_node_name': '{} Cooling Coil Inlet',
                                                        'air_outlet_node_name': '{} Cooling Coil Outlet',
                                                        'water_inlet_node_name': '{} Cooling Coil Chw Inlet',
                                                        'water_outlet_node_name': '{} Cooling Coil Chw Outlet'
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
                                    }
                                }
                            }
                        ]
                    }
                }
            }
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
                    'Inlet': 'outdoor_air_stream_node_name',
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


class TestExpandObjectsYaml(BaseTest, unittest.TestCase):
    """
    General handling and processing of YAML instructions
    """
    def setUp(self):
        return

    def teardown(self):
        return

    def test_get_option_tree_from_yaml(self):
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=mock_zone_option_tree)
        structure_hierarcy = ['OptionTree', 'Zone', 'VAV']
        output = eo._get_option_tree(structure_hierarchy=structure_hierarcy)
        key_check = True
        try:
            output['BaseObjects']
        except KeyError:
            key_check = False
        self.assertTrue(key_check)
        # test without OptionTree
        structure_hierarchy = ['Zone', 'VAV']
        output = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        key_check = True
        try:
            output['BaseObjects']
        except KeyError:
            key_check = False
        self.assertTrue(key_check)
        return

    def test_reject_bad_yaml(self):
        bad_yaml_string = "bad brackets: ]["
        with self.assertRaisesRegex(PyExpandObjectsYamlError, 'Problem loading'):
            ExpandObjects(
                template=mock_zone_template,
                expansion_structure=bad_yaml_string)
        return

    def test_reject_bad_structure_format(self):
        with self.assertRaisesRegex(PyExpandObjectsTypeError, '.*is not a file path or dictionary.*'):
            ExpandObjects(
                template=mock_zone_template,
                expansion_structure=[])
        return

    def test_reject_bad_option_tree_request(self):
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=mock_zone_option_tree)
        structure_hierarchy = 'BadString'
        with self.assertRaisesRegex(PyExpandObjectsTypeError, 'Call to YAML object'):
            eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        return

    def test_template_object_with_none_option_creates_object(self):
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure={
                'OptionTree': {
                    'HVACTemplate': {
                        'Zone': {
                            'VAV': {
                                'BaseObjects': {
                                    'Objects': {}
                                },
                                'TemplateObjects': {
                                    'SomeNonPresentField': {
                                        'None': {
                                            'Objects': [
                                                {
                                                    'Object:1': {
                                                        'name': 'object_name',
                                                        'template_test_field': 'template_test_value'
                                                    }
                                                }
                                            ]
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        )
        eo._create_objects()
        self.assertEqual('template_test_value', eo.epjson['Object:1']['object_name']['template_test_field'])
        return

    def test_option_tree_leaf(self):
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=mock_zone_option_tree)
        structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        key_check = True
        for k in option_tree_leaf.keys():
            if k not in ['Objects', 'Transitions', 'Mappings']:
                key_check = False
        self.assertTrue(key_check)
        return

    def test_reject_option_tree_leaf_bad_structure(self):
        option_tree = {'mock': 'object'}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        eo.get_structure = MagicMock()
        eo.get_structure.return_value = {'BadKey': []}
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Invalid or missing Objects'):
            eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        return

    def test_option_tree_leaf_multiple_mappings(self):
        eo = ExpandObjects()
        eo.supply_fan_part_load_power_coefficients = 'InletVaneDampers'
        option_tree = {
            'BaseObjects': {
                'Objects': [
                    {
                        'Fan:VariableVolume': {
                            'name': 'test_name'
                        }
                    }
                ],
                'Mappings': [
                    {
                        'Fan:.*': {
                            'supply_fan_part_load_power_coefficients': {
                                'InletVaneDampers': {
                                    'fan_power_coefficient_1': 0.35071223,
                                    'fan_power_coefficient_2': 0.30850535,
                                    'fan_power_coefficient_3': -0.54137364,
                                    'fan_power_coefficient_4': 0.87198823,
                                    'fan_power_coefficient_5': 0
                                }
                            }
                        }
                    }
                ]
            }
        }
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        object_list = eo._apply_transitions(option_tree_leaf=option_tree_leaf)
        self.assertEqual(0.35071223, object_list[0]['Fan:VariableVolume']['fan_power_coefficient_1'])
        return

    def test_option_tree_leaf_without_transitions(self):
        # remove Transitions for this test
        bad_mock_zone_option_tree = copy.deepcopy(mock_zone_option_tree)
        bad_mock_zone_option_tree['OptionTree']['Zone']['VAV']['BaseObjects'].pop('Transitions')
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=bad_mock_zone_option_tree)
        structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        self.assertIsNone(option_tree_leaf['Transitions'])
        return

    def test_apply_transitions(self):
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=mock_zone_option_tree)
        structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        transitioned_option_tree_leaf = eo._apply_transitions(option_tree_leaf)
        self.assertEqual(
            'template_test_value',
            transitioned_option_tree_leaf[0]['ZoneHVAC:AirDistributionUnit']['object_test_field'])
        return

    def test_apply_transitions_bad_transition_structure(self):
        option_tree_leaf = {
            'Objects': {'test': 'val'},
            'BadKey': {'test': 'val'},
            'Transitions': [{'BadKey': {'bad_obj_ref': 'bad_obj_structure'}}]}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'OptionTree leaf is incorrectly formatted'):
            eo._apply_transitions(option_tree_leaf=option_tree_leaf)
        return

    def test_apply_transitions_bad_mapping_structure(self):
        option_tree_leaf = {
            'Objects': {'test': 'val'},
            'BadKey': {'test': 'val'},
            'Mappings': [{'BadKey': {'bad_obj_ref': 'bad_obj_structure'}}]}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'OptionTree leaf is incorrectly formatted'):
            eo._apply_transitions(option_tree_leaf=option_tree_leaf)
        return

    def test_apply_transitions_and_map(self):
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=mock_zone_option_tree)
        structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        transitioned_option_tree_leaf = eo._apply_transitions(option_tree_leaf)
        self.assertEqual(
            'test_mapped_value',
            transitioned_option_tree_leaf[0]['ZoneHVAC:AirDistributionUnit']['test_map_field'])
        return

    def test_yaml_list_to_dictionary_regular_object(self):
        dict_1 = {
            "Object:1": {
                "name": "test_name",
                "field_1": "val_1"
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        output = eo.yaml_list_to_epjson_dictionaries([dict_1, ])
        self.assertEqual('val_1', output['Object:1']['test_name']['field_1'])
        return

    def test_yaml_list_to_dictionary_super_object(self):
        dict_1 = {
            "Object:1": {
                "Fields": {
                    "name": "test_name",
                    "field_1": "val_1"
                },
                "Connectors": {}
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        output = eo.yaml_list_to_epjson_dictionaries([dict_1, ])
        self.assertEqual('val_1', output['Object:1']['test_name']['field_1'])
        return

    # Commented out because error statement was getting over used
    # def test_warning_on_bad_apply_transitions(self):
    #     # make a bad template reference
    #     bad_mock_zone_option_tree = mock_zone_option_tree
    #     bad_mock_zone_option_tree['OptionTree']['Zone']['VAV']['BaseObjects']['Transitions'] = [
    #         {
    #             "ZoneHVAC:AirDistributionUnit": {
    #                 "template_bad_field": "object_test_field"
    #             }
    #         }
    #     ]
    #     eo = ExpandObjects(
    #         template=mock_zone_template,
    #         expansion_structure=bad_mock_zone_option_tree)
    #     structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
    #     option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
    #     option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
    #     eo._apply_transitions(option_tree_leaf)
    #     # Logger (Parent class of ExpandObjects) keeps logs in self.stream
    #     self.assertIn(
    #         'A template value was attempted to be applied',
    #         eo.stream.getvalue()
    #     )
    #     return

    def test_error_on_bad_object(self):
        # make a bad template reference
        # check missing 'name' field
        bad_mock_zone_option_tree = copy.deepcopy(mock_zone_option_tree)
        bad_mock_zone_option_tree['OptionTree']['Zone']['VAV']['BaseObjects']['Objects'] = [
            {
                'ZoneHVAC:AirDistributionUnit': {
                    "bad_name": "{} ATU"
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=bad_mock_zone_option_tree)
        structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        object_list = eo._apply_transitions(option_tree_leaf)
        with self.assertRaises(PyExpandObjectsYamlStructureException):
            eo.yaml_list_to_epjson_dictionaries(object_list)
        # more than one object in a dictionary
        bad_mock_zone_option_tree = copy.deepcopy(mock_zone_option_tree)
        bad_mock_zone_option_tree['OptionTree']['Zone']['VAV']['BaseObjects']['Objects'] = [
            {
                'ZoneHVAC:AirDistributionUnit': {
                    "name": "{} ATU"
                },
                'ZoneHVAC:AirDistributionUnit2': {
                    "name": "{} ATU"
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=bad_mock_zone_option_tree)
        structure_hierarchy = ['OptionTree', 'Zone', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        option_tree_leaf = eo._get_option_tree_leaf(option_tree=option_tree, leaf_path=['BaseObjects', ])
        object_list = eo._apply_transitions(option_tree_leaf)
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'YAML object is incorrectly formatted'):
            eo.yaml_list_to_epjson_dictionaries(object_list)
        return

    def test_retrieve_objects_from_option_tree(self):
        eo = ExpandZone(template=mock_zone_template)
        structure_hierarchy = ['OptionTree', 'HVACTemplate', 'Zone', 'VAV']
        template_objects = eo._get_option_tree_objects(structure_hierarchy=structure_hierarchy)
        self.assertEqual(
            eo.summarize_epjson(template_objects),
            {'AirTerminal:SingleDuct:VAV:Reheat': 1,
             'Branch': 1,
             'Coil:Heating:Water': 1,
             'DesignSpecification:OutdoorAir': 1,
             'DesignSpecification:ZoneAirDistribution': 1,
             'Sizing:Zone': 1,
             'ZoneHVAC:AirDistributionUnit': 1,
             'ZoneHVAC:EquipmentConnections': 1,
             'ZoneHVAC:EquipmentList': 1}
        )
        return

    def test_get_option_tree_no_match(self):
        structure_hierarchy = ['mock', 'object']
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        eo._get_option_tree = MagicMock()
        eo._get_option_tree.return_value = {'TemplateObjects': {'field_name': {'field_value': 'object_field'}}}
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'template option was not applied for '
                                                                           'template field'):
            eo._get_option_tree_objects(structure_hierarchy=structure_hierarchy)
        return

    def test_get_option_tree_bad_value(self):
        structure_hierarchy = ['mock', 'object']
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        eo._get_option_tree = MagicMock()
        eo._get_option_tree.return_value = {'TemplateObjects': ['test', ]}
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'TemplateObjects section for system'):
            eo._get_option_tree_objects(structure_hierarchy=structure_hierarchy)
        return

    def test_complex_inputs_simple(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": "value_1"
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_1",
            input_value="{} test_val"
        )
        self.assertEqual('test zone test_val', [i for i in output][0]["value"])
        # number test
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_1",
            input_value=3
        )
        self.assertEqual(3, [i for i in output][0]["value"])
        return

    def test_complex_inputs_class_attribute_reference_string(self):
        eo = ExpandZone(template=mock_zone_template)
        eo.test_val = "test_string"
        output = eo._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value="{test_val}"
        )
        self.assertEqual('test_string', [o for o in output][0]['value'])
        return

    def test_reject_complex_inputs_class_attribute_reference_bad_string(self):
        eo = ExpandZone(template=mock_zone_template)
        eo.bad_test_val = "test_string"
        output = eo._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value="{test_val}"
        )
        self.assertIsNone([o for o in output][0]['value'])
        return

    def test_skip_none_resolve_objects_attribute_reference_bad_string(self):
        eo = ExpandZone(template=mock_zone_template)
        eo.test_val = "test_string"
        output = eo.resolve_objects(epjson={
            "Object:1":
                {
                    "object_1_name": {
                        "field_1": '{bad_test_val}',
                        "field_2": '{test_val}'
                    }
                }
        })
        self.assertEqual(1, len(output['Object:1']['object_1_name'].keys()))
        self.assertEqual('test_string', output['Object:1']['object_1_name']['field_2'])
        return

    def test_complex_inputs_class_attribute_reference_float(self):
        eo = ExpandZone(template=mock_zone_template)
        eo.test_val = "1.0"
        output = eo._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value="{test_val}"
        )
        self.assertTrue(isinstance([o for o in output][0]['value'], float))
        return

    def test_complex_inputs_class_attribute_reference_int(self):
        eo = ExpandZone(template=mock_zone_template)
        eo.test_val = "1"
        output = eo._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value="{test_val}"
        )
        self.assertTrue(isinstance([o for o in output][0]['value'], int))
        return

    def test_complex_inputs_dictionary(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": "value_1"
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        # field value check
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_1",
            input_value={
                "Object:1": "field_1"
            }
        )
        tmp_d = {}
        for o in output:
            tmp_d[o['field']] = o['value']
        self.assertEqual('field_1', list(tmp_d.keys())[0])
        self.assertEqual('value_1', tmp_d['field_1'])
        # dictionary key check
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_1",
            input_value={
                "Object:1": "self"
            }
        )
        tmp_d = {}
        for o in output:
            tmp_d[o['field']] = o['value']
        self.assertEqual('Object:1', tmp_d['field_1'])
        # name check
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_1",
            input_value={
                "Object:1": "key"
            }
        )
        tmp_d = {}
        for o in output:
            tmp_d[o['field']] = o['value']
        self.assertEqual('name_1', tmp_d['field_1'])
        return

    def test_complex_inputs_recursion_dictionary(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": "value_1"
                }
            },
            "Object:2": {
                "name_1": {
                    "field_1": {
                        "Object:1": "field_1"
                    }
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        # field value check
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_test",
            input_value={
                "Object:2": "field_1"
            }
        )
        tmp_d = {}
        for o in output:
            tmp_d[o['field']] = o['value']
        self.assertEqual('value_1', tmp_d['field_test'])
        return

    def test_complex_inputs_recursion_limit(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": {
                        "Object:2": "field_1"
                    }
                }
            },
            "Object:2": {
                "name_1": {
                    "field_1": {
                        "Object:1": "field_1"
                    }
                }
            }
        }
        eo = ExpandObjects(
            template=mock_zone_template,
            expansion_structure=mock_zone_option_tree)
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Maximum Recursion limit exceeded when '
                                                                           'resolving'):
            output = eo._resolve_complex_input(
                epjson=test_d,
                field_name="field_test",
                input_value={
                    "Object:2": "field_1"
                }
            )
            tmp_d = {}
            for o in output:
                tmp_d[o['field']] = o['value']
        return

    def test_complex_inputs_list(self):
        test_d = {
            "Object:2": {
                "name_1": {
                    "field_1": "value_1"
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        # field value check
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_test",
            input_value=[
                {"field_sub_test": {"Object:2": "field_1"}}
            ]
        )
        tmp_d = {}
        for o in output:
            tmp_d[o['field']] = o['value']
        self.assertEqual('value_1', tmp_d['field_test'][0]['field_sub_test'])
        return

    def test_complex_inputs_list_recursion(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": "value_1"
                }
            },
            "Object:2": {
                "name_1": {
                    "field_1": {
                        "Object:1": "field_1"
                    }
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        # field value check
        output = eo._resolve_complex_input(
            epjson=test_d,
            field_name="field_test",
            input_value=[
                {"field_sub_test": {"Object:2": "field_1"}}
            ]
        )
        tmp_d = {}
        for o in output:
            tmp_d[o['field']] = o['value']
        self.assertEqual('value_1', tmp_d['field_test'][0]['field_sub_test'])
        return

    def test_resolve_complex_inputs_object(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": "value_1"
                }
            },
            "Object:2": {
                "name_1": {
                    "field_1": {
                        "Object:1": "field_1"
                    }
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        eo.resolve_objects(epjson=test_d)
        self.assertEqual('value_1', test_d['Object:2']['name_1']['field_1'])
        return

    def test_resolve_complex_inputs_object_with_template_reference(self):
        test_d = {
            "Object:1": {
                "name_1": {
                    "field_1": 'test {template_field}'
                }
            }
        }
        eo = ExpandZone(template=mock_zone_template)
        eo.resolve_objects(epjson=test_d)
        self.assertEqual('test template_test_value', test_d['Object:1']['name_1']['field_1'])
        return

    def test_complex_nested_test(self):
        test_d = {
            'AirTerminal:SingleDuct:VAV:Reheat': {
                'SPACE1-1 VAV Reheat': {
                    'air_inlet_node_name': '{} Zone Equip Inlet',
                    'air_outlet_node_name': '{} Supply Inlet',
                    'damper_air_outlet_node_name': '{} Damper Outlet',
                    'damper_heating_action': 'Reverse',
                    'maximum_air_flow_rate': 'Autosize',
                    'maximum_hot_water_or_steam_flow_rate': 'Autosize',
                    'reheat_coil_name': '{} Reheat Coil',
                    'reheat_coil_object_type': 'Coil:Heating:Water',
                    'zone_minimum_air_flow_input_method': 'Constant'}},
            'Branch': {
                'SPACE1-1 HW Reheat Branch': {
                    'components': [
                        {
                            'component_inlet_node_name': {'Coil:Heating:Water': 'water_inlet_node_name'},
                            'component_name': {'Coil:Heating:Water': 'key'},
                            'component_object_type': {'Coil:Heating:Water': 'self'},
                            'component_outlet_node_name': {'Coil:Heating:Water': 'water_outlet_node_name'}
                        }
                    ]
                }
            },
            'Coil:Heating:Water': {
                'SPACE1-1 Reheat Coil': {
                    'air_inlet_node_name': '{} Damper Outlet',
                    'air_outlet_node_name': '{} Supply Inlet',
                    'maximum_water_flow_rate': 'Autosize',
                    'performance_input_method': 'UFactorTimesAreaAndDesignWaterFlowRate',
                    'rated_capacity': 'Autosize',
                    'rated_inlet_air_temperature': 16.6,
                    'rated_inlet_water_temperature': 82.2,
                    'rated_outlet_air_temperature': 32.2,
                    'rated_outlet_water_temperature': 71.1,
                    'rated_ratio_for_air_and_water_convection': 0.5,
                    'u_factor_times_area_value': 'Autosize',
                    'water_inlet_node_name': '{} Heating Coil Hw Inlet',
                    'water_outlet_node_name': '{} Heating Coil Hw Outlet'}},
            'ZoneHVAC:AirDistributionUnit': {
                'SPACE1-1 ATU': {
                    'air_distribution_unit_outlet_node_name':
                        {'^AirTerminal:.*': 'air_outlet_node_name'},
                    'air_terminal_name':
                        {'^AirTerminal:.*': 'key'},
                    'air_terminal_object_type':
                        {'^AirTerminal:.*': 'self'}
                }
            }
        }
        eo = ExpandZone(
            template=mock_zone_template)
        eo.resolve_objects(epjson=test_d)
        # Check that no string remains unformatted.  The * and ^ are the common regex special characters.
        self.assertNotIn('{}', eo.epjson)
        self.assertNotIn('^', eo.epjson)
        self.assertNotIn('*', eo.epjson)
        return

    def test_complex_inputs_bad_reference_object(self):
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Complex input reference is invalid'):
            output = eo._resolve_complex_input(
                epjson={},
                field_name="field_1",
                input_value={
                    "Bad Reference 1": 'val',
                    "Bad Reference 2": 'val'
                }
            )
            print([i for i in output])
        return

    def test_complex_inputs_bad_build_path_reference_object(self):
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Build Path complex input was specified '
                                                                           'with no build path'):
            output = eo._resolve_complex_input(
                epjson={},
                field_name="field_1",
                input_value={
                    'BuildPathReference': {}
                }
            )
            print([i for i in output])
        return

    def test_complex_inputs_bad_build_path_reference_object_instructions(self):
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Object field could not be resolved'):
            output = eo._resolve_complex_input(
                epjson={},
                field_name="field_1",
                input_value={
                    'BuildPathReference': {}
                },
                build_path=['test', ]
            )
            print([i for i in output])
        return

    def test_complex_inputs_bad_build_path_reference_maximum_recursion(self):
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        eo.build_path = [
            {
                "Object1": {
                    "Fields": {
                        'field1': {
                            'BuildPathReference': {
                                'Location': 1,
                                'ValueLocation': 'Inlet'
                            }
                        }
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1'
                        }
                    }
                }
            },
            {
                "Object2": {
                    "Fields": {
                        'field1': {
                            'BuildPathReference': {
                                'Location': 0,
                                'ValueLocation': 'Inlet'
                            }
                        }
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1'
                        }
                    }
                }
            }
        ]
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Object field could not be resolved'):
            output = eo._resolve_complex_input(
                epjson={},
                field_name="field_1",
                input_value={
                    'BuildPathReference': {
                        'Location': 0,
                        'ValueLocation': 'Inlet'
                    }
                }
            )
            print([i for i in output])
        return

    def test_complex_inputs_from_build_path_no_location(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Build Path Location or ValueLocation '
                                                                           'reference is invalid'):
            eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        return

    def test_complex_inputs_from_build_path_bad_location(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {
            "Location": 5,
            'ValueLocation': 'Inlet'
        }
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Invalid build path or lookup instructions'):
            eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        return

    def test_complex_inputs_from_build_path_bad_occurrence(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {
            'Location': 'Object1',
            'ValueLocation': 'Inlet',
            'Occurrence': 'BadVal'}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Occurrence key in complex reference is '
                                                                           'not an integer'):
            eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        return

    def test_complex_inputs_from_build_path_occurrences_not_reached(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {
            'Location': 'Object1',
            'ValueLocation': 'Inlet',
            'Occurrence': 2}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        output = eo.stream.getvalue()
        self.assertRegex(output, 'The number of occurrence matches')
        return

    def test_complex_inputs_from_build_path_bad_valuelocation(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {
            'Location': 'Object1',
            'ValueLocation': 'Bad'}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Invalid complex input for Build Path Lookup'):
            eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        return

    def test_complex_inputs_from_build_path_by_reference(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {
            'Location': 'Object1',
            'ValueLocation': 'Inlet'}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        output = eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        self.assertEqual(output, 'val1')
        return

    def test_complex_inputs_from_build_path_by_location(self):
        build_path = [
            {
                'Object1': {
                    "Fields": {
                        'field1': 'val1',
                        'field2': 'val1'
                    },
                    'Connectors': {
                        'AirLoop': {
                            'Inlet': 'field1',
                            'Outlet': 'field2'
                        }
                    }
                }
            }
        ]
        lookup_instructions = {
            'Location': 0,
            'ValueLocation': 'Inlet'}
        eo = ExpandObjects()
        eo.template_type = 'test type'
        eo.template_name = 'test name'
        output = eo._resolve_complex_input_from_build_path(build_path=build_path, lookup_instructions=lookup_instructions)
        self.assertEqual(output, 'val1')
        return

    def test_field_with_zero_value_processing(self):
        eo = ExpandObjects()
        output = eo.yaml_list_to_epjson_dictionaries([{
            'Object:1': {
                'name': 'test_name',
                'field': 0
            }
        }])
        output = eo.resolve_objects(epjson=output)
        self.assertEqual(0, output['Object:1']['test_name']['field'])
        return

    def test_build_path_action_non_super_object_processed_and_saved_to_epjson(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 1,
            'Occurrence': 1,
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                },
                {
                    'non_super_object': {
                        'name': 'test_non_super_object',
                        'test_non_super_field': 'test_non_super_val'
                    }
                }
            ]
        }
        eo = ExpandObjects()
        eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual(
            'test_non_super_val', eo.epjson['non_super_object']['test_non_super_object']['test_non_super_field'])
        return

    def test_build_path_connections(self):
        # Note ObjectReference is not needed
        eo = ExpandSystem(template=mock_system_template)
        object_list = eo._process_build_path(
            option_tree=mock_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath'])
        self.assertEqual(
            '{} Cooling Coil Outlet',
            object_list[-1]['Fan:VariableVolume']['air_inlet_node_name']
        )
        return

    def test_build_path_bad_build_path(self):
        # Note ObjectReference is not needed
        eo = ExpandSystem(template=mock_system_template)
        eo._get_option_tree_leaf = MagicMock()
        eo._get_option_tree_leaf.return_value = {}
        eo._apply_transitions = MagicMock()
        eo._apply_transitions.return_value = {}
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Build Path action is incorrectly formatted'):
            eo._process_build_path(
                option_tree={'Actions': [{'bad': 'structure'}]})
        return

    def test_build_path_action_never_applied(self):
        # Note ObjectReference is not needed
        eo = ExpandSystem(template=mock_system_template)
        eo._get_option_tree_leaf = MagicMock()
        eo._get_option_tree_leaf.return_value = {}
        eo._apply_transitions = MagicMock()
        eo._apply_transitions.return_value = {}
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'A build path action was not applied '
                                                                           'for template field'):
            eo._process_build_path(
                option_tree={
                    'Actions': [
                        {
                            'template_field': {
                                'Location': 'Coil:Cooling.*',
                                'ActionType': 'Insert',
                                'Objects': []
                            }
                        }
                    ]
                }
            )
        return

    def test_build_path_action_insert_by_location(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 1,
            'Occurrence': 1,
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        build_path = eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual('test_object_type', list(build_path[1].keys())[0])
        return

    def test_build_path_action_replace_by_location(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 1,
            'ActionType': 'Replace',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        build_path = eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual('test_object_type', list(build_path[1].keys())[0])
        self.assertEqual(2, len(build_path))
        return

    def test_build_path_action_remove_by_location(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 1,
            'ActionType': 'Remove'
        }
        eo = ExpandObjects()
        build_path = eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual('OutdoorAir:Mixer', list(build_path[0].keys())[0])
        self.assertEqual(1, len(build_path))
        return

    def test_build_path_action_insert_by_object_reference(self):
        build_path = mock_build_path
        action_instruction = {
            'ObjectReference': 'OutdoorAir:Mixer',
            'Location': 'After',
            'Occurrence': 1,
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        build_path = eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual('test_object_type', list(build_path[1].keys())[0])
        return

    def test_build_path_action_remove_by_object_reference(self):
        build_path = mock_build_path
        action_instruction = {
            'ObjectReference': 'OutdoorAir:Mixer',
            'ActionType': 'Remove'
        }
        eo = ExpandObjects()
        build_path = eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual('Fan:VariableVolume', list(build_path[0].keys())[0])
        return

    def test_build_path_action_replace_by_object_reference(self):
        build_path = mock_build_path
        action_instruction = {
            'ObjectReference': 'OutdoorAir:Mixer',
            'ActionType': 'Replace',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        build_path = eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        self.assertEqual('test_object_type', list(build_path[0].keys())[0])
        return

    def test_reject_build_path_action_with_bad_occurrence(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 1,
            'Occurrence': 'bad',
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'must be a non-negative integer'):
            eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        action_instruction = {
            'Location': 1,
            'Occurrence': -1,
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'must be a non-negative integer'):
            eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        return

    def test_reject_build_path_action_with_bad_action(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 1,
            'Occurrence': 0,
            'ActionType': 'bad',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Invalid action type'):
            eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        return

    def test_reject_build_path_action_with_bad_location_text(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'Location': 'bad',
            'Occurrence': 0,
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Build path action insert reference value '):
            eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        return

    def test_reject_build_path_action_with_occurrence_too_high(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'ObjectReference': 'OutdoorAir:Mixer',
            'Location': 'Before',
            'Occurrence': 10,
            'ActionType': 'Insert',
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'The number of occurrence matches'):
            eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        return

    def test_reject_build_path_action_missing_keys(self):
        build_path = mock_build_path
        # Note ObjectReference is not needed
        action_instruction = {
            'ObjectReference': 'OutdoorAir:Mixer',
            'Location': 'Before',
            'Occurrence': 10,
            'Objects': [
                {
                    "test_object_type": {
                        "Fields": {
                            'name': 'test_object_name',
                            'test_field': 'test_value',
                            'test_field_3': 'test_value_3'
                        },
                        "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                    }
                }
            ]
        }
        eo = ExpandObjects()
        with self.assertRaisesRegex(
                PyExpandObjectsYamlStructureException, 'Build Path action is missing required instructions'):
            eo._apply_build_path_action(build_path=build_path, action_instructions=action_instruction)
        return

    def test_insert_on_build_path_from_option_tree(self):
        test_system_option_tree = copy.deepcopy(mock_system_option_tree)
        test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath']['Actions'] = [
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 1,
                        'Occurrence': 1,
                        'ActionType': 'Insert',
                        'Objects': [
                            {
                                "test_object_type": {
                                    "Fields": {
                                        'name': 'test_object_name',
                                        'test_field': 'test_value',
                                        'test_field_3': 'test_value_3'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                                }
                            }
                        ]
                    }
                }
            },
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 2,
                        'Occurrence': 1,
                        'ActionType': 'Insert',
                        'Objects': [
                            {
                                "test_object_type_2": {
                                    "Fields": {
                                        'name': 'test_object_name_2',
                                        'test_field_2': 'test_value_2',
                                        'test_field_4': 'test_value_4'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field_2', "Outlet": "test_field_4"}}
                                }
                            }
                        ]
                    }
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_system_template,
            expansion_structure=test_system_option_tree)
        structure_hierarchy = ['OptionTree', 'HVACTemplate', 'System', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        build_path = eo._process_build_path(option_tree=option_tree['BuildPath'])
        self.assertEqual('test_object_type', list(build_path[1].keys())[0])
        self.assertEqual('test_object_type_2', list(build_path[2].keys())[0])
        return

    def test_insert_on_build_path_from_option_tree_with_none_value(self):
        test_system_option_tree = copy.deepcopy(mock_system_option_tree)
        test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath']['Actions'] = [
            {
                'non_present_template_field': {
                    'None': {
                        'Location': 1,
                        'Occurrence': 1,
                        'ActionType': 'Insert',
                        'Objects': [
                            {
                                "test_object_type": {
                                    "Fields": {
                                        'name': 'test_object_name',
                                        'test_field': 'test_value',
                                        'test_field_3': 'test_value_3'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                                }
                            }
                        ]
                    }
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_system_template,
            expansion_structure=test_system_option_tree)
        structure_hierarchy = ['OptionTree', 'HVACTemplate', 'System', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        build_path = eo._process_build_path(option_tree=option_tree['BuildPath'])
        self.assertEqual('test_object_type', list(build_path[1].keys())[0])
        return

    def test_replace_on_build_path_from_option_tree(self):
        test_system_option_tree = copy.deepcopy(mock_system_option_tree)
        test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath']['Actions'] = [
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 1,
                        'Occurrence': 1,
                        'ActionType': 'Insert',
                        'Objects': [
                            {
                                "test_object_type": {
                                    "Fields": {
                                        'name': 'test_object_name',
                                        'test_field': 'test_value',
                                        'test_field_3': 'test_value_3'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                                }
                            }
                        ]
                    }
                }
            },
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 2,
                        'Occurrence': 1,
                        'ActionType': 'Replace',
                        'Objects': [
                            {
                                "test_object_type_2": {
                                    "Fields": {
                                        'name': 'test_object_name_2',
                                        'test_field_2': 'test_value_2',
                                        'test_field_4': 'test_value_4'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field_2', "Outlet": "test_field_4"}}
                                }
                            }
                        ]
                    }
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_system_template,
            expansion_structure=test_system_option_tree)
        structure_hierarchy = ['OptionTree', 'HVACTemplate', 'System', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        build_path = eo._process_build_path(option_tree=option_tree['BuildPath'])
        self.assertEqual('test_object_type', list(build_path[1].keys())[0])
        self.assertEqual('test_object_type_2', list(build_path[2].keys())[0])
        return

    def test_remove_on_build_path_from_option_tree(self):
        test_system_option_tree = copy.deepcopy(mock_system_option_tree)
        test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath']['Actions'] = [
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 0,
                        'ActionType': 'Remove',
                    }
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_system_template,
            expansion_structure=test_system_option_tree)
        structure_hierarchy = ['OptionTree', 'HVACTemplate', 'System', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        build_path = eo._process_build_path(option_tree=option_tree['BuildPath'])
        self.assertEqual('Fan:VariableVolume', list(build_path[0].keys())[0])
        return

    def test_complex_actions_on_build_path_from_option_tree(self):
        test_system_option_tree = copy.deepcopy(mock_system_option_tree)
        test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath']['Actions'] = [
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 0,
                        'Occurrence': 1,
                        'ActionType': 'Replace',
                        'Objects': [
                            {
                                "test_object_type": {
                                    "Fields": {
                                        'name': 'test_object_name',
                                        'test_field': 'test_value',
                                        'test_field_3': 'test_value_3'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                                }
                            }
                        ]
                    }
                }
            },
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 0,
                        'ActionType': 'Remove',
                    }
                }
            }
        ]
        eo = ExpandObjects(
            template=mock_system_template,
            expansion_structure=test_system_option_tree)
        structure_hierarchy = ['OptionTree', 'HVACTemplate', 'System', 'VAV']
        option_tree = eo._get_option_tree(structure_hierarchy=structure_hierarchy)
        build_path = eo._process_build_path(option_tree=option_tree['BuildPath'])
        self.assertEqual(1, len(build_path))
        return

    def test_convert_build_path(self):
        build_path = [
            {
                "Object:1": {
                    "Fields": {
                        "field_1": "value_1",
                        "field_2": "value_2"
                    },
                    "Connectors": {
                        "AirLoop": {
                            "Inlet": "field_1",
                            "Outlet": "field_2"
                        }
                    }
                }
            },
            {
                "Object:2": {
                    "Fields": {
                        "field_3": "value_3",
                        "field_4": "value_4"
                    },
                    "Connectors": {
                        "AirLoop": {
                            "Inlet": "field_3",
                            "Outlet": "field_4"
                        }
                    }
                }
            }
        ]
        eo = ExpandObjects()
        output = eo._connect_and_convert_build_path_to_object_list(build_path=build_path)
        self.assertEqual("value_2", output[1]["Object:2"]["field_3"])
        return

    def test_convert_build_path_no_build_path(self):
        build_path = []
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsException, 'Build path was not provided nor was it'):
            eo._connect_and_convert_build_path_to_object_list(build_path=build_path)
        return

    def test_convert_build_path_no_connectors(self):
        build_path = [
            {
                "Object:1": {
                    "Fields": {
                        "field_1": "value_1",
                        "field_2": "value_2"
                    }
                }
            }
        ]
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Referenced super object is missing Connectors key'):
            eo._connect_and_convert_build_path_to_object_list(build_path=build_path)
        return

    def test_convert_build_path_bad_connectors(self):
        build_path = [
            {
                "Object:1": {
                    "Fields": {
                        "field_1": "value_1",
                        "field_2": "value_2"
                    },
                    "Connectors": {
                        "AirLoop": {
                            "Inlet": "field_4",
                            "Outlet": "field_5"
                        }
                    }
                }
            }
        ]
        eo = ExpandObjects()
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'There is a Field/Connector mismatch'):
            eo._connect_and_convert_build_path_to_object_list(build_path=build_path)
        return

    def test_retrieve_build_path_objects_from_option_tree(self):
        eo = ExpandSystem(template=mock_system_template)
        test_system_option_tree = copy.deepcopy(mock_system_option_tree)
        test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']['BuildPath']['Actions'] = [
            {
                'template_field': {
                    'template_test_value': {
                        'Location': 1,
                        'Occurrence': 1,
                        'ActionType': 'Insert',
                        'Objects': [
                            {
                                "test_object_type": {
                                    "Fields": {
                                        'name': 'test_object_name',
                                        'test_field': 'test_value',
                                        'test_field_3': 'test_value_3'
                                    },
                                    "Connectors": {'AirLoop': {"Inlet": 'test_field', "Outlet": "test_field_3"}}
                                }
                            }
                        ]
                    }
                }
            },
        ]
        eo._get_option_tree = MagicMock()
        eo._get_option_tree.return_value = test_system_option_tree['OptionTree']['HVACTemplate']['System']['VAV']
        output = eo._get_option_tree_objects(structure_hierarchy=['not', 'important', 'because', 'mocked'])
        self.assertEqual(
            eo.summarize_epjson(output),
            {'OutdoorAir:Mixer': 1, 'test_object_type': 1, 'Fan:VariableVolume': 1}
        )
        return

    def test_complex_inputs_build_path(self):
        es = ExpandSystem(template=mock_system_template)
        output = es._resolve_complex_input(
            epjson={},
            build_path=mock_build_path,
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": 1,
                    "ValueLocation": "Outlet"
                }
            }
        )
        self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        return

    def test_complex_inputs_build_path_reference_by_object_type(self):
        es = ExpandSystem(template=mock_system_template)
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
                            'Outlet': 'air_outlet_node_name'}}}}]
        es.unique_name = 'template_name'
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": 'OutdoorAir:M.*',
                    "ValueLocation": "Outlet"
                }
            }
        )
        self.assertEqual('template_name Mixed Air Outlet', [i for i in output][0]['value'])
        return

    def test_complex_inputs_build_path_reference_by_object_type_occurence(self):
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
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": 'Fan:.*',
                    'Occurrence': 2,
                    "ValueLocation": "Outlet"
                }
            }
        )
        self.assertEqual(
            'TEST SYSTEM Supply Fan Outlet',
            [o for o in output][0]['value']
        )
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": 'Fan:.*',
                    'Occurrence': -1,
                    "ValueLocation": "Outlet"
                }
            }
        )
        self.assertEqual(
            'TEST SYSTEM Supply Fan Outlet',
            [o for o in output][0]['value']
        )
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": 'Fan:.*',
                    'Occurrence': 1,
                    "ValueLocation": "Outlet"
                }
            }
        )
        self.assertEqual(
            'TEST SYSTEM Return Fan Outlet',
            [o for o in output][0]['value']
        )
        return

    def test_reject_complex_inputs_build_path_reference_by_object_type_bad_occurence(self):
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
                            'Outlet': 'air_outlet_node_name'}}}}]
        es.unique_name = 'TEST SYSTEM'
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": 'Fan:.*',
                    'Occurrence': 'BadValue',
                    "ValueLocation": "Outlet"
                }
            }
        )
        with self.assertRaises(PyExpandObjectsYamlStructureException):
            print([o for o in output])
        return

    def test_complex_inputs_build_path_class_attribute(self):
        es = ExpandSystem(template=mock_system_template)
        es.expansion_structure = copy.deepcopy(mock_system_option_tree)
        es._create_objects()
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": -1,
                    "ValueLocation": "Outlet"
                }
            }
        )
        self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        return

    def test_complex_inputs_build_path_class_attribute_get_object(self):
        es = ExpandSystem(template=mock_system_template)
        es.expansion_structure = copy.deepcopy(mock_system_option_tree)
        es._create_objects()
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": -1,
                    "ValueLocation": "self"
                }
            }
        )
        self.assertEqual('Fan:VariableVolume', [i for i in output][0]['value'])
        return

    def test_complex_inputs_build_path_class_attribute_get_name(self):
        es = ExpandSystem(template=mock_system_template)
        es.expansion_structure = copy.deepcopy(mock_system_option_tree)
        es._create_objects()
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": -1,
                    "ValueLocation": "key"
                }
            }
        )
        self.assertEqual('template_name Supply Fan', [i for i in output][0]['value'])
        return

    def test_separate_objects_build_path(self):
        eo = ExpandObjects()
        eo.epjson = {}
        object_list = [
            {
                "Object:Type1": {
                    "Fields": {
                        "name": "ObjectName1",
                        "field_1": "val_1",
                        "field_2": "val_2"
                    },
                    "Connectors": {
                        "AirLooop": {
                            "Inlet": "field_1",
                            "Outlet": "field_2"
                        }
                    }
                }
            },
            {
                "Object:Type2": {
                    "name": "ObjectName2",
                    "field_3": "val_3",
                    "field_4": "val_4"
                }
            }
        ]
        output = eo._parse_build_path(object_list=object_list)
        self.assertEqual('Object:Type1', list(output[0].keys())[0])
        self.assertEqual('val_3', eo.epjson['Object:Type2']['ObjectName2']['field_3'])
        return

    def test_reject_complex_inputs_build_path_reference_no_build_path(self):
        es = ExpandSystem(template=mock_system_template)
        es.build_path = {}
        output = es._resolve_complex_input(
            epjson={},
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": -1,
                    "ValueLocation": "Outlet"
                }
            }
        )
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Build Path complex input was specified'):
            self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        return

    def test_reject_complex_inputs_build_path_missing_inputs(self):
        es = ExpandSystem(template=mock_system_template)
        output = es._resolve_complex_input(
            epjson={},
            build_path=mock_build_path,
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "ValueLocation": "Outlet"
                }
            }
        )
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Object field could not be'):
            self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        output = es._resolve_complex_input(
            epjson={},
            build_path=mock_build_path,
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "Location": -1
                }
            }
        )
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Object field could not be'):
            self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        return

    def test_reject_complex_inputs_build_path_bad_location(self):
        es = ExpandSystem(template=mock_system_template)
        output = es._resolve_complex_input(
            epjson={},
            build_path=mock_build_path,
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "ValueLocation": "Outlet",
                    "Location": 10
                }
            }
        )
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Object field could not be'):
            self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        return

    def test_reject_complex_inputs_build_path_bad_value(self):
        es = ExpandSystem(template=mock_system_template)
        output = es._resolve_complex_input(
            epjson={},
            build_path=mock_build_path,
            field_name="field_1",
            input_value={
                "BuildPathReference": {
                    "ValueLocation": "Bad_value",
                    "Location": -1
                }
            }
        )
        with self.assertRaisesRegex(PyExpandObjectsYamlStructureException, 'Object field could not be'):
            self.assertEqual('template_name Supply Fan Outlet', [i for i in output][0]['value'])
        return

    def test_complex_inputs_compact_schedule(self):
        eo = ExpandObjects()
        output = eo.resolve_objects(epjson={
            'Schedule:Compact': {
                "HVACTemplate-Always12.8": {
                    'structure': 'Objects:Common:Objects:Schedule:Compact:ALWAYS_VAL',
                    'insert_values': [12.8, ]
                }
            }
        })
        self.assertEqual(
            12.8,
            output['Schedule:Compact']['HVACTemplate-Always12.8']['data'][-1]['field'])
        return

    def test_complex_inputs_compact_schedule_full(self):
        eo = ExpandObjects()
        output = eo.resolve_objects(epjson={
            'Schedule:Compact': {
                "HVACTemplate-Always12.8": {
                    'schedule_type_limits_name': 'Any Number',
                    'data': [
                        {'field': 'Through 12/31'},
                        {'field': 'For AllDays'},
                        {'field': 'Until 24:00'},
                        {'field': 12.8}
                    ]
                }
            }
        })
        self.assertEqual(
            12.8,
            output['Schedule:Compact']['HVACTemplate-Always12.8']['data'][-1]['field'])
        return

    def test_complex_inputs_create_schedule_from_transition(self):
        es = ExpandSystem(template={
            'HVACTemplate:System:VAV': {
                'template_name': {
                    'template_field': 'template_test_value',
                    'cooling_coil_design_setpoint': 12.8
                }
            }
        })
        es.expansion_structure = {
            'Objects': {
                'Common': {
                    'Objects': {
                        'Schedule': {
                            'Compact': {
                                'ALWAYS_VAL': {
                                    'name': 'HVACTemplate-Always{}',
                                    'schedule_type_limits_name': 'Any Number',
                                    'data': [
                                        {'field': 'Through 12/31'},
                                        {'field': 'For AllDays'},
                                        {'field': 'Until 24:00'},
                                        {'field': '{}'}
                                    ]
                                }
                            }
                        }
                    }
                }
            },
            'OptionTree': {
                'HVACTemplate': {
                    'System': {
                        'VAV': {
                            'BaseObjects': {
                                'Objects': [
                                    {
                                        'SetpointManager:Scheduled': {
                                            'name': '{} Cooling Supply Air Temp Manager',
                                            'control_variable': 'Temperature'
                                        }
                                    }
                                ],
                                'Transitions': [
                                    {
                                        "SetpointManager:Scheduled": {
                                            "cooling_coil_design_setpoint": {
                                                'schedule_name': 'HVACTemplate-Always{cooling_coil_design_setpoint}'
                                            }
                                        }
                                    }
                                ]
                            }
                        }
                    }
                }
            }
        }
        es._create_objects()
        es.epjson = es.resolve_objects(epjson=es.epjson)
        self.assertEqual(
            12.8,
            es.epjson['Schedule:Compact']['HVACTemplate-Always12.8']['data'][-1]['field'])
        return

    def test_zonehvac_equipmentlist(self):
        ez = ExpandZone(template={
            'HVACTemplate:Zone:FanCoil': {
                'zone_template_name': {
                    "cooling_coil_type": "ChilledWater",
                    "heating_coil_type": "HotWater",
                    'zone_name': 'test_zone'}}})
        ez._create_objects()
        self.assertEqual(
            'test_zone Fan Coil',
            ez.epjson['ZoneHVAC:EquipmentList']['test_zone Equipment']['equipment'][0]['zone_equipment_name'])
        return

    def test_zonehvac_equipmentlist_baseboard(self):
        ez = ExpandZone(template={
            'HVACTemplate:Zone:FanCoil': {
                'zone_template_name': {
                    "cooling_coil_type": "ChilledWater",
                    "heating_coil_type": "HotWater",
                    'zone_name': 'test_zone',
                    'baseboard_heating_type': 'HotWater'}}})
        ez._create_objects()
        self.assertEqual(
            'test_zone Baseboard Heat',
            ez.epjson['ZoneHVAC:EquipmentList']['test_zone Equipment']['equipment'][1]['zone_equipment_name'])
        return

    def test_zonehvac_equipmentlist_doas(self):
        ez = ExpandZone(template={
            'HVACTemplate:Zone:FanCoil': {
                'zone_template_name': {
                    "cooling_coil_type": "ChilledWater",
                    "heating_coil_type": "HotWater",
                    'zone_name': 'test_zone',
                    'dedicated_outdoor_air_system_name': 'DOAS'}}})
        ez._create_objects()
        self.assertEqual(
            'test_zone DOAS ATU',
            ez.epjson['ZoneHVAC:EquipmentList']['test_zone Equipment']['equipment'][0]['zone_equipment_name'])
        return

    def test_zonehvac_equipmentlist_doas_baseboard(self):
        ez = ExpandZone(template={
            'HVACTemplate:Zone:FanCoil': {
                'zone_template_name': {
                    "cooling_coil_type": "ChilledWater",
                    "heating_coil_type": "HotWater",
                    'zone_name': 'test_zone',
                    'baseboard_heating_type': 'HotWater',
                    'dedicated_outdoor_air_system_name': 'DOAS'}}})
        ez._create_objects()
        self.assertEqual(
            'test_zone DOAS ATU',
            ez.epjson['ZoneHVAC:EquipmentList']['test_zone Equipment']['equipment'][0]['zone_equipment_name'])
        self.assertEqual(
            'test_zone Baseboard Heat',
            ez.epjson['ZoneHVAC:EquipmentList']['test_zone Equipment']['equipment'][-1]['zone_equipment_name'])
        return
