Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### Output:Constructions for WindowMaterial:Shade
Data for Output:Constructions for WindowMaterial:Shade were misaligned with the column headings in the eio output and the table Initialization Summary report. Data has shifted one column to the left.

See PR [#10750](https://github.com/NatLabRockies/EnergyPlus/pull/10750)

### Table Output, Sizing Output, and Output:Variable Spelling Corrections
* "Equipment Summary" table report, sub-table "VAV DX Cooling Standard Rating Details", column heading, "Assocated Fan" --> "Associated Fan".

* "Equipment Summary" table report, sub-table "Air Heat Recovery", column "Input object type", "Dessicant Balanced" --> "Desiccant Balanced" (for object type HeatExchanger:Desiccant:BalancedFlow).

* Output:Variable "Zone Hybrid Unitary HVAC DehumidificationLoad to Humidistat Setpoint Heat Tansfer Energy" --> "Zone Hybrid Unitary HVAC Dehumidification Load to Humidistat Setpoint Heat Transfer Energy".

* Output:Variable "Zone Hybrid Unitary HVAC Humidification Load to Humidistat Setpoint Heat Tansfer Energy" --> "Zone Hybrid Unitary HVAC Humidification Load to Humidistat Setpoint Heat Transfer Energy".

* Sizing output for ZoneHVAC:Baseboard:RadiantConvective:Steam, "User-Speicified Maximum Steam Flow Rate [m3/s]" --> "User-Specified Maximum Steam Flow Rate [m3/s]".

* Sizing output for Chiller:Absorption, "Iniital Design Size Design Generator Fluid Flow Rate [m3/s]" --> "Initial Design Size Design Generator Fluid Flow Rate [m3/s]"

* eio output header for "\<ShadingProperty Reflectance\>", "Contruction" --> "Construction". Also appears in the "Initialization Summary" table output, "ShadingProperty Reflectance" sub-table, column heading.

See Pull Request [#10760](https://github.com/NatLabRockies/EnergyPlus/pull/10760).

### Table Output, Envelope Summary Report, Add Space and Zone Columns
* Added a column for *Space* to sub-tables "Opaque Exterior" and "Opaque Interior".

* Added columns for *Zone* and *Space* to sub-tables "Exterior Fenestration", "Interior Fenestration", "Exterior Door", and "Interior Door".

See Pull Request [#10914](https://github.com/NatLabRockies/EnergyPlus/pull/10914).

### Infiltration and Ventilation Output Variable Name Change and New Outputs
* Changed the name of these output variables:
```
Infiltration Air Change Rate --> Infiltration Current Density Air Change Rate
Zone Infiltration Air Change Rate --> Zone Infiltration Current Density Air Change Rate
Zone Ventilation Air Change Rate --> Zone Ventilation Current Density Air Change Rate
```

* Added new output variables:
```
Infiltration Outdoor Density Volume Flow Rate
Infiltration Standard Density Air Change Rate
Infiltration Outdoor Density Air Change Rate
Zone Infiltration Outdoor Density Volume Flow Rate
Zone Infiltration Standard Density Air Change Rate
Zone Infiltration Outdoor Density Air Change Rate
Zone Ventilation Outdoor Density Volume Flow Rate
Zone Ventilation Standard Density Air Change Rate
Zone Ventilation Outdoor Density Air Change Rate
```

See Pull Request [#10940](https://github.com/NatLabRockies/EnergyPlus/pull/10940).
