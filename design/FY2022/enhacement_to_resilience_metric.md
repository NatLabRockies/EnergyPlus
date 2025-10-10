Enhancement to Resilience Metrics Reporting
================

**Xuan Luo, Yujie Xu, Tianzhen Hong**

**Lawrence Berkeley National Laboratory**

 - Original Date: October 21, 2021
 - Modified Date: October 27, 2021

## Justification for New Feature ##

With the increasing frequency and severity of extreme weather events (e.g., heat waves), it is crucial to ensure urban buildings and infrastructure are resilient to provide critical services to preserve human life and properties during natural disasters. Building resilience could become an additional value proposition for technologies and systems if it can be reliably quantified, valued, and trusted by stakeholders. Resilience metrics or an assessment of potential vulnerability, likelihood, and consequence for each risk would help to prioritize further consideration of these risk factors of building resilience.

EnergyPlus currently calculates a set of thermal, visual, and indoor air quality (IAQ) resilience metrics as optional reportable variables. The resilience reports are presented as three tabular summary reports (Thermal Resilience Summary, Indoor Air Quality Resilience Summary, and Visual Resilience Summary) and can be requested as part of the tabular reports. The thermal resilience report includes Heat Index, Humidex, and Standard Effective Temperature (SET) hours as three separate metrics. To support the need for new resilience metrics in building simulation and to provide a more convenient path in adopting existing metrics, we propose to (1) calculate and report Hours of Safety, Setpoint Degree Hours Unmet, Discomfort Weighted Hours as three additional building thermal resilience metrics following EPA and ASHRAE Standard 55–2020 guidance, and (2) improve current resilience metrics reporting by allowing users to specify the period of calculating and reporting the metrics which is not necessarily the same as the simulation period.

## Overview ##

### 1.1 1. Hours of Safety ###

Hours of Safety is a framework developed by EPA and RMI to help understand how long a home can maintain thresholds of comfort and safety before reaching unsafe indoor temperature levels [1]. The concept attempts to define the duration of time that homes can be expected to provide safe temperatures when the power goes out based on building characteristics and energy efficiency levels (e.g., insulation, infiltration). This metric can be used to quantify the amount of time people are exposed to extremely hot or cold temperatures indoors. The information can be used to guide weatherization efforts and emergency response measures in considering the health and safety of vulnerable populations as extreme weather events increase in frequency.

Hours of Safety for cold or hot weather events should be defined by the longest duration (number of hours), starting from the beginning time of the risk period (e.g., the start time of a power outage), to not exceed temperature thresholds defined for a specific type of population (e.g., cold stress safety temperature for the healthy population as 60°F, or 16°C). To define the thresholds, we will use EnergyPlus’ existing thermal comfort model controlled primarily by the People input object, and add two fields in the *People* object, namely "*Cold Stress Temperature Threshold*" and "*Heat Stress Temperature Threshold*". To define the risk period, we will add a new object in EnergyPlus to declare the start and end time to evaluate the resilience metrics within those dates.

The default "*Cold Stress Temperature Thresholds*": 60°F (16°C) for the healthy population [1]. Other values can be used: 64°F (18°C) for the elderly [1], and 71°F (22°C) for nursing home residents [2]. The default "*Hot Stress Temperature Threshold*" is selected as 86°F (30°C) [2]. Users can modify these default thresholds with the "*Cold Stress Temperature Threshold*" and "*Heat Stress Temperature Threshold*" input fields in the *People* object.

We will report both the zone and building-level Hours of Safety. For the zone level, we will report the longest duration in a risk period a zone can maintain comfort and safety before reaching unsafe indoor temperature thresholds. For the building level, we’ll provide four summary statistics across all zones in the building: min, max, average, and sum.

In addition to the final value of hours of safety, three related quantities are also reported: the time when the zone temperature reaches the cold or hot stress threshold (column 2 in Table 1), the total number of hours when the zone temperature falls in the dangerous range throughout the reporting period (column 3 in Table 1) and during the occupied hours (column 4 in Table 1).

### 2. Setpoint Unmet Degree Hours (UDH) ###

The concept of UDH is analogous to that of temperature-weighted exceedance hours, a metric defined in Section L.3.2.2(b) of ASHRAE Standard 55–2020 [3]. The UDH metric is based on indoor cooling or heating setpoint and weights each hour that the temperature of a conditioned zone exceeds a certain threshold by the number of degrees Celsius by which it surpasses that threshold. Compared with average temperature or unmet hours, UDH provides a more complete picture of the overall history temperature exceedance.

UDH is calculated as follows:

![UDH_calc](https://github.com/NREL/EnergyPlus/blob/enhanceResilienceMetrics/design/FY2022/UDH_eq.png)

where T is the indoor air temperature [°C]; t is time [h]; and x<sub>+</sub> = x if x > 0, or 0 otherwise. T<sub>threshold</sub> is the indoor cooling or heating setpoint [°C] in both the grid-on and grid-off scenarios. A similar metric, the Exceedance Degree-Hour, is recently developed by Salimi et al [4]. Instead of thresholding, this metric weights each hour by the distance from the current SET to the comfort zone [4].

For reporting, four variables will be computed: heating and cooling setpoint UDH during occupied and the whole reporting period (see Table 3). For the building level, we’ll provide four summary statistics across all zones in the building: min, max, average, and sum.

### 3. Discomfort-weighted Exceedance Hours ###

Discomfort-weighted exceedance hours is the sum of the positive values of predicted mean vote (PMV) exceedance during occupied hours, where PMV exceedance = (PMV – threshold) for warm or very-hot conditions, and PMV exceedance = (threshold - PMV) for cool or very-cold conditions [5]. Warm, cool, very-hot, and very-cold exceedance hours are the number of hours in which occupants are uncomfortably warm (PMV>0.7), cool (PMV<−0.7), very hot (PMV>3), and very cold (PMV<−3). For example, discomfort-weighted warm exceedance hour is the sum of the positive values of (PMV - 0.7) during occupied hours, while discomfort-weighted cool exceedance hour is the sum of the positive values of (-0.7 - PMV) during occupied hours. Discomfort-weighted very-hot and very-cold exceedance hours are calculated analogously, using thresholds of 3 and -3, respectively. CBE developed an online tool to compute thermal comfort metrics including PMV, PPD, thermal sensation, SET, etc [6].

For reporting, we will present the zone-level and the aggregated building-level Occupant • Hours falling into the dis-comfort PMV ranges. For the building level, we’ll provide four summary statistics across all zones in the building: min, max, average, and sum.

### 4. Timespan of Report ###

Resilience metrics are more often evaluated during a certain period when a building is at risk (e.g., during the power outage event or heatwave event), and the period is not necessarily the same as the whole simulation period. We propose to improve current resilience metrics tabular reporting (in the html output) by allowing users to specify the period of calculating and reporting the metrics.

We propose to add an *Output:Table:ReportPeriod* that takes an input of a Summary Report name (e.g. *ThermalResilienceSummary*). The reporting period can be defined with a start-date field and an end-date field as supplementary inputs to the *ThermalResilienceSummary* report.

## Approach ##

### 1. Addition of reporting tables.

The following tables will be appended to the existing Thermal Resilience Summary reports as part of the tabular summary reports.

Report: **Thermal Resilience Summary for Reporting Period 1**

For: **Entire Facility**

Timestamp: **2022-04-13 10:00:00**

**Reporting period: 1/1 8:00 -- 1/3 18:00**

**Hours of Safety for Cold Events**

|     |	Hours of Safety [hr]	| End Time of the Safety Duration | Safe Temperature Exceedance Hours [hr] | Safe Temperature Exceedance OccupantHours [hr] | Safe Temperature Exceedance OccupiedHours [hr]
|-----|------------------|---------------------------------|---------------------------------------|------------------|--------------|
|Space<sub>1</sub>||||
|...||||
|Space<sub>N</sub>||||
|Min||||
|Max||||
|Average||||
|Sum||||

<p style="text-align: center;"> Table 1. Sample report table of hours of safety for cold events.</p>

**Hours of Safety for Hot Events**

|     |	Hours of Safety [hr]	| End Time of the Safety Duration | Safe Temperature Exceedance Hours [hr] | Safe Temperature Exceedance OccupantHours [hr] | Safe Temperature Exceedance OccupiedHours [hr]
|-----|------------------|---------------------------------|---------------------------------------|------------------|--------------|
|Space<sub>1</sub>||||
|...||||
|Space<sub>N</sub>||||
|Min||||
|Max||||
|Average||||
|Sum||||

<p style="text-align: center;"> Table 2. Sample report table of hours of safety for hot events.</p>

**Unmet Degree Hours**

|     |Cooling Setpoint Unmet Degree-Hours (°C·h) | Cooling Setpoint Unmet Occupant-Weighted Degree-Hours (°C·h) | Cooling Setpoint Unmet Occupied Degree-Hours (°C·h) | Heating Setpoint Unmet Degree-Hours (°C·h) | Heating Setpoint Unmet Occupant-Weighted Degree-Hours (°C·h) | Heating Setpoint Unmet Occupied Degree-Hours (°C·h)
|----|------------------|---------------------------------|---------------------------------------|---------------------------------------|---------------------------------------|------------------|
|Space<sub>1</sub>|||||||
|...|||||||
|Space<sub>N</sub>|||||||
|Min|||||||
|Max|||||||
|Average|||||||
|Sum|||||||

<p style="text-align: center;"> Table 3. Sample report table of unmet degree hours (UDH).</p>

**Discomfort-weighted Exceedance Hours**

|     |	Very-cold Exceedance OccupantHours [hr] | Cool Exceedance OccupantHours [hr] | Warm Exceedance OccupantHours [hr] | Very-hot Exceedance OccupantHours [hr]
|-----|------------------|---------------------------------|---------------------------------------|------------------|
|Space<sub>1</sub>||||
|...||||
|Space<sub>N</sub>||||
|Min||||
|Max||||
|Average||||
|Sum||||

<p style="text-align: center;"> Table 4. Sample report table of discomfort-weighted exceedance occupanthours.</p>

**Discomfort-weighted Exceedance OccupiedHours**

|     |	Very-cold Exceedance OccupiedHours [hr] | Cool Exceedance OccupiedHours [hr] | Warm Exceedance OccupiedHours [hr] | Very-hot Exceedance OccupiedHours [hr]
|-----|------------------|---------------------------------|---------------------------------------|------------------|
|Space<sub>1</sub>||||
|...||||
|Space<sub>N</sub>||||
|Min||||
|Max||||
|Average||||
|Sum||||

<p style="text-align: center;"> Table 5. Sample report table of discomfort-weighted exceedance occupiedhours.</p>

When *ThermalResilienceSummary* is declared in *Output:Table:SummaryReports*, the tables above will be generated and presented in the tabular reports.

### 2. Additional Warnings ###

In the calculation of zone-level Heating/Cooling SET Degree-Hours, if multiple *People* objects are defined for one zone, and they have different SET, PMV, or Heat/Cold Stress Temperature Threshold, EnergyPlus will use the SET/PMV/Threshold Temperature of one *People* object to compute the zone level SET Degree-Hours. In this feature update, a warning message will be produced to caution users that such zone-level values will only be meaningful if all *People* object defined for the same zone agree on these measures.

### 3. Bug fixes ###
In the develop branch, for Heating and Cooling SET degree-hours tabular report, the hours are only accumulated for occupied hours for both the "SET > 30°C Hours (°C)" column and the column "SET > 30°C OccupantHours (°C)", due to the `NumOcc > 0` check and the `state.dataHeatBalFanSys->ZoneNumOcc(ZoneNum) > 0` check when passing values from `state.dataThermalComforts->ThermalComfortData(iPeople).PierceSET` to `state.dataHeatBalFanSys->ZoneOccPierceSET(ZoneNum)`.

Another issue more general to all thermal resilience metrics is that when computing the OccupantHour values, the floating point version of the number of occupants for a specific zone and timestep is cast into an integer first, than multiplied when computing the OccupantHour values. This means when the number of occupants is between 0 and 1, the tabular thermal resilience report treats it as not occupied.

This feature branch removes the restriction in SET degree-hour calculation and computes the aggregated SET degree-hours for the whole run/reporting period in the first column of the SET degree-hour tabular report. It will also remove the integer casting when computing the OccupantHour columns.

### 4. Documentation updates ###

The documentation of the *Output:Table:ReportPeriod* will be updated, suggesting users to include adaptation strategies people usually adopt during power outages, such as increased airflow rate from window usage during hot events and the use of alternative space heating strategies, such as fireplaces, natural gas or propane portable heaters during code events.

## Testing/Validation/Data Source(s): ##

Two example files (the DOE reference small office and the one zone uncontrolled model) will be modified to demonstrate the use of the new feature. Simulation results will be manually checked/benchmarked using an excel spreadsheet with input and output from EnergyPlus runs.

## IDD Object changes: ##

Two extra fields will be added to the current People object to define the safe temperature thresholds for a certain kind of population.

    People,
      N7, \field Cold Stress Temperature Threshold
          \type real
          \units C
          \note this is the indoor safe temperature threshold for cold stress
    \default 15.56
      N8, \field Heat Stress Temperature Threshold
          \type real
          \units C
          \note this is the indoor safe temperature threshold for heat stress
          \default 30

We will add a new object *Output:Table:ReportPeriod* to add supplementary information to define the reporting period of certain summary tables.

    Output:Table:ReportPeriod,
        \memo This object allows the user to add a reporting period to a certain tabular report when the reporting period is not the whole simulation period. When defined, the declared summary report is based on aggregations from the start date to the end date defined.
        \memo Multiple run periods may be input.
      A1, \field Name,
          \required-field
          \note descriptive name
          \note cannot be blank and must be unique
      A2, \field Report Name,
          \key ThermalResilienceSummary
          \key CO2ResilienceSummary
          \key VisualResilienceSummary
          \key AllResilienceSummaries
          \key …
      N1 , \field Begin Year
          \note start year of reporting, if specified
          \type integer
      N2 , \field Begin Month
          \required-field
          \minimum 1
          \maximum 12
          \type integer
      N3 , \field Begin Day of Month
          \required-field
          \minimum 1
          \maximum 31
          \type integer
      N4 , \field Begin Hour of Day
          \required-field
          \minimum 1
          \maximum 24
          \type integer
      N5 , \field End Year
          \note start year of reporting, if specified
          \type integer
      N6 , \field End Month
          \required-field
          \minimum 1
          \maximum 12
          \type integer
      N7 , \field End Day of Month
          \required-field
          \minimum 1
          \maximum 31
          \type integer
      N8 , \field End Hour of Day
          \required-field
          \minimum 1
          \maximum 24
          \type integer

## Proposed additions to Meters: ##

N/A

## Proposed Report Variables: ##

The following variables will be added to the "advanced report variable" set: Heating Unmet Degree Hours, Cooling Unmet Degree Hours, Very-cold Exceedance OccupantHours, Cool Exceedance OccupantHours, Warm Exceedance OccupantHours, Very-hot Exceedance OccupantHours

## References ##

[1]	S. Ayyagari, M. Gartman, and J. Corvidae, “A Framework for Considering Resilience in Building Envelope Design and Construction,” Feb. 2020.<br>
[2]	USGBC, “Passive Survivability and Back-up Power During Disruptions | U.S. Green Building Council,” Oct. 2018. https://www.usgbc.org/credits/passivesurvivability (accessed Oct. 26, 2021).<br>
[3]	ASHRAE, “Thermal Environmental Conditions for Human Occupancy,” p. 9, Apr. 2021.<br>
[4]	S. Salimi, E. Estrella Guillén, and H. Samuelson, “Exceedance Degree-Hours: A new method for assessing long-term thermal conditions,” Indoor Air, vol. 31, no. 6, pp. 2296–2311, 2021, doi: 10.1111/ina.12855.<br>
[5]	R. Levinson et al., “Key performance indicators for cool envelope materials, windows and shading, natural ventilation, and personal comfort systems,” Nov. 10, 2020.<br>
[6]	F. Tartarini, S. Schiavon, T. Cheung, and T. Hoyt, “CBE Thermal Comfort Tool: Online tool for thermal comfort calculations and visualizations,” SoftwareX, vol. 12, p. 100563, Jul. 2020, doi: 10.1016/j.softx.2020.100563.<br>
