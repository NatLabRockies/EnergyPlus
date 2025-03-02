from pathlib import Path
import unittest
import tempfile

from . import BaseTest
from src.epjson_handler import EPJSON
# must import exceptions directly from test code
from src.epjson_handler import UniqueNameException, PyExpandObjectsTypeError, \
    PyExpandObjectsFileNotFoundError, PyExpandObjectsSchemaError, InvalidEpJSONException


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

test_dir = Path(__file__).parent


class TestEPJSONHandler(BaseTest, unittest.TestCase):
    def setUp(self):
        self.epjson_handler = EPJSON()
        self.epjson_handler_no_schema = EPJSON(no_schema=True)
        self.epjson_handler.logger.setLevel('ERROR')
        return

    def test_merge_bad_objects(self):
        dict_1 = {
            "Zone": {
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 239.247360229,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        dict_2 = {"Zone": ""}
        with self.assertRaises(PyExpandObjectsTypeError):
            self.epjson_handler.merge_epjson(
                super_dictionary=dict_1,
                object_dictionary=dict_2)
        return

    def test_merge_same_object_type(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        dict_2 = {
            "Zone": {
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 239.247360229,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        self.epjson_handler.merge_epjson(
            super_dictionary=dict_1,
            object_dictionary=dict_2,
        )
        self.assertIn('SPACE1-1', dict_1['Zone'].keys())
        self.assertIn('SPACE2-1', dict_1['Zone'].keys())
        return

    def test_merge_two_objects(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        dict_2 = {
            "ZoneInfiltration:DesignFlowRate": {
                "SPACE1-1 Infil 1": {
                    "constant_term_coefficient": 0,
                    "design_flow_rate": 0.0167,
                    "design_flow_rate_calculation_method": "Flow/Zone",
                    "schedule_name": "INFIL-SCH",
                    "temperature_term_coefficient": 0,
                    "velocity_squared_term_coefficient": 0,
                    "velocity_term_coefficient": 0.2237,
                    "zone_or_zonelist_name": "SPACE1-1"
                }
            }
        }
        self.epjson_handler.merge_epjson(
            super_dictionary=dict_1,
            object_dictionary=dict_2,
        )
        self.assertIn('Zone', dict_1.keys())
        self.assertGreater(len(dict_1['Zone']['SPACE1-1'].keys()), 0)
        self.assertIn('ZoneInfiltration:DesignFlowRate', dict_1.keys())
        self.assertGreater(len(dict_1['ZoneInfiltration:DesignFlowRate']['SPACE1-1 Infil 1'].keys()), 0)
        return

    def test_merge_duplicate_name(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        dict_2 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        with self.assertRaises(UniqueNameException):
            self.epjson_handler.merge_epjson(
                super_dictionary=dict_1,
                object_dictionary=dict_2,
                unique_name_override=False)
        return

    def test_merge_duplicate_name_skip_object(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        dict_2 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        }
        self.epjson_handler.merge_epjson(
            super_dictionary=dict_1,
            object_dictionary=dict_2,
            unique_name_override=False,
            unique_name_fail=False)
        self.assertEqual(1, len(dict_1["Zone"].keys()))
        return

    def test_merge_duplicate_name_skip_schedule_compact_always(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            },
            'Schedule:Compact': {
                "HVACTemplate-AlwaysTEST": {
                    'field_1': 'val_1'
                }
            }
        }
        dict_2 = {
            "Zone": {
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            },
            'Schedule:Compact': {
                "HVACTemplate-AlwaysTEST": {
                    'field_2': 'val_2'
                }
            }
        }
        self.epjson_handler.merge_epjson(
            super_dictionary=dict_1,
            object_dictionary=dict_2,
            unique_name_override=False)
        self.assertEqual('val_1', dict_1['Schedule:Compact']['HVACTemplate-AlwaysTEST']['field_1'])
        return

    def test_unpack_epjson(self):
        outputs = self.epjson_handler.epjson_genexp({
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                },
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            }
        })
        key_check = True
        for output in outputs:
            (object_type, object_structure), = output.items()
            (name, _), = object_structure.items()
            if name not in ['SPACE1-1', 'SPACE2-1']:
                key_check = False
        self.assertTrue(key_check)
        return

    def test_purge_epjson(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                },
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            },
            "ThermostatSetpoint:DualSetpoint": {
                "All Zones Dual SP Control": {
                    "cooling_setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                    "heating_setpoint_temperature_schedule_name": "Htg-SetP-Sch"
                }
            }
        }
        output = self.epjson_handler.purge_epjson(
            epjson=dict_1,
            purge_dictionary={
                "Zone": ["SPACE1-1", ]
            }
        )
        self.assertEqual(1, len(output['Zone'].keys()))
        self.assertTrue("All Zones Dual SP Control" == list(output['ThermostatSetpoint:DualSetpoint'].keys())[0])
        output = self.epjson_handler.purge_epjson(
            epjson=dict_1,
            purge_dictionary={
                "Zone": '.*'
            }
        )
        self.assertTrue("All Zones Dual SP Control" == list(output['ThermostatSetpoint:DualSetpoint'].keys())[0])
        with self.assertRaises(KeyError):
            output['Zone']
        return

    def test_epjson_count_summary(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                },
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            },
            "ThermostatSetpoint:DualSetpoint": {
                "All Zones Dual SP Control": {
                    "cooling_setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                    "heating_setpoint_temperature_schedule_name": "Htg-SetP-Sch"
                }
            }
        }
        output = self.epjson_handler.summarize_epjson(dict_1)
        self.assertEqual(2, output['Zone'])
        self.assertEqual(1, output['ThermostatSetpoint:DualSetpoint'])
        return

    def test_default_schema_is_valid(self):
        self.epjson_handler._load_schema()
        assert self.epjson_handler.schema_is_valid
        return

    def test_blank_schema_is_not_valid(self):
        with self.assertRaises(PyExpandObjectsSchemaError):
            self.epjson_handler._validate_schema({"properties": {"id": "asdf"}})
        return

    def test_good_object_is_valid(self):
        self.epjson_handler.epjson_process(
            epjson_ref={
                **minimum_objects_d,
                "Version": {
                    "Version 1": {
                        "version_identifier": "9.4"
                    }
                }
            }
        )
        self.assertTrue(self.epjson_handler.input_epjson_is_valid)
        return

    def test_good_file_is_verified(self):
        self.epjson_handler.epjson_process(
            epjson_ref=str(
                test_dir / '..' / 'simulation' / 'ExampleFiles' / 'HVACTemplate-5ZoneVAVWaterCooledExpanded.epJSON')
        )
        self.assertTrue(self.epjson_handler.input_epjson_is_valid)
        return

    def test_no_schema_returns_json(self):
        self.epjson_handler_no_schema._load_epjson({
            **minimum_objects_d,
            "Version": {
                "Version 1": {
                    "version_identifier": "9.4"
                }
            }
        })
        self.assertIsNone(self.epjson_handler_no_schema.schema)
        self.assertIsNone(self.epjson_handler_no_schema.schema_is_valid)
        self.assertIs(True, self.epjson_handler_no_schema.input_epjson_is_valid)
        self.assertEqual(len(self.epjson_handler_no_schema.input_epjson.keys()), 3)
        return

    def test_bad_file_path_object_returns_error(self):
        with self.assertRaisesRegex(PyExpandObjectsFileNotFoundError, 'input is not a string'):
            self.epjson_handler._get_json_file(
                json_location={})
        return

    def test_bad_file_path_returns_error(self):
        with self.assertRaisesRegex(PyExpandObjectsFileNotFoundError, 'file does not exist'):
            self.epjson_handler._get_json_file(
                json_location='bad/file/path.epJSON')
        return

    def test_bad_file_returns_error(self):
        with self.assertRaisesRegex(PyExpandObjectsTypeError, 'not a valid json'):
            with tempfile.NamedTemporaryFile(suffix='.epJSON', mode='w') as temp_file:
                temp_file.write('bad file data')
                temp_file.seek(0)
                self.epjson_handler._get_json_file(
                    json_location=temp_file.name)
        return

    def test_get_epjson_objects_object_type(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                },
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            },
            "ThermostatSetpoint:DualSetpoint": {
                "All Zones Dual SP Control": {
                    "cooling_setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                    "heating_setpoint_temperature_schedule_name": "Htg-SetP-Sch"
                }
            }
        }
        output = self.epjson_handler.get_epjson_objects(epjson=dict_1, object_type_regexp='^Z.*')
        self.assertEqual({'Zone': 2}, self.epjson_handler.summarize_epjson(output))
        return

    def test_get_epjson_object_name_reference(self):
        dict_1 = {
            "Zone": {
                "SPACE1-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                },
                "SPACE2-1": {
                    "ceiling_height": 2.438400269,
                    "direction_of_relative_north": 0,
                    "multiplier": 1,
                    "type": 1,
                    "volume": 103.311355591,
                    "x_origin": 0,
                    "y_origin": 0,
                    "z_origin": 0
                }
            },
            "ThermostatSetpoint:DualSetpoint": {
                "All Zones Dual SP Control": {
                    "cooling_setpoint_temperature_schedule_name": "Clg-SetP-Sch",
                    "heating_setpoint_temperature_schedule_name": "Htg-SetP-Sch"
                }
            }
        }
        output = self.epjson_handler.get_epjson_objects(
            epjson=dict_1,
            object_type_regexp='^Z.*',
            object_name_regexp='^Space2.*')
        self.assertEqual({'Zone': 1}, self.epjson_handler.summarize_epjson(output))
        return

    def test_reject_get_epjson_object_bad_input(self):
        dict_1 = []
        with self.assertRaisesRegex(InvalidEpJSONException, 'Invalid epJSON'):
            self.epjson_handler.get_epjson_objects(
                epjson=dict_1,
                object_type_regexp='^Z.*',
                object_name_regexp='^Space2.*')
        return
