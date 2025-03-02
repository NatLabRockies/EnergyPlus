import unittest
from unittest.mock import MagicMock, PropertyMock

from src.expand_objects import ExpandZone
from src.hvac_template import HVACTemplate
from src.expand_objects import InvalidTemplateException
from . import BaseTest

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


class TestExpandZone(BaseTest, unittest.TestCase):
    def setUp(self):
        self.hvac_template = HVACTemplate()
        self.hvac_template.logger.setLevel('INFO')
        self.hvac_template._load_schema()
        return

    def teardown(self):
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Zone:Input Template Required")
    def test_check_templates_are_required(self):
        with self.assertRaises(TypeError):
            ExpandZone()
        return

    def test_zone_name_provided(self):
        with self.assertRaises(InvalidTemplateException):
            ExpandZone(template={})
        return

    def test_zone_name_not_null(self):
        with self.assertRaises(InvalidTemplateException):
            ExpandZone(
                template={
                    "HVACTemplate:Zone:VAV": {
                        "HVACTemplate:Zone:VAV 1": {
                            "zone_name": None
                        }
                    }
                }
            )
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Zone:Verify valid template object")
    def test_verify_good_template(self):
        output = ExpandZone(template=mock_zone_template)
        self.assertEqual('HVACTemplate:Zone:VAV 1', output.template_name)
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:Zone:Test all components")
    def test_processing(self):
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
        ez = ExpandZone(template=mock_zone_template)
        zone_output = ez._create_objects()
        zone_connection_output = self.hvac_template._create_zonecontrol_thermostat(zone_class_object=ez)
        ez.merge_epjson(
            super_dictionary=zone_output,
            object_dictionary=zone_connection_output
        )
        summarized_output = ez.summarize_epjson(zone_output)
        expected_summary = {
            'ZoneHVAC:AirDistributionUnit': 1,
            'ZoneHVAC:EquipmentList': 1,
            'ZoneHVAC:EquipmentConnections': 1,
            'DesignSpecification:OutdoorAir': 1,
            'DesignSpecification:ZoneAirDistribution': 1,
            'Sizing:Zone': 1,
            'AirTerminal:SingleDuct:VAV:Reheat': 1,
            'Coil:Heating:Water': 1,
            'Branch': 1,
            'Schedule:Compact': 1,
            'ZoneControl:Thermostat': 1}
        self.assertDictEqual(expected_summary, summarized_output)
        return
