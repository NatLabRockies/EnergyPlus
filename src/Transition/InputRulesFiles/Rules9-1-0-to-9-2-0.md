Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: `Foundation:Kiva`
Field 1 remains the same.
After F1, insert new numeric input field (N1) 'Initial Indoor Air Temperature'.  The field is blank by default.
Shift all later fields down by 1. The old input fields F2-F56 become the new F3-F57.

# Object Change: `RunPeriod`
Field A1 "Name" is now a required field. If blank, add a name. If the old idf has more than one RunPeriod object, all of the Name fields must be unique.

# Object Change: `Schedule:File`
The only change is for Field A4 (Column Separator). If A4 = "Fixed" then replace with "Space". Field A1-A3, A5, and N1-N4 remain the same.

# Object Change: `ThermalStorage:Ice:Detailed`
Fields A5 and A7 change description to Discharging Curve Variable Specifications and Charging Curve Variable Specifications, respectively.  The old options for these fields were either QuadraticLinear or CubicLinear.

For A5 (field 6), if the field previously had QuadraticLinear, it should be changed to FractionDischargedLMTD.  If A5 was defined as CubicLinear, it should be changed to LMTDMassFlow.

For A7 (field 7), if the field had previously been QuadraticLinear, it should now say FractionChargedLMTD.  If A7 was CubicLinear, it should be changed to LMTDMassFlow.

# Object Change: `ZoneHVAC:EquipmentList`
Zone Equipment <n></n> Sequential Cooling/Heating Fraction fields are now schedules.

Fields 7 & 8 (old N3 and N4) change from a numeric input to a schedule name.
If the field is blank, then leave it blank.
If the fied is not blank, then add a Schedule:Constant object using the old numeric value and replace the fraction with the new schedule name.
To avoid warnings, add a ScheduleTypeLimits object and reference it in the new schedule objects.

Repeat this for each equipment group of 6 fields; fields 13 & 14, 19 & 20, etc.

# Object Change: `ObjectNameC`

# Object Change: `ObjectNameD`

# Object Change: `ObjectNameE`

# Object Change: `ObjectNameF`
