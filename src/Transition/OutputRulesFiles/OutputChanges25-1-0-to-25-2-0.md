Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### Table Output, Equipment Summary Report, Air Heat Recovery subtable
* Delete "Name" column.

* Change "Input Object Type" heading to "Type".

* In the "Plate/Rotary" column, "FlatPlate" is now "Plate"

* Reorder, rename, and change units for last two columns:

    "Exhaust Airflow [kg/s]" --> "Exhaust Air Flow Rate [m3/s]

    "Outdoor Airflow [kg/s]" --> "Supply Air Flow Rate [m3/s]"

See Pull Request [#10995](https://github.com/NREL/EnergyPlus/pull/10995).

### EIO and HTML Table Output: Initialization Summary

A number of changes related to finding duplicated HTML tables (based on FullName) have been made.

See Pull Request [#11106](https://github.com/NREL/EnergyPlus/pull/11106).

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
