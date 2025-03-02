import copy
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

mock_hw_plant_loop_template = {
    "HVACTemplate:Plant:HotWaterLoop": {
        "Hot Water Loop": {
            "hot_water_design_setpoint": 82,
            "hot_water_plant_operation_scheme_type": "Default",
            "hot_water_pump_configuration": "ConstantFlow",
            "hot_water_pump_rated_head": 179352,
            "hot_water_reset_outdoor_dry_bulb_high": 10,
            "hot_water_reset_outdoor_dry_bulb_low": -6.7,
            "hot_water_setpoint_at_outdoor_dry_bulb_high": 65.6,
            "hot_water_setpoint_at_outdoor_dry_bulb_low": 82.2,
            "hot_water_setpoint_reset_type": "OutdoorAirTemperatureReset",
            "pump_control_type": "Intermittent"
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

    def test_verify_chilled_water_objects(self):
        ep = ExpandPlantLoop(template=mock_chw_plant_loop_template)
        output = ep.run()
        summarized_output = {
            'AvailabilityManager:LowTemperatureTurnOff': 1,
            'AvailabilityManagerAssignmentList': 1,
            'Branch': 6,
            'OutdoorAir:Node': 1,
            'Pipe:Adiabatic': 5,
            'PlantEquipmentOperation:CoolingLoad': 1,
            'PlantEquipmentOperationSchemes': 1,
            'PlantLoop': 1,
            'Pump:ConstantSpeed': 1,
            'Schedule:Compact': 2,
            'SetpointManager:Scheduled': 1,
            'Sizing:Plant': 1,
        }
        self.assertEqual(summarized_output, ep.summarize_epjson(output.epjson))
        return

    def test_verify_hot_water_constant_primary_objects(self):
        ep = ExpandPlantLoop(template=mock_hw_plant_loop_template)
        output = ep.run()
        self.assertEqual(
            {
                'AvailabilityManagerAssignmentList': 1,
                'Branch': 6,
                'Pipe:Adiabatic': 5,
                'PlantEquipmentOperation:HeatingLoad': 1,
                'PlantEquipmentOperationSchemes': 1,
                'PlantLoop': 1,
                'Pump:ConstantSpeed': 1,
                'Schedule:Compact': 1,
                'SetpointManager:OutdoorAirReset': 1,
                'Sizing:Plant': 1
            },
            ep.summarize_epjson(output.epjson)
        )
        return

    def test_verify_hot_water_variable_primary_objects(self):
        tmp_mock = copy.deepcopy(mock_hw_plant_loop_template)
        tmp_mock['HVACTemplate:Plant:HotWaterLoop']['Hot Water Loop']['hot_water_pump_configuration'] = 'VariableFlow'
        ep = ExpandPlantLoop(template=tmp_mock)
        output = ep.run()
        self.assertEqual(
            {
                'AvailabilityManagerAssignmentList': 1,
                'Branch': 6,
                'Pipe:Adiabatic': 5,
                'PlantEquipmentOperation:HeatingLoad': 1,
                'PlantEquipmentOperationSchemes': 1,
                'PlantLoop': 1,
                'Pump:VariableSpeed': 1,
                'Schedule:Compact': 1,
                'SetpointManager:OutdoorAirReset': 1,
                'Sizing:Plant': 1
            },
            ep.summarize_epjson(output.epjson)
        )
        return
