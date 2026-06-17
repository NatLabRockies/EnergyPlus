Other Fuel Heating Coils
========================

**Noel Merket, NREL**

 - Original Date: 9 Aug 2016
 - Revision Date: 12 Aug 2016


## Justification for New Feature ##

Similar to [#5746](https://github.com/NatLabRockies/EnergyPlus/issues/5746) , furnaces currently only support gas or electric heating coils. Propane and fuel oil are commonly used fuel types, especially for residential models. This is similar to [#5656](https://github.com/NatLabRockies/EnergyPlus/issues/5656) / [#5746](https://github.com/NatLabRockies/EnergyPlus/issues/5746) which added a fuel type field to `OtherEquipment`.

## E-mail and  Conference Call Conclusions ##

See GitHub issue [#5752](https://github.com/NatLabRockies/EnergyPlus/issues/5752).

## Overview ##

I propose adding a user selectable fuel field to the `Coil:Heating:Gas` object and renaming it to something more appropriate.

## Approach ##

Modify the code in EnergyPlus that handles `Coil:Heating:Gas` objects to appropriately set the fuel type for reporting and metering depending on which field type was chosen.

## Testing/Validation/Data Sources ##

I will use the existing EnergyPlus test files and modify a few of them to use the new object and see if the results come out as expected.

## Input Output Reference Documentation ##

See the latex source on this branch for proposed documentation changes.

## Engineering Reference ##

See the latex source on this branch for proposed documentation changes.

## Example File and Transition Changes ##

I will create an example file based on some of the other example files with this new object.

Transition will include renaming the `Coil:Heating:Gas` object and adding the fuel type field, moving the other fields down.

## References ##

n/a
