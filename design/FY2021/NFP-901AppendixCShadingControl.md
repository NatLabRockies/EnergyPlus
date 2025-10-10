# NFP ASHRAE 90.1 2019 Appendix C Shading Control

**Xuechen (Jerry) Lei, Jeremy Lerond, and Jian Zhang. PNNL**

- Original Date: 08/11/2021
- Revision Date: 01/06/2022

## Justification for New Feature

In "ASHRAE Standard 90.1-2019, Appendix C - Methodology for Building Envelope Trade-Off Option in Section 5.6", there is a shading control requirement:

> **C3.5.5.1 Shading**
>
> Manually operated interior shades shall be modeled on all vertical fenestration. **Shades shall be modeled to be in the lowered position when either the transmitted luminance is greater than 200 cd/ft2 or the direct solar transmitted energy exceeds 30 Btu/h·ft2 and then remain lowered for rest of the day.** Shades shall be modeled with visible light transmittance of 0.10, visible light reflectance of 0.40, solar transmittance of 0.21, and solar reflectance of 0.23. Permanent shading devices such as fins and overhangs shall be modeled.

This shading control (bolded in quote) can be summarizied / generalized as: if (transmitted luminance > threshold_x) or (direct solar transmitted energy > threshold_y), then shades should be on and stay on till the end of the day.

Everyone who tries to adopt Appendix C in their model should be able to implement this control in a straightfoward manner without explicitly specifying the control logic through EMS related objects. Currently, EnergyPlus has various `Shading Control Type` options in the `WindowShadingControl` object (EP 9.5 I/O Reference 1.10.58) but this is not one of them. We would like to add this control feature to allow users to capture 90.1-2019 Section C3.5.5.1 easily.

## E-mail and Conference Call Conclusions

The current ASHRAE 90.1 narrative does not descibe sufficiently on the detail about 1) whether the luminance requirement for shading control in Appendix C is regarding a photosensor, and if it is, where should that photosensor location be; 2) what exactly should be considered as "rest of the day" in this context. The PNNL team reached out to ASHRAE 90.1 ESC (Envelope Subcommittee) and confirmed that 1) the luminance value for this control is to be based on the photosensor location defined in the daylighting reference point; 2) once the shade is lowered, it will remain down until the end of the day, i.e. midnight.

When interpreting "rest of the day" being "midnight", feedback collected during technicalities call suggests expanding the implementation to cover three additional shading off options: 1) sunset; 2) sunrise next day; and 3) when luminance value drops below a setpoint, which requires adding one more field 'Setpoint3' under WindowShadingControl.

<!-- **The rest of this document describes the case of adding one new windows shading control option which accounts for the control option of shading off by midnight, other control options will be implemented in similar ways.** -->

At this moment, we decided not to implement window shading control strategy that would turn off shade when luminance value drops below a setpoint in this NFP because 1) it requires adding more setpoints (e.g. a solar gain lower bound to turn off shades and a luminance lower bound to turn off shades to make them equal) to the WindowShadingControl object, which already has two setpoints fields, and having too many setpoints can be confusing and not ideal; 2) turning on and off shades based on setpoints may cause shades to turn on and off frequently (oscillate) during a short period of time when luminance / solar gain value variates up and down.

Feedback collected during technicalities call also suggests not adding luminance from different windows. We further confirm with lightning experts that the luminance control described in the code implies control per window and adding luminance values from different windows view at the same reference point does not make sense.

## Overview

Three new `Shading Control Type` options in the `WindowShadingControl` object needs to be implemented: `OnIfHighSolarOrHighLuminanceTillMidnight`, `OnIfHighSolarOrHighLuminanceTillSunset`, `OnIfHighSolarOrHighLuminanceTillNextMorning`.

Two setpoints need to be specificd for this shading control option in the idf file. There are two existing setpoints fields in `WindowShadingControl` object:

- Setpoint: units depend on the type of trigger:
  - W/m2 for solar-based controls
  - W for cooling- or heating-based controls
  - Degrees C for temperature-based controls
- Setpoint2: only used for certain two-setpoint control types.
  - `OnIfHighOutdoorAirTempAndHighSolarOnWindow`
  - `OnIfHighOutdoorAirTempAndHighHorizontalSolar`
  - `OnIfHighZoneAirTempAndHighSolarOnWindow`
  - `OnIfHighZoneAirTempAndHighHorizontalSolar`

Based on the current setpoints setup, for the new control option, direct solar transmitted energy can be directly specified in `Setpoint` (W/m2).

To add a setpoint for luminance based control in the option to be implemented, we will be using the Setpoint2 field of `WindowShadingControl` object for the Zone luminance setpoint and expand the application of this field with "cd/m2 for luminance-based controls". We choose not to use `Daylighting:Control` object as this object is dedicated for illuminance based control and adding luminance based control would change its scope and cause confusion (for instance, we would not support luminance based electric lighting control).

In the meantime, we decide to continue using the daylighting objects (control and reference points) to specify daylighting reference points for measuring luminance values and to calculate luminance values with existing routines associated with daylighting control object.

## Approach

1. Expand enumerations `WindowShadingControlType` (`DataSurfaces.hh`) with new control types. Related variable `cValidWindowShadingControlTypes` (`SurfaceGeometry.cc`) is also updated.
2. New state variable `state.dataEnvrn->SunIsUpPrevTS` is added for use in `OnIfHighSolarOrHighLuminanceTillNextMorning`.
3. Add three new control options and non-luminance related control logic in `WindowShadingManager` (`SolarShading.cc`).
4. Add luminance based shading control logic in `DayltgInteriorIllum` (`DaylightingManager.cc`)

****
## Testing/Validation/Data Sources

Two unit tests are added:
- `DaylightingManager_DayltgInteriorIllum_LuminanceShading_Test` tests luminance-based control in `DaylightingManager::DayltgInteriorIllum`
- `WindowShadingManager_Lum_Test` tests control type selection and solar-based control in `SolarShading::WindowShadingManager`
## Input Output Reference Documentation

In `group-thermal-zone-description-geometry.tex`, Under `WindowShadingControl`

- Description of new control options are added to `Field: Shading Control Type`.
- New control option names are added to `Field: setpoint 2`.

## Input Description

- Add three new `WindowShadingControl` `Shading Control Type` options: `OnIfHighSolarOrHighLuminanceTillMidnight`, `OnIfHighSolarOrHighLuminanceTillSunset`, and `OnIfHighSolarOrHighLuminanceTillNextMorning`.
- Change units for `WindowShadingControl` `Setpoint 2` from 'W/m2 or deg C' to 'W/m2, deg C or cd/m2'.

## Outputs Description

N/A

## Example File and Transition Changes

Existing test file `WindowTests.idf` is updated with shading of two window surfaces control by `OnIfHighSolarOrHighLuminanceTillMidnight`, in which the luminance and solar set points for the newly added `WindowShadingControl` object follows typical threshold settings from 90.1 Appendix C. No transition change is expected.

## References

- ASHRAE. 2019. ANSI/ASHRAE/IES 90.1-2019, Energy Standard for Buildings Except Low-Rise
  Residential Buildings. ASHRAE, Atlanta, GA
- [Add additional option for shading control for WindowProperty:ShadingControl object #7081](https://github.com/NREL/EnergyPlus/issues/7081)
