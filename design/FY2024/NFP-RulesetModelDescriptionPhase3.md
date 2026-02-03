Ruleset Model Description Phase 3
================

**Jason Glazer, GARD Analytics**

 - January 18, 2024
 - January 25, 2024 - added cross referencing in createRMD Enhancements section and other fixes
 - February 2, 2024 - added section on handling compliance parameters

## Justification for New Feature ##

This continues the work from 2022 and 2023 to develop the createRMD script that creates a JSON file consistent with the
ASHRAE Standard 229 Ruleset Model Description schema to show the feasibility of the schema, uncover problems with
implementation, and provide an implementation for early adopters. The goal is to provide an RMD file that fully supports
the data currently used by PNNL Ruleset Checking Tool. The repo for the script development is here:

https://github.com/JasonGlazer/createRulesetModelDescription

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

ASHRAE Standard 229 has not been published and is under development by the ASHRAE SPC 229P committee which is expecting
a public review shortly. The intention of the standard is to provide code officials and rating authorities with files
that they can use with a Ruleset Checking Tool (currently, an example is under development at PNNL) to automatically
check if a ruleset (such as 90.1 Appendix G, RESNET, California Title 24 performance paths, or Canada National Energy
Code for Buildings performance path) has been implemented correctly. Since each EnergyPlus IDF file could generate an
RMD file, the Ruleset Checking Tool will be able to see if the changes between the RMD files correspond to rules in
the ruleset by looking at both the baseline and proposed RMD file.

The schema is described here:

https://github.com/open229/ruleset-model-description-schema/blob/main/schema-source/ASHRAE229_extra.schema.yaml

## E-mail and  Conference Call Conclusions ##

no discussion yet

## Overview ##

The initial phase was on focused on data groups describing the building envelope and internal loads. The second phase added
support for some HVAC data groups and elements. This phase will continue working on the HVAC data groups and elements. The
PNNL Ruleset Checking Tool (RCT) supports evaluation of rules using many data groups but does not evaluate all data groups.
The data groups that are not included are related to refrigeration, elevators, SWH, transformers, district and data validation
points. These excluded data groups will not be supported in this phase of development for createRMD.

For this phase, the focus is on:

- implementing additional output reports in EnergyPlus
- filling in gaps in existing data groups
- supporting more data groups
- methods to merge compliance parameters

Targeted data groups from the 229 RMD Schema for this effort are the HVAC portion of the schema:

- HeatingVentilatingAirConditioningSystem
- HeatingSystem
- CoolingSystem
- FanSystem
- AirEconomizer
- AirEnergyRecovery
- Fan
- Terminal
- FluidLoop
- FluidLoopDesignAndControl
- Pump
- Boiler
- Chiller
- HeatRejection

This effort will focus on trying to implement as many data groups and elements as possible with the expectation to support
all the data elements used by the PNNL Ruleset Checking Tool.

Since this NFP builds on the efforts described in previous NFPs, some details are skipped that are already described in those
ealier NFPs:

https://github.com/NatLabRockies/EnergyPlus/blob/develop/design/FY2022/NFP-InitialRulesetModelDescription.md

https://github.com/NatLabRockies/EnergyPlus/blob/develop/design/FY2023/NFP-RulesetModelDescriptionPhase2.md

Some lessons learned from the initial effort in initial 2022 are described here:

https://github.com/JasonGlazer/createRulesetModelDescription/blob/main/docs/lessons_learned.rst

## Approach ##

The effort will continue the approach used in 2022 and 2023 development of the createRMD but
will add more reporting to EnergyPlus to be consistent with the goal of being driven by EnergyPlus
outputs and not needing to read the IDF or epJSON files. The approach has two focuses:

- Enhance EnergyPlus Tabular Output Reports
- Support New Data Groups in createRMD Python script

In addition, selected fixes of the createRMD Python script will be incorporated from the current list of issues
in the repository.

To understand how when data elements can be populated by either EnergyPlus input or EnergyPlus output as the
source of the RMD data elements, a specially tagged version of the schema file was created here:

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

The goal of this task is to support all the data groups and data elements currently used by the PNNL Ruleset Checking
Tool. If that is not possible within the budget of this task, a focus will be made on data elements and data groups
used most often.

The following sections describe enhancements to the existing reports and new reports that will be added in EnergyPlus

### Enhance Existing EnergyPlus Tabular Output Reports ###

#### Equipment Summary - Heating Coils ####

The current columns are:
- Type
- Design Coil Load [W]
- Nominal Total Capacity [W]
- Nominal Efficiency [W/W]
- Used as Supplementary Heat
- Airloop name
- Plantloop name

The new columns would be:
- Airloop branch name
- Plantloop branch name

#### Equipment Summary - DX Heating Coils ####

The current columns are:
- DX Heating Coil Type
- High Temperature Heating (net) Rating Capacity [W]
- Low Temperature Heating (net) Rating Capacity [W]
- HSPF [Btu/W-h]
- Region Number
- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation
- Airloop name

The new columns would be:
- Supplemental heat high shutoff temperature [C]
- Airloop branch name

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
- Purpose (supply, return, exhaust, relief)
- Is autosized
- Motor efficiency
- Motor heat to zone fraction
- Airloop name

The new columns would be:
- Occupied operation (cycling, continuous, off)
- Unoccupied operation (cycling, continuous, off)
- Locked out during central heating (yes, no, N/A)
- Motor loss zone name
- Airloop branch name

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

The new columns would be:
- Type (CO2 Zone/Other)

#### Coil Sizing Details - Coil Connections ####

The current columns are:
- Coil Name
- Coil Type
- Coil Location
- HVAC Type
- HVAC Name
- Zone Name(s)
- Supply Fan Name for Coil
- Supply Fan Type for Coil
- Airloop Name
- Plant Name for Coil
- Plant Loop Name

The new columns would be:
- Airloop Branch Name
- Location count on Airloop Branch
- Plant Branch Name
- Location count on Plantloop Branch

#### Equipment Summary - PlantLoop or CondenserLoop ####

The current columns are (need to verify that these are populated):
- name
- type (PlantLoop or CondenserLoop)
- provides heating
- provides cooling
- Maximum Loop Flow Rate
- Minimum Loop Flow Rate

The new columns would be:
- Total pump power on loop

#### Equipment Summary - AirTerminals ####

The current columns are:
- name
- Zone Name
- Minimum Flow
- Minimum Outdoor Flow
- Supply cooling setpoint
- Supply heating setpoint
- Heating capacity
- Cooling capacity

The new columns would be:
- type of input object
- Heat/Reheat Coil Object Type
- Hot Water Plant Loop Name
- Hot Water Plant Branch Name
- Chilled Water Coil Object Type
- Chilled Water Plant Loop Name
- Chilled Water Plant Branch Name
- Fan Object Type
- Fan Name
- Primary Air Flow Rate
- Secondary Air Flow Rate
- Minimum Flow Schedule Name
- Maximum Flow During Reheat
- Minimum Outdoor Flow Schedule Name
- Temperature control

#### Equipment Summary - Air Heat Recovery ####

The current columns are (need to verify that these are populated):
- name
- input object type
- plate/rotary
- Sensible Effectiveness at 100% Heating Air Flow
- Sensible Effectiveness at 100% Cooling Air Flow
- Latent Effectiveness at 100% Heating Air Flow
- Latent Effectiveness at 100% Cooling Air Flow
- Exhaust airflow
- Outdoor airflow

#### Component Sizing Summary - PlantLoop ####

The current columns are:
- Initial Maximum Loop Flow Rate [m3/s]
- Initial Plant Loop Volume [m3]
- Maximum Loop Flow Rate [m3/s]
- Plant Loop Volume [m3]

The new columns would be:
- Design supply temperature
- Design return temperature
- Sizing option (Coincident/NonCoincident)
- Minimum Loop Flow Rate [m3/s]


### Add New EnergyPlus Tabular Reports and Tables ###

#### HVAC Topology ####

HVAC Topology

This report would consists of many tables, one for each air loop, plant loop, or zone equipment. Each would be in the order of how
the branches appear in the loop and the order of components within each branch. After each splitter would be each branch.

The columns would be:

- Side
- Parent Type
- Parent Name
- Component Type
- Component Name
- Connection Type (if time allows)

The "Side" would be either "Demand" or "Supply" for airloops and plant loops. For zone equipment no "side" column is necessary.

A "Parent Type" could be: Branch, UnitarySystem, and would be blank for the demand side of airloops.

The "Component Type" and "Component Name" would usually list equipment or other HVAC components with their names but should also
include Zone and the name of the zone for the demand side of an airloop.

No node names would be included.

If time allows, the "Connection Type" would be included for components that have multiple pairs of nodes. The "Connection Type"
would describe the type for that pair of nodes. For example, a water coil with both air and water nodes would say "air" or "water"
as appropriate.  A chiller would have "chilled water" and "condenser water" and maybe "heat recovery." For
"PlantEquipmentOperation" it might be "Reference" and for AvailabilityManager it might show as "Sensor" and SetPointManger
might have "setpoint" here.

For implementation, the following data structures are likely to be used

- air loops (this is all supply side) DataAirSystems::DefinePrimaryAirSystem PrimaryAirSystems
- state.dataAirSystemsData->PrimaryAirSystems
- state.dataZoneEquip->ZoneEquipList
- state.dataPlnt->VentRepPlant
- state.dataPlnt->VentRepCond
- state.dataZoneEquip->SupplyAirPath
- state.dataZoneEquip->ReturnAirPath
- SimAirServingZones::InitAirLoop for supply/return path indexes to the AirPrimaryLoop struct
- thisZoneEquipConfig.AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathIndex - this gets set in SystemReports::InitEnergyReports
- for connection type, SystemReports::CreateEnergyReportStructure could potentially add calls to predefined reports right in here
as each component gets added to the data structures to deduce connection type

Eventually, anything in the Energy+.idd that has a \type node field would be represented.

#### Controls ####

A new report that includes Setpoint Managers, Controllers, Availability Managers, PlantEquipmentOperation, and components
with control options and identifies the type, where they apply, sense, and control parameters from the input. Need:

- minimum and maximum setpoint temperatures
- minimum turndown ratio
- schedules
- load range lower limit
- load range upper limit
- Setpoint at Outdoor Low Temperature
- Outdoor Low Temperature
- Setpoint at Outdoor High Temperature
- Outdoor High Temperature

This is probably a series of tables, one for each input object. The report would include the pertinent information,
including the controls, related temperatures, and what they apply to but would generally be limited to what is already
available within the data structure that would be useful to report. Include specifically:

- Heat Recovery - Operation With Economizer
- Heat Recovery - Supply Air Temperature Control
- Setpoint Managers - FanSystem - temperature_control
- Setpoint Managers - FanSystem - reset_differential_temperature
- Setpoint Managers - FanSystem - supply_air_temperature_reset_load_fraction
- AvailabilityManager:Scheduled
- Pump and fan availability schedules
- Setpoint Managers - FluidLoopDesignAndControl - temperature_reset_type


## Testing/Validation/Data Sources ##

Since the output JSON file needs to comply with a JSON schema, it should be easy to confirm that it is valid.
The Python jsonschema library can be used to confirm that the RMD file is consistent with the schema. Comparison
of input and related outputs should provide the final check if the reporting is being done correctly.

## Input Output Reference Documentation ##

While no specific changes for input are expected, we are considering allowing compliance parameters in the IDF/epJSON file by
following the generic tag/properties described in this issue:

https://github.com/NatLabRockies/EnergyPlus/issues/8775

Additional tabular outputs will be described in the IOref.

## createRMD Enhancements ###

The following describes the new data elements and data groups that will be added to createRMD. These are based on
what is needed since they are used by PNNL's Ruleset Checking Tool. Note that not all data elements will be
supported that are shown below. Data elements followed by (cp) are compliance parameters and are not able to be
captured in input to, or output from, EnergyPlus. In addition, data elements followed by (nc) have no corresponding
data column from EnergyPlus output and no plans to support that data element. These are typically only available
as input or would be complicated to support as output. After other data elements in parenthesis is the EnergyPlus
report that can be used to populate that data element.

^means that it is in in schema.yaml and column is defined-delete after assessment

HeatingVentilatingAirConditioningSystem
- preheat_system (topology report)

CoolingSystem:
- dehumidification_type (topology report)

FanSystem:
- return_fans (PROBABLY topology report)
- exhaust_fans (Component Sizing Summary AND topology report)
- relief_fans (PROBABLY topology report)
- air_economizer (System Summary - Economizer)
- air_energy_recovery (Component Sizing Summary)
- temperature_control (Controls report)
- operation_during_occupied (Equipment Summary - Fans)
- operation_during_unoccupied (Equipment Summary - Fans)
- fan_control (cp)
- reset_differential_temperature (Controls report)
- supply_air_temperature_reset_load_fraction (Controls report)
- fan_volume_reset_type (nc)
- fan_volume_reset_fraction (nc)
- minimum_airflow (Standard 62.1 Summary)
- minimum_outdoor_airflow (Component Sizing Summary)
- maximum_outdoor_airflow (Component Sizing Summary)
- demand_control_ventilation_control (System Summary - Demand Controlled Ventilation using Controller:MechanicalVentilation - Type)

AirEconomizer
- type (System Summary - Economizer)
- high_limit_shutoff_temperature (System Summary - Economizer)

AirEnergyRecovery
- enthalpy_recovery_ratio (cp)
- energy_recovery_operation (cp)

FanOutputValidationPoint
- airflow (nc)
- result (nc)

Terminal (probably should do early since critical)
- type (Equipment Summary - AirTerminals - type)
- served_by_heating_ventilating_air_conditioning_system (topology report)
- heating_source (Equipment Summary - AirTerminals - Heat/Reheat Coil Object Type)
- heating_from_loop (Equipment Summary - AirTerminals - Hot Water Plant Loop Name)
- cooling_source (Equipment Summary - AirTerminals, Chilled Water Coil Object Type)
- cooling_from_loop (Equipment Summary - AirTerminals, Chilled Water Plant Loop Name)
- fan (Equipment Summary - AirTerminals, Fan Name)
- fan_configuration (Equipment Summary - AirTerminals, type of input object)
- primary_airflow (Equipment Summary - AirTerminals, Primary Air Flow Rate)
- supply_design_heating_setpoint_temperature (Equipment Summary - AirTerminals, Supply heating setpoint)
- supply_design_cooling_setpoint_temperature (Equipment Summary - AirTerminals, Supply cooling setpoint)
- minimum_airflow (Standard 62.1 Summary)
- minimum_outdoor_airflow (Standard 62.1 Summary)
- minimum_outdoor_airflow_multiplier_schedule (Equipment Summary - AirTerminals, Minimum Flow Schedule Name)
- heating_capacity (Equipment Summary - AirTerminals, heating capacity)
- is_supply_ducted (cp)
- has_demand_control_ventilation (System Summary - Demand Controlled Ventilation using Controller:MechanicalVentilation, Controller:MechanicalVentilation Name, Demand Controlled Ventilation)

FluidLoop
- type (Equipment Summary - PlantLoop or CondenserLoop)
- pump_power_per_flow_rate (Equipment Summary - PlantLoop or CondenserLoop, Total pump power on loop)
- child_loops (topology report)
- cooling_or_condensing_design_and_control (Equipment Summary - PlantLoop or CondenserLoop)
- heating_design_and_control (Equipment Summary - PlantLoop or CondenserLoop)

FluidLoopDesignAndControl
- design_supply_temperature (Component Sizing Summary - PlantLoop, Design supply temperature)
- design_return_temperature (Component Sizing Summary - PlantLoop, Design return temperature)
- is_sized_using_coincident_load (Component Sizing Summary - PlantLoop, Sizing option)
- minimum_flow_fraction (Component Sizing Summary - PlantLoop, Minimum Loop Flow Rate)
- operation (Equipment Summary - Pumps, Control)
- operation_schedule (Controls)
- flow_control (Equipment Summary - Pumps, Type)
- temperature_reset_type (Controls)
- outdoor_high_for_loop_supply_reset_temperature (Controls)
- outdoor_low_for_loop_supply_reset_temperature (Controls)
- loop_supply_temperature_at_outdoor_high (Controls)
- loop_supply_temperature_at_outdoor_low (Controls)
- loop_supply_temperature_at_low_load (Controls)
- has_integrated_waterside_economizer (topology)

Boiler
- draft_type (cp)
- operation_lower_limit (Controls)
- operation_upper_limit (Controls)

Chiller
- compressor_type (cp)
- is_chilled_water_pump_interlocked (topology)
- is_condenser_water_pump_interlocked (topology)

HeatRejection
- type (Equipment Summary - Cooling Towers and Fluid Coolers, type)
- fan_shaft_power (cp)
- fan_speed_control (Equipment Summary - Cooling Towers and Fluid Coolers, type)
- rated_water_flowrate (cp)
- leaving_water_setpoint_temperature (Equipment Summary - Cooling Towers and Fluid Coolers, Leaving Water Setpoint Temperature)

ExternalFluidSource
- loop (topology)
- type (topology)

OutputInstance:
- ruleset_model_type (cp)
- rotation_angle (cp, could be from Compliance:Building)
- unmet_load_hours (system summary)
- unmet_load_hours_heating (system summary)
- unmet_occupied_load_hours_heating (system summary)
- unmet_load_hours_cooling (system summary)
- unmet_occupied_load_hours_cooling  (system summary)
- building_peak_cooling_load (HVAC Sizing Summary)


Some remaining non-HVAC related items that may be added include:

RulesetModelDescription
- measured_infiltration_pressure_difference (cp)

Building
- building_open_schedule (cp)

BuildingSegment
- is_all_new (cp)
- area_type_vertical_fenestration (cp)
- lighting_building_area_type (cp)
- area_type_heating_ventilating_air_conditioning_system (cp)

Zone:
- floor_name (cp)
- design_thermostat_cooling_setpoint (nc)
- design_thermostat_heating_setpoint (nc)
- maximum_humidity_setpoint_schedule (nc)
- zonal_exhaust_fan (nc)
- non_mechanical_cooling_fan_airflow (cp)
- air_distribution_effectiveness (nc)

Space:
- occupant_sensible_heat_gain (nc)
- occupant_latent_heat_gain (nc)
- status_type (cp)
- function (cp)

Infiltration:
- measured_air_leakage_rate (cp)

Construction
- c_factor (cp)
- f_factor (cp)

SurfaceOpticalProperties:
- absorptance_thermal_exterior (nc)
- absorptance_solar_exterior (nc)

Subsurface:
- classification (cp)
- subclassification (cp)
- dynamic_glazing_type (nc)
- has_shading_overhang (nc)
- has_shading_sidefins (nc)
- has_manual_interior_shades (cp)

InteriorLighting
- occupancy_control_type (cp)

MiscellaneousEquipment
- type (cp)

Transformer
- type (cp)
- phase (nc)
- efficiency (nc)
- capacity (nc)

Schedule
- hourly_heating_design_day (nc)
- hourly_cooling_design_day (nc)
- type (nc)

Weather
- ground_temperature_schedule (nc)
- data_source_type (cp)

(cp) indicates that it may need to be compliance parameter not based on simulation inputs or outputs
(nc) indicates that no corresponding data column exists and we have no plans to support that data element

This list is based on two sources:
- baseline_model.json
- docs\data_elements_used.yaml

The baseline_model.json file is from https://github.com/pnnl/ruleset-checking-tool/blob/develop/examples/chicago_demo/baseline_model.json

The data_elements_used.yaml file was generated by script from the files related to the PNNL Ruleset Checking
Tool in /ruletest_engine/ruletest_jsons/ashrae9012019

## Handling Compliance Parameters ##

Compliance parameters are those that cannot be determined by the simulation input or output and instead need to be
provided by the modeler as they are interpreting the design. These parameters should be organized so they can be
merged into the RMD file that is produced by the script.

Options for compliance parameters:

1. Add new fields to existing input objects associated with them and create or add to tabular report that echos
them out
2. Add new fields to existing input objects associated with them and parse the epJSON files to get them
3. Add new Compliance input objects like existing Compliance:Building and report them in tabular reports
4. Add new Compliance input objects like existing Compliance:Building and parse the epJSON to get them (essentially
ignore these inputs by EnergyPlus like HVACTemplate objects are ignored). A preprocessor would need to delete them.
5. Add new generic tag object (https://github.com/NatLabRockies/EnergyPlus/issues/8775) to associate new key-value pairs for
compliance parameters and create a tabular report that echos them out
6. Create a JSON file that contains the compliance parameters but the layout would need to match the layout of the
RMD file so that it could be merged. Use createRMD to make an empty file to make it easier for user to populate
and would be different for each EnergyPlus input file. GUI developers would probably not need the created empty files.
7. Create a YAML file that contains the compliance parameters but the layout would need to match the layout of the
RMD file so that it could be merged. Use createRMD to make an empty file to make it easier for user to populate and
would be different for each EnergyPlus input file.  GUI developers would probably not need the created empty files.
8. Create a JSON patch file (https://jsonpatch.com/) containing the compliance parameters and values. Use createRMD
to make an empty file to make it easier for user to populate and would be different for each EnergyPlus input file.
9. Create a separate IDD for the compliance parameters and users can create a special IDF that just contains them
that can be read. Keeping both files synchronized would be difficult.

For many of these the factor of being able to use IDF Editor (or eventually the epJSON Editor) or simply add and
manipulate an IDF file using current workflows is important. For GUIs they would probably also prefer to simply
generate an IDF or epJSON file directly that contains everything since the GUI would also now need new entry
fields for users to populate. A GUI could also generate the JSON patch file directly.

Options 1 to 5 are all keeping the information together in a single IDF or epJSON file and would probably be
easiest for the user or GUI developer.

Options 6 to 8 may be easier to program but more difficult for users and developers. If no creation of empty
compliance parameter file is done, they are definitely the easiest approach.

For now the recommendation is to use option 8. In the future, options 2 and 4 could be pursued and they could create
a JSON patch file as part of the preprocessor. Option 8 is the easiest option in the short term to implement.

## Outputs Description ##

The new output file is a JSON file following the schema described here:

https://github.com/open229/ruleset-model-description-schema

## Engineering Reference ##

No changes

## Example File and Transition Changes ##

None required.

## Design Document ##

In general, to try to maximize the number of new data elements implemented during this phase, items should be added based
on complexity.

For EnergyPlus changes, add new columns in the following order based on difficulty:
1. Existing tables where new output is simple echo of input or already computed and in data structure
2. New tables where new output is simple echo of input or already computed and in data structure
3. New or existing tables where new output is easy to determine
4. More complicated outputs

Echos of input are indicated in the eptags schema file.

Provide draft updated output from EnergyPlus for OpenStudio team to test prior to release.

For the createRulesetModelDecription Python script, will continue being developed using the same approach
as the 2022 adn 2023 work and will continue to include unit tests. This includes:

 - The Python utility is separate from EnergyPlus and will eventually be packaged with the EnergyPlus installer. It will
 continue to be developed in its own repository but eventually this may be merged or linked from the  EnergyPlus repository.

 - The Python utility reads the JSON files that EnergyPlus produces when  the output:JSON input object is used as the primary
 source of information. As a secondary source of information, the epJSON input is read.

 - The Ruleset Model Description (RMD) format will be produced by the utility and is also a JSON format.

 - Verification that the RMD output produced by the new utility is consistent with the RMD schema is performed by using
 the jsonschema Python library "validate" method.

 - The PathLib library is used for accessing files.

 - The unittest library is used for providing unit testing. The goal is to have tests for almost all of the methods.

 - Only a subset of data groups from the RMD schema will be generated and only data elements that are most direct will be
 implemented. This is expected to be the first step in an ongoing effort to fully implement the RMD schema as an output format.

## References ##

The original NFP is here:

https://github.com/NatLabRockies/EnergyPlus/blob/develop/design/FY2022/NFP-InitialRulesetModelDescription.md

The 2023 NFP is here:

https://github.com/NatLabRockies/EnergyPlus/blob/develop/design/FY2023/NFP-RulesetModelDescriptionPhase2.md

The repo for the script development is here:

https://github.com/JasonGlazer/createRulesetModelDescription

Some lessons learned from the initial effort in 2022 are described here:

https://github.com/JasonGlazer/createRulesetModelDescription/blob/main/docs/lessons_learned.rst

The schema is described here:

https://github.com/open229/ruleset-model-description-schema/blob/main/schema-source/ASHRAE229_extra.schema.yaml
