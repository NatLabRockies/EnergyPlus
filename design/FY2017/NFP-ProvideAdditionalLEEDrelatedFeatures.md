Provide Additional LEED Related Features
================

**Jason Glazer, GARD Analytics**

 - July 21, 2017


## Justification for New Feature ##

The current Leadership in Energy and Environmental Design (LEED) Summary Report available in EnergyPlus helps users working with LEED forms. However, the report functionality has not been updated since it was first implemented in 2006. An update of the Summary Report functionality would help LEED users and Green Building Council Certification (GBCI) reviewers. EnergyPlus will be enhanced to provide additional LEED related outputs and objects with the end use subcategory field that will allow an OpenStudio measure to gather the outputs and report them in a form to help users complete the LEED forms.

## E-mail and  Conference Call Conclusions ##

Based on the feedback from the [NFP to Update the LEED Summary Report](https://github.com/NatLabRockies/EnergyPlus/blob/Update_LEED_Reporting/design/FY2017/NFP-UpdateLEEDSummaryReport.md) we have decided to limit the changes to EnergyPlus to those that need to be in the calculation engine and use OpenStudio measure to create the output report. This NFP has a narrower scope and the original NFP file was left in place for reference.

Other feedback received was not have an explicit list of end-uses but just continue to allow any string to differentiate them.

Add total operating hours to schedule, these would be hours where the schedule had a value greater than zero.

Add CoolingTower:VariableSpeed to list of objects.

## Overview ##

The scope of work for this NFP is to enhance the functionality of reporting for LEED:

 - To provide more information as required by LEED submittals.
 - With details as required by the U.S. Green Building Council (USGBC), including the information needed for the Energy Performance Calculator and Table 1.4 spreadsheets.
 - The updates will be focused on just the results that are not otherwise available in the EnergyPlus tabular reports.

The expectation is that the results of this task will be used by an OpenStudio Measure for LEED Reporting being developed separately.


## Approach ##

The NFP was based on reviewing the following file which is submitted when using LEEDv4 to:

- v4_Minimum Energy Performance Calculator_v04.xlsm

as well as reaching out to stakeholders. More details are available at  [NFP to Update the LEED Summary Report](https://github.com/NatLabRockies/EnergyPlus/blob/Update_LEED_Reporting/design/FY2017/NFP-UpdateLEEDSummaryReport.md).

Several issues related to the LEED reporting may be separately addressed, including:

- https://github.com/NatLabRockies/EnergyPlus/issues/5672 EAp2-4/5. Demand data taken from Design Days instead of from Annual Peak
- https://github.com/NatLabRockies/EnergyPlus/issues/5559 Add LEED reporting of peak end use and EFLH
- https://github.com/NatLabRockies/EnergyPlus/issues/5895 LEED Summary EAp2-4/5 Report Problems
- https://github.com/NatLabRockies/EnergyPlus/issues/5613 LEED Tables service water heating doesn't appear
- https://github.com/NatLabRockies/EnergyPlus/issues/5956 Report VAV minimum volume setpoint
- https://github.com/NatLabRockies/EnergyPlus/issues/5298 Outdoor Air Summary report improvements


## Testing/Validation/Data Sources ##

Compare outputs manually.

## Input Output Reference Documentation ##

Update the IORef to correspond with the IDD changes shown in the next section.

Also need to update the Using EnergyPlus for Compliance documentation.

## Input Description ##

Many objects such as Lights and ElectricEquipment have an End-Use Subcategory field. This same field will be added to the following objects:

 - Boiler:HotWater
 - Boiler:Steam
 - Chiller:Electric:EIR
 - Chiller:Electric:ReformulatedEIR
 - Chiller:Electric
 - CoolingTower:SingleSpeed
 - CoolingTower:TwoSpeed
 - CoolingTower:VariableSpeed
 - CoolingTower:VariableSpeed:Merkel
 - WaterHeater:Mixed

Using the following (or similar) field:

```
  A5 , \field End-Use Subcategory
       \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
       \type alpha
       \retaincase
       \default General
```

Some strings used in the end-use subcategory fields have special meaning to categorize them on in the EAp2-4/5 Performance Rating Method Compliance table but this special categorization will be removed and replaced with all end use subcategories provided by the user being shown on the EAp2-4/5 table.

All new End-Use Subcategory fields will be added to the end of the objects to minimize transitioning issues.


## Outputs Description ##

The following changes will be made to EnergyPlus output to provide input to the OpenStudio measure

- Add table to show each schedule EFLH. The table would show the first object referencing that schedule (if simple to do). Also add hours with values greater than zero.

- Add table to show the setpoint schedule values at 11am and 11pm and the first object referencing that schedule.

- Change the energy and demand (EA2-4/5 table) so that it reflects each end use subcategory. The table will no longer use special end-use category strings to aggregate values in the table. Demands shown on the table would be the peak demand for the end-use or end-use subcategory. When an end-use subcategory is used, all energy not in that subcategory would be shown as the end-use and the word "other".

EnergyPlus output reporting would enhanced only to add new outputs that are not already gatherable and are computationally expensive to produce.

<b>Schedules-EFLH (Schedule Type=Fraction)</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Equivalent Full Load Hours of Operation Per Year</td>
    <td align="right">Hours Greater than Zero Per Year</td>
  </tr>
  <tr>
    <td align="right">LIGHTS-1</td>
    <td align="right">3066</td>
    <td align="right">5780</td>
  </tr>
  <tr>
    <td align="right">EQUIP-1</td>
    <td align="right">3430</td>
    <td align="right">5780</td>
  </tr>
  <tr>
    <td align="right">Always On Sch</td>
    <td align="right">8760</td>
    <td align="right">8760</td>
  </tr>
  <tr>
    <td align="right">OCCUPY-1</td>
    <td align="right">2758</td>
    <td align="right">4703</td>
  </tr>
</table>

<b>Schedules-SetPoints (Schedule Type=Temperature)</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">First Object Used</td>
    <td align="right">11am Monday [C]</td>
    <td align="right">11pm Monday [C]</td>
  </tr>
  <tr>
    <td align="right">Htg-SetP-Sch</td>
    <td align="right">HeatingSetpoint</td>
    <td align="right">21.1</td>
    <td align="right">12.8</td>
  </tr>
  <tr>
    <td align="right">Clg-SetP-Sch</td>
    <td align="right">CoolingSetpoint</td>
    <td align="right">23.9</td>
    <td align="right">40.0</td>
  </tr>
</table>

<b>EAp2-4/5. Performance Rating Method Compliance</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Electric Energy Use [GJ]</td>
    <td align="right">Electric Demand [W]</td>
    <td align="right">Natural Gas Energy Use [GJ]</td>
    <td align="right">Natural Gas Demand [W]</td>
    <td align="right">Additional Energy Use [GJ]</td>
    <td align="right">Additional Demand [W]</td>
  </tr>
  <tr>
    <td align="right">Interior Lighting-Other</td>
    <td align="right">       11.50</td>
    <td align="right">    12107.98</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Interior Lighting-Process</td>
    <td align="right">        0.41</td>
    <td align="right">      450.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Exterior Lighting</td>
    <td align="right">        5.78</td>
    <td align="right">      800.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Space Heating</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        2.24</td>
    <td align="right">    40770.02</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Space Cooling</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Pumps</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Heat Rejection</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Fans-Other</td>
    <td align="right">        1.70</td>
    <td align="right">     2862.20</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Fans-Parking Garage</td>
    <td align="right">        2.68</td>
    <td align="right">     1000.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Service Water Heating</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">       43.85</td>
    <td align="right">    85690.02</td>
  </tr>
  <tr>
    <td align="right">Interior Equipment-Other</td>
    <td align="right">        4.44</td>
    <td align="right">     4719.56</td>
    <td align="right">        0.66</td>
    <td align="right">       16.16</td>
    <td align="right">        1.11</td>
    <td align="right">     1215.00</td>
  </tr>
  <tr>
    <td align="right">Interior Equipment-Industrial Process</td>
    <td align="right">        2.46</td>
    <td align="right">     2700.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Interior Equipment-Elevators and Escalators</td>
    <td align="right">        0.80</td>
    <td align="right">     1000.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Refrigeration Equipment</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Cooking</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.82</td>
    <td align="right">       20.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Total Line</td>
    <td align="right">       29.77</td>
    <td align="right">&nbsp;</td>
    <td align="right">        3.73</td>
    <td align="right">&nbsp;</td>
    <td align="right">       44.96</td>
    <td align="right">&nbsp;</td>
  </tr>
</table>


## Engineering Reference ##

No changes

## Example File and Transition Changes ##

The end use subcategory fields will be updated.

## References ##

v4_Minimum Energy Performance Calculator_v04.xlsm
