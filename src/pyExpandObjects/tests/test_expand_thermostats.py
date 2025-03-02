import unittest

from src.expand_objects import ExpandThermostat
from src.expand_objects import InvalidTemplateException
from . import BaseTest


class TestExpandThermostat(BaseTest, unittest.TestCase):
    def setUp(self):
        return

    def teardown(self):
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Input Template Required")
    def test_check_templates_are_required(self):
        with self.assertRaises(TypeError):
            ExpandThermostat()
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Verify valid template object")
    def test_verify_good_template(self):
        template = {
            'HVACTemplate:Thermostats': {
                "All Zones": {
                    "heating_setpoint_schedule_name": "Htg-SetP-Sch",
                    "cooling_setpoint_schedule_name": "Clg-SetP-Sch"
                }
            }
        }
        output = ExpandThermostat(template=template)
        self.assertEqual('All Zones', output.template_name)
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Create schedule from constant setpoint")
    def test_create_thermostat_schedule_from_constant_setpoint(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'constant_cooling_setpoint': 18,
                    'constant_heating_setpoint': 12
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        schedule_name_match = False
        schedule_names = [i for i in list(eo.epjson['Schedule:Compact'].keys())]
        schedule_match = [i for i in schedule_names if i == 'HVACTemplate-Always12']
        if any(schedule_match):
            schedule_name_match = True
        self.assertTrue(schedule_name_match)
        self.assertEqual(len(schedule_names), 2)
        self.assertEqual(
            eo.epjson['Schedule:Compact']['HVACTemplate-Always12']['data'][-1]['field'],
            12)
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Assign schedule from constant setpoint")
    def test_assign_thermostat_schedule_from_constant_setpoint(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'constant_heating_setpoint': 12
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        self.assertEqual('HVACTemplate-Always12', eo.heating_setpoint_schedule_name)
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Reject empty template")
    def test_reject_no_inputs(self):
        thermostat_template = {"HVACTemplate:Thermostat": {'Thermostat 1': {}}}
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        with self.assertRaisesRegex(InvalidTemplateException, 'No setpoints or schedules provided to object'):
            eo._create_thermostat_setpoints()
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:DualSetpoint from setpoints")
    def test_create_dual_thermostat_from_setpoints(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'constant_cooling_setpoint': 18,
                    'constant_heating_setpoint': 12
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        eo._create_thermostat_setpoints()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:DualSetpoint']['Thermostat 1 SP Control']
            ['cooling_setpoint_temperature_schedule_name'],
            'HVACTemplate-Always18')
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:DualSetpoint']['Thermostat 1 SP Control']
            ['heating_setpoint_temperature_schedule_name'],
            'HVACTemplate-Always12')
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:DualSetpoint from schedules")
    def test_create_dual_thermostat_from_schedules(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'cooling_setpoint_schedule_name': 'HVACTemplate-Always18',
                    'heating_setpoint_schedule_name': 'HVACTemplate-Always12'
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        eo._create_thermostat_setpoints()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:DualSetpoint']['Thermostat 1 SP Control']
            ['cooling_setpoint_temperature_schedule_name'],
            'HVACTemplate-Always18')
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:DualSetpoint']['Thermostat 1 SP Control']
            ['heating_setpoint_temperature_schedule_name'],
            'HVACTemplate-Always12')
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:SingleHeating from setpoint")
    def test_create_thermostat_single_heating_from_setpoint(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'constant_heating_setpoint': 12
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        eo._create_thermostat_setpoints()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:SingleHeating']['Thermostat 1 SP Control']
            ['setpoint_temperature_schedule_name'],
            'HVACTemplate-Always12')
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:SingleHeating from schedule")
    def test_create_thermostat_single_heating_from_schedule(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'heating_setpoint_schedule_name': 'HVACTemplate-Always12'
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        eo._create_thermostat_setpoints()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:SingleHeating']['Thermostat 1 SP Control']
            ['setpoint_temperature_schedule_name'],
            'HVACTemplate-Always12')
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:SingleCooling from setpoint")
    def test_create_thermostat_single_cooling_from_setpoint(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'constant_cooling_setpoint': 18
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        eo._create_thermostat_setpoints()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:SingleCooling']['Thermostat 1 SP Control']
            ['setpoint_temperature_schedule_name'],
            'HVACTemplate-Always18')
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:SingleCooling from schedule")
    def test_create_thermostat_single_cooling_from_schedule(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'cooling_setpoint_schedule_name': 'HVACTemplate-Always18'
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo._create_and_set_schedules()
        eo._create_thermostat_setpoints()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:SingleCooling']['Thermostat 1 SP Control']
            ['setpoint_temperature_schedule_name'],
            'HVACTemplate-Always18')
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Options:Object summary test")
    def test_check_output_objects(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'cooling_setpoint_schedule_name': 'HVACTemplate-Always18',
                    'heating_setpoint_schedule_name': 'HVACTemplate-Always12'
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo.run()
        output = eo.summarize_epjson(eo.epjson)
        expected_summary = {'ThermostatSetpoint:DualSetpoint': 1}
        self.assertDictEqual(expected_summary, output)
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Thermostat:Processing test")
    def test_processing(self):
        thermostat_template = {
            'HVACTemplate:Thermostats': {
                'Thermostat 1': {
                    'cooling_setpoint_schedule_name': 'HVACTemplate-Always18',
                    'heating_setpoint_schedule_name': 'HVACTemplate-Always12'
                }
            }
        }
        eo = ExpandThermostat(template=thermostat_template)
        eo.run()
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:DualSetpoint']['Thermostat 1 SP Control']
            ['cooling_setpoint_temperature_schedule_name'],
            'HVACTemplate-Always18')
        self.assertEqual(
            eo.epjson['ThermostatSetpoint:DualSetpoint']['Thermostat 1 SP Control']
            ['heating_setpoint_temperature_schedule_name'],
            'HVACTemplate-Always12')
        return
