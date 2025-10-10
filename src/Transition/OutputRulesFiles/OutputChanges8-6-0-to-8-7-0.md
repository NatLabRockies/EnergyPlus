Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### New table, Hybrid Model: Internal Thermal Mass, is added to tabular report.

The table is only shown when hybrid model: internal thermal mass is flagged.  It has two columns, the first column indicates Hybrid Modeling (Y/N) and the second column provides Temperature Capacitance Multiplier values for each zone.

See [5882](https://github.com/NREL/EnergyPlus/pull/5882)

### Daylighting Table Fixes in Lighting Summary tabular report

The column labeled Zone used to show the daylighting control name but has been fixed to show the zone name. In addition, the control name now appears in a column with the heading control name. Also the column heading "Daylighting Type" was changed to "Daylighting Method" to be more consistent with the input field of that it is reporting. This column also was always just showing "Detailed" previously and now shows either SplitFlux or DElight as appropriate.

See [5945](https://github.com/NREL/EnergyPlus/pull/5945).

### Initialization Summary and EIO file

A new tabular report was added called the Initialization Summary that includes the contents of the EIO file. The new report contains many tables one for each "heading" line shown in the EIO file with lines of data shown as rows in the table. Due to this, the new Initialization Summary report contains a large number of tables. Some of the heading and data lines in the EIO file needed to be changed in order for the parsing routine to work consistently. The data shown in the various lines in the EIO file were not changed but the formatting for some lines were changed. The changed lines in the EIO file are described below.

The following added explicit monthly columns to the heading rows of the EIO file.

- Site:GroundTemperature:BuildingSurface
- Site:GroundTemperature:Deep
- Site:GroundTemperature:FCfactorMethod
- Site:GroundTemperature:Shallow
- Site:GroundReflectance
- Site:GroundReflectance:Snow
- Site:GroundReflectance:Snow:Daylighting

For the following the word "nominal" was shown in the data row and the heading row removed the dash before the word nominal:

- Airflow Stats Nominal
- Internal Gains Nominal

Heading rows that represented multiple data rows using slashes were broken up into individual heading rows:

- "Zone Internal Gains/Equipment Information" changed to separate rows labeled "Zone Internal Gains Nominal" and "Equipment Gains Nominal"
- "Zone/Shading Surfaces" changed to separate rows labeled "Zone Surfaces" and "Shading Surfaces"
- "HeatTransfer/Shading/Frame/Divider_Surface" changed to separate rows labeled "HeatTransfer Surface", "Shading Surface" and "Frame/Divider Surface"
- For these surface reports, the separate "Units" header has been deleted and the units are part of each heading row.

Underscores were removed from row labels:

- Heading rows with labels ending with "_Surface" have been changed to ending with " Surface"
- "Shading_Surfaces" was changed to "Shading Surfaces"
- "Zone_Surfaces" was changed to "Zone Surfaces"
- "Environment:Design_Day_Misc" was changed to "Environment:Design Day Misc"

In addition:

- Data rows labeled "Infiltration" have been changed to be "ZoneInfiltration" to correspond with the appropriate heading row
- Heading rows labeled "<Shadowing/Sun Position Calculations> [Annual Simulations]" was changed to "<Shadowing/Sun Position Calculations Annual Simulations>" and the corresponding data rows labled "Shadowing/Sun Position Calculations" were changed to "Shadowing/Sun Position Calculations Annual Simulations"
- Heading rows labeled "SurfaceGeometry" were changed to "Surface Geometry"
- In the heading row "Environment:Weather Station" the column headings "Wind Speed Modifier Coefficient [Internal]" and "Temperature Modifier Coefficient [Internal]" were changed to "Wind Speed Modifier Coefficient-Internal" and "Temperature Modifier Coefficient-Internal"

See [5928](https://github.com/NREL/EnergyPlus/pull/5928).
See also [6002](https://github.com/NREL/EnergyPlus/pull/6002).

