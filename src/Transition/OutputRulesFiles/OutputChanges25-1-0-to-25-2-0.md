Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### System Summary table report, Demand Controlled Ventilation using Controller:MechanicalVentilation" Subtable
In the first column use ZoneName for zones with a simple DSOA reference (same as before), and use ZoneName:SpaceName for the spaces in a DSOA:SpaceList.

See Pull Request [#11051](https://github.com/NREL/EnergyPlus/pull/11051).

### Table Output, Equipment Summary Report, Air Heat Recovery subtable

* Delete "Name" column.

* Change "Input Object Type" heading to "Type".

* In the "Plate/Rotary" column, "FlatPlate" is now "Plate"

* Reorder, rename, and change units for last two columns:

    "Exhaust Airflow [kg/s]" --> "Exhaust Air Flow Rate [m3/s]

    "Outdoor Airflow [kg/s]" --> "Supply Air Flow Rate [m3/s]"
    
* Add more new columns:

  - Heat Recovery Active ("WhenFansOn", "Scheduled", "WhenOutsideEconomizerLimits", "WhenMinimumOutdoorAir")
  - Zone HVAC Name
  - Airloop Name
  - OA System Name
  - OA Controller Name

See Pull Request [#10995](https://github.com/NREL/EnergyPlus/pull/10995).
See Pull Request [#11138](https://github.com/NREL/EnergyPlus/pull/11138).

### Table Output, Equipment Summary Report, Air Terminals subtable
Add two new columns:
- PIU Heating Control Type
- PIU Fan Control Type

See Pull Request [#11138](https://github.com/NREL/EnergyPlus/pull/11138).

### Table Output, Equipment Summary Report, Fans subtable
Add two new columns:
- Speed Control Method
- Number of Speeds

See Pull Request [#11138](https://github.com/NREL/EnergyPlus/pull/11138).

### Table Output, System Summary Report, Fan Operation subtable
New subtable with the following columns:
- Occupied Time [hr]
- Occupied Continuous Fan [hr]
- Occupied Cycling Fan [hr]
- Occupied Fan Off [hr]
- Unoccupied Time [hr]
- Unoccupied Continuous Fan [hr]
- Unoccupied Cycling Fan [hr]
- Unoccupied Fan Off [hr]

See Pull Request [#11138](https://github.com/NREL/EnergyPlus/pull/11138).

### Component Sizing (eio and tables) for PlantLoop and CondenserLoop

* Change "Sizing option (Coincident/NonCoincident), 1.00000" to "Sizing Option, NonCoincident"

* Always report sizing values whether autosized or hard-sized.

See Pull Request [#10998](https://github.com/NREL/EnergyPlus/pull/10998).

### Table Output, Equipment Summary, PlantLoop or CondenserLoop subtable

* Always report sizing values whether autosized or hard-sized.

* Add columns for "Design Supply Temperature", "Design ReturnTemperature", and "Design Capacity".
  
See Pull Request [#10998](https://github.com/NREL/EnergyPlus/pull/10998).

### Table Output, Equipment Summary Report, Fan Power Fractions subtable
New table output showing fraction of full load fan power vs flow fraction.

See Pull Request [#11153](https://github.com/NREL/EnergyPlus/pull/11153).

### EIO and HTML Table Output: Initialization Summary

A number of changes related to finding duplicated HTML tables (based on FullName) have been made.

See Pull Request [#11106](https://github.com/NREL/EnergyPlus/pull/11106).

### Table Output, DX Heating Coils
* Add column Heating to Cooling Capacity Sizing Ratio

See Pull Request [#11130](https://github.com/NREL/EnergyPlus/pull/11130).

### Table Output, Heat Pump ACCA Manual S Report
* New Table added.
* Columns: Heat Pump Name, Heat Pump Type, Heat Pump Coil Type, Sizing Method, Total Load, Sensible Load, Total Capacity, Sensible Capacity, Total Capacity Sizing Factor, Sensible Capacity Sizing Factor, Latent Capacity Sizing Factor

See Pull Request [#11130](https://github.com/NREL/EnergyPlus/pull/11130).

#### Schedules

The `Output:Schedules` object controls whether the schedules are reported to the EIO, which would end up in the Initialization Summary report of the HTML if the `Output:Table:SummaryReports` asks for it.
The `Output:Schedules` object has a `Key Field` which can be either `Timestep` or `Hourly`, and it is **not** a unique object, meaning you can request **both**.

A few issues were occurring with the former EIO report (which is parsed to produce the HTML tables):

* The headers where not specific to a given `Key Field` (= Report Level), so if you requested both they would show in both reports in the HTML table
* The headers were NOT aligned with the data rows, so some tables were not present in the HTML (WeekSchedule, ScheduleTypeLimots=
* The `ScheduleTypeLimits` were issued in both `Timestep` and `Hourly` reports in the EIO

```diff
 ! <Output Reporting Tolerances>, Tolerance for Time Heating Setpoint Not Met, Tolerance for Zone Cooling Setpoint Not Met Time
  Output Reporting Tolerances, 0.200, 0.200,
+! <ScheduleTypeLimits>,Name,Limited? {Yes/No},Minimum,Maximum,Continuous? {Yes/No - Discrete}
+ScheduleTypeLimits,FRACTION,Yes,0.00,1.00,Yes
+ScheduleTypeLimits,ANY NUMBER,No,N/A,N/A,N/A
 ! Schedule Details Report=Timestep =====================
-! <ScheduleType>,Name,Limited? {Yes/No},Minimum,Maximum,Continuous? {Yes/No - Discrete}
-! <DaySchedule>,Name,ScheduleType,Interpolated {Yes/No},Time (HH:MM) =>,00:15,00:30,[...],23:30,23:45,24:00
+! <DaySchedule - Timestep>,Name,ScheduleType,Interpolated {Average/Linear/No},Time (HH:MM) =>,00:15,00:30,[...],23:30,23:45,24:00
-! <WeekSchedule>,Name,Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday,Holiday,SummerDesignDay,WinterDesignDay,CustomDay1,CustomDay2
+! <WeekSchedule - Timestep>,Name,Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday,Holiday,SummerDesignDay,WinterDesignDay,CustomDay1,CustomDay2
-! <Schedule>,Name,ScheduleType,{Until Date,WeekSchedule}** Repeated until Dec 31
+! <Schedule - Timestep>,Name,ScheduleType,{Until Date,WeekSchedule}** Repeated until Dec 31
-ScheduleTypeLimits,FRACTION,Average,0.00,1.00,Yes
-ScheduleTypeLimits,ANY NUMBER,No,N/A,N/A,N/A
-DaySchedule,TDEFAULTPROFILE,FRACTION,No,Values:,0.88,0.88,[...],0.82,0.82
+DaySchedule - Timestep,TDEFAULTPROFILE,FRACTION,No,Values:,0.88,0.88,[...],0.82,0.82
-Schedule:Week:Daily,CT WEEKSCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE
+WeekSchedule - Timestep,CT WEEKSCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE
-Schedule,ZONE CONTROL TYPE SCHEDULE,CONTROLTYPE,Through Dec 31,CT WEEKSCHEDULE
+Schedule - Timestep,ZONE CONTROL TYPE SCHEDULE,CONTROLTYPE,Through Dec 31,CT WEEKSCHEDULE
! Schedule Details Report=Hourly =====================
-! <ScheduleType>,Name,Limited? {Yes/No},Minimum,Maximum,Continuous? {Yes/No - Discrete}
-! <DaySchedule>,Name,ScheduleType,Interpolated {Yes/No},Time (HH:MM) =>,01:00,02:00,[...],23:00,24:00
+! <DaySchedule - Hourly>,Name,ScheduleType,Interpolated {Average/Linear/No},Time (HH:MM) =>,01:00,02:00,[...],23:00,24:00
-! <WeekSchedule>,Name,Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday,Holiday,SummerDesignDay,WinterDesignDay,CustomDay1,CustomDay2
+! <WeekSchedule - Hourly>,Name,Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday,Holiday,SummerDesignDay,WinterDesignDay,CustomDay1,CustomDay2
-! <Schedule>,Name,ScheduleType,{Until Date,WeekSchedule}** Repeated until Dec 31
+! <Schedule - Hourly>,Name,ScheduleType,{Until Date,WeekSchedule}** Repeated until Dec 31
-ScheduleTypeLimits,FRACTION,Average,0.00,1.00,Yes
-ScheduleTypeLimits,ANY NUMBER,No,N/A,N/A,N/A
-DaySchedule,TDEFAULTPROFILE,FRACTION,No,Values:,0.88,0.92,[...],0.75,0.82
+DaySchedule - Hourly,TDEFAULTPROFILE,FRACTION,No,Values:,0.88,0.92,[...],0.75,0.82
-Schedule:Week:Daily,CT WEEKSCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE
+WeekSchedule - Hourly,CT WEEKSCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE,CT DAY SCHEDULE
-Schedule,ZONE CONTROL TYPE SCHEDULE,CONTROLTYPE,Through Dec 31,CT WEEKSCHEDULE
+Schedule - Hourly,ZONE CONTROL TYPE SCHEDULE,CONTROLTYPE,Through Dec 31,CT WEEKSCHEDULE
```

#### Warmup Convergence Information

When `Output:Diagnostics` has the `ReportDetailedWarmupConvergence` enabled, both the regular report and the detailed one were named `Warmup Convergence Information` resulting in duplicated HTML tables that would include lines from the other report.

The `ReportDetailedWarmupConvergence` report (the one that shows the status at each timestep/hour until convergence is reached) was renamed to `Warmup Convergence Information - Detailed`.

#### Material:Air

The `Output:Constructions` has two possible keys: `Materials` and `Constructions`. Both were adding a table named `Material:Air`. The Constructions one was renamed to `Material:Air CTF Summary` to match the surrounding tables.

#### Fuel Supply

When using `Generator:FuelSupply`, the header was written twice in the EIO Initialization Summary as `! <Fuel Supply>,...` leading to two identical tables in the HTML report.

#
### ZoneHVAC:IdealLoadsAirSystem
 
* Added eight new report variables for ZoneHVAC:IdealLoadsAirSystem object:

```
   (1) * Zone Ideal Loads Zone Heating Fuel Energy Rate [W] *
   (2) * Zone Ideal Loads Zone Cooling Fuel Energy Rate [W]            
   (3) * Zone Ideal Loads Zone Heating Fuel Energy [J]
   (4) * Zone Ideal Loads Zone Cooling Fuel Energy [J]
   (5) * Zone Ideal Loads Supply Air Total Heating Fuel Energy Rate [W] *
   (6) * Zone Ideal Loads Supply Air Total Cooling Fuel Energy Rate [W] *
   (7) * Zone Ideal Loads Supply Air Total Heating Fuel Energy [J] *
   (8) * Zone Ideal Loads Supply Air Total Cooling Fuel Energy [J] *
```

See pull request [#10971] https://github.com/NREL/EnergyPlus/pull/10971




### EIO and HTML Table Output - Schedule-Hourly/Schedule-Timestep

Change header to the EIO table to make more compatible with the parsing that the Initialization Summary so that columns for more
that the first Until Date/WeekSchedule are shown. Made explicit for the first 9 pairs of Until's. The use of 9 pairs of columns is preferred to a report that has a flexible number of columns.

Previous
```
! <Schedule - Hourly>,Name,ScheduleType,{Until Date,WeekSchedule}** Repeated until Dec 31
``` 

Changed
```
! <Schedule - Hourly>,Name,ScheduleType,Until Date 1,WeekSchedule 1,Until Date 2,WeekSchedule 2,Until Date 3,WeekSchedule 3,Until Date 4,WeekSchedule 4,Until Date 5,WeekSchedule 5,Until Date 6,WeekSchedule 6,Until Date 7,WeekSchedule 7,Until Date 8,WeekSchedule 8,Until Date 9,WeekSchedule 9,
```

#### EIO and HTML Table Output - Airflow Stats Nominal

The EIO tables related to Airflow Stats Nominal and the Initialization Summary which include:

- ZoneInfiltration Airflow Stats Nominal
- ZoneVentilation Airflow Stats Nominal
- Mixing Airflow Stats Nominal
- CrossMixing Airflow Stats Nominal

Have been updated to add the input object name. This was done to identify the type of algorithm used.

#### EquipmentSummary Service Water Heating - Additional Columns

Several new columns were added to the Service Water Heating table:

- Fuel Type
- Set Point Schedule Name for Heater 1
- Set Point at 11am First Wednesday for Heater 1
- Days with Same 11am Value for Heater 1
- Month Assumed for Heater 1
- Set Point Schedule Name for Heater 2
- Set Point at 11am First Wednesday for Heater 2
- Days with Same 11am Value for Heater 2
- Month Assumed for Heater 2
- Peak Use Water Flow Rate
- Use Flow Rate Fraction Schedule Name
- Ambient Temperature Zone Name

#### SystemSummary Economizer - Additional Column

A new column was added to the System Summary Economizer table:

- AirLoopHVAC:OutdoorAirSystem Name

#### Control Summary - SetpointManager:OutdoorAirReset

This is entirely new table

- Setpoint Nodes
- Setpoint Node PlantLoop Name
- Control Type
- Setpoint at Outdoor Low Temperature
- Setpoint at Outdoor High Temperature
- Outdoor Low Temperature
- Outdoor High Temperature
- Schedule Name
- Setpoint at Outdoor Low Temperature 2
- Setpoint at Outdoor High Temperature 2
- Outdoor Low Temperature 2
- Outdoor High Temperature 2

#### Control Summary - SetpointManager:ReturnTemperature

This is entirely new table

- Type
- Plant Loop Supply Outlet Node
- Plant Loop Supply Inlet Node
- PlantLoop Name
- Minimum Supply Temperature Setpoint [C]
- Maximum Supply Temperature Setpoint [C]
- Return Temperature Setpoint Input Type
- Return Temperature Setpoint [C]
- Return Temperature Setpoint Schedule Name

#### Control Summary - AvailabilityManager:Scheduled

This is entirely new table

- AirLoop Name
- AvailabilityManager Name
- Type
- Schedule Name

#### Control Summary -  PlantEquipmentOperation Load Based

This is entirely new table

- Plant Loop Name
- Name
- Type
- Schedule Name
- Index
- Lower Limit [W]
- Upper Limit [W]
- Equipment List Name
- Equipment
