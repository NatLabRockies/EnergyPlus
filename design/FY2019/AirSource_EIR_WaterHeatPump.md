# Adding Air Source Capability to the EIR Water-to-Water Heat Pump

**Matt Mitchell, Edwin Lee - NREL**

- Original NFP: August 30, 2019
- Revision Date: N/A

## Justification for New Feature

The EIR-formulated water-to-water heat pump model was added with EnergyPlus 9.1.0. This model allows users to evaluate energy calculations on water source plants where a heat pump provides heating and cooling. The EIR formulation is desired by users as the formulation is accepted as an accurate representation of heat pump operation under common conditions.

The water-to-water model is a welcome addition to the EnergyPlus codebase, however an even further desired feature is to collect multiple water-based components and provide them with heating and cooling from an air-source heat pump. As such, this task will modify the current EIR-formulated heat pump and enable the physics of an air-source heat pump.

## Email and Conference Call Conclusions

N/A

## Overview

Since there is an already-existing EIR-WWHP model, this work should integrated with it rather than creating a new object. The current object is:

- HeatPump:WaterToWater:EIR:(Cooling/Heating)

This model is a water-source heat pump, however, the objective of this adapt the model to be able to operate as an air-source heat pump. The EIR formulation will remain the same as the current model, as shown in the equation.

```CapacityMultiplier(temperature) = a + b*(T_load) + c*(T_load^2) + d*(T_source) + e*(T_source^2) + f*(T_load*T_source)```

This current project will rename this existing object to allow for water source and air source operation. It will also adapt the input structure of the exisiting object so that no new objects are created.

## Approach

- Rename the existing EIR-WWHP from:

  ```HeatPump:WaterToWater:EIR:(Cooling/Heating)```

  to:

  ```HeatPump:PlantLoop:EIR:(Cooling/Heating)```

  "PlantLoop" could also be replaced by suggestions from the reviewers. Perhaps, "WaterSide" or "WaterSideLoad"?

- Implement code changes

- Develop new unit tests

- Develop new input file

## Testing/Validation

- Many of the unit tests for this already exist, however, any changes will be tested with additional unit tests.
- At least 1 new test file will be developed, therefore, integration tests will used to show proper operation.

## I/O Changes

The following examples are the suggested format for the modified EIR-HP object. A ```Condenser Type``` input field has been added. Valid options for this field currenly include ```WaterSource``` and ```AirSource```. All other inputs remain the same.

EIR-WSHP - Cooling/Heating

```
HeatPump:PlantLoop:EIR:Cooling,
  Cooling Coil,            !- Name
  CC Load Inlet Node,      !- Load Side Inlet Node Name
  CC Load Outlet Node,     !- Load Side Outlet Node Name
  WaterSource,             !- Condenser Type
  CC Source Inlet Node,    !- Source Side Inlet Node Name
  CC Source Outlet Node,   !- Source Side Outlet Node Name
  Heating Coil,            !- Companion Heating Heat Pump Name
  0.005,                   !- Load Side Design Volume Flow Rate {m3/s}
  0.003,                   !- Source Side Design Volume Flow Rate {m3/s}
  400000,                  !- Reference Capacity
  3.5,                     !- Reference COP
  ,                        !- Sizing Factor
  CapCurveFuncTempClg,     !- Cooling Capacity Modifier Function of Temperature Curve Name
  EIRCurveFuncTempClg,     !- Electric Input to Cooling Output Ratio Modifier Function of Temperature Curve Name
  EIRCurveFuncPLRClg;      !- Electric Input to Cooling Output Ratio Modifier Function of Part Load Ratio Curve Name
```

```
HeatPump:PlantLoop:EIR:Heating,
  Heating Coil,            !- Name
  HC Load Inlet Node,      !- Load Side Inlet Node Name
  HC Load Outlet Node,     !- Load Side Outlet Node Name
  WaterSource,             !- Evaporator Type
  HC Source Inlet Node,    !- Source Side Inlet Node Name
  HC Source Outlet N0de,   !- Source Side Outlet Node Name
  Cooling Coil,            !- Companion Coil Name
  0.005,                   !- Load Side Design Volume Flow Rate {m3/s}
  0.002,                   !- Source Side Design Volume Flow Rate {m3/s}
  80000,                   !- Reference Capacity
  3.5,                     !- Reference COP
  ,                        !- Sizing Factor
  CapCurveFuncTempHtg,     !- Heating Capacity Function of Temperature Curve Name
  EIRCurveFuncTempHtg,     !- Electric Input to Heating Output Ratio Function of Temperature Curve Name
  EIRCurveFuncPLRHtg;      !- Electric Input to Heating Output Ratio Function of Part Load Ratio Curve Name
```



EIR-ASHP - Cooling/Heating

```
HeatPump:PlantLoop:EIR:Cooling,
  Cooling Coil,            !- Name
  CC Load Inlet Node,      !- Load Side Inlet Node Name
  CC Load Outlet Node,     !- Load Side Outlet Node Name
  AirSource,               !- Condenser Type
  Outdoor Air HP Inlet Node,    !- Source Side Inlet Node Name
  Outdoor Air HP  Outlet Node,  !- Source Side Outlet Node Name
  Heating Coil,            !- Companion Heating Heat Pump Name
  0.005,                   !- Load Side Design Volume Flow Rate {m3/s}
  0.003,                   !- Source Side Design Volume Flow Rate {m3/s}
  400000,                  !- Reference Capacity
  3.5,                     !- Reference COP
  ,                        !- Sizing Factor
  CapCurveFuncTempClg,     !- Cooling Capacity Modifier Function of Temperature Curve Name
  EIRCurveFuncTempClg,     !- Electric Input to Cooling Output Ratio Modifier Function of Temperature Curve Name
  EIRCurveFuncPLRClg;      !- Electric Input to Cooling Output Ratio Modifier Function of Part Load Ratio Curve Name
```

```
HeatPump:PlantLoop:EIR:Heating,
  Heating Coil,            !- Name
  HC Load Inlet Node,      !- Load Side Inlet Node Name
  HC Load Outlet Node,     !- Load Side Outlet Node Name
  AirSource,               !- Evaporator Type
  Outdoor Air HP Inlet Node,    !- Source Side Inlet Node Name
  Outdoor Air HP  Outlet Node,  !- Source Side Outlet Node Name
  Cooling Coil,            !- Companion Coil Name
  0.005,                   !- Load Side Design Volume Flow Rate {m3/s}
  0.002,                   !- Source Side Design Volume Flow Rate {m3/s}
  80000,                   !- Reference Capacity
  3.5,                     !- Reference COP
  ,                        !- Sizing Factor
  CapCurveFuncTempHtg,     !- Heating Capacity Function of Temperature Curve Name
  EIRCurveFuncTempHtg,     !- Electric Input to Heating Output Ratio Function of Temperature Curve Name
  EIRCurveFuncPLRHtg;      !- Electric Input to Heating Output Ratio Function of Part Load Ratio Curve Name
```



## I/O Documentation

Documentation will be updated to add a description of the new condenser type field. Additional examples will be given that show application of the ASHP options. Output field will be unchanged.

## Code Changes

It is expected that the code changes will be primarily in the EIR Water to Water Heat Pump Module. These code changes will largely affect the input/setup routines. Computations are expected to be nearly identical, except for the differences in how water/glycol vs. air properties are determined.
There will be other stray code changes in plant management routines to instantiate and manage simulation of the newly modified component.

## Example Files

A new example file will be generated to demonstrate usage of the new EIR-ASHP.

## Transition

A transition rule will be written to insert the ```Condenser Type``` field into existing objects.

## Engineering Reference

No changes expected, aside from any minor changes requried to explain the water- vs. air-source operation.



