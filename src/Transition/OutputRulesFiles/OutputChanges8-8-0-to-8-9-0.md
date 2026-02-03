Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### Additional fields for Shadowing/Sun Position Calculations Annual Simulations

The existing Shadowing/Sun Position Calculations Annual Simulations report in the eio output has additional field at the end:
External Shading Calculation Method, Output External Shading Calculation Results, Disable Self-Shading Within Shading Zone Groups, Disable Self-Shading From Shading Zone Groups to Other Zones

See [6390](https://github.com/NatLabRockies/EnergyPlus/pull/6390)

### New System Sizing Outputs

A part of a system sizing refactor which includes bug fixes for Std 62.1 sizing, there are several new component sizing outputs that appear in every file with an airloop:
```
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Sum of Air Terminal Maximum Heating Flow Rates [m3/s], 0.38521
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Sum of Air Terminal Minimum Heating Flow Rates [m3/s], 0.38521
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Sum of Air Terminal Maximum Flow Rates [m3/s], 1.28404
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Adjusted Heating Design Air Flow Rate [m3/s], 0.49638
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Adjusted Cooling Design Air Flow Rate [m3/s], 1.28404
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Adjusted Main Design Air Flow Rate [m3/s], 1.28404
Component Sizing Information, AirLoopHVAC, VAV SYS 1, User Heating Air Flow Ratio [], 0.30000
Component Sizing Information, AirLoopHVAC, VAV SYS 1, Calculated Heating Air Flow Ratio [], 0.38657
```

See [6372](https://github.com/NatLabRockies/EnergyPlus/pull/6372)

### New Coil Sizing Reports

A new subtable "Coil Sizing Summary" has been added at the end of the "HVAC Sizing Summary" (HVACSizingSummary) report. This new subtable is always produced with this report.

A new "Coils Sizing Details" (CoilSizingDetails) report has been added. It is an optional report which is included with AllSummary*. The new report has over 80 columns of data. The "Coil Sizing Summary" subtable described above is a straight subset of this report with only about 30 columns.

See [6454](https://github.com/NatLabRockies/EnergyPlus/pull/6454)

### Remove blank fields in Output:Table:SummaryReports
If any field is blank, remove it and move remaining fields up one.

See [6919](https://github.com/NatLabRockies/EnergyPlus/issues/6919)
