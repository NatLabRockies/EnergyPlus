Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.


### New Underground Gross Wall Area Added to Zone Summary Table in Input Verification and Results Summary Report

The Zone Summary table in the Input Verification and Results Summary Report always had a column labeled "Gross Wall Area" which was the above ground wall area for each zone. This change relabeled that column to be called "Above Ground Gross Wall Area" and added an entirely new column called "Underground Gross Wall Area" that shows the amount of underground wall area for each zone.

See [5588](https://github.com/NREL/EnergyPlus/pull/5588).


### Life-Cycle Cost Report Updated for Water

1. The "Energy Cost Cash Flows (Without Escalation)" table has been renamed "Energy and Water Cost Cash Flows (Without Escalation)".

2. The "Operating Cash Flow by Category (Without Escalation)" table used to combine energy and water costs in the Energy column and have zeros in the Water column. Now energy costs are in the Energy column and water costs are in the Water column.

3. The "Present Value for Recurring, Nonrecurring and Energy Costs (Before Tax)" table used to show water costs with the Energy in the Category column and Energy Cost in the Kind column but now show Water and Water Cost in those respective columns.

4. The Present Value by Category table used to include the sum of energy and water present values in the Energy row and zero in the Water row but now the present values of energy costs and water costs are in the appropriate row.

See [5594](https://github.com/NREL/EnergyPlus/pull/5594).

### Degree days in tabular reports correspond to ASHRAE Standard 90.1 degree day bases

Impacts the LEED Summary report table SEc1.1A-General Information and the Climatic Data Summary report. The #5083 issue was resolved and indicated that the temperature bases used for reporting the heating and cooling degree days were not consistent with ASHRAE Standard 90.1 which uses CDD50 and HDD65 (Fahrenheit). These correspond to CDD10 and HDD18 (Celsius). Previously these were reversed and using CDD65 and HDD50 instead. In addition made changes to use the terms "standard" to be "ASHRAE Handbook 2009" for the reporting of the degree day values in the Climatic Data Summary report.

See [5572](https://github.com/NREL/EnergyPlus/pull/5572).

### Economics, Tariff, and Life Cycle Cost Reports Explicitly Requested

The Economics Results Summary Report, Tariff Report, and Life-Cycle Cost Report now need to be specifically requested rather than being automatically generated if specific objects are present in the file. To get the reports individually use EconomicResultSummary, TariffReport, and LifeCycleCostReport respectively in the Output:Table:SummaryReports object. These are also available when using any of the AllSummary options. The reports are unchanged just they way of generating them has changed.

See [5598](https://github.com/NREL/EnergyPlus/pull/5598).


### JtoKWH Unit Conversion Impacts LEED Summary Report

When the JtoKHW unit conversion option is used in the OutputControl:Table:Style object it now changes the units used in the LEED Summary report. Columns that were previously left as GJ are now expressed as kWh and columns that were previously MJW/m2 are now expressed as kWh/m2.

See [5616](https://github.com/NREL/EnergyPlus/pull/5616).

### New Rows for District Heating and Cooling Virtual Rate in LEED EAp2-3 Table

When district heating or cooling is used, additional rows in the LEED EAp2-3 are shown with the virtual rates for those energy sources are shown.

See [5639](https://github.com/NREL/EnergyPlus/pull/5639).

### New column in Zone Summary table in IVRS report for Opening Area

A new columns for the window opening area (glass plus frame) has been added to the Zone Summary table under the Input Verification and Results Summary tabular report. The value shown in that column is consistent with the window opening area used in the Window-Wall Ratio tables in the same report and with the Exterior Fenestration table in the Envelope Summary tabular report.

See [5652](https://github.com/NREL/EnergyPlus/pull/5652).


### New smart defaulting of VAV reheat zone air terminal units

Changes to the single duct VAV air terminal unit's sizing routines (AirTerminal:SingleDuct:VAV:Reheat) also included revisions to the sizing summary data reported to the EIO and Component Sizing Summary tabular data.  Sizing for AirTerminal:SingleDuct:VAV:Reheat has been expanded to calculate and report a design size for the VAV minimum stop, or minimum flow fraction, as well as the user-specified constant minimum flow fraction.  New sizing reports to watch for include: Design Size Constant Minimum Air Flow Fraction, User-Specified Constant Minimum Air Flow Fraction, and Design Size Minimum Air Flow Rate [m3/s].

See [5646](https://github.com/NREL/EnergyPlus/pull/5646).
### Changes to Demand Controlled Ventilation table of the System Summary report

Four new columns were added in the middle of this table which summarizes the Controller:MechanicalVentilation inputs:
 - Outdoor Air Per Zone [m3/s]
 - Outdoor Air ACH [ach]
 - Outdoor Air Method (one of "Flow/Person", "Flow/Zone", "Flow/Area", "AirChanges/Hour", "Sum", "Maximum")
 - Outdoor Air Schedule Name (if no schedule is specified, then this column is blank)

The following changes were made to existing columns
 - Air Distribution Effectiveness in Cooling Mode and Air Distribution Effectiveness in Heating Mode are now blank if there is an Air Distribution Effectiveness Schedule Name specified (previously these columns would show 1.0)
 - Air Distribution Effectiveness Schedule heading changed to Air Distribution Effectiveness Schedule Name

See [5794](https://github.com/NREL/EnergyPlus/pull/5794).

### Daylighting output variables reference control object rather than zone object

The variables such as these (as well as ones with higher numbers) use the name of the Daylighting:Controls object instead of the name of the zone.

- Daylighting Reference Point 1 Illuminance
- Daylighting Reference Point 1 Glare Index []
- Daylighting Reference Point 1 Glare Index Setpoint Exceeded Time [hr]
- Daylighting Reference Point 1 Daylight Illuminance Setpoint Exceeded Time [hr]
- Daylighting Lighting Power Multiplier []

See [5725](https://github.com/NREL/EnergyPlus/pull/5725) and [5826](https://github.com/NREL/EnergyPlus/pull/5826).
