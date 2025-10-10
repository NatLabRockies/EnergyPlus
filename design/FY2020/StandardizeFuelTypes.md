Standardize Fuel Types and Resources (Issue #5941)
================

**M.J. Witte, GARD Analytics**

 - Original 01 Oct 2019
 - Revision Date


## Background ##

Excerpts lifted from [issue #5941](https://github.com/NREL/EnergyPlus/issues/5941)

 - This issue was prompted by the discussion of Propane vs PropaneGas as a fuel type in [#5940](https://github.com/NREL/EnergyPlus/pull/5940),
which change Coil:Heating:Gas to Coil:Heating:Fuel.

 - At that time, the concensus was that Propane is preferred over PropaneGas.

 - But many existing objects use PropaneGas instead.

 - NaturalGas is consistently used as a fuel type and resource type everywhere, except Coil:Heating:Fuel lists both Gas and NaturalGas as choices.

 - But in output, Gas is used for meters and output variable names rather than NaturalGas or Natural Gas.

 - Most other fuel type choices are consistent across all object types.

 - But it has been suggested that we drop the # symbol from FuelOil#1 and FuelOil#2

  - Propane is currently an idd choice for Fuel Type in Coil:Heating:Fuel, Coil:Cooling:DX:MultiSpeed
  Coil:Heating:DX:MultiSpeed

  - Propane is also an idd choice for Resource Type in EnergyManagementSystem:MeteredOutputVariable,
  LifeCycleCost:UsePriceEscalation, LifeCycleCost:UseAdjustment, FuelFactors

  - Propane is also an idd choice for Constituent Name in Generator:FuelSupply
searches of the code and IDD show that there are some variations in the list of valid fuel types/resource types in the IDD, input processing functions and the documentation.  This should be cleaned up to a single standard list.

## E-mail and  Conference Call Conclusions ##

*None yet.*

## Proposed Standard List of Fuel/Resource Types ##
Electricity
NaturalGas
Propane (preferred over PropaneGas per discussions in [#5940](https://github.com/NREL/EnergyPlus/pull/5940)
Diesel
Gasoline
FuelOil#1 (or FuelOil1)
FuelOil#2 (or FuelOil2)
Coal
OtherFuel1
OtherFuel2

*Note there are many other resource types which are not fuels, such as Water, DistrictHeat, SO2, etc.) These will remain untouched here.*

## 3 Decisions to Make ##

### Replace "PropaneGas" with "Propane" In Input Choices? ###

 - 25 objects currently use PropaneGas

 - 7 objects currently use Propane

 - Propane is currently used in output variable names and meter resource types

### Replace "Gas" with "NaturalGas" in Meters and Output Variable Names? ###

 - 1 object currently accepts both Gas and NaturalGas as a fuel type

 - All other objects consistently accept NaturalGas as an input

 - Gas is currently used in output variable names and meter resource types

### Replace "FuelOil#"" with "FuelOil" in Input Choices, Meters and Output Variable Names? ###

 - All objects currently accept FuelOil#1 and FuelOil#2 as a fuel type or resource type

 - FuelOil#1 and FuelOil#2 are currently used in output variable names and meter resource types

 - The # was originally used, because "#1  fuel oil" is a commonly used designation

 - In constrast, OtherFuel1 and OtherFuel2 do not have the #.

## Approach ##

Once the above decisions are made:

 - A single function will be added to validate fuel types on input instead of the many if blocks that are
 scattered throughout the code. There are a couple of existing functions which may be useful with modifications.

 - The list of fuel types and corresponding strings will be build with enums similar to the way output units
 are currently.

 - Remove the special JSON schema modifications section in [modify_schema.py](https://github.com/NREL/EnergyPlus/blob/60ed7207c2f3058fe8dc400a1aa741149094bebe/scripts/dev/generate_epJSON_schema/modify_schema.py#L267-L297)
 which allows any old fuel type synonyms to pass through and be valid.


## Testing/Validation/Data Sources ##

There's potentially going to be many string diffs, but numbers should all remain the same.

## Input Output Reference Documentation ##

There's potentially many doc changes required.

## Example File and Transition Changes ##

And transition will be required.

 - For any changed standard fuel types.

 - For all of the old synonyms that are still valid.
