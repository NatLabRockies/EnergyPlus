# Add More Coil/WaterHeater Choices for Coil:WaterHeating:Desuperheater


**Yueyue Zhou**

**National Renewable Energy Laboratory**

**April 2, 2019**


## Justification for New Feature ##

Desuperheater is a widely used new technology making use of waste heat from HVAC system to heat domestic hot water. Desuperheater is available in E+ only supporting
a very limited amount of HVAC systems and water heater tanks, while the most widely used occasion for desuperheater: ground source heat pump system is not yet available.
 Given only 8 valid choices of cooling and refrigeration components and only 1 tank option, it constrained the variations people modeling their hybrid systems and impeded
 the promotions of using this new energy efficiency techonology. Users and interface developers are requesting EnergyPlus broaden its capability of modeling desuperheater
 with more coils and tanks options.Some other energy modeling  software tools like Rem/Rate, Ekotrope, Wrightsoft are already capable to model this hybrid GSHP system with
 simple assumptions and methods. Since EnergyPlus already supports some systems, it would be more straight-forward to add more enumarations to EnergyPlus.


## Overview ##

EnergyPlus would be modified to add more enumarations in Coil:WaterHeating:Desuperheater object's heating source object type and tank object type fields.

## Approach ##

EnergyPlus already handles HVAC/water heating hybrid systems for several coil types. The workflow is straight-forward to add the same structure as those systems. The steps to new enumarations are given below.


1. IDD file modification: **Coil:WaterHeating:Desuperheater** A7 add **WaterHeater:Stratified** ; A9 add **Coil:Cooling:WaterToAirHeatPump:EquationFit**,
**Coil:Cooling:WaterToAirHeatPump:ParameterEstimation**, **Coil:Cooling:DX:MultiSpeed** , etc.

2. Available reclaimed heat data passing: The available waste heat, heat source name and type data is spread across three namespaces (DataHeatBalance, WaterThermalTank, and the corresponding
HVAC coil namespace). Arrays and structures added to pass data. The same method that the quantity of available heat is delivered from HVAC loop while doesn't impact compressor performance by limiting its
reclaiming efficiency factor (would be discussed if the assumption is acceptable).
=======
1. IDD file modification: See below the entire list of **Coil:WaterHeating:Desuperheater** modified field:

```
Coil:WaterHeating:Desuperheater,
  A7 , \field Tank Object Type
       \type choice
       \key WaterHeater:Mixed
       [Add]\key WaterHeater:Stratified
       \default WaterHeater:Mixed
       \note Specify the type of water heater tank used by this desuperheater water heating coil.

  A9 , \field Heating Source Object Type
       \required-field
       \type choice
       \key Coil:Cooling:DX:SingleSpeed
       \key Coil:Cooling:DX:TwoSpeed
       \key Coil:Cooling:DX:TwoStageWithHumidityControlMode
       \key Coil:Cooling:DX:VariableSpeed
       [Add]\key Coil:Cooling:WaterToAirHeatPump:EquationFit
       [Add]\key Coil:Cooling:WaterToAirHeatPump:ParameterEstimation
       [Add]\key Coil:Cooling:DX:MultiSpeed
       [Add]\key ......
       \key Refrigeration:CompressorRack
       \key Refrigeration:Condenser:AirCooled
       \key Refrigeration:Condenser:EvaporativeCooled
       \key Refrigeration:Condenser:WaterCooled
       \note The type of DX system that is providing waste heat for reclaim.
```


2. Available reclaimed heat data passing: The available waste heat, heat source name and type data is spread across three namespaces (DataHeatBalance, WaterThermalTank, and the corresponding
HVAC coil namespace). Arrays and structures added to pass data. The heat relcaim from water cooled HVAC system would
cause decreasing of the heat transferred to the condenser plant loop thus impact the HVAC performance. Therefore, the
reclaimed heat would be passed back to HVAC water coil namespace and be substracted from the total source heat in the loop to reflect the impact.


3. Reclaimed heat calculation: The calculation of reclaimed heat is coded in WaterThermalTank namespace, the same approach as other available systems for mixed water heater tank.


## Testing/Validation/Data Sources ##

Existing EnergyPlus test files would be modified to produce models that utilizes ground source heat pump systems, stratified tanks, and other types of coils that are widely used in industry.

## Input Output Reference Documentation ##

The documentation of **Coil:WaterHeating:Desuperheater** inputs will be modified to reflect the new enumarations.

## Input Description ##

**Field: Heating Source Object Type**

Would add valid enumarations for water-to-air cooling coils etc.

**Field: Tank Object Type**

Would add stratified water heater tank.

## Outputs Description ##

No need to add more outputs

## Engineering Reference ##

Engineering reference would be checked to make sure it documents all the options

## Example File and Transition Changes ##

New enumarations would be enabled in object **Coil:WaterHeating:Desuperheater**

## References ##

Unmet hour unsolved question link:
https://unmethours.com/question/19819/modeling-water-to-air-heat-pump-with-desuperheater-for-dhw/
