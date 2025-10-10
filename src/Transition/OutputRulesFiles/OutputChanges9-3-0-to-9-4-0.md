Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.


### Daylight Factors output in EIO and DFS now supports more than two reference points

[PR#8017](https://github.com/NREL/EnergyPlus/pull/8017) changed the output format in both EIO and DFS (when using `Output:DaylightFactors`)
to support more than two `Daylighting:ReferencePoint`.

#### EIO

The old format looked like the following, and when no second Reference Point exited, it reported zero for that point. Any additional reference point was not reported.

```
! <Sky Daylight Factors>, MonthAndDay, Zone Name, Window Name, Daylight Fac: Ref Pt #1, Daylight Fac: Ref Pt #2
 Clear Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.9074E-002,1.8074E-002
 Clear Turbid Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.4830E-002,1.4700E-002
 Intermediate Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.0647E-002,1.0547E-002
 Overcast Sky Daylight Factors,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,1.4127E-002,1.3127E-002
```

The new format looks like the following, and specifies the name of the reference point in question,
as well as conforming to EIO format by adding the section name `Sky Daylight Factors` to each record:

```
! <Sky Daylight Factors>, MonthAndDay, Zone Name, Window Name, Reference Point, Daylight Factor
 Sky Daylight Factors,Clear Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.9074E-002
 Sky Daylight Factors,Clear Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.8074E-002
 Sky Daylight Factors,Clear Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.7074E-002
 Sky Daylight Factors,Clear Turbid Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.4830E-002
 Sky Daylight Factors,Clear Turbid Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.4700E-002
 Sky Daylight Factors,Clear Turbid Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.4230E-002
 Sky Daylight Factors,Intermediate Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.0647E-002
 Sky Daylight Factors,Intermediate Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.0547E-002
 Sky Daylight Factors,Intermediate Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.0347E-002
 Sky Daylight Factors,Overcast Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT1,1.4127E-002
 Sky Daylight Factors,Overcast Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT2,1.3127E-002
 Sky Daylight Factors,Overcast Sky,01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,ZN_1_DAYLREFPT3,1.2127E-002
```

#### DFS

The old format looked like this, each actual line of record included the hour, then 2 groups of 4 different Sky Conditions corresponding to two reference points.
The second group was always zero if only one reference point existed (see the 2nd line of the header):

```
This file contains daylight factors for all exterior windows of daylight zones.
If only one reference point the last 4 columns in the data will be zero.
MonthAndDay,Zone Name,Window Name,Window State
Hour,Daylight Factor for Clear Sky at Reference point 1,Daylight Factor for Clear Turbid Sky at Reference point 1,Daylight Factor for Intermediate Sky at Reference point 1,Daylight Factor for Overcast Sky at Reference point 1,Daylight Factor for Clear Sky at Reference point 2,Daylight Factor for Clear Turbid Sky at Reference point 2,Daylight Factor for Intermediate Sky at Reference point 2,Daylight Factor for Overcast Sky at Reference point 2
01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,Base Window
1,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000
[...]
9,2.14657E-002,1.73681E-002,1.38241E-002,1.41272E-002,2.14657E-002,1.73680E-002,1.38240E-002,1.41273E-002
[...]
24,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000
```

In the new format, one line was stripped from the header, and the name of the reference point was added to each record. It now supports more than 2 reference points.
The former format for `Window State` used a *blank* to indicate a window in a shaded state by shades, screens, or blinds with a fixed slat angle.
Instead of a blank, the new format now uses `Blind or Slat Applied`.

```
This file contains daylight factors for all exterior windows of daylight zones.
MonthAndDay,Zone Name,Window Name,Window State
Hour,Reference Point,Daylight Factor for Clear Sky,Daylight Factor for Clear Turbid Sky,Daylight Factor for Intermediate Sky,Daylight Factor for Overcast Sky
01/21,ZN_1,ZN_1_WALL_NORTH_WINDOW,Base Window
1,ZN_1_DAYLREFPT1,0.00000,0.00000,0.00000,0.00000
1,ZN_1_DAYLREFPT2,0.00000,0.00000,0.00000,0.00000
1,ZN_1_DAYLREFPT3,0.00000,0.00000,0.00000,0.00000
[...]
9,ZN_1_DAYLREFPT1,2.14657E-002,1.73681E-002,1.38241E-002,1.41272E-002
9,ZN_1_DAYLREFPT2,2.14657E-002,1.73680E-002,1.38240E-002,1.41273E-002
9,ZN_1_DAYLREFPT3,2.14655E-002,1.73678E-002,1.38239E-002,1.41274E-002
[...]
24,ZN_1_DAYLREFPT1,0.00000,0.00000,0.00000,0.00000
24,ZN_1_DAYLREFPT2,0.00000,0.00000,0.00000,0.00000
24,ZN_1_DAYLREFPT3,0.00000,0.00000,0.00000,0.00000
```
### Surface Order in Output Reports

The internal ordering of surfaces has changed. Previously subsurfaces (doors and windows) immdediately followed their respective base surface.
Now subsurfaces are at the end of each group of zone surfaces.Many reports preserve the old order, but some outputs do not.
Changed outputs include the rdd, edd, eso (and resulting csv), shd, and sci output files.

See [PR#7847](https://github.com/NREL/EnergyPlus/pull/7847)

### Report zero values with zero Zone Cooling and Heating Loads in Report: HVAC Sizing Summary

When Zone Sensible Cooling = 0 or Zone Sensible Heating = 0, empty values are shown in the table:

Zone Sensible Cooling

	Calculated Design Load [W] 	User Design Load [W] 	User Design Load per Area [W/m2] 	Calculated Design Air Flow [m3/s] 	User Design Air Flow [m3/s] 	Design Day Name 	Date/Time Of Peak {TIMESTAMP} 	Thermostat Setpoint Temperature at Peak Load [C] 	Indoor Temperature at Peak Load [C] 	Indoor Humidity Ratio at Peak Load [kgWater/kgDryAir] 	Outdoor Temperature at Peak Load [C] 	Outdoor Humidity Ratio at Peak Load [kgWater/kgDryAir] 	Minimum Outdoor Air Flow Rate [m3/s] 	Heat Gain Rate from DOAS [W]
None

This causes difficulty for SQLite to retrieve data.

The fix reports zero values when loads = 0, so that SQLite is able to process non-empty values, shown as below:

Zone Sensible Cooling

	Calculated Design Load [W] 	User Design Load [W] 	User Design Load per Area [W/m2] 	Calculated Design Air Flow [m3/s] 	User Design Air Flow [m3/s] 	Design Day Name 	Date/Time Of Peak {TIMESTAMP} 	Thermostat Setpoint Temperature at Peak Load [C] 	Indoor Temperature at Peak Load [C] 	Indoor Humidity Ratio at Peak Load [kgWater/kgDryAir] 	Outdoor Temperature at Peak Load [C] 	Outdoor Humidity Ratio at Peak Load [kgWater/kgDryAir] 	Minimum Outdoor Air Flow Rate [m3/s] 	Heat Gain Rate from DOAS [W]
LIVING SPACE 	0.0 	0.0 	0.0 	0.0 	0.0 	N/A 	N/A 	0.0 	0.0 	0.0 	0.0 	0.0 	0.0 	0.0

The fix is applied both both tables of Zone Sensible Cooling abd Zone Sensible Heating.

See [PR#8145](https://github.com/NREL/EnergyPlus/pull/8145)

### New reporting items added to the _perflog.csv log file for PerformancePrecisionTradeoffs

Two new reporting variables (two columns) related to the new PerformancePrecisionTradeoffs modes are added to the _perflog.csv log file to help with performance tuning of newly added and expanded PerformancePrecisionTradeoffs modes.
In the log file with _perflog.csv suffix, each simulation will add a line that contains the following the reporting variables (items) below. The two newly added reporting items (Minimum System Timestep, and MaxAllowedDelTemp) are denoted with (*) marks in the following table:

- Program
- Version
- TimeStamp
- Use Coil Direct Solution
- Zone Radiant Exchange Algorithm
- Number of Timesteps per Hour
- Minimum Number of Warmup Days
- SuppressAllBeginEnvironmentResets
- Minimum System Timestep (*)
- MaxZoneTempDiff
- MaxAllowedDelTemp (*)
- Electricity ABUPS Total [J]
- Natural Gas ABUPS Total [J]
- Additional Fuel ABUPS Total [J]
- District Cooling ABUPS Total [J]
- District Heating ABUPS Total [J]
- Water ABUPS Total [m3]
- Values Gathered Over [hours]
- Run Time [seconds]
- Run Time [string]
- Number of Warnings
- Number of Severe

See [8121](https://github.com/NREL/EnergyPlus/pull/8121)

### EIO reporting items added for PerformancePrecisionTradeoffs

Two new reporting variables (two columns) related to the new PerformancePrecisionTradeoffs modes are added to the .eio file to help with tracing the and tuning of newly added and expanded PerformancePrecisionTradeoffs modes.
In the PerformancePrecisionTradeoffs section of .eio file, a line of header of parameter names and a line of corresponding parameter values are reported. Two newly added reporting items (Minimum System Timestep, and MaxAllowedDelTemp) are now added to reporting lines: the parameter names are added to the headline (first line); and the parameter values are added to the data line (secon line). Now the new reporting lines look like the following:

! <Performance Precision Tradeoffs>, Use Coil Direct Simulation, Zone Radiant Exchange Algorithm, Override Mode, Number of Timestep In Hour, Force Euler Method, Minimum Number of Warmup Days, Force Suppress All Begin Environment Resets, Minimum System Timestep, MaxZoneTempDiff, MaxAllowedDelTemp
 Performance Precision Tradeoffs, No, ScriptF, MODE07, 1, Yes, 1, Yes, 60.0, 1.000, 0.1000

See [8121](https://github.com/NREL/EnergyPlus/pull/8121)

### Output:Variables, Output:Meter, Output:Meter:MeterFileOnly, Output:Table:Monthly

*FuelType* changed:

(a) From "FuelOil#1" in 9.3 to "FuelOilNo1" in 9.4
(b) From "FuelOil#2" in 9.3 to "FuelOilNo2" in 9.4
(c) From "Fuel Oil #1" in 9.3 to "FuelOilNo1" in 9.4
(d) From "Fuel Oil #2" in 9.3 to "FuelOilNo2" in 9.4

### table output headings moving from:
 (a) "Fuel Oil #1" in 9.3 to "Fuel Oil No 1" in 9.4
 (b) "Fuel Oil #2" in 9.3 to "Fuel Oil No 2" in 9.4

See [8301](https://github.com/NREL/EnergyPlus/pull/8304)