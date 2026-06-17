Enhance Outdoor Air Reporting
================

**Jason Glazer and Mike Witte, GARD Analytics**

 - November 20, 2019
 - December 30, 2019 - added Outdoor Air Limiting Factor tables, new output variables, shortened some table title names, added at and above
 minimum to Total Outdoor Air by Airloop table, added "time at minimum" and "time above minimum" and moved "time below minimum" to
 a single table.
 - January 23, 2020 - remove ACH (air changes per hour) tables and columns, remove minimum outdoor air during occupancy tables,
 move and rename the required airflow rate based on 62.1 computed each timestep to Voz-dyn and the associated times,
 added Time Above Zero When Unoccupied
 - January 27, 2020 - changed "Dynamic Zone Outdoor Airflow - Voz-dyn" to "Dynamic Target Ventilation - Voz-dyn"
 - April 16, 2020 - added Design Document portion


## Justification for New Feature ##

Currently, users seeking to use EnergyPlus for LEED work need to document the use of Outdoor Air (OA) in their models. While
the current Outdoor Air Summary in EnergyPlus provides some information to help these users, it does not provide enough information.
Users often find it difficult to compare the OA delivered for proposed vs. baseline simulations, especially with economizers
and zone equipment.

## Overview ##

Specific tasks for this include:

- Expand the scope of the OA Summary to include zone heating, ventilation and air conditioning (HVAC) equipment.

- Add tracking and table outputs for the minimum requested OA flow in addition to the existing output for actual OA flow.

- Track times when the delivered OA is less than the minimum requirement, and

- Provide outputs at standard air density.

An issue was started to gather further input on this topic:

https://github.com/NatLabRockies/EnergyPlus/issues/5298

Which stated:

> Users often find it difficult to compare the outdoor air delivered for proposed vs baseline simulations. The logical place to look
> is the Outdoor Air Summary Report and the component sizing report. The component sizing report shows the minimum outdoor air flow
> rates for Controller:OutdoorAir objects that have autosized OA flow rates, but there is no building total. But this would only
> cover air handlers, not zone equipment. OA flow rates show up in some other tables as well, but again, no building totals.
> The Outdoor Air Summary report has average and minimum OA flow rates during occupied hours listed by zone, reported as ACH. It
> would be helpful to add flow in CFM or m3/s to these tables, and to add a total row for the entire building. Economizers can
> cloud the issue as well. Is it useful to report what the minimum flows would be if the economizer was not operating?

More than a dozen people who were known to regularly submit projects under LEED using EnergyPlus were specifically asked to comment on
the issue and provided comments:

- The total minimum OA for the building needs to be compared between the Baseline and Proposed Case models.

- The Controller:OutdoorAir component sizing summary only shows values if the min/max were autosized. If the user enters a hard value
the table is empty. The hard value should be reported as well when not autosized.

- Include design (not including the economizer mode where OA may equal SA) and minimum (to check that area ventilation requirement is
modeled correctly if DCV is included) occupied ventilation by system (both zone level and air handlers) and a total for the building
in the Outdoor Air Summary report.

- If it would make more sense, this information could be included in a new report. You could label the new report as ASHRAE 62
Ventilation Summary, Mechanical Ventilation Summary, etc.

- Thinking about tabular reports (hence forgetting about tables of hour versus month (weekends v weekends too) showing OA numbers),
I would say that two metrics would be useful: "total outdoor air volume (m3) for the run period, per zone and building" and "total
outdoor air volume when economizer is off." That would help see whether the deviation in absolute numbers are off because of economizer
or not. It would require setting a new output variable to keep track of OA only when econ isn't running, so annoying and probably
tedious but doable.



## E-mail and  Conference Call Conclusions ##

No call yet

## Approach ##

The current report is shown below

### Current Existing Report ###

<p>Report:<b> Outdoor Air Summary</b></p>
<p>For:<b> Entire Facility</b></p>
<p>Timestamp: <b>2019-11-13
    07:52:15</b></p>
<b>Average Outdoor Air During Occupied Hours</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Average Number of Occupants</td>
    <td align="right">Nominal Number of Occupants</td>
    <td align="right">Zone Volume [ft3]</td>
    <td align="right">Mechanical Ventilation [ACH]</td>
    <td align="right">Infiltration [ACH]</td>
    <td align="right">AFN Infiltration [ACH]</td>
    <td align="right">Simple Ventilation [ACH]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1</td>
    <td align="right">        9.50</td>
    <td align="right">       11.00</td>
    <td align="right">     8450.18</td>
    <td align="right">       1.591</td>
    <td align="right">       0.057</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1</td>
    <td align="right">        4.32</td>
    <td align="right">        5.00</td>
    <td align="right">     3648.94</td>
    <td align="right">       2.921</td>
    <td align="right">       0.057</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1</td>
    <td align="right">        9.50</td>
    <td align="right">       11.00</td>
    <td align="right">     8450.18</td>
    <td align="right">       1.484</td>
    <td align="right">       0.057</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1</td>
    <td align="right">        4.32</td>
    <td align="right">        5.00</td>
    <td align="right">     3648.94</td>
    <td align="right">       3.098</td>
    <td align="right">       0.057</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1</td>
    <td align="right">       17.27</td>
    <td align="right">       20.00</td>
    <td align="right">    15812.07</td>
    <td align="right">       0.882</td>
    <td align="right">       0.059</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
</table>
<i>Values shown for a single zone without multipliers</i>
<br><br>
<b>Minimum Outdoor Air During Occupied Hours</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Average Number of Occupants</td>
    <td align="right">Nominal Number of Occupants</td>
    <td align="right">Zone Volume [ft3]</td>
    <td align="right">Mechanical Ventilation [ACH]</td>
    <td align="right">Infiltration [ACH]</td>
    <td align="right">AFN Infiltration [ACH]</td>
    <td align="right">Simple Ventilation [ACH]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1</td>
    <td align="right">        9.50</td>
    <td align="right">       11.00</td>
    <td align="right">     8450.18</td>
    <td align="right">       0.030</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1</td>
    <td align="right">        4.32</td>
    <td align="right">        5.00</td>
    <td align="right">     3648.94</td>
    <td align="right">       0.058</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1</td>
    <td align="right">        9.50</td>
    <td align="right">       11.00</td>
    <td align="right">     8450.18</td>
    <td align="right">       0.028</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1</td>
    <td align="right">        4.32</td>
    <td align="right">        5.00</td>
    <td align="right">     3648.94</td>
    <td align="right">       0.068</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1</td>
    <td align="right">       17.27</td>
    <td align="right">       20.00</td>
    <td align="right">    15812.07</td>
    <td align="right">       0.016</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
    <td align="right">       0.000</td>
  </tr>
</table>
<i>Values shown for a single zone without multipliers</i>
<br><br>

After reviewing the original task as well as the input received issue #5298, the proposed updated Outdoor Air Summary report
will appear as shown below.

### Proposed New Report ###

<p>Report:<b> Outdoor Air Summary</b></p>
<p>For:<b> Entire Facility</b></p>
<p>Timestamp: <b>2019-11-13
    07:52:15</b></p>

<b>Mechanical Ventilation Parameters by Zone</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">AirLoop Name</td>
    <td align="right">Average Number of Occupants</td>
    <td align="right">Nominal Number of Occupants</td>
    <td align="right">Zone Volume [ft3]</td>
    <td align="right">Zone Area [ft2]</td>
    <td align="right">Design Zone Outdoor Airflow - Voz [ft3/min]</td>
    <td align="right">Minimum Dynamic Target Ventilation - Voz-dyn-min [ft3/min]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1</td>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1</td>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1</td>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1</td>
    <td align="right">Loop-2</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1</td>
    <td align="right">Loop-2</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        </td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<i>Zone values shown for a single zone without multipliers at standard air density. The Totals row includes zone multipliers.</i>
<br><br>


<b>Total Outdoor Air by Zone</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Mechanical Ventilation [ft3]</td>
    <td align="right">Natural Ventilation [ft3]</td>
    <td align="right">Total Ventilation [ft3]</td>
    <td align="right">Infiltration [ft3]</td>
    <td align="right">Total Ventilation and Infiltration [ft3]</td>
    <td align="right">Dynamic Target Ventilation - Voz-dyn [ft3]</td>
    <td align="right">Time Below Voz-dyn [Hours]</td>
    <td align="right">Time At Voz-dyn [Hours]</td>
    <td align="right">Time Above Voz-dyn [Hours]</td>
    <td align="right">Time Above Zero When Unoccupied [Hours]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<i>Zone values shown for a single zone without multipliers at standard air density. The Totals row includes zone multipliers.</i>
<br><br>

<b>Average Outdoor Air During Occupancy by Zone - Flow Rates</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Mechanical Ventilation [ft3/min]</td>
    <td align="right">Natural Ventilation [ft3/min]</td>
    <td align="right">Total Ventilation [ft3/min]</td>
    <td align="right">Infiltration [ft3/min]</td>
    <td align="right">Total Ventilation and Infiltration [ft3/min]</td>
    <td align="right">Dynamic Target Ventilation - Voz-dyn [ft3/min]</td>
    <td align="right">Time Below Voz-dyn [Hours]</td>
    <td align="right">Time At Voz-dyn [Hours]</td>
    <td align="right">Time Above Voz-dyn [Hours]</td>
  </tr>
  <tr>
    <td align="right">SPACE1-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE2-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE3-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE4-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">SPACE5-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<i>Zone values shown for a single zone without multipliers at standard air density. The Totals row includes zone multipliers.</i>
<br><br>


<b>Total Outdoor Air by Airloop</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Mechanical Ventilation [ft3]</td>
    <td align="right">Sum Zone Dynamic Target Ventilation - Voz-sum-dyn [ft3]</td>
    <td align="right">Time Below Voz-sum-dyn [Hours]</td>
    <td align="right">Time At Voz-sum-dyn [Hours]</td>
    <td align="right">Time Above Voz-sum-dyn [Hours]</td>
    <td align="right">Time Above Zero When Unoccupied [Hours]</td>
  </tr>
  <tr>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Loop-2</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<i>Values shown at standard air density. Infiltration and natural ventilation are not included.</i>
<br><br>


<b>Average Outdoor Air During Occupancy by Airloop</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">Mechanical Ventilation [ft3/min]</td>
    <td align="right">Sum Zone Dynamic Zone Outdoor Airflow - Voz-sum-dyn [ft3/min]</td>
    <td align="right">Time Below Voz-sum-dyn [Hours]</td>
    <td align="right">Time At Voz-sum-dyn [Hours]</td>
    <td align="right">Time Above Voz-sum-dyn [Hours]</td>
  </tr>
  <tr>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Loop-2</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<i>Values shown at standard air density. Infiltration and natural ventilation are not included.</i>
<br><br>



<b>Times for Outdoor Air Limiting Factors During Occupancy</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">No Limiting Factor [hours]</td>
    <td align="right">High Humidity [hours]</td>
    <td align="right">Night Ventilation [hours]</td>
    <td align="right">Economizer [hours]</td>
    <td align="right">Demand Controlled Ventilation [hours]</td>
    <td align="right">Exhaust Flow [hours]</td>
    <td align="right">Limits and Scheduled Limits [hours]</td>
    <td align="right">Demand Limiting [hours]</td>
    <td align="right">Energy Management System [hours]</td>
  </tr>
  <tr>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Loop-2</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<br><br>

<b>Average Outdoor Air For Each Outdoor Air Limiting Factors During Occupancy</b><br><br>
<table border="1" cellpadding="4" cellspacing="0">
  <tr><td></td>
    <td align="right">No Limiting Factor [ft3/min]</td>
    <td align="right">High Humidity [ft3/min]</td>
    <td align="right">Night Ventilation [ft3/min]</td>
    <td align="right">Economizer [ft3/min]</td>
    <td align="right">Demand Controlled Ventilation [ft3/min]</td>
    <td align="right">Exhaust Flow [ft3/min]</td>
    <td align="right">Limits and Scheduled Limits [ft3/min]</td>
    <td align="right">Demand Limiting [ft3/min]</td>
    <td align="right">Energy Management System [ft3/min]</td>
  </tr>
  <tr>
    <td align="right">Loop-1</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Loop-2</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
  <tr>
    <td align="right">Totals</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
    <td align="right">        0.00</td>
  </tr>
</table>
<i>Values shown at standard air density.</i>
<br><br>

## Testing/Validation/Data Sources ##

Compare current outputs including the sum of timestep results to annual results shown in the new reports.

## Input Output Reference Documentation ##

Add a paragraph for each table describing the contents of the table as part of the section on Output:Table:SummaryReports.

## Input Description ##

No changes to input.

## Outputs Description ##

New output variables would include:

- Zone time below minimum outdoor air
- Zone time at minimum outdoor air
- Zone time above minimum outdoor air
- Airloop limiting factor for outdoor air (0 to 9 corresponding to that table)

Additional output varibles will be included that correspond to columns not already defined.

The OutputDetailsAndExamples documentation includes a section on eplustbl.htm containing a
description and example of the Outdoor Air Summary which would be updated.

## Engineering Reference ##

No changes to the engineering reference are expected.

## Example File and Transition Changes ##

No changes to example files or transition files are expected.

## References ##

None.

## Design Document ##

To implement the updated outdoor air summary report the following will be done:

- Update OutputReportPredefined::SetPredefinedTables() to remove unused portions of the old report, move the columns
from the old report to the new report, and add subtables and new columns to support the new version of the report.

- Update OutputReportTabular::FillRemainingPredefinedEntries() to support the new report columns and remove unused columns
of results from the old report.

- Update SystemReports::ReportMaxVentilationLoads() which calculates the OA mass flow rate for each zone for both zone
equipment and air loops.

- Update DataZoneEquipment::CalcDesignSpecificationOutdoorAir() which is called by SingleDuct.cc::CalcOAMassFlow()
to support the new output columns.

- Update MixedAir::CalcOAController to support the new output columns.

Additional changes may be required to create the data needed for all the columns for the revised outdoor air summary
report subtables.
