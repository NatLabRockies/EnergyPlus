Dew-point Temperature Humidistat
================

**Jeremy Lerond, Pacific Northwest National Laboratory**

 - Original Date: 03/03/2026
 - Revision Date: 03/03/2026


## Justification for New Feature ##

Currently, EnergyPlus humidistats are used to control zone humidity using relative humidity (RH). However, in practice it is also common for thermostats/humidity controllers to control (or limit) humidity using dew-point temperature. In addition, standards such as ASHRAE 62.1 include requirements—under certain conditions—that the zone (space) dew-point temperature must not exceed a specified limit (see Addendum k to ASHRAE 62.1–2022).

This feature will allow users to specify a maximum/minimum allowable space dew-point temperature (i.e., a dew-point setpoint) to trigger humidity control.

While controlling humidity based on dew-point is currently possible in EnergyPlus using EMS programs, it requires having multiple programs (or a large one) to recalculate each zone's relative humidity setpoint based on the targeted dew-point temperature. This new feature will provide a more streamlined and user-friendly way to control humidity based on dew-point temperature.

## E-mail and Conference Call Conclusions ##

N/A

## Overview ##

The `ZoneControl:Humidistat` object uses both a humidifying and dehumidifying setpoint schedule based on RH. The new features will add a new `Control Variable` input field to specify what variable is described in the schedule. The value for this new input would be `RelativeHumidity` (current approach) and `Dew-point Temperature` (new approach).

## Approach ##

First, the existing schedule fields would be renamed as follows:
- `Humidifying Relative Humidity Setpoint Schedule Name` -> `Humidifying Setpoint Schedule Name`
- `Dehumidifying Relative Humidity Setpoint Schedule Name` -> `Dehumidifying Setpoint Schedule Name`

Second, a new input field would be added to the `ZoneControl:Humidistat` object: `Control Variable`.

Third, that control variable would be used to determine In `calcPredictedCorrector` the current schedule inputs are used to calculate `WZoneSetPoint` which is the zone humidity setpoint. The new schedules would be used to also calculate `WZoneSetpoint` but based on a dew-point temperature target rather than a relative humidity target. The user could mix/match schedules to control zone humidity, for instance, use a dew-point-based dehumidification schedule and a relative humidity-based humidification schedule.

## Testing/Validation/Data Sources ##

A unit test will be included to test the new feature.

## Input Output Reference Documentation ##

The following field description would be added to the input/output reference guide:

```latex
\paragraph{Field: Control Variable}\label{field-humidistat-control-variable}

This field describes if the schedules describe relative humidity or dew-point temperature values.
```

## Input Description ##

The following fields would be added to the `ZoneControl:Humidistat` object:

```
ZoneControl:Humidistat,
[...]
  A5 ; \field Control Variable
       \note When using RelativeHumidity, the schedule values should be in percentages.
       \type choice
       \key RelativeHumidity
       \key Dewpoint
       \default RelativeHumidity
```

## Outputs Description ##

No new outputs will be added.

## Engineering Reference ##

This new feature doesn't require any engineering reference changes.

## Example File and Transition Changes ##

This new feature doesn't require any transition changes.

## References ##

- ASHRAE Standard 62.1-2022 Addendum k, https://www.ashrae.org/file%20library/technical%20resources/standards%20and%20guidelines/standards%20errata/standards/62_1_2022_k_20240422.pdf
- Lennox S40 Smart Thermostat, https://www.lennox.com/dA/6003ce4d6f/508223-03b.pdf
