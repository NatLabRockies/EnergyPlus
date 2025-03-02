import unittest
from unittest.mock import MagicMock, PropertyMock

from src.expand_objects import ExpandPlantEquipment
from . import BaseTest


class TestExpandPlantEquipmentObjects(BaseTest, unittest.TestCase):
    """
    EnergyPlus object testing for ExpandPlantEquipment operations
    """
    def setUp(self):
        return

    def teardown(self):
        return

    def test_water_cooled_chiller_equipment_objects_created(self):
        epl = MagicMock()
        template_type = PropertyMock(return_value='HVACTemplate:Plant:ChilledWaterLoop')
        type(epl).template_type = template_type
        expanded_plant_loops = {'Test Loop': epl}
        del epl.chilled_water_primary_pump_type
        epe = ExpandPlantEquipment(
            template={
                "HVACTemplate:Plant:Chiller": {
                    "Main Chiller": {
                        "capacity": "Autosize",
                        "chiller_type": "ElectricReciprocatingChiller",
                        "condenser_type": "WaterCooled",
                        "nominal_cop": 3.2,
                        "priority": "1"
                    }
                }
            },
            plant_loop_class_objects=expanded_plant_loops)
        output = epe.run()
        self.assertEqual(
            {
                'Branch': 2,
                'Chiller:Electric:EIR': 1,
                'Curve:Biquadratic': 2,
                'Curve:Quadratic': 1
            },
            epe.summarize_epjson(output.epjson))
        return

    def test_hot_water_boiler_objects_created(self):
        epl = MagicMock()
        template_type = PropertyMock(return_value='HVACTemplate:Plant:HotWaterLoop')
        type(epl).template_type = template_type
        del epl.hot_water_pump_type
        expanded_plant_loops = {'Test Loop': epl}
        epe = ExpandPlantEquipment(
            template={
                "HVACTemplate:Plant:Boiler": {
                    "Main Boiler": {
                        "boiler_type": "HotWaterBoiler",
                        "capacity": "Autosize",
                        "efficiency": 0.8,
                        "fuel_type": "NaturalGas",
                        "priority": "1"
                    }
                }
            },
            plant_loop_class_objects=expanded_plant_loops)
        output = epe.run()
        self.assertEqual(
            {'Boiler:HotWater': 1, 'Branch': 1, 'Curve:Quadratic': 1},
            epe.summarize_epjson(output.epjson)
        )
        return
