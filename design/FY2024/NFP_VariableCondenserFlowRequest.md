Variable Condenser Flow Request
=================================

**Jeremy Lerond, Pacific Northwest National Laboratory**


## Justification for New Feature ##

Issue [#7111](https://github.com/NatLabRockies/EnergyPlus/issues/7111)
- "User file with Chiller:Electric:EIR always requests full condenser flow rate"
- "This is a problem not only when using cooling towers, but also when using seawater cooling by PlantComponent:TemperatureSource. Unlike cooling towers, it is not possible to make a compromise of variable tower fans instead of variable condenser water."

## E-mail and  Conference Call Conclusions ##

Notes from the 5/15/2024 technicalities call:
- TRANE trace scales the condenser flow request based on the chiller PLR
- The new chiller input is different than the existing chiller flow mode input (evaporator side)
- Implement the new inputs and logic for both the `Chiller:Electric:EIR` and `Chiller:Electric:ReformulatedEIR`

## Overview ##

Chilled water plant designs can include variable flow condenser requests from chillers. Currently, the `Chiller:Electric:EIR` and `Chiller:Electric:ReformulatedEIR` objects request the maximum condenser flow rate every time a chiller is active. This new feature will implement new control strategies to allow users to model variable flow condenser requests from chillers.

## Approach ##

Three new control approaches will be implemented:
1. When used in conjunction with a cooling tower, users will be able to use the approach from the _Fundamentals of Design and Control of Central Chilled-Water Plants_ which correlates the condenser water flow ratio to the chilled water plant's PLR as follows: `CWFR = C . PLR + D`, where `CWFR` is the condenser water flow ration (actual/design), `C` and `D` are user specified coefficients (for proposed values see _Optimizing Design & Control Of Chilled Water Plants, Part 5_, S. Taylor, ASHRAE Journal June 2012) and `PLR` is the chilled water plant loop part load ratio (actual/design). The condenser loop flow would be calculated as `m_dot_CW = CWFR * m_dot_CW_design`. Because the request comes from the chillers, the chiller request will be determined as follows: `m_dot_CW_chiller = m_dot_CW * Q_chiller / Q_CHW`. Where `Q_chiller` is the chiller load and `Q_CHW` is the chilled water loop load. So if a chilled water plant loop has two chillers, then `Q_CHW = Q_chiller_1 + Q_chiller_2`.
2. When no cooling tower are used, users will be able to specify that the condenser water flow request will be determined as follows: `m_dot_CW_chiuller = Q_condenser / (C_p .dT)` where dT is the delta T across the condenser.
3. A simplified approach to 1. will also be implemented which will set the request condenser flow rate for each chiller to be `m_dot_CW_chiller = PLR_chiller * m_dot_max_condenser_chiller`.

## Testing/Validation/Data Sources ##

Unit tests will be added to confirm the correct implementation of these control strategies.

## Input Output Reference Documentation ##

A description of the new field will be added to the input output reference manual.

## Input Description ##

```
Chiller:Electric:EIR,
\min-fields 23
   \memo This chiller model is the empirical model from the DOE-2 building Energy
   \memo simulation program. Chiller performance at off-reference conditions is modeled
   \memo using three polynomial equations. Three curves objects are required.
[...]
A16, \field End-Use Subcategory
     \note Any text may be used here to categorize the end-uses in the ABUPS End Uses by Subcategory table.
     \type alpha
     \retaincase
     \default General
A17, \field Condenser Flow Control
     \note Select the chiller condenser flow request mode. With "ConstantFlow" a chiller will always request
     \note its maximum condenser flow rate. With "ModulatedChillerPLR" the condenser flow request corresponds
     \note to the chiller part load ratio multiplied by the chiller maximum condenser flow rate.  With
     \note "ModulatedLoopPLR" the chiller will request a flow rate that is function of the chilled water
     \note loop's part load ratio, see the "Condenser Loop Flow Rate Fraction Function of Loop Part Load Ratio
     \note Curve Name" input. With "ModulatedDeltaTemperature" the chiller will request the flow rate required
     \note to meet a condenser loop delta temperature, see the "Temperature Difference Across Condenser"  and
     \note "Temperature Difference Across Condenser Schedule Name" input.
     \note Use "ConstantFlow" when modeling a constant flow condenser plant loop, choose one of the other inputs
     \note when modeling a variable flow condenser plant loop.
     \key ConstantFlow
     \key ModulatedChillerPLR
     \key ModulatedLoopPLR
     \key ModulatedDeltaTemperature
     \default ConstantFlow
A18, \field Condenser Loop Flow Rate Fraction Function of Loop Part Load Ratio Curve Name
     \note Condenser loop flow rate fraction as a function of the chiller water loop part load ratio. A linear
     \note curve is expected.
     \note CWFR = C * PLR + D
     \note Where:
     \note CWFR is the condenser water flow fraction (actual/design)
     \note C and D are coefficients, see "Optimizing Design & Control Of Chilled Water Plants, Part 5",
     \note S. Taylor, ASHRAE Journal June 2012 PLR is the chilled water plant loop part load ratio
     \note (actual/design). This input is only used when the "ModulatedLoopPLR" condenser flow control
     \note option is used.
     \type object-list
     \object-list UnivariateFunctions
A20, \field Temperature Difference Across Condenser Schedule Name
     \note A schedule that defines the temperature difference across the condenser. This input is used to
     \note calculate the condenser flow. This input is only used when "Condenser Flow Control" is set to
     \note "ModulatedDeltaTemperature".
     \type object-list
     \object-list ScheduleNames
N19; \field Condenser Minimum Flow Fraction
     \note This input corresponds to the minimum flow fraction to be simulated. The minimum condenser flow
     \note corresponds to this fraction multiplied by the maximum condenser flow rate. This input is only used
     \note when the "Condenser Flow Control" input is set to "ModulatedChillerPLR", "ModulatedLoopPLR" or
     \note "ModulatedDeltaTemperature".
     \type real
     \minimum 0.0
     \maximum 1.0
     \default 0.2
```

Similar input will be implemented for the `Chiller:Electric:ReformulatedEIR` object.

## Outputs Description ##

No new output will be added.

## Engineering Reference ##

A new section will be added to the Chiller section of the Engineering Reference to provide details on the control approach as shown in the Approach section of this document.

A note will be added indicating that chiller performance will be only impacted when simulating a `Chiller:Electric:ReformulatedEIR` since this chiller object has its part load performance impacted by the leaving chiller condenser temperature.

## Example File and Transition Changes ##

No transition will be necessary.

New example files will be provided to showcase the new feature.

## References ##

- "Optimizing Design & Control Of Chilled Water Plants, Part 2", S. Taylor, ASHRAE Journal Sept 2011
- "Optimizing Design & Control Of Chilled Water Plants, Part 5", S. Taylor, ASHRAE Journal June 2012
- "Optimizing Chilled Water Plan Control", M. Hydeman, ASHRAE Journal June 2007
- "Carrier® ChillerVu™ Variable Flow Condenser Pump Application Guide", https://www.shareddocs.com/hvac/docs/1000/Public/0A/11-808-577-01.pdf
- "Achieving Variable Condenser Water Flow with VFDs" https://www.esmagazine.com/articles/99689-achieving-variable-condenser-water-flow-with-vfds
