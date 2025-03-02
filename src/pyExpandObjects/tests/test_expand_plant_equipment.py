import copy
import unittest
from unittest.mock import MagicMock, PropertyMock

from src.expand_objects import ExpandPlantEquipment
from src.expand_objects import InvalidTemplateException
from . import BaseTest

mock_plant_equipment_template = {
    "HVACTemplate:Plant:Chiller": {
        "Main Chiller": {
            "capacity": "Autosize",
            "chiller_type": "ElectricReciprocatingChiller",
            "condenser_type": "WaterCooled",
            "nominal_cop": 3.2,
            "priority": "1"
        }
    }
}


class TestExpandPlantEquipment(BaseTest, unittest.TestCase):
    """
    General processing of ExpandPlantEquipmentLoop operations
    """
    def setUp(self):
        return

    def teardown(self):
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:PlantEquipment:Input Template Required")
    def test_check_templates_are_required(self):
        with self.assertRaises(TypeError):
            ExpandPlantEquipment()
        return

    @BaseTest._test_logger(doc_text="HVACTemplate:PlantEquipment:Verify valid template object")
    def test_verify_good_template(self):
        epl = MagicMock()
        template_type = PropertyMock(return_value='HVACTemplate:Plant:ChilledWaterLoop')
        type(epl).template_type = template_type
        del epl.chilled_water_primary_pump_type
        expanded_plant_loops = {'Test Loop': epl}
        output = ExpandPlantEquipment(
            template=mock_plant_equipment_template,
            plant_loop_class_objects=expanded_plant_loops)
        self.assertEqual('Main Chiller', output.template_name)
        return

    def test_verify_plant_loop_type_is_set_default(self):
        epl = MagicMock()
        template_type = PropertyMock(return_value='HVACTemplate:Plant:ChilledWaterLoop')
        type(epl).template_type = template_type
        del epl.chilled_water_primary_pump_type
        expanded_plant_loops = {'Test Loop': epl}
        output = ExpandPlantEquipment(
            template=mock_plant_equipment_template,
            plant_loop_class_objects=expanded_plant_loops)
        self.assertEqual('ChilledWaterLoop', output.template_plant_loop_type)
        return

    def test_verify_first_plant_loop_type_is_set(self):
        expanded_plant_loops = {}
        for loop_type in ['Hot', 'Mixed']:
            epl = MagicMock()
            template_type = PropertyMock(return_value='HVACTemplate:Plant:{}WaterLoop'.format(loop_type))
            type(epl).template_type = template_type
            del epl.hot_water_pump_type
            expanded_plant_loops['{} Loop'.format(loop_type)] = epl
        output = ExpandPlantEquipment(
            template={'HVACTemplate:Plant:Boiler': {'Main Boiler': {}}},
            plant_loop_class_objects=expanded_plant_loops)
        self.assertEqual('HotWaterLoop', output.template_plant_loop_type)
        return

    def test_verify_plant_loop_type_is_set_from_template(self):
        tmp_mock = copy.deepcopy(mock_plant_equipment_template)
        tmp_mock['HVACTemplate:Plant:Chiller']['Main Chiller']['template_plant_loop_type'] = 'Test'
        epl = MagicMock()
        template_type = PropertyMock(return_value='TestLoop')
        type(epl).template_type = template_type
        expanded_plant_loops = {'Test Loop': epl}
        output = ExpandPlantEquipment(
            template=tmp_mock,
            plant_loop_class_objects=expanded_plant_loops)
        self.assertEqual('TestLoop', output.template_plant_loop_type)
        return

    def test_reject_plant_loop_type_with_no_loops(self):
        tmp_mock = {
            "HVACTemplate:Plant:BadEquipment": {
                "Bad Name": {
                    "capacity": "Autosize"
                }
            }
        }
        with self.assertRaisesRegex(InvalidTemplateException, 'An invalid number of plant loops'):
            ExpandPlantEquipment(template=tmp_mock)
        return

    def test_reject_plant_loop_type_with_mismatch_loops(self):
        tmp_mock = copy.deepcopy(mock_plant_equipment_template)
        epl = MagicMock()
        template_type = PropertyMock(return_value='TestLoop')
        type(epl).template_type = template_type
        expanded_plant_loops = {'Test Loop': epl}
        with self.assertRaisesRegex(InvalidTemplateException, 'Plant equipment loop type did not'):
            ExpandPlantEquipment(
                template=tmp_mock,
                plant_loop_class_objects=expanded_plant_loops)
        return
