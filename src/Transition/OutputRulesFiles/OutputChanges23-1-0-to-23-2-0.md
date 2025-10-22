Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### EIO System Sizing Information User Design Capacity header
Missing unit is added to the EIO System Sizing Information table "User Design Capacity" header as shown below:
- <System Sizing Information>, System Name, Load Type, Peak Load Kind, User Design Capacity [W], Calc Des Air Flow Rate [m3/s], User Des Air Flow Rate [m3/s], Design Day Name, Date/Time of Peak
- This change also impacts the html tabular output report file: report name "Initialization Summary" and table name "System Sizing Information".

See pull request [#9967](https://github.com/NREL/EnergyPlus/pull/9967) for more details.

### Std 229 Ruleset Description Model (Phase 2) tabular output changes

This work started since 2022 aims to develop a script that creates a JSON file consistent with the ASHRAE Standard 229 Ruleset Model Description schema to show the feasibility of the schema, uncover problems with implementation, and provide an implementation for early adopters. The full title for ASHRAE Standard 229 is "Protocols for Evaluating Ruleset Application in Building Performance Models." This standard establishes tests and acceptance criteria for application of rulesets and related reporting for building performance models; and applies to evaluation of the implementation of rulesets associated with new or existing buildings, their systems, controls, sites, and other aspects described by the ruleset. It establishes requirements for: (2.1) building performance modeling software; and (2.2) software that evaluates building performance models and associated information to check the application of a ruleset. Currently, ASHRAE Standard 229 has not been published or even gone through public review and is under development by the ASHRAE SPC 229P committee. However, the intention of the standard is to provide code officials and rating authorities with files that they can use with a Ruleset Checking Tool (currently, an example is under development at PNNL) to automatically check if a ruleset (such as 90.1 Appendix G, RESNET, California Title 24 performance paths, or Canada National Energy Code for Buildings performance path) has been implemented correctly.

Since each EnergyPlus IDF file could generate an RMD file, the Ruleset Checking Tool will be able to see if the changes between the RMD files correspond to rules in the ruleset by looking at both the baseline and proposed RMD file. The work in the current stage prioritize the support on the selection of baseline HVAC system in Appendix G which includes changes to or new EnergyPlus reports:

#### Adding reporting entries to existing tabular reports

New variables related to Std 229 are appended to the right of the following existing tables:

- Equipment Summary - Heating Coils
- Equipment Summary - DX Heating Coils
- Equipment Summary - Fans
- Equipment Summary - Pumps
- Component Sizing Summary - PlantLoop

#### Adding additional central plant and equipment reports

The following new tables or subtables are created:

- Equipment Summary Coil Connections
- Equipment Summary - Chillers
- Equipment Summary - Boiler
- Equipment Summary - Fluid Coolers
- Equipment Summary - PlantLoop and CondenserLoop
- Equipment Summary - Air Terminals
- Equipment Summary - Air Heat Recovery

See Pull Request [#10051] (https://github.com/NREL/EnergyPlus/pull/10051) for more details about the NFP; and [#10143] (https://github.com/NREL/EnergyPlus/pull/10143) for details about current implementations.

### Space Sizing Output in EIO and Tables

When `ZoneAirHeatBalanceAlgorithm` "Do Space Heat Balance for Sizing" = Yes, zone sizing is also done for all spaces. The HVAC Sizing Summary table report will include subtables for Space Sensible Cooling and Heating as well as for Zone Sensible Cooling and Heating. Space Sizing will also be reported to the eio output.

See pull request [#9982](https://github.com/NREL/EnergyPlus/pull/9982) for more details.


### Output:Variables, Output:Meter, Output:Meter:MeterFileOnly, Output:Table:Monthly

*FuelType* changed:
 (a) "District Heating" to "District Heating Water"
 (b) "DistrictHeating" to "DistrictHeatingWater"
 (c) "Steam" to "District Heating Steam"
 (d) "Steam" to "DistrictHeatingSteam"

 See [9260](https://github.com/NREL/EnergyPlus/pull/9260)

### Table output headings moving from:

 (a) "District Heating" to "District Heating Water"
 (b) "DistrictHeating" to "DistrictHeatingWater"
 (c) "Steam" to "District Heating Steam"
 (d) "Steam" to "DistrictHeatingSteam"

 See [9260](https://github.com/NREL/EnergyPlus/pull/9260)

### WaterHeaterReportMonthly report

Column "Water Heater Source Energy []" renamed to "Water Heater Source Side Heat Transfer Energy [J]".

See [10209](https://github.com/NREL/EnergyPlus/pull/10209)
### Heat Pump Water Heater Information table

Columns `"DX Coil Total Cooling Rate {W` and `HPWH Only}"` have been merged into a single `DX Coil Total Cooling Rate {W}`.

 See [10214](https://github.com/NREL/EnergyPlus/pull/10214)
