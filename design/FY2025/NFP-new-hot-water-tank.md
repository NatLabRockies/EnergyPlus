TITLE OF FEATURE
================

**Yujie Xu, Tianzhen Hong, LBNL**

 - Original Date: March 2025
 - Revision Date: April 2025

## Justification for New Feature ##

Currently, EnergyPlus does not have a direct or explicit way to model passive
hot water tanks , which stores hot water in a way that naturally maintains
temperature stratification without the use of active mixing or heating elements
(see Figure 1). Users have used the heat pump water heater, WaterHeater:Mixed
and WaterHeater:Stratified with tricks and limitations. This hinders the ability
of users to model complex thermal energy storage systems.

<img src="hotWaterTank_Figure 1.png" alt="awhp-figure-1" width="800px">
</img>
Figure 1. Schematic diagram of a stratified passive hot water tank

Key drawbacks of the existing tank object include,
- Ambiguity in Control: Due to the dual purpose of the water heater and water
  storage tank, it is sometimes unclear whether the setpoint temperature
  controls the source side flow request or the internal heater operation.
- Fixed Flow Direction: In complex HVAC systems, such as the Time Independent
  Energy Recovery (TIER) system defined in [1] defined in , the water flow
  direction may change. For example, in the heating mode, water flows in at the
  bottom and exits at the top of the tank, whereas in the cooling mode, water
  flows in at the top and exits at the bottom. The current object has fixed
  inlet and outlet water heights, thereby only allowing a fixed flow direction.
  In contrast, the water storage tank implemented in Modelica is more flexible
  [2], with the flow direction indicated by the sign of the flow. Additionally,
  temperature sensors can be placed in all layers (at different heights) of the
  stratified tank, providing information for more complex control logic.
- Simplified Charging Logic: The current object determines tank charges (source
  side requesting water flow) based on one temperature setpoint (default) or a
  temperature upper limit (storage tank mode). However, more complex charging
  control is sometimes needed. For example, in some applications, the charging
  percentage needs to be a function of the top and bottom of the tank temperature, 
  for example, a common logic of charging could be as follows
  - if the bottom temperature < setpoint at the bottom, request flow. 
  - if the top temperature > setpoint at the top, stop request flow
  The current object does not allow for such complex charging percentage 
  calculations nor does it permit control of this parameter via EMS or Python 
  plug-ins.
- **Issue with autosizing**: Tank autosizing capability is present but with
  multiple issues (issue 8412, 8451, 9956). Actuators might need to be added to
  enable auto-sizing with external code (EMS or python API), especially for
  complex systems with heat pumps, heat recovery chillers, and thermal storage
  tanks.

The proposed new feature will introduce a passive hot water tank, similar to the
chilled water tank, but designed to hold hot water. It will also include various
actuators to control flow direction and tank charging. The autosizing
functionality will be thoroughly tested to ensure it functions properly. This
new tank can be incorporated into complex energy recovery systems like the TIER
system. Note that the tank will charge and discharge via direct fluid change,
unlike the ice or PCM tanks via heat exchanger coils.


## Overview ##

### EnergyPlus existing tank objects ###

There are six types of thermal storage objects available for ice, chilled water,
and hot water. Additionally, a new object (ThermalStorage:PCM) is currently
under development. Hot water storage is modeled using the WaterHeater:Mixed or
WaterHeater:Stratified objects by setting the heater capacity to zero. This
approach unnecessarily complicates the storage tank, adding 20 more fields
compared to the chilled water storage tanks (Table 1, Table 2).

Table 1. Thermal storage objects in EnergyPlus

|object|Media| \# fields| Volume autosizable |
|----|----|----|----|
|ThermalStorage:Ice:Simple | ice | 5 | No|
| ThermalStorage:Ice:Detailed | Ice | 15 | No|
| ThermalStorage:ChilledWater:Mixed | Chilled water | 22 | No|
| ThermalStorage:ChilledWater:Stratified | Chilled water | 43 | No|
| WaterHeater:Mixed | Hot water | 42 | Yes|
| WaterHeater:Stratified | Hot water | 67 | Yes|

Table 2. Comparison of input and output fields of the hot and water stratified tank
<img src="hotWaterTank_Table 2.png" alt="awhp-figure-1" width="800px">

Table 3 and Table 4 compares the input and output fields among the
ThermalStorage:* objects. The chilled water stratified tank has inputs in most
input categories except for the charging related ones, where the ice storage
tank has more information. The new hot water tank will include setpoints at both 
the top and bottom of the tank to more flexibly control the tank charging. 
This enhancement can facilitate a better model of more complex tank
charging/discharging control.

Table 3. Comparison of input fields of thermal storage tanks
<img src="hotWaterTank_Table 3-1.png" alt="awhp-figure-1" width="800px">

<img src="hotWaterTank_Table 3-2.png" alt="awhp-figure-1" width="800px">

Table 4. Comparison of output fields of thermal storage tanks
<img src="hotWaterTank_Table 4.png" alt="awhp-figure-1" width="800px">

The ThermalStorage:* object all have the Plant Component ThermalStorage:* on/off
supervisory actuator that can switch on/off the flow of the tank object (as well
as items on the same branch). The ThermalStorage:Ice:* objects have additionally
the following actuators related to the inlet/outlet node setpoints.

- <TANK NAME>,INLET NODE,System Node Setpoint,Temperature Setpoint,[C]
- <TANK NAME>,INLET NODE,System Node Setpoint,Temperature Minimum Setpoint,[C]
- <TANK NAME>,INLET NODE,System Node Setpoint,Temperature Maximum Setpoint,[C]
- <TANK NAME>,INLET NODE,System Node Setpoint,Humidity Ratio Setpoint,[kgWater/kgDryAir]
- <TANK NAME>,INLET NODE,System Node Setpoint,Humidity Ratio Maximum Setpoint,[kgWater/kgDryAir]
- <TANK NAME>,INLET NODE,System Node Setpoint,Humidity Ratio Minimum Setpoint,[kgWater/kgDryAir]
- <TANK NAME>,INLET NODE,System Node Setpoint,Mass Flow Rate Setpoint,[kg/s]
- <TANK NAME>,INLET NODE,System Node Setpoint,Mass Flow Rate Maximum Available Setpoint,[kg/s]
- <TANK NAME>,INLET NODE,System Node Setpoint,Mass Flow Rate Minimum Available Setpoint,[kg/s]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Temperature Setpoint,[C]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Temperature Minimum Setpoint,[C]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Temperature Maximum Setpoint,[C]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Humidity Ratio Setpoint,[kgWater/kgDryAir]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Humidity Ratio Maximum Setpoint,[kgWater/kgDryAir]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Humidity Ratio Minimum Setpoint,[kgWater/kgDryAir]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Mass Flow Rate Setpoint,[kg/s]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Mass Flow Rate Maximum Available Setpoint,[kg/s]
- <TANK NAME>,OUTLET NODE,System Node Setpoint,Mass Flow Rate Minimum Available Setpoint,[kg/s]

## Approach ##

### The new IDD object ###

A passive hot water storage tank object will be added as follows.

    ThermalStorage:HotWater:Stratified,
    A1 , \field Name
        \required-field
        \type alpha
        \reference-class-name validPlantEquipmentTypes
        \reference validPlantEquipmentNames
        \reference-class-name validCondenserEquipmentTypes
        \reference validCondenserEquipmentNames
        \reference-class-name validBranchEquipmentTypes
        \reference validBranchEquipmentNames
    N1 , \field Tank Volume
        \required-field
        \type real
        \units m3
        \minimum> 0.0
        \ip-units gal
    N2 , \field Tank Height
        \required-field
        \type real
        \units m
        \minimum> 0.0
        \note Height is measured in the axial direction for horizontal cylinders
    A2 , \field Tank Shape
        \type choice
        \key VerticalCylinder
        \key HorizontalCylinder
        \key Other
        \default VerticalCylinder
    N3 , \field Tank Perimeter
        \type real
        \units m
        \minimum 0.0
        \note Only used if Tank Shape is Other
    A3 , \field Top Setpoint Temperature Schedule Name
        \type object-list
        \object-list ScheduleNames
    A4 , \field Bottom Setpoint Temperature Schedule Name
        \type object-list
        \object-list ScheduleNames
    N4 , \field Deadband Temperature Difference
        \type real
        \units deltaC
        \minimum 0.0
        \default 0.0
    N5,  \field Top Temperature Sensor Height
        \units m
        \minimum 0.0
    N6,  \field Bottom Temperature Sensor Height
        \units m
        \minimum 0.0
    N7 , \field Maximum Temperature Limit
        \type real
        \units C
    N8 , \field Nominal Heating Capacity
        \type real
        \units W
    A5 , \field Ambient Temperature Indicator
        \required-field
        \type choice
        \key Schedule
        \key Zone
        \key Outdoors
    A6 , \field Ambient Temperature Schedule Name
        \type object-list
        \object-list ScheduleNames
    A7 , \field Ambient Temperature Zone Name
        \type object-list
        \object-list ZoneNames
    A8, \field Ambient Temperature Outdoor Air Node Name
        \type node
        \note required for Ambient Temperature Indicator=Outdoors
    N9 , \field Uniform Skin Loss Coefficient per Unit Area to Ambient Temperature
        \type real
        \units W/m2-K
        \minimum 0.0
    A9, \field Use Side Inlet Node Name
        \type node
    A10, \field Use Side Outlet Node Name
        \type node
    A11, Use Side Flow Direction Schedule
        \note allowed value is -1 and 1. When the value is 1, water flows in from
        \note the inlet and flows out from the outlet node. When the value is -1, water
        \note flows in from the outlet node and flows out from the inlet node (i.e. the
        \note flow direction is reversed)
    N10, \field Use Side Heat Transfer Effectiveness
        \type real
        \minimum 0.0
        \maximum 1.0
        \default 1.0
        \note The use side effectiveness in the stratified tank model is a simplified analogy of
        \note a heat exchanger's effectiveness. This effectiveness is equal to the fraction of
        \note use mass flow rate that directly mixes with the tank fluid. And one minus the
        \note effectiveness is the fraction that bypasses the tank. The use side mass flow rate
        \note that bypasses the tank is mixed with the fluid or water leaving the stratified tank.
    A12, \field Use Side Availability Schedule Name
        \note Availability schedule name for use side. Schedule value > 0 means the system is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
    N11 , \field Use Side Inlet Height
        \type real
        \units m
        \minimum 0.0
        \autocalculatable
        \default autocalculate
        \note Defaults to top of tank
    N12, \field Use Side Outlet Height
        \type real
        \units m
        \minimum 0.0
        \default 0.0
        \note Defaults to bottom of tank
    N13, \field Use Side Design Flow Rate
        \type real
        \autosizable
        \default autosize
        \units m3/s
        \ip-units gal/min
        \minimum 0.0
    A13, \field Source Side Inlet Node Name
        \type node
    A14, \field Source Side Outlet Node Name
        \type node
    A15, Source Side Flow Direction Schedule
        \type alpha
        \note allowed value is -1 and 1. When the value is 1, water flows in from
        \note the inlet and flows out from the outlet node. When the value is -1, water
        \note flows in from the outlet node and flows out from the inlet node (i.e. the
        \note flow direction is reversed)
    N14, \field Source Side Heat Transfer Effectiveness
        \type real
        \minimum> 0.0
        \maximum 1.0
        \default 1.0
        \note The source side effectiveness in the stratified tank model is a simplified analogy of
        \note a heat exchanger's effectiveness. This effectiveness is equal to the fraction of
        \note source mass flow rate that directly mixes with the tank fluid. And one minus the
        \note effectiveness is the fraction that bypasses the tank. The source side mass flow rate
        \note that bypasses the tank is mixed with the fluid or water leaving the stratified tank.
    A16, \field Source Side Availability Schedule Name
        \note Availability schedule name for use side. Schedule value > 0 means the system is available.
        \note If this field is blank, the system is always available.
        \type object-list
        \object-list ScheduleNames
    N15, \field Source Side Inlet Height
        \type real
        \units m
        \minimum 0.0
        \default 0.0
        \note Defaults to bottom of tank
    N16, \field Source Side Outlet Height
        \type real
        \units m
        \minimum 0.0
        \autocalculatable
        \default autocalculate
        \note Defaults to top of tank
    N17, \field Source Side Design Flow Rate
        \type real
        \autosizable
        \default autosize
        \units m3/s
        \ip-units gal/min
        \minimum 0.0
    N18, \field Tank Recovery Time
        \type real
        \default 4.0
        \note Parameter for autosizing design flow rates for indirectly cooled water tanks
        \note time required to lower temperature of entire tank from 14.4C to 9.0C
        \units hr
        \minimum> 0.0
    A17, \field Inlet Mode
        \type choice
        \key Fixed
        \key Seeking
        \default Fixed
    N19, \field Number of Nodes
        \type integer
        \minimum 1
        \maximum 10
        \default 1
    N20, \field Additional Destratification Conductivity
        \type real
        \units W/m-K
        \minimum 0.0
        \default 0.0
    N21, \field Node 1 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N22, \field Node 2 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N23, \field Node 3 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N24, \field Node 4 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N25, \field Node 5 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N26, \field Node 6 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N27, \field Node 7 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N28, \field Node 8 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N29, \field Node 9 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0
    N30; \field Node 10 Additional Loss Coefficient
        \type real
        \units W/K
        \default 0.0

### Actuators ###

The following list of actuators will be added to allow for more complex controls.

- Charging percent: specifies how full the tank is charged. It can be a function
  of all temperature nodes in the stratified tank. If this is defined, then the
  tank control is based on this quantity rather than the temperature set points 
  at the top and bottom of the tank.
- Node k temperature: the temperature at the kth stratified node. "k" ranges
  from 1 to the number of nodes.
- Sizing-related actuators
    - Peak load or load profile
    - Charging and discharge rate (source and use side flow rate)
    - Ambient temperature
    - Desired temperature range

### Output ###

The following output variables will be made available

- Hot Water Thermal Storage Tank Temperature
- Hot Water Thermal Storage Final Tank Temperature
- Hot Water Thermal Storage Tank Heat Gain Rate
- Hot Water Thermal Storage Tank Heat Gain Energy
- Hot Water Thermal Storage Use Side Mass Flow Rate
- Hot Water Thermal Storage Use Side Inlet Temperature
- Hot Water Thermal Storage Use Side Outlet Temperature
- Hot Water Thermal Storage Use Side Heat Transfer Rate
- Hot Water Thermal Storage Use Side Heat Transfer Energy
- Hot Water Thermal Storage Source Side Mass Flow Rate
- Hot Water Thermal Storage Source Side Inlet Temperature
- Hot Water Thermal Storage Source Side Outlet Temperature
- Hot Water Thermal Storage Source Side Heat Transfer Rate
- Hot Water Thermal Storage Source Side Heat Transfer Energy
- Hot Water Thermal Storage Temperature Node <1 - 10>
- Hot Water Thermal Storage Final Temperature Node <1 -- 10>

## Testing/Validation/Data Sources ##

The feature will be tested and demonstrated with a test file derived from the PlantApplicationsGuide_Example2.idf

## References ##

[1] Gill, B. (2021). Solving the Large Building All-Electric Heating Problem. ASHRAE Journal, 63(10).
[2] Lawrence Berkeley National Laboratory. (n.d.). Buildings.Fluid.Storage.UsersGuide. Simulation Research Group. Retrieved [2025-04-11], from https://simulationresearch.lbl.gov/modelica/releases/v10.0.0/help/Buildings_Fluid_Storage_UsersGuide.html#Buildings.Fluid.Storage.UsersGuide

