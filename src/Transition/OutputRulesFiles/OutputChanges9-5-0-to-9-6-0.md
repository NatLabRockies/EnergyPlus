Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### Surface Inside Face Convection Classification Index

For some configurations, this can report a "0", which represents and uninitialized condition. Internal refactoring has changed this invalid, uninitialized condition to "-1". This condition will be investigated and eliminated in the future.

### Surface Outside Face Convection Classification Index

For some configurations, this can report a "0", which represents and uninitialized condition. Internal refactoring has changed this invalid, uninitialized condition to "-1". This condition will be investigated and eliminated in the future.

### Daylight Map Report now supports more than two reference points
The header of the map file was changed from only allowing two reference points to allow more than two.  The additional points are listed in comma separated order where there used to be only two points.  The format will show up in the header as something like the following:

“ RefPt1=(2.50:2.00:0.80), RefPt2=(2.50:18.00:0.80), RefPt3=(2.50:18.00:0.50)”

[PR#8889](https://github.com/NatLabRockies/EnergyPlus/pull/8889) changed the output format in both the MAP file and the SQL output.

### Table Output Column Headings Changed for Central Plant:

 Modified Existing Column Headings:
 (a) "Nominal Capacity [W]" to "Reference Capacity [W]"
 (b) "Nominal Efficiency [W/W]" to "Reference Efficiency [W/W]"

 Added Two New Variables (Added Two New Column Headings):
 (a) "Rated Capacity [W]"
 (b) "Rated Efficiency [W/W]"

See [8192](https://github.com/NatLabRockies/EnergyPlus/pull/8959/)
See [PR#8889](https://github.com/NatLabRockies/EnergyPlus/pull/8889) changed the output format in both the MAP file and the SQL output.

### Table outputs for Space and Space Type

The following table outputs have changed:

**Lighting Summary**

*Interior Lighting*

  * The "Zone" column heading was changed to "Zone Name".
  * new columns were added for "Space Name" and "Space Type".
  * The "Zone Area" column heading was change to "Space Area".

The following table outputs are new:

**Annual Building Utility Performance Summary**

*End Uses By Space Type*

**Input Verification and Results Summary**

*Space Summary*

*Space Type Summary*


See [PR#8394](https://github.com/NatLabRockies/EnergyPlus/pull/8394)


### Table Output Changes in Standard 62.1 Summary Report
The System Ventilation Requirements for Cooling/Heating tables have two new columns: "Origin of D" and "Calculation Method for Ev". The former indicates how D has been calculated (user specified value, or using the occupancy design levels and schedules), and the latter indicates which method was used to determined that value.

The Zone Ventilation Calculations for Cooling/Heating Design tables have one new column: "Is Vpz-min calculated using the Standard 62.1 Simplified Procedure?". As its name indicates, it specifies if the zone minimum primary air flow was determined using the ASHRAE Standard 62.1 Simplified Procedure.

See PR [#8891](https://github.com/NatLabRockies/EnergyPlus/pull/8891).

### VentilationRateProcedure Name Change
The name of the `VentilationRateProcedure` method was changed to `Standard62.1VentilationRateProcedure` to be consistent with the Standard 62.1 Summary Report. The name of the method has been changed throughout EnergyPlus and is thus reflected in the EIO files.

See PR [#8891](https://github.com/NatLabRockies/EnergyPlus/pull/8891).

### Output:Table:Monthly and Output:Table:Annual Column Headers

For both objects `Output:Table:Monthly` and `Output:Table:Annual`

For aggregation types `Maximum`, `Minimum`, `MaximumDuringHoursShown` and `MinimumDuringHoursShown`, a space was missing between the aggregation type and the units
when writing the tabular report, which wasn't following the convention of other columns with different aggregation types,
and causing the SQL table `TabularDataWithStrings` to be missing a curly brace. See issue [#8921](https://github.com/NatLabRockies/EnergyPlus/issues/8921).

In the HTML file, the column headers will change:

```diff
-ELECTRICITY:FACILITY {Maximum}[W]
+ELECTRICITY:FACILITY {Maximum} [W]
 ELECTRICITY:FACILITY {TIMESTAMP}
```

And in the SQL file, `ColumnName` will as a result change as follows:

```diff
-ELECTRICITY:FACILITY {MAXIMUM
+ELECTRICITY:FACILITY {MAXMIUM}
 ELECTRICITY:FACILITY {TIMESTAMP}
```
