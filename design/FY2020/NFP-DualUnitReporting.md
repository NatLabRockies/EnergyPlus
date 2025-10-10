Allow Different Units in SQL and HTML Tabular Reports
================

**Jason Glazer, GARD Analytics**

 - May 12, 2020
 - May 29, 2020 - added the IOref text and Design Document portion


## Justification for New Feature ##

Some users want the EnergyPlus tabular reports in IP units but user interfaces for EnergyPlus may want
the SQL in SI units for the same simulation. OpenStudio for example, does post processing and depends
on the tabular reports in the SQL file being in SI units. The mixture of both units in output has been
requested by user interface developers.

See:

https://github.com/NREL/EnergyPlus/issues/5435

and

https://github.com/NREL/OpenStudio/issues/2039


## E-mail and  Conference Call Conclusions ##

No changes were based on the feedback from the reviewers of the intial NFP.

## Overview ##

Continue to allow EnergyPlus HTML tabular reports in various SI and IP units while adding the ability to keep
the SQL output file in native SI units.

## Approach ##

Many different input objects control the production of tabular reports in EnergyPlus including:

- OutputControl:Table:Style
- Output:JSON
- Output:SQLite
- OutputControl:IlluminanceMap:Style

And additional input objects control the production of timeseries data. A current effort is underway
to directly provide timeseries data in CSV format instead of ESO/MTR format:

https://github.com/NREL/EnergyPlus/pull/7904

While some refactoring of the input objects that control reporting have been considered and perhaps
should be considered in the future, no specific plan is recommended at this point.
In order to satisfy the original need identified of producing SQL files for tabular reports in SI
units even if the HTML file is in IP units, a small change to the input structure is required.

A second field will be added to the Output:SQLite object that will control unit conversion for the
SQL file.

```
Output:SQLite,
       \memo Output from EnergyPlus can be written to an SQLite format file.
       \unique-object
       \min-fields 1
  A1 , \field Option Type
       \type choice
       \key Simple
       \key SimpleAndTabular
  A2;  \field Tabular Unit Conversion
       \type choice
       \key UseOutputControlTableStyle
       \key None
       \key JtoKWH
       \key JtoMJ
       \key JtoGJ
       \key InchPound
       \default UseOutputControlTableStyle

```

Note: min-fields 1 needs to be added to the idd.

The default of UseOutputControlTableStyle will let the tables in the SQL file follow the unit conversion
specified in the OutputControl:Table:Style just as it currently does but the other options would allow different
unit conversions or no unit conversions at all to apply to the tables in the SQL file. The timeseries data
in the SQL file does not have, and will not have, unit conversions performed.


## Testing/Validation/Data Sources ##

Will test that the unit conversion in the SQL file for tables is correct.

## Input Output Reference Documentation ##

Add the following description of the proposed field to the Output:SQLite input object.

```
Field: Tabular Unit Conversion

When the Option Type field is set to SimpleAndTabular so that the tabular reports are included in the SQL file,
this field is active. The Tabular Unit Conversion field determines if any unit conversions will be performed for
the values of the tabular reports that are included in the SQL file. This field only affects the unit conversions
for the tabular reports in the SQL file and does not convert the units of the time series data in the SQL file. It also
has no impact on the unit conversions for the tabular reports in the HTML or other output formats.

This field is optional and, if not specified, defaults to UseOutputControlTableStyle which is using the same unit
conversion that appears in the OutputControl:Table:Style object. Five different input options
are available:

- UseOutputControlTableStyle - use the same unit conversion as in OutputControl:Table:Style
- None – no conversions performed
- JtoKWH – Joules converted into kWh (1 / 3,600,000)
- JtoMJ – Joules converted into Megajoules (1 / 1,000,000)
- JtoGJ – Joules converted into Gigajoules (1 / 1,000,000,000)
- InchPound – convert all annual, monthly, economics and timebins tabular values to common InchPound
equivalent

When no unit conversions is selected the energy is reported in the form of Joules but since the magnitude of those numbers
for many buildings is very large, these other conversion factors are available.

The JtoKWH, JtoMJ and JtoGJ unit conversion input option applies only to the Output:Table:Monthly
reports and partially to the ABUPS report. For ABUPS, the JtoKWH option changes the report but the
JtoMJ and JtoGJ options do not change the report since they it is already in MJ/m2 and GJ. In addition,
the JtoKWH option also changes results in the LEED Summary report changing GJ to kWh and MJW/m2
to kWh/m2. The InchPound unit conversion input option applies to all annual, monthly, timebins and
economic reports that appear in the SQL file.

An example IDF input object follows.

Output:SQLite,
   SimpleAndTabular,    !- Option Type
   InchPound;           !- Unit Conversion

```

This text was modified from the existing text in the IOref for the Unit Conversion field of the
OutputControl:Table:Style input object.

## Engineering Reference ##

No changes

## Example File and Transition Changes ##

No transition changes expected. An example file will be modified to include this option.

## References ##

None

## Design Document ##

Currently, the structure to write the tabular reports to the SQL file is located within each function that writes different
groups of tabular reports. The list of functions called is in OutputReportTabular::WriteTabular():

- WriteBEPSTable()
- WriteTableOfContents();
- WriteVeriSumTable(outputFiles);
- WriteDemandEndUseSummary();
- WriteSourceEnergyEndUseSummary();
- WriteComponentSizing();
- WriteSurfaceShadowing();
- WriteCompCostTable();
- WriteAdaptiveComfortTable();
- WriteEioTables(OutputFiles::getSingleton());
- WriteLoadComponentSummaryTables();
- WriteHeatEmissionTable();
- coilSelectionReportObj->finishCoilSummaryReportTable(state);
- WritePredefinedTables();
- WriteMonthlyTables();
- WriteTimeBinTables();
- OutputReportTabularAnnual::WriteAnnualTables();

For each of these functions, unit conversions are performed as needed on numbers and then converted into arrays of strings
representing the row and column headings as well as the body of the subtable.  These arrays are then passed to routines that
either write the individual tabular report to the HTML (or TXT or other formats) by calling:

- OutputReportTabular::WriteTable()

and then the same string arrays are also passed to

- SQLProcedures::CreateSQLiteDatabase()

and for producing JSON and related formats for the tabular reports they are also passed to

- ResultsFramework::OutputSchema-\>TabularReportsCollection.addReportTable()

Given this structure, to implement the features we will update:

- OutputReportsTabular::WriteTabularReports()

that calls all the specific reporting functions to loop through the list of calls twice. When the unit conversions match
(both the tabular HTML and tabular SQL are in IP units for example) it would go through the loop once as it does now.
If the unit conversions did not match, the loop would allow the reports to be produced twice, passing the correct unit
conversion with the creation of the appropriate file type. A parameter to be passed (the unit conversion needed) and
appropriate flags for which files need to be generated. Each indivdual routine would then use the passed flags to determine
which types of outputs should be produced for a given iteration using specific unit conversions.

Also, the SQLProcedures::CreateSQLiteDatabase() function will be modified to add the reading the additional field
"Unit Conversion."

In addition, appropriate unit tests will be added.
