Input Changes
=============

This file documents the structural changes on the input of EnergyPlus that could affect interfaces, etc.
This was previously an Excel workbook that made for very difficult version control, especially during busy times around code freezes.

# Object Change: `ZoneHVAC:EquipmentList`

Field 1 remains the same.
After F1, insert new alpha input choice field (A2) 'Load Distribution Scheme' with default key input value of 'SequentialLoad'.
Shift all later fields down by 1. The old input fields F2 - Fn become the new F3-Fn+1. The new input field becomes F2.

# Object Change: Schedule:Day:Interval

Field A3 Interpolate to Timestep, change value of "Yes" to "Average"

# Object Change: Schedule:Day:List

Field A3 Interpolate to Timestep, change value of "Yes" to "Average"

# Object Change: Schedule:Compact

For any field A3 and later, change values of "Interpolate:Yes" to "Interpolate:Average"

# Object Change: ElectricEquipment:ITE:AirCooled
Fields 1 and 2 remain tha same
After F2, insert new alpha input choice field (A3) 'Air Flow Calculation Method' with default kay value 'FlowFromSystem'.
Shift all later fields down by 1. The old input fields F3 - Fn become the new F4-Fn+1. The new input field becomes F3.
At the end, there are four new fields, but these don't need to be added - they are required only when using the new option
introduced in new field A3.


# Transitioning "AirTerminal:SingleDuct:Uncontrolled" to "AirTerminal:SingleDuct:ConstantVolume:NoReheat". Require five steps:
*Note: these rules were not implemented in Transition8-8-0-to-8-9-0. AirTerminal:SingleDuct:Uncontrolled was simply tagged as obsolete.*

# (1) Object Change: ZoneHVAC:EquipmentList

Field F3(A3): Zone Equipment 1 Object Type, replace "AirTerminal:SingleDuct:Uncontrolled" with "ZoneHVAC:AirDistributionUnit"

Field F4(A4): Zone Equipment 1 Name, create a unique new name. The new name will be created based on the name of "AirTerminal:SingleDuct:Uncontrolled" object". "AirTerminal:SingleDuct:Uncontrolled" object Name + "ADU" -> "SPACE3-1 Direct Air ADU"

save the new name of the "ZoneHVAC:AirDistributionUnit" object

save the "AirTerminal:SingleDuct:Uncontrolled" object Name for later use ( will be used as name for new air terminal no reheat object)

Field F5(N1) - F6(N2), no change.


# (2) Add Object: ZoneHVAC:AirDistributionUnit

Field F1(A1): Name, insert the unique name created for "ZoneHVAC:AirDistributionUnit" in STEP 1 above, example "SPACE3-1 Direct Air ADU"

Field F2(A2): Air Distribution Unit Outlet Node Name, use Zone Supply Air Node Name input field (A3) value of the "AirTerminal:SingleDuct:Uncontrolled" object

Field F3(A3): Air Terminal Object Type, insert "AirTerminal:SingleDuct:ConstantVolume:NoReheat"

Field F4(A4): Air Terminal Object Type, insert the Name of the "AirTerminal:SingleDuct:Uncontrolled" object, example "SPACE3-1 Direct Air"

Field >=F5(N1): Leave blank. Will use default.



# (3) Modify Object: AirTerminal:SingleDuct:Uncontrolled

#### replace the "AirTerminal:SingleDuct:Uncontrolled" object with "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object

Field F1(A1): Name, no change

Field F2(A2): Availability Schedule Name, no change.

Field F3(A3): Air Outlet Node Name, no change.

Field F4(A4): Air Inlet Node Name, create a new inlet node name using the Field F3(A3) value above

          Field F4(A4 = Field F3(A3) + "2AT" -> example, "SPACE3-1 Supply Inlet 2AT",

          save this new inlet node name for use as the "AirLoopHVAC:ZoneSplitter" or "AirLoopHVAC:SupplyPlenum" outlet node name

Field F5(N1): Maximum Air Flow Rate {m3/s}, no change.

Field F6(A5): Design Specification Outdoor Air Object Name, no change.

Field F7(A6) : Per Person Ventilation Rate Mode, no change.


# (4) Modify Object: AirLoopHVAC:ZoneSplitter

Field F1(A1): Name, no change.

Field F2(A2): Inlet Node Name, no change

(4) modify one of the "Outlet X Node Name" of the "AirLoopHVAC:ZoneSplitter" if it matches the Zone Supply Air Node Name input field value of current "AirTerminal:SingleDuct:Uncontrolled" object

    use the new Air Inlet Node Name created in Step 3 above for "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object

Field >=F3: Outlet X Node Name, modify the matching Outlet X Node Name, should be the same node name as the new Air Inlet Node Name of the "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object, old name + 2AT -> example, "SPACE3-1 Supply Inlet 2AT"

# (5) Modify Object: AirLoopHVAC:SupplyPlenum

    Step 5 is not required if Step 4 is successful. If not do the following:

Field F1(A1): Name, no change.

Field F2(A2): Zone Name, no change

Field F3(A3): Zone Node Name, no change

Field F4(A4): Inlet Node Name, no change

    modify one of the "Outlet X Node Name" of the "AirLoopHVAC:SupplyPlenum" that matches the Zone Supply Air Node Name input field value of current "AirTerminal:SingleDuct:Uncontrolled" object

    use the new Air Inlet Node Name created in Step 3 above for "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object.

Field >=FX(AX): Outlet X Node Name, modify the matching Outlet X Node Name, should be the same node name as the new Air Inlet Node Name of "AirTerminal:SingleDuct:ConstantVolume:NoReheat" object, old name + 2AT -> example, "SPACE3-1 Supply Inlet 2AT"

# Object Change: Boiler and Chiller flow mode synonyms
For all Boiler:HotWater and all Chiller:\*

Field: Flow Mode, change VariableFlow to LeavingSetpointModulated.

# Object Change: HeatBalanceAlgorithm
Field 1, Algorithm:

a) change "DEFAULT" or "CTF" to "ConductionTransferFunction"

b) change "EMPD" to "MoisturePenetrationDepthConductionTransferFunction"

c) change "CONDFD" or "CONDUCTIONFINITEDIFFERENCEDETAILED" to "ConductionFiniteDifference"

d) change "HAMT" to "CombinedHeatAndMoistureFiniteElement"

# Object Change: ZoneAirHeatBalanceAlgorithm
Field 1, Algorithm, change "3RDORDERBACKWARDDIFFERENCE" to "ThirdOrderBackwardDifference"

# Object Change: Zone
Field 12 Zone Outside Convection Algorithm, change "DOE2" to "DOE-2"

# Object Change: Generator:WindTurbine
Field 3 Rotor Type:

a) change "HAWT" or "None" to "HorizontalAxisWindTurbine"

b) change "VAWT" to "VerticalAxisWindTurbine"

Field 4 Control Type:

a) change "FSFP" to "FixedSpeedFixedPitch"

b) change "FSVP" to "FixedSpeedVariablePitch"

c) change "VSFP" to "VariableSpeedFixedPitch"

d) change "VSVP" or "None" to "VariableSpeedVariablePitch"

# Object Change: SizingPeriod:DesignDay
Field 18, Rain Indicator
Field 19, Snow Indicator
Field 20, Daylight Saving Time Indicator

a) change "0" to "No"

b) change "1" to "Yes"

# Object Change: Output:Constructions
Fields 1 and 2

a) change "Construc\*" to "Constructions"

b) change "Mat\*" to "Materials"

# Object Change: Fuel type synonyms
For Boiler:Steam, WaterHeater:\* , Generator:\* , FuelFactors, ChillerHeater:Absorption:DirectFired, AirConditioner:VariableRefrigerantFlow,
LifeCycleCost:UsePriceEscalation and other objects with a fuel type field

Field: Fuel Type (or similar)


a) change "Gas" or "Natural Gas" to "NaturalGas"

b) change "Electric" or "Elec" to "Electricity"

c) change "Fuel Oil #1" or "Fuel Oil" or "Distillate Oil" to "FuelOil#1"

d) change "Fuel Oil #2" or "Residual Oil" to "FuelOil#2"

e) change "Propane" or "LPG" or "Propane Gas" to "PropaneGas"

*Note for some objects, these synonyms for fuel types remained valid thru v9.2 due to modifications to the JSON schema, this includes: FuelFactors, AirConditioner:VariableRefrigerantFlow,
WaterHeater:Mixed (Heater Fuel Type only), and Boiler:HotWater. So these transition rules appear again for v9.3 along with additional rules to standardize fule types across all objects.*

# Object Change: GroundHeatExchanger:Vertical

Old GroundHeatExchanger:Vertical becomes four new objects:
GroundHeatExchanger:System
GroundHeatExchanger:Vertical:Properties
Site:GroundTemperature:Undisturbed:KusudaAchenbach
GroundHeatExchanger:ResponseFactors

*All OLD fields below are incoming from the original GroundHeatExchanger:Vertical object.*

## 1. New GroundHeatExchanger:System object

Fields F1:F4, no change (from

Insert new F5 = "Site:GroundTemperature:Undisturbed:KusudaAchenbach"

New field F6 = <object-name old F1> + " Ground Temps"

New fields F7:F8 = old F8:F9

New field F9 = <object-name old F1> + " Response Factors"

*End of new GroundHeatExchanger:System object.*

## 2. New GroundHeatExchanger:Vertical:Properties object

*Old F fields are still incoming from old GroundHeatExchanger:Vertical object.*

New field F1 = <object-name old F1> + " Properties"

New F2 = "1"

New F3 = old F6

New F4 = old F7 * 2.0

New F5 = old F11

New F6 = "3.90E+06"

New F7 = old F12

New F8 = "1.77E+06"

New F9 = old F13

New F10 = old F15

New F11 = old F14

*End of new GroundHeatExchanger:Vertical:Properties object.*

## 3. New Site:GroundTemperature:Undisturbed:KusudaAchenbach object

*Old F fields are still incoming from old GroundHeatExchanger:Vertical object.*

New field F1 = <object-name old F1> + " Ground Temps"

New F2 = old F8

New F3 = 920

New F4 = old F9 / 920

New F5 = old F10

New F6 = 3.2

New F7 = 8

*End of new Site:GroundTemperature:Undisturbed:KusudaAchenbach object.*

## 4. New Site:GroundHeatExchanger:ResponseFactors object

*Old F fields are still incoming from old GroundHeatExchanger:Vertical object.*

New field F1 = <object-name old F1> + " Response Factors"

New F2 = <object-name old F1> + " Properties"

New F3 = old F5

New F4 = old F17

New F5:end = old F19:end

*End of new Site:GroundHeatExchanger:ResponseFactors object.*

