Ruleset Project Description Phase 4
================

**Jason Glazer, GARD Analytics**

 - February 21, 2025
 - July 16, 2025 - update with focus on EnergyPlus table implementation

## Justification for New Feature ##

This continues the work from 2022 to 2024 to develop the createRPD script that creates a JSON Ruleset Project
Description (RPD) file consistent with the ASHRAE Standard 229 Ruleset Evaluation Schema (RES) to show the feasibility
of the schema, uncover problems with implementation, and provide an implementation for early adopters. The goal is to
provide an RPD file that fully supports the data currently used by the PNNL Ruleset Checking Tool. Additional effort is
needed to match the capabilities of the PNNL Ruleset Checking Tool and to coordinate with the RECI project led by
Karpman Consulting. In that project, the RPD export will be implemented in OpenStudio, Trace 3DPlus, and EnergyGauge and
tested with selected jurisdictions. For those tools, the createRulesetProjectDescription will be leveraged to ease the
effort of exporting to the RPD format. This will enable the utility to be used as much as possible during the pilot
project tests for the RECI project. The focus of this effort is on the remaining envelope, HVAC airside, and central
plant data groups, supporting any revisions to the 229 RPD schema and new features that are supported by the Ruleset
Checking Tool being developed by PNNL.

The repo for the script development is here:

https://github.com/JasonGlazer/createRulesetProjectDescription

The title, purpose, and scope of ASHRAE Standard 229 are:

Title: 

- Protocols for Evaluating Ruleset Application in Building Performance Models

Purpose: 

- This standard establishes tests and acceptance criteria for application of rulesets and related reporting for building 
performance models.

Scope:

- This standard applies to evaluation of the implementation of rulesets associated with new or existing buildings, their 
systems, controls, sites, and other aspects described by the ruleset. It establishes requirements for:  
- 2.1 building performance modeling software    
- 2.2 software that evaluates building performance models and associated information to check the application of a ruleset

ASHRAE Standard 229 has not been published and is under development by the ASHRAE SPC 229P committee. The first public
review was in 2024, and another public review is expected in 2025.

The intention of the standard is to provide code officials and rating authorities with files that they can use with a
Ruleset Checking Tool (currently, an example is under development at PNNL) to automatically check if a ruleset (such as
90.1 Appendix G, RESNET, California Title 24 performance paths, or Canada National Energy Code for Buildings performance
path) has been implemented correctly. Since each EnergyPlus IDF file could generate an RPD file, the Ruleset Checking
Tool will be able to see if the changes between the RPD files correspond to rules in the ruleset by looking at both the
baseline and proposed RPD file. 

The schema is described here:

https://github.com/open229/ruleset-model-description-schema/blob/main/schema-source/ASHRAE229_extra.schema.yaml

## E-mail and  Conference Call Conclusions ##

no discussion yet

## Overview ##

Previous tasks have focused on the envelope, internal loads, and primary and secondary HVAC equipment and systems. This
phase will continue to focus on those same areas of implementing the Ruleset Evaluation Schema. Additional code changes
are needed for the Python utility that takes advantage of some of the new reports that were added during the previous
task. Enhancements to output reporting from EnergyPlus, while still needed overall, will be smaller for this task since
much of the effort will be focused on utilizing the new reports, such as the HVAC Topology report and other tables or new
columns in existing tables.

Since Standard 229P is still being developed, the Ruleset Evaluation Schema is continuing to change. So far, Standard
229P has gone through one public review process, and the version number of schema at that point was 0.0.36. It is
continuing to be developed, and the most recent schema version number is 0.1.13. We expect that before the second public
review some additional revisions to the schema are going to be included. This impacts not only the createRPD software but also
the PNNL RCT tool and other software tools that are using createRPD, as well as indirectly impacting software tools that
generate RPD files for other simulation software (such as eQUEST). Due to the need to coordinate all of these pieces,
the various software developers have agreed to coordinate and support the latest schema that has gone through public
review. Since it is likely that the second public review will be voted out of the SPC 229 committee in the upcoming
months, it is likely that a schema with a new version of 0.1.13 will be used for all of these software tools, including
createRPD. Support of a single version is obviously more efficient than trying to support multiple versions, but if
necessary, such as if the second public review does not occur in a timely manner, supporting multiple versions of the
schema may be added.

With other software vendors planning on using the createRPD with their own software, more EnergyPlus input files have
been attempted to be used with createRPD, and this has revealed some weaknesses in the software. While some file testing
has been done, and many unit tests are present in the software repository, the limited number of files used for testing
has clearly not exercised the code well. Additional integrated tests and unit tests are needed to be added to move the
createRPD software from a proof-of-concept Python script to a reliable library that other software vendors and their
users can depend on. One form of testing that will be added is the RPD generation tests that are included in the
standard that will allow us to compare outputs against a truth standard. These test cases will either be created or
borrowed.

Since this NFP builds on the efforts described in previous NFPs, some details are skipped that are already described in
those earlier NFPs:

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2022/NFP-InitialRulesetModelDescription.md

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2023/NFP-RulesetModelDescriptionPhase2.md

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2024/NFP-RulesetModelDescriptionPhase3.md

Some lessons learned from the initial effort in 2022 are described here:

https://github.com/JasonGlazer/createRulesetModelDescription/blob/main/docs/lessons_learned.rst


## Approach ##

The overall approach will follow these steps:

1. Fix bugs in EnergyPlus related to output that createRPD uses
2. Fix bugs in createRPD including reviewing and merging pull requests from others
3. Implement additional elements in createRPD from existing output reports
4. Enhance automatic testing with more integrated file testing and unit testing
5. If effort remains available, enhance EnergyPlus with new reports
6. Implement additional elements in createRPD based on new reports

A specially tagged verson of the schema file was created to help keep track of the implementation status for each data
element generated by createRPD and the EnergyPlus inputs and outputs that are used to populate each data element. Since
there are so many data elements it is convenient to keep information pertinant to the createRPD implementation in
locations adjacent to the data elements themselves.

https://github.com/open229/ruleset-model-description-schema/blob/EPtags/schema-source/ASHRAE229_extra.schema.yaml

Tags added are:

- Used by RCT Test
- EPin Object
- EPin Field
- EPout File
- EPout Report
- EPout Table
- EPout Column
- EPstatus
- EP Notes

In order to determine which data elements are important, a script was written called yaml_data_elements.py (thanks
KarenW!) that identifies all elements used by the RCT tests that have been developed.

https://github.com/JasonGlazer/createRulesetProjectDescription/blob/ea7e7f7f74d6ca4ebd3187b3971e3a7816bc396d/utilities/yaml_data_elements.py

It looks through the RCT tests here:

https://github.com/pnnl/ruleset-checking-tool/tree/develop/rct229/ruletest_engine/ruletest_jsons/ashrae9012019

And identifies all data elements needed by some rule. This approach is very thorough but not necessarily comprehensive
since tests are still being added to the RCT repo and will continue to be added as that software is being developed.
Running the script identified a large number of data elements that are required. Those that were within scope were
changed to a EPstatus of ToDo, indicating that they needed to be done soon, which included 85 data elements. The ones
that were out of scope included elements related to service water heating, refrigeration, and elevators. These were
categorized into two groups: those that needed new output reports from EnergyPlus and those that could use existing
outputs from EnergyPlus.

Data elements to be implemented using existing outputs from EnergyPlus
-----------------------------------------------------------------------

- RulesetProjectDescription.metadata
- RulesetProjectDescription.output
- Metadata.id
- Metadata.time_of_creation
- Metadata.version
- RulesetModelDescription.external_fluid_sources
- RulesetModelDescription.calendar
- RulesetModelDescription.weather
- RulesetModelDescription.model_output
- Zone.floor_name
- Zone.design_thermostat_cooling_setpoint
- Zone.design_thermostat_heating_setpoint
- Zone.minimum_humidity_setpoint_schedule
- Zone.maximum_humidity_setpoint_schedule
- Zone.air_distribution_effectiveness
- Zone.zonal_exhaust_fan
- Surface.optical_properties
- SurfaceOpticalProperties.id
- SurfaceOpticalProperties.absorptance_thermal_exterior
- Schedule.type
- Weather.ground_temperature_schedule
- HeatingVentilatingAirConditioningSystem.preheat_system
- HeatingVentilatingAirConditioningSystem.humidification_type
- HeatingSystem.heatpump_auxiliary_heat_type
- CoolingSystem.dehumidification_type
- FanSystem.return_fans
- FanSystem.relief_fans
- FanSystem.air_energy_recovery
- AirEnergyRecovery.id
- AirEnergyRecovery.design_sensible_effectiveness
- AirEnergyRecovery.design_latent_effectiveness
- AirEnergyRecovery.outdoor_airflow
- AirEnergyRecovery.exhaust_airflow
- FluidLoop.child_loops
- FluidLoopDesignAndControl.design_supply_temperature
- FluidLoopDesignAndControl.design_return_temperature
- FluidLoopDesignAndControl.is_sized_using_coincident_load
- FluidLoopDesignAndControl.minimum_flow_fraction
- Chiller.efficiency_metric_values
- Chiller.efficiency_metric_types
- HeatRejection.fan_design_electric_power
- ExternalFluidSource.id
- ExternalFluidSource.loop
- ExternalFluidSource.type

These will be the focus of the effort under this task. If any remaining effort is available, as previously described in
the approach above, the following additional data elements may be implemented as new outputs from EnergyPlus and
additional programming in createRPD.

Data Elements that Require New Outputs from EnergyPlus
------------------------------------------------------
The following list was updated July 9 when we had a shift to focus on EnergyPlus table enhancements. The 
list was redone to include all data elements used by the PNNL RCT for 90.1-2019 appendix G based on RDS.
This included service water heating and refrigation.

- RulesetModelDescription.service_water_heating_distribution_systems
- RulesetModelDescription.service_water_heating_equipment
- RulesetModelDescription.service_water_heating_uses
- Building.refrigerated_cases
- BuildingSegment.service_water_heating_area_type
- Space.occupant_sensible_heat_gain
- Space.occupant_latent_heat_gain
- Space.service_water_heating_uses
- Infiltration.modeling_method
- Infiltration.algorithm_name
- Infiltration.flow_rate
- Infiltration.multiplier_schedule
- Surface.adjacent_to
- Construction.id
- Construction.c_factor*
- Construction.f_factor*
- Subsurface.classification
- Subsurface.has_shading_overhang*
- Subsurface.has_shading_sidefins*
- Schedule.hourly_heating_design_day
- Schedule.hourly_cooling_design_day
- FanSystem.air_energy_recovery
- FanSystem.temperature_control
- FanSystem.operation_during_occupied
- FanSystem.operation_during_unoccupied
- FanSystem.fan_control
- FanSystem.reset_differential_temperature
- FanSystem.supply_air_temperature_reset_load_fraction
- FanSystem.fan_volume_reset_type
- FanSystem.fan_volume_reset_fraction
- FanSystem.operating_schedule
- AirEconomizer.id
- AirEconomizer.type
- AirEconomizer.high_limit_shutoff_temperature
- AirEnergyRecovery.energy_recovery_operation
- Fan.operating_points
- FanOperatingPoint.airflow
- FanOperatingPoint.power
- Terminal.has_demand_control_ventilation
- Terminal.is_fan_first_stage_heat*
- FluidLoopDesignAndControl.temperature_reset_type
- FluidLoopDesignAndControl.outdoor_high_for_loop_supply_reset_temperature
- FluidLoopDesignAndControl.outdoor_low_for_loop_supply_reset_temperature
- FluidLoopDesignAndControl.loop_supply_temperature_at_outdoor_high
- FluidLoopDesignAndControl.loop_supply_temperature_at_outdoor_low
- FluidLoopDesignAndControl.loop_supply_temperature_at_low_load
- Boiler.operation_lower_limit
- Boiler.operation_upper_limit
- ServiceWaterHeatingDistributionSystem.id
- ServiceWaterHeatingDistributionSystem.tanks
- ServiceWaterHeatingDistributionSystem.service_water_piping
- ServiceWaterHeatingDistributionSystem.drain_heat_recovery_efficiency
- ServiceWaterHeatingDistributionSystem.entering_water_mains_temperature_schedule
- ServiceWaterPiping.id
- ServiceWaterPiping.is_recirculation_loop
- ServiceWaterPiping.are_thermal_losses_modeled
- SolarThermal.id
- ServiceWaterHeatingEquipment.heater_fuel_type
- ServiceWaterHeatingEquipment.distribution_system
- ServiceWaterHeatingEquipment.efficiency_metric_types
- ServiceWaterHeatingEquipment.efficiency_metric_values
- ServiceWaterHeatingEquipment.draw_pattern
- ServiceWaterHeatingEquipment.first_hour_rating
- ServiceWaterHeatingEquipment.setpoint_temperature
- ServiceWaterHeatingEquipment.solar_thermal_systems
- ServiceWaterHeatingUse.id
- ServiceWaterHeatingUse.area_type
- ServiceWaterHeatingUse.served_by_distribution_system
- ServiceWaterHeatingUse.use
- ServiceWaterHeatingUse.use_units
- ServiceWaterHeatingUse.use_multiplier_schedule
- ServiceWaterHeatingUse.temperature_at_fixture
- ServiceWaterHeatingUse.is_heat_recovered_by_drain
- RefrigeratedCase.id
- RefrigeratedCase.type

The ones with asterisks* indicate more substantial effort needed to implement. If support for these is added, this NFP
will be updated to describe the exact report changes made.

If reviewers see a critical data element from this second list that needs to be implemented. Please let us know.

In addition, a few items have been partially implemented in previous tasks but can be completed now due to additional
reporting being added (primarily the HVAC Topology report), including:

- Infiltration.modeling_method
- Infiltration.algorithm_name
- Infiltration.flow_rate
- Subsurface.classification
- Subsurface.opaque_area
- Subsurface.u_factor
- HeatingSystem.efficiency_metric_values
- HeatingSystem.efficiency_metric_types
- Terminal.served_by_heating_ventilating_air_conditioning_system
- Surface.adjacent_to
- Subsurface.glazed_area
- FanSystem.fan_control
- Terminal.type
- Terminal.heating_capacity


## Enhance Existing EnergyPlus Tabular Output Reports ##

The following sections describe enhancements to the existing reports and new reports that will be added in EnergyPlus

New columns shown with:
- Percent % are based directly on input and could be pulled from epJSON file.
- Caret ^ are outputs from EnergyPlus that cannot be found within epJSON file.

#### Initialization Summary - People Internal Gains Nominal ####

The current columns are:
- Name
- Schedule Name
- Zone Name
- Zone Floor Area {m2}
- Number Zone Occupants
- Number of People {}
- People/Floor Area {person/m2}
- Floor Area per person {m2/person}
- Fraction Radiant
- Fraction Convected
- Sensible Fraction Calculation
- Activity level
- ASHRAE 55 Warnings
- Carbon Dioxide Generation Rate
- Minimum Number of People for All Day Types
- Maximum Number of People for All Day Types
- Minimum Number of People for Weekdays
- Maximum Number of People for Weekdays
- Minimum Number of People for Weekends/Holidays
- Maximum Number of People for Weekends /Holidays
- Minimum Number of People for Summer Design Days
- Maximum Number of People for Summer Design Days
- Minimum Number of People for Winter Design Days
- Maximum Number of People for Winter Design Days

The new columns would be:
- Sensible heat per occupant^
- Latent heat per occupant^

This suppports:
- Space.occupant_sensible_heat_gain
- Space.occupant_latent_heat_gain

(this change is not needed with an additional requirement to include an
Output:Table:Annual named People Internal Gain Annual

https://github.com/JasonGlazer/createRulesetProjectDescription/issues/75
)


#### Initialization Summary - ZoneInfiltration Airflow Stats Nominal ####

The current columns are:
- Name
- Schedule Name
- Zone Name
- Zone Floor Area {m2}
- Number Zone Occupants
- Design Volume Flow Rate {m3/s}
- Volume Flow Rate/Floor Area {m3/s-m2}
- Volume Flow Rate/Exterior Surface Area {m3/s-m2}
- ACH - Air Changes per Hour
- Equation A - Constant Term Coefficient {}
- Equation B - Temperature Term Coefficient {1/C}
- Equation C - Velocity Term Coefficient {s/m}
- Equation D - Velocity Squared Term Coefficient {s2/m2}

The new columns would be:
- Zone Infiltration Type% (the type of infiltration object used)

This suppports:
- Infiltration.modeling_method
- Infiltration.algorithm_name

#### Outdoor Air Summary - Airflow Network Infiltration ####

New table

The new columns would be:
- Name%
- Schedule Name^
- Zone Name%
- Zone Floor Area {m2}^
- Number of Zone Occupants%
- Design Volume Flow Rate {m3/s}^
- Volume Flow Rate/Floor Area {m3/s-m2}^
- Volume Flow Rate/Exterior Surface Area {m3/s-m2}^

This suppports:
- Infiltration.modeling_method
- Infiltration.algorithm_name
- Infiltration.flow_rate
- Infiltration.multiplier_schedule

Would need to require hourly output for zone infiltration from airflownetwork

(this change is not needed with an additional requirement to include an
Output:Table:Annual named AFN Zone Infiltration Annual

https://github.com/JasonGlazer/createRulesetProjectDescription/issues/76
)


#### EnvelopeSummary - Exterior Fenestration ####

The current columns are:
- Construction
- Zone
- Space
- Frame and Divider
- Glass Area [m2]
- Frame Area [m2]
- Divider Area [m2]
- Area of One Opening [m2]
- Area of Multiplied Openings [m2]
- Glass U-Factor [W/m2-K]
- Glass SHGC
- Glass Visible Transmittance
- Frame Conductance [W/m2-K]
- Divider Conductance [W/m2-K]
- NFRC Product Type
- Assembly U-Factor [W/m2-K]
- Assembly SHGC
- Assembly Visible Transmittance
- Shade Control
- Parent Surface
- Azimuth [deg]
- Tilt [deg]
- Cardinal Direction

The new columns would be:
- Surface Type% (Window, Door, Glassdoor, etc.)
- Overhang Depth^
- Fin Depth^

This suppports:
- Subsurface.classification
- Subsurface.has_shading_overhang
- Subsurface.has_shading_sidefins


#### Equipment Summary - Air Heat Recovery ####

The current columns are:
- Name
- Input object type
- Plate/Rotary
- Sensible Effectiveness at 100% Heating Air Flow
- Sensible Effectiveness at 100% Cooling Air Flow
- Latent Effectiveness at 100% Heating Air Flow
- Latent Effectiveness at 100% Cooling Air Flow
- Exhaust Airflow [kg/s]
- Outdoor Airflow [kg/s]

The new columns would be:
- Operation With Economizer%
- AirLoop Name^

This suppports (indirectly):
- FanSystem.air_energy_recovery
- AirEnergyRecovery.energy_recovery_operation  (WHEN_FANS_ON, WHEN_MINIMUM_OUTSIDE_AIR, SCHEDULED)


#### Equipment Summary - Air Terminals ####

The current columns are:
- Zone Name
- Minimum Flow [m3/s]
- Minimum Outdoor Flow [m3/s]
- Supply Cooling Setpoint [C]
- Supply Heating Setpoint [C]
- Heating Capacity [W]
- Cooling Capacity [W]
- Type of Input Object
- Heat/Reheat Coil Object Type
- Chilled Water Coil Object Type
- Fan Object Type
- Fan Name
- Primary Air Flow Rate [m3/s]
- Secondary Air Flow Rate [m3/s]
- Minimum Flow Schedule Name
- Maximum Flow During Reheat [m3/s]
- Minimum Outdoor Flow Schedule Name

The new columns would be:
- Heating Control Type% (staged or modulated for AirTerminal:SingleDuct:SeriesPIU:Reheat and AirTerminal:SingleDuct:ParallelPIU:Reheat only)

This suppports:
- Terminal.is_fan_first_stage_heat

#### Equipment Summary - Fans ####

The current columns are:
- Type
- Total Efficiency [W/W]
- Delta Pressure [pa]
- Max Air Flow Rate [m3/s]
- Rated Electricity Rate [W]
- Rated Power Per Max Air Flow Rate [W-s/m3]
- Motor Heat In Air Fraction
- Fan Energy Index
- End Use Subcategory
- Design Day Name for Fan Sizing Peak
- Date/Time for Fan Sizing Peak
- Purpose
- Is Autosized
- Motor Efficiency
- Motor Heat to Zone Fraction
- Motor Loss Zone Name
- Airloop Name

The new columns would be:
- Speed Control Method^
- Number of Speeds%

This suppports:
- FanSystem.operation_during_occupied
- FanSystem.operation_during_unoccupied

#### Equipment Summary - Fan Operating Points ####

New table

The new columns would be:
- Fan Name%
- Airflow^
- Power^

This suppports:
- Fan.operating_points
- FanOperatingPoint.airflow
- FanOperatingPoint.power


#### System Summary - Economizer ####

The current columns are:
- High Limit Shutoff Control
- Minimum Outdoor Air [m3/s]
- Maximum Outdoor Air [m3/s]
- Return Air Temp Limit
- Return Air Enthalpy Limit
- Outdoor Air Temperature Limit [C]
- Outdoor Air Enthalpy Limit [C]

The new columns would be:
- AirLoop Name^ (not done because connection alreaady appears in HVAC Topology)
- AirLoopHVAC:OutdoorAirSystem Name^ 

This suppports (indirectly):
- AirEconomizer.id
- AirEconomizer.type
- AirEconomizer.high_limit_shutoff_temperature

#### Controls Summary - PlantEquipmentOperation:HeatingLoad ####

New table

The new columns would be:
- Name% (of PlantEquipmentOperation:HeatingLoad)
- PlantLoop Name%
- Load Range Index%
- Lower Limit%
- Upper Limit%
- Equipment List% (equipment names separated by semicolons or else multiple columns

This suppports:
- Boiler.operation_lower_limit
- Boiler.operation_upper_limit

(these inputs can be pulled from epJSON file including the name of PlantLoop since it also includes
the name of the operation schema from PlantLoop)


#### Controls Summary - SetpointManager:OutdoorAirReset ####

New Table

The new columns would be:
- Name%
- Control Variable%
- Setpoint at Outdoor Low Temperature% {C}
- Setpoint at Outdoor High Temperature% {C}
- Outdoor Low Temperature% {C}
- Outdoor High Temperature% {C}
- Setpoint Node or NodeList Name%
- Setpoint Node Loop Name^
- Schedule Name%
- Setpoint at Outdoor Low Temperature 2% {C}
- Setpoint at Outdoor High Temperature 2% {C}
- Outdoor Low Temperature 2% {C}
- Outdoor High Temperature 2% {C}

This suppports:
- FluidLoopDesignAndControl.outdoor_high_for_loop_supply_reset_temperature
- FluidLoopDesignAndControl.outdoor_low_for_loop_supply_reset_temperature
- FluidLoopDesignAndControl.loop_supply_temperature_at_outdoor_high
- FluidLoopDesignAndControl.loop_supply_temperature_at_outdoor_low
- FluidLoopDesignAndControl.temperature_reset_type (OUTSIDE_AIR_RESET) 

#### Controls Summary - SetpointManager:ReturnTemperature ####

New Table

The new columns would be:
- Name%
- Type^ (chilledwater or hotwater)
- Minimum Supply Temperature Setpoint%
- Maximum Supply Temperature Setpoint%
- Return Temperature Setpoint Input Type% (Constant, Scheduled or ReturnTemperatureSetpoint)
- Return Temperature Setpoint% (C)
- Return Temperature Setpoint Schedule Name%

This suppports:
- FluidLoopDesignAndControl.loop_supply_temperature_at_low_load
- FluidLoopDesignAndControl.temperature_reset_type (LOAD_RESET) 


#### System Summary - Demand Controlled Ventilation using Controller:MechanicalVentilation ####

The current columns are:
- Controller:MechanicalVentilation Name
- Outdoor Air Per Person [m3/s-person]
- Outdoor Air Per Area [m3/s-m2]
- Outdoor Air Per Zone [m3/s]
- Outdoor Air ACH [ach]
- Outdoor Air Method
- Outdoor Air Schedule Name
- Air Distribution Effectiveness in Cooling Mode
- Air Distribution Effectiveness in Heating Mode
- Air Distribution Effectiveness Schedule Name
- Type

The new columns would be:
- Demand Controlled Ventilation Active% (Yes or No)

This suppports:
- Terminal.has_demand_control_ventilation


#### Controls Summary - AvailabilityManager:Scheduled #### 

New Table 

The new columns would be:
- Name%
- Type (Scheduled, ScheduledOn, ScheduledOff)
- AirLoop Name^
- Schedule Name%

This suppports:
- FanSystem.operating_schedule

Covers AvailabilityManager:Scheduled, AvailabilityManager:ScheduledOn, AvailabilityManager:ScheduledOff

Note: Hours Supply Fan Operating Mode Cycling was previously part of this planned table but it seems to be 
covered separately by new System Summary - Fan Operation report under MJW PR#11138.


#### EquipmentSummary - Unitary Fan Mode ####

New Table

The new columns would be:
- Name%
- Hours Supply Air Fan Operating Mode Continuous When Occupied^
- Hours Supply Air Fan Operating Mode Cycling When Occupied^
- Hours Supply Air Fan Operating Mode Continuous When Unoccupied^
- Hours Supply Air Fan Operating Mode Cycling When Unoccupied^

This suppports:
- FanSystem.operation_during_occupied
- FanSystem.operation_during_unoccupied

We would need to develop way to differentiate occupied vs unoccupied times.


#### Controls Summary - AvailabilityManager:NightCycle ####

New Table

The new columns would be:
- Name%
- AirLoop Name%
- Control Type%

This suppports:
- FanSystem.operation_during_unoccupied


#### EquipmentSummary -  Service Water Heating ####

The current columns are:
- Type
- Storage Volume [m3]
- Input [W]
- Thermal Efficiency [W/W]
- Recovery Efficiency [W/W] 
- Energy Factor

The new columns would be:
- Heater Fuel Type%
- Daytime Setpoint Temperature^
- Draw pattern (this is a new input for Uniform Energy Factor calculation)
- Uniform Energy Factor^ (this will require new calculations and new input)
- Standby Loss Fraction^ (not needed by 90.1) 
- Standby Loss Energy^ (not needed by 90.1)
- First hour rating^ (not needed by 90.1)
- Peak Use Flow Rate% (for water heaters in stand-alone operation)
- Use Flow Rate Fraction Schedule Name% (for water heaters in stand-alone operation)
- Ambient Temperature Zone Name%

This suppports:
- ServiceWaterHeatingEquipment.heater_fuel_type
- ServiceWaterHeatingEquipment.setpoint_temperature
- ServiceWaterHeatingEquipment.efficiency_metric_types
- ServiceWaterHeatingEquipment.efficiency_metric_values
- ServiceWaterHeatingEquipment.first_hour_rating
- ServiceWaterHeatingEquipment.draw_pattern (new input or new compliance parameter)
- RulesetModelDescription.service_water_heating_equipment
- ServiceWaterHeatingUse.use (in stand-alone operation)
- ServiceWaterHeatingUse.use_units (in stand-alone operation)
- ServiceWaterHeatingUse.use_multiplier_schedule (in stand-alone operation)

Note: Recovery Efficiency and Energy Factor do not seem to be working for WaterHeater:Stratified. Uniform 
Energy Factor calculation would need to be added which, I believe uses draw factor as an input.
(For Peak Use Flow Rate need to add some Output:Table:Annual reports when water use equipment/connections 
are used see https://github.com/JasonGlazer/createRulesetProjectDescription/issues/77)
 

#### EquipmentSummary -  Water Use Equipment ####

NEW REPORT

The new columns would be:
- Name%
- Zone Name%
- End-Use Subcategory%
- Peak Flow Rate% [m3/s]
- Flow Rate Faction Schedule Name%
- Target Temperature Schedule Name%
- WaterUse Connection Name%

This suppports:
- ServiceWaterHeatingUse.id
- RulesetModelDescription.service_water_heating_uses
- Space.service_water_heating_uses
- ServiceWaterHeatingUse.area_type (end-use subcategory or compliance parameter)
- BuildingSegment.service_water_heating_area_type (end-use subcategory or compliance parameter)
- ServiceWaterHeatingUse.use
- ServiceWaterHeatingUse.use_units
- ServiceWaterHeatingUse.use_multiplier_schedule
- ServiceWaterHeatingUse.temperature_at_fixture

#### EquipmentSummary -  Water Use Connections ####

NEW REPORT

The new columns would be:
- Name%
- PlantLoop Name
- Drain Water Heat Exchanger Type%
- Drain Water Heat Exchanger Destination%
- Drain Water Heat Exchanger U-Factor Times Area%
- Average Heat Recovery Effectiveness^
- Cold Water Supply Temperature Schedule Name% (if missing use Site:WaterMainsTemperature)
- Pipe:Indoor are used^ (Yes or No)
- Pipe:Outdoor are used^ (Yes or No)

This suppports:
- ServiceWaterHeatingDistributionSystem.drain_heat_recovery_efficiency
- ServiceWaterHeatingUse.is_heat_recovered_by_drain
- ServiceWaterHeatingUse.served_by_distribution_system
- ServiceWaterHeatingDistributionSystem.entering_water_mains_temperature_schedule
- RulesetModelDescription.service_water_heating_distribution_systems
- ServiceWaterHeatingDistributionSystem.id
- ServiceWaterHeatingEquipment.distribution_system
- ServiceWaterHeatingDistributionSystem.service_water_piping
- ServiceWaterPiping.id
- ServiceWaterPiping.are_thermal_losses_modeled

(Note Pipe:Indoor, Pipe:Outdoor, Pipe:Underground, PipingSystem:Underground:PipeCircuit were not 
added to the report since they are in the HVAC Topology report - if present. So this report just 
has the PlantLoop and Branch Names)

#### EquipmentSummary -  Refrigeration:Case ####

NEW TABLE

The new columns would be:
- Name%
- Zone Name%
- Case Operating Temperature%
- Case Length%
- Case Height%

(this table is primarly needed to establish the list of cases so additional values should be added that are easy)

This suppports:
- Building.refrigerated_cases
- RefrigeratedCase.id 

#### Envelope Summary - At or Below Grade Constructions ####

NEW TABLE

The new columns would be:
- Name%
- Location^ (on-grade or below-grade)
- C-Factor^
- F-Factor^

(note the calculation of these may require additional inputs for all the different ways to 
describe slab-on-grade and below-grade surfaces)

This suppports:
- Construction.id
- Construction.c_factor
- Construction.f_factor


#### Initialization Summary - Schedule ####

The current columns are:
- Name
- ScheduleType
- Until Date
- WeekSchedule

The new columns would be:
- Cooling Design Day Schedule Name^
- Heating Design Day Schedule Name^

These might be able to be extracted from input but it would be complicated.

This suppports:
- Schedule.hourly_heating_design_day
- Schedule.hourly_cooling_design_day

(this change is not needed with a fix to the EIO/Schedule-Hourly table) 

#### report name - table name ####

The current columns are:
- x

The new columns would be:
- x

This suppports:
- x

## New Output for Echoing Inputs ##

In order fill in any gaps that might have been found, a new output object is recommended

Output:Table:InputEcho
- Input Object Name 
- Show number of extensible sets of fields
 
These would appear in a new report called "InputEcho" which would simply echo the input from the IDF or epJSON
file into the output. A table for each type of input object. Within each table, one column for each field. For 
extensible input objects it would be specified by user how many to produce

A second table would be created that would show uses of the each input object and would contain:
- Input Object Name 
- Used by Input Object Class
- Used by Input Object Name
- Used by Object Field Name

These would list when the original input object is used by other input objects.

This new input echo report may not be necessary if reading the epJSON is done instead.

## EnergyPlus Bugs to be Fixed ##

Add a switch to disable rounding or truncating digits for tabular reports.

https://github.com/NREL/EnergyPlus/issues/10896

The Equipment Summary - Air Heat Recovery table does not show the input object type and should instead just shows
"generic" for each one, and the outdoor and exhaust airflow columns show zeros for some example files (not yet posted).

The Component Sizing Summary - PlantLoop table does not show the design supply or return temperature for all example
files, and sometimes the return temperature for the cooling loop is lower. The sizing option column shows a number intead
of either coincident or noncoincident as intended, and the minimum flow fraction shows zero when it should sometimes show a
positive value. In addition this table does not appear to be consistently populated (not yet posted).

## createRPD Bugs to be Fixed ##

About 24 open issues exist in the repo:

https://github.com/JasonGlazer/createRulesetProjectDescription/issues

Some are not critical, but many are and will be addressed.

## Testing/Validation/Data Sources ##

The Python jsonschema library is being used to confirm that the RPD file is consistent with the schema. Comparison
of input and related outputs should provide the final check if the reporting is being done correctly.

## Input Output Reference Documentation ##

No changes to the documentation are expected.

## Outputs Description ##

The new output file is a JSON file following the schema described here:

https://github.com/open229/ruleset-model-description-schema

## Engineering Reference ##

No changes

## Example File and Transition Changes ##

None required.

## Design Document ##

The createRulesetModelDecription Python script will continue being developed using the same approach as the
previous work and will continue to include unit tests. This includes:

 - The Python utility is separate from EnergyPlus and will eventually be packaged with the EnergyPlus installer. It will
 continue to be developed in its own repository, but eventually, this may be merged or linked from the EnergyPlus
 repository.

 - The Python utility reads the JSON files that EnergyPlus produces when  the output:JSON input object is used as the
 primary source of information. As a secondary source of information, the epJSON input is read.

 - The Ruleset Project Description (RPD) format will be produced by the utility and is also a JSON format.

 - Verification that the RPD output produced by the new utility is consistent with the RES is performed by using
 the jsonschema Python library "validate" method.

 - The PathLib library is used for accessing files.

 - The unittest library is used for providing unit testing. The goal is to have tests for almost all of the methods.

 - Only a subset of data groups from the RMD schema will be generated and only data elements that are most direct will
 be implemented. This is expected to be the first step in an ongoing effort to fully implement the RMD schema as an
 output format.

## References ##

The original NFP is here:

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2022/NFP-InitialRulesetModelDescription.md

The 2023 NFP is here:

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2023/NFP-RulesetModelDescriptionPhase2.md

The 2023 NFP is here:

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2024/NFP-RulesetModelDescriptionPhase3.md

The repo for the script development is here:

https://github.com/JasonGlazer/createRulesetModelDescription

Some lessons learned from the initial effort in 2022 are described here:

https://github.com/JasonGlazer/createRulesetModelDescription/blob/main/docs/lessons_learned.rst

The schema is described here:

https://github.com/open229/ruleset-model-description-schema/blob/main/schema-source/ASHRAE229_extra.schema.yaml
