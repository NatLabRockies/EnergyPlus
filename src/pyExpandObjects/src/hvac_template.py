import re
import copy
from epjson_handler import EPJSON
from expand_objects import ExpandObjects, ExpandThermostat, ExpandZone, ExpandSystem, ExpandPlantLoop, \
    ExpandPlantEquipment
from custom_exceptions import InvalidTemplateException, InvalidEpJSONException, PyExpandObjectsYamlStructureException


class HVACTemplate(EPJSON):
    """
    Handle HVACTemplate conversion process and connect created objects together.

    Attributes:
        templates: HVACTemplate objects from epJSON file

        base_objects: Non-HVACTemplate objects from epJSON file

        templates_zones: HVACTemplate:Zone: objects

        templates_systems: HVACTemplate:System: objects

        templates_plant_equipment: HVACTemplate:Plant equipment objects

        templates_plant_loops: HVACTemplate:Plant: loop objects

        expanded_*: List of class objects for each template type

        epjson: epJSON used to store connection objects
    """

    def __init__(
            self,
            no_schema=False,
            logger_level='WARNING',
            logger_name='console_only_logger',
            reset_stream=True):
        """
        :param no_schema: Boolean flag for skipping schema validation
        """
        super().__init__(no_schema=no_schema, logger_level=logger_level, logger_name=logger_name,
                         reset_stream=reset_stream)
        self.logger_level = logger_level
        self.logger_name = logger_name
        self.templates = {}
        self.base_objects = {}
        self.templates_systems = {}
        self.templates_zones = {}
        self.templates_plant_equipment = {}
        self.templates_plant_loops = {}
        self.templates_thermostats = {}
        self.expanded_thermostats = {}
        self.expanded_zones = {}
        self.expanded_systems = {}
        self.expanded_plant_loops = {}
        self.expanded_plant_equipment = {}
        self.epjson = {}
        return

    def _hvac_template_preprocess(self, epjson):
        """
        Organize epJSON and assign template objects to specific class attributes

        :param epjson: Input epJSON object
        :return: organized epJSON template objects into templates, and templates_* as class attributes
        """
        self.logger.info('##### HVACTemplate #####')
        for object_type, object_structure in epjson.items():
            if re.match('^HVACTemplate:*', object_type):
                if re.match('^HVACTemplate:Thermostat$', object_type):
                    self.merge_epjson(
                        super_dictionary=self.templates_thermostats,
                        object_dictionary={object_type: object_structure},
                        unique_name_override=False)
                elif re.match('^HVACTemplate:Zone:('
                              'ConstantVolume|BaseboardHeat|FanCoil|IdealLoadsAirSystem|PTAC|PTHP|WaterToAirHeatPump|'
                              'VRF|Unitary|VAV|VAV:FanPowered|VAV:HeatAndCool|DualDuct)$',
                              object_type):
                    zone_default_map = {
                        'HVACTemplate:Zone:BaseboardHeat': {
                            'baseboard_heating_type': 'HotWater',
                            'outdoor_air_method': 'Flow/Person'
                        },
                        'HVACTemplate:Zone:ConstantVolume': {
                            'outdoor_air_method': 'Flow/Person',
                            'zone_cooling_design_supply_air_temperature_input_method': 'SystemSupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature': 12.8,
                            'zone_cooling_design_supply_air_temperature_difference': 11.11,
                            'zone_heating_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature': 50.0,
                            'zone_heating_design_supply_air_temperature_difference': 30.0
                        },
                        'HVACTemplate:Zone:FanCoil': {
                            'cooling_coil_type': 'ChilledWater',
                            'heating_coil_type': 'HotWater',
                            'outdoor_air_method': 'Flow/Person',
                            'supply_fan_delta_pressure': 75,
                            'cooling_coil_design_setpoint': 14.0,
                            'heating_coil_design_setpoint': 50.0,
                            'zone_cooling_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature_difference': 11.11,
                            'zone_heating_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature_difference': 30.0
                        },
                        'HVACTemplate:Zone:PTAC': {
                            'outdoor_air_method': 'Flow/Person',
                            'supply_fan_placement': 'DrawThrough',
                            'cooling_coil_type': 'SingleSpeedDX',
                            'supply_fan_total_efficiency': 0.7,
                            'cooling_coil_gross_rated_cooling_cop': 3.0,
                            'heating_coil_type': 'Electric',
                            'zone_cooling_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature': 14.0,
                            'zone_cooling_design_supply_air_temperature_difference': 11.0,
                            'zone_heating_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature': 50.0,
                            'zone_heating_design_supply_air_temperature_difference': 30.0
                        },
                        'HVACTemplate:Zone:PTHP': {
                            'outdoor_air_method': 'Flow/Person',
                            'supply_fan_placement': 'DrawThrough',
                            'cooling_coil_type': 'SingleSpeedDX',
                            'cooling_coil_gross_rated_cop': 3.0,
                            'supply_fan_total_efficiency': 0.7,
                            'heat_pump_heating_coil_type': 'SingleSpeedDXHeatPump',
                            'heat_pump_heating_coil_gross_rated_cop': 2.75,
                            'heat_pump_heating_minimum_outdoor_dry_bulb_temperature': -8.0,
                            'heat_pump_defrost_maximum_outdoor_dry_bulb_temperature': 5.0,
                            'heat_pump_defrost_strategy': 'ReverseCycle',
                            'heat_pump_defrost_control': 'Timed',
                            'supplemental_heating_coil_type': 'Electric',
                            'supplemental_heating_coil_maximum_outdoor_dry_bulb_temperature': 21.0,
                            'zone_cooling_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature': 14.0,
                            'zone_cooling_design_supply_air_temperature_difference': 11.11,
                            'zone_heating_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature': 50.0,
                            'zone_heating_design_supply_air_temperature_difference': 30.0
                        },
                        'HVACTemplate:Zone:Unitary': {
                            'outdoor_air_method': 'Flow/Person',
                            'zone_cooling_design_supply_air_temperature_input_method': 'SystemSupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature': 12.8,
                            'zone_cooling_design_supply_air_temperature_difference': 11.11,
                            'zone_heating_design_supply_air_temperature_input_method': 'SystemSupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature': 50.0,
                            'zone_heating_design_supply_air_temperature_difference': 30.0
                        },
                        'HVACTemplate:Zone:VRF': {
                            'outdoor_air_method': 'Flow/Person',
                            'supply_air_fan_placement': 'BlowThrough',
                            'cooling_coil_type': 'VariableRefrigerantFlowDX',
                            'supply_fan_total_efficiency': 0.7,
                            'heating_coil_type': 'VariableRefrigerantFlowDX',
                            'zone_cooling_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature': 14.0,
                            'zone_cooling_design_supply_air_temperature_difference': 11.11,
                            'zone_heating_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature': 50.0,
                            'zone_heating_design_supply_air_temperature_difference': 30.0
                        },
                        'HVACTemplate:Zone:WaterToAirHeatPump': {
                            'outdoor_air_method': 'Flow/Person',
                            'supply_fan_placement': 'DrawThrough',
                            'cooling_coil_type': 'Coil:Cooling:WaterToAirHeatPump:EquationFit',
                            'cooling_coil_gross_rated_cop': 3.5,
                            # todo_eo: The template and ZoneHVAC:WaterToAirHeatPump defaults are mismatched for this
                            #  field. This is not default efficiency for Fan:OnOff
                            'supply_fan_total_efficiency': 0.7,
                            'heat_pump_heating_coil_type': 'Coil:Heating:WaterToAirHeatPump:EquationFit',
                            'heat_pump_heating_coil_gross_rated_cop': 4.2,
                            'maximum_cycling_rate': 2.5,
                            'supplemental_heating_coil_type': 'Electric',
                            'zone_cooling_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_cooling_design_supply_air_temperature': 14.0,
                            'zone_cooling_design_supply_air_temperature_difference': 11.11,
                            'zone_heating_design_supply_air_temperature_input_method': 'SupplyAirTemperature',
                            'zone_heating_design_supply_air_temperature': 50.0,
                            'zone_heating_design_supply_air_temperature_difference': 30.0,
                            'heat_pump_coil_water_flow_mode': 'Cycling'
                        }
                    }
                    for object_name, object_fields in object_structure.items():
                        # set defaults
                        selected_default_map = zone_default_map.get(object_type)
                        if selected_default_map:
                            for field, default_value in selected_default_map.items():
                                if not object_fields.get(field):
                                    object_fields[field] = default_value
                    # set a mapping of zone template type to look up parent system
                    zone_template_map = {
                        ('HVACTemplate:Zone:ConstantVolume', ):
                            (
                                'template_constant_volume_system_name',
                                ['HVACTemplate:System:ConstantVolume', ]),
                        ('HVACTemplate:Zone:BaseboardHeat', 'HVACTemplate:Zone:FanCoil', 'HVACTemplate:Zone:PTAC',
                         'HVACTemplate:Zone:PTHP', 'HVACTemplate:Zone:WaterToAirHeatPump', 'HVACTemplate:Zone:VRF', ):
                            (
                                'dedicated_outdoor_air_system_name',
                                ['HVACTemplate:System:DedicatedOutdoorAir', ]),
                        ('HVACTemplate:Zone:Unitary', ):
                            (
                                'template_unitary_system_name',
                                ['HVACTemplate:System:Unitary', 'HVACTemplate:System:UnitaryHeatPump',
                                 'HVACTemplate:System:UnitaryHeatPump:AirToAir', 'HVACTemplate:System:UnitarySystem']),
                        ('HVACTemplate:Zone:VAV', 'HVACTemplate:Zone:VAVFanPowered'):
                            (
                                'template_vav_system_name',
                                ['HVACTemplate:System:VAV', 'HVACTemplate:System:PackagedVAV']),
                        ('HVACTemplate:Zone:DualDuct', ):
                            (
                                'template_dual_duct_system_name',
                                ['HVACTemplate:System:DualDuct', ]),
                        ('HVACTemplate:Zone:vrf', ):
                            (
                                'template_vrf_system_name',
                                ['HVACTemplate:System:VRF', ])}
                    # Check the referenced system against the epjson and issue a warning if it isn't found
                    system_check_list = [v for k, v in zone_template_map.items() if object_type in k]
                    if system_check_list:
                        system_check_list = system_check_list[0]
                        for object_name, object_fields in object_structure.items():
                            system_name = object_fields.get(system_check_list[0])
                            if not system_name and system_check_list[0] == 'dedicated_outdoor_air_system_name':
                                continue
                            else:
                                template_system_name = None
                                for system_type in system_check_list[1]:
                                    system_group = epjson.get(system_type)
                                    if system_group:
                                        template_system_name = True if system_name in system_group else False
                                        if template_system_name:
                                            break
                                if not template_system_name:
                                    raise InvalidTemplateException(
                                        'Error: In {} ({}) Could not find air handler name referenced ({})'
                                        .format(object_type, object_name, system_name))
                    # check fields
                    for object_name, object_fields in object_structure.items():
                        # check for required info
                        if not object_fields.get('template_thermostat_name', None):
                            self.logger.info(
                                'In {} ({}) template thermostat name not provided'
                                .format(object_type, object_name))
                        # check baseboard settings
                        if object_fields.get('baseboard_heating_type', None) == 'HotWater' and (
                                not epjson.get('HVACTemplate:Plant:HotWaterLoop') or not
                                epjson.get('HVACTemplate:Plant:Boiler')):
                            self.logger.warning(
                                'Warning: Both a HVACTemplate:Plant:HotWaterLoop and a HVACTemplate:Plant:Boiler are '
                                'needed when using hot water baseboards.  Template name: {}'.format(object_name))
                        # fan coil capacity control with doas
                        if object_type == 'HVACTemplate:Zone:FanCoil':
                            if object_fields.get('capacity_control_method') == 'ConstantFanVariableFlow' and \
                                    object_fields.get('dedicated_outdoor_air_system_name', '') != '':
                                self.logger.warning(
                                    'Warning: In {} ({})'
                                    ' the Capacity Control Method is {}'
                                    ' and the zone is served by a dedicated outdoor air system.'
                                    .format(object_type, object_name, object_fields.get('capacity_control_method')))
                        # IdealLoads input check
                        if object_type == 'HVACTemplate:Zone:IdealLoadsAirSystem':
                            heating_limit = object_fields.get('heating_limit')
                            maximum_heating_air_flow_rate = object_fields.get('maximum_heating_air_flow_rate', '')
                            maximum_sensible_heating_capacity = \
                                object_fields.get('maximum_sensible_heating_capacity', '')
                            cooling_limit = object_fields.get('cooling_limit')
                            maximum_cooling_air_flow_rate = object_fields.get('maximum_cooling_air_flow_rate', '')
                            maximum_total_cooling_capacity = \
                                object_fields.get('maximum_total_cooling_capacity', '')
                            if heating_limit == 'LimitFlowRate' and maximum_heating_air_flow_rate == '':
                                raise InvalidTemplateException(
                                    'Error: In {} ({})'
                                    ' the Heating Limit field is {} but the Maximum Heating Air Flow Rate field is '
                                    'blank. Enter a value or autosize in this field.'
                                    .format(object_type, object_name, object_fields.get('heating_limit')))
                            elif heating_limit == 'LimitCapacity' and maximum_sensible_heating_capacity == '':
                                raise InvalidTemplateException(
                                    'Error: In {} ({})'
                                    ' the Heating Limit field is {} but the Maximum Sensible Heating Capacity field is '
                                    'blank. Enter a value or autosize in this field.'
                                    .format(object_type, object_name, object_fields.get('heating_limit')))
                            elif heating_limit == 'LimitFlowRateAndCapacity' and (
                                    maximum_heating_air_flow_rate == '' or maximum_sensible_heating_capacity == ''):
                                msg = []
                                if maximum_heating_air_flow_rate == '':
                                    msg.append('the Maximum Heating Air Flow Rate field is blank')
                                if maximum_sensible_heating_capacity == '':
                                    msg.append('the Maximum Sensible Heating Capacity field is blank')
                                raise InvalidTemplateException(
                                    'Error: In {} ({})'
                                    ' the Heating Limit field is {} but {}. Enter a value or autosize in this field.'
                                    .format(
                                        object_type,
                                        object_name,
                                        object_fields.get('heating_limit'),
                                        ' and '.join(msg)))
                            if cooling_limit == 'LimitFlowRate' and maximum_cooling_air_flow_rate == '':
                                raise InvalidTemplateException(
                                    'Error: In {} ({})'
                                    ' the Heating Limit field is {} but the Maximum Cooling Air Flow Rate field is '
                                    'blank. Enter a value or autosize in this field.'
                                    .format(object_type, object_name, object_fields.get('cooling_limit')))
                            elif cooling_limit == 'LimitCapacity' and maximum_total_cooling_capacity == '':
                                raise InvalidTemplateException(
                                    'Error: In {} ({})'
                                    ' the Cooling Limit field is {} but the Maximum Total Cooling Capacity field is '
                                    'blank. Enter a value or autosize in this field.'
                                    .format(object_type, object_name, object_fields.get('cooling_limit')))
                            elif cooling_limit == 'LimitFlowRateAndCapacity' and (
                                    maximum_cooling_air_flow_rate == '' or maximum_total_cooling_capacity == ''):
                                msg = []
                                if maximum_cooling_air_flow_rate == '':
                                    msg.append('the Maximum Cooling Air Flow Rate field is blank')
                                if maximum_total_cooling_capacity == '':
                                    msg.append('the Maximum Total Cooling Capacity field is blank')
                                raise InvalidTemplateException(
                                    'Error: In {} ({})'
                                    ' the Cooling Limit field is {} but {}. Enter a value or autosize in this field.'
                                    .format(
                                        object_type,
                                        object_name,
                                        object_fields.get('cooling_limit'),
                                        ' and '.join(msg)))
                    self.merge_epjson(
                        super_dictionary=self.templates_zones,
                        object_dictionary={object_type: object_structure},
                        unique_name_override=False)
                elif re.match('^HVACTemplate:System:('
                              'VRF|Unitary|UnitaryHeatPump:AirToAir|UnitarySystem|VAV|PackagedVAV|'
                              'ConstantVolume|DualDuct|DedicatedOutdoorAir'
                              ')$', object_type):
                    # check for individual template issues
                    system_default_map = {
                        'HVACTemplate:System:ConstantVolume': {
                            'cooling_coil_type': 'ChilledWater',
                            'cooling_coil_design_setpoint_temperature': 12.8,
                            'cooling_coil_setpoint_at_outdoor_dry_bulb_low': 15.6,
                            'cooling_coil_reset_outdoor_dry_bulb_low': 15.6,
                            'cooling_coil_setpoint_at_outdoor_dry_bulb_high': 12.8,
                            'cooling_coil_reset_outdoor_dry_bulb_high': 23.3,
                            'economizer_type': 'NoEconomizer',
                            'heating_coil_type': 'HotWater',
                            'heating_coil_design_setpoint': 10,
                            'heating_coil_setpoint_at_outdoor_dry_bulb_low': 15.0,
                            'heating_coil_reset_outdoor_dry_bulb_low': 7.8,
                            'heating_coil_setpoint_at_outdoor_dry_bulb_high': 12.2,
                            'heating_coil_reset_outdoor_dry_bulb_high': 12.2
                        },
                        'HVACTemplate:System:DedicatedOutdoorAir': {
                            'air_outlet_type': 'DirectIntoZone',
                            'cooling_coil_type': 'ChilledWater',
                            'cooling_coil_design_setpoint_temperature': 12.8,
                            'cooling_coil_setpoint_at_outdoor_dry_bulb_low': 15.6,
                            'cooling_coil_reset_outdoor_dry_bulb_low': 15.6,
                            'cooling_coil_setpoint_at_outdoor_dry_bulb_high': 12.8,
                            'cooling_coil_reset_outdoor_dry_bulb_high': 23.3,
                            'dx_cooling_coil_gross_rated_cop': 3.0,
                            'heating_coil_type': 'HotWater',
                            'heating_coil_design_setpoint': 12.2,
                            'heating_coil_setpoint_at_outdoor_dry_bulb_low': 15.0,
                            'heating_coil_reset_outdoor_dry_bulb_low': 7.8,
                            'heating_coil_setpoint_at_outdoor_dry_bulb_high': 12.2,
                            'heating_coil_reset_outdoor_dry_bulb_high': 12.2,
                            'humidifier_rated_capacity': 1e-06,
                            'humidifier_constant_setpoint': 0.003
                        },
                        'HVACTemplate:System:DualDuct': {
                            'system_configuration_type': 'SingleFanConstantVolume',
                            'main_supply_fan_minimum_flow_fraction': 0.2,
                            'cold_duct_supply_fan_minimum_flow_fraction': 0.2,
                            'cold_duct_supply_fan_placement': 'DrawThrough',
                            'hot_duct_supply_fan_minimum_flow_fraction': 0.2,
                            'hot_duct_supply_fan_placement': 'DrawThrough',
                            'cooling_coil_type': 'ChilledWater',
                            'cooling_coil_setpoint_control_type': 'FixedSetpoint',
                            'cooling_coil_design_setpoint_temperature': 12.8,
                            'cooling_coil_setpoint_at_outdoor_dry_bulb_low': 15.6,
                            'cooling_coil_reset_outdoor_dry_bulb_low': 15.6,
                            'cooling_coil_setpoint_at_outdoor_dry_bulb_high': 12.8,
                            'cooling_coil_reset_outdoor_dry_bulb_high': 23.3,
                            'heating_coil_type': 'HotWater',
                            'heating_coil_setpoint_control_type': 'FixedSetpoint',
                            'heating_coil_design_setpoint': 50,
                            'heating_coil_setpoint_at_outdoor_dry_bulb_low': 50,
                            'heating_coil_reset_outdoor_dry_bulb_low': 7.8,
                            'heating_coil_setpoint_at_outdoor_dry_bulb_high': 26,
                            'heating_coil_reset_outdoor_dry_bulb_high': 12.2,
                            'preheat_coil_design_setpoint': 7.2
                        },
                        'HVACTemplate:System:PackagedVAV': {
                            'cooling_coil_type': 'TwoSpeedDX',
                            'cooling_coil_design_setpoint': 12.8,
                            'cooling_coil_gross_rated_cop': 3.0,
                            'heating_coil_design_setpoint': 10
                        },
                        'HVACTemplate:System:Unitary': {
                            'cooling_coil_type': 'SingleSpeedDX',
                            'cooling_design_supply_air_temperature': 12.8,
                            'cooling_coil_gross_rated_cop': 3.0,
                            'heating_design_supply_air_temperature': 50.0,
                            'economizer_type': 'NoEconomizer',
                            'economizer_lockout': 'NoLockout',
                            'supply_fan_placement': 'BlowThrough',
                            'dehumidification_setpoint': 60.0,
                            'humidifier_rated_capacity': 1e-06,
                            'humidifier_setpoint': 30.0
                        },
                        'HVACTemplate:System:UnitarySystem': {
                            'control_type': 'Load',
                            'supply_fan_placement': 'BlowThrough',
                            'cooling_coil_type': 'SingleSpeedDX',
                            'number_of_speeds_for_cooling': 1,
                            'dx_cooling_coil_gross_rated_cop': 3.0,
                            'heating_coil_type': 'Gas',
                            'number_of_speeds_or_stages_for_heating': 1,
                            'heat_pump_heating_coil_gross_rated_cop': 2.75,
                            'heat_pump_heating_minimum_outdoor_dry_bulb_temperature': -8.0,
                            'heat_pump_defrost_maximum_outdoor_dry_bulb_temperature': 5.0,
                            'heat_pump_defrost_strategy': 'ReverseCycle',
                            'heat_pump_defrost_control': 'Timed',
                            'supplemental_heating_or_reheat_coil_type': 'None',
                            'supplemental_heating_or_reheat_coil_maximum_outdoor_dry_bulb_temperature': 21.0,
                            'economizer_type': 'NoEconomizer',
                            'economizer_lockout': 'NoLockout',
                            'heat_recovery_frost_control_type': 'None',
                            'dehumidification_control_type': 'None',
                            'dehumidification_relative_humidity_setpoint': 60.0,
                            'humidifier_type': 'None',
                            'humidifier_rated_capacity': 1e-06,
                            'humidifier_relative_humidity_setpoint': 30.0,
                            'sizing_option': 'NonCoincident',
                            'return_fan': 'No'
                        },
                        'HVACTemplate:System:VAV': {
                            'cooling_coil_type': 'ChilledWater',
                            'cooling_coil_design_setpoint': 12.8,
                            'heating_coil_type': 'None',
                            'heating_coil_design_setpoint': 10,
                            'preheat_coil_design_setpoint': 7.2,
                            'humidifier_rated_capacity': 1e-06
                        }
                    }
                    for object_name, object_fields in object_structure.items():
                        # set defaults
                        selected_default_map = system_default_map.get(object_type)
                        if selected_default_map:
                            for field, default_value in selected_default_map.items():
                                if not object_fields.get(field):
                                    object_fields[field] = default_value
                        try:
                            zone_system_field = self._get_zone_template_field_from_system_type(object_type)
                        except InvalidTemplateException:
                            continue
                        system_names = [
                            zone_fields.get(zone_system_field) for zone_type, zone_structure in epjson.items()
                            if re.match(r'HVACTemplate:Zone:.*', zone_type)
                            for zone_template_name, zone_fields in zone_structure.items()]
                        if object_name not in system_names:
                            raise InvalidTemplateException(
                                'Error: In {} ({}) Did not find any HVACTemplate:Zone objects connected to system.'
                                'There must be at least one zone object which specifies '
                                'this system as the Template Unitary System Name.'
                                .format(object_type, object_name))
                        if object_fields.get('night_cycle_control', 'None') == 'CycleOnControlZone' and \
                                object_fields.get('night_cycle_control_zone_name', 'None') == 'None':
                            self.logger.warning('Warning: A zone name must be specified when Night Cycle Control is '
                                                'set to Cycle on Control Zone for {} with unique name {}'
                                                .format(object_type, object_name))
                    # check for control zones
                    if object_type in ['HVACTemplate:System:Unitary',
                                       'HVACTemplate:System:ConstantVolume',
                                       'HVACTemplate:System:UnitarySystem']:
                        for object_name, object_fields in object_structure.items():
                            try:
                                zone_system_field = self._get_zone_template_field_from_system_type(object_type)
                            except InvalidTemplateException:
                                continue
                            try:
                                zones_served = [
                                    zone_fields.get('zone_name') for zone_type, zone_structure in epjson.items()
                                    if re.match(r'HVACTemplate:Zone:.*', zone_type)
                                    for zone_template_name, zone_fields in zone_structure.items()
                                    if zone_fields.get(zone_system_field) == object_name]
                            except AttributeError:
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) No HVACTemplate:Zone template objects reference'
                                    ' the system object'
                                    .format(object_type, object_name))
                            if object_type in ['HVACTemplate:System:Unitary', 'HVACTemplate:System:UnitarySystem'] and \
                                    object_fields.get('control_zone_or_thermostat_location_name') and \
                                    object_fields.get('control_zone_or_thermostat_location_name') not in zones_served:
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) for the field control_zone_or_thermostat_location_name could '
                                    'not find a matching HVACTemplate:Zone:Unitary named {}'
                                    .format(
                                        object_type,
                                        object_name,
                                        object_fields.get('control_zone_or_thermostat_location_name')))
                            elif object_type in ['HVACTemplate:System:Unitary',
                                                 'HVACTemplate:System:UnitarySystem'] and \
                                    not object_fields.get('control_zone_or_thermostat_location_name'):
                                raise InvalidTemplateException(
                                    'Error: control_zone_or_thermostat_location_name must '
                                    'be specified for {} which is a {}'.format(object_name, object_type))
                            elif object_type == 'HVACTemplate:System:ConstantVolume' and \
                                    object_fields.get('cooling_coil_control_zone_name') and \
                                    object_fields.get('cooling_coil_control_zone_name') not in zones_served:
                                raise InvalidTemplateException(
                                    'Error: In {} named {} for the field cooling_coil_control_zone_name could '
                                    'not find a matching HVACTemplate:Zone:Unitary named {}'
                                    .format(
                                        object_type,
                                        object_name,
                                        object_fields.get('cooling_coil_control_zone_name')))
                            elif object_type == 'HVACTemplate:System:ConstantVolume' and \
                                    object_fields.get('heating_coil_control_zone_name') and \
                                    object_fields.get('heating_coil_control_zone_name') not in zones_served:
                                raise InvalidTemplateException(
                                    'Error: In {} named {} for the field heating_coil_control_zone_name could '
                                    'not find a matching HVACTemplate:Zone:Unitary named {}'
                                    .format(
                                        object_type,
                                        object_name,
                                        object_fields.get('heating_coil_control_zone_name')))
                    # check vrf master thermostat referenced zone
                    if object_type in ['HVACTemplate:System:VRF', ]:
                        for object_name, object_fields in object_structure.items():
                            try:
                                zone_system_field = self._get_zone_template_field_from_system_type(object_type)
                            except InvalidTemplateException:
                                continue
                            try:
                                zones_served = [
                                    zone_fields.get('zone_name') for zone_type, zone_structure in epjson.items()
                                    if re.match(r'HVACTemplate:Zone:.*', zone_type)
                                    for zone_template_name, zone_fields in zone_structure.items()
                                    if zone_fields.get(zone_system_field) == object_name]
                            except AttributeError:
                                raise InvalidTemplateException('Error: No HVACTemplate:Zone:Unitary template objects reference'
                                                               ' the {} object'.format(object_type))
                            if object_fields.get('master_thermostat_priority_control_type') == \
                                    'MasterThermostatPriority' and \
                                    object_fields.get('zone_name_for_master_thermostat_location') not in zones_served:
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) for the field Zone Name for '
                                    'Master Thermostat Location could not find a matching '
                                    'HVACTemplate:Zone:VRF named: {}'
                                    .format(
                                        object_type,
                                        object_name,
                                        object_fields.get('zone_name_for_master_thermostat_location')))
                            if object_fields.get('master_thermostat_priority_control_type') == 'Scheduled' and \
                                    not object_fields.get('thermostat_priority_schedule_name'):
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) the Master Thermostat '
                                    'Priority Control Type = Scheduled, but the Thermostat Priority Schedule Name '
                                    'is blank.'.format(object_type, object_name))
                    self.merge_epjson(
                        super_dictionary=self.templates_systems,
                        object_dictionary={object_type: object_structure},
                        unique_name_override=False)
                elif re.match('^HVACTemplate:Plant:(ChilledWater|HotWater|MixedWater)Loop$', object_type):
                    if len(object_structure.keys()) > 1:
                        self.logger.warning('Warning: Only one {} allowed per file.'.format(object_type))
                    plant_loop_default_map = {
                        'HVACTemplate:Plant:ChilledWaterLoop': {
                            'chilled_water_design_setpoint': 7.22,
                            'condenser_water_design_setpoint': 29.4,
                            'chilled_water_pump_configuration': 'ConstantPrimaryNoSecondary',
                            'chilled_water_setpoint_at_outdoor_dry_bulb_low': 12.2,
                            'chilled_water_reset_outdoor_dry_bulb_low': 15.6,
                            'chilled_water_setpoint_at_outdoor_dry_bulb_high': 6.7,
                            'chilled_water_reset_outdoor_dry_bulb_high': 26.7
                        },
                        'HVACTemplate:Plant:HotWaterLoop': {
                            'hot_water_design_setpoint': 82,
                            'hot_water_pump_configuration': 'ConstantFlow',
                            'hot_water_setpoint_at_outdoor_dry_bulb_low': 82.2,
                            'hot_water_reset_outdoor_dry_bulb_low': -6.7,
                            'hot_water_setpoint_at_outdoor_dry_bulb_high': 65.6,
                            'hot_water_reset_outdoor_dry_bulb_high': 10
                        },
                        'HVACTemplate:Plant:MixedWaterLoop': {
                            'high_temperature_design_setpoint': 33,
                            'low_temperature_design_setpoint': 20,
                            'water_pump_configuration': 'ConstantFlow'
                        }
                    }
                    for object_name, object_fields in object_structure.items():
                        # set defaults
                        selected_default_map = plant_loop_default_map.get(object_type)
                        if selected_default_map:
                            for field, default_value in selected_default_map.items():
                                if not object_fields.get(field):
                                    object_fields[field] = default_value
                    if object_type == 'HVACTemplate:Plant:HotWaterLoop':
                        loop_system_list = [
                            'HVACTemplate:System:VAV', 'HVACTemplate:Zone:FanCoil', 'HVACTemplate:Zone:Unitary',
                            'HVACTemplate:Zone:PTAC', 'HVACTemplate:Zone:PTHP', 'HVACTemplate:Zone:WaterToAirHeatPump',
                            'HVACTemplate:System:UnitaryHeatPump:AirToAir', 'HVACTemplate:System:PackagedVAV',
                            'HVACTemplate:System:DedicatedOutdoorAir', 'HVACTemplate:System:ConstantVolume',
                            'HVACTemplate:System:DualDuct', 'HVACTemplate:Zone:BaseboardHeat',
                            'HVACTemplate:System:UnitarySystem', 'HVACTemplate:System:VRF']
                        if not any(hwlst in loop_system_list for hwlst in epjson.keys()):
                            self.logger.warning(
                                'Warning: You must specify at least one {} '
                                'if a HVACTemplate:Plant:HotWaterLoop is defined.'
                                .format(' or '.join(loop_system_list)))
                    if object_type == 'HVACTemplate:Plant:ChilledWaterLoop':
                        loop_system_list = [
                            'HVACTemplate:System:VAV', 'HVACTemplate:Zone:FanCoil',
                            'HVACTemplate:System:DedicatedOutdoorAir', 'HVACTemplate:System:ConstantVolume',
                            'HVACTemplate:System:DualDuct', 'HVACTemplate:System:UnitarySystem']
                        if not any(hwlst in loop_system_list for hwlst in epjson.keys()):
                            self.logger.warning(
                                'Warning: You must specify at least one {} '
                                'if a HVACTemplate:Plant:ChilledWaterLoop is defined.'
                                .format(' or '.join(loop_system_list)))
                    if object_type == 'HVACTemplate:Plant:MixedWaterLoop':
                        loop_system_list = [
                            'HVACTemplate:Zone:WaterToAirHeatPump', 'HVACTemplate:System:VRF',
                            'HVACTemplate:System:UnitarySystem']
                        if not any(hwlst in loop_system_list for hwlst in epjson.keys()):
                            self.logger.warning(
                                'Warning: You must specify at least one {} '
                                'if a HVACTemplate:Plant:MixedWaterLoop is defined.'
                                .format(' or '.join(loop_system_list)))
                        if 'HVACTemplate:Plant:HotWaterLoop' in epjson.keys():
                            self.logger.warning(
                                'Warning: In {}'
                                ' an HVACTemplate:Plant:HotWaterLoop is also present.  All boilers with blank Template '
                                'Loop Type field will be connected to the Hot Water Loop.'
                                .format(object_type))
                    self.merge_epjson(
                        super_dictionary=self.templates_plant_loops,
                        object_dictionary={object_type: object_structure},
                        unique_name_override=False)
                elif re.match('^HVACTemplate:Plant:(Chiller|Tower|Boiler)(:ObjectReference)*$', object_type):
                    boiler_default_map = {
                        'HVACTemplate:Plant:Boiler': {
                            'fuel_type': 'NaturalGas',
                            'priority': '1',
                            'efficiency': 0.8,
                            'water_outlet_upper_temperature_limit': 100.0
                        },
                        'HVACTemplate:Plant:Boiler:ObjectReference': {
                            'boiler_object_type': 'Boiler:HotWater',
                            'priority': '1'
                        },
                        'HVACTemplate:Plant:Chiller': {
                            'condenser_type': 'WaterCooled'
                        },
                        'HVACTemplate:Plant:Chiller:ObjectReference': {
                            'chiller_object_type': 'Chiller:Electric:EIR',
                            'priority': '1'
                        },
                        'HVACTemplate:Plant:Tower:ObjectReference': {
                            'cooling_tower_object_type': 'CoolingTower:SingleSpeed'
                        },
                    }
                    for object_name, object_fields in object_structure.items():
                        # set defaults
                        selected_default_map = boiler_default_map.get(object_type)
                        if selected_default_map:
                            for field, default_value in selected_default_map.items():
                                if not object_fields.get(field):
                                    object_fields[field] = default_value
                    # Check boiler inputs
                    if object_type == 'HVACTemplate:Plant:Boiler':
                        for object_name, object_fields in object_structure.items():
                            if not object_fields.get('fuel_type') and \
                                    object_fields.get('boiler_type') != 'DistrictHotWater':
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) fuel_type must be specified when boiler_type is not '
                                    'DistrictHotWater'.format(object_type, object_name))
                    # Check tower inputs
                    if object_type == 'HVACTemplate:Plant:Tower':
                        for object_name, object_fields in object_structure.items():
                            high_speed_nominal_capacity = object_fields.get('high_speed_nominal_capacity', 'Autosize')
                            free_convection_capacity = object_fields.get('free_convection_capacity', 'Autosize')
                            if (str(high_speed_nominal_capacity).lower() == 'autosize' and str(
                                    free_convection_capacity).lower() != 'autosize') or \
                                    (str(high_speed_nominal_capacity).lower() != 'autosize' and str(
                                    free_convection_capacity).lower() == 'autosize'):
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) For a {} tower the high speed capacity and free '
                                    'convection capacity both need to be specified or set to autosize.'
                                    .format(object_type, object_name, object_fields.get('tower_type')))
                    # for plant equipment object references, add the referenced object to epjson for complex input resolution
                    #  later on.  For chiller objects, also identify condenser type and make it a template attribute.
                    elif object_type == 'HVACTemplate:Plant:Boiler:ObjectReference':
                        for object_name, object_fields in object_structure.items():
                            reference_object_structure = epjson.get(object_fields['boiler_object_type'])
                            if not reference_object_structure:
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) Referenced boiler not found: {}'
                                    .format(object_type, object_name, object_fields))
                            for reference_object_name, reference_object_fields in reference_object_structure.items():
                                if reference_object_name == object_fields['boiler_name']:
                                    if reference_object_fields.get('boiler_water_inlet_node_name', '') in ['', 'None']:
                                        raise InvalidTemplateException(
                                            'Error: In {} ({}) Blank Inlet Node Name found in referenced boiler: {}'
                                            .format(object_type, object_name, object_fields))
                                    if reference_object_fields.get('boiler_water_outlet_node_name', '') in ['', 'None']:
                                        raise InvalidTemplateException(
                                            'Error: In {} ({}) Blank Outlet Node Name found in referenced boiler: {}'
                                            .format(object_type, object_name, object_fields))
                                    if reference_object_fields.get('boiler_water_inlet_node_name') == \
                                            reference_object_fields.get('boiler_water_outlet_node_name'):
                                        raise InvalidTemplateException(
                                            'Error: in {} ({}) Duplicate hot water node name found in '
                                            'referenced boiler. All boiler inlet and outlet node names '
                                            'must be unique'
                                            .format(object_type, object_name))
                                    object_structure[object_name]['epjson'] = \
                                        {object_fields['boiler_object_type']: {reference_object_name: reference_object_fields}}
                                    break
                            if not object_structure[object_name].get('epjson'):
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) Referenced boiler not found: {}'
                                    .format(object_type, object_name, object_fields))
                    elif object_type == 'HVACTemplate:Plant:Chiller:ObjectReference':
                        for object_name, object_fields in object_structure.items():
                            reference_object_structure = epjson.get(object_fields['chiller_object_type'])
                            if not reference_object_structure:
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) Referenced chiller not found: {}'
                                    .format(object_type, object_name, object_fields))
                            for reference_object_name, reference_object_fields in reference_object_structure.items():
                                if reference_object_name == object_fields['chiller_name']:
                                    if reference_object_fields.get('chilled_water_inlet_node_name', '') in ['', 'None']:
                                        raise InvalidTemplateException(
                                            'Error: In {} ({}) Blank chilled water Inlet Node Name found in '
                                            'referenced chiller: {}'
                                            .format(object_type, object_name, object_fields))
                                    if reference_object_fields.get('chilled_water_outlet_node_name', '') in ['', 'None']:
                                        raise InvalidTemplateException(
                                            'Error: In {} ({}) Blank chilled water Outlet Node Name found in '
                                            'referenced chiller: {}'
                                            .format(object_type, object_name, object_fields))
                                    if reference_object_fields.get('chilled_water_inlet_node_name') == \
                                            reference_object_fields.get('chilled_water_outlet_node_name'):
                                        raise InvalidTemplateException(
                                            'Error: in {} ({}) Duplicate chilled water node name found in '
                                            'referenced chiller. All chiller inlet and outlet node names '
                                            'must be unique'
                                            .format(object_type, object_name))
                                    try:
                                        object_structure[object_name]['condenser_type'] = reference_object_fields['condenser_type']
                                    except (KeyError, AttributeError):
                                        object_structure[object_name]['condenser_type'] = 'WaterCooled'
                                    if object_structure[object_name]['condenser_type'] == 'WaterCooled':
                                        if reference_object_fields.get('condenser_inlet_node_name', '') in ['', 'None']:
                                            raise InvalidTemplateException(
                                                'Error: In {} ({}) Blank condenser water Inlet Node Name found in '
                                                'referenced chiller: {}'
                                                .format(object_type, object_name, object_fields))
                                        if reference_object_fields.get('condenser_outlet_node_name', '') in ['', 'None']:
                                            raise InvalidTemplateException(
                                                'Error: In {} ({}) Blank condenser water Outlet Node Name found in '
                                                'referenced chiller: {}'
                                                .format(object_type, object_name, object_fields))
                                        if reference_object_fields.get('condenser_inlet_node_name') == \
                                                reference_object_fields.get('condenser_outlet_node_name'):
                                            raise InvalidTemplateException(
                                                'Error: in {} ({}) Duplicate condenser water node name found in '
                                                'referenced chiller. All chiller inlet and outlet node names '
                                                'must be unique'
                                                .format(object_type, object_name))
                                    object_structure[object_name]['epjson'] = \
                                        {object_fields['chiller_object_type']: {reference_object_name: reference_object_fields}}
                                    break
                            if not object_structure[object_name].get('epjson'):
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) Referenced chiller not found: {}'
                                    .format(object_type, object_name, object_fields))
                    elif object_type == 'HVACTemplate:Plant:Tower:ObjectReference':
                        for object_name, object_fields in object_structure.items():
                            reference_object_structure = epjson.get(object_fields['cooling_tower_object_type'])
                            if not reference_object_structure:
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) Referenced tower not found: {}'
                                    .format(object_type, object_name, object_fields))
                            for reference_object_name, reference_object_fields in reference_object_structure.items():
                                if reference_object_name == object_fields['cooling_tower_name']:
                                    if reference_object_fields.get('water_inlet_node_name', '') in ['', 'None']:
                                        raise InvalidTemplateException(
                                            'Error: In {} ({}) Blank Inlet Node Name found in '
                                            'referenced chiller: {}'
                                            .format(object_type, object_name, object_fields))
                                    if reference_object_fields.get('water_outlet_node_name', '') in ['', 'None']:
                                        raise InvalidTemplateException(
                                            'Error: In {} ({}) Blank Outlet Node Name found in '
                                            'referenced chiller: {}'
                                            .format(object_type, object_name, object_fields))
                                    if reference_object_fields.get('water_inlet_node_name') == \
                                            reference_object_fields.get('water_outlet_node_name'):
                                        raise InvalidTemplateException(
                                            'Error: in {} ({}) Duplicate node name found in referenced tower.  '
                                            'All tower inlet and outlet node names must be unique'
                                            .format(object_type, object_name))
                                    object_structure[object_name]['epjson'] = \
                                        {object_fields['cooling_tower_object_type']: {reference_object_name: reference_object_fields}}
                                    break
                            if not object_structure[object_name].get('epjson'):
                                raise InvalidTemplateException(
                                    'Error: In {} ({}) Referenced tower not found: {}'
                                    .format(object_type, object_name, object_fields))
                    self.merge_epjson(
                        super_dictionary=self.templates_plant_equipment,
                        object_dictionary={object_type: object_structure},
                        unique_name_override=False)
                else:
                    raise InvalidTemplateException(
                        'Error: Template object type {} was not recognized'.format(object_type))
                # store original templates into dictionary
                self.merge_epjson(
                    super_dictionary=self.templates,
                    object_dictionary={object_type: object_structure},
                    unique_name_override=False)
            else:
                # store all non-template objects into a base epjson object.
                self.merge_epjson(
                    super_dictionary=self.base_objects,
                    object_dictionary={object_type: object_structure},
                    unique_name_override=False)
        return

    def _expand_templates(self, templates, expand_class, **kwargs):
        """
        Run Expand operations on multiple templates
        :param templates: dictionary of HVACTemplate:.* objects
        :param expand_class: ExpandObjects child class to operate on template (e.g. ExpandZone).
        :return: dictionary of expanded objects with unique name as key
        """
        expanded_template_dictionary = {}
        templates = self.epjson_genexp(templates)
        for template in templates:
            (_, template_structure), = template.items()
            (template_name, template_fields), = template_structure.items()
            external_epjson_objects = template_fields.pop('epjson', None)
            expanded_template = expand_class(
                template=template,
                epjson=external_epjson_objects,
                logger_level=self.logger_level,
                logger_name=self.logger_name,
                **kwargs).run()
            expanded_template_dictionary[template_name] = expanded_template
        return expanded_template_dictionary

    def _create_zonecontrol_thermostat(self, zone_class_object):
        """
        Create ZoneControl:Thermostat objects.  This operations is performed outside of ExpandObjects because it
        requires cross-referencing between HVACTemplate:Zone and HVACTemplate:Thermostat objects

        :param zone_class_object: ExpandZone object
        :return: Updated class epJSON dictionary with ThermostatSetpoint objects added.  Objects are also added
            to the class self.epsjon dictionary.
        """
        # Retreive the thermostat object
        try:
            thermostat_template_name = getattr(zone_class_object, 'template_thermostat_name')
        except AttributeError:
            self.logger.info(
                'In {} ({}) Zone object does not reference a thermostat class object'
                .format(zone_class_object.template_type, zone_class_object.unique_name))
            return
        except ValueError:
            raise InvalidTemplateException('Error: Zone template ({}) is improperly formatted.'
                                           .format(zone_class_object.unique_name))
        try:
            thermostat_object = self.expanded_thermostats[thermostat_template_name]
        except (ValueError, KeyError):
            raise InvalidTemplateException('Error: Thermostat object does not exist ({}) but is reference by '
                                           'zone template {}'
                                           .format(thermostat_template_name, zone_class_object.unique_name))
        # Evaluate the thermostat type in the thermostat object and format the output object accordingly
        try:
            zone_name = getattr(zone_class_object, 'zone_name')
            thermostat_epjson = {t_type: t_struct for t_type, t_struct
                                 in thermostat_object.epjson.items()
                                 if re.match(r'^ThermostatSetpoint.*', t_type)}
            (thermostat_type, thermostat_structure), = thermostat_epjson.items()
            (thermostat_name, _), = thermostat_structure.items()
            # create control schedule based on thermostat type
            if thermostat_type == "ThermostatSetpoint:SingleHeating":
                control_schedule = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)\
                    .build_compact_schedule(
                        structure_hierarchy=['Objects', 'Common', 'Objects', 'Schedule', 'Compact', 'ALWAYS_VAL'],
                        insert_values=[1, ])
            elif thermostat_type == "ThermostatSetpoint:SingleCooling":
                control_schedule = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)\
                    .build_compact_schedule(
                        structure_hierarchy=['Objects', 'Common', 'Objects', 'Schedule', 'Compact', 'ALWAYS_VAL'],
                        insert_values=[2, ])
            elif thermostat_type == "ThermostatSetpoint:DualSetpoint":
                control_schedule = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)\
                    .build_compact_schedule(
                        structure_hierarchy=['Objects', 'Common', 'Objects', 'Schedule', 'Compact', 'ALWAYS_VAL'],
                        insert_values=[4, ])
            else:
                raise InvalidTemplateException("Error: {} ({}) Invalid thermostat type set in ExpandThermostat"
                                               .format(thermostat_type, thermostat_object.unique_name))
            # create zonecontrol object
            (_, schedule_structure), = control_schedule.items()
            (schedule_name, _), = schedule_structure.items()
            zonecontrol_thermostat = {
                "ZoneControl:Thermostat": {
                    "{} Thermostat".format(zone_name): {
                        "control_1_name": thermostat_name,
                        "control_1_object_type": thermostat_type,
                        "control_type_schedule_name": schedule_name,
                        "zone_or_zonelist_name": "{}".format(zone_name)
                    }
                }
            }
            self.merge_epjson(
                super_dictionary=self.epjson,
                object_dictionary=dict(control_schedule, **zonecontrol_thermostat),
                unique_name_override=True
            )
            return dict(control_schedule, **zonecontrol_thermostat)
        except (ValueError, AttributeError, KeyError):
            raise InvalidTemplateException(
                "Error: HVACTemplate failed to build ZoneControl:Thermostat from zone template "
                "{}".format(zone_class_object.unique_name))  # pragma: no cover - catchall

    @staticmethod
    def _get_zone_template_field_from_system_type(template_type):
        """
        Retrieve the corresponding zone field name for a system template type
        :param template_type: HVACTemplate:System object type
        :return: zone field name
        """
        # get the zone field_name that will identify the system template name
        if re.match(r'HVACTemplate:System:ConstantVolume', template_type):
            zone_system_template_field_name = 'template_constant_volume_system_name'
        elif re.match(r'HVACTemplate:System:DedicatedOutdoorAir', template_type):
            zone_system_template_field_name = 'dedicated_outdoor_air_system_name'
        elif re.match(r'HVACTemplate:System:DualDuct', template_type):
            zone_system_template_field_name = 'template_dual_duct_system_name'
        elif re.match(r'HVACTemplate:System:Unitary.*', template_type):
            zone_system_template_field_name = 'template_unitary_system_name'
        elif re.match(r'HVACTemplate:System:.*VAV$', template_type):
            zone_system_template_field_name = 'template_vav_system_name'
        elif re.match(r'HVACTemplate:System:VRF', template_type):
            zone_system_template_field_name = 'template_vrf_system_name'
        else:
            raise InvalidTemplateException(
                "Error: Invalid system type passed to supply path creation function: {}".format(template_type))
        return zone_system_template_field_name

    def _create_system_path_connection_objects(self, system_class_object, expanded_zones):
        """
        Create objects connecting system supply air to zone objects.  An AirLoopHVAC:SupplyPath object is created with
        either an AirLoopHVAC:SupplyPlenum or an AirLoopHVAC:ZoneSplitter object.  The same is true for
        AirLoopHVAC:ReturnPath and AirLoopHVAC:ReturnPlenum/AirLoopHVAC:ZoneMixer.

        :param system_class_object: Expanded HVACTemplate:System:.* class object
        :param expanded_zones: dictionary of ExpandZone objects
        :return: system supply air connection objects.  AirLoopHVAC:SupplyPath object and either
            AirLoopHVAC:SupplyPlenum or AirLoopHVAC:ZoneSplitter object as well ass AirLoopHVAC:ReturnPath and either
            AirLoopHVAC:ReturnPlenum or AirLoopHVAC:ZoneMixer.
        """
        zone_system_template_field_name = \
            self._get_zone_template_field_from_system_type(template_type=system_class_object.template_type)
        # iterate over inlet node name types.  For DualDuct, this is two entries (hot/cold).  For all other systems,
        # this is a single value
        if system_class_object.template_type == 'HVACTemplate:System:DualDuct':
            inlet_nodes = ['cold_air_inlet_node_name', 'hot_air_inlet_node_name']
        else:
            inlet_nodes = ['air_inlet_node_name', ]
        # create ExpandObjects class object to use some yaml and epjson functions
        eo = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)
        eo.unique_name = getattr(system_class_object, 'template_name')
        # iterate over expanded zones and if the system reference field exists, and is for the referenced system,
        # append them in the splitter and mixer lists
        zone_return_plenums = []
        zone_induced_air_nodes = []
        for node_idx, inlet_node in enumerate(inlet_nodes):
            zone_splitters = []
            zone_mixers = []
            zone_supply_plenums = []
            for _, ez in expanded_zones.items():
                if getattr(ez, zone_system_template_field_name, None) == system_class_object.template_name:
                    if getattr(ez, 'flow_type', None) in ['SeriesFromPlenum', 'ParallelFromPlenum']:
                        zone_induced_air_node = ez.unique_name
                    else:
                        zone_induced_air_node = None
                    if getattr(ez, 'supply_plenum_name', None) or (
                            getattr(ez, 'cold_supply_plenum_name', None) and inlet_node == 'cold_air_inlet_node_name') or (
                            getattr(ez, 'hot_supply_plenum_name', None) and inlet_node == 'hot_air_inlet_node_name'):
                        try:
                            zone_supply_equipment = {'AirLoopHVAC:SupplyPlenum': ez.epjson['AirLoopHVAC:SupplyPlenum']}
                        except (KeyError, AttributeError):
                            raise InvalidTemplateException(
                                'Error: supply_plenum_name indicated for zone template {} but '
                                'AirLoopHVAC:SupplyPlenum was not created'.format(ez.unique_name))
                    else:
                        zone_supply_equipment = self.get_epjson_objects(
                            epjson=ez.epjson,
                            object_type_regexp=r'^AirTerminal:.*')
                    try:
                        (zone_supply_equipment_type, zone_supply_equipment_structure), = zone_supply_equipment.items()
                        (zone_supply_equipment_name, zone_supply_equipment_fields), = zone_supply_equipment_structure.items()

                        if zone_supply_equipment_type == 'AirLoopHVAC:SupplyPlenum':
                            outlet_node_name = zone_supply_equipment_fields['inlet_node_name']
                            zone_supply_plenums.append({
                                'component_name': zone_supply_equipment_name,
                                'component_object_type': zone_supply_equipment_type
                            })
                        elif zone_supply_equipment_type in ['AirTerminal:SingleDuct:SeriesPIU:Reheat',
                                                            'AirTerminal:SingleDuct:ParallelPIU:Reheat']:
                            # Raise error if inlet node name is overridden for multi-inlet node systems (DualDuct)
                            if len(inlet_nodes) > 1:
                                raise InvalidTemplateException(
                                    'Error: Series or Parallel PIU is being referenced '
                                    'by an invalid system {}'.format(system_class_object.template_type))
                            outlet_node_name = zone_supply_equipment_fields['supply_air_inlet_node_name']
                        else:
                            outlet_node_name = zone_supply_equipment_fields[inlet_node]
                    except (KeyError, AttributeError, ValueError):
                        raise InvalidTemplateException(
                            'Error: Search for zone equipment from Supply Path creation failed for '
                            'outlet node.  system {}, zone {}, zone equipment {}'
                            .format(system_class_object.template_name, ez.unique_name, zone_supply_equipment))
                    if getattr(ez, 'return_plenum_name', None):
                        try:
                            zone_return_equipment = {'AirLoopHVAC:ReturnPlenum': ez.epjson['AirLoopHVAC:ReturnPlenum']}
                        except (KeyError, AttributeError):
                            raise InvalidTemplateException(
                                'Error: return_plenum_name indicated for zone template {} but '
                                'AirLoopHVAC:ReturnPlenum was not created'.format(ez.unique_name))
                    else:
                        try:
                            zone_return_equipment = {'ZoneHVAC:EquipmentConnections': ez.epjson['ZoneHVAC:EquipmentConnections']}
                        except (KeyError, AttributeError, ValueError):
                            raise InvalidTemplateException(
                                'Error: Search for ZoneHVAC:EquipmentConnections object from Supply '
                                'Path creation failed for inlet node.  system {}, zone {}'
                                .format(system_class_object.template_name, ez.unique_name))
                    try:
                        (zone_return_equipment_type, zone_return_equipment_structure), = zone_return_equipment.items()
                        (zone_return_equipment_name, zone_return_equipment_fields), = zone_return_equipment_structure.items()
                        if zone_return_equipment_type == 'AirLoopHVAC:ReturnPlenum':
                            inlet_node_name = zone_return_equipment_fields['outlet_node_name']
                            # use node_idx to prevent multiple zone_return_plenum objects from being created in dualduct zones
                            if node_idx == 0:
                                zone_return_plenums.append({
                                    'component_name': zone_return_equipment_name,
                                    'component_object_type': zone_return_equipment_type
                                })
                        else:
                            inlet_node_name = zone_return_equipment_fields['zone_return_air_node_or_nodelist_name']
                    except (KeyError, AttributeError, ValueError):
                        raise InvalidTemplateException(
                            'Error: Search for zone equipment from Return Path creation failed for '
                            'inlet node.  system {}, zone {}, zone equipment {}'
                            .format(system_class_object.template_name, ez.unique_name, zone_return_equipment))
                    zone_splitters.append(
                        {
                            "outlet_node_name": outlet_node_name
                        }
                    )
                    zone_mixers.append(
                        {
                            "inlet_node_name": inlet_node_name
                        }
                    )
                    if zone_induced_air_node:
                        # This is for PIU objects that use SeriesFromPlenum or ParallelFromPlenum
                        zone_induced_air_nodes.append(
                            {
                                "node_name": '{} Return'.format(zone_induced_air_node)
                            }
                        )
            # create plenums or spliters/mixers, depending on template inputs
            supply_object = None
            supply_plenum_name = getattr(system_class_object, 'supply_plenum_name', None)
            cold_supply_plenum_name = getattr(system_class_object, 'cold_supply_plenum_name', None)
            hot_supply_plenum_name = getattr(system_class_object, 'hot_supply_plenum_name', None)
            if system_class_object.template_type == 'HVACTemplate:System:DualDuct' and \
                    cold_supply_plenum_name and inlet_node.startswith('cold_air'):
                eo.cold_supply_plenum_name = cold_supply_plenum_name
                cold_supply_object = eo.get_structure(structure_hierarchy=[
                    'AutoCreated', 'System', 'AirLoopHVAC', 'SupplyPlenum', 'DualDuct', 'Cold'])
                cold_supply_object['nodes'] = zone_splitters
                supply_object = {'AirLoopHVAC:SupplyPlenum': cold_supply_object}
            elif system_class_object.template_type == 'HVACTemplate:System:DualDuct' and \
                    hot_supply_plenum_name and inlet_node.startswith('hot_air'):
                eo.hot_supply_plenum_name = hot_supply_plenum_name
                hot_supply_object = eo.get_structure(structure_hierarchy=[
                    'AutoCreated', 'System', 'AirLoopHVAC', 'SupplyPlenum', 'DualDuct', 'Hot'])
                hot_supply_object['nodes'] = zone_splitters
                supply_object = {'AirLoopHVAC:SupplyPlenum': hot_supply_object}
            elif supply_plenum_name:
                # set return plenum name attribute for transition and mapping processing
                eo.supply_plenum_name = supply_plenum_name
                supply_object = eo.get_structure(structure_hierarchy=[
                    'AutoCreated', 'System', 'AirLoopHVAC', 'SupplyPlenum', 'Base'])
                supply_object['nodes'] = zone_splitters
                supply_object = {'AirLoopHVAC:SupplyPlenum': supply_object}
            else:
                supply_object = eo.get_structure(structure_hierarchy=[
                    'AutoCreated', 'System', 'AirLoopHVAC', 'ZoneSplitter', 'Base'])
                supply_object['nodes'] = zone_splitters
                supply_object = {'AirLoopHVAC:ZoneSplitter': supply_object}
            # Add Path objects
            supply_path_object = {'AirLoopHVAC:SupplyPath':
                                  eo.get_structure(structure_hierarchy=[
                                      'AutoCreated', 'System', 'AirLoopHVAC', 'SupplyPath', 'Base'])}
            # add zone supply plenums if they were created
            if zone_supply_plenums:
                (_, supply_path_object_fields), = supply_path_object.items()
                supply_path_object_fields['components'].extend(zone_supply_plenums)
            # Rename objects if multi-inlet node system is used
            if system_class_object.template_type == 'HVACTemplate:System:DualDuct':
                (_, supply_object_fields), = supply_object.items()
                (_, supply_path_object_fields), = supply_path_object.items()
                if inlet_node.startswith('cold_air'):
                    supply_object_fields['name'] = supply_object_fields['name'].replace('{}', '{} Cold')
                    supply_object_fields['inlet_node_name'] = supply_object_fields['inlet_node_name'].replace('{}', '{} Cold')
                    supply_path_object_fields['name'] = supply_path_object_fields['name'].replace('{}', '{} Cold')
                if inlet_node.startswith('hot_air'):
                    supply_object_fields['name'] = supply_object_fields['name'].replace('{}', '{} Hot')
                    supply_object_fields['inlet_node_name'] = supply_object_fields['inlet_node_name'].replace('{}', '{} Hot')
                    supply_path_object_fields['name'] = supply_path_object_fields['name'].replace('{}', '{} Hot')
            path_dictionary = eo.yaml_list_to_epjson_dictionaries(
                yaml_list=[supply_object, supply_path_object])
            resolved_path_dictionary = eo.resolve_objects(epjson=path_dictionary)
            # save output to class epsjon
            self.merge_epjson(
                super_dictionary=self.epjson,
                object_dictionary=resolved_path_dictionary)
        # Create return objects
        return_plenum_name = getattr(system_class_object, 'return_plenum_name', None)
        return_nodelist = {}
        if return_plenum_name:
            # set return plenum name attribute for transition and mapping processing
            eo.return_plenum_name = return_plenum_name
            return_object = eo.get_structure(structure_hierarchy=[
                'AutoCreated', 'System', 'AirLoopHVAC', 'ReturnPlenum', 'Base'])
            return_object['nodes'] = zone_mixers
            return_object = {'AirLoopHVAC:ReturnPlenum': return_object}
            if zone_induced_air_nodes:
                return_object['AirLoopHVAC:ReturnPlenum']['induced_air_outlet_node_or_nodelist_name'] = \
                    '{} Induced Air Nodes'.format(system_class_object.template_name)
                return_nodelist = {
                    'NodeList': {
                        'name': '{} Induced Air Nodes'.format(system_class_object.template_name),
                        "nodes": zone_induced_air_nodes
                    }
                }
        else:
            return_object = eo.get_structure(structure_hierarchy=[
                'AutoCreated', 'System', 'AirLoopHVAC', 'ZoneMixer', 'Base'])
            return_object['nodes'] = zone_mixers
            return_object = {'AirLoopHVAC:ZoneMixer': return_object}
        # Add Path objects
        return_path_object = {
            'AirLoopHVAC:ReturnPath':
            eo.get_structure(structure_hierarchy=[
                'AutoCreated', 'System', 'AirLoopHVAC', 'ReturnPath', 'Base'])}
        # add zone return plenums if they were created
        if zone_return_plenums:
            (_, return_path_object_fields), = return_path_object.items()
            # only take the first item, subsequent items are only duplicates from dualduct zone templates
            return_path_object_fields['components'] = zone_return_plenums + return_path_object_fields['components']
        path_dictionary = eo.yaml_list_to_epjson_dictionaries(
            yaml_list=[return_object, return_path_object, return_nodelist])
        resolved_path_dictionary = eo.resolve_objects(epjson=path_dictionary)
        # save output to class epsjon
        self.merge_epjson(
            super_dictionary=self.epjson,
            object_dictionary=resolved_path_dictionary)
        return resolved_path_dictionary

    def _create_system_vrf_path_connection_objects(self, system_class_object, expanded_zones):
        """
        Create objects connecting VRF system to zone objects.

        :param system_class_object: Expanded HVACTemplate:System:.* class object
        :param expanded_zones: dictionary of ExpandZone objects
        :return: system supply air connection objects.  AirLoopHVAC:SupplyPath object and either
            AirLoopHVAC:SupplyPlenum or AirLoopHVAC:ZoneSplitter object as well ass AirLoopHVAC:ReturnPath and either
            AirLoopHVAC:ReturnPlenum or AirLoopHVAC:ZoneMixer.
        """
        # create ExpandObjects class object to use some yaml and epjson functions
        eo = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)
        eo.unique_name = getattr(system_class_object, 'template_name')
        vrf_object_name_list = []
        zone_system_template_field_name = \
            self._get_zone_template_field_from_system_type(template_type=system_class_object.template_type)
        for _, ez in expanded_zones.items():
            if getattr(ez, zone_system_template_field_name, None) == system_class_object.template_name:
                try:
                    vrf_object = ez.epjson['ZoneHVAC:TerminalUnit:VariableRefrigerantFlow']
                    (vrf_object_name, _), = vrf_object.items()
                except (KeyError, AttributeError):
                    raise InvalidTemplateException(
                        "Error: VRF zone template {} expanded with no "
                        "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object".format(ez.unique_name))
                except ValueError:
                    raise InvalidTemplateException(
                        'ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object incorrectly formatted: {}'
                        .format(ez.epjson.get('ZoneHVAC:TerminalUnit:VariableRefrigerantFlow', 'None')))
                vrf_object_name_list.append({'zone_terminal_unit_name': vrf_object_name})
        if vrf_object_name_list:
            vrf_terminal_object = eo.get_structure(structure_hierarchy=[
                'AutoCreated', 'System', 'ZoneTerminalUnitList', 'Base'])
            vrf_terminal_object['terminal_units'] = vrf_object_name_list
            path_dictionary = eo.yaml_list_to_epjson_dictionaries(
                yaml_list=[{'ZoneTerminalUnitList': vrf_terminal_object}, ])
            resolved_path_dictionary = eo.resolve_objects(epjson=path_dictionary)
            # save output to class epsjon
            self.merge_epjson(
                super_dictionary=self.epjson,
                object_dictionary=resolved_path_dictionary)
        else:
            raise InvalidTemplateException(
                'Error: Failed to create VRF terminal unit list for {}'.format(system_class_object.template_name))
        return

    def _create_templates_from_plant_equipment(self, plant_equipment_class_object, expanded_plant_loops):
        """
        Create plant and platn equipment loop templates from ExpandPlantEquipment object attributes.
        These outputs will be used as inputs to the initialize new ExpandPlantLoop and ExpandPlantLoopEquipment classes.
        This process must be performed because ExpandPlantLoop must be
        run before ExpandPlantEquipment.  However, certain equipment inputs can cause for new loops to be created.

        :param plant_equipment_class_object: ExpandPlantEquipment class object
        :param expanded_plant_loops: ExpandPlantLoop objects
        :return: Array of Dictionary of HVAC:Template:Plant template objects to create an ExpandPlantLoop object
        """
        # create dictionary to store plant loops
        plant_loop_dictionary = {}
        plant_equipment_dictionary = {}
        # get each loop type specified in the existing plant loop class objects
        plant_loops = [getattr(pl, 'template_type').lower() for pl in expanded_plant_loops.values()]
        # create condenser water loop for water cooled condensers
        if getattr(plant_equipment_class_object, 'template_type', None).lower() in \
                ['hvactemplate:plant:chiller', 'hvactemplate:plant:chiller:objectreference'] \
                and getattr(plant_equipment_class_object, 'condenser_type', 'WaterCooled').lower() == 'watercooled' \
                and 'hvactemplate:plant:condenserwaterloop' not in plant_loops \
                and getattr(plant_equipment_class_object, 'chiller_type', None) != 'DistrictChilledWater':
            # try to get the chilled water loop attributes to transition to condenser water
            chw_loop = [
                pl for pl
                in expanded_plant_loops.values()
                if getattr(pl, 'template_type').lower() == 'hvactemplate:plant:chilledwaterloop']
            cndw_attributes = {}
            # transfer ChilledWaterLoop attributes to CondenserWaterLoop
            if chw_loop:
                for cndw_attribute, chw_attribute in zip(
                        ['condenser_water_pump_rated_head', 'condenser_water_design_setpoint',
                         'condenser_plant_operation_scheme_type', 'condenser_equipment_operation_schemes_name',
                         'condenser_water_temperature_control_type', 'condenser_water_setpoint_schedule_name',
                         'pump_schedule_name', 'pump_control_type', 'condenser_water_pump_type',
                         'condenser_water_supply_side_bypass_pipe', 'condenser_water_demand_side_bypass_pipe',
                         'condenser_water_load_distribution_scheme'],
                        ['condenser_water_pump_rated_head', 'condenser_water_design_setpoint',
                         'condenser_plant_operation_scheme_type', 'condenser_equipment_operation_schemes_name',
                         'condenser_water_temperature_control_type', 'condenser_water_setpoint_schedule_name',
                         'pump_schedule_name', 'pump_control_type', 'condenser_water_pump_type',
                         'condenser_water_supply_side_bypass_pipe', 'condenser_water_demand_side_bypass_pipe',
                         'condenser_water_load_distribution_scheme']):
                    try:
                        cndw_attributes[cndw_attribute] = getattr(chw_loop[0], chw_attribute)
                    except AttributeError:
                        self.logger.debug('Chilled water attribute {} not set by user, using default for '
                                          'condenser water'.format(chw_attribute))
            cndw_attributes['template_plant_loop_type'] = 'CondenserWaterLoop'
            self.merge_epjson(
                super_dictionary=plant_loop_dictionary,
                object_dictionary={
                    'HVACTemplate:Plant:CondenserWaterLoop': {
                        'Condenser Water Loop': cndw_attributes
                    }
                })
            # append plant loop to list to prevent another one being added.
            plant_loops.append('hvactemplate:plant:condenserwaterloop')
        return plant_loop_dictionary, plant_equipment_dictionary

    def _create_additional_plant_loops_and_equipment_from_equipment(
            self,
            expanded_plant_equipment,
            expanded_plant_loops):
        """
        Create additional HVACTemplate:Plant:.*Loops based on HVACTemplate:Plant:(Chiller|Tower|Boiler) inputs

        :param expanded_plant_equipment: ExpandPlantEquipment objects
        :param expanded_plant_loops: ExpandPlantLoop objects
        :return: Additional plant loop and equipment templates and objects added to expanded classes attributes
        """
        # create deepcopy to iterate over because the expanded_plant_equipment object may change size during iteration
        epe = copy.deepcopy(expanded_plant_equipment)
        for epl_name, epl in epe.items():
            plant_loop_template, plant_equipment_template = self._create_templates_from_plant_equipment(
                plant_equipment_class_object=epl,
                expanded_plant_loops=expanded_plant_loops)
            # If a plant loop was created, reprocess it here.
            if plant_loop_template:
                # add new plant loop to the templates
                for tmpl in [self.templates, self.templates_plant_loops]:
                    self.merge_epjson(
                        super_dictionary=tmpl,
                        object_dictionary=plant_loop_template
                    )
                # Expand new plant loop and add to the class objects
                additional_plant_loops = self._expand_templates(
                    templates=plant_loop_template,
                    expand_class=ExpandPlantLoop
                )
                try:
                    for expanded_name, expanded_object in additional_plant_loops.items():
                        if expanded_name not in expanded_plant_loops.keys():
                            expanded_plant_loops[expanded_name] = expanded_object
                except (AttributeError, ValueError):
                    InvalidTemplateException(
                        'Error: A Plant loop was specified to be created from a plant equipment object '
                        '{}, but the process failed to attach the created objects'.format(epl_name))
            # if a plant equipment template was created, process it here
            if plant_equipment_template:
                # add new plant equipment to the templates
                for tmpl in [self.templates, self.templates_plant_equipment]:
                    self.merge_epjson(
                        super_dictionary=tmpl,
                        object_dictionary=plant_equipment_template
                    )
                # Expand new plant equipment and add to the class objects
                # pass updated expanded_plant_loops to the class initialization as well.
                additional_plant_equipment = self._expand_templates(
                    templates=plant_equipment_template,
                    expand_class=ExpandPlantEquipment,
                    plant_loop_class_objects=expanded_plant_loops
                )
                try:
                    for expanded_name, expanded_object in additional_plant_equipment.items():
                        if expanded_name not in expanded_plant_loops.keys():
                            expanded_plant_equipment[expanded_name] = expanded_object
                except (AttributeError, ValueError):
                    raise InvalidTemplateException(
                        'Error: A Plant equipment was specified to be created from a plant '
                        'equipment object {}, but the process failed to attach the create objects'.format(epl_name))
        return

    @staticmethod
    def _get_plant_equipment_waterloop_branches_by_loop_type(
            plant_loop_class_object,
            expanded_plant_equipment):
        """
        Extract plant equipment branches by loop type and store in epJSON formatted dictionary

        :param plant_loop_class_object: ExpandPlantLoop object
        :param expanded_plant_equipment: dictionary of ExpandPlantEquipment objects
        :return: epJSON formatted dictionary of branch objects for loop connections
        """
        branch_dictionary = {}
        for pe in expanded_plant_equipment.values():
            branch_objects = copy.deepcopy(pe.epjson.get('Branch', {}))
            for branch_name, branch_structure in branch_objects.items():
                components = branch_structure.get('components')
                if not components:
                    raise InvalidTemplateException(
                        'Error: In {} ({}) A branch object failed to create component fields {}'
                        .format(pe.template_type, pe.template_name, branch_name))
            # Special handling for chillers with condenser water and chilled water branches
            # todo_eo: Currently the chilled and condenser water branches are separated by parsing the names.  A more
            #  robust solution should be investigated.
            if pe.template_type in ['HVACTemplate:Plant:Chiller', 'HVACTemplate:Plant:Chiller:ObjectReference'] \
                    and getattr(pe, 'condenser_type', 'WaterCooled') == 'WaterCooled':
                for branch_name, branch_structure in branch_objects.items():
                    if 'chilledwater' in plant_loop_class_object.template_type.lower() and 'chw' in branch_name.lower():
                        branch_dictionary.update({branch_name: branch_objects[branch_name]})
                    if 'condenserwater' in plant_loop_class_object.template_type.lower() and 'cnd' in branch_name.lower():
                        branch_dictionary.update({branch_name: branch_objects[branch_name]})
            # typical handling when all plant equipment branches belong in one loop
            elif pe.template_plant_loop_type in plant_loop_class_object.template_type:
                branch_dictionary.update(branch_objects)
        if branch_dictionary:
            return {'Branch': branch_dictionary}
        else:
            return None

    @staticmethod
    def _get_zone_system_waterloop_branches_by_loop_type(
            plant_loop_class_object,
            expanded_zones,
            expanded_systems):
        """
        Extract zone and system branch objects by loop type and store in epJSON formatted dictionary

        :param plant_loop_class_object: ExpandPlantLoop class object
        :param expanded_zones: ExpandZone objects
        :param expanded_systems: ExpandSystem objects
        :return: epJSON formatted dictionary of branch objects
        """
        # create list of regex matches for the given loop
        if 'chilledwater' in plant_loop_class_object.template_type.lower():
            branch_rgx = ['^Coil:Cooling:Water($|:DetailedGeometry)+', ]
        elif 'hotwater' in plant_loop_class_object.template_type.lower():
            branch_rgx = ['^Coil:Heating:Water($|:DetailedGeometry)+', '^ZoneHVAC:Baseboard.*Water']
        elif 'mixedwater' in plant_loop_class_object.template_type.lower():
            branch_rgx = ['^Coil:.*HeatPump.*', '^AirConditioner:VariableRefrigerantFlow$']
        elif 'condenserwater' in plant_loop_class_object.template_type.lower():
            return None
        else:
            InvalidTemplateException('an invalid loop type was specified when creating plant loop connections: {}'
                                     .format(plant_loop_class_object.template_type))
        branch_dictionary = {}
        object_list = [expanded_zones or {}, expanded_systems or {}]
        for class_object in object_list:
            for co in class_object.values():
                branch_objects = copy.deepcopy(co.epjson.get('Branch', {}))
                for branch_name, branch_structure in branch_objects.items():
                    # the regex check for 'main branch' is to avoid DualDuct main branches from accidentally being
                    # included since they have coil objects in them as well.  They typical main branch is never accidentally
                    # caught because the coil objects are never in the 0th position.
                    for br in branch_rgx:
                        if re.match(br, branch_structure['components'][0]['component_object_type']) and not \
                                re.match('.*main branch$', branch_name.lower()):
                            branch_dictionary.update({branch_name: branch_objects[branch_name]})
        if branch_dictionary:
            return {'Branch': branch_dictionary}
        else:
            return None

    def _split_supply_and_demand_side_branches(
            self,
            plant_loop_class_object,
            expanded_plant_equipment,
            expanded_systems,
            expanded_zones):
        """
        Separate plant equipment, zone, and system branches into supply and demand sides for a given ExpandPlantLoop
        object.

        :param plant_loop_class_object: ExpandPlantLoop class object
        :param expanded_plant_equipment: expanded dictionary of ExpandPlantEquipment objects
        :param expanded_systems: expanded dictionary of ExpandSystem objects
        :param expanded_zones: expanded dictionary of ExpandZone objects
        :return: tuple of demand and supply side branches for processing
        """
        # Get plant equipment, zone, and system branches
        plant_equipment_branch_dictionary = self._get_plant_equipment_waterloop_branches_by_loop_type(
            plant_loop_class_object=plant_loop_class_object,
            expanded_plant_equipment=expanded_plant_equipment
        )
        zone_system_branch_dictionary = self._get_zone_system_waterloop_branches_by_loop_type(
            plant_loop_class_object=plant_loop_class_object,
            expanded_zones=expanded_zones,
            expanded_systems=expanded_systems
        )
        # get branches in the loop
        demand_branches = {}
        # Special handling for condenser water loop where the chiller objects are the demand side.
        if 'condenserwater' in plant_loop_class_object.template_type.lower():
            pebd = copy.deepcopy(plant_equipment_branch_dictionary)
            for object_name, object_structure in plant_equipment_branch_dictionary['Branch'].items():
                try:
                    if re.match(r'Chiller:.*', object_structure['components'][0]['component_object_type']):
                        demand_branches.update({object_name: pebd['Branch'].pop(object_name)})
                except (AttributeError, KeyError):
                    raise InvalidTemplateException(
                        'Error: Branch object is incorrectly formatted: {}'.format(plant_equipment_branch_dictionary))
            supply_branches = pebd['Branch']
        else:
            demand_branches = zone_system_branch_dictionary.get('Branch') if zone_system_branch_dictionary else None
            supply_branches = plant_equipment_branch_dictionary.get('Branch') \
                if plant_equipment_branch_dictionary else None
        return demand_branches, supply_branches

    def _create_water_loop_connectors_and_nodelist(
            self,
            plant_loop_class_object,
            expanded_plant_equipment,
            expanded_zones=None,
            expanded_systems=None):
        """
        Create Branchlist, Connector, ConnectorList, and supply NodeLists objects that connect the PlantLoop to supply
        and demand water objects.  This operation is performed outside of ExpandObjects because it requires outputs
        from ExpandPlantEquipment, ExpandZone, and ExpandSystem objects.

        :param plant_loop_class_object: ExpandPlantLoop class object
        :param expanded_plant_equipment: expanded dictionary of ExpandPlantEquipment objects
        :param expanded_systems: expanded dictionary of ExpandSystem objects
        :param expanded_zones: expanded dictionary of ExpandZone objects
        :return: Updated class epjson attribute with Branchlist, Connector, and ConnectorList objects.
        """
        # Get plant equipment, zone, and system branches.  Split them into demand and supply sides
        demand_branches, supply_branches = self._split_supply_and_demand_side_branches(
            plant_loop_class_object=plant_loop_class_object,
            expanded_plant_equipment=expanded_plant_equipment,
            expanded_systems=expanded_systems,
            expanded_zones=expanded_zones
        )
        # check to make sure loops aren't empty
        if demand_branches:
            if plant_loop_class_object.template_type == 'HVACTemplate:Plant:ChilledWaterLoop':
                try:
                    equipment_types = [
                        (component[-1]['component_name'], component[-1]['component_object_type']) for
                        object_name, object_structure in supply_branches.items()
                        for component in object_structure.values()]
                except AttributeError:
                    raise PyExpandObjectsYamlStructureException(
                        'Error: In {} ({}) No supply branches found plant loop object'
                        .format(plant_loop_class_object.template_type, plant_loop_class_object.unique_name))

                chillers = [i for i in equipment_types if re.match(r'Chiller:.*', i[1])]
                towers = [i for i in equipment_types if re.match(r'CoolingTower:.*', i[1])]
                # For water-cooled chillers, the tower is in the condenserloop so that needs to be checked instead of
                #  the chilledwaterloop
                if 'CondenserWaterLoop' in [
                    ep_structure.template_plant_loop_type for ep_name, ep_structure in expanded_plant_equipment.items()
                        if ep_structure.template_type in ['HVACTemplate:Plant:Tower',
                                                          'HVACTemplate:Plant:Tower:ObjectReference']]:
                    towers = True
                if chillers and not towers and 'CondenserWaterLoop' in [
                        ep_structure.template_plant_loop_type
                        for ep_name, ep_structure in expanded_plant_equipment.items()]:
                    raise InvalidTemplateException(
                        'Error: In {} ({})'
                        ' there is one or more water cooled chiller(s) but there are no towers serving this loop.'
                        .format(plant_loop_class_object.template_type, plant_loop_class_object.unique_name))
        if not demand_branches or not supply_branches:
            msg = []
            if not demand_branches:
                msg.append('There is no demand-side equipment connected to this loop.')
            if not supply_branches:
                msg.append('There is no supply-side equipment serving this loop.')
            raise InvalidTemplateException(
                'Error: in {} ({}). {}'
                .format(plant_loop_class_object.template_type, plant_loop_class_object.unique_name,
                        ' '.join(msg)))
        # Use ExpandObjects class for helper functions
        eo = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)
        eo.unique_name = getattr(plant_loop_class_object, 'template_name')
        # create connector objects based on template attributes
        if (plant_loop_class_object.template_type == 'HVACTemplate:Plant:ChilledWaterLoop' and getattr(
                plant_loop_class_object, 'chilled_water_supply_side_bypass_pipe', 'Yes') == 'No') or \
                (plant_loop_class_object.template_type == 'HVACTemplate:Plant:CondenserWaterLoop' and getattr(
                plant_loop_class_object, 'condenser_water_supply_side_bypass_pipe', 'Yes') == 'No') or \
                (plant_loop_class_object.template_type == 'HVACTemplate:Plant:HotWaterLoop' and getattr(
                plant_loop_class_object, 'supply_side_bypass_pipe', 'Yes') == 'No') or \
                (plant_loop_class_object.template_type == 'HVACTemplate:Plant:MixedWaterLoop' and getattr(
                plant_loop_class_object, 'supply_side_bypass_pipe', 'Yes') == 'No'):
            supply_branchlist = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'BranchList', 'SupplyNoBypass'])
            connector_supply_mixer = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Mixer', 'SupplyNoBypass'])
            connector_supply_splitter = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Splitter', 'SupplyNoBypass'])
            # set the 'branches' value type to list if it's none
            if not connector_supply_mixer['branches']:
                connector_supply_splitter['branches'] = []
            if not connector_supply_splitter['branches']:
                connector_supply_mixer['branches'] = []
        else:
            supply_branchlist = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'BranchList', 'Supply'])
            connector_supply_mixer = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Mixer', 'Supply'])
            connector_supply_splitter = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Splitter', 'Supply'])
        if (plant_loop_class_object.template_type == 'HVACTemplate:Plant:ChilledWaterLoop' and getattr(
                plant_loop_class_object, 'chilled_water_demand_side_bypass_pipe', 'Yes') == 'No') or \
                (plant_loop_class_object.template_type == 'HVACTemplate:Plant:CondenserWaterLoop' and getattr(
                plant_loop_class_object, 'condenser_water_demand_side_bypass_pipe', 'Yes') == 'No') or \
                (plant_loop_class_object.template_type == 'HVACTemplate:Plant:HotWaterLoop' and getattr(
                    plant_loop_class_object, 'demand_side_bypass_pipe', 'Yes') == 'No') or \
                (plant_loop_class_object.template_type == 'HVACTemplate:Plant:MixedWaterLoop' and getattr(
                    plant_loop_class_object, 'demand_side_bypass_pipe', 'Yes') == 'No'):
            demand_branchlist = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'BranchList', 'DemandNoBypass'])
            connector_demand_splitter = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Splitter', 'DemandNoBypass'])
            connector_demand_mixer = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Mixer', 'DemandNoBypass'])
            # set the 'branches' value type to list if it's none
            if not connector_demand_mixer['branches']:
                connector_demand_splitter['branches'] = []
            if not connector_demand_splitter['branches']:
                connector_demand_mixer['branches'] = []
        else:
            demand_branchlist = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'BranchList', 'Demand'])
            connector_demand_splitter = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Splitter', 'Demand'])
            connector_demand_mixer = eo.get_structure(
                structure_hierarchy=['AutoCreated', 'PlantLoop', 'Connector', 'Mixer', 'Demand'])
        # create supply nodelist
        supply_nodelist = eo.get_structure(
            structure_hierarchy=['AutoCreated', 'PlantLoop', 'NodeList', 'Supply'])
        # apply branches
        try:
            for branch in demand_branches:
                demand_branchlist['branches'].insert(1, {'branch_name': branch})
                connector_demand_splitter['branches'].append({'outlet_branch_name': branch})
                connector_demand_mixer['branches'].append({'inlet_branch_name': branch})
            for branch in supply_branches:
                supply_branchlist['branches'].insert(1, {'branch_name': branch})
                connector_supply_splitter['branches'].insert(-1, {'outlet_branch_name': branch})
                connector_supply_mixer['branches'].insert(-1, {'inlet_branch_name': branch})
                supply_nodelist['nodes'].insert(
                    0,
                    {'node_name': supply_branches[branch]['components'][-1]['component_outlet_node_name']})
        except (KeyError, AttributeError):
            raise PyExpandObjectsYamlStructureException(
                'Error: In {} AutoCreated PlantLoop Connector YAML object was '
                'improperly formatted'.format(plant_loop_class_object.template_type))
        # add connector list
        demand_connectorlist = eo.get_structure(
            structure_hierarchy=['AutoCreated', 'PlantLoop', 'ConnectorList', 'Demand']
        )
        supply_connectorlist = eo.get_structure(
            structure_hierarchy=['AutoCreated', 'PlantLoop', 'ConnectorList', 'Supply']
        )
        # format yaml objects into epJSON dictionaries, resolve, and output
        connector_dictionary = eo.yaml_list_to_epjson_dictionaries(
            yaml_list=[
                {'BranchList': demand_branchlist},
                {'BranchList': supply_branchlist},
                {'Connector:Splitter': connector_demand_splitter},
                {'Connector:Splitter': connector_supply_splitter},
                {'Connector:Mixer': connector_demand_mixer},
                {'Connector:Mixer': connector_supply_mixer},
                {'ConnectorList': demand_connectorlist},
                {'ConnectorList': supply_connectorlist},
                {'NodeList': supply_nodelist}
            ])
        resolved_path_dictionary = eo.resolve_objects(epjson=connector_dictionary)
        # save output to class epsjon
        self.merge_epjson(
            super_dictionary=self.epjson,
            object_dictionary=resolved_path_dictionary
        )
        return

    def _create_plant_equipment_lists(
            self,
            plant_loop_class_object,
            expanded_plant_equipment):
        """
        Create PlantEquipmentList and CondenserEquipmentList for a given ExpandPlantLoop class object.
        This operation is performed outside of ExpandObjects because it requires outputs from
        ExpandPlantEquipment objects.

        :param plant_loop_class_object: ExpandPlantLoop class object
        :param expanded_plant_equipment: expanded dictionary of ExpandPlantEquipment objects
        :return: Updated class epjson attribute with PlantEquipmentList or CondenserEquipmentlist.
        """
        # Get plant equipment, zone, and system branches.  Split them into demand and supply sides
        _, supply_branches = self._split_supply_and_demand_side_branches(
            plant_loop_class_object=plant_loop_class_object,
            expanded_plant_equipment=expanded_plant_equipment,
            expanded_systems=None,
            expanded_zones=None
        )
        equipment = []
        # Extract priority from each equipment object referenced by the branch and use it to order the equipment list
        supply_branches_with_priority = []
        for sb in supply_branches.values():
            for equipment_name, equipment_class in expanded_plant_equipment.items():
                if equipment_class.template_type == 'HVACTemplate:Plant:Boiler:ObjectReference':
                    equipment_name = equipment_class.boiler_name
                elif equipment_class.template_type == 'HVACTemplate:Plant:Chiller:ObjectReference':
                    equipment_name = equipment_class.chiller_name
                elif equipment_class.template_type == 'HVACTemplate:Plant:Tower:ObjectReference':
                    equipment_name = equipment_class.cooling_tower_name
                if sb['components'][-1]['component_name'] == equipment_name:
                    # make tuple of (object, priority)
                    # if priority isn't set, use infinity to push it to the end when sorted
                    supply_branches_with_priority.append((sb, getattr(equipment_class, 'priority', float('inf'))))
        supply_branches_ordered = [
            branch for branch, priority
            in sorted(supply_branches_with_priority, key=lambda s: s[1])]
        for sb in supply_branches_ordered:
            equipment.append({
                'equipment_name': sb['components'][-1]['component_name'],
                'equipment_object_type': sb['components'][-1]['component_object_type']
            })
        # use ExpandObjects functions
        eo = ExpandObjects(logger_level=self.logger_level, logger_name=self.logger_name)
        eo.unique_name = getattr(plant_loop_class_object, 'template_name')
        if 'hotwater' in plant_loop_class_object.template_type.lower() or \
                'chilledwater' in plant_loop_class_object.template_type.lower():
            list_dictionary = \
                eo.get_structure(structure_hierarchy=['AutoCreated', 'PlantLoop', 'PlantEquipmentList'])
            list_dictionary['equipment'] = equipment
            equipment_list_dictionary = [{'PlantEquipmentList': list_dictionary}, ]
        elif 'mixedwater' in plant_loop_class_object.template_type.lower():
            heating_equipment = [i for i in equipment if re.match(r'Boiler:.*', i['equipment_object_type'])]
            heating_list_dictionary = \
                eo.get_structure(structure_hierarchy=['AutoCreated', 'PlantLoop', 'PlantEquipmentListMixedWaterHeating'])
            heating_list_dictionary['equipment'] = heating_equipment
            cooling_equipment = [i for i in equipment if re.match(r'CoolingTower:.*', i['equipment_object_type'])]
            cooling_list_dictionary = \
                eo.get_structure(structure_hierarchy=['AutoCreated', 'PlantLoop', 'PlantEquipmentListMixedWaterCooling'])
            cooling_list_dictionary['equipment'] = cooling_equipment
            equipment_list_dictionary = [
                {'PlantEquipmentList': cooling_list_dictionary},
                {'PlantEquipmentList': heating_list_dictionary}]
        elif 'condenserwater' in plant_loop_class_object.template_type.lower():
            list_dictionary = \
                eo.get_structure(structure_hierarchy=['AutoCreated', 'PlantLoop', 'CondenserEquipmentList'])
            list_dictionary['equipment'] = equipment
            equipment_list_dictionary = [{'CondenserEquipmentList': list_dictionary}, ]
        else:
            raise InvalidTemplateException(
                'Error: an invalid loop type was specified when creating plant loop connections: {}'
                .format(plant_loop_class_object.template_type))
        equipment_list_formatted_dictionary = eo.yaml_list_to_epjson_dictionaries(
            yaml_list=equipment_list_dictionary)
        resolved_path_dictionary = eo.resolve_objects(epjson=equipment_list_formatted_dictionary)
        # save output to class epsjon
        self.merge_epjson(
            super_dictionary=self.epjson,
            object_dictionary=resolved_path_dictionary)
        return

    def run(self, input_epjson=None):
        """
        Execute HVAC Template process workflow

        :param input_epjson: input epJSON file
        :return: epJSON containing expanded objects from templates
        """
        if not input_epjson:
            if self.input_epjson:
                input_epjson = self.input_epjson
            else:
                raise InvalidEpJSONException("No epJSON file loaded or provided to HVACTemplate processor")
        self.epjson_process(epjson_ref=input_epjson)
        self.logger.info('##### PreProcessing Data #####')
        self._hvac_template_preprocess(epjson=self.input_epjson)
        self.logger.info('##### Processing Thermostats #####')
        self.expanded_thermostats = self._expand_templates(
            templates=self.templates_thermostats,
            expand_class=ExpandThermostat)
        self.logger.info('##### Processing Systems #####')
        self.expanded_systems = self._expand_templates(
            templates=self.templates_systems,
            expand_class=ExpandSystem)
        self.logger.info('##### Processing Zones #####')
        self.expanded_zones = self._expand_templates(
            templates=self.templates_zones,
            expand_class=ExpandZone,
            system_class_objects=self.expanded_systems)
        self.logger.info('##### Building Zone-Thermostat Connections #####')
        for _, zone_class_object in self.expanded_zones.items():
            self._create_zonecontrol_thermostat(zone_class_object=zone_class_object)
        self.logger.info('##### Building System-Zone Connections #####')
        for _, system_class_object in self.expanded_systems.items():
            # VRF systems do not connect via air paths, and need a separate function.
            if system_class_object.template_type == 'HVACTemplate:System:VRF':
                self._create_system_vrf_path_connection_objects(
                    system_class_object=system_class_object,
                    expanded_zones=self.expanded_zones)
            else:
                self._create_system_path_connection_objects(
                    system_class_object=system_class_object,
                    expanded_zones=self.expanded_zones)
        self.logger.info('##### Processing Plant Loops #####')
        self.expanded_plant_loops = self._expand_templates(
            templates=self.templates_plant_loops,
            expand_class=ExpandPlantLoop)
        self.logger.info('##### Processing Plant Equipment #####')
        self.expanded_plant_equipment = self._expand_templates(
            templates=self.templates_plant_equipment,
            expand_class=ExpandPlantEquipment,
            plant_loop_class_objects=self.expanded_plant_loops)
        # Pass through expanded plant equipment objects to create additional plant loops and equipment if necessary
        self._create_additional_plant_loops_and_equipment_from_equipment(
            expanded_plant_equipment=self.expanded_plant_equipment,
            expanded_plant_loops=self.expanded_plant_loops)
        self.logger.info('##### Building Plant-Plant Equipment Connections #####')
        for expanded_pl in self.expanded_plant_loops.values():
            self._create_water_loop_connectors_and_nodelist(
                plant_loop_class_object=expanded_pl,
                expanded_plant_equipment=self.expanded_plant_equipment,
                expanded_systems=self.expanded_systems,
                expanded_zones=self.expanded_zones)
            self._create_plant_equipment_lists(
                plant_loop_class_object=expanded_pl,
                expanded_plant_equipment=self.expanded_plant_equipment)
        self.logger.info('##### Creating epJSON #####')
        # Merge each set of epJSON dictionaries
        merge_list = [
            self.epjson,
            self.base_objects,
            *[j.epjson for i, j in self.expanded_thermostats.items()],
            *[j.epjson for i, j in self.expanded_zones.items()],
            *[j.epjson for i, j in self.expanded_systems.items()],
            *[j.epjson for i, j in self.expanded_plant_loops.items()],
            *[j.epjson for i, j in self.expanded_plant_equipment.items()]]
        output_epjson = {}
        # The unique_name_override option is enabled here due to ObjectReference templates having the base equipment
        # in them as well as being present in the base epjson.  A better solution should be investigated so that this
        # option can be turned back off.
        for merge_dictionary in merge_list:
            self.merge_epjson(
                super_dictionary=output_epjson,
                object_dictionary=merge_dictionary,
                unique_name_override=True)
        # Use this for file debugging
        # import json
        # with open('test.epJSON', 'w') as base_file:
        #     json.dump(output_epjson, base_file, indent=4, sort_keys=True)
        # Create output format
        output_epjson = {
            "epJSON": output_epjson,
            "epJSON_base": self.base_objects,
            "epJSON_hvac_templates": self.templates,
            'Output:PreprocessorMessage': self.stream.getvalue()
        }
        return output_epjson
