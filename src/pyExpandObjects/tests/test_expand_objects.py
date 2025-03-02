import unittest
import os
import re

from src.expand_objects import ExpandObjects
from src.expand_objects import InvalidTemplateException, PyExpandObjectsTypeError
from . import BaseTest

mock_template = {
    'HVACTemplate:Thermostat': {
        'template_name': {
            'field': 'value'
        }
    }
}

mock_zone_template = {
    "HVACTemplate:Zone:VAV": {
        "HVACTemplate:Zone:VAV 1": {
            "baseboard_heating_capacity": "Autosize",
            "baseboard_heating_type": "None",
            "constant_minimum_air_flow_fraction": 0.3,
            "damper_heating_action": "Reverse",
            "outdoor_air_flow_rate_per_person": 0.00944,
            "outdoor_air_flow_rate_per_zone": 0.0,
            "outdoor_air_flow_rate_per_zone_floor_area": 0.0,
            "outdoor_air_method": "Flow/Person",
            "reheat_coil_type": "HotWater",
            "supply_air_maximum_flow_rate": "Autosize",
            "template_thermostat_name": "All Zones",
            "template_vav_system_name": "VAV Sys 1",
            "zone_cooling_design_supply_air_temperature_input_method": "SystemSupplyAirTemperature",
            "zone_heating_design_supply_air_temperature": 50.0,
            "zone_heating_design_supply_air_temperature_input_method": "SupplyAirTemperature",
            "zone_minimum_air_flow_input_method": "Constant",
            "zone_name": "SPACE1-1"
        }
    }
}


class TestExpandObjects(BaseTest, unittest.TestCase):
    """
    General processing of ExpandObjects operations
    """
    def setUp(self):
        return

    def teardown(self):
        return

    def test_reject_bad_expansion_file_path(self):
        expansion_file_location = 'does/not/exist.yaml'
        with self.assertRaises(FileNotFoundError):
            ExpandObjects(template=mock_template, expansion_structure=expansion_file_location)
        return

    def test_reject_bad_expansion_file_format(self):
        expansion_file_location = os.path.abspath(__file__)
        with self.assertRaises(TypeError):
            ExpandObjects(template=mock_template, expansion_structure=expansion_file_location)
        return

    def test_expansion_dictionary_okay(self):
        expansion_dictionary = {'test': 'val'}
        expand_object = ExpandObjects(
            template=mock_template,
            expansion_structure=expansion_dictionary)
        self.assertEqual('val', expand_object.expansion_structure['test'])
        return

    def test_bad_expansion_dictionary_rejected(self):
        expansion_dictionary = []
        with self.assertRaises(TypeError):
            ExpandObjects(template=mock_template, expansion_structure=expansion_dictionary)
        return

    def test_retrieve_structure(self):
        structure_hierarchy = ['Objects', 'Common', 'Objects', 'Schedule', 'Compact', 'ALWAYS_VAL']
        eo = ExpandObjects(template=mock_template)
        structure = eo.get_structure(structure_hierarchy=structure_hierarchy)
        name_match_rgx = re.compile(r'^HVACTemplate.*')
        name_match = False
        if re.match(name_match_rgx, structure['name']):
            name_match = True
        self.assertTrue(name_match)
        return

    def test_exception_with_bad_format(self):
        structure_hierarchy = {}
        with self.assertRaisesRegex(TypeError, 'Structure hierarchy input'):
            eo = ExpandObjects(template=mock_template)
            eo.get_structure(structure_hierarchy=structure_hierarchy)
        return

    def test_exception_with_bad_structure(self):
        structure_hierarchy = ['Schedule', 'Compact', 'Bad']
        with self.assertRaisesRegex(TypeError, 'YAML structure does not exist'):
            eo = ExpandObjects(template=mock_template)
            eo.get_structure(structure_hierarchy=structure_hierarchy)
        return

    def test_reject_bad_template(self):
        templates = {"bad": "input"}
        with self.assertRaisesRegex(InvalidTemplateException, 'An invalid object'):
            ExpandObjects(template=templates)
        templates = ["bad_input"]
        with self.assertRaisesRegex(PyExpandObjectsTypeError, 'Template must be a dictionary'):
            ExpandObjects(template=templates)
        return

    def test_reject_bad_template_format(self):
        bad_template = {
            'HVACTemplate:Zone:VAV': {
                'template_name': []
            }
        }
        with self.assertRaisesRegex(InvalidTemplateException, 'An invalid object'):
            ExpandObjects(
                template=bad_template,
                expansion_structure={})
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Create always value schedule")
    def test_make_compact_schedule_always_val(self):
        structure_hierarchy = ['Objects', 'Common', 'Objects', 'Schedule', 'Compact', 'ALWAYS_VAL']
        eo = ExpandObjects(template=mock_template)
        schedule = eo.build_compact_schedule(structure_hierarchy=structure_hierarchy, insert_values=[3, ])
        (schedule_name, schedule_fields), = schedule['Schedule:Compact'].items()
        name_match_rgx = re.compile(r'^HVACTemplate-Always3')
        name_match = False
        if re.match(name_match_rgx, schedule_name):
            name_match = True
        self.assertTrue(name_match)
        set_value = schedule_fields['data'][-1]['field']
        self.assertEqual(3, set_value)
        return
