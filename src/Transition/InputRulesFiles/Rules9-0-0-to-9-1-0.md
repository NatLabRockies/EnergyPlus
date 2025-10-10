Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: `HybridModel:Zone`

Fields F1-F4 remains the same.
After F4, insert new alpha input choice field (A5) 'Calculate Zone People Count' with the default value 'No'.
Old field F5 (A5) shifts down to F6 (A6) 'Zone Measured Air Temperature Schedule Name'
Insert 10 new fields, new F7-F16, all blank.
Shift all later fields down by 11. The old input fields F6-F9 (N1-N4) become the new F17-F20.

# Object Change: `ZoneHVAC:EquipmentList`

Fields F1-F2 remain the same.
Each extensible block has 2 numeric fields added to the end of it, making the extensible block 6 items long instead of 4.
The additional fields can be left empty.

