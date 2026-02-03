Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: `Coil:Cooling:DX:MultiSpeed`

The only change is for field F16, which is A12.  Logic to apply:

if key = blank, change to = 'NaturalGas'
if key = 'PropaneGas', change to = 'Propane'

All other fields stay the same.

# Object Change: `Coil:Heating:DX:MultiSpeed`

The only change is for field F16, which is A9.  Logic to apply:

if key = blank, change to = 'NaturalGas'
if key = 'PropaneGas', change to = 'Propane'

All other fields stay the same.

# Object Change: `CoolingTower:SingleSpeed`

Fields 1-16 remain the same.
After F16, insert four new blank fields that represent user-enterable design conditions.
Shift all later fields down by four.

# Object Change: `CoolingTower:TwoSpeed`

Fields 1-24 remain the same.
After F24, insert four new blank fields that represent user-enterable design conditions.
Shift all later fields down by four.

# Object Change: `CoolingTower:VariableSpeed:Merkel`

Fields 1-24 remain the same.
After F24`, insert four new blank fields that represent user-enterable design conditions.
Shift all later fields down by four.

# Object Change: `AirflowNetwork:SimulationControl`

Field A4, F4 (Wind Pressure Coefficient Array Name) is removed, all other fields rename the same.

# Object Change: `ZoneCapacitanceMultiplier:ResearchSpecial`

Insert new field 1 set to 'Multiplier'.
Insert new blank field 2 (for Zone Name).
Shift all later fields down by two.

# Object Change: `WaterHeater:HeatPump:WrappedCondenser`

Field A27 (Tank Element Control Logic) had a typo in previous versions.
There was a key `MutuallyExlcusive` that should be `MutuallyExclusive`.
This is field F35.  So the transition is simply to check if the typo version was there and replace it.
