Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### Additional field for Equipment Summary:Fans report

A new column "Fan Energy Index" has been added to the Equipment Summary:Fans report

See [6812](https://github.com/NatLabRockies/EnergyPlus/pull/6812)

### Corrected EIO output of Advanced Single-Sided Natural Ventilation Wind Angles

The advanced single-sided natural ventilation feature uses a higher-resolution table of wind angles, and these angles are now correctly represented in the EIO file.

### Reporting of Year numbers

Simulation progress messages, now include the simulation year in the date stamp. e.g.
```
Updating Shadowing Calculations, Start Date=01/21/2020
Continuing Simulation at 01/21/2020 for RUNPERIOD 1
```
eio output for RunPeriod Environment now includes the simulation year for Start Date and End Date
```
! <Environment>,Environment Name,Environment Type, Start Date, End Date, Start DayOfWeek, Duration {#days}, Source:Start DayOfWeek,  Use Daylight Saving, Use Holidays, Apply Weekend Holiday Rule,  Use Rain Values, Use Snow Values
Environment,RUNPERIOD 1,WeatherFileRunPeriod,01/01/2019,12/31/2019,Tuesday,365,Use RunPeriod Specified Day,Yes,Yes,No,Yes,Yes
```


See [6478](https://github.com/NatLabRockies/EnergyPlus/pull/6478)
