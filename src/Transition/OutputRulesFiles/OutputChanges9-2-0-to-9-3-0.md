Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### Additional field for Coil Sizing Details:Coils report

A new column "Peak Load Type to Size On" has been added to the `Coil Sizing Details:Coils` report

See [7397](https://github.com/NatLabRockies/EnergyPlus/pull/7397)

### Name re-ordering in the sizing labels and modified variable names for MulitSpeed DX Cooling and Heating Coils

The change impacts rated capacity, rated flow rate, rated sensible heat ratio, etc, see sample changes below


*Before:*
```
'Speed X' Design Size Design Size Rated Total Cooling Capacity
'Speed X' Design Size Gross Rated Heating Capacity
'Speed X' Design Size Rated Air Flow Rate
'Speed X' Design Size Rated Sensible Heat Ratio
'Speed X' Design Size Evaporative Condenser Air Flow Rate
'Speed X' Design Size Rated Evaporative Condenser Pump Power Consumption

'Speed X' User-Specified User-Specified Total Cooling Capacity
'Speed X' User-Specified Gross Rated Heating Capacity
'Speed X' User-Specified Rated Air Flow Rate
'Speed X' User-Specified Rated Sensible Heat Ratio
'Speed X' User-Specified Evaporative Condenser Air Flow Rate
'Speed X' User-Specified Rated Evaporative Condenser Pump Power Consumption
```

*After:*
```
Design Size 'Speed X' Gross Rated Total Cooling Capacity
Design Size 'Speed X' Gross Rated Heating Capacity
Design Size 'Speed X' Rated Air Flow Rate
Design Size 'Speed X' Rated Sensible Heat Ratio
Design Size 'Speed X' Evaporative Condenser Air Flow Rate
Design Size 'Speed X' Rated Evaporative Condenser Pump Power Consumption

User-Specified 'Speed X' Gross Rated Total Cooling Capacity
User-Specified 'Speed X' Gross Rated Heating Capacity
User-Specified 'Speed X' Rated Air Flow Rate
User-Specified 'Speed X' Rated Sensible Heat Ratio
User-Specified 'Speed X' Evaporative Condenser Air Flow Rate
User-Specified 'Speed X' Rated Evaporative Condenser Pump Power Consumption
```

### EIO output for SizingPeriod:DesignDay

In the eio and table output related to `SizingPeriod:DesignDay`, the Humidity Indicating Type and Value are now correctly reported. Example:

*Before:*
```
! <Environment:Design Day Data>, Max Dry-Bulb Temp {C}, Temp Range {dC}, Temp Range Ind Type, Hum Ind Value at Max Temp, Hum Ind Type,Pressure {Pa}, Wind Direction {deg CW from N}, Wind Speed {m/s}, Clearness, Rain, Snow
Environment:Design Day Data,33.00,6.60,DefaultMultipliers,100511,220,3.2,0.00,No,No
```

If we realign to the header, we see that two values related to Humidity Indicating Value were missing.

```
# Realign to header
Environment:Design Day Data,33.00,6.60,DefaultMultipliers,<MISSING>,<MISSING>,100511,220,3.2,0.00,No,No
```

*v9.3.0:* The `Hum Ind Type` and `Hum Ind Value at Max Temp` are swapped and actually output to the EIO, and the `Hum Ind Units` is added:

```
! <Environment:Design Day Data>, Max Dry-Bulb Temp {C}, Temp Range {dC}, Temp Range Ind Type, Hum Ind Type, Hum Ind Value at Max Temp, Hum Ind Units, Pressure {Pa}, Wind Direction {deg CW from N}, Wind Speed {m/s}, Clearness, Rain, Snow
Environment:Design Day Data,33.00,6.60,DefaultMultipliers,Enthalpy,90500.00,{J/kgDryAir},100511,220,3.2,0.00,No,No
```

See [#PR7577](https://github.com/NatLabRockies/EnergyPlus/pull/7577)

### EIO output for ShadowCalculations

The eio output related to `ShadowCalculations` has been changed to reflect the new input fields. See Rules9-2-0-to-9-3-0.md.

See [#PR7302](https://github.com/NatLabRockies/EnergyPlus/pull/7302)

### End Use By Subcategory in SQL

In the SQL Output file, for `ReportName = "AnnualBuildingUtilityPerformanceSummary"` and `ReportName = "DemandEndUseComponentsSummary"`,
the tables `TableName = "End Uses by Subcategory"` have been refactored. `RowName` is now in the format `<End Use Category>:<End Use Subcategory>`.
This will allow querying a specific End Use Subcategory in the SQL file more easily.

Example SQL Queries:

* Get the Value corresponding to a specific Fuel Type "Electricity", End Use "Interior Lighting", Subcategory "GeneralLights":

```sql
SELECT Value FROM TabularDataWithStrings
  WHERE TableName = 'End Uses By Subcategory'
  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'
  AND ColumnName = 'Electricity'
  AND RowName = 'Interior Lighting:GeneralLights'
```

* Return all rows (one row per fuel type) for End Use "Interior Lighting", Subcategory "GeneralLights":

```sql
SELECT ColumnName as FuelType, Value FROM TabularDataWithStrings
  WHERE TableName = 'End Uses By Subcategory'
  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'
  AND RowName = 'Interior Lighting:GeneralLights'
```

| FuelType         | Value |
|------------------|-------|
| Electricity      | 83.33 |
| Natural Gas      | 0.00  |
| Additional Fuel  | 0.00  |
| District Cooling | 0.00  |
| District Heating | 0.00  |
| Water            | 0.00  |

* Get all rows related to Electricity usage by Interior Lighting:

```sql
SELECT RowName as "End Use&Subcategory", Value FROM TabularDataWithStrings
  WHERE TableName = 'End Uses By Subcategory'
  AND ReportName = 'AnnualBuildingUtilityPerformanceSummary'
  AND ColumnName = 'Electricity'
  AND RowName LIKE 'Interior Lighting:%'
```

| End Use&Subcategory                   | Value  |
|---------------------------------------|--------|
| Interior Lighting:GeneralLights       | 166.67 |
| Interior Lighting:AnotherEndUseSubCat | 83.33  |

See [PR#7584](https://github.com/NatLabRockies/EnergyPlus/pull/7584).

### Standardize units for humidity ratio and add where missing

Units for humidity ratio standardized to "kgWater/kgDryAir" and "lbWater/lbDryAir".

Impacts eio sizing output and table reports including Coil Sizing Summary and Details.

See [7571](https://github.com/NatLabRockies/EnergyPlus/pull/7571)


### Water to Water Air-Source EIR Heat Pump
The HeatPump:WaterToWater:EIR:Heating and HeatPump:WaterToWater:EIR:Cooling objects have been renamed to HeatPump:PlantLoop:EIR:Heating and HeatPump:PlantLoop:EIR:Cooling, and the respective output variables have been renamed from:

*Before*
```
Water to Water Heat Pump Load Side Heat Transfer Rate
Water to Water Heat Pump Load Side Heat Transfer Energy
Water to Water Heat Pump Source Side Heat Transfer Rate
Water to Water Heat Pump Source Side Heat Transfer Energy
Water to Water Heat Pump Load Side Inlet Temperature
Water to Water Heat Pump Load Side Outlet Temperature
Water to Water Heat Pump Source Side Inlet Temperature
Water to Water Heat Pump Source Side Outlet Temperature
Water to Water Heat Pump Electric Power
Water to Water Heat Pump Electric Energy
Water to Water Heat Pump Load Side Mass Flow Rate
Water to Water Heat Pump Source Side Mass Flow Rate
```

*9.3.0*
```
Heat Pump Load Side Heat Transfer Rate
Heat Pump Load Side Heat Transfer Energy
Heat Pump Source Side Heat Transfer Rate
Heat Pump Source Side Heat Transfer Energy
Heat Pump Load Side Inlet Temperature
Heat Pump Load Side Outlet Temperature
Heat Pump Source Side Inlet Temperature
Heat Pump Source Side Outlet Temperature
Heat Pump Electric Power
Heat Pump Electric Energy
Heat Pump Load Side Mass Flow Rate
Heat Pump Source Side Mass Flow Rate
```

See [7489](https://github.com/NatLabRockies/EnergyPlus/pull/7489/)

### EIO output for Environment

In the eio and table output related to `Environment`, the Sky Temperature Model are now correctly reported at the end. Example:

*Before:*
```
! <Environment>,Environment Name,Environment Type, Start Date, End Date, Start DayOfWeek, Duration {#days}, Source:Start DayOfWeek,  Use Daylight Saving, Use Holidays, Apply Weekend Holiday Rule,  Use Rain Values, Use Snow Values
Environment,DENVER CENTENNIAL GOLDEN ANN HTG 99% CONDNS DB DEFAULT,SizingPeriod:DesignDay,12/21,12/21,WinterDesignDay,1,N/A,N/A,N/A,N/A,N/A,N/A
```

*After:*
```
! <Environment>,Environment Name,Environment Type, Start Date, End Date, Start DayOfWeek, Duration {#days}, Source:Start DayOfWeek,  Use Daylight Saving, Use Holidays, Apply Weekend Holiday Rule,  Use Rain Values, Use Snow Values, Sky Temperature Model
Environment,DENVER CENTENNIAL GOLDEN ANN HTG 99% CONDNS DB DEFAULT,SizingPeriod:DesignDay,12/21,12/21,WinterDesignDay,1,N/A,N/A,N/A,N/A,N/A,N/A,Clark and Allen
```

See [PR #7562](https://github.com/NatLabRockies/EnergyPlus/pull/7562)

### EIO output for PerformancePrecisionTradeoffs
In the eio and table output, new lines report the status of options for the PerformancePrecisionTradeoffs object.

See [PR #7743](https://github.com/NatLabRockies/EnergyPlus/pull/7743/)

### Default end use subcategory to General
In the AUBPS end-use subcategory table, the "other" row would show up whenever an end-use subcategory
was not specified. Now unless specified, end-uses are assumed to be "general" and show up in that row.
It is now unlikely that the "other" row will be shown.

This results in additional end-use meters, e.g. General:Cooling:Electricity which may have not been present before.
And it results in more outputs attached to the General:* meters.

See [PR #7794](https://github.com/NatLabRockies/EnergyPlus/pull/7794)
See [7743](https://github.com/NatLabRockies/EnergyPlus/pull/7743/)

See [7761](https://github.com/NatLabRockies/EnergyPlus/pull/7761)
and the [NFP](https://github.com/energy-plus/EnergyPlus/blob/PerfOverrideAndReporting/design/FY2020/NFP-PerformanceOverRidesAndReporting.md)

### New _perflog.csv file as Log File for PerformancePrecisionTradeoffs

A new file has been added with the suffix _perflog.csv specifically to help with
performance tuning for a specific input file. Instead of disappearing, it gets
appended to each simulation so experimentation can be done. It appears when
ever the PerformancePrecisionTradeoffs object is used. For each simulation a line is added
that contains the following:

- Program
- Version
- TimeStamp
- Use Coil Direct Solution
- Zone Radiant Exchange Algorithm
- Number of Timesteps per Hour
- Minimum Number of Warmup Days
- SuppressAllBeginEnvironmentResets
- MaxZoneTempDiff
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

See [7761](https://github.com/NatLabRockies/EnergyPlus/pull/7761)
and the [NFP](https://github.com/energy-plus/EnergyPlus/blob/PerfOverrideAndReporting/design/FY2020/NFP-PerformanceOverRidesAndReporting.md)
