An Air-to-Water Heat Pump Model
================

**Yujie Xu, Tianzhen Hong**

**Lawrence Berkeley National Laboratory***

 - Original Date: February, 2025
 - Final Date: March, 2025

## Justification for a feature update ##

Heat pumps (HP) are an essential technology for decarbonizing buildings.
EnergyPlus is capable of modeling several types of heat pumps, e.g., for space
cooling or heating: air to air, air to water, water to water; and heat pump
water heaters. Although there is an existing feature to model air-to-water heat
pumps in EnergyPlus, its capability is limited and its implementation is not
straightforward compared with other types of HP. Some of the limitations
include: Cannot specify parameters like single, multi-speed, or variable speed
for capacity control (for example, the Daikin model EWYT-CZ series is inverter
controlled, and the EWYQ series is a fixed-speed two-stage model.) Missing
details of crankcase heater control and more flexible defrost control The naming
of the object is not intuitive - it does not refer to water to water or air to
water, making searching for the object difficult Missing the input of an
availability schedule (which disables the HP for seasonal consideration)
Designed as separate heating and cooling objects while in reality, it should be
one piece of equipment

This feature will create a new air-to-water heat pump (AWHP) model that includes
heating and cooling components in one single object, capable of multi-speed
controls, and includes more equipment details such as fan characteristics, PLR
ranges, availability schedules, etc.

## Overview ##

### Introduction ###

Air-to-water heat pumps are becoming increasingly popular in the U.S., with an
expected annual growth of 15.5% in market share from 2023 to 2032 [1]. The
following are some commercial AWHP products from one manufacturer. The AWHP can
provide heating, cooling, and hot water at up to 60C (140F). The unit has
variable speed control (inverter).

<img src="awhp-figure-1.png" alt="awhp-figure-1" width="800px">
</img>

Figure 1. Schematic diagram of an AWHP system

<img src="awhp-table-1-1.png" alt="awhp-table-1-1" width="800px">
</img>
<img src="awhp-table-1-2.png" alt="awhp-table-1-2" width="800px">
</img>

Table 1. A sample air-to-water heat pump product from Daikin (EWYT-CZ series) [2].

### Existing approaches ###

Currently, EnergyPlus uses a pair of objects, HeatPump:PlantLoop:EIR:Heating and
HeatPump:PlantLoop:EIR:Cooling, with "Condenser Type" set to "AirSource", to
model air-to-water heat pump as part of a central plant cooling and/or heating
loop, which traditionally includes chillers to produce chilled water and a
boiler to produce hot water. The following is an example of the current object.
The object specifies capacity and EIR as functions of temperature and part load
ratio, with three performance curves.

Example:

    HeatPump:PlantLoop:EIR:Heating,
      Heating Coil,        	!- Name
      Heating Coil Load Loop Intermediate Node,  !- Load Side Inlet Node Name
      Heating Coil Load Loop Supply Outlet Node,  !- Load Side Outlet Node Name
      AirSource,         	! Condenser Type
      Outdoor Air Inlet Node,  !- Source Side Inlet Node Name
      Outdoor Air Outlet Node, !- Source Side Outlet Node Name
      Cooling Coil,        	!- Companion Heat Pump Name
      0.005,               	!- Load Side Design Volume Flow Rate {m3/s}
      0.002,               	!- Source Side Design Volume Flow Rate {m3/s}
      80000,               	!- Reference Capacity
      3.5,                 	!- Reference COP
      ,                    	!- Sizing Factor
      CapCurveFuncTemp,    	!- Capacity Modifier Function of Temperature Curve Name
      EIRCurveFuncTemp,  !- Electric Input to Heating Output Ratio Modifier Function of Temperature Curve Name
      EIRCurveFuncPLR;    !- Electric Input to Heating Output Ratio Modifier Function of Part Load Ratio Curve Name

    HeatPump:PlantLoop:EIR:Cooling,
      Cooling Coil,            !- Name
      Cooling Coil Load Loop Intermediate Node,  !- Load Side Inlet Node Name
      Cooling Coil Load Loop Supply Outlet Node,  !- Load Side Outlet Node Name
      AirSource,               !- Condenser Type
      Outdoor Air Inlet Node,  !- Source Side Inlet Node Name
      Outdoor Air Outlet Node, !- Source Side Outlet Node Name
      Heating Coil,            !- Companion Heat Pump Name
      0.005,                   !- Load Side Reference Flow Rate {m3/s}
      20,                      !- Source Side Reference Flow Rate {m3/s}
      400000,                  !- Reference Capacity {W}
      3.5,                     !- Reference Coefficient of Performance {W/W}
      ,                        !- Sizing Factor
      CapCurveFuncTemp2,       !- Capacity Modifier Function of Temperature Curve Name
      EIRCurveFuncTemp2,       !- Electric Input to Output Ratio Modifier Function of Temperature Curve Name
      EIRCurveFuncPLR2;        !- Electric Input to Output Ratio Modifier Function of Part Load Ratio Curve Name

    !- control operation schemes for the corresponding plant loops of the heating and cooling coils.
    PlantEquipmentOperationSchemes,
      Heating Coil Load Loop Operation,  !- Name
      PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type
      Heating Coil Load Loop Heating Operation,  !- Control Scheme 1 Name
      AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name

    PlantEquipmentOperation:HeatingLoad,
      Heating Coil Load Loop Heating Operation,  !- Name
      0,                       !- Load Range 1 Lower Limit {W}
      10000000,                !- Load Range 1 Upper Limit {W}
      Heating Coil Load Loop Heating Equipment;  !- Range 1 Equipment List Name

    PlantEquipmentList,
      Heating Coil Load Loop Heating Equipment,  !- Name
      HeatPump:PlantLoop:EIR:Heating,  !- Equipment 1 Object Type
      Heating Coil;            !- Equipment 1 Name

    PlantEquipmentOperationSchemes,
      Cooling Coil Load Loop Operation,  !- Name
      PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type
      Cooling Coil Load Loop Cooling Operation,  !- Control Scheme 1 Name
      AlwaysOnSchedule;        !- Control Scheme 1 Schedule Name

    PlantEquipmentOperation:CoolingLoad,
      Cooling Coil Load Loop Cooling Operation,  !- Name
      0,                       !- Load Range 1 Lower Limit {W}
      10000000,                !- Load Range 1 Upper Limit {W}
      Cooling Coil Load Loop Cooling Equipment;  !- Range 1 Equipment List Name

    PlantEquipmentList,
      Cooling Coil Load Loop Cooling Equipment,  !- Name
      HeatPump:PlantLoop:EIR:Cooling,  !- Equipment 1 Object Type
      Cooling Coil;            !- Equipment 1 Name

### Gaps in modeling capability ###

**The air-to-water heat pump is one of the five types of heat pumps that can be modeled in EnergyPlus:**

- Air-to-air, using the AirLoopHVAC:UnitaryHeatPump:AirToAir object
- Water-to-air, using the AirLoopHVAC:UnitaryHeatPump:WaterToAir object
- Water-to-water, using either object:
- HeatPump:WaterToWater:EquationFit:Heating or Cooling
- HeatPump:WaterToWater:ParameterEstimation:Heating or Cooling
- Air-to-water, using the HeatPump:PlantLoop:EIR:Cooling and HeatPump:PlantLoop:EIR:Heating objects
- Air source integrated heat pump to provide space heating and water heating,
  using the CoilSystem:IntegratedHeatPump:AirSource object

The following table summarizes the input fields of the various heat pump models
in EnergyPlus. The "x" in each table cell indicates the field in the
corresponding row is present in the heat pump input object in the corresponding
column.

Table 2. Input fields of the heat pump objects in EnergyPlus

<img src="awhp-idd-cmp-1.png" alt="awhp-idd-cmp-1" width="800px">
</img>
<img src="awhp-idd-cmp-2.png" alt="awhp-idd-cmp-2" width="800px">
</img>

From the column title, we can see that all objects are marked with air to air,
water to air, water to water, except for the air to water one, which is called
HeatPump:PlantLoop:EIR:Cooling or HeatPump:PlantLoop:EIR:Cooling. This could
make it hard for users to search for the object.

In terms of the coverage of the input object, compared with the first two
columns, the air-to-water heat pump (column 4) has very few "x", indicating very
few input fields. Apart from the common object identifier and loop connections,
the inputs in the current air-to-water heat pump object focus on the COP and
power section. Other details are left out including: 
- Cannot specify parameters like single, multi-speed, or variable speed for capacity control 
- The naming of the object is not intuitive - it does not refer to water to water or air to
water, making searching for the object difficult (not intuitive to users)
- Missing the input of an availability schedule (which disables the HP for
seasonal considerations)

## Proposed Approach ##

This new feature proposes to create a new input object, HeatPump:AirToWater.

    HeatPump:AirToWater,
        \memo air-to-water heat pump system which provides either chilled or hot water with single- or variable speed compressors.
        \min-fields 56
      A1 , \field Name
        \required-field
        \type alpha
        \note Enter a unique name for this air-to-water heat pump
      A2 , \field Availability Schedule Name Heating
        \type object-list
        \object-list ScheduleNames
        \note Enter the name of a schedule that defines the availability of the unit in cooling mode
        \note Schedule values of 0 denote the unit is off. All other values denote the unit is available
        \note If this field is left blank, the unit is available the entire simulation
      A3 , \field Availability Schedule Name Cooling
        \type object-list
        \object-list ScheduleNames
        \note Enter the name of a schedule that defines the availability of the unit in cooling mode
        \note Schedule values of 0 denote the unit is off. All other values denote the unit is available
        \note If this field is left blank, the unit is available the entire simulation
      A4 , \field Operating Mode Control Method
        \type choice
        \key ScheduledModes
        \key EMSControlled
        \key Load
        \default Load
      A5 , \field Operating Mode Control Schedule Name
        \note This field is used if the control method is set to ScheduledModes
        \note Schedule values control operating mode: 0=off, 1=cooling, 2=heating
        \type object-list
        \object-list ScheduleNames
      N1 , \field Rated Heating Capacity
        \required-field
        \type real
        \units W
        \minimum> 0
        \note Heating capacity at the rated inlet air temperatures, rated condenser inlet
        \note water temperature, rated air flow rate, and rated water flow rate.
        \note Can optionally include condenser pump heat.
      N2, \field Reference COP for heating
        \type real
        \minimum> 0.0
        \units W/W
        \default 3.0
      N3 , \field Rated Inlet Air Temperature in Heating Mode
        \type real
        \units C
        \minimum> 5
        \default 8
        \note inlet air dry-bulb temperature corresponding to rated heat pump performance
        \note (capacity, COP, and SHR).
      N4 , \field Rated Air Flow Rate in Heating Mode
        \type real
        \units m3/s
        \minimum> 0
        \autosizable
        \default autosize
        \note air flow rate corresponding to rated heat pump performance
        \note (capacity, COP, and SHR).
      N5 , \field Rated Leaving Water Temperature in Heating Mode
        \type real
        \units C
        \minimum> 25
        \default 40
        \note outlet water temperature corresponding to rated heat pump performance
        \note (heating capacity, COP, and SHR).
      N6 , \field Rated Water Flow Rate in Heating Mode
        \type real
        \units m3/s
        \ip-units gal/min
        \minimum> 0
        \autosizable
        \default autosize
        \note Condenser water flow rate corresponding to rated heat pump performance
        \note (capacity, COP and SHR).
      N7, \field Minimum Outdoor Air Temperature in Heating Mode
        \type real
        \units C
        \default -10.0
        \note Enter the minimum outdoor temperature allowed for heating operation
        \note Heating is disabled below this temperature
      N8, \field Maximum Outdoor Air Temperature in Heating Mode
        \type real
        \units C
        \default 16
        \note Enter the maximum outdoor temperature allowed for heating operation
        \note Heating is disabled below this temperature
      N9, \field Sizing Factor for Heating
        \note Multiplies the autosized capacity and flow rates
        \type real
        \minimum> 0.0
        \default 1.0
      N10 , \field Rated Cooling Capacity
        \required-field
        \type real
        \units W
        \minimum> 0
        \note Cooling capacity at the rated inlet air temperatures, rated condenser inlet
        \note water temperature, rated air flow rate, and rated water flow rate.
      N11, \field Reference COP for cooling
        \type real
        \minimum> 0.0
        \units W/W
        \default 3.0
      N12, \field Rated Inlet Air Temperature in Cooling Mode
        \type real
        \units C
        \minimum> 5
        \default 30
        \note inlet air dry-bulb temperature corresponding to rated coil performance
        \note (capacity, COP, and SHR).
      N13 , \field Rated Air Flow Rate in Cooling Mode
        \type real
        \units m3/s
        \minimum> 0
        \autosizable
        \default autosize
        \note air flow rate corresponding to rated heat pump performance
        \note (capacity, COP, and SHR).
      N14 , \field Rated Leaving Water Temperature in Cooling Mode
        \type real
        \units C
        \minimum> 0
        \default 8
        \note inlet water temperature corresponding to rated coil performance
        \note (heating capacity, COP, and SHR).
      N15 , \field Rated Water Flow Rate in Cooling Mode
        \type real
        \units m3/s
        \ip-units gal/min
        \minimum> 0
        \autosizable
        \default autosize
        \note Condenser water flow rate corresponding to rated heat pump performance
        \note (capacity, COP, and SHR).
      N16 , \field Minimum Outdoor Air Temperature in Cooling Mode
        \type real
        \units C
        \default -6.0
        \note Enter the minimum outdoor temperature allowed for cooling operation
        \note Cooling is disabled below this temperature
      N17, \field Maximum Outdoor Air Temperature in Cooling Mode
        \type real
        \units C
        \default 43.0
        \note Enter the maximum outdoor temperature allowed for cooling operation
        \note Cooling is disabled above this temperature
      N18, \field Sizing Factor for Cooling
        \note Multiplies the autosized capacity and flow rates
        \type real
        \minimum> 0.0
        \default 1.0
      A6 , \field Air Inlet Node Name
        \required-field
        \type node
        \note The node from which the DX coil draws its inlet air.
      A7 , \field Air Outlet Node Name
        \required-field
        \type node
        \note The node to which the DX coil sends its outlet air.
      A8 , \field Hot Water Inlet Node Name
        \required-field
        \type node
        \note The node connects to the hot water loop
      A9 , \field Hot Water Outlet Node Name
        \required-field
        \type node
        \note The node connects to the hot water loop
      A10 , \field Chilled Water Inlet Node Name
        \required-field
        \type node
        \note The node connects to the chilled water loop
      A11 , \field Chilled Water Outlet Node Name
        \required-field
        \type node
        \note The node connects to the chilled water loop
      A12, \field Heat Pump Defrost Control
        \type choice
        \key None
        \key Timed
        \key OnDemand
        \key TimedEmpirical
      \note A blank field is the same as None.
      N19, \field Defrost Time Period Fraction
        \type real
        \minimum 0.0
        \default 0.058333
        \note Fraction of time in defrost mode
        \note only applicable if timed defrost control is specified
      N20, \field Resistive Defrost Heater Capacity
        \type real
        \minimum 0.0
        \default 0.0
        \autosizable
        \units W
        \note only applicable if resistive defrost strategy is specified
        \ip-units W
      A13, \field Defrost Energy Input Ratio Function of Temperature Curve Name
        \type object-list
        \object-list BivariateFunctions
        \note univariate curve = a + b*OAT is typical, other univariate curves may be used
        \note bivariate curve = a + b*WB + c*WB**2 + d*OAT + e*OAT**2 + f*WB*OAT
        \note OAT = outdoor air dry-bulb temperature (C)
        \note WB = wet-bulb temperature (C) of air entering the indoor coil
        \note only required if Timed or OnDemand defrost strategy is specified
      A14, \field Defrost Capacity Ratio Function of Temperature Curve Name
        \type object-list
        \object-list BivariateFunctions
        \note defrost capacity could degrade with extreme temperature. This field adjusts the defrost capacity according to air and water temperature.
      N21, \field Compressor Multiplier
        \type integer
        \default 1
        \note intend to model modular compressors
      A15, \field Control Type
        \type choice
        \key FixedSpeed
        \key VariableSpeed
      N22, \field Crankcase Heater Capacity
        \type real
        \minimum 0
        \default 0
        \units W
        \note The compressor crankcase heater only operates when the dry-bulb temperature of air
        \note surrounding the compressor is below the Maximum Ambient Temperature for Crankcase
        \note Heater Operation and the DX coil is off. The ambient temperature surrounding the
        \note compressor is set by the WaterHeater:HeatPump parent object (field Compressor Location).
      A16, \field Crankcase Heater Capacity Function of Temperature Curve Name
        \note A Curve:* or Table:Lookup object encoding the relationship between
        \note the crankcase heater capacity and the outdoor air temperature. When this field is
        \note missing or empty, constant crankcase heater capacity will be assumed.
        \type object-list
        \object-list UnivariateFunctions
      N23, \field Maximum Ambient Temperature for Crankcase Heater Operation
        \type real
        \minimum 0
        \default 10
        \units C
        \note The compressor crankcase heater only operates when the dry-bulb temperature of air
        \note surrounding the compressor is below the Maximum Outdoor Temperature for Crankcase
        \note Heater Operation and the unit is off.
      N24, \field Number of Speeds for Heating
        \type integer
        \note the number of speed levels in heating mode
        \maximum 5
        \default 1
        \note if there's only cooling component, set this field to 0
      A17, \field Normalized Heating Capacity Function of Temperature Curve Name at Speed 1
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A18, \field Heating Energy Input Ratio Function of Temperature Curve Name at Speed 1
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A19, \field Heating Energy Input Ratio Function of PLR Curve Name at Speed 1
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A20, \field Normalized Heating Capacity Function of Temperature Curve Name at Speed 2
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A21, \field Heating Energy Input Ratio Function of Temperature Curve Name at Speed 2
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A22, \field Heating Energy Input Ratio Function of PLR Curve Name at Speed 2
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A23, \field Normalized Heating Capacity Function of Temperature Curve Name at Speed 3
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A24, \field Heating Energy Input Ratio Function of Temperature Curve Name at Speed 3
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A25, \field Heating Energy Input Ratio Function of PLR Curve Name at Speed 3
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A26, \field Normalized Heating Capacity Function of Temperature Curve Name at Speed 4
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A27, \field Heating Energy Input Ratio Function of Temperature Curve Name at Speed 4
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A28, \field Heating Energy Input Ratio Function of PLR Curve Name at Speed 4
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A29, \field Normalized Heating Capacity Function of Temperature Curve Name at Speed 5
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A30, \field Heating Energy Input Ratio Function of Temperature Curve Name at Speed 5
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A31, \field Heating Energy Input Ratio Function of PLR Curve Name at Speed 5
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      N25, \field Number of Speeds for Cooling
        \type integer
        \maximum 5
        \default 1
        \note the number of speed levels in cooling mode
        \note if there's only heating component, set this field to 0
      A32, \field Normalized Cooling Capacity Function of Temperature Curve Name at Speed 1
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A33, \field Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 1
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A34, \field Cooling Energy Input Ratio Function of PLR Curve Name at Speed 1
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A35, \field Normalized Cooling Capacity Function of Temperature Curve Name at Speed 2
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A36, \field Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 2
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A37, \field Cooling Energy Input Ratio Function of PLR Curve Name at Speed 2
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A38, \field Normalized Cooling Capacity Function of Temperature Curve Name at Speed 3
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A39, \field Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 3
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A40, \field Cooling Energy Input Ratio Function of PLR Curve Name at Speed 3
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A41, \field Normalized Cooling Capacity Function of Temperature Curve Name at Speed 4
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A42, \field Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 4
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A43, \field Cooling Energy Input Ratio Function of PLR Curve Name at Speed 4
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.
      A44, \field Normalized Cooling Capacity Function of Temperature Curve Name at Speed 5
        \type object-list
        \object-list BivariateFunctions
        \note: CAPFT - Normalized Capacity Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A45, \field Cooling Energy Input Ratio Function of Temperature Curve Name at Speed 5
        \type object-list
        \object-list BivariateFunctions
        \note EIRFT - Fuel Energy Input Ratio Function of Temperature Curve Name,
        \note which is a biquadratic curve or a lookup table of air and water temperature.
      A46; \field Cooling Energy Input Ratio Function of PLR Curve Name at Speed 5
        \type object-list
        \object-list UnivariateFunctions
        \note EIRFPLR - Fuel Energy Input Ratio Function of Part Load Ratio(PLR) Curve Name,
        \note which is a cubic curve or a lookup table.

### Actuators to enable defrost control through EnergyPlus Python API ###

The following actuators will be added to enable various defrost control strategies using the EnergyPlus API:
- heat pump operation mode, Mode
- compressor suction temperature, Tsuc
- entering water temperature, enteringTemp
- time since last defrost, timeSinceLast
- leaving water temperature, leavingTemp
- time since defrost started, timeSinceStart

Note that ambient air temperature is already an existing EnergyPlus actuator. It could be used in defrost calculation as well.

## Engineering reference highlights of the new object ##

The AWHP is a 2-pipe unit that transfers thermal energy from the outside to a
water loop. It does not provide simultaneous hot and chilled water; instead, it
switches between cooling and heating modes based on the temperature requirements
of the connected loop.

In cooling mode, it functions like an air-cooled chiller, using the
refrigeration cycle to absorb heat from the water loop and dissipate it to the
ambient air. Conversely, in heating mode, it acts as a heat pump, absorbing heat
from the ambient air and transferring it to the water loop.

Depending on the application, the air-to-water heat pump can be assigned to a
chilled water loop, a heating hot water loop, a domestic hot water loop, or a
condenser water loop.

In this proposed object:
- The source side is connected to an outdoor air node
- The heating load side can connect to a hot water loop, a domestic hot water loop, or a condenser water loop
- The cooling load side can connect to a chilled water loop or a condenser water loop

If the heating (or cooling) load side node names are left empty, the air-to-water heat pump will only produce chilled water (or hot water).

### Performance calculations ###

Similar to the existing AWHP objects (i.e. the HeatPump:PlantLoop:EIR:Cooling
and HeatPump:PlantLoop:EIR:Heating), the new object also characterizes the
heating/cooling capacity and electricity consumption for different temperatures
and part load ratios. However, the new object will have more than one group of
such curves to model air-to-water heat pumps with fixed-speed compressors and
inverter-driven compressors.

#### Heating mode ####

The heating capacity at speed level k is derived from the nominal (rated) capacity and a normalized capacity function of temperatures in Equation (1).

$$
Capacity_{heating,k}=Capacity_{heating,rated}\times CAPFT_{heating,k}(T_{amb},T_{leaving}) (1)
$$

Where $Capacity_{heating,k}$ is the heating capacity at speed level $k$.
Capacityheating,rated is the rated capacity for heating. $T_{amb}$ is the ambient air
temperature. Tleaving is the load side leaving water temperature $CAPFT_{heating,k}$
is a normalized bivariate function characterizing the relationship between
capacity and two temperature inputs: the ambient air temperature and the leaving
water temperature. It is derived based on manufacturing data.

The heating electricity use of the heat pump is calculated using the rated EIR,
the EIR adjustment function, and the EIR part load ratio adjustment curve, as is
shown in equation (2).

$$
Electricity_{heating,k}=EIR_{heating,rated} \times EIRFT_{heating,k}(T_{amb}, T_{leaving}) \times EIRPLR_{heating,k}(PLR)       (2)
$$

$Electricity_{heating,k}$ is the electricity usage for heating at speed level k.
$EIR_{heating,rated}$ is the rated heating energy input ratio. The
$EIRFT_{heating,k}(\cdot, \cdot)$ is a normalized bivariate function adjusting
the EIR relative to the given ambient temperature and leaving water temperature,
at speed level k. $EIRPLR_{heating,k}(\cdot)$ is a normalized univariate
function adjusting the EIR based on the part load ratio.

#### Cooling Mode ####

The cooling capacity at speed level k is derived from the nominal (rated)
capacity and a normalized capacity function of temperatures in Equation (3).

$$
Capacity_{cooling,k}=Capacity_{cooling,rated} \times CAPFT_{cooling,k}(T_{amb},T_{leaving}) (3)
$$

Where $Capacity_{cooling,k}$ is the cooling capacity at speed level k.
$Capacity_{cooling,rated}$ is the rated capacity for cooling. $T_{amb}$ is the ambient air
temperature. $T_leaving$ is the load side leaving water temperature $CAPFT_{cooling,k}$
is a normalized bivariate function characterizing the relationship between
capacity and two temperature inputs: the ambient air temperature and the leaving
water temperature. It is derived based on manufacturing data.

The cooling electricity use of the heat pump is calculated using the rated EIR,
the EIR adjustment function, and the EIR part load ratio adjustment curve, as is
shown in equation (4).

$$
Electricity_{cooling,k}=EIR_{cooling,rated} \times EIRFT_{cooling,k}(T_{amb}, T_{leaving}) \times EIRPLR_{cooling,k}(PLR)       (4)
$$

$Electricity_{cooling,k}$ is the electricity usage for cooling at speed level k.
$EIR_{cooling,rated}$ is the rated cooling energy input ratio. The
$EIRFT_{cooling,k}(\cdot, \cdot)$ is a normalized bivariate function adjusting
the EIR relative to the given ambient temperature and leaving water temperature,
at speed level k. $EIRPLR_{cooling,k}(\cdot)$ is a normalized univariate
function adjusting the EIR based on the part load ratio.

### Defrost ###

#### Defrost strategy ####

Two defrost strategies are allowed: ReverseCycle or Resistive. If the
ReverseCycle strategy is selected, the heating cycle is reversed periodically to
provide heat to melt frost accumulated on the outdoor coil. If a Resistive
defrost strategy is selected, the frost is melted using an electric resistance
heater.

#### Defrost control ####

As the new heat pump object is performance-curve-based and does not model
refrigerant pressure, the object cannot model products with refrigerant pressure
as control targets.

The defrost control of the new object will resemble the HeatPump:PlantLoop:EIR:*
object, with three options: Timed, OnDemand, or TimedEmpirical.

- when Timed or OnDemand is selected, defrost power is calculated as shown in equation (5)

$$
Power_{defrost} = EIRFT_{defrost}(Capacity_{heating,rated}/1.01667) \times fracDefrostTime (5)
$$

$Power_{defrost}$ is the power usage of defrosting. $EIRFT_{defrost}(\cdot)$ is
from user input "Defrost Energy Input Ratio Function of Temperature Curve" if
it's present or 1/COP if it's blank. $Capacity_{heating,rated}$ is the "Rated Heating
Capacity" input field. "Timed" and "OnDemand" differ in the fractional Defrost
Time (fracDefrostTime). For "Timed", it gets value from the user input "Heat
Pump Defrost Time Period Fraction". For "OnDemand", it is calculated as

$$
fracDefrostTime =1.0 / (1.0 + 0.01446 / OutdoorCoildw)                  (6)
$$

Where

$$
OutdoorCoildw=HumidityRatio_{outdoor} - HumidityRatio_{saturated}              (7)
$$

$HumidityRatio_{saturated}$ is calculated using outdoor temperature and outdoor air barometric pressure

- when TimedEmpirical is selected, defrost power is calculated as shown in equation (8)

$$
Power_{defrost}=Capacity_{heating,rated}/COP_{heating,ref} \times
defrostHeatEnergyFraction * DefrostCycles                                           (8)
$$

where defrostHeatEnergyFraction is based on the user input curve "Timed
Empirical Defrost Heat Input Energy Fraction Curve". $COP_{heating,ref}$ is the
reference heating COP. The DefrostCycles is the number of defrost cycles in an
hour. It is determined by the "Timed Empirical Defrost Frequency Curve", part
load ratio, and cycling ratio.

The following actuators will be added to enable more flexible custom defrost controls:
- heat pump operation mode, Mode
- compressor suction temperature, Tsuc
- entering water temperature, enteringTemp
- time since last defrost, timeSinceLast
- leaving water temperature, leavingTemp
- time since defrost started, timeSinceStart

### Temperature and Flow Calculations ###

#### Heating mode ####

Initially, the flow rate of the air and water side will both be rated flow rate.
The air side inlet temperature will be the ambient outdoor temperature. The
waterside outlet temperature will be assumed to reach the leaving water
temperature setpoint.

With this information, for a series of fixed-speed compressors, the available
capacity for the kth fixed-speed compressor can be calculated using eq (9).

$$
Capacity_{avail,heating,k}=Capacity_{heating,rated}CAPFT_{heating,k}(T_{air,in},T_{leavingWaterSetpoint}) (9)
$$

The appropriate number of fixed-speed compressors, a, is selected such that

$$
\sum_{i=1}^{a} Capacity_{avail,heating,i} < Demand_{plantLoop} \leq \sum_{i=1}^{a+1}Capacity_{avail,heating,i} (10)
$$

The cycling ratio of the (a+1)th compressor will be calculated

$$
CyclingRatio = (Demand_{plantLoop} - \sum_{i=1}^{a} Capacity_{avail,heating,i}) / Capacity_{avail,heating,a+1} (11)
$$

When the plant loop demand does not exceed the available capacity, load-side
actual heat transfer just equals the plant loop demand.

$$
Q_{load}=Demand_{plantLoop}                                                                                       (12)
$$

When the plant loop demand is larger than the available capacity, then the actual load transfer is

$$
Q_{load}=\sum_{i=1}^{maxSpeedLevel} Capacity_{avail,heating,i}                                                 (13)
$$

where maxSpeedLevel is the maximum number of compressor speed levels.

The heating mode load-side (water) outlet temperature will be

$$
T_{out,load}=T_{in,load} + Q_{load}/(m_{load}C_{load})                                                         (14)
$$

Where $T_{in,load}$ is the load side inlet temperature. $m_{load}$ is the load
side mass flow rate. $C_{load}$ is the load-side specific heat.

Power is evaluated using equation (2).

In heating mode, source-side heat transfer will be

$$
Q_{source}=Q_{load} - Power                                                                                     (15)
$$

The source-side outlet temperature is thus

$$
T_{out,source} = T_{in,source}-Q_{source}/(m_{source}C_{source})                                                  (16)
$$

For a variable-speed compressor, the available capacity for the speed level k
can be calculated using eq (1) as well. The appropriate compressor speed is
calculated by interpolation between $Capacity_{avail,heating,a}$ and
$Capacity_{avail,heating,a+1}$ such that
$Capacity_{avail,heating,a} < Demand_{plantLoop} \leq Capacity_{avail,heating,a+1}$

When the plant loop demand is larger than the available capacity, then the
actual load transfer is $Q_{load}=Capacity_{avail,heating,maxSpeedLevel}$. The
other calculations are the same as the fixed-speed compressors.

#### Cooling mode ####

The available cooling capacity at speed-level k can be calculated using eq (17).

$$
Capacity_{avail,cooling,k}=Capacity_{cooling,rated} \times CAPFT_{cooling,k}(T_{air,in},T_{leavingWaterSetpoint}) (17)
$$

The appropriate number of fixed-speed compressors, a, is selected such that

$$
\sum_{i=1}^{a}Capacity_{avail,cooling,i} < Demand_{plantLoop} \leq \sum_{i=1}^{a+1}Capacity_{avail,cooling,i}     (18)
$$

The cyclingCycling ratio of the a+1 stage compressor will be calculated

$$
CyclingRatio = (Demand_{plantLoop} - \sum_{i=1}^{a}Capacity_{avail,heating,i}) / Capacity_{avail,heating,a+1} (19)
$$

when the plant loop demand does not exceed the available capacity, load-side
actual heat transfer just equals the plant loop demand.

$$
Q_{load}=Demand_{plantLoop}                                                                                        (20)
$$

When the plant loop demand is larger than the available capacity, then the actual load transfer is

$$
Q_{load}=\sum_{i=1}^{maxSpeedLevel}Capacity_{avail,cooling,i}                                                      (21)
$$

The cooling mode load-side (water) outlet temperature will be

$$
T_{out,load}=T_{in,load}-Q_{load}/(m_{load}C_{load})                                                               (22)
$$

Where $T_{in,load}$ is the load side inlet temperature. mload is the load side
mass flow rate. Cload is the load-side specific heat.

Power is evaluated using equation (2).

In cooling mode, source-side heat transfer will be

$$
Q_{source}=Q_{load} + Power                                                                                         (23)
$$

The source-side outlet temperature is thus

$$
T_{out,source}=T_{in,source} + Q_{source}/(m_{source}C_{source})                                                    (24)
$$

#### Crankcase heater ####

When the outdoor air dry-bulb temperature is below the value specified in the
input field Maximum Outdoor Dry-bulb Temperature for Crankcase Heater Operation,
the crankcase heater is enabled during the time that the compressor is not
running. The capacity is specified in the input field "crankcase heater
capacity". If "Crankcase Heater Capacity Function of Temperature Curve Name" is
defined, a multiplier is applied to adjust the crankcase heater capacity based
on the ambient temperature condition.

### Outputs ###

The following output variables will be produced
- HVAC,Average,Heat Pump Operating Mode \[\]
- HVAC,Average,Heat Pump Total Cooling Rate \[W\]
- HVAC,Average,Heat Pump Total Heating Rate \[W\]
- HVAC,Average,Heat Pump Cooling COP \[\]
- HVAC,Average,Heat Pump Heating COP \[\]
- HVAC,Average,Heat Pump COP \[\]
- HVAC,Average,Heat Pump Part Load Ratio  \[\]
- HVAC,Average,Heat Pump Cycling Ratio  \[\]
- HVAC,Average,Heat Pump Electricity Rate \[W\]
- HVAC,Sum,Heat Pump Electricity Energy \[J\]
- HVAC,Average,Heat Pump Load Side Heat Transfer Rate \[W\]
- HVAC,Sum,Heat Pump Load Side Heat Transfer Energy \[J\]
- HVAC,Average,Heat Pump Source Side Heat Transfer Rate \[W\]
- HVAC,Sum,Heat Pump Source Side Heat Transfer Energy \[J\]
- HVAC,Average,Heat Pump Load Side Inlet Temperature \[$^\circ$C\]
- HVAC,Average,Heat Pump Load Side Outlet Temperature \[$^\circ$C\]
- HVAC,Average,Heat Pump Source Side Inlet Temperature \[$^\circ$C\]
- HVAC,Average,Heat Pump Source Side Outlet Temperature \[$^\circ$C\]
- HVAC,Average,Heat Pump Load Side Mass Flow Rate \[kg/s\]
- HVAC,Average,Heat Pump Source Side Mass Flow Rate \[kg/s\]
- HVAC,Average,Heat Pump Load Due To Defrost  \[W\]
- HVAC,Average,Heat Pump Fractional Defrost Time  \[hr\]
- HVAC,Average,Heat Pump Defrost Electricity Rate  \[W\]
- HVAC,Average,Heat Pump Defrost Electricity Energy  \[J\]
- HVAC,Average,Heat Pump Crankcase Heater Electricity Rate \[W\]
- HVAC,Sum,Heat Pump Crankcase Heater Electricity Energy \[J\]

In the EquipmentSummary report in the html output table

- An entry will be added to the "central plant" table to display the overall
  capacity, efficiency, and IPLV of the air-to-water heat pump.
- A table showing the following rating information of the air-to-water heat pump
  - Type (fixed-speed, variable-speed)
  - Rated heating (cooling) capacity
  - Rated heating (cooling) COP
  - SEER
  - HSPF
  - Ambient air temperature at rated condition in heating (cooling) mode
  - Entering water temperature at rated condition in heating (cooling) mode
  - Leaving water temperature at rated condition in heating (cooling) mode
  - Rated air flow rate in heating (cooling) mode
  - Rated water flow rate in heating (cooling) mode
  - Plantloop names
  - Branch names

### Notes on Selecting the Appropriate Object ###

To model an air-to-water heat pump in EnergyPlus, there are a few approaches to consider:

- Using the proposed HeatPump:AirToWater
- Using HeatPump:PlantLoop:EIR:Heating and HeatPump:PlantLoop:EIR:Cooling
- Using a heat pump water heater indirectly

*Approach 1: HeatPump:AirToWater*

This approach is preferred when the target heat pump product is variable-speed
or multi-speed, or when the system includes multiple copies of the same
compressors to provide larger capacities in a modular fashion. Additionally,
this approach offers a list of defrost-related actuators to enable more complex
defrost controls.

*Approach 2: HeatPump:PlantLoop:EIR:Heating and HeatPump:PlantLoop:EIR:Cooling*

This approach should be selected if heat recovery modeling is required, as
Approach 1 does not currently support this feature (though it will be added in
the future). For users needing to model heat recovery, Approach 2 is the
recommended choice.

*Approach 3: Indirect Use of a Heat Pump Water Heater*

This approach is generally not recommended due to its complexity and the higher
likelihood of configuration errors.

## Testing/Validation/Data Source(s) ##

The feature will be tested and demonstrated with a test file derived from
*5ZoneWaterSystems.idf* with an added air-to-water heat pump object developed in
this feature. The zone will have a Coil:Cooling:Water and a Coil:Heating:Water
as terminal units receiving the hot and cold water from the air-to-water heat
pump. A more complex test case will be built with a cascading system with a
combination of air-to-water, water-to-water, and water-to-air heat pumps.
Verification of the model will be done through manual inspection of time-series
simulation results in an Excel spreadsheet.

## Acknowledgments ##

Paul Hydukovich of Daikin Applied and Dr. Bo Shen of ORNL provided valuable inputs.

## References ##

[1] https://www.gminsights.com/industry-analysis/air-to-water-heat-pump-market
[2] https://www.daikin.eu/en_us/products/product.html/EWYT-CZP.html

