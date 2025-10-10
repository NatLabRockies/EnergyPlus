Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited.  It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo.  At least a PR number or whatever.

### Timestamp tag added to tabular headers

With Pull Request [#5295](https://github.com/NREL/EnergyPlus/pull/5295), some tabular report headers were changed to include the tag ```{TIMESTAMP}``` to make them more consistent with the use of the that tag with monthly and annual reports when the minimum or maximum was shown. This impacts the HVAC Sizing Summary tables, the Plant Loop Coincident Design Fluid Flow Rate Adjustments table,  the Energy Meters tables, and the Sensible Heat Gain Summary tables.

### TimeBins value rounding

The general algorithm for choosing when an output variable is in various time bins was altered slightly with this change. Now rounding occurs to the same number of significant digits as shown in the headings of the TimeBin reports. For example, the value of 4.997 would have been put in the 4.00 to 5.00 to  bin previously since less than 5 but now would be put in the 5.00 to 5.99 bin since the rounded value would be 5.00.  This stabilizes the total hours in particular bins when comparing cases that vary slightly. See Pull Request [#5295](https://github.com/NREL/EnergyPlus/pull/5295) for the actual code change.

### Add units and change output variable name for Availability Manager Optimum Start

The output variable which was previously "Availability Manager Optimum Start Hours Before Occupancy []" has been changed to be more consistent with other length of time output variable names, and units have been added:
"Availability Manager Optimum Start Time Before Occupancy [hr]".

### Plant Pump Models

The plant pump models have had some wording changes.  Use of the word *Rated* has been changed to *Design*. The reason is that these values are not really ratings in the sense that they correspond to the results of a standard rating system or a specific rating point that might appear in product catalog data.  Rather they are just the design values used in the pump model calculations.  This affects not only the input fields in the pump objects, but also the strings reported for sizing outcomes.

For version 8.5, the strings used to report pump sizes have changed In the EIO file and table summary reports in the following ways:

    Rated Flow Rate [m3/s] => Design Flow Rate [m3/s]
    Initial Rated Flow Rate [m3/s] => Initial Design Flow Rate [m3/s]
    Rated Power Consumption [W] => Design Power Consumption [W]
    Initial Rated Power Consumption [W] => Initial Design Power Consumption [W]

### Net Area added to Envelope Summary tabular report

Added new "Net Area" column to the Envelope Summary report Opaque Exterior table that represents the gross wall area minus the window and door areas.

### Electric Load Center Changes

The metering of photovoltaic power production has been changed.  Previously it was assumed that all AC power produced by an inverter was from photovoltaics because that was the only DC generator.  However new features for charging DC storage from grid supplied power make this assumption untenable.  Photovoltaic generators now meter their DC energy production directly (on Photovoltaic:ElectrictyProduced) and the inverter losses handled separately.

There is a new sub end use meter type called PowerConversion which ends up on "PowerConversion:ElectricityProduced" and the inverter (and new AC to DC Converter) meter the electric power losses from power conversion on this meter (as negative values).

The summary table called Electric Loads Satisfied has been revised to include a new row called Power Conversion. The precision of reported values has been increased. This is the sum of power losses from converting between AC and DC or between different voltage AC.  The inverter, converter, and some applications of transformers now meter their losses on PowerConversion:ElectricityProduced and that is what fills this new row.  The old values for "Photovoltaic Power" should match the sum of the new values for "Photovoltaic Power" and "Power Conversion."  This row includes transformer power conversion losses when the transformer is used to export power back to the grid.

### Outdoor Air Summary tabular report bugfix ###
Fixed the number of occupants in the Outdoor Air Summary Report for multiplied zones. The values are now consistent with the table footnote: *Values shown for a single zone without multipliers*. [#5216](https://github.com/NREL/EnergyPlus/pull/5216)

### Standard 62.1 Summary System Ventilation Requirements tabular report ###
This report is now populated for Sizing:System System Outdoor Air Method = ZoneSum and VentilationRateProcedure. Previously this report was generated with no data (empty cells) for OA method=ZoneSum. [#5216](https://github.com/NREL/EnergyPlus/pull/5216)