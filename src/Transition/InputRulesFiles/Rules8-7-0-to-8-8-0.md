Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: `AvailabilityManager:NightCycle`

Fields 1-5 remain the same.
After F5, insert new alpha input choice field (A5) 'Cycling Run Time Control Type' with default key input value of 'FixedRunTime'.
Shift all later fields down by 1. The old input field F6 - F10 becomes the new F7-11. The new input field becomes F6.

# Object Change: `ZoneControl:ContaminantController`

Insert new blank field 6 (A6) (for Maximum Carbon Dioxide Concentration Schedule Name).
Shift all later fields down by one.

# Object Change: `ZoneHVAC:IdealLoadsAirSystem`

Insert new blank field 5 (A5) (for System Inlet Air Node Name).
Shift all later fields down by one.

# Object Change: `Table:TwoIndependentVariables`

Insert new blank field 14 (A7) (for External File Name).
Shift all later fields down by one.

# Object Change: `Output:Surfaces:List`

The only change is for field F1, which is A1.  Logic to apply:

if key = 'DecayCurvesfromZoneComponentLoads', change to = 'DecayCurvesFromComponentLoadsSummary'

# Object Change: `SurfaceProperty:ExposedFoundationPerimeter`

Insert new field 2 (A2) (for Exposed Perimeter Calculation Method).
Shift all later fields down by one.

This object is now required for any floor surface with a Foundation Outside Boundary Condition.

# Object Change: `Foundation:Kiva:Settings`

If field 8 (A1) was "Autocalculate" change to "Autoselect" (to match choice key).

# Object Change: `UnitarySystemPerformance:Multispeed`

Fields 1-4 remain the same.
After F4, insert one new blank field that represents the No Load Supply Air Flow Rate Ratio.
Shift all later fields down by one.

# Object Change: `Coil:Cooling:DX:SingleSpeed`

The only change is for field F15, which is N6.  Logic to apply:

Fields 1-14 remain the same.
After F14, insert one new blank field that represents N6 as Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}.
Shift all later fields down by one (old N6 becomes N7).

# Object Change: `Coil:Cooling:DX:TwoSpeed`

The only change is for field F23, which is N10.  Logic to apply:

Fields 1-22 remain the same.
After F22, insert one new blank field that represents N10 as Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}.
Shift all later fields down by one (old N10 becomes N11).

# Object Change: `Coil:Cooling:DX:MultiSpeed`

The only change is for field F7, which is N1.  Logic to apply:

Fields 1-6 remain the same.
After F6, insert one new blank field that represents N1 as Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}.
Shift all later fields down by one (old N1 becomes N2).

# Object Change: `Coil:Cooling:DX:VariableSpeed`

The only change is for field F16, which is N10.  Logic to apply:

Fields 1-15 remain the same.
After F15, insert one new blank field that represents N10 as Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}.
Shift all later fields down by one (old N10 becomes N11).

# Object Change: `Coil:Cooling:DX:TwoStageWithHumidityControlMode`

The only change is for field F19, which is N5.  Logic to apply:

Fields 1-18 remain the same.
After F18, insert one new blank field that represents N5 as Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}.
Shift all later fields down by one (old N5 becomes N6).

# Object Change: `ZoneHVAC:PackagedTerminalHeatPump`

The only change is for field F18, which was N8.  Logic to apply:

Fields 1-17 remain the same.
After F17, delete one field. This field was removed as redundant: Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}.
Shift all later fields up by one (old N9 becomes N8).

# Object Change: `WindowMaterial:Blind:EquivalentLayer`

The only change is for field F6 value. Th number of fields remains the same:

if SlatAngle < 90, New SlatAngle = SlatAngle
if SlatAngle >= 90, New SlatAngle =  90.0 - SlatAngle
