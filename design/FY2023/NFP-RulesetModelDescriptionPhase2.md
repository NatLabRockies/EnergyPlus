Ruleset Model Description Phase 2
================

**Jason Glazer, GARD Analytics**

 - May 31, 2023
 - June 19, 2023 updates based on feedback and added design document

## Justification for New Feature ##

This continues the work from 2022 to develop a script that creates a JSON file consistent with the ASHRAE Standard 229
Ruleset Model Description schema to show the feasibility of the schema, uncover problems with implementation, and provide
an implementation for early adopters.

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

ASHRAE Standard 229 has not been published or even gone through public review and is under development
by the ASHRAE SPC 229P committee. The intention of the standard is to provide code officials and rating
authorities with files that they can use with a Ruleset Checking Tool (currently, an example is under development at
PNNL) to automatically check if a ruleset (such as 90.1 Appendix G, RESNET, California Title 24 performance
paths, or Canada National Energy Code for Buildings performance path) has been implemented correctly.
Since each EnergyPlus IDF file could generate an RMD file, the Ruleset Checking Tool will be able to see if the
changes between the RMD files correspond to rules in the ruleset by looking at both the baseline
and proposed RMD file.

The original NFP is here:

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2022/NFP-InitialRulesetModelDescription.md

The repo for the script development is here:

https://github.com/JasonGlazer/createRulesetModelDescription

Some lessons learned from the initial effort in 2022 are described here:

https://github.com/JasonGlazer/createRulesetModelDescription/blob/main/docs/lessons_learned.rst

The schema is described here:

https://github.com/open229/ruleset-model-description-schema/blob/main/schema-source/ASHRAE229_extra.schema.yaml

## E-mail and  Conference Call Conclusions ##

Feedback received is shown below:

----

Concern about reporting equipment performance metric values that don't match manufacturers' reported values

----

From Standard 229 and RCT development perspectives, I think the highest priority is to implement RMD generation for components
planned for inclusion in the 229 RMD tests. As you know, these tests are based on the modified medium office prototype model.
Examples of modifications include the following:
- support for all (or most) of Appendix G baseline HVAC system types
- support several commonly used system types (such as WSHP) and designs (such as DOAS + space conditioning system;
space conditioning system + HW baseboard).
- detailed window specification
- handling of plenums (schema WG deemed that they should be aggregated with the parent space in the RMD file).

-----

I would suggest adding Output data, or at least a portion of the Output since some HVAC rules checking the peak cooling load and unmet hours.

-----

I just completed the review for Section 21 and 22 rules and attached is a list of keys required in addition to system type test jsons

https://github.com/pnnl/ruleset-checking-tool/tree/develop/rct229/ruletest_engine/ruletest_jsons/ashrae9012019/system_types

boilers
- draft_type
- rated_capacity
- operation_lower_limit
- operation_upper_limit
- efficiency
-	efficiency_metric

Chiller
- condensing_loop
- rated_capacity
- compressor_type
- full_load_efficiency
- is_chilled_water_pump_interlocked
- is_condenser_water_pump_interlocked
- part_load_efficiency
- part_load_efficiency_metric

FluidLoop
- heating_design_and_control
    - design_supply_temperature
    - design_return_temperature
    - flow_control
    - operation
    - minimum_flow_fraction
- pump_power_per_flow_rate
- cooling_or_condensing_design_and_control
    - design_supply_temperature
    - design_return_temperature
    - temperature_reset_type
    - outdoor_high_for_loop_supply_reset_temperature
    - outdoor_low_for_loop_supply_reset_temperature
    - loop_supply_temperature_at_outdoor_high
    - loop_supply_temperature_at_outdoor_low
    - minimum_flow_fraction
    - is_sized_using_coincident_load
    - flow_control

HeatRejection
- loop
- range
- design_wetbult_temperature
- approach
- leaving_water_setpoint_temperature

Output
- output_instance
    - unmet_load_hours_heating
    - unmet_occupied_load_hours_heating
    - unmet_load_hours_cooling
    - unmet_occupied_load_hours_cooling
    - building_peak_cooling_load
    - unmet_load_hours

FanSystem
- temperature_control
- reset_differential_temperature

Terminals
- primary_airflow
- minimum_outdoor_airflow
- minimum_airflow
- design_thermostat_heating_setpoint

(Later additions covering part of Section 19)

HeatingSystem
- heating_coil_setpoint

FanSystem
- operation_during_occupied
- demand_control_ventilation_control
- operation_during_unoccupied
- temperature_control
- reset_differential_temperature

-----

Adding more outputs to the database tables is generally not problematic, but I would suggest we get OpenStudio to do some
testing prior to anything merging into E+ develop branch.  Their testing on tabular outputs is above and beyond our own,
and they have a tendency to find issues after we've closed for IO freeze if we don't engage them early.

-------

Add chillers, boilers and cooling towers to equipment summary report as separate tables.

-------

Selecting the baseline system in 90.1 Appendix G is a high priority. The following data elements are needed for that:

- ASHRAE229: id^, ruleset_model_instances^
- RulesetModelInstance: id^, buildings^, fluid_loops~, pumps~, boilers~, chillers~
- Building: id^, building_segments^, building_open_schedule*
- BuildingSegment: id^, zones^, heating_ventilating_air_conditioning_systems~
- Zone: id^, thermostat_cooling_setpoint_schedule*^, thermostat_heating_setpoint_schedule*^, terminals
- HeatingVentilatingAirConditioningSystem: id~, fan_system~, heating_system~, cooling_system~, preheat_system
- HeatingSystem: id~, energy_source_type*, hot_water_loop
- CoolingSystem: id~, chilled_water_loop
- FanSystem: id~, supply_fans, return_fans, fan_control*
- Fan: id
- Terminal: id, type*, served_by_heating_ventilating_air_conditioning_system, heating_source*, heating_from_loop,
cooling_source*, cooling_from_loop, fan, fan_configuration*, is_supply_ducted*
- FluidLoop: id~, type*, child_loops
- Pump: id~, loop_or_piping, speed_control*~
- Boiler: id~, loop, energy_source_type*
- Chiller: id~, cooling_loop

The key below is shown for indicators:
- '*' indicates data elements that are not connective
- '^' indicates data elements are already complete
- '~' indicates a new data element that can be implemented without EnergyPlus enhancement

--------


Based on this feedback, the NFP was updated.


## Overview ##

The initial version did not support the full schema and needed polishing up before being used in widespread workflows.
For this phase, the focus is on methods to merge compliance parameters, implementing additional output reports in EnergyPlus,
filling in gaps in existing data groups, and supporting more data groups.

Targeted data groups from the 229 RMD Schema for this effort include the HVAC portion of the schema:

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

This effort will focus on trying to implement as many data groups as possible. It is unlikely that full support
for all data groups and data elements for these will be achieved in this phase, so the focus will again be on
implementing the more direct data elements within each data group so that as many data elements can be implemented
as possible.

In future efforts, related data groups will be supported:
- FanOutputValidationPoint
- BoilerOutputValidationPoint
- ChillerCapacityValidationPoint
- ChillerPowerValidationPoint

## Approach ##

The effort will continue the approach used in 2022 during the initial development of the createRMD but
will add more reporting to EnergyPlus to be consistent with the goal of being driven by EnergyPlus
outputs and not needing to read the IDF or epJSON files. The approach has two focuses:

- Enhance EnergyPlus Tabular Output Reports
- Support New Data Groups in createRMD Python script

In addition, some fixes of the createRMD Python script will be incorporated from the current list of issues:

- Ensure that all IDs are unique and consistent between instances (issue #4)
- Add Infiltration for each Zone even if not present in EnergyPlus (issue #5)
- Utilize additional Space tags in E+ for other enumerations of Space data group (issue #6)
- Add docstrings (issue #7)
- Fix grouping of HVAC related datagroups and use Construction:FfactorGroundFloor (issue #8)
- Ground Surfaces cannot be identified (issue #12)
- Add method to merge compliance parameters (issue #14)
- Add support for unmet load hours and peak building cooling load (issue #15)

To understand how when data elements can be populated by either EnergyPlus input or
EnergyPlus output as the source of the RMD data elements, a specially tagged version of the
schema file was created here:

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

Most of these were used previously during the 2022 effort.

## Prioritization ##

It is unlikely that all table enhancements and RMD data groups described in this NFP will be able to be supported
with the budget for this task, so some prioritization of what should be in this effort versus in following efforts
will be needed. The prioritization will link the support of the enhanced tables to the support of data elements in
the RMD data elements. Feedback on the priorities is welcome, either overall (air side vs. water side) or detailed,
such as specific new columns and data elements that are most important.

The feedback on prioritization was to first support the selection of baseline HVAC system in Appendix G which includes
changes to or new EnergyPlus reports:

- Equipment Summary – Fans
- HVAC Topology
- Equipment Summary - PlantLoop or CondenserLoop
- Equipment Summary – AirTerminals

After that supporting additional central plant reports in EnergyPlus enhancements:

- Equipment Summary – Pumps
- Component Sizing Summary – PlantLoop
- Equipment Summary – Chiller
- Equipment Summary – Boiler
- Equipment Summary - cooling towers and fluid coolers

While the EnergyPlus enhancements are underway, data elements that can be supported by current outputs should be
implemented.

Based on shorter deadline for the IO freeze of EnergyPlus, changes to existing reports will be implemented sooner than brand
new output tables.


### Enhance Existing EnergyPlus Tabular Output Reports ###

#### Equipment Summary - Heating Coils ####

The current columns are:
- Type
- Design Coil Load [W]
- Nominal Total Capacity [W]
- Nominal Efficiency [W/W]

The new columns would be:
- Used as Supplementary Heat
- Airloop name
- Airloop branch name
- Plantloop name
- Plantloop branch name
- Supplemental heat high shutoff temperature [C]

#### Equipment Summary - DX Heating Coils ####

The current columns are:
- DX Heating Coil Type
- High Temperature Heating (net) Rating Capacity [W]
- Low Temperature Heating (net) Rating Capacity [W]
- HSPF [Btu/W-h]
- Region Number

The new columns would be:
- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation
- Airloop name
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

The new columns would be:
- Purpose (supply, return, exhaust, relief)
- Occupied operation (cycling, continuous, off)
- Unoccupied operation (cycling, continuous, off)
- Locked out during central heating (yes, no, N/A)
- Is autosized
- Motor efficiency
- Motor heat to zone fraction
- Motor loss zone name
- Airloop name
- Airloop branch name

#### Equipment Summary - Pumps ####

The current columns are:
- Type
- Control
- Head [pa]
- Water Flow [m3/s]
- Electricity Rate [W]
- Power Per Water Flow Rate [W-s/m3]
- Motor Efficiency [W/W]
- End Use Subcategory

The new columns would be:
- Is autosized
- Plantloop name
- Plantloop branch name

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

#### Component Sizing Summary - PlantLoop ####

The current columns are:
- Maximum Loop Flow Rate [m3/s]
- Plant Loop Volume [m3]

The new columns would be:
- Design supply temperature
- Design return temperature
- Sizing option (Coincident/NonCoincident)

Note: the component sizing summary uses a slightly different approach to defining the table entries.

### Add New EnergyPlus Tabular Reports and Tables ###

#### HVAC Topology ####

Consists of tables for air and plant loops

For each Plant loop and AirLoopHVAC a table is created that has:
- Component Type (normal components including Connector:Splitter and Connector:Mixer)
- Component Name
- Zone Name (for demand side)
- Loop side (supply side/demand side)
- Order (2nd of 5 items on branch, items in parallel after split would have same number)
- Branch Name
- Split (either y/n or “split 3 of 5”)
- node name in (multiple names for mixer)
- node name out (multiple names for splitter)
- other control objects that reference node in as sensor (or node out?)
- other control objects that reference node in as actuator (or node out?)

Will this completely describe the zone equipment?

Probably also need a table to describe the components on an outdoor air branch.

The table would list where controllers, setpoint managers, and other items that reference nodes.

Additionally, the BND file may also be reviewed for what can easily be added to the topology report with minimal effort.

#### Coil Sizing Details - Coil Connections ####

A new table under the current Coil Sizing Details report called "Coil Connections". Each row would be a coil:

- Coil Name
- Coil Type*
- Coil Location*
- HVAC Type*
- HVAC Name*
- Zone Name(s)*
- Supply Fan Name for Coil*
- Supply Fan Type for Coil*
- Plant Name for Coil*
- Airloop Name
- Airloop Branch Name
- Location count on Airloop Branch
- Plant Loop Name
- Plant Branch Name
- Location count on Plantloop Branch

Some of these columns (shown with asterisks) are currently in the "Coils" table and could be removed from
that table since it is very wide already, or they could be kept for compatibility.

#### Controls ####

A new report that includes Setpoint Managers, Controllers, Availability Managers, PlantEquipmentOperation, and components
with control options and identifies the type, where they apply, sense, and control parameters from the input. Need:

- minimum and maximum setpoint temperatures
- minimum turndown ratio
- schedules
- load ranges
- Setpoint at Outdoor Low Temperature
- Outdoor Low Temperature
- Setpoint at Outdoor High Temperature
- Outdoor High Temperature

This is probably a series of tables, one for each input object. The report would include the pertinent information,
including the controls, related temperatures, and what they apply to but would generally be limited to what is already
available within the data structure that would be useful to report. Include specifically:

- Heat Recovery - Operation With Economizer
- Heat Recovery - Supply Air Temperature Control

#### Equipment Summary - Chiller ####

A new report for Chillers.
 - Type
 - Reference Capacity [W]
 - Reference Efficiency [W/W]
 - Rated Capacity [W]
 - Rated Efficiency [W/W]
 - IPLV in SI Units [W/W]
 - IPLV in IP Units [Btu/W-h]
 - Plantloop name
 - Plantloop branch name
 - Condenser loop name
 - Condenser loop branch name
 - Minimum part load ratio
 - Fuel type
 - Rated entering condenser temperature
 - Rated leaving evaporator temperature
 - Reference entering condenser temperature
 - Reference leaving evaporator temperature
 - Design Size Reference Chilled Water Flow Rate
 - Design Size Reference Condenser Fluid Flow Rate
 - Heat recovery Plantloop name
 - Heat recovery Plantloop branch name
 - Recovery Relative Capacity Fraction

#### Equipment Summary - Boiler ####

A new report for Boilers.
 - Type
 - Reference Capacity [W]
 - Reference Efficiency [W/W]
 - Rated Capacity [W]
 - Rated Efficiency [W/W]
 - Plantloop name
 - Plantloop branch name
 - Minimum part load ratio
 - Fuel type
 - Parasitic electric load


#### Equipment Summary - cooling towers and fluid coolers ####

A new report for cooling towers and fluid coolers.
 - Type
 - Condenser loop name
 - Condenser loop branch name
 - Fluid type
 - Range
 - Approach
 - Design Fan Power
 - Design inlet air wet-bulb temperature
 - Design Water Flow Rate
 - Leaving water setpoint temperature


#### Equipment Summary - PlantLoop or CondenserLoop ####

A new report for PlantLoop objects. One row for each Plant or CondenserLoop
- name
- type (PlantLoop or CondenserLoop)
- provides heating
- provides cooling
- Maximum Loop Flow Rate
- Minimum Loop Flow Rate
- Total pump power on loop
- Branch name

#### Equipment Summary - AirTerminals ####

A new report for AirTerminals.
- name
- type of input object
- Heat/Reheat Coil Object Type
- Zone Name
- Hot Water Plant Loop Name
- Hot Water Plant Branch Name
- Chilled Water Coil Object Type
- Chilled Water Plant Loop Name
- Chilled Water Plant Branch Name
- Fan Object Type
- Fan Name
- Primary Air Flow Rate
- Secondary Air Flow Rate
- Minimum Flow
- Minimum Flow Schedule Name
- Maximum Flow During Reheat
- Minimum Outdoor Flow
- Minimum Outdoor Flow Schedule Name
- Supply cooling setpoint
- Supply heating setpoint
- Temperature control
- Heating capacity
- Cooling capacity

#### Equipment Summary - Air Heat Recovery ####

A new report for each HeatExchanger:AirToAir:SensibleAndLatent and HeatExchanger:AirToAir:FlatPlate:
- name
- input object type
- plate/rotary
- Sensible Effectiveness at 100% Heating Air Flow
- Sensible Effectiveness at 100% Cooling Air Flow
- Latent Effectiveness at 100% Heating Air Flow
- Latent Effectiveness at 100% Cooling Air Flow
- Exhaust airflow
- Outdoor airflow

We might want to add the other effectivenesses at different air flow rates to make the report more comprehensive.

## Testing/Validation/Data Sources ##

Since the output JSON file needs to comply with a JSON schema, it should be easy to confirm that it is valid.
The Python jsonschema library can be used to confirm that the RMD file is consistent with the schema. Comparison
of input and related outputs should provide the final check if the reporting is being done correctly.

## Input Output Reference Documentation ##

No specific changes for input are expected.

Additional tabular outputs will be described in the IOref.

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

The order recommended in the Prioritization section above will be followed.

Echos of input are indicated in the eptags schema file.

Provide draft updated output from EnergyPlus for OpenStudio team to test prior to release.

For the createRulesetModelDecription Python script, will continue being developed using the same approach
as the 2022 work and will continue to include unit tests. This includes:

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

https://github.com/NREL/EnergyPlus/blob/develop/design/FY2022/NFP-InitialRulesetModelDescription.md

The repo for the script development is here:

https://github.com/JasonGlazer/createRulesetModelDescription

Some lessons learned from the initial effort in 2022 are described here:

https://github.com/JasonGlazer/createRulesetModelDescription/blob/main/docs/lessons_learned.rst

The schema is described here:

https://github.com/open229/ruleset-model-description-schema/blob/main/schema-source/ASHRAE229_extra.schema.yaml
