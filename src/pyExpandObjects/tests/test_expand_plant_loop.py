import unittest

from src.expand_objects import ExpandPlantLoop
from . import BaseTest

mock_chw_plant_loop_template = {
    "HVACTemplate:Plant:ChilledWaterLoop": {
        "Chilled Water Loop": {
            "chilled_water_design_setpoint": 7.22,
            "chilled_water_pump_configuration": "ConstantPrimaryNoSecondary",
            "chilled_water_reset_outdoor_dry_bulb_high": 26.7,
            "chilled_water_reset_outdoor_dry_bulb_low": 15.6,
            "chilled_water_setpoint_at_outdoor_dry_bulb_high": 6.7,
            "chilled_water_setpoint_at_outdoor_dry_bulb_low": 12.2,
            "chilled_water_setpoint_reset_type": "None",
            "chiller_plant_operation_scheme_type": "Default",
            "condenser_plant_operation_scheme_type": "Default",
            "condenser_water_design_setpoint": 29.4,
            "condenser_water_pump_rated_head": 179352,
            "minimum_outdoor_dry_bulb_temperature": 7.22,
            "primary_chilled_water_pump_rated_head": 179352,
            "pump_control_type": "Intermittent",
            "secondary_chilled_water_pump_rated_head": 179352
        }
    }
}


class TestExpandPlantLoopObjects(BaseTest, unittest.TestCase):
    """
    General processing of ExpandPlantLoop operations
    """
    def setUp(self):
        return

    def teardown(self):
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:PlantLoop:Input Template Required")
    def test_check_templates_are_required(self):
        with self.assertRaises(TypeError):
            ExpandPlantLoop()
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:PlantLoop:Verify valid template object")
    def test_verify_good_template(self):
        output = ExpandPlantLoop(template=mock_chw_plant_loop_template)
        self.assertEqual('Chilled Water Loop', output.template_name)
        return
