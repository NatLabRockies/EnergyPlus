import unittest
from pathlib import Path
import os
import re
from argparse import Namespace
import tempfile
import json

from . import BaseTest
from src.main import main

test_dir = Path(__file__).parent

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


class TestMain(BaseTest, unittest.TestCase):
    """
    Test behaviour of calling main.py
    """
    def setUp(self):
        return

    def tearDown(self):
        return

    def test_no_schema_main(self):
        output = {}
        exception_raised = False
        try:
            with tempfile.TemporaryDirectory() as output_directory:
                output = main(
                    Namespace(
                        no_schema=True,
                        file=str(
                            test_dir / '..' / 'simulation' / 'ExampleFiles' / 'HVACTemplate-5ZoneVAVWaterCooled.epJSON'
                        ),
                        output_directory=output_directory
                    )
                )
        except Exception as e:
            self.assertEqual(e, e)
            exception_raised = True
        self.assertFalse(exception_raised)
        self.assertIn('Output:PreprocessorMessage', output.keys())
        return

    def test_output_message_contains_class_keys(self):
        output = {}
        exception_raised = False
        try:
            with tempfile.TemporaryDirectory() as output_directory:
                output = main(
                    Namespace(
                        no_schema=True,
                        file=str(
                            test_dir / '..' / 'simulation' / 'ExampleFiles' / 'HVACTemplate-5ZoneVAVWaterCooled.epJSON'
                        ),
                        output_directory=output_directory,
                        logger_level='INFO'
                    )
                )
        except Exception as e:
            self.assertEqual(e, e)
            exception_raised = True
        self.assertFalse(exception_raised)
        self.assertIn('Output:PreprocessorMessage', output.keys())
        msg_rgx = re.match('.*##### HVACTemplate #####.*', output['Output:PreprocessorMessage'].replace('\n', ' '))
        msg_status = False
        if msg_rgx:
            msg_status = True
        self.assertTrue(msg_status)
        return

    def test_bad_file_path_returns_message(self):
        with tempfile.TemporaryDirectory() as output_directory:
            output = main(
                Namespace(
                    no_schema=True,
                    file='bad_path.epJSON',
                    output_directory=output_directory
                )
            )
        msg_rgx = re.match('.*File does not exist.*', output['Output:PreprocessorMessage'].replace('\n', ' '))
        msg_status = False
        if msg_rgx:
            msg_status = True
        self.assertTrue(msg_status)
        return

    def test_bad_file_extension_returns_message(self):
        output = main(
            Namespace(
                no_schema=True,
                file='bad_extension.epJSON_bad'
            )
        )
        msg_rgx = re.match('.*Bad file extension.*', output['Output:PreprocessorMessage'].replace('\n', ' '))
        msg_status = False
        if msg_rgx:
            msg_status = True
        self.assertTrue(msg_status)
        return

    def test_write_output(self):
        with tempfile.TemporaryDirectory() as output_directory:
            with tempfile.NamedTemporaryFile(suffix='.epJSON', mode='w', dir=output_directory) as temp_file:
                json.dump(
                    {
                        **minimum_objects_d,
                        "HVACTemplate:Thermostat": {
                            "All Zones Dual": {
                                "cooling_setpoint_schedule_name": "Clg-SetP-Sch",
                                "heating_setpoint_schedule_name": "Htg-SetP-Sch"
                            }
                        }
                    },
                    temp_file)
                temp_file.seek(0)
                output = main(
                    Namespace(
                        file=temp_file.name,
                        no_schema=True,
                        output_directory=output_directory,
                        no_backup=False
                    )
                )
                self.assertGreater(
                    os.stat(os.path.join(output_directory, output['output_files']['expanded'])).st_size,
                    100)
                self.assertGreater(
                    os.stat(os.path.join(output_directory, output['output_files']['hvac_templates'])).st_size,
                    100)
                self.assertGreater(
                    os.stat(os.path.join(output_directory, output['output_files']['base'])).st_size,
                    100)
        return

    def test_write_output_no_directory_specified(self):
        with tempfile.NamedTemporaryFile(suffix='.epJSON', mode='w') as temp_file:
            json.dump(
                {
                    **minimum_objects_d,
                    "HVACTemplate:Thermostat": {
                        "All Zones Dual": {
                            "cooling_setpoint_schedule_name": "Clg-SetP-Sch",
                            "heating_setpoint_schedule_name": "Htg-SetP-Sch"
                        }
                    }
                },
                temp_file)
            temp_file.seek(0)
            output = main(
                Namespace(
                    file=temp_file.name,
                    no_schema=True,
                    no_backup=False
                )
            )
            self.assertTrue(output['output_files']['expanded'].startswith(os.path.dirname(temp_file.name)))
        return
