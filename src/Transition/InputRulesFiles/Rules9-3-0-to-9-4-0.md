Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: ‘Construction:InternalSource’

Summary: A new field was added into the middle of the object, everything else states the same (or shifted back if occurring after the new field).

Fields 1-5 remain the same.

NEW Field F6 (N5): Two-Dimensional Position of Interior Temperature Calculation Request.  Leave blank or assign a zero value (default).

Old Fields 6-15 remain the same, just shifted back one field to Fields 7-16.

# Object Change: 'ZoneHVAC:HybridUnitaryHVAC'

Fields 1-14 remain the same.

Insert field 15, Fan Heat Included in Lookup Tables.

Insert field 16, Fan Heat Gain Location.

Insert field 17, Fan Heat Gain In Airstream Fraction.

Field 18, previous field 15. No other changes.

Field 19, previous field 17. No other changes.

Remaining fields 20 onward are the same as previous fields 18 onwards. No other changes.

# Object Change: Output:DebuggingData

This object uses to accept numeric entries, where 1 = Yes and all others = No. This was changed to a choice type, accepting 'Yes' or 'No', and defaulting to 'No'.

cf #7740.

# Object Change: Output:Diagnostics

This object was made unique, and with an extensible "Key" field to add specific diagnostics.

cf #7742.

# Object Change: ‘ZoneHVAC:LowTemperatureRadiant:VariableFlow’

Summary: A new field was added into the middle of the object, everything else states the same (or shifted back if occurring after the new field).

Fields 1-4 remain the same.

NEW Field F5 (A5): Fluid to Radiant Surface Heat Transfer Model.  Leave blank or assign a value of “ConvectionOnly”.

Old Field 5 becomes NEW Field F6.

NEW Field F7 (N2): Hydronic Tubing Outside Diameter.  Leave blank or assign a value of “0.016”.

Old Field 6 becomes NEW Field F8.

NEW Field F9 (N4): Hydronic Tubing Conductivity.  Leave blank or assign a value of “0.35”.

Old Field 7 becomes New Field F10.

NEW Field F11 (A6): Setpoint Control Type.  Leave blank or assign a value of “HalfFlowPower” (default).

Old Fields 8-29 remain the same, just shifted back one field to Fields 12-33.

# Object Change: ‘ZoneHVAC:LowTemperatureRadiant:Electric’

Summary: A new field was added into the middle of the object, everything else states the same (or shifted back if occurring after the new field).

Fields 1-9 remain the same.

NEW Field F10 (A7): Setpoint Control Type.  Leave blank or assign a value of “HalfFlowPower” (default).

Old Fields 10-11 remain the same, just shifted back one field to Fields 11-12.

# Object Change: ‘ZoneHVAC:LowTemperatureRadiant:ConstantFlow’

Summary: A new field was added into the middle of the object, everything else states the same (or shifted back if occurring after the new field).

Fields 1-4 remain the same.

NEW Field F5 (A5): Fluid to Radiant Surface Heat Transfer Model.  Leave blank or assign a value of “ConvectionOnly”.

Old Field 5 becomes NEW Field F6.

NEW Field F7 (N2): Hydronic Tubing Outside Diameter.  Leave blank or assign a value of “0.016”.

Old Field 6 becomes NEW Field F8.

NEW Field F9 (N4): Hydronic Tubing Conductivity.  Leave blank or assign a value of “0.35”.

Old Field 7 becomes New Field F10.

NEW Field F11 (N5): Running Mean Outdoor Air Temperature Weighting Factor.  Leave blank or assign a value of “0.8” (default).

Old Fields 8-29 remain the same, just shifted back one field to Fields 12-33.

# Minor Changes:

## ZoneProperty:UserViewFactors:BySurfaceName,

The case was changed from `ZoneProperty:UserViewFactors:bySurfaceName` to `ZoneProperty:UserViewFactors:BySurfaceName` to match conventions.
This will have no effect on EnergyPlus which handles this in a case-insensitive manner.

cf #7499.

## Curve:DoubleExponentialDecay,

Two typos were corrected in field names.

9.3.0:

```
  N4 , \field Coefficient3 C4
  N5 , \field Coefficient3 C5
```

9.4.0:

```
  N4 , \field Coefficient4 C4
  N5 , \field Coefficient5 C5
```

# Object Change: Fuel type synonyms
For Output:Variable, Output:Meter*, Meter:Custom, Meter:CustomDecrement,
Output:Table:Monthly, Output:Table:Annual, Output:Table:TimeBins, EnergyManagementSystem:Sensor, DemandManagerAssignmentList,
ElectricLoadCenter:Distribution, UtilityCost:Tariff and other objects with a Meter name or Output:Variable name as an input field:

Field: Fuel Type (or similar)

9.3.0:

"FuelOil#1"
"FuelOil#2"

"Fuel Oil #1"
"Fuel Oil #2"


9.4.0:

"FuelOilNo1"
"FuelOilNo2"

"Fuel Oil No 1"
"Fuel Oil No 2"

### EnergyManagementSystem:Actuator Actuated Component Control Type

| Component Type     | Component Control Type (v9.3) | Component Control Type (v9.4) |
|--------------------|-------------------------------|-------------------------------|
| Lights             | Electric Power Level          | Electricity Rate              |
| ElectricEquipment  | Electric Power Level          | Electricity Rate              |
| GasEquipment       | Gas Power Level               | NaturalGas Rate               |

See [8304](https://github.com/NREL/EnergyPlus/pull/8304)