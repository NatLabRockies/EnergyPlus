Add Checksums Tabular Output Reports
================

**Jason Glazer, GARD Analytics**

 - October 31, 2016
 - November 10, 2016 - Revision 1


## Justification for New Feature ##

Output reporting in EnergyPlus is extremely detail-oriented, which is one of the strengths of the simulation engine. However, in some cases it would be useful to have in EnergyPlus a way to present to the user a summary of some of the key outputs, for a quick analysis of results. Users have specifically suggested that EnergyPlus should have some tabular output reports similar to the Checksums reports that Trane Trace 700 generates.


## E-mail and  Conference Call Conclusions ##

Comments received are summarized as:

- Add difference between actual and design peak load
- Add total heating and cooling capacity per area report in the engineering checks section
- Add total condenser-water-side pump power per flow rate for facility level engineering checks section
- Add total chilled-water-side pump power per flow rate for facility level engineering checks section
- Add table of zones for AirLoop report
- In general, add more checks to help detect errors

## Overview ##

The Checksum reports consists of several parts including a breakdown of cooling and heating peak load, coil selection, areas, temperatures, airflows and engineering checks. The breakdown of cooling and heating peak load are very similar to the existing Zone Load Component Summary table that appears in the tabular reports. Due to this, the Zone Load Component Summary report will serve as the basis of the reporting enhancements.

The Checksum reports appear at the room, the zone, and the system level. The "room" has no direct correspondence in EnergyPlus but the system is essentially the AirLoop level. In addition, a facility  level summary would be added. The term "facility" is used to be consistent with the use with output meters to include all zones and HVAC.

The reports that will be included will be:

- Zone Load Component Summary
- Airloop Load Component Summary
- Facility Load Component Summary

The current Zone Load Component Summary will be enhanced with new subtables which will also appear in the Airloop Load Component Summary and Facility Load Component Summary reports. For the AirLoop Load Component Summary and the Facilty Load Component summary, they will show the values for the peak load and components at the time and sizing period of the maximum for the AirLoop and Facilty.

The enhancements for the tabular reports will be:

- The main "Estimated Peak Load Components" subtables will have two new columns for areas and total load per area. These will show values for the envelope related rows of the table and for the internal loads (which will use the floor area). New subtotal rows will be added to the subtable for internal loads, envelope, air, and other.

- The "Peak Conditions" subtable will have additional temperatures such as terminal supply temperature, the supply air temperature after the fan, and the mixed air temperature. The "Peak Conditions" subtable will have airflows reported at the terminal, main fan, outside air, infiltration, and exhaust. The heating and cooling "Peak Conditions" subtable will be combined.

 - A new "Engineering Checks" subtable that will show the percent outside air, airflow per floor area, airflow per cooling capacity, area per cooling capcity and number of people.

While these new tables and enhancements to the tables are inspired by the Checksum reports, they will be focused on outputs that are meaningful in EnergyPlus and easily gathered from existing data structures.

All of these reporting enhancements will appear also in the SQL files since these support the tabular reports.

## Approach ##

The peak time for each airloop and the facilty will be used to collect additional data points using the same methodology as the Zone Component Load Summary report described in the Engineering Reference using the delayed components computed using the same "decay curve" approach as used in the Zone Load Component Summary report.


## Testing/Validation/Data Sources ##

Individual values being reported for the first time will be compared to values shown in the data structures.

## Documentation ##

The existing InputOutputReference documentation on Output:Table:SummaryReports will be modified to describe the new reports and new AirLoopLoadComponentSummary and FacilityLoadComponentSummary options will be added.

The OutputDetailsAndExamples documentation will include new sections on these reports.

The EngineeringReference documentation will be edited to reference all three of the Load Component Summary reports.


## Outputs Description ##

The subtables would be updated as shown below (similar changes to heating tables would be made).

Report: **Zone Component Load Summary**

For: **SPACE1-1**

Timestamp: **2016-10-28 15:58:05**

**Estimated Cooling Peak Load Components**

| |Sensible - Instant [W] | Sensible - Delayed [W] | Sensible - Return Air [W] | Latent [W] | Total [W] | %Grand Total | Related Area [m2] | Total per Area [W/m2] |
| --- | ---:| ---:| ---:| ---:| ---:|---:|---:|---:|
| People | 536.56 | 258.31 | |523.12 | 1317.99 | 35.33 | 99.16 | 13.3 |
| Lights | 15.84 | 1335.72 | 316.80 | |1668.36 | 44.72 | 99.16 | 16.8 |
| Equipment | 535.92 | 316.37 | | 0.00 | 852.29 | 22.84 | 99.16 | 8.59 |
| Refrigeration | 0.00 | |0.00 | 0.00 | 0.00 | 0.00 |  99.16 | 0.00|
| Water Use Equipment | 0.00 | | |0.00 | 0.00 | 0.00 |  99.16 | 0.00|
| HVAC Equipment Losses | 0.00 | 0.00 | | | 0.00 | 0.00 |
| Power Generation Equipment | 0.00 | 0.00 | | | 0.00 | 0.00 |
| DOAS Direct to Zone | 0.00 | | | 0.00 | 0.00 | 0.00 |
| Infiltration | 0.00 | | | 0.00 | 0.00 | 0.00 |
| Zone Ventilation | 0.00 | | |0.00 | 0.00 | 0.00 |
| Interzone Mixing | 0.00 | | | 0.00 | 0.00 | 0.00 |
| Roof | |0.00 | | | 0.00 | 0.00 | 0.00 | 0.00|
| Interzone Ceiling | | 467.30 | | | 467.30 | 12.53 | 99.16 | 4.7|
| Other Roof | | 0.00 | | | 0.00 | 0.00 | 0.00 |  0.00|
| Exterior Wall | | 374.68 | | | 374.68 | 10.04 | 73.20 | 5.1 |
| Interzone Wall | | -645.5 | | | -645.5 | -17.3 | 130.00 | -4.9 |
| Ground Contact Wall | | 0.00 | | | 0.00 | 0.00 | 0.00| 0.00|
| Other Wall | | 0.00 | | | 0.00 | 0.00 |0.00| 0.00|
| Exterior Floor | | 0.00 | | | 0.00 | 0.00 |0.00| 0.00|
| Interzone Floor | | 0.00 | | | 0.00 | 0.00 |0.00| 0.00|
| Ground Contact Floor | | -2177.1 | | | -2177.1 | -58.4 | 99.16| -21.9 |
| Other Floor | | 0.00 | | | 0.00 | 0.00 |0.00| 0.00 |
| Fenestration Conduction | 782.86 | | | | 782.86 | 20.98 | 21.81 | 35.8|
| Fenestration Solar | | 1089.93 | | | 1089.93 | 29.21 | 21.81 | 49.9 |
| Opaque Door | | 0.00 | | | 0.00 | 0.00 | 0.00 | 0.00|
| Subtotal Internal  | 1088.3 | 1910.4 | 316.8 | 523.12 |3838.6 | 102.89 |99.16 |38.7
| Subtotal Envelope | 782.8  |  -890.69|  0.0 | 0.0  | -107.8 | -2.89 | 422.5 | -0.25
| Subtotal Air |  0.0 | 0.0 |0.0 | 0.0 |0.0 |0.00 |
| Subtotal Other |   0.0 | 0.0 |0.0 | 0.0 |0.0 |0.00 |
| Grand Total | 1871.18 | 1019.70 | 316.80 | 523.12 | 3730.80 | 100.00

**Peak Conditions**

| | Cooling | Heating |
| --- | ---: | ---: |
| Time of Peak Load | 7/21 15:45:00 | 1/21 24:00:00 |
| Outside Dry Bulb Temperature [C] | 31.02 | -17.3 |
| Outside Wet Bulb Temperature [C] | 23.14 | -17.3 |
| Outside Humidity Ratio at Peak [kgWater/kgAir] | 0.01459 | 0.0084 |
| Zone Dry Bulb Temperature [C] | 23.90 | 22.20 |
| Zone Relative Humdity [%] | 52.91 | 22.78 |
| Zone Humidity Ratio at Peak [kgWater/kgAir] | 0.00979 | 0.00377 |
| Terminal supply temperature [C] | 10.0 | 30.0 |
| Supply air temperature [C] | 11.0 | 29.0 |
| Mixed air temperature [C] | 17.0 | 17.0 |
| Terminal air flow [m3/s]| 1.2| 1.2 |
| main fan air flow [m3/s]| 1.2 | 1.2 |
| outside air flow [m3/s]| 0.3 | 0.1 |
| infiltration air flow [m3/s]| 0.15| 0.15 |
| exhaust air flow [m3/s]| 0.10| 0.10 |
| Design peak load [W] | 2600.00 | -1600.00 |
| Difference between Design and Peak Load [W] | 47.21 | -24.10 |
| Peak Design Sensible Load [W] | 2647.21 | -1624.1 |
| Estimated Instant + Delayed Sensible Load [W] | 2890.88 | -1619.7|
| Difference beween Peak and Estimated Sensible Load [W] | -243.7 | -5.5 |

**Engineering Checks**

| | Cooling | Heating |
| --- | ---: | ---: |
| Outside Air (%)| 14.0 | 10.1 |
| Airflow per floor area [m3/s-m2] | 1.1 | 1.1 |
| Airflow per total capacity [m3/2-W] | 1.1 | 1.1 |
| Area per total capacity [m2/W]  | 1.1 | 1.1 |
| Total capacity per area [W/m2]  | 0.9 | 0.9 |
| Chiller pump power per flow [W/m3] | 0.004 | 0.003 |
| Condenser pump power per flow [W/m3] | 0.003 | 0.002 |
| Number of people  | 4 | 4 |

**Zones Included**

|     | Name  |
| --- | ---   |
|   1 | Zone A |
|   2 | Zone B |
|   3 | Zone C |
|   4 | Zone D |
|   5 | Zone E |

The Engineering Checks table would not include pump related lines for the zone or airloop level reports. Additional lines in the engineering checks table may be added if time permits. The Zones Included table would only appear for the AirLoop level report.


## Example File and Transition Changes ##

The new reports will be added to several example files.





